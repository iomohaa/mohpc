#pragma once

#include "../Common/str.h"
#include "RemoteIdentifier.h"
#include "../Object.h"

#include <cstdint>

namespace MOHPC
{
	/**
	 * Empty identifier.
	 */
	class NullRemoteIdentifier : public IRemoteIdentifier
	{
		MOHPC_OBJECT_DECLARATION(NullRemoteIdentifier);
	
	public:
		str getString() const override;

	protected:
		bool isIdentifier(const IRemoteIdentifier& other) const override;
	};
}