#include <MOHPC/Network/Socket.h>
#include <MOHPC/Network/Types.h>
#include "network.h"
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace MOHPC;
using namespace Network;

#pragma comment(lib, "Ws2_32.lib")

class WindowsUDPSocket : public IUdpSocket
{
private:
	SOCKET conn;

public:
	WindowsUDPSocket(addressType_e type, const bindv4_t* bindAddress)
	{
		sockaddr_in local;

		switch (type)
		{
		case addressType_e::IPv4:
			conn = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			local.sin_family = AF_INET;
			break;
		case addressType_e::IPv6:
			conn = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
			local.sin_family = AF_INET6;
			break;
		default:
			conn = NULL;
			break;
		}

		if (conn == INVALID_SOCKET) {
			throw("socket creation failed");
		}

		//int val = 1;
		//setsockopt(conn, SOL_SOCKET, SO_REUSEADDR, (const char*)val, sizeof(val));

		if(!bindAddress)
		{
			// Bind a default port
			local.sin_port = 0;
			inet_pton(AF_INET, "0.0.0.0", &local.sin_addr);
		}
		else
		{
			local.sin_port = htons(bindAddress->port);
			memcpy(&local.sin_addr.s_addr, bindAddress->ip, sizeof(uint8_t[4]));
		}

		bind(conn, (sockaddr*)&local, sizeof(local));
	}

	~WindowsUDPSocket()
	{
		closesocket(conn);
	}

	virtual size_t send(const MOHPC::Network::netadr_t& to, const void* buf, size_t bufsize) override
	{
		uint8_t zeroIP[sizeof(to.ip)]{ 0xFF, 0xFF, 0xFF, 0xFF };

		if (memcmp(to.ip, zeroIP, sizeof(to.ip)))
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
		srvAddr.sin_family = AF_INET;
		srvAddr.sin_port = htons(to.port);
		memcpy(&srvAddr.sin_addr, to.ip, sizeof(srvAddr.sin_addr));

		return sendto(
			conn,
			(const char*)buf,
			(int)bufsize,
			0,
			(sockaddr*)&srvAddr,
			sizeof(srvAddr)
		);
	}

	virtual size_t receive(void* buf, size_t maxsize, MOHPC::Network::netadr_t& from) override
	{
		sockaddr_in fromAddr;
		int addrSz = sizeof(fromAddr);

		const size_t bytesWritten = recvfrom(
			conn,
			(char*)buf,
			(int)maxsize,
			0,
			(sockaddr*)&fromAddr,
			&addrSz
		);

		memcpy(from.ip, &fromAddr.sin_addr, sizeof(from.ip));
		from.type = MOHPC::Network::netadrtype_t::IP;
		from.port = htons(fromAddr.sin_port);

		return bytesWritten;
	}

	virtual bool wait(size_t timeout) override
	{
		timeval t{0};
		t.tv_usec = (long)timeout * 1000;

		fd_set readfds;
		readfds.fd_count = 1;
		readfds.fd_array[0] = conn;

		int result = select(0, &readfds, NULL, NULL, timeout != -1 ? &t : NULL);
		return result != SOCKET_ERROR && result == 1;
	}

	virtual bool dataAvailable() override
	{
		u_long count;
		ioctlsocket(conn, FIONREAD, &count);
		return count > 0;
	}
};

class WindowsTCPSocket : public ITcpSocket
{
private:
	SOCKET conn;

public:
	WindowsTCPSocket(addressType_e type, const netadr_t& addr)
	{
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

		if (conn == INVALID_SOCKET) {
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
		memcpy(&srvAddr.sin_addr, addr.ip, sizeof(addr.ip));
		int srvAddrSz = sizeof(srvAddr);

		// Set non-blocking
		u_long mode = 1;
		ioctlsocket(conn, FIONBIO, &mode);

		// Connect to remote
		connect(conn, (sockaddr*)&srvAddr, sizeof(srvAddr));
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
		readfds.fd_count = 1;
		readfds.fd_array[0] = conn;

		int result = select(0, &readfds, NULL, NULL, timeout != -1 ? &t : NULL);
		if (result != 1) {
			return false;
		}

		// now check if there is any data
		char buf;
		size_t numBytes = recv(conn, &buf, 1, MSG_PEEK);
		if (numBytes == -1 || numBytes == 0) {
			return false;
		}

		return true;
	}

	virtual bool dataAvailable() override
	{
		u_long count;
		ioctlsocket(conn, FIONREAD, &count);
		return count > 0;
	}
};

class WindowsSocketFactory : public ISocketFactory
{
private:
	bool isValidWSA;

public:
	WindowsSocketFactory()
	{
		WSADATA wsaData;

		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult == NO_ERROR) {
			isValidWSA = true;
		}
	}

	~WindowsSocketFactory()
	{
		WSACleanup();
	}

	virtual IUdpSocketPtr createUdp(addressType_e addressType, const bindv4_t* bindAddress) override
	{
		return makeShared<WindowsUDPSocket>(addressType, bindAddress);
	}

	virtual ITcpSocketPtr createTcp(addressType_e socketType, const netadr_t& address) override
	{
		return makeShared<WindowsTCPSocket>(socketType, address);
	}

	virtual ITcpServerSocketPtr createTcpListener(addressType_e socketType) override
	{
		// FIXME
		return nullptr;
	}

	virtual netadr_t getHost(const char* domain) override
	{
		struct addrinfo hints { 0 }, *result;
		hints.ai_family = AF_UNSPEC;

		int retval = getaddrinfo(domain, NULL, &hints, &result);
		if (retval != NO_ERROR) {
			return netadr_t();
		}

		sockaddr_in* sockad = (sockaddr_in*)result->ai_addr;

		netadr_t adr;
		adr.type = netadrtype_t::IP;
		memcpy(&adr.ip, &sockad->sin_addr, sizeof(adr.ip));

		freeaddrinfo(result);

		return adr;
	}
};

ISocketFactoryPtr Platform::Network::createSockFactory()
{
	return makeShared<WindowsSocketFactory>();
}

