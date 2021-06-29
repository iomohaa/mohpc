#pragma once

#include "../../NetGlobal.h"
#include "../../../Common/Vector.h"
#include "../../Types/PlayerState.h"
#include "../../../Utility/HandlerList.h"

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
namespace Network
{
	class Pmove;
	class usercmd_t;
	class UserInput;

	namespace CGame
	{
		class SnapshotProcessor;

		namespace CGameHandlers
		{
			/**
			 * Called each frame for replaying move that have not been executed yet on server.
			 *
			 * @param	ucmd		Input to replay.
			 * @param	ps			Player state where to apply movement.
			 * @param	frameTime	delta time between last cmd time and playerState command time. Usually client's frametime.
			 */
			struct ReplayMove : public HandlerNotifyBase<void(const usercmd_t& ucmd, playerState_t& ps, float frameTime)> {};
		}

		/**
		 * Client settings data structure.
		 */
		class MOHPC_NET_EXPORTS PredictionSettings
		{
		public:
			PredictionSettings();

			/**
			 * Update pmove sub-ticking.
			 * @param value Values are in range through [8, 33]
			 */
			void setPmoveMsec(uint32_t value);

			/** Return pmove sub-ticking milliseconds. */
			uint32_t getPmoveMsec() const;

			/**
			 * Set whether or not pmove sub-ticking should be using a fixed time.
			 *
			 * @param value true for fixed sub-ticking.
			 */
			void setPmoveFixed(bool value);

			/** Return whether or not pmove sub-ticking is a fixed time. */
			bool isPmoveFixed() const;

			/** Disable local prediction on client. */
			void disablePrediction();

			/** Enable local prediction on client. */
			void enablePrediction();

			/** Return whether or not the prediction is disabled. */
			bool isPredictionDisabled() const;

		private:
			uint32_t pmove_msec;
			bool pmove_fixed : 1;
			bool forceDisablePrediction : 1;
		};

		class Prediction
		{
		public:
			struct HandlerList
			{
				FunctionList<CGameHandlers::ReplayMove> replayCmdHandler;
			};

		public:
			Prediction();

			void setUserInputPtr(const UserInput* userInputPtr);
			void process(uint64_t serverTime, SnapshotProcessor& processedSnapshots);

			/** Return the prediction settings. */
			MOHPC_NET_EXPORTS PredictionSettings& getSettings();

			/** Return the predicted player state. */
			MOHPC_NET_EXPORTS const playerState_t& getPredictedPlayerState() const;

			/**
			 * Predict the player state movement.
			 * It is not called in the tick() function. It gives a chance for the caller
			 * to immediately predict player state after the input has been processed.
			 */
			MOHPC_NET_EXPORTS void predictPlayerState(uint64_t serverTime, SnapshotProcessor& processedSnapshots);

			MOHPC_NET_EXPORTS const HandlerList& handlers() const;
			MOHPC_NET_EXPORTS HandlerList& handlers();

			/**
			 * Player state calculation
			 */
			void interpolatePlayerState(uint64_t serverTime, SnapshotProcessor& processedSnapshots, bool grabAngles);
			void interpolatePlayerStateCamera(uint64_t serverTime, SnapshotProcessor& processedSnapshots);
			//====

			/**
			 * Player movement
			 */
			bool replayAllCommands(SnapshotProcessor& processedSnapshots);
			bool tryReplayCommand(Pmove& pmove, SnapshotProcessor& processedSnapshots, const playerState_t& oldPlayerState, const usercmd_t& latestCmd, uintptr_t cmdNum);
			bool replayMove(Pmove& pmove, usercmd_t& cmd);
			void extendMove(Pmove& pmove, uint32_t msec);
			void physicsNoclip(Pmove& pmove, float frametime);

		private:
			const UserInput* userInput;
			uint32_t physicsTime;
			float frameInterpolation;
			float cameraFov;
			Vector predictedError;
			Vector cameraAngles;
			Vector cameraOrigin;
			playerState_t predictedPlayerState;
			PredictionSettings settings;
			HandlerList handlerList;
			bool validPPS : 1;
		};
	}
}
}