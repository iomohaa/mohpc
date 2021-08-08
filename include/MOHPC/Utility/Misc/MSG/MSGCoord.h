#pragma once

#include "../../UtilityGlobal.h"
#include "MSG.h"
#include "../../../Common/SimpleVector.h"

namespace MOHPC
{
	class MOHPC_UTILITY_EXPORTS MsgCoordHelper : public MsgBaseHelper
	{
	public:
		MsgCoordHelper(MSG& inMsg);

		/** Read a coordinate value. */
		float ReadCoord();

		/** Read a coordinate value. */
		float ReadCoordSmall();

		/** Read a coordinate value. */
		int32_t ReadDeltaCoord(uint32_t offset);

		/** Read a coordinate value. */
		int32_t ReadDeltaCoordExtra(uint32_t offset);

		/** Read a coordinate value. */
		void ReadVectorCoord(vec3_t& out);

		/** Read a coordinate value. */
		void ReadVectorFloat(vec3_t& out);

		/** Read a coordinate value. */
		void ReadDir(vec3_t& out);

		/** Write a coordinate value. */
		void WriteCoord(float value);

		/** Write a coordinate value. */
		void WriteCoordSmall(float value);

		/** Read a coordinate value. */
		void WriteDeltaCoord(uint32_t from, uint32_t to);

		/** Read a coordinate value. */
		void WriteDeltaCoordExtra(uint32_t from, uint32_t to);

		/** Write a coordinate value. */
		void WriteVectorCoord(const_vec3r_t value);

		/** Write a coordinate value. */
		void WriteDir(const_vec3r_t dir);
	};
}