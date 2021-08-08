#include <MOHPC/Network/pm/bg_trace.h>

using namespace MOHPC;
using namespace MOHPC::Network;

class NoDelete
{
public:
	void operator()(ITraceFunction* func)
	{
		// Do not delete the pointer
	}
};

class StubTrace : public ITraceFunction
{
public:
	void trace(
		trace_t* results,
		const_vec3r_t start,
		const_vec3r_t mins,
		const_vec3r_t maxs,
		const_vec3r_t end,
		entityNum_t passEntityNum,
		uint32_t contentMask,
		bool capsule,
		bool traceDeep
	) override
	{
		memset(results, 0, sizeof(trace_t));

		VecCopy(end, results->endpos);
		results->fraction = 1.f;
	}

	uint32_t pointContents(const_vec3r_t point, uintptr_t passEntityNum) override
	{
		return 0;
	}
};
static StubTrace stubTrace;
ITraceFunction* Network::PmoveNoTrace = &stubTrace;
ITraceFunctionPtr Network::PmoveNoTracePtr(static_cast<ITraceFunction*>(&stubTrace), NoDelete());

ITraceFunction::~ITraceFunction()
{
}
