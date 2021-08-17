#pragma once

#include "../NetGlobal.h"
#include "../../Utility/Misc/MSG/Serializable.h"

namespace MOHPC
{
namespace Network
{
	class MOHPC_NET_EXPORTS SerializableAngle : public ISerializableMessage
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

	class MOHPC_NET_EXPORTS SerializableAngle8 : public SerializableAngle
	{
	public:
		SerializableAngle8() = default;
		SerializableAngle8(float inValue);
		void Load(MSG& msg) override;
		void Save(MSG& msg) const override;
	};

	class MOHPC_NET_EXPORTS SerializableAngle16 : public SerializableAngle
	{
	public:
		SerializableAngle16() = default;
		SerializableAngle16(float inValue);
		void Load(MSG& msg) override;
		void Save(MSG& msg) const override;
	};
}
}