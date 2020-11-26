#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Asset.h>

void ExampleCode()
{
	MOHPC::AssetManagerPtr AM = MOHPC::AssetManager::create();

	// Retrieves the file manager associated with the asset manager
	// So we can open a file
	MOHPC::FileManager* FM = AM->GetFileManager();

	// Adds up the main folder from MOHAA
	FM->AddGameDirectory("C:\\Games\\MOHPC\\main");

	// Parses pak0
	FM->AddPakFile("C:\\Games\\MOHPC\\Pak0.pk3");

	// Parses pak1 : Similar files present in pak0 and pak6 will be overriden by pak1
	FM->AddPakFile("C:\\Games\\MOHPC\\Pak6.pk3");

	// Open bar.tik, it will be the bar.tik from Pak6, because it is lastly present in pak6 rather than pak0
	MOHPC::FilePtr File = FM->OpenFile("models/weapons/bar.tik");

	char* Buffer = nullptr;

	// Puts the content of bar.tik into a buffer...
	uint64_t Size = File->ReadBuffer((void**)&Buffer);

	// No need to close the file, shared_ptr will do it after auto-destruction
	// AssetManager will automatically destroy everything once it begins destruction.
}
