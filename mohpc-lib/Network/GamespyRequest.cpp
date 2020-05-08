#include <MOHPC/Network/GamespyRequest.h>
#include <MOHPC/Utilities/Info.h>

using namespace MOHPC;
using namespace Network;

void IGamespyRequest::generateOutput(IMessageStream& output)
{
	Info info;
	generateInfo(info);

	// Send the type of query
	const char* queryName = queryId();

	const char* infoStr = info.GetString();
	if (infoStr)
	{
		output.Write(infoStr, strlen(infoStr));
		output.Write("\\final\\", 7);

		if (queryName) {
			output.Write("\\queryid\\", 9);
			output.Write(queryName, strlen(queryName));
			output.Write("\\", 1);
		}
	}
}

GamespyRequestParam::GamespyRequestParam(const ITcpSocketPtr& inSocket)
	: socket(inSocket)
{
}

MOHPC::Network::GamespyRequestParam::GamespyRequestParam()
{
}

void GamespyRequestParam::send(const uint8_t* buf, size_t size)
{
	socket->send(buf, size);
}

size_t GamespyRequestParam::receive(uint8_t* buf, size_t size)
{
	return socket->receive(buf, size);
}

bool GamespyRequestParam::hasData() const
{
	return socket && socket->wait(0);
}

size_t GamespyRequestParam::receiveSize() const
{
	return 512;
}

void MOHPC::Network::IGamespyServerRequest::generateOutput(IMessageStream& output)
{
	// Send the type of query
	const char* queryName = generateQuery();

	output.Write("\\", 1);
	output.Write(queryName, strlen(queryName));
	output.Write("\\", 1);
}

MOHPC::Network::GamespyUDPRequestParam::GamespyUDPRequestParam(const IUdpSocketPtr& inSocket, const netadr_t& inAddr)
	: socket(inSocket)
	, addr(inAddr)
{
}

MOHPC::Network::GamespyUDPRequestParam::GamespyUDPRequestParam()
{
}

void MOHPC::Network::GamespyUDPRequestParam::send(const uint8_t* buf, size_t size)
{
	socket->send(addr, buf, size);
}

size_t MOHPC::Network::GamespyUDPRequestParam::receive(uint8_t* buf, size_t size)
{
	return socket->receive(buf, size, addr);
}

bool MOHPC::Network::GamespyUDPRequestParam::hasData() const
{
	return socket && socket->wait(0);
}

size_t MOHPC::Network::GamespyUDPRequestParam::receiveSize() const
{
	return 2048;
}

const MOHPC::Network::netadr_t& MOHPC::Network::GamespyUDPRequestParam::getLastIp() const
{
	return addr;
}

MOHPC::Network::GamespyUDPBroadcastRequestParam::GamespyUDPBroadcastRequestParam()
{

}

MOHPC::Network::GamespyUDPBroadcastRequestParam::GamespyUDPBroadcastRequestParam(const IUdpSocketPtr& inSocket, uint16_t inStartPort, uint16_t inEndPort)
	: socket(inSocket)
	, startPort(inStartPort)
	, endPort(inEndPort)
{
}

void MOHPC::Network::GamespyUDPBroadcastRequestParam::send(const uint8_t* buf, size_t size)
{
	netadr_t addr;
	addr.ip[0] = 0xFF; addr.ip[1] = 0xFF; addr.ip[2] = 0xFF; addr.ip[3] = 0xFF;

	for (uint16_t i = startPort; i < endPort; ++i)
	{
		addr.port = i;
		socket->send(addr, buf, size);
	}
}

size_t MOHPC::Network::GamespyUDPBroadcastRequestParam::receive(uint8_t* buf, size_t size)
{
	return socket->receive(buf, size, lastIp);
}

bool MOHPC::Network::GamespyUDPBroadcastRequestParam::hasData() const
{
	return socket && socket->wait(0);
}

size_t MOHPC::Network::GamespyUDPBroadcastRequestParam::receiveSize() const
{
	return 2048;
}

const MOHPC::Network::netadr_t& MOHPC::Network::GamespyUDPBroadcastRequestParam::getLastIp() const
{
	return lastIp;
}
