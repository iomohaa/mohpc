#pragma once

#include <stdint.h>
#include <stddef.h>
#include "HuffmanTree.h"
#include <exception>
#include "../../Script/str.h"

namespace MOHPC
{
	class IMessageCodec;
	class IMessageStream;
	class ISerializableMessage;

	class MessageException : public std::exception
	{
	};

	/**
	 * Used to send string over a message
	 */
	class MOHPC_EXPORTS StringMessage
	{
	private:
		char* strData;
		bool isAlloced;

	public:
		StringMessage() noexcept;
		StringMessage(const char* str) noexcept;
		StringMessage(const str& str) noexcept;
		~StringMessage() noexcept;

		StringMessage(StringMessage&& str) noexcept;
		StringMessage& operator=(StringMessage&& str) noexcept;

		StringMessage(const StringMessage&) = delete;
		StringMessage& operator=(const StringMessage&) = delete;

		char* getData() noexcept;
		char* getData() const noexcept;
		void preAlloc(size_t len) noexcept;
		void writeChar(char c, size_t i) noexcept;
		operator const char* () const noexcept;
	};

	/**
	 * Allow compressing message using the huffman algorithm
	 */
	class MOHPC_EXPORTS CompressedMessage
	{
	private:
		IMessageStream& inputStream;
		IMessageStream& outputStream;

	public:
		CompressedMessage(IMessageStream& inStream, IMessageStream& outStream)
			: inputStream(inStream)
			, outputStream(outStream)
		{
		}

		void Compress(size_t offset, size_t len) noexcept;
		void Decompress(size_t offset, size_t len) noexcept;

	protected:
		IMessageStream& input() const { return inputStream; };
		IMessageStream& output() const { return outputStream; };
	};

	enum class msgMode_e : uint8_t
	{
		Reading,
		Writing,
		Both
	};

	/**
	 * Base interface for sending data over any type of support.
	 *
	 * Unlike STL containers, there is no operator<< function, to have more attention
	 * of what type of data to serialize.
	 */
	class MOHPC_EXPORTS MSG
	{
	private:
		IMessageStream& msgStream;
		IMessageCodec* msgCodec;
		msgMode_e mode;
		size_t bit;
		uint8_t bitData[32];

	public:
		MSG(IMessageStream& stream, msgMode_e mode) noexcept;
		~MSG();

		/** Change the codec of the message. See msg_codec.h for codecs. */
		void SetCodec(IMessageCodec& codec) noexcept;

		void SetMode(msgMode_e mode);

		/** Reset the message and start with the stream's current position. */
		void Reset();

		/** Flush any pending data to the stream. */
		void Flush();

		/** Serialize specified bits. */
		void SerializeBits(void* value, intptr_t bits);

		/** Serialize an array of bytes. */
		MSG& Serialize(void* data, size_t length) noexcept;

		/** Serialize b if b is different than a. */
		MSG& SerializeDelta(const void* a, void* b, size_t bits);

		/** Serialize b if b is different than a, with a key so output will be XORed. */
		MSG& SerializeDelta(const void* a, void* b, intptr_t key, size_t bits);

		/** Serialize b if b is different than a. */
		MSG& SerializeDeltaClass(const ISerializableMessage* a, ISerializableMessage* b);

		/** Serialize b if b is different than a, with a key so output will be XORed. */
		MSG& SerializeDeltaClass(const ISerializableMessage* a, ISerializableMessage* b, intptr_t key);

		template<typename T>
		MSG& SerializeDeltaType(const T& a, T& b) noexcept
		{
			static_assert(std::is_arithmetic<T>::value, "Type must be an arithmetic type");

			bool isSame = a != b;
			SerializeBits(&isSame, 1);
			if (!isSame) SerializeBits(&b, sizeof(T) << 3);
			return *this;
		}

		template<typename T>
		MSG& SerializeDeltaType(const T& a, T& b, intptr_t key) noexcept
		{
			static_assert(std::is_arithmetic<T>::value, "Type must be an arithmetic type");

			if (!IsReading())
			{
				bool isDiff = a != b;
				SerializeBits(&isDiff, 1);

				if (isDiff)
				{
					T val = XORType(b, key);
					SerializeBits(&val, sizeof(T) << 3);
				}
			}
			else
			{
				bool isDiff;
				SerializeBits(&isDiff, 1);

				if (isDiff)
				{
					SerializeBits(&b, sizeof(T) << 3);
					b = XORType(b, key);
				}
			}
			return *this;
		}

		template<>
		MSG& SerializeDeltaType<bool>(const bool& a, bool& b, intptr_t key) noexcept;

		/** Serialize a boolean value with 1 bit. */
		MSG& SerializeBool(bool& value) noexcept;

		/** Serialize a boolean value, byte-sized. */
		MSG& SerializeByteBool(bool& value) noexcept;

		/** Serialize a char value. */
		MSG& SerializeChar(char& value) noexcept;

		/** Serialize a byte value. */
		MSG& SerializeByte(unsigned char& value) noexcept;

		/** Serialize a short value. */
		MSG& SerializeShort(short& value) noexcept;

		/** Serialize an unsigned short value. */
		MSG& SerializeUShort(unsigned short& value) noexcept;

		/** Serialize an integer value. */
		MSG& SerializeInteger(int& value) noexcept;

		/** Serialize an unsigned integer value. */
		MSG& SerializeUInteger(unsigned int& value) noexcept;

		/** Serialize a float value. */
		MSG& SerializeFloat(float& value) noexcept;

		/** Serialize a class that supports message serialization. */
		MSG& SerializeClass(ISerializableMessage* value) noexcept;
		MSG& SerializeClass(ISerializableMessage& value) noexcept;

		/** Serialize a string value. */
		void SerializeString(StringMessage& s);

		/** Serialize a string value. */
		void SerializeStringWithMapping(StringMessage& s);

		/** Return the size of this message. */
		size_t Size() const;

		/** Return the actual position of the message. */
		size_t GetPosition() const;

		/** Return the current bit position of the message. */
		size_t GetBitPosition() const;

		/** Return whether or not this message is for reading. */
		bool IsReading() noexcept;

		/** Return whether or not this message is for writing. */
		bool IsWriting() noexcept;
		
		/** Read data from message. */
		void ReadData(void* data, size_t length) noexcept;

		/** Read specified bits. */
		void ReadBits(void* value, intptr_t bits);

		/** Read a boolean value with 1 bit. */
		bool ReadBool() noexcept;

		/** Read a boolean value, byte-sized. */
		bool ReadByteBool() noexcept;

		/** Read a char value. */
		char ReadChar() noexcept;

		/** Read a byte value. */
		unsigned char ReadByte() noexcept;

		/** Read a short value. */
		short ReadShort() noexcept;

		/** Read an unsigned short value. */
		unsigned short ReadUShort() noexcept;

		/** Read an integer value. */
		int ReadInteger() noexcept;

		/** Read an unsigned integer value. */
		unsigned int ReadUInteger() noexcept;

		/** Read a float value. */
		float ReadFloat() noexcept;

		/** Read a string value. */
		StringMessage ReadString();

		/** Read a string value. */
		StringMessage ReadScrambledString(const char* byteCharMapping);

		template<typename T>
		T ReadDeltaType(const T& a) noexcept
		{
			static_assert(std::is_arithmetic<T>::value, "Type must be an arithmetic type");

			const bool isDiff = ReadBool();
			if (isDiff)
			{
				T val;
				ReadBits(&val, sizeof(T) << 3);
				return val;
			}

			return a;
		}

		template<typename T>
		T ReadDeltaTypeKey(const T& a, intptr_t key) noexcept
		{
			static_assert(std::is_arithmetic<T>::value, "Type must be an arithmetic type");

			const bool isDiff = ReadBool();
			if (isDiff)
			{
				T val;
				ReadBits(&val, sizeof(T) << 3);
				XORType(val, key);
				return val;
			}
			return a;
		}

		/** Serialize b if b is different than a. */
		MSG& ReadDeltaClass(const ISerializableMessage* a, ISerializableMessage* b) noexcept;

		/** Serialize b if b is different than a, with a key so output will be XORed. */
		MSG& ReadDeltaClass(const ISerializableMessage* a, ISerializableMessage* b, intptr_t key) noexcept;

		/** Write data to message. */
		MSG& WriteData(const void* data, uintptr_t size);

		/** Write specified bits. */
		MSG& WriteBits(const void* value, intptr_t bits);

		/** Write a boolean value with 1 bit. */
		MSG& WriteBool(bool value) noexcept;

		/** Write a boolean value, byte-sized. */
		MSG& WriteByteBool(bool value) noexcept;

		/** Write a char value. */
		MSG& WriteChar(char value) noexcept;

		/** Write a byte value. */
		MSG& WriteByte(unsigned char value) noexcept;

		/** Write a short value. */
		MSG& WriteShort(short value) noexcept;

		/** Write an unsigned short value. */
		MSG& WriteUShort(unsigned short value) noexcept;

		/** Write an integer value. */
		MSG& WriteInteger(int value) noexcept;

		/** Write an unsigned integer value. */
		MSG& WriteUInteger(unsigned int value) noexcept;

		/** Write a float value. */
		MSG& WriteFloat(float value) noexcept;

		/** Write a string value. */
		MSG& WriteString(const StringMessage& s);

		/** Write a string value. */
		MSG& WriteScrambledString(const StringMessage& s, const uint8_t* charByteMapping);

		template<typename T>
		MSG& WriteDeltaType(const T& a, T& b) noexcept
		{
			static_assert(std::is_arithmetic<T>::value, "Type must be an arithmetic type");

			bool isSame = a != b;
			SerializeBits(&isSame, 1);
			if (!isSame) SerializeBits(&b, sizeof(T) << 3);
			return *this;
		}

		template<typename T>
		MSG& WriteDeltaTypeKey(const T& a, T& b, intptr_t key) noexcept
		{
			static_assert(std::is_arithmetic<T>::value, "Type must be an arithmetic type");

			bool isDiff;
			SerializeBits(&isDiff, 1);

			if (isDiff)
			{
				SerializeBits(&b, sizeof(T) << 3);
				b = XORType(b, key);
			}
			return *this;
		}


		/** Serialize b if b is different than a. */
		MSG& WriteDeltaClass(const ISerializableMessage* a, ISerializableMessage* b) noexcept;

		/** Serialize b if b is different than a, with a key so output will be XORed. */
		MSG& WriteDeltaClass(const ISerializableMessage* a, ISerializableMessage* b, intptr_t key) noexcept;

		/** Return the stream associated to this message. */
		IMessageStream& stream() const noexcept
		{
			return msgStream;
		};

		/** Return the codec associated to this message. */
		IMessageCodec& codec() const noexcept
		{
			return *msgCodec;
		};

	private:
		template<typename T>
		T XORType(const T& b, intptr_t key)
		{
			return T(b ^ key);
		}

		// Special code for float
		template<>
		float XORType(const float& b, intptr_t key);

		// Special code for double
		template<>
		double XORType(const double& b, intptr_t key);

		StringMessage ReadStringInternal(const char* byteCharMapping);
public:
	};

	class MOHPC_EXPORTS MsgBaseHelper
	{
	protected:
		MSG& msg;

	public:
		MsgBaseHelper(MSG& inMsg)
			: msg(inMsg)
		{}

		virtual ~MsgBaseHelper() = default;
	};

	class MOHPC_EXPORTS MsgTypesHelper : public MsgBaseHelper
	{
	public:
		MsgTypesHelper(MSG& inMsg)
			: MsgBaseHelper(inMsg)
		{}

		/** Read a coordinate value. */
		float ReadCoord() noexcept;

		/** Read a coordinate value. */
		float ReadCoordSmall() noexcept;

		/** Read a coordinate value. */
		int32_t ReadDeltaCoord(int32_t offset) noexcept;

		/** Read a coordinate value. */
		int32_t ReadDeltaCoordExtra(int32_t offset) noexcept;

		/** Read a coordinate value. */
		Vector ReadVectorCoord() noexcept;

		/** Read a coordinate value. */
		Vector ReadDir() noexcept;

		/** Read an entity number. */
		uint16_t ReadEntityNum();

		/** Read an entity number (protocol >= 15). */
		uint16_t ReadEntityNum2();

		/** Write a coordinate value. */
		void WriteCoord(float& value) noexcept;

		/** Write a coordinate value. */
		void WriteCoordSmall(float& value) noexcept;

		/** Write a coordinate value. */
		void WriteVectorCoord(Vector& value) noexcept;

		/** Write a coordinate value. */
		void WriteDir(Vector& dir) noexcept;
	};
};

