#pragma once

#include "../Global.h"
#include "Socket.h"
#include "Server.h"
#include "GamespyRequest.h"
#include "../Utilities/RequestHandler.h"
#include "../Managers/NetworkManager.h"
#include <stdint.h>
#include  <queue>

namespace MOHPC
{
	class IMessageStream;
	class Info;

	namespace Network
	{
		using FoundServerCallback = std::function<void(const IServerPtr& server)>;
		using MasterServerDone = std::function<void()>;

		enum class gameListType_e : uint8_t
		{
			mohaa,
			mohaas,
			mohaab,
			max
		};

		/** Abstract server list class. */
		class IServerList : public ITickableNetwork
		{
		public:
			IServerList(NetworkManager* inManager);
			/**
			 * Fetch the list of servers.
			 *
			 * @param	callback	Called each time it finds a server.
			 */
			 virtual void fetch(FoundServerCallback&& callback, MasterServerDone&& doneCallback = MasterServerDone()) = 0;
		};

		class ServerList : public IServerList
		{
		private:
			class Request_SendCon : public IGamespyRequest
			{
			private:
				NetworkManager* networkManager;
				gameListType_e gameType;

			public:
				Request_SendCon(NetworkManager* networkManager, gameListType_e inType);

				virtual void generateInfo(Info& info);
				virtual SharedPtr<IRequestBase> process(RequestData& data) override;
			};

			class Request_SendToken : public IGamespyRequest
			{
			private:
				NetworkManager* networkManager;
				gameListType_e gameType;
				char encoded[64];

			public:
				Request_SendToken(NetworkManager* networkManager, const char* challenge, gameListType_e inType);
				virtual void generateInfo(Info& info);
				virtual bool mustProcess() const;
				virtual SharedPtr<IRequestBase> process(RequestData& data) override;
				virtual const char* queryId() const;

			private:
				void swapByte(uint8_t* a, uint8_t* b);
				uint8_t encodeChar(uint8_t c);
				void encode(uint8_t* ins, int size, uint8_t* result);
				void encrypt(const uint8_t* key, int key_len, uint8_t* buffer_ptr, int buffer_len);
			};

			class Request_FetchServers : public IGamespyRequest, public std::enable_shared_from_this<Request_FetchServers>
			{
			private:
				NetworkManager* networkManager;
				const uint8_t* key;
				const char* game;
				FoundServerCallback callback;
				MasterServerDone doneCallback;
				size_t pendingLen;
				char pendingData[6];

			public:
				Request_FetchServers(NetworkManager* networkManager, gameListType_e inGameType, FoundServerCallback&& inCallback, MasterServerDone&& doneCallback);

				virtual void generateInfo(Info& info);
				virtual SharedPtr<IRequestBase> process(RequestData& data) override;


				virtual SharedPtr<IRequestBase> timedOut() override;

			private:
				void nullCallback(const IServerPtr& server);
			};

		private:
			ITcpSocketPtr socket;
			RequestHandler<IGamespyRequest, GamespyRequestParam> handler;
			gameListType_e gameType;

		public:
			MOHPC_EXPORTS ServerList(NetworkManager* inManager, gameListType_e type);

			MOHPC_EXPORTS virtual void fetch(FoundServerCallback&& callback, MasterServerDone&& doneCallback) override;
			virtual void tick(uint64_t deltaTime, uint64_t currentTime) override;

		private:
			void sendRequest(IGamespyRequestPtr&& newRequest);
		};

		class ServerListLAN : public IServerList
		{
		private:
			class Request_InfoBroadcast : public IRequestBase, public std::enable_shared_from_this<Request_InfoBroadcast>
			{
			private:
				NetworkManager* networkManager;
				FoundServerCallback response;

			public:
				Request_InfoBroadcast(NetworkManager* inNetworkManager, FoundServerCallback&& inResponse);

				virtual void generateOutput(IMessageStream& output) override;
				virtual SharedPtr<IRequestBase> process(RequestData& data) override;
			};

		private:
			IUdpSocketPtr socket;
			RequestHandler<IRequestBase, GamespyUDPBroadcastRequestParam> handler;

		public:
			MOHPC_EXPORTS ServerListLAN(NetworkManager* inManager);

			void fetch(FoundServerCallback&& callback, MasterServerDone&& doneCallback) override;
			virtual void tick(uint64_t deltaTime, uint64_t currentTime) override;
		};
	}
}
