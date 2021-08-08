#pragma once

#include "../NetGlobal.h"
#include "../ProtocolSingleton.h"

#include <functional>
#include <cstdint>

namespace MOHPC
{
class MSG;

namespace Network
{
	struct mapInfo_t;
	class ICommandSequence;
	class IReliableSequence;
	class usereyes_t;
	class usercmd_t;

	namespace Parsing
	{
		class IString;

		/**
		 * Abstract hash interface singleton for string hashing.
		 */
		class MOHPC_NET_EXPORTS IPacketHeader : public IProtocolSingleton<IPacketHeader>
		{
		public:
			virtual void writeHeader(MSG& msg, const mapInfo_t& mapInfo, const ICommandSequence& commandSequence, uint32_t sequenceNum) const = 0;
			virtual void readHeader(MSG& msg, mapInfo_t& mapInfo, IReliableSequence& reliableSequence, uint32_t& sequenceNum) const = 0;
		};

		class MOHPC_NET_EXPORTS IRemoteCommand : public IProtocolSingleton<IRemoteCommand>
		{
		public:
			virtual void writeCommands(MSG& msg, const IReliableSequence& reliableSequence, const IString& stringParser) const = 0;
			virtual void readCommands(MSG& msg, ICommandSequence& commandSequence, const IString& stringParser) const = 0;
		};

		using CmdGetterFunc = std::function<const usercmd_t& (size_t i)>;

		class MOHPC_NET_EXPORTS IUserMove : public IProtocolSingleton<IUserMove>
		{
		public:
			/**
			 * Append movement data to the message.
			 *
			 * @param oldEyes previous eyes information (for delta).
			 * @param eyes information about the client view.
			 * @param cmdGetter function for getting the command at index.
			 * @param count number of commands to send.
			 * @param key used to hash the command data.
			 * @param deltaMove whether or not this should be a delta message.
			 * @return oldcmd the last command written into the message.
			 */
			virtual void writeMovement(
				MSG& msg,
				const usereyes_t& oldEyes,
				const usereyes_t& eyes,
				const CmdGetterFunc& cmdGetter,
				size_t count,
				uint32_t key,
				bool deltaMove
			) const = 0;
		};
	}
}
}
