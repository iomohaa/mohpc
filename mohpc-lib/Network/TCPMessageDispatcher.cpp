#include <MOHPC/Network/TCPMessageDispatcher.h>
#include <MOHPC/Network/Socket.h>

using namespace MOHPC;
using namespace MOHPC::Network;

MOHPC_OBJECT_DEFINITION(TCPCommunicator)

TCPCommunicator::TCPCommunicator(const ITcpSocketPtr& inSocket)
	: socket(inSocket)
{
	remoteId = makeShared<IPRemoteIdentifier>(socket->getAddress());
}

size_t TCPCommunicator::send(const IRemoteIdentifier& identifier, const uint8_t* data, size_t size)
{
	if (!socket) {
		return 0;
	}

	// send to the address
	return socket->send(data, size);
}

size_t TCPCommunicator::receive(IRemoteIdentifierPtr& remoteAddress, uint8_t* data, size_t size)
{
	if (!socket) {
		return 0;
	}

	remoteAddress = remoteId;
	return socket->receive(data, size);
}

size_t TCPCommunicator::getIncomingSize()
{
	if (!socket) {
		return 0;
	}

	return socket->dataCount();
}

bool TCPCommunicator::waitIncoming(uint64_t timeout)
{
	if (!socket) {
		return false;
	}

	if (timeout)
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

const MOHPC::IRemoteIdentifierPtr& TCPCommunicator::getRemoteIdentifier() const
{
	return remoteId;
}
