#pragma once

#include "../../NetGlobal.h"
#include "../../NetObject.h"
#include "../../Configstring.h"

#include "../../../Common/Vector.h"
#include "../../../Common/str.h"
#include "../../../Utility/SharedPtr.h"

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
			vec3_t location;

		public:
			objective_t();

		public:
			/** Objective flags. See above. */
			MOHPC_NET_EXPORTS uint32_t getFlags() const;

			/** Objective text. */
			MOHPC_NET_EXPORTS const char* getText() const;

			/** Objective location. */
			MOHPC_NET_EXPORTS const_vec3p_t getLocation() const;
		};

		class ObjectiveManager
		{
			MOHPC_NET_OBJECT_DECLARATION(ObjectiveManager);

		public:
			MOHPC_NET_EXPORTS ObjectiveManager();
			MOHPC_NET_EXPORTS ~ObjectiveManager();
			ObjectiveManager(ObjectiveManager&&) = delete;
			ObjectiveManager(const ObjectiveManager&) = delete;
			ObjectiveManager& operator=(ObjectiveManager&&) = delete;
			ObjectiveManager& operator=(const ObjectiveManager&) = delete;

			/** Get an objective in the interval of [0, MAX_OBJECTIVES]. */
			MOHPC_NET_EXPORTS const objective_t& getCurrent() const;
			MOHPC_NET_EXPORTS void setCurrent(uint32_t objNum);
			MOHPC_NET_EXPORTS const objective_t& get(uint32_t objNum) const;
			MOHPC_NET_EXPORTS const objective_t& set(const ReadOnlyInfo& info, uint32_t objNum);

		private:
			objective_t* objectives;
			uint32_t current;
		};
		using ObjectiveManagerPtr = SharedPtr<ObjectiveManager>;
	}
}
}