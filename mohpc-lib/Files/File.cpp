#include <MOHPC/Files/File.h>
#include <MOHPC/Files/FileHelpers.h>

using namespace MOHPC;
namespace fs = std::filesystem;

FileEntryList::FileEntryList()
{
}

FileEntryList::FileEntryList(std::vector<FileEntry>&& inFileList) noexcept
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

FileEntry::FileEntry()
{
	bIsDir = false;
}

FileEntry::FileEntry(const fs::path::value_type* Filename) noexcept
{
	Name = Filename;
	bIsDir = !Name.has_filename();
}

FileEntry::FileEntry(const std::filesystem::path& Filename) noexcept
{
	Name = Filename;
	bIsDir = !Name.has_filename();
}

FileEntry::FileEntry(std::filesystem::path&& Filename) noexcept
{
	Name = std::move(Filename);
	bIsDir = !Name.has_filename();
}

FileEntry::~FileEntry()
{
}

bool FileEntry::IsDirectory() const
{
	return bIsDir;
}

const fs::path::value_type* FileEntry::GetExtension() const
{
	return FileHelpers::getExtension(Name.c_str());
}

const fs::path::value_type* FileEntry::GetRawName() const
{
	return Name.c_str();
}

const std::filesystem::path& FileEntry::getName() const
{
	return Name;
}

FileEntry::operator const std::filesystem::path& () const
{
	return Name;
}

IFile::IFile(const fs::path& fileName)
	: name(fileName)
{
}

IFile::~IFile()
{
}

const fs::path& IFile::getName() const
{
	return name;
}
