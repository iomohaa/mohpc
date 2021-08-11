#pragma once

#include "Base.h"
#include "../Hud.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		namespace Handlers
		{
			/**
			 * Called to print a message on console.
			 *
			 * @param	type	Type of the message (see hudMessage_e).
			 * @param	text	Text to print.
			 */
			struct Print : public HandlerNotifyBase<void(hudMessage_e type, const char* text)> {};

			/**
			 * Called to print a message that is displayed on HUD, yellow color.
			 *
			 * @param	text	Text to print.
			 */
			struct HudPrint : public HandlerNotifyBase<void(const char* text)> {};
		}

		class PrintCommandHandler : public ServerCommandHandlerBase
		{
			MOHPC_NET_OBJECT_DECLARATION(PrintCommandHandler);

			struct HandlerList
			{
				FunctionList<Handlers::Print> printHandler;
				FunctionList<Handlers::HudPrint> hudPrintHandler;
			};

		public:
			MOHPC_NET_EXPORTS PrintCommandHandler(const SnapshotProcessorPtr& snapshotProcessor);
			MOHPC_NET_EXPORTS ~PrintCommandHandler();

			MOHPC_NET_EXPORTS HandlerList& getHandlerList();

		private:
			void handlePrint(TokenParser& args);
			void handleHudPrint(TokenParser& args);

		private:
			HandlerList handlerList;
			CommandTemplate<PrintCommandHandler, &PrintCommandHandler::handlePrint> printHandler;
			CommandTemplate<PrintCommandHandler, &PrintCommandHandler::handleHudPrint> hudPrintHandler;
		};
		using PrintCommandHandlerPtr = SharedPtr<PrintCommandHandler>;
	}
}
}
