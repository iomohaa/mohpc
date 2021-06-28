#pragma once

#include "../NetGlobal.h"
#include "../NetObject.h"
#include "../../Utility/Tick.h"
#include "../Remote/Socket.h"
#include "../Remote/Encoding.h"
#include "../Remote/Channel.h"

#include <morfuse/Container/Container.h>

#include <chrono>

namespace MOHPC
{
	class MSG;

	namespace Network
	{
		using clientNetTime = std::chrono::time_point<std::chrono::steady_clock>;

		static constexpr size_t MAX_RELIABLE_COMMANDS = 64;
		static constexpr size_t MAX_STRING_CHARS = 2048;

		class Challenge
		{
			MOHPC_NET_OBJECT_DECLARATION(Challenge);

		public:
			Challenge(const NetAddrPtr& inFrom, clientNetTime inTime, uint32_t inChallenge);

			bool hasElapsed(std::chrono::milliseconds elapsedTime) const;
			uint32_t getChallenge() const;
			const NetAddr& getSourceAddress() const;

		private:
			clientNetTime time;
			uint32_t challenge;
			NetAddrPtr from;
		};

		class ClientData
		{
			MOHPC_NET_OBJECT_DECLARATION(ClientData);

		public:
			size_t numCommands;
			uint32_t clientSequence;

		public:
			ClientData(const IUdpSocketPtr& socket, const NetAddrPtr& from, uint16_t qport, uint32_t challengeNum);

			const NetAddr& getAddress() const;
			uint16_t getQPort() const;
			Encoding& getEncoding() const;
			uint32_t newSequence();

		private:
			IUdpSocketPtr socket;
			EncodingPtr encoding;
			NetAddrPtr source;
			uint32_t challengeNum;
			uint32_t sequenceNum;
			uint16_t qport;
			char reliableCommands[MAX_RELIABLE_COMMANDS * MAX_STRING_CHARS]{ 0 };
			char serverCommands[MAX_RELIABLE_COMMANDS * MAX_STRING_CHARS]{ 0 };
			char* reliableCommandList[MAX_RELIABLE_COMMANDS];
			char* serverCommandList[MAX_RELIABLE_COMMANDS];
		};
		using ClientDataPtr = SharedPtr<ClientData>;

		class ServerHost : public ITickable
		{
			MOHPC_NET_OBJECT_DECLARATION(ServerHost);

		public:
			MOHPC_NET_EXPORTS ServerHost();

			void tick(uint64_t deltaTime, uint64_t currentTime) override;

			ClientData& createClient(const NetAddrPtr& from, uint16_t qport, uint32_t challengeNum);
			ClientData* findClient(const NetAddr& from, uint16_t qport) const;

			Challenge& createChallenge(const NetAddrPtr& from);
			uintptr_t getChallenge(const NetAddr& from) const;

			void processClient(ClientData& client, uint32_t sequenceNum, IMessageStream& stream, MSG& msg);

		private:
			void connectionLessReply(const NetAddrPtr& target, const char* reply);

			void processRequests();
			void sendStateToClients();
			void sendClientMessage(ClientData& client);
			void sendGameStateToClient(ClientData& client);

		private:
			uint32_t serverId;
			IUdpSocketPtr serverSocket;
			INetchanPtr conChan;
			mfuse::con::Container<ClientDataPtr> clientList;
			mfuse::con::Container<Challenge> challenges;
		};
		using ServerHostPtr = SharedPtr<ServerHost>;
}
}