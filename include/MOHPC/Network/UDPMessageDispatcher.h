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
	class UDPCommunicator : public ICommunicator
	{
		MOHPC_OBJECT_DECLARATION(UDPCommunicator);

	public:
		MOHPC_EXPORTS UDPCommunicator(const IUdpSocketPtr& inSocket = nullptr);

		size_t send(const IRemoteIdentifier& identifier, const uint8_t* data, size_t size) override;
		size_t receive(IRemoteIdentifierPtr& remoteAddress, uint8_t* data, size_t size) override;
		size_t getIncomingSize() override;
		bool waitIncoming(uint64_t timeout) override;
		IUdpSocketPtr getSocket() const;

	protected:
		IUdpSocketPtr socket;
	};
	using UDPCommunicatorPtr = SharedPtr<UDPCommunicator>;

	class UDPBroadcastCommunicator : public UDPCommunicator
	{
		MOHPC_OBJECT_DECLARATION(UDPBroadcastCommunicator);

	public:
		MOHPC_EXPORTS UDPBroadcastCommunicator(const IUdpSocketPtr& inSocket = nullptr, uint16_t startPort = 12203, uint16_t endPort = 12218);

		size_t send(const IRemoteIdentifier& identifier, const uint8_t* data, size_t size) override;

	private:
		uint16_t startPort;
		uint16_t endPort;
	};
	using UDPBroadcastCommunicatorPtr = SharedPtr<UDPBroadcastCommunicator>;
}
}