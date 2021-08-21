#pragma once

#include "Socket.h"
#include "../../Utility/SharedPtr.h"

#include <deque>
#include <chrono>
#include <random>
#include <thread>
#include <shared_mutex>

namespace MOHPC
{
namespace Network
{
	class PacketContainer
	{
	public:
		PacketContainer();
		PacketContainer(const NetAddrPtr& addressPtr, void* bufferPtr, size_t sizeVal, std::chrono::nanoseconds timeVal);
		PacketContainer(PacketContainer&& other);
		PacketContainer& operator=(PacketContainer&& other);
		~PacketContainer();

		const NetAddrPtr& getAddress() const;
		size_t getBufferSize() const;
		void* getBuffer() const;
		std::chrono::nanoseconds getTime() const;

	private:
		NetAddrPtr address;
		void* buffer;
		size_t size;
		std::chrono::nanoseconds time;
	};

	/**
	 * UDP Socket proxy for simulating network latency.
	 */
	class UdpSocketSimLatency : public IUdpSocket
	{
		MOHPC_NET_OBJECT_DECLARATION(UdpSocketSimLatency);

	public:
		MOHPC_NET_EXPORTS UdpSocketSimLatency(const IUdpSocketPtr& originalSocketPtr);
		MOHPC_NET_EXPORTS ~UdpSocketSimLatency();

		/**
		 * Set the minimum latency of the udp socket.
		 *
		 * @param latency The latency in milliseconds.
		 */
		MOHPC_NET_EXPORTS void setLatency(uint32_t latency);

		/**
		 * Set the jittering. Random amount is added to the latency.
		 *
		 * @param variance Jittering, in milliseconds.
		 */
		MOHPC_NET_EXPORTS void setLatencyVariance(uint32_t variance);

		/**
		 * Send packets out-of-order.
		 *
		 * @param enabled whether or not to send packets out-of-order.
		 */
		MOHPC_NET_EXPORTS void setOutOfOrder(bool enabled);

	public:
		size_t send(const NetAddrPtr& to, const void* buf, size_t bufsize) override;
		size_t receive(void* buf, size_t maxsize, NetAddrPtr& from) override;
		bool wait(uint64_t timeout) override;
		size_t dataCount() override;
		void* getRaw() override;

	private:
		std::chrono::nanoseconds calculatePacketTime() const;
		std::chrono::nanoseconds calculatePacketTime(std::chrono::time_point<std::chrono::steady_clock> currentTime) const;
		PacketContainer& waitIncoming(std::unique_lock<std::mutex>& lk);
		PacketContainer* waitIncomingSafe(std::unique_lock<std::mutex>& lk);
		PacketContainer* waitIncoming(std::unique_lock<std::mutex>& lk, std::chrono::nanoseconds timeout);
		void waitFor(std::chrono::nanoseconds packetTime) const;
		void waitFor(std::chrono::nanoseconds packetTime, std::chrono::nanoseconds maxTime) const;
		std::chrono::nanoseconds getWaitTime(std::chrono::nanoseconds packetTime) const;
		bool shouldSendOutOfOrder() const;

	private:
		size_t getSocketDataCount();
		void incomingThread();
		void outgoingThread();

	private:
		IUdpSocketPtr originalSocket;
		bool enableOOD;
		bool isThreadActive;
		std::condition_variable incomingEvent;
		std::condition_variable outgoingEvent;
		std::mutex incomingMutex;
		std::mutex outboundMutex;
		std::deque<PacketContainer> incomings;
		std::deque<PacketContainer> outbounds;
		std::chrono::time_point<std::chrono::steady_clock> startTime;
		uint32_t minLatency;
		uint32_t jittering;
		std::thread socketIncomingThread;
		std::thread socketOutgoingThread;
	};
	using UdpSocketSimLatencyPtr = SharedPtr<UdpSocketSimLatency>;

	/**
	 * UDP Socket proxy used to simulate packet loss and duplication.
	 */
	class UdpSocketSimLoss : public IUdpSocket
	{
		MOHPC_NET_OBJECT_DECLARATION(UdpSocketSimLoss);

	public:
		MOHPC_NET_EXPORTS UdpSocketSimLoss(const IUdpSocketPtr& originalSocketPtr);
		MOHPC_NET_EXPORTS ~UdpSocketSimLoss();

		/**
		 * Percentage of chance of packet loss for incoming packets.
		 *
		 * @param alpha Packet loss alpha [0...1]
		 */
		MOHPC_NET_EXPORTS void setInboundPacketLossAlpha(float alpha);

		/**
		 * Percentage of chance of packet loss for outgoing packets.
		 *
		 * @param alpha Packet loss alpha [0...1]
		 */
		MOHPC_NET_EXPORTS void setOutboundPacketLossAlpha(float alpha);

		/**
		 * Percentage of chance of sending a duplicate packet.
		 *
		 * @param alpha Packet dup alpha [0...1]
		 */
		MOHPC_NET_EXPORTS void setDuplicateAlpha(float alpha);

	public:
		size_t send(const NetAddrPtr& to, const void* buf, size_t bufsize) override;
		size_t receive(void* buf, size_t maxsize, NetAddrPtr& from) override;
		bool wait(uint64_t timeout) override;
		size_t dataCount() override;
		void* getRaw() override;

	private:
		void dropIncoming();
		bool shouldDropIncoming();
		bool shouldDropOutbound();
		bool shouldDuplicate();

	private:
		std::random_device rd;
		IUdpSocketPtr originalSocket;
		float inboundLossAlpha;
		float outboundLossAlpha;
		float dupAlpha;
		bool mustProcessIncoming;
	};
	using UdpSocketSimLossPtr = SharedPtr<UdpSocketSimLoss>;
}
}