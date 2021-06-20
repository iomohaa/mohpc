#include <MOHPC/Network/Client/ServerQuery.h>
#include <utility>

using namespace MOHPC;
using namespace Network;

MOHPC_OBJECT_DEFINITION(ServerQuery);

ServerQuery::ServerQuery(const IServerPtr& serverValue, Callbacks::Query&& responseFuncValue, Callbacks::ServerTimeout&& timeoutFuncValue, size_t timeoutValue)
	: server(serverValue)
	, responseFunc(std::move(responseFuncValue))
	, timeoutFunc(std::move(timeoutFuncValue))
	, timeout(timeoutValue)
{
}

void ServerQuery::transmit()
{
	using namespace std::placeholders;

	server->query(
		std::bind(&ServerQuery::queryResponse, this, _1),
		std::bind(&ServerQuery::queryTimeout, this)
	);
}

void ServerQuery::queryResponse(const ReadOnlyInfo& response)
{
	responseFunc(response);
	done();
}

void ServerQuery::queryTimeout()
{
	timeoutFunc();
	done();
}
