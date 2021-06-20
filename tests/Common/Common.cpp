#include "Common.h"

MOHPC::Log::ILogPtr logPtr;

void InitCommon()
{
	using namespace MOHPC::Log;
	logPtr = MOHPC::makeShared<Logger>();
	ILog::set(logPtr);
}
