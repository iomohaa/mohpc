#pragma once

#include "../NetGlobal.h"
#include "../../Utility/Misc/MSG/Serializable.h"
#include "../Types/Entity.h"

namespace MOHPC
{
namespace Network
{
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
}
}