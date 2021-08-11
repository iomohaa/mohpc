#pragma once

#include "../Snapshot.h"
#include "../../../Configstring.h"

#include <map>

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		class ConfigstringMonitor
		{
		public:
			ConfigstringMonitor(const SnapshotProcessorPtr& snapshotProcessor);
			virtual ~ConfigstringMonitor();

		protected:
			/**
			 * Callback for when a config-string is modified.
			 *
			 * @param num Configstring number.
			 * @param cs Configstring value.
			 */
			virtual void configStringModified(csNum_t num, const char* cs) = 0;

		private:
			SnapshotProcessorPtr snapshotProcessor;
			fnHandle_t csHandle;
		};

		template<typename T>
		class ConfigstringMonitorTemplate : public ConfigstringMonitor
		{
		public:
			ConfigstringMonitorTemplate(const SnapshotProcessorPtr& snapshotProcessorPtr)
				: ConfigstringMonitor(snapshotProcessorPtr)
			{}

			void configStringModified(csNum_t num, const char* cs) override final
			{
				const auto callback = T::callbackMap.find(num);
				if (callback != T::callbackMap.end())
				{
					// can call it
					Callback c = callback->second;
					(static_cast<T*>(this)->*c)(cs);
				}
			}

		private:
			using Callback = void (T::*)(const char* cs);
			static std::map<csNum_t, Callback> callbackMap;
		};
	}
}
}