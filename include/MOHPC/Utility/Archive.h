#pragma once

#include "../Utility/Misc/Endian.h"

#include <cstdint>

namespace MOHPC
{
	class IArchive
	{
	public:
		virtual void serialize(void* value, size_t size) = 0;
		virtual bool isReading() const = 0;

	public:
		IArchive& operator>>(char& value)
		{
			serialize(&value, sizeof(value));
			return *this;
		}

		IArchive& operator>>(uint8_t& value)
		{
			serialize(&value, sizeof(value));
			return *this;
		}

		IArchive& operator>>(bool& value)
		{
			serialize(&value, sizeof(value));
			return *this;
		}

		IArchive& operator>>(int16_t& value)
		{
			serialize(&value, sizeof(value));
			value = Endian.LittleShort(value);
			return *this;
		}

		IArchive& operator>>(uint16_t& value)
		{
			serialize(&value, sizeof(value));
			value = Endian.LittleShort(value);
			return *this;
		}

		IArchive& operator>>(int32_t& value)
		{
			serialize(&value, sizeof(value));
			value = Endian.LittleInteger(value);
			return *this;
		}

		IArchive& operator>>(uint32_t& value)
		{
			serialize(&value, sizeof(value));
			value = Endian.LittleInteger(value);
			return *this;
		}

		IArchive& operator>>(int64_t& value)
		{
			serialize(&value, sizeof(value));
			value = Endian.LittleLong64(value);
			return *this;
		}

		IArchive& operator>>(uint64_t& value)
		{
			serialize(&value, sizeof(value));
			value = Endian.LittleLong64(value);
			return *this;
		}

		IArchive& operator>>(float& value)
		{
			serialize(&value, sizeof(value));
			value = Endian.LittleFloat(value);
			return *this;
		}


		IArchive& operator<<(char value)
		{
			serialize(&value, sizeof(value));
			return *this;
		}

		IArchive& operator<<(uint8_t value)
		{
			serialize(&value, sizeof(value));
			return *this;
		}

		IArchive& operator<<(bool value)
		{
			serialize((void*)&value, sizeof(value));
			return *this;
		}

		IArchive& operator<<(int16_t value)
		{
			value = Endian.LittleShort(value);
			serialize((void*)&value, sizeof(value));
			return *this;
		}

		IArchive& operator<<(uint16_t value)
		{
			value = Endian.LittleShort(value);
			serialize((void*)&value, sizeof(value));
			return *this;
		}

		IArchive& operator<<(int32_t value)
		{
			value = Endian.LittleInteger(value);
			serialize((void*)&value, sizeof(value));
			return *this;
		}

		IArchive& operator<<(uint32_t value)
		{
			value = Endian.LittleInteger(value);
			serialize((void*)&value, sizeof(value));
			return *this;
		}

		IArchive& operator<<(int64_t value)
		{
			value = Endian.LittleLong64(value);
			serialize((void*)&value, sizeof(value));
			return *this;
		}

		IArchive& operator<<(uint64_t value)
		{
			value = Endian.LittleLong64(value);
			serialize((void*)&value, sizeof(value));
			return *this;
		}

		IArchive& operator<<(float value)
		{
			value = Endian.LittleFloat(value);
			serialize((void*)&value, sizeof(value));
			return *this;
		}

		template<typename T>
		void operator()(T& value)
		{
			if (isReading()) {
				*this >> value;
			}
			else {
				*this << value;
			}
		}
	};

	class IArchiveReader : public IArchive
	{
	public:
		virtual bool isReading() const override final
		{
			return true;
		}
	};

	class IArchiveWriter : public IArchive
	{
		virtual bool isReading() const override final
		{
			return false;
		}
	};
}
