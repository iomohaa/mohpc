#pragma once

#include "../Global.h"
#include "../Utility/SharedPtr.h"

namespace MOHPC
{
	namespace Log
	{
		enum class logType_e
		{
			/** Severe error events that will presumably lead to application to abort. */
			Fatal,
			/** Errors events not supposed to stop the application. */
			Error,
			/** Potentially harmful situations. */
			Warn,
			/** Informational messages, useful for showing progress. */
			Info,
			/** Fine-grained informational events, useful for debugging. */
			Debug,
			/** More fine-grained informational events than debug. */
			Trace
		};

		using ILogPtr = SharedPtr<class ILog>;

		class MOHPC_EXPORTS ILog
		{
		public:
			virtual ~ILog() = default;
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

#define MOHPC_LOG(type, fmt, ...) MOHPC::Log::ILog::get().log(MOHPC::Log::logType_e::type, MOHPC_LOG_NAMESPACE, fmt, ##__VA_ARGS__)
#define MOHPC_LOG_FN(type, fmt, ...) MOHPC::Log::ILog::get().log(MOHPC::Log::logType_e::type, MOHPC_LOG_NAMESPACE, __FUNCTION__ ": " fmt, ##__VA_ARGS__)
	}
}
