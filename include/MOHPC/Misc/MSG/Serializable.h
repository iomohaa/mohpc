#pragma once

#include <stdint.h>
#include "../../Vector.h"
#include <exception>

namespace MOHPC
{
	class MSG;

	class MOHPC_EXPORTS ISerializableMessage
	{
	public:
		virtual ~ISerializableMessage() {};
		virtual void Serialize(MSG& msg) {};
		virtual void SerializeDelta(MSG& msg, const ISerializableMessage* from) {};
		virtual void SerializeDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) {};
		virtual void Save(MSG& msg) {};
		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from) {};
		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) {};
		virtual void Load(MSG& msg) {};
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from) {};
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) {};
	};

	class MOHPC_EXPORTS SerializableAngle : public ISerializableMessage
	{
	protected:
		float value;

	public:
		SerializableAngle() = default;
		SerializableAngle(float inValue);

		operator float();
		operator float() const;
		operator float* ();
		operator const float* () const;
	};

	class MOHPC_EXPORTS SerializableAngle8 : public SerializableAngle
	{
	public:
		SerializableAngle8() = default;
		SerializableAngle8(float inValue);
		virtual void Serialize(MSG& msg) override;
	};

	class MOHPC_EXPORTS SerializableAngle16 : public SerializableAngle
	{
	public:
		SerializableAngle16() = default;
		SerializableAngle16(float inValue);
		virtual void Serialize(MSG& msg) override;
	};
};
