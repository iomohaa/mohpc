#pragma once

#include "ConfigStringMonitor.h"
#include "../Objective.h"
#include "../../../../Utility/SharedPtr.h"
#include "../../../../Utility/HandlerList.h"

namespace MOHPC
{
namespace Network
{
class ServerGameState;

namespace CGame
{
	namespace Handlers
	{
		struct ReceivedModel : public HandlerNotifyBase<void(const char* modelName)> {};
		struct ReceivedSound : public HandlerNotifyBase<void(const char* soundName)> {};
		struct ReceivedImage : public HandlerNotifyBase<void(const char* imageName)> {};
		struct ReceivedLightStyle : public HandlerNotifyBase<void(const char* lightStyleName)> {};
	}
	/**
	 * Monitors name of assets that are sent by the server through configString.
	 * Useful to precache assets like models.
	 */
	class AssetsMonitor : public ConfigstringMonitor
	{
		MOHPC_NET_OBJECT_DECLARATION(AssetsMonitor);

		struct HandlerList
		{
			FunctionList<Handlers::ReceivedModel> modelHandler;
			FunctionList<Handlers::ReceivedSound> soundHandler;
			FunctionList<Handlers::ReceivedImage> imageHandler;
			FunctionList<Handlers::ReceivedLightStyle> lightStyleHandler;
		};

	public:
		MOHPC_NET_EXPORTS AssetsMonitor(const SnapshotProcessorPtr& snapshotProcessor);
		MOHPC_NET_EXPORTS ~AssetsMonitor();

		HandlerList& getHandlerList();

		bool configStringModified(csNum_t num, const char* cs) override;

	private:
		void modelModified(size_t index, const char* value);
		void soundModified(size_t index, const char* value);
		void imageModified(size_t index, const char* value);
		void lightStyleModified(size_t index, const char* value);

	private:
		HandlerList handlerList;
	};

	using AssetsMonitorPtr = SharedPtr<AssetsMonitor>;
}
}
}
