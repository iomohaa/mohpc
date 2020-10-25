#pragma once

#include "../Global.h"
#include "../Common/str.h"
#include <stdint.h>

namespace MOHPC
{
	class MOHPC_EXPORTS InfoIterator
	{
	private:
		const char* keyBuffer;
		const char* endBuffer;
		char* tmpKey;
		char* tmpValue;
		const char* keyPtr;

	public:
		InfoIterator(const char* text, size_t size);
		~InfoIterator();

		const char* key() const;
		const char* value() const;

		void operator++();
		InfoIterator& operator++(int);
		bool isLast() const;
		operator bool() const;

	private:
		void parseInfo();
		void deleteTmps();
	};

	class MOHPC_EXPORTS ReadOnlyInfo
	{
	private:
		const char* keyBuffer;
		size_t size;

	public:
		ReadOnlyInfo();
		ReadOnlyInfo(const char* existingBuffer, size_t len = 0);
		ReadOnlyInfo(const ReadOnlyInfo&) = delete;
		ReadOnlyInfo& operator=(const ReadOnlyInfo&) = delete;
		ReadOnlyInfo(ReadOnlyInfo&& other) noexcept;
		ReadOnlyInfo& operator=(ReadOnlyInfo&& other) noexcept;

		str ValueForKey(const char* key) const;
		const char* ValueForKey(const char* key, size_t& outLen) const;
		uint32_t IntValueForKey(const char* key) const;
		uint64_t LongValueForKey(const char* key) const;

		const char* GetString() const;
		size_t GetInfoLength() const;

		InfoIterator createConstIterator() const;
	};

	class MOHPC_EXPORTS Info
	{
	private:
		char* keyBuffer;
		size_t alloced;
		size_t size;

	public:
		Info();
		Info(const char* existingBuffer);
		Info(Info&& info);
		~Info();
		Info& operator=(Info&& info);

		void SetValueForKey(const char* key, const char* value);
		void RemoveKey(const char* key);
		str ValueForKey(const char* key) const;
		uint32_t IntValueForKey(const char* key) const;
		uint64_t LongValueForKey(const char* key) const;

		const char* GetString() const;
		size_t GetInfoLength() const;

		InfoIterator createConstIterator() const;
	};

	class JsonStyleBeautifier
	{
	public:
		static void writeScopeBegin(str& json)
		{
			json += "\n";
		}

		static void writeScopeEnd(str& json)
		{
			json += "\n";
		}

		static void writeObjectBegin(str& json)
		{
			json += "  ";
		}

		static void writeObjectEndLine(str& json)
		{
			json += "\n";
		}

		static void writeLastObjectEndLine(str& json)
		{
		}

		static void writeObjectBeginValue(str& json)
		{
			json += " ";
		}
	};

	class JsonStyleCompressed
	{
	public:
		static void writeScopeBegin(str& json) {}
		static void writeScopeEnd(str& json) {}
		static void writeObjectBegin(str& json) {}
		static void writeObjectEndLine(str& json) {}
		static void writeLastObjectEndLine(str& json) {}
		static void writeObjectBeginValue(str& json) {}
	};

	/**
	 * A class used to convert info data into a json data.
	 */
	class InfoJson
	{
	public:
		/**
		 * Convert the input info into a json string.
		 */
		template<typename JsonStyle>
		static str toJson(const ReadOnlyInfo& info)
		{
			return toJson<JsonStyle>(info.GetString(), info.GetInfoLength());
		}

		template<typename JsonStyle>
		static str toJson(const Info& info)
		{
			return toJson<JsonStyle>(info.GetString(), info.GetInfoLength());
		}

	private:
		template<typename JsonStyle>
		static str toJson(const char* infoString, size_t length)
		{
			str json;

			json.reserve(length);
			json += "{";

			JsonStyle::writeScopeBegin(json);

			for (InfoIterator it(infoString, length); it; ++it)
			{
				JsonStyle::writeObjectBegin(json);

				// write key
				json += "\"";
				json += it.key();
				json += "\"";

				json += ":";
				JsonStyle::writeObjectBeginValue(json);

				// write value
				json += "\"";
				json += it.value();
				json += "\"";

				if (!it.isLast())
				{
					json += ",";
					JsonStyle::writeObjectEndLine(json);
				}
				else {
					JsonStyle::writeLastObjectEndLine(json);
				}
			}

			JsonStyle::writeScopeEnd(json);

			json += "}";

			return json;
		}
	};
}
