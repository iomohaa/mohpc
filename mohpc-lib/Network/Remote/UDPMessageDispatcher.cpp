#include <MOHPC/Network/Remote/UDPMessageDispatcher.h>
#include <MOHPC/Network/Remote/Socket.h>

using namespace MOHPC;
using namespace MOHPC::Network;

MOHPC_OBJECT_DEFINITION(UDPCommunicator);

UDPCommunicator::UDPCommunicator(const IUdpSocketPtr& inSocket)
	: socket(inSocket ? inSocket : ISocketFactory::get()->createUdp())
{
}

size_t UDPCommunicator::send(const IRemoteIdentifier& identifier, const uint8_t* data, size_t size)
{
	if(!socket) {
		return 0;
	}

	const IPRemoteIdentifier* ipId = dynamic_cast<const IPRemoteIdentifier*>(&identifier);
	if(ipId)
	{
		// send to the address
		return socket->send(ipId->getAddress(), data, size);
	}

	return 0;
}

size_t UDPCommunicator::receive(IRemoteIdentifierPtr& remoteAddress, uint8_t* data, size_t size)
{
	if (!socket) {
		return 0;
	}

	NetAddrPtr addressPtr;
	size_t result = socket->receive(data, size, addressPtr);
	if(addressPtr)
	{
		// set the remote identifier from the ip address and return
		remoteAddress = makeShared<IPRemoteIdentifier>(addressPtr);
	}

	return result != -1 ? result : 0;
}

size_t UDPCommunicator::getIncomingSize()
{
	if (!socket) {
		return 0;
	}

	return socket->dataCount();
}

bool UDPCommunicator::waitIncoming(uint64_t timeout)
{
	if (!socket) {
		return false;
	}

	if(timeout)
	{
		// call wait if there is a timeout value
		return socket->wait(timeout);
	}
	else
	{
		// otherwise, just check if the data count is positive
		return socket->dataCount() > 0;
	}
}

IUdpSocketPtr UDPCommunicator::getSocket() const
{
	return socket;
}

MOHPC_OBJECT_DEFINITION(UDPBroadcastCommunicator);

UDPBroadcastCommunicator::UDPBroadcastCommunicator(const IUdpSocketPtr& inSocket, uint16_t inStartPort, uint16_t inEndPort)
	: UDPCommunicator(inSocket)
	, startPort(inStartPort)
	, endPort(inEndPort)
{
}

size_t UDPBroadcastCommunicator::send(const IRemoteIdentifier& identifier, const uint8_t* data, size_t size)
{
	if (!socket) {
		return 0;
	}

	// ignore the identifier and do a broadcast instead
	NetAddr4Ptr addr = NetAddr4::create();
	addr->setIp(-1, -1, -1, -1);

	for (uint16_t i = startPort; i < endPort; ++i)
	{
		addr->setPort(i);
		socket->send(addr, data, size);
	}

	return 0;
}
