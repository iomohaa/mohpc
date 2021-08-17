#pragma once

#include "../NetGlobal.h"
#include "../NetObject.h"
#include "../Types/NetTime.h"
#include "../../Utility/Function.h"
#include "../../Utility/SharedPtr.h"

namespace MOHPC
{
namespace Network
{
	class ServerConnection;
	class ClientTime;
	struct rawSnapshot_t;

	struct pingPacket_t
	{
		tickTime_t lastPacketTime;
		tickTime_t lastSnapTime;

		pingPacket_t();
	};

	class PingCalculator
	{
		MOHPC_NET_OBJECT_DECLARATION(PingCalculator);

	public:
		MOHPC_NET_EXPORTS PingCalculator(const SharedPtr<ServerConnection>& connection);
		MOHPC_NET_EXPORTS ~PingCalculator();

		MOHPC_NET_EXPORTS deltaTime_t getLatency() const;

	private:
		const pingPacket_t& getPacket(size_t index) const;
		pingPacket_t& getPacket(size_t index);
		const pingPacket_t& getLastPacket() const;
		pingPacket_t& createPacket();
		size_t getNumPackets() const;
		deltaTime_t getMinDeltaTime() const;
		void postWritePacket(const ClientTime& time, uint32_t sequenceNum);
		void snapReceived(const rawSnapshot_t& snap);

	private:
		WeakPtr<ServerConnection> connection;
		fnHandle_t packetHandle;
		fnHandle_t snapReceivedHandle;
		pingPacket_t packets[100];
		deltaTime_t latency;
		uint8_t currentPacket;
	};
	using PingCalculatorPtr = SharedPtr<PingCalculator>;
}
}
