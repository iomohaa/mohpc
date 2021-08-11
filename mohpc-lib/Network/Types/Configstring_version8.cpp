#include <MOHPC/Network/Types/Configstring.h>

using namespace MOHPC;
using namespace MOHPC::Network;

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
		if (num <= CS::WARMUP || num >= 26) {
			return num;
		}

		return num - 2;
	}

	uint32_t translateToNet(csNum_t num) const override
	{
		if (num <= CS::WARMUP || num >= 26) {
			return num;
		}

		return num + 2;
	}
};

static ConfigStringTranslator_ver8 csTranslatorVersion8;
