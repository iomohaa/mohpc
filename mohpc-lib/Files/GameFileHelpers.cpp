#include <MOHPC/Files/GameFileHelpers.h>
#include <MOHPC/Files/Managers/FileManager.h>
#include <MOHPC/Common/Log.h>

using namespace MOHPC;

static constexpr char MOHPC_LOG_NAMESPACE[] = "file_game_helpers";

#ifdef _WIN32
static constexpr unsigned char PLATFORM_SLASH = '\\';
#define PLATFORM_SLASH_MACRO "\\"
#else
static constexpr unsigned char PLATFORM_SLASH = '/';
#define PLATFORM_SLASH_MACRO "/"
#endif

static bool HasTrailingSlash(const char* dir)
{
	const size_t dirLen = strlen(dir);

	const char last = dir[dirLen - 1];
	return last == PLATFORM_SLASH;
}

bool FileHelpers::FillGameDirectory(FileManager& fm, const char* directory)
{
	str gameDir;
	str gameDirStr;

	MOHPC_LOG(Info, "Loading pak files...");

	if (!HasTrailingSlash(directory))
	{
		gameDirStr = directory;
		gameDirStr += '/';
		gameDir = gameDirStr;
	}
	else {
		gameDir = directory;
	}

#define PSL PLATFORM_SLASH_MACRO

	bool success = true;
	success &= fm.AddGameDirectory((gameDir + "main").c_str(), "AA");
	success &= fm.AddPakFile((gameDir + "main" + PSL "Pak0.pk3").c_str(), "AA");
	success &= fm.AddPakFile((gameDir + "main" + PSL "Pak1.pk3").c_str(), "AA");
	success &= fm.AddPakFile((gameDir + "main" + PSL "Pak2.pk3").c_str(), "AA");
	success &= fm.AddPakFile((gameDir + "main" + PSL "Pak3.pk3").c_str(), "AA");
	success &= fm.AddPakFile((gameDir + "main" + PSL "Pak4.pk3").c_str(), "AA");
	success &= fm.AddPakFile((gameDir + "main" + PSL "Pak5.pk3").c_str(), "AA");
	success &= fm.AddPakFile((gameDir + "main" + PSL "Pak6.pk3").c_str(), "AA");
	success &= fm.AddPakFile((gameDir + "mainta" + PSL "Pak1.pk3").c_str(), "SH");
	success &= fm.AddPakFile((gameDir + "mainta" + PSL "Pak2.pk3").c_str(), "SH");
	success &= fm.AddPakFile((gameDir + "mainta" + PSL "Pak3.pk3").c_str(), "SH");
	success &= fm.AddPakFile((gameDir + "mainta" + PSL "Pak4.pk3").c_str(), "SH");
	success &= fm.AddPakFile((gameDir + "mainta" + PSL "Pak5.pk3").c_str(), "SH");
	success &= fm.AddPakFile((gameDir + "maintt" + PSL "Pak1.pk3").c_str(), "BT");
	success &= fm.AddPakFile((gameDir + "maintt" + PSL "Pak2.pk3").c_str(), "BT");
	success &= fm.AddPakFile((gameDir + "maintt" + PSL "Pak3.pk3").c_str(), "BT");
	success &= fm.AddPakFile((gameDir + "maintt" + PSL "Pak4.pk3").c_str(), "BT");

	MOHPC_LOG(Info, "%d pak(s) loaded in %s.", fm.GetNumPakFiles(), directory);

	return success;
}
