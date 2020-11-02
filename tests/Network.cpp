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
#include <MOHPC/Common/str.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Utilities/TokenParser.h>
#include <MOHPC/Formats/BSP.h>
#include <MOHPC/Collision/Collision.h>
#include <MOHPC/Log.h>
#include "UnitTest.h"
#include "platform.h"

#include <ctime>
#include <cstdarg>
#include <iostream>
#include <vector>
#include <locale>
#include <thread>
#include <mutex>
#include <string>

#define MOHPC_LOG_NAMESPACE "test_net"

class CNetworkUnitTest : public IUnitTest
{
public:
	virtual unsigned int priority()
	{
		return 1;
	}

	virtual const char* name() override
	{
		return "Networking";
	}

	virtual void run(const MOHPC::AssetManagerPtr& AM) override
	{
		using namespace MOHPC;
		using namespace MOHPC::Network;

		memset(buf, 0, sizeof(buf));
		count = 0;
		wantsDisconnect = false;

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

		NetAddr4Ptr adr = NetAddr4::create();
		//adr->setIp(127, 0, 0, 1);
		adr->setIp(192, 168, 1, 85);
		adr->setPort(12203);

		// Send remote command
		RemoteConsolePtr RCon = RemoteConsole::create(manager, adr, "12345");
		RCon->getHandlerList().printHandler.add([](const char* text)
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

		float forwardValue = 0.f;
		float rightValue = 0.f;
		float angle = 0.f;
		struct {
			weaponCommand_e weaponCommand;
			bool shouldJump;
			bool attackPrimary;
			bool attackSecondary;
			bool use;
		} buttons;
		memset(&buttons, 0, sizeof(buttons));

		str mapfilename;

		CollisionWorldPtr cm;

		Network::ClientInfoPtr clientInfo = Network::ClientInfo::create();
		clientInfo->setName("mohpc_test");
		clientInfo->setRate(25000);
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

				connection->getHandlerList().errorHandler.add([](const Network::NetworkException& exception)
					{
						MOHPC_LOG(Log, "Exception of type \"%s\": \"%s\"", typeid(exception).name(), exception.what().c_str());
					});

				connection->getHandlerList().gameStateParsedHandler.add([&AM, &connection, cgame, &cm, &mapfilename](const Network::gameState_t& gameState, bool differentLevel)
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

				connection->getHandlerList().serverRestartedHandler.add([]()
					{
						MOHPC_LOG(Log, "server restarted");
					});

				fnHandle_t cb = cgame->getHandlerList().entityAddedHandler.add([&connection](const EntityInfo& entity)
					{
						const char* modelName = connection->getGameState().getConfigString(CS_MODELS + entity.nextState.modelindex);
						MOHPC_LOG(VeryVerbose, "new entity %d, model \"%s\"", entity.nextState.number, modelName);
					});

				cgame->getHandlerList().entityRemovedHandler.add([&connection](const EntityInfo& entity)
					{
						const char* modelName = connection->getGameState().getConfigString(CS_MODELS + entity.currentState.modelindex);
						MOHPC_LOG(VeryVerbose, "entity %d deleted (was model \"%s\")", entity.currentState.number, modelName);
					});

				cgame->getHandlerList().makeBulletTracerHandler.add([](const Vector& barrel, const Vector& start, const Vector& end, uint32_t numBullets, uint32_t iLarge, uint32_t numTracersVisible, float bulletSize)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "bullet %zu", num++);
					});

				cgame->getHandlerList().impactHandler.add([](const Vector& origin, const Vector& normal, uint32_t large)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "impact %zu", num++);
					});

				cgame->getHandlerList().makeExplosionEffectHandler.add([](const Vector& origin, effects_e type)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "explosionfx %zu: type \"%s\"", num++, getEffectName(type));
					});

				cgame->getHandlerList().makeEffectHandler.add([](const Vector& origin, const Vector& normal, effects_e type)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "effect %zu: type \"%s\"", num++, getEffectName(type));
					});

				cgame->getHandlerList().spawnDebrisHandler.add([](CGameHandlers::debrisType_e debrisType, const Vector& origin, uint32_t numDebris)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "debris %zu: type %d", num++, debrisType);
					});

				cgame->getHandlerList().hitNotifyHandler.add([]()
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "hit %zu", num++);
					});

				cgame->getHandlerList().killNotifyHandler.add([]()
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "kill %zu", num++);
					});

				cgame->getHandlerList().voiceMessageHandler.add([](const Vector& origin, bool local, uint8_t clientNum, const char* soundName)
					{
						static size_t num = 0;
						//MOHPC_LOG(VeryVerbose, "voice %d: sound \"%s\"", num++, soundName);
					});

				cgame->getHandlerList().printHandler.add([](hudMessage_e hudMessage, const char* text)
					{
						MOHPC_LOG(VeryVerbose, "server print (%d): \"%s\"", hudMessage, text);
					});

				cgame->getHandlerList().hudPrintHandler.add([](const char* text)
					{
						MOHPC_LOG(VeryVerbose, "server print \"%s\"", text);
					});

				cgame->getHandlerList().huddrawShaderHandler.add([](uint8_t index, const char* shaderName)
					{
						MOHPC_LOG(Verbose, "huddraw_shader : %d \"%s\"", index, shaderName);
					});

				cgame->getHandlerList().voteModifiedHandler.add([](const voteInfo_t& voteInfo)
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

				connection->getHandlerList().disconnectHandler.add([this](const char* reason)
					{
						MOHPC_LOG(VeryVerbose, "Requested disconnect. Reason -> \"%s\"", reason ? reason : "");
						wantsDisconnect = true;
					});

				connection->getHandlerList().timeoutHandler.add([]()
					{
						MOHPC_LOG(VeryVerbose, "Server connection timed out");
					});

				connection->getHandlerList().userInputHandler.add(
				[&forwardValue, &rightValue, &angle, &buttons, cgame, &connection, &cm](usercmd_t& ucmd, usereyes_t& eyeinfo)
					{
						const uint32_t seq = connection->getCurrentServerMessageSequence();

						eyeinfo.setAngles(30.f, angle);
						ucmd.setButtonFlags(BUTTON_RUN);
						ucmd.moveForward((int8_t)(forwardValue * 127.f));
						ucmd.moveRight((int8_t)(rightValue * 127.f));
						ucmd.setAngles(30.f, angle, 0.f);
						if (buttons.shouldJump)
						{
							ucmd.jump();
						}

						if (buttons.attackPrimary) ucmd.setButtonFlags(BUTTON_ATTACK_PRIMARY);
						if (buttons.attackSecondary) ucmd.setButtonFlags(BUTTON_ATTACK_SECONDARY);
						if (buttons.use) ucmd.setButtonFlags(BUTTON_USE);
						if (buttons.weaponCommand != weaponCommand_e::none) ucmd.setWeaponCommand(buttons.weaponCommand);
					});

				connection->getHandlerList().preWritePacketHandler.add([&buttons]()
				{
					buttons.shouldJump = false;
					buttons.attackPrimary = false;
					buttons.attackSecondary = false;
					buttons.use = false;
					buttons.weaponCommand = weaponCommand_e::none;
				});
			},
			[this]()
			{
				MOHPC_LOG(Error, "connect to server timed out");
				wantsDisconnect = true;
			});

		std::thread inputThread(&CNetworkUnitTest::readCommandThread, this);

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

			if (count)
			{
				std::scoped_lock(bufLock);

				TokenParser parser;
				parser.Parse(buf, count);

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
					buttons.shouldJump = true;
				}
				else if (!strcmp(cmd, "attackprimary")) {
					buttons.attackPrimary = true;
				}
				else if (!strcmp(cmd, "attacksecondary")) {
					buttons.attackSecondary = true;
				}
				else if (!strcmp(cmd, "use")) {
					buttons.use = true;
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
				else if (!strcmp(cmd, "useweaponclass"))
				{
					const char* wpClass = parser.GetToken(false);
					if(!strcmp(wpClass, "pistol")) {
						buttons.weaponCommand = weaponCommand_e::usePistol;
					}
					else if (!strcmp(wpClass, "rifle")) {
						buttons.weaponCommand = weaponCommand_e::useRifle;
					}
					else if (!strcmp(wpClass, "smg")) {
						buttons.weaponCommand = weaponCommand_e::useSmg;
					}
					else if (!strcmp(wpClass, "mg")) {
						buttons.weaponCommand = weaponCommand_e::useMg;
					}
					else if (!strcmp(wpClass, "grenade")) {
						buttons.weaponCommand = weaponCommand_e::useGrenade;
					}
					else if (!strcmp(wpClass, "heavy")) {
						buttons.weaponCommand = weaponCommand_e::useHeavy;
					}
					else if (!strcmp(wpClass, "item")) {
						buttons.weaponCommand = weaponCommand_e::useItem;
					}
					else if (!strcmp(wpClass, "item2")) {
						buttons.weaponCommand = weaponCommand_e::useItem2;
					}
					else if (!strcmp(wpClass, "item3")) {
						buttons.weaponCommand = weaponCommand_e::useItem3;
					}
					else if (!strcmp(wpClass, "item4")) {
						buttons.weaponCommand = weaponCommand_e::useItem4;
					}
				}
				else if (!strcmp(cmd, "weapnext")) {
					buttons.weaponCommand = weaponCommand_e::nextWeapon;
				}
				else if (!strcmp(cmd, "weapprev")) {
					buttons.weaponCommand = weaponCommand_e::prevWeapon;
				}
				else if (!strcmp(cmd, "useLast")) {
					buttons.weaponCommand = weaponCommand_e::useLast;
				}
				else if (!strcmp(cmd, "holster")) {
					buttons.weaponCommand = weaponCommand_e::holster;
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

				count = 0;
			}

			manager->processTicks();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		inputThread.detach();
	}

	void readCommandThread()
	{
		while(!wantsDisconnect)
		{
			std::string str;
			std::getline(std::cin, str);

			{
				std::scoped_lock(bufLock);
				strncpy(buf, str.c_str(), sizeof(buf));
				count = std::min(sizeof(buf), str.length());
			}
		}
	}

private:
	std::mutex bufLock;
	bool wantsDisconnect;
	size_t count = 0;
	char buf[512];
};
static CNetworkUnitTest unitTest;
