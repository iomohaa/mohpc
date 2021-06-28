#include <MOHPC/Network/Types/Snapshot.h>

using namespace MOHPC;
using namespace MOHPC::Network;

rawSnapshot_t::rawSnapshot_t()
	: valid(false)
	, snapFlags(0)
	, serverTime(0)
	, serverTimeResidual(0)
	, messageNum(0)
	, deltaNum(0)
	, areamask{ 0 }
	, cmdNum(0)
	, numEntities(0)
	, parseEntitiesNum(0)
	, serverCommandNum(0)
	, numSounds(0)
{
}


Network::SnapshotInfo::SnapshotInfo()
	: snapFlags(SNAPFLAG_NOT_ACTIVE)
	, ping(0)
	, serverTime(0)
	, areamask{ 0 }
	, numEntities(0)
	, numServerCommands(0)
	, serverCommandSequence(0)
{
}

sound_t::sound_t()
	: entity(nullptr)
	, soundName(nullptr)
	, hasStopped(false)
	, isStreamed(false)
	, isSpatialized(false)
	, hasVolume(false)
	, hasDist(false)
	, hasPitch(false)
	, channel(0)
	, volume(0.f)
	, minDist(0.f)
	, maxDist(0.f)
	, pitch(0.f)
{
}

const entityState_t* sound_t::getEntityState() const
{
	return entity;
}

const char* sound_t::getName() const
{
	return soundName;
}

const Vector& sound_t::getOrigin() const
{
	return origin;
}

float sound_t::getVolume() const
{
	return volume;
}

float sound_t::getMinimumDistance() const
{
	return minDist;
}

float sound_t::getMaximumDistance() const
{
	return maxDist;
}

float sound_t::getPitch()
{
	return pitch;
}

uint8_t sound_t::getChannel() const
{
	return channel;
}

bool sound_t::hasSoundStopped() const
{
	return hasStopped;
}

bool sound_t::isStreamedSound() const
{
	return isStreamed;
}

bool sound_t::isSpatializedSound() const
{
	return isSpatialized;
}

bool sound_t::hasSoundVolume() const
{
	return hasVolume;
}

bool sound_t::hasSoundDistance() const
{
	return hasDist;
}

bool sound_t::hasSoundPitch() const
{
	return hasPitch;
}

size_t SnapshotInfo::getNumEntities() const
{
	return numEntities;
}

size_t SnapshotInfo::getNumSounds() const
{
	return numSounds;
}

size_t SnapshotInfo::getNumServerCommands()
{
	return numServerCommands;
}

size_t SnapshotInfo::getServerCommandSequence() const
{
	return serverCommandSequence;
}

uint32_t SnapshotInfo::getSnapFlags() const
{
	return snapFlags;
}

uint32_t SnapshotInfo::getPing() const
{
	return ping;
}

uint32_t SnapshotInfo::getServerTime() const
{
	return serverTime;
}

const playerState_t& SnapshotInfo::getPlayerState() const
{
	return ps;
}

uint8_t SnapshotInfo::getAreaMask(uint8_t index) const
{
	return areamask[index];
}

const entityState_t& SnapshotInfo::getEntityState(entityNum_t index) const
{
	return entities[index];
}

const entityState_t* MOHPC::Network::SnapshotInfo::getEntityStateByNumber(entityNum_t entityNum) const
{
	for (size_t i = 0; i < numEntities; ++i)
	{
		const entityState_t& es = entities[i];
		if (es.number == entityNum)
		{
			// found the entity
			return &es;
		}
	}

	return nullptr;
}

const sound_t& SnapshotInfo::getSound(uint8_t index) const
{
	return sounds[index];
}

