#pragma once

#include "Base.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		namespace Handlers
		{
			/**
			 * Called to display a stopwatch.
			 *
			 * @param	startTime	Start time of the stopwatch.
			 * @param	endTime		End time of the stopwatch.
			 */
			struct ReceivedStopwatch : public HandlerNotifyBase<void(netTime_t startTime, netTime_t endTime)> {};
		}

		class StopwatchCommandHandler : public SingleCommandHandlerBase<Handlers::ReceivedStopwatch>
		{
			MOHPC_NET_OBJECT_DECLARATION(StopwatchCommandHandler);

		public:
			MOHPC_NET_EXPORTS StopwatchCommandHandler(const SnapshotProcessorPtr& snapshotProcessor);
			MOHPC_NET_EXPORTS ~StopwatchCommandHandler();

		private:
			void handle(TokenParser& args) override;
		};
		using StopwatchCommandHandlerPtr = SharedPtr<StopwatchCommandHandler>;
}
}
}
