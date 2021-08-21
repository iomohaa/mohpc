#pragma once

#include "FilesGlobal.h"
#include "FileDefs.h"

namespace MOHPC
{
	class FileCategoryManager;
	class PakFileManager;
	class SystemFileManager;

	namespace FileHelpers
	{
		/**
		 * Auto add standard pak files and directories from root game directory.
		 *
		 * @param Directory - The full path to the game directory.
		 */
		MOHPC_FILES_EXPORTS bool FillGameDirectory(FileCategoryManager& categoryManager, PakFileManager& pakFM, SystemFileManager& sysFM, const fs::path& directory);
	}
}
