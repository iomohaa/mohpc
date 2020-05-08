#pragma once

#include "Global.h"
#include "Utilities/SharedPtr.h"

namespace MOHPC
{
	namespace Log
	{
		enum class logType_e
		{
			VeryVerbose,
			Verbose,
			Log,
			Warning,
			Error,
			Disconnect
		};

		using ILogPtr = SharedPtr<class ILog>;

		class MOHPC_EXPORTS ILog
		{
		public:
			virtual void log(logType_e type, const char* serviceName, const char* fmt, ...) = 0;

			void* operator new(size_t sz);
			void operator delete(void* ptr);

			/** Get the current logger. */
			static ILog& get();

			/** Set a new logger. */
			static void set(const ILogPtr& newLogger);

			/** Revert to the default logger. */
			static void reset();
		};

#define MOHPC_LOG(type, fmt, ...) MOHPC::Log::ILog::get().log(MOHPC::Log::logType_e::type, MOHPC_LOG_NAMESPACE, fmt, __VA_ARGS__)
	}
}
