#include <MOHPC/Network/Types/GameState.h>

#include "Common/Common.h"

#include <cassert>

using namespace MOHPC;
using namespace MOHPC::Network;

static constexpr char MOHPC_LOG_NAMESPACE[] = "test_gamestate";

void assertConfigString(const ConfigStringManager& csMan, csNum_t num, const char* expected)
{
	const char* cs = csMan.getConfigString(num);
	assert(!strHelpers::cmp(cs, expected));
}

void testConfigstring(gameState_t& gs)
{
	ConfigStringManager& csMan = gs.getConfigstringManager();

	csMan.setConfigString(0, "test1");
	csMan.setConfigString(1, "test2222");
	csMan.setConfigString(2, "test12345");

	assertConfigString(csMan, 0, "test1");
	assertConfigString(csMan, 1, "test2222");
	assertConfigString(csMan, 2, "test12345");

	csMan.setConfigString(0, "test");
	assertConfigString(csMan, 0, "test");
	assertConfigString(csMan, 1, "test2222");

	csMan.setConfigString(0, "test100");
	assertConfigString(csMan, 0, "test100");
	assertConfigString(csMan, 1, "test2222");

	csMan.setConfigString(1, "");
	assertConfigString(csMan, 1, "");
	assertConfigString(csMan, 2, "test12345");
	csMan.setConfigString(1, "something");
	assertConfigString(csMan, 1, "something");

	csMan.setConfigString(4, "cs4");
	csMan.setConfigString(3, "cs3_");
	assertConfigString(csMan, 4, "cs4");
	assertConfigString(csMan, 3, "cs3_");

	csMan.setConfigString(4, "");
	assertConfigString(csMan, 4, "");
	assertConfigString(csMan, 3, "cs3_");

	csMan.setConfigString(5, "test12");
	csMan.setConfigString(5, "test123");
	csMan.setConfigString(5, "test1");

	assertConfigString(csMan, 0, "test100");
	assertConfigString(csMan, 1, "something");
	assertConfigString(csMan, 2, "test12345");
	assertConfigString(csMan, 3, "cs3_");
	assertConfigString(csMan, 4, "");
	assertConfigString(csMan, 5, "test1");

	for (csNum_t i = 0; i < 6; ++i)
	{
		MOHPC_LOG(Info, "cs %d: '%s'", i, csMan.getConfigString(i));
	}
}

void testGameState()
{
	gameState_t gs(2000, 40000, 1024);
	testConfigstring(gs);
}

int main(int argc, const char* argv[])
{
	InitCommon();

	testGameState();
}
