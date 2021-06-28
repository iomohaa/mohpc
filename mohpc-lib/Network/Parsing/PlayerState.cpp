#include <MOHPC/Network/Parsing/PlayerState.h>
#include <MOHPC/Network/Types/PlayerState.h>
#include <MOHPC/Network/Serializable/PlayerState.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::Parsing;


class PlayerState8 : public Parsing::IPlayerState
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 5;
		maxRange = 8;
	}

	void readDeltaPlayerState(MSG& msg, const playerState_t* from, playerState_t* to) const override
	{
		SerializablePlayerState toSerialize(*to);
		if (from)
		{
			SerializablePlayerState fromSerialize(*const_cast<playerState_t*>(from));
			msg.ReadDeltaClass(&fromSerialize, &toSerialize);
		}
		else
		{
			// no delta
			msg.ReadDeltaClass(nullptr, &toSerialize);
		}
	}

	void writeDeltaPlayerState(MSG& msg, const playerState_t* from, const playerState_t* to) const override
	{
		SerializablePlayerState toSerialize(*const_cast<playerState_t*>(to));
		if (from)
		{
			SerializablePlayerState fromSerialize(*const_cast<playerState_t*>(from));
			msg.WriteDeltaClass(&fromSerialize, &toSerialize);
		}
		else
		{
			// no delta
			msg.WriteDeltaClass(nullptr, &toSerialize);
		}
	}
};

class PlayerState17 : public PlayerState8
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 15;
		maxRange = 17;
	}

	void readDeltaPlayerState(MSG& msg, const playerState_t* from, playerState_t* to) const override
	{
		SerializablePlayerState_ver15 toSerialize(*to);
		if (from)
		{
			SerializablePlayerState_ver15 fromSerialize(*const_cast<playerState_t*>(from));
			msg.ReadDeltaClass(&fromSerialize, &toSerialize);
		}
		else
		{
			// no delta
			msg.ReadDeltaClass(nullptr, &toSerialize);
		}
	}

	void writeDeltaPlayerState(MSG& msg, const playerState_t* from, const playerState_t* to) const override
	{
		SerializablePlayerState_ver15 toSerialize(*const_cast<playerState_t*>(to));
		if (from)
		{
			SerializablePlayerState_ver15 fromSerialize(*const_cast<playerState_t*>(from));
			msg.WriteDeltaClass(&fromSerialize, &toSerialize);
		}
		else
		{
			// no delta
			msg.WriteDeltaClass(nullptr, &toSerialize);
		}
	}
};

using PlayerStateDefault = ProtocolSingletonInherit<PlayerState17, 0>;

static PlayerState8 playerStateVersion8;
static PlayerState17 playerStateVersion17;
static PlayerStateDefault playerStateDefaultVersion;
