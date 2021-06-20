#pragma once

#include "SharedPtr.h"
#include <MOHPC/Common/str.h>

namespace MOHPC
{
	/**
	 * Interface used to identify a recipient.
	 */
	class IRemoteIdentifier
	{
	public:
		virtual ~IRemoteIdentifier() = default;

		/** Determine whether or not this identifier is the other identifier specified. */
		bool operator==(const IRemoteIdentifier& other) const;
		bool operator!=(const IRemoteIdentifier& other) const;

		/** Return the identifier as string. */
		virtual str getString() const = 0;

	protected:
		virtual bool isIdentifier(const IRemoteIdentifier& other) const = 0;
	};
	using IRemoteIdentifierPtr = SharedPtr<IRemoteIdentifier>;
}