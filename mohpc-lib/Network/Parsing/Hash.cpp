#include <MOHPC/Network/Parsing/Hash.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::Parsing;

class Hash8 : public Parsing::IHash
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 5;
		maxRange = 8;
	}

	uint32_t hashKey(const char* string, size_t maxlen) const override
	{
		uint32_t hash = 0;

		for (size_t i = 0; i < maxlen && string[i]; i++) {
			hash += string[i] * (119 + (uint32_t)i);
		}

		hash = (hash ^ (hash >> 10) ^ (hash >> 20));
		return hash;
	}
};

class Hash17 : public Hash8
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 15;
		maxRange = 17;
	}

	uint32_t hashKey(const char* string, size_t maxlen) const override
	{
		uint32_t hash = 0;

		for (size_t i = 0; i < maxlen && string[i]; i++) {
			hash += string[i] * (119 + (uint32_t)i);
		}

		hash = (hash ^ ((hash ^ (hash >> 10)) >> 10));
		return hash;
	}
};

using HashDefault = ProtocolSingletonInherit<Hash17, 0>;

static Hash8 hashVersion8;
static Hash17 hashVersion17;
