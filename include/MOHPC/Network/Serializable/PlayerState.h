#pragma once

#include "../NetGlobal.h"
#include "../../Utility/Misc/MSG/Serializable.h"

namespace MOHPC
{
namespace Network
{
	class playerState_t;

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

		void SaveDelta(MSG& msg, const ISerializableMessage* from) override;
		void LoadDelta(MSG& msg, const ISerializableMessage* from) override;
		void NormalizePlayerState(playerState_t* ps) const;
		void UnNormalizePlayerState(playerState_t* ps) const;

		playerState_t* GetState() const { return &state; }
	};

	class MOHPC_NET_EXPORTS SerializablePlayerState_ver15 : public SerializablePlayerStateBase
	{
	public:
		SerializablePlayerState_ver15(playerState_t& inState);

		void SaveDelta(MSG& msg, const ISerializableMessage* from) override;
		void LoadDelta(MSG& msg, const ISerializableMessage* from) override;
	};
}
}