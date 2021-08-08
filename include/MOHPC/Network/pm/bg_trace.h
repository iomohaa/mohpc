#pragma once

#include "../NetGlobal.h"
#include "../../Utility/Function.h"
#include "../../Utility/Collision/Collision.h"
#include "../../Common/Vector.h"
#include "../Types/Entity.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	using TraceFunction = Function<void(trace_t* results, const_vec3r_t start, const_vec3r_t mins, const_vec3r_t maxs, const_vec3r_t end, uintptr_t passEntityNum, uintptr_t contentMask, bool capsule, bool traceDeep)>;
	using PointContentsFunction = Function<uint32_t(const_vec3r_t point, uintptr_t passEntityNum)>;

	class MOHPC_NET_EXPORTS ITraceFunction
	{
	public:
		virtual ~ITraceFunction();

		/** @see CollisionWorld::BoxTrace. */
		virtual void trace(
			trace_t* results,
			const_vec3r_t start,
			const_vec3r_t mins,
			const_vec3r_t maxs,
			const_vec3r_t end,
			entityNum_t passEntityNum,
			uint32_t contentMask,
			bool capsule,
			bool traceDeep
		) = 0;

		/** @see CollisionWorld::PointContents. */
		virtual uint32_t pointContents(const_vec3r_t point, uintptr_t passEntityNum) = 0;
	};
	using ITraceFunctionPtr = SharedPtr<ITraceFunction>;

	extern MOHPC_NET_EXPORTS ITraceFunction* PmoveNoTrace;
	extern MOHPC_NET_EXPORTS ITraceFunctionPtr PmoveNoTracePtr;
}
}