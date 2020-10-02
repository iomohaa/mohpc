#pragma once

#include "../../Global.h"
#include "../../Object.h"
#include "../../Managers/NetworkManager.h"
#include "../Types.h"
#include "../Socket.h"
#include "../Encoding.h"
#include "../Channel.h"

#include "../../Script/Container.h"

namespace MOHPC
{
	class MSG;

	namespace Network
	{
		using clientNetTime = std::chrono::time_point<std::chrono::steady_clock>;

		class Challenge
		{
			MOHPC_OBJECT_DECLARATION(Challenge);

		public:
			Challenge(const netadr_t& inFrom, clientNetTime inTime, uint32_t inChallenge);

			bool hasElapsed(std::chrono::milliseconds elapsedTime) const;
			uint32_t getChallenge() const;
			const netadr_t& getSourceAddress() const;

		private:
			clientNetTime time;
			uint32_t challenge;
			netadr_t from;
		};

		class ClientData
		{
			MOHPC_OBJECT_DECLARATION(ClientData);

		public:
			size_t numCommands;
			uint32_t clientSequence;

		public:
			ClientData(const IUdpSocketPtr& socket, const netadr_t& from, uint16_t qport, uint32_t challengeNum);

			const netadr_t& getAddress() const;
			uint16_t getQPort() const;
			Encoding& getEncoding() const;
			uint32_t newSequence();

		private:
			IUdpSocketPtr socket;
			EncodingPtr encoding;
			netadr_t source;
			uint32_t challengeNum;
			uint32_t sequenceNum;
			uint16_t qport;
			char reliableCommands[MAX_RELIABLE_COMMANDS * MAX_STRING_CHARS]{ 0 };
			char serverCommands[MAX_RELIABLE_COMMANDS * MAX_STRING_CHARS]{ 0 };
			char* reliableCommandList[MAX_RELIABLE_COMMANDS];
			char* serverCommandList[MAX_RELIABLE_COMMANDS];
		};
		using ClientDataPtr = SharedPtr<ClientData>;

		class ServerHost : public ITickableNetwork
		{
			MOHPC_OBJECT_DECLARATION(ServerHost);

		public:
			MOHPC_EXPORTS ServerHost(const NetworkManagerPtr& networkManager);

			void tick(uint64_t deltaTime, uint64_t currentTime) override;

			ClientData& createClient(const netadr_t& from, uint16_t qport, uint32_t challengeNum);
			ClientData* findClient(const netadr_t& from, uint16_t qport) const;

			Challenge& createChallenge(const netadr_t& from);
			uintptr_t getChallenge(const netadr_t& from) const;

			void processClient(ClientData& client, uint32_t sequenceNum, IMessageStream& stream, MSG& msg);

		private:
			void connectionLessReply(const netadr_t& target, const char* reply);

			void processRequests();
			void sendStateToClients();
			void sendClientMessage(ClientData& client);
			void sendGameStateToClient(ClientData& client);

		private:
			uint32_t serverId;
			IUdpSocketPtr serverSocket;
			INetchanPtr conChan;
			Container<ClientDataPtr> clientList;
			Container<Challenge> challenges;
		};
		using ServerHostPtr = SharedPtr<ServerHost>;
}
}