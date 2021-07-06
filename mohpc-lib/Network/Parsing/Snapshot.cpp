#include <MOHPC/Network/Parsing/Snapshot.h>
#include <MOHPC/Network/Parsing/Entity.h>
#include <MOHPC/Network/Parsing/PlayerState.h>
#include <MOHPC/Network/Types/Snapshot.h>
#include <MOHPC/Network/Types/GameState.h>
#include <MOHPC/Network/Types/Reliable.h>
#include <MOHPC/Network/Serializable/Entity.h>
#include <MOHPC/Network/Serializable/EntityField.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Common/Log.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::Parsing;

constexpr char MOHPC_LOG_NAMESPACE[] = "parsing_snapshot";

class SnapshotBase : public Parsing::ISnapshot
{
public:
	SnapshotBase()
		: entityParser(nullptr)
		, playerStateParser(nullptr)
	{
	}

	void parseSnapshot(
		uint64_t currentTime,
		MSG& msg,
		const gameState_t& gameState,
		ICommandSequence* serverCommands,
		rawSnapshot_t& rawSnapshot,
		snapshotParm_t& snapshotParm,
		uint32_t serverMessageSequence
	) const override
	{
		rawSnapshot.serverCommandNum = serverCommands->getCommandSequence();

		rawSnapshot.serverTime = msg.ReadUInteger();
		rawSnapshot.serverTimeResidual = msg.ReadByte();

		// Insert the sequence num
		rawSnapshot.messageNum = serverMessageSequence;

		// get the old frame of the snap
		const rawSnapshot_t* old = readOldSnapshot(msg, rawSnapshot, snapshotParm);
		snapshotParm.oldSnap = old;

		rawSnapshot.snapFlags = msg.ReadByte();

		readAreaMask(msg, rawSnapshot);

		// Read player state
		const playerState_t* oldps = old ? &old->ps : nullptr;
		playerStateParser->readDeltaPlayerState(msg, oldps, &rawSnapshot.ps);

		// Read all entities in this snap
		parsePacketEntities(msg, old, &rawSnapshot, snapshotParm, gameState);

		// Parse sounds
		parseSounds(msg, &rawSnapshot, gameState);

		if (!rawSnapshot.valid) {
			return;
		}
	}

	const rawSnapshot_t* readOldSnapshot(MSG& msg, rawSnapshot_t& snap, const snapshotParm_t& snapshotParm) const
	{
		const uint8_t deltaNum = msg.ReadByte();
		if (!deltaNum) {
			snap.deltaNum = -1;
		}
		else {
			snap.deltaNum = snap.messageNum - deltaNum;
		}

		if (snap.deltaNum <= 0)
		{
			// uncompressed frame
			snap.valid = true;
			return nullptr;
		}

		const rawSnapshot_t* old = &snapshotParm.oldSnapshots[snap.deltaNum % snapshotParm.numOldSnapshots];
		if (!old->valid) {
			// should never happen
			// FIXME: throw?
			MOHPC_LOG(Warn, "Delta from invalid frame (not supposed to happen!).");
		}
		else if (old->messageNum != snap.deltaNum) {
			// The frame that the server did the delta from
			// is too old, so we can't reconstruct it properly.
			// FIXME: throw?
			MOHPC_LOG(Warn, "Delta frame too old.");
		}
		else if (snapshotParm.parseEntitiesNum > old->parseEntitiesNum + (snapshotParm.maxParseEntities - snapshotParm.maxSnapshotEntities)) {
			// FIXME: throw?
			MOHPC_LOG(Warn, "Delta parseEntitiesNum too old.");
		}
		else {
			snap.valid = true;	// valid delta parse
		}

		return old;
	}

	void readAreaMask(MSG& msg, rawSnapshot_t& snap) const
	{
		const uint8_t areaLen = msg.ReadByte();

		if (areaLen > sizeof(snap.areamask)) {
			throw SnapshotError::AreaMaskBadSize(areaLen);
		}

		// Read the area mask
		msg.ReadData(snap.areamask, areaLen);
	}

	void parsePacketEntities(MSG& msg, const rawSnapshot_t* oldFrame, rawSnapshot_t* newFrame, snapshotParm_t& snapshotParm, const gameState_t& gameState) const
	{
		newFrame->parseEntitiesNum = snapshotParm.parseEntitiesNum;
		newFrame->numEntities = 0;

		// delta from the entities present in oldframe
		entityState_t* oldState = NULL;
		uint32_t oldIndex = 0;
		uint32_t oldNum = 0;
		if (!oldFrame) {
			oldNum = 99999;
		}
		else
		{
			oldState = &snapshotParm.parseEntities[oldFrame->parseEntitiesNum % snapshotParm.maxParseEntities];
			oldNum = oldState->number;
		}

		const EntityList& entityBaselines = gameState.getEntityBaselines();

		for (;;)
		{
			const uint16_t newNum = entityParser->readEntityNum(msg);

			if (newNum == ENTITYNUM_NONE) {
				break;
			}

			while (oldNum < newNum)
			{
				parseDeltaEntity(msg, newFrame, snapshotParm, oldNum, oldState, true);

				++oldIndex;

				if (oldIndex >= oldFrame->numEntities) {
					oldNum = 99999;
				}
				else {
					oldState = &snapshotParm.parseEntities[(oldFrame->parseEntitiesNum + oldIndex) % snapshotParm.maxParseEntities];
					oldNum = oldState->number;
				}
			}

			if (oldNum == newNum)
			{
				// delta from previous state
				parseDeltaEntity(msg, newFrame, snapshotParm, newNum, oldState, false);

				++oldIndex;

				if (oldIndex >= oldFrame->numEntities) {
					oldNum = 99999;
				}
				else {
					oldState = &snapshotParm.parseEntities[(oldFrame->parseEntitiesNum + oldIndex) % snapshotParm.maxParseEntities];
					oldNum = oldState->number;
				}
				continue;
			}

			if (oldNum > newNum)
			{
				// delta from baseline
				parseDeltaEntity(msg, newFrame, snapshotParm, newNum, &entityBaselines.getEntity(newNum), false);
				continue;
			}
		}

		// any remaining entities in the old frame are copied over
		while (oldNum != 99999)
		{
			// one or more entities from the old packet are unchanged
			parseDeltaEntity(msg, newFrame, snapshotParm, oldNum, oldState, true);

			++oldIndex;

			if (oldIndex >= oldFrame->numEntities) {
				oldNum = 99999;
			}
			else
			{
				oldState = &snapshotParm.parseEntities[(oldFrame->parseEntitiesNum + oldIndex) % snapshotParm.maxParseEntities];
				oldNum = oldState->number;
			}
		}
	}

	void parseDeltaEntity(MSG& msg, rawSnapshot_t* frame, snapshotParm_t& snapshotParm, uint32_t newNum, const entityState_t* old, bool unchanged) const
	{
		entityState_t* state = &snapshotParm.parseEntities[snapshotParm.parseEntitiesNum % snapshotParm.maxParseEntities];

		if (unchanged) {
			*state = *old;
		}
		else {
			entityParser->readDeltaEntity(msg, old, state, newNum, snapshotParm.deltaTime);
		}

		if (state->number == ENTITYNUM_NONE)
		{
			// entity was delta removed
			return;
		}

		++snapshotParm.parseEntitiesNum;
		frame->numEntities++;
	}

	void parseSounds(MSG& msg, rawSnapshot_t* newFrame, const gameState_t& gameState) const
	{
		const bool hasSounds = msg.ReadBool();
		if (!hasSounds) {
			return;
		}

		const uint8_t numSounds = msg.ReadNumber<uint8_t>(7);
		if (numSounds > MAX_SERVER_SOUNDS) {
			return;
		}

		newFrame->numSounds = numSounds;

		const EntityList& entityBaselines = gameState.getEntityBaselines();

		MsgTypesHelper msgHelper(msg);
		for (size_t i = 0; i < numSounds; ++i)
		{
			sound_t& sound = newFrame->sounds[i];
			sound.hasStopped = msg.ReadBool();

			if (sound.hasStopped)
			{
				const uint16_t entityNum = entityParser->readEntityNum(msg);
				sound.entity = &entityBaselines.getEntity(entityNum);

				const uint8_t channel = msg.ReadNumber<uint8_t>(7);
				sound.channel = channel;
			}
			else
			{
				sound.isStreamed = msg.ReadBool();
				sound.isSpatialized = msg.ReadBool();

				if (sound.isSpatialized) {
					sound.origin = msgHelper.ReadVectorFloat();
				}

				const uint16_t entityNum = msg.ReadNumber<uint16_t>(11);
				sound.entity = &entityBaselines.getEntity(entityNum);

				if (entityNum >= MAX_GENTITIES) {
					throw SerializableErrors::BadEntityNumberException("sound", entityNum);
				}

				const uint8_t channel = msg.ReadNumber<uint8_t>(7);
				sound.channel = channel;

				const uint16_t soundIndex = msg.ReadNumber<uint16_t>(9);

				if (soundIndex < MAX_SOUNDS)
				{
					// Get the sound name from configstrings
					sound.soundName = gameState.getConfigstringManager().getConfigString(soundIndex);
				}
				else {
					// FIXME: Throw?
				}

				sound.hasVolume = msg.ReadBool();
				if (sound.hasVolume) {
					sound.volume = msg.ReadFloat();
				}
				else {
					sound.volume = -1.f;
				}

				sound.hasDist = msg.ReadBool();
				if (sound.hasDist) {
					sound.minDist = msg.ReadFloat();
				}
				else {
					sound.minDist = -1.f;
				}

				sound.hasPitch = msg.ReadBool();
				if (sound.hasPitch)
				{
					sound.pitch = msg.ReadFloat();
				}
				else {
					sound.pitch = -1.f;
				}

				sound.maxDist = msg.ReadFloat();
			}
		}
	}

protected:
	const Parsing::IEntity* entityParser;
	const Parsing::IPlayerState* playerStateParser;
};

class Snapshot8 : public SnapshotBase
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 5;
		maxRange = 8;
	}

	Snapshot8()
	{
		entityParser = Parsing::IEntity::get(8);
		playerStateParser = Parsing::IPlayerState::get(8);
	}

	void getProtocol(uint32_t& minRange, uint32_t& maxRange)
	{
		minRange = 5;
		maxRange = 8;
	}
};

class Snapshot17 : public SnapshotBase
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 15;
		maxRange = 17;
	}

	Snapshot17()
	{
		entityParser = Parsing::IEntity::get(17);
		playerStateParser = Parsing::IPlayerState::get(17);
	}

	void getProtocol(uint32_t& minRange, uint32_t& maxRange)
	{
		minRange = 5;
		maxRange = 8;
	}
};

Snapshot8 snapshot8;
Snapshot17 snapshot17;

SnapshotError::AreaMaskBadSize::AreaMaskBadSize(uint8_t inSize)
	: size(inSize)
{}

uint8_t SnapshotError::AreaMaskBadSize::getSize() const
{
	return size;
}

str SnapshotError::AreaMaskBadSize::what() const
{
	return str((int)getSize());
}
