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
			 * The server notifies clients when it is experiencing lags, such as hitches.
			 */
			struct ServerLag : public HandlerNotifyBase<void(netTime_t time)> {};
		}

		class ServerLagCommandHandler : public SingleCommandHandlerBase<Handlers::ServerLag>
		{
			MOHPC_NET_OBJECT_DECLARATION(ServerLagCommandHandler);

		public:
			MOHPC_NET_EXPORTS ServerLagCommandHandler(const SnapshotProcessorPtr& snapshotProcessor);
			MOHPC_NET_EXPORTS ~ServerLagCommandHandler();

		private:
			void handle(TokenParser& args);
		};
		using ServerLagCommandHandlerPtr = SharedPtr<ServerLagCommandHandler>;
	}
}
}
