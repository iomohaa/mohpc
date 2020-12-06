#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/SoundManager.h>
#include "UnitTest.h"

class CSoundAliasTest : public IUnitTest
{
public:
	virtual const char* name() override
	{
		return "Sound alias";
	}

	virtual long priority() override
	{
		return 0;
	}

	virtual void run(const MOHPC::AssetManagerPtr& AM) override
	{
		MOHPC::SoundManagerPtr SoundManager = AM->GetManager<MOHPC::SoundManager>();
		const MOHPC::SoundNode* Alias = SoundManager->FindAlias("30cal_leftwing");
	}
};
static CSoundAliasTest unitTest;
