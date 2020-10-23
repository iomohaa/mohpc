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

FixedDataMessageStream::FixedDataMessageStream(void* data, size_t size, size_t readableLength) noexcept
	: storage((uint8_t*)data)
	, length(readableLength)
	, maxLength(size)
	, curPos(0)
{
}

void FixedDataMessageStream::Read(void* data, size_t readLen)
{
	if(readLen)
	{
		if (curPos + readLen - 1 > std::min(length, maxLength)) {
			throw StreamOverflowException(StreamOverflowException::Reading);
		}

		std::memcpy(data, storage + curPos, readLen);
		curPos += readLen;
	}
}

void FixedDataMessageStream::Write(const void* data, size_t writeLen)
{
	if(writeLen)
	{
		if (curPos + writeLen > maxLength) {
			throw StreamOverflowException(StreamOverflowException::Writing);
		}

		std::memcpy(storage + curPos, data, writeLen);
		curPos += writeLen;
		// set the appropriate length
		if(curPos > length) length = curPos;
	}
}

void FixedDataMessageStream::Seek(size_t offset, SeekPos from) noexcept
{
	switch (from)
	{
	default:
	case SeekPos::Begin:
		curPos = std::min(maxLength, offset);
		break;
	case SeekPos::Current:
		curPos = std::min(maxLength, curPos + offset);
		break;
	case SeekPos::End:
		curPos = std::min(maxLength, length - offset);
		break;
	}

	if(curPos >= length) length = curPos;
}

size_t FixedDataMessageStream::GetLength() const noexcept
{
	return std::min(length, maxLength);
}

size_t FixedDataMessageStream::GetPosition() const noexcept
{
	return curPos;
}

DynamicDataMessageStream::DynamicDataMessageStream() noexcept
	: storage(nullptr)
	, length(0)
	, maxLength(0)
	, curPos(0)
{
}

DynamicDataMessageStream::~DynamicDataMessageStream() noexcept
{
	if (storage) delete[] storage;
}

DynamicDataMessageStream::DynamicDataMessageStream(DynamicDataMessageStream&& other) noexcept
	: storage(other.storage)
	, length(other.length)
	, maxLength(other.maxLength)
	, curPos(other.curPos)
{
	other.storage = nullptr;
	other.length = other.maxLength = other.curPos = 0;
}

DynamicDataMessageStream& DynamicDataMessageStream::operator=(DynamicDataMessageStream&& other) noexcept
{
	storage = other.storage;
	length = other.length;
	maxLength = other.maxLength;
	curPos = other.curPos;
	other.storage = nullptr;
	other.length = other.maxLength = other.curPos = 0;

	return *this;
}

void DynamicDataMessageStream::Read(void* data, size_t readLen)
{
	if (curPos + readLen - 1 > length) {
		throw StreamOverflowException(StreamOverflowException::Reading);
	}

	std::memcpy(data, storage + curPos, readLen);
	curPos += readLen;
}

void DynamicDataMessageStream::Write(const void* data, size_t writeLen)
{
	if (curPos + writeLen > maxLength)
	{
		// allocate a new buffer
		allocate(writeLen);
	}

	std::memcpy(storage + curPos, data, writeLen);
	curPos += writeLen;
	if(curPos > length) length = curPos;
}

void DynamicDataMessageStream::allocate(size_t writeLength)
{
	return reallocate((maxLength + writeLength) * 2);
}

void DynamicDataMessageStream::reallocate(size_t newLen)
{
	if (storage)
	{
		// allocate a new buffer
		uint8_t* newStorage = new uint8_t[newLen];
		// and copy the data of the current buffer into it
		memcpy(newStorage, storage, std::min(newLen, maxLength));

		// delete the previous buffer
		delete[] storage;

		storage = newStorage;
	} else {
		storage = new uint8_t[newLen];
	}

	maxLength = newLen;
}

void DynamicDataMessageStream::reserve(size_t size)
{
	if(size > maxLength)
	{
		// reallocate the size of the buffer
		reallocate(size);
	}
}

void DynamicDataMessageStream::Seek(size_t offset, SeekPos from) noexcept
{
	switch (from)
	{
	default:
	case SeekPos::Begin:
		curPos = offset;
		break;
	case SeekPos::Current:
		curPos = curPos + offset;
		break;
	case SeekPos::End:
		curPos = length - offset;
		break;
	}

	if (curPos > length)
	{
		length = curPos;
		if (length > maxLength)
		{
			// seeking out of the buffer, allocate a new one
			allocate(length - maxLength);
		}
	}
}

void DynamicDataMessageStream::clear(bool freeMemory)
{
	if (storage && freeMemory)
	{
		delete[] storage;
		storage = nullptr;
		maxLength = 0;
	}

	length = 0;
	curPos = 0;
}

size_t DynamicDataMessageStream::GetPosition() const noexcept
{
	return curPos;
}

size_t DynamicDataMessageStream::GetLength() const noexcept
{
	return length;
}

uint8_t* DynamicDataMessageStream::getStorage()
{
	return storage;
}

const uint8_t* DynamicDataMessageStream::getStorage() const
{
	return storage;
}
