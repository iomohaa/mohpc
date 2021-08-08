#pragma once

#include "../NetGlobal.h"
#include "../../Utility/Misc/MSG/Serializable.h"
#include "../../Utility/TickTypes.h"
#include "../Types/Entity.h"

namespace MOHPC
{
namespace Network
{
	class SerializableEntityState : public ISerializableMessage
	{
	protected:
		entityState_t& state;
		entityNum_t entNum;

	public:
		MOHPC_NET_EXPORTS SerializableEntityState(entityState_t& inState, entityNum_t newNum);

		void SaveDelta(MSG& msg, const ISerializableMessage* from) const override;
		void LoadDelta(MSG& msg, const ISerializableMessage* from) override;
		entityState_t* GetState() const { return &state; }
	};

	class SerializableEntityState_ver15 : public SerializableEntityState
	{
	public:
		MOHPC_NET_EXPORTS SerializableEntityState_ver15(entityState_t& inState, entityNum_t newNum, deltaTimeFloat_t timeDeltaValue);

		void SaveDelta(MSG& msg, const ISerializableMessage* from) const override;
		void LoadDelta(MSG& msg, const ISerializableMessage* from) override;

	private:
		deltaTimeFloat_t timeDelta;
	};
}
}