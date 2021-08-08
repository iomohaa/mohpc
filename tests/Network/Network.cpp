#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Network/Client/ServerConnection.h>
#include <MOHPC/Network/Client/CGame/Module.h>
#include <MOHPC/Network/Client/MasterList.h>
#include <MOHPC/Network/Client/RemoteConsole.h>
#include <MOHPC/Network/Client/ServerQuery.h>
#include <MOHPC/Network/Client/Protocol.h>
#include <MOHPC/Network/Client/CGame/Prediction.h>
#include <MOHPC/Network/Remote/TCPMessageDispatcher.h>
#include <MOHPC/Network/Remote/UDPMessageDispatcher.h>
#include <MOHPC/Network/Remote/SocketUdpDelay.h>
#include <MOHPC/Common/str.h>
#include <MOHPC/Utility/Misc/MSG/Stream.h>
#include <MOHPC/Utility/Info.h>
#include <MOHPC/Utility/TokenParser.h>
#include <MOHPC/Utility/Tick.h>
#include <MOHPC/Utility/MessageQueueDispatcher.h>
#include <MOHPC/Assets/Formats/BSP.h>
#include <MOHPC/Utility/Collision/Collision.h>
#include <MOHPC/Common/Log.h>
#include <MOHPC/Version.h>
#include "Common/Common.h"
#include "Common/platform.h"

#include <ctime>
#include <cstdarg>
#include <iostream>
#include <vector>
#include <locale>
#include <thread>
#include <mutex>
#include <string>
#include <chrono>

#define MOHPC_LOG_NAMESPACE "test_net"

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;
using namespace std::placeholders;
using namespace std::chrono;

std::mutex bufLock;
bool wantsDisconnect;
size_t count = 0;
char buf[512];

void readCommandThread();
void testMasterServer(const NetAddr4Ptr& bindAddress);
ServerListPtr testGame(const NetAddr4Ptr& bindAddress, const NetAddr4Ptr& addr, const MessageDispatcherPtr& dispatcher, const UDPCommunicatorPtr& udpComm, Network::gameListType_e type);
void testLANQuery(const NetAddr4Ptr& bindAddress);
void fetchServer(const IServerPtr& ptr, MessageQueuePtr queuePtr, size_t& countAlive, size_t& countTotal);
void fetchServerLan(const IServerPtr& ptr, size_t& countAlive, size_t& countTotal);

class StufftextHandler : public ICommand
{
public:
	void execute(TokenParser& tokenized) override
	{
		// (don't) handle it
		MOHPC_LOG(Warn, "received stufftext command: [%s]", tokenized.GetLine(true));
	}
};

void waitFor(time_point<steady_clock> startTime, nanoseconds maxDelay);

int main(int argc, const char* argv[])
{
	InitCommon();

	MOHPC_LOG(Info, "Supported protocol(s):");
	for (const IClientProtocol* proto = IClientProtocol::getHead(); proto; proto = proto->getNext())
	{
		// print each supported protocol
		MOHPC_LOG(Info, "- protocol %d game version %s", proto->getServerProtocol(), proto->getVersion());
	}

	const MOHPC::AssetManagerPtr AM = AssetLoad(GetGamePathFromCommandLine(argc, argv));

	memset(buf, 0, sizeof(buf));
	count = 0;
	wantsDisconnect = false;

	// create a message dispatcher for server list
	NetAddr4Ptr bindAdr = NetAddr4::create();
	//bindAdr->setIp(0, 0, 0, 0);
	bindAdr->setIp(127, 0, 0, 1);
	bindAdr->setPort(rand() % 65536);
	//testMasterServer(bindAdr);
	//testLANQuery(bindAdr);

	NetAddr4Ptr adr = NetAddr4::create();
	NetAddr4Ptr adrGs = NetAddr4::create();
	adr->setIp(127, 0, 0, 1);
	//adr->setIp(149, 202, 88, 84);
	//adr->setIp(192, 168, 1, 85);
	adr->setPort(12203);
	adrGs->setIp(127, 0, 0, 1);
	//adrGs->setIp(149, 202, 88, 84);
	adrGs->setPort(12300);

	const IRemoteIdentifierPtr remoteIdentifier = IPRemoteIdentifier::create(adr);
	const IRemoteIdentifierPtr remoteIdentifierGs = IPRemoteIdentifier::create(adrGs);

	const MessageDispatcherPtr dispatcher = MessageDispatcher::create();

	IUdpSocketPtr udpSocket = ISocketFactory::get()->createUdp(bindAdr.get());
	UdpSocketSimLossPtr udpLossSim = UdpSocketSimLoss::create(udpSocket);
	UdpSocketSimLatencyPtr udpLatencySim = UdpSocketSimLatency::create(udpLossSim);
	const UDPCommunicatorPtr udpComm = UDPCommunicator::create(udpLatencySim);
	dispatcher->addComm(udpComm);

	// Send remote command
	/*
	RemoteConsolePtr RCon = RemoteConsole::create(dispatcher, udpComm, remoteIdentifier, "12345");
	RCon->getHandlerList().printHandler.add([](const char* text)
	{
		MOHPC_LOG(Info, "Remote console \"%s\"", text);
	});
	RCon->send("echo test");
	*/

	// process rcon command
	//dispatcher->processIncomingMessages();

	const EngineServerPtr clientBase = Network::EngineServer::create(dispatcher, udpComm, remoteIdentifier);
	const GSServerPtr gsServer = Network::GSServer::create(dispatcher, udpComm, remoteIdentifierGs);

	clientBase->getInfo([](const ReadOnlyInfo* info)
		{
			str jsonString = InfoJson::toJson<JsonStyleBeautifier>(*info);
			MOHPC_LOG(Info, "ServerInfo: %s", jsonString.c_str());
		});

	gsServer->query([](const ReadOnlyInfo& info)
		{
			str jsonString = InfoJson::toJson<JsonStyleBeautifier>(info);
			MOHPC_LOG(Info, "ServerGSInfo: %s", jsonString.c_str());
		},
		[]()
		{
			MOHPC_LOG(Error, "Timed out");
		}, 3000);

	// process server info command (can be batched)
	dispatcher->processIncomingMessages();

	Network::ServerConnectionPtr connection;

	milliseconds maxDelay(1000 / 20);
	bool circle = false;
	float forwardValue = 0.f;
	float rightValue = 0.f;
	float angleYaw = 0.f;
	float anglePitch = 0.f;
	struct {
		const WeaponCommand* weaponCommand;
		bool shouldJump;
		bool attackPrimary;
		bool attackSecondary;
		bool use;
	} buttons;
	memset(&buttons, 0, sizeof(buttons));

	str mapfilename;

	CollisionWorldPtr cm;

	const Network::UserInfoPtr userInfo = Network::UserInfo::create();
	Network::UserGameInputModulePtr inputModule;
	ModuleBase* cgame = nullptr;
	Network::CGame::PredictionPtr prediction;
	userInfo->setName("mohpc_test");
	userInfo->setRate(25000);
	const Network::ConnectSettingsPtr connectSettings = Network::ConnectSettings::create();
	connectSettings->setQport(rand() % 45536 + 20000);
	connectSettings->setCDKey("12345");
	usercmd_t oldcmd;
	uint8_t lastVMChanged = 0;

	StufftextHandler stufftextHandler;

	const TickableObjectsPtr tickableObjects = TickableObjects::create();

	clientBase->connect(userInfo, connectSettings, [&](const Network::ServerConnectionPtr& cg, const char* errorMessage)
		{
			if (errorMessage)
			{
				MOHPC_LOG(Error, "server returned error: \"%s\"", errorMessage);
				wantsDisconnect = true;
				return;
			}

			connection = cg;
			tickableObjects->addTickable(connection.get());

			connection->getRemoteCommandManager().add("stufftext", &stufftextHandler);

			cgame = connection->getCGModule();
			prediction = Network::CGame::Prediction::create(cg->getProtocolType());

			connection->getHandlerList().errorHandler.add([](const Network::NetworkException& exception)
				{
					MOHPC_LOG(Info, "Exception of type \"%s\": \"%s\"", typeid(exception).name(), exception.what().c_str());
				});

			//connection->getGameState().handlers().gameStateParsedHandler.add([&AM, &connection, cgame, &cm, &mapfilename, &inputModule](const Network::ServerGameState& gameState, bool differentLevel)
			connection->getGameState().handlers().gameStateParsedHandler.add([&](const Network::ServerGameState& gameState, bool differentLevel)
				{
					const cgsInfo& cgs = cgame->getServerInfo();
					const str& loadedMap = cgs.getMapFilenameStr();
					if (differentLevel)
					{
						MOHPC_LOG(Info, "New map: \"%s\"", loadedMap.c_str());
						mapfilename = loadedMap;
					}

					/*
					BSPPtr Asset = AM->LoadAsset<BSP>(mapfilename.c_str());
					if (Asset)
					{
						cm = CollisionWorld::create();
						Asset->FillCollisionWorld(*cm);
					}
					*/

					inputModule = UserGameInputModule::create(connection->getProtocolType());
					connection->setInputModule(inputModule);

					inputModule->getHandlerList().userInputHandler.add(
						[&](usercmd_t& ucmd, usereyes_t& eyeinfo)
						{
							/*
							using namespace MOHPC::ticks;

							MOHPC_LOG(
								Trace, "time: %llu / %llu",
								duration_cast<milliseconds>(ucmd.getServerTime().time_since_epoch()).count(),
								connection->getSnapshotManager().getServerTime().time_since_epoch().count()
							);
							*/

							const uint32_t seq = connection->getCurrentServerMessageSequence();
							const playerState_t& ps = prediction->getPredictedPlayerState();

							uint16_t deltaAngles[3];
							ps.getDeltaAngles(deltaAngles);

							float pitch = ShortToAngle(deltaAngles[0]);
							float yaw = ShortToAngle(deltaAngles[1]);
							static int8_t zHeight = 82;

							eyeinfo.setOffset(0, 0, zHeight);
							eyeinfo.setAngles(pitch + anglePitch, yaw + angleYaw);
							ucmd.getAction().addButton(UserButtons::Run);
							ucmd.getMovement().moveForward((int8_t)(forwardValue * 127.f));
							ucmd.getMovement().moveRight((int8_t)(rightValue * 127.f));
							ucmd.getMovement().setAngles(anglePitch, angleYaw, 0.f);
							if (buttons.shouldJump)
							{
								UserExecuteMovementJump::execute(ucmd.getMovement());
							}

							if (buttons.attackPrimary) ucmd.getAction().addButton(UserButtons::AttackPrimary);
							if (buttons.attackSecondary) ucmd.getAction().addButton(UserButtons::AttackSecondary);
							if (buttons.use) ucmd.getAction().addButton(UserButtons::Use);
							if (buttons.weaponCommand != nullptr) ucmd.getAction().addWeaponCommand(*buttons.weaponCommand);

							if (oldcmd.getAction().isHeld(UserButtons::AttackPrimary) != ucmd.getAction().isHeld(UserButtons::AttackPrimary))
							{
								MOHPC_LOG(Trace, "fire (time %llu)", ucmd.getServerTime().time_since_epoch().count());
							}

							oldcmd = ucmd;
						});
				});

			connection->getSnapshotManager().getHandlers().serverRestartedHandler.add([]()
				{
					MOHPC_LOG(Info, "server restarted");
				});

			connection->getGameState().handlers().configStringHandler.add([](csNum_t csNum, const char* configString)
				{
					MOHPC_LOG(Info, "cs %d modified: %s", csNum, configString);
				});

			fnHandle_t cb = cgame->getSnapshotProcessor().handlers().entityAddedHandler.add([&connection](const entityState_t& state)
				{
					const char* modelName = connection->getGameState().get().getConfigstringManager().getConfigString(CS_MODELS + state.modelindex);
					MOHPC_LOG(Trace, "new entity %d, model \"%s\"", state.number, modelName);
				});

			cgame->getSnapshotProcessor().handlers().entityRemovedHandler.add([&connection](const entityState_t& state)
				{
					const char* modelName = connection->getGameState().get().getConfigstringManager().getConfigString(CS_MODELS + state.modelindex);
					MOHPC_LOG(Trace, "entity %d deleted (was model \"%s\")", state.number, modelName);
				});

			cgame->getGameplayNotify().getBulletNotify().createBulletTracerHandler.add([](const_vec3r_t barrel, const_vec3r_t start, const_vec3r_t end, uint32_t numBullets, uint32_t iLarge, uint32_t numTracersVisible, float bulletSize)
				{
					static size_t num = 0;
					//MOHPC_LOG(Trace, "bullet %zu", num++);
				});

			cgame->getGameplayNotify().getImpactNotify().impactHandler.add([](const_vec3r_t origin, const_vec3r_t normal, uint32_t large)
				{
					static size_t num = 0;
					//MOHPC_LOG(Trace, "impact %zu", num++);
				});

			cgame->getGameplayNotify().getImpactNotify().explosionHandler.add([](const_vec3r_t origin, const char* modelName)
				{
					static size_t num = 0;
					//MOHPC_LOG(Trace, "explosionfx %zu: type \"%s\"", num++, getEffectName(type));
				});

			cgame->getGameplayNotify().getEffectNotify().spawnEffectHandler.add([](const_vec3r_t origin, const_vec3r_t normal, const char* modelName)
				{
					static size_t num = 0;
					//MOHPC_LOG(Trace, "effect %zu: type \"%s\"", num++, getEffectName(type));
				});

			cgame->getGameplayNotify().getEffectNotify().spawnDebrisHandler.add([](debrisType_e debrisType, const_vec3r_t origin, uint32_t numDebris)
				{
					static size_t num = 0;
					//MOHPC_LOG(Trace, "debris %zu: type %d", num++, debrisType);
				});

			cgame->getGameplayNotify().getEventNotify().hitHandler.add([]()
				{
					static size_t num = 0;
					//MOHPC_LOG(Trace, "hit %zu", num++);
				});

			cgame->getGameplayNotify().getEventNotify().gotKillHandler.add([]()
				{
					static size_t num = 0;
					//MOHPC_LOG(Trace, "kill %zu", num++);
				});

			cgame->getGameplayNotify().getEventNotify().voiceMessageHandler.add([](const_vec3r_t origin, bool local, uint8_t clientNum, const char* soundName)
				{
					static size_t num = 0;
					//MOHPC_LOG(Trace, "voice %d: sound \"%s\"", num++, soundName);
				});

			cgame->getGameplayNotify().getHUDNotify().setShaderHandler.add([](uint8_t index, const char* shaderName)
				{
					MOHPC_LOG(Debug, "huddraw_shader : %d \"%s\"", index, shaderName);
				});

			cgame->handlers().printHandler.add([](hudMessage_e hudMessage, const char* text)
				{
					MOHPC_LOG(Trace, "server print (%d): \"%s\"", hudMessage, text);
				});

			cgame->handlers().hudPrintHandler.add([](const char* text)
				{
					MOHPC_LOG(Trace, "server print \"%s\"", text);
				});

			cgame->getVoteManager().handlers().voteModifiedHandler.add([](const VoteManager& voteInfo)
				{
					if (voteInfo.getVoteTime())
					{
						MOHPC_LOG(Debug, "vote: \"%s\"", voteInfo.getVoteString());

						uint32_t numVotesYes = 0, numVotesNo = 0, numVotesUndecided = 0;
						voteInfo.getVotesCount(numVotesYes, numVotesNo, numVotesUndecided);
						MOHPC_LOG(
							Debug,
							"vote: yes %d no %d undecided %d",
							numVotesYes,
							numVotesNo,
							numVotesUndecided
						);
					}
					else {
						MOHPC_LOG(Debug, "vote: \"%s\" has ended", voteInfo.getVoteString());
					}
				});

			connection->getHandlerList().disconnectHandler.add([](const char* reason)
				{
					MOHPC_LOG(Trace, "Requested disconnect. Reason -> \"%s\"", reason ? reason : "");
					wantsDisconnect = true;
				});

			connection->getHandlerList().timeoutHandler.add([]()
				{
					MOHPC_LOG(Trace, "Server connection timed out");
				});

			connection->getHandlerList().preWritePacketHandler.add([&buttons]()
				{
					buttons.shouldJump = false;
					buttons.attackPrimary = false;
					buttons.attackSecondary = false;
					buttons.use = false;
					buttons.weaponCommand = nullptr;
				});
		},
		[]()
		{
			MOHPC_LOG(Error, "connect to server timed out");
			wantsDisconnect = true;
		});

	dispatcher->processIncomingMessages();

	std::thread inputThread(&readCommandThread);

	nanoseconds timeRemaining = nanoseconds();
	nanoseconds deltaTime = nanoseconds();

	while (!wantsDisconnect && tickableObjects->hasAnyTicks())
	{
		/*
		if (connection)
		{
			ModuleBase* cgame = connection->getCGModule();
			if (cgame)
			{
				const playerState_t& ps = cgame->getPredictedPlayerState();
				MOHPC_LOG(Debug, "(t %d, %d), origin = (%f %f %f)", connection->getServerTime(), ps.commandTime, ps.origin[0], ps.origin[1], ps.origin[2]);
			}
		}
		*/

		const time_point<steady_clock> startTime = steady_clock::now();

		if (count)
		{
			std::scoped_lock l(bufLock);

			TokenParser parser;
			parser.Parse(buf, count);

			const char* cmd = parser.GetToken(false);

			if (!strcmp(cmd, "forward")) {
				forwardValue = 1.f;
			}
			else if (!strcmp(cmd, "backward")) {
				forwardValue = -1.f;
			}
			else if (!strcmp(cmd, "right")) {
				rightValue = 1.f;
			}
			else if (!strcmp(cmd, "left")) {
				rightValue = -1.f;
			}
			else if (!strcmp(cmd, "circle")) {
				circle = true;
			}
			else if (!strcmp(cmd, "stop"))
			{
				forwardValue = 0.f;
				rightValue = 0.f;
				circle = false;
			}
			else if (!strcmp(cmd, "turnleft")) {
				angleYaw += 20.f;
			}
			else if (!strcmp(cmd, "turnright")) {
				angleYaw -= 20.f;
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
					const UserInfoPtr& userInfo = connection->getUserInfo();
					userInfo->setRate(30000);
					userInfo->setName("modified");
					userInfo->setSnaps(1);
					connection->updateUserInfo();
				}
			}
			else if (!strcmp(cmd, "useweaponclass"))
			{
				const char* wpClass = parser.GetToken(false);
				if (!strcmp(wpClass, "pistol")) {
					buttons.weaponCommand = &WeaponCommands::UsePistol;
				}
				else if (!strcmp(wpClass, "rifle")) {
					buttons.weaponCommand = &WeaponCommands::UseRifle;
				}
				else if (!strcmp(wpClass, "smg")) {
					buttons.weaponCommand = &WeaponCommands::UseSmg;
				}
				else if (!strcmp(wpClass, "mg")) {
					buttons.weaponCommand = &WeaponCommands::UseMg;
				}
				else if (!strcmp(wpClass, "grenade")) {
					buttons.weaponCommand = &WeaponCommands::UseGrenade;
				}
				else if (!strcmp(wpClass, "heavy")) {
					buttons.weaponCommand = &WeaponCommands::UseHeavy;
				}
				else if (!strcmp(wpClass, "item")) {
					buttons.weaponCommand = &WeaponCommands::UseItem1;
				}
				else if (!strcmp(wpClass, "item2")) {
					buttons.weaponCommand = &WeaponCommands::UseItem2;
				}
				else if (!strcmp(wpClass, "item3")) {
					buttons.weaponCommand = &WeaponCommands::UseItem3;
				}
				else if (!strcmp(wpClass, "item4")) {
					buttons.weaponCommand = &WeaponCommands::UseItem4;
				}
			}
			else if (!strcmp(cmd, "weapnext")) {
				buttons.weaponCommand = &WeaponCommands::NextWeapon;
			}
			else if (!strcmp(cmd, "weapprev")) {
				buttons.weaponCommand = &WeaponCommands::PreviousWeapon;
			}
			else if (!strcmp(cmd, "useLast")) {
				buttons.weaponCommand = &WeaponCommands::UseLast;
			}
			else if (!strcmp(cmd, "holster")) {
				buttons.weaponCommand = &WeaponCommands::Holster;
			}
			else if (!strcmp(cmd, "set"))
			{
				const char* key = parser.GetToken(false);
				if (!strcmp(key, "rate"))
				{
					const uint32_t rate = parser.GetInteger(false);
					if (connection)
					{
						const UserInfoPtr& userInfo = connection->getUserInfo();
						userInfo->setRate(rate);
						connection->updateUserInfo();
					}
				}
				else if (!strcmp(key, "snaps"))
				{
					const uint32_t snaps = parser.GetInteger(false);
					if (connection)
					{
						const UserInfoPtr& userInfo = connection->getUserInfo();
						userInfo->setSnaps(snaps);
						connection->updateUserInfo();
					}
				}
				else if (!strcmp(key, "timeNudge"))
				{
					const uint32_t timeNudge = parser.GetInteger(false);
					if (connection)
					{
						connection->getClientTime().setTimeNudge(timeNudge);
					}
				}
				else if (!strcmp(key, "maxpackets"))
				{
					const uint32_t maxPackets = parser.GetInteger(false);
					if (connection) {
						connection->getSettings().setMaxPackets(maxPackets);
					}
				}
				else if (!strcmp(key, "maxfps"))
				{
					const uint32_t maxfps = parser.GetInteger(false);
					if (maxfps > 0) {
						maxDelay = milliseconds(1000 / maxfps);
					}
				}
				else if (!strcmp(key, "minPing"))
				{
					const uint32_t minPing = parser.GetInteger(false);
					udpLatencySim->setLatency(minPing);
				}
				else if (!strcmp(key, "pingVariance"))
				{
					const uint32_t pingVariance = parser.GetInteger(false);
					udpLatencySim->setLatencyVariance(pingVariance);
				}
				else if (!strcmp(key, "ood"))
				{
					const bool ood = parser.GetBoolean(false);
					udpLatencySim->setOutOfOrder(ood);
				}
				else if (!strcmp(key, "packetRecvLoss"))
				{
					const float loss = parser.GetFloat(false);
					udpLossSim->setInboundPacketLossAlpha(loss / 100);
				}
				else if (!strcmp(key, "packetSendLoss"))
				{
					const float loss = parser.GetFloat(false);
					udpLossSim->setOutboundPacketLossAlpha(loss / 100);
				}
				else if (!strcmp(key, "packetDup"))
				{
					const float dup = parser.GetFloat(false);
					udpLossSim->setDuplicateAlpha(dup / 100);
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
					connection->getReliableSequence()->addCommand(buf);
				}
			}

			count = 0;
		}

		tickableObjects->processTicks();

		/*
		using namespace MOHPC::ticks;

		const ClientTime& clientTime = connection->getClientTime();
		MOHPC_LOG(
			Trace, "time: %llu / %llu / %llu",
			duration_cast<milliseconds>(clientTime.getRemoteStartTime().time_since_epoch()).count(),
			duration_cast<milliseconds>(clientTime.getRemoteTime().time_since_epoch()).count(),
			duration_cast<milliseconds>(clientTime.getSimulatedRemoteTime().time_since_epoch()).count()
		);
		*/

		if (connection->getServerChannel().isChannelValid())
		{
			// process prediction stuff
			PredictionParm predictionParm{
				cgame->getSnapshotProcessor(),
				cgame->getServerInfo(),
				&inputModule->getUserInput()
			};

			prediction->process(connection->getClientTime(), predictionParm, true);

			const SnapshotInfo* snap = cgame->getSnapshotProcessor().getSnap();
			if (snap)
			{
				const uint8_t iVMChanged = snap->getPlayerState().iViewModelAnimChanged;
				if (iVMChanged != lastVMChanged)
				{
					lastVMChanged = iVMChanged;
					MOHPC_LOG(Trace, "viewmodelanim changed (time %llu)", connection->getClientTime().getSimulatedRemoteTime().time_since_epoch().count());
				}
			}
		}

		waitFor(startTime, maxDelay);

		if (circle)
		{
			const nanoseconds deltaTime = steady_clock::now() - startTime;
			const deltaTimeFloat_t frameTime = deltaTimeFloat_t(deltaTime);
			angleYaw = angleYaw + 90.f * frameTime.count();
		}
	}

	inputThread.detach();

	return 0;
}

/*
void waitFor(time_point<steady_clock> startTime, nanoseconds maxDelay)
{
	const nanoseconds deltaTime = steady_clock::now() - startTime;
	if (deltaTime < maxDelay)
	{
		// remaining
		setTimerResolution(1);
		std::this_thread::sleep_for(maxDelay - deltaTime);
		restoreTimerSolution(1);
	}
}
*/

void waitFor(time_point<steady_clock> startTime, nanoseconds maxDelay)
{
	for (;;)
	{
		const nanoseconds deltaTime = steady_clock::now() - startTime;
		if (deltaTime >= maxDelay)
		{
			break;
		}
	}
}

void readCommandThread()
{
	while (!wantsDisconnect)
	{
		std::string str;
		std::getline(std::cin, str);

		{
			std::scoped_lock l(bufLock);
			strncpy(buf, str.c_str(), sizeof(buf));
			count = std::min(sizeof(buf), str.length());
		}
	}
}

void testMasterServer(const NetAddr4Ptr& bindAddress)
{
	NetAddr4Ptr addr = ISocketFactory::get()->getHost("master.x-null.net");
	addr->setPort(28900);

	MessageQueueDispatcher queueDispatcher(20);

	const MessageDispatcherPtr& msgDispatcher = queueDispatcher.getDispatcher();
	const MessageQueuePtr& msgQueue = queueDispatcher.getQueue();

	const UDPCommunicatorPtr udpComm = UDPCommunicator::create();
	msgDispatcher->addComm(udpComm);

	// Query server list
	size_t countAlive = 0;
	size_t countTotal = 0;
	ServerListPtr masters[3];

	for (size_t i = 0; i < 3; ++i)
	{
		ServerListPtr& master = masters[i];
		master = testGame(bindAddress, addr, msgDispatcher, udpComm, (Network::gameListType_e)((uint32_t)Network::gameListType_e::mohaa + i));
		master->fetch(std::bind(&fetchServer, _1, msgQueue, std::ref(countAlive), std::ref(countTotal)));
	}

	MOHPC_LOG(Info, "Requesting master server...");

	queueDispatcher.process(100);

	MOHPC_LOG(Info, "Done querying, %zu alive(s), %zu total", countAlive, countTotal);
}

ServerListPtr testGame(const NetAddr4Ptr& bindAddress, const NetAddr4Ptr& addr, const MessageDispatcherPtr& dispatcher, const UDPCommunicatorPtr& udpComm, Network::gameListType_e type)
{
	const TCPCommunicatorPtr tcpComm = TCPCommunicator::create(ISocketFactory::get()->createTcp(addr, bindAddress.get()));
	dispatcher->addComm(tcpComm);

	return ServerList::create(dispatcher, tcpComm, udpComm, tcpComm->getRemoteIdentifier(), type);
}

void testLANQuery(const NetAddr4Ptr& bindAddress)
{
	MessageDispatcherPtr msgDispatcher = MessageDispatcher::create();

	const UDPBroadcastCommunicatorPtr brdComm = UDPBroadcastCommunicator::create(ISocketFactory::get()->createUdp(bindAddress.get()));

	msgDispatcher->addComm(brdComm);

	ServerListLANPtr serverList = ServerListLAN::create(msgDispatcher, brdComm);

	size_t countAlive = 0;
	size_t countTotal = 0;
	serverList->fetch(std::bind(&fetchServerLan, _1, std::ref(countAlive), std::ref(countTotal)));

	MOHPC_LOG(Info, "Requesting LAN...");
	msgDispatcher->processIncomingMessages();
	MOHPC_LOG(Info, "Done querying lan servers : %zu LAN server(s).", countAlive);
}

void fetchServer(const IServerPtr& ptr, MessageQueuePtr queuePtr, size_t& countAlive, size_t& countTotal)
{
	ServerQueryPtr serverQuery = ServerQuery::create(ptr, [ptr, &countAlive, &countTotal](const ReadOnlyInfo& response)
		{
			const IRemoteIdentifierPtr& identifier = ptr->getIdentifier();
			const str version = response.ValueForKey("gamever");
			MOHPC_LOG(Info, "Ping: %s -> version %s", identifier->getString().c_str(), version.c_str());
			++countAlive;
			++countTotal;
		},
		[ptr, &countTotal]()
		{
			const IRemoteIdentifierPtr& identifier = ptr->getIdentifier();
			MOHPC_LOG(Info, "Timed out: %s", identifier->getString().c_str());
			++countTotal;
		}
		);

	queuePtr->transmit(serverQuery);
}

void fetchServerLan(const IServerPtr& ptr, size_t& countAlive, size_t& countTotal)
{
	ptr->query([ptr, &countAlive, &countTotal](const ReadOnlyInfo& response)
		{
			const IRemoteIdentifierPtr& identifier = ptr->getIdentifier();
			const str version = response.ValueForKey("gamever");
			MOHPC_LOG(Info, "Ping: %s -> version %s", identifier->getString().c_str(), version.c_str());
			++countAlive;
			++countTotal;
		},
		[ptr, &countTotal]()
		{
			const IRemoteIdentifierPtr& identifier = ptr->getIdentifier();
			MOHPC_LOG(Info, "Timed out: %s", identifier->getString().c_str());
			++countTotal;
		}
		);
}
