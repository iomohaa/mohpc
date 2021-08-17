#include "ArchiveFile.h"
#include "decompression_streambuf.h"

using namespace MOHPC;

ArchiveFile::ArchiveFile(const fs::path& nameRef, void* file, void* pos, uint64_t uncompressedSize)
	: IFile(nameRef)
	, streamBuf(new basic_decompression_buf<char>(file, *(unz_file_pos*)pos, uncompressedSize))
	, archiveStream(streamBuf)
	, buffer(nullptr)
	, archiveSize(uncompressedSize)

{
}

ArchiveFile::~ArchiveFile()
{
	if (buffer)
	{
		delete[] buffer;
	}
}

std::istream* ArchiveFile::GetStream()
{
	return &archiveStream;
}

uint64_t ArchiveFile::ReadBuffer(void** Out)
{
	uint64_t length = 0;

	if (!buffer)
	{
		std::istream* stream = GetStream();

		if (!archiveSize)
		{
			stream->seekg(0, stream->end);
			archiveSize = stream->tellg();
			stream->seekg(0, stream->beg);
		}
		length = archiveSize;

		buffer = new char[(size_t)archiveSize + 1];
		if (buffer)
		{
			stream->read(buffer, archiveSize);
		}
		buffer[length] = 0;
	}

	*Out = buffer;
	return length;
}
