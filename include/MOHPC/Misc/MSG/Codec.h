#pragma once

namespace MOHPC
{

/**
 * Base interface for encoding/decoding bits
 */
class MOHPC_EXPORTS IMessageCodec
{
public:
	virtual ~IMessageCodec() {}

	/** Encode the specified data into a buffer, flushing if necessary into the stream. */
	virtual void Encode(const void* data, size_t bits, size_t& bit, IMessageStream& stream, uint8_t* bitBuffer, size_t bufsize) = 0;

	/** Decode the specified data into a buffer, reading any data from the stream if necessary. */
	virtual void Decode(void* data, size_t bits, size_t& bit, IMessageStream& stream, uint8_t* bitBuffer, size_t bufsize) = 0;
};

namespace MessageCodecs
{
	void FlushBits(size_t& bit, IMessageStream& stream, uint8_t* bitData, size_t bitDataSize);
	void ReadBits(size_t& bit, IMessageStream& stream, uint8_t* bitData, size_t bitDataSize);

	extern MOHPC_EXPORTS IMessageCodec& Bit;
	extern MOHPC_EXPORTS IMessageCodec& OOB;
}


class CodecException : public std::exception {};

/**
 * Bad number of bits. For OOB, bits must be 8, 16, 32 or 64.
 */
class BadBitsException : public CodecException
{
public:
	BadBitsException(size_t inBits);

	size_t getBits() const;

private:
	size_t bits;
};

}
