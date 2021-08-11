#pragma once

#include "Base.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		namespace Handlers
		{
			/**
			 * Called from server to make the client process a console command.
			 *
			 * @param tokenized Arguments of stufftext (command to exec on console).
			 * @note This is a dangerous command and should be parsed with precautions.
			 */
			struct ReceivedStufftext : public HandlerNotifyBase<void(TokenParser& tokenized)> {};
		}

		class StufftextCommandHandler : public SingleCommandHandlerBase<Handlers::ReceivedStufftext>
		{
			MOHPC_NET_OBJECT_DECLARATION(StufftextCommandHandler);

		public:
			MOHPC_NET_EXPORTS StufftextCommandHandler(const SnapshotProcessorPtr& snapshotProcessor);
			MOHPC_NET_EXPORTS ~StufftextCommandHandler();

		private:
			void handle(TokenParser& args) override;
		};
		using StufftextCommandHandlerPtr = SharedPtr<StufftextCommandHandler>;
	}
}
}
