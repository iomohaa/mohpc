#pragma once

#include "Base.h"
#include "../ServerInfo.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		class Scoreboard;

		namespace Handlers
		{
			/**
			 * Called from server after score has been parsed.
			 *
			 * @param	scores	Score data.
			 */
			struct ReceivedScores : public HandlerNotifyBase<void(const Scoreboard& scores)> {};
		}

		class ScoresCommandHandler : public SingleCommandHandlerBase<Handlers::ReceivedScores>
		{
			MOHPC_NET_OBJECT_DECLARATION(ScoresCommandHandler);

		public:
			MOHPC_NET_EXPORTS ScoresCommandHandler(const SnapshotProcessorPtr& snapshotProcessor, const cgsInfoPtr& cgsPtr);
			MOHPC_NET_EXPORTS ~ScoresCommandHandler();

		private:
			void handle(TokenParser& args) override;

		private:
			cgsInfoPtr cgs;
		};
		using ScoresCommandHandlerPtr = SharedPtr<ScoresCommandHandler>;
	}
}
}
