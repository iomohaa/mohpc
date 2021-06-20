#pragma once

#include "../../Utility/SharedPtr.h"
#include "../../Utility/RequestHandler.h"
#include "../Types.h"
#include "../Socket.h"

namespace MOHPC
{
	class Info;
	class IMessageStream;

	namespace Network
	{
		/**
		 * Base class for handling gamespy request from master server.
		 */
		class IGamespyRequest : public IRequestBase
		{
		public:
			virtual ~IGamespyRequest() = default;

			/** Return a supplied info request string. */
			virtual void generateInfo(Info& info) = 0;

			/** Return the query id. */
			virtual const char* queryId() const { return nullptr; };

			void generateOutput(IMessageStream& output) override final;
		};

		/**
		 * Base class for handling gamespy request from server.
		 */
		class IGamespyServerRequest : public IRequestBase
		{
		public:
			virtual ~IGamespyServerRequest() = default;

			/** Return a supplied info request string. */
			virtual const char* generateQuery() = 0;

			void generateOutput(IMessageStream& output) override final;
		};

		using IGamespyRequestPtr = SharedPtr<IGamespyRequest>;
	}
}
