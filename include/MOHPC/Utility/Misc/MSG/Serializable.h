#pragma once

#include "../../UtilityGlobal.h"
#include "../../../Common/Vector.h"

#include <stdint.h>
#include <exception>

namespace MOHPC
{
	class MSG;

	class MOHPC_UTILITY_EXPORTS ISerializableMessage
	{
	public:
		virtual ~ISerializableMessage() {};
		virtual void Save(MSG& msg) const {};
		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from) const {};
		virtual void SaveDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) const {};
		virtual void Load(MSG& msg) {};
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from) {};
		virtual void LoadDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) {};
	};

	class MOHPC_UTILITY_EXPORTS SerializableAngle : public ISerializableMessage
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

	class MOHPC_UTILITY_EXPORTS SerializableAngle8 : public SerializableAngle
	{
	public:
		SerializableAngle8() = default;
		SerializableAngle8(float inValue);
		void Load(MSG& msg) override;
		void Save(MSG& msg) const override;
	};

	class MOHPC_UTILITY_EXPORTS SerializableAngle16 : public SerializableAngle
	{
	public:
		SerializableAngle16() = default;
		SerializableAngle16(float inValue);
		void Load(MSG& msg) override;
		void Save(MSG& msg) const override;
	};
};
