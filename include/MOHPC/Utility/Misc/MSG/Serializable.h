#pragma once

#include "../../UtilityGlobal.h"
#include "../../../Common/Vector.h"

#include <stdint.h>
#include <exception>

namespace MOHPC
{
	class MSG;

	class MOHPC_UTILITY_EXPORTS ISerializableMessage
	{
	public:
		virtual ~ISerializableMessage() {};
		virtual void Save(MSG& msg) const {};
		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from) const {};
		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) const {};
		virtual void Load(MSG& msg) {};
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from) {};
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) {};
	};
};
