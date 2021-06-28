#include <MOHPC/Network/Client/MasterList.h>
#include <MOHPC/Network/Client/Server.h>
#include <MOHPC/Network/GameSpy/Encryption.h>
#include <MOHPC/Network/Remote/UDPMessageDispatcher.h>
#include <MOHPC/Utility/Misc/MSG/Stream.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Misc/MSG/Codec.h>
#include <MOHPC/Utility/Info.h>
#include <MOHPC/Common/Log.h>
#include <MOHPC/Common/Math.h>
#include <vector>

using namespace MOHPC;
using namespace Network;

#define MOHPC_LOG_NAMESPACE "masterlist"

static const uint8_t gameKeys[(size_t)gameListType_e::max][7] =
{
	// MOH:AA
	{ 'M', '5', 'F', 'd', 'w', 'c' },
	// MOH:SH
	{ 'h', '2', 'P', '1', 'c', '9' },
	// MOH:BT
	{ 'y', '3', '2', 'F', 'D', 'c' }
};

static const char* gameName[(size_t)gameListType_e::max] =
{
	"mohaa",
	"mohaas",
	"mohaab"
};

IServerList::IServerList()
{
}

MOHPC_OBJECT_DEFINITION(ServerList);

ServerList::ServerList(const MessageDispatcherPtr& inDispatcher, const ICommunicatorPtr& inMasterComm, const ICommunicatorPtr& inComm, const IRemoteIdentifierPtr& masterId, gameListType_e type)
	: comm(inComm)
	, dispatcher(inDispatcher)
	, handler(inDispatcher.get(), inMasterComm, masterId)
	, gameType(type)
{
	//NetAddr4 addr = ISocketFactory::get()->getHost("master.x-null.net");
	//addr.port = 28900;
	//socket = ISocketFactory::get()->createTcp(addr);

	sendRequest(makeShared<Request_SendCon>(type));
}

void ServerList::fetch(FoundServerCallback&& callback)
{
	sendRequest(makeShared<Request_FetchServers>(dispatcher, comm, gameType, std::move(callback)));
}

void ServerList::sendRequest(IGamespyRequestPtr&& newRequest)
{
	handler.sendRequest(std::move(newRequest), 10000);
}

//===================
//= SendCon         =
//===================
ServerList::Request_SendCon::Request_SendCon(gameListType_e inType)
	: gameType(inType)
{
}

SharedPtr<IRequestBase> ServerList::Request_SendCon::process(InputRequest& data)
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

	MOHPC_LOG(Debug, "connected to master and got token %s", token);

	// Return the token to server
	return makeShared<Request_SendToken>(token, gameType);
}

void ServerList::Request_SendCon::generateInfo(Info& info)
{
	// Request will be sent after TCP acknowledge
}

//===================
//= SendToken       =
//===================
ServerList::Request_SendToken::Request_SendToken(const char* challenge, gameListType_e inType)
	: gameType(inType)
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

	MOHPC_LOG(Debug, "sending back encrypted token %s", encoded);
}

void ServerList::Request_SendToken::generateInfo(Info& info)
{
	info.SetValueForKey("gamename", gameName[(uint8_t)gameType]);
	info.SetValueForKey("gamever", "3");
	info.SetValueForKey("location", "0");
	info.SetValueForKey("validate", encoded);

	// Encrypted data
	//info.SetValueForKey("enctype", "2");
}

bool ServerList::Request_SendToken::mustProcess() const
{
	return false;
}

const char* ServerList::Request_SendToken::queryId() const
{
	return "1.1";
}

SharedPtr<IRequestBase> ServerList::Request_SendToken::process(InputRequest& data)
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
ServerList::Request_FetchServers::Request_FetchServers(const MessageDispatcherPtr& inDispatcher, const ICommunicatorPtr& inComm, gameListType_e inGameType, FoundServerCallback&& inCallback)
	: dispatcher(inDispatcher)
	, comm(inComm)
	, key(gameKeys[(uint8_t)inGameType])
	, game(gameName[(uint8_t)inGameType])
	, callback(std::move(inCallback))
	, pendingLen(0)
{
	if (!callback)
	{
		using namespace std::placeholders;
		callback = std::bind(&Request_FetchServers::nullCallback, this, _1);

		MOHPC_LOG(Warn, "No callback was given for fetching servers! Fallback to a null callback");
	}
}

void ServerList::Request_FetchServers::generateInfo(Info& info)
{
	info.SetValueForKey("list", "cmp");
	info.SetValueForKey("gamename", game);
	info.SetValueForKey("where", "");
}

SharedPtr<IRequestBase> ServerList::Request_FetchServers::process(InputRequest& data)
{
	char dataStr[sizeof(pendingData) * 100];

	// copy pending data
	memcpy(dataStr, pendingData, pendingLen);

	size_t processedLen = 0;
	size_t len = 0;
	const size_t streamLen = data.stream.GetLength();
	for(size_t processedLen = 0; processedLen < streamLen; processedLen += len)
	{
		const size_t remainingLen = streamLen - processedLen;
		len = std::min(remainingLen, sizeof(dataStr));
		data.stream.Read(dataStr + pendingLen, len);

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
				return nullptr;
			}

			const uint8_t ip[4] = { (uint8_t)*p++, (uint8_t)*p++, (uint8_t)*p++, (uint8_t)*p++ };
			const uint16_t port = rotl(*(uint16_t*)p, 8);
			p += sizeof(port);

			// the master server only return IPv4 entries
			// for now, only use netadr4_t
			NetAddr4Ptr adr = makeShared<NetAddr4>();
			const IRemoteIdentifierPtr identifier = makeShared<IPRemoteIdentifier>(adr);
			memcpy(adr->ip, ip, sizeof(adr->ip));
			adr->port = port;
	
			IServerPtr ptr = makeShared<GSServer>(dispatcher, comm, identifier);
			callback(ptr);
		}

		memcpy(pendingData, p, pendingLen);
	}

	return shared_from_this();
}

void ServerList::Request_FetchServers::nullCallback(const IServerPtr& server)
{
	const IRemoteIdentifierPtr& identifier = server->getIdentifier();

	MOHPC_LOG(Trace, "Request_FetchServers::nullCallback: found %s", identifier->getString().c_str());
}

MOHPC_OBJECT_DEFINITION(ServerListLAN);

ServerListLAN::ServerListLAN(const MessageDispatcherPtr& dispatcher, const ICommunicatorPtr& comm, uint64_t timeoutTimeValue)
	: handler(dispatcher.get(), comm, nullptr)
	, timeoutTime(timeoutTimeValue)
{
}

void ServerListLAN::fetch(FoundServerCallback&& callback)
{
	handler.sendRequest(makeShared<Request_InfoBroadcast>(std::forward<FoundServerCallback>(callback)), timeoutTime);
}
/*
void ServerListLAN::tick(uint64_t deltaTime, uint64_t currentTime)
{
	handler.handle();
}
*/

ServerListLAN::Request_InfoBroadcast::Request_InfoBroadcast(FoundServerCallback&& inResponse)
	: response(inResponse)
{
}

void ServerListLAN::Request_InfoBroadcast::generateOutput(IMessageStream& output)
{
	MSG msg(output, msgMode_e::Writing);
	msg.SetCodec(MessageCodecs::OOB);

	msg.WriteUInteger(~0);
	msg.WriteByte(2);
	msg.WriteString("getInfo xxx");
}

SharedPtr<IRequestBase> ServerListLAN::Request_InfoBroadcast::process(InputRequest& data)
{
	const size_t len = data.stream.GetLength();

	char* dataStr = new char[len + 1];
	data.stream.Read(dataStr, len);
	dataStr[len] = 0;

	//const GamespyUDPBroadcastRequestParam& param = data.getParam<GamespyUDPBroadcastRequestParam>();
	//const NetAddrPtr& addr = param.getLastIp();

	// Don't create a server without a callback
	if (response)
	{
		IServerPtr lanServer = makeShared<LANServer>(data.identifier, dataStr, strlen(dataStr));
		response(lanServer);
	}

	// Always return self so slow servers can return in time (before timeout)
	return shared_from_this();
}

SharedPtr<IRequestBase> ServerList::Request_FetchServers::timedOut()
{
	return nullptr;
}

bool ServerListLAN::Request_InfoBroadcast::isThisRequest(InputRequest& data) const
{
	MSG msg(data.stream, msgMode_e::Reading);
	msg.SetCodec(MessageCodecs::OOB);

	const uint32_t marker = msg.ReadUInteger();
	if (marker != -1)
	{
		// must be a connectionless packet
		return false;
	}

	const netsrc_e dirByte = (netsrc_e)msg.ReadByte();
	if (dirByte != netsrc_e::Client)
	{
		// this must be targeted at us, the client
		return false;
	}

	StringMessage arg = msg.ReadString();
	const size_t len = strlen(arg);
	if (len <= 0)
	{
		// invalid length
		return false;
	}

	TokenParser parser;
	parser.Parse(arg, strlen(arg) + 1);

	const char* command = parser.GetToken(false);
	return !str::icmp(command, "infoResponse");
}
