#include <MOHPC/Network/Client/RemoteConsole.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include <MOHPC/Utilities/TokenParser.h>
#include <MOHPC/Log.h>

using namespace MOHPC;
using namespace MOHPC::Network;

#define MOHPC_LOG_NAMESPACE "rcon"

MOHPC_OBJECT_DEFINITION(RemoteConsole);

constexpr unsigned long MAX_RCON_RECV_PACKETS = 10;

RemoteConsole::RemoteConsole(const NetworkManagerPtr& networkManager, const NetAddrPtr& inAddress, const char* inPassword)
	: ITickableNetwork(networkManager)
	, password(inPassword)
	, address(inAddress)
{
	socket = ISocketFactory::get()->createUdp();
}

MOHPC::Network::RemoteConsole::~RemoteConsole()
{
}

void MOHPC::Network::RemoteConsole::tick(uint64_t deltaTime, uint64_t currentTime)
{
	size_t numPackets = 0;

	while (socket->dataAvailable() && numPackets++ < MAX_RCON_RECV_PACKETS)
	{
		uint8_t buf[MAX_UDP_DATA_SIZE];

		NetAddrPtr from;
		const size_t len = socket->receive(buf, sizeof(buf), from);
		if (len == -1) {
			break;
		}

		if (from != address) {
			continue;
		}

		buf[len] = 0;

		FixedDataMessageStream stream(buf, sizeof(buf));
		MSG msg(stream, msgMode_e::Reading);
		// RCon messages are using OOB codec
		msg.SetCodec(MessageCodecs::OOB);

		const uint32_t sequenceNum = msg.ReadUInteger();
		if (sequenceNum != -1)
		{
			// Only connectionless packets are accepted
			continue;
		}

		const uint8_t dir = msg.ReadByte();

		const StringMessage cmd = msg.ReadString();

		TokenParser parser;
		parser.Parse(cmd, strlen(cmd));

		const char* token = parser.GetToken(false);
		if (!str::icmp(token, "print"))
		{
			// Print text
			const char* text = parser.GetCurrentScript();
			handlerList.notify<RConHandlers::Print>(text);
		}
		else {
			MOHPC_LOG(Warning, "Unexpected rcon result command: \"%s\" (arguments \"%s\")", token, parser.GetCurrentScript());
		}
	}
}

RemoteConsole::RConHandlerList& RemoteConsole::getHandlerList()
{
	return handlerList;
}

void RemoteConsole::send(const char* command)
{
	uint8_t buf[MAX_UDP_DATA_SIZE];
	// Bind a stream to the buffer
	FixedDataMessageStream stream(buf, sizeof(buf));
	// Create a message using this stream
	MSG msg(stream, msgMode_e::Writing);

	// Write in OOB mode
	msg.SetCodec(MessageCodecs::OOB);

	// Sequence number (-1 for connectionless)
	msg.WriteUInteger(-1);

	// Direction (to server)
	msg.WriteByte(2);

	str rconStr = str::printf("rcon %s %s", password.c_str(), command);
	// Write the string
	msg.WriteString(rconStr.c_str());

	// Write all pending data
	msg.Flush();

	// Send the buffer data
	socket->send(*address, buf, stream.GetPosition());
}
