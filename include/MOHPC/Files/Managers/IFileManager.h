#pragma once

#include "../FilesGlobal.h"
#include "../FileDefs.h"
#include "../File.h"
#include "../../Utility/SharedPtr.h"

#include <set>

namespace MOHPC
{
	class FileCategory;
	struct FileNameCompare;

	using ExtensionList = std::vector<std::filesystem::path>;

	class IFileManager
	{
	public:
		MOHPC_FILES_EXPORTS IFileManager();
		MOHPC_FILES_EXPORTS virtual ~IFileManager();


		/**
		 * Return true if a file exists, false otherwise.
		 *
		 * @param Filename - The game path to the file
		 * @return true if the file exists.
		 */
		MOHPC_FILES_EXPORTS virtual bool FileExists(const fs::path& fileName, const FileCategory* category = nullptr) = 0;

		/**
		 * Open a file
		 *
		 * @param Filename - The game path to the file
		 * @param CategoryName - The name of the category to open the file in.
		 * @return a pointer to the File class, NULL if not found.
		 */
		MOHPC_FILES_EXPORTS virtual IFilePtr OpenFile(const fs::path& fileName, const FileCategory* category = nullptr) = 0;

		/**
		 * Return a list of files in a directory and optionally its sub-directories.
		 *
		 * @param directory - The game path directory to list files from
		 * @param extensions - List of extensions to only include, empty array for all
		 * @param isRecursive - True to also search in subdirectories
		 * @param category - The category to search for.
		 * @return a list of files.
		 */
		MOHPC_FILES_EXPORTS virtual FileEntryList ListFilteredFiles(const std::filesystem::path& directory, const ExtensionList& extensions = ExtensionList(), bool isRecursive = true, const FileCategory* category = nullptr);
		MOHPC_FILES_EXPORTS virtual FileEntryList ListFilteredFiles(const std::filesystem::path& directory, const std::filesystem::path& extension, bool isRecursive = true, const FileCategory* category = nullptr);
		MOHPC_FILES_EXPORTS virtual FileEntryList ListAllFilteredFiles(const std::filesystem::path& directory, bool isRecursive = true, const FileCategory* category = nullptr);

	public:
		using filteredFiles_t = std::set<FileEntry, FileNameCompare>;

		virtual void ListFilteredFilesInternal(filteredFiles_t& fileList, const std::filesystem::path& directory, const ExtensionList& extensions = ExtensionList(), bool isRecursive = true, const FileCategory* category = nullptr) = 0;
	};
	using IFileManagerPtr = SharedPtr<IFileManager>;

	template<typename T>
	class FileManagerProxy : public IFileManager
	{
	public:
		FileManagerProxy();
		FileManagerProxy(const IFileManagerPtr& nextPtr);
		~FileManagerProxy();

		const IFileManagerPtr& getNext() const;
		T& get();
		const T& get() const;

	public:
		bool FileExists(const fs::path& fileName, const FileCategory* category) override;
		IFilePtr OpenFile(const fs::path& fileName, const FileCategory* category) override;

	protected:
		void ListFilteredFilesInternal(filteredFiles_t& fileList, const std::filesystem::path& directory, const ExtensionList& extensions, bool isRecursive, const FileCategory* category) override;

	private:
		IFileManagerPtr next;
		T manager;
	};

	template<typename T>
	FileManagerProxy<T>::FileManagerProxy()
	{
	}

	template<typename T>
	FileManagerProxy<T>::FileManagerProxy(const IFileManagerPtr& nextPtr)
		: next(nextPtr)
	{
	}

	template<typename T>
	FileManagerProxy<T>::~FileManagerProxy()
	{
	}

	template<typename T>
	const IFileManagerPtr& FileManagerProxy<T>::getNext() const
	{
		return next;
	}

	template<typename T>
	T& MOHPC::FileManagerProxy<T>::get()
	{
		return manager;
	}

	template<typename T>
	const T& MOHPC::FileManagerProxy<T>::get() const
	{
		return manager;
	}

	template<typename T>
	bool FileManagerProxy<T>::FileExists(const fs::path& fileName, const FileCategory* category)
	{
		if (next)
		{
			if (next->FileExists(fileName, category)) {
				return true;
			}
		}

		return manager.FileExists(fileName, category);
	}

	template<typename T>
	IFilePtr FileManagerProxy<T>::OpenFile(const fs::path& fileName, const FileCategory* category)
	{
		if (next)
		{
			const IFilePtr file = next->OpenFile(fileName, category);
			if (file) {
				return file;
			}
		}

		return manager.OpenFile(fileName, category);
	}

	template<typename T>
	void FileManagerProxy<T>::ListFilteredFilesInternal(filteredFiles_t& fileList, const std::filesystem::path& directory, const ExtensionList& extensions, bool isRecursive, const FileCategory* category)
	{
		if (next) {
			next->ListFilteredFilesInternal(fileList, directory, extensions, isRecursive, category);
		}

		manager.ListFilteredFilesInternal(fileList, directory, extensions, isRecursive, category);
	}
}
