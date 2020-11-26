#include <MOHPC/Network/IPRemoteIdentifier.h>
#include <MOHPC/Network/Socket.h>

using namespace MOHPC;
using namespace MOHPC::Network;

MOHPC_OBJECT_DEFINITION(IPRemoteIdentifier);

IPRemoteIdentifier::IPRemoteIdentifier(const NetAddrPtr& inAddress)
	: address(inAddress)
{
}

str IPRemoteIdentifier::getString() const
{
	return address ? address->asString() : str();
}

const NetAddrPtr& IPRemoteIdentifier::getAddress() const
{
	return address;
}

bool IPRemoteIdentifier::isIdentifier(const IRemoteIdentifier& other) const
{
	const IPRemoteIdentifier* ipOther = dynamic_cast<const IPRemoteIdentifier*>(&other);
	// the other identifer must be a valid IP address identifier
	return ipOther ? *address == *ipOther->address : false;
}
