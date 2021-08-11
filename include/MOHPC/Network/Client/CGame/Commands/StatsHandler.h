#pragma once

#include "Base.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		struct stats_t;

		namespace Handlers
		{
			/**
			 * Called from server for statistics.
			 *
			 * @param	stats	Statistics.
			 */
			struct ReceivedStats : public HandlerNotifyBase<void(const stats_t& stats)> {};
		}

		class StatsCommandHandler : public SingleCommandHandlerBase<Handlers::ReceivedStats>
		{
			MOHPC_NET_OBJECT_DECLARATION(StatsCommandHandler);

		public:
			MOHPC_NET_EXPORTS StatsCommandHandler(const SnapshotProcessorPtr& snapshotProcessor);
			MOHPC_NET_EXPORTS ~StatsCommandHandler();

		private:
			void handle(TokenParser& args) override;
		};
		using StatsCommandHandlerPtr = SharedPtr<StatsCommandHandler>;
	}
}
}
