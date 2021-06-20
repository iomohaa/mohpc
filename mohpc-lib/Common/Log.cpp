#include <MOHPC/Common/Log.h>
#include <cstdlib>
#include <MOHPC/Utility/SharedPtr.h>
#include <MOHPC/Utility/WeakPtr.h>

using namespace MOHPC;
using namespace Log;

class NullLogger : public ILog
{
public:
	virtual void log(logType_e type, const char* serviceName, const char* fmt, ...) override
	{
		// Nothing
	}
};

static SharedPtr<NullLogger> nullLogger = makeShared<NullLogger>();

static WeakPtr<ILog> logger = nullLogger;

void* ILog::operator new(size_t sz)
{
	return malloc(sz);
}

void ILog::operator delete(void* ptr)
{
	return free(ptr);
}

void MOHPC::Log::ILog::set(const ILogPtr& newLogger)
{
	logger = newLogger;
}

void MOHPC::Log::ILog::reset()
{
	logger = nullLogger;
}

ILog& MOHPC::Log::ILog::get()
{
	return !logger.expired() ? *logger.lock().get() : *nullLogger.get();
}
