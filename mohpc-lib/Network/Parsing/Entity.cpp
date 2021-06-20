#include <MOHPC/Network/Parsing/Entity.h>
#include <MOHPC/Network/SerializableTypes.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::Parsing;

class Entity8 : public Parsing::IEntity
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 5;
		maxRange = 8;
	}

	entityNum_t readEntityNum(MSG& msg) const override
	{
		MsgTypesHelper helper(msg);
		return helper.ReadEntityNum();
	}

	void writeEntityNum(MSG& msg, entityNum_t num) const override
	{
		MsgTypesHelper helper(msg);
		return helper.WriteEntityNum(num);
	}

	void readDeltaEntity(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum, float deltaTime) const override
	{
		SerializableEntityState toSerialize(*to, newNum);
		if (from)
		{
			SerializableEntityState fromSerialize(*const_cast<entityState_t*>(from), newNum);
			msg.ReadDeltaClass(&fromSerialize, &toSerialize);
		}
		else
		{
			// no delta
			msg.ReadDeltaClass(nullptr, &toSerialize);
		}
	}

	void writeDeltaEntity(MSG& msg, const entityState_t* from, const entityState_t* to, entityNum_t newNum, float deltaTime) const override
	{
		SerializableEntityState toSerialize(*const_cast<entityState_t*>(to), newNum);
		if (from)
		{
			SerializableEntityState fromSerialize(*const_cast<entityState_t*>(from), newNum);
			msg.WriteDeltaClass(&fromSerialize, &toSerialize);
		}
		else
		{
			// no delta
			msg.WriteDeltaClass(nullptr, &toSerialize);
		}
	}
};

class Entity17 : public Entity8
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 15;
		maxRange = 17;
	}

	entityNum_t readEntityNum(MSG& msg) const override
	{
		MsgTypesHelper helper(msg);
		return helper.ReadEntityNum2();
	}

	void writeEntityNum(MSG& msg, entityNum_t num) const override
	{
		MsgTypesHelper helper(msg);
		return helper.WriteEntityNum2(num);
	}

	void readDeltaEntity(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum, float deltaTime) const override
	{
		SerializableEntityState_ver15 toSerialize(*to, newNum, deltaTime);
		if (from)
		{
			SerializableEntityState_ver15 fromSerialize(*const_cast<entityState_t*>(from), newNum, deltaTime);
			msg.ReadDeltaClass(&fromSerialize, &toSerialize);
		}
		else
		{
			// no delta
			msg.ReadDeltaClass(nullptr, &toSerialize);
		}
	}

	void writeDeltaEntity(MSG& msg, const entityState_t* from, const entityState_t* to, entityNum_t newNum, float deltaTime) const override
	{
		SerializableEntityState_ver15 toSerialize(*const_cast<entityState_t*>(to), newNum, deltaTime);
		if (from)
		{
			SerializableEntityState_ver15 fromSerialize(*const_cast<entityState_t*>(from), newNum, deltaTime);
			msg.WriteDeltaClass(&fromSerialize, &toSerialize);
		}
		else
		{
			// no delta
			msg.WriteDeltaClass(nullptr, &toSerialize);
		}
	}
};

using EntityDefault = ProtocolSingletonInherit<Entity17, 0>;

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

static Entity8 entityVersion8;
static Entity17 entityVersion17;
static EntityDefault entityDefaultVersion;

static PlayerState8 playerStateVersion8;
static PlayerState17 playerStateVersion17;
static PlayerStateDefault playerStateDefaultVersion;
