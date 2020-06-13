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

namespace fs = std::filesystem;

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
		str Name;
		str Hash;
		unzFile ZipFile;
		unz_file_pos Pos;
		size_t uncompressedSize;
	};

	struct PakFile
	{
		PakFile* Next;
		FileManagerCategory* Category;
		unzFile ZipFile;
		Container<PakFileEntry*> entries;
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

		static const char *StripBase(const std::string& path, const std::string& base)
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

		static size_t GetParentDir(const str& path)
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

		static const char *GetFilename(const str& path)
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

		static const char *GetExtension(const str& path)
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

		bool operator() (const str& s1, const str& s2) const
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
		bool operator() (const str& s1, const str& s2) const
		{
			return !stricmp(s1.c_str(), s2.c_str());
		}
	};

	struct FileNameHash
	{
		size_t operator()(const str& s1) const
		{
			size_t hash = 0;

			const char *p = s1.c_str();
			while (*p) {
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
		str categoryName;
		std::unordered_map<str, PakFileEntry*, FileNameHash, FileNameMapCompare> m_PakFilesMap;
		Container<PakFileEntry*> m_PakFilesList;
	};

	struct FileManagerData
	{
		FileManagerCategory defaultCategory;
		std::vector<FileManagerCategory*> categoryList;
	};
}

#ifdef _WIN32
static constexpr unsigned char PLATFORM_SLASH = '\\';
#define PLATFORM_SLASH_MACRO "\\"
#else
static constexpr unsigned char PLATFORM_SLASH = '/';
#define PLATFORM_SLASH_MACRO "/"
#endif

static bool HasTrailingSlash(const char* dir)
{
	const size_t dirLen = strlen(dir);

	const char last = dir[dirLen - 1];
	return last == PLATFORM_SLASH;
}

FileEntryList::FileEntryList()
{
}

FileEntryList::FileEntryList(MOHPC::Container<FileEntry>&& inFileList) noexcept
	: fileList(std::move(inFileList))
{
}

FileEntryList::~FileEntryList()
{
}

size_t FileEntryList::GetNumFiles() const
{
	return fileList.NumObjects();
}

const FileEntry* FileEntryList::GetFileEntry(size_t Index) const
{
	return Index < fileList.NumObjects() ? &fileList[Index] : nullptr;
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
		unzGetCurrentFileInfo(ZipFile, NULL, (char*)Entry->Name.c_str() + 1, FileInfo.size_filename, NULL, 0, NULL, 0);

		*entries++ = Entry;
	}

	numPaks++;

	return true;
}

bool FileManager::FillGameDirectory(const char* Directory)
{
	const char* gameDir;
	str gameDirStr;

	if(!HasTrailingSlash(Directory))
	{
		gameDirStr = Directory;
		gameDirStr += '/';
		gameDir = gameDirStr.c_str();
	}
	else {
		gameDir = Directory;
	}

	#define PSL PLATFORM_SLASH_MACRO

	bool success = true;
	success &= AddGameDirectory(str::printf("%smain", gameDir), "AA");
	success &= AddPakFile(str::printf("%smain" PSL "Pak0.pk3", gameDir), "AA");
	success &= AddPakFile(str::printf("%smain" PSL "Pak1.pk3", gameDir), "AA");
	success &= AddPakFile(str::printf("%smain" PSL "Pak2.pk3", gameDir), "AA");
	success &= AddPakFile(str::printf("%smain" PSL "Pak3.pk3", gameDir), "AA");
	success &= AddPakFile(str::printf("%smain" PSL "Pak4.pk3", gameDir), "AA");
	success &= AddPakFile(str::printf("%smain" PSL "Pak5.pk3", gameDir), "AA");
	success &= AddPakFile(str::printf("%smain" PSL "Pak6.pk3", gameDir), "AA");
	success &= AddPakFile(str::printf("%smainta" PSL "Pak1.pk3", gameDir), "SH");
	success &= AddPakFile(str::printf("%smainta" PSL "Pak2.pk3", gameDir), "SH");
	success &= AddPakFile(str::printf("%smainta" PSL "Pak3.pk3", gameDir), "SH");
	success &= AddPakFile(str::printf("%smainta" PSL "Pak4.pk3", gameDir), "SH");
	success &= AddPakFile(str::printf("%smainta" PSL "Pak5.pk3", gameDir), "SH");
	success &= AddPakFile(str::printf("%smaintt" PSL "Pak1.pk3", gameDir), "BT");
	success &= AddPakFile(str::printf("%smaintt" PSL "Pak2.pk3", gameDir), "BT");
	success &= AddPakFile(str::printf("%smaintt" PSL "Pak3.pk3", gameDir), "BT");
	success &= AddPakFile(str::printf("%smaintt" PSL "Pak4.pk3", gameDir), "BT");

	return success;
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
	if (!m_pData->defaultCategory.m_PakFilesList.NumObjects())
	{
		FileManager *This = (FileManager *)this;
		This->CacheFiles();
	}

	FileManagerCategory* Category = GetCategory(CategoryName);

	const str NewFilename = GetFixedPath(Filename);

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

FilePtr FileManager::OpenFile(const char* Filename, const char* CategoryName)
{
	if (!m_pData->defaultCategory.m_PakFilesList.NumObjects()) {
		CacheFiles();
	}

	FileManagerCategory* Category = GetCategory(CategoryName);

	const str pathString = GetFixedPath(Filename);

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

		return SharedPtr<File>(file);
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
					return SharedPtr<File>(file);
				}
			}
		}

		delete ifs;
	}

	return FilePtr();
}

str FileManager::GetFileHash(const char* Filename, const char* CategoryName)
{
	if (!m_pData->defaultCategory.m_PakFilesList.NumObjects())
	{
		FileManager *This = (FileManager *)this;
		This->CacheFiles();
	}

	FileManagerCategory* Category = GetCategory(CategoryName);

	const str pathString = GetFixedPath(Filename);

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

	str hashString;

	FilePtr File = OpenFile(Filename);
	if (File)
	{
		SHA256 Context;
		Context.Init();

		std::istream* stream = File->GetStream();
		while (!stream->eof())
		{
			size_t length = 64 * 1024;
			char *buf = new char[length];
			stream->read(buf, length);
			Context.Update((unsigned char*)buf, (size_t)stream->gcount());
			delete[] buf;
		}

		hashString = Context.Final().c_str();

		if (Entry)
		{
			Entry->Hash = hashString;
		}
	}

	return hashString;
}

static bool SortFileList(const str& s1, const str& s2)
{
	return FileNameCompare()(s1, s2);
}

static bool AreFilesEqual(const str& s1, const str& s2)
{
	return !stricmp(s1.c_str(), s2.c_str());
}

static void InsertVirtualFile(const std::string& dir, const fs::path& path, const std::string& requestedDir, const Container<str>& Extensions, std::set<FileEntry, FileNameCompare>& fileList)
{
	const bool bAnyExtension = Extensions.NumObjects() <= 0;

	if (!path.empty())
	{
		const std::string pathString = path.generic_string();
		const char *virtualPath = FileNameCompare::StripBase(pathString, dir) - 1;

		if (!strnicmp(virtualPath, requestedDir.c_str(), requestedDir.length()))
		{
			if (fs::is_directory(path))
			{
				const std::string virtualFolderPath = std::string(virtualPath) + "/";
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
					for (size_t e = 0; e < Extensions.NumObjects(); e++)
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

FileEntryList FileManager::ListFilteredFiles(const char* Directory, const MOHPC::Container<str>& Extensions, bool bRecursive, bool bInPakOnly, const char* CategoryName) const
{
	FileManagerCategory* Category = GetCategory(CategoryName);

	if (!Category->m_PakFilesList.NumObjects())
	{
		FileManager *This = const_cast<FileManager*>(this);
		This->CacheFiles();
	}

	if (*Directory != '/' && *Directory != '\\') {
		return Container<FileEntry>();
	}

	std::set<FileEntry, FileNameCompare> fileList;

	const bool bAnyExtension = Extensions.NumObjects() <= 0;

	fs::path requestedPath;

	if (*Directory != '/' && *Directory != '\\') {
		requestedPath = std::string("/") + Directory;
	}
	else {
		requestedPath = Directory;
	}

	{
		const std::string nativePath = requestedPath.generic_string();
		const char end = *(nativePath.end() - 1);
		if (end != '/' && end != '\\')
		{
			requestedPath.append("");
		}
	}

	const std::string requestedDir = requestedPath.generic_string();
	const char *requestedDirP = requestedDir.c_str();
	size_t requestedDirLength = requestedDir.length();

	bool bInDir = false;
	PakFileEntry **Entries = Category->m_PakFilesList.Data();
	size_t numEntries = Category->m_PakFilesList.NumObjects();
	for (size_t i = 0; i < numEntries; i++)
	{
		const PakFileEntry *Entry = Entries[i];
		const char *pathStringP = Entry->Name.c_str();
		if ((requestedDirLength == 1 && requestedDir[0] == '/') || !strnicmp(pathStringP, requestedDirP, requestedDirLength))
		{
			if (bRecursive || !strnicmp(pathStringP, requestedDirP, FileNameCompare::GetParentDir(Entry->Name)))
			{
				bInDir = true;

				if (bAnyExtension) {
					fileList.insert(Entry->Name);
				}
				else
				{
					const char *fileExtension = FileNameCompare::GetExtension(Entry->Name);
					for (size_t e = 0; e < Extensions.NumObjects(); e++)
					{
						if (!stricmp(Extensions[e].c_str(), fileExtension)) {
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
			const std::string currentDir = GP->Directory.generic_string();
			const std::string requestedFullDir = std::string(currentDir).append(requestedDir, 1, std::string::npos);

			if (bRecursive)
			{
				for (auto it = fs::recursive_directory_iterator(requestedFullDir, std::filesystem::directory_options::skip_permission_denied); it != fs::recursive_directory_iterator(); it++) {
					InsertVirtualFile(currentDir, it->path(), requestedDir, Extensions, fileList);
				}
			}
			else
			{
				for (auto it = fs::directory_iterator(requestedFullDir, std::filesystem::directory_options::skip_permission_denied); it != fs::directory_iterator(); it++) {
					InsertVirtualFile(currentDir, it->path(), requestedDir, Extensions, fileList);
				}
			}
		}
	}

	const size_t numFiles = fileList.size();
	Container<FileEntry> out;
	out.SetNumObjects(numFiles);
	std::move(fileList.begin(), fileList.end(), out.begin());

	return out;
}

FileEntryList FileManager::ListFilteredFiles(const char* Directory, const char* Extension, bool bRecursive, bool bInPakOnly, const char* CategoryName) const
{
	Container<str> Extensions;

	if (*Extension && strlen(Extension)) {
		Extensions.AddObject(Extension);
	}

	return ListFilteredFiles(Directory, Extensions, bRecursive, bInPakOnly, CategoryName);
}

void FileManager::SortFileList(Container<FileEntry>& FileList)
{
	std::sort(FileList.begin(), FileList.end(), ::SortFileList);
	//FileList.erase(std::unique(FileList.begin(), FileList.end(), AreFilesEqual), FileList.end());

	// Remove duplicate file names
	for (size_t i = FileList.NumObjects(); i > 0; i--)
	{
		for (size_t j = i; j > 0; j--)
		{
			if (AreFilesEqual(FileList[i].GetStr(), FileList[j].GetStr()))
			{
				FileList.RemoveObjectAt(i);
				break;
			}
		}
	}
}

str FileManager::GetFixedPath(const str& Path) const
{
	const char *Filename = Path.c_str();
	size_t pathLength = Path.length();

	str out;
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

void FileManager::GetCategoryList(Container<const char*>& OutList) const
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

str FileManager::SetFileExtension(const char* Filename, const char* NewExtension)
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

	return newfile.c_str();
}

str FileManager::GetDefaultFileExtension(const char* Filename, const char* DefaultExtension)
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

	const str newfile = Filename + str(".") + DefaultExtension;

	return newfile.c_str();
}

str FileManager::CanonicalFilename(const char* Filename)
{
	str newString;
	newString.reserve(strlen(Filename) + 1);

	for(const char* p = Filename; *p; p++)
	{
		newString += *p;
		while (p[0] == '/' && p[1] == '/')
		{
			p++;
		}
	}

	//newString.shrink_to_fit();
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
	Category->m_PakFilesList.SetNumObjectsUninitialized(numFiles);
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
	bIsDir = false;
}

FileEntry::FileEntry(const char *Filename) noexcept
{
	Name = Filename;

	const char *ptr = Name.c_str();
	const size_t endPos = Name.length() - 1;
	bIsDir = ptr[endPos] == '/' || ptr[endPos] == '\\';
}

FileEntry::FileEntry(const str& Filename) noexcept
{
	Name = Filename;

	const char *ptr = Name.c_str();
	const size_t endPos = Name.length() - 1;
	bIsDir = ptr[endPos] == '/' || ptr[endPos] == '\\';
}

FileEntry::FileEntry(str&& Filename) noexcept
{
	Name = std::move(Filename);

	const char* ptr = Name.c_str();
	const size_t endPos = Name.length() - 1;
	bIsDir = ptr[endPos] == '/' || ptr[endPos] == '\\';
}

FileEntry::~FileEntry()
{
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
	return Name.c_str();
}

const str& FileEntry::GetStr() const
{
	return Name;
}

FileEntry::operator const str& () const
{
	return Name;
}
