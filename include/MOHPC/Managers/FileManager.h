#pragma once

#include "../Global.h"
#include "../Script/Container.h"
#include "../Script/str.h"
#include "../Utilities/SharedPtr.h"
#include <istream>
#include <vector>
#include <string>
#include <set>

namespace MOHPC
{
	class FileEntry
	{
	private:
		str Name;
		bool bIsDir;

	public:
		MOHPC_EXPORTS FileEntry();
		FileEntry(const char *Filename) noexcept;
		FileEntry(const str& Filename) noexcept;
		FileEntry(str&& Filename) noexcept;
		FileEntry(const FileEntry& Entry) = default;
		FileEntry& operator=(const FileEntry& Entry) = default;
		FileEntry(FileEntry&& Entry) noexcept = default;
		FileEntry& operator=(FileEntry&& Entry) noexcept = default;
		MOHPC_EXPORTS ~FileEntry();

		/** Return true if the path is a directory. */
		MOHPC_EXPORTS bool IsDirectory() const;

		/** Return the extension for this file. */
		MOHPC_EXPORTS const char *GetExtension() const;

		/** Return the file name. */
		MOHPC_EXPORTS const char* GetRawName() const;

		/** Return the file name. */
		MOHPC_EXPORTS const str& GetStr() const;

		operator const str& () const;

		friend bool operator==(const MOHPC::str& lhs, const FileEntry& rhs)
		{
			return lhs == *rhs.Name;
		}

		friend bool operator==(const FileEntry& lhs, const MOHPC::str& rhs)
		{
			return *lhs.Name == rhs;
		}
	};

	class FileEntryList
	{
	private:
		MOHPC::Container<FileEntry> fileList;

	public:
		MOHPC_EXPORTS FileEntryList();
		FileEntryList(Container<FileEntry>&& inFileList) noexcept;
		MOHPC_EXPORTS FileEntryList(const FileEntryList& other) = delete;
		MOHPC_EXPORTS FileEntryList& operator=(const FileEntryList& other) = delete;
		MOHPC_EXPORTS FileEntryList(FileEntryList&& other) noexcept = default;
		MOHPC_EXPORTS FileEntryList& operator=(FileEntryList && other) noexcept = default;
		MOHPC_EXPORTS ~FileEntryList();

		MOHPC_EXPORTS size_t GetNumFiles() const;
		MOHPC_EXPORTS const FileEntry* GetFileEntry(size_t Index) const;
	};

	class MOHPC_EXPORTS File
	{
		friend class FileManager;

	private:
		struct FileData* m_data;
		bool m_bInPak;

	public:
		File();
		~File();

		/** Return whether or not the file is stored in a pak file. */
		bool IsInPak() const;

		/** Return the stream associated with this file. */
		std::istream* GetStream() const;

		/**
		 * Read the entire stream and return the total size read
		 *
		 * @param Out - Specify a pointer to a variable that the function will output the buffer to
		 * @return the total bytes read, 0 if the stream is empty or was not read.
		 */
		std::streamsize ReadBuffer(void** Out);
	};

	struct FileManagerCategory;
	struct PakFileEntry;
	struct PakFileEntryCompare;

	typedef SharedPtr<File> FilePtr;

	class MOHPC_EXPORTS FileManager
	{
	public:
		FileManager();
		~FileManager();

		/*
		 * Add a directory to the game path chain (priority starts from the last added path).
		 *
		 * @param Directory - The full path to the directory.
		 * @param CategoryName - The category to put the directory in.
		 * @return true if the directory was added successfully.
		 */
		bool AddGameDirectory(const char* Directory, const char* CategoryName = nullptr);

		/**
		 * Add a pak file and its file list (priority starts from the last added pak).
		 *
		 * @param Filename - The full path to the pak file.
		 * @param CategoryName - The category to put the pak in.
		 * @return true if the pak was added successfully.
		 */
		bool AddPakFile(const char* Filename, const char* CategoryName = nullptr);

		/** Return the number of added game dirs. */
		size_t GetNumDirectories() const;

		/** Return the number of pak files. */
		size_t GetNumPakFiles() const;

		/**
		 * Return true if a file exists, false otherwise.
		 *
		 * @param Filename - The game path to the file
		 * @return true if the file exists.
		 */
		bool FileExists(const char* Filename, bool bInPakOnly = true, const char* CategoryName = nullptr) const;
		
		/**
		 * Open a file
		 *
		 * @param Filename - The game path to the file
		 * @param CategoryName - The name of the category to open the file in.
		 * @return a pointer to the File class, NULL if not found.
		 */
		FilePtr OpenFile(const char* Filename, const char* CategoryName = nullptr);
		
		/**
		 * Retrieve the hash of a file
		 *
		 * @param Filename - The game path to the file
		 * @return the string of the hash from the file
		 */
		MOHPC::str GetFileHash(const char* Filename, const char* CategoryName = nullptr);
		
		/**
		 * Return a list of files in a directory and optionally its sub-directories.
		 *
		 * @param Directory - The game path directory to list files from
		 * @param Extensions - List of extensions to only include, empty array for all
		 * @param bRecursive - True to also search in subdirectories
		 * @param bInPakOnly - True to list files that are only in paks
		 * @return a list of files.
		 */
		FileEntryList ListFilteredFiles(const char* Directory, const MOHPC::Container<str>& Extensions = MOHPC::Container<str>(), bool bRecursive = true, bool bInPakOnly = true, const char* CategoryName = nullptr) const;
		FileEntryList ListFilteredFiles(const char* Directory, const char* Extension = "", bool bRecursive = true, bool bInPakOnly = true, const char* CategoryName = nullptr) const;
		
		/**
		 * Sort a given list of files.
		 *
		 * @param FileList - List of files
		 */
		void SortFileList(Container<FileEntry>& FileList);

		/**
		 * Return a corrected game path
		 *
		 * @param Path - Game path to correct
		 */
		str GetFixedPath(const str& Path) const;
		
		/**
		 * Return the list of category names
		 *
		 * @param OutList - The list of category names.
		 */
		void GetCategoryList(Container<const char*>& OutList) const;

		/**
		 * Return the extension of the given filename (without the dot), an empty string is returned if there is no extension.
		 *
		 * @param Filename - The game path to the file
		 * @return The extension
		 */
		static const char *GetFileExtension(const char* Filename);
		
		/**
		 * Return the file name with a new extension
		 *
		 * @param Filename - The game path to the file
		 * @return The file name with new extension
		 */
		static str SetFileExtension(const char* Filename, const char* NewExtension = "");
		
		/**
		 * Return the filename with a default extension if unspecified.
		 *
		 * @param Filename - The game path to the file
		 * @return The file name with a default specified extension.
		 */
		static str GetDefaultFileExtension(const char* Filename, const char* DefaultExtension);


		/**
		 * Return the canonical filename (without double slashes '/')
		 *
		 * @param Filename - The file name
		 * @return The canonical file name.
		 */
		static str CanonicalFilename(const char* Filename);

	private:
		FileManagerCategory* GetCategory(const char* CategoryName) const;
		FileManagerCategory* GetOrCreateCategory(const char* CategoryName);
		void CategorizeFiles(std::set<PakFileEntry*, PakFileEntryCompare>& FileList, FileManagerCategory* Category);
		void CacheFiles();
		void CacheFilesCategory(FileManagerCategory* Category);

		struct GamePath* m_pGamePath;
		struct PakFile* m_pPak;
		struct FileManagerData* m_pData;
		size_t numGamePaths;
		size_t numPaks;
	};
}
