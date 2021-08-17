#pragma once

#include "../NetGlobal.h"
#include "../../Utility/SharedPtr.h"

#include <cstdint>

namespace MOHPC
{
class MSG;

namespace Network
{
	class ClientTime;

	class IUserInputModule
	{
	public:
		virtual ~IUserInputModule();

		virtual void process(const ClientTime& time) = 0;
		virtual bool canWrite() const = 0;
		virtual void write(MSG& msg, uint32_t key, bool deltaMove) = 0;
	};
	using IUserInputModulePtr = SharedPtr<IUserInputModule>;
}
}
