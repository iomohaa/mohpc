#include <MOHPC/Utility/Info.h>

#include <algorithm>
#include <cstring>

using namespace MOHPC;

Info::Info()
	: keyBuffer(nullptr)
	, size(0)
	, alloced(0)
{
}

Info::Info(Info&& info)
{
	*this = std::move(info);
}

Info::~Info()
{
	if (keyBuffer) delete[] keyBuffer;
}

Info& Info::operator=(Info&& info)
{
	// move values out
	keyBuffer = info.keyBuffer;
	size = info.size;
	alloced = info.alloced;

	// empty out the older instance
	info.keyBuffer = nullptr;
	info.size = info.alloced = 0;

	return *this;
}

void Info::SetValueForKey(const char* key, const char* value)
{
	if (!key || !*key) {
		return;
	}

	// append at the end
	const size_t keyLen = strHelpers::len(key);

	// remove existing key
	RemoveKey(key, keyLen);

	const size_t valueLen = strHelpers::len(value);
	const size_t sizeNoNul = (size ? size - 1 : 0);
	const size_t newsz = sizeNoNul + 1 + keyLen + 1 + valueLen + 1;

	if (newsz > alloced)
	{
		// allocate a new buffer
		char* newBuf = new char[newsz];

		alloced = newsz;

		if (keyBuffer)
		{
			// copy without the null-terminated character
			std::memcpy(newBuf, keyBuffer, size - 1);
			delete[] keyBuffer;
		}

		keyBuffer = newBuf;
	}

	char* pbuf = keyBuffer + sizeNoNul;
	// copy the key
	*(pbuf++) = '\\';
	std::memcpy(pbuf, key, keyLen);
	pbuf += keyLen;

	// copy the value
	*(pbuf++) = '\\';
	std::memcpy(pbuf, value, valueLen + 1);

	size = newsz;
}

void Info::RemoveKey(const char* key)
{
	RemoveKey(key, strHelpers::len(key));
}

void Info::RemoveKey(const char* key, size_t keyLen)
{
	if (!keyBuffer) {
		return;
	}

	if (strchr(key, '\\')) {
		return;
	}

	char* pkey = keyBuffer;

	while(*pkey)
	{
		char* startKey = pkey;
		if (*pkey == '\\') pkey++;

		char* endKey = pkey;
		while (*endKey && *endKey != '\\') ++endKey;

		char* startValue = endKey + 1;
		char* endValue = startValue;

		while (*endValue && *endValue != '\\') ++endValue;

		const size_t ckeyLen = endKey - pkey;
		if (keyLen == ckeyLen && !strHelpers::cmpn(pkey, key, ckeyLen))
		{
			const size_t len = endValue - startKey;
			std::memmove(startKey, endValue, keyBuffer + size - endValue);
			size -= len;

			break;
		}

		pkey = endValue;
	}
}

str Info::ValueForKey(const char* key) const
{
	char* pkey = keyBuffer;

	while (*pkey)
	{
		char* startKey = pkey;
		if (*pkey == '\\') pkey++;

		char* endKey = pkey;
		while (*endKey && *endKey != '\\') ++endKey;

		char* startValue = endKey + 1;
		char* endValue = startValue;

		while (*endValue && *endValue != '\\') ++endValue;

		if (!strHelpers::cmpn(pkey, key, endKey - pkey))
		{
			return str(
				startValue,
				0,
				endValue - startValue
			);
		}

		pkey = endValue;
	}

	return str();
}

bool MOHPC::Info::BoolValueForKey(const char* key) const
{
	return atoi(ValueForKey("key").c_str()) ? true : false;
}

uint32_t Info::IntValueForKey(const char* key) const
{
	return atoi(ValueForKey("key").c_str());
}

uint64_t Info::LongValueForKey(const char* key) const
{
	return atoll(ValueForKey("key").c_str());
}

const char* Info::GetString() const
{
	return keyBuffer;
}

size_t Info::GetInfoLength() const
{
	return size;
}

InfoIterator Info::createConstIterator() const
{
	return InfoIterator(keyBuffer, size);
}

ReadOnlyInfo::ReadOnlyInfo(const char* existingBuffer, size_t len)
	: keyBuffer(existingBuffer)
{
	if (!len && *existingBuffer) {
		size = (strHelpers::len(keyBuffer) + 1);
	}
	else {
		size = len;
	}
}

ReadOnlyInfo::ReadOnlyInfo(ReadOnlyInfo&& other) noexcept
{
	keyBuffer = other.keyBuffer;
	size = other.size;
}

ReadOnlyInfo::ReadOnlyInfo()
	: keyBuffer(nullptr)
	, size(0)
{
}

ReadOnlyInfo& ReadOnlyInfo::operator=(ReadOnlyInfo&& other) noexcept
{
	keyBuffer = other.keyBuffer;
	size = other.size;
	return *this;
}

str ReadOnlyInfo::ValueForKey(const char* key) const
{
	size_t keyLen;
	const char* foundKey = ValueForKey(key, keyLen);

	if(keyLen)
	{
		return str(
			foundKey,
			0,
			keyLen
		);
	}

	return str();
}

const char* ReadOnlyInfo::ValueForKey(const char* key, size_t& outLen) const
{
	const char* pkey = keyBuffer;
	const char* endBuf = keyBuffer + size;

	const size_t myKeyLen = strHelpers::len(key);

	outLen = 0;

	while (pkey < endBuf)
	{
		const char* startKey = pkey;
		while (*pkey == '\\') pkey++;

		const char* endKey = pkey;
		while (endKey < endBuf && *endKey != '\\') ++endKey;

		const char* startValue = endKey + 1;
		const char* endValue = startValue;

		while (endValue < endBuf && *endValue != '\\') ++endValue;

		const size_t keyLen = endKey - pkey;
		if (keyLen > 0)
		{
			if (keyLen == myKeyLen && !strHelpers::cmpn(pkey, key, keyLen))
			{
				outLen = endValue - startValue;
				return startValue;
			}
		}

		pkey = endValue;
	}

	return "";
}

bool ReadOnlyInfo::BoolValueForKey(const char* key) const
{
	return atoi(ValueForKey("key").c_str()) ? true : false;
}

uint32_t ReadOnlyInfo::IntValueForKey(const char* key) const
{
	return atoi(ValueForKey(key).c_str());
}

uint64_t ReadOnlyInfo::LongValueForKey(const char* key) const
{
	return atoll(ValueForKey(key).c_str());
}

const char* ReadOnlyInfo::GetString() const
{
	return keyBuffer;
}

size_t ReadOnlyInfo::GetInfoLength() const
{
	return size;
}

InfoIterator ReadOnlyInfo::createConstIterator() const
{
	return InfoIterator(keyBuffer, size);
}

InfoIterator::InfoIterator(const char* text, size_t size)
	: keyBuffer(text)
	, tmpKey(nullptr)
	, tmpValue(nullptr)
{
	keyPtr = keyBuffer;
	while (*keyPtr && *keyPtr == '\\') ++keyPtr;
	endBuffer = keyBuffer + size;
	parseInfo();
}

InfoIterator::~InfoIterator()
{
	deleteTmps();
}

const char* InfoIterator::key() const
{
	return tmpKey;
}

const char* InfoIterator::value() const
{
	return tmpValue;
}

void InfoIterator::operator++()
{
	parseInfo();
}

InfoIterator& InfoIterator::operator++(int)
{
	parseInfo();
	return *this;
}

InfoIterator::operator bool() const
{
	return tmpKey != nullptr;
}

void InfoIterator::parseInfo()
{
	if (isLast())
	{
		deleteTmps();
		tmpKey = tmpValue = nullptr;
		return;
	}

	// Get key
	const char* p1 = keyPtr;
	const char* end_p1 = p1;

	while (end_p1 < endBuffer && *end_p1 != '\\') ++end_p1;

	// Get value
	const char* p2 = end_p1 < endBuffer ? end_p1 + 1 : end_p1;
	const char* end_p2 = p2;

	while (end_p2 < endBuffer && *end_p2 != '\\') ++end_p2;

	// Get new index
	keyPtr = (end_p2 < endBuffer) ? end_p2 + 1 : end_p2;

	// Delete previously allocated string
	deleteTmps();

	const size_t p1len = end_p1 - p1;
	const size_t p2len = end_p2 - p2;

	tmpKey = new char[p1len + 1];
	std::memcpy(tmpKey, p1, p1len);
	tmpKey[p1len] = 0;

	tmpValue = new char[p2len + 1];
	std::memcpy(tmpValue, p2, p2len);
	tmpValue[p2len] = 0;
}

void InfoIterator::deleteTmps()
{
	if (tmpKey) delete tmpKey;
	if (tmpValue) delete tmpValue;
}

bool InfoIterator::isLast() const
{
	return keyPtr >= endBuffer;
}
