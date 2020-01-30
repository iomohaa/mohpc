#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/SoundManager.h>

void TestSoundAlias(MOHPC::AssetManager& AM)
{
	MOHPC::SoundManager* SoundManager = AM.GetManager<MOHPC::SoundManager>();
	const MOHPC::SoundNode* Alias = SoundManager->FindAlias("30cal_leftwing");
}