#include <MOHPC/Network/Parsing/String.h>
#include <MOHPC/Network/Parsing/Hash.h>
#include <MOHPC/Network/Parsing/Entity.h>
#include <MOHPC/Network/Parsing/GameState.h>
#include <MOHPC/Network/Parsing/PVS.h>
#include <MOHPC/Network/Types/GameState.h>

#include <MOHPC/Utility/Info.h>
#include <MOHPC/Utility/Misc/MSG/Stream.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>

#include "Common/Common.h"

using namespace MOHPC;
using namespace MOHPC::Network;

static constexpr char MOHPC_LOG_NAMESPACE[] = "test_netparse";

uint32_t testVersions[] =
{
	5,
	6,
	8,
	15,
	16,
	17
};

template<typename...Args>
void testAllVersions(void (*testFunction)(uint32_t version, Args... args), Args&&...args)
{
	for (size_t i = 0; i < sizeof(testVersions) / sizeof(testVersions[0]); ++i)
	{
		MOHPC_LOG(Info, "Testing version %d", testVersions[i]);
		testFunction(testVersions[i], std::forward<Args>(args)...);
	}
}

bool approximation(float number, float expect, float tolerance)
{
	return number >= expect - tolerance && number <= expect + tolerance;
}

void hashTestInternal(uint32_t version, const char* string, uint32_t expected)
{
	const Parsing::IHash* hash = Parsing::IHash::get(version);
	assert(hash);
	assert(hash->hashKey(string, str::len(string)) == expected);
}

void hashTest()
{
	testAllVersions(&hashTestInternal, (const char*)"abcdefghijklmnopqrstuvwxyz", uint32_t(376140));
}

void stringTestInternal(uint32_t version)
{
	const Parsing::IString* string = Parsing::IString::get(version);
	assert(string);

	uint8_t streamData[256];
	FixedDataMessageStream stream(streamData, sizeof(streamData));

	MSG msg(stream, msgMode_e::Both);
	string->writeString(msg, "testString");

	msg.Flush();
	stream.Seek(0);
	msg.Reset();

	StringMessage stringMessage = string->readString(msg);
	assert(!str::cmp(stringMessage.c_str(), "testString"));
}

void stringTest()
{
	testAllVersions(&stringTestInternal);
}

void entityTestInternal(uint32_t version)
{
	const Parsing::IEntity* entity = Parsing::IEntity::get(version);
	assert(entity);

	const Parsing::IPlayerState* playerState = Parsing::IPlayerState::get(version);
	assert(playerState);

	uint8_t streamData[256];
	FixedDataMessageStream stream(streamData, sizeof(streamData));

	entityState_t nullState;
	playerState_t nullPS;
	const float deltaTime = 0.05f;

	{
		MSG msg(stream, msgMode_e::Writing);
		entity->writeEntityNum(msg, 1);
		entity->writeEntityNum(msg, 2);
		entity->writeEntityNum(msg, 3);
		entity->writeEntityNum(msg, 4);
		entity->writeEntityNum(msg, 5);

		entityState_t validEntity;
		validEntity.actionWeight = 0.1f;
		validEntity.scale = 0.75f;
		validEntity.netorigin[0] = 1.f;
		validEntity.netorigin[1] = 100.f;
		validEntity.netorigin[2] = 1001.f;
		validEntity.netangles[1] = 50.f;
		validEntity.number = 5;
		entity->writeDeltaEntity(msg, nullptr, &validEntity, 5, deltaTime);

		playerState_t validPS;
		validPS.fLeanAngle = 5.f;
		validPS.origin[1] = 5000.f;
		validPS.origin[2] = 8002.f;
		validPS.iViewModelAnim = 2;
		validPS.speed = 400;
		playerState->writeDeltaPlayerState(msg, nullptr, &validPS);

		playerState_t validPS2 = validPS;
		validPS2.velocity[0] = 522.f;
		validPS2.velocity[1] = 250.f;
		validPS2.velocity[2] = 24.f;
		playerState->writeDeltaPlayerState(msg, &validPS, &validPS2);
	}

	stream.Seek(0);
	{
		MSG msg(stream, msgMode_e::Reading);
		assert(entity->readEntityNum(msg) == 1);
		assert(entity->readEntityNum(msg) == 2);
		assert(entity->readEntityNum(msg) == 3);
		assert(entity->readEntityNum(msg) == 4);
		assert(entity->readEntityNum(msg) == 5);

		entityState_t validEntity;
		entity->readDeltaEntity(msg, nullptr, &validEntity, 5, deltaTime);

		assert(approximation(validEntity.actionWeight, 0.1f, 0.01f));
		assert(validEntity.scale == 0.75f);
		assert(validEntity.netorigin[0] == 1.f);
		assert(validEntity.netorigin[1] == 100.f);
		assert(validEntity.netorigin[2] == 1001.f);
		assert(approximation(validEntity.netangles[1], 50.f, 0.5f));
		assert(validEntity.number == 5);


		playerState_t validPS;
		playerState->readDeltaPlayerState(msg, nullptr, &validPS);

		assert(validPS.fLeanAngle == 5.f);
		assert(validPS.origin[1] == 5000.f);
		assert(validPS.origin[2] == 8002.f);
		assert(validPS.iViewModelAnim == 2);
		assert(validPS.speed == 400);

		playerState->readDeltaPlayerState(msg, &validPS, &validPS);
		assert(validPS.velocity[0] == 522.f);
		assert(validPS.velocity[1] == 250.f);
		assert(validPS.velocity[2] == 24.f);
	}
}

void entityTest()
{
	testAllVersions(&entityTestInternal);
}

void gameStateTestInternal(uint32_t version)
{
	const Parsing::IGameState* gameStateParsing = Parsing::IGameState::get(version);

	uint8_t streamBuffer[32768];

	{
		FixedDataMessageStream stream(streamBuffer, sizeof(streamBuffer));

		gameState_t gs = gameStateParsing->create();

		MSG msg(stream, msgMode_e::Writing);

		ConfigStringManager& csMan = gs.getConfigstringManager();

		{
			Info systemInfo;
			systemInfo.SetValueForKey("sv_serverid", "1234567890");

			csMan.setConfigString(CS_SYSTEMINFO, systemInfo.GetString());
		}

		{
			Info serverInfo;
			serverInfo.SetValueForKey("sv_fps", "20");
			serverInfo.SetValueForKey("sv_misc_var1", "5555");
			serverInfo.SetValueForKey("g_var2", "stringValue");

			for (uint32_t i = 0; i < 100; ++i)
			{
				str keyName = "g_vari_" + str(i);
				str keyValue(i);
				serverInfo.SetValueForKey(keyName.c_str(), keyValue.c_str());
			}

			csMan.setConfigString(CS_SERVERINFO, serverInfo.GetString());
		}

		Parsing::gameStateClient_t client;
		client.commandSequence = 98765;
		client.clientNum = 1;
		client.checksumFeed = 0xABCDEF00;
		gameStateParsing->saveGameState(msg, gs, client);
	}

	{
		FixedDataMessageStream stream(streamBuffer, sizeof(streamBuffer));

		gameState_t gs = gameStateParsing->create();

		MSG msg(stream, msgMode_e::Reading);

		Parsing::gameStateClient_t clResults;
		gameStateParsing->parseGameState(msg, gs, clResults);

		Parsing::gameStateResults_t results;
		gameStateParsing->parseConfig(gs, results);

		assert(clResults.commandSequence == 98765);
		assert(clResults.clientNum == 1);
		assert(clResults.checksumFeed == 0xABCDEF00);
		assert(results.serverId == 1234567890);
		assert(results.serverDeltaTime == 50);
		assert(results.serverDeltaTimeSeconds == 0.05f);

		ReadOnlyInfo serverInfo(gs.getConfigstringManager().getConfigString(CS_SERVERINFO));

		assert(serverInfo.IntValueForKey("sv_misc_var1") == 5555);
		assert(!str::cmp(serverInfo.ValueForKey("g_var2"), "stringValue"));

		for (uint32_t i = 0; i < 100; ++i)
		{
			str keyName = "g_vari_" + str(i);
			str keyValue(i);
			const char* val = keyValue.c_str();
			assert(!str::cmp(serverInfo.ValueForKey(keyName.c_str()), keyValue.c_str()));
		}

		for (InfoIterator it(serverInfo.GetString(), serverInfo.GetInfoLength()); it; ++it)
		{
			MOHPC_LOG(Info, "\"%s\": \"%s\"", it.key(), it.value());
		}
	}
}

void gameStateTest()
{
	testAllVersions(&gameStateTestInternal);
}

void PVSTestAssert(uint32_t version, const Vector& origin1, const Vector& origin2, float yaw)
{
	const Parsing::IPVS* pvsParsing = Parsing::IPVS::get(version);

	radarInfo_t radarInfo;
	{
		Parsing::pvsParm_t parm;
		parm.clientNum = 3;
		parm.origin = origin1;

		radarUnpacked_t unpacked;
		unpacked.clientNum = 1;
		unpacked.x = origin2.x;
		unpacked.y = origin2.y;
		unpacked.yaw = yaw;

		radarInfo = pvsParsing->writeNonPVSClient(parm, unpacked);
	}

	{
		Parsing::pvsParm_t parm;
		parm.clientNum = 3;
		parm.origin = origin1;
		parm.radarRange = radarInfo.getCoordPrecision();
		parm.radarFarMult = 1.f;

		const float minX = origin1.x + radarInfo.getMinCoord();
		const float minY = origin1.y + radarInfo.getMinCoord();
		const float maxX = origin1.x + radarInfo.getMaxCoord();
		const float maxY = origin1.y + radarInfo.getMaxCoord();

		radarUnpacked_t unpacked;
		if (pvsParsing->readNonPVSClient(radarInfo, parm, unpacked))
		{
			assert(approximation(unpacked.yaw, yaw, radarInfo.getYawPrecision()));
			assert(approximation(unpacked.x, origin2.x, 1.f) || unpacked.x == minX || unpacked.x == maxX);
			assert(approximation(unpacked.y, origin2.y, 1.f) || unpacked.y == minY || unpacked.y == maxY);
		}
	}
}

void PVSTestInternal(uint32_t version)
{
	PVSTestAssert(version, Vector(100.f, 100.f, 100.f), Vector(105.f, 97.f, 100.f), 275.f);
	PVSTestAssert(version, Vector(100.f, 104.f, 100.f), Vector(88.f, 55.f, 100.f), 122.f);
	PVSTestAssert(version, Vector(101.f, 98.f, 100.f), Vector(88.f, 24.f, 100.f), 14.f);
	PVSTestAssert(version, Vector(311.4f, 250.1f, 100.f), Vector(188.2f, 31.3f, 100.f), 352.f);
}

void PVSTest()
{
	testAllVersions(&PVSTestInternal);
}

int main(int argc, const char* argv[])
{
	InitCommon();

	hashTest();
	stringTest();
	entityTest();
	gameStateTest();
	PVSTest();
}