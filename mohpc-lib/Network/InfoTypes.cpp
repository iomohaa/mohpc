#include <MOHPC/Network/InfoTypes.h>
#include <MOHPC/Network/Types.h>

using namespace MOHPC;
using namespace Network;

const char Network::CLIENT_VERSION[] = "3.00";

NetAddr::NetAddr()
	: port(0)
{}

void NetAddr::setPort(uint16_t value)
{
	port = value;
}

uint16_t NetAddr::getPort() const
{
	return port;
}

bool NetAddr::operator==(const NetAddr& other) const
{
	const size_t sz = getAddrSize();
	if(sz != other.getAddrSize()) {
		return false;
	}

	const uint8_t* buf = getAddress();
	return !memcmp(buf, other.getAddress(), sz);
}

bool NetAddr::operator!=(const NetAddr& other) const
{
	return !(*this == other);
}

MOHPC_OBJECT_DEFINITION(NetAddr4);

NetAddr4::NetAddr4()
	: ip{ 0 }
{
}

void NetAddr4::setIp(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	ip[0] = a;
	ip[1] = b;
	ip[2] = c;
	ip[3] = d;
}

size_t NetAddr4::getAddrSize() const
{
	return sizeof(ip);
}

const uint8_t* NetAddr4::getAddress() const
{
	return ip;
}

str NetAddr4::asString() const
{
	str adrBuf;
	adrBuf.reserve(sizeof(ip));
	for (size_t i = 0; i < sizeof(ip); ++i)
	{
		if (i != 0) adrBuf += '.';
		adrBuf += ip[i];
	}

	return adrBuf;
}

MOHPC_OBJECT_DEFINITION(NetAddr6);

NetAddr6::NetAddr6()
	: ip{0}
{
}

void NetAddr6::setIp(uint16_t a, uint16_t b, uint16_t c, uint16_t d, uint16_t e, uint16_t f, uint16_t g, uint16_t h)
{
	ip[0] = a & 0xFF; ip[1] = (a >> 8) & 0xFF;
	ip[2] = b & 0xFF; ip[3] = (b >> 8) & 0xFF;
	ip[4] = c & 0xFF; ip[5] = (c >> 8) & 0xFF;
	ip[6] = d & 0xFF; ip[7] = (d >> 8) & 0xFF;
	ip[8] = e & 0xFF; ip[9] = (e >> 8) & 0xFF;
	ip[10] = f & 0xFF; ip[11] = (f >> 8) & 0xFF;
	ip[12] = g & 0xFF; ip[13] = (g >> 8) & 0xFF;
	ip[14] = h & 0xFF; ip[15] = (h >> 8) & 0xFF;
}

size_t NetAddr6::getAddrSize() const
{
	return sizeof(ip);
}

const uint8_t* NetAddr6::getAddress() const
{
	return ip;
}

str NetAddr6::asString() const
{
	str adrBuf;
	adrBuf.reserve(sizeof(ip));
	for (size_t i = 0; i < sizeof(ip); ++i)
	{
		if (i != 0) adrBuf += '.';
		adrBuf += ip[i];
	}

	return adrBuf;
}

usercmd_t::usercmd_t(uint32_t inServerTime)
	: usercmd_t()
{
	serverTime = inServerTime;
}

usercmd_t::usercmd_t()
	: serverTime(0)
	, buttons{ 0 }
	, angles{ 0 }
	, forwardmove(0)
	, rightmove(0)
	, upmove(0)
{
}

int8_t usercmd_t::getForwardValue() const
{
	return forwardmove;
}

int8_t usercmd_t::getRightValue() const
{
	return rightmove;
}

int8_t usercmd_t::getUpValue() const
{
	return upmove;
}

void usercmd_t::getAngles(uint16_t& pitch, uint16_t& yaw, uint16_t& roll)
{
	pitch = angles[0];
	yaw = angles[1];
	roll = angles[2];
}

usercmd_t::buttons_t usercmd_t::getButtons() const
{
	return buttons;
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

void usercmd_t::setAnglesRelativeTo(const playerState_t& ps, float pitch, float yaw, float roll)
{
	angles[0] = AngleToShort(pitch) - ps.delta_angles[0];
	angles[1] = AngleToShort(yaw) - ps.delta_angles[1];
	angles[2] = AngleToShort(roll) + ps.delta_angles[2];
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

void usereyes_t::getOffset(int8_t xyz[3])
{
	xyz[0] = ofs[0];
	xyz[1] = ofs[1];
	xyz[2] = ofs[2];
}

void usereyes_t::setAngles(float pitch, float yaw)
{
	angles[0] = pitch;
	angles[1] = yaw;
}

void usereyes_t::getAngles(float& pitch, float& yaw)
{
	pitch = angles[0];
	yaw = angles[1];
}

radarInfo_t::radarInfo_t()
	: radarValue(0)
{

}

radarInfo_t::radarInfo_t(uint32_t value)
	: radarValue(value)
{
}

int8_t radarInfo_t::clientNum() const
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

playerState_t::playerState_t()
	: commandTime(0)
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

uint32_t playerState_t::getCommandTime() const
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

uint16_t playerState_t::getPlayerMoveTime() const
{
	return pm_time;
}

uint8_t playerState_t::getBobCycle() const
{
	return bobCycle;
}

const Vector& playerState_t::getOrigin() const
{
	return origin;
}

const Vector& playerState_t::getVelocity() const
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

void playerState_t::getDeltaAngles(uint16_t angles[3]) const
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

const Vector& playerState_t::getFalldir() const
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

const Vector& playerState_t::getViewAngles() const
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

const Vector& playerState_t::getCameraOrigin() const
{
	return camera_origin;
}

const Vector& playerState_t::getCameraAngles() const
{
	return camera_angles;
}

float playerState_t::getCameraTime() const
{
	return camera_time;
}

const Vector& playerState_t::getCameraOffset() const
{
	return camera_offset;
}

const Vector& playerState_t::getCameraPositionOffset() const
{
	return camera_posofs;
}

uint16_t playerState_t::getCameraFlags() const
{
	return camera_flags;
}

const Vector& playerState_t::getDamageAngles() const
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

trajectory_t::trajectory_t()
	: trTime(0)
{}

const Vector& trajectory_t::getDelta() const
{
	return trDelta;
}

uint32_t trajectory_t::getTime() const
{
	return trTime;
}

frameInfo_t::frameInfo_t()
	: index(0)
	, time(0.f)
	, weight(0.f)
{}

uint32_t frameInfo_t::getIndex() const
{
	return index;
}

float frameInfo_t::getTime() const
{
	return time;
}

float frameInfo_t::getWeight() const
{
	return weight;
}

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
{
}

entityNum_t entityState_t::getNumber() const
{
	return number;
}

entityType_e entityState_t::getType() const
{
	return eType;
}

uint16_t entityState_t::getEntityFlags() const
{
	return eFlags;
}

const trajectory_t& entityState_t::getTrajectory() const
{
	return pos;
}

const Vector& entityState_t::getNetOrigin() const
{
	return netorigin;
}

const MOHPC::Vector& entityState_t::getAlternateOrigin() const
{
	return origin2;
}

const Vector& entityState_t::getNetAngles() const
{
	return netangles;
}

uint32_t entityState_t::getConstantLight() const
{
	return constantLight;
}

void entityState_t::getConstantLight(uint8_t& red, uint8_t& green, uint8_t& blue, uint8_t& intensity)
{
	red			= (constantLight & 0xFF);
	green		= (constantLight >> 8) & 0xFF;
	blue		= (constantLight >> 16) & 0xFF;
	intensity	= (constantLight >> 24) & 0xFF;
}

bool entityState_t::doesLoopsound() const
{
	return loopSound;
}

float entityState_t::getLoopsoundVolume() const
{
	return loopSoundVolume;
}

float entityState_t::getLoopsoundMinDist() const
{
	return loopSoundMinDist;
}

float entityState_t::getLoopsoundMaxDist() const
{
	return loopSoundMaxDist;
}

uint32_t entityState_t::getLoopsoundFlags() const
{
	return loopSoundFlags;
}

entityNum_t entityState_t::getParent() const
{
	return parent;
}

uint16_t entityState_t::getTagNum() const
{
	return tag_num;
}

bool entityState_t::doesAttachUseAngle() const
{
	return attach_use_angles;
}

const Vector& entityState_t::getAttachOffset() const
{
	return attach_offset;
}

uint16_t entityState_t::getBeamEntityNum() const
{
	return beam_entnum;
}

uint16_t entityState_t::getModelIndex() const
{
	return modelindex;
}

uint16_t entityState_t::getUsageIndex() const
{
	return usageIndex;
}

uint16_t entityState_t::getSkinNum() const
{
	return skinNum;
}

uint16_t entityState_t::getWasFrame() const
{
	return wasframe;
}

const frameInfo_t& entityState_t::getFrameInfo(uint8_t frameIndex) const
{
	return frameInfo[frameIndex];
}

float entityState_t::getActionWeight() const
{
	return actionWeight;
}

uint8_t entityState_t::getBoneTag(uint8_t index) const
{
	return bone_tag[index];
}

const Vector& entityState_t::getBoneAngles(uint8_t index)
{
	return bone_angles[index];
}

uint8_t entityState_t::getSurface(uint8_t surfaceIndex) const
{
	return surfaces[surfaceIndex];
}

uint8_t entityState_t::getClientNum() const
{
	return clientNum;
}

uint16_t entityState_t::getGroundEntityNum() const
{
	return groundEntityNum;
}

uint32_t entityState_t::getSolid() const
{
	return solid;
}

float entityState_t::getScale() const
{
	return scale;
}

float entityState_t::getAlpha() const
{
	return alpha;
}

uint32_t entityState_t::getRenderFlags() const
{
	return renderfx;
}

float entityState_t::getShaderData(uint8_t shaderIndex) const
{
	return shader_data[shaderIndex];
}

float entityState_t::getShaderTime() const
{
	return shader_time;
}

Network::SnapshotInfo::SnapshotInfo()
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

const entityState_t* sound_t::getEntityState() const
{
	return entity;
}

const char* sound_t::getName() const
{
	return soundName;
}

const Vector& sound_t::getOrigin() const
{
	return origin;
}

float sound_t::getVolume() const
{
	return volume;
}

float sound_t::getMinimumDistance() const
{
	return minDist;
}

float sound_t::getMaximumDistance() const
{
	return maxDist;
}

float sound_t::getPitch()
{
	return pitch;
}

uint8_t sound_t::getChannel() const
{
	return channel;
}

bool sound_t::hasSoundStopped() const
{
	return hasStopped;
}

bool sound_t::isStreamedSound() const
{
	return isStreamed;
}

bool sound_t::isSpatializedSound() const
{
	return isSpatialized;
}

bool sound_t::hasSoundVolume() const
{
	return hasVolume;
}

bool sound_t::hasSoundDistance() const
{
	return hasDist;
}

bool sound_t::hasSoundPitch() const
{
	return hasPitch;
}

protocolType_c::protocolType_c()
	: serverType(serverType_e::none)
	, protocolVersion(protocolVersion_e::bad)
{}

protocolType_c::protocolType_c(serverType_e inServerType, protocolVersion_e inProtocolVersion)
	: serverType(inServerType)
	, protocolVersion(inProtocolVersion)
{}

serverType_e protocolType_c::getServerType() const
{
	return serverType;
}

protocolVersion_e protocolType_c::getProtocolVersion() const
{
	return protocolVersion;
}

size_t SnapshotInfo::getNumEntities() const
{
	return numEntities;
}

size_t SnapshotInfo::getNumSounds() const
{
	return numSounds;
}

size_t SnapshotInfo::getNumServerCommands()
{
	return numServerCommands;
}

size_t SnapshotInfo::getServerCommandSequence() const
{
	return serverCommandSequence;
}

uint32_t SnapshotInfo::getSnapFlags() const
{
	return snapFlags;
}

uint32_t SnapshotInfo::getPing() const
{
	return ping;
}

uint32_t SnapshotInfo::getServerTime() const
{
	return serverTime;
}

const playerState_t& SnapshotInfo::getPlayerState() const
{
	return ps;
}

uint8_t SnapshotInfo::getAreaMask(uint8_t index) const
{
	return areamask[index];
}

const entityState_t& SnapshotInfo::getEntityState(entityNum_t index) const
{
	return entities[index];
}

const entityState_t* MOHPC::Network::SnapshotInfo::getEntityStateByNumber(entityNum_t entityNum) const
{
	for (size_t i = 0; i < numEntities; ++i)
	{
		const entityState_t& es = entities[i];
		if (es.number == entityNum)
		{
			// found the entity
			return &es;
		}
	}

	return nullptr;
}

const sound_t& SnapshotInfo::getSound(uint8_t index) const
{
	return sounds[index];
}
