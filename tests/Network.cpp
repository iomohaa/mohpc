#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/NetworkManager.h>
//#include <MOHPC/Network/Client.h>
#include <MOHPC/Network/ClientGame.h>
#include <MOHPC/Network/CGModule.h>
#include <MOHPC/Network/Event.h>
#include <MOHPC/Network/Types.h>
#include <MOHPC/Network/MasterList.h>
#include <MOHPC/Network/RemoteConsole.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Script/str.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Utilities/TokenParser.h>
#include <MOHPC/Formats/BSP.h>
#include <MOHPC/Collision/Collision.h>
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

#define MOHPC_LOG_NAMESPACE "testnet"

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
			using namespace MOHPC::Log;

			char mbstr[100];
			time_t t = std::time(nullptr);
			std::strftime(mbstr, sizeof(mbstr), "%H:%M:%S", std::localtime(&t));

			const char* typeStr;
			switch (type)
			{
			case logType_e::VeryVerbose:
				typeStr = "dbg";
				break;
			case logType_e::Verbose:
				typeStr = "verb";
				break;
			case logType_e::Log:
				typeStr = "info";
				break;
			case logType_e::Warning:
				typeStr = "warn";
				break;
			case logType_e::Error:
				typeStr = "err";
				break;
			case logType_e::Disconnect:
				typeStr = "fatal";
				break;
			default:
				typeStr = "";
				break;
			}

			printf("[%s] (%s) @_%s => ", mbstr, serviceName, typeStr);

			va_list va;
			va_start(va, fmt);
			vprintf(fmt, va);
			va_end(va);

			printf("\n");
			// Immediately print
			fflush(stdout);
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

	virtual void run(const MOHPC::AssetManagerPtr& AM) override
	{
		using namespace MOHPC;
		using namespace MOHPC::Network;

		Info info;
		info.SetValueForKey("testKey1", "value");
		info.SetValueForKey("testKey2", "value2");
		info.SetValueForKey("somekey", "somevalue");
		info.SetValueForKey("keyToBe", "deleted");
		info.SetValueForKey("foo", "bar");
		info.SetValueForKey("fu", "boor");
		info.RemoveKey("keyToBe");
		info.SetValueForKey("afterkey", "deletion");

		str someKeyVal = info.ValueForKey("somekey");
		assert(someKeyVal == "somevalue");

		someKeyVal = info.ValueForKey("keyToBe");
		assert(someKeyVal.isEmpty());

		NetworkManagerPtr manager = AM->GetManager<NetworkManager>();

		// Set new log
		using namespace Log;
		ILogPtr logPtr = std::make_shared<Logger>();
		ILog::set(logPtr);

		Network::netadr_t adr;
		adr.ip[0] = 127; adr.ip[1] = 0; adr.ip[2] = 0; adr.ip[3] = 1;
		//adr.ip[0] = 0; adr.ip[1] = 0; adr.ip[2] = 0; adr.ip[3] = 0;

		adr.port = 12203;

		// Send remote command
		RemoteConsolePtr RCon = RemoteConsole::create(manager, adr, "12345");
		RCon->getHandlerList().set<RConHandlers::Print>([](const char* text)
		{
			MOHPC_LOG(Log, "Remote console \"%s\"", text);
		});
		RCon->send("echo test");

		Network::EngineServerPtr clientBase = Network::EngineServer::create(manager, adr);

#if 0
		// Query server list
		Network::ServerList master(manager, Network::gameListType_e::mohaa);
		master.fetch(
			[](const Network::IServerPtr& ptr)
			{
				ptr->query([ptr](const ReadOnlyInfo& response)
					{
						const Network::netadr_t& address = ptr->getAddress();
						const str version = response.ValueForKey("gamever");
						printf("Ping: %d.%d.%d.%d:%d -> version %s\n", address.ip[0], address.ip[1], address.ip[2], address.ip[3], address.port, version.c_str());
					},
					[ptr]()
					{
						const Network::netadr_t& address = ptr->getAddress();
						printf("Timed out: %d.%d.%d.%d:%d\n", address.ip[0], address.ip[1], address.ip[2], address.ip[3], address.port);
					});
			},
			[]()
			{
				printf("done listing\n");
			});
#endif

		Network::ClientGameConnectionPtr connection;
		bool wantsDisconnect = false;

		float forwardValue = 0.f;
		float rightValue = 0.f;
		float angle = 0.f;

		str mapfilename;

		BSPPtr Asset = AM->LoadAsset<BSP>("/maps/dm/mohdm6.bsp");
		CollisionWorldPtr cm = CollisionWorld::create();
		Asset->FillCollisionWorld(*cm);

		Network::ClientInfoPtr clientInfo = Network::ClientInfo::create();
		clientInfo->setName("mohpc");
		clientInfo->setRate(25000);
		clientBase->connect(clientInfo, [&](const Network::ClientGameConnectionPtr& cg, const char* errorMessage)
			{
				connection = cg;
				CGameModuleBase& cgame = connection->getCGModule();

				if (errorMessage)
				{
					printf("server returned error: \"%s\"\n", errorMessage);
					return;
				}

				connection->setCallback<ClientHandlers::Error>([](const Network::NetworkException& exception)
					{

					});

				connection->setCallback<ClientHandlers::GameStateParsed>([&cgame, &mapfilename](const Network::gameState_t& gameState, bool differentMap)
					{
						const cgsInfo& cgs = cgame.getServerInfo();
						const str& loadedMap = cgs.getMapFilenameStr();
						if (differentMap)
						{
							MOHPC_LOG(Log, "New map: \"%s\"", loadedMap.c_str());
							mapfilename = loadedMap;
						}
					});

				fnHandle_t cb = cgame.setCallback<CGameHandlers::EntityAdded>([&connection](const EntityInfo& entity)
					{
						const char* modelName = connection->getGameState().getConfigString(CS_MODELS + entity.currentState.modelindex);
						//MOHPC_LOG(VeryVerbose, "new entity %d, model \"%s\"", entity.currentState.number, modelName);
					});

				cgame.setCallback<CGameHandlers::EntityRemoved>([&connection](const EntityInfo& entity)
					{
						const char* modelName = connection->getGameState().getConfigString(CS_MODELS + entity.currentState.modelindex);
						//MOHPC_LOG(VeryVerbose, "entity %d deleted (was model \"%s\")", entity.currentState.number, modelName);
					});

				cgame.setCallback<CGameHandlers::MakeBulletTracer>([&logPtr](const Vector& barrel, const Vector& start, const Vector& end, uint32_t numBullets, uint32_t iLarge, uint32_t numTracersVisible, float bulletSize)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "bullet %zu", num++);
					});

				cgame.setCallback<CGameHandlers::Impact>([&logPtr](const Vector& origin, const Vector& normal, uint32_t large)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "impact %zu", num++);
					});

				cgame.setCallback<CGameHandlers::MakeExplosionEffect>([&logPtr](const Vector& origin, effects_e type)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "explosionfx %zu: type \"%s\"", num++, getEffectName(type));
					});

				cgame.setCallback<CGameHandlers::MakeEffect>([&logPtr](const Vector& origin, const Vector& normal, effects_e type)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "effect %zu: type \"%s\"", num++, getEffectName(type));
					});

				cgame.setCallback<CGameHandlers::SpawnDebris>([&logPtr](CGameHandlers::debrisType_e debrisType, const Vector& origin, uint32_t numDebris)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "debris %zu: type %d", num++, debrisType);
					});

				cgame.setCallback<CGameHandlers::HitNotify>([&logPtr]()
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "hit %zu", num++);
					});

				cgame.setCallback<CGameHandlers::KillNotify>([&logPtr]()
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "kill %zu", num++);
					});

				cgame.setCallback<CGameHandlers::VoiceMessage>([&logPtr](const Vector& origin, bool local, uint8_t clientNum, const char* soundName)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "voice %d: sound \"%s\"", num++, soundName);
					});

				cgame.setCallback<CGameHandlers::Print>([&logPtr](hudMessage_e hudMessage, const char* text)
					{
						//MOHPC_LOG(VeryVerbose, "server print: \"%s\"", text);
					});

				cgame.setCallback<CGameHandlers::HudDraw_Shader>([&logPtr](uint8_t index, const char* shaderName)
					{
						MOHPC_LOG(Verbose, "huddraw_shader : %d \"%s\"", index, shaderName);
					});

				connection->setCallback<ClientHandlers::Disconnect>([&wantsDisconnect](const char* reason)
					{
						//MOHPC_LOG(VeryVerbose, "Requested disconnect. Reason -> \"%s\"", reason ? reason : "");
						wantsDisconnect = true;
					});

				connection->setCallback<ClientHandlers::Timeout>([]()
					{
						//MOHPC_LOG(VeryVerbose, "Server connection timed out");
					});

				connection->setCallback<ClientHandlers::UserInput>([&forwardValue, &rightValue, &angle, &cgame, &cm](usercmd_t& ucmd, usereyes_t& eyeinfo)
				{
					eyeinfo.setAngles(30.f, angle);
					ucmd.buttons.fields.button.run = true;
					ucmd.moveForward((int8_t)(forwardValue * 127.f));
					ucmd.moveRight((int8_t)(rightValue * 127.f));
					ucmd.setAngles(30.f, angle, 0.f);

					//Vector start(684.04f, -332.63f, -145.f);
					//Vector end(241.34f, -328.43f, -145.f);
					trace_t tr;

					{
						Vector start(499.125000f + 16.f, -427.312500f, -151.875000f);
						Vector end(499.125824f, -426.720612f, -151.875000f);

						cgame.trace(*cm, tr, start, Vector(-15, -15, 0), Vector(15, 15, 96), end, 0, ContentFlags::MASK_PLAYERSOLID, true, true);
					}

					{
						Vector start(499.133942f, -427.044525f, -151.875000f);
						Vector end(499.125824f, -426.720612f, -151.875000f);

						cgame.trace(*cm, tr, start, Vector(-15, -15, 0), Vector(15, 15, 96), end, 0, ContentFlags::MASK_PLAYERSOLID, true, true);
					}
				});


			});

		char buf[512];
		size_t count = 0;

		while(!wantsDisconnect)
		{
			if (kbhit())
			{
				const char c = _getch();
				if (c == '\n' || c == '\r')
				{
					count = 0;

					TokenParser parser;
					parser.Parse(buf, strlen(buf));

					const char* cmd = parser.GetToken(false);

					if (!strcmp(cmd, "forward")) {
						forwardValue = 1.f;
					}
					else if(!strcmp(cmd, "backward")) {
						forwardValue = -1.f;
					}
					else if (!strcmp(cmd, "right")) {
						rightValue = 1.f;
					}
					else if (!strcmp(cmd, "left")) {
						rightValue = -1.f;
					}
					else if (!strcmp(cmd, "stop"))
					{
						forwardValue = 0.f;
						rightValue = 0.f;
					}
					else if (!strcmp(cmd, "turnleft")) {
						angle += 20.f;
					}
					else if (!strcmp(cmd, "turnright")) {
						angle -= 20.f;
					}
					else if (!strcmp(cmd, "testuinfo"))
					{
						if (connection)
						{
							const ClientInfoPtr& userInfo = connection->getUserInfo();
							userInfo->setRate(30000);
							userInfo->setName("modified");
							userInfo->setSnaps(1);
							connection->updateUserInfo();
						}
					}
					else if (!strcmp(cmd, "set"))
					{
						const char* key = parser.GetToken(false);
						if (!strcmp(key, "rate"))
						{
							const uint32_t rate = parser.GetInteger(false);
							if(connection)
							{
								const ClientInfoPtr& userInfo = connection->getUserInfo();
								userInfo->setRate(rate);
								connection->updateUserInfo();
							}
						}
						else if (!strcmp(key, "snaps"))
						{
							const uint32_t snaps = parser.GetInteger(false);
							if (connection)
							{
								const ClientInfoPtr& userInfo = connection->getUserInfo();
								userInfo->setSnaps(snaps);
								connection->updateUserInfo();
							}
						}
						else if (!strcmp(key, "maxpackets"))
						{
							const uint32_t maxPackets = parser.GetInteger(false);
							if (connection) {
								connection->setMaxPackets(maxPackets);
							}
						}
					}
					else if (!strcmp(cmd, "disconnect") || !strcmp(cmd, "quit") || !strcmp(cmd, "exit"))
					{
						if (connection) {
							connection->disconnect();
						}
					}
					else
					{
						if (connection) {
							connection->addReliableCommand(buf);
						}
					}
				}
				else if (c == '\b')
				{
					if(count > 0) count--;
					buf[count] = 0;
				}
				else
				{
					buf[count++] = c;
					buf[count] = 0;
				}

				printf("%c", c);
			}

			manager->processTicks();
			Sleep(10);
		}
	}
};
