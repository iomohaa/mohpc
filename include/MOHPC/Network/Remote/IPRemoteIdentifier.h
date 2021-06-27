#pragma once

#include "../NetGlobal.h"
#include "../../Utility/RemoteIdentifier.h"
#include "../Types.h"

#include <MOHPC/Common/str.h>

namespace MOHPC
{
namespace Network
{
	class IPRemoteIdentifier : public IRemoteIdentifier
	{
		MOHPC_NET_OBJECT_DECLARATION(IPRemoteIdentifier);

	public:
		MOHPC_NET_EXPORTS IPRemoteIdentifier(const NetAddrPtr& inAddress);

		str getString() const override;
		const NetAddrPtr& getAddress() const;

	protected:
		bool isIdentifier(const IRemoteIdentifier& other) const override;

	private:
		NetAddrPtr address;
	};
}
}