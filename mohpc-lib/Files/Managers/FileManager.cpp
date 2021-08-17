#include <Shared.h>

#include "../ArchiveFile.h"
#include "../NormalFile.h"

#include <MOHPC/Files/Managers/FileManager.h>
#include <MOHPC/Files/FileHelpers.h>
#include <MOHPC/Files/FileMap.h>
#include <MOHPC/Utility/Misc/SHA256.h>
#include <MOHPC/Common/Log.h>

#include <zlib/contrib/minizip/unzip.h>

#include <fstream>
#include <unordered_map>
#include <set>
#include <filesystem>
#include <cstring>

#define MOHPC_LOG_NAMESPACE "fileman"

using namespace MOHPC;

namespace fs = std::filesystem;

namespace MOHPC
{
	struct GamePath
	{
		GamePath* Next;
		FileManagerCategory* Category;
		fs::path Directory;
	};

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
			std::ostream* ofs = (std::ostream*)stream;

			std::streamoff off = ofs->tellp();
			ofs->write((const char*)buf, size);
			return (uLong)(ofs->tellp() - off);
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
		fs::path Name;
		unz_file_pos Pos;
		unzFile ZipFile;
		size_t uncompressedSize;
	};

	struct PakFile
	{
	public:
		fs::path fileName;
		PakFile* Next;
		FileManagerCategory* Category;
		std::ifstream stream;
		unzFile ZipFile;
		std::vector<PakFileEntry*> entries;
	};

	struct PakFileEntryCompare
	{
		bool operator() (const PakFileEntry* e1, const PakFileEntry* e2) const
		{
			return FileNameCompare()(e1->Name, e2->Name);
		}
	};

	struct PakFileEntryHash
	{
		size_t operator()(const PakFileEntry* e) const
		{
			size_t hash = 0;

			const auto *p = e->Name.c_str();
			while (*p)
			{
				hash = *p++;
			}

			return hash;
		}
	};

	struct FileManagerCategory
	{
		str categoryName;
		std::unordered_map<fs::path, PakFileEntry*, FileNameHash, FileNameMapCompare> m_PakFilesMap;
		std::vector<PakFileEntry*> m_PakFilesList;
	};

	struct FileManagerData
	{
		FileManagerCategory defaultCategory;
		std::vector<FileManagerCategory*> categoryList;
	};
}

MOHPC_OBJECT_DEFINITION(FileManager);

FileManager::FileManager()
{
	m_pPak = NULL;
	m_pGamePath = NULL;
	m_pData = new FileManagerData;
	numGamePaths = 0;
	numPaks = 0;
}

FileManager::~FileManager()
{
	PakFile* tmppak;

	MOHPC_LOG(Debug, "Cleaning up file manager.");
	MOHPC_LOG(Debug, "Removing pak files...");

	for (PakFile* P = m_pPak; P != NULL; P = tmppak)
	{
		tmppak = P->Next;

		for (size_t i = 0; i < P->entries.size(); i++)
		{
			delete P->entries[i];
		}

		delete P;
	}

	MOHPC_LOG(Debug, "Removing game paths...");

	GamePath* tmpsp;

	for (GamePath* G = m_pGamePath; G != NULL; G = tmpsp)
	{
		tmpsp = G->Next;
		delete G;
	}

	MOHPC_LOG(Debug, "Removing categories...");

	const size_t numCategories = m_pData->categoryList.size();
	for (size_t i = 0; i < numCategories; i++)
	{
		FileManagerCategory* pCategory = m_pData->categoryList[i];
		delete pCategory;
	}

	delete m_pData;
}

bool FileManager::AddGameDirectory(const fs::path& Directory, const char* CategoryName)
{
	if (!fs::exists(Directory))
	{
		MOHPC_LOG(Error, "Directory %s for %s doesn't exist.", Directory, CategoryName);
		return false;
	}

	fs::path dir = Directory;
	if (dir.has_filename()) dir.append("");

	if (findGamePath(dir))
	{
		// already exists
		return false;
	}

	GamePath* G = new GamePath;
	G->Directory = dir;
	G->Category = GetOrCreateCategory(CategoryName);
	G->Next = m_pGamePath;
	m_pGamePath = G;
	numGamePaths++;

	MOHPC_LOG(Info, "Added directory %s for %s.", Directory.generic_string().c_str(), CategoryName);

	return true;
}

GamePath* FileManager::findGamePath(const fs::path& path) const
{
	for (GamePath* G = m_pGamePath; G; G = G->Next)
	{
		if (G->Directory == path)
		{
			// path matching
			return G;
		}
	}

	return nullptr;
}

bool FileManager::AddPakFile(const fs::path& fileName, const char* CategoryName)
{
	if (findPakFile(fileName))
	{
		// already exists
		return false;
	}

	ZipContext context(fileName);
	unzFile ZipFile = unzOpen2_64("", &context.def);
	if (!ZipFile)
	{
		MOHPC_LOG(Error, "Pak file %s for %s doesn't exist.", fileName.generic_string().c_str(), CategoryName);
		return false;
	}

	PakFile* Pak = new PakFile;
	Pak->fileName = fileName;
	Pak->Category = GetOrCreateCategory(CategoryName);
	Pak->Next = m_pPak;
	m_pPak = Pak;

	unz_global_info64 globalInfo;
	unzGetGlobalInfo64(ZipFile, &globalInfo);

	Pak->entries.resize((size_t)globalInfo.number_entry);
	PakFileEntry **entries = Pak->entries.data();

	for (int res = unzGoToFirstFile(ZipFile); res != UNZ_END_OF_LIST_OF_FILE; res = unzGoToNextFile(ZipFile))
	{
		PakFileEntry *Entry = new PakFileEntry;
		Entry->ZipFile = ZipFile;

		unzGetFilePos(Entry->ZipFile, &Entry->Pos);

		unz_file_info FileInfo;
		unzGetCurrentFileInfo(ZipFile, &FileInfo, NULL, 0, NULL, 0, NULL, 0);

		str name;
		name.resize(FileInfo.size_filename + 1);
		name[0] = '/';
		Entry->uncompressedSize = FileInfo.uncompressed_size;
		unzGetCurrentFileInfo(ZipFile, NULL, (char*)name.c_str() + 1, FileInfo.size_filename, NULL, 0, NULL, 0);

		Entry->Name = name;

		*entries++ = Entry;
	}

	numPaks++;

	MOHPC_LOG(Info, "Loaded pak %s (for %s).", fileName.generic_string().c_str(), CategoryName);

	return true;
}

PakFile* FileManager::findPakFile(const std::filesystem::path& path) const
{
	for (PakFile* p = m_pPak; p != nullptr; p = p->Next)
	{
		if (p->fileName == path)
		{
			// pak file path matches
			return p;
		}
	}

	return nullptr;
}

size_t FileManager::GetNumDirectories() const
{
	return numGamePaths;
}

size_t FileManager::GetNumPakFiles() const
{
	return numPaks;
}

bool FileManager::FileExists(const fs::path& Filename, bool bInPakOnly, const char* CategoryName) const
{
	if (!m_pData->defaultCategory.m_PakFilesList.size())
	{
		FileManager *This = const_cast<FileManager*>(this);
		This->CacheFiles();
	}

	FileManagerCategory* Category = GetCategory(CategoryName);

	auto it = Category->m_PakFilesMap.find(Filename);
	if (it != Category->m_PakFilesMap.end())
	{
		return true;
	}
	else if (!bInPakOnly)
	{
		// Find file locally
		for (GamePath* G = m_pGamePath; G != NULL; G = G->Next)
		{
			if (Category == &m_pData->defaultCategory || G->Category == Category)
			{
				fs::path path = G->Directory;
				path.append(FileHelpers::removeRootDir(Filename.c_str()));
				const std::string FullPath = path.generic_string();

				if (fs::exists(FullPath))
				{
					return true;
				}
			}
		}
	}

	return false;
}

IFilePtr FileManager::OpenFile(const fs::path& Filename, const char* CategoryName)
{
	if (!m_pData->defaultCategory.m_PakFilesList.size()) {
		CacheFiles();
	}

	FileManagerCategory* Category = GetCategory(CategoryName);

	auto it = Category->m_PakFilesMap.find(Filename);
	if (it != Category->m_PakFilesMap.end())
	{
		// Found file in pak
		PakFileEntry* entry = it->second;

		return SharedPtr<IFile>(new ArchiveFile(it->first, entry->ZipFile, &entry->Pos, entry->uncompressedSize));
	}
	else
	{
		// Find file locally
		for (GamePath* G = m_pGamePath; G != NULL; G = G->Next)
		{
			if (Category == &m_pData->defaultCategory || G->Category == Category)
			{
				fs::path path = G->Directory;
				path.append(FileHelpers::removeRootDir(Filename.c_str()));

				std::ifstream ifs;
				ifs.open(path, std::ios::binary);

				if (ifs.is_open())
				{
					return SharedPtr<IFile>(new NormalFile(Filename, std::move(ifs)));
				}
			}
		}

		MOHPC_LOG(Warn, "Tried to open non-existent file %s for category %s", Filename.generic_string().c_str(), CategoryName);
	}

	return IFilePtr();
}

static bool SortFileList(const fs::path& s1, const fs::path& s2)
{
	return FileNameCompare()(s1, s2);
}

template<typename T>
static bool AreFilesEqual(const T* s1, const T* s2)
{
	return !strHelpers::icmp(s1, s2);
}

static void InsertVirtualFile(const fs::path& dir, const fs::path& path, const fs::path& requestedDir, const ExtensionList& Extensions, std::set<FileEntry, FileNameCompare>& fileList)
{
	const bool bAnyExtension = Extensions.size() <= 0;

	if (!path.empty())
	{
		const fs::path::string_type pathString = path.native();
		const fs::path::value_type* virtualPath = FileNameCompare::StripBase(pathString.c_str(), dir.c_str()) - 1;

		if (!strHelpers::icmpn(virtualPath, requestedDir.c_str(), requestedDir.native().length()))
		{
			if (fs::is_directory(path))
			{
				fs::path virtualFolderPath = virtualPath;
				virtualFolderPath.append("/");
				fileList.insert(virtualFolderPath);
			}
			else
			{
				if (bAnyExtension)
				{
					fileList.insert(virtualPath);
				}
				else
				{
					const fs::path::value_type* fileExtension = FileHelpers::getExtension(virtualPath);
					for (size_t e = 0; e < Extensions.size(); e++)
					{
						if (!strHelpers::icmp(Extensions[e].c_str(), fileExtension))
						{
							fileList.insert(virtualPath);
						}
					}
				}
			}
		}
	}
}

FileEntryList FileManager::ListFilteredFiles(const fs::path& Directory, const ExtensionList& Extensions, bool bRecursive, bool bInPakOnly, const char* CategoryName) const
{
	FileManagerCategory* Category = GetCategory(CategoryName);

	if (!Category->m_PakFilesList.size())
	{
		FileManager *This = const_cast<FileManager*>(this);
		This->CacheFiles();
	}

	std::set<FileEntry, FileNameCompare> fileList;

	const bool bAnyExtension = Extensions.size() <= 0;

	const fs::path::value_type* requestedDirP = FileHelpers::removeRootDir(Directory.c_str());
	const size_t requestedDirLength = Directory.native().length() - (requestedDirP - Directory.c_str());

	bool bInDir = false;
	PakFileEntry **Entries = Category->m_PakFilesList.data();
	size_t numEntries = Category->m_PakFilesList.size();
	for (size_t i = 0; i < numEntries; i++)
	{
		const PakFileEntry *Entry = Entries[i];
		const fs::path::value_type* pathStringP = FileHelpers::removeRootDir(Entry->Name.c_str());
		const size_t rootLength = pathStringP - Entry->Name.c_str();
		if (!requestedDirLength || !strHelpers::icmpn(pathStringP, requestedDirP, requestedDirLength))
		{
			const size_t parentDir = FileNameCompare::GetParentDir(Entry->Name) - 1 - rootLength;
			if (bRecursive || !strHelpers::icmpn(pathStringP, requestedDirP, parentDir))
			{
				bInDir = true;

				if (bAnyExtension) {
					fileList.insert(Entry->Name);
				}
				else
				{
					const fs::path::value_type* fileExtension = FileHelpers::getExtension(Entry->Name.c_str());
					for (size_t e = 0; e < Extensions.size(); e++)
					{
						if (!strHelpers::icmp(Extensions[e].c_str(), fileExtension)) {
							fileList.insert(Entry->Name);
						}
					}
				}
			}
			else if (bInDir)
			{
				break;
			}
		}
		else if (bInDir)
		{
			break;
		}
	}

	if (!bInPakOnly)
	{
		for (GamePath* GP = m_pGamePath; GP != nullptr; GP = GP->Next)
		{
			const fs::path& currentDir = GP->Directory;
			fs::path requestedFullDir;
			if (*requestedDirP) {
				requestedFullDir = fs::path::string_type(currentDir).append(requestedDirP, 1, std::string::npos);
			}
			else {
				requestedFullDir = currentDir;
			}

			try
			{
				using namespace std::filesystem;

				if (bRecursive)
				{
					for (auto it = fs::recursive_directory_iterator(requestedFullDir, directory_options::skip_permission_denied | directory_options::follow_directory_symlink); it != fs::recursive_directory_iterator(); it++) {
						InsertVirtualFile(currentDir, it->path(), requestedDirP, Extensions, fileList);
					}
				}
				else
				{
					for (auto it = fs::directory_iterator(requestedFullDir, directory_options::skip_permission_denied | directory_options::follow_directory_symlink); it != fs::directory_iterator(); it++) {
						InsertVirtualFile(currentDir, it->path(), requestedDirP, Extensions, fileList);
					}
				}
			}
			catch (std::filesystem::filesystem_error&)
			{

			}
		}
	}

	const size_t numFiles = fileList.size();
	std::vector<FileEntry> out;
	out.resize(numFiles);
	std::move(fileList.begin(), fileList.end(), out.begin());

	return out;
}

FileEntryList FileManager::ListFilteredFiles(const fs::path& Directory, const fs::path& Extension, bool bRecursive, bool bInPakOnly, const char* CategoryName) const
{
	std::vector<fs::path> Extensions;

	if (Extension.native().length()) {
		Extensions.push_back(Extension);
	}

	return ListFilteredFiles(Directory, Extensions, bRecursive, bInPakOnly, CategoryName);
}

void FileManager::SortFileList(std::vector<FileEntry>& FileList)
{
	std::sort(FileList.begin(), FileList.end(), ::SortFileList);
	//FileList.erase(std::unique(FileList.begin(), FileList.end(), AreFilesEqual), FileList.end());

	// Remove duplicate file names
	for (size_t i = FileList.size(); i > 0; i--)
	{
		for (size_t j = i; j > 0; j--)
		{
			if (AreFilesEqual(FileList[i - 1].GetStr().native().c_str(), FileList[j - 1].GetStr().native().c_str()))
			{
				auto it = FileList.begin() + i;
				FileList.erase(it);
				break;
			}
		}
	}
}

fs::path FileManager::GetFixedPath(const fs::path& path) const
{
	using charT = fs::path::value_type;

	const charT* Filename = path.c_str();
	size_t pathLength = path.native().length();

	fs::path::string_type out;
	charT* start;
	charT* end;

	if (*Filename != '/' && *Filename != '\\')
	{
		out.resize(pathLength + 1);
		out[0] = '/';
		start = (charT*)out.c_str() + 1;
		end = (charT*)start + pathLength;
	}
	else
	{
		out = Filename;
		out.resize(pathLength);
		start = (charT*)out.c_str();
		end = (charT*)start + pathLength;
	}

	for (charT* p = start; p != end; p++)
	{
		if (*Filename != '\\')
		{
			*p = *Filename;
		}
		else
		{
			*p = '/';
		}
		Filename++;
	}

	return out;
}

void FileManager::GetCategoryList(std::vector<const char*>& OutList) const
{
	const size_t numCategories = m_pData->categoryList.size();
	for (size_t i = 0; i < numCategories; i++)
	{
		FileManagerCategory* pCategory = m_pData->categoryList[i];
		OutList.push_back(pCategory->categoryName.c_str());
	}
}

FileManagerCategory* FileManager::GetCategory(const char* CategoryName) const
{
	if (CategoryName == nullptr)
	{
		return &m_pData->defaultCategory;
	}
	else
	{
		const size_t numCategories = m_pData->categoryList.size();
		for (size_t i = 0; i < numCategories; i++)
		{
			FileManagerCategory* pCategory = m_pData->categoryList[i];
			if (pCategory->categoryName == CategoryName)
			{
				return pCategory;
			}
		}
	}

	return nullptr;
}

FileManagerCategory* FileManager::GetOrCreateCategory(const char* CategoryName)
{
	FileManagerCategory* Category = GetCategory(CategoryName);
	if (!Category)
	{
		Category = new FileManagerCategory;
		Category->categoryName = CategoryName;
		m_pData->categoryList.push_back(Category);
	}

	return Category;
}

void FileManager::CacheFiles()
{
	MOHPC_LOG(Info, "Caching files for the first time...");

	CacheFilesCategory(nullptr);

	const size_t numCategories = m_pData->categoryList.size();
	for (size_t i = 0; i < numCategories; i++)
	{
		CacheFilesCategory(m_pData->categoryList[i]);
	}
}

void FileManager::CacheFilesCategory(FileManagerCategory* Category)
{
	std::set<PakFileEntry*, PakFileEntryCompare> fileList;

	if (!Category)
	{
		for (PakFile *Pak = m_pPak; Pak != nullptr; Pak = Pak->Next)
		{
			size_t numFiles = Pak->entries.size();
			for (size_t i = 0; i < numFiles; i++)
			{
				PakFileEntry *Entry = Pak->entries[i];
				fileList.insert(Entry);
			}
		}

		CategorizeFiles(fileList, &m_pData->defaultCategory);
	}
	else
	{
		for (PakFile *Pak = m_pPak; Pak != nullptr; Pak = Pak->Next)
		{
			if (Pak->Category == Category)
			{
				size_t numFiles = Pak->entries.size();
				for (size_t i = 0; i < numFiles; i++)
				{
					PakFileEntry *Entry = Pak->entries[i];
					fileList.insert(Entry);
				}
			}
		}

		CategorizeFiles(fileList, Category);
	}
}

void FileManager::CategorizeFiles(std::set<PakFileEntry*, PakFileEntryCompare>& FileList, FileManagerCategory* Category)
{
	size_t numFiles = FileList.size();
	Category->m_PakFilesList.resize(numFiles);
	std::move(FileList.begin(), FileList.end(), Category->m_PakFilesList.begin());

	Category->m_PakFilesMap.reserve(numFiles);

	for (auto it = Category->m_PakFilesList.begin(); it != Category->m_PakFilesList.end(); it++)
	{
		PakFileEntry *Entry = *it;
		Category->m_PakFilesMap.emplace(Entry->Name, Entry);
	}
}

FileErrors::PathNotFound::PathNotFound(const std::filesystem::path& pathRef)
	: path(pathRef)
{
}

const std::filesystem::path& FileErrors::PathNotFound::getPath() const
{
	return path;
}
