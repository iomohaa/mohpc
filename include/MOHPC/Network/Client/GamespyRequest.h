#pragma once

#include "../../Utilities/SharedPtr.h"
#include "../../Utilities/RequestHandler.h"
#include "../Types.h"
#include "../Socket.h"

namespace MOHPC
{
	class Info;
	class str;
	class IMessageStream;

	namespace Network
	{
		/**
		 * Base class for handling gamespy request
		 */
		class IGamespyRequest : public IRequestBase
		{
		public:
			virtual ~IGamespyRequest() = default;

			/** Return a supplied info request string. */
			virtual void generateInfo(Info& info) = 0;

			/** Return the query id. */
			virtual const char* queryId() const { return nullptr; };

			virtual void generateOutput(IMessageStream& output) override final;
		};

		/**
		 * Base class for handling gamespy request
		 */
		class IGamespyServerRequest : public IRequestBase
		{
		public:
			virtual ~IGamespyServerRequest() = default;

			/** Return a supplied info request string. */
			virtual const char* generateQuery() = 0;

			virtual void generateOutput(IMessageStream& output) override final;
		};

		using IGamespyRequestPtr = SharedPtr<IGamespyRequest>;

		class GamespyRequestParam
		{
		private:
			ITcpSocketPtr socket;

		public:
			GamespyRequestParam();
			GamespyRequestParam(const ITcpSocketPtr& inSocket);

			void send(const uint8_t* buf, size_t size);
			size_t receive(uint8_t* buf, size_t size);
			bool hasData() const;
			size_t receiveSize() const;
		};

		class GamespyUDPRequestParam
		{
		private:
			IUdpSocketPtr socket;
			NetAddrPtr addr;

		public:
			GamespyUDPRequestParam();
			GamespyUDPRequestParam(const IUdpSocketPtr& inSocket, const NetAddrPtr& inAddr);

			void send(const uint8_t* buf, size_t size);
			size_t receive(uint8_t* buf, size_t size);
			bool hasData() const;
			size_t receiveSize() const;

			const NetAddr& getLastIp() const;
		};

		class GamespyUDPBroadcastRequestParam
		{
		private:
			IUdpSocketPtr socket;
			uint16_t startPort;
			uint16_t endPort;
			NetAddrPtr lastIp;

		public:
			GamespyUDPBroadcastRequestParam();
			GamespyUDPBroadcastRequestParam(const IUdpSocketPtr& inSocket, uint16_t inStartPort, uint16_t inEndPort);

			void send(const uint8_t* buf, size_t size);
			size_t receive(uint8_t* buf, size_t size);
			bool hasData() const;
			size_t receiveSize() const;

			const NetAddrPtr& getLastIp() const;
		};
	}
}
