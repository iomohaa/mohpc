#pragma once

#include <stdint.h>
#include <exception>
#include "../Misc/MSG/Serializable.h"
#include "Types.h"

namespace MOHPC
{
	class usercmd_t;
	class usereyes_t;
	class playerState_t;
	class entityState_t;

	/**
	 * Field type for protocol version 8.
	 */
	enum class fieldType_ver6_e : uint8_t
	{
		number,
		angle,
		animTime,
		animWeight,
		scale,
		alpha,
		largeCoord,
		smallCoord,
	};

	/**
	 * Field type for protocol version 15.
	 * Introduced mediumCoord, presumably to provide something between large and medium
	 */
	enum class fieldType_ver15_e : uint8_t
	{
		number,
		angle,
		animTime,
		animWeight,
		scale,
		alpha,
		mediumCoord,
		largeCoord,
		smallCoord,
	};

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

	namespace EntityField
	{
		void ReadNumberPlayerStateField(MSG& msg, size_t bits, void* toF, size_t size);
		void WriteNumberPlayerStateField(MSG& msg, size_t bits, void* toF);

		void ReadRegular(MSG& msg, intptr_t bits, void* toF, size_t size);
		void ReadRegular2(MSG& msg, intptr_t bits, void* toF, size_t size);
		void WriteNumberEntityField(MSG& msg, size_t bits, void* toF, size_t size);

		float ReadAngleField(MSG& msg, size_t bits);
		void WriteAngleField(MSG& msg, size_t bits, float angle);

		float ReadTimeField(MSG& msg, size_t bits);
		void WriteTimeField(MSG& msg, float time);

		float ReadSmallTimeField(MSG& msg, size_t bits);
		void WriteSmallTimeField(MSG& msg, float time);

		float UnpackAngle(int result, intptr_t bits, bool isNeg);
		float UnpackAnimTime(int result);
		float UnpackAnimWeight(int result, intptr_t bits);
		float UnpackScale(int result);
		float UnpackAlpha(int result, intptr_t bits);
		void CopyFields(entityState_t* other, entityState_t* target, size_t from, size_t to, const netField_t* fieldlist);
		int32_t PackCoord(float val);
		int32_t PackCoordExtra(float val);
		float UnpackCoord(int32_t val);
		float UnpackCoordExtra(int32_t val);
	}

	class MOHPC_EXPORTS EntityException : public Network::NetworkException {};

	class MOHPC_EXPORTS BadEntityNumberException : public EntityException
	{
	private:
		size_t badNumber;

	public:
		BadEntityNumberException(size_t inBadNumber);

		size_t getNumber() const;
		str what() const override;
	};

	class MOHPC_EXPORTS BadEntityFieldException : public EntityException
	{
	private:
		uint8_t fieldType;
		const char* fieldName;

	public:
		BadEntityFieldException(uint8_t inFieldType, const char* inFieldName);

		uint8_t getFieldType() const;
		const char* getFieldName() const;
		str what() const override;
	};

	class MOHPC_EXPORTS BadEntityFieldCountException : public EntityException
	{
	private:
		uint8_t count;

	public:
		BadEntityFieldCountException(uint8_t inCount);

		uint8_t getCount() const;
		str what() const override;
	};

	class MOHPC_EXPORTS SerializableUsercmd : public ISerializableMessage
	{
	private:
		usercmd_t& ucmd;

	public:
		SerializableUsercmd(usercmd_t& inCmd)
			: ucmd(inCmd)
		{}

		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) override;
		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) const override;
	};

	class MOHPC_EXPORTS SerializableUserEyes : public ISerializableMessage
	{
	private:
		usereyes_t& eyesInfo;

	public:
		SerializableUserEyes(usereyes_t& inEyesInfo)
			: eyesInfo(inEyesInfo)
		{}

		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from) const override;
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from) override;
	};

	class MOHPC_EXPORTS SerializablePlayerState : public ISerializableMessage
	{
	protected:
		playerState_t& state;

	public:
		SerializablePlayerState(playerState_t& inState)
			: state(inState)
		{}

		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from) const override;
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from) override;

		playerState_t* GetState() const { return &state; }
	};

	class MOHPC_EXPORTS SerializablePlayerState_ver15 : public SerializablePlayerState
	{
	public:
		SerializablePlayerState_ver15(playerState_t& inState)
			: SerializablePlayerState(inState)
		{}

		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from) const override;
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from) override;
	};

	class MOHPC_EXPORTS SerializableEntityState : public ISerializableMessage
	{
	protected:
		entityState_t& state;
		entityNum_t entNum;

	public:
		SerializableEntityState(entityState_t& inState, entityNum_t newNum)
			: state(inState)
			, entNum(newNum)
		{}

		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from) const override;
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from) override;
		entityState_t* GetState() const { return &state; }
	};

	class MOHPC_EXPORTS SerializableEntityState_ver15 : public SerializableEntityState
	{
	public:
		SerializableEntityState_ver15(entityState_t& inState, entityNum_t newNum)
			: SerializableEntityState(inState, newNum)
		{}

		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from) const override;
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from) override;

	private:

	};
}