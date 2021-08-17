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

#define PSL PLATFORM_SLASH_MACRO

template<typename T>
static bool HasTrailingSlash(const T* dir)
{
	const size_t dirLen = strHelpers::len(dir);

	const T last = dir[dirLen - 1];
	return last == PLATFORM_SLASH;
}

struct pathCategory_t
{
	const char* path;
	const char* categoryName;
};

bool FileHelpers::FillGameDirectory(FileManager& fm, const char* directory)
{
	const pathCategory_t gameDirs[] =
	{
		{ "main", "AA" },
		{ "mainta", "SH" },
		{ "maintt", "BT" }
	};

	const pathCategory_t defaultPaks[] =
	{
		{ "main" PSL "Pak0.pk3", "AA" },
		{ "main" PSL "Pak1.pk3", "AA" },
		{ "main" PSL "Pak2.pk3", "AA" },
		{ "main" PSL "Pak3.pk3", "AA" },
		{ "main" PSL "Pak4.pk3", "AA" },
		{ "main" PSL "Pak5.pk3", "AA" },
		{ "main" PSL "Pak6.pk3", "AA" },
		{ "mainta" PSL "Pak1.pk3", "SH" },
		{ "mainta" PSL "Pak2.pk3", "SH" },
		{ "mainta" PSL "Pak3.pk3", "SH" },
		{ "mainta" PSL "Pak4.pk3", "SH" },
		{ "maintt" PSL "Pak1.pk3", "BT" },
		{ "maintt" PSL "Pak2.pk3", "BT" },
		{ "maintt" PSL "Pak3.pk3", "BT" },
		{ "maintt" PSL "Pak4.pk3", "BT" }
	};

	constexpr size_t numDirs = sizeof(gameDirs) / sizeof(gameDirs[0]);
	constexpr size_t numPaks = sizeof(defaultPaks) / sizeof(defaultPaks[0]);

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

	bool success = true;
	for (size_t i = 0; i < numDirs; ++i)
	{
		const pathCategory_t& category = gameDirs[i];
		success &= fm.AddGameDirectory((gameDir + category.path).c_str(), category.categoryName);
	}

	for (size_t i = 0; i < numPaks; ++i)
	{
		const pathCategory_t& category = defaultPaks[i];
		success &= fm.AddPakFile((gameDir + category.path).c_str(), category.categoryName);
	}

	MOHPC_LOG(Info, "%d pak(s) loaded in %s.", fm.GetNumPakFiles(), directory);

	return success;
}
