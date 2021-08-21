#include "IFileManager.h"

#include <list>

namespace MOHPC
{
	struct PakFile;
	struct PakCategory;
	struct PakFileManagerData;
	struct PakFileEntry;
	struct PakFileEntryCompare;

	class PakFileManager : public IFileManager
	{
		MOHPC_FILES_OBJECT_DECLARATION(PakFileManager);

	public:
		MOHPC_FILES_EXPORTS PakFileManager();
		MOHPC_FILES_EXPORTS ~PakFileManager();

		/**
		 * Add a pak file and its file list (priority starts from the last added pak).
		 *
		 * @param fileName - The full path to the pak file.
		 * @param category - The category to put the pak in.
		 * @return true if the pak was added successfully.
		 */
		MOHPC_FILES_EXPORTS bool AddPakFile(const fs::path& fileName, const FileCategory* category);

		/** Return the number of pak files. */
		MOHPC_FILES_EXPORTS size_t GetNumPakFiles() const;

		virtual bool FileExists(const fs::path& fileName, const FileCategory* category = nullptr) override;
		virtual IFilePtr OpenFile(const fs::path& fileName, const FileCategory* category = nullptr) override;
		virtual void ListFilteredFilesInternal(filteredFiles_t& fileList, const std::filesystem::path& directory, const ExtensionList& extensions = ExtensionList(), bool isRecursive = true, const FileCategory* category = nullptr) override;

	private:
		using fileSet_t = std::set<const PakFileEntry*, PakFileEntryCompare>;

		void clearData();
		const PakFile* findPakFile(const std::filesystem::path& path) const;
		PakCategory* GetCategory(const FileCategory* category) const;
		PakCategory* findCategory(const FileCategory* category) const;
		PakCategory* tryCreateCategory(const FileCategory* category);
		void CategorizeFiles(fileSet_t& FileList, PakCategory& Category);
		void CacheFiles();
		void CacheDefaultFilesCategory();
		void CacheFilesCategory(PakCategory& Category);

	private:
		PakFileManagerData* m_pData;
		std::list<PakFile> m_pPak;
		size_t numPaks;
	};
	using PakFileManagerPtr = SharedPtr<PakFileManager>;

	class PakFileManagerProxy : public FileManagerProxy<PakFileManager>
	{
		using FileManagerProxy::FileManagerProxy;
		MOHPC_FILES_OBJECT_DECLARATION(PakFileManagerProxy);
	};
	using PakFileManagerProxyPtr = SharedPtr<PakFileManagerProxy>;
}
