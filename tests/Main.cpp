#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Managers/ShaderManager.h>
#include <MOHPC/Log.h>
#include <MOHPC/Utilities/SharedPtr.h>
#include "UnitTest.h"

#include <cstdarg>
#include <ctime>

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

int main(int argc, char *argv[])
{
#ifdef _MSC_VER
	// Get current flag
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

	// Debug allocation
	tmpFlag |= _CRTDBG_ALLOC_MEM_DF;

	// Leak detection
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

	// Memory check (very slow)
	//tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;

	// Check for _CRT_BLOCK memory (internal CRT memory)
	//tmpFlag |= _CRTDBG_CHECK_CRT_DF;

	// Set flag to the new value.
	_CrtSetDbgFlag(tmpFlag);
#endif

	srand((unsigned int)time(NULL));


	// Set new log
	using namespace MOHPC::Log;
	ILogPtr logPtr = MOHPC::makeShared<Logger>();
	ILog::set(logPtr);

	// Test if it destroys well
	{
		MOHPC::AssetManagerPtr AM = MOHPC::AssetManager::create();
		MOHPC::FileManager* FM = AM->GetFileManager();
	}

	{
		MOHPC::AssetManagerPtr AM = MOHPC::AssetManager::create();
		MOHPC::FileManager* FM = AM->GetFileManager();

		printf("Loading pak files...\n");

#ifdef _WIN32
		FM->FillGameDirectory("I:\\Jeux\\Mohaa\\");
#else
		FM->FillGameDirectory("/mnt/i/Jeux/Mohaa");
#endif
		{
			MOHPC::FileEntryList entries = FM->ListFilteredFiles("/maps", "", true, false);
		}

		IUnitTest::runAll(AM);
	}
	
#ifdef _WIN32
	//_CrtDumpMemoryLeaks();
#endif

	// The leak detector may output the following :
	//
	// Detected memory leaks!
	// Dumping objects ->
	// {4111180} normal block at 0x0000013D5C656E00, 8 bytes long.
	//  Data: <   ]=   > 00 04 03 5D 3D 01 00 00 
	// {4111011} normal block at 0x0000013D5C6504B0, 8 bytes long.
	//  Data: <   ]=   > 80 AB 03 5D 3D 01 00 00 
	// {4110656} normal block at 0x0000013D5C651360, 8 bytes long.
	//  Data: < z ]=   > C0 7A 03 5D 3D 01 00 00 
	// {315813} normal block at 0x0000013D5B59AB30, 2480 bytes long.
	//  Data: <                > 00 00 00 00 00 00 00 00 00 CD CD CD 00 00 00 00
	//
	// This is due to the event system allocating memory at initialization
	// And it is destroyed during destruction (after leak detection)

	return 0;
}
