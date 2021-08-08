#pragma once

#include "../../NetGlobal.h"
#include "../../pm/bg_public.h"
#include "../../Types/Snapshot.h"
#include "../../../Common/Vector.h"

#include <cstdint>

namespace MOHPC
{
class CollisionWorld;

namespace Network
{
	namespace CGame
	{
		struct EntityInfo;
		class SnapshotProcessor;

		class TraceManager
		{
		public:
			TraceManager();

			/**
			 * Trace through various entities. This function should be used in conjunction to a previous trace.
			 *
			 * @param	cm			Collision world to use.
			 * @param	start		Start trace.
			 * @param	mins		Bounding box of the trace.
			 * @param	maxs		Bounding box of the trace.
			 * @param	end			End trace.
			 * @param	skipNumber	Entity to ignore.
			 * @param	mask		Trace mask.
			 * @param	tr			Input/Output results.
			 */
			MOHPC_NET_EXPORTS void clipMoveToEntities(CollisionWorld& cm, const_vec3r_t start, const_vec3r_t mins, const_vec3r_t maxs, const_vec3r_t end, entityNum_t skipNumber, uint32_t mask, bool cylinder, trace_t& tr) const;

			/**
			 * Perform a trace from start to the end, taking entities into account.
			 *
			 * @param	cm			Collision world to use.
			 * @param	start		Start trace.
			 * @param	mins		Bounding box of the trace.
			 * @param	maxs		Bounding box of the trace.
			 * @param	end			End trace.
			 * @param	skipNumber	Entity to ignore.
			 * @param	mask		Trace mask.
			 * @param	tr			Input/Output results.
			 */
			MOHPC_NET_EXPORTS void trace(CollisionWorld& cm, trace_t& tr, const_vec3r_t start, const_vec3r_t mins, const_vec3r_t maxs, const_vec3r_t end, entityNum_t skipNumber, uint32_t mask, bool cylinder, bool cliptoentities) const;

			/**
			 * Get contents of point.
			 *
			 * @param	cm				Collision world to use.
			 * @param	point			Location to get contents from.
			 * @param	passEntityNum	Entity number to skip.
			 */
			MOHPC_NET_EXPORTS uint32_t pointContents(CollisionWorld& cm, const_vec3r_t point, uintptr_t passEntityNum) const;

			/**
			 * Build the solid list from snapshot.
			 */
			void buildSolidList(const SnapshotProcessor& snapshotProcessor);

		private:
			CollisionWorldPtr boxHull;
			size_t numSolidEntities;
			size_t numTriggerEntities;
			const EntityInfo* solidEntities[MAX_ENTITIES_IN_SNAPSHOT];
			const EntityInfo* triggerEntities[MAX_ENTITIES_IN_SNAPSHOT];
		};
	}
}
}