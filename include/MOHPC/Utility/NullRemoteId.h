#pragma once

#include "RemoteIdentifier.h"
#include "UtilityObject.h"

#include "../Common/str.h"

#include <cstdint>

namespace MOHPC
{
	/**
	 * Empty identifier.
	 */
	class NullRemoteIdentifier : public IRemoteIdentifier
	{
		MOHPC_UTILITY_OBJECT_DECLARATION(NullRemoteIdentifier);
	
	public:
		str getString() const override;

	protected:
		bool isIdentifier(const IRemoteIdentifier& other) const override;
	};
}