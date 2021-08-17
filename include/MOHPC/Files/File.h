#pragma once

#include "FilesGlobal.h"
#include "FilesObject.h"
#include "FileDefs.h"
#include "../Common/str.h"

#include <filesystem>
#include <vector>

#include <tchar.h>

namespace MOHPC
{
	/**
	 * This class maps a file entry.
	 */
	class FileEntry
	{
	public:
		MOHPC_FILES_EXPORTS FileEntry();
		FileEntry(const std::filesystem::path::value_type* Filename) noexcept;
		FileEntry(const std::filesystem::path& Filename) noexcept;
		FileEntry(std::filesystem::path&& Filename) noexcept;
		FileEntry(const FileEntry& Entry) = default;
		FileEntry& operator=(const FileEntry& Entry) = default;
		FileEntry(FileEntry&& Entry) noexcept = default;
		FileEntry& operator=(FileEntry&& Entry) noexcept = default;
		MOHPC_FILES_EXPORTS ~FileEntry();

		/** Return true if the path is a directory. */
		MOHPC_FILES_EXPORTS bool IsDirectory() const;

		/** Return the extension for this file. */
		MOHPC_FILES_EXPORTS const fs::path::value_type* GetExtension() const;

		/** Return the file name. */
		MOHPC_FILES_EXPORTS const fs::path::value_type* GetRawName() const;

		/** Return the file name. */
		MOHPC_FILES_EXPORTS const fs::path& GetStr() const;

		operator const fs::path& () const;

		friend bool operator==(const str& lhs, const FileEntry& rhs)
		{
			return lhs == rhs.Name;
		}

		friend bool operator==(const FileEntry& lhs, const str& rhs)
		{
			return lhs.Name == rhs;
		}

	private:
		fs::path Name;
		bool bIsDir;
	};

	class FileEntryList
	{
	private:
		std::vector<FileEntry> fileList;

	public:
		MOHPC_FILES_EXPORTS FileEntryList();
		FileEntryList(std::vector<FileEntry>&& inFileList) noexcept;
		MOHPC_FILES_EXPORTS FileEntryList(const FileEntryList& other) = delete;
		MOHPC_FILES_EXPORTS FileEntryList& operator=(const FileEntryList& other) = delete;
		MOHPC_FILES_EXPORTS FileEntryList(FileEntryList&& other) noexcept = default;
		MOHPC_FILES_EXPORTS FileEntryList& operator=(FileEntryList&& other) noexcept = default;
		MOHPC_FILES_EXPORTS ~FileEntryList();

		MOHPC_FILES_EXPORTS size_t GetNumFiles() const;
		MOHPC_FILES_EXPORTS const FileEntry* GetFileEntry(size_t Index) const;
	};

	class IFile
	{
		friend class FileManager;

	public:
		IFile(const fs::path& fileName);
		virtual ~IFile();

		/** Return the name of the file. */
		MOHPC_FILES_EXPORTS const fs::path& getName() const;

		/** Return the stream associated with this file. */
		virtual std::istream* GetStream() = 0;

		/**
		 * Read the entire stream and return the total size read
		 *
		 * @param Out - Specify a pointer to a variable that the function will output the buffer to
		 * @return the total bytes read, 0 if the stream is empty or was not read.
		 */
		virtual uint64_t ReadBuffer(void** Out) = 0;

	private:
		fs::path name;
	};
	using IFilePtr = SharedPtr<IFile>;
}