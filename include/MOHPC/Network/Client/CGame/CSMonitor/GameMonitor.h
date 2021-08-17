#pragma once

#include "ConfigStringMonitor.h"
#include "../ServerInfo.h"
#include "../../../../Utility/SharedPtr.h"
#include "../../../../Utility/HandlerList.h"

namespace MOHPC
{
class ReadOnlyInfo;

namespace Network
{
namespace CGame
{
	namespace Handlers
	{
		/**
		 * Called when the soundtrack was changed.
		 *
		 * @param name Soundtrack name.
		 */
		class SoundtrackSet : public HandlerNotifyBase<void(const char* name)> {};

		/**
		 * Called when the music was changed.
		 *
		 * @param name Soundtrack name.
		 */
		class MusicSet : public HandlerNotifyBase<void(const char* name)> {};
	}
	class GameMonitor : public ConfigstringMonitorTemplate<GameMonitor>
	{
		MOHPC_NET_OBJECT_DECLARATION(GameMonitor);

		friend ConfigstringMonitorTemplate;

		struct HandlerList
		{
			FunctionList<Handlers::SoundtrackSet> soundTrackHandler;
			FunctionList<Handlers::MusicSet> musicHandler;
		};

	public:
		MOHPC_NET_EXPORTS GameMonitor(const SnapshotProcessorPtr& snapshotProcessor, const cgsInfoPtr& cgsPtr);
		MOHPC_NET_EXPORTS ~GameMonitor();

		HandlerList& getHandlerList();

	private:
		void fillObjectives(const ReadOnlyInfo& info, str* objectives, const char* varName, size_t count);
		void parseServerInfo(const char* cs);
		void setWarmup(const char* cs);
		void setLevelStartTime(const char* cs);
		void setMatchEnd(const char* cs);
		void setSoundtrack(const char* cs);
		void setGameVersion(const char* cs);
		void setMessage(const char* cs);
		void setSaveName(const char* cs);
		void setMotd(const char* cs);
		void setMusic(const char* cs);

	private:
		cgsInfoPtr cgs;
		HandlerList handlerList;
	};
	using GameMonitorPtr = SharedPtr<GameMonitor>;
}
}
}
