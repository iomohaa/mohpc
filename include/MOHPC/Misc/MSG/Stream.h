#pragma once

#include "../../Global.h"
#include <cstdint>
#include <cstddef>
#include <algorithm>
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

	namespace StreamHelpers
	{
		/**
		 * Copy content from input stream to output stream.
		 *
		 * @tparam BLOCK_SIZE The size of each block to read at once.
		 * @param in The input stream to copy from.
		 * @param out The output stream to copy to.
		 * @param len Length to copy.
		 */
		template<size_t BLOCK_SIZE>
		void Copy(IMessageStream& out, IMessageStream& in, size_t len)
		{
			for (size_t block = 0; block < len; block += BLOCK_SIZE)
			{
				uint8_t buf[BLOCK_SIZE];

				const size_t remaining = std::min(len - in.GetPosition(), BLOCK_SIZE);

				// read the data and save it
				in.Read(buf, remaining);
				out.Write(buf, remaining);
			}
		}
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
		virtual void Seek(size_t offset, SeekPos from = IMessageStream::SeekPos::Begin) noexcept override;
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

		DynamicDataMessageStream(DynamicDataMessageStream&& other) noexcept;
		DynamicDataMessageStream& operator=(DynamicDataMessageStream&& other) noexcept;

		virtual void Read(void* data, size_t length) override;
		virtual void Write(const void* data, size_t length) override;
		virtual void Seek(size_t offset, SeekPos from = IMessageStream::SeekPos::Begin) noexcept override;
		virtual size_t GetPosition() const noexcept override;
		virtual size_t GetLength() const noexcept override;

		void clear(bool freeMemory = true);
		void reserve(size_t size);
		uint8_t* getStorage();
		const uint8_t* getStorage() const;

	private:
		void reallocate(size_t newLen);
		void allocate(size_t writeLength);
	};
}
