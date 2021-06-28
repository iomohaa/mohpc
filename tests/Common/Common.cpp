#include "Common.h"

#include <MOHPC/Version.h>

using namespace MOHPC;

constexpr char MOHPC_LOG_NAMESPACE[] = "common";

MOHPC::Log::ILogPtr logPtr;

void InitCommon()
{
	using namespace MOHPC::Log;
	logPtr = MOHPC::makeShared<Logger>();
	ILog::set(logPtr);

	MOHPC_LOG(Info, "MOHPC %s %s", VERSION_STRING, VERSION_ARCHITECTURE);
}
