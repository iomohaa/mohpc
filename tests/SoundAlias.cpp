#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/SoundManager.h>
#include "UnitTest.h"

class CSoundAliasTest : public IUnitTest, public TAutoInst<CSoundAliasTest>
{
public:
	virtual const char* name() override
	{
		return "Sound alias";
	}

	virtual void run(MOHPC::AssetManager& AM) override
	{
		MOHPC::SoundManager* SoundManager = AM.GetManager<MOHPC::SoundManager>();
		const MOHPC::SoundNode* Alias = SoundManager->FindAlias("30cal_leftwing");
	}
};
