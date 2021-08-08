#pragma once

#include "../NetGlobal.h"
#include "../../Utility/DynamicEnum.h"
#include "../../Utility/TickTypes.h"
#include "NetTime.h"
#include "Angles.h"
#include "UserWeaponCommands.h"
#include "UserButtons.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	/**
	 * Manages buttons and weapon commands.
	 */
	class MOHPC_NET_EXPORTS UserActionInput
	{
	public:
		UserActionInput();

		/** Add a weapon command. */
		void addWeaponCommand(const WeaponCommand& weaponCommand);
		/** Remove a weapon command. */
		void removeWeaponCommand(const WeaponCommand& weaponCommand);

		/** Check if a button is held. */
		bool isHeld(const UserButton& button);

		/** Check if any of specified user buttons is present. */
		template<typename...Args>
		bool checkAnyHeld(Args&&... button)
		{
			return (isHeld(std::forward<Args>(button)) || ...);
		}

		/** Check if all user buttons are present. */
		template<typename...Args>
		bool checkAllHeld(Args&&... button)
		{
			return (isHeld(std::forward<Args>(button)) && ...);
		}

		void addButton(const UserButton& newButton);
		void removeButton(const UserButton& button);

		/** Return this action flags. */
		uint16_t getFlags() const;
		/** Set action flags. */
		void setFlags(uint16_t newFlags);

	private:
		uint16_t flags;
	};

	/**
	 * Represents the user movement and rotation.
	 */
	class MOHPC_NET_EXPORTS UserMovementInput
	{
	public:
		UserMovementInput();

		/** Converts and set angles. */
		void setAngles(float pitch, float yaw, float roll);
		void setAngles(uint16_t pitch, uint16_t yaw, uint16_t roll);
		void setAngles(const netAngles_t& newAngles);

		/** Get the user input angles. */
		void getAngles(netAngles_t& anglesRef);
		const netAngles_t& getAngles() const;

		/** Move forward by the specified value. Range [-128, 127]. */
		void moveForward(int8_t value);
		/** Move right by the specified value. Range [-128, 127]. */
		void moveRight(int8_t value);
		/** Move up by the specified value. Range [-128, 127]. */
		void moveUp(int8_t value);
		/** Clear all movement. */
		void clear();

		/** Get the forward value. Range [-128, 127]. */
		int8_t getForwardValue() const;
		/** Get the right value. Range [-128, 127]. */
		int8_t getRightValue() const;
		/** Get the up value. Range [-128, 127]. */
		int8_t getUpValue() const;

	private:
		netAngles_t angles;
		int8_t forwardmove, rightmove, upmove;
	};

	/** Jump. Same as calling moveUp with a value of 127. */
	class MOHPC_NET_EXPORTS UserExecuteMovementJump
	{
	public:
		static void execute(UserMovementInput& mInput);
	};

	/** Crouch. Same as calling moveUp with a value of -128. */
	class MOHPC_NET_EXPORTS UserExecuteMovementCrouch
	{
	public:
		static void execute(UserMovementInput& mInput);
	};

	void setUserRelativeAngles(UserMovementInput& mInput, const netAngles_t& angles, float pitch, float yaw, float roll);

	/**
	 * Covers user movement and input.
	 */
	class usercmd_t
	{
	public:
		MOHPC_NET_EXPORTS usercmd_t();
		MOHPC_NET_EXPORTS usercmd_t(tickTime_t serverTimeValue);
		MOHPC_NET_EXPORTS usercmd_t(const usercmd_t& other);
		MOHPC_NET_EXPORTS usercmd_t& operator=(const usercmd_t& other);

		/** Return the movement input. */
		MOHPC_NET_EXPORTS const UserMovementInput& getMovement() const;
		MOHPC_NET_EXPORTS UserMovementInput& getMovement();

		/** Return the action input. */
		MOHPC_NET_EXPORTS const UserActionInput& getAction() const;
		MOHPC_NET_EXPORTS UserActionInput& getAction();

		MOHPC_NET_EXPORTS tickTime_t getServerTime() const;
		MOHPC_NET_EXPORTS void setServerTime(tickTime_t newTime);

	private:
		tickTime_t serverTime;
		UserMovementInput movement;
		UserActionInput action;
	};

	class MOHPC_NET_EXPORTS usereyes_t
	{
	public:
		/** Pitch and yaw of eyes. */
		float angles[2];

		/** Position of eyes. */
		int8_t ofs[3];

	public:
		usereyes_t();

		/** Set the offset of eyes view. The offset is the head position starting from the player's origin (feet). */
		void setOffset(int8_t x, int8_t y, int8_t z);

		/** Return the view offset. */
		void getOffset(int8_t xyz[3]);

		/** Set the eyes angles. */
		void setAngles(float pitch, float yaw);

		/** Return the eyes angles. */
		void getAngles(float& pitch, float& yaw);
	};
}
}