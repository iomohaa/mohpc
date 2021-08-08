#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Assets/Managers/SoundManager.h>
#include "Common/Common.h"

#include <cassert>

int main(int argc, const char* argv[])
{
	const MOHPC::AssetManagerPtr AM = AssetLoad(GetGamePathFromCommandLine(argc, argv));

	MOHPC::SoundManagerPtr SoundManager = AM->GetManager<MOHPC::SoundManager>();

	const MOHPC::SoundNode* Alias = SoundManager->FindAlias("30cal_leftwing");
	assert(Alias);
}
