#include <MOHPC/Network/Types/Configstring.h>
#include <MOHPC/Network/Types/GameState.h>

using namespace MOHPC;
using namespace MOHPC::Network;

ConfigStringManager::ConfigStringManager()
	: stringOffsets(nullptr)
	, stringData(nullptr)
	, maxChars(0)
	, maxConfigStrings(0)
	, highestConfigstring(0)
	, dataCount(0)
{
}

ConfigStringManager::ConfigStringManager(const size_t numConfigStrings, const size_t maxCharsValue)
	: maxConfigStrings(numConfigStrings)
	, maxChars(maxCharsValue)
{
	if (maxConfigStrings && maxChars)
	{
		uint8_t* buf = new uint8_t[sizeof(size_t) * maxConfigStrings + sizeof(char) * maxChars]{ 0 };

		stringOffsets = (size_t*)buf;
		stringData = (char*)(buf + sizeof(size_t) * maxConfigStrings);

		// leave the first 0 for empty/uninitialized strings
		dataCount = 1;
	}
	else
	{
		stringOffsets = nullptr;
		stringData = nullptr;
		maxChars = maxConfigStrings = 0;
		dataCount = 0;
	}

	highestConfigstring = 0;
}

ConfigStringManager::ConfigStringManager(ConfigStringManager&& other)
	: stringOffsets(other.stringOffsets)
	, stringData(other.stringData)
	, maxChars(other.maxChars)
	, maxConfigStrings(other.maxChars)
	, highestConfigstring(other.highestConfigstring)
	, dataCount(other.dataCount)
{
	other.stringOffsets = nullptr;
	other.stringData = nullptr;
}

ConfigStringManager& ConfigStringManager::operator=(ConfigStringManager&& other)
{
	freeStrings();

	stringOffsets = other.stringOffsets;
	stringData = other.stringData;
	maxChars = other.maxChars;
	maxConfigStrings = other.maxConfigStrings;
	highestConfigstring = other.highestConfigstring;
	dataCount = other.dataCount;
	other.stringOffsets = nullptr;
	other.stringData = nullptr;

	return *this;
}

ConfigStringManager::~ConfigStringManager()
{
	freeStrings();
}

void ConfigStringManager::freeStrings()
{
	uint8_t* const buf = (uint8_t*)stringOffsets;
	if (buf)
	{
		// delete the existing buffer
		delete[] buf;
	}
}

const char* ConfigStringManager::getConfigString(csNum_t num) const
{
	if (num > maxConfigStrings) {
		throw ConfigstringErrors::MaxConfigStringException("getConfigString", num);
	}

	return &stringData[stringOffsets[num]];
}

const char* ConfigStringManager::getConfigStringChecked(csNum_t num) const
{
	return &stringData[stringOffsets[num]];
}

void ConfigStringManager::setConfigString(csNum_t num, const char* configString)
{
	setConfigString(num, configString, std::char_traits<char>::length(configString));
}

void ConfigStringManager::setConfigString(csNum_t num, const char* configString, size_t len)
{
	if (num > maxConfigStrings) {
		throw ConfigstringErrors::MaxConfigStringException("gameState_t::setConfigString", num);
	}

	size_t& stringOffset = stringOffsets[num];
  	if (stringOffset)
	{
		// existing config-string

		char* stringBuffer = &stringData[stringOffset];
		const size_t currentLen = std::char_traits<char>::length(stringBuffer);
		const size_t remainingLen = dataCount - stringOffset;
		const size_t oldDataCount = dataCount;

		if (len)
		{
			const size_t newCount = dataCount + len - currentLen;
			// only check when the length is changing
			// no need to check for empty config-string (it will always be below the length required)
			if (newCount > maxChars) {
				throw ConfigstringErrors::MaxGameStateCharsException(len);
			}

			// move everything after the actual string, to after the future string
			std::memmove(stringBuffer + len + 1, stringBuffer + currentLen + 1, remainingLen);
			std::copy(configString, configString + len, stringBuffer);
			stringBuffer[len] = 0;

			if (num >= highestConfigstring)
			{
				// the number is an higher config-string
				highestConfigstring = num;
			}

			dataCount = newCount;
		}
		else
		{
			// get rid of the string
			std::memmove(stringBuffer + len, stringBuffer + currentLen + 1, remainingLen);
			// assign to empty string
			stringOffsets[num] = 0;

			for (; highestConfigstring > 0; --highestConfigstring)
			{
				if (stringOffsets[highestConfigstring])
				{
					// found a valid one
					break;
				}
			}

			dataCount -= currentLen + 1;
		}

		// replace offsets for strings that are after the given config-string.
		const size_t diffLen = dataCount - oldDataCount;
		const size_t targetOffset = stringBuffer + currentLen + 1 - stringData;
		for (size_t i = 0; i <= highestConfigstring; ++i)
		{
			if (stringOffsets[i] >= targetOffset) {
				stringOffsets[i] += diffLen;
			}
		}
	}
	else if (len > 0)
	{
		// original string is null, append to the end of the data
		const size_t newSz = dataCount + len;

		if (newSz > maxChars) {
			throw ConfigstringErrors::MaxGameStateCharsException(len);
		}

		// position the new config-string offset after the last string
		stringOffset = dataCount;

		// now copy the input string
		char* stringBuffer = &stringData[stringOffset];
		std::copy(configString, configString + len + 1, stringBuffer);

		dataCount += len + 1;

		if (num >= highestConfigstring)
		{
			// the number is an higher config-string
			highestConfigstring = num;
		}
	}
	else
	{
		// the offset points at an empty string
		// and the given config-string is an empty string
	}
}

size_t ConfigStringManager::getMaxChars() const
{
	return maxChars;
}

size_t ConfigStringManager::getMaxConfigStrings() const
{
	return maxConfigStrings;
}

size_t ConfigStringManager::getDataCount() const
{
	return dataCount;
}

void ConfigStringManager::reset()
{
	dataCount = 1;
	highestConfigstring = 0;
	memset(stringData, 0, maxChars * sizeof(char));
	memset(stringOffsets, 0, maxConfigStrings * sizeof(size_t));
}

ConfigStringManager::iterator::iterator(const ConfigStringManager& csManValue)
	: csMan(csManValue)
	, currentNum(0)
{
	iter();
}

ConfigStringManager::iterator::~iterator()
{
}

void ConfigStringManager::iterator::iter()
{
	while (!csMan.stringOffsets[currentNum] && currentNum < csMan.maxConfigStrings)
	{
		// loop until there is a non-zero offset
		currentNum++;
	}
}

ConfigStringManager::iterator ConfigStringManager::iterator::operator++(int)
{
	++currentNum;
	iter();

	return iterator(*this);
}

ConfigStringManager::iterator& ConfigStringManager::iterator::operator++()
{
	++currentNum;
	iter();

	return *this;
}

ConfigStringManager::iterator::operator bool() const
{
	return currentNum < csMan.maxConfigStrings;
}

csNum_t ConfigStringManager::iterator::getNumber() const
{
	return currentNum;
}

const char* ConfigStringManager::iterator::getConfigString() const
{
	return csMan.getConfigStringChecked(currentNum);
}

ConfigstringErrors::MaxConfigStringException::MaxConfigStringException(const char* inName, csNum_t inConfigStringNum)
	: name(inName)
	, configStringNum(inConfigStringNum)
{}

const char* ConfigstringErrors::MaxConfigStringException::getName() const
{
	return name;
}

csNum_t ConfigstringErrors::MaxConfigStringException::getConfigstringNum() const
{
	return configStringNum;
}

str ConfigstringErrors::MaxConfigStringException::what() const
{
	return std::to_string(getConfigstringNum());
}

ConfigstringErrors::MaxGameStateCharsException::MaxGameStateCharsException(size_t inStringLen)
	: stringLen(inStringLen)
{}

size_t ConfigstringErrors::MaxGameStateCharsException::GetStringLength() const
{
	return stringLen;
}

str ConfigstringErrors::MaxGameStateCharsException::what() const
{
	return std::to_string(GetStringLength());
}

class ConfigStringTranslator_ver8 : public IConfigStringTranslator
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 5;
		maxRange = 8;
	}

	csNum_t translateFromNet(uint32_t num) const override
	{
		if (num <= CS_WARMUP || num >= 26) {
			return num;
		}

		return num - 2;
	}

	uint32_t translateToNet(csNum_t num) const override
	{
		if (num <= CS_WARMUP || num >= 26) {
			return num;
		}

		return num + 2;
	}
};

class ConfigStringTranslator_ver17 : public IConfigStringTranslator
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 15;
		maxRange = 17;
	}

	csNum_t translateFromNet(uint32_t num) const override
	{
		return num;
	}

	uint32_t translateToNet(csNum_t num) const override
	{
		return num;
	}
};

static ConfigStringTranslator_ver8 csTranslatorVersion8;
static ConfigStringTranslator_ver17 csTranslatorVersion17;
