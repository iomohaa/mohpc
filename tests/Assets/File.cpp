#include "Common/Common.h"
#include <MOHPC/Files/Managers/IFileManager.h>
#include <MOHPC/Files/Managers/PakFileManager.h>
#include <MOHPC/Files/Managers/SystemFileManager.h>
#include <MOHPC/Files/Category.h>

#include <cassert>
#include <string>

using namespace MOHPC;

void testExtensions()
{
	const char* ext = FileHelpers::getExtension("file.ext");
	assert(!strHelpers::icmp(ext, "ext"));

	const std::wstring extU = FileHelpers::getExtension<std::wstring>("file.extuni");
	assert(!strHelpers::icmp(extU.c_str(), L"extuni"));
}

void testCategories()
{
	const AssetManagerPtr AM = AssetLoad(GetGamePathFromCommandLine());
	const IFileManagerPtr& FM = AM->GetFileManager();
	const FileCategoryManagerPtr& catMan = AM->GetFileCategoryManager();

	const FileCategory* category = catMan->findCategory("BT");
	assert(FM->FileExists("models/weapons/enfield_lite.tik", category));
	assert(FM->FileExists("models/weapons/enfield_lite.tik"));

	category = catMan->findCategory("BT_240");
	assert(FM->FileExists("models/weapons/enfield_lite.tik", category));
	assert(FM->FileExists("models/weapons/It_W_Breda.tik", category));
	assert(!FM->FileExists("models/static/v2.tik", category));

	category = catMan->findCategory("AA");
	assert(FM->FileExists("models/static/v2.tik", category));
}

void testFiles()
{
	const AssetManagerPtr AM = AssetLoad(GetGamePathFromCommandLine());
	const IFileManagerPtr& FM = AM->GetFileManager();
	FileEntryList entries = FM->ListAllFilteredFiles("/");
	assert(entries.GetNumFiles() > 0);

	assert(FM->FileExists("/default.cfg"));
	assert(FM->FileExists("default.cfg"));
	assert(FM->OpenFile("/default.cfg"));
	assert(FM->OpenFile("default.cfg"));

	FileEntryList cfgEntries = FM->ListFilteredFiles("/", "cfg");
	assert(cfgEntries.GetNumFiles() > 0);

	assert(FM->FileExists("/newconfig.cfg"));
	assert(FM->FileExists("newconfig.cfg"));
	assert(FM->OpenFile("/newconfig.cfg"));
	assert(FM->OpenFile("newconfig.cfg"));

	FileEntryList script1 = FM->ListFilteredFiles("/scripts", "");
	FileEntryList script2 = FM->ListFilteredFiles("scripts", "");
	FileEntryList script3 = FM->ListFilteredFiles("scripts/", "");
	FileEntryList script4 = FM->ListFilteredFiles("/scripts/", "");

	assert(script1.GetNumFiles() > 0);
	assert(script2.GetNumFiles() == script1.GetNumFiles());
	assert(script3.GetNumFiles() == script1.GetNumFiles());
	assert(script4.GetNumFiles() == script1.GetNumFiles());

	ExtensionList extension{ "bsp", "scr" };
	entries = FM->ListFilteredFiles("maps", extension);
}

int main(int argc, const char* argv[])
{
	InitCommon(argc, argv);

	testCategories();
	testFiles();
	testExtensions();
}
