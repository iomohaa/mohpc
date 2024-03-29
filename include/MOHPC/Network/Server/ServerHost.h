#pragma once

#include "../NetGlobal.h"
#include "../NetObject.h"
#include "../../Utility/Tick.h"
#include "../Remote/Socket.h"
#include "../Remote/Encoding.h"
#include "../Remote/Channel.h"
#include "../Types/ReliableTemplate.h"

#include <vector>
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

			const NetAddrPtr& getAddress() const;
			uint16_t getQPort() const;
			XOREncoding& getEncoder();
			XOREncoding& getDecoder();
			uint32_t newSequence();

		private:
			IUdpSocketPtr socket;
			XOREncoding encoder;
			XOREncoding decoder;
			NetAddrPtr source;
			uint32_t challengeNum;
			uint32_t sequenceNum;
			uint16_t qport;
			RemoteCommandSequenceTemplate<64, 2048> reliableCommands;
			SequenceTemplate<64, 2048> serverCommands;
		};
		using ClientDataPtr = SharedPtr<ClientData>;

		class ServerHost : public ITickable
		{
			MOHPC_NET_OBJECT_DECLARATION(ServerHost);

		public:
			MOHPC_NET_EXPORTS ServerHost();

			void tick(deltaTime_t deltaTime, tickTime_t currentTime) override;

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
			std::vector<ClientDataPtr> clientList;
			std::vector<Challenge> challenges;
		};
		using ServerHostPtr = SharedPtr<ServerHost>;
}
}