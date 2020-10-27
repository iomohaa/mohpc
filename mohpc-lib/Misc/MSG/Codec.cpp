#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Misc/MSG/HuffmanTree.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include "CodecCompression.h"
#include <stdint.h>
#include <algorithm>
#include <cstring>
#include <cassert>

using namespace MOHPC;

class COOBCodec : public IMessageCodec
{
public:
	virtual void Encode(const void* data, size_t bits, size_t& bit, IMessageStream& stream, uint8_t* bitBuffer, size_t bufsize) override
	{
		if (bits == 8) {
			stream.Write(data, 1);
		}
		else if (bits == 16) {
			stream.Write(data, 2);
		}
		else if (bits == 32) {
			stream.Write(data, 4);
		}
		else {
			throw 0;
			return;
		}
	}

	virtual void Decode(void* data, size_t bits, size_t& bit, IMessageStream& stream, uint8_t* bitBuffer, size_t bufsize) override
	{
		const size_t n = bit >> 3;
		if (bits == 8) {
			memcpy(data, bitBuffer + n, 1);
		}
		else if (bits == 16) {
			memcpy(data, bitBuffer + n, 2);
		}
		else if (bits == 32) {
			memcpy(data, bitBuffer + n, 4);
		}
		else
		{
			throw 0;
		}

		bit += bits;
		MessageCodecs::ReadBits(bit, stream, bitBuffer, bufsize);
	}
};

class CBitCodec : public IMessageCodec
{
private:
	//Huff compressor, decompressor;

public:
	CBitCodec()
	{
		/*
		for (int i = 0; i < 256; i++)
		{
			for (int j = 0; j < msg_hData[i]; j++)
			{
				// Do update for each
				compressor.addRef((uint8_t)i);
				decompressor.addRef((uint8_t)i);
			}
		}
		*/


	}

	/*
	void test()
	{
		uint8_t bitBuffer[8]{ 0 };

		for (size_t i = 0; i < 256; i += 2)
		{
			uint8_t value = (uint8_t)i;
			size_t bit = 0;
			MessageCodecs::Compression::huff.offsetTransmit(value, bitBuffer, &bit);
			printf("char %d wrote %d bits >", value, bit);
			++value;
			size_t bit2 = 0;
			size_t startBit = bit;
			MessageCodecs::Compression::huff.offsetTransmit(value, bitBuffer, &bit);
			MessageCodecs::Compression::huff.offsetTransmit(value, bitBuffer, &bit2);
			printf("%d wrote %d bits (%d total bit, %d diff)\n", value, bit2, bit, bit - startBit - bit2);
		}
	}
	*/

	virtual void Encode(const void* data, size_t bits, size_t& bit, IMessageStream& stream, uint8_t* bitBuffer, size_t bufsize) override
	{
		const uint8_t* buf = (const uint8_t*)data;
		size_t remainingBits = bits;
		size_t bitNum = 0;

		// Write partial bytes
		if (remainingBits & 7)
		{
			size_t nbits = bits & 7;

			uint8_t bitVal = buf[0];

			// Write each bit into a single byte
			for (size_t i = 0; i < nbits; i++)
			{
				Huff::addBit((bitVal & 1), bitBuffer, bit);
				bitVal >>= 1;
			}

			remainingBits -= nbits;
			bitNum = nbits;
			MessageCodecs::FlushBits(bit, stream, bitBuffer, bufsize);
		}

		// Write remaining bytes
		if (remainingBits)
		{
			if (bitNum & 7)
			{
				// Write partial bytes
				for (size_t i = 0; bitNum < remainingBits; i += 8, bitNum += 8)
				{
					const size_t index = bitNum >> 3;
					const size_t subBitNum = bitNum - i;
					//const uint8_t* p1 = buf + index;
					//const uint8_t* p2 = buf + index + 1;
					const uint16_t* p = (uint16_t*)(buf + index);
					//assert(p2 <= buf + (bits >> 3));
					//const uint8_t value = (*p1 >> subBitNum) | (*p2 << (8 - subBitNum));
					const uint8_t value = (*p >> subBitNum) | ((*p >> 8) << (8 - subBitNum));
					MessageCodecs::Compression::huff.offsetTransmit(value, bitBuffer, &bit);
					MessageCodecs::FlushBits(bit, stream, bitBuffer, bufsize);
				}
			}
			else
			{
				// Write full bytes
				const uint8_t* p = buf + (bitNum >> 3);
				for (size_t i = 0; i < remainingBits; i += 8, ++p)
				{
					const uint8_t value = *p;
					MessageCodecs::Compression::huff.offsetTransmit(value, bitBuffer, &bit);
					MessageCodecs::FlushBits(bit, stream, bitBuffer, bufsize);
				}
			}
		}
	}

	virtual void Decode(void* data, size_t bits, size_t& bit, IMessageStream& stream, uint8_t* bitBuffer, size_t bufsize) override
	{
		uint8_t* buf = (uint8_t*)data;
		size_t remainingBits = bits;
		size_t bitNum = 0;

		// Read partial bytes
		if (remainingBits & 7)
		{
			MessageCodecs::ReadBits(bit, stream, bitBuffer, bufsize);

			// Clear the buffer to prevent any undefined behavior
			uint8_t bitVal = 0;

			const size_t nbits = bits & 7;
			for (size_t i = 0; i < nbits; i++) {
				bitVal |= Huff::getBit(bitBuffer, bit) << i;
			}

			remainingBits -= nbits;
			bitNum = nbits;
			buf[0] = bitVal;
		}

		// Read remaining bytes
		if (remainingBits)
		{
			if (bitNum & 7)
			{
				// Read partial bytes
				for (size_t i = 0; i < remainingBits; i += 8, bitNum += 8)
				{
					const size_t index = bitNum >> 3;
					const size_t subBitNum = bitNum - i;
					//uint8_t* p1 = buf + index;
					//uint8_t* p2 = buf + index + 1;
					uint16_t* p = (uint16_t*)(buf + index);
					//assert(p2 <= buf + (bits >> 3));

					MessageCodecs::ReadBits(bit, stream, bitBuffer, bufsize);
					const uint8_t received = (uint8_t)MessageCodecs::Decompression::huff.offsetReceive(bitBuffer, &bit);

					//*p1 |= received << subBitNum;
					//*p2 = received >> (8 - subBitNum);
					*p |= (received << subBitNum) | ((received << 8) >> (8 - subBitNum));
				}
			}
			else
			{
				// Read a complete byte
				uint8_t* p = buf + (bitNum >> 3);
				for (size_t i = 0; i < remainingBits; i += 8, ++p)
				{
					MessageCodecs::ReadBits(bit, stream, bitBuffer, bufsize);
					*p = (uint8_t)MessageCodecs::Decompression::huff.offsetReceive(bitBuffer, &bit);
				}
			}
		}
	}
};

namespace MOHPC
{
	namespace MessageCodecs
	{
		void FlushBits(size_t& bit, IMessageStream& stream, uint8_t* bitData, size_t bitDataSize)
		{
			const size_t n = bit >> 3;
			if (n > (bitDataSize >> 1))
			{
				stream.Write(bitData, n);

				const size_t dataSize = bitDataSize;
				const size_t lastNum = dataSize - n;

				std::memmove(bitData, bitData + n, lastNum);
				std::memset(bitData + lastNum, 0, bitDataSize - lastNum);

				bit &= 7;
			}
		}

		void ReadBits(size_t& bit, IMessageStream& stream, uint8_t* bitData, size_t bitDataSize)
		{
			// Prepare the next data to be read
			const size_t n = bit >> 3;
			if (n > (bitDataSize >> 1))
			{
				const size_t dataSize = bitDataSize;
				const size_t lastNum = dataSize - n;

				std::memmove(bitData, bitData + n, lastNum);

				const size_t remainingRead = std::min((size_t)n, stream.GetLength() - stream.GetPosition());
				if (remainingRead) {
					stream.Read(&bitData[lastNum], remainingRead);
				}
				else
				{
					if (std::all_of(bitData, bitData + bitDataSize, [](uint8_t byte) { return !byte; }))
					{
						// the stream and the buffer has been fully read already, don't continue
						throw StreamOverflowException(StreamOverflowException::Reading);
					}
				}

				// clear out the remaining bytes if there is nothing to read
				std::memset(bitData + lastNum + remainingRead, 0, bitDataSize - lastNum - remainingRead);

				bit &= 7;
			}
		}

		CBitCodec InternalBitCodec;
		COOBCodec InternalOOBCodec;

		IMessageCodec& Bit = InternalBitCodec;
		IMessageCodec& OOB = InternalOOBCodec;
	}
}
