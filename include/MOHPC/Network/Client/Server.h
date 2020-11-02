#pragma once

#include "ClientGame.h"
#include "GamespyRequest.h"
#include "../Types.h"
#include "../../Utilities/Info.h"
#include "../../Utilities/LazyPtr.h"
#include "../../Utilities/RequestHandler.h"
#include "../../Managers/NetworkManager.h"
#include "../../Object.h"
#include <functional>

namespace MOHPC
{
	namespace Network
	{
		namespace Callbacks
		{
			using Query = std::function<void(const ReadOnlyInfo& Info)>;

			using Connect = std::function<void(const ClientGameConnectionPtr& client, const char* errorMessage)>;
			using Response = std::function<void(const ReadOnlyInfo* info)>;
			using ServerTimeout = std::function<void()>;
		}

		struct ConnectSettings
		{
			MOHPC_OBJECT_DECLARATION(ConnectSettings);

		public:
			MOHPC_EXPORTS ConnectSettings();

		public:
			/**
			 * Deferred values time should be higher enough, but not too high,
			 * to not make the server drop too soon packets
			 * and to not make the server think the client has an high ping.
			 * 100 is frequently a great deal.
			 */

			/** The time to wait before sending a challenge. */
			MOHPC_EXPORTS void setDeferredChallengeTime(size_t newTime);
			MOHPC_EXPORTS size_t getDeferredChallengeTime() const;

			/** The time to wait before sending connect message (after challenging). */
			MOHPC_EXPORTS void setDeferredConnectTime(size_t newTime);
			MOHPC_EXPORTS size_t getDeferredConnectTime() const;

			/** The qport the player is using, to be able to connect on the same public network. */
			MOHPC_EXPORTS void setQport(uint16_t newValue);
			MOHPC_EXPORTS uint16_t getQport() const;

			/** The version that is sent when connecting. Set to null or empty to set to the default version (CLIENT_VERSION). */
			MOHPC_EXPORTS void setVersion(const char* value);
			MOHPC_EXPORTS const char* getVersion() const;

			/** The CD-Key to use when asked for an authorize request. */
			MOHPC_EXPORTS void setCDKey(const char* value);
			MOHPC_EXPORTS const char* getCDKey() const;

		private:
			const char* version;
			const char* cdKey;
			size_t deferredConnectTime;
			size_t deferredChallengeTime;
			uint16_t qport;
		};
		using ConnectSettingsPtr = SharedPtr<ConnectSettings>;

		class IServer : public ITickableNetwork
		{
		private:
			NetAddrPtr address;

		public:
			IServer(const NetworkManagerPtr& inManager, const NetAddrPtr& adr);

			virtual void query(Callbacks::Query&& response, Callbacks::ServerTimeout&& timeoutResult = Callbacks::ServerTimeout(), size_t timeoutTime = 10000) = 0;
			MOHPC_EXPORTS const NetAddrPtr& getAddress() const;
		};

		using IServerPtr = SharedPtr<IServer>;
		using IServerLazyPtr = LazyPtr<IServer>;

		class GSServer : public IServer, public std::enable_shared_from_this<GSServer>
		{
		private:
			class Request_Query : public IGamespyServerRequest, public std::enable_shared_from_this<Request_Query>
			{
			private:
				str infoStr;
				Callbacks::Query response;
				Callbacks::ServerTimeout timeout;

			public:
				Request_Query(Callbacks::Query&& response, Callbacks::ServerTimeout&& timeoutResult);

				virtual const char* generateQuery() override;
				virtual SharedPtr<IRequestBase> process(RequestData& data) override;
				virtual SharedPtr<IRequestBase> timedOut() override;

			};
			
		private:
			RequestHandler<IGamespyServerRequest, GamespyUDPRequestParam> handler;
			IUdpSocketPtr socket;

		public:
			GSServer(const NetworkManagerPtr& inManager, const NetAddrPtr& adr);

			virtual void tick(uint64_t deltaTime, uint64_t currentTime) override;

			void query(Callbacks::Query&& response, Callbacks::ServerTimeout&& timeoutResult, size_t timeoutTime) override;
		};

		class LANServer : public IServer
		{
		private:
			ReadOnlyInfo info;
			char* dataStr;

		public:
			LANServer(const NetworkManagerPtr& inManager, const NetAddrPtr& inAddress, char* inInfo, size_t infoSize);
			~LANServer();

			virtual void tick(uint64_t deltaTime, uint64_t currentTime) override;

			void query(Callbacks::Query&& response, Callbacks::ServerTimeout&& timeoutResult, size_t timeoutTime) override;
		};

		class EngineServer : public ITickableNetwork, public std::enable_shared_from_this<EngineServer>
		{
			MOHPC_OBJECT_DECLARATION(EngineServer);

		private:
			using ConnectResponse = std::function<void(uint16_t qport, uint32_t challenge, const protocolType_c& protoType, const ClientInfoPtr& cInfo, const char* errorMessage)>;

			struct ConnectionParams
			{
				ConnectResponse response;
				Callbacks::ServerTimeout timeoutCallback;
				ClientInfoPtr info;
				ConnectSettingsPtr settings;

				ConnectionParams() {}
				// Remove copy constructor
				ConnectionParams(const ConnectionParams& other) = delete;
				ConnectionParams& operator=(const ConnectionParams& other) = delete;
				ConnectionParams(ConnectionParams&& other) = default;
				ConnectionParams& operator=(ConnectionParams&& other) = default;
			};

			class IEngineRequest : public IRequestBase
			{
			public:
				Callbacks::ServerTimeout timeoutCallback;

			public:
				IEngineRequest() = default;
				IEngineRequest(Callbacks::ServerTimeout&& inTimeoutCallback);

				/** Called to generate a request string. */
				virtual str generateRequest() = 0;

				/** Whether or not the request should be huff-compressed. */
				virtual bool shouldCompressRequest(size_t& offset) { return false; };

				/** True if the request supports the given event response. */
				virtual bool supportsEvent(const char* name) = 0;

				/** Pass response to the request so it can handle it. Can return another request if necessary. */
				virtual IRequestPtr handleResponse(const char* name, TokenParser& parser) = 0;

				virtual void generateOutput(IMessageStream& output) override final;

				virtual IRequestPtr process(RequestData& data) override final;
				virtual IRequestPtr timedOut() override;

			};

			using IEngineRequestPtr = SharedPtr<IEngineRequest>;

			class VerBeforeChallengeRequest : public IEngineRequest
			{
			private:
				ConnectionParams data;

			public:
				VerBeforeChallengeRequest(ConnectionParams&& inData);
				virtual str generateRequest();
				virtual bool supportsEvent(const char* name) override;
				virtual IRequestPtr handleResponse(const char* name, TokenParser& parser) override;
			};
			class ChallengeRequest : public IEngineRequest, public std::enable_shared_from_this<ChallengeRequest>
			{
			private:
				ConnectionParams data;
				protocolType_c protocol;
				uint8_t numRetries;

			public:
				ChallengeRequest(const protocolType_c& proto, ConnectionParams&& inData, Callbacks::ServerTimeout&& inTimeoutCallback);
				virtual str generateRequest();
				virtual bool supportsEvent(const char* name) override;
				virtual IRequestPtr handleResponse(const char* name, TokenParser& parser) override;
				virtual uint64_t overrideTimeoutTime(bool& overriden) override;
				virtual IRequestPtr timedOut() override;
				virtual uint64_t deferredTime() override;
			};

			class AuthorizeRequest : public IEngineRequest, public std::enable_shared_from_this<AuthorizeRequest>
			{
			private:
				ConnectionParams data;
				protocolType_c protocol;
				str challenge;
				uint8_t numRetries;

			public:
				AuthorizeRequest(const protocolType_c& proto, ConnectionParams&& inData, const char* challenge, Callbacks::ServerTimeout&& inTimeoutCallback);
				virtual str generateRequest();
				virtual bool supportsEvent(const char* name) override;
				virtual IRequestPtr handleResponse(const char* name, TokenParser& parser) override;
				virtual uint64_t overrideTimeoutTime(bool& overriden) override;
				virtual IRequestPtr timedOut() override;
			};

			class ConnectRequest : public IEngineRequest, public std::enable_shared_from_this<ConnectRequest>
			{
			private:
				ConnectionParams data;
				protocolType_c protocol;
				uint32_t challenge;
				uint16_t qport;
				uint8_t numRetries;

			public:
				ConnectRequest(const protocolType_c& proto, ConnectionParams&& inData, uint32_t challenge, Callbacks::ServerTimeout&& inTimeoutCallback);
				virtual str generateRequest();
				virtual bool shouldCompressRequest(size_t& offset);
				virtual bool supportsEvent(const char* name) override;
				virtual IRequestPtr handleResponse(const char* name, TokenParser& parser) override;
				virtual uint64_t overrideTimeoutTime(bool& overriden) override;
				virtual IRequestPtr timedOut() override;
				virtual uint64_t deferredTime() override;
			};

			class StatusRequest : public IEngineRequest
			{
			private:
				Callbacks::Response response;

			public:
				StatusRequest(Callbacks::Response&& inResponse);
				virtual str generateRequest() override;
				virtual bool supportsEvent(const char* name) override;
				virtual IRequestPtr handleResponse(const char* name, TokenParser& parser) override;
			};

			class InfoRequest : public IEngineRequest
			{
			private:
				Callbacks::Response response;

			public:
				InfoRequest(Callbacks::Response&& inResponse);
				virtual str generateRequest() override;
				virtual bool supportsEvent(const char* name) override;
				virtual IRequestPtr handleResponse(const char* name, TokenParser& parser) override;
			};

			struct EmbeddedRequest
			{
				IRequestPtr request;
				Callbacks::ServerTimeout timedout;

			public:
				EmbeddedRequest() {}
				EmbeddedRequest(const IRequestPtr& inRequest, Callbacks::ServerTimeout&& inTimedout);
				// Remove copy constructor
				EmbeddedRequest(const EmbeddedRequest& other) = delete;
				EmbeddedRequest& operator=(const EmbeddedRequest& other) = delete;
				EmbeddedRequest(EmbeddedRequest&& other) = default;
				EmbeddedRequest& operator=(EmbeddedRequest&& other) = default;
			};

		private:
			IUdpSocketPtr socket;
			NetAddrPtr address;
			RequestHandler<IRequestBase, GamespyUDPRequestParam> handler;

		public:
			MOHPC_EXPORTS EngineServer(const NetworkManagerPtr& inManager, const NetAddrPtr& inAddress, const IUdpSocketPtr& existingSocket = nullptr);
			MOHPC_EXPORTS ~EngineServer();

			virtual void tick(uint64_t deltaTime, uint64_t currentTime) override;

			/**
			 * Callbacks::Connect to the specified server.
			 *
			 * @param	clientInfo		Settings containing some important information (to avoid useless allocations this parameter will be moved out).
			 * @param	connectSettings	Settings to use before connecting.
			 * @param	result			Callback that will be called when finished connecting or when an error has occurred.
			 * @param	timeoutResult	Callback is called on connect timeout.
			 */
			MOHPC_EXPORTS void connect(const ClientInfoPtr& clientInfo, const ConnectSettingsPtr& connectSettings, Callbacks::Connect&& result, Callbacks::ServerTimeout&& timeoutResult = Callbacks::ServerTimeout());

			/**
			 * Retrieve various settings from the specified server.
			 *
			 * @param	to		Server to get status from
			 * @param	result	Callback that will be called after receiving response
			 */
			MOHPC_EXPORTS void getStatus(Callbacks::Response&& result, Callbacks::ServerTimeout&& timeoutResult = Callbacks::ServerTimeout(), size_t timeoutTime = 10000);

			/**
			 * Retrieve various engine info from the specified server.
			 *
			 * @param	to		Server to get status from
			 * @param	result	Callback that will be called after receiving response
			 */
			MOHPC_EXPORTS void getInfo(Callbacks::Response&& result, Callbacks::ServerTimeout&& timeoutResult = Callbacks::ServerTimeout(), size_t timeoutTime = 10000);

			/**
			 * Set the timeout value in ms.
			 *
			 * @param	milliseconds	Number of milliseconds before any request timeout
			 */
			MOHPC_EXPORTS void setRequestTimeout(size_t milliseconds);

		private:
			const IRequestPtr& currentRequest() const;
			void sendRequest(IEngineRequestPtr&& req, Callbacks::ServerTimeout&& timeoutResult = Callbacks::ServerTimeout(), size_t timeoutTime = 10000);

		private:
			void onConnect(const Callbacks::Connect result, uint16_t qport, uint32_t challengeResponse, const protocolType_c& protoType, const ClientInfoPtr& cInfo, const char* errorMessage);
		};
		using EngineServerPtr = SharedPtr<EngineServer>;
	}
}
