#pragma once

#include "IFileManager.h"

#include <list>

namespace MOHPC
{
	struct GamePath;

	class SystemFileManager : public IFileManager
	{
		MOHPC_FILES_OBJECT_DECLARATION(SystemFileManager);

	public:
		MOHPC_FILES_EXPORTS SystemFileManager();
		MOHPC_FILES_EXPORTS ~SystemFileManager();

		/*
		 * Add a directory to the directory chain (priority starts from the last added path).
		 *
		 * @param Directory - The full path to the directory.
		 * @param CategoryName - The category to put the directory in.
		 * @return true if the directory was added successfully.
		 */
		MOHPC_FILES_EXPORTS bool AddDirectory(const fs::path& directory, const FileCategory* category);

		/** Return the number of added game dirs. */
		MOHPC_FILES_EXPORTS size_t GetNumDirectories() const;

	public:
		bool FileExists(const fs::path& fileName, const FileCategory* category = nullptr) override;
		IFilePtr OpenFile(const fs::path& fileName, const FileCategory* category = nullptr) override;
		void ListFilteredFilesInternal(filteredFiles_t& fileList, const std::filesystem::path& directory, const ExtensionList& extensions = ExtensionList(), bool isRecursive = true, const FileCategory* category = nullptr) override;

	private:
		const GamePath* findGamePath(const std::filesystem::path& path) const;
		bool fileExists(const GamePath& path, const std::filesystem::path& fileName) const;
		IFilePtr openFile(const GamePath& path, std::filesystem::path&& fileName) const;

	private:
		std::list<GamePath> paths;
	};
	using SystemFileManagerPtr = SharedPtr<SystemFileManager>;

	class SystemFileManagerProxy : public FileManagerProxy<SystemFileManager>
	{
		using FileManagerProxy::FileManagerProxy;
		MOHPC_FILES_OBJECT_DECLARATION(SystemFileManagerProxy);
	};
	using SystemFileManagerProxyPtr = SharedPtr<SystemFileManagerProxy>;
}