#pragma once

#include "../../NetGlobal.h"
#include "../../Configstring.h"

#include "../../../Common/Vector.h"
#include "../../../Common/str.h"

#include <cstdint>

namespace MOHPC
{
class ReadOnlyInfo;

namespace Network
{
	namespace CGame
	{
		struct objective_t
		{
		public:
			// List of valid flags

			/** The objective shouldn't be drawn. */
			static constexpr unsigned int FLAGS_DONTDRAW = (1 << 0);

			/** The objective is currently in progress. */
			static constexpr unsigned int FLAGS_INPROGRESS = (1 << 1);

			/** The objective had been completed. */
			static constexpr unsigned int FLAGS_COMPLETED = (1 << 2);

		public:
			uint32_t flags;
			str text;
			Vector location;

		public:
			objective_t();

		public:
			/** Objective flags. See above. */
			MOHPC_NET_EXPORTS uint32_t getFlags() const;

			/** Objective text. */
			MOHPC_NET_EXPORTS const char* getText() const;

			/** Objective location. */
			MOHPC_NET_EXPORTS const Vector& getLocation() const;
		};

		class ObjectiveManager
		{
		public:
			/** Get an objective in the interval of [0, MAX_OBJECTIVES]. */
			MOHPC_NET_EXPORTS const objective_t& get(uint32_t objNum) const;
			const objective_t& set(const ReadOnlyInfo& info, uint32_t objNum);

		private:
			objective_t objectives[MAX_OBJECTIVES];
		};
	}
}
}