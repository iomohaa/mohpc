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

	enum class fieldType_e : uint8_t
	{
		number,
		angle,
		time,
		field03,
		time2,
		field05,
		largeCoord,
		smallCoord,
	};

	enum class fieldType_ver17_e : uint8_t
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
		size_t offset;
		size_t size;
		intptr_t bits;
		// see fieldType_e
		uint8_t type : 4;

		constexpr netField_t(const char* inName, size_t inOffset, size_t inSize, intptr_t inBits, uint8_t inType)
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

	namespace EntityField
	{
		void ReadNumberPlayerStateField(MSG& msg, size_t bits, void* toF, size_t size);
		void WriteNumberPlayerStateField(MSG& msg, size_t bits, void* toF);

		void ReadRegular(MSG& msg, size_t bits, void* toF, size_t size);
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

	class EntityException : public Network::NetworkException {};
	class BadEntityNumberException : public EntityException
	{
	private:
		size_t badNumber;

	public:
		BadEntityNumberException(size_t inBadNumber)
			: badNumber(inBadNumber)
		{}

		size_t getNumber() const { return badNumber; }
		virtual str what() { return str(badNumber); }
	};

	class BadEntityFieldException : public EntityException
	{
	private:
		uint8_t fieldType;
		const char* fieldName;

	public:
		BadEntityFieldException(uint8_t inFieldType, const char* inFieldName)
			: fieldType(inFieldType)
			, fieldName(inFieldName)
		{}

		uint8_t getFieldType() const { return fieldType; }
		const char* getFieldName() const { return fieldName; }
		virtual str what() { return str::printf("%s: %d", getFieldName(), getFieldType()); }
	};

	class BadEntityFieldCountException : public EntityException
	{
	private:
		uint8_t count;

	public:
		BadEntityFieldCountException(uint8_t inCount)
			: count(inCount)
		{}

		uint8_t getCount() const { return count; }
		virtual str what() { return str((int)getCount()); }
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
		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) override;
	};

	class MOHPC_EXPORTS SerializableUserEyes : public ISerializableMessage
	{
	private:
		usereyes_t& eyesInfo;

	public:
		SerializableUserEyes(usereyes_t& inEyesInfo)
			: eyesInfo(inEyesInfo)
		{}

		virtual void SerializeDelta(MSG & msg, const ISerializableMessage * from) override;
	};

	class MOHPC_EXPORTS SerializablePlayerState : public ISerializableMessage
	{
	protected:
		playerState_t& state;

	public:
		SerializablePlayerState(playerState_t& inState)
			: state(inState)
		{}

		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from) override;
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from) override;

		playerState_t* GetState() const { return &state; }
	};

	class MOHPC_EXPORTS SerializablePlayerState_ver17 : public SerializablePlayerState
	{
	public:
		SerializablePlayerState_ver17(playerState_t& inState)
			: SerializablePlayerState(inState)
		{}

		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from) override;
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from) override;
	};

	class MOHPC_EXPORTS SerializableEntityState : public ISerializableMessage
	{
	protected:
		entityState_t& state;

	public:
		SerializableEntityState(entityState_t& inState)
			: state(inState)
		{}

		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from) override;
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from) override;
		entityState_t* GetState() const { return &state; }
	};

	class MOHPC_EXPORTS SerializableEntityState_ver17 : public SerializableEntityState
	{
	public:
		SerializableEntityState_ver17(entityState_t& inState)
			: SerializableEntityState(inState)
		{}

		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from) override;
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from) override;

	private:

	};
}