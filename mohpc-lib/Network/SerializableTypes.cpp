#include <MOHPC/Network/InfoTypes.h>
#include <MOHPC/Network/SerializableTypes.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Misc/Endian.h>
#include <MOHPC/Common/Log.h>
#include <cstring>

#define MOHPC_LOG_NAMESPACE "nettypes"

using namespace MOHPC;

using StandardCoord = NetCoord<16, -8192, 8192>;
using ExtraCoord = NetCoord<18, -8192, 8192>;

template<typename T>
static T packType(T val)
{
	T packed = (val << 1) - 0x7A000000;
	if (val < 0) packed |= 1;
	return packed;
}

template<typename T>
static T unpackType(T packed)
{
	if (packed & 1) {
		return ((packed + 0x7A000000) >> 1) | 0x80000000;
	}
	else {
		return (packed + 0x7A000000) >> 1;
	}
}

void SerializableUsercmd::LoadDelta(MSG& msg, const ISerializableMessage* from, intptr_t key)
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

void SerializableUsercmd::SaveDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) const
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
{ PSF(commandTime), 32, fieldType_ver6_e::regular },
{ PSF(origin[0]), 0, fieldType_ver6_e::coord },
{ PSF(origin[1]), 0, fieldType_ver6_e::coord },
{ PSF(viewangles[1]), 0, fieldType_ver6_e::regular },
{ PSF(velocity[1]), 0, fieldType_ver6_e::velocity },
{ PSF(velocity[0]), 0, fieldType_ver6_e::velocity },
{ PSF(viewangles[0]), 0, fieldType_ver6_e::regular },
{ PSF(pm_time), -16, fieldType_ver6_e::regular },
{ PSF(origin[2]), 0, fieldType_ver6_e::coord },
{ PSF(velocity[2]), 0, fieldType_ver6_e::velocity },
{ PSF(iViewModelAnimChanged), 2, fieldType_ver6_e::regular },
{ PSF(damage_angles[0]), -13, fieldType_ver6_e::angle },
{ PSF(damage_angles[1]), -13, fieldType_ver6_e::angle },
{ PSF(damage_angles[2]), -13, fieldType_ver6_e::angle },
{ PSF(speed), 16, fieldType_ver6_e::regular },
{ PSF(delta_angles[1]), 16, fieldType_ver6_e::regular },
{ PSF(viewheight), -8, fieldType_ver6_e::regular },
{ PSF(groundEntityNum), GENTITYNUM_BITS, fieldType_ver6_e::regular },
{ PSF(delta_angles[0]), 16, fieldType_ver6_e::regular },
{ PSF(iViewModelAnim), 4, fieldType_ver6_e::regular },
{ PSF(fov), 0, fieldType_ver6_e::regular },
{ PSF(current_music_mood), 8, fieldType_ver6_e::regular },
{ PSF(gravity), 16, fieldType_ver6_e::regular },
{ PSF(fallback_music_mood), 8, fieldType_ver6_e::regular },
{ PSF(music_volume), 0, fieldType_ver6_e::regular },
{ PSF(pm_flags), 16, fieldType_ver6_e::regular },
{ PSF(clientNum), 8, fieldType_ver6_e::regular },
{ PSF(fLeanAngle), 0, fieldType_ver6_e::regular },
{ PSF(blend[3]), 0, fieldType_ver6_e::regular },
{ PSF(blend[0]), 0, fieldType_ver6_e::regular },
{ PSF(pm_type), 8, fieldType_ver6_e::regular },
{ PSF(feetfalling), 8, fieldType_ver6_e::regular },
{ PSF(camera_angles[0]), 16, fieldType_ver6_e::angle },
{ PSF(camera_angles[1]), 16, fieldType_ver6_e::angle },
{ PSF(camera_angles[2]), 16, fieldType_ver6_e::angle },
{ PSF(camera_origin[0]), 0, fieldType_ver6_e::coord },
{ PSF(camera_origin[1]), 0, fieldType_ver6_e::coord },
{ PSF(camera_origin[2]), 0, fieldType_ver6_e::coord },
{ PSF(camera_posofs[0]), 0, fieldType_ver6_e::coord },
{ PSF(camera_posofs[2]), 0, fieldType_ver6_e::coord },
{ PSF(camera_time), 0, fieldType_ver6_e::regular },
{ PSF(bobCycle), 8, fieldType_ver6_e::regular },
{ PSF(delta_angles[2]), 16, fieldType_ver6_e::regular },
{ PSF(viewangles[2]), 0, fieldType_ver6_e::regular },
{ PSF(music_volume_fade_time), 0, fieldType_ver6_e::regular },
{ PSF(reverb_type), 6, fieldType_ver6_e::regular },
{ PSF(reverb_level), 0, fieldType_ver6_e::regular },
{ PSF(blend[1]), 0, fieldType_ver6_e::regular },
{ PSF(blend[2]), 0, fieldType_ver6_e::regular },
{ PSF(camera_offset[0]), 0, fieldType_ver6_e::regular },
{ PSF(camera_offset[1]), 0, fieldType_ver6_e::regular },
{ PSF(camera_offset[2]), 0, fieldType_ver6_e::regular },
{ PSF(camera_posofs[1]), 0, fieldType_ver6_e::coord },
{ PSF(camera_flags), 16, fieldType_ver6_e::regular }
};

static_assert(sizeof(playerStateFields) == sizeof(netField_template_t<fieldType_ver6_e>) * 54);

const netField_template_t<fieldType_ver15_e> playerStateFields_ver15[] =
{
{ PSF(commandTime), 32, fieldType_ver15_e::regular },
{ PSF(origin[0]), 0, fieldType_ver15_e::coordExtra },
{ PSF(origin[1]), 0, fieldType_ver15_e::coordExtra },
{ PSF(viewangles[1]), 0, fieldType_ver15_e::regular },
{ PSF(velocity[1]), 0, fieldType_ver15_e::velocity },
{ PSF(velocity[0]), 0, fieldType_ver15_e::velocity },
{ PSF(viewangles[0]), 0, fieldType_ver15_e::regular },
{ PSF(origin[2]), 0, fieldType_ver15_e::coordExtra },
{ PSF(velocity[2]), 0, fieldType_ver15_e::velocity },
{ PSF(iViewModelAnimChanged), 2, fieldType_ver15_e::regular },
{ PSF(damage_angles[0]), -13, fieldType_ver15_e::angle },
{ PSF(damage_angles[1]), -13, fieldType_ver15_e::angle },
{ PSF(damage_angles[2]), -13, fieldType_ver15_e::angle },
{ PSF(speed), 16, fieldType_ver15_e::regular },
{ PSF(delta_angles[1]), 16, fieldType_ver15_e::regular },
{ PSF(viewheight), -8, fieldType_ver15_e::regular },
{ PSF(groundEntityNum), GENTITYNUM_BITS, fieldType_ver15_e::regular },
{ PSF(delta_angles[0]), 16, fieldType_ver15_e::regular },
{ PSF(iViewModelAnim), 4, fieldType_ver15_e::regular },
{ PSF(fov), 0, fieldType_ver15_e::regular },
{ PSF(current_music_mood), 8, fieldType_ver15_e::regular },
{ PSF(gravity), 16, fieldType_ver15_e::regular },
{ PSF(fallback_music_mood), 8, fieldType_ver15_e::regular },
{ PSF(music_volume), 0, fieldType_ver15_e::regular },
{ PSF(pm_flags), 16, fieldType_ver15_e::regular },
{ PSF(clientNum), 8, fieldType_ver15_e::regular },
{ PSF(fLeanAngle), 0, fieldType_ver15_e::regular },
{ PSF(blend[3]), 0, fieldType_ver15_e::regular },
{ PSF(blend[0]), 0, fieldType_ver15_e::regular },
{ PSF(pm_type), 8, fieldType_ver15_e::regular },
{ PSF(feetfalling), 8, fieldType_ver15_e::regular },
{ PSF(radarInfo), 26, fieldType_ver15_e::regular },
{ PSF(camera_angles[0]), 16, fieldType_ver15_e::angle },
{ PSF(camera_angles[1]), 16, fieldType_ver15_e::angle },
{ PSF(camera_angles[2]), 16, fieldType_ver15_e::angle },
{ PSF(camera_origin[0]), 0, fieldType_ver15_e::coordExtra },
{ PSF(camera_origin[1]), 0, fieldType_ver15_e::coordExtra },
{ PSF(camera_origin[2]), 0, fieldType_ver15_e::coordExtra },
{ PSF(camera_posofs[0]), 0, fieldType_ver15_e::coordExtra },
{ PSF(camera_posofs[2]), 0, fieldType_ver15_e::coordExtra },
{ PSF(camera_time), 0, fieldType_ver15_e::regular },
{ PSF(bVoted), 1, fieldType_ver15_e::regular },
{ PSF(bobCycle), 8, fieldType_ver15_e::regular },
{ PSF(delta_angles[2]), 16, fieldType_ver15_e::regular },
{ PSF(viewangles[2]), 0, fieldType_ver15_e::regular },
{ PSF(music_volume_fade_time), 0, fieldType_ver15_e::regular },
{ PSF(reverb_type), 6, fieldType_ver15_e::regular },
{ PSF(reverb_level), 0, fieldType_ver15_e::regular },
{ PSF(blend[1]), 0, fieldType_ver15_e::regular },
{ PSF(blend[2]), 0, fieldType_ver15_e::regular },
{ PSF(camera_offset[0]), 0, fieldType_ver15_e::regular },
{ PSF(camera_offset[1]), 0, fieldType_ver15_e::regular },
{ PSF(camera_offset[2]), 0, fieldType_ver15_e::regular },
{ PSF(camera_posofs[1]), 0, fieldType_ver15_e::coordExtra },
{ PSF(camera_flags), 16, fieldType_ver15_e::regular }
};

static_assert(sizeof(playerStateFields_ver15) == sizeof(netField_template_t<fieldType_ver15_e>) * 55);

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
static const playerState_t nullPS;

void SerializablePlayerState::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{
	MsgTypesHelper msgHelper(msg);

	constexpr size_t numFields = sizeof(playerStateFields) / sizeof(playerStateFields[0]);

	const playerState_t* fromPS = from ? ((const SerializablePlayerState*)from)->GetState() : &nullPS;
	const netField_t* field;
	bool deltaNeededList[numFields];

	uint8_t lc = 0;
	uint8_t i;

	// Calculate the number of changes
	for (i = 0, field = playerStateFields; i < numFields; i++, field++)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->offset);
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)&state + field->offset);

		const bool deltaNeeded = EntityField::DeltaNeeded(fromF, toF, field);
		deltaNeededList[i] = deltaNeeded;
		if (deltaNeeded) {
			lc = i + 1;
		}
	}

	// Serialize the number of changes
	msg.WriteByte(lc);

	for (i = 0, field = playerStateFields; i < lc; ++i, ++field)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->offset);
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)GetState() + field->offset);

		const bool hasChange = deltaNeededList[i];

		msg.WriteBool(hasChange);
		if (!hasChange) {
			continue;
		}

		switch (fieldType_ver6_e(field->type))
		{
		case fieldType_ver6_e::regular:
			EntityField::WriteNumberPlayerStateField(msg, field->bits, toF, field->size);
			break;
		case fieldType_ver6_e::angle:
			EntityField::WriteAngleField(msg, field->bits, *(float*)toF);
			break;
		case fieldType_ver6_e::coord:
			msgHelper.WriteCoord(*(float*)toF);
			break;
		case fieldType_ver6_e::velocity:
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

void SerializablePlayerState::LoadDelta(MSG& msg, const ISerializableMessage* from)
{
	MsgTypesHelper msgHelper(msg);

	constexpr size_t numFields = sizeof(playerStateFields) / sizeof(playerStateFields[0]);

	const SerializablePlayerState* srFrom = (const SerializablePlayerState*)from;
	const playerState_t* fromPS = srFrom ? srFrom->GetState() : &nullPS;

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
		case fieldType_ver6_e::regular:
			EntityField::ReadNumberPlayerStateField(msg, field->bits, toF, field->size);
			break;
		case fieldType_ver6_e::angle:
			*(float*)toF = EntityField::ReadAngleField(msg, field->bits);
			break;
		case fieldType_ver6_e::coord:
			*(float*)toF = msgHelper.ReadCoord();
			break;
		case fieldType_ver6_e::velocity:
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

void SerializablePlayerState_ver15::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{
	MsgTypesHelper msgHelper(msg);

	constexpr size_t numFields = sizeof(playerStateFields_ver15) / sizeof(playerStateFields_ver15[0]);

	const playerState_t* fromPS = from ? ((SerializablePlayerState*)from)->GetState() : &nullPS;
	const netField_t* field;
	bool deltaNeededList[numFields];

	uint8_t lc = 0;
	uint8_t i;

	// Calculate the number of changes
	for (i = 0, field = playerStateFields_ver15; i < numFields; i++, field++)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->offset);
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)&state + field->offset);

		const bool deltaNeeded = EntityField::DeltaNeeded(fromF, toF, field);
		deltaNeededList[i] = deltaNeeded;
		if (deltaNeeded) {
			lc = i + 1;
		}
	}

	// Serialize the number of changes
	msg.WriteByte(lc);

	for (i = 0, field = playerStateFields_ver15; i < lc; ++i, ++field)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->offset);
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)GetState() + field->offset);

		const bool hasChange = deltaNeededList[i];

		msg.WriteBool(hasChange);
		if (!hasChange) {
			continue;
		}

		uint32_t packedValue;

		switch (fieldType_ver15_e(field->type))
		{
		case fieldType_ver15_e::regular:
			EntityField::WriteRegular2(msg, field->bits, toF, field->size);
			break;
		case fieldType_ver15_e::angle:
			packedValue = EntityField::PackAngle2(*(float*)toF, field->bits);
			msg.WriteNumber<uint32_t>(packedValue, field->bits);
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

void SerializablePlayerState_ver15::LoadDelta(MSG& msg, const ISerializableMessage* from)
{
	MsgTypesHelper msgHelper(msg);

	constexpr size_t numFields = sizeof(playerStateFields_ver15) / sizeof(playerStateFields_ver15[0]);

	const SerializablePlayerState* srFrom = (const SerializablePlayerState*)from;
	const playerState_t* fromPS = srFrom ? srFrom->GetState() : &nullPS;

	// Serialize the number of changes
	const uint8_t lc = msg.ReadByte();
	if (lc > numFields) {
		throw SerializableErrors::BadEntityFieldCountException(lc);
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

		uint32_t result;
		intptr_t bits = 0;
		int32_t coordOffset, coordVal;

		switch (fieldType_ver15_e(field->type))
		{
		case fieldType_ver15_e::regular:
			EntityField::ReadRegular2(msg, field->bits, toF, field->size);
			break;
		case fieldType_ver15_e::angle: // anglestmp = 1.0f;
			result = msg.ReadNumber<uint32_t>(field->bits < 0 ? -field->bits : field->bits);
			*(float*)toF = EntityField::UnpackAngle2(result, field->bits);
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

void SerializableEntityState::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{
	MsgTypesHelper msgHelper(msg);
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

		msgHelper.WriteEntityNum(fromEnt->number);
		// removed
		msg.WriteBool(true);
		return;
	}

	// not removed
	msg.WriteBool(false);

	// build the change vector as bytes so it is endien independent
	for (i = 0, field = entityStateFields; i < numFields; i++, field++)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromEnt + field->offset);
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)&state + field->offset);

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
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromEnt + field->offset);
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)&state + field->offset);

		bool isDiff = deltaNeededList[i];
		msg.WriteBool(isDiff);
		if (!isDiff) {
			continue;
		}

		uint32_t packedValue;

		switch (fieldType_ver6_e(field->type))
		{
		case fieldType_ver6_e::regular:
			EntityField::WriteNumberEntityField(msg, field->bits, toF, field->size);
			break;
		case fieldType_ver6_e::angle:
			EntityField::WriteAngleField(msg, field->bits, *(float*)toF);
			break;
		case fieldType_ver6_e::animTime:
			packedValue = EntityField::PackAnimTime(*(float*)toF, field->bits);
			msg.WriteNumber(packedValue, field->bits);
			break;
		case fieldType_ver6_e::animWeight:
			packedValue = EntityField::PackAnimWeight(*(float*)toF, field->bits);
			msg.WriteNumber(packedValue, field->bits);
			break;
		case fieldType_ver6_e::scale:
			packedValue = EntityField::PackScale(*(float*)toF, field->bits);
			msg.WriteNumber(packedValue, field->bits);
			break;
		case fieldType_ver6_e::alpha:
			packedValue = EntityField::PackAlpha(*(float*)toF, field->bits);
			msg.WriteNumber(packedValue, field->bits);
			break;
		case fieldType_ver6_e::coord:
			msgHelper.WriteCoord(*(float*)toF);
			break;
		case fieldType_ver6_e::velocity:
			msgHelper.WriteCoordSmall(*(float*)toF);
			break;
		case fieldType_ver6_e::simple:
			EntityField::WriteSimple(msg, field->bits, toF, field->size);
			break;
		default:
			throw SerializableErrors::BadEntityFieldException(field->type, field->name);
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
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromEnt + field->offset);
		uint8_t* toF = (uint8_t*)((uint8_t*)&state + field->offset);

		const bool isDiff = msg.ReadBool();
		if (!isDiff)
		{
			// no changes
			memcpy(toF, fromF, field->size);
			continue;
		}

		uint32_t packedValue;

		switch (fieldType_ver6_e(field->type))
		{
		case fieldType_ver6_e::regular:
			EntityField::ReadRegular(msg, field->bits, toF, field->size);
			break;
		case fieldType_ver6_e::angle:
			*(float*)toF = EntityField::ReadAngleField(msg, field->bits);
			break;
		case fieldType_ver6_e::animTime:
			packedValue = msg.ReadNumber<uint32_t>(field->bits);
			*(float*)toF = EntityField::UnpackAnimTime(packedValue);
			break;
		case fieldType_ver6_e::animWeight:
			packedValue = msg.ReadByte();
			*(float*)toF = EntityField::UnpackAnimWeight(packedValue, field->bits);
			break;
		case fieldType_ver6_e::scale:
			packedValue = msg.ReadNumber<uint32_t>(field->bits);
			*(float*)toF = EntityField::UnpackScale(packedValue);
			break;
		case fieldType_ver6_e::alpha:
			packedValue = msg.ReadByte();
			*(float*)toF = EntityField::UnpackAlpha(packedValue, field->bits);
			break;
		case fieldType_ver6_e::coord:
			*(float*)toF = msgHelper.ReadCoord();
			break;
		case fieldType_ver6_e::velocity:
			*(float*)toF = msgHelper.ReadCoordSmall();
			break;
		case fieldType_ver6_e::simple:
			EntityField::ReadSimple(msg, field->bits, toF, field->size);
			break;
		default:
			throw SerializableErrors::BadEntityFieldException(field->type, field->name);
		}
	}

	// assign unchanged fields accordingly
	EntityField::CopyFields(fromEnt, GetState(), lc, numFields, entityStateFields);
}

void SerializableEntityState_ver15::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{
	MsgTypesHelper msgHelper(msg);
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

		msgHelper.WriteEntityNum(fromEnt->number);
		// removed
		msg.WriteBool(true);
		return;
	}

	// not removed
	msg.WriteBool(false);

	// build the change vector as bytes so it is endien independent
	for (i = 0, field = entityStateFields_ver15; i < numFields; i++, field++)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromEnt + field->offset);
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)&state + field->offset);

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
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromEnt + field->offset);
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)&state + field->offset);

		bool isDiff = deltaNeededList[i];
		msg.WriteBool(isDiff);
		if (!isDiff) {
			continue;
		}

		uint32_t packedValue;

		switch (fieldType_ver15_e(field->type))
		{
		case fieldType_ver15_e::regular:
			EntityField::WriteRegular2(msg, field->bits, toF, field->size);
			break;
		case fieldType_ver15_e::angle:
			packedValue = EntityField::PackAngle2(*(float*)toF, field->bits);
			msg.WriteNumber<uint32_t>(packedValue, field->bits);
			break;
		case fieldType_ver15_e::animTime:
			if (fabs(*(float*)toF - *(float*)fromF) >= 0.001f)
			{
				msg.WriteBool(true);

				packedValue = EntityField::PackAnimTime(*(float*)toF, field->bits);
				msg.WriteNumber(packedValue, field->bits);
			}
			else
			{
				// no changes
				msg.WriteBool(false);
			}
			break;
		case fieldType_ver15_e::animWeight:
			packedValue = EntityField::PackAnimWeight(*(float*)toF, field->bits);
			msg.WriteNumber(packedValue, field->bits);
			break;
		case fieldType_ver15_e::scale:
			packedValue = EntityField::PackScale(*(float*)toF, field->bits);
			msg.WriteNumber(packedValue, field->bits);
			break;
		case fieldType_ver15_e::alpha:
			packedValue = EntityField::PackAlpha(*(float*)toF, field->bits);
			msg.WriteNumber(packedValue, field->bits);
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
			EntityField::WriteSimple(msg, field->bits, toF, field->size);
			break;
		default:
			throw SerializableErrors::BadEntityFieldException(field->type, field->name);
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
		case fieldType_ver15_e::regular:
			EntityField::ReadRegular2(msg, field->bits, toF, field->size);
			break;
		case fieldType_ver15_e::angle:
			result = msg.ReadNumber<uint32_t>(field->bits < 0 ? -field->bits : field->bits);
			*(float*)toF = EntityField::UnpackAngle2(result, field->bits);
			break;
		case fieldType_ver15_e::animTime: // time
			result = 0;
			if (msg.ReadBool())
			{
				result = msg.ReadNumber<int>(field->bits);
				*(float*)toF = EntityField::UnpackAnimTime(result);
			}
			else
			{
				// use delta time instead
				*(float*)toF = *(float*)fromF + timeDelta;
			}
			break;
		case fieldType_ver15_e::animWeight:
			result = msg.ReadNumber<int32_t>(field->bits);
			*(float*)toF = EntityField::UnpackAnimWeight(result, field->bits);
			break;
		case fieldType_ver15_e::scale:
			result = msg.ReadNumber<int32_t>(field->bits);
			*(float*)toF = EntityField::UnpackScale(result);
			break;
		case fieldType_ver15_e::alpha:
			result = msg.ReadNumber<int32_t>(field->bits);
			*(float*)toF = EntityField::UnpackAlpha(result, field->bits);
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
			EntityField::ReadSimple(msg, field->bits, toF, field->size);
			break;
		default:
			throw SerializableErrors::BadEntityFieldException(field->type, field->name);
		}
	}

	// assign unchanged fields accordingly
	EntityField::CopyFields(fromEnt, GetState(), lc, numFields, entityStateFields_ver15);
}

SerializableEntityState_ver15::SerializableEntityState_ver15(entityState_t& inState, entityNum_t newNum, float timeDeltaValue)
	: SerializableEntityState(inState, newNum)
	, timeDelta(timeDeltaValue)
{

}

void EntityField::ReadNumberPlayerStateField(MSG& msg, intptr_t bits, void* toF, size_t size)
{
	if (bits == 0)
	{
		// float
		const bool isFullFloat = msg.ReadBool();
		if (!isFullFloat)
		{
			// integral float
			int32_t truncFloat = msg.ReadNumber<int32_t>(FLOAT_INT_BITS);
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
	else
	{
		// integer
		std::memset(toF, 0, size);
		msg.ReadBits(toF, bits);
		Endian.LittlePointer(toF, size);
	}
}

void EntityField::WriteNumberPlayerStateField(MSG& msg, intptr_t bits, const void* toF, size_t size)
{
	if (bits == 0)
	{
		float floatVal = *(float*)toF;
		int truncFloat = (int)floatVal;

		const bool isFullFloat =
			truncFloat == floatVal && truncFloat + FLOAT_INT_BIAS >= 0 &&
			truncFloat + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS);

		msg.WriteBool(isFullFloat);

		if (!isFullFloat)
		{
			int truncated = truncFloat + FLOAT_INT_BIAS;
			msg.WriteNumber(truncated, FLOAT_INT_BITS);
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
		uint32_t tmp = 0;
		std::memcpy(&tmp, toF, size);
		Endian.LittlePointer(&tmp, size);

		msg.WriteBits(&tmp, bits);
	}
}

void EntityField::ReadRegular(MSG& msg, intptr_t bits, void* toF, size_t size)
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
				int32_t truncFloat = msg.ReadNumber<int32_t>(FLOAT_INT_BITS);
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
			Endian.LittlePointer(toF, size);
		}
	}
}

void EntityField::WriteNumberEntityField(MSG& msg, intptr_t bits, const void* toF, size_t size)
{
	if (bits == 0)
	{
		// float
		const float floatVal = *(float*)toF;
		const int32_t truncFloat = (int)floatVal;

		const bool hasValue = floatVal != 0.f;
		msg.WriteBool(hasValue);

		if (hasValue)
		{
			const bool isPartial = truncFloat == floatVal && truncFloat + FLOAT_INT_BIAS >= 0 &&
				truncFloat + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS);

			msg.WriteBool(!isPartial);

			if (isPartial)
			{
				// send as small integer
				int newVal = truncFloat + FLOAT_INT_BIAS;
				msg.WriteNumber(newVal, FLOAT_INT_BITS);
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
		bool hasValue = false;
		for (size_t i = 0; i < size; ++i)
		{
			if (((uint8_t*)toF)[i] != 0)
			{
				hasValue = true;
				break;
			}
		}

		msg.WriteBool(hasValue);

		if (hasValue)
		{
			uint32_t tmp = 0;
			std::memcpy(&tmp, toF, size);
			Endian.LittlePointer(&tmp, size);
			// integer
			msg.WriteBits(&tmp, bits);
		}
	}
}

void EntityField::ReadRegular2(MSG& msg, intptr_t bits, void* toF, size_t size)
{
	if (bits == 0)
	{
		// float
		const bool hasValue = msg.ReadBool();

		if (!hasValue) {
			*(float*)toF = 0.0f;
		}
		else
		{
			const bool isFullFloat = msg.ReadBool();
			if (!isFullFloat)
			{
				// integral float
				int32_t truncFloat = msg.ReadNumber<int32_t>(FLOAT_INT_BITS);
				unshiftValue(&truncFloat, sizeof(int32_t));

				*(float*)toF = (float)truncFloat;
			}
			else
			{
				// full floating point value
				uint32_t packedFloat = msg.ReadInteger();

				*(uint32_t*)toF = unpackType(packedFloat);
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
			Endian.LittlePointer(toF, size);
		}

		if (bits < 0)
		{
			size_t nbits = -bits;
			unshiftValue(toF, size);
		}
	}
}

void EntityField::WriteRegular2(MSG& msg, intptr_t bits, const void* toF, size_t size)
{
	if (bits == 0)
	{
		// float
		const float floatVal = *(float*)toF;
		const int32_t truncFloat = (int32_t)floatVal;

		const bool hasValue = floatVal != 0.f;
		msg.WriteBool(hasValue);

		if (hasValue)
		{
			const bool isPartial = truncFloat == floatVal && truncFloat + FLOAT_INT_BIAS >= 0 &&
				truncFloat + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS);

			msg.WriteBool(!isPartial);

			if (isPartial)
			{
				// send as small integer
				msg.WriteNumber(shiftValue(truncFloat), FLOAT_INT_BITS);
			}
			else
			{
				// send as full floating point value
				const uint32_t packedValue = packType(*(int32_t*)toF);
				msg.WriteUInteger(packedValue);
			}
		}
	}
	else
	{
		bool hasValue = false;
		for (size_t i = 0; i < size; ++i)
		{
			if (((uint8_t*)toF)[i] != 0)
			{
				hasValue = true;
				break;
			}
		}

		msg.WriteBool(hasValue);

		if (hasValue)
		{
			uint32_t tmp = 0;
			std::memcpy(&tmp, toF, size);
			Endian.LittlePointer(&tmp, size);

			if (bits < 0)
			{
				size_t nbits = -bits;
				shiftValue(&tmp, size);
			}

			// integer
			msg.WriteBits(&tmp, bits);
		}
	}
}

float EntityField::ReadAngleField(MSG& msg, size_t bits)
{
	if (bits < 0)
	{
		const bool isNeg = msg.ReadBool();
		const uint32_t packedValue = msg.ReadNumber<uint32_t>(~bits);
		return EntityField::UnpackAngle(packedValue, ~bits, isNeg);
	}
	else
	{
		const uint32_t packedValue = msg.ReadNumber<uint32_t>(bits);
		return EntityField::UnpackAngle(packedValue, bits, false);
	}
}

void EntityField::WriteAngleField(MSG& msg, size_t bits, float angle)
{
	const uint32_t packedValue = EntityField::PackAngle(angle, bits);
	if (bits < 0)
	{
		msg.WriteBool(angle < 0);
		msg.WriteNumber<uint32_t>(packedValue, ~bits);
	}
	else
	{
		msg.WriteNumber<uint32_t>(packedValue, bits);
	}
}

float EntityField::ReadTimeField(MSG& msg, size_t bits)
{
	int result = msg.ReadNumber<int>(15);
	return EntityField::UnpackAnimTime(result);
}

void EntityField::WriteTimeField(MSG& msg, float time)
{
	const uint32_t packedValue = EntityField::PackAnimTime(time, 15);
	msg.WriteNumber(packedValue, 15);
}

float EntityField::ReadScaleField(MSG& msg, size_t bits)
{
	int16_t result = msg.ReadNumber<int16_t>(10);
	return EntityField::UnpackScale(result);
}

void EntityField::WriteScaleField(MSG& msg, float time)
{
	float tmp = time;
	intptr_t value = intptr_t(tmp * 100.0f);
	if (value < 0) value = 0;
	else if (value > 1023) value = 1023;

	msg.WriteNumber(value, 10);
}

void EntityField::CopyFields(const entityState_t* other, entityState_t* target, size_t from, size_t to, const netField_t* fieldlist)
{
	size_t i;
	const netField_t* field;
	for (i = from, field = &fieldlist[from]; i < to; i++, field++)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)other + field->offset);
		uint8_t* toF = (uint8_t*)((uint8_t*)target + field->offset);

		// no change
		std::memcpy(toF, fromF, field->size);
	}
}

int32_t EntityField::PackCoord(float val)
{
	return StandardCoord().pack(val);
}

int32_t EntityField::PackCoordExtra(float val)
{
	const int32_t packedValue = ExtraCoord().pack(val);
	if (!(packedValue & ExtraCoord::coordMask)) {
		MOHPC_LOG(Warn, "Illegal XYZ coordinates for an entity, information lost in transmission");
	}

	return packedValue;
}

float EntityField::UnpackCoord(int32_t val)
{
	return StandardCoord().unpack(val);
}

float EntityField::UnpackCoordExtra(int32_t val)
{
	return ExtraCoord().unpack(val);
}

uint32_t EntityField::PackAnimWeight(float weight, size_t bits)
{
	const int32_t max = (1 << bits) - 1;
	int32_t packedValue = (uint32_t)roundf(weight * max);

	if (packedValue < 0) packedValue = 0;
	else if (packedValue > max) packedValue = max;

	return packedValue;
}

float EntityField::UnpackAnimWeight(int result, intptr_t bits)
{
	const int32_t max = (1 << bits) - 1;
	const float tmp = (float)result / (float)max;

	if (tmp < 0.0f) return 0.f;
	else if (tmp > 1.0f) return 1.f;
	else return tmp;
}

uint32_t EntityField::PackScale(float scale, size_t bits)
{
	int32_t value = (uint32_t)roundf(scale * 100.0f);
	const int32_t max = (1 << bits) - 1;

	if (value < 0) value = 0;
	else if (value > max) value = max;

	return value;
}

float EntityField::UnpackScale(int16_t result)
{
	return result / 100.f;
}

uint32_t EntityField::PackAlpha(float alpha, size_t bits)
{
	const int32_t max = (1 << bits) - 1;
	int32_t packedValue = (uint32_t)roundf(alpha * max);

	if (packedValue < 0) packedValue = 0;
	else if (packedValue > max) packedValue = max;

	return packedValue;
}

float EntityField::UnpackAlpha(int result, intptr_t bits)
{
	const int32_t max = (1 << bits) - 1;
	const float tmp = (float)result / (float)max;

	if (tmp < 0.0f) return 0.f;
	else if (tmp > 1.0f) return 1.f;

	else return tmp;
}

uint32_t EntityField::PackAngle(float angle, intptr_t bits)
{
	float tmp = angle;
	if (bits < 0)
	{
		if (tmp < 0) tmp = -tmp;
		bits = ~bits;
	}

	if (bits == 12)
	{
		tmp = tmp * 4096.0f / 360.0f;
		return (uint32_t)tmp & 4095;
	}
	else if (bits == 8)
	{
		tmp = tmp * 256.0f / 360.0f;
		return (uint32_t)tmp & 255;
	}
	else if (bits == 16)
	{
		tmp = tmp * 65536.0f / 360.0f;
		return (uint32_t)tmp & 65535;
	}
	else
	{
		tmp = tmp * (1 << (uint8_t)bits) / 360.0f;
		return ((uint32_t)tmp) & ((1 << (uint8_t)bits) - 1);
	}
}

uint32_t EntityField::PackAngle2(float angle, intptr_t bits)
{
	uint32_t flags = 0;
	float tmp = angle;

	if (bits < 0)
	{
		bits = ~bits;
		if (tmp < 0)
		{
			tmp = -tmp;
			flags = 1 << bits;
		}
	}

	if (bits == 12)
	{
		tmp = floorf(tmp * 4096.0f / 360.0f);
		return flags | ((uint32_t)tmp & 4095);
	}
	else if (bits == 8)
	{
		tmp = floorf(tmp * 256.0f / 360.0f);
		return flags | ((uint32_t)tmp & 255);
	}
	else if (bits == 16)
	{
		tmp = floorf(tmp * 65536.0f / 360.0f);
		return flags | ((uint32_t)tmp & 65535);
	}
	else
	{
		tmp = floorf(tmp * (1 << (uint8_t)bits) / 360.0f);
		return flags | (((uint32_t)tmp) & ((1 << (uint8_t)bits) - 1));
	}
}

float EntityField::UnpackAngle(int result, intptr_t bits, bool isNeg)
{
	float tmp = isNeg ? -1.f : 1.f;
	if (bits < 0) bits = ~bits;
	else bits = bits;

	if (bits == 12) return result * (360.f / 4096.f) * tmp;
	else if (bits == 8) return result * (360.f / 256.f) * tmp;
	else if (bits == 16) return result * (360.f / 65536.f) * tmp;
	else return result * (1 << bits) * tmp / 360.0f;
}

float EntityField::UnpackAngle2(uint32_t packedValue, intptr_t bits)
{
	const int32_t max = (1 << bits);
	uint32_t packedAngle = packedValue;

	float tmp = 1.f;
	if (bits < 0)
	{
		bits = ~bits;
		if (packedValue & max)
		{
			packedAngle = packedValue & ~max;
			tmp = -1.f;
		}
	}

	if (bits == 12) return packedAngle * (360.f / 4096.f) * tmp;
	else if (bits == 8) return packedAngle * (360.f / 256.f) * tmp;
	else if (bits == 16) return packedAngle * (360.f / 65536.f) * tmp;
	else return packedAngle * (1 << bits) * tmp / 360.0f;
}

uint32_t EntityField::PackAnimTime(float time, size_t bits)
{
	int32_t value = (uint32_t)roundf(time * 100.0f);
	const int32_t max = (1 << bits) - 1;

	if (value < 0) value = 0;
	else if (value > max) value = max;

	return value;
}

float EntityField::UnpackAnimTime(int result)
{
	return result / 100.f;
}

bool EntityField::DeltaNeeded(const void* fromField, const void* toField, const netField_t* field)
{
	if (!memcmp(fromField, toField, field->size))
	{
		// same values, not needed
		return false;
	}

	switch (fieldType_ver6_e(field->type))
	{
	case fieldType_ver6_e::regular:
	{
		if (!field->bits || field->bits == 32) {
			return true;
		}

		const uint32_t maxValue = (1 << field->bits) - 1;

		uint32_t xoredValue = 0;
		for (size_t i = 0; i < field->size; ++i)
		{
			uint8_t fromVal = ((const uint8_t*)fromField)[i];
			uint8_t toVal = ((const uint8_t*)toField)[i];
			xoredValue += fromVal ^ toVal;
		}

		return (xoredValue & maxValue) != 0;
	}
	case fieldType_ver6_e::angle:
		return EntityField::PackAngle(*(float*)fromField, field->bits) != EntityField::PackAngle(*(float*)toField, field->bits);
	case fieldType_ver6_e::animTime:
		return EntityField::PackAnimTime(*(float*)fromField, field->bits) != EntityField::PackAnimTime(*(float*)toField, field->bits);
	case fieldType_ver6_e::animWeight:
		return EntityField::PackAnimWeight(*(float*)fromField, field->bits) != EntityField::PackAnimWeight(*(float*)toField, field->bits);
	case fieldType_ver6_e::scale:
		return EntityField::PackScale(*(float*)fromField, field->bits) != EntityField::PackScale(*(float*)toField, field->bits);
	case fieldType_ver6_e::alpha:
		return EntityField::PackAlpha(*(float*)fromField, field->bits) != EntityField::PackAlpha(*(float*)toField, field->bits);
	case fieldType_ver6_e::coord:
	case fieldType_ver6_e::velocity:
		return true;
	default:
		return true;
	}
}

void EntityField::ReadSimple(MSG& msg, intptr_t bits, void* toF, size_t size)
{
	if (msg.ReadBool())
	{
		// has a new value
		msg.ReadBits(toF, bits);
	}
	else
	{
		memset(toF, 0, size);
	}
}

void EntityField::WriteSimple(MSG& msg, intptr_t bits, const void* toF, size_t size)
{
	const uint32_t packedValue = *(uint32_t*)toF >> 24;
	if (packedValue & 0xFF)
	{
		// write true for delta change
		msg.WriteBool(true);
		msg.WriteNumber<uint32_t>(packedValue & 0xFF, bits);
	}
	else
	{
		// no delta
		msg.WriteBool(false);
	}
}

bool EntityField::DeltaNeeded_ver15(const void* fromField, const void* toField, const netField_t* field)
{
	if (!memcmp(fromField, toField, field->size))
	{
		// same values, not needed
		return false;
	}

	switch (fieldType_ver15_e(field->type))
	{
	case fieldType_ver15_e::regular:
	{
		if (!field->bits || field->bits == 32) {
			return true;
		}

		const uint32_t maxValue = (1 << field->bits) - 1;

		uint32_t xoredValue = 0;
		for (size_t i = 0; i < field->size; ++i)
		{
			uint8_t fromVal = ((const uint8_t*)fromField)[i];
			uint8_t toVal = ((const uint8_t*)toField)[i];
			xoredValue += fromVal ^ toVal;
		}

		return (xoredValue & maxValue) != 0;
	}
	case fieldType_ver15_e::angle:
		return EntityField::PackAngle2(*(float*)fromField, field->bits) != EntityField::PackAngle2(*(float*)toField, field->bits);
	case fieldType_ver15_e::animTime:
		return EntityField::PackAnimTime(*(float*)fromField, field->bits) != EntityField::PackAnimTime(*(float*)toField, field->bits);
	case fieldType_ver15_e::animWeight:
		return EntityField::PackAnimWeight(*(float*)fromField, field->bits) != EntityField::PackAnimWeight(*(float*)toField, field->bits);
	case fieldType_ver15_e::scale:
		return EntityField::PackScale(*(float*)fromField, field->bits) != EntityField::PackScale(*(float*)toField, field->bits);
	case fieldType_ver15_e::alpha:
		return EntityField::PackAlpha(*(float*)fromField, field->bits) != EntityField::PackAlpha(*(float*)toField, field->bits);
	case fieldType_ver15_e::coord:
		return EntityField::PackCoord(*(float*)fromField) != EntityField::PackCoord(*(float*)toField);
	case fieldType_ver15_e::coordExtra:
		return EntityField::PackCoordExtra(*(float*)fromField) != EntityField::PackCoordExtra(*(float*)toField);
	case fieldType_ver15_e::velocity:
	case fieldType_ver15_e::simple:
		return true;
	default:
		return true;
	}
}

SerializableErrors::BadEntityNumberException::BadEntityNumberException(const char* inName, size_t inBadNumber)
	: name(inName)
	, badNumber(inBadNumber)
{}

const char* SerializableErrors::BadEntityNumberException::getName() const
{
	return name;
}

size_t SerializableErrors::BadEntityNumberException::getNumber() const
{
	return badNumber;
}
str SerializableErrors::BadEntityNumberException::what() const
{
	return str(badNumber);
}

SerializableErrors::BadEntityFieldException::BadEntityFieldException(uint8_t inFieldType, const char* inFieldName)
	: fieldType(inFieldType)
	, fieldName(inFieldName)
{}

uint8_t SerializableErrors::BadEntityFieldException::getFieldType() const
{
	return fieldType;
}

const char* SerializableErrors::BadEntityFieldException::getFieldName() const
{
	return fieldName;
}

str SerializableErrors::BadEntityFieldException::what() const
{
	return str::printf("%s: %d", getFieldName(), getFieldType());
}

SerializableErrors::BadEntityFieldCountException::BadEntityFieldCountException(uint8_t inCount)
	: count(inCount)
{}

uint8_t SerializableErrors::BadEntityFieldCountException::getCount() const
{
	return count;
}

str SerializableErrors::BadEntityFieldCountException::what() const
{
	return str((int)getCount());
}

SerializableErrors::BadCommandByteException::BadCommandByteException(uint8_t inCmdNum)
	: cmdNum(inCmdNum)
{}

uint8_t SerializableErrors::BadCommandByteException::getLength() const
{
	return cmdNum;
}

str SerializableErrors::BadCommandByteException::what() const
{
	return str((int)getLength());
}
