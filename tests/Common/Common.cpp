#include "Common.h"

#include <MOHPC/Version.h>
#include <MOHPC/Files/GameFileHelpers.h>
#include <MOHPC/Files/Managers/PakFileManager.h>
#include <MOHPC/Files/Managers/SystemFileManager.h>
#include <MOHPC/Files/Category.h>

#include <tclap/CmdLine.h>

#include <algorithm>
#include <cstdarg>
#include <ctime>

constexpr char MOHPC_LOG_NAMESPACE[] = "common";

std::filesystem::path gameDir;

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
		case logType_e::Fatal:
			typeStr = "fatal";
			break;
		case logType_e::Error:
			typeStr = "err";
			break;
		case logType_e::Warn:
			typeStr = "warn";
			break;
		case logType_e::Info:
			typeStr = "info";
			break;
		case logType_e::Debug:
			typeStr = "dbg";
			break;
		case logType_e::Trace:
			typeStr = "trace";
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

MOHPC::Log::ILogPtr logPtr;

void InitCommands(int argc, const char* argv[])
{
	TCLAP::CmdLine cmd("Commands", ' ', MOHPC::VERSION_STRING);

	TCLAP::ValueArg<std::filesystem::path> pathArg("p", "path", "Game path", false, std::filesystem::path(), "string");
	cmd.add(pathArg);

	cmd.parse(argc, argv);

	gameDir = pathArg.getValue();
}

void InitCommon(int argc, const char* argv[])
{
	using namespace MOHPC::Log;
	logPtr = MOHPC::makeShared<Logger>();
	ILog::set(logPtr);

	MOHPC_LOG(Info, "MOHPC %s %s", MOHPC::VERSION_STRING, MOHPC::VERSION_ARCHITECTURE);

	InitCommands(argc, argv);
}

const std::filesystem::path& GetGamePathFromCommandLine()
{
	return gameDir;
}

MOHPC::AssetManagerPtr AssetLoad(const MOHPC::fs::path& path)
{
	MOHPC::FileCategoryManagerPtr catMan = MOHPC::FileCategoryManager::create();
	MOHPC::PakFileManagerProxyPtr pakFM = MOHPC::PakFileManagerProxy::create();
	MOHPC::SystemFileManagerProxyPtr sysFM = MOHPC::SystemFileManagerProxy::create(pakFM);

	MOHPC::AssetManagerPtr AM = MOHPC::AssetManager::create(sysFM, catMan);

	if (path.native().length()) {
		MOHPC::FileHelpers::FillGameDirectory(*catMan, pakFM->get(), sysFM->get(), path);
	}
	return AM;
}
