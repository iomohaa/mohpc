#include <MOHPC/Network/Types/UserInput.h>
#include <MOHPC/Network/Types/PlayerState.h>
#include <MOHPC/Common/Math.h>

using namespace MOHPC;
using namespace MOHPC::Network;

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
	buttons.flags |= ((uint16_t)weaponCommand & WEAPONCOMMAND_MASK) << 7;
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

void usercmd_t::setButtonFlags(uint32_t flags)
{
	buttons.flags |= flags;
}

void usercmd_t::removeButtonFlags(uint32_t flags)
{
	buttons.flags &= ~flags;
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
	ofs[2] = z;
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
