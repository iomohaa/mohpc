#include <MOHPC/Network/Client/Protocol.h>

using namespace MOHPC;
using namespace Network;

template<bool supported, uint32_t serverProto, uint32_t clientProto>
class ClientProtocol_Template : public IClientProtocol
{
public:
	ClientProtocol_Template(const char* versionValue) : version(versionValue), clientType(nullptr) {}
	ClientProtocol_Template(const char* versionValue, const char* clientTypeValue) : version(versionValue), clientType(clientTypeValue) {}

	uint32_t getServerProtocol() const override { return serverProto; }
	uint32_t getBestCompatibleProtocol() const override { return clientProto; }
	const char* getClientType() const override { return clientType; };
	const char* getVersion() const override { return version; };

private:
	const char* clientType;
	const char* version;
};

#define DEFINE_PROTOCOL(version, supported, serverProto, clientProto) \
ClientProtocol_Template<supported, serverProto, clientProto> ConnectVersion_Ver##version(#version);

#define DEFINE_PROTOCOL_TYPE(version, supported, serverProto, clientProto, clientType) \
ClientProtocol_Template<supported, serverProto, clientProto> ConnectVersion_Ver##version(#version, clientType);

DEFINE_PROTOCOL(005, true, 5, 5);
DEFINE_PROTOCOL(100, true, 6, 6);
DEFINE_PROTOCOL(110, true, 8, 8);
DEFINE_PROTOCOL(200, true, 15, 15);
DEFINE_PROTOCOL_TYPE(211dem, true, 16, 17, "Breakthrough");
DEFINE_PROTOCOL_TYPE(211, true, 17, 17, "Breakthrough");
