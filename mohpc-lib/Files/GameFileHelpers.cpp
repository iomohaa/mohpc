#include <MOHPC/Files/GameFileHelpers.h>
#include <MOHPC/Files/Managers/SystemFileManager.h>
#include <MOHPC/Files/Managers/PakFileManager.h>
#include <MOHPC/Files/Category.h>
#include <MOHPC/Common/Log.h>

using namespace MOHPC;

static constexpr char MOHPC_LOG_NAMESPACE[] = "file_game_helpers";

struct pathCategory_t
{
	const char* path;
	const FileCategory* category;
};

bool FileHelpers::FillGameDirectory(FileCategoryManager& categoryManager, PakFileManager& pakFM, SystemFileManager& sysFM, const fs::path& directory)
{
	FileCategory* categoryAA = categoryManager.createCategory("AA");
	FileCategory* categorySH = categoryManager.createCategory("SH");
	FileCategory* categoryBT = categoryManager.createCategory("BT");
	FileCategory* categoryAA_111 = categoryManager.createCategory(categoryAA, "AA_111");
	FileCategory* categorySH_211 = categoryManager.createCategory(categorySH, "SH_211");
	FileCategory* categorySH_215 = categoryManager.createCategory(categorySH, "SH_215");
	FileCategory* categoryBT_240 = categoryManager.createCategory(categoryBT, "BT_240");

	const pathCategory_t gameDirs[] =
	{
		{ "main", categoryAA },
		{ "mainta", categorySH },
		{ "maintt", categoryBT }
	};

	const pathCategory_t defaultPaks[] =
	{
		{ "main/Pak0.pk3", categoryAA },
		{ "main/Pak1.pk3", categoryAA },
		{ "main/Pak2.pk3", categoryAA },
		{ "main/Pak3.pk3", categoryAA },
		{ "main/Pak4.pk3", categoryAA },
		{ "main/Pak5.pk3", categoryAA },
		{ "main/pak6.pk3", categoryAA_111 },
		{ "mainta/pak1.pk3", categorySH },
		{ "mainta/pak2.pk3", categorySH },
		{ "mainta/pak3.pk3", categorySH_211 },
		{ "mainta/pak4.pk3", categorySH_215 },
		{ "maintt/pak1.pk3", categoryBT },
		{ "maintt/pak2.pk3", categoryBT },
		{ "maintt/pak3.pk3", categoryBT_240 },
		{ "maintt/pak4.pk3", categoryBT_240 }
	};

	constexpr size_t numDirs = sizeof(gameDirs) / sizeof(gameDirs[0]);
	constexpr size_t numPaks = sizeof(defaultPaks) / sizeof(defaultPaks[0]);

	MOHPC_LOG(Info, "Loading pak files...");

	bool success = true;
	for (size_t i = 0; i < numDirs; ++i)
	{
		const pathCategory_t& category = gameDirs[i];
		success &= sysFM.AddDirectory(directory / category.path, category.category);
	}

	for (size_t i = 0; i < numPaks; ++i)
	{
		const pathCategory_t& category = defaultPaks[i];
		success &= pakFM.AddPakFile(directory / category.path, category.category);
	}

	MOHPC_LOG(Info, "%d pak(s) loaded in %s.", pakFM.GetNumPakFiles(), directory.generic_string().c_str());

	return success;
}
