#pragma once

#include "../../NetGlobal.h"
#include "../../NetObject.h"

#include "Trace.h"
#include "../../../Utility/Collision/Collision.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		class TraceWithWorld : public ITraceFunction
		{
			MOHPC_NET_OBJECT_DECLARATION(TraceWithWorld);

		public:
			/**
			 * Initialize the trace function interface.
			 *
			 * @param collisionWorld The world collision to use.
			 * @param traceManager The trace manager, used to trace through world + entities.
			 */
			TraceWithWorld(const CollisionWorldPtr& collisionWorld, const TraceManager& traceManager);

			void trace(
				trace_t* results,
				const Vector& start,
				const Vector& mins,
				const Vector& maxs,
				const Vector& end,
				entityNum_t passEntityNum,
				uint32_t contentMask,
				bool capsule,
				bool traceDeep
			) override;

			uint32_t pointContents(const Vector& point, uintptr_t passEntityNum) override;

		private:
			CollisionWorldPtr cm;
			const TraceManager& tm;
		};
	}
}
}