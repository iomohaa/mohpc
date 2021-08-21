#include <MOHPC/Network/Client/CGame/TraceFunction.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(TraceWithWorld);
TraceWithWorld::TraceWithWorld(const CollisionWorldPtr& collisionWorld, const TraceManager& traceManager)
	: cm(collisionWorld)
	, tm(traceManager)
{
}

TraceWithWorld::~TraceWithWorld()
{
}

void TraceWithWorld::trace(trace_t* results, const_vec3r_t start, const_vec3r_t mins, const_vec3r_t maxs, const_vec3r_t end, entityNum_t passEntityNum, uint32_t contentMask, bool capsule, bool traceDeep)
{
	tm.trace(*cm, *results, start, mins, maxs, end, passEntityNum, contentMask, capsule, true);
}

uint32_t TraceWithWorld::pointContents(const_vec3r_t point, uintptr_t passEntityNum)
{
	return tm.pointContents(*cm, point, passEntityNum);
}
