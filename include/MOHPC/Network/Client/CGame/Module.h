#pragma once

#include "../../NetGlobal.h"
#include "../../NetObject.h"
#include "CSMonitor/GameMonitor.h"
#include "GameplayNotify.h"
#include "MessageParse.h"
#include "ServerInfo.h"
#include "Snapshot.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
	struct Imports
	{
	public:
		const ServerSnapshotManager& snapshotManager;
		const ClientTime& clientTime;
		const ICommandSequence& commandSequence;
	};

	/**
	 * Base CG module, contains most implementations.
	 */
	class ModuleBase
	{
	public:
		ModuleBase();
		virtual ~ModuleBase();

		void setProtocol(protocolType_c protocol);
		void setImports(const Imports& imports);

		void init(const ServerGameStatePtr& gs, uintptr_t serverMessageSequence, rsequence_t serverCommandSequence);

		/** Tick function for CGame module. */
		void tick(deltaTime_t deltaTime, tickTime_t currentTime, tickTime_t simulatedServerTime);
		
		/** Get the current client time. */
		MOHPC_NET_EXPORTS tickTime_t getTime() const;

		MOHPC_NET_EXPORTS GameplayNotify& getGameplayNotify();
		MOHPC_NET_EXPORTS const GameplayNotify& getGameplayNotify() const;

		MOHPC_NET_EXPORTS const cgsInfo& getServerInfo() const;

		/** Return a reference to the snapshot manager/processor. */
		MOHPC_NET_EXPORTS SnapshotProcessor& getSnapshotProcessor();
		MOHPC_NET_EXPORTS const SnapshotProcessor& getSnapshotProcessor() const;
		MOHPC_NET_EXPORTS const SnapshotProcessorPtr& getSnapshotProcessorPtr();
		MOHPC_NET_EXPORTS ConstSnapshotProcessorPtr getSnapshotProcessorPtr() const;

		/** CG message notification. */
		void parseCGMessage(MSG& msg);

	private:
		void parseServerInfo(const char* cs);

	private:
		const IMessageParser* messageParser;
		HandlerList handlerList;
		cgsInfoPtr cgs;
		SnapshotProcessorPtr processedSnapshots;
		GameplayNotify gameplayNotify;
		GameMonitor gameMonitor;
	};
	}
}
}
