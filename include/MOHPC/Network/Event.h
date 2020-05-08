#pragma once

#include "../Global.h"
#include <stdint.h>
#include "../Script/Container.h"

namespace MOHPC
{
	class str;

	namespace Network
	{
		/**
		 * Class for a parameter
		 */
		class MOHPC_EXPORTS EventData
		{
		public:
			enum class dataType_e : uint8_t
			{
				None,
				Integer,
				String,
				Pointer
			};

		private:
			dataType_e dataType;
			union {
				str* stringValue;
				uint32_t intValue;
				void* voidValue;
			} data;

		public:
			EventData(int32_t value);
			EventData(const char* value);
			~EventData();


			EventData(EventData&& other);
			EventData& operator=(EventData&& other);
			EventData(const EventData& other) = delete;
			EventData& operator=(const EventData& other) = delete;

			int32_t GetInteger() const;
			str GetString() const;
		};

		/**
		 * Parameters from a network event
		 */
		class Event
		{
		private:
			Container<EventData> data;

		public:
			MOHPC_EXPORTS Event();

			MOHPC_EXPORTS void AddInteger(int32_t value);
			MOHPC_EXPORTS void AddString(const char* value);

			MOHPC_EXPORTS int32_t GetInteger(size_t index) const;
			MOHPC_EXPORTS str GetString(size_t index) const;
		};

		class INetworkResponse
		{
		public:
			virtual ~INetworkResponse() = default;
		};
	}
}
