#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/NetworkManager.h>
//#include <MOHPC/Network/Client.h>
#include <MOHPC/Network/Client/ClientGame.h>
#include <MOHPC/Network/Client/CGModule.h>
#include <MOHPC/Network/Client/MasterList.h>
#include <MOHPC/Network/Client/RemoteConsole.h>
#include <MOHPC/Network/Event.h>
#include <MOHPC/Network/Types.h>
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

#define MOHPC_LOG_NAMESPACE "testnet"

class CNetworkUnitTest : public IUnitTest, public TAutoInst<CNetworkUnitTest>
{
private:
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

		{
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
		}

		NetworkManagerPtr manager = AM->GetManager<NetworkManager>();

		// Set new log
		using namespace Log;
		ILogPtr logPtr = std::make_shared<Logger>();
		ILog::set(logPtr);

		NetAddr4Ptr adr = NetAddr4::create();
		adr->setIp(127, 0, 0, 1);
		adr->setPort(12203);

		// Send remote command
		RemoteConsolePtr RCon = RemoteConsole::create(manager, adr, "12345");
		RCon->getHandlerList().set<RConHandlers::Print>([](const char* text)
		{
			MOHPC_LOG(Log, "Remote console \"%s\"", text);
		});
		RCon->send("echo test");

		Network::EngineServerPtr clientBase = Network::EngineServer::create(
			manager,
			adr
			//ISocketFactory::get()->createUdp(&bindAddress)
		);

		// Query server list
#if 0
		Network::ServerListPtr master = ServerList::create(manager, Network::gameListType_e::mohaab);
		master->fetch(
			[](const Network::IServerPtr& ptr)
			{
				ptr->query([ptr](const ReadOnlyInfo& response)
					{
						const NetAddrPtr& address = ptr->getAddress();
						const str version = response.ValueForKey("gamever");
						printf("Ping: %s:%d -> version %s\n", address->asString().c_str(), address->getPort(), version.c_str());
					},
					[ptr]()
					{
						const NetAddrPtr& address = ptr->getAddress();
						printf("Timed out: %s:%d\n", address->asString().c_str(), address->getPort());
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
		bool shouldJump = false;

		str mapfilename;

		CollisionWorldPtr cm;

		Network::ClientInfoPtr clientInfo = Network::ClientInfo::create();
		clientInfo->setName("mohpc_test");
		clientInfo->setRate(25000);

		srand((unsigned int)time(NULL));

		Network::ConnectSettingsPtr connectSettings = Network::ConnectSettings::create();
		connectSettings->setQport(rand() % 45536 + 20000);
		connectSettings->setCDKey("12345");

		clientBase->connect(clientInfo, connectSettings, [&](const Network::ClientGameConnectionPtr& cg, const char* errorMessage)
			{
				if (errorMessage)
				{
					MOHPC_LOG(Error, "server returned error: \"%s\"", errorMessage);
					wantsDisconnect = true;
					return;
				}

				connection = cg;

				CGameModuleBase* cgame = connection->getCGModule();

				connection->setCallback<ClientHandlers::Error>([](const Network::NetworkException& exception)
					{
						MOHPC_LOG(Log, "Exception of type \"%s\": \"%s\"", typeid(exception).name(), exception.what().c_str());
					});

				connection->setCallback<ClientHandlers::GameStateParsed>([&AM, &connection, cgame, &cm, &mapfilename](const Network::gameState_t& gameState, bool differentLevel)
					{
						const cgsInfo& cgs = cgame->getServerInfo();
						const str& loadedMap = cgs.getMapFilenameStr();
						if (differentLevel)
						{
							MOHPC_LOG(Log, "New map: \"%s\"", loadedMap.c_str());
							mapfilename = loadedMap;
						}
						
						BSPPtr Asset = AM->LoadAsset<BSP>(mapfilename.c_str());
						if(Asset)
						{
							cm = CollisionWorld::create();
							Asset->FillCollisionWorld(*cm);
						}
						
						connection->markReady();
					});

				connection->setCallback<ClientHandlers::ServerRestarted>([]()
					{
						MOHPC_LOG(Log, "server restarted");
					});

				fnHandle_t cb = cgame->setCallback<CGameHandlers::EntityAdded>([&connection](const EntityInfo& entity)
					{
						const char* modelName = connection->getGameState().getConfigString(CS_MODELS + entity.nextState.modelindex);
						MOHPC_LOG(VeryVerbose, "new entity %d, model \"%s\"", entity.nextState.number, modelName);
					});

				cgame->setCallback<CGameHandlers::EntityRemoved>([&connection](const EntityInfo& entity)
					{
						const char* modelName = connection->getGameState().getConfigString(CS_MODELS + entity.currentState.modelindex);
						MOHPC_LOG(VeryVerbose, "entity %d deleted (was model \"%s\")", entity.currentState.number, modelName);
					});

				cgame->setCallback<CGameHandlers::MakeBulletTracer>([&logPtr](const Vector& barrel, const Vector& start, const Vector& end, uint32_t numBullets, uint32_t iLarge, uint32_t numTracersVisible, float bulletSize)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "bullet %zu", num++);
					});

				cgame->setCallback<CGameHandlers::Impact>([&logPtr](const Vector& origin, const Vector& normal, uint32_t large)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "impact %zu", num++);
					});

				cgame->setCallback<CGameHandlers::MakeExplosionEffect>([&logPtr](const Vector& origin, effects_e type)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "explosionfx %zu: type \"%s\"", num++, getEffectName(type));
					});

				cgame->setCallback<CGameHandlers::MakeEffect>([&logPtr](const Vector& origin, const Vector& normal, effects_e type)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "effect %zu: type \"%s\"", num++, getEffectName(type));
					});

				cgame->setCallback<CGameHandlers::SpawnDebris>([&logPtr](CGameHandlers::debrisType_e debrisType, const Vector& origin, uint32_t numDebris)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "debris %zu: type %d", num++, debrisType);
					});

				cgame->setCallback<CGameHandlers::HitNotify>([&logPtr]()
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "hit %zu", num++);
					});

				cgame->setCallback<CGameHandlers::KillNotify>([&logPtr]()
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "kill %zu", num++);
					});

				cgame->setCallback<CGameHandlers::VoiceMessage>([&logPtr](const Vector& origin, bool local, uint8_t clientNum, const char* soundName)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "voice %d: sound \"%s\"", num++, soundName);
					});

				cgame->setCallback<CGameHandlers::Print>([&logPtr](hudMessage_e hudMessage, const char* text)
					{
						MOHPC_LOG(VeryVerbose, "server print (%d): \"%s\"", hudMessage, text);
					});

				cgame->setCallback<CGameHandlers::HudPrint>([&logPtr](const char* text)
					{
						MOHPC_LOG(VeryVerbose, "server print \"%s\"", text);
					});

				cgame->setCallback<CGameHandlers::HudDraw_Shader>([&logPtr](uint8_t index, const char* shaderName)
					{
						MOHPC_LOG(Verbose, "huddraw_shader : %d \"%s\"", index, shaderName);
					});

				cgame->setCallback<CGameHandlers::VoteModified>([](const voteInfo_t& voteInfo)
					{
						if(voteInfo.getVoteTime())
						{
							MOHPC_LOG(Verbose, "vote: \"%s\"", voteInfo.getVoteString());
							MOHPC_LOG(
								Verbose,
								"vote: yes %d no %d undecided %d",
								voteInfo.getNumVotesYes(),
								voteInfo.getNumVotesNo(),
								voteInfo.getNumVotesUndecided()
							);
						}
						else {
							MOHPC_LOG(Verbose, "vote: \"%s\" has ended", voteInfo.getVoteString());
						}
					});

				connection->setCallback<ClientHandlers::Disconnect>([&wantsDisconnect](const char* reason)
					{
						MOHPC_LOG(VeryVerbose, "Requested disconnect. Reason -> \"%s\"", reason ? reason : "");
						wantsDisconnect = true;
					});

				connection->setCallback<ClientHandlers::Timeout>([]()
					{
						MOHPC_LOG(VeryVerbose, "Server connection timed out");
					});

				connection->setCallback<ClientHandlers::UserInput>(
				[&forwardValue, &rightValue, &angle, &shouldJump, cgame, &connection, &cm](usercmd_t& ucmd, usereyes_t& eyeinfo)
					{
						const uint32_t seq = connection->getCurrentServerMessageSequence();

						eyeinfo.setAngles(30.f, angle);
						ucmd.buttons.fields.button.run = true;
						ucmd.moveForward((int8_t)(forwardValue * 127.f));
						ucmd.moveRight((int8_t)(rightValue * 127.f));
						ucmd.setAngles(30.f, angle, 0.f);
						if (shouldJump)
						{
							ucmd.jump();
						}
					});

				connection->setCallback<ClientHandlers::PreWritePacket>([&shouldJump]()
				{
					shouldJump = false;
				});
			},
			[&wantsDisconnect]()
			{
				MOHPC_LOG(Error, "connect to server timed out");
				wantsDisconnect = true;
			});

		char buf[512];
		size_t count = 0;

		while(!wantsDisconnect)
		{
			/*
			if (connection)
			{
				CGameModuleBase* cgame = connection->getCGModule();
				if (cgame)
				{
					const playerState_t& ps = cgame->getPredictedPlayerState();
					MOHPC_LOG(Verbose, "(t %d, %d), origin = (%f %f %f)", connection->getServerTime(), ps.commandTime, ps.origin[0], ps.origin[1], ps.origin[2]);
				}
			}
			*/

			if (kbhit())
			{
				const char c = _getch();
				if (c == '\n' || c == '\r')
				{
					if(!count) buf[0] = 0;
					printf("\n");
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
					else if (!strcmp(cmd, "jump")) {
						shouldJump = true;
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
								connection->getSettings().setMaxPackets(maxPackets);
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
							connection->sendCommand(buf);
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
			Sleep(15);
		}
	}
};
