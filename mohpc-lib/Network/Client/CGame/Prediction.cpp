#include <MOHPC/Network/Client/CGame/Prediction.h>
#include <MOHPC/Network/Client/CGame/Snapshot.h>
#include <MOHPC/Network/Client/CGame/ServerInfo.h>
#include <MOHPC/Network/Client/UserInput.h>

#include <MOHPC/Network/pm/bg_public.h>

#include <MOHPC/Assets/Managers/ShaderManager.h>

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
		pmove.canLeanWhileMoving = serverInfo.hasAnyDMFlags(DMFlags::DF_ALLOW_LEAN);
		pmove.clearLeanOnExit = true;
	}
};



ProtocolClassInstancier_Template<PmovePredict_ver8, IPmovePredict, 5, 8> pmovePredict8;
ProtocolClassInstancier_Template<PmovePredict_ver17, IPmovePredict, 15, 17> pmovePredict17;

Prediction::Prediction()
	: userInput(nullptr)
	, pmovePredict(nullptr)
	, validPPS(false)
	, traceFunction(PmoveNoTracePtr)
{
}

void Prediction::setProtocol(protocolType_c protocol)
{
	pmovePredict = IPmovePredict::get(protocol.getProtocolVersionNumber());
}

void Prediction::process(uint64_t serverTime, const PredictionParm& pparm)
{
	const SnapshotInfo* snap = pparm.processedSnapshots.getSnap();
	const SnapshotInfo* nextSnap = pparm.processedSnapshots.getNextSnap();
	if (snap && !(snap->snapFlags & SNAPFLAG_NOT_ACTIVE))
	{
		// Calculate the interpolation time
		if (nextSnap)
		{
			const uint32_t delta = nextSnap->serverTime - snap->serverTime;
			if (!delta) {
				frameInterpolation = 0.f;
			}
			else {
				frameInterpolation = (float)(serverTime - snap->serverTime) / (float)delta;
			}
		}
		else {
			frameInterpolation = 0.f;
		}

		predictPlayerState(serverTime, pparm);
	}
}

const playerState_t& Prediction::getPredictedPlayerState() const
{
	return predictedPlayerState;
}

void Prediction::predictPlayerState(uint64_t serverTime, const PredictionParm& pparm)
{
	const SnapshotInfo* snap = pparm.processedSnapshots.getSnap();
	const SnapshotInfo* nextSnap = pparm.processedSnapshots.getNextSnap();

	if (!snap || (snap->snapFlags & SNAPFLAG_NOT_ACTIVE))
	{
		// avoid predicting while in an invalid snap
		return;
	}

	if (!validPPS)
	{
		validPPS = true;
		predictedPlayerState = snap->ps;
	}

	// server can freeze the player and/or disable prediction for the player
	if (snap->ps.pm_flags & PMF_NO_PREDICTION || snap->ps.pm_flags & PMF_FROZEN)
	{
		interpolatePlayerState(serverTime, pparm, false);
		return;
	}

	// non-predicting local movement will grab the latest angles
	if (settings.isPredictionDisabled())
	{
		interpolatePlayerState(serverTime, pparm, true);
		return;
	}

	// replay all commands for this frame
	const bool moved = replayAllCommands(pparm);

	// interpolate camera view (spectator, cutscenes, etc)
	interpolatePlayerStateCamera(serverTime, pparm);

	const EntityInfo* entInfo = pparm.processedSnapshots.getEntity(predictedPlayerState.groundEntityNum);
	if (entInfo && entInfo->interpolate)
	{
		const float f = frameInterpolation - 1.f;

		predictedPlayerState.origin = predictedPlayerState.origin + (entInfo->nextState.netorigin - entInfo->currentState.netorigin) * f;
	}
}

void Prediction::interpolatePlayerState(uint64_t serverTime, const PredictionParm& pparm, bool grabAngles)
{
	const SnapshotInfo* snap = pparm.processedSnapshots.getSnap();
	const SnapshotInfo* nextSnap = pparm.processedSnapshots.getNextSnap();

	playerState_t* out = &predictedPlayerState;

	*out = snap->ps;

	// interpolate the camera if necessary
	interpolatePlayerStateCamera(serverTime, pparm);

	// if we are still allowing local input, short circuit the view angles
	if (grabAngles)
	{
		const uintptr_t cmdNum = userInput->getCurrentCmdNumber();

		usercmd_t cmd;
		userInput->getUserCmd(cmdNum, cmd);

		Pmove::PM_UpdateViewAngles(out, &cmd);
	}

	// if the next frame is a teleport, we can't lerp to it
	if (pparm.processedSnapshots.doesTeleportNextFrame()) {
		return;
	}

	const SnapshotInfo* const prev = snap;
	const SnapshotInfo* const next = nextSnap;

	if (!next || next->serverTime <= prev->serverTime)
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

void Prediction::interpolatePlayerStateCamera(uint64_t serverTime, const PredictionParm& pparm)
{
	const SnapshotInfo* snap = pparm.processedSnapshots.getSnap();
	const SnapshotInfo* nextSnap = pparm.processedSnapshots.getNextSnap();

	//
	// copy in the current ones if nothing else
	//
	cameraAngles = predictedPlayerState.camera_angles;
	cameraOrigin = predictedPlayerState.camera_origin;
	cameraFov = predictedPlayerState.fov;

	// if the next frame is a teleport, we can't lerp to it
	if (pparm.processedSnapshots.doesCameraCutNextFrame()) {
		return;
	}

	const SnapshotInfo* const prev = snap;
	const SnapshotInfo* const next = nextSnap;

	if (!next || next->serverTime <= prev->serverTime) {
		return;
	}

	const float f = (float)(serverTime - prev->serverTime) / (next->serverTime - prev->serverTime);

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

void Prediction::setUserInputPtr(const UserInput* userInputPtr)
{
	userInput = userInputPtr;
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

	pm.noFootsteps = pparm.serverInfo.hasAnyDMFlags(DMFlags::DF_NO_FOOTSTEPS);
	// Set settings depending on the protocol/version
	pmovePredict->setupPmove(pparm.serverInfo, pmove);

	const playerState_t oldPlayerState = predictedPlayerState;
	const uintptr_t current = userInput->getCurrentCmdNumber();

	// Grab the latest cmd
	usercmd_t latestCmd;
	userInput->getUserCmd(current, latestCmd);

	if (nextSnap && !pparm.processedSnapshots.doesTeleportNextFrame() && !pparm.processedSnapshots.doesTeleportThisFrame())
	{
		predictedPlayerState = nextSnap->ps;
		physicsTime = nextSnap->serverTime;
	}
	else
	{
		predictedPlayerState = snap->ps;
		physicsTime = snap->serverTime;
	}

	const uint32_t pmove_msec = settings.getPmoveMsec();
	pm.pmove_fixed = settings.isPmoveFixed();
	pm.pmove_msec = pmove_msec;

	bool moved = false;
	// play all previous commands up to the current
	for (uintptr_t cmdNum = CMD_BACKUP; cmdNum > 0; --cmdNum)
	{
		moved |= tryReplayCommand(pmove, pparm, oldPlayerState, latestCmd, current - cmdNum + 1);
	}

	transitionPlayerState(pparm, predictedPlayerState, &oldPlayerState);

	return moved;
}

bool Prediction::tryReplayCommand(Pmove& pmove, const PredictionParm& pparm, const playerState_t& oldPlayerState, const usercmd_t& latestCmd, uintptr_t cmdNum)
{
	pmove_t& pm = pmove.get();

	userInput->getUserCmd(cmdNum, pm.cmd);

	if (pm.pmove_fixed) {
		pmove.PM_UpdateViewAngles(pm.ps, &pm.cmd);
	}

	// don't do anything if the time is before the snapshot player time
	if (pm.cmd.serverTime <= predictedPlayerState.commandTime) {
		return false;
	}

	// don't do anything if the command was from a previous map_restart
	if (pm.cmd.serverTime > latestCmd.serverTime) {
		return false;
	}

	if (predictedPlayerState.commandTime == oldPlayerState.commandTime)
	{
		if (pparm.processedSnapshots.doesTeleportThisFrame())
		{
			predictedError = Vector();
			pparm.processedSnapshots.clearTeleportThisFrame();
		}

		// FIXME: Should it have some sort of predicted error?
	}

	if (pm.pmove_fixed)
	{
		pm.cmd.serverTime = (
			(pm.cmd.serverTime + pm.pmove_msec - 1)
			/ pm.pmove_msec
			) * pm.pmove_msec;
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
		cmd.moveForward(0);
		cmd.moveRight(0);
	}

	// calculate delta time between server command time and current client time
	const uint32_t msec = pm.cmd.serverTime - pm.ps->commandTime;

	// call move handler
	pmove.move();

	// additional movement
	// can be anything, from jumping to events/fire prediction
	extendMove(pmove, msec);

	// valid move
	return true;
}

void Prediction::extendMove(Pmove& pmove, uint32_t msec)
{
	const pmove_t& pm = pmove.get();

	const float frametime = float(msec / 1000.f);

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

void Prediction::physicsNoclip(Pmove& pmove, float frametime)
{
	const pmove_t& pm = pmove.get();
	pm.ps->origin += pm.ps->velocity * frametime;
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
	, forceDisablePrediction(false)
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

void PredictionSettings::disablePrediction()
{
	forceDisablePrediction = true;
}

void PredictionSettings::enablePrediction()
{
	forceDisablePrediction = false;
}

bool PredictionSettings::isPredictionDisabled() const
{
	return forceDisablePrediction;
}
