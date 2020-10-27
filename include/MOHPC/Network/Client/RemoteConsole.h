#pragma once

#include "../Types.h"
#include "../Socket.h"
#include "../../Managers/NetworkManager.h"
#include "../../Object.h"
#include "../../Utilities/SharedPtr.h"
#include "../../Utilities/HandlerList.h"
#include "../../Utilities/RequestHandler.h"

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
		class RemoteConsole : public ITickableNetwork
		{
			MOHPC_OBJECT_DECLARATION(RemoteConsole);

		private:
			struct RConHandlerList
			{
			public:
				FunctionList<RConHandlers::Print> printHandler;
			};

		private:
			RConHandlerList handlerList;
			IUdpSocketPtr socket;
			str password;
			NetAddrPtr address;
			// FIXME: RequestHandler (queue response each requests)
			// FIXME: HandlerList (callbacks, for all rcon response)

		public:
			MOHPC_EXPORTS RemoteConsole(const NetworkManagerPtr& networkManager, const NetAddrPtr& address, const char* password);
			~RemoteConsole();

			virtual void tick(uint64_t deltaTime, uint64_t currentTime) override;

			/** Return the handler list. */
			MOHPC_EXPORTS RConHandlerList& getHandlerList();

			/** Send a remote console command. */
			MOHPC_EXPORTS void send(const char* command);
		};
		using RemoteConsolePtr = SharedPtr<RemoteConsole>;
	}
}