#include <MOHPC/Network/Parsing/Input.h>
#include <MOHPC/Network/Parsing/String.h>
#include <MOHPC/Network/Types/Reliable.h>
#include <MOHPC/Network/Types/GameState.h>
#include <MOHPC/Network/Types/UserInput.h>
#include <MOHPC/Network/Remote/Ops.h>
#include <MOHPC/Network/Serializable/UserInput.h>
#include <MOHPC/Common/Log.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::Parsing;

constexpr char MOHPC_LOG_NAMESPACE[] = "net_parse_input";

class PacketHeaderAny : public IPacketHeader
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		// concerns all protocol
		minRange = maxRange = 0;
	}

	void writeHeader(MSG& msg, const mapInfo_t& mapInfo, const ICommandSequence& commandSequence, uint32_t sequenceNum) const override
	{
		// write the server id (given with the gameState)
		msg.WriteUInteger(mapInfo.getServerId());
		// write the server sequence number (packet number)
		msg.WriteUInteger(sequenceNum);
		// write the command sequence acknowledge
		msg.WriteUInteger(commandSequence.getCommandSequence());
	}

	void readHeader(MSG& msg, mapInfo_t& mapInfo, IReliableSequence& reliableSequence, uint32_t& sequenceNum) const override
	{
		// initialize the client's map info
		mapInfo = mapInfo_t(msg.ReadUInteger(), 0);
		sequenceNum = msg.ReadUInteger();
		// NOTE: Should it error if the acknowledge is out of bound?
		reliableSequence.updateAcknowledge(msg.ReadUInteger());
	}
};

class RemoteCommandAny : public IRemoteCommand
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		// concerns all protocol
		minRange = maxRange = 0;
	}

	void writeCommands(MSG& msg, const IReliableSequence& reliableSequence, const IString& stringParser) const override
	{
		const rsequence_t reliableAcknowledge = reliableSequence.getReliableAcknowledge();
		const rsequence_t reliableSequenceNum = reliableSequence.getReliableSequence();
		for (uint32_t i = reliableAcknowledge + 1; i <= reliableSequenceNum; ++i)
		{
			msg.WriteByte(clc_ops_e::ClientCommand);
			msg.WriteInteger(i);
			stringParser.writeString(msg, reliableSequence.getSequence(i));
		}
	}

	void readCommands(MSG& msg, ICommandSequence& commandSequence, const IString& stringParser) const override
	{

	}
};

class UserMoveAny : public IUserMove
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		// concerns all protocol
		minRange = maxRange = 0;
	}

	void writeMovement(MSG& msg, const usereyes_t& oldEyes, const usereyes_t& eyes, const CmdGetterFunc& cmdGetter, size_t count, uint32_t key, bool deltaMove) const override
	{
		if (!count)
		{
			// no commands to send
			return;
		}

		// write the operation type
		if (deltaMove)
		{
			// this is a delta move
			msg.WriteByteEnum<clc_ops_e>(clc_ops_e::Move);
		}
		else {
			msg.WriteByteEnum<clc_ops_e>(clc_ops_e::MoveNoDelta);
		}
		// write the number of commands
		msg.WriteByte((uint8_t)count);
		// write delta eyes
		const SerializableUserEyes oldEyeInfo(const_cast<usereyes_t&>(oldEyes));
		SerializableUserEyes userEyesWrite(const_cast<usereyes_t&>(eyes));
		msg.WriteDeltaClass(&oldEyeInfo, &userEyesWrite);

		// now write all commands
		writeAllCommands(msg, cmdGetter, count, key);
	}

	void writeAllCommands(MSG& msg, const CmdGetterFunc& cmdGetter, size_t count, uint32_t key) const
	{
		const usercmd_t nullcmd;
		const usercmd_t* oldcmd = &nullcmd;

		// write all the commands, including the predicted command
		// first cmd is the most recent command
		for (size_t i = count; i > 0; i--)
		{
			const usercmd_t& cmd = cmdGetter(i - 1);
			// write a delta of the command by using the old
			SerializableUsercmd oldCmdRead(*const_cast<usercmd_t*>(oldcmd));
			SerializableUsercmd inputCmd(const_cast<usercmd_t&>(cmd));
			msg.WriteDeltaClass(&oldCmdRead, &inputCmd, key);

			oldcmd = &cmd;
		}
	}
};

PacketHeaderAny packetHeaderAny;
RemoteCommandAny remoteCommandAny;
UserMoveAny userMoveAny;
