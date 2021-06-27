#pragma once

#include <stdint.h>
#include <exception>
#include "../Utility/Misc/MSG/Serializable.h"
#include "Types.h"

#include <type_traits>

namespace MOHPC
{
	class usercmd_t;
	class usereyes_t;
	class playerState_t;
	class entityState_t;

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
		 * This field is only present and used in Mac builds in moh.
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
		 * This field is only present and used in Mac builds in moh.
		 */
		simple
	};

	static constexpr uint32_t MAX_COORDS = 1 << 19;

	struct netField_t
	{
		const char* name;
		uint16_t offset;
		uint8_t size;
		int8_t bits;
		// see fieldType_e
		uint8_t type : 4;

		constexpr netField_t(const char* inName, uint16_t inOffset, uint8_t inSize, int8_t inBits, uint8_t inType)
			: name(inName)
			, offset(inOffset)
			, size(inSize)
			, bits(inBits)
			, type(inType)
		{
			assert(size_t((bits < 0 ? ~bits : bits) >> 3) <= size);
			assert(bits || size == sizeof(float));
			assert(type < 6 || size >= 3);
			assert(type != 1 || size == sizeof(float));
		}
	};

	template<typename fieldType>
	struct netField_template_t : public netField_t
	{
		using fieldType_e = fieldType_ver6_e;

		constexpr netField_template_t(const char* inName, uint16_t inOffset, uint8_t inSize, int8_t inBits, fieldType inType)
			: netField_t(inName, inOffset, inSize, inBits, (uint8_t)inType)
		{}
	};

	template<size_t maxBits, intptr_t minCoord, intptr_t maxCoord>
	class NetCoord
	{
		static_assert(maxBits < 64, "Bits must be under 64");
	public:
		using packedint_t = std::conditional_t<maxBits <= 8, uint8_t,
			std::conditional_t<maxBits <= 16, uint16_t,
			std::conditional_t<maxBits <= 32, uint32_t,
			std::conditional_t<maxBits <= 64, uint64_t, uint64_t>>>>;

	public:
		packedint_t pack(float val)
		{
			return (packedint_t)roundf(maxBitCoordSigned + val * precision);
		}

		float unpack(packedint_t packedValue)
		{
			return (float)(int64_t)(packedValue - maxBitCoordSigned) / precision;
		}

	public:
		static constexpr uint64_t maxBitCoord = (1ull << maxBits);
		static constexpr uint64_t maxBitCoordSigned = maxBitCoord / 2;
		static constexpr uint64_t coordMask = maxBitCoord - 1;
		static constexpr uint64_t maxDelta = (maxCoord - minCoord);
		static constexpr uint64_t precision = maxBitCoord / maxDelta;
	};

	namespace EntityField
	{
		void ReadNumberPlayerStateField(MSG& msg, intptr_t bits, void* toF, size_t size);
		void WriteNumberPlayerStateField(MSG& msg, intptr_t bits, const void* toF, size_t size);

		void ReadRegular(MSG& msg, intptr_t bits, void* toF, size_t size);
		void WriteNumberEntityField(MSG& msg, intptr_t bits, const void* toF, size_t size);
		void ReadRegular2(MSG& msg, intptr_t bits, void* toF, size_t size);
		void WriteRegular2(MSG& msg, intptr_t bits, const void* toF, size_t size);

		void ReadSimple(MSG& msg, intptr_t bits, void* toF, size_t size);
		void WriteSimple(MSG& msg, intptr_t bits, const void* toF, size_t size);

		float ReadAngleField(MSG& msg, size_t bits);
		void WriteAngleField(MSG& msg, size_t bits, float angle);

		float ReadTimeField(MSG& msg, size_t bits);
		void WriteTimeField(MSG& msg, float time);

		float ReadScaleField(MSG& msg, size_t bits);
		void WriteScaleField(MSG& msg, float time);

		uint32_t PackAngle(float angle, intptr_t bits);
		float UnpackAngle(int result, intptr_t bits, bool isNeg);
		uint32_t PackAngle2(float angle, intptr_t bits);
		float UnpackAngle2(uint32_t result, intptr_t bits);

		uint32_t PackAnimTime(float time, size_t bits);
		float UnpackAnimTime(int result);

		uint32_t PackAnimWeight(float weight, size_t bits);
		float UnpackAnimWeight(int result, intptr_t bits);

		uint32_t PackScale(float scale, size_t bits);
		float UnpackScale(int16_t result);

		uint32_t PackAlpha(float alpha, size_t bits);
		float UnpackAlpha(int result, intptr_t bits);

		void CopyFields(const entityState_t* other, entityState_t* target, size_t from, size_t to, const netField_t* fieldlist);
		int32_t PackCoord(float val);
		float UnpackCoord(int32_t val);

		int32_t PackCoordExtra(float val);
		float UnpackCoordExtra(int32_t val);


		bool DeltaNeeded(const void* fromField, const void* toField, const netField_t* field);
		bool DeltaNeeded_ver15(const void* fromField, const void* toField, const netField_t* field);
	}

	class MOHPC_NET_EXPORTS SerializableUsercmd : public ISerializableMessage
	{
	private:
		usercmd_t& ucmd;

	public:
		SerializableUsercmd(usercmd_t& inCmd)
			: ucmd(inCmd)
		{}

		void LoadDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) override;
		void SaveDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) const override;
	};

	class MOHPC_NET_EXPORTS SerializableUserEyes : public ISerializableMessage
	{
	private:
		usereyes_t& eyesInfo;

	public:
		SerializableUserEyes(usereyes_t& inEyesInfo)
			: eyesInfo(inEyesInfo)
		{}

		void SaveDelta(MSG& msg, const ISerializableMessage* from) const override;
		void LoadDelta(MSG& msg, const ISerializableMessage* from) override;
	};

	class MOHPC_NET_EXPORTS SerializablePlayerStateBase : public ISerializableMessage
	{
	public:
		SerializablePlayerStateBase(playerState_t& inState);

	public:
		playerState_t* GetState() const { return &state; }

	protected:
		playerState_t& state;
	};

	class MOHPC_NET_EXPORTS SerializablePlayerState : public SerializablePlayerStateBase
	{
	public:
		SerializablePlayerState(playerState_t& inState);

		void SaveDelta(MSG& msg, const ISerializableMessage* from) const override;
		void LoadDelta(MSG& msg, const ISerializableMessage* from) override;
		void NormalizePlayerState(playerState_t* ps) const;
		void UnNormalizePlayerState(playerState_t* ps) const;

		playerState_t* GetState() const { return &state; }
	};

	class MOHPC_NET_EXPORTS SerializablePlayerState_ver15 : public SerializablePlayerStateBase
	{
	public:
		SerializablePlayerState_ver15(playerState_t& inState);

		void SaveDelta(MSG& msg, const ISerializableMessage* from) const override;
		void LoadDelta(MSG& msg, const ISerializableMessage* from) override;
	};

	class MOHPC_NET_EXPORTS SerializableEntityState : public ISerializableMessage
	{
	protected:
		entityState_t& state;
		entityNum_t entNum;

	public:
		SerializableEntityState(entityState_t& inState, entityNum_t newNum)
			: state(inState)
			, entNum(newNum)
		{}

		void SaveDelta(MSG& msg, const ISerializableMessage* from) const override;
		void LoadDelta(MSG& msg, const ISerializableMessage* from) override;
		entityState_t* GetState() const { return &state; }
	};

	class MOHPC_NET_EXPORTS SerializableEntityState_ver15 : public SerializableEntityState
	{
	public:
		SerializableEntityState_ver15(entityState_t& inState, entityNum_t newNum, float timeDeltaValue);

		void SaveDelta(MSG& msg, const ISerializableMessage* from) const override;
		void LoadDelta(MSG& msg, const ISerializableMessage* from) override;

	private:
		float timeDelta;
	};

	namespace SerializableErrors
	{
		class Base : public Network::NetworkException {};

		class BadEntityNumberException : public Base
		{
		public:
			BadEntityNumberException(const char* name, size_t inBadNumber);

			MOHPC_NET_EXPORTS const char* getName() const;
			MOHPC_NET_EXPORTS size_t getNumber() const;
			MOHPC_NET_EXPORTS str what() const override;

		private:
			const char* name;
			size_t badNumber;
		};

		class BadEntityFieldException : public Base
		{
		private:
			uint8_t fieldType;
			const char* fieldName;

		public:
			BadEntityFieldException(uint8_t inFieldType, const char* inFieldName);

			MOHPC_NET_EXPORTS uint8_t getFieldType() const;
			MOHPC_NET_EXPORTS const char* getFieldName() const;
			MOHPC_NET_EXPORTS str what() const override;
		};

		class BadEntityFieldCountException : public Base
		{
		private:
			uint8_t count;

		public:
			BadEntityFieldCountException(uint8_t inCount);

			MOHPC_NET_EXPORTS uint8_t getCount() const;
			MOHPC_NET_EXPORTS str what() const override;
		};


		/**
		 * Invalid command while parsing game state.
		 */
		class BadCommandByteException : public Base
		{
		public:
			BadCommandByteException(uint8_t inCmdNum);

			MOHPC_NET_EXPORTS uint8_t getLength() const;
			MOHPC_NET_EXPORTS str what() const override;

		private:
			uint8_t cmdNum;
		};
	}
}