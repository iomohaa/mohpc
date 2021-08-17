#pragma once

#include "Base.h"
#include "../Vote.h"

namespace MOHPC
{
	namespace Network
	{
		namespace CGame
		{
			namespace Handlers
			{
				/**
				 * The server notifies clients when it is experiencing lags, such as hitches.
				 */
				struct VoteParsed : public HandlerNotifyBase<void(const VoteOptions& options)> {};
			}

			class VoteCommandHandler : public ServerCommandHandlerBase
			{
				MOHPC_NET_OBJECT_DECLARATION(VoteCommandHandler);

			public:
				MOHPC_NET_EXPORTS VoteCommandHandler(const SnapshotProcessorPtr& snapshotProcessor);
				MOHPC_NET_EXPORTS ~VoteCommandHandler();

				FunctionList<Handlers::VoteParsed>& getHandler();

			private:
				void commandStartReadFromServer(TokenParser& args);
				void commandContinueReadFromServer(TokenParser& args);
				void commandFinishReadFromServer(TokenParser& args);

			private:
				CommandTemplate<VoteCommandHandler, &VoteCommandHandler::commandStartReadFromServer> startReadFromServerHandler;
				CommandTemplate<VoteCommandHandler, &VoteCommandHandler::commandContinueReadFromServer> continueReadFromServerHandler;
				CommandTemplate<VoteCommandHandler, &VoteCommandHandler::commandFinishReadFromServer> finishReadFromServerHandler;

				VoteOptionsParser parser;
				FunctionList<Handlers::VoteParsed> voteParsedHandler;
			};
			using VoteCommandHandlerPtr = SharedPtr<VoteCommandHandler>;
		}
	}
}
