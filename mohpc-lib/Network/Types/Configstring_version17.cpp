#include <MOHPC/Network/Types/Configstring.h>

using namespace MOHPC;
using namespace MOHPC::Network;

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

static ConfigStringTranslator_ver17 csTranslatorVersion17;
