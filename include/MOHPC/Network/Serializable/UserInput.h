#pragma once

#include "../NetGlobal.h"
#include "../../Utility/Misc/MSG/Serializable.h"

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
		SerializableUsercmd(usercmd_t& inCmd);

		void LoadDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) override;
		void SaveDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) override;

	private:
		uint32_t time32;
	};

	class MOHPC_NET_EXPORTS SerializableUserEyes : public ISerializableMessage
	{
	private:
		usereyes_t& eyesInfo;

	public:
		SerializableUserEyes(usereyes_t& inEyesInfo)
			: eyesInfo(inEyesInfo)
		{}

		void SaveDelta(MSG& msg, const ISerializableMessage* from) override;
		void LoadDelta(MSG& msg, const ISerializableMessage* from) override;
	};
}
}