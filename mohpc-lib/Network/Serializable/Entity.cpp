#include <MOHPC/Network/Serializable/Entity.h>
#include <MOHPC/Network/Serializable/NetField.h>
#include <MOHPC/Network/Serializable/EntityField.h>

#include <MOHPC/Network/Types/Entity.h>

using namespace MOHPC;
using namespace MOHPC::Network;

#define	NETF(x) #x,(uint16_t)(size_t)&((entityState_t*)0)->x,sizeof(entityState_t::x)

const netField_template_t<fieldType_ver6_e> entityStateFields[] =
{
{ NETF(netorigin[0]), 0, fieldType_ver6_e::coord },
{ NETF(netorigin[1]), 0, fieldType_ver6_e::coord },
{ NETF(netangles[1]), 12, fieldType_ver6_e::angle },
{ NETF(frameInfo[0].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[1].time), 15, fieldType_ver6_e::animTime },
{ NETF(bone_angles[0][0]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[3][0]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[1][0]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[2][0]), -13, fieldType_ver6_e::angle },
{ NETF(netorigin[2]), 0, fieldType_ver6_e::coord },
{ NETF(frameInfo[0].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[1].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[2].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[3].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[0].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[1].index), 12, fieldType_ver6_e::regular },
{ NETF(actionWeight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[2].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[3].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[2].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[3].index), 12, fieldType_ver6_e::regular },
{ NETF(eType), 8, fieldType_ver6_e::regular },
{ NETF(modelindex), 16, fieldType_ver6_e::regular },
{ NETF(parent), 16, fieldType_ver6_e::regular },
{ NETF(constantLight), 32, fieldType_ver6_e::regular },
{ NETF(renderfx), 32, fieldType_ver6_e::regular },
{ NETF(bone_tag[0]), -8, fieldType_ver6_e::regular },
{ NETF(bone_tag[1]), -8, fieldType_ver6_e::regular },
{ NETF(bone_tag[2]), -8, fieldType_ver6_e::regular },
{ NETF(bone_tag[3]), -8, fieldType_ver6_e::regular },
{ NETF(bone_tag[4]), -8, fieldType_ver6_e::regular },
{ NETF(scale), 10, fieldType_ver6_e::scale },
{ NETF(alpha), 8, fieldType_ver6_e::alpha },
{ NETF(usageIndex), 16, fieldType_ver6_e::regular },
{ NETF(eFlags), 16, fieldType_ver6_e::regular },
{ NETF(solid), 32, fieldType_ver6_e::regular },
{ NETF(netangles[2]), 12, fieldType_ver6_e::angle },
{ NETF(netangles[0]), 12, fieldType_ver6_e::angle },
{ NETF(tag_num), 10, fieldType_ver6_e::regular },
{ NETF(bone_angles[1][2]), -13, fieldType_ver6_e::angle },
{ NETF(attach_use_angles), 1, fieldType_ver6_e::regular },
{ NETF(origin2[1]), 0, fieldType_ver6_e::coord },
{ NETF(origin2[0]), 0, fieldType_ver6_e::coord },
{ NETF(origin2[2]), 0, fieldType_ver6_e::coord },
{ NETF(bone_angles[0][2]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[2][2]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[3][2]), -13, fieldType_ver6_e::angle },
{ NETF(surfaces[0]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[1]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[2]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[3]), 8, fieldType_ver6_e::regular },
{ NETF(bone_angles[0][1]), -13, fieldType_ver6_e::angle },
{ NETF(surfaces[4]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[5]), 8, fieldType_ver6_e::regular },
{ NETF(pos.trTime), 32, fieldType_ver6_e::regular },
//{ NETF(pos.trBase[0]), 0, fieldType_ver6_e::regular },
//{ NETF(pos.trBase[1]), 0, fieldType_ver6_e::regular },
{ NETF(pos.trDelta[0]), 0, fieldType_ver6_e::velocity },
{ NETF(pos.trDelta[1]), 0, fieldType_ver6_e::velocity },
//{ NETF(pos.trBase[2]), 0, fieldType_ver6_e::regular },
//{ NETF(apos.trBase[1]), 0, fieldType_ver6_e::regular },
{ NETF(pos.trDelta[2]), 0, fieldType_ver6_e::velocity },
//{ NETF(apos.trBase[0]), 0, fieldType_ver6_e::regular },
{ NETF(loopSound), 16, fieldType_ver6_e::regular },
{ NETF(loopSoundVolume), 0, fieldType_ver6_e::regular },
{ NETF(loopSoundMinDist), 0, fieldType_ver6_e::regular },
{ NETF(loopSoundMaxDist), 0, fieldType_ver6_e::regular },
{ NETF(loopSoundPitch), 0, fieldType_ver6_e::regular },
{ NETF(loopSoundFlags), 8, fieldType_ver6_e::regular },
{ NETF(attach_offset[0]), 0, fieldType_ver6_e::regular },
{ NETF(attach_offset[1]), 0, fieldType_ver6_e::regular },
{ NETF(attach_offset[2]), 0, fieldType_ver6_e::regular },
{ NETF(beam_entnum), 16, fieldType_ver6_e::regular },
{ NETF(skinNum), 16, fieldType_ver6_e::regular },
{ NETF(wasframe), 10, fieldType_ver6_e::regular },
{ NETF(frameInfo[4].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[5].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[6].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[7].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[8].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[9].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[10].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[11].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[12].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[13].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[14].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[15].index), 12, fieldType_ver6_e::regular },
{ NETF(frameInfo[4].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[5].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[6].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[7].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[8].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[9].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[10].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[11].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[12].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[13].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[14].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[15].time), 15, fieldType_ver6_e::animTime },
{ NETF(frameInfo[4].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[5].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[6].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[7].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[8].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[9].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[10].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[11].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[12].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[13].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[14].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[15].weight), 8, fieldType_ver6_e::animWeight },
{ NETF(bone_angles[1][1]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[2][1]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[3][1]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[4][0]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[4][1]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[4][2]), -13, fieldType_ver6_e::angle },
{ NETF(clientNum), 8, fieldType_ver6_e::regular },
{ NETF(groundEntityNum), GENTITYNUM_BITS, fieldType_ver6_e::regular },
{ NETF(shader_data[0]), 0, fieldType_ver6_e::regular },
{ NETF(shader_data[1]), 0, fieldType_ver6_e::regular },
{ NETF(shader_time), 0, fieldType_ver6_e::regular },
{ NETF(eyeVector[0]), 0, fieldType_ver6_e::regular },
{ NETF(eyeVector[1]), 0, fieldType_ver6_e::regular },
{ NETF(eyeVector[2]), 0, fieldType_ver6_e::regular },
{ NETF(surfaces[6]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[7]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[8]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[9]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[10]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[11]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[12]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[13]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[14]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[15]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[16]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[17]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[18]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[19]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[20]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[21]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[22]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[23]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[24]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[25]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[26]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[27]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[28]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[29]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[30]), 8, fieldType_ver6_e::regular },
{ NETF(surfaces[31]), 8, fieldType_ver6_e::regular }
};

static_assert(sizeof(entityStateFields) == sizeof(netField_t) * 146);

// Fields for SH & BT
const netField_template_t<fieldType_ver15_e> entityStateFields_ver15[] =
{
{ NETF(netorigin[0]), 0, fieldType_ver15_e::coord },
{ NETF(netorigin[1]), 0, fieldType_ver15_e::coord },
{ NETF(netangles[1]), 12, fieldType_ver15_e::angle },
{ NETF(frameInfo[0].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[1].time), 15, fieldType_ver15_e::animTime },
{ NETF(bone_angles[0][0]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[3][0]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[1][0]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[2][0]), -13, fieldType_ver15_e::angle },
{ NETF(netorigin[2]), 0, fieldType_ver15_e::coord },
{ NETF(frameInfo[0].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[1].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[2].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[3].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[0].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[1].index), 12, fieldType_ver15_e::regular },
{ NETF(actionWeight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[2].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[3].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[2].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[3].index), 12, fieldType_ver15_e::regular },
{ NETF(eType), 8, fieldType_ver15_e::regular },
{ NETF(modelindex), 16, fieldType_ver15_e::regular },
{ NETF(parent), 16, fieldType_ver15_e::regular },
{ NETF(constantLight), 32, fieldType_ver15_e::regular },
{ NETF(renderfx), 32, fieldType_ver15_e::regular },
{ NETF(bone_tag[0]), -8, fieldType_ver15_e::regular },
{ NETF(bone_tag[1]), -8, fieldType_ver15_e::regular },
{ NETF(bone_tag[2]), -8, fieldType_ver15_e::regular },
{ NETF(bone_tag[3]), -8, fieldType_ver15_e::regular },
{ NETF(bone_tag[4]), -8, fieldType_ver15_e::regular },
{ NETF(scale), 10, fieldType_ver15_e::scale },
{ NETF(alpha), 8, fieldType_ver15_e::alpha },
{ NETF(usageIndex), 16, fieldType_ver15_e::regular },
{ NETF(eFlags), 16, fieldType_ver15_e::regular },
{ NETF(solid), 32, fieldType_ver15_e::regular },
{ NETF(netangles[2]), 12, fieldType_ver15_e::angle },
{ NETF(netangles[0]), 12, fieldType_ver15_e::angle },
{ NETF(tag_num), 10, fieldType_ver15_e::regular },
{ NETF(bone_angles[1][2]), -13, fieldType_ver15_e::angle },
{ NETF(attach_use_angles), 1, fieldType_ver15_e::regular },
{ NETF(origin2[1]), 0, fieldType_ver15_e::coord },
{ NETF(origin2[0]), 0, fieldType_ver15_e::coord },
{ NETF(origin2[2]), 0, fieldType_ver15_e::coord },
{ NETF(bone_angles[0][2]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[2][2]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[3][2]), -13, fieldType_ver15_e::angle },
{ NETF(surfaces[0]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[1]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[2]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[3]), 8, fieldType_ver15_e::regular },
{ NETF(bone_angles[0][1]), -13, fieldType_ver15_e::angle },
{ NETF(surfaces[4]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[5]), 8, fieldType_ver15_e::regular },
{ NETF(pos.trTime), 32, fieldType_ver15_e::regular },
{ NETF(pos.trDelta[0]), 0, fieldType_ver15_e::velocity },
{ NETF(pos.trDelta[1]), 0, fieldType_ver15_e::velocity },
{ NETF(pos.trDelta[2]), 0, fieldType_ver15_e::velocity },
{ NETF(loopSound), 16, fieldType_ver15_e::regular },
{ NETF(loopSoundVolume), 0, fieldType_ver15_e::regular },
{ NETF(loopSoundMinDist), 0, fieldType_ver15_e::regular },
{ NETF(loopSoundMaxDist), 0, fieldType_ver15_e::regular },
{ NETF(loopSoundPitch), 0, fieldType_ver15_e::regular },
{ NETF(loopSoundFlags), 8, fieldType_ver15_e::regular },
{ NETF(attach_offset[0]), 0, fieldType_ver15_e::regular },
{ NETF(attach_offset[1]), 0, fieldType_ver15_e::regular },
{ NETF(attach_offset[2]), 0, fieldType_ver15_e::regular },
{ NETF(beam_entnum), 16, fieldType_ver15_e::regular },
{ NETF(skinNum), 16, fieldType_ver15_e::regular },
{ NETF(wasframe), 10, fieldType_ver15_e::regular },
{ NETF(frameInfo[4].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[5].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[6].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[7].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[8].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[9].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[10].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[11].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[12].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[13].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[14].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[15].index), 12, fieldType_ver15_e::regular },
{ NETF(frameInfo[4].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[5].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[6].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[7].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[8].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[9].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[10].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[11].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[12].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[13].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[14].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[15].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[4].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[5].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[6].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[7].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[8].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[9].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[10].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[11].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[12].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[13].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[14].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[15].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(bone_angles[1][1]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[2][1]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[3][1]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[4][0]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[4][1]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[4][2]), -13, fieldType_ver15_e::angle },
{ NETF(clientNum), 8, fieldType_ver15_e::regular },
{ NETF(groundEntityNum), GENTITYNUM_BITS, fieldType_ver15_e::regular },
{ NETF(shader_data[0]), 0, fieldType_ver15_e::regular },
{ NETF(shader_data[1]), 0, fieldType_ver15_e::regular },
{ NETF(shader_time), 0, fieldType_ver15_e::regular },
{ NETF(eyeVector[0]), 0, fieldType_ver15_e::regular },
{ NETF(eyeVector[1]), 0, fieldType_ver15_e::regular },
{ NETF(eyeVector[2]), 0, fieldType_ver15_e::regular },
{ NETF(surfaces[6]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[7]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[8]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[9]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[10]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[11]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[12]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[13]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[14]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[15]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[16]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[17]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[18]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[19]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[20]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[21]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[22]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[23]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[24]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[25]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[26]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[27]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[28]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[29]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[30]), 8, fieldType_ver15_e::regular },
{ NETF(surfaces[31]), 8, fieldType_ver15_e::regular }
};

static_assert(sizeof(entityStateFields_ver15) == sizeof(netField_t) * 146);

static const entityState_t nullstate;

void SerializableEntityState::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{
	MsgTypesHelper msgHelper(msg);
	MsgTypesEntityHelper msgEntHelper(msg);
	constexpr size_t numFields = sizeof(entityStateFields) / sizeof(entityStateFields[0]);

	uint8_t lc = 0;
	uint8_t i = 0;
	const netField_t* field;
	bool deltaNeededList[numFields];

	const entityState_t* fromEnt = from ? ((const SerializableEntityState*)from)->GetState() : &nullstate;

	if (state.number == ENTITYNUM_NONE)
	{
		if (!fromEnt) {
			return;
		}

		msgEntHelper.WriteEntityNum(fromEnt->number);
		// removed
		msg.WriteBool(true);
		return;
	}

	// not removed
	msg.WriteBool(false);

	// build the change vector as bytes so it is endien independent
	for (i = 0, field = entityStateFields; i < numFields; i++, field++)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromEnt + field->getOffset());
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)&state + field->getOffset());

		const bool deltaNeeded = EntityField::DeltaNeeded(fromF, toF, field);
		deltaNeededList[i] = deltaNeeded;
		if (deltaNeeded) {
			lc = i + 1;
		}
	}

	const bool hasDelta = lc > 0;
	// write true if it has delta
	msg.WriteBool(hasDelta);

	if (!hasDelta)
	{
		// no delta
		return;
	}

	// write # of changes
	msg.WriteByte(lc);
	if (lc > numFields) {
		throw SerializableErrors::BadEntityFieldCountException(lc);
	}

	for (i = 0, field = entityStateFields; i < lc; i++, field++)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromEnt + field->getOffset());
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)&state + field->getOffset());

		bool isDiff = deltaNeededList[i];
		msg.WriteBool(isDiff);
		if (!isDiff) {
			continue;
		}

		uint32_t packedValue;

		switch (fieldType_ver6_e(field->getType()))
		{
		case fieldType_ver6_e::regular:
			EntityField::WriteNumberEntityField(msg, field->getBits(), toF, field->getSize());
			break;
		case fieldType_ver6_e::angle:
			EntityField::WriteAngleField(msg, field->getBits(), *(float*)toF);
			break;
		case fieldType_ver6_e::animTime:
			packedValue = EntityField::PackAnimTime(*(float*)toF, field->getBits());
			msg.WriteNumber(packedValue, field->getBits());
			break;
		case fieldType_ver6_e::animWeight:
			packedValue = EntityField::PackAnimWeight(*(float*)toF, field->getBits());
			msg.WriteNumber(packedValue, field->getBits());
			break;
		case fieldType_ver6_e::scale:
			packedValue = EntityField::PackScale(*(float*)toF, field->getBits());
			msg.WriteNumber(packedValue, field->getBits());
			break;
		case fieldType_ver6_e::alpha:
			packedValue = EntityField::PackAlpha(*(float*)toF, field->getBits());
			msg.WriteNumber(packedValue, field->getBits());
			break;
		case fieldType_ver6_e::coord:
			msgHelper.WriteCoord(*(float*)toF);
			break;
		case fieldType_ver6_e::velocity:
			msgHelper.WriteCoordSmall(*(float*)toF);
			break;
		case fieldType_ver6_e::simple:
			EntityField::WriteSimple(msg, field->getBits(), toF, field->getSize());
			break;
		default:
			throw SerializableErrors::BadEntityFieldException(field->getType(), field->getName());
		}
	}
}

void SerializableEntityState::LoadDelta(MSG& msg, const ISerializableMessage* from)
{
	MsgTypesHelper msgHelper(msg);

	const entityState_t* fromEnt = from ? ((const SerializableEntityState*)from)->GetState() : &nullstate;

	const bool removed = msg.ReadBool();
	if (removed)
	{
		state = entityState_t();
		state.number = ENTITYNUM_NONE;
		return;
	}

	const bool hasDelta = msg.ReadBool();
	if (!hasDelta)
	{
		state = *(entityState_t*)fromEnt;
		state.number = entNum;
		return;
	}

	// Set the new number
	state.number = entNum;

	constexpr size_t numFields = sizeof(entityStateFields) / sizeof(entityStateFields[0]);

	// # of changes
	const uint8_t lc = msg.ReadByte();
	if (lc > numFields) {
		throw SerializableErrors::BadEntityFieldCountException(lc);
	}

	size_t i;
	const netField_t* field;

	for (i = 0, field = entityStateFields; i < lc; i++, field++)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromEnt + field->getOffset());
		uint8_t* toF = (uint8_t*)((uint8_t*)&state + field->getOffset());

		const bool isDiff = msg.ReadBool();
		if (!isDiff)
		{
			// no changes
			memcpy(toF, fromF, field->getSize());
			continue;
		}

		uint32_t packedValue;

		switch (fieldType_ver6_e(field->getType()))
		{
		case fieldType_ver6_e::regular:
			EntityField::ReadRegular(msg, field->getBits(), toF, field->getSize());
			break;
		case fieldType_ver6_e::angle:
			*(float*)toF = EntityField::ReadAngleField(msg, field->getBits());
			break;
		case fieldType_ver6_e::animTime:
			packedValue = msg.ReadNumber<uint32_t>(field->getBits());
			*(float*)toF = EntityField::UnpackAnimTime(packedValue);
			break;
		case fieldType_ver6_e::animWeight:
			packedValue = msg.ReadByte();
			*(float*)toF = EntityField::UnpackAnimWeight(packedValue, field->getBits());
			break;
		case fieldType_ver6_e::scale:
			packedValue = msg.ReadNumber<uint32_t>(field->getBits());
			*(float*)toF = EntityField::UnpackScale(packedValue);
			break;
		case fieldType_ver6_e::alpha:
			packedValue = msg.ReadByte();
			*(float*)toF = EntityField::UnpackAlpha(packedValue, field->getBits());
			break;
		case fieldType_ver6_e::coord:
			*(float*)toF = msgHelper.ReadCoord();
			break;
		case fieldType_ver6_e::velocity:
			*(float*)toF = msgHelper.ReadCoordSmall();
			break;
		case fieldType_ver6_e::simple:
			EntityField::ReadSimple(msg, field->getBits(), toF, field->getSize());
			break;
		default:
			throw SerializableErrors::BadEntityFieldException(field->getType(), field->getName());
		}
	}

	// assign unchanged fields accordingly
	CopyFields<entityState_t>(fromEnt, GetState(), lc, numFields, entityStateFields);
}

void SerializableEntityState_ver15::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{
	MsgTypesHelper msgHelper(msg);
	MsgTypesEntityHelper msgEntHelper(msg);
	constexpr size_t numFields = sizeof(entityStateFields_ver15) / sizeof(entityStateFields_ver15[0]);

	uint8_t lc = 0;
	uint8_t i = 0;
	const netField_t* field;
	bool deltaNeededList[numFields];

	const entityState_t* fromEnt = from ? ((const SerializableEntityState*)from)->GetState() : &nullstate;

	if (state.number == ENTITYNUM_NONE)
	{
		if (!fromEnt) {
			return;
		}

		msgEntHelper.WriteEntityNum(fromEnt->number);
		// removed
		msg.WriteBool(true);
		return;
	}

	// not removed
	msg.WriteBool(false);

	// build the change vector as bytes so it is endien independent
	for (i = 0, field = entityStateFields_ver15; i < numFields; i++, field++)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromEnt + field->getOffset());
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)&state + field->getOffset());

		const bool deltaNeeded = EntityField::DeltaNeeded_ver15(fromF, toF, field);
		deltaNeededList[i] = deltaNeeded;
		if (deltaNeeded) {
			lc = i + 1;
		}
	}

	const bool hasDelta = lc > 0;
	// write true if it has delta
	msg.WriteBool(hasDelta);

	if (!hasDelta)
	{
		// no delta
		return;
	}

	// # of changes
	msg.WriteByte(lc);
	if (lc > numFields) {
		throw SerializableErrors::BadEntityFieldCountException(lc);
	}

	for (i = 0, field = entityStateFields_ver15; i < lc; i++, field++)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromEnt + field->getOffset());
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)&state + field->getOffset());

		bool isDiff = deltaNeededList[i];
		msg.WriteBool(isDiff);
		if (!isDiff) {
			continue;
		}

		uint32_t packedValue;

		switch (fieldType_ver15_e(field->getType()))
		{
		case fieldType_ver15_e::regular:
			EntityField::WriteRegular2(msg, field->getBits(), toF, field->getSize());
			break;
		case fieldType_ver15_e::angle:
			packedValue = EntityField::PackAngle2(*(float*)toF, field->getBits());
			msg.WriteNumber<uint32_t>(packedValue, field->getBits());
			break;
		case fieldType_ver15_e::animTime:
			if (fabs(*(float*)toF - *(float*)fromF) >= 0.001f)
			{
				msg.WriteBool(true);

				packedValue = EntityField::PackAnimTime(*(float*)toF, field->getBits());
				msg.WriteNumber(packedValue, field->getBits());
			}
			else
			{
				// no changes
				msg.WriteBool(false);
			}
			break;
		case fieldType_ver15_e::animWeight:
			packedValue = EntityField::PackAnimWeight(*(float*)toF, field->getBits());
			msg.WriteNumber(packedValue, field->getBits());
			break;
		case fieldType_ver15_e::scale:
			packedValue = EntityField::PackScale(*(float*)toF, field->getBits());
			msg.WriteNumber(packedValue, field->getBits());
			break;
		case fieldType_ver15_e::alpha:
			packedValue = EntityField::PackAlpha(*(float*)toF, field->getBits());
			msg.WriteNumber(packedValue, field->getBits());
			break;
		case fieldType_ver15_e::coord:
			msgHelper.WriteDeltaCoord(
				EntityField::PackCoord(*(float*)fromF),
				EntityField::PackCoord(*(float*)toF)
			);
			break;
		case fieldType_ver15_e::coordExtra:
			msgHelper.WriteDeltaCoordExtra(
				EntityField::PackCoordExtra(*(float*)fromF),
				EntityField::PackCoordExtra(*(float*)toF)
			);
			break;
		case fieldType_ver15_e::velocity:
			msgHelper.WriteCoordSmall(*(float*)toF);
			break;
		case fieldType_ver15_e::simple:
			EntityField::WriteSimple(msg, field->getBits(), toF, field->getSize());
			break;
		default:
			throw SerializableErrors::BadEntityFieldException(field->getType(), field->getName());
		}
	}
}

void SerializableEntityState_ver15::LoadDelta(MSG& msg, const ISerializableMessage* from)
{
	MsgTypesHelper msgHelper(msg);

	const entityState_t* fromEnt = from ? ((SerializableEntityState*)from)->GetState() : &nullstate;

	const bool removed = msg.ReadBool();
	if (removed)
	{
		state = entityState_t();
		state.number = ENTITYNUM_NONE;
		return;
	}

	const bool hasDelta = msg.ReadBool();
	if (!hasDelta)
	{
		state = *fromEnt;
		state.number = entNum;
		return;
	}

	// Set the new number
	state.number = entNum;

	constexpr size_t numFields = sizeof(entityStateFields_ver15) / sizeof(entityStateFields_ver15[0]);

	// # of changes
	const uint8_t lc = msg.ReadByte();
	if (lc > numFields) {
		throw SerializableErrors::BadEntityFieldCountException(lc);
	}

	size_t i;
	const netField_t* field;

	for (i = 0, field = entityStateFields_ver15; i < lc; i++, field++)
	{
		uint8_t* fromF = (uint8_t*)((uint8_t*)fromEnt + field->getOffset());
		uint8_t* toF = (uint8_t*)((uint8_t*)&state + field->getOffset());

		const bool hasChange = msg.ReadBool();
		if (!hasChange)
		{
			// no change
			std::memcpy(toF, fromF, field->getSize());
			continue;
		}

		int result;
		int32_t coordOffset, coordVal;

		switch (fieldType_ver15_e(field->getType()))
		{
		case fieldType_ver15_e::regular:
			EntityField::ReadRegular2(msg, field->getBits(), toF, field->getSize());
			break;
		case fieldType_ver15_e::angle:
			result = msg.ReadNumber<uint32_t>(field->getBits() < 0 ? -field->getBits() : field->getBits());
			*(float*)toF = EntityField::UnpackAngle2(result, field->getBits());
			break;
		case fieldType_ver15_e::animTime: // time
			result = 0;
			if (msg.ReadBool())
			{
				result = msg.ReadNumber<int>(field->getBits());
				*(float*)toF = EntityField::UnpackAnimTime(result);
			}
			else
			{
				// use delta time instead
				*(float*)toF = *(float*)fromF + timeDelta;
			}
			break;
		case fieldType_ver15_e::animWeight:
			result = msg.ReadNumber<int32_t>(field->getBits());
			*(float*)toF = EntityField::UnpackAnimWeight(result, field->getBits());
			break;
		case fieldType_ver15_e::scale:
			result = msg.ReadNumber<int32_t>(field->getBits());
			*(float*)toF = EntityField::UnpackScale(result);
			break;
		case fieldType_ver15_e::alpha:
			result = msg.ReadNumber<int32_t>(field->getBits());
			*(float*)toF = EntityField::UnpackAlpha(result, field->getBits());
			break;
		case fieldType_ver15_e::coord:
			coordOffset = EntityField::PackCoord(*(float*)fromF);
			coordVal = msgHelper.ReadDeltaCoord(coordOffset);
			*(float*)toF = EntityField::UnpackCoord(coordVal);
			break;
		case fieldType_ver15_e::coordExtra:
			coordOffset = EntityField::PackCoordExtra(*(float*)fromF);
			coordVal = msgHelper.ReadDeltaCoordExtra(coordOffset);
			*(float*)toF = EntityField::UnpackCoordExtra(coordVal);
			break;
		case fieldType_ver15_e::velocity:
			*(float*)toF = msgHelper.ReadCoordSmall();
			break;
		case fieldType_ver15_e::simple:
			EntityField::ReadSimple(msg, field->getBits(), toF, field->getSize());
			break;
		default:
			throw SerializableErrors::BadEntityFieldException(field->getType(), field->getName());
		}
	}

	// assign unchanged fields accordingly
	CopyFields<entityState_t>(fromEnt, GetState(), lc, numFields, entityStateFields_ver15);
}

SerializableEntityState_ver15::SerializableEntityState_ver15(entityState_t& inState, entityNum_t newNum, float timeDeltaValue)
	: SerializableEntityState(inState, newNum)
	, timeDelta(timeDeltaValue)
{

}
