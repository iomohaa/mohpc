#include <MOHPC/Network/Client/ServerSnapshot.h>
#include <MOHPC/Network/Client/Time.h>
#include <MOHPC/Network/Client/GameState.h>

using namespace MOHPC;
using namespace Network;


ServerSnapshotManager::ServerSnapshotManager(protocolType_c protocol)
	: parseEntitiesNum(0)
	, lastSnapFlags(0)
	, snapActive(false)
	, newSnapshots(false)
{
	const uint32_t version = protocol.getProtocolVersionNumber();

	entityParser = Parsing::IEntity::get(version);
	playerStateParser = Parsing::IPlayerState::get(version);
	PVSParser = Parsing::IPVS::get(version);
	snapshotParser = Parsing::ISnapshot::get(version);
}

void ServerSnapshotManager::setNewSnap(rawSnapshot_t& newSnap)
{
	uint32_t oldMessageNum = currentSnap.messageNum + 1;

	if (newSnap.messageNum >= PACKET_BACKUP + oldMessageNum) {
		oldMessageNum = newSnap.messageNum - (PACKET_BACKUP - 1);
	}

	for (; oldMessageNum < newSnap.messageNum; oldMessageNum++) {
		snapshots[oldMessageNum & PACKET_MASK].valid = false;
	}

	currentSnap = newSnap;

	snapshots[currentSnap.messageNum & PACKET_MASK] = currentSnap;
	newSnapshots = true;
}

void ServerSnapshotManager::parseSnapshot(
	MSG& msg,
	const ServerGameState& clGameState,
	ClientTime& clientTime,
	ICommandSequence* serverCommands,
	tickTime_t currentTime,
	uint32_t serverMessageSequence,
	uint32_t outSequenceNum
)
{
	Parsing::snapshotParm_t snapParm;
	snapParm.deltaTime = clientTime.getDeltaTimeSeconds();
	snapParm.maxParseEntities = MAX_PARSE_ENTITIES;
	snapParm.maxSnapshotEntities = 128;
	snapParm.oldSnapshots = snapshots;
	snapParm.numOldSnapshots = PACKET_BACKUP;
	snapParm.parseEntities = parseEntities;
	snapParm.parseEntitiesNum = parseEntitiesNum;

	rawSnapshot_t newSnap;

	snapshotParser->parseSnapshot(
		msg,
		clGameState.get(),
		serverCommands,
		newSnap,
		snapParm,
		serverMessageSequence
	);

	parseEntitiesNum = snapParm.parseEntitiesNum;

	// notify about the player start
	const playerState_t* oldps = snapParm.oldSnap ? &snapParm.oldSnap->ps : nullptr;
	handlerList.playerStateReadHandler.broadcast(const_cast<const playerState_t*>(oldps), const_cast<const playerState_t*>(&newSnap.ps));

	if (!newSnap.valid) {
		return;
	}

	if (currentSnap.valid && ((currentSnap.snapFlags ^ newSnap.snapFlags) & SNAPFLAG_SERVERCOUNT))
	{
		// server time starts from here
		clientTime.setStartTime(currentTime);
		clientTime.setRemoteStartTime(newSnap.serverTime);
	}

	// set the new snap and calculate the ping
	setNewSnap(newSnap);

	Parsing::pvsParm_t parm;
	parm.clientNum = clGameState.getClientNum();
	VectorCopy(currentSnap.ps.getOrigin(), parm.origin);
	// FIXME: make the value modifiable as an option?
	// currently 1 radar unit = 63 world units
	parm.radarRange = radarInfo_t::getCoordPrecision();
	parm.radarFarMult = 1024.f;

	// read and unpack radar info if it exists on the server
	radarUnpacked_t unpacked;
	if (PVSParser->readNonPVSClient(currentSnap.ps.getRadarInfo(), parm, unpacked))
	{
		handlerList.readNonPVSClientHandler.broadcast(unpacked);
	}

	getHandlers().snapshotParsedHandler.broadcast(currentSnap);
}

void ServerSnapshotManager::notifySounds(const rawSnapshot_t* newFrame)
{
	for (size_t i = 0; i < newFrame->numSounds; ++i)
	{
		const sound_t& sound = newFrame->sounds[i];
		getHandlers().soundHandler.broadcast(sound);
	}
}

bool ServerSnapshotManager::firstSnapshot()
{
	if (currentSnap.snapFlags & SNAPFLAG_NOT_ACTIVE) {
		return false;
	}

	updateSnapFlags();

	snapActive = true;

	// Notify about the snapshot
	getHandlers().firstSnapshotHandler.broadcast(currentSnap);

	return true;
}

void ServerSnapshotManager::serverRestarted()
{
	updateSnapFlags();

	handlerList.serverRestartedHandler.broadcast();
}

void ServerSnapshotManager::updateSnapFlags()
{
	lastSnapFlags = currentSnap.snapFlags;
}

bool ServerSnapshotManager::checkTime(ClientTime& clientTime)
{
	if (!snapActive)
	{
		if (newSnapshots)
		{
			newSnapshots = false;
			return firstSnapshot();
		}
	}

	// Check when server has restarted
	if ((currentSnap.snapFlags ^ lastSnapFlags) & SNAPFLAG_SERVERCOUNT) {
		serverRestarted();
	}

	// already active or no new snapshots
	newSnapshots = false;
	return false;
}

bool ServerSnapshotManager::isActive() const
{
	return snapActive;
}

ServerSnapshotManager::HandlerList& ServerSnapshotManager::getHandlers()
{
	return handlerList;
}

const ServerSnapshotManager::HandlerList& ServerSnapshotManager::getHandlers() const
{
	return handlerList;
}

uint32_t ServerSnapshotManager::getCurrentSnapNumber() const
{
	return currentSnap.messageNum;
}

bool ServerSnapshotManager::getSnapshot(uintptr_t snapshotNum, SnapshotInfo& outSnapshot) const
{
	// if the frame has fallen out of the circular buffer, we can't return it
	if (currentSnap.messageNum >= PACKET_BACKUP + snapshotNum) {
		return false;
	}

	// if the frame is not valid, we can't return it
	const rawSnapshot_t* foundSnap = &snapshots[snapshotNum & PACKET_MASK];
	if (!foundSnap->valid) {
		return false;
	}

	// if the entities in the frame have fallen out of their
	// circular buffer, we can't return it
	if (parseEntitiesNum >= MAX_PARSE_ENTITIES + foundSnap->parseEntitiesNum) {
		return false;
	}

	// Copy snapshot data
	outSnapshot.snapFlags = foundSnap->snapFlags;
	outSnapshot.serverCommandSequence = foundSnap->serverCommandNum;
	outSnapshot.serverTime = foundSnap->serverTime;
	memcpy(outSnapshot.areamask, foundSnap->areamask, sizeof(outSnapshot.areamask));
	outSnapshot.ps = foundSnap->ps;

	size_t count = foundSnap->numEntities;
	if (count > MAX_ENTITIES_IN_SNAPSHOT) {
		count = MAX_ENTITIES_IN_SNAPSHOT;
	}

	outSnapshot.numEntities = count;

	// Copy entities to snapshot
	for (uintptr_t i = 0; i < count; i++) {
		outSnapshot.entities[i] = parseEntities[(foundSnap->parseEntitiesNum + i) & (MAX_PARSE_ENTITIES - 1)];
	}

	outSnapshot.numSounds = foundSnap->numSounds;
	for (uintptr_t i = 0; i < outSnapshot.numSounds; i++) {
		outSnapshot.sounds[i] = foundSnap->sounds[i];
	}

	return true;
}

bool ServerSnapshotManager::getSnapshot(SnapshotInfo& outSnapshot) const
{
	return getSnapshot(getCurrentSnapNumber(), outSnapshot);
}

const rawSnapshot_t& ServerSnapshotManager::getCurrentSnap() const
{
	return currentSnap;
}

bool ServerSnapshotManager::isSnapshotValid() const
{
	return currentSnap.valid;
}

bool ServerSnapshotManager::hasNewSnapshots() const
{
	return newSnapshots;
}

netTime_t ServerSnapshotManager::getServerTime() const
{
	return currentSnap.serverTime;
}

