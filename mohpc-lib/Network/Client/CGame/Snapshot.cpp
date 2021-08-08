#include <MOHPC/Network/Client/CGame/Snapshot.h>
#include <MOHPC/Network/Client/ServerSnapshot.h>
#include <MOHPC/Network/Client/Time.h>
#include <MOHPC/Utility/TokenParser.h>
#include <MOHPC/Utility/CommandManager.h>

#include <cassert>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

EntityInfo::EntityInfo()
	: currentValid(false)
	, interpolate(false)
	, teleported(false)
	, snapshotTime(std::chrono::milliseconds())
{
}

const entityState_t& EntityInfo::getCurrentState() const
{
	return currentState;
}

const entityState_t& EntityInfo::getNextState() const
{
	return nextState;
}

netTime_t EntityInfo::getSnapshotTime() const
{
	return snapshotTime;
}

bool EntityInfo::isValid() const
{
	return currentValid;
}

bool EntityInfo::isInterpolating() const
{
	return interpolate;
}

bool EntityInfo::hasTeleported() const
{
	return teleported;
}

SnapshotProcessor::SnapshotProcessor(CommandManager& serverCommandManagerRef)
	: serverCommandManager(serverCommandManagerRef)
	, nextSnap(nullptr)
	, snap(nullptr)
	, processedSnapshotNum(0)
	, latestSnapshotNum(0)
	, nextFrameTeleport(false)
	, nextFrameCameraCut(false)
	, thisFrameTeleport(false)
{
}

void SnapshotProcessor::init(uintptr_t serverMessageSequence, rsequence_t serverCommandSequence)
{
	processedSnapshotNum = serverMessageSequence;
	latestSnapshotNum = processedSnapshotNum;
	latestCommandSequence = serverCommandSequence;
}

const SnapshotInfo& SnapshotProcessor::getOldSnap() const
{
	return oldSnap;
}

const MOHPC::Network::SnapshotInfo* SnapshotProcessor::getSnap() const
{
	return snap;
}

const MOHPC::Network::SnapshotInfo* SnapshotProcessor::getNextSnap() const
{
	return nextSnap;
}

const EntityInfo* SnapshotProcessor::getEntity(entityNum_t num) const
{
	if (num >= MAX_GENTITIES) {
		return nullptr;
	}

	const EntityInfo& entInfo = clientEnts[num];
	if (!entInfo.currentValid)
	{
		// invalid entity
		return nullptr;
	}

	// Only return entities that are valid
	return &entInfo;
}

SnapshotInfo* SnapshotProcessor::readNextSnapshot()
{
	SnapshotInfo* dest;

	while (processedSnapshotNum < latestSnapshotNum)
	{
		dest = &activeSnapshots[processedSnapshotNum % MAX_ACTIVE_SNAPSHOTS];

		processedSnapshotNum++;
		bool r = snapshotManager->getSnapshot(processedSnapshotNum, *dest);
		if (r)
		{
			return dest;
		}
	}

	return nullptr;
}

void SnapshotProcessor::processSnapshots(tickTime_t simulatedRemoteTime)
{
	using namespace std::chrono;

	const uintptr_t n = snapshotManager->getCurrentSnapNumber();
	if (n != latestSnapshotNum)
	{
		if (n < latestSnapshotNum)
		{
			// This should never happen
			throw CGSnapshotError::SnapNumWentBackward(n, latestSnapshotNum);
		}

		latestSnapshotNum = n;
	}

	SnapshotInfo* foundSnap;
	while (!this->snap)
	{
		foundSnap = readNextSnapshot();
		if (!foundSnap)
		{
			// we can't continue until we get a snapshot
			return;
		}

		if (!(foundSnap->snapFlags & SNAPFLAG_NOT_ACTIVE)) {
			setInitialSnapshot(foundSnap);
		}

		if (!this->snap) {
			return;
		}
	}

	// loop until we either have a valid nextSnap with a serverTime
	// greater than cg.time to interpolate towards, or we run
	// out of available snapshots
	for (;;)
	{
		// if we don't have a nextframe, try and read a new one in
		if (!this->nextSnap)
		{
			foundSnap = readNextSnapshot();

			// if we still don't have a nextframe, we will just have to
			// extrapolate
			if (!foundSnap) {
				break;
			}

			setNextSnap(foundSnap);

			if (nextSnap->serverTime < snap->serverTime)
			{
				// server time went backward
				throw CGSnapshotError::NextSnapTimeWentBackward(snap->getServerTime(), nextSnap->getServerTime());
			}
		}

		const tickTime_t serverStartTime = time_cast<tickTime_t>(clientTime->getRemoteStartTime());
		const tickTime_t snapTime = time_cast<tickTime_t>(snap->getServerTime());
		const tickTime_t nextSnapTime = time_cast<tickTime_t>(nextSnap->getServerTime());
		// if our time is < nextFrame's, we have a nice interpolating state
		if (simulatedRemoteTime >= snapTime && simulatedRemoteTime < snapTime && snapTime > serverStartTime) {
			break;
		}

		// we have passed the transition from nextFrame to frame
		transitionSnapshot(snapTime <= serverStartTime);
	}

	assert(this->snap);
	if (!snap) {
		throw CGSnapshotError::NullSnapshot();
	}

	if (simulatedRemoteTime < time_cast<tickTime_t>(snap->getServerTime())) {
		simulatedRemoteTime = time_cast<tickTime_t>(snap->getServerTime());
	}

	if (nextSnap && time_cast<tickTime_t>(nextSnap->getServerTime()) <= simulatedRemoteTime) {
		throw CGSnapshotError::NextSnapTimeWentBackward(time_cast<netTime_t>(simulatedRemoteTime), nextSnap->getServerTime());
	}
}

void SnapshotProcessor::setNextSnap(SnapshotInfo* newSnap)
{
	this->nextSnap = newSnap;

	// The current player will always interpolate
	clientEnts[newSnap->ps.clientNum].interpolate = true;

	for (size_t i = 0; i < newSnap->numEntities; ++i)
	{
		entityState_t& es = newSnap->entities[i];
		EntityInfo& entInfo = clientEnts[es.number];

		// Copy the next state
		entInfo.nextState = es;

		// if this frame is a teleport, or the entity wasn't in the
		// previous frame, don't interpolate
		if (!entInfo.currentValid ||
			((entInfo.currentState.eFlags ^ es.eFlags) & EF_TELEPORT_BIT) ||
			(entInfo.currentState.parent != es.parent) ||
			(entInfo.currentState.modelindex != es.modelindex)
			)
		{
			entInfo.interpolate = false;
			if (entInfo.currentValid) {
				entInfo.teleported = true;
			}
		}
		else {
			entInfo.interpolate = true;
		}
	}

	// if the next frame is a teleport for the playerstate, we
	// can't interpolate during demos
	if (snap && (snap->ps.pm_flags & PMF_RESPAWNED)) {
		nextFrameTeleport = true;
	}
	else {
		nextFrameTeleport = false;
	}

	// if changing follow mode, don't interpolate
	if (nextSnap->ps.clientNum != snap->ps.clientNum) {
		nextFrameTeleport = true;
	}

	// if the camera cut bit changed, than the next frame is a camera cut
	if ((nextSnap->ps.camera_flags & CF_CAMERA_CUT_BIT) != (snap->ps.camera_flags & CF_CAMERA_CUT_BIT)) {
		nextFrameCameraCut = true;
	}
	else {
		nextFrameCameraCut = false;
	}

	if (snap->getServerTime() < clientTime->getRemoteStartTime()) {
		nextFrameTeleport = true;
	}

	// sort out solid entities
	//buildSolidList();
}

void SnapshotProcessor::setInitialSnapshot(SnapshotInfo* newSnap)
{
	this->snap = newSnap;

	// sort out solid entities
	//buildSolidList();
	// execute all commands at once that was received in this snap
	executeNewServerCommands(this->snap->serverCommandSequence, false);

	// Prepare entities that are present in this snapshot
	for (uintptr_t i = 0; i < newSnap->numEntities; ++i)
	{
		const entityState_t& state = newSnap->entities[i];
		EntityInfo& entInfo = clientEnts[state.number];

		entInfo.currentState = state;
		entInfo.nextState = state;
		entInfo.interpolate = false;
		entInfo.currentValid = true;

		// This is the first snapshot, notify about each entities
		handlers().entityAddedHandler.broadcast(state);
	}
}

void SnapshotProcessor::transitionSnapshot(bool differentServer)
{
	assert(snap);
	assert(nextSnap);

	// execute any server string commands before transitioning entities
	executeNewServerCommands(nextSnap->serverCommandSequence, differentServer);

	const SnapshotInfo* const from = snap;
	const SnapshotInfo* const target = nextSnap;

	if (!(from->snapFlags & SNAPFLAG_NOT_ACTIVE))
	{
		// Check for new entities
		for (size_t i = 0; i < target->numEntities; ++i)
		{
			const entityState_t& es = target->entities[i];

			const entityState_t* foundEnt = from->getEntityStateByNumber(es.number);
			if (!foundEnt) {
				handlers().entityAddedHandler.broadcast(es);
			}
		}
	}
	else
	{
		// Check for new entities
		for (size_t i = 0; i < target->numEntities; ++i)
		{
			const EntityInfo& entInfo = clientEnts[target->entities[i].number];
			handlers().entityAddedHandler.broadcast(entInfo.getCurrentState());
		}
	}

	for (uintptr_t i = 0; i < from->numEntities; i++)
	{
		const EntityInfo& entInfo = clientEnts[from->entities[i].number];
		const entityState_t& current = entInfo.getCurrentState();
		const entityState_t* next = target->getEntityStateByNumber(current.number);

		// only check if the entity is present in snap
		// otherwise it's useless
		if (next && memcmp(&current, &next, sizeof(entityState_t)))
		{
			// notify about modification
			handlers().entityModifiedHandler.broadcast(current, *next);
		}
	}

	// clear the currentValid flag for all entities in the existing snapshot
	for (uintptr_t i = 0; i < snap->numEntities; i++)
	{
		EntityInfo& entInfo = clientEnts[snap->entities[i].number];
		entInfo.currentValid = false;
	}

	if (!(from->snapFlags & SNAPFLAG_NOT_ACTIVE))
	{
		// Check for entities that are removed
		for (size_t i = 0; i < from->numEntities; ++i)
		{
			const entityState_t& es = from->entities[i];

			const entityState_t* foundEnt = target->getEntityStateByNumber(es.number);
			if (!foundEnt) {
				handlers().entityRemovedHandler.broadcast(es);
			}
		}
	}

	this->oldSnap = *this->snap;
	this->snap = nextSnap;

	for (uintptr_t i = 0; i < snap->numEntities; ++i)
	{
		EntityInfo& entInfo = clientEnts[snap->entities[i].number];
		transitionEntity(entInfo);
		entInfo.snapshotTime = this->snap->getServerTime();
	}

	this->nextSnap = nullptr;

	playerState_t* ops = &oldSnap.ps;
	playerState_t* ps = &snap->ps;

	// teleporting checks are irrespective of prediction
	if (ps->pm_flags & PMF_RESPAWNED) {
		thisFrameTeleport = true;
	}
}

void SnapshotProcessor::transitionEntity(EntityInfo& entInfo)
{
	entInfo.currentState = entInfo.nextState;
	entInfo.currentValid = true;
	entInfo.interpolate = false;
	entInfo.teleported = false;
}

void SnapshotProcessor::executeNewServerCommands(uintptr_t serverCommandSequence, bool differentServer)
{
	while (this->latestCommandSequence < serverCommandSequence)
	{
		const char* commandString = commandSequence->getSequence(++latestCommandSequence);
		if (commandString)
		{
			// process the command
			serverCommandManager.process(commandString);
		}
	}
}

const SnapshotProcessor::HandlerList& SnapshotProcessor::handlers() const
{
	return handlerList;
}

SnapshotProcessor::HandlerList& SnapshotProcessor::handlers()
{
	return handlerList;
}

bool SnapshotProcessor::doesTeleportNextFrame() const
{
	return nextFrameTeleport;
}

bool SnapshotProcessor::doesCameraCutNextFrame() const
{
	return nextFrameCameraCut;
}

bool SnapshotProcessor::doesTeleportThisFrame() const
{
	return thisFrameTeleport;
}

void SnapshotProcessor::makeTeleportThisFrame()
{
	thisFrameTeleport = true;
}

void SnapshotProcessor::clearTeleportThisFrame()
{
	thisFrameTeleport = false;
}

void SnapshotProcessor::setPtrs(const ClientTime* clientTimePtr, const ServerSnapshotManager* snapshotManagerPtr, const ICommandSequence* commandSequencePtr)
{
	clientTime = clientTimePtr;
	snapshotManager = snapshotManagerPtr;
	commandSequence = commandSequencePtr;
}

CGSnapshotError::NextSnapTimeWentBackward::NextSnapTimeWentBackward(netTime_t inPrevTime, netTime_t inTime)
	: oldTime(inPrevTime)
	, time(inTime)
{
}

netTime_t CGSnapshotError::NextSnapTimeWentBackward::getClientTime() const
{
	return oldTime;
}

netTime_t CGSnapshotError::NextSnapTimeWentBackward::getSnapTime() const
{
	return time;
}

CGSnapshotError::SnapNumWentBackward::SnapNumWentBackward(uintptr_t inNewNum, uintptr_t inLatestNum)
	: newNum(inNewNum)
	, latestNum(inLatestNum)
{
}

uintptr_t CGSnapshotError::SnapNumWentBackward::getNewNum() const
{
	return newNum;
}

uintptr_t CGSnapshotError::SnapNumWentBackward::getLatestNum() const
{
	return latestNum;
}
