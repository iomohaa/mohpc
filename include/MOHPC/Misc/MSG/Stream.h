#pragma once

#include "../../Global.h"
#include <stdint.h>
#include <stddef.h>
#include <exception>

namespace MOHPC
{
	class StreamMessageException : public std::exception
	{
	};

	class StreamOverflowException : public StreamMessageException
	{
	public:
		enum Mode
		{
			Reading,
			Writing
		};

	private:
		Mode mode;

	public:
		StreamOverflowException(Mode mode) noexcept;

		Mode getMode() const noexcept;
	};

	class MOHPC_EXPORTS IMessageStream
	{
	public:
		enum class SeekPos : uint8_t
		{
			Begin,
			Current,
			End
		};

		virtual ~IMessageStream() {}

		/** Read data from the stream. */
		virtual void Read(void* data, size_t length) = 0;

		/** Write data to the stream. */
		virtual void Write(const void* data, size_t length) = 0;

		/** Set the stream position. */
		virtual void Seek(size_t offset, SeekPos from = SeekPos::Begin) = 0;

		/** Return the current position of the stream. */
		virtual size_t GetPosition() const = 0;

		/** Return the length of the stream. */
		virtual size_t GetLength() const = 0;
	};

	class MOHPC_EXPORTS FixedDataMessageStream : public IMessageStream
	{
	private:
		uint8_t* storage;
		size_t length;
		size_t maxLength;
		size_t curPos;

	public:
		FixedDataMessageStream(void* data, size_t size, size_t readableLength = -1) noexcept;

		virtual void Read(void* data, size_t length) override;
		virtual void Write(const void* data, size_t length) override;
		virtual void Seek(size_t offset, SeekPos from) noexcept override;
		virtual size_t GetPosition() const noexcept override;
		virtual size_t GetLength() const noexcept override;
	};

	class MOHPC_EXPORTS DynamicDataMessageStream : public IMessageStream
	{
	private:
		uint8_t* storage;
		size_t length;
		size_t maxLength;
		size_t curPos;

	public:
		DynamicDataMessageStream() noexcept;
		~DynamicDataMessageStream() noexcept;

		virtual void Read(void* data, size_t length) override;
		virtual void Write(const void* data, size_t length) override;
		virtual void Seek(size_t offset, SeekPos from) noexcept override;
		virtual size_t GetPosition() const noexcept override;
		virtual size_t GetLength() const noexcept override;
		void reserve(size_t size);
		const uint8_t* getStorage() const;

	private:
		void reallocate(size_t newLen);
		void allocate(size_t writeLength);
	};
}
