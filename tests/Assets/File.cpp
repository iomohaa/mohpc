#include "Common/Common.h"
#include <MOHPC/Files/Managers/FileManager.h>

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

int main(int argc, const char* argv[])
{
	InitCommon();

	const AssetManagerPtr AM = AssetLoad(GetGamePathFromCommandLine(argc, argv));
	const FileManagerPtr& FM = AM->GetFileManager();
	FileEntryList entries = FM->ListFilteredFiles("/", "");
	assert(entries.GetNumFiles() > 0);

	assert(FM->FileExists("/default.cfg"));
	assert(FM->FileExists("default.cfg"));
	assert(FM->OpenFile("/default.cfg"));
	assert(FM->OpenFile("default.cfg"));

	FileEntryList cfgEntries = FM->ListFilteredFiles("/", "cfg");
	assert(cfgEntries.GetNumFiles() > 0);

	assert(FM->FileExists("/newconfig.cfg", false));
	assert(FM->FileExists("newconfig.cfg", false));
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

	testExtensions();
}
