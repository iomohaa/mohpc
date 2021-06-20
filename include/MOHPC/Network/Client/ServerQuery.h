#pragma once

#include "../../Utility/MessageQueue.h"
#include "Server.h"

namespace MOHPC
{
namespace Network
{
	class ServerQuery : public IMessage
	{
		MOHPC_NET_OBJECT_DECLARATION(ServerQuery);

	public:
		MOHPC_NET_EXPORTS ServerQuery(const IServerPtr& serverValue, Callbacks::Query&& responseFuncValue, Callbacks::ServerTimeout&& timeoutFuncValue, size_t timeoutValue = 5000);

		void transmit() override;

	private:
		void queryResponse(const ReadOnlyInfo& response);
		void queryTimeout();

	private:
		IServerPtr server;
		Callbacks::Query responseFunc;
		Callbacks::ServerTimeout timeoutFunc;
		size_t timeout;
	};
	using ServerQueryPtr = SharedPtr<ServerQuery>;
}
}
