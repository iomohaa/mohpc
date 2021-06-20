#pragma once

#include "../Types.h"
#include "../Socket.h"
#include "../NetObject.h"
#include "../../Utility/SharedPtr.h"
#include "../../Utility/HandlerList.h"
#include "../../Utility/RequestHandler.h"
#include "../../Utility/MessageDispatcher.h"

namespace MOHPC
{
	namespace Network
	{
		namespace RConHandlers
		{
			/**
			 * Called from the server to print text (in a response of a command that was previously sent remotely).
			 *
			 * @param	text	Text as response.
			 */
			struct Print : public HandlerNotifyBase<void(const char* text)> {};
		}

		/**
		 * Remote console for server (RCon).
		 */
		class RemoteConsole
		{
			MOHPC_NET_OBJECT_DECLARATION(RemoteConsole);

		private:
			struct RConHandlerList
			{
			public:
				FunctionList<RConHandlers::Print> printHandler;
			};

		private:
			RConHandlerList handlerList;
			str password;
			IncomingMessageHandler handler;

		public:
			MOHPC_NET_EXPORTS RemoteConsole(const MessageDispatcherPtr& dispatcher, const ICommunicatorPtr& comm, const IRemoteIdentifierPtr& remoteIdentifier, const char* password);
			~RemoteConsole();

			/** Return the handler list. */
			MOHPC_NET_EXPORTS RConHandlerList& getHandlerList();

			/** Send a remote console command. */
			MOHPC_NET_EXPORTS void send(const char* command, uint64_t timeoutValue = 5000);

		private:
			class RConMessageRequest : public IRequestBase
			{
			public:
				RConMessageRequest(const RConHandlerList& handlerList, const char* command, const char* password);

				/** Return a supplied info request string. */
				void generateOutput(IMessageStream& output) override;

				/** Return another request to execute, or finish it by returning NULL. */
				SharedPtr<IRequestBase> process(InputRequest& data) override;

			private:
				const RConHandlerList& handlerList;
				const char* command;
				const char* password;
			};
		};
		using RemoteConsolePtr = SharedPtr<RemoteConsole>;
	}
}