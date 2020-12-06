#include <MOHPC/Formats/Sound.h>
#include "UnitTest.h"

class CSoundUnitTest : public IUnitTest
{
public:
	long priority() override
	{
		return 0;
	}

	const char* name() override
	{
		return "Sound";
	}

	void run(const MOHPC::AssetManagerPtr& AM) override
	{
		MOHPC::SoundPtr Sound = AM->LoadAsset<MOHPC::Sound>("/sound/weapons/Foley/aagun_snd_off.wav");
		MOHPC::SoundPtr Music = AM->LoadAsset<MOHPC::Sound>("/sound/music/mus_MainTheme.mp3");
		assert(Sound != nullptr);
	}
};
static CSoundUnitTest soundUnitTest;
