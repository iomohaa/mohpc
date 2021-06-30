#include <MOHPC/Network/Client/CGame/TraceFunction.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

TraceWithWorld::TraceWithWorld(const CollisionWorldPtr& collisionWorld, const TraceManager& traceManager)
	: cm(collisionWorld)
	, tm(traceManager)
{
}

void TraceWithWorld::trace(trace_t* results, const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, entityNum_t passEntityNum, uint32_t contentMask, bool capsule, bool traceDeep)
{
	tm.trace(*cm, *results, start, mins, maxs, end, passEntityNum, contentMask, capsule, true);
}

uint32_t TraceWithWorld::pointContents(const Vector& point, uintptr_t passEntityNum)
{
	return tm.pointContents(*cm, point, passEntityNum);
}
