#include <MOHPC/Assets/Formats/Sound.h>
#include <MOHPC/Files/Managers/IFileManager.h>
#include "Common/Common.h"

#include <cassert>

int main(int argc, const char* argv[])
{
	InitCommon(argc, argv);

	const MOHPC::AssetManagerPtr AM = AssetLoad(GetGamePathFromCommandLine());

	MOHPC::ExtensionList ExtensionsStd;
	ExtensionsStd.push_back("wav");

	MOHPC::FileEntryList FilesStd = AM->GetFileManager()->ListFilteredFiles("/", ExtensionsStd, true);

	MOHPC::SoundPtr Sound = AM->readAsset<MOHPC::SoundReader>("/sound/weapons/Foley/aagun_snd_off.wav");
	MOHPC::SoundPtr Music = AM->readAsset<MOHPC::SoundReader>("/sound/music/mus_MainTheme.mp3");
	assert(Sound);
	assert(Music);

	return 0;
}
