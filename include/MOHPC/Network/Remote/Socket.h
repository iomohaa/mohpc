#pragma once

#include "../NetGlobal.h"
#include "../../Utility/SharedPtr.h"
#include "../../Utility/WeakPtr.h"
#include "Address.h"

#include <cstdint>

namespace MOHPC
{
	namespace Network
	{
		struct NetAddr;

		static constexpr unsigned long MAX_UDP_DATA_SIZE = 65507u;

		enum class addressType_e : unsigned char {
			IPv4,
			IPv6
		};

		class ISocket
		{
		public:
			virtual ~ISocket() = default;

			/**
			 * Wait for data to be available for reading in the specified amount of milliseconds.
			 *
			 * @param	timeout		Time to wait. (0 = no wait)
			 * @return	True if there is any pending data.
			 */
			virtual bool wait(size_t timeout) = 0;

			/**
			 * Return whether or not there is any data available in the socket.
			 *
			 * @return	True if there is any pending data.
			 */
			virtual size_t dataCount() = 0;

			/** Return the underlying socket. */
			virtual void* getRaw() = 0;
		};

		using ISocketPtr = SharedPtr<ISocket>;
		using ISocketWeakPtr = WeakPtr<ISocket>;

		/**
		 * Abstract class for UDP socket
		 */
		class IUdpSocket : public ISocket
		{
		public:
			/**
			 * Send data to the socket.
			 *
			 * @param	to		Target to send data to
			 * @param	buf		Data to send
			 * @param	bufsize	Size of the data
			 * @return	Size of the data that was successfully sent
			 */
			virtual size_t send(const NetAddr& to, const void* buf, size_t bufsize) = 0;

			/**
			 * Receive data from the socket.
			 *
			 * @param	buf			Buffer to receive data to
			 * @param	maxsize		Size of data to receive
			 * @param	from		Client that sent the data
			 * @return	Size of the data that was successfully received
			 */
			virtual size_t receive(void* buf, size_t maxsize, NetAddrPtr& from) = 0;
		};

		using IUdpSocketPtr = SharedPtr<IUdpSocket>;
		using IUdpSocketWeakPtr = WeakPtr<IUdpSocket>;

		/**
		 * Abstract class for TCP socket
		 */
		class ITcpSocket : public ISocket
		{
		public:
			/**
			 * Send data to the socket.
			 *
			 * @param	buf		Data to send
			 * @param	bufsize	Size of the data
			 * @return	Size of the data that was successfully sent
			 */
			virtual size_t send(const void* buf, size_t bufsize) = 0;

			/**
			 * Receive data from the socket.
			 *
			 * @param	buf			Buffer to receive data to
			 * @param	maxsize		Size of data to receive
			 * @return	Size of the data that was successfully received
			 */
			virtual size_t receive(void* buf, size_t maxsize) = 0;

			/**
			 * Return the address of the tcp socket.
			 */
			virtual NetAddrPtr getAddress() = 0;
		};

		using ITcpSocketPtr = SharedPtr<ITcpSocket>;
		using ITcpSocketWeakPtr = WeakPtr<ITcpSocket>;

		class ITcpServerSocket : public ISocket
		{
		public:
			/** Listen for new connections. */
			virtual void listen() = 0;

			/**
			 * Accept new connection and return its associated socket.
			 *
			 * @param	from	Address of the client connecting
			 * @eturn	Client socket connection
			 */
			virtual ITcpSocketPtr accept(NetAddrPtr& from) = 0;
		};

		using ITcpServerSocketPtr = SharedPtr<ITcpServerSocket>;
		using ITcpServerSocketWeakPtr = WeakPtr<ITcpServerSocket>;

		using ISocketFactoryPtr = SharedPtr<class ISocketFactory>;
		using ISocketFactoryWeakPtr = WeakPtr<class ISocketFactory>;

		/**
		 * Abstract class for creating sockets
		 */
		class ISocketFactory
		{
		public:
			virtual ~ISocketFactory() = default;

			/**
			 * This function must create an UDP socket that is ready to send/receive.
			 *
			 * @param	addressType		Socket domain type
			 * @return	The UDP socket on success
			 */
			virtual IUdpSocketPtr createUdp(const NetAddr4* bindAddress = nullptr) = 0;
			virtual IUdpSocketPtr createUdp6(const NetAddr6* bindAddress = nullptr) = 0;

			/**
			 * Create a TCP socket that is ready to send/receive.
			 *
			 * @param	addressType		Socket domain type
			 * @param	address			IP/hostname/domain to connect to
			 * @return	The UDP socket on success
			 */
			virtual ITcpSocketPtr createTcp(const NetAddr4Ptr& address, const NetAddr4* bindAddress = nullptr) = 0;
			virtual ITcpSocketPtr createTcp6(const NetAddr6Ptr& address, const NetAddr6* bindAddress = nullptr) = 0;

			/**
			 * Create a TCP socket that is ready to accept incoming connections.
			 *
			 * @param	socketType		Socket domain type
			 * @return	The TCP server connection socket
			 */
			virtual ITcpServerSocketPtr createTcpListener() = 0;

			/**
			 * Convert an host domain name to an IP address
			 *
			 * @param	domain	Domain name
			 */
			virtual NetAddr4Ptr getHost(const char* domain) = 0;

			/**
			 * Wait for one or multiple sockets.
			 *
			 * @param sockets Socket to wait.
			 * @param signaledSockets Sockets that was signaled. Must be the same size as sockets.
			 * @param num Number of sockets to wait.
			 * @param return Number of signaled sockets.
			 */
			 virtual size_t wait(const ISocketPtr sockets[], ISocketPtr signaledSockets[], size_t num, size_t timeout) = 0;

			/** Get the factory used to create sockets. */
			MOHPC_NET_EXPORTS static ISocketFactory* get();

			/** Set the factory used to create sockets. */
			MOHPC_NET_EXPORTS static void set(const ISocketFactoryWeakPtr& newFactory);
		};
	}
}
