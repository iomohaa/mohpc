#pragma once

#include "../Global.h"
#include "../Utilities/MessageDispatcher.h"
#include "../Utilities/SharedPtr.h"
#include "../Object.h"
#include "IPRemoteIdentifier.h"
#include "Socket.h"

namespace MOHPC
{
namespace Network
{
	class TCPCommunicator : public ICommunicator
	{
		MOHPC_OBJECT_DECLARATION(TCPCommunicator);

	public:
		MOHPC_EXPORTS TCPCommunicator(const ITcpSocketPtr& inSocket = nullptr);

		size_t send(const IRemoteIdentifier& identifier, const uint8_t* data, size_t size) override;
		size_t receive(IRemoteIdentifierPtr& remoteAddress, uint8_t* data, size_t size) override;
		size_t getIncomingSize() override;
		bool waitIncoming(uint64_t timeout) override;
		MOHPC_EXPORTS const IRemoteIdentifierPtr& getRemoteIdentifier() const;

	private:
		ITcpSocketPtr socket;
		IRemoteIdentifierPtr remoteId;
	};
	using TCPCommunicatorPtr = SharedPtr<TCPCommunicator>;
}
}