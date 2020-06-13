#include <MOHPC/Network/MasterList.h>
#include <MOHPC/Network/Types.h>
#include <MOHPC/Network/Server.h>
#include <MOHPC/Network/GameSpy/Encryption.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include <MOHPC/Utilities/Info.h>
#include <MOHPC/Log.h>
#include <MOHPC/Math.h>
#include <vector>

using namespace MOHPC;
using namespace Network;

#define MOHPC_LOG_NAMESPACE "masterlist"

static const uint8_t gameKeys[gameListType_e::max][7] =
{
	// MOH:AA
	{ 'M', '5', 'F', 'd', 'w', 'c' },
	// MOH:SH
	{ 'h', '2', 'P', '1', 'c', '9' },
	// MOH:BT
	{ 'y', '3', '2', 'F', 'D', 'c' }
};

static const char* gameName[gameListType_e::max] =
{
	"mohaa",
	"mohaas",
	"mohaab"
};

MOHPC::Network::IServerList::IServerList(NetworkManager* inManager)
	: ITickableNetwork(inManager)
{
}

Network::ServerList::ServerList(NetworkManager* inManager, gameListType_e type)
	: IServerList(inManager)
	, gameType(type)
{
	netadr_t addr = ISocketFactory::get()->getHost("master.x-null.net");
	addr.port = 28900;
	socket = ISocketFactory::get()->createTcp(addressType_e::IPv4, addr);

	sendRequest(makeShared<Request_SendCon>(inManager, type));
}

void Network::ServerList::fetch(FoundServerCallback&& callback, MasterServerDone&& doneCallback)
{
	sendRequest(makeShared<Request_FetchServers>(getManager(), gameType, std::move(callback), std::move(doneCallback)));
}

void Network::ServerList::tick(uint64_t deltaTime, uint64_t currentTime)
{
	handler.handle();
}

void Network::ServerList::sendRequest(IGamespyRequestPtr&& newRequest)
{
	GamespyRequestParam param(socket);
	handler.sendRequest(std::move(newRequest), std::move(param), 10000);
}

//===================
//= SendCon         =
//===================
Network::ServerList::Request_SendCon::Request_SendCon(NetworkManager* inNetworkManager, gameListType_e inType)
	: networkManager(inNetworkManager)
	, gameType(inType)
{
}

SharedPtr<IRequestBase> Network::ServerList::Request_SendCon::process(RequestData& data)
{
	char dataStr[256];

	const size_t len = data.stream.GetLength();
	data.stream.Read(dataStr, len);
	dataStr[len] = 0;

	char* ptr = strstr(dataStr, "\\secure\\");
	if (!ptr) {
		return nullptr;
	}

	const char* token = ptr + 8;

	MOHPC_LOG(Verbose, "connected to master and got token %s", token);

	// Return the token to server
	return makeShared<Request_SendToken>(networkManager, token, gameType);
}

void Network::ServerList::Request_SendCon::generateInfo(Info& info)
{
}

//===================
//= SendToken       =
//===================
Network::ServerList::Request_SendToken::Request_SendToken(NetworkManager* inNetworkManager, const char* challenge, gameListType_e inType)
	: networkManager(inNetworkManager)
	, gameType(inType)
{
	char encdata[6];
	memcpy(encdata, challenge, sizeof(encdata));

	const uint8_t* key = gameKeys[(uint8_t)gameType];
	// Take the challenge and encrypt it with the key
	encrypt(key, 6, (uint8_t*)encdata, 6);

	// XOR the data
	//for (size_t i = 0; i < 6; ++i) {
	//	encdata[i] ^= key[i];
	//}

	// Encode the data
	encode((uint8_t*)encdata, 6, (uint8_t*)encoded);

	MOHPC_LOG(Verbose, "sending back encrypted token %s", encoded);
}

void Network::ServerList::Request_SendToken::generateInfo(Info& info)
{
	info.SetValueForKey("gamename", gameName[(uint8_t)gameType]);
	info.SetValueForKey("gamever", "3");
	info.SetValueForKey("location", "0");
	info.SetValueForKey("validate", encoded);

	// Encrypted data
	//info.SetValueForKey("enctype", "2");
}

bool Network::ServerList::Request_SendToken::mustProcess() const
{
	return false;
}

const char* Network::ServerList::Request_SendToken::queryId() const
{
	return "1.1";
}

SharedPtr<IRequestBase> Network::ServerList::Request_SendToken::process(RequestData& data)
{
	return nullptr;
}

void ServerList::Request_SendToken::swapByte(uint8_t* a, uint8_t* b)
{
	uint8_t swapByte;

	swapByte = *a;
	*a = *b;
	*b = swapByte;
}

uint8_t ServerList::Request_SendToken::encodeChar(uint8_t c)
{
	if (c < 26) return (uint8_t)('A' + c);
	if (c < 52) return (uint8_t)('a' + c - 26);
	if (c < 62) return (uint8_t)('0' + c - 52);
	if (c == 62) return (uint8_t)('+');
	if (c == 63) return (uint8_t)('/');

	return 0;
}

void ServerList::Request_SendToken::encode(uint8_t* ins, int size, uint8_t* result)
{
	int    i, pos;
	uint8_t  trip[3];
	uint8_t  kwart[4];

	i = 0;
	while (i < size)
	{
		for (pos = 0; pos <= 2; pos++, i++)
			if (i < size) trip[pos] = *ins++;
			else trip[pos] = '\0';
		kwart[0] = (unsigned char)((trip[0]) >> 2);
		kwart[1] = (unsigned char)((((trip[0]) & 3) << 4) + ((trip[1]) >> 4));
		kwart[2] = (unsigned char)((((trip[1]) & 15) << 2) + ((trip[2]) >> 6));
		kwart[3] = (unsigned char)((trip[2]) & 63);
		for (pos = 0; pos <= 3; pos++) *result++ = encodeChar(kwart[pos]);
	}
	*result = '\0';
}

void ServerList::Request_SendToken::encrypt(const uint8_t* key, int key_len, uint8_t* buffer_ptr, int buffer_len)
{
	short counter;
	uint8_t x, y, xorIndex;
	uint8_t state[256];

	for (counter = 0; counter < 256; counter++) state[counter] = (uint8_t)counter;

	x = 0; y = 0;
	for (counter = 0; counter < 256; counter++)
	{
		y = (uint8_t)((key[x] + state[counter] + y) % 256);
		x = (uint8_t)((x + 1) % key_len);
		swapByte(&state[counter], &state[y]);
	}

	x = 0; y = 0;
	for (counter = 0; counter < buffer_len; counter++)
	{
		x = (uint8_t)((x + buffer_ptr[counter] + 1) % 256);
		y = (uint8_t)((state[x] + y) % 256);
		swapByte(&state[x], &state[y]);
		xorIndex = (uint8_t)((state[x] + state[y]) % 256);
		buffer_ptr[counter] ^= state[xorIndex];
	}
}

//===================
//= FetchServers    =
//===================
Network::ServerList::Request_FetchServers::Request_FetchServers(NetworkManager* inNetworkManager, gameListType_e inGameType, FoundServerCallback&& inCallback, MasterServerDone&& inDoneCallback)
	: networkManager(inNetworkManager)
	, key(gameKeys[(uint8_t)inGameType])
	, game(gameName[(uint8_t)inGameType])
	, callback(std::move(inCallback))
	, doneCallback(std::move(inDoneCallback))
	, pendingLen(0)
{
	if (!callback)
	{
		using namespace std::placeholders;
		callback = std::bind(&Request_FetchServers::nullCallback, this, _1);

		MOHPC_LOG(Warning, "No callback was given for fetching servers! Fallback to a null callback");
	}
}

void Network::ServerList::Request_FetchServers::generateInfo(Info& info)
{
	info.SetValueForKey("list", "cmp");
	info.SetValueForKey("gamename", game);
	info.SetValueForKey("where", "");
}

SharedPtr<IRequestBase> Network::ServerList::Request_FetchServers::process(RequestData& data)
{
	char dataStr[512 + sizeof(pendingData)];

	memcpy(dataStr, pendingData, pendingLen);

	const size_t len = data.stream.GetLength();
	data.stream.Read(dataStr + pendingLen, len - pendingLen);

	//EncryptionLevel2 enc;
	//enc.decode(key, (unsigned char*)dataStr + pendingLen, len - pendingLen);

	const size_t totalLen = len + pendingLen;
	pendingLen = totalLen % 6;
	const size_t maxlen = len - pendingLen;

	const char* p = dataStr;
	const char* endbuf = dataStr + maxlen;
	while (p < endbuf)
	{
		if (!strncmp(p, "\\final\\", 7))
		{
			// finished processing
			if (doneCallback) doneCallback();
			return nullptr;
		}

		const uint8_t ip[4] = { (uint8_t)*p++, (uint8_t)*p++, (uint8_t)*p++, (uint8_t)*p++ };
		const uint16_t port = rotl(*(uint16_t*)p, 8);
		p += sizeof(port);

		netadr_t adr;
		memcpy(adr.ip, ip, sizeof(adr.ip));
		adr.port = port;
	
		IServerPtr ptr = makeShared<GSServer>(networkManager, adr);
		callback(ptr);
	}

	memcpy(pendingData, p, pendingLen);

	return shared_from_this();
}

void Network::ServerList::Request_FetchServers::nullCallback(const IServerPtr& server)
{
	const netadr_t& address = server->getAddress();

	MOHPC_LOG(VeryVerbose, "Request_FetchServers::nullCallback: found %d.%d.%d.%d:%d", address.ip[0], address.ip[1], address.ip[2], address.ip[3], address.port);
}

ServerListLAN::ServerListLAN(NetworkManager* inManager)
	: IServerList(inManager)
{
	socket = ISocketFactory::get()->createUdp(addressType_e::IPv4);
}

void ServerListLAN::fetch(FoundServerCallback&& callback, MasterServerDone&& doneCallback)
{
	netadr_t to;

	GamespyUDPBroadcastRequestParam param(socket, 12203, 12218);
	handler.sendRequest(makeShared<Request_InfoBroadcast>(getManager(), std::forward<FoundServerCallback>(callback)), std::move(param), 1000);
}

void ServerListLAN::tick(uint64_t deltaTime, uint64_t currentTime)
{
	handler.handle();
}

Network::ServerListLAN::Request_InfoBroadcast::Request_InfoBroadcast(NetworkManager* inNetworkManager, FoundServerCallback&& inResponse)
	: networkManager(inNetworkManager)
	, response(inResponse)
{

}

void Network::ServerListLAN::Request_InfoBroadcast::generateOutput(IMessageStream& output)
{
	MSG msg(output, msgMode_e::Writing);
	msg.SetCodec(MessageCodecs::OOB);

	msg.WriteUInteger(~0);
	msg.WriteByte(2);
	msg.WriteString("getInfo xxx");
}

SharedPtr<IRequestBase> Network::ServerListLAN::Request_InfoBroadcast::process(RequestData& data)
{
	const size_t len = data.stream.GetLength();

	char* dataStr = new char[len + 1];
	data.stream.Read(dataStr, len);
	dataStr[len] = 0;

	const GamespyUDPBroadcastRequestParam& param = data.getParam<GamespyUDPBroadcastRequestParam>();
	netadr_t addr = param.getLastIp();

	// Don't create a server without a callback
	if (response)
	{
		IServerPtr lanServer = makeShared<LANServer>(networkManager, addr, dataStr, strlen(dataStr));
		response(lanServer);
	}

	// Always return self so slow servers can return in time (before timeout)
	return shared_from_this();
}

MOHPC::SharedPtr<MOHPC::IRequestBase> MOHPC::Network::ServerList::Request_FetchServers::timedOut()
{
	if (doneCallback) doneCallback();
	return nullptr;
}
