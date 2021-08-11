#pragma once

#include "Base.h"
#include "PrintHandler.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		class ChatPrintCommandHandler : public SingleCommandHandlerBase<Handlers::Print>
		{
			MOHPC_NET_OBJECT_DECLARATION(ChatPrintCommandHandler);

		public:
			MOHPC_NET_EXPORTS ChatPrintCommandHandler(const SnapshotProcessorPtr& snapshotProcessor);
			MOHPC_NET_EXPORTS ~ChatPrintCommandHandler();

		private:
			void handle(TokenParser& args);
		};
		using ChatPrintCommandHandlerPtr = SharedPtr<ChatPrintCommandHandler>;
	}
}
}
