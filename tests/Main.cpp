#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/FileManager.h>

extern void TestAnimRendering(MOHPC::AssetManager& AM);
extern void TestCDKey();
extern void TestEmitter(MOHPC::AssetManager& AM);
extern void TestLevel(MOHPC::AssetManager& AM);
extern void TestShader(MOHPC::AssetManager& AM);
extern void TestSoundAlias(MOHPC::AssetManager& AM);
extern void TestTiki(MOHPC::AssetManager& AM);

int main(int argc, char *argv[])
{
#ifdef _MSC_VER
	// Get current flag
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

	// Debug allocation
	tmpFlag |= _CRTDBG_ALLOC_MEM_DF;

	// Leak detection
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

	// Memory check (very slow)
	//tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;

	// Check for _CRT_BLOCK memory (internal CRT memory)
	//tmpFlag |= _CRTDBG_CHECK_CRT_DF;

	// Set flag to the new value.
	_CrtSetDbgFlag(tmpFlag);
#endif

	{
		MOHPC::AssetManager AM;
		MOHPC::FileManager* FM = AM.GetFileManager();

		printf("Loading pak files...\n");

#ifdef _WIN32
		FM->AddGameDirectory("I:\\Jeux\\Mohaa\\main", "AA");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\main\\Pak0.pk3", "AA");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\main\\Pak1.pk3", "AA");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\main\\Pak2.pk3", "AA");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\main\\Pak3.pk3", "AA");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\main\\Pak4.pk3", "AA");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\main\\Pak5.pk3", "AA");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\main\\Pak6.pk3", "AA");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\mainta\\Pak1.pk3", "SH");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\mainta\\Pak2.pk3", "SH");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\mainta\\Pak3.pk3", "SH");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\mainta\\Pak4.pk3", "SH");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\mainta\\Pak5.pk3", "SH");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\maintt\\Pak1.pk3", "BT");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\maintt\\Pak2.pk3", "BT");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\maintt\\Pak3.pk3", "BT");
		FM->AddPakFile("I:\\Jeux\\Mohaa\\maintt\\Pak4.pk3", "BT");
#else
		FM->AddGameDirectory("/mnt/i/Jeux/Mohaa/main");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/main/Pak0.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/main/Pak1.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/main/Pak2.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/main/Pak3.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/main/Pak4.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/main/Pak5.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/main/Pak6.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/mainta/Pak1.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/mainta/Pak2.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/mainta/Pak3.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/mainta/Pak4.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/mainta/Pak5.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/maintt/Pak1.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/maintt/Pak2.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/maintt/Pak3.pk3");
		FM->AddPakFile("/mnt/i/Jeux/Mohaa/maintt/Pak4.pk3");
#endif

		MOHPC::FileEntryList entries = FM->ListFilteredFiles("/", "", true, false);

		TestShader(AM);
		TestCDKey();
		TestEmitter(AM);
		TestAnimRendering(AM);
		TestTiki(AM);
		TestSoundAlias(AM);
		TestLevel(AM);
	}
	
#ifdef _WIN32
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}
