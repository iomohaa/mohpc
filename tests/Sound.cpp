#include <MOHPC/Assets/Formats/Sound.h>
#include "Common/Common.h"

int main(int argc, const char* argv[])
{
	const MOHPC::AssetManagerPtr AM = AssetLoad();

	MOHPC::SoundPtr Sound = AM->LoadAsset<MOHPC::Sound>("/sound/weapons/Foley/aagun_snd_off.wav");
	MOHPC::SoundPtr Music = AM->LoadAsset<MOHPC::Sound>("/sound/music/mus_MainTheme.mp3");
	assert(Sound && Music);

	return 0;
}
