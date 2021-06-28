#include <MOHPC/Network/Types/Protocol.h>

using namespace MOHPC;
using namespace MOHPC::Network;

protocolType_c::protocolType_c()
	: serverType(serverType_e::normal)
	, protocolVersion(protocolVersion_e::bad)
{}

protocolType_c::protocolType_c(serverType_e inServerType, protocolVersion_e inProtocolVersion)
	: serverType(inServerType)
	, protocolVersion(inProtocolVersion)
{}

serverType_e protocolType_c::getServerType() const
{
	return serverType;
}

protocolVersion_e protocolType_c::getProtocolVersion() const
{
	return protocolVersion;
}

unsigned int protocolType_c::getProtocolVersionNumber() const
{
	return ::getProtocolVersionNumber(protocolVersion);
}
