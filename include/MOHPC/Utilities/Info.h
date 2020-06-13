#pragma once

#include "../Global.h"
#include "../Script/str.h"
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
		str ValueForKey(const char* key);

		const char* GetString() const;
		size_t GetInfoLength() const;

		InfoIterator createConstIterator() const;
	};
}
