#include "NormalFile.h"

using namespace MOHPC;

NormalFile::NormalFile(fs::path&& nameRef, std::ifstream&& fileRef)
	: IFile(std::move(nameRef))
	, fileStream(std::move(fileRef))
	, buffer(nullptr)
	, streamSize(0)

{
}

NormalFile::~NormalFile()
{
	if (buffer)
	{
		delete[] buffer;
	}
}

std::istream* NormalFile::GetStream()
{
	return &fileStream;
}

uint64_t NormalFile::ReadBuffer(void** Out)
{
	uint64_t length = 0;

	if (!buffer)
	{
		std::istream* stream = GetStream();

		if (!streamSize)
		{
			stream->seekg(0, stream->end);
			streamSize = stream->tellg();
			stream->seekg(0, stream->beg);
		}
		length = streamSize;

		buffer = new char[(size_t)streamSize + 1];
		if (buffer)
		{
			stream->read(buffer, streamSize);
		}
		buffer[length] = 0;
	}

	*Out = buffer;
	return length;
}
