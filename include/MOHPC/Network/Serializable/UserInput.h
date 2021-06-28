#pragma once

#include "../../Utility/Misc/MSG/Serializable.h"

#include "../NetGlobal.h"

namespace MOHPC
{
namespace Network
{
	class usercmd_t;
	class usereyes_t;

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
}
}