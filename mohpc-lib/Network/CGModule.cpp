#include <MOHPC/Network/CGModule.h>
#include <MOHPC/Network/ClientGame.h>
#include <MOHPC/Managers/ShaderManager.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Vector.h>
#include <MOHPC/Log.h>
#include <MOHPC/Utilities/TokenParser.h>

#include <bitset>

using namespace MOHPC;
using namespace Network;

#define MOHPC_LOG_NAMESPACE "cgame" 

const char* effectsModel[] =
{
"models/fx/barrel_oil_leak_big.tik",
"models/fx/barrel_oil_leak_medium.tik",
"models/fx/barrel_oil_leak_small.tik",
"models/fx/barrel_oil_leak_splat.tik",
"models/fx/barrel_water_leak_big.tik",
"models/fx/barrel_water_leak_medium.tik",
"models/fx/barrel_water_leak_small.tik",
"models/fx/barrel_water_leak_splat.tik",
"models/fx/bazookaexp_base.tik",
"models/fx/bazookaexp_dirt.tik",
"models/fx/bazookaexp_snow.tik",
"models/fx/bazookaexp_stone.tik",
"models/fx/bh_carpet_hard.tik",
"models/fx/bh_carpet_lite.tik",
"models/fx/bh_dirt_hard.tik",
"models/fx/bh_dirt_lite.tik",
"models/fx/bh_foliage_hard.tik",
"models/fx/bh_foliage_lite.tik",
"models/fx/bh_glass_hard.tik",
"models/fx/bh_glass_lite.tik",
"models/fx/bh_grass_hard.tik",
"models/fx/bh_grass_lite.tik",
"models/fx/bh_human_uniform_hard.tik",
"models/fx/bh_human_uniform_lite.tik",
"models/fx/bh_metal_hard.tik",
"models/fx/bh_metal_lite.tik",
"models/fx/bh_mud_hard.tik",
"models/fx/bh_mud_lite.tik",
"models/fx/bh_paper_hard.tik",
"models/fx/bh_paper_lite.tik",
"models/fx/bh_sand_hard.tik",
"models/fx/bh_sand_lite.tik",
"models/fx/bh_snow_hard.tik",
"models/fx/bh_snow_lite.tik",
"models/fx/bh_stone_hard.tik"
"models/fx/bh_stone_lite.tik",
"models/fx/bh_water_hard.tik",
"models/fx/bh_water_lite.tik",
"models/fx/bh_wood_hard.tik",
"models/fx/bh_wood_lite.tik",
"models/fx/fs_dirt.tik",
"models/fx/fs_grass.tik",
"models/fx/fs_heavy_dust.tik",
"models/fx/fs_light_dust.tik",
"models/fx/fs_mud.tik",
"models/fx/fs_puddle.tik",
"models/fx/fs_sand.tik",
"models/fx/fs_snow.tik",
"models/fx/fx_fence_wood.tik",
"models/fx/grenexp_base.tik",
"models/fx/grenexp_carpet.tik",
"models/fx/grenexp_dirt.tik",
"models/fx/grenexp_foliage.tik",
"models/fx/grenexp_grass.tik",
"models/fx/grenexp_gravel.tik",
"models/fx/grenexp_metal.tik",
"models/fx/grenexp_mud.tik",
"models/fx/grenexp_paper.tik",
"models/fx/grenexp_sand.tik",
"models/fx/grenexp_snow.tik",
"models/fx/grenexp_stone.tik",
"models/fx/grenexp_water.tik",
"models/fx/grenexp_wood.tik",
"models/fx/heavyshellexp_base.tik",
"models/fx/heavyshellexp_dirt.tik",
"models/fx/heavyshellexp_snow.tik",
"models/fx/heavyshellexp_stone.tik",
"models/fx/tankexp_base.tik",
"models/fx/tankexp_dirt.tik",
"models/fx/tankexp_snow.tik",
"models/fx/tankexp_stone.tik",
"models/fx/water_ripple_moving.tik",
"models/fx/water_ripple_still.tik",
"models/fx/water_trail_bubble.tik"
};

EntityInfo::EntityInfo()
	: currentValid(false)
	, interpolate(false)
	, teleported(false)
	, notified(false)
	, snapshotTime(0)
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

uint32_t EntityInfo::getSnapshotTime() const
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

CGameModuleBase::CGameModuleBase(const CGameImports& inImports)
	: imports(inImports)
	, snap(nullptr)
	, nextSnap(nullptr)
	, nextFrameTeleport(false)
	, thisFrameTeleport(false)
	, validPPS(false)
	, forceDisablePrediction(false)
	, processedSnapshotNum(0)
	, latestSnapshotNum(0)
	, latestCommandSequence(0)
	, numSolidEntities(0)
{
	traceFunction = stubTrace;
	pointContentsFunction = stubPointContents;

	// Create predefined box hull
	boxHull.CM_InitBoxHull();
}

const CGameModuleBase::HandlerListCGame& CGameModuleBase::handlers() const
{
	return handlerList;
}

void CGameModuleBase::parseCGMessage(MSG& msg)
{
	MsgTypesHelper msgHelper(msg);

	// loop until there is no message
	bool hasMessage;
	do
	{
		uint8_t msgType = 0;
		msg.ReadBits(&msgType, 6);

		handleCGMessage(msg, msgType);
		hasMessage = msg.ReadBool();
	} while (hasMessage);
}

void CGameModuleBase::tick(uint64_t deltaTime, uint64_t currentTime, uint64_t serverTime)
{
	svTime = serverTime;

	// Set snapshots transition
	processSnapshots();

	if(snap && !(snap->snapFlags & SNAPFLAG_NOT_ACTIVE))
	{
		// Calculate the interpolation time
		if (nextSnap)
		{
			uint32_t delta = nextSnap->serverTime - snap->serverTime;
			if (!delta) {
				frameInterpolation = 0.f;
			}
			else {
				frameInterpolation = (float)(svTime - snap->serverTime) / (float)delta;
			}
		}
		else {
			frameInterpolation = 0.f;
		}

		predictPlayerState(deltaTime);
		// Process entities
		addPacketEntities();
	}
}


float CGameModuleBase::getFrameInterpolation() const
{
	return frameInterpolation;
}

const EntityInfo* CGameModuleBase::getEntity(entityNum_t num)
{
	if (num >= MAX_GENTITIES) {
		return nullptr;
	}

	EntityInfo& entInfo = clientEnts[num];
	if (!entInfo.currentValid)
	{
		// invalid entity
		return nullptr;
	}

	// Only return entities that are valid
	return &entInfo;
}

SnapshotInfo* CGameModuleBase::getCurrentSnapshot() const
{
	return snap;
}

SnapshotInfo* CGameModuleBase::getNextSnapshot() const
{
	return nextSnap;
}

const playerState_t& CGameModuleBase::getPredictedPlayerState() const
{
	return predictedPlayerState;
}

SnapshotInfo* CGameModuleBase::readNextSnapshot()
{
	SnapshotInfo* dest;

	while (processedSnapshotNum < latestSnapshotNum)
	{
		dest = &activeSnapshots[processedSnapshotNum % MAX_ACTIVE_SNAPSHOTS];

		processedSnapshotNum++;
		bool r = getImports().getSnapshot(processedSnapshotNum, *dest);
		if (r)
		{
			// Normalize fields so that they are compatible between AA/SH/BT
			normalizePlayerState(dest->ps);
			return dest;
		}
	}

	return nullptr;
}

void CGameModuleBase::processSnapshots()
{
	uintptr_t n = getImports().getCurrentSnapshotNumber();
	if (n != latestSnapshotNum) {
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
		}

		const uint64_t serverStartTime = getImports().getServerStartTime();
		// if our time is < nextFrame's, we have a nice interpolating state
		if (svTime >= snap->serverTime && svTime < nextSnap->serverTime && snap->serverTime > serverStartTime) {
			break;
		}

		// we have passed the transition from nextFrame to frame
		transitionSnapshot(snap->serverTime <= serverStartTime);
	}

	assert(this->snap);
	
	if (svTime < snap->serverTime) {
		svTime = snap->serverTime;
	}

	if (nextSnap && nextSnap->serverTime <= svTime) {
		// FIXME: throw?
	}
}

void CGameModuleBase::setNextSnap(SnapshotInfo* newSnap)
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
			entInfo.notified = false;
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

	if (snap->serverTime < getImports().getServerStartTime()) {
		nextFrameTeleport = true;
	}

	// sort out solid entities
	buildSolidList();
}

void CGameModuleBase::setInitialSnapshot(SnapshotInfo* newSnap)
{
	this->snap = newSnap;

	// sort out solid entities
	buildSolidList();
	// execute all commands at once that was received in this snap
	executeNewServerCommands(this->snap->serverCommandSequence, false);

	// Prepare entities that are present in this snapshot
	for (uintptr_t i = 0; i < newSnap->numEntities; ++i)
	{
		entityState_t& state = newSnap->entities[i];
		EntityInfo& entInfo = clientEnts[state.number];

		entInfo.currentState = state;
		entInfo.interpolate = false;
		entInfo.currentValid = true;
		entInfo.notified = false;
	}
}

void CGameModuleBase::transitionSnapshot(bool differentServer)
{
	assert(snap);
	assert(nextSnap);

	// execute any server string commands before transitioning entities
	executeNewServerCommands(nextSnap->serverCommandSequence, differentServer);

	// clear the currentValid flag for all entities in the existing snapshot
	for (uintptr_t i = 0; i < snap->numEntities; i++)
	{
		EntityInfo& entInfo = clientEnts[snap->entities[i].number];
		entInfo.currentValid = false;
		entInfo.notified = false;
	}

	this->oldSnap = *this->snap;
	this->snap = nextSnap;

	for (uintptr_t i = 0; i < snap->numEntities; ++i)
	{
		EntityInfo& entInfo = clientEnts[snap->entities[i].number];
		transitionEntity(entInfo);
		entInfo.snapshotTime = this->snap->serverTime;
	}

	this->nextSnap = nullptr;

	playerState_t* ops = &oldSnap.ps;
	playerState_t* ps = &snap->ps;

	// teleporting checks are irrespective of prediction
	if (ps->pm_flags & PMF_RESPAWNED) {
		thisFrameTeleport = true;
	}
}

void CGameModuleBase::transitionEntity(EntityInfo& entInfo)
{
	entInfo.currentState = entInfo.nextState;
	entInfo.currentValid = true;
	entInfo.interpolate = false;
	entInfo.teleported = false;
}

void CGameModuleBase::addPacketEntities()
{
	if(!(oldSnap.snapFlags & SNAPFLAG_NOT_ACTIVE))
	{
		// Check for new entities
		for (size_t i = 0; i < snap->numEntities; ++i)
		{
			const entityState_t& es1 = snap->entities[i];
			EntityInfo& entInfo = clientEnts[es1.number];
			bool bFound = false;

			if (entInfo.notified) continue;

			for (size_t j = 0; j < oldSnap.numEntities; ++j)
			{
				const entityState_t& es2 = oldSnap.entities[j];
				if (es2.number == es1.number)
				{
					bFound = true;
					break;
				}
			}

			if (!bFound) {
				handlers().notify<CGameHandlers::EntityAdded>(entInfo);
			}
		}
	}
	else
	{
		// Check for new entities
		for (size_t i = 0; i < snap->numEntities; ++i)
		{
			EntityInfo& entInfo = clientEnts[snap->entities[i].number];

			if (entInfo.notified) continue;

			handlers().notify<CGameHandlers::EntityAdded>(entInfo);
		}
	}

	bool processed[1024]{ false };

	for (size_t i = 0; i < snap->numEntities; ++i)
	{
		uint32_t number = snap->entities[i].number;
		EntityInfo& entInfo = clientEnts[number];

		uint32_t parentNum = entInfo.currentState.parent;
		while (parentNum != ENTITYNUM_NONE && !processed[parentNum])
		{
			processed[parentNum] = true;
			addEntity(clientEnts[parentNum]);
		}

		if (!processed[number])
		{
			processed[number] = true;
			addEntity(entInfo);
		}
	}

	if (!(oldSnap.snapFlags & SNAPFLAG_NOT_ACTIVE))
	{
		// Check for entities that are removed
		for (size_t i = 0; i < oldSnap.numEntities; ++i)
		{
			const entityState_t& es1 = oldSnap.entities[i];
			EntityInfo& entInfo = clientEnts[es1.number];
			bool bFound = false;

			if (entInfo.notified) continue;

			for (size_t j = 0; j < snap->numEntities; ++j)
			{
				const entityState_t& es2 = snap->entities[j];
				if (es2.number == es1.number)
				{
					bFound = true;
					break;
				}
			}

			if (!bFound) {
				handlers().notify<CGameHandlers::EntityRemoved>(entInfo);
			}
		}
	}

	// Set notify to true to avoid sending the same notifications for each entity more than once
	for (size_t i = 0; i < snap->numEntities; ++i)
	{
		EntityInfo& entInfo = clientEnts[snap->entities[i].number];
		entInfo.notified = true;
	}

	if (!(oldSnap.snapFlags & SNAPFLAG_NOT_ACTIVE))
	{
		for (size_t i = 0; i < oldSnap.numEntities; ++i)
		{
			EntityInfo& entInfo = clientEnts[oldSnap.entities[i].number];
			entInfo.notified = true;
		}
	}
}

void CGameModuleBase::addEntity(EntityInfo& entInfo)
{
	if (memcmp(&entInfo.currentState, &entInfo.nextState, sizeof(entityState_t)))
	{
		handlers().notify<CGameHandlers::EntityModified>(entInfo);
		return;
	}
}

void CGameModuleBase::predictPlayerState(uint64_t deltaTime)
{
	if (!validPPS)
	{
		validPPS = true;
		predictedPlayerState = snap->ps;
	}

	if (snap->ps.pm_flags & PMF_NO_PREDICTION || snap->ps.pm_flags & PMF_FROZEN)
	{
		interpolatePlayerState(false);
		return;
	}

	// non-predicting local movement will grab the latest angles
	if (forceDisablePrediction)
	{
		interpolatePlayerState(true);
		return;
	}

	// Pmove
	Pmove pmove;
	pmove_t& pm = pmove.get();
	pm.ps = &predictedPlayerState;
	pm.pointcontents = pointContentsFunction;
	pm.trace = traceFunction;
	
	if (pm.ps->pm_type == pmType_e::PM_DEAD) {
		pm.tracemask = ContentFlags::MASK_PLAYERSOLID & ~ContentFlags::MASK_DYNAMICBODY;
	}
	else {
		pm.tracemask = ContentFlags::MASK_PLAYERSOLID;
	}
	
	pm.noFootsteps = cgs.hasAnyDMFlags(DMFlags::DF_NO_FOOTSTEPS);
	// Set settings depending on the protocol/version
	setupMove(pmove);

	playerState_t oldPlayerState = predictedPlayerState;
	const uintptr_t current = getImports().getCurrentCmdNumber();

	// Grab the latest cmd
	usercmd_t latestCmd;
	getImports().getUserCmd(current, latestCmd);

	if (nextSnap && !nextFrameTeleport && !thisFrameTeleport)
	{
		predictedPlayerState = nextSnap->ps;
		physicsTime = nextSnap->serverTime;
	}
	else
	{
		predictedPlayerState = snap->ps;
		physicsTime = snap->serverTime;
	}

	pm.pmove_fixed = 0;
	pm.pmove_msec = 8;

	bool moved = false;
	for (uintptr_t cmdNum = current - CMD_BACKUP + 1; cmdNum <= current; ++cmdNum)
	{
		getImports().getUserCmd(cmdNum, pm.cmd);

		// don't do anything if the time is before the snapshot player time
		if (pm.cmd.serverTime <= predictedPlayerState.commandTime) {
			continue;
		}

		// don't do anything if the command was from a previous map_restart
		if (pm.cmd.serverTime > latestCmd.serverTime) {
			continue;
		}

		if (predictedPlayerState.commandTime == oldPlayerState.commandTime)
		{
			if (thisFrameTeleport)
			{
				predictedError = Vector();
				thisFrameTeleport = false;
			}

			// FIXME: Should it have some sort of predicted error?
		}

		// Replay movement
		moved |= replayMove(pmove);
	}

	// interpolate camera view (spectator, cutscenes, etc)
	interpolatePlayerStateCamera();

	if (predictedPlayerState.groundEntityNum != ENTITYNUM_NONE && predictedPlayerState.groundEntityNum != ENTITYNUM_WORLD)
	{
		EntityInfo& entInfo = clientEnts[predictedPlayerState.groundEntityNum];
		if (entInfo.interpolate)
		{
			const float f = frameInterpolation - 1.f;

			predictedPlayerState.origin = predictedPlayerState.origin + (entInfo.nextState.origin - entInfo.currentState.origin) * f;
		}
	}
}

void CGameModuleBase::interpolatePlayerState(bool grabAngles)
{
	playerState_t* out = &predictedPlayerState;

	*out = snap->ps;

	// interpolate the camera if necessary
	interpolatePlayerStateCamera();

	// if we are still allowing local input, short circuit the view angles
	if (grabAngles)
	{
		uintptr_t cmdNum = getImports().getCurrentCmdNumber();

		usercmd_t cmd;
		getImports().getUserCmd(cmdNum, cmd);

		Pmove::PM_UpdateViewAngles(out, &cmd);
	}

	// if the next frame is a teleport, we can't lerp to it
	if (nextFrameTeleport) {
		return;
	}

	const SnapshotInfo* prev = snap;
	const SnapshotInfo* next = nextSnap;

	if (!next || next->serverTime <= prev->serverTime)
	{
		if(!next) {
			MOHPC_LOG(VeryVerbose, "CGameModuleBase::interpolatePlayerState: nextSnap == NULL");
		}
		return;
	}

	const float f = frameInterpolation;

	uint32_t i = next->ps.bobCycle;
	if (i < prev->ps.bobCycle)
	{
		// handle wraparound
		i += 256;
	}

	// interpolate the bob cycle
	out->bobCycle = (uint8_t)((float)prev->ps.bobCycle + f * (float)(i - prev->ps.bobCycle));

	// interpolate the lean angle
	out->fLeanAngle = prev->ps.fLeanAngle +
		f * (next->ps.fLeanAngle - prev->ps.fLeanAngle);

	// interpolate angles, origin and velocity
	for (i = 0; i < 3; i++) {
		out->origin[i] = prev->ps.origin[i] + f * (next->ps.origin[i] - prev->ps.origin[i]);
		if (!grabAngles) {
			out->viewangles[i] = LerpAngle(
				prev->ps.viewangles[i], next->ps.viewangles[i], f);
		}
		out->velocity[i] = prev->ps.velocity[i] +
			f * (next->ps.velocity[i] - prev->ps.velocity[i]);
	}
}

void CGameModuleBase::interpolatePlayerStateCamera()
{
	//
	// copy in the current ones if nothing else
	//
	cameraAngles = predictedPlayerState.camera_angles;
	cameraOrigin = predictedPlayerState.camera_origin;
	cameraFov = predictedPlayerState.fov;

	// if the next frame is a teleport, we can't lerp to it
	if (nextFrameCameraCut) {
		return;
	}

	SnapshotInfo* prev = snap;
	SnapshotInfo* next = nextSnap;

	if (!next || next->serverTime <= prev->serverTime) {
		return;
	}

	const float f = (float)(svTime - prev->serverTime) / (next->serverTime - prev->serverTime);

	// interpolate fov
	cameraFov = prev->ps.fov + f * (next->ps.fov - prev->ps.fov);

	if (!(snap->ps.pm_flags & PMF_CAMERA_VIEW))
	{
		// only interpolate if the player is in camera view
		return;
	}

	if (predictedPlayerState.camera_flags & CF_CAMERA_ANGLES_TURRETMODE)
	{
		predictedPlayerState.camera_origin = next->ps.camera_origin;
		predictedPlayerState.camera_angles = next->ps.camera_angles;
		return;
	}

	for (uint32_t i = 0; i < 3; i++)
	{
		predictedPlayerState.camera_origin[i] = prev->ps.camera_origin[i] + f * (next->ps.camera_origin[i] - prev->ps.camera_origin[i]);
		predictedPlayerState.camera_angles[i] = LerpAngle(prev->ps.camera_angles[i], next->ps.camera_angles[i], f);
	}
}

uint64_t CGameModuleBase::getTime() const
{
	return svTime;
}

void CGameModuleBase::setTraceFunction(TraceFunction&& inTraceFunction)
{
	traceFunction = std::move(inTraceFunction);
}

void CGameModuleBase::setPointContentsFunction(PointContentsFunction&& inPointContentsFunction)
{
	pointContentsFunction = std::move(inPointContentsFunction);
}

void CGameModuleBase::clipMoveToEntities(CollisionWorld& cm, const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, uint16_t skipNumber, uint32_t mask, bool cylinder, trace_t& tr)
{
	// iterate through entities and test their collision
	for (size_t i = 0; i < numSolidEntities; i++)
	{
		const EntityInfo* cent = solidEntities[i];
		const entityState_t* ent = &cent->currentState;

		if (ent->number == skipNumber) {
			continue;
		}

		CollisionWorld* world = &cm;
		clipHandle_t cmodel;
		Vector bmins, bmaxs;
		Vector origin, angles;

		if (ent->solid == SOLID_BMODEL)
		{
			// special value for bmodel
			cmodel = cm.inlineModel(ent->modelindex);
			if(!cmodel) continue;
			angles = cent->currentState.netangles;
			origin = cent->currentState.netorigin;
		}
		else
		{
			// encoded bbox
			IntegerToBoundingBox(ent->solid, bmins, bmaxs);
			cmodel = boxHull.CM_TempBoxModel(bmins, bmaxs, ContentFlags::CONTENTS_BODY);
			angles = vec3_origin;
			origin = cent->currentState.netorigin;
			// trace to the boxhull instead
			// entities with a boundingbox use a boxhull
			world = &boxHull;
		}

		trace_t trace;
		// trace through the entity's submodel
		world->CM_TransformedBoxTrace(&trace, start, end,
			mins, maxs, cmodel, mask, origin, angles, cylinder);

		if (trace.allsolid || trace.fraction < tr.fraction) {
			trace.entityNum = ent->number;
			tr = trace;
		}
		else if (trace.startsolid) {
			tr.startsolid = true;
		}
	}
}

void CGameModuleBase::executeNewServerCommands(uintptr_t serverCommandSequence, bool differentServer)
{
	while (this->latestCommandSequence < serverCommandSequence)
	{
		TokenParser tokenized;
		if (getImports().getServerCommand(++latestCommandSequence, tokenized)) {
			processServerCommand(tokenized);
		}
	}
}

void CGameModuleBase::processServerCommand(TokenParser& tokenized)
{
	struct cmd_t
	{
		const char* cmdName;
		void (CGameModuleBase::*function)(TokenParser& arguments);
	};

	// predefined commands
	// custom commands can be processed with a callback to server command
	static cmd_t cmds[] =
	{
		{ "print", &CGameModuleBase::SCmd_Print },
		{ "hudprint", &CGameModuleBase::SCmd_HudPrint },
		{ "scores", &CGameModuleBase::SCmd_Scores },
		{ "stats", &CGameModuleBase::SCmd_Stats },
		{ "stopwatch", &CGameModuleBase::SCmd_Stopwatch },
		{ "svlag", &CGameModuleBase::SCmd_ServerLag },
		{ "stufftext", &CGameModuleBase::SCmd_Stufftext },
	};
	static constexpr size_t numCmds = sizeof(cmds)/sizeof(cmds[0]);

	const char* command = tokenized.GetToken(false);
	// find the command in list
	for (size_t i = 0; i < numCmds; ++i)
	{
		const cmd_t& cmd = cmds[i];
		if (!str::icmp(command, cmd.cmdName))
		{
			// Call the correct function
			(this->*cmd.function)(tokenized);
			break;
		}
	}
}

void CGameModuleBase::trace(CollisionWorld& cm, trace_t& tr, const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, uint16_t skipNumber, uint32_t mask, bool cylinder, bool cliptoentities)
{
	// if there is a loaded collision from the game, use it instead
	cm.CM_BoxTrace(&tr, start, end, mins, maxs, 0, mask, cylinder);
	// collision defaults to world
	tr.entityNum = tr.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;

	if (tr.startsolid) {
		tr.entityNum = ENTITYNUM_WORLD;
	}

	if(cliptoentities)
	{
		// also trace through entities
		clipMoveToEntities(cm, start, mins, maxs, end, skipNumber, mask, cylinder, tr);
	}
}

uint32_t Network::CGameModuleBase::pointContents(CollisionWorld& cm, const Vector& point, uintptr_t passEntityNum)
{
	// get the contents in world
	uint32_t contents = cm.CM_PointContents(point, 0);

	// also iterate through entities (that are using a submodel) to check if the point is inside
	for (size_t i = 0; i < numSolidEntities; i++)
	{
		const EntityInfo* cent = solidEntities[i];
		const entityState_t* ent = &cent->currentState;

		if (ent->number == passEntityNum) {
			continue;
		}

		// special value for bmodel
		if (ent->solid != SOLID_BMODEL) {
			continue;
		}

		clipHandle_t cmodel = cm.inlineModel(ent->modelindex);
		if (!cmodel) {
			continue;
		}

		contents |= cm.CM_TransformedPointContents(point, cmodel, ent->origin, ent->angles);
	}

	return contents;
}

void Network::CGameModuleBase::disablePrediction()
{
	forceDisablePrediction = true;
}

void Network::CGameModuleBase::enablePrediction()
{
	forceDisablePrediction = false;
}

const objective_t& Network::CGameModuleBase::getObjective(uint32_t objNum) const
{
	return objectives[objNum];
}

const clientInfo_t& Network::CGameModuleBase::getClientInfo(uint32_t clientNum) const
{
	return clientInfo[clientNum];
}

const Network::CGameImports& Network::CGameModuleBase::getImports() const
{
	return imports;
}

void Network::CGameModuleBase::setupMove(Pmove& pmove)
{
}

void Network::CGameModuleBase::normalizePlayerState(playerState_t& ps)
{
}

bool Network::CGameModuleBase::replayMove(Pmove& pmove)
{
	pmove_t& pm = pmove.get();

	if (pm.ps->feetfalling && pm.waterlevel <= 1)
	{
		// clear movement when falling/under water
		pm.cmd.forwardmove = 0;
		pm.cmd.rightmove = 0;
	}

	// calculate delta time
	const uint32_t msec = pm.cmd.serverTime - pm.ps->commandTime;

	// call move handler
	pmove.move();

	// additional movement
	// can be anything, from jumping to events/fire prediction
	extendMove(pmove, msec);

	// valid move
	return true;
}

void Network::CGameModuleBase::extendMove(Pmove& pmove, uint32_t msec)
{
	const pmove_t& pm = pmove.get();

	if (pm.ps->pm_type == pmType_e::PM_NOCLIP)
	{
		const float frametime = msec / 1000.f;
		pm.ps->origin += pm.ps->velocity * frametime;
	}

	// Notify to replay/predict other events that are not present in the original pm code
	// It can be in PM related code, but it's not what the server expect
	// PM must remain identical to the server PM code
	handlers().notify<CGameHandlers::ReplayMove>(pm.cmd, *pm.ps, msec);
}

const rain_t& CGameModuleBase::getRain() const
{
	return rain;
}

const environment_t& CGameModuleBase::getEnvironment() const
{
	return environment;
}

const cgsInfo& CGameModuleBase::getServerInfo() const
{
	return cgs;
}

cgsInfo::cgsInfo()
	: matchStartTime(0)
	, matchEndTme(0)
	, levelStartTime(0)
	, serverLagTime(0)
	, gameType(gameType_e::FreeForAll)
	, dmFlags(0)
	, teamFlags(0)
	, maxClients(0)
	, fragLimit(0)
	, timeLimit(0)
{
}

uint64_t cgsInfo::getMatchStartTime() const
{
	return matchStartTime;
}

uint64_t cgsInfo::getMatchEndTime() const
{
	return matchEndTme;
}

uint64_t cgsInfo::getLevelStartTime() const
{
	return levelStartTime;
}

uint64_t cgsInfo::getServerLagTime() const
{
	return serverLagTime;
}

gameType_e cgsInfo::getGameType() const
{
	return gameType;
}

uint32_t cgsInfo::getDeathmatchFlags() const
{
	return dmFlags;
}

uint32_t cgsInfo::getTeamFlags() const
{
	return teamFlags;
}

uint32_t cgsInfo::getMaxClients() const
{
	return maxClients;
}

int32_t cgsInfo::getFragLimit() const
{
	return fragLimit;
}

int32_t cgsInfo::getTimeLimit() const
{
	return timeLimit;
}

const char* cgsInfo::getMapName() const
{
	return mapName.c_str();
}

const char* cgsInfo::getMapFilename() const
{
	return mapFilename.c_str();
}

const char* cgsInfo::getAlliedText(size_t index) const
{
	return alliedText[index].c_str();
}

const char* cgsInfo::getAxisText(size_t index)
{
	return axisText[index].c_str();
}

const char* cgsInfo::getScoreboardPic() const
{
	return scoreboardPic.c_str();
}

const char* cgsInfo::getScoreboardPicOver() const
{
	return scoreboardPicOver.c_str();
}

bool Network::cgsInfo::hasAnyDMFlags(uint32_t flags) const
{
	return (dmFlags & flags) != 0;
}

bool Network::cgsInfo::hasAllDMFlags(uint32_t flags) const
{
	return (dmFlags & flags) == flags;
}

uint64_t Network::cgsInfo::getVoteTime() const
{
	return voteTime;
}

uint32_t Network::cgsInfo::getNumVotesYes() const
{
	return numVotesYes;
}

uint32_t Network::cgsInfo::getNumVotesNo() const
{
	return numVotesNo;
}

uint32_t Network::cgsInfo::getNumVotesUndecided() const
{
	return numUndecidedVotes;
}

const char* Network::cgsInfo::getVoteString() const
{
	return voteString.c_str();
}

void CGameModuleBase::configStringModified(uint16_t num)
{
	const char* cs = getImports().getGameState().getConfigString(num);
	switch (num)
	{
	case CS_SERVERINFO:
		parseServerInfo(cs);
		break;
	case CS_RAIN_DENSITY:
		rain.density = (float)atof(cs);
		break;
	case CS_RAIN_SPEED:
		rain.speed = (float)atof(cs);
		break;
	case CS_RAIN_SPEEDVARY:
		rain.speedVary = atol(cs);
		break;
	case CS_RAIN_SLANT:
		rain.slant = atol(cs);
		break;
	case CS_RAIN_LENGTH:
		rain.length = (float)atof(cs);
		break;
	case CS_RAIN_MINDIST:
		rain.minDist = (float)atof(cs);
		break;
	case CS_RAIN_WIDTH:
		rain.width = (float)atof(cs);
		break;
	case CS_RAIN_SHADER:
		if(rain.numShaders)
		{
			for (size_t i = 0; i < rain.numShaders; ++i)
			{
				// Multiple shaders
				rain.shader[i] = str::printf("%s%i", cs, i);
			}
		}
		else
		{
			// Single shader
			rain.shader[0] = cs;
		}
		break;
	case CS_RAIN_NUMSHADERS:
		rain.numShaders = atol(cs);
		{
			const str tmp = rain.shader[0];
			for (size_t i = 0; i < rain.numShaders; ++i)
			{
				// Append shader number to the previous shader
				rain.shader[i] = str::printf("%s%i", tmp.c_str(), i);
			}
		}
		break;
	case CS_WARMUP:
		cgs.matchStartTime = atoll(cs);
		break;
	case CS_FOGINFO:
		{
			// Fog differs between games
			parseFogInfo(cs, environment);
		}
		break;
	case CS_SKYINFO:
		{
			int tmp = 0;
			sscanf(cs, "%f %d", &environment.skyAlpha, &tmp);
			// Parse bool
			environment.skyPortal = tmp;
		}
		break;
	case CS_LEVEL_START_TIME:
		cgs.levelStartTime = atol(cs);
		break;
	case CS_MATCHEND:
		cgs.matchEndTme = atol(cs);
		break;
	case CS_VOTE_TIME:
		cgs.voteTime = atol(cs);
		break;
	case CS_VOTE_STRING:
		cgs.voteString = cs;
		break;
	case CS_VOTES_YES:
		cgs.numVotesYes = atol(cs);
		break;
	case CS_VOTES_NO:
		cgs.numVotesNo = atol(cs);
		break;
	case CS_VOTES_UNDECIDED:
		cgs.numUndecidedVotes = atol(cs);
		break;
	}

	// Add objectives
	if (num >= CS_OBJECTIVES && num < CS_OBJECTIVES + MAX_OBJECTIVES)
	{
		ReadOnlyInfo info(cs);

		objective_t& objective = objectives[num - CS_OBJECTIVES];
	
		size_t strLen;
		// Get objective flags
		const char* flStr = info.ValueForKey("flags", strLen);

		objective.flags = atol(flStr);
		// Get objective text
		objective.text = info.ValueForKey("text");

		// Get objective location
		const char* locStr = info.ValueForKey("loc", strLen);
		sscanf(locStr, "%f %f %f", &objective.location[0], &objective.location[1], &objective.location[2]);
	}

	// Add all clients
	if (num >= CS_PLAYERS && num < CS_PLAYERS + MAX_CLIENTS)
	{
		ReadOnlyInfo info(cs);

		clientInfo_t& client = clientInfo[num - CS_PLAYERS];
		client.name = info.ValueForKey("name");
		// Specify a valid name if empty
		if(!client.name.length()) client.name = "UnnamedSolider";

		size_t strLen;
		// Get the current team
		const char* teamStr = info.ValueForKey("team", strLen);
		client.team = (teamType_e)atoi(teamStr);

		// Add other unknown properties
		for (InfoIterator it = info.createConstIterator(); it; ++it)
		{
			const char* key = it.key();
			// Don't add first properties above
			if (str::icmp(key, "name") && str::icmp(key, "team")) {
				client.properties.SetPropertyValue(key, it.value());
			}
		}
	}
}

void CGameModuleBase::parseServerInfo(const char* cs)
{
	ReadOnlyInfo info(cs);

	// Parse match settings
	cgs.gameType = (gameType_e)atol(info.ValueForKey("g_gametype"));
	cgs.dmFlags = atol(info.ValueForKey("dmflags"));
	cgs.teamFlags = atol(info.ValueForKey("teamflags"));
	cgs.fragLimit = atol(info.ValueForKey("fraglimit"));
	cgs.timeLimit = atol(info.ValueForKey("timelimit"));
	cgs.maxClients = atol(info.ValueForKey("sv_maxclients"));
	// Parse map
	cgs.mapName = info.ValueForKey("mapname");
	cgs.mapFilename = "maps/" + cgs.mapName + ".bsp";

	// Parse scoreboard info
	cgs.alliedText[0] = info.ValueForKey("g_obj_alliedtext1");
	cgs.alliedText[1] = info.ValueForKey("g_obj_alliedtext2");
	cgs.alliedText[2] = info.ValueForKey("g_obj_alliedtext3");
	cgs.axisText[0] = info.ValueForKey("g_obj_axistext1");
	cgs.axisText[1] = info.ValueForKey("g_obj_axistext2");
	cgs.axisText[2] = info.ValueForKey("g_obj_axistext3");
	cgs.scoreboardPic = info.ValueForKey("g_scoreboardpic");
	cgs.scoreboardPicOver = info.ValueForKey("g_scoreboardpicover");
}

void CGameModuleBase::buildSolidList()
{
	numSolidEntities = 0;
	numTriggerEntities = 0;

	SnapshotInfo* snap;

	if (nextSnap && !nextFrameTeleport && !thisFrameTeleport) {
		snap = nextSnap;
	}
	else {
		snap = this->snap;
	}

	for (uintptr_t i = 0; i < snap->numEntities; i++)
	{
		EntityInfo* cent = &clientEnts[snap->entities[i].number];
		entityState_t* ent = &cent->currentState;

		// Ignore item/triggers, they're always non-solid
		if (ent->eType == entityType_e::item || ent->eType == entityType_e::push_trigger || ent->eType == entityType_e::teleport_trigger) {
			continue;
		}

		if (cent->nextState.solid)
		{
			// Add solid entities
			solidEntities[numSolidEntities] = cent;
			numSolidEntities++;
			continue;
		}
	}
}

void CGameModuleBase::SCmd_Print(TokenParser& args)
{
	const char* text = args.GetString(true, false);
	const hudMessage_e type = (hudMessage_e )*(text++);
	handlers().notify<CGameHandlers::ServerCommand_Print>(type, text);
}

void CGameModuleBase::SCmd_HudPrint(TokenParser& args)
{
	const char* text = args.GetString(true, false);
	const hudMessage_e type = (hudMessage_e)*(text++);
	handlers().notify<CGameHandlers::ServerCommand_HudPrint>(type, text);
}

void CGameModuleBase::SCmd_Scores(TokenParser& args)
{
	Scoreboard scoreboard(cgs.gameType);
	scoreboard.parse(args);
	// Pass the parsed scoreboard
	handlers().notify<CGameHandlers::ServerCommand_Scores>(scoreboard);
}

void CGameModuleBase::SCmd_Stats(TokenParser& args)
{
	stats_t stats;

	stats.numObjectives			= args.GetInteger(false);
	stats.numComplete			= args.GetInteger(false);
	stats.numShotsFired			= args.GetInteger(false);
	stats.numHits				= args.GetInteger(false);
	stats.accuracy				= args.GetInteger(false);
	stats.preferredWeapon		= args.GetToken(false);
	stats.numHitsTaken			= args.GetInteger(false);
	stats.numObjectsDestroyed	= args.GetInteger(false);
	stats.numEnemysKilled		= args.GetInteger(false);
	stats.headshots				= args.GetInteger(false);
	stats.torsoShots			= args.GetInteger(false);
	stats.leftLegShots			= args.GetInteger(false);
	stats.rightLegShots			= args.GetInteger(false);
	stats.groinShots			= args.GetInteger(false);
	stats.leftArmShots			= args.GetInteger(false);
	stats.rightArmShots			= args.GetInteger(false);
	stats.gunneryEvaluation		= args.GetInteger(false);
	stats.gotMedal				= args.GetInteger(false);
	stats.success				= args.GetInteger(false);
	stats.failed				= args.GetInteger(false);

	// Notify about stats
	handlers().notify<CGameHandlers::ServerCommand_Stats, const stats_t&>(stats);
}

void CGameModuleBase::SCmd_Stopwatch(TokenParser& args)
{
	const uint64_t startTime = args.GetInteger64(false);
	const uint64_t endTime = args.GetInteger64(false);
	handlers().notify<CGameHandlers::ServerCommand_Stopwatch>(startTime, endTime);
}

void CGameModuleBase::SCmd_ServerLag(TokenParser& args)
{
	cgs.serverLagTime = getTime();
	handlers().notify<CGameHandlers::ServerCommand_ServerLag>();
}

void CGameModuleBase::SCmd_Stufftext(TokenParser& args)
{
	handlers().notify<CGameHandlers::ServerCommand_Stufftext>(args);
}

CGameModule6::CGameModule6(const CGameImports& inImports)
	: CGameModuleBase(inImports)
{
}

void CGameModule6::handleCGMessage(MSG& msg, uint8_t msgId)
{
	// Version 6
	enum class cgmessage_e
	{
		bullet1 = 1,
		bullet2,
		bullet3,
		bullet4,
		bullet5,
		impact1,
		impact2,
		impact3,
		impact4,
		impact5,
		impact_melee,
		explo1,
		explo2,
		unk1,
		effect1,
		effect2,
		effect3,
		effect4,
		effect5,
		effect6,
		effect7,
		effect8,
		debris_crate,
		debris_window,
		tracer_visible,
		tracer_hidden,
		huddraw_shader,
		huddraw_align,
		huddraw_rect,
		huddraw_virtualscreen,
		huddraw_color,
		huddraw_alpha,
		huddraw_string,
		huddraw_font,
		notify_hit,
		notify_kill,
		playsound_entity,
	};

	uint32_t temp;
	uint32_t count;
	uint8_t index;
	uint32_t large = false;
	StringMessage strVal;
	Vector vecTmp;
	Vector vecStart, vecEnd;
	Vector vecArray[64];
	MsgTypesHelper msgHelper(msg);

	const cgmessage_e msgType((cgmessage_e)msgId);
	switch (msgType)
	{
	case cgmessage_e::bullet1:
		vecTmp = msgHelper.ReadVectorCoord();
	case cgmessage_e::bullet2:
	case cgmessage_e::bullet5:
	{
		vecStart = msgHelper.ReadVectorCoord();
		vecTmp = vecStart;
		vecArray[0] = msgHelper.ReadVectorCoord();
		large = msg.ReadBool();

		if (msgType == cgmessage_e::bullet1 || msgType == cgmessage_e::bullet2)
		{
			handlers().notify<CGameHandlers::MakeBulletTracer>(
				const_cast<const Vector&>(vecStart),
				const_cast<const Vector&>(vecTmp),
				const_cast<const Vector&>(vecArray[0]),
				1u,
				large,
				msgType == cgmessage_e::bullet1 ? 1u : 0u,
				1.f
			);
		}
		else
		{
			handlers().notify<CGameHandlers::MakeBubbleTrail>(
				const_cast<const Vector&>(vecStart),
				const_cast<const Vector&>(vecEnd),
				large,
				1.f
			);
		}
		break;
	}
	case cgmessage_e::bullet3:
	{
		vecTmp = msgHelper.ReadVectorCoord();
		temp = 0;
		msg.ReadBits(&temp, 6);
	}
	case cgmessage_e::bullet4:
	{
		vecStart = msgHelper.ReadVectorCoord();
		large = msg.ReadBool();
		msg.ReadBits(&count, 6);

		for (size_t i = 0; i < count; ++i) {
			vecArray[i] = msgHelper.ReadVectorCoord();
		}

		handlers().notify<CGameHandlers::MakeBulletTracer>(
			const_cast<const Vector&>(vecTmp),
			const_cast<const Vector&>(vecTmp),
			const_cast<const Vector&>(vecArray[0]),
			count,
			large,
			temp,
			1.f
			);
		break;
	}
	case cgmessage_e::impact1:
	case cgmessage_e::impact2:
	case cgmessage_e::impact3:
	case cgmessage_e::impact4:
	case cgmessage_e::impact5:
		vecStart = msgHelper.ReadVectorCoord();
		vecEnd = msgHelper.ReadDir();
		large = msg.ReadBool();

		handlers().notify<CGameHandlers::Impact>(
			const_cast<const Vector&>(vecStart),
			const_cast<const Vector&>(vecEnd),
			large
		);
		break;
	case cgmessage_e::impact_melee:
		vecStart = msgHelper.ReadVectorCoord();
		vecEnd = msgHelper.ReadVectorCoord();

		handlers().notify<CGameHandlers::MeleeImpact>(
			const_cast<const Vector&>(vecStart),
			const_cast<const Vector&>(vecEnd)
			);
		break;
	case cgmessage_e::explo1:
	case cgmessage_e::explo2:
	{
		uint32_t effectId = msgId == 12 || msgId != 13 ? 63 : 64;
		vecStart = msgHelper.ReadVectorCoord();

		handlers().notify<CGameHandlers::MakeExplosionEffect>(
			const_cast<const Vector&>(vecStart),
			getEffectId(effectId)
			);
		break;
	}
	case cgmessage_e::effect1:
	case cgmessage_e::effect2:
	case cgmessage_e::effect3:
	case cgmessage_e::effect4:
	case cgmessage_e::effect5:
	case cgmessage_e::effect6:
	case cgmessage_e::effect7:
	case cgmessage_e::effect8:
		vecStart = msgHelper.ReadVectorCoord();
		vecEnd = msgHelper.ReadDir();

		handlers().notify<CGameHandlers::MakeEffect>(
			const_cast<const Vector&>(vecStart),
			const_cast<const Vector&>(vecEnd),
			getEffectId(msgId + 67)
		);
		break;
	case cgmessage_e::debris_crate:
	case cgmessage_e::debris_window:
		vecStart = msgHelper.ReadVectorCoord();
		temp = msg.ReadByte();
		if (msgType == cgmessage_e::debris_crate)
		{
			handlers().notify<CGameHandlers::SpawnDebris>(
				CGameHandlers::debrisType_e::crate,
				const_cast<const Vector&>(vecStart),
				temp
			);
		}
		else
		{
			handlers().notify<CGameHandlers::SpawnDebris>(
				CGameHandlers::debrisType_e::window,
				const_cast<const Vector&>(vecStart),
				temp
			);
		}
		break;
	case cgmessage_e::tracer_visible:
		vecTmp = msgHelper.ReadVectorCoord();
		vecStart = msgHelper.ReadVectorCoord();
		vecArray[0] = msgHelper.ReadVectorCoord();
		large = msg.ReadBool();

		handlers().notify<CGameHandlers::MakeBulletTracer>(
			const_cast<const Vector&>(vecTmp),
			const_cast<const Vector&>(vecStart),
			const_cast<const Vector&>(vecArray[0]),
			1u,
			large,
			1u,
			1.f
		);
		break;
	case cgmessage_e::tracer_hidden:
		vecTmp = vec_zero;
		vecStart = msgHelper.ReadVectorCoord();
		vecArray[0] = msgHelper.ReadVectorCoord();
		large = msg.ReadBool();
		
		handlers().notify<CGameHandlers::MakeBulletTracer>(
			const_cast<const Vector&>(vecTmp),
			const_cast<const Vector&>(vecStart),
			const_cast<const Vector&>(vecArray[0]),
			1u,
			large,
			0u,
			1.f
		);
		break;
	case cgmessage_e::huddraw_shader:
		index = msg.ReadByte();
		strVal = msg.ReadString();
		
		handlers().notify<CGameHandlers::HudDraw_Shader>(index, (const char*)strVal);
		break;
	case cgmessage_e::huddraw_align:
	{
		index = msg.ReadByte();
		uint8_t hAlign = 0, vAlign = 0;
		msg.ReadBits(&hAlign, 2);
		msg.ReadBits(&vAlign, 2);

		using namespace CGameHandlers;
		handlers().notify<CGameHandlers::HudDraw_Align>(index, horizontalAlign_e(hAlign), verticalAlign_e(vAlign));
		break;
	}
	case cgmessage_e::huddraw_rect:
	{
		index = msg.ReadByte();

		const uint16_t x = msg.ReadUShort();
		const uint16_t y = msg.ReadUShort();
		const uint16_t width = msg.ReadUShort();
		const uint16_t height = msg.ReadUShort();

		handlers().notify<CGameHandlers::HudDraw_Rect>(index, x, y, width, height);
		break;
	}
	case cgmessage_e::huddraw_virtualscreen:
	{
		index = msg.ReadByte();
		const bool virtualScreen = msg.ReadBool();

		handlers().notify<CGameHandlers::HudDraw_VirtualScreen>(index, virtualScreen);
		break;
	}
	case cgmessage_e::huddraw_color:
	{
		index = msg.ReadByte();
		const Vector col =
		{
			(float)msg.ReadByte() / 255.f,
			(float)msg.ReadByte() / 255.f,
			(float)msg.ReadByte() / 255.f
		};

		// Divide by 255 to get float color
		handlers().notify<CGameHandlers::HudDraw_Color>(index, col);
		break;
	}
	case cgmessage_e::huddraw_alpha:
	{
		index = msg.ReadByte();
		const float alpha = (float)msg.ReadByte() / 255.f;

		handlers().notify<CGameHandlers::HudDraw_Alpha>(index, alpha);
		break;
	}
	case cgmessage_e::huddraw_string:
	{
		index = msg.ReadByte();
		strVal = msg.ReadString();

		handlers().notify<CGameHandlers::HudDraw_String>(index, (const char*)strVal);
		break;
	}
	case cgmessage_e::huddraw_font:
	{
		index = msg.ReadByte();
		strVal = msg.ReadString();

		handlers().notify<CGameHandlers::HudDraw_Font>(index, (const char*)strVal);
		break;
	}
	case cgmessage_e::notify_hit:
		handlers().notify<CGameHandlers::HitNotify>();
		break;
	case cgmessage_e::notify_kill:
		handlers().notify<CGameHandlers::KillNotify>();
		break;
	case cgmessage_e::playsound_entity:
		vecStart = msgHelper.ReadVectorCoord();

		msg.ReadBits(&temp, 1);
		msg.ReadBits(&index, 6);
		strVal = msg.ReadString();

		handlers().notify<CGameHandlers::VoiceMessage>(
			const_cast<const Vector&>(vecStart),
			(bool)temp,
			(uint8_t)index,
			(const char*)strVal
		);
		break;
	}
}

effects_e CGameModule6::getEffectId(uint32_t effectId)
{
	static effects_e effectList[] =
	{
	effects_e::bh_paper_lite,
	effects_e::bh_paper_hard,
	effects_e::bh_wood_lite,
	effects_e::bh_wood_hard,
	effects_e::bh_metal_lite,
	effects_e::bh_metal_hard,
	effects_e::bh_stone_lite,
	effects_e::bh_stone_hard,
	effects_e::bh_dirt_lite,
	effects_e::bh_dirt_hard,
	// 10
	effects_e::bh_metal_lite,
	effects_e::bh_metal_hard,
	effects_e::bh_grass_lite,
	effects_e::bh_grass_hard,
	effects_e::bh_mud_lite,
	// 15
	effects_e::bh_mud_hard,
	effects_e::bh_water_lite,
	effects_e::bh_water_hard,
	effects_e::bh_glass_lite,
	effects_e::bh_glass_hard,
	// 20
	effects_e::bh_stone_lite,
	effects_e::bh_stone_hard,
	effects_e::bh_sand_lite,
	effects_e::bh_sand_hard,
	effects_e::bh_foliage_lite,
	// 25
	effects_e::bh_foliage_hard,
	effects_e::bh_snow_lite,
	effects_e::bh_snow_hard,
	effects_e::bh_carpet_lite,
	effects_e::bh_carpet_hard,
	// 30
	effects_e::bh_human_uniform_lite,
	effects_e::bh_human_uniform_hard,
	effects_e::water_trail_bubble,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 35
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 40
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 45
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 50
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 55
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 60
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::grenexp_base,
	effects_e::bazookaexp_base,
	// 65
	effects_e::grenexp_paper,
	effects_e::grenexp_wood,
	effects_e::grenexp_metal,
	effects_e::grenexp_stone,
	effects_e::grenexp_dirt,
	// 70
	effects_e::grenexp_metal,
	effects_e::grenexp_grass,
	effects_e::grenexp_mud,
	effects_e::grenexp_water,
	effects_e::bh_stone_hard,
	// 75
	effects_e::grenexp_gravel,
	effects_e::grenexp_sand,
	effects_e::grenexp_foliage,
	effects_e::grenexp_snow,
	effects_e::grenexp_carpet,
	// 80
	effects_e::water_ripple_still,
	effects_e::water_ripple_moving,
	effects_e::barrel_oil_leak_big,
	effects_e::barrel_oil_leak_medium,
	effects_e::barrel_oil_leak_small,
	// 85
	effects_e::barrel_oil_leak_splat,
	effects_e::barrel_water_leak_big,
	effects_e::barrel_water_leak_medium,
	effects_e::barrel_water_leak_small,
	effects_e::barrel_water_leak_splat,
	// 90
	effects_e::fs_light_dust,
	effects_e::fs_heavy_dust,
	effects_e::fs_dirt,
	effects_e::fs_grass,
	effects_e::fs_mud,
	// 95
	effects_e::fs_puddle,
	effects_e::fs_sand,
	effects_e::fs_snow,
	effects_e::bh_stone_hard
	};

	static constexpr size_t numEffects = sizeof(effectList) / sizeof(effectList[0]);
	static_assert(numEffects == 99);

	if (effectId < numEffects) {
		return effectList[effectId];
	}

	return effects_e::bh_stone_hard;
}

void Network::CGameModule6::normalizePlayerState(playerState_t& ps)
{
	const uint32_t pmFlags = ps.pm_flags;
	uint32_t newPmFlags = 0;

	// Convert AA PlayerMove flags to SH/BT flags
	newPmFlags |= pmFlags & PMF_DUCKED;
	for (size_t i = 2; i < 32; ++i)
	{
		if (pmFlags & (1 << (i + 2))) {
			newPmFlags |= (1 << i);
		}
	}

	// So that flags are normalized across modules
	ps.pm_flags = newPmFlags;
}

void Network::CGameModule6::parseFogInfo(const char* s, environment_t& env)
{
	int tmp = 0;
	sscanf(
		s,
		"%d %f %f %f %f",
		&tmp,
		&env.farplaneDistance,
		&env.farplaneColor[0],
		&env.farplaneColor[1],
		&env.farplaneColor[2]
	);
	// Parse bool
	env.farplaneCull = tmp;
}

CGameModule15::CGameModule15(const CGameImports& inImports)
	: CGameModuleBase(inImports)
{
}

void CGameModule15::handleCGMessage(MSG& msg, uint8_t msgId)
{
	// Version 15
	enum class cgmessage_e
	{
		bullet1 = 1,
		bullet2,
		bullet3,
		bullet4,
		bullet5,
		impact1,
		impact2,
		impact3,
		impact4,
		impact5,
		impact6,
		impact_melee,
		explo1,
		explo2,
		explo3,
		explo4,
		unk1,
		effect1,
		effect2,
		effect3,
		effect4,
		effect5,
		effect6,
		effect7,
		effect8,
		debris_crate,
		debris_window,
		tracer_visible,
		tracer_hidden,
		huddraw_shader,
		huddraw_align,
		huddraw_rect,
		huddraw_virtualscreen,
		huddraw_color,
		huddraw_alpha,
		huddraw_string,
		huddraw_font,
		notify_hit,
		notify_kill,
		playsound_entity,
		// FIXME: must analyze precisely what it does
		effect9
	};

	uint32_t temp;
	uint32_t count;
	uint32_t effectId;
	uint8_t index;
	uint32_t large = false;
	StringMessage strVal;
	Vector vecTmp;
	Vector vecStart, vecEnd;
	Vector vecArray[64];
	MsgTypesHelper msgHelper(msg);

	struct
	{
		float readBulletSize(MSG& msg)
		{
			static constexpr float MIN_BULLET_SIZE = 1.f / 512.f;

			const bool hasSize = msg.ReadBool();
			if (hasSize)
			{
				uint16_t intSize = 0;
				msg.ReadBits(&intSize, 10);

				float bulletSize = (float)intSize / 512.f;
				if (bulletSize < MIN_BULLET_SIZE) bulletSize = MIN_BULLET_SIZE;

				return bulletSize;
			}
			else {
				return 1.f;
			}
		}
	} utils;

	cgmessage_e msgType((cgmessage_e)msgId);
	switch (msgType)
	{
	case cgmessage_e::bullet1:
		vecTmp = msgHelper.ReadVectorCoord();
	case cgmessage_e::bullet2:
	case cgmessage_e::bullet5:
	{
		vecStart = msgHelper.ReadVectorCoord();
		vecTmp = vecStart;
		vecArray[0] = msgHelper.ReadVectorCoord();

		uint8_t iLarge = 0;
		msg.ReadBits(&iLarge, 2);

		const float bulletSize = utils.readBulletSize(msg);

		if (msgType == cgmessage_e::bullet1 || msgType == cgmessage_e::bullet2)
		{
			handlers().notify<CGameHandlers::MakeBulletTracer>(
				const_cast<const Vector&>(vecStart),
				const_cast<const Vector&>(vecTmp),
				const_cast<const Vector&>(vecArray[0]),
				1u,
				large,
				msgType == cgmessage_e::bullet1 ? 1u : 0u,
				1.f
			);
		}
		else
		{
			handlers().notify<CGameHandlers::MakeBubbleTrail>(
				const_cast<const Vector&>(vecStart),
				const_cast<const Vector&>(vecEnd),
				large,
				1.f
			);
		}
		break;
	}
	case cgmessage_e::bullet3:
	case cgmessage_e::bullet4:
	{
		if (msgType == cgmessage_e::bullet3)
		{
			vecTmp = msgHelper.ReadVectorCoord();
			temp = 0;
			msg.ReadBits(&temp, 6);
		}
		else {
			temp = 0;
		}

		vecStart = msgHelper.ReadVectorCoord();

		large = 0;
		msg.ReadBits(&large, 2);

		const float bulletSize = utils.readBulletSize(msg);

		count = 0;
		msg.ReadBits(&count, 6);

		for (size_t i = 0; i < count; ++i) {
			vecArray[i] = msgHelper.ReadVectorCoord();
		}

		if (count)
		{
			handlers().notify<CGameHandlers::MakeBulletTracer>(
				const_cast<const Vector&>(vecStart),
				const_cast<const Vector&>(vecTmp),
				const_cast<const Vector&>(vecArray[0]),
				count,
				large,
				temp,
				1.f
				);
		}
		break;
	}
	case cgmessage_e::impact1:
	case cgmessage_e::impact2:
	case cgmessage_e::impact3:
	case cgmessage_e::impact4:
	case cgmessage_e::impact5:
	{
		vecStart = msgHelper.ReadVectorCoord();
		vecEnd = msgHelper.ReadDir();

		uint8_t iLarge = 0;
		msg.ReadBits(&iLarge, 2);

		handlers().notify<CGameHandlers::Impact>(
			const_cast<const Vector&>(vecStart),
			const_cast<const Vector&>(vecEnd),
			large
		);
		break;
	}
	case cgmessage_e::impact_melee:
		vecStart = msgHelper.ReadVectorCoord();
		vecEnd = msgHelper.ReadVectorCoord();

		handlers().notify<CGameHandlers::MeleeImpact>(
			const_cast<const Vector&>(vecStart),
			const_cast<const Vector&>(vecEnd)
		);
		break;
	case cgmessage_e::explo1:
	case cgmessage_e::explo2:
	case cgmessage_e::explo3:
	case cgmessage_e::explo4:
		vecStart = msgHelper.ReadVectorCoord();

		switch (msgType)
		{
		case cgmessage_e::explo1:
			effectId = 63;
			break;
		case cgmessage_e::explo2:
			effectId = 64;
			break;
		case cgmessage_e::explo3:
			effectId = 65;
			break;
		case cgmessage_e::explo4:
			effectId = 66;
			break;
		default:
			effectId = 63;
		}

		handlers().notify<CGameHandlers::MakeExplosionEffect>(
			const_cast<const Vector&>(vecStart),
			getEffectId(effectId)
		);
		break;
	case cgmessage_e::effect1:
	case cgmessage_e::effect2:
	case cgmessage_e::effect3:
	case cgmessage_e::effect4:
	case cgmessage_e::effect5:
	case cgmessage_e::effect6:
	case cgmessage_e::effect7:
	case cgmessage_e::effect8:
		vecStart = msgHelper.ReadVectorCoord();
		vecEnd = msgHelper.ReadDir();

		handlers().notify<CGameHandlers::MakeEffect>(
			const_cast<const Vector&>(vecStart),
			const_cast<const Vector&>(vecEnd),
			getEffectId(msgId + 75)
		);
		break;
	case cgmessage_e::debris_crate:
	case cgmessage_e::debris_window:
		vecStart = msgHelper.ReadVectorCoord();
		temp = msg.ReadByte();

		if (msgType == cgmessage_e::debris_crate)
		{
			handlers().notify<CGameHandlers::SpawnDebris>(
				CGameHandlers::debrisType_e::crate,
				const_cast<const Vector&>(vecStart),
				temp
			);
		}
		else
		{
			handlers().notify<CGameHandlers::SpawnDebris>(
				CGameHandlers::debrisType_e::window,
				const_cast<const Vector&>(vecStart),
				temp
			);
		}
		break;
	case cgmessage_e::tracer_visible:
	{
		vecTmp = msgHelper.ReadVectorCoord();
		vecStart = msgHelper.ReadVectorCoord();
		vecArray[0] = msgHelper.ReadVectorCoord();

		uint8_t iLarge = 0;
		msg.ReadBits(&iLarge, 2);
		const float bulletSize = utils.readBulletSize(msg);

		handlers().notify<CGameHandlers::MakeBulletTracer>(
			const_cast<const Vector&>(vecTmp),
			const_cast<const Vector&>(vecStart),
			const_cast<const Vector&>(vecArray[0]),
			1u,
			large,
			1u,
			1.f
		);
		break;
	}
	case cgmessage_e::tracer_hidden:
	{
		vecTmp = vec_zero;
		vecStart = msgHelper.ReadVectorCoord();
		vecArray[0] = msgHelper.ReadVectorCoord();

		uint8_t iLarge = 0;
		msg.ReadBits(&iLarge, 2);
		const float bulletSize = utils.readBulletSize(msg);

		handlers().notify<CGameHandlers::MakeBulletTracer>(
			const_cast<const Vector&>(vecTmp),
			const_cast<const Vector&>(vecStart),
			const_cast<const Vector&>(vecArray[0]),
			1u,
			large,
			0u,
			1.f
		);
		break;
	}
	case cgmessage_e::huddraw_shader:
		index = msg.ReadByte();
		strVal = msg.ReadString();

		handlers().notify<CGameHandlers::HudDraw_Shader>(index, (const char*)strVal);
		break;
	case cgmessage_e::huddraw_align:
	{
		index = msg.ReadByte();
		uint8_t hAlign = 0, vAlign = 0;
		msg.ReadBits(&hAlign, 2);
		msg.ReadBits(&vAlign, 2);

		using namespace CGameHandlers;
		handlers().notify<CGameHandlers::HudDraw_Align>(index, horizontalAlign_e(hAlign), verticalAlign_e(vAlign));
		break;
	}
	case cgmessage_e::huddraw_rect:
	{
		index = msg.ReadByte();

		const uint16_t x = msg.ReadUShort();
		const uint16_t y = msg.ReadUShort();
		const uint16_t width = msg.ReadUShort();
		const uint16_t height = msg.ReadUShort();

		handlers().notify<CGameHandlers::HudDraw_Rect>(index, x, y, width, height);
		break;
	}
	case cgmessage_e::huddraw_virtualscreen:
	{
		index = msg.ReadByte();
		const bool virtualScreen = msg.ReadBool();

		handlers().notify<CGameHandlers::HudDraw_VirtualScreen>(index, virtualScreen);
		break;
	}
	case cgmessage_e::huddraw_color:
	{
		index = msg.ReadByte();
		const Vector col =
		{
			(float)msg.ReadByte() / 255.f,
			(float)msg.ReadByte() / 255.f,
			(float)msg.ReadByte() / 255.f
		};

		// Divide by 255 to get float color
		handlers().notify<CGameHandlers::HudDraw_Color>(index, col);
		break;
	}
	case cgmessage_e::huddraw_alpha:
	{
		index = msg.ReadByte();
		const float alpha = (float)msg.ReadByte() / 255.f;

		handlers().notify<CGameHandlers::HudDraw_Alpha>(index, alpha);
		break;
	}
	case cgmessage_e::huddraw_string:
	{
		index = msg.ReadByte();
		strVal = msg.ReadString();

		handlers().notify<CGameHandlers::HudDraw_String>(index, (const char*)strVal);
		break;
	}
	case cgmessage_e::huddraw_font:
	{
		index = msg.ReadByte();
		strVal = msg.ReadString();

		handlers().notify<CGameHandlers::HudDraw_Font>(index, (const char*)strVal);
		break;
	}
	case cgmessage_e::notify_hit:
		handlers().notify<CGameHandlers::HitNotify>();
		break;
	case cgmessage_e::notify_kill:
		handlers().notify<CGameHandlers::KillNotify>();
		break;
	case cgmessage_e::playsound_entity:
		vecStart = msgHelper.ReadVectorCoord();

		msg.ReadBits(&temp, 1);
		msg.ReadBits(&index, 6);
		strVal = msg.ReadString();

		handlers().notify<CGameHandlers::VoiceMessage>(
			const_cast<const Vector&>(vecStart),
			(bool)temp,
			(uint8_t)index,
			(const char*)strVal
		);
		break;
	case cgmessage_e::effect9:
	{
		vecStart = msgHelper.ReadVectorCoord();
		vecEnd = msgHelper.ReadVectorCoord();

		const uint8_t val1 = msg.ReadByte();
		const uint8_t val2 = msg.ReadByte();
		// FIXME: not sure what it does
	}

	}
}

effects_e CGameModule15::getEffectId(uint32_t effectId)
{
	static effects_e effectList[] =
	{
	effects_e::bh_paper_lite,
	effects_e::bh_paper_hard,
	effects_e::bh_wood_lite,
	effects_e::bh_wood_hard,
	effects_e::bh_metal_lite,
	effects_e::bh_metal_hard,
	effects_e::bh_stone_lite,
	effects_e::bh_stone_hard,
	effects_e::bh_dirt_lite,
	effects_e::bh_dirt_hard,
	// 10
	effects_e::bh_metal_lite,
	effects_e::bh_metal_hard,
	effects_e::bh_grass_lite,
	effects_e::bh_grass_hard,
	effects_e::bh_mud_lite,
	// 15
	effects_e::bh_mud_hard,
	effects_e::bh_water_lite,
	effects_e::bh_water_hard,
	effects_e::bh_glass_lite,
	effects_e::bh_glass_hard,
	// 20
	effects_e::bh_stone_lite,
	effects_e::bh_stone_hard,
	effects_e::bh_sand_lite,
	effects_e::bh_sand_hard,
	effects_e::bh_foliage_lite,
	// 25
	effects_e::bh_foliage_hard,
	effects_e::bh_snow_lite,
	effects_e::bh_snow_hard,
	effects_e::bh_carpet_lite,
	effects_e::bh_carpet_hard,
	// 30
	effects_e::bh_human_uniform_lite,
	effects_e::bh_human_uniform_hard,
	effects_e::water_trail_bubble,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 35
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 40
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 45
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 50
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 55
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	// 60
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::bh_stone_hard,
	effects_e::grenexp_base,
	effects_e::bazookaexp_base,
	// 65
	effects_e::heavyshellexp_base,
	effects_e::tankexp_base,
	effects_e::grenexp_paper,
	effects_e::grenexp_wood,
	effects_e::grenexp_metal,
	// 70
	effects_e::grenexp_stone,
	effects_e::grenexp_dirt,
	effects_e::grenexp_metal,
	effects_e::grenexp_grass,
	effects_e::grenexp_mud,
	// 75
	effects_e::grenexp_water,
	effects_e::bh_stone_hard,
	effects_e::grenexp_gravel,
	effects_e::grenexp_sand,
	effects_e::grenexp_foliage,
	// 80
	effects_e::grenexp_snow,
	effects_e::grenexp_carpet,
	effects_e::heavyshellexp_dirt,
	effects_e::heavyshellexp_stone,
	effects_e::heavyshellexp_snow,
	// 85
	effects_e::tankexp_dirt,
	effects_e::tankexp_stone,
	effects_e::tankexp_snow,
	effects_e::bazookaexp_dirt,
	effects_e::bazookaexp_stone,
	// 90
	effects_e::bazookaexp_snow,
	effects_e::water_ripple_still,
	effects_e::water_ripple_moving,
	effects_e::barrel_oil_leak_big,
	effects_e::barrel_oil_leak_medium,
	// 95
	effects_e::barrel_oil_leak_small,
	effects_e::barrel_oil_leak_splat,
	effects_e::barrel_water_leak_big,
	effects_e::barrel_water_leak_medium,
	effects_e::barrel_water_leak_small,
	// 100
	effects_e::barrel_water_leak_splat,
	effects_e::fs_light_dust,
	effects_e::fs_heavy_dust,
	effects_e::fs_dirt,
	effects_e::fs_grass,
	// 105
	effects_e::fs_mud,
	effects_e::fs_puddle,
	effects_e::fs_sand,
	effects_e::fs_snow,
	effects_e::fx_fence_wood,
	// 110
	effects_e::bh_stone_hard,
	};

	static constexpr size_t numEffects = sizeof(effectList) / sizeof(effectList[0]);
	static_assert(numEffects == 111);

	if (effectId < numEffects) {
		return effectList[effectId];
	}

	return effects_e::bh_stone_hard;
}

void Network::CGameModule15::setupMove(Pmove& pmove)
{
	pmove_t& pm = pmove.get();
	// in SH/BT, can't lean by default
	// unless a dmflag specify it
	pm.canLean = getServerInfo().hasAnyDMFlags(DMFlags::DF_ALLOW_LEAN);
}

void Network::CGameModule15::parseFogInfo(const char* s, environment_t& env)
{
	int tmp = 0, tmp2 = 0;
	sscanf(
		s,
		"%d %f %f %f %f %f %f %f %d %f %f %f %f",
		&tmp,
		&env.farplaneDistance,
		&env.farplaneBias,
		&env.skyboxFarplane,
		&env.skyboxSpeed,
		&env.farplaneColor[0],
		&env.farplaneColor[1],
		&env.farplaneColor[2],
		&tmp2,
		&env.farclipOverride,
		&env.farplaneColorOverride[0],
		&env.farplaneColorOverride[1],
		&env.farplaneColorOverride[2]
	);
	// Parse bool
	env.farplaneCull = tmp;
	env.renderTerrain = tmp2;
}

const char* Network::getEffectName(effects_e effect)
{
	return effectsModel[(size_t)effect];
}

rain_t::rain_t()
	: density(0.f)
	, speed(0.f)
	, speedVary(0)
	, slant(0)
	, length(0.f)
	, minDist(0.f)
	, width(0.f)
	, numShaders(0)
{
}

float rain_t::getDensity() const
{
	return density;
}

float rain_t::getSpeed() const
{
	return speed;
}

float rain_t::getLength() const
{
	return length;
}

float rain_t::getMinimumDistance() const
{
	return minDist;
}

float rain_t::getWidth() const
{
	return width;
}

uint32_t rain_t::getSpeedVariation() const
{
	return speedVary;
}

uint32_t rain_t::getSlant() const
{
	return slant;
}

uint32_t rain_t::getNumShaders() const
{
	return numShaders;
}

const char* rain_t::getShader(uint8_t index) const
{
	return shader[index].c_str();
}

objective_t::objective_t()
	: flags(0)
{
}

uint32_t objective_t::getFlags() const
{
	return flags;
}

const char* objective_t::getText() const
{
	return text;
}

const Vector& objective_t::getLocation() const
{
	return location;
}

environment_t::environment_t()
	: farplaneCull(false)
	, farplaneDistance(0.f)
	, skyAlpha(0.f)
	, skyPortal(false)
	, skyboxFarplane(0.f)
	, skyboxSpeed(0.f)
	, farplaneBias(0.f)
	, farclipOverride(0.f)
	, renderTerrain(true)
{
}

bool environment_t::isFarplaneCulling() const
{
	return farplaneCull;
}

float environment_t::getFarplane() const
{
	return farplaneDistance;
}

const Vector& environment_t::getFarplaneColor() const
{
	return farplaneColor;
}

float environment_t::getSkyAlpha() const
{
	return skyAlpha;
}

bool environment_t::isSkyPortal() const
{
	return skyPortal;
}

float Network::environment_t::getFarplaneBias() const
{
	return farplaneBias;
}

float Network::environment_t::getSkyboxFarplane() const
{
	return skyboxFarplane;
}

float Network::environment_t::getSkyboxSpeed() const
{
	return skyboxSpeed;
}

float Network::environment_t::getFarclipOverride() const
{
	return farclipOverride;
}

const Vector& Network::environment_t::getFarplaneColorOverride() const
{
	return farplaneColorOverride;
}

bool Network::environment_t::shouldRenderTerrain() const
{
	return renderTerrain;
}

Scoreboard::Scoreboard(gameType_e inGameType)
	: gameType(inGameType)
{
}

size_t Scoreboard::getNumTeams() const
{
	return teamEntries.NumObjects();
}

const Scoreboard::teamEntry_t& Scoreboard::getTeam(size_t index)
{
	return teamEntries[index];
}

const Scoreboard::teamEntry_t* Scoreboard::getTeamByType(teamType_e type)
{
	const size_t numTeams = teamEntries.NumObjects();
	for (size_t i = 0; i < numTeams; ++i)
	{
		const teamEntry_t& entry = teamEntries[i];
		if (entry.teamNum == (uint32_t)type) {
			return &entry;
		}
	}

	return nullptr;
}

size_t Scoreboard::getNumPlayers() const
{
	return playerList.NumObjects();
}

const Scoreboard::player_t& Scoreboard::getPlayer(size_t index) const
{
	return playerList[index];
}

void Scoreboard::parse(TokenParser& tokenizer)
{
	uint32_t numEntries = tokenizer.GetInteger(false);
	if (numEntries > MAX_ENTRIES)
	{
		// Better not overflow with a bad number
		numEntries = MAX_ENTRIES;
	}

	playerList.reserve(numEntries);

	if (gameType > gameType_e::FreeForAll)
	{
		teamEntries.reserve(4);

		// TDM
		for (size_t i = 0; i < numEntries; ++i)
		{
			const int32_t id = tokenizer.GetInteger(false);
			if (id == -1) {
				parseTeamInfo(tokenizer);
			}
			else if (id == -2) {
				parseTeamEmpty(tokenizer);
			}
			else {
				parseTeamPlayer(id, tokenizer);
			}
		}
	}
	else
	{
		// Free-for-All
		for (size_t i = 0; i < numEntries; ++i)
		{
			const int32_t id = tokenizer.GetInteger(false);
			if (id == -1 || id == -2) {
				parseEmpty(tokenizer);
			}
			else {
				parsePlayer(id, tokenizer);
			}
		}
	}
}

void Scoreboard::parseTeamInfo(TokenParser& tokenizer)
{
	teamEntry_t* entry = new(teamEntries) teamEntry_t;

	entry->teamNum = tokenizer.GetInteger(false);
	entry->numKills = tokenizer.GetInteger(false);
	entry->numDeaths = tokenizer.GetInteger(false);

	// Skip the team string
	tokenizer.GetToken(false);

	entry->ping = tokenizer.GetInteger(false);
}

void Scoreboard::parseTeamPlayer(uint32_t clientNum, TokenParser& tokenizer)
{
	player_t* player = new(playerList) player_t;

	const int32_t teamNum = tokenizer.GetInteger(false);

	player->clientNum = clientNum;
	player->teamNum = abs(teamNum);
	// A negative teamNumber means that the player is dead
	player->alive = teamNum >= 0;
	player->numKills = tokenizer.GetInteger(false);
	player->numDeaths = tokenizer.GetInteger(false);
	const str timeString = tokenizer.GetToken(false);
	player->ping = tokenizer.GetInteger(false);

	// Parse time
	player->timeStamp = parseTime(timeString.c_str());
}

void Scoreboard::parseTeamEmpty(TokenParser& tokenizer)
{
	for (size_t i = 0; i < 5; ++i) {
		tokenizer.GetToken(false);
	}
}

void Scoreboard::parsePlayer(uint32_t clientNum, TokenParser& tokenizer)
{
	player_t* player = new(playerList) player_t;

	player->clientNum = clientNum;
	player->numKills = tokenizer.GetInteger(false);
	player->numDeaths = tokenizer.GetInteger(false);
	const str timeString = tokenizer.GetToken(false);
	player->ping = tokenizer.GetInteger(false);

	// Parse time
	player->timeStamp = parseTime(timeString.c_str());

	// No team
	player->teamNum = 0;
	// Can't determine with the data if the player is dead or alive.
	player->alive = true;
}

void Scoreboard::parseEmpty(TokenParser& tokenizer)
{
	for (size_t i = 0; i < 4; ++i) {
		tokenizer.GetToken(false);
	}
}

uint64_t Scoreboard::parseTime(const char* timeStr)
{
	uint32_t hours, minutes, seconds;
	if (sscanf(timeStr, "%i:%02i:%02i", &hours, &minutes, &seconds) != 3)
	{
		hours = 0;
		sscanf(timeStr, "%i:%02i", &minutes, &seconds);
	}

	return (seconds + minutes * 60 + hours * 24 * 60) * 1000;
}

uint32_t Scoreboard::teamEntry_t::getTeamNum() const
{
	return teamNum;
}

uint32_t Scoreboard::teamEntry_t::getNumKills() const
{
	return numKills;
}

uint32_t Scoreboard::teamEntry_t::getNumDeaths() const
{
	return numDeaths;
}

uint32_t Scoreboard::teamEntry_t::getPing() const
{
	return ping;
}

uint32_t Scoreboard::player_t::getClientNum() const
{
	return clientNum;
}

uint32_t Scoreboard::player_t::getTeamNum() const
{
	return teamNum;
}

uint32_t Scoreboard::player_t::getNumKills() const
{
	return numKills;
}

uint32_t Scoreboard::player_t::getNumDeaths() const
{
	return numDeaths;
}

uint64_t Scoreboard::player_t::getTimeStamp() const
{
	return timeStamp;
}

uint32_t Scoreboard::player_t::getPing() const
{
	return ping;
}

bool Scoreboard::player_t::isAlive() const
{
	return alive;
}

Network::clientInfo_t::clientInfo_t()
	: team(teamType_e::None)
{
}

const char* Network::clientInfo_t::getName() const
{
	return name.c_str();
}

Network::teamType_e Network::clientInfo_t::getTeam() const
{
	return team;
}

const PropertyObject& Network::clientInfo_t::getProperties() const
{
	return properties;
}
