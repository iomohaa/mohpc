#include <MOHPC/Network/Socket.h>
#include <MOHPC/Network/Types.h>
#include "network.h"
#include "generic_sockets.h"
#include <type_traits>

using namespace MOHPC;
using namespace Network;

template<addressType_e type>
static constexpr unsigned long addressSize = (type == addressType_e::IPv6) ? (sizeof(uint8_t) * 16) : (sizeof(uint8_t) * 4);

template<addressType_e type>
static constexpr unsigned int family = (type == addressType_e::IPv6) ? AF_INET6 : AF_INET;

template<addressType_e type>
class WindowsUDPSocket : public IUdpSocket
{
	static constexpr unsigned long addressSize = ::addressSize<type>;
	static constexpr unsigned int family = ::family<type>;

	using sockaddr_template = typename std::conditional<type == addressType_e::IPv6, sockaddr_in6, sockaddr_in>::type;

public:
	WindowsUDPSocket(const NetAddr* bindAddress)
	{
		if(bindAddress && bindAddress->getAddrSize() != addressSize)
		{
			// FIXME: throw?
			return;
		}

		conn = socket(family, SOCK_DGRAM, IPPROTO_UDP);
		if (conn == badSocket) 
		{
			// FIXME: throw?
			return;
		}

		sockaddr_in local;
		local.sin_family = family;

		//int val = 1;
		//setsockopt(conn, SOL_SOCKET, SO_REUSEADDR, (const char*)val, sizeof(val));

		if(!bindAddress)
		{
			// bind a default port
			local.sin_port = 0;
			// set address to 0
			memset(&local.sin_addr, 0, sizeof(local.sin_addr));
		}
		else
		{
			local.sin_port = htons(bindAddress->port);
			memcpy(&local.sin_addr.s_addr, bindAddress->getAddress(), addressSize);
		}

		bind(conn, (sockaddr*)&local, sizeof(local));
	}

	~WindowsUDPSocket()
	{
		closeSocket(conn);
	}

	virtual size_t send(const NetAddr& to, const void* buf, size_t bufsize) override
	{
		const size_t inAddrSize = to.getAddrSize();
		if(inAddrSize != addressSize) {
			return 0;
		}

		const uint8_t* inAddr = to.getAddress();

		// check if the address is a broadcast ip
		if (memcmp(inAddr, broadcastIP, addressSize))
		{
			int val = 0;
			setsockopt(conn, SOL_SOCKET, SO_BROADCAST, (const char*)&val, sizeof(val));
		}
		else
		{
			int val = 1;
			setsockopt(conn, SOL_SOCKET, SO_BROADCAST, (const char*)&val, sizeof(val));
		}

		sockaddr_in srvAddr;
		srvAddr.sin_family = family;
		srvAddr.sin_port = htons(to.port);
		memcpy(&srvAddr.sin_addr, inAddr, sizeof(srvAddr.sin_addr));

		return sendto(
			conn,
			(const char*)buf,
			(int)bufsize,
			0,
			(sockaddr*)&srvAddr,
			sizeof(srvAddr)
		);
	}

	size_t receive(void* buf, size_t maxsize, NetAddrPtr& from)
	{
		sockaddr_template fromAddr;
		socklen_t addrSz = sizeof(fromAddr);
		
		const size_t bytesWritten = recvfrom(
			conn,
			(char*)buf,
			(int)maxsize,
			0,
			(sockaddr*)&fromAddr,
			&addrSz
		);

		// create the corresponding address and return
		from = createAddress(fromAddr);

		return bytesWritten;
	}

	virtual bool wait(size_t timeout) override
	{
		timeval t{0};
		t.tv_usec = (long)timeout * 1000;

		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(conn, &readfds);

		int result = select(1, &readfds, NULL, NULL, timeout != -1 ? &t : NULL);
		return result == 1;
	}

	virtual bool dataAvailable() override
	{
		u_long count = 0;
		ioctlSocket(conn, FIONREAD, &count);
		return count > 0;
	}

	NetAddrPtr createAddress(const sockaddr_template& addr) const;

private:
	socket_t conn;
	static uint8_t broadcastIP[addressSize];
};

template<>
uint8_t WindowsUDPSocket<addressType_e::IPv4>::broadcastIP[] = { 0xFF, 0xFF, 0xFF, 0xFF };

template<>
uint8_t WindowsUDPSocket<addressType_e::IPv6>::broadcastIP[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

template <>
NetAddrPtr WindowsUDPSocket<addressType_e::IPv4>::createAddress(const sockaddr_in& addr) const
{
	const NetAddr4Ptr netAddress = NetAddr4::create();
	memcpy(netAddress->ip, &addr.sin_addr, sizeof(netAddress->ip));
	netAddress->port = htons(addr.sin_port);

	return netAddress;
}

template <>
NetAddrPtr WindowsUDPSocket<addressType_e::IPv6>::createAddress(const sockaddr_in6& addr) const
{
	const NetAddr6Ptr netAddress = NetAddr6::create();
	memcpy(netAddress->ip, &addr.sin6_addr, sizeof(netAddress->ip));
	netAddress->port = htons(addr.sin6_port);

	return netAddress;
}

template<addressType_e type>
class WindowsTCPSocket : public ITcpSocket
{
	static constexpr unsigned long addressSize = ::addressSize<type>;
	static constexpr unsigned int family = ::family<type>;

private:
	socket_t conn;

public:
	WindowsTCPSocket(const NetAddr& addr)
	{
		if (addr.getAddrSize() != addressSize)
		{
			// FIXME: throw?
			return;
		}

		switch (type)
		{
		case addressType_e::IPv4:
			conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			break;
		case addressType_e::IPv6:
			conn = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
			break;
		default:
			conn = NULL;
			break;
		}

		if (conn == badSocket) {
			throw("socket creation failed");
		}

		//int val = 1;
		//setsockopt(conn, SOL_SOCKET, SO_REUSEADDR, (const char*)val, sizeof(val));

		sockaddr_in local;
		local.sin_family = AF_INET;
		//local.sin_port = htons(12205);
		local.sin_port = 0;
		inet_pton(AF_INET, "0.0.0.0", &local.sin_addr);

		// Bind to local address
		bind(conn, (sockaddr*)&local, sizeof(local));

		sockaddr_in srvAddr;
		srvAddr.sin_family = AF_INET;
		srvAddr.sin_port = htons(addr.port);
		memcpy(&srvAddr.sin_addr, addr.getAddress(), addressSize);
		int srvAddrSz = sizeof(srvAddr);

		// Set non-blocking
		u_long mode = 1;
		ioctlSocket(conn, FIONBIO, &mode);

		// Connect to remote
		connect(conn, (sockaddr*)&srvAddr, sizeof(srvAddr));
	}

	~WindowsTCPSocket()
	{
		closeSocket(conn);
	}

	virtual size_t send(const void* buf, size_t bufsize) override
	{
		return ::send(conn, (const char*)buf, (int)bufsize, 0);
	}

	virtual size_t receive(void* buf, size_t maxsize) override
	{
		return ::recv(conn, (char*)buf, (int)maxsize, 0);
	}

	virtual bool wait(size_t timeout) override
	{
		timeval t;
		t.tv_usec = (long)timeout * 1000;

		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(conn, &readfds);

		int result = select(1, &readfds, NULL, NULL, timeout != -1 ? &t : NULL);
		if (result != 1) {
			return false;
		}

		// now check if there is any data
		char buf;
		size_t numBytes = recv(conn, &buf, 1, MSG_PEEK);
		if (!numBytes || numBytes == -1) {
			return false;
		}

		return true;
	}

	virtual bool dataAvailable() override
	{
		u_long count = 0;
		ioctlSocket(conn, FIONREAD, &count);
		return count > 0;
	}
};

class WindowsSocketFactory : public ISocketFactory
{
public:
	WindowsSocketFactory()
	{
		initSocket();
	}

	~WindowsSocketFactory()
	{
		cleanupSocket();
	}

	virtual IUdpSocketPtr createUdp(const NetAddr4* bindAddress) override
	{
		return makeShared<WindowsUDPSocket<addressType_e::IPv4>>(bindAddress);
	}

	virtual IUdpSocketPtr createUdp6(const NetAddr6* bindAddress) override
	{
		return makeShared<WindowsUDPSocket<addressType_e::IPv6>>(bindAddress);
	}

	virtual ITcpSocketPtr createTcp(const NetAddr4& address) override
	{
		return makeShared<WindowsTCPSocket<addressType_e::IPv4>>(address);
	}

	virtual ITcpSocketPtr createTcp6(const NetAddr6& address) override
	{
		return makeShared<WindowsTCPSocket<addressType_e::IPv6>>(address);
	}

	virtual ITcpServerSocketPtr createTcpListener() override
	{
		// FIXME
		return nullptr;
	}

	virtual NetAddr4 getHost(const char* domain) override
	{
		struct addrinfo hints { 0 }, *result;
		hints.ai_family = AF_UNSPEC;

		int retval = getaddrinfo(domain, NULL, &hints, &result);
		if (retval != 0) {
			return NetAddr4();
		}

		sockaddr_in* sockad = (sockaddr_in*)result->ai_addr;

		NetAddr4 adr;
		memcpy(&adr.ip, &sockad->sin_addr, sizeof(adr.ip));

		freeaddrinfo(result);

		return adr;
	}
};

ISocketFactoryPtr Platform::Network::createSockFactory()
{
	return makeShared<WindowsSocketFactory>();
}

