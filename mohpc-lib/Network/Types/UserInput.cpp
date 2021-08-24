#include <MOHPC/Network/Types/UserInput.h>
#include <MOHPC/Network/Types/PlayerState.h>
#include <MOHPC/Common/Math.h>

using namespace MOHPC;
using namespace MOHPC::Network;

uint32_t GetWeaponCommandMask()
{
	return WeaponCommands::MAX << 7;
}

usercmd_t::usercmd_t()
	: serverTime(std::chrono::milliseconds())
{
}

usercmd_t::usercmd_t(tickTime_t serverTimeValue)
	: serverTime(serverTimeValue)
{
}

usercmd_t::usercmd_t(const usercmd_t& other)
	: serverTime(other.serverTime)
	, movement(other.movement)
	, action(other.action)
{
}

usercmd_t& usercmd_t::operator=(const usercmd_t& other)
{
	serverTime = other.serverTime;
	movement = other.movement;
	action = other.action;
	return *this;
}

tickTime_t usercmd_t::getServerTime() const
{
	return serverTime;
}

void usercmd_t::setServerTime(tickTime_t newTime)
{
	serverTime = newTime;
}

UserMovementInput& usercmd_t::getMovement()
{
	return movement;
}

const UserMovementInput& usercmd_t::getMovement() const
{
	return movement;
}

UserActionInput& usercmd_t::getAction()
{
	return action;
}

const UserActionInput& usercmd_t::getAction() const
{
	return action;
}

UserActionInput::UserActionInput()
	: flags(0)
{
}

bool UserActionInput::isHeld(userButton_t button) const
{
	return flags & button;
}

void UserActionInput::setFlags(uint16_t newFlags)
{
	flags = newFlags;
}

uint16_t UserActionInput::getFlags() const
{
	return flags;
}

void UserActionInput::addWeaponCommand(weaponCommand_t weaponCommand)
{
	flags |= (weaponCommand << 7) & GetWeaponCommandMask();
}

void UserActionInput::removeWeaponCommand(weaponCommand_t weaponCommand)
{
	flags &= ~(weaponCommand << 7) & GetWeaponCommandMask();
}

void UserActionInput::addButton(userButton_t newButton)
{
	flags |= newButton;
}

void UserActionInput::removeButton(userButton_t button)
{
	flags &= ~button;
}

void UserActionInput::removeAllButtons()
{
	flags = 0;
}

void UserActionInput::setOnlyButton(userButton_t button)
{
	flags = button;
}

UserMovementInput::UserMovementInput()
	: angles{ 0 }
	, forwardmove(0)
	, rightmove(0)
	, upmove(0)
{

}

void UserMovementInput::setAngles(float pitch, float yaw, float roll)
{
	setAngles(AngleToShort(pitch), AngleToShort(yaw), AngleToShort(roll));
}

void UserMovementInput::setAngles(uint16_t pitch, uint16_t yaw, uint16_t roll)
{
	setAngles({ pitch, yaw, roll });
}

void UserMovementInput::setAngles(const netAngles_t& newAngles)
{
	angles[0] = newAngles[0];
	angles[1] = newAngles[1];
	angles[2] = newAngles[2];
}

int8_t UserMovementInput::getForwardValue() const
{
	return forwardmove;
}

int8_t UserMovementInput::getRightValue() const
{
	return rightmove;
}

int8_t UserMovementInput::getUpValue() const
{
	return upmove;
}

void UserMovementInput::getAngles(netAngles_t& anglesRef)
{
	anglesRef[0] = angles[0];
	anglesRef[1] = angles[1];
	anglesRef[2] = angles[2];
}

void setUserRelativeAngles(UserMovementInput& mInput, const netAngles_t& angles, float pitch, float yaw, float roll)
{
	mInput.setAngles(
		uint16_t(AngleToShort(pitch) - angles[0]),
		uint16_t(AngleToShort(yaw) - angles[1]),
		uint16_t(AngleToShort(roll) + angles[2])
	);
}

void UserMovementInput::moveForward(int8_t value)
{
	forwardmove = value;
}

void UserMovementInput::moveRight(int8_t value)
{
	rightmove = value;
}

void UserMovementInput::moveUp(int8_t value)
{
	upmove = value;
}

void UserMovementInput::clear()
{
	forwardmove = 0;
	rightmove = 0;
	upmove = 0;
}

void UserExecuteMovementJump::execute(UserMovementInput& mInput)
{
	mInput.moveUp(127);
}

void UserExecuteMovementCrouch::execute(UserMovementInput& mInput)
{
	mInput.moveUp(-128);
}

const netAngles_t& UserMovementInput::getAngles() const
{
	return angles;
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
