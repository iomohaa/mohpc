#include <MOHPC/Network/InfoTypes.h>
#include <MOHPC/Network/Types.h>

using namespace MOHPC;
using namespace Network;

usercmd_t::usercmd_t(uint32_t inServerTime)
	: usercmd_t()
{
	serverTime = inServerTime;
}

MOHPC::usercmd_t::usercmd_t()
	: serverTime(0)
	, buttons{ 0 }
	, angles{ 0 }
	, forwardmove(0)
	, rightmove(0)
	, upmove(0)
{
}

void usercmd_t::setWeaponCommand(weaponCommand_e weaponCommand)
{
	buttons.bitflags.weaponFlags = 1 << (uint8_t)weaponCommand;
}

void usercmd_t::setAngles(float pitch, float yaw, float roll)
{
	angles[0] = AngleToShort(pitch);
	angles[1] = AngleToShort(yaw);
	angles[2] = AngleToShort(roll);
}

void usercmd_t::moveForward(int8_t value)
{
	forwardmove = value;
}

void usercmd_t::moveRight(int8_t value)
{
	rightmove = value;
}

void usercmd_t::moveUp(int8_t value)
{
	upmove = value;
}

void usercmd_t::jump()
{
	upmove = 127;
}

void usercmd_t::crouch()
{
	upmove = -128;
}

usereyes_t::usereyes_t()
	: angles{ 0 }
	, ofs{ 0 }
{
}

void usereyes_t::setOffset(int8_t x, int8_t y, int8_t z)
{
	ofs[0] = x;
	ofs[1] = y;
	ofs[1] = z;
}

void usereyes_t::setAngle(float pitch, float yaw)
{
	angles[0] = pitch;
	angles[1] = yaw;
}

playerState_t::playerState_t()
	: commandTime(0)
	, pm_type(pmType_e::PM_NORMAL)
	, bobCycle(0)
	, pm_flags(0)
	, pm_time(0)
	, gravity(0)
	, speed(0)
	, delta_angles{ 0 }
	, groundEntityNum(0)
	, walking(false)
	, groundPlane(false)
	, feetfalling(0)
	, radarInfo(0)
	, clientNum(0)
	, viewheight(0)
	, fLeanAngle(0.f)
	, iViewModelAnim(0)
	, iViewModelAnimChanged(0)
	, bVoted(false)
	, stats{ 0 }
	, activeItems{ 0 }
	, ammo_name_index{ 0 }
	, ammo_amount{ 0 }
	, max_ammo_amount{ 0 }
	, current_music_mood(0)
	, fallback_music_mood(0)
	, music_volume(0.f)
	, music_volume_fade_time(0.f)
	, reverb_type(0)
	, reverb_level(0)
	, blend{ 0 }
	, fov(0.f)
	, camera_time(0.f)
	, camera_flags(0)
{
}

trajectory_t::trajectory_t()
	: trTime(0)
{}

frameInfo_t::frameInfo_t()
	: index(0)
	, time(0.f)
	, weight(0.f)
{}

entityState_t::entityState_t()
	: number(0)
	, eType(entityType_e::modelanim_skel)
	, eFlags(0)
	, constantLight(0)
	, loopSound(0)
	, loopSoundVolume(0.f)
	, loopSoundMinDist(0.f)
	, loopSoundMaxDist(0.f)
	, loopSoundPitch(0.f)
	, loopSoundFlags(0)
	, parent(0)
	, tag_num(0)
	, attach_use_angles(false)
	, beam_entnum(0)
	, modelindex(0)
	, usageIndex(0)
	, skinNum(0)
	, wasframe(0)
	, actionWeight(0.f)
	, bone_tag{ 0 }
	, surfaces{ 0 }
	, clientNum(0)
	, groundEntityNum(0)
	, solid(0)
	, scale(0.f)
	, alpha(0.f)
	, renderfx(0)
	, shader_data{ 0.f }
	, shader_time(0.f)
	, quat{ 0.f }

{
}

MOHPC::Network::SnapshotInfo::SnapshotInfo()
	: snapFlags(SNAPFLAG_NOT_ACTIVE)
	, ping(0)
	, serverTime(0)
	, areamask{0}
	, numEntities(0)
	, numServerCommands(0)
	, serverCommandSequence(0)
{
}

sound_t::sound_t()
	: entity(nullptr)
	, soundName(nullptr)
	, hasStopped(false)
	, isStreamed(false)
	, isSpatialized(false)
	, hasVolume(false)
	, hasDist(false)
	, hasPitch(false)
	, channel(0)
	, volume(0.f)
	, minDist(0.f)
	, maxDist(0.f)
	, pitch(0.f)
{
}

protocolType_c::protocolType_c()
	: serverType(0)
	, protocolVersion(protocolVersion_e::bad)
{}

protocolType_c::protocolType_c(uint8_t inServerType, protocolVersion_e inProtocolVersion)
	: serverType(inServerType)
	, protocolVersion(inProtocolVersion)
{}

uint8_t protocolType_c::getServerType() const
{
	return serverType;
}

protocolVersion_e protocolType_c::getProtocolVersion() const
{
	return protocolVersion;
}
