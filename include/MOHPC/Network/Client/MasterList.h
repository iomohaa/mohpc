#pragma once

#include "Server.h"
#include "GamespyRequest.h"
#include "../../Utility/RequestHandler.h"
#include "../../Utility/MessageDispatcher.h"
#include "../NetGlobal.h"
#include "../NetObject.h"
#include "../Socket.h"
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
		class IServerList
		{
		public:
			IServerList();
			virtual ~IServerList() = default;

			/**
			 * Fetch the list of servers.
			 *
			 * @param	callback	Called each time it finds a server.
			 */
			 virtual void fetch(FoundServerCallback&& callback) = 0;
		};
		using IServerListPtr = SharedPtr<IServerList>;

		class ServerList : public IServerList
		{
			MOHPC_NET_OBJECT_DECLARATION(ServerList);

		private:
			class Request_SendCon : public IGamespyRequest
			{
			public:
				Request_SendCon(gameListType_e inType);

				virtual void generateInfo(Info& info) override;
				virtual SharedPtr<IRequestBase> process(InputRequest& data) override;

			private:
				gameListType_e gameType;
			};

			class Request_SendToken : public IGamespyRequest
			{
			private:
				gameListType_e gameType;
				char encoded[64];

			public:
				Request_SendToken(const char* challenge, gameListType_e inType);
				virtual void generateInfo(Info& info) override;
				virtual bool mustProcess() const override;
				virtual SharedPtr<IRequestBase> process(InputRequest& data) override;
				virtual const char* queryId() const override;

			private:
				void swapByte(uint8_t* a, uint8_t* b);
				uint8_t encodeChar(uint8_t c);
				void encode(uint8_t* ins, int size, uint8_t* result);
				void encrypt(const uint8_t* key, int key_len, uint8_t* buffer_ptr, int buffer_len);
			};

			class Request_FetchServers : public IGamespyRequest, public std::enable_shared_from_this<Request_FetchServers>
			{
			private:
				MessageDispatcherPtr dispatcher;
				ICommunicatorPtr comm;
				const uint8_t* key;
				const char* game;
				FoundServerCallback callback;
				size_t pendingLen;
				char pendingData[6];

			public:
				Request_FetchServers(const MessageDispatcherPtr& dispatcher, const ICommunicatorPtr& comm, gameListType_e inGameType, FoundServerCallback&& inCallback);

				virtual void generateInfo(Info& info) override;
				virtual SharedPtr<IRequestBase> process(InputRequest& data) override;


				virtual SharedPtr<IRequestBase> timedOut() override;

			private:
				void nullCallback(const IServerPtr& server);
			};

		private:
			ICommunicatorPtr comm;
			MessageDispatcherPtr dispatcher;
			IncomingMessageHandler handler;
			gameListType_e gameType;

		public:
			MOHPC_NET_EXPORTS ServerList(const MessageDispatcherPtr& dispatcher, const ICommunicatorPtr& masterComm, const ICommunicatorPtr& comm, const IRemoteIdentifierPtr& masterId, gameListType_e type);

			MOHPC_NET_EXPORTS virtual void fetch(FoundServerCallback&& callback) override;

		private:
			void sendRequest(IGamespyRequestPtr&& newRequest);
		};
		using ServerListPtr = SharedPtr<ServerList>;

		class ServerListLAN : public IServerList
		{
			MOHPC_NET_OBJECT_DECLARATION(ServerListLAN);

		private:
			class Request_InfoBroadcast : public IRequestBase, public std::enable_shared_from_this<Request_InfoBroadcast>
			{
			private:
				FoundServerCallback response;

			public:
				Request_InfoBroadcast(FoundServerCallback&& inResponse);

				virtual void generateOutput(IMessageStream& output) override;
				virtual SharedPtr<IRequestBase> process(InputRequest& data) override;
				virtual bool isThisRequest(InputRequest& data) const override;
			};

		public:
			MOHPC_NET_EXPORTS ServerListLAN(const MessageDispatcherPtr& dispatcher, const ICommunicatorPtr& comm, uint64_t timeoutTimeValue = 1000);

			void fetch(FoundServerCallback&& callback) override;

		private:
			IUdpSocketPtr socket;
			//RequestHandler<IRequestBase, GamespyUDPBroadcastRequestParam> handler;
			IncomingMessageHandler handler;
			uint64_t timeoutTime;
		};
		using ServerListLANPtr = SharedPtr<ServerListLAN>;
	}
}
