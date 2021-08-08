#include <MOHPC/Network/Types/PlayerState.h>

using namespace MOHPC;
using namespace MOHPC::Network;

radarInfo_t::radarInfo_t()
	: radarValue(0)
{

}

radarInfo_t::radarInfo_t(uint32_t value)
	: radarValue(value)
{
}

radarInfo_t::radarInfo_t(uint8_t clientNum, int8_t x, int8_t y, int8_t yaw, int8_t flags)
{
	radarValue = 0;
	radarValue |= clientNum & BIT_CLIENT;
	radarValue |= (((x + BIT_CLIENT) & BIT_COORD) << 6);
	radarValue |= (((y + BIT_CLIENT) & BIT_COORD) << 13);
	radarValue |= ((yaw & BIT_YAW) << 20);
	radarValue |= flags << 25;
}

uint8_t radarInfo_t::clientNum() const
{
	return radarValue & 0x3F;
}

int8_t radarInfo_t::x() const
{
	return ((radarValue >> 6) & 0x7F) - 0x3F;
}

int8_t radarInfo_t::y() const
{
	return ((radarValue >> 13) & 0x7F) - 0x3F;
}

int8_t radarInfo_t::yaw() const
{
	return (radarValue >> 20) & 0x1F;
}

int8_t radarInfo_t::flags() const
{
	return radarValue >> 25;
}

uint32_t radarInfo_t::getRaw() const
{
	return radarValue;
}

int8_t radarInfo_t::getMinCoord()
{
	return -(int8_t)(BIT_COORD >> 1);
}

int8_t radarInfo_t::getMaxCoord()
{
	return BIT_COORD >> 1;
}

float radarInfo_t::getCoordPrecision()
{
	return float(BIT_COORD >> 1);
}

float radarInfo_t::getYawPrecision()
{
	return 360.f / (BIT_YAW + 1);
}

playerState_t::playerState_t()
	: commandTime(std::chrono::milliseconds())
	, origin{ 0 }
	, velocity{ 0 }
	, falldir{ 0 }
	, viewangles{ 0 }
	, camera_origin{ 0 }
	, camera_angles{ 0 }
	, camera_offset{ 0 }
	, camera_posofs{ 0 }
	, damage_angles{ 0 }
	, pm_type(pmType_e::Normal)
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

netTime_t playerState_t::getCommandTime() const
{
	return commandTime;
}

pmType_e playerState_t::getPlayerMoveType() const
{
	return pm_type;
}

uint16_t playerState_t::getPlayerMoveFlags() const
{
	return pm_flags;
}

deltaTime16_t playerState_t::getPlayerMoveTime() const
{
	return pm_time;
}

uint8_t playerState_t::getBobCycle() const
{
	return bobCycle;
}

const_vec3p_t playerState_t::getOrigin() const
{
	return origin;
}

const_vec3p_t playerState_t::getVelocity() const
{
	return velocity;
}

uint16_t playerState_t::getGravity() const
{
	return gravity;
}

uint16_t playerState_t::getSpeed() const
{
	return speed;
}

void playerState_t::getDeltaAngles(netAngles_t& angles) const
{
	angles[0] = delta_angles[0];
	angles[1] = delta_angles[1];
	angles[2] = delta_angles[2];
}

entityNum_t playerState_t::getGroundEntityNum() const
{
	return groundEntityNum;
}

bool playerState_t::isWalking() const
{
	return walking;
}

bool playerState_t::isGroundPlane() const
{
	return groundPlane;
}

uint8_t playerState_t::getFeetFalling() const
{
	return feetfalling;
}

const_vec3p_t playerState_t::getFalldir() const
{
	return falldir;
}

const trace_t& playerState_t::getGroundTrace() const
{
	return groundTrace;
}

uint8_t playerState_t::getClientNum() const
{
	return clientNum;
}

const_vec3p_t playerState_t::getViewAngles() const
{
	return viewangles;
}

uint8_t playerState_t::getViewHeight() const
{
	return viewheight;
}

float playerState_t::getLeanAngles() const
{
	return fLeanAngle;
}

uint8_t playerState_t::getViewModelAnim() const
{
	return iViewModelAnim;
}

uint8_t playerState_t::getViewModelAnimChanges() const
{
	return iViewModelAnimChanged;
}

uint16_t playerState_t::getStats(playerstat_e statIndex) const
{
	return stats[(uint32_t)statIndex];
}

uint16_t playerState_t::getActiveItems(uint32_t index) const
{
	return activeItems[index];
}

uint16_t playerState_t::getAmmoNameIndex(uint32_t index) const
{
	return ammo_name_index[index];
}

uint16_t playerState_t::getAmmoAmount(uint32_t index) const
{
	return ammo_amount[index];
}

uint16_t playerState_t::getMaxAmmoAmount(uint32_t index) const
{
	return max_ammo_amount[index];
}

uint8_t playerState_t::getCurrentMusicMood() const
{
	return current_music_mood;
}

uint8_t playerState_t::getFallbackMusicMood() const
{
	return fallback_music_mood;
}

float playerState_t::getMusicVolume() const
{
	return music_volume;
}

float playerState_t::getMusicVolumeFadeTime() const
{
	return music_volume_fade_time;
}

uint8_t playerState_t::getReverbType() const
{
	return reverb_type;
}

float playerState_t::getReverbLevel() const
{
	return reverb_level;
}

void playerState_t::getBlend(float outBlend[4]) const
{
	outBlend[0] = blend[0];
	outBlend[1] = blend[1];
	outBlend[2] = blend[2];
	outBlend[3] = blend[3];
}

float playerState_t::getFov() const
{
	return fov;
}

const_vec3p_t playerState_t::getCameraOrigin() const
{
	return camera_origin;
}

const_vec3p_t playerState_t::getCameraAngles() const
{
	return camera_angles;
}

float playerState_t::getCameraTime() const
{
	return camera_time;
}

const_vec3p_t playerState_t::getCameraOffset() const
{
	return camera_offset;
}

const_vec3p_t playerState_t::getCameraPositionOffset() const
{
	return camera_posofs;
}

uint16_t playerState_t::getCameraFlags() const
{
	return camera_flags;
}

const_vec3p_t playerState_t::getDamageAngles() const
{
	return damage_angles;
}

radarInfo_t playerState_t::getRadarInfo() const
{
	return radarInfo;
}

bool playerState_t::hasVoted() const
{
	return bVoted;
}

