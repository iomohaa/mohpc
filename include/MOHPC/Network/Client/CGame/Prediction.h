#pragma once

#include "../../NetGlobal.h"
#include "../../NetObject.h"
#include "../../ProtocolSingleton.h"
#include "../../Types/PlayerState.h"
#include "../../Types/Protocol.h"
#include "../../Types/NetTime.h"

#include "../../../Common/Vector.h"
#include "../../../Utility/HandlerList.h"
#include "../../../Utility/SharedPtr.h"
#include "../../../Utility/TickTypes.h"

#include "../../pm/bg_trace.h"

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
namespace Network
{
	class Pmove;
	class usercmd_t;
	class UserInput;
	class ITraceFunction;
	class ClientTime;

	namespace CGame
	{
		class cgsInfo;
		class SnapshotProcessor;

		namespace CGameHandlers
		{
			/**
			 * Called for replaying a move that have not been executed yet on server. At this point movement has been predicted.
			 * This is purely for movement/calculation algorithm.
			 *
			 * @param	ucmd		Input to replay.
			 * @param	ps			Player state where to apply movement.
			 * @param	frameTime	delta time between last cmd time and playerState command time. Usually client's frametime.
			 */
			struct ReplayMove : public HandlerNotifyBase<void(const usercmd_t& ucmd, playerState_t& ps, deltaTimeFloat_t frameTime)> {};

			/**
			 * Called when done predicting player state (transitioned). Can be used for predicting effects like fire and stance.
			 *
			 * @param	current		The final predicted player state.
			 * @param	old			Player state before prediction.
			 * @param	frameTime	delta time between last cmd time and playerState command time. Usually client's frametime.
			 */
			struct TransitionPlayerState : public HandlerNotifyBase<void(const playerState_t& current, const playerState_t& old)> {};
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

		private:
			uint32_t pmove_msec;
			bool pmove_fixed : 1;
		};

		struct PredictionParm
		{
		public:
			SnapshotProcessor& processedSnapshots;
			const cgsInfo& serverInfo;
			const UserInput* userInput;
		};

		class IPmovePredict : public IProtocolSingleton<IPmovePredict>
		{
		public:
			virtual void setupPmove(const cgsInfo& serverInfo, Pmove& pmove) const = 0;
		};

		/**
		 * This module predicts input and movement that were sent to the server.
		 */
		class Prediction
		{
			MOHPC_NET_OBJECT_DECLARATION(Prediction);

		public:
			struct HandlerList
			{
				FunctionList<CGameHandlers::ReplayMove> replayCmdHandler;
				FunctionList<CGameHandlers::TransitionPlayerState> transitionPlayerStateHandler;
			};

		public:
			MOHPC_NET_EXPORTS Prediction(const protocolType_c& protocolType);

			/**
			 * Process the prediction.
			 */
			MOHPC_NET_EXPORTS void process(const ClientTime& clientTime, const PredictionParm& pparm, bool predict);

			/** Return the prediction settings. */
			MOHPC_NET_EXPORTS PredictionSettings& getSettings();

			/** Set the trace function to use for pmove. */
			MOHPC_NET_EXPORTS void setTraceFunction(const ITraceFunctionPtr& func);

			/** Return the predicted player state. */
			MOHPC_NET_EXPORTS const playerState_t& getPredictedPlayerState() const;

			/**
			 * Predict the player state movement.
			 * It is not called in the tick() function. It gives a chance for the caller
			 * to immediately predict player state after the input has been processed.
			 */
			void predictPlayerState(tickTime_t simulatedRemoteTime, const PredictionParm& pparm);

			MOHPC_NET_EXPORTS const HandlerList& handlers() const;
			MOHPC_NET_EXPORTS HandlerList& handlers();

		private:
			/**
			 * Player state calculation
			 */
			void interpolatePlayerState(tickTime_t simulatedRemoteTime, const PredictionParm& pparm, bool grabAngles);
			void interpolatePlayerStateCamera(tickTime_t simulatedRemoteTime, const PredictionParm& pparm);
			//====

			/**
			 * Player movement
			 */
			bool replayAllCommands(const PredictionParm& pparm);
			bool tryReplayCommand(Pmove& pmove, const PredictionParm& pparm, const playerState_t& oldPlayerState, const usercmd_t& latestCmd, uintptr_t cmdNum);
			bool replayMove(Pmove& pmove, usercmd_t& cmd);
			void extendMove(Pmove& pmove, deltaTime_t deltaTime);
			void physicsNoclip(Pmove& pmove, deltaTimeFloat_t frametime);

		private:
			void transitionPlayerState(const PredictionParm& pparm, const playerState_t& current, const playerState_t* old);

		private:
			const IPmovePredict* pmovePredict;
			ITraceFunctionPtr traceFunction;
			netTime_t physicsTime;
			float frameInterpolation;
			float cameraFov;
			vec3_t predictedError;
			vec3_t cameraAngles;
			vec3_t cameraOrigin;
			playerState_t predictedPlayerState;
			PredictionSettings settings;
			HandlerList handlerList;
			bool validPPS : 1;
		};
		using PredictionPtr = SharedPtr<Prediction>;
	}
}
}
