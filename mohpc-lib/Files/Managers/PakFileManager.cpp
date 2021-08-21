#include <MOHPC/Files/Managers/PakFileManager.h>
#include <MOHPC/Files/FileMap.h>
#include <MOHPC/Files/Category.h>
#include <MOHPC/Common/Log.h>

#include "../ArchiveFile.h"
#include <zlib/contrib/minizip/unzip.h>

#include <fstream>
#include <cassert>
#include <unordered_map>

using namespace MOHPC;

static constexpr char MOHPC_LOG_NAMESPACE[] = "pakfileman";

namespace MOHPC
{
	struct FileManagerCategory;

	struct ZipContext
	{
		struct data_t
		{
			fs::path fileName;
		};

	public:
		ZipContext(const fs::path& fileName)
		{
			def.zopen64_file = &ZipContext::zopen_file;
			def.zread_file = &ZipContext::zread_file;
			def.zwrite_file = &ZipContext::zwrite_file;
			def.ztell64_file = &ZipContext::ztell_file;
			def.zseek64_file = &ZipContext::zseek_file;
			def.zclose_file = &ZipContext::zclose_file;
			def.zerror_file = &ZipContext::zerror_file;
			def.opaque = &data;
			data.fileName = fileName;
		}

		~ZipContext()
		{
		}

		static voidpf zopen_file(voidpf opaque, const void* filename, int mode)
		{
			data_t* This = static_cast<data_t*>(opaque);

			std::ifstream* ifs = new std::ifstream(This->fileName, std::ios::in | std::ios::binary);
			if (!ifs->is_open())
			{
				delete ifs;
				return nullptr;
			}

			return ifs;
		}

		static uLong zread_file(voidpf opaque, voidpf stream, void* buf, uLong size)
		{
			std::istream* ifs = (std::ifstream*)stream;

			ifs->read((char*)buf, size);
			return (uLong)ifs->gcount();
		}

		static uLong zwrite_file(voidpf opaque, voidpf stream, const void* buf, uLong size)
		{
			assert(!"Unsupported method");
			return 0;
			/*
			std::ostream* ofs = (std::ostream*)stream;

			std::streamoff off = ofs->tellp();
			ofs->write((const char*)buf, size);
			return (uLong)(ofs->tellp() - off);
			*/
		}

		static ZPOS64_T ztell_file(voidpf opaque, voidpf stream)
		{
			std::istream* ifs = (std::ifstream*)stream;
			return ifs->tellg();
		}

		static long zseek_file(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin)
		{
			std::istream* ifs = (std::ifstream*)stream;
			switch (origin)
			{
			case ZLIB_FILEFUNC_SEEK_CUR:
				ifs->seekg(offset, ifs->cur);
				break;
			case ZLIB_FILEFUNC_SEEK_END:
				ifs->seekg(offset, ifs->end);
				break;
			case ZLIB_FILEFUNC_SEEK_SET:
				ifs->seekg(offset, ifs->beg);
				break;
			default: return -1;
			}

			return 0;
		}

		static int zclose_file(voidpf opaque, voidpf stream)
		{
			std::istream* ifs = (std::ifstream*)stream;
			delete ifs;

			return 0;
		}

		static int zerror_file(voidpf opaque, voidpf stream)
		{
			std::ifstream* ifs = (std::ifstream*)stream;
			if (!ifs->is_open())
			{
				// not opened
				return 1;
			}

			if (ifs->fail())
			{
				// bad
				return 1;
			}

			return 0;
		}

	public:
		zlib_filefunc64_def def;
		data_t data;
	};

	struct PakFileEntry
	{
	public:
		template<typename T>
		PakFileEntry(PakFile& pakRef, T first, T last)
			: pak(pakRef)
			, Name(first, last)
		{}

	public:
		fs::path Name;
		PakFile& pak;
		unz_file_pos Pos;
		size_t uncompressedSize;
	};

	struct PakFile
	{
	public:
		PakFile(const fs::path& name, const FileCategory* categoryPtr)
			: fileName(name)
			, category(categoryPtr)
			, zipFile(nullptr)
		{
		}

		~PakFile()
		{
			if (zipFile) {
				unzClose(zipFile);
			}
		}

	public:
		fs::path fileName;
		const FileCategory* category;
		unzFile zipFile;
		std::vector<PakFileEntry> entries;
	};

	struct PakFileEntryCompare
	{
		bool operator() (const PakFileEntry* e1, const PakFileEntry* e2) const
		{
			return FileNameCompare()(e1->Name, e2->Name);
		}
	};

	using pakFileMap_t = std::unordered_map<fs::path, const PakFileEntry*, FileNameHash, FileNameMapCompare>;
	struct PakCategory
	{
	public:
		PakCategory()
			: category(nullptr)
			, parent(nullptr)
		{}

		PakCategory(const FileCategory* categoryPtr)
			: category(categoryPtr)
			, parent(nullptr)
		{}

		PakCategory(const FileCategory * categoryPtr, PakCategory* parentPtr)
			: category(categoryPtr)
			, parent(parentPtr)
		{}

	public:
		const FileCategory* category;
		PakCategory* parent;
		std::vector<PakCategory*> children;
		pakFileMap_t m_PakFilesMap;
		std::vector<const PakFileEntry*> m_PakFilesList;
	};

	struct PakFileManagerData
	{
		PakCategory defaultCategory;
		std::vector<PakCategory> categoryList;
	};
}

template<typename T>
bool forEachPakCategory(const PakCategory* category, T func)
{
	for (auto it = category->children.rbegin(); it != category->children.rend(); ++it)
	{
		if (!forEachPakCategory(*it, func)) {
			return false;
		}
	}

	return func(category);
}

template<typename T>
bool forEachFiles(const PakCategory* category, T func)
{
	for (auto it = category->children.rbegin(); it != category->children.rend(); ++it)
	{
		if (!forEachFiles(*it, func)) {
			return false;
		}
	}

	for (auto it = category->m_PakFilesList.begin(); it != category->m_PakFilesList.end(); ++it)
	{
		if (!func(*it)) {
			return false;
		}
	}

	return true;
}

MOHPC_OBJECT_DEFINITION(PakFileManager);
PakFileManager::PakFileManager()
{
	m_pData = nullptr;
}

PakFileManager::~PakFileManager()
{
	if (m_pData) {
		clearData();
	}
}

void PakFileManager::clearData()
{
	delete m_pData;
}

bool PakFileManager::AddPakFile(const fs::path& fileName, const FileCategory* category)
{
	if (findPakFile(fileName))
	{
		// already exists
		return false;
	}

	const char* const categoryName = category ? category->getName() : "";

	ZipContext context(fileName);
	// cheat by giving a custom context, for unicode support
	unzFile zipFile = unzOpen2_64("", &context.def);
	if (!zipFile)
	{
		MOHPC_LOG(Error, "Pak file %s for %s doesn't exist.", fileName.generic_string().c_str(), categoryName);
		return false;
	}

	if (m_pData)
	{
		// due to the change in files structure, entries will need to be rebuilt
		MOHPC_LOG(Info, "About to add a new pak file ('%s'), entries will need to be rebuilt", fileName.generic_string().c_str());
		clearData();
	}

	PakFile& pak = m_pPak.emplace_back(fileName, category);
	pak.zipFile = zipFile;

	unz_global_info64 globalInfo;
	unzGetGlobalInfo64(zipFile, &globalInfo);

	pak.entries.reserve((size_t)globalInfo.number_entry);

	if (globalInfo.number_entry)
	{
		uLong preallocatedLength = 128;
		char* preallocated = new char[preallocatedLength];

		size_t numFiles = 0;
		for (int res = unzGoToFirstFile(zipFile); res != UNZ_END_OF_LIST_OF_FILE; res = unzGoToNextFile(zipFile))
		{
			unz_file_info fileInfo;
			unzGetCurrentFileInfo(zipFile, &fileInfo, preallocated, preallocatedLength, NULL, 0, NULL, 0);

			if (fileInfo.size_filename > preallocatedLength)
			{
				delete[] preallocated;
				preallocatedLength = fileInfo.size_filename * 2;
				preallocated = new char[preallocatedLength];

				unzGetCurrentFileInfo(zipFile, NULL, preallocated, preallocatedLength, NULL, 0, NULL, 0);
			}

			PakFileEntry& entry = pak.entries.emplace_back(pak, preallocated, preallocated + fileInfo.size_filename);
			unzGetFilePos(zipFile, &entry.Pos);
			entry.uncompressedSize = fileInfo.uncompressed_size;
			++numFiles;
		}

		delete[] preallocated;

		assert(pak.entries.size() == numFiles);
	}

	numPaks++;

	MOHPC_LOG(Info, "Loaded pak %s (for %s).", fileName.generic_string().c_str(), categoryName);

	return true;
}

size_t PakFileManager::GetNumPakFiles() const
{
	return m_pPak.size();
}

bool PakFileManager::FileExists(const fs::path& fileName, const FileCategory* category)
{
	if (!m_pData) {
		CacheFiles();
	}

	const PakCategory* const pakCategory = GetCategory(category);
	const fs::path fileNameP = FileHelpers::removeRootDir(fileName.c_str());

	// true if the for-each didn't stop
	return !forEachPakCategory(pakCategory, [fileNameP](const PakCategory* current)
		{
			auto it = current->m_PakFilesMap.find(fileNameP);
			return it == current->m_PakFilesMap.end();
		});
}

IFilePtr PakFileManager::OpenFile(const fs::path& fileName, const FileCategory* category)
{
	if (!m_pData) {
		CacheFiles();
	}

	const PakCategory* const pakCategory = GetCategory(category);
	fs::path fileNameP = FileHelpers::removeRootDir(fileName.c_str());

	IFilePtr file;
	forEachPakCategory(pakCategory, [fileNameP, &file](const PakCategory* current)
		{
			auto it = current->m_PakFilesMap.find(fileNameP);
			if (it != current->m_PakFilesMap.end())
			{
				const PakFileEntry* entry = it->second;
				file = SharedPtr<IFile>(new ArchiveFile(it->first, entry->pak.zipFile, &entry->Pos, entry->uncompressedSize));

				return false;
			}

			return true;
		});

	return file;
}

void PakFileManager::ListFilteredFilesInternal(filteredFiles_t& fileList, const std::filesystem::path& directory, const ExtensionList& extensions, bool isRecursive, const FileCategory* category)
{
	if (!m_pData) {
		CacheFiles();
	}

	const PakCategory* const pakCategory = GetCategory(category);

	const bool bAnyExtension = extensions.size() <= 0;

	const fs::path::value_type* requestedDirP = FileHelpers::removeRootDir(directory.c_str());
	const size_t requestedDirLength = directory.native().length() - (requestedDirP - directory.c_str());

	if (!requestedDirLength)
	{
		// everything
		if (bAnyExtension)
		{
			// add everything
			forEachFiles(pakCategory, [&fileList](const PakFileEntry* entry) -> bool
				{
					fileList.insert(entry->Name);
					return true;
				});
		}
		else
		{
			forEachFiles(pakCategory, [&fileList, &extensions](const PakFileEntry* entry) -> bool
				{
					const fs::path::value_type* fileExtension = FileHelpers::getExtension(entry->Name.c_str());
					for (auto it = extensions.begin(); it != extensions.end(); ++it)
					{
						if (!strHelpers::icmp(it->c_str(), fileExtension)) {
							fileList.insert(entry->Name);
						}
					}

					return true;
				});
		}
	}
	else
	{
		bool bInDir = false;
		forEachFiles(pakCategory, [&](const PakFileEntry* entry) -> bool
			{
				const fs::path::value_type* pathStringP = entry->Name.c_str();
				const size_t rootLength = pathStringP - entry->Name.c_str();
				const fs::path::value_type* entryDirP = FileHelpers::getLastPath(pathStringP);
				const size_t entryDirLength = entryDirP - pathStringP;
				const fs::path::value_type* parentDirP = FileHelpers::getParentPath(pathStringP);
				const size_t parentDirLength = parentDirP - pathStringP;

				// check if 
				if (entryDirLength && !strHelpers::icmpn(pathStringP, requestedDirP, requestedDirLength))
				{
					if (isRecursive || !strHelpers::icmpn(pathStringP, requestedDirP, parentDirLength))
					{
						bInDir = true;

						if (bAnyExtension) {
							fileList.insert(entry->Name);
						}
						else
						{
							const fs::path::value_type* fileExtension = FileHelpers::getExtension(entry->Name.c_str());
							for (auto it = extensions.begin(); it != extensions.end(); ++it)
							{
								if (!strHelpers::icmp(it->c_str(), fileExtension)) {
									fileList.insert(entry->Name);
								}
							}
						}
					}
					else if (bInDir)
					{
						return false;
					}
				}
				else if (bInDir)
				{
					return false;
				}

				return true;
			});
	}
}

const PakFile* PakFileManager::findPakFile(const std::filesystem::path& path) const
{
	for (auto it = m_pPak.begin(); it != m_pPak.end(); ++it)
	{
		const PakFile& p = *it;
		if (p.fileName == path)
		{
			// pak file path matches
			return &p;
		}
	}

	return nullptr;
}

PakCategory* PakFileManager::GetCategory(const FileCategory* category) const
{
	if (category == nullptr)
	{
		// return the default category
		return &m_pData->defaultCategory;
	}
	else
	{
		for (auto it = m_pData->categoryList.begin(); it != m_pData->categoryList.end(); ++it)
		{
			PakCategory& pCategory = *it;
			if (pCategory.category == category) {
				return &pCategory;
			}
		}
	}

	return nullptr;
}

PakCategory* PakFileManager::findCategory(const FileCategory* category) const
{
	for (auto it = m_pData->categoryList.begin(); it != m_pData->categoryList.end(); ++it)
	{
		if (it->category == category) {
			return &*it;
		}
	}

	return nullptr;
}

PakCategory* PakFileManager::tryCreateCategory(const FileCategory* category)
{
	PakCategory* parent = nullptr;
	if (category->getParent()) {
		parent = tryCreateCategory(category->getParent());
	}

	PakCategory* pakCategory = findCategory(category);

	if (!pakCategory) {
		m_pData->categoryList.emplace_back(category);
	}

	return pakCategory;
}

void PakFileManager::CacheFiles()
{
	MOHPC_LOG(Info, "Caching files for the first time...");

	m_pData = new PakFileManagerData;

	size_t numCategories = m_pPak.size();
	auto it = m_pPak.begin();
	for (; it != m_pPak.end(); ++it)
	{
		for (auto jt = it; jt != m_pPak.end(); ++jt)
		{
			if (it == jt) {
				continue;
			}

			if (!it->category) {
				continue;
			}

			if (it->category == jt->category)
			{
				--numCategories;
				break;
			}
		}
	}

	m_pData->categoryList.reserve(numCategories);
	for (auto it = m_pPak.begin(); it != m_pPak.end(); ++it)
	{
		if (!it->category)
		{
			// skip null categories
			continue;
		}

		tryCreateCategory(it->category);
	}

	assert(m_pData->categoryList.size() == numCategories);
	assert(m_pData->categoryList.size() == m_pData->categoryList.capacity());

	// setup children
	for (auto it = m_pData->categoryList.begin(); it != m_pData->categoryList.end(); ++it)
	{
		PakCategory& c1 = *it;

		size_t numChildren = 0;
		for (auto jt = m_pData->categoryList.begin(); jt != m_pData->categoryList.end(); ++jt)
		{
			PakCategory& c2 = *jt;
			if (c2.category->parent == c1.category) {
				++numChildren;
			}
		}

		c1.children.reserve(numChildren);
		for (auto jt = m_pData->categoryList.begin(); jt != m_pData->categoryList.end(); ++jt)
		{
			PakCategory& c2 = *jt;
			if (c2.category->parent == c1.category) {
				c1.children.push_back(&c2);
			}
		}
	}

	CacheDefaultFilesCategory();

	for (auto it = m_pData->categoryList.begin(); it != m_pData->categoryList.end(); ++it)
	{
		CacheFilesCategory(*it);
	}
}

void PakFileManager::CacheDefaultFilesCategory()
{
	fileSet_t fileList;

	for (auto it = m_pPak.begin(); it != m_pPak.end(); ++it)
	{
		for (auto jt = it->entries.begin(); jt != it->entries.end(); ++jt)
		{
			const PakFileEntry& Entry = *jt;
			fileList.insert(&Entry);
		}
	}

	CategorizeFiles(fileList, m_pData->defaultCategory);
}

void PakFileManager::CacheFilesCategory(PakCategory& Category)
{
	fileSet_t fileList;

	for (auto it = m_pPak.begin(); it != m_pPak.end(); ++it)
	{
		if (it->category == Category.category)
		{
			for (auto jt = it->entries.begin(); jt != it->entries.end(); ++jt)
			{
				const PakFileEntry& entry = *jt;
				fileList.insert(&entry);
			}
		}
	}

	CategorizeFiles(fileList, Category);
}

void PakFileManager::CategorizeFiles(fileSet_t& FileList, PakCategory& Category)
{
	const size_t numFiles = FileList.size();
	Category.m_PakFilesList.resize(numFiles);
	std::move(FileList.begin(), FileList.end(), Category.m_PakFilesList.begin());

	Category.m_PakFilesMap.reserve(numFiles);

	for (auto it = Category.m_PakFilesList.begin(); it != Category.m_PakFilesList.end(); it++)
	{
		const PakFileEntry* Entry = *it;
		Category.m_PakFilesMap.emplace(Entry->Name, Entry);
	}

	assert(Category.m_PakFilesList.size() == FileList.size());
	assert(Category.m_PakFilesMap.size() == FileList.size());
}

MOHPC_OBJECT_DEFINITION(PakFileManagerProxy);
