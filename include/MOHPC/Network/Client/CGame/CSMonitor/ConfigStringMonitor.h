#pragma once

#include "../Snapshot.h"
#include "../../../Configstring.h"
#include "../../../../Utility/HandlerList.h"

#include <map>

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		namespace Handlers
		{
			/**
			 * Called when all configstrings has been modified and handled.
			 */
			struct ConfigstringHandled : public HandlerNotifyBase<void()> {};
		}

		class ConfigstringMonitor
		{
		public:
			ConfigstringMonitor(const SnapshotProcessorPtr& snapshotProcessor);
			virtual ~ConfigstringMonitor();

			FunctionList<Handlers::ConfigstringHandled>& getHandler();

		protected:
			/**
			 * Callback for when a config-string is modified.
			 *
			 * @param num Configstring number.
			 * @param cs Configstring value.
			 */
			virtual bool configStringModified(csNum_t num, const char* cs) = 0;

		private:
			void configStringModifiedInternal(csNum_t num, const char* cs);

		private:
			SnapshotProcessorPtr snapshotProcessor;
			fnHandle_t csHandle;
			FunctionList<Handlers::ConfigstringHandled> handler;
		};

		template<typename T>
		class ConfigstringMonitorTemplate : public ConfigstringMonitor
		{
		public:
			ConfigstringMonitorTemplate(const SnapshotProcessorPtr& snapshotProcessorPtr)
				: ConfigstringMonitor(snapshotProcessorPtr)
			{}

			bool configStringModified(csNum_t num, const char* cs) override final
			{
				const auto callback = T::callbackMap.find(num);
				if (callback != T::callbackMap.end())
				{
					// can call it
					Callback c = callback->second;
					(static_cast<T*>(this)->*c)(cs);

					// notify about the configstring change
					return true;
				}

				return false;
			}

		private:
			using Callback = void (T::*)(const char* cs);
			static std::map<csNum_t, Callback> callbackMap;
		};
	}
}
}