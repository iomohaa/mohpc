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
			MOHPC_NET_EXPORTS TraceWithWorld(const CollisionWorldPtr& collisionWorld, const TraceManager& traceManager);
			MOHPC_NET_EXPORTS ~TraceWithWorld();

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
			) override;

			uint32_t pointContents(const_vec3r_t point, uintptr_t passEntityNum) override;

		private:
			CollisionWorldPtr cm;
			const TraceManager& tm;
		};
		using TraceWithWorldPtr = SharedPtr<TraceWithWorld>;
	}
}
}