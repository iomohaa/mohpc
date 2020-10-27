#include <MOHPC/Network/Client/GamespyRequest.h>
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

GamespyRequestParam::GamespyRequestParam()
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

void IGamespyServerRequest::generateOutput(IMessageStream& output)
{
	// Send the type of query
	const char* queryName = generateQuery();

	output.Write("\\", 1);
	output.Write(queryName, strlen(queryName));
	output.Write("\\", 1);
}

GamespyUDPRequestParam::GamespyUDPRequestParam(const IUdpSocketPtr& inSocket, const NetAddrPtr& inAddr)
	: socket(inSocket)
	, addr(inAddr)
{
}

GamespyUDPRequestParam::GamespyUDPRequestParam()
{
}

void GamespyUDPRequestParam::send(const uint8_t* buf, size_t size)
{
	socket->send(*addr, buf, size);
}

size_t GamespyUDPRequestParam::receive(uint8_t* buf, size_t size)
{
	return socket->receive(buf, size, addr);
}

bool GamespyUDPRequestParam::hasData() const
{
	return socket && socket->dataAvailable();
}

size_t GamespyUDPRequestParam::receiveSize() const
{
	return 2048;
}

const NetAddr& GamespyUDPRequestParam::getLastIp() const
{
	return *addr;
}

GamespyUDPBroadcastRequestParam::GamespyUDPBroadcastRequestParam()
{

}

GamespyUDPBroadcastRequestParam::GamespyUDPBroadcastRequestParam(const IUdpSocketPtr& inSocket, uint16_t inStartPort, uint16_t inEndPort)
	: socket(inSocket)
	, startPort(inStartPort)
	, endPort(inEndPort)
{
}

void GamespyUDPBroadcastRequestParam::send(const uint8_t* buf, size_t size)
{
	NetAddr4 addr;
	addr.ip[0] = 0xFF; addr.ip[1] = 0xFF; addr.ip[2] = 0xFF; addr.ip[3] = 0xFF;

	for (uint16_t i = startPort; i < endPort; ++i)
	{
		addr.port = i;
		socket->send(addr, buf, size);
	}
}

size_t GamespyUDPBroadcastRequestParam::receive(uint8_t* buf, size_t size)
{
	return socket->receive(buf, size, lastIp);
}

bool GamespyUDPBroadcastRequestParam::hasData() const
{
	return socket && socket->wait(0);
}

size_t GamespyUDPBroadcastRequestParam::receiveSize() const
{
	return 2048;
}

const NetAddrPtr& GamespyUDPBroadcastRequestParam::getLastIp() const
{
	return lastIp;
}
