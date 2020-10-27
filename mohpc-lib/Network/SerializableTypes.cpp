#include <MOHPC/Network/InfoTypes.h>
#include <MOHPC/Network/SerializableTypes.h>
#include <MOHPC/Misc/MSG/MSG.h>
#include <cstring>

using namespace MOHPC;

BadEntityNumberException::BadEntityNumberException(size_t inBadNumber)
	: badNumber(inBadNumber)
{}

size_t BadEntityNumberException::getNumber() const
{
	return badNumber;
}
str BadEntityNumberException::what() const
{
	return str(badNumber);
}

BadEntityFieldException::BadEntityFieldException(uint8_t inFieldType, const char* inFieldName)
	: fieldType(inFieldType)
	, fieldName(inFieldName)
{}

uint8_t BadEntityFieldException::getFieldType() const
{
	return fieldType;
}

const char* BadEntityFieldException::getFieldName() const
{
	return fieldName;
}

str BadEntityFieldException::what() const
{
	return str::printf("%s: %d", getFieldName(), getFieldType());
}

BadEntityFieldCountException::BadEntityFieldCountException(uint8_t inCount)
	: count(inCount)
{}

uint8_t BadEntityFieldCountException::getCount() const
{
	return count;
}
str BadEntityFieldCountException::what() const
{
	return str((int)getCount());
}

void MOHPC::SerializableUsercmd::LoadDelta(MSG& msg, const ISerializableMessage* from, intptr_t key)
{
	const usercmd_t* fromCmd = &((SerializableUsercmd*)from)->ucmd;

	const bool isByteTime = msg.ReadBool();
	if (isByteTime)
	{
		const uint8_t deltaTime = msg.ReadByte();
	}
	else {
		ucmd.serverTime = msg.ReadUInteger();
	}

	const bool hasChanges = msg.ReadBool();
	if (hasChanges)
	{
		key = (uint32_t)((uint32_t)key ^ ucmd.serverTime);
		ucmd.angles[0] = msg.ReadDeltaTypeKey(fromCmd->angles[0], key);
		ucmd.angles[1] = msg.ReadDeltaTypeKey(fromCmd->angles[1], key);
		ucmd.angles[2] = msg.ReadDeltaTypeKey(fromCmd->angles[2], key);
		ucmd.forwardmove = msg.ReadDeltaTypeKey(fromCmd->forwardmove, key);
		ucmd.rightmove = msg.ReadDeltaTypeKey(fromCmd->rightmove, key);
		ucmd.upmove = msg.ReadDeltaTypeKey(fromCmd->upmove, key);
		ucmd.buttons.flags = msg.ReadDeltaTypeKey(fromCmd->buttons.flags, key);
	}
}

void MOHPC::SerializableUsercmd::SaveDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) const
{
	const usercmd_t* fromCmd = &((SerializableUsercmd*)from)->ucmd;

	const uint32_t deltaTime = ucmd.serverTime - fromCmd->serverTime;
	const bool isByteTime = deltaTime < 256;

	msg.WriteBool(isByteTime);
	if (isByteTime) {
		msg.WriteByte(deltaTime);
	}
	else {
		msg.WriteUInteger(ucmd.serverTime);
	}

	const bool hasChanges =
		fromCmd->angles[0] != ucmd.angles[0] ||
		fromCmd->angles[1] != ucmd.angles[1] ||
		fromCmd->angles[2] != ucmd.angles[2] ||
		fromCmd->forwardmove != ucmd.forwardmove ||
		fromCmd->rightmove != ucmd.rightmove ||
		fromCmd->upmove != ucmd.upmove ||
		fromCmd->buttons.flags != ucmd.buttons.flags;

	msg.WriteBool(hasChanges);
	if (hasChanges)
	{
		const uint32_t keyTime = (uint32_t)key ^ ucmd.serverTime;
		msg.WriteDeltaTypeKey(fromCmd->angles[0], ucmd.angles[0], keyTime, 16);
		msg.WriteDeltaTypeKey(fromCmd->angles[1], ucmd.angles[1], keyTime, 16);
		msg.WriteDeltaTypeKey(fromCmd->angles[2], ucmd.angles[2], keyTime, 16);
		msg.WriteDeltaTypeKey(fromCmd->forwardmove, ucmd.forwardmove, keyTime, 8);
		msg.WriteDeltaTypeKey(fromCmd->rightmove, ucmd.rightmove, keyTime, 8);
		msg.WriteDeltaTypeKey(fromCmd->upmove, ucmd.upmove, keyTime, 8);
		msg.WriteDeltaTypeKey(fromCmd->buttons.flags, ucmd.buttons.flags, keyTime, 16);
	}
}

void SerializableUserEyes::LoadDelta(MSG& msg, const ISerializableMessage* from)
{
	const usereyes_t& fromEye = ((SerializableUserEyes*)from)->eyesInfo;

	const bool hasChanges = msg.ReadBool();
	if (hasChanges)
	{
		eyesInfo.ofs[0] = msg.ReadDeltaType(fromEye.ofs[0]);
		eyesInfo.ofs[1] = msg.ReadDeltaType(fromEye.ofs[1]);
		eyesInfo.ofs[2] = msg.ReadDeltaType(fromEye.ofs[2]);
		eyesInfo.angles[0] = msg.ReadDeltaType(fromEye.angles[0]);
		eyesInfo.angles[1] = msg.ReadDeltaType(fromEye.angles[1]);
	}
}

void SerializableUserEyes::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{
	const usereyes_t& fromEye = ((SerializableUserEyes*)from)->eyesInfo;

	const bool hasChanges =
		fromEye.angles[0] != eyesInfo.angles[0]
		|| fromEye.angles[1] != eyesInfo.angles[1]
		|| fromEye.ofs[0] != eyesInfo.ofs[0]
		|| fromEye.ofs[1] != eyesInfo.ofs[1]
		|| fromEye.ofs[2] != eyesInfo.ofs[2];

	msg.WriteBool(hasChanges);
	if (hasChanges)
	{
		msg.WriteDeltaType(fromEye.ofs[0], eyesInfo.ofs[0]);
		msg.WriteDeltaType(fromEye.ofs[1], eyesInfo.ofs[1]);
		msg.WriteDeltaType(fromEye.ofs[2], eyesInfo.ofs[2]);
		msg.WriteDeltaType(fromEye.angles[0], eyesInfo.angles[0]);
		msg.WriteDeltaType(fromEye.angles[1], eyesInfo.angles[1]);
	}
}

#define	PSF(x) #x,(uint16_t)(size_t)&((playerState_t*)0)->x,sizeof(playerState_t::x)
static constexpr intptr_t FLOAT_INT_BITS = 13;
static constexpr size_t FLOAT_INT_BIAS = (1 << (FLOAT_INT_BITS - 1));

const netField_template_t<fieldType_ver6_e> playerStateFields[] =
{
{ PSF(commandTime), 32, fieldType_ver6_e::number },
{ PSF(origin[0]), 0, fieldType_ver6_e::largeCoord },
{ PSF(origin[1]), 0, fieldType_ver6_e::largeCoord },
{ PSF(viewangles[1]), 0, fieldType_ver6_e::number },
{ PSF(velocity[1]), 0, fieldType_ver6_e::smallCoord },
{ PSF(velocity[0]), 0, fieldType_ver6_e::smallCoord },
{ PSF(viewangles[0]), 0, fieldType_ver6_e::number },
{ PSF(pm_time), -16, fieldType_ver6_e::number },
{ PSF(origin[2]), 0, fieldType_ver6_e::largeCoord },
{ PSF(velocity[2]), 0, fieldType_ver6_e::smallCoord },
{ PSF(iViewModelAnimChanged), 2, fieldType_ver6_e::number },
{ PSF(damage_angles[0]), -13, fieldType_ver6_e::angle },
{ PSF(damage_angles[1]), -13, fieldType_ver6_e::angle },
{ PSF(damage_angles[2]), -13, fieldType_ver6_e::angle },
{ PSF(speed), 16, fieldType_ver6_e::number },
{ PSF(delta_angles[1]), 16, fieldType_ver6_e::number },
{ PSF(viewheight), -8, fieldType_ver6_e::number },
{ PSF(groundEntityNum), GENTITYNUM_BITS, fieldType_ver6_e::number },
{ PSF(delta_angles[0]), 16, fieldType_ver6_e::number },
{ PSF(iViewModelAnim), 4, fieldType_ver6_e::number },
{ PSF(fov), 0, fieldType_ver6_e::number },
{ PSF(current_music_mood), 8, fieldType_ver6_e::number },
{ PSF(gravity), 16, fieldType_ver6_e::number },
{ PSF(fallback_music_mood), 8, fieldType_ver6_e::number },
{ PSF(music_volume), 0, fieldType_ver6_e::number },
{ PSF(pm_flags), 16, fieldType_ver6_e::number },
{ PSF(clientNum), 8, fieldType_ver6_e::number },
{ PSF(fLeanAngle), 0, fieldType_ver6_e::number },
{ PSF(blend[3]), 0, fieldType_ver6_e::number },
{ PSF(blend[0]), 0, fieldType_ver6_e::number },
{ PSF(pm_type), 8, fieldType_ver6_e::number },
{ PSF(feetfalling), 8, fieldType_ver6_e::number },
{ PSF(camera_angles[0]), 16, fieldType_ver6_e::angle },
{ PSF(camera_angles[1]), 16, fieldType_ver6_e::angle },
{ PSF(camera_angles[2]), 16, fieldType_ver6_e::angle },
{ PSF(camera_origin[0]), 0, fieldType_ver6_e::largeCoord },
{ PSF(camera_origin[1]), 0, fieldType_ver6_e::largeCoord },
{ PSF(camera_origin[2]), 0, fieldType_ver6_e::largeCoord },
{ PSF(camera_posofs[0]), 0, fieldType_ver6_e::largeCoord },
{ PSF(camera_posofs[2]), 0, fieldType_ver6_e::largeCoord },
{ PSF(camera_time), 0, fieldType_ver6_e::number },
{ PSF(bobCycle), 8, fieldType_ver6_e::number },
{ PSF(delta_angles[2]), 16, fieldType_ver6_e::number },
{ PSF(viewangles[2]), 0, fieldType_ver6_e::number },
{ PSF(music_volume_fade_time), 0, fieldType_ver6_e::number },
{ PSF(reverb_type), 6, fieldType_ver6_e::number },
{ PSF(reverb_level), 0, fieldType_ver6_e::number },
{ PSF(blend[1]), 0, fieldType_ver6_e::number },
{ PSF(blend[2]), 0, fieldType_ver6_e::number },
{ PSF(camera_offset[0]), 0, fieldType_ver6_e::number },
{ PSF(camera_offset[1]), 0, fieldType_ver6_e::number },
{ PSF(camera_offset[2]), 0, fieldType_ver6_e::number },
{ PSF(camera_posofs[1]), 0, fieldType_ver6_e::largeCoord },
{ PSF(camera_flags), 16, fieldType_ver6_e::number }
};

static_assert(sizeof(playerStateFields) == sizeof(netField_template_t<fieldType_ver6_e>) * 54);

const netField_template_t<fieldType_ver15_e> playerStateFields_ver15[] =
{
{ PSF(commandTime), 32, fieldType_ver15_e::number },
{ PSF(origin[0]), 0, fieldType_ver15_e::largeCoord },
{ PSF(origin[1]), 0, fieldType_ver15_e::largeCoord },
{ PSF(viewangles[1]), 0, fieldType_ver15_e::number },
{ PSF(velocity[1]), 0, fieldType_ver15_e::smallCoord },
{ PSF(velocity[0]), 0, fieldType_ver15_e::smallCoord },
{ PSF(viewangles[0]), 0, fieldType_ver15_e::number },
{ PSF(origin[2]), 0, fieldType_ver15_e::largeCoord },
{ PSF(velocity[2]), 0, fieldType_ver15_e::smallCoord },
{ PSF(iViewModelAnimChanged), 2, fieldType_ver15_e::number },
{ PSF(damage_angles[0]), -13, fieldType_ver15_e::angle },
{ PSF(damage_angles[1]), -13, fieldType_ver15_e::angle },
{ PSF(damage_angles[2]), -13, fieldType_ver15_e::angle },
{ PSF(speed), 16, fieldType_ver15_e::number },
{ PSF(delta_angles[1]), 16, fieldType_ver15_e::number },
{ PSF(viewheight), -8, fieldType_ver15_e::number },
{ PSF(groundEntityNum), GENTITYNUM_BITS, fieldType_ver15_e::number },
{ PSF(delta_angles[0]), 16, fieldType_ver15_e::number },
{ PSF(iViewModelAnim), 4, fieldType_ver15_e::number },
{ PSF(fov), 0, fieldType_ver15_e::number },
{ PSF(current_music_mood), 8, fieldType_ver15_e::number },
{ PSF(gravity), 16, fieldType_ver15_e::number },
{ PSF(fallback_music_mood), 8, fieldType_ver15_e::number },
{ PSF(music_volume), 0, fieldType_ver15_e::number },
{ PSF(pm_flags), 16, fieldType_ver15_e::number },
{ PSF(clientNum), 8, fieldType_ver15_e::number },
{ PSF(fLeanAngle), 0, fieldType_ver15_e::number },
{ PSF(blend[3]), 0, fieldType_ver15_e::number },
{ PSF(blend[0]), 0, fieldType_ver15_e::number },
{ PSF(pm_type), 8, fieldType_ver15_e::number },
{ PSF(feetfalling), 8, fieldType_ver15_e::number },
{ PSF(radarInfo), 26, fieldType_ver15_e::number },
{ PSF(camera_angles[0]), 16, fieldType_ver15_e::angle },
{ PSF(camera_angles[1]), 16, fieldType_ver15_e::angle },
{ PSF(camera_angles[2]), 16, fieldType_ver15_e::angle },
{ PSF(camera_origin[0]), 0, fieldType_ver15_e::largeCoord },
{ PSF(camera_origin[1]), 0, fieldType_ver15_e::largeCoord },
{ PSF(camera_origin[2]), 0, fieldType_ver15_e::largeCoord },
{ PSF(camera_posofs[0]), 0, fieldType_ver15_e::largeCoord },
{ PSF(camera_posofs[2]), 0, fieldType_ver15_e::largeCoord },
{ PSF(camera_time), 0, fieldType_ver15_e::number },
{ PSF(bVoted), 1, fieldType_ver15_e::number },
{ PSF(bobCycle), 8, fieldType_ver15_e::number },
{ PSF(delta_angles[2]), 16, fieldType_ver15_e::number },
{ PSF(viewangles[2]), 0, fieldType_ver15_e::number },
{ PSF(music_volume_fade_time), 0, fieldType_ver15_e::number },
{ PSF(reverb_type), 6, fieldType_ver15_e::number },
{ PSF(reverb_level), 0, fieldType_ver15_e::number },
{ PSF(blend[1]), 0, fieldType_ver15_e::number },
{ PSF(blend[2]), 0, fieldType_ver15_e::number },
{ PSF(camera_offset[0]), 0, fieldType_ver15_e::number },
{ PSF(camera_offset[1]), 0, fieldType_ver15_e::number },
{ PSF(camera_offset[2]), 0, fieldType_ver15_e::number },
{ PSF(camera_posofs[1]), 0, fieldType_ver15_e::largeCoord },
{ PSF(camera_flags), 16, fieldType_ver15_e::number }
};

static_assert(sizeof(playerStateFields_ver15) == sizeof(netField_template_t<fieldType_ver15_e>) * 55);

#define	NETF(x) #x,(uint16_t)(size_t)&((entityState_t*)0)->x,sizeof(entityState_t::x)

const netField_template_t<fieldType_ver6_e> entityStateFields[] =
{
{ NETF(netorigin[0]), 0, fieldType_ver6_e::largeCoord },
{ NETF(netorigin[1]), 0, fieldType_ver6_e::largeCoord },
{ NETF(netangles[1]), 12, fieldType_ver6_e::angle },
{ NETF(frameInfo[0].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[1].time), 0, fieldType_ver6_e::animTime },
{ NETF(bone_angles[0][0]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[3][0]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[1][0]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[2][0]), -13, fieldType_ver6_e::angle },
{ NETF(netorigin[2]), 0, fieldType_ver6_e::largeCoord },
{ NETF(frameInfo[0].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[1].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[2].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[3].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[0].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[1].index), 12, fieldType_ver6_e::number },
{ NETF(actionWeight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[2].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[3].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[2].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[3].index), 12, fieldType_ver6_e::number },
{ NETF(eType), 8, fieldType_ver6_e::number },
{ NETF(modelindex), 16, fieldType_ver6_e::number },
{ NETF(parent), 16, fieldType_ver6_e::number },
{ NETF(constantLight), 32, fieldType_ver6_e::number },
{ NETF(renderfx), 32, fieldType_ver6_e::number },
{ NETF(bone_tag[0]), -8, fieldType_ver6_e::number },
{ NETF(bone_tag[1]), -8, fieldType_ver6_e::number },
{ NETF(bone_tag[2]), -8, fieldType_ver6_e::number },
{ NETF(bone_tag[3]), -8, fieldType_ver6_e::number },
{ NETF(bone_tag[4]), -8, fieldType_ver6_e::number },
{ NETF(scale), 0, fieldType_ver6_e::scale },
{ NETF(alpha), 0, fieldType_ver6_e::alpha },
{ NETF(usageIndex), 16, fieldType_ver6_e::number },
{ NETF(eFlags), 16, fieldType_ver6_e::number },
{ NETF(solid), 32, fieldType_ver6_e::number },
{ NETF(netangles[2]), 12, fieldType_ver6_e::angle },
{ NETF(netangles[0]), 12, fieldType_ver6_e::angle },
{ NETF(tag_num), 10, fieldType_ver6_e::number },
{ NETF(bone_angles[1][2]), -13, fieldType_ver6_e::angle },
{ NETF(attach_use_angles), 1, fieldType_ver6_e::number },
{ NETF(origin2[1]), 0, fieldType_ver6_e::largeCoord },
{ NETF(origin2[0]), 0, fieldType_ver6_e::largeCoord },
{ NETF(origin2[2]), 0, fieldType_ver6_e::largeCoord },
{ NETF(bone_angles[0][2]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[2][2]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[3][2]), -13, fieldType_ver6_e::angle },
{ NETF(surfaces[0]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[1]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[2]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[3]), 8, fieldType_ver6_e::number },
{ NETF(bone_angles[0][1]), -13, fieldType_ver6_e::angle },
{ NETF(surfaces[4]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[5]), 8, fieldType_ver6_e::number },
{ NETF(pos.trTime), 32, fieldType_ver6_e::number },
//{ NETF(pos.trBase[0]), 0, fieldType_ver6_e::number },
//{ NETF(pos.trBase[1]), 0, fieldType_ver6_e::number },
{ NETF(pos.trDelta[0]), 0, fieldType_ver6_e::smallCoord },
{ NETF(pos.trDelta[1]), 0, fieldType_ver6_e::smallCoord },
//{ NETF(pos.trBase[2]), 0, fieldType_ver6_e::number },
//{ NETF(apos.trBase[1]), 0, fieldType_ver6_e::number },
{ NETF(pos.trDelta[2]), 0, fieldType_ver6_e::smallCoord },
//{ NETF(apos.trBase[0]), 0, fieldType_ver6_e::number },
{ NETF(loopSound), 16, fieldType_ver6_e::number },
{ NETF(loopSoundVolume), 0, fieldType_ver6_e::number },
{ NETF(loopSoundMinDist), 0, fieldType_ver6_e::number },
{ NETF(loopSoundMaxDist), 0, fieldType_ver6_e::number },
{ NETF(loopSoundPitch), 0, fieldType_ver6_e::number },
{ NETF(loopSoundFlags), 8, fieldType_ver6_e::number },
{ NETF(attach_offset[0]), 0, fieldType_ver6_e::number },
{ NETF(attach_offset[1]), 0, fieldType_ver6_e::number },
{ NETF(attach_offset[2]), 0, fieldType_ver6_e::number },
{ NETF(beam_entnum), 16, fieldType_ver6_e::number },
{ NETF(skinNum), 16, fieldType_ver6_e::number },
{ NETF(wasframe), 10, fieldType_ver6_e::number },
{ NETF(frameInfo[4].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[5].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[6].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[7].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[8].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[9].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[10].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[11].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[12].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[13].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[14].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[15].index), 12, fieldType_ver6_e::number },
{ NETF(frameInfo[4].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[5].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[6].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[7].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[8].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[9].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[10].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[11].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[12].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[13].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[14].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[15].time), 0, fieldType_ver6_e::animTime },
{ NETF(frameInfo[4].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[5].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[6].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[7].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[8].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[9].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[10].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[11].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[12].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[13].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[14].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(frameInfo[15].weight), 0, fieldType_ver6_e::animWeight },
{ NETF(bone_angles[1][1]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[2][1]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[3][1]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[4][0]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[4][1]), -13, fieldType_ver6_e::angle },
{ NETF(bone_angles[4][2]), -13, fieldType_ver6_e::angle },
{ NETF(clientNum), 8, fieldType_ver6_e::number },
{ NETF(groundEntityNum), GENTITYNUM_BITS, fieldType_ver6_e::number },
{ NETF(shader_data[0]), 0, fieldType_ver6_e::number },
{ NETF(shader_data[1]), 0, fieldType_ver6_e::number },
{ NETF(shader_time), 0, fieldType_ver6_e::number },
{ NETF(eyeVector[0]), 0, fieldType_ver6_e::number },
{ NETF(eyeVector[1]), 0, fieldType_ver6_e::number },
{ NETF(eyeVector[2]), 0, fieldType_ver6_e::number },
{ NETF(surfaces[6]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[7]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[8]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[9]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[10]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[11]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[12]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[13]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[14]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[15]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[16]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[17]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[18]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[19]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[20]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[21]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[22]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[23]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[24]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[25]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[26]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[27]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[28]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[29]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[30]), 8, fieldType_ver6_e::number },
{ NETF(surfaces[31]), 8, fieldType_ver6_e::number }
};

static_assert(sizeof(entityStateFields) == sizeof(netField_t) * 146);

// Fields for SH & BT
const netField_template_t<fieldType_ver15_e> entityStateFields_ver15[] =
{
{ NETF(netorigin[0]), 0, fieldType_ver15_e::mediumCoord },
{ NETF(netorigin[1]), 0, fieldType_ver15_e::mediumCoord },
{ NETF(netangles[1]), 12, fieldType_ver15_e::angle },
{ NETF(frameInfo[0].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[1].time), 15, fieldType_ver15_e::animTime },
{ NETF(bone_angles[0][0]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[3][0]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[1][0]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[2][0]), -13, fieldType_ver15_e::angle },
{ NETF(netorigin[2]), 0, fieldType_ver15_e::mediumCoord },
{ NETF(frameInfo[0].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[1].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[2].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[3].time), 15, fieldType_ver15_e::animTime },
{ NETF(frameInfo[0].index), 12, fieldType_ver15_e::number },
{ NETF(frameInfo[1].index), 12, fieldType_ver15_e::number },
{ NETF(actionWeight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[2].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[3].weight), 8, fieldType_ver15_e::animWeight },
{ NETF(frameInfo[2].index), 12, fieldType_ver15_e::number },
{ NETF(frameInfo[3].index), 12, fieldType_ver15_e::number },
{ NETF(eType), 8, fieldType_ver15_e::number },
{ NETF(modelindex), 16, fieldType_ver15_e::number },
{ NETF(parent), 16, fieldType_ver15_e::number },
{ NETF(constantLight), 32, fieldType_ver15_e::number },
{ NETF(renderfx), 32, fieldType_ver15_e::number },
{ NETF(bone_tag[0]), -8, fieldType_ver15_e::number },
{ NETF(bone_tag[1]), -8, fieldType_ver15_e::number },
{ NETF(bone_tag[2]), -8, fieldType_ver15_e::number },
{ NETF(bone_tag[3]), -8, fieldType_ver15_e::number },
{ NETF(bone_tag[4]), -8, fieldType_ver15_e::number },
{ NETF(scale), 10, fieldType_ver15_e::scale },
{ NETF(alpha), 8, fieldType_ver15_e::alpha },
{ NETF(usageIndex), 16, fieldType_ver15_e::number },
{ NETF(eFlags), 16, fieldType_ver15_e::number },
{ NETF(solid), 32, fieldType_ver15_e::number },
{ NETF(netangles[2]), 12, fieldType_ver15_e::angle },
{ NETF(netangles[0]), 12, fieldType_ver15_e::angle },
{ NETF(tag_num), 10, fieldType_ver15_e::number },
{ NETF(bone_angles[1][2]), -13, fieldType_ver15_e::angle },
{ NETF(attach_use_angles), 1, fieldType_ver15_e::number },
{ NETF(origin2[1]), 0, fieldType_ver15_e::mediumCoord },
{ NETF(origin2[0]), 0, fieldType_ver15_e::mediumCoord },
{ NETF(origin2[2]), 0, fieldType_ver15_e::mediumCoord },
{ NETF(bone_angles[0][2]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[2][2]), -13, fieldType_ver15_e::angle },
{ NETF(bone_angles[3][2]), -13, fieldType_ver15_e::angle },
{ NETF(surfaces[0]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[1]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[2]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[3]), 8, fieldType_ver15_e::number },
{ NETF(bone_angles[0][1]), -13, fieldType_ver15_e::angle },
{ NETF(surfaces[4]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[5]), 8, fieldType_ver15_e::number },
{ NETF(pos.trTime), 32, fieldType_ver15_e::number },
{ NETF(pos.trDelta[0]), 0, fieldType_ver15_e::smallCoord },
{ NETF(pos.trDelta[1]), 0, fieldType_ver15_e::smallCoord },
{ NETF(pos.trDelta[2]), 0, fieldType_ver15_e::smallCoord },
{ NETF(loopSound), 16, fieldType_ver15_e::number },
{ NETF(loopSoundVolume), 0, fieldType_ver15_e::number },
{ NETF(loopSoundMinDist), 0, fieldType_ver15_e::number },
{ NETF(loopSoundMaxDist), 0, fieldType_ver15_e::number },
{ NETF(loopSoundPitch), 0, fieldType_ver15_e::number },
{ NETF(loopSoundFlags), 8, fieldType_ver15_e::number },
{ NETF(attach_offset[0]), 0, fieldType_ver15_e::number },
{ NETF(attach_offset[1]), 0, fieldType_ver15_e::number },
{ NETF(attach_offset[2]), 0, fieldType_ver15_e::number },
{ NETF(beam_entnum), 16, fieldType_ver15_e::number },
{ NETF(skinNum), 16, fieldType_ver15_e::number },
{ NETF(wasframe), 10, fieldType_ver15_e::number },
{ NETF(frameInfo[4].index), 12, fieldType_ver15_e::number },
{ NETF(frameInfo[5].index), 12, fieldType_ver15_e::number },
{ NETF(frameInfo[6].index), 12, fieldType_ver15_e::number },
{ NETF(frameInfo[7].index), 12, fieldType_ver15_e::number },
{ NETF(frameInfo[8].index), 12, fieldType_ver15_e::number },
{ NETF(frameInfo[9].index), 12, fieldType_ver15_e::number },
{ NETF(frameInfo[10].index), 12, fieldType_ver15_e::number },
{ NETF(frameInfo[11].index), 12, fieldType_ver15_e::number },
{ NETF(frameInfo[12].index), 12, fieldType_ver15_e::number },
{ NETF(frameInfo[13].index), 12, fieldType_ver15_e::number },
{ NETF(frameInfo[14].index), 12, fieldType_ver15_e::number },
{ NETF(frameInfo[15].index), 12, fieldType_ver15_e::number },
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
{ NETF(clientNum), 8, fieldType_ver15_e::number },
{ NETF(groundEntityNum), GENTITYNUM_BITS, fieldType_ver15_e::number },
{ NETF(shader_data[0]), 0, fieldType_ver15_e::number },
{ NETF(shader_data[1]), 0, fieldType_ver15_e::number },
{ NETF(shader_time), 0, fieldType_ver15_e::number },
{ NETF(eyeVector[0]), 0, fieldType_ver15_e::number },
{ NETF(eyeVector[1]), 0, fieldType_ver15_e::number },
{ NETF(eyeVector[2]), 0, fieldType_ver15_e::number },
{ NETF(surfaces[6]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[7]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[8]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[9]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[10]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[11]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[12]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[13]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[14]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[15]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[16]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[17]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[18]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[19]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[20]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[21]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[22]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[23]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[24]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[25]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[26]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[27]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[28]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[29]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[30]), 8, fieldType_ver15_e::number },
{ NETF(surfaces[31]), 8, fieldType_ver15_e::number }
};

static_assert(sizeof(entityStateFields_ver15) == sizeof(netField_t) * 146);

void MOHPC::SerializablePlayerState::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{
	MsgTypesHelper msgHelper(msg);

	constexpr size_t numFields = sizeof(playerStateFields) / sizeof(playerStateFields[0]);

	playerState_t* fromPS = ((SerializablePlayerState*)from)->GetState();
	const netField_t* field;

	uint8_t lc = 0;
	uint8_t i;

	// Calculate the number of changes
	for (i = 0, field = playerStateFields; i < numFields; ++i, ++field)
	{
		const uint8_t* fromF = (uint8_t*)((const uint8_t*)fromPS + field->offset);
		uint8_t* toF = (uint8_t*)((uint8_t*)GetState() + field->offset);

		if (memcmp(fromF, toF, field->size)) {
			lc = i + 1;
		}
	}

	// Serialize the number of changes
	msg.WriteByte(lc);

	for (i = 0, field = playerStateFields; i < lc; ++i, ++field)
	{
		const uint8_t* fromF = (uint8_t*)((uint8_t*)fromPS + field->offset);
		uint8_t* toF = (uint8_t*)((uint8_t*)GetState() + field->offset);
		bool hasChange = memcmp(fromF, toF, field->size);

		msg.SerializeBool(hasChange);
		if (!hasChange) {
			continue;
		}

		switch (fieldType_ver6_e(field->type))
		{
		case fieldType_ver6_e::number:
			EntityField::WriteNumberPlayerStateField(msg, field->bits, toF);
			break;
		case fieldType_ver6_e::angle:
			EntityField::WriteAngleField(msg, field->bits, *(float*)toF);
			break;
		case fieldType_ver6_e::largeCoord:
			msgHelper.WriteCoord(*(float*)toF);
			break;
		case fieldType_ver6_e::smallCoord:
			msgHelper.WriteCoordSmall(*(float*)toF);
			break;
		default:
			break;
		}
	}

	uint32_t statsBits = 0;
	uint32_t activeItemsBits = 0;
	uint32_t ammoAmountBits = 0;
	uint32_t ammoBits = 0;
	uint32_t maxAmmoAmountBits = 0;

	//
	// Serialize arrays
	//
	for (i = 0; i < playerState_t::MAX_STATS; ++i)
	{
		if (state.stats[i] != fromPS->stats[i]) {
			statsBits |= 1 << i;
		}
	}

	for (i = 0; i < playerState_t::MAX_ACTIVEITEMS; ++i)
	{
		if (state.activeItems[i] != fromPS->activeItems[i]) {
			activeItemsBits |= 1 << i;
		}
	}

	for (i = 0; i < playerState_t::MAX_AMMO_AMOUNT; ++i)
	{
		if (state.ammo_amount[i] != fromPS->ammo_amount[i]) {
			ammoAmountBits |= 1 << i;
		}
	}

	for (i = 0; i < playerState_t::MAX_AMMO; ++i)
	{
		if (state.ammo_name_index[i] != fromPS->ammo_name_index[i]) {
			ammoBits |= 1 << i;
		}
	}

	for (i = 0; i < playerState_t::MAX_MAX_AMMO_AMOUNT; ++i)
	{
		if (state.max_ammo_amount[i] != fromPS->max_ammo_amount[i]) {
			maxAmmoAmountBits |= 1 << i;
		}
	}

	const bool hasStatsChanges = statsBits || activeItemsBits || ammoBits || ammoAmountBits || maxAmmoAmountBits;

	msg.WriteBool(hasStatsChanges);
	if (!hasStatsChanges) {
		return;
	}

	// Serialize stats
	bool hasStatsChanged = statsBits ? true : false;
	msg.WriteBool(hasStatsChanged);
	if (hasStatsChanged)
	{
		msg.WriteUInteger(statsBits);
		for (i = 0; i < playerState_t::MAX_STATS; ++i)
		{
			if (statsBits & (1 << i)) {
				msg.WriteUShort(state.stats[i]);
			}
		}
	}

	// Serialize active items
	bool hasActiveItemsChanged = activeItemsBits ? true : false;
	msg.WriteBool(hasActiveItemsChanged);
	if (hasActiveItemsChanged)
	{
		msg.WriteByte(activeItemsBits);
		for (i = 0; i < playerState_t::MAX_ACTIVEITEMS; ++i)
		{
			if (activeItemsBits & (1 << i)) {
				msg.WriteUShort(state.activeItems[i]);
			}
		}
	}

	// Serialize ammo amount
	bool hasAmmoAmountChanges = ammoAmountBits ? true : false;
	msg.WriteBool(hasAmmoAmountChanges);
	if (hasAmmoAmountChanges)
	{
		msg.WriteUShort(ammoAmountBits);
		for (i = 0; i < playerState_t::MAX_AMMO_AMOUNT; ++i)
		{
			if (ammoAmountBits & (1 << i)) {
				msg.WriteUShort(state.ammo_amount[i]);
			}
		}
	}

	// Serialize ammo
	bool hasAmmoBitsChanges = ammoBits ? true : false;
	msg.WriteBool(hasAmmoBitsChanges);
	if (hasAmmoBitsChanges)
	{
		msg.WriteUShort(ammoBits);
		for (i = 0; i < playerState_t::MAX_AMMO; ++i)
		{
			if (ammoBits & (1 << i)) {
				msg.WriteUShort(state.ammo_name_index[i]);
			}
		}
	}

	// Serialize max ammo
	bool hasMaxAmmoAmountChanges = maxAmmoAmountBits ? true : false;
	msg.WriteBool(hasMaxAmmoAmountChanges);
	if (hasMaxAmmoAmountChanges)
	{
		msg.WriteUShort(maxAmmoAmountBits);
		for (i = 0; i < playerState_t::MAX_MAX_AMMO_AMOUNT; ++i)
		{
			if (maxAmmoAmountBits & (1 << i)) {
				msg.WriteUShort(state.max_ammo_amount[i]);
			}
		}
	}
}

void MOHPC::SerializablePlayerState::LoadDelta(MSG& msg, const ISerializableMessage* from)
{
	MsgTypesHelper msgHelper(msg);

	constexpr size_t numFields = sizeof(playerStateFields) / sizeof(playerStateFields[0]);

	static const playerState_t nullstate;

	const SerializablePlayerState* srFrom = (const SerializablePlayerState*)from;
	const playerState_t* fromPS = srFrom ? srFrom->GetState() : &nullstate;
	if (fromPS)
	{
		// copy initial values
		state = *fromPS;
	}

	// Serialize the number of changes
	const uint8_t lc = msg.ReadByte();

	size_t i;
	const netField_t* field;
	for (i = 0, field = playerStateFields; i < lc; ++i, ++field)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->offset);
		uint8_t* toF = (uint8_t*)((uint8_t*)GetState() + field->offset);
		bool hasChange;

		hasChange = msg.ReadBool();
		if (!hasChange)
		{
			// no change
			std::memcpy(toF, fromF, field->size);
			continue;
		}

		switch (fieldType_ver6_e(field->type))
		{
		case fieldType_ver6_e::number:
			EntityField::ReadNumberPlayerStateField(msg, field->bits, toF, field->size);
			break;
		case fieldType_ver6_e::angle:
			*(float*)toF = EntityField::ReadAngleField(msg, field->bits);
			break;
		case fieldType_ver6_e::largeCoord:
			*(float*)toF = msgHelper.ReadCoord();
			break;
		case fieldType_ver6_e::smallCoord:
			*(float*)toF = msgHelper.ReadCoordSmall();
			break;
		default:
			break;
		}
	}

	if (fromPS)
	{
		// assign unchanged fields accordingly
		for (i = lc, field = &playerStateFields[lc]; i < numFields; i++, field++)
		{
			const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->offset);
			uint8_t* toF = (uint8_t*)((uint8_t*)GetState() + field->offset);

			// no change
			std::memcpy(toF, fromF, field->size);
		}
	}

	uint32_t statsBits = 0;
	uint32_t activeItemsBits = 0;
	uint32_t ammoAmountBits = 0;
	uint32_t ammoBits = 0;
	uint32_t maxAmmoAmountBits = 0;

	const bool hasStatsChanges = msg.ReadBool();
	if (!hasStatsChanges) {
		return;
	}

	// Serialize stats
	const bool hasStatsChanged = msg.ReadBool();
	if (hasStatsChanged)
	{
		statsBits = msg.ReadUInteger();
		for (i = 0; i < playerState_t::MAX_STATS; ++i)
		{
			if (statsBits & (1 << i)) {
				state.stats[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize active items
	const bool hasActiveItemsChanged = msg.ReadBool();
	if (hasActiveItemsChanged)
	{
		activeItemsBits = msg.ReadByte();
		for (i = 0; i < playerState_t::MAX_ACTIVEITEMS; ++i)
		{
			if (activeItemsBits & (1 << i)) {
				state.activeItems[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize ammo amount
	const bool hasAmmoAmountChanges = msg.ReadBool();
	if (hasAmmoAmountChanges)
	{
		ammoAmountBits = msg.ReadUShort();
		for (i = 0; i < playerState_t::MAX_AMMO_AMOUNT; ++i)
		{
			if (ammoAmountBits & (1 << i)) {
				state.ammo_amount[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize ammo
	const bool hasAmmoBitsChanges = msg.ReadBool();
	if (hasAmmoBitsChanges)
	{
		ammoBits = msg.ReadUShort();
		for (i = 0; i < playerState_t::MAX_AMMO; ++i)
		{
			if (ammoBits & (1 << i)) {
				state.ammo_name_index[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize max ammo
	const bool hasMaxAmmoAmountChanges = msg.ReadBool();
	if (hasMaxAmmoAmountChanges)
	{
		maxAmmoAmountBits = msg.ReadUShort();
		for (i = 0; i < playerState_t::MAX_MAX_AMMO_AMOUNT; ++i)
		{
			if (maxAmmoAmountBits & (1 << i)) {
				state.max_ammo_amount[i] = msg.ReadUShort();
			}
		}
	}
}

void MOHPC::SerializablePlayerState_ver15::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{

}

void MOHPC::SerializablePlayerState_ver15::LoadDelta(MSG& msg, const ISerializableMessage* from)
{
	MsgTypesHelper msgHelper(msg);

	constexpr size_t numFields = sizeof(playerStateFields_ver15) / sizeof(playerStateFields_ver15[0]);

	static playerState_t nullstate;

	const SerializablePlayerState* srFrom = (const SerializablePlayerState*)from;
	const playerState_t* fromPS = srFrom ? srFrom->GetState() : &nullstate;
	if (fromPS)
	{
		// copy initial values
		state = *fromPS;
	}

	// Serialize the number of changes
	const uint8_t lc = msg.ReadByte();
	if (lc > numFields) {
		throw BadEntityFieldCountException(lc);
	}
	
	size_t i;
	const netField_t* field;
	for (i = 0, field = playerStateFields_ver15; i < lc; ++i, ++field)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->offset);
		uint8_t* toF = (uint8_t*)((uint8_t*)GetState() + field->offset);

		const bool hasChange = msg.ReadBool();
		if (!hasChange)
		{
			// no change
			std::memcpy(toF, fromF, field->size);
			continue;
		}

		int result;
		intptr_t bits = 0;
		int32_t coordOffset, coordVal;

		switch (fieldType_ver15_e(field->type))
		{
		case fieldType_ver15_e::number:
			EntityField::ReadRegular2(msg, field->bits, toF, field->size);
			break;
		case fieldType_ver15_e::angle: // anglestmp = 1.0f;
			result = 0;
			msg.ReadBits(&result, field->bits < 0 ? -field->bits : field->bits);
			*(float*)toF = EntityField::UnpackAngle(result, field->bits, field->bits < 0);
			break;
		case fieldType_ver15_e::mediumCoord: // changed in SH/BT
			coordOffset = EntityField::PackCoord(*(float*)fromF);
			coordVal = msgHelper.ReadDeltaCoord(coordOffset);
			*(float*)toF = EntityField::UnpackCoord(coordVal);
			break;
		case fieldType_ver15_e::largeCoord: // changed in SH/BT
			coordOffset = EntityField::PackCoordExtra(*(float*)fromF);
			coordVal = msgHelper.ReadDeltaCoordExtra(coordOffset);
			*(float*)toF = EntityField::UnpackCoordExtra(coordVal);
			break;
		case fieldType_ver15_e::smallCoord: // New in SH/BT
			*(float*)toF = msgHelper.ReadCoordSmall();
			break;
		default:
			break;
		}
	}

	if (fromPS)
	{
		// assign unchanged fields accordingly
		for (i = lc, field = &playerStateFields_ver15[lc]; i < numFields; i++, field++)
		{
			const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->offset);
			uint8_t* toF = (uint8_t*)((uint8_t*)GetState() + field->offset);

			// no change
			std::memcpy(toF, fromF, field->size);
		}
	}

	uint32_t statsBits = 0;
	uint32_t activeItemsBits = 0;
	uint32_t ammoAmountBits = 0;
	uint32_t ammoBits = 0;
	uint32_t maxAmmoAmountBits = 0;

	const bool hasStatsChanges = msg.ReadBool();
	if (!hasStatsChanges) {
		return;
	}

	// Serialize stats
	const bool hasStatsChanged = msg.ReadBool();
	if (hasStatsChanged)
	{
		statsBits = msg.ReadUInteger();
		for (i = 0; i < playerState_t::MAX_STATS; ++i)
		{
			if (statsBits & (1 << i)) {
				state.stats[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize active items
	const bool hasActiveItemsChanged = msg.ReadBool();
	if (hasActiveItemsChanged)
	{
		activeItemsBits = msg.ReadByte();
		for (i = 0; i < playerState_t::MAX_ACTIVEITEMS; ++i)
		{
			if (activeItemsBits & (1 << i)) {
				state.activeItems[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize ammo amount
	const bool hasAmmoAmountChanges = msg.ReadBool();
	if (hasAmmoAmountChanges)
	{
		ammoAmountBits = msg.ReadUShort();
		for (i = 0; i < playerState_t::MAX_AMMO_AMOUNT; ++i)
		{
			if (ammoAmountBits & (1 << i)) {
				state.ammo_amount[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize ammo
	const bool hasAmmoBitsChanges = msg.ReadBool();
	if (hasAmmoBitsChanges)
	{
		ammoBits = msg.ReadUShort();
		for (i = 0; i < playerState_t::MAX_AMMO; ++i)
		{
			if (ammoBits & (1 << i)) {
				state.ammo_name_index[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize max ammo
	const bool hasMaxAmmoAmountChanges = msg.ReadBool();
	if (hasMaxAmmoAmountChanges)
	{
		maxAmmoAmountBits = msg.ReadUShort();
		for (i = 0; i < playerState_t::MAX_MAX_AMMO_AMOUNT; ++i)
		{
			if (maxAmmoAmountBits & (1 << i)) {
				state.max_ammo_amount[i] = msg.ReadUShort();
			}
		}
	}
}

void MOHPC::SerializableEntityState::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{
	MsgTypesHelper msgHelper(msg);
	constexpr size_t numFields = sizeof(entityStateFields) / sizeof(entityStateFields[0]);

	uint8_t lc = 0;
	uint8_t i = 0;
	const netField_t* field;

	static const entityState_t nullstate;
	const entityState_t* fromEnt = from ? ((const SerializableEntityState*)from)->GetState() : &nullstate;

	// build the change vector as bytes so it is endien independent
	for (i = 0, field = entityStateFields; i < numFields; i++, field++)
	{
		uint8_t* fromF = (uint8_t*)((uint8_t*)fromEnt + field->offset);
		uint8_t* toF = (uint8_t*)((uint8_t*)&state + field->offset);
		if (memcmp(fromF, toF, field->size)) {
			lc = i + 1;
		}
	}

	const bool removed = false;
	msg.WriteBool(removed);
	if (removed)
	{
		state = entityState_t();
		state.number = ENTITYNUM_NONE;
		return;
	}

	const bool hasDelta = lc > 0;
	msg.WriteBool(hasDelta);

	if (!hasDelta)
	{
		state = *(entityState_t*)from;
		return;
	}

	// # of changes
	msg.WriteByte(lc);
	if (lc > numFields) {
		throw BadEntityFieldCountException(lc);
	}

	for (i = 0, field = entityStateFields; i < lc; i++, field++)
	{
		const uint8_t* fromF = (uint8_t*)((uint8_t*)fromEnt + field->offset);
		uint8_t* toF = (uint8_t*)((uint8_t*)&state + field->offset);

		bool isDiff = memcmp(fromF, toF, field->size);
		msg.WriteBool(isDiff);
		if (!isDiff) {
			continue;
		}

		float tmp;
		intptr_t bits;

		switch (fieldType_ver6_e(field->type))
		{
		case fieldType_ver6_e::number:
			EntityField::WriteNumberEntityField(msg, field->bits, toF, field->size);
			break;
		case fieldType_ver6_e::angle:
			EntityField::WriteAngleField(msg, field->bits, *(float*)toF);
			break;
		case fieldType_ver6_e::animTime:
			EntityField::WriteTimeField(msg, *(float*)toF);
			break;
		case fieldType_ver6_e::animWeight: // nasty!
			tmp = *(float*)toF;

			bits = intptr_t((tmp * 255.0f) + 0.5f);
			if (bits < 0) bits = 0;
			else if (bits > 255) bits = 255;
			msg.WriteBits(&bits, 8);
			break;
		case fieldType_ver6_e::scale:
			EntityField::WriteSmallTimeField(msg, *(float*)toF);
			break;
		case fieldType_ver6_e::alpha:
			tmp = *(float*)toF;

			bits = intptr_t((tmp * 255.0f) + 0.5f);
			if (bits < 0) bits = 0;
			else if (bits > 255) bits = 255;
			msg.WriteBits(&bits, 8);
			break;
		case fieldType_ver6_e::largeCoord:
			msgHelper.WriteCoord(*(float*)toF);
			break;
		case fieldType_ver6_e::smallCoord:
			msgHelper.WriteCoordSmall(*(float*)toF);
			break;
		default:
			throw BadEntityFieldException(field->type, field->name);
		}
	}
}

void MOHPC::SerializableEntityState::LoadDelta(MSG& msg, const ISerializableMessage* from)
{
	MsgTypesHelper msgHelper(msg);

	entityState_t* fromEnt = ((SerializableEntityState*)from)->GetState();

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
		throw BadEntityFieldCountException(lc);
	}

	size_t i;
	const netField_t* field;

	for (i = 0, field = entityStateFields; i < lc; i++, field++)
	{
		const uint8_t* fromF = (uint8_t*)((uint8_t*)fromEnt + field->offset);
		uint8_t* toF = (uint8_t*)((uint8_t*)&state + field->offset);

		const bool isDiff = msg.ReadBool();
		if (!isDiff)
		{
			// no changes
			memcpy(toF, fromF, field->size);
			continue;
		}

		int result;

		switch (fieldType_ver6_e(field->type))
		{
		case fieldType_ver6_e::number:
			EntityField::ReadRegular(msg, field->bits, toF, field->size);
			break;
		case fieldType_ver6_e::angle:
			*(float*)toF = EntityField::ReadAngleField(msg, field->bits);
			break;
		case fieldType_ver6_e::animTime:
			*(float*)toF = EntityField::ReadTimeField(msg, field->bits);
			break;
		case fieldType_ver6_e::animWeight:
			result = 0;
			msg.ReadBits(&result, 8);
			*(float*)toF = EntityField::UnpackAnimWeight(result, 8);
			break;
		case fieldType_ver6_e::scale:
			*(float*)toF = EntityField::ReadSmallTimeField(msg, field->bits);
			break;
		case fieldType_ver6_e::alpha:
			result = 0;
			msg.ReadBits(&result, 8);
			*(float*)toF = EntityField::UnpackAnimWeight(result, 8);
			break;
		case fieldType_ver6_e::largeCoord:
			*(float*)toF = msgHelper.ReadCoord();
			break;
		case fieldType_ver6_e::smallCoord:
			*(float*)toF = msgHelper.ReadCoordSmall();
			break;
		default:
			throw BadEntityFieldException(field->type, field->name);
		}
	}

	// assign unchanged fields accordingly
	EntityField::CopyFields(fromEnt, GetState(), lc, numFields, entityStateFields);

	// FIXME: not sure if origin, angles and bone_angles should be set
}

void MOHPC::SerializableEntityState_ver15::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{

}

void MOHPC::SerializableEntityState_ver15::LoadDelta(MSG& msg, const ISerializableMessage* from)
{
	MsgTypesHelper msgHelper(msg);

	static entityState_t nullstate;
	entityState_t* fromEnt = from ? ((SerializableEntityState*)from)->GetState() : &nullstate;

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
		throw BadEntityFieldCountException(lc);
	}

	size_t i;
	const netField_t* field;

	for (i = 0, field = entityStateFields_ver15; i < lc; i++, field++)
	{
		uint8_t* fromF = (uint8_t*)((uint8_t*)fromEnt + field->offset);
		uint8_t* toF = (uint8_t*)((uint8_t*)&state + field->offset);

		const bool hasChange = msg.ReadBool();
		if (!hasChange)
		{
			// no change
			std::memcpy(toF, fromF, field->size);
			continue;
		}

		int result;
		int32_t coordOffset, coordVal;

		switch (fieldType_ver15_e(field->type))
		{
		case fieldType_ver15_e::number:
			EntityField::ReadRegular2(msg, field->bits, toF, field->size);
			break;
		case fieldType_ver15_e::angle:
			result = 0;
			msg.ReadBits(&result, field->bits < 0 ? -field->bits : field->bits);
			*(float*)toF = EntityField::UnpackAngle(result, field->bits, field->bits < 0);
			break;
		case fieldType_ver15_e::animTime: // time
			result = 0;
			if (msg.ReadBool())
			{
				msg.ReadBits(&result, field->bits);
				*(float*)toF = EntityField::UnpackAnimTime(result);
			}
			else {
				// FIXME
				//*(float*)toF += timeInc
			}
			break;
		case fieldType_ver15_e::animWeight:
			result = 0;
			msg.ReadBits(&result, field->bits);
			*(float*)toF = EntityField::UnpackAnimWeight(result, field->bits);
			break;
		case fieldType_ver15_e::scale:
			result = 0;
			msg.ReadBits(&result, field->bits);
			*(float*)toF = EntityField::UnpackScale(result);
			break;
		case fieldType_ver15_e::alpha:
			result = 0;
			msg.ReadBits(&result, field->bits);
			*(float*)toF = EntityField::UnpackAlpha(result, field->bits);
			break;
		case fieldType_ver15_e::mediumCoord: // changed in SH/BT
			coordOffset = EntityField::PackCoord(*(float*)fromF);
			coordVal = msgHelper.ReadDeltaCoord(coordOffset);
			*(float*)toF = EntityField::UnpackCoord(coordVal);
			break;
		case fieldType_ver15_e::largeCoord: // changed in SH/BT
			coordOffset = EntityField::PackCoordExtra(*(float*)fromF);
			coordVal = msgHelper.ReadDeltaCoordExtra(coordOffset);
			*(float*)toF = EntityField::UnpackCoordExtra(coordVal);
			break;
		case fieldType_ver15_e::smallCoord: // New in SH/BT
			*(float*)toF = msgHelper.ReadCoordSmall();
			break;
		default:
			throw BadEntityFieldException(field->type, field->name);
		}
	}

	// assign unchanged fields accordingly
	EntityField::CopyFields(fromEnt, GetState(), lc, numFields, entityStateFields_ver15);
}

void MOHPC::EntityField::ReadNumberPlayerStateField(MSG& msg, size_t bits, void* toF, size_t size)
{
	if (bits == 0)
	{
		// float
		bool isFullFloat;
		msg.SerializeBool(isFullFloat);
		if (!isFullFloat)
		{
			// integral float
			int32_t truncFloat = 0;
			msg.SerializeBits(&truncFloat, FLOAT_INT_BITS);
			// bias to allow equal parts positive and negative
			truncFloat -= FLOAT_INT_BIAS;
			*(float*)toF = (float)truncFloat;
		}
		else
		{
			// full floating point value
			msg.SerializeFloat(*(float*)toF);
		}
	}
	else
	{
		// integer
		std::memset(toF, 0, size);
		msg.SerializeBits(toF, bits);
	}
}

void MOHPC::EntityField::WriteNumberPlayerStateField(MSG& msg, size_t bits, void* toF)
{
	if (bits == 0)
	{
		float floatVal = *(float*)toF;
		int truncFloat = (int)floatVal;

		bool isFullFloat =
			truncFloat == floatVal && truncFloat + FLOAT_INT_BIAS >= 0 &&
			truncFloat + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS);

		msg.WriteBool(isFullFloat);

		if (!isFullFloat)
		{
			int truncated = truncFloat + FLOAT_INT_BIAS;
			msg.WriteBits(&truncated, FLOAT_INT_BITS);
		}
		else
		{
			// Send a full float value
			msg.WriteFloat(floatVal);
		}
	}
	else
	{
		// Integer
		msg.WriteBits(toF, bits);
	}
}

void MOHPC::EntityField::ReadRegular(MSG& msg, intptr_t bits, void* toF, size_t size)
{
	if (bits == 0)
	{
		// float
		bool hasValue = msg.ReadBool();

		if (!hasValue) {
			*(float*)toF = 0.0f;
		}
		else
		{
			const bool isFullFloat = msg.ReadBool();
			if (!isFullFloat)
			{
				// integral float
				int32_t truncFloat = 0;
				msg.ReadBits(&truncFloat, FLOAT_INT_BITS);
				// bias to allow equal parts positive and negative
				truncFloat -= FLOAT_INT_BIAS;
				*(float*)toF = (float)truncFloat;
			}
			else
			{
				// full floating point value
				*(float*)toF = msg.ReadFloat();
			}
		}
	}
	else
	{
		memset(toF, 0, size);

		const bool hasValue = msg.ReadBool();
		if (hasValue)
		{
			// integer
			msg.ReadBits(toF, bits);
		}
	}
}

template<typename T>
void shiftType(T& val)
{
	if (val & 1) {
		val = ~(val >> 1);
	}
	else {
		val = val >> 1;
	}
}

void MOHPC::EntityField::ReadRegular2(MSG& msg, intptr_t bits, void* toF, size_t size)
{
	if (bits == 0)
	{
		// float
		bool hasValue = msg.ReadBool();

		if (!hasValue) {
			*(float*)toF = 0.0f;
		}
		else
		{
			const bool isFullFloat = msg.ReadBool();
			if (!isFullFloat)
			{
				// integral float
				int32_t truncFloat = 0;
				msg.ReadBits(&truncFloat, -FLOAT_INT_BITS);
				shiftType(truncFloat);

				*(float*)toF = (float)truncFloat;
			}
			else
			{
				// full floating point value
				*(float*)toF = msg.ReadFloat();
			}
		}
	}
	else
	{
		memset(toF, 0, size);

		const bool hasValue = msg.ReadBool();
		if (hasValue)
		{
			// integer
			msg.ReadBits(toF, bits);
		}

		if (bits < 0)
		{
			size_t nbits = -bits;

			if (size == 1)
			{
				shiftType(*(uint8_t*)toF);
			}
			else if (size == 2)
			{
				shiftType(*(uint16_t*)toF);
			}
			else if (size == 4)
			{
				shiftType(*(uint32_t*)toF);
			}
			else
			{
				shiftType(*(uint32_t*)toF);
			}
		}
	}
}

void MOHPC::EntityField::WriteNumberEntityField(MSG& msg, size_t bits, void* toF, size_t size)
{
	if (bits == 0)
	{
		// float
		float floatVal = *(float*)toF;
		int32_t truncFloat = (int)floatVal;

		const bool hasValue = floatVal != 0.f;
		msg.WriteBool(hasValue);

		if (hasValue)
		{
			bool isFullFloat = truncFloat == floatVal && truncFloat + FLOAT_INT_BIAS >= 0 &&
				truncFloat + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS);

			msg.WriteBool(isFullFloat);

			if (!isFullFloat) {
				// send as small integer
				int newVal = truncFloat + FLOAT_INT_BIAS;
				msg.WriteBits(&newVal, FLOAT_INT_BITS);
			}
			else
			{
				// send as full floating point value
				msg.WriteFloat(*(float*)toF);
			}
		}
	}
	else
	{
		bool hasValue = memcmp(toF, "", size);
		msg.WriteBool(hasValue);

		if (hasValue)
		{
			// integer
			msg.WriteBits(toF, bits);
		}
	}
}

float MOHPC::EntityField::ReadAngleField(MSG& msg, size_t bits)
{
	bool isNeg;

	if (bits < 0)
	{
		isNeg = msg.ReadBool();
		bits = ~bits;
	}
	else
	{
		isNeg = false;
		bits = bits;
	}

	int result = 0;
	msg.SerializeBits(&result, bits);
	return EntityField::UnpackAngle(result, bits, isNeg);
}

void MOHPC::EntityField::WriteAngleField(MSG& msg, size_t bits, float angle)
{
	float tmp = angle;
	if (bits < 0)
	{
		bool isNeg = tmp < 0.f;
		msg.WriteBool(isNeg);
		tmp = -tmp;
		bits = ~bits;
	}
	else {
		bits = bits;
	}

	if (bits == 12) {
		tmp = tmp * 4096.0f / 360.0f;
		int32_t truncFloat = (int)tmp & 4095;
		msg.WriteBits(&truncFloat, 12);
	}
	else if (bits == 8) {
		tmp = tmp * 256.0f / 360.0f;
		int32_t truncFloat = (int)tmp & 255;
		msg.WriteBits(&truncFloat, 8);
	}
	else if (bits == 16) {
		tmp = tmp * 65536.0f / 360.0f;
		int32_t truncFloat = (int)tmp & 65535;
		msg.WriteBits(&truncFloat, 16);
	}
	else {
		tmp = tmp * (1 << (uint8_t)bits) / 360.0f;
		int32_t truncFloat = ((int)tmp) & ((1 << (uint8_t)bits) - 1);
		msg.WriteBits(&truncFloat, bits);
	}
}

float MOHPC::EntityField::ReadTimeField(MSG& msg, size_t bits)
{
	int result = 0;
	msg.SerializeBits(&result, 15);
	return EntityField::UnpackAnimTime(result);
}

void MOHPC::EntityField::WriteTimeField(MSG& msg, float time)
{
	float tmp = time;
	intptr_t value = intptr_t(tmp * 100.0f);
	if (value < 0) {
		value = 0;
	}
	else if (value > 32767) {
		value = 32767;
	}

	msg.WriteBits(&value, 15);
}

float MOHPC::EntityField::ReadSmallTimeField(MSG& msg, size_t bits)
{
	int result = 0;
	msg.SerializeBits(&result, 10);
	return EntityField::UnpackAnimTime(result);
}

void MOHPC::EntityField::WriteSmallTimeField(MSG& msg, float time)
{
	float tmp = time;
	intptr_t value = intptr_t(tmp * 100.0f);
	if (value < 0) value = 0;
	else if (value > 1023) value = 1023;

	msg.WriteBits(&value, 10);
}

void MOHPC::EntityField::CopyFields(entityState_t* other, entityState_t* target, size_t from, size_t to, const netField_t* fieldlist)
{
	size_t i;
	const netField_t* field;
	for (i = from, field = &fieldlist[from]; i < to; i++, field++)
	{
		const uint8_t* fromF = (uint8_t*)((uint8_t*)other + field->offset);
		uint8_t* toF = (uint8_t*)((uint8_t*)target + field->offset);

		// no change
		std::memcpy(toF, fromF, field->size);
	}
}

int32_t MOHPC::EntityField::PackCoord(float val)
{
	return (int32_t)(val * 4.f + 32768.f);
}

int32_t MOHPC::EntityField::PackCoordExtra(float val)
{
	return (int32_t)(val * 16.f + 131072.f);
}

float MOHPC::EntityField::UnpackCoord(int32_t val)
{
	return (float)(val - 32768) / 4.f;
}

float MOHPC::EntityField::UnpackCoordExtra(int32_t val)
{
	return (float)(val - 131072) / 16.f;
}

float MOHPC::EntityField::UnpackAnimWeight(int result, intptr_t bits)
{
	const float tmp = (float)result / (float)((1 << bits) - 1);

	if (tmp < 0.0f) return 0.f;
	else if (tmp > 1.0f) return 1.f;
	else return tmp;
}

float MOHPC::EntityField::UnpackScale(int result)
{
	return result / 100.f;
}

float MOHPC::EntityField::UnpackAlpha(int result, intptr_t bits)
{
	const float tmp = (float)result / (float)((1 << bits) - 1);

	if (tmp < 0.0f) return 0.f;
	else if (tmp > 1.0f) return 1.f;
	else return tmp;
}

float MOHPC::EntityField::UnpackAngle(int result, intptr_t bits, bool isNeg)
{
	const float tmp = isNeg ? -1.f : 1.f;
	if (bits < 0) bits = ~bits;
	else bits = bits;

	if (bits == 12) return result * 0.087890625f * tmp;
	else if (bits == 8) return result * 1.411764705882353f * tmp;
	else if (bits == 16) return result * 0.0054931640625f * tmp;
	else return result * (1 << bits) * tmp / 360.0f;
}

float MOHPC::EntityField::UnpackAnimTime(int result)
{
	return result / 100.f;
}
