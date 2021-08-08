#pragma once

#include "FilesGlobal.h"

namespace MOHPC
{
	class FileManager;

	namespace FileHelpers
	{
		/**
		 * Auto add standard pak files and directories from root game directory.
		 *
		 * @param Directory - The full path to the game directory.
		 */
		MOHPC_FILES_EXPORTS bool FillGameDirectory(FileManager& fm, const char* directory);
	}
}
