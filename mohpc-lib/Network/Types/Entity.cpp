#include <MOHPC/Network/Types/Entity.h>

using namespace MOHPC;
using namespace MOHPC::Network;

entityState_t::entityState_t()
	: number(0)
	, eType(entityType_e::modelanim_skel)
	, eFlags(0)
	, constantLight(-1)
	, loopSound(0)
	, loopSoundVolume(0.f)
	, loopSoundMinDist(0.f)
	, loopSoundMaxDist(0.f)
	, loopSoundPitch(0.f)
	, loopSoundFlags(0)
	, parent(ENTITYNUM_NONE)
	, tag_num(-1)
	, attach_use_angles(false)
	, beam_entnum(0)
	, modelindex(0)
	, usageIndex(0)
	, skinNum(0)
	, wasframe(0)
	, actionWeight(0.f)
	, surfaces{ 0 }
	, clientNum(0)
	, groundEntityNum(0)
	, solid(0)
	, scale(1.f)
	, alpha(1.f)
	, renderfx(RF_FRAMELERP)
	, shader_data{ 0.f }
	, shader_time(0.f)
{
	for (size_t i = 0; i < NUM_BONE_CONTROLLERS; ++i)
	{
		bone_tag[i] = uint8_t(~0);
	}
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
	red = (constantLight & 0xFF);
	green = (constantLight >> 8) & 0xFF;
	blue = (constantLight >> 16) & 0xFF;
	intensity = (constantLight >> 24) & 0xFF;
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

entityState_t MOHPC::Network::getNullEntityState()
{
	entityState_t nullState;
	nullState.alpha = 1.0f;
	nullState.scale = 1.0f;
	nullState.parent = ENTITYNUM_NONE;
	nullState.tag_num = -1;
	nullState.constantLight = -1;
	nullState.renderfx = 16;
	nullState.bone_tag[4] = -1;
	nullState.bone_tag[3] = -1;
	nullState.bone_tag[2] = -1;
	nullState.bone_tag[1] = -1;
	nullState.bone_tag[0] = -1;
	return nullState;
}

MsgTypesEntityHelper::MsgTypesEntityHelper(MSG& inMsg)
	: MsgBaseHelper(inMsg)
{
}

uint32_t MsgTypesEntityHelper::ReadEntityNum()
{
	const entityNum_t entNum = msg.ReadNumber<entityNum_t>(GENTITYNUM_BITS);
	return entNum & (MAX_GENTITIES - 1);
}

uint32_t MsgTypesEntityHelper::ReadEntityNum2()
{
	const entityNum_t entNum = msg.ReadNumber<entityNum_t>(GENTITYNUM_BITS);
	return (entNum - 1) & (MAX_GENTITIES - 1);
}

void MsgTypesEntityHelper::WriteEntityNum(uint32_t num)
{
	msg.WriteNumber<entityNum_t>(num, GENTITYNUM_BITS);
}

void MsgTypesEntityHelper::WriteEntityNum2(uint32_t num)
{
	msg.WriteNumber<entityNum_t>(num + 1, GENTITYNUM_BITS);
}
