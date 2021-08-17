#include "Common.h"

#include <MOHPC/Version.h>
#include <MOHPC/Files/GameFileHelpers.h>

#include <cstdarg>
#include <ctime>

using namespace MOHPC;

constexpr char MOHPC_LOG_NAMESPACE[] = "common";

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

void InitCommon()
{
	using namespace MOHPC::Log;
	logPtr = MOHPC::makeShared<Logger>();
	ILog::set(logPtr);

	MOHPC_LOG(Info, "MOHPC %s %s", VERSION_STRING, VERSION_ARCHITECTURE);
}

const char* GetGamePathFromCommandLine(int argc, const char* argv[])
{
	return argv[1];
}

MOHPC::AssetManagerPtr AssetLoad(const char* path)
{
	FileManagerPtr FM = FileManager::create();
	AssetManagerPtr AM = AssetManager::create(FM);

	FileHelpers::FillGameDirectory(*FM, path);
	return AM;
}
