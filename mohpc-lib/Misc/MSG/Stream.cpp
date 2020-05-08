#include <MOHPC/Global.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <cstring>
#include <algorithm>

using namespace MOHPC;

StreamOverflowException::StreamOverflowException(Mode inMode) noexcept
	: mode(inMode)
{
}

StreamOverflowException::Mode StreamOverflowException::getMode() const noexcept
{
	return mode;
}

FixedDataMessageStream::FixedDataMessageStream(void* data, size_t size) noexcept
	: storage((uint8_t*)data)
	, length(size)
	, curPos(0)
{
}

void FixedDataMessageStream::Read(void* data, size_t readLen)
{
	if (curPos + readLen - 1 > length) {
		throw StreamOverflowException(StreamOverflowException::Reading);
	}

	std::memcpy(data, storage + curPos, readLen);
	curPos += readLen;
}

void FixedDataMessageStream::Write(const void* data, size_t writeLen)
{
	if (curPos + writeLen > length) {
		throw StreamOverflowException(StreamOverflowException::Writing);
	}

	std::memcpy(storage + curPos, data, writeLen);
	curPos += writeLen;
}

void FixedDataMessageStream::Seek(size_t offset, SeekPos from) noexcept
{
	switch (from)
	{
	default:
	case SeekPos::Begin:
		curPos = std::min(length, offset);
		break;
	case SeekPos::Current:
		curPos = std::min(length, curPos + offset);
		break;
	case SeekPos::End:
		curPos = std::min(length, length - offset);
		break;
	}
}

size_t FixedDataMessageStream::GetLength() const noexcept
{
	return length;
}

size_t MOHPC::FixedDataMessageStream::GetPosition() const noexcept
{
	return curPos;
}

MOHPC::DynamicDataMessageStream::DynamicDataMessageStream() noexcept
	: storage(nullptr)
	, length(0)
	, curPos(0)
{
}

MOHPC::DynamicDataMessageStream::~DynamicDataMessageStream() noexcept
{
	if (storage) delete[] storage;
}

void MOHPC::DynamicDataMessageStream::Read(void* data, size_t readLen)
{
	if (curPos + readLen - 1 > length) {
		throw StreamOverflowException(StreamOverflowException::Reading);
	}

	std::memcpy(data, storage + curPos, readLen);
	curPos += readLen;
}

void MOHPC::DynamicDataMessageStream::Write(const void* data, size_t writeLen)
{
	if (curPos + writeLen > length)
	{
		if (storage)
		{
			size_t newLen = (length + writeLen) * 2;
			uint8_t* newStorage = new uint8_t[newLen];
			memcpy(newStorage, storage, length);
			delete[] storage;
			storage = newStorage;
			length = newLen;
		}
		else
		{
			length = 4 + writeLen;
			storage = new uint8_t[length];
		}
	}

	std::memcpy(storage + curPos, data, writeLen);
	curPos += writeLen;
}

void MOHPC::DynamicDataMessageStream::Seek(size_t offset, SeekPos from) noexcept
{

}

size_t MOHPC::DynamicDataMessageStream::GetPosition() const noexcept
{
	return curPos;
}

size_t MOHPC::DynamicDataMessageStream::GetLength() const noexcept
{
	return length;
}

const uint8_t* DynamicDataMessageStream::getStorage() const
{
	return storage;
}
