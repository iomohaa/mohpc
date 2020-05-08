#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/NetworkManager.h>
#include <MOHPC/Network/Client.h>
#include <MOHPC/Network/ClientGame.h>
#include <MOHPC/Network/Event.h>
#include <MOHPC/Network/Types.h>
#include <MOHPC/Network/MasterList.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Script/str.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Log.h>
#include "UnitTest.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <conio.h>
#include <ctime>
#include <locale>

#pragma comment(lib, "Ws2_32.lib")

class CNetworkUnitTest : public IUnitTest, public TAutoInst<CNetworkUnitTest>
{
private:
	class WinSocket : public MOHPC::Network::IUdpSocket
	{
	private:
		SOCKET conn;
		sockaddr_in srvAddr;

	public:
		WinSocket(SOCKET inConn, const sockaddr_in& inSrvAddr)
			: conn(inConn)
			, srvAddr(inSrvAddr)
		{}

		virtual size_t send(const MOHPC::Network::netadr_t& to, const void* buf, size_t bufsize)
		{
			sockaddr_in srvAddr;
			srvAddr.sin_family = AF_INET;
			srvAddr.sin_port = htons(to.port);
			memcpy(&srvAddr.sin_addr, to.ip, sizeof(srvAddr.sin_addr));
			int srvAddrSz = sizeof(srvAddr);

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

			memcpy(from.ip, &fromAddr.sin_addr, sizeof(fromAddr));
			from.type = MOHPC::Network::netadrtype_t::IP;
			from.port = htons(fromAddr.sin_port);

			return bytesWritten;
		}

		virtual bool wait(size_t timeout) override
		{
			timeval t;
			t.tv_sec = (long)timeout;

			fd_set readfds;
			readfds.fd_count = 1;
			readfds.fd_array[0] = conn;

			int result = select(0, &readfds, NULL, NULL, timeout != -1 ? &t : NULL);
			return result != SOCKET_ERROR && result == 1;
		}
	};

	class Logger : public MOHPC::Log::ILog
	{
	public:
		virtual void log(MOHPC::Log::logType_e type, const char* serviceName, const char* fmt, ...) override
		{
			char mbstr[100];
			time_t t = std::time(nullptr);
			std::strftime(mbstr, sizeof(mbstr), "%H:%M:%S", std::localtime(&t));

			printf("[%s] (%s): ", mbstr, serviceName);
			va_list va;
			va_start(va, fmt);
			vprintf(fmt, va);

			va_end(va);

			printf("\n");
		}
	};

public:
	virtual unsigned int priority()
	{
		return 2;
	}

	virtual const char* name() override
	{
		return "Networking";
	}

	virtual void run(MOHPC::AssetManager& AM) override
	{
		MOHPC::Info info;
		info.SetValueForKey("testKey1", "value");
		info.SetValueForKey("testKey2", "value2");
		info.SetValueForKey("somekey", "somevalue");
		info.SetValueForKey("keyToBe", "deleted");
		info.SetValueForKey("foo", "bar");
		info.SetValueForKey("fu", "boor");
		info.RemoveKey("keyToBe");
		info.SetValueForKey("afterkey", "deletion");

		MOHPC::str someKeyVal = info.ValueForKey("somekey");
		assert(someKeyVal == "somevalue");

		someKeyVal = info.ValueForKey("keyToBe");
		assert(someKeyVal.isEmpty());

		MOHPC::NetworkManager* manager = AM.GetManager<MOHPC::NetworkManager>();

		// Set new log
		MOHPC::Log::ILogPtr logPtr = std::make_shared<Logger>();
		MOHPC::Log::ILog::set(logPtr);

		MOHPC::Network::netadr_t adr;
		adr.ip[0] = 127; adr.ip[1] = 0; adr.ip[2] = 0; adr.ip[3] = 1;
		//adr.ip[0] = 99; adr.ip[1] = 231; adr.ip[2] = 117; adr.ip[3] = 205;
		adr.port = 12203;

		MOHPC::Network::EngineServerPtr clientBase = MOHPC::makeShared<MOHPC::Network::EngineServer>(adr);
		MOHPC::Network::ClientInstance netInst(nullptr);

		/*
		clientBase->getInfo([](const MOHPC::ReadOnlyInfo* info)
			{
				for (MOHPC::InfoIterator it = info->createConstIterator(); it; ++it)
				{
					std::cout << "key: " << it.key() << std::endl;
					std::cout << "value: " << it.value() << std::endl;
				}
			}, []()
			{
				std::cout << "timed out" << std::endl;
			});
		*/

		/*
		clientBase->getStatus(adr, [](const MOHPC::ReadOnlyInfo* info)
			{
				for (MOHPC::InfoIterator it = info->createConstIterator(); it; ++it)
				{
					std::cout << "key: " << it.key() << std::endl;
					std::cout << "value: " << it.value() << std::endl;
				}
			});
		*/

		/*
		MOHPC::Network::ServerList master(MOHPC::Network::gameListType_e::mohaab);
		master.fetch(
			[](const MOHPC::Network::IServerPtr& ptr)
			{
				ptr->query([ptr](const MOHPC::ReadOnlyInfo& response)
					{
						const MOHPC::Network::netadr_t& address = ptr->getAddress();
						const MOHPC::str version = response.ValueForKey("gamever");
						printf("Ping: %d.%d.%d.%d:%d -> version %s\n", address.ip[0], address.ip[1], address.ip[2], address.ip[3], address.port, version.c_str());
					},
					[ptr]()
					{
						const MOHPC::Network::netadr_t& address = ptr->getAddress();
						printf("Timed out: %d.%d.%d.%d:%d\n", address.ip[0], address.ip[1], address.ip[2], address.ip[3], address.port);
					});
			},
			[]()
			{
				printf("done listing\n");
			});
		*/

		MOHPC::Network::ClientGameConnectionPtr connection;

		MOHPC::Network::ClientInfo clientInfo;
		clientInfo.setName("test");
		clientBase->connect(std::move(clientInfo), [&connection, &clientBase](const MOHPC::Network::ClientGameConnectionPtr& cg, const char* errorMessage)
			{
				if (errorMessage)
				{
					printf("server returned error: \"%s\"\n", errorMessage);
					return;
				}
				connection = cg;
			});
		char cmd[512];
		size_t count = 0;

		for (;;)
		{
			if (kbhit())
			{
				const char c = _getch();
				if (c == '\n' || c == '\r')
				{
					count = 0;
					if (!strcmp(cmd, "disconnect")) {
						break;
					}

					if (connection) {
						connection->addReliableCommand(cmd);
					}
				}
				else if (c == '\b')
				{
					if(count > 0) count--;
					cmd[count] = 0;
				}
				else
				{
					cmd[count++] = c;
					cmd[count] = 0;
				}

				printf("%c", c);
			}

			manager->processTicks();
			Sleep(50);
		}
	}
};
