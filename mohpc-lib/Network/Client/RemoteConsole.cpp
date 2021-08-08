#include <MOHPC/Network/Client/RemoteConsole.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Misc/MSG/Stream.h>
#include <MOHPC/Utility/Misc/MSG/Codec.h>
#include <MOHPC/Utility/TokenParser.h>
#include <MOHPC/Common/Log.h>

using namespace MOHPC;
using namespace MOHPC::Network;

#define MOHPC_LOG_NAMESPACE "rcon"

MOHPC_OBJECT_DEFINITION(RemoteConsole);

constexpr unsigned long MAX_RCON_RECV_PACKETS = 10;

RemoteConsole::RemoteConsole(const MessageDispatcherPtr& dispatcher, const ICommunicatorPtr& comm, const IRemoteIdentifierPtr& remoteIdentifier, const char* inPassword)
	: password(inPassword)
	, handler(dispatcher.get(), comm, remoteIdentifier)
{
}

MOHPC::Network::RemoteConsole::~RemoteConsole()
{
}

/*
void MOHPC::Network::RemoteConsole::tick(uint64_t deltaTime, uint64_t currentTime)
{
	size_t numPackets = 0;

	while (socket->dataCount() && numPackets++ < MAX_RCON_RECV_PACKETS)
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

		const stringMessage cmd = msg.ReadString();

		TokenParser parser;
		parser.Parse(cmd, strlen(cmd));

		const char* token = parser.GetToken(false);
		if (!str::icmp(token, "print"))
		{
			// Print text
			const char* text = parser.GetCurrentScript();
			//handlerList.notify<RConHandlers::Print>(text);
			handlerList.printHandler.broadcast(text);
		}
		else {
			MOHPC_LOG(Warn, "Unexpected rcon result command: \"%s\" (arguments \"%s\")", token, parser.GetCurrentScript());
		}
	}
}
*/

RemoteConsole::RConHandlerList& RemoteConsole::getHandlerList()
{
	return handlerList;
}

void RemoteConsole::send(const char* command, uint64_t timeoutValue)
{
	handler.sendRequest(makeShared<RConMessageRequest>(handlerList, command, password.c_str()), timeoutValue);
}

RemoteConsole::RConMessageRequest::RConMessageRequest(const RConHandlerList& inHandlerList, const char* inCommand, const char* inPassword)
	: handlerList(inHandlerList)
	, command(inCommand)
	, password(inPassword)
{
}

void RemoteConsole::RConMessageRequest::generateOutput(IMessageStream& output)
{
	// Create a message using this stream
	MSG msg(output, msgMode_e::Writing);

	// Write in OOB mode
	msg.SetCodec(MessageCodecs::OOB);

	// Sequence number (-1 for connectionless)
	msg.WriteUInteger(-1);

	// Direction (to server)
	msg.WriteByte(2);

	str rconStr = str("rcon ") + password + str(" ") + command;
	// Write the string
	msg.WriteString(rconStr.c_str());

	// Write all pending data
	msg.Flush();
}

SharedPtr<MOHPC::IRequestBase> RemoteConsole::RConMessageRequest::process(InputRequest& data)
{
	MSG msg(data.stream, msgMode_e::Reading);
	// RCon messages are using OOB codec
	msg.SetCodec(MessageCodecs::OOB);

	const uint32_t sequenceNum = msg.ReadUInteger();
	if (sequenceNum != -1)
	{
		// Only connectionless packets are accepted
		return nullptr;
	}

	const uint8_t dir = msg.ReadByte();

	const StringMessage cmd = msg.ReadString();

	TokenParser parser;
	parser.Parse(cmd, strlen(cmd));

	const char* token = parser.GetToken(false);
	if (!strHelpers::icmp(token, "print"))
	{
		// Print text
		const char* text = parser.GetCurrentScript();
		//handlerList.notify<RConHandlers::Print>(text);
		handlerList.printHandler.broadcast(text);
	}
	else {
		MOHPC_LOG(Warn, "Unexpected rcon result command: \"%s\" (arguments \"%s\")", token, parser.GetCurrentScript());
	}

	return nullptr;
}
