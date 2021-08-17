#pragma once

#include "../FilesGlobal.h"
#include "../../Common/str.h"
#include "../../Utility/SharedPtr.h"
#include "../File.h"

#include <vector>
#include <istream>
#include <vector>
#include <string>
#include <set>
#include <filesystem>
#include <exception>

namespace MOHPC
{
	struct FileManagerCategory;
	struct PakFileEntry;
	struct PakFileEntryCompare;
	struct GamePath;
	struct PakFile;
	struct FileManagerData;

	using ExtensionList = std::vector<std::filesystem::path>;

	class FileManager
	{
		MOHPC_FILES_OBJECT_DECLARATION(FileManager);

	public:
		MOHPC_FILES_EXPORTS FileManager();
		MOHPC_FILES_EXPORTS ~FileManager();

		/*
		 * Add a directory to the game path chain (priority starts from the last added path).
		 *
		 * @param Directory - The full path to the directory.
		 * @param CategoryName - The category to put the directory in.
		 * @return true if the directory was added successfully.
		 */
		MOHPC_FILES_EXPORTS bool AddGameDirectory(const fs::path& Directory, const char* CategoryName = nullptr);

		/**
		 * Add a pak file and its file list (priority starts from the last added pak).
		 *
		 * @param Filename - The full path to the pak file.
		 * @param CategoryName - The category to put the pak in.
		 * @return true if the pak was added successfully.
		 */
		MOHPC_FILES_EXPORTS bool AddPakFile(const fs::path& fileName, const char* CategoryName = nullptr);

		/** Return the number of added game dirs. */
		MOHPC_FILES_EXPORTS size_t GetNumDirectories() const;

		/** Return the number of pak files. */
		MOHPC_FILES_EXPORTS size_t GetNumPakFiles() const;

		/**
		 * Return true if a file exists, false otherwise.
		 *
		 * @param Filename - The game path to the file
		 * @return true if the file exists.
		 */
		MOHPC_FILES_EXPORTS bool FileExists(const fs::path& Filename, bool bInPakOnly = true, const char* CategoryName = nullptr) const;
		
		/**
		 * Open a file
		 *
		 * @param Filename - The game path to the file
		 * @param CategoryName - The name of the category to open the file in.
		 * @return a pointer to the File class, NULL if not found.
		 */
		MOHPC_FILES_EXPORTS IFilePtr OpenFile(const fs::path& Filename, const char* CategoryName = nullptr);
		
		/**
		 * Return a list of files in a directory and optionally its sub-directories.
		 *
		 * @param Directory - The game path directory to list files from
		 * @param Extensions - List of extensions to only include, empty array for all
		 * @param bRecursive - True to also search in subdirectories
		 * @param bInPakOnly - True to list files that are only in paks
		 * @return a list of files.
		 */
		MOHPC_FILES_EXPORTS FileEntryList ListFilteredFiles(const std::filesystem::path& Directory, const ExtensionList& Extensions = ExtensionList(), bool bRecursive = true, bool bInPakOnly = true, const char* CategoryName = nullptr) const;
		MOHPC_FILES_EXPORTS FileEntryList ListFilteredFiles(const std::filesystem::path& Directory, const std::filesystem::path& Extension = "", bool bRecursive = true, bool bInPakOnly = true, const char* CategoryName = nullptr) const;
		
		/**
		 * Sort a given list of files.
		 *
		 * @param FileList - List of files
		 */
		MOHPC_FILES_EXPORTS void SortFileList(std::vector<FileEntry>& FileList);

		/**
		 * Return a corrected game path
		 *
		 * @param Path - Game path to correct
		 */
		MOHPC_FILES_EXPORTS fs::path GetFixedPath(const fs::path& path) const;
		
		/**
		 * Return the list of category names
		 *
		 * @param OutList - The list of category names.
		 */
		MOHPC_FILES_EXPORTS void GetCategoryList(std::vector<const char*>& OutList) const;

	private:
		FileManagerCategory* GetCategory(const char* CategoryName) const;
		FileManagerCategory* GetOrCreateCategory(const char* CategoryName);
		GamePath* findGamePath(const std::filesystem::path& path) const;
		PakFile* findPakFile(const std::filesystem::path& path) const;
		void CategorizeFiles(std::set<PakFileEntry*, PakFileEntryCompare>& FileList, FileManagerCategory* Category);
		void CacheFiles();
		void CacheFilesCategory(FileManagerCategory* Category);

		GamePath* m_pGamePath;
		PakFile* m_pPak;
		FileManagerData* m_pData;
		size_t numGamePaths;
		size_t numPaks;
	};
	using FileManagerPtr = SharedPtr<FileManager>;

	namespace FileErrors
	{
		class Base : public std::exception {};

		class PathNotFound : public Base
		{
		public:
			PathNotFound(const std::filesystem::path& path);

			const std::filesystem::path& getPath() const;

		private:
			std::filesystem::path path;
		};
	}
}
