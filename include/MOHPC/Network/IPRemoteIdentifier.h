#pragma once

#include "../common/str.h"
#include "../Utilities/RemoteIdentifier.h"
#include "Types.h"

namespace MOHPC
{
namespace Network
{
	class IPRemoteIdentifier : public IRemoteIdentifier
	{
		MOHPC_OBJECT_DECLARATION(IPRemoteIdentifier);

	public:
		MOHPC_EXPORTS IPRemoteIdentifier(const NetAddrPtr& inAddress);

		str getString() const override;
		const NetAddrPtr& getAddress() const;

	protected:
		bool isIdentifier(const IRemoteIdentifier& other) const override;

	private:
		NetAddrPtr address;
	};
}
}