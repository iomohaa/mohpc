#pragma once

#include "../../../NetGlobal.h"
#include "../../../NetObject.h"
#include "../Snapshot.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		class ServerCommandHandlerBase
		{
		public:
			ServerCommandHandlerBase(const SnapshotProcessorPtr& snapshotProcessor);
			virtual ~ServerCommandHandlerBase();

			SnapshotProcessor& getSnapshotProcessor();
			const SnapshotProcessor& getSnapshotProcessor() const;

		private:
			SnapshotProcessorPtr snapshotProcessor;
		};

		template<typename Handler>
		class SingleCommandHandlerBase : public ServerCommandHandlerBase
		{
		public:
			SingleCommandHandlerBase(const SnapshotProcessorPtr& snapshotProcessorPtr, const char* commandName)
				: ServerCommandHandlerBase(snapshotProcessorPtr)
				, command(*this)
			{
				CommandManager& m = getSnapshotProcessor().getCommandManager();
				m.add(commandName, &command);
			}

			~SingleCommandHandlerBase()
			{
				CommandManager& m = getSnapshotProcessor().getCommandManager();
				m.remove(&command);
			}

		public:
			MOHPC_NET_EXPORTS FunctionList<Handler>& getHandler()
			{
				return handler;
			}

			virtual void handle(TokenParser& args) = 0;

		private:
			FunctionList<Handler> handler;
			CommandTemplate<SingleCommandHandlerBase, &SingleCommandHandlerBase::handle> command;
		};
}
}
}
