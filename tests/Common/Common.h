#pragma once

#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Files/Managers/FileManager.h>
#include <MOHPC/Common/Log.h>

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

inline MOHPC::AssetManagerPtr AssetLoad()
{
	MOHPC::AssetManagerPtr AM = MOHPC::AssetManager::create();
	MOHPC::FileManager* FM = AM->GetFileManager();

#ifdef _WIN32
	FM->FillGameDirectory("I:\\Jeux\\MOHAA\\Install\\Windows");
#else
	FM->FillGameDirectory("/mnt/i/Jeux/MOHAA/Install/Windows");
#endif

	return AM;
}

void InitCommon();
