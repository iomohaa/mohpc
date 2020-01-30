#include <Shared.h>
#include <fstream>
#include <unordered_map>
#include <set>
#include <MOHPC/Managers/FileManager.h>
#include <zlib/contrib/minizip/unzip.h>
#include <filesystem>
#include "../Misc/decompression_streambuf.h"
#include <string.h>
#include <MOHPC/Misc/SHA256.h>

using namespace MOHPC;
using namespace std;

namespace fs = filesystem;

namespace MOHPC
{
	struct FileData
	{
		basic_decompression_buf<char> streamBuf;
		std::istream* LinkedStream;
		char* Buffer;
		std::streamoff BufferSize;
	};

	struct GamePath
	{
		GamePath* Next;
		FileManagerCategory* Category;
		fs::path Directory;
	};

	struct PakFileEntry
	{
		std::string Name;
		std::string Hash;
		unzFile ZipFile;
		unz_file_pos Pos;
		size_t uncompressedSize;
	};

	struct PakFile
	{
		PakFile* Next;
		FileManagerCategory* Category;
		unzFile ZipFile;
		vector<PakFileEntry*> entries;
	};

	struct FileNameCompare
	{
		static bool CompareCharPath(char c1, char c2)
		{
			return tolower(c1) < tolower(c2);
		}

		static bool CompareCharPathEquality(char c1, char c2)
		{
			return tolower(c1) == tolower(c2);
		}

		static const char *StripBase(const string& path, const string& base)
		{
			const char *p1 = path.c_str();
			const char *p2 = base.c_str();

			while (*p1 && *p2)
			{
				if (tolower(*p1++) != tolower(*p2++))
				{
					return p1;
				}
			}

			return p1;
		}

		static size_t GetParentDir(const string& path)
		{
			const char *str = path.c_str();
			for (const char *p = str + path.length() - 1; p != str - 1; p--)
			{
				if (*p == '/')
				{
					return p - str + 1;
				}
			}

			return 0;
		}

		static const char *GetFilename(const string& path)
		{
			const char *str = path.c_str();
			for (const char *p = str + path.length() - 1; p != str - 1; p--)
			{
				if (*p == '/')
				{
					return p + 1;
				}
			}

			return str;
		}

		static const char *GetExtension(const string& path)
		{
			const char *str = path.c_str();
			for (const char *p = str + path.length() - 1; p != str - 1; p--)
			{
				if (*p == '.')
				{
					return p + 1;
				}
			}

			return "";
		}

		static const char *GetExtension(const char* path)
		{
			size_t length = strlen(path);
			for (const char *p = path + length - 1; p != path - 1; p--)
			{
				if (*p == '.')
				{
					return p + 1;
				}
			}

			return "";
		}

		bool operator() (const std::string& s1, const std::string& s2) const
		{
			const size_t parentdirl1 = GetParentDir(s1);
			const size_t parentdirl2 = GetParentDir(s2);
			char *p1 = (char *)s1.c_str();
			char *p2 = (char *)s2.c_str();
			char *pend1 = p1 + parentdirl1;
			char *pend2 = p2 + parentdirl2;
			const char ch1 = *pend1;
			const char ch2 = *pend2;
			//int comp = ls1.compare(0, parentdirl1, ls2.c_str(), parentdirl2);
			//bool comp = std::equal(p1 + 1, p1 + parentdirl1, p2 + 1, p2 + parentdirl2, &CompareCharPathEquality);

			// HACK HACK : Faster than std::equal combined with std::lexicographical_compare
			*pend1 = 0;
			*pend2 = 0;
			int comp = stricmp(p1 + 1, p2 + 1);
			*pend1 = ch1;
			*pend2 = ch2;
			if (!comp)
			{
				const char *fname1 = GetFilename(s1);
				const char *fname2 = GetFilename(s2);

				if ((*fname1 == '.') ^ (*fname2 == '.'))
				{
					return *fname1 == '.';
				}
				else
				{
					return stricmp(fname1, fname2) < 0;
				}
			}
			return comp < 0;
			//return std::lexicographical_compare(p1 + 1, p1 + parentdirl1, p2 + 1, p2 + parentdirl2, &CompareCharPath);
		}
	};

	struct FileNameMapCompare
	{
		bool operator() (const std::string& s1, const std::string& s2) const
		{
			return !stricmp(s1.c_str(), s2.c_str());
		}
	};

	struct FileNameHash
	{
		size_t operator()(const std::string& s1) const
		{
			size_t hash = 0;

			const char *p = s1.c_str();
			while (*p)
			{
				hash = tolower(*p++) + 31 * hash;
			}

			return hash;
		}
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

			const char *p = e->Name.c_str();
			while (*p)
			{
				hash = *p++;
			}

			return hash;
		}
	};

	struct FileManagerCategory
	{
		std::string categoryName;
		std::unordered_map<std::string, PakFileEntry*, FileNameHash, FileNameMapCompare> m_PakFilesMap;
		std::vector<PakFileEntry*> m_PakFilesList;
	};

	struct FileManagerData
	{
		FileManagerCategory defaultCategory;
		std::vector<FileManagerCategory*> categoryList;
	};

}

FileEntryList::FileEntryList()
{
}

FileEntryList::FileEntryList(std::vector<FileEntry>&& inFileList)
	: fileList(std::move(inFileList))
{
}

FileEntryList::~FileEntryList()
{
}

size_t FileEntryList::GetNumFiles() const
{
	return fileList.size();
}

const FileEntry* FileEntryList::GetFileEntry(size_t Index) const
{
	return Index < fileList.size() ? &fileList[Index] : nullptr;
}

File::File()
{
	m_data = new FileData;
	m_data->LinkedStream = NULL;
	m_data->Buffer = NULL;
	m_bInPak = false;
}

File::~File()
{
	/*
	if (m_data->ZipEntry != NULL)
	{
		m_data->ZipEntry->CloseDecompressionStream();
	}
	else
	*/
	{
		delete m_data->LinkedStream;
	}

	if (m_data->Buffer)
	{
		delete[] m_data->Buffer;
	}

	delete m_data;
}

bool File::IsInPak() const
{
	return m_bInPak;
}

std::istream* File::GetStream() const
{
	return m_data->LinkedStream;
}

std::streamsize File::ReadBuffer(void** Out)
{
	std::streamsize length = 0;

	if (!m_data->Buffer)
	{
		std::istream* stream = GetStream();

		if (!m_data->BufferSize)
		{
			stream->seekg(0, stream->end);
			m_data->BufferSize = stream->tellg();
			stream->seekg(0, stream->beg);
		}
		length = m_data->BufferSize;

		m_data->Buffer = new char[(size_t)m_data->BufferSize + 1];
		if (m_data->Buffer)
		{
			stream->read(m_data->Buffer, m_data->BufferSize);
		}
		m_data->Buffer[length] = 0;
	}

	*Out = m_data->Buffer;
	return length;
}

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

	for (PakFile* P = m_pPak; P != NULL; P = tmppak)
	{
		tmppak = P->Next;
		unzClose(P->ZipFile);

		for (size_t i = 0; i < P->entries.size(); i++)
		{
			delete P->entries[i];
		}

		delete P;
	}

	GamePath* tmpsp;

	for (GamePath* G = m_pGamePath; G != NULL; G = tmpsp)
	{
		tmpsp = G->Next;
		delete G;
	}

	const size_t numCategories = m_pData->categoryList.size();
	for (size_t i = 0; i < numCategories; i++)
	{
		FileManagerCategory* pCategory = m_pData->categoryList[i];
		delete pCategory;
	}

	delete m_pData;
}

bool FileManager::AddGameDirectory(const char* Directory, const char* CategoryName)
{
	if (!fs::exists(Directory))
	{
		return false;
	}

	GamePath* G = new GamePath;
	G->Directory = Directory;
	if (G->Directory.has_filename()) G->Directory.append("");
	G->Category = GetOrCreateCategory(CategoryName);
	G->Next = m_pGamePath;
	m_pGamePath = G;
	numGamePaths++;

	return true;
}

bool FileManager::AddPakFile(const char* Filename, const char* CategoryName)
{
	unzFile ZipFile = unzOpen(Filename);
	if (!ZipFile)
	{
		return false;
	}

	PakFile* Pak = new PakFile;
	Pak->Category = GetOrCreateCategory(CategoryName);
	Pak->ZipFile = ZipFile;
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

		Entry->Name.resize(FileInfo.size_filename + 1);
		Entry->Name[0] = '/';
		Entry->uncompressedSize = FileInfo.uncompressed_size;
		unzGetCurrentFileInfo(ZipFile, NULL, (char*)Entry->Name.data() + 1, FileInfo.size_filename, NULL, 0, NULL, 0);

		*entries++ = Entry;
	}

	numPaks++;

	return true;
}


size_t FileManager::GetNumDirectories() const
{
	return numGamePaths;
}

size_t FileManager::GetNumPakFiles() const
{
	return numPaks;
}

bool FileManager::FileExists(const char* Filename, bool bInPakOnly, const char* CategoryName) const
{
	if (!m_pData->defaultCategory.m_PakFilesList.size())
	{
		FileManager *This = (FileManager *)this;
		This->CacheFiles();
	}

	FileManagerCategory* Category = GetCategory(CategoryName);

	const string NewFilename = GetFixedPath(Filename);

	auto it = Category->m_PakFilesMap.find(NewFilename);
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
				path.append(NewFilename.c_str() + 1);
				const string FullPath = path.generic_string();

				if (fs::exists(FullPath))
				{
					return true;
				}
			}
		}
	}

	return false;
}

FilePtr FileManager::OpenFile(const char* Filename, const char* CategoryName)
{
	if (!m_pData->defaultCategory.m_PakFilesList.size())
	{
		CacheFiles();
	}

	FileManagerCategory* Category = GetCategory(CategoryName);

	const string pathString = GetFixedPath(Filename);

	auto it = Category->m_PakFilesMap.find(pathString);
	if (it != Category->m_PakFilesMap.end())
	{
		// Found file in pak
		File* file = new File;
		PakFileEntry *Entry = it->second;

		file->m_data->streamBuf.init(Entry->ZipFile, Entry->Pos, Entry->uncompressedSize);
		file->m_data->BufferSize = Entry->uncompressedSize;
		std::istream* stream = new std::istream(&file->m_data->streamBuf);

		file->m_data->LinkedStream = stream;
		file->m_bInPak = true;

		return shared_ptr<File>(file);
	}
	else
	{
		std::ifstream* ifs = new std::ifstream;

		// Find file locally
		for (GamePath* G = m_pGamePath; G != NULL; G = G->Next)
		{
			if (Category == &m_pData->defaultCategory || G->Category == Category)
			{
				fs::path path = G->Directory;
				path.append(pathString.c_str() + 1);
				std::string FullPath = path.generic_string();

				ifs->open(FullPath, std::ios::binary);

				if (ifs->is_open())
				{
					File* file = new File;
					file->m_data->BufferSize = 0;
					file->m_data->LinkedStream = ifs;
					return shared_ptr<File>(file);
				}
			}
		}

		delete ifs;
	}

	return FilePtr();
}

std::string FileManager::GetFileHash(const char* Filename, const char* CategoryName)
{
	if (!m_pData->defaultCategory.m_PakFilesList.size())
	{
		FileManager *This = (FileManager *)this;
		This->CacheFiles();
	}

	FileManagerCategory* Category = GetCategory(CategoryName);

	const string pathString = GetFixedPath(Filename);

	PakFileEntry* Entry = nullptr;

	auto it = Category->m_PakFilesMap.find(pathString);
	if (it != Category->m_PakFilesMap.end())
	{
		Entry = it->second;

		if (Entry->Hash.length())
		{
			return Entry->Hash;
		}
	}

	string hashString;

	FilePtr File = OpenFile(Filename);
	if (File)
	{
		SHA256 Context;
		Context.Init();

		istream* stream = File->GetStream();
		while (!stream->eof())
		{
			size_t length = 64 * 1024;
			char *buf = new char[length];
			stream->read(buf, length);
			Context.Update((unsigned char*)buf, stream->gcount());
			delete[] buf;
		}

		hashString = Context.Final();

		if (Entry)
		{
			Entry->Hash = hashString;
		}
	}

	return hashString;
}

static bool SortFileList(const string& s1, const string& s2)
{
	return FileNameCompare()(s1, s2);
}

static bool AreFilesEqual(const string& s1, const string& s2)
{
	return !stricmp(s1.c_str(), s2.c_str());
}

static void InsertVirtualFile(const string& dir, const fs::path& path, const string& requestedDir, const std::vector<string>& Extensions, std::set<FileEntry, FileNameCompare>& fileList)
{
	const bool bAnyExtension = Extensions.size() <= 0;

	if (!path.empty())
	{
		const string pathString = path.generic_string();
		const char *virtualPath = FileNameCompare::StripBase(pathString, dir) - 1;

		if (!strnicmp(virtualPath, requestedDir.c_str(), requestedDir.length()))
		{
			if (fs::is_directory(path))
			{
				const string virtualFolderPath = string(virtualPath) + "/";
				fileList.insert(virtualFolderPath.c_str());
			}
			else
			{
				if (bAnyExtension)
				{
					fileList.insert(virtualPath);
				}
				else
				{
					const char *fileExtension = FileNameCompare::GetExtension(virtualPath);
					for (size_t e = 0; e < Extensions.size(); e++)
					{
						if (!stricmp(Extensions[e].c_str(), fileExtension))
						{
							fileList.insert(virtualPath);
						}
					}
				}
			}
		}
	}
}

FileEntryList FileManager::ListFilteredFiles(const char* Directory, const vector<string>& Extensions, bool bRecursive, bool bInPakOnly, const char* CategoryName) const
{
	FileManagerCategory* Category = GetCategory(CategoryName);

	if (!Category->m_PakFilesList.size())
	{
		FileManager *This = (FileManager *)this;
		This->CacheFiles();
	}

	if (*Directory != '/' && *Directory != '\\')
	{
		return vector<FileEntry>();
	}

	std::set<FileEntry, FileNameCompare> fileList;

	const bool bAnyExtension = Extensions.size() <= 0;

	fs::path requestedPath;

	if (*Directory != '/' && *Directory != '\\')
	{
		requestedPath = string("/") + Directory;
	}
	else
	{
		requestedPath = Directory;
	}
	{
		const string nativePath = requestedPath.generic_string();
		const char end = *(nativePath.end() - 1);
		if (end != '/' && end != '\\')
		{
			requestedPath.append("");
		}
	}

	const string requestedDir = requestedPath.generic_string();
	const char *requestedDirP = requestedDir.c_str();
	size_t requestedDirLength = requestedDir.length();

	bool bInDir = false;
	PakFileEntry **Entries = Category->m_PakFilesList.data();
	size_t numEntries = Category->m_PakFilesList.size();
	for (size_t i = 0; i < numEntries; i++)
	{
		const PakFileEntry *Entry = Entries[i];
		const char *pathStringP = Entry->Name.c_str();
		if ((requestedDirLength == 1 && requestedDir[0] == '/') || !strnicmp(pathStringP, requestedDirP, requestedDirLength))
		{
			if (bRecursive || !strnicmp(pathStringP, requestedDirP, FileNameCompare::GetParentDir(Entry->Name)))
			{
				bInDir = true;

				if (bAnyExtension)
				{
					fileList.insert(Entry->Name);
				}
				else
				{
					const char *fileExtension = FileNameCompare::GetExtension(Entry->Name);
					for (size_t e = 0; e < Extensions.size(); e++)
					{
						if (!stricmp(Extensions[e].c_str(), fileExtension))
						{
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
			const string currentDir = GP->Directory.generic_string();
			const string requestedFullDir = string(currentDir).append(requestedDir, 1, std::string::npos);

			if (bRecursive)
			{
				for (auto it = fs::recursive_directory_iterator(requestedFullDir, std::filesystem::directory_options::skip_permission_denied); it != fs::recursive_directory_iterator(); it++)
				{
					InsertVirtualFile(currentDir, it->path(), requestedDir, Extensions, fileList);
				}
			}
			else
			{
				for (auto it = fs::directory_iterator(requestedFullDir, std::filesystem::directory_options::skip_permission_denied); it != fs::directory_iterator(); it++)
				{
					InsertVirtualFile(currentDir, it->path(), requestedDir, Extensions, fileList);
				}
			}
		}
	}

	size_t numFiles = fileList.size();
	vector<FileEntry> out(numFiles);
	std::move(fileList.begin(), fileList.end(), out.begin());

	return out;
}

FileEntryList FileManager::ListFilteredFiles(const char* Directory, const char* Extension, bool bRecursive, bool bInPakOnly, const char* CategoryName) const
{
	vector<string> Extensions;

	if (*Extension && strlen(Extension))
	{
		Extensions.push_back(Extension);
	}

	return ListFilteredFiles(Directory, Extensions, bRecursive, bInPakOnly, CategoryName);
}

void FileManager::SortFileList(std::vector<FileEntry>& FileList)
{
	std::sort(FileList.begin(), FileList.end(), ::SortFileList);
	FileList.erase(std::unique(FileList.begin(), FileList.end(), AreFilesEqual), FileList.end());
}

string FileManager::GetFixedPath(const string& Path) const
{
	const char *Filename = Path.c_str();
	size_t pathLength = Path.length();

	string out;
	char *start;
	char *end;

	if (*Filename != '/' && *Filename != '\\')
	{
		out.resize(pathLength + 1);
		out[0] = '/';
		start = (char*)out.c_str() + 1;
		end = (char*)start + pathLength;
	}
	else
	{
		out = Filename;
		out.resize(pathLength);
		start = (char*)out.c_str();
		end = (char*)start + pathLength;
	}

	for (char *p = start; p != end; p++)
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

const char *FileManager::GetFileExtension(const char* Filename)
{
	const char *p = Filename + strlen(Filename);
	while (*p != '.')
	{
		if (*p == '/' || p == Filename)
		{
			return "";
		}
		p--;
	}

	return p + 1;
}

std::string FileManager::SetFileExtension(const char* Filename, const char* NewExtension)
{
	const size_t len = strlen(Filename);
	const char *p = Filename + len;
	while (*p != '.')
	{
		if (*p == '/' || p == Filename)
		{
			break;
		}
		p--;
	}

	const size_t newlen = p <= Filename ? len : (p - Filename);
	const std::string newfile = std::string().assign(Filename, newlen) + "." + std::string(NewExtension);

	return newfile;
}

std::string FileManager::GetDefaultFileExtension(const char* Filename, const char* DefaultExtension)
{
	const char *p = Filename + strlen(Filename) - 1;

	while (*p != '/' && p != Filename)
	{
		if (*p == '.')
		{
			return Filename;
		}
		p--;
	}

	const std::string newfile = std::string(Filename) + "." + std::string(DefaultExtension);

	return newfile;
}

std::string FileManager::CanonicalFilename(const char* Filename)
{
	std::string newString;
	newString.reserve(strlen(Filename) + 1);

	for(const char* p = Filename; *p; p++)
	{
		newString += *p;
		while (p[0] == '/' && p[1] == '/')
		{
			p++;
		}
	}

	newString.shrink_to_fit();
	return newString;
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

FileEntry::FileEntry()
{
	Name = nullptr;
	bIsRef = false;
	bIsDir = false;
}

FileEntry::FileEntry(const char *Filename)
{
	Name = new string(Filename);
	bIsRef = false;

	const char *ptr = Name->c_str();
	const size_t endPos = Name->length() - 1;
	bIsDir = ptr[endPos] == '/' || ptr[endPos] == '\\';
}

FileEntry::FileEntry(const string& Filename)
{
	Name = &Filename;
	bIsRef = true;

	const char *ptr = Name->c_str();
	const size_t endPos = Name->length() - 1;
	bIsDir = ptr[endPos] == '/' || ptr[endPos] == '\\';
}

FileEntry::FileEntry(const FileEntry& Entry)
{
	if (Entry.bIsRef)
	{
		Name = Entry.Name;
		bIsRef = true;
	}
	else
	{
		Name = new string(*Entry.Name);
		bIsRef = false;
	}
	bIsDir = Entry.bIsDir;
}

FileEntry::FileEntry(FileEntry&& Entry)
{
	Name = Entry.Name;
	bIsRef = Entry.bIsRef;
	bIsDir = Entry.bIsDir;
	Entry.Name = nullptr;
}

FileEntry& FileEntry::operator=(const FileEntry& Entry)
{
	if (Entry.bIsRef)
	{
		Name = Entry.Name;
		bIsRef = true;
	}
	else
	{
		Name = new string(*Entry.Name);
		bIsRef = false;
	}
	bIsDir = Entry.bIsDir;
	return *this;
}

FileEntry& FileEntry::operator=(FileEntry&& Entry)
{
	Name = Entry.Name;
	bIsDir = Entry.bIsDir;
	bIsRef = Entry.bIsRef;
	Entry.Name = nullptr;
	return *this;
}

FileEntry::~FileEntry()
{
	if (Name && !bIsRef)
	{
		delete Name;
	}
}

bool FileEntry::IsDirectory() const
{
	return bIsDir;
}

const char *FileEntry::GetExtension() const
{
	return FileNameCompare::GetExtension(*Name);
}

const char *FileEntry::GetRawName() const
{
	return Name->c_str();
}

FileEntry::operator const string&() const
{
	return *Name;
}

FileEntry::operator const char *() const
{
	return Name->c_str();
}
