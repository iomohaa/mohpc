#include <MOHPC/Network/Client/CGame/Prediction.h>
#include <MOHPC/Network/Client/CGame/Snapshot.h>
#include <MOHPC/Network/Client/CGame/ServerInfo.h>
#include <MOHPC/Network/Client/Time.h>
#include <MOHPC/Network/Client/UserInput.h>
#include <MOHPC/Network/pm/bg_public.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>
#include <MOHPC/Common/Math.h>
#include <MOHPC/Common/Log.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

static constexpr char MOHPC_LOG_NAMESPACE[] = "cgame_prediction";

class PmovePredict_ver8 : public IPmovePredict
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 5;
		maxRange = 8;
	}

	void setupPmove(const cgsInfo& serverInfo, Pmove& pmove) const override
	{
		pmove.canLeanWhileMoving = true;
		pmove.clearLeanOnExit = false;
	}
};

class PmovePredict_ver17 : public IPmovePredict
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 15;
		maxRange = 17;
	}

	void setupPmove(const cgsInfo& serverInfo, Pmove& pmove) const override
	{
		// in SH/BT, can't lean by default
		// unless a dmflag specify it
		pmove.canLeanWhileMoving = serverInfo.hasAnyDMFlags(DF::ALLOW_LEAN);
		pmove.clearLeanOnExit = true;
	}
};

PmovePredict_ver8 pmovePredict8;
PmovePredict_ver17 pmovePredict17;

MOHPC_OBJECT_DEFINITION(Prediction)
Prediction::Prediction(const protocolType_c& protocolType)
	: validPPS(false)
	, traceFunction(PmoveNoTracePtr)
{
	pmovePredict = IPmovePredict::get(protocolType.getProtocolVersionNumber());
}

void Prediction::process(const ClientTime& clientTime, const PredictionParm& pparm, bool predict)
{
	const SnapshotInfo* snap = pparm.processedSnapshots.getSnap();
	const SnapshotInfo* nextSnap = pparm.processedSnapshots.getNextSnap();
	// avoid predicting while in an invalid snap
	if (snap && !(snap->snapFlags & SNAPFLAG_NOT_ACTIVE))
	{
		if (!validPPS)
		{
			validPPS = true;
			predictedPlayerState = snap->ps;
		}

		// Calculate the interpolation time
		if (nextSnap)
		{
			using namespace ticks;

			const deltaTime_t delta = nextSnap->getServerTime() - snap->getServerTime();
			if (delta == deltaTime_t::zero()) {
				frameInterpolation = 0.f;
			}
			else
			{
				const deltaTime_t deltaSnap = clientTime.getSimulatedRemoteTime() - tickTime_t(snap->getServerTime().time_since_epoch());
				frameInterpolation = deltaSnap / duration_cast<deltaTimeFloat_t>(delta);
			}
		}
		else {
			frameInterpolation = 0.f;
		}

		if (predict)
		{
			// time to try predicting the player state
			predictPlayerState(clientTime.getSimulatedRemoteTime(), pparm);
		}
		else
		{
			// non-predicting local movement will grab the latest angles
			interpolatePlayerState(clientTime.getSimulatedRemoteTime(), pparm, true);
			return;
		}
	}
}

const playerState_t& Prediction::getPredictedPlayerState() const
{
	return predictedPlayerState;
}

void Prediction::predictPlayerState(tickTime_t simulatedRemoteTime, const PredictionParm& pparm)
{
	const SnapshotInfo* snap = pparm.processedSnapshots.getSnap();
	const SnapshotInfo* nextSnap = pparm.processedSnapshots.getNextSnap();

	// server can freeze the player and/or disable prediction for the player
	if (snap->ps.pm_flags & PMF_NO_PREDICTION || snap->ps.pm_flags & PMF_FROZEN)
	{
		interpolatePlayerState(simulatedRemoteTime, pparm, false);
		return;
	}

	// replay all commands for this frame
	const bool moved = replayAllCommands(pparm);

	// interpolate camera view (spectator, cutscenes, etc)
	interpolatePlayerStateCamera(simulatedRemoteTime, pparm);

	const EntityInfo* entInfo = pparm.processedSnapshots.getEntity(predictedPlayerState.groundEntityNum);
	if (entInfo && entInfo->interpolate)
	{
		const float f = frameInterpolation - 1.f;

		vec3_t delta;
		VecSubtract(entInfo->nextState.netorigin, entInfo->currentState.netorigin, delta);
		VectorScale(delta, f, delta);
		VecAdd(predictedPlayerState.origin, delta, predictedPlayerState.origin);
	}
}

void Prediction::interpolatePlayerState(tickTime_t simulatedRemoteTime, const PredictionParm& pparm, bool grabAngles)
{
	const SnapshotInfo* snap = pparm.processedSnapshots.getSnap();
	const SnapshotInfo* nextSnap = pparm.processedSnapshots.getNextSnap();

	playerState_t* out = &predictedPlayerState;

	*out = snap->ps;

	// interpolate the camera if necessary
	interpolatePlayerStateCamera(simulatedRemoteTime, pparm);

	// if we are still allowing local input, short circuit the view angles
	if (grabAngles)
	{
		const uintptr_t cmdNum = pparm.userInput->getCurrentCmdNumber();

		usercmd_t cmd;
		pparm.userInput->getUserCmd(cmdNum, cmd);

		Pmove::PM_UpdateViewAngles(out, cmd.getMovement());
	}

	// if the next frame is a teleport, we can't lerp to it
	if (pparm.processedSnapshots.doesTeleportNextFrame()) {
		return;
	}

	const SnapshotInfo* const prev = snap;
	const SnapshotInfo* const next = nextSnap;

	if (!next || next->getServerTime() <= prev->getServerTime())
	{
		if (!next) {
			MOHPC_LOG(Debug, "Prediction::interpolatePlayerState: nextSnap == NULL");
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

void Prediction::interpolatePlayerStateCamera(tickTime_t simulatedRemoteTime, const PredictionParm& pparm)
{
	const SnapshotInfo* snap = pparm.processedSnapshots.getSnap();
	const SnapshotInfo* nextSnap = pparm.processedSnapshots.getNextSnap();

	//
	// copy in the current ones if nothing else
	//
	VectorCopy(predictedPlayerState.camera_angles, cameraAngles);
	VectorCopy(predictedPlayerState.camera_origin, cameraOrigin);
	cameraFov = predictedPlayerState.fov;

	// if the next frame is a teleport, we can't lerp to it
	if (pparm.processedSnapshots.doesCameraCutNextFrame()) {
		return;
	}

	const SnapshotInfo* const prev = snap;
	const SnapshotInfo* const next = nextSnap;

	if (!next || next->getServerTime() <= prev->getServerTime()) {
		return;
	}

	using namespace ticks;
	const deltaTime_t currentSincePrev = simulatedRemoteTime - time_cast<tickTime_t>(prev->getServerTime());
	const deltaTime_t maxTime = next->getServerTime() - prev->getServerTime();
	const float f = currentSincePrev / duration_cast<deltaTimeFloat_t>(maxTime);

	// interpolate fov
	cameraFov = prev->ps.fov + f * (next->ps.fov - prev->ps.fov);

	if (!(snap->ps.pm_flags & PMF_CAMERA_VIEW))
	{
		// only interpolate if the player is in camera view
		return;
	}

	if (predictedPlayerState.camera_flags & CF_CAMERA_ANGLES_TURRETMODE)
	{
		VectorCopy(next->ps.camera_origin, predictedPlayerState.camera_origin);
		VectorCopy(next->ps.camera_angles, predictedPlayerState.camera_angles);
		return;
	}

	for (uint32_t i = 0; i < 3; i++)
	{
		predictedPlayerState.camera_origin[i] = prev->ps.camera_origin[i] + f * (next->ps.camera_origin[i] - prev->ps.camera_origin[i]);
		predictedPlayerState.camera_angles[i] = LerpAngle(prev->ps.camera_angles[i], next->ps.camera_angles[i], f);
	}
}

PredictionSettings& Prediction::getSettings()
{
	return settings;
}

bool Prediction::replayAllCommands(const PredictionParm& pparm)
{
	const SnapshotInfo* snap = pparm.processedSnapshots.getSnap();
	const SnapshotInfo* nextSnap = pparm.processedSnapshots.getNextSnap();

	// Pmove
	Pmove pmove;
	pmove_t& pm = pmove.get();
	pm.ps = &predictedPlayerState;
	pm.traceInterface = traceFunction.get();

	pm.tracemask = ContentFlags::MASK_PLAYERSOLID;
	if (pm.ps->pm_type == pmType_e::Dead)
	{
		// ignore other players
		pm.tracemask &= ~ContentFlags::MASK_DYNAMICBODY;
	}

	pm.noFootsteps = pparm.serverInfo.hasAnyDMFlags(DF::NO_FOOTSTEPS);
	// Set settings depending on the protocol/version
	pmovePredict->setupPmove(pparm.serverInfo, pmove);

	const playerState_t oldPlayerState = predictedPlayerState;

	// Grab the latest cmd
	const usercmd_t& latestCmd = pparm.userInput->getCommandFromLast(0);

	if (nextSnap && !pparm.processedSnapshots.doesTeleportNextFrame() && !pparm.processedSnapshots.doesTeleportThisFrame())
	{
		predictedPlayerState = nextSnap->ps;
		physicsTime = nextSnap->getServerTime();
	}
	else
	{
		predictedPlayerState = snap->ps;
		physicsTime = snap->getServerTime();
	}

	const uint32_t pmove_msec = settings.getPmoveMsec();
	pm.pmove_fixed = settings.isPmoveFixed();
	pm.pmove_msec = pmove_msec;

	bool moved = false;
	// play all previous commands up to the current
	for (uintptr_t cmdNum = pparm.userInput->getNumCommands(); cmdNum > 0; --cmdNum)
	{
		moved |= tryReplayCommand(pmove, pparm, oldPlayerState, latestCmd, cmdNum);
	}

	transitionPlayerState(pparm, predictedPlayerState, &oldPlayerState);

	return moved;
}

bool Prediction::tryReplayCommand(Pmove& pmove, const PredictionParm& pparm, const playerState_t& oldPlayerState, const usercmd_t& latestCmd, uintptr_t cmdNum)
{
	pmove_t& pm = pmove.get();

	pm.cmd = pparm.userInput->getCommandFromLast(cmdNum);

	if (pm.pmove_fixed) {
		pmove.PM_UpdateViewAngles(pm.ps, pm.cmd.getMovement());
	}

	// don't do anything if the time is before the snapshot player time
	if (pm.cmd.getServerTime() <= time_cast<tickTime_t>(predictedPlayerState.getCommandTime())) {
		return false;
	}

	// don't do anything if the command was from a previous map_restart
	if (pm.cmd.getServerTime() > latestCmd.getServerTime()) {
		return false;
	}

	if (predictedPlayerState.commandTime == oldPlayerState.commandTime)
	{
		if (pparm.processedSnapshots.doesTeleportThisFrame())
		{
			VectorClear(predictedError);
			pparm.processedSnapshots.clearTeleportThisFrame();
		}

		// FIXME: Should it have some sort of predicted error?
	}

	if (pm.pmove_fixed)
	{
		using namespace ticks;
		const milliseconds msec(pm.pmove_msec);

		pm.cmd.setServerTime(pm.cmd.getServerTime() + msec - milliseconds(1));
	}

	// Replay movement
	return replayMove(pmove, pm.cmd);
}

bool Prediction::replayMove(Pmove& pmove, usercmd_t& cmd)
{
	pmove_t& pm = pmove.get();

	if (pm.ps->feetfalling && pm.waterlevel <= 1)
	{
		// clear xy movement when falling or when under water
		cmd.getMovement().moveForward(0);
		cmd.getMovement().moveRight(0);
	}

	// calculate delta time between server command time and current client time
	const deltaTime_t deltaTime = pm.cmd.getServerTime() - time_cast<tickTime_t>(pm.ps->getCommandTime());

	// call move handler
	pmove.move();

	// additional movement
	// can be anything, from jumping to events/fire prediction
	extendMove(pmove, deltaTime);

	// valid move
	return true;
}

void Prediction::extendMove(Pmove& pmove, deltaTime_t deltaTime)
{
	const pmove_t& pm = pmove.get();

	using namespace ticks;
	const deltaTimeFloat_t frametime = duration_cast<deltaTimeFloat_t>(deltaTime);

	switch (pm.ps->pm_type)
	{
	case pmType_e::Noclip:
		// on the server, the origin is changed in pm code and in G_Physics_Noclip;
		// as a consequence, do the same client-side
		physicsNoclip(pmove, frametime);
		break;
	default:
		break;
	}

	// let the callee replay/predict other events that are not present in the original pm code
	// the event could be called inside the PM code, but it's not what the server expect (no sub-ticking)
	// and PM must remain identical to the server PM code
	handlers().replayCmdHandler.broadcast(pm.cmd, *pm.ps, frametime);
}

void Prediction::physicsNoclip(Pmove& pmove, deltaTimeFloat_t frametime)
{
	const pmove_t& pm = pmove.get();

	vec3_t delta;
	VectorScale(pm.ps->velocity, frametime.count(), delta);
	VecAdd(pm.ps->origin, delta, pm.ps->origin);
}

void Prediction::transitionPlayerState(const PredictionParm& pparm, const playerState_t& current, const playerState_t* old)
{
	if (current.getClientNum() != old->getClientNum())
	{
		pparm.processedSnapshots.makeTeleportThisFrame();
		// avoid any unwanted transition effects
		old = &current;
	}

	// FIXME: transition?

	// call custom handler for transition effects
	handlers().transitionPlayerStateHandler.broadcast(current, *old);
}

const Prediction::HandlerList& Prediction::handlers() const
{
	return handlerList;
}

Prediction::HandlerList& Prediction::handlers()
{
	return handlerList;
}

void Prediction::setTraceFunction(const ITraceFunctionPtr& func)
{
	traceFunction = func;
}

PredictionSettings::PredictionSettings()
	: pmove_msec(8)
	, pmove_fixed(false)
{
}

void PredictionSettings::setPmoveMsec(uint32_t value)
{
	pmove_msec = value;
}

uint32_t PredictionSettings::getPmoveMsec() const
{
	return pmove_msec;
}

void PredictionSettings::setPmoveFixed(bool value)
{
	pmove_fixed = value;
}

bool PredictionSettings::isPmoveFixed() const
{
	return pmove_fixed;
}
