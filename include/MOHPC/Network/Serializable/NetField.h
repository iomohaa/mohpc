#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace MOHPC
{
namespace Network
{
	/**
	 * Field type for protocol version 8.
	 */
	enum class fieldType_ver6_e
	{
		regular,
		angle,
		animTime,
		animWeight,
		scale,
		alpha,
		coord,
		velocity,
		/**
		 * Simple number type field that use field->bits.
		 * This field is only present and used in Mac builds of mohaa.
		 */
		simple
	};

	/**
	 * Field type for protocol version 15.
	 * Introduced coordExtra, for more precise coordinates.
	 */
	enum class fieldType_ver15_e
	{
		regular,
		angle,
		animTime,
		animWeight,
		scale,
		alpha,
		coord,
		coordExtra,
		velocity,
		/**
		 * Simple number type field that use field->bits.
		 * This field is only present and used in Mac builds of mohaa.
		 */
		simple
	};

	/**
	 * This class represent a field for a specific struct.
	 */
	struct netField_t
	{
	public:
		/**
		 * Initialize the net field.
		 *
		 * @param nameVal	The field name to use.
		 * @param offsetVal	The offset of the member in the struct.
		 * @param sizeVal	The size of the struct member.
		 * @param bitsVal	The number of bits to transmit.
		 *  > 0 = normal number.
		 *	  0	= float.
		 *  < 0	= use the sign bit.
		 */
		constexpr netField_t(const char* nameVal, uint16_t offsetVal, uint8_t sizeVal, int8_t bitsVal, uint8_t typeVal)
			: name(nameVal)
			, offset(offsetVal)
			, size(sizeVal)
			, bits(bitsVal)
			, type(typeVal)
		{
			assert(size_t((bits < 0 ? ~bits : bits) >> 3) <= size);
			assert(bits || size == sizeof(float));
			assert(type < 6 || size >= 3);
			assert(type != 1 || size == sizeof(float));
		}

	public:
		/** Return the name of the field. */
		const char* getName() const;

		/** Return the offset that the field is in. */
		uint16_t getOffset() const;

		/** Return the field size. */
		uint8_t getSize() const;

		/** Return the number of bits that this field take in transmission. */
		int8_t getBits() const;

		/** Return the field type (version specific). */
		uint8_t getType() const;

	private:
		const char* name;
		uint16_t offset;
		uint8_t size;
		int8_t bits;
		// see fieldType_e
		uint8_t type : 4;

	};

	/**
	 * Template for specific field type.
	 */
	template<typename fieldType>
	struct netField_template_t : public netField_t
	{
		using fieldType_e = fieldType_ver6_e;

		constexpr netField_template_t(const char* nameVal, uint16_t offsetVal, uint8_t sizeVal, int8_t bitsVal, fieldType typeVal)
			: netField_t(nameVal, offsetVal, sizeVal, bitsVal, (uint8_t)typeVal)
		{}
	};

	template<typename T>
	void CopyFields(const T* other, T* target, size_t from, size_t to, const netField_t* fieldlist)
	{
		size_t i;
		const netField_t* field;
		for (i = from, field = &fieldlist[from]; i < to; i++, field++)
		{
			const uint8_t* fromF = (const uint8_t*)((const uint8_t*)other + field->getOffset());
			uint8_t* toF = (uint8_t*)((uint8_t*)target + field->getOffset());

			// no change
			std::memcpy(toF, fromF, field->getSize());
		}
	}
}
}