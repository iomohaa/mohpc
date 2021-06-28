#pragma once

#include "../NetGlobal.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	enum class weaponCommand_e : unsigned char
	{
		none,
		usePistol,
		useRifle,
		useSmg,
		useMg,
		useGrenade,
		useHeavy,
		useItem,
		useItem2,
		useItem3,
		useItem4,
		prevWeapon,
		nextWeapon,
		useLast,
		holster,
		drop,
		max
	};

	static const unsigned int BUTTON_ATTACK_PRIMARY = (1 << 0);
	static const unsigned int BUTTON_ATTACK_SECONDARY = (1 << 1);
	static const unsigned int BUTTON_RUN = (1 << 2);
	static const unsigned int BUTTON_USE = (1 << 3);
	static const unsigned int BUTTON_LEANLEFT = (1 << 4);
	static const unsigned int BUTTON_LEANRIGHT = (1 << 5);
	static const unsigned int WEAPONCOMMAND_MASK = ((unsigned int)weaponCommand_e::max) - 1;

	class playerState_t;

	class MOHPC_NET_EXPORTS usercmd_t
	{
	public:
		uint32_t serverTime;
		struct buttons_t {
			uint16_t flags;
		} buttons;
		uint16_t angles[3];
		int8_t forwardmove, rightmove, upmove;

	public:
		usercmd_t();
		usercmd_t(uint32_t inServerTime);

		/** Get the user angles. */
		void getAngles(uint16_t& pitch, uint16_t& yaw, uint16_t& roll);

		/** Get the forward value. Range [-128, 127]. */
		int8_t getForwardValue() const;
		/** Get the right value. Range [-128, 127]. */
		int8_t getRightValue() const;
		/** Get the up value. Range [-128, 127]. */
		int8_t getUpValue() const;

		/** Sends a weapon command. */
		void setWeaponCommand(weaponCommand_e weaponCommand);

		/** Converts and set angles. */
		void setAngles(float pitch, float yaw, float roll);

		/** Set angles relative to player's deltaAngles. */
		void setAnglesRelativeTo(const playerState_t& ps, float pitch, float yaw, float roll);

		/** Move forward by the specified value. Range [-128, 127]. */
		void moveForward(int8_t value);

		/** Move right by the specified value. Range [-128, 127]. */
		void moveRight(int8_t value);

		/** Move up by the specified value. Range [-128, 127]. */
		void moveUp(int8_t value);

		/** Jump. Same as calling moveUp with a value of 127. */
		void jump();

		/** Crouch. Same as calling moveUp with a value of -128. */
		void crouch();

		/** Return buttons that are held. */
		buttons_t getButtons() const;

		void setButtonFlags(uint32_t flags);
		void removeButtonFlags(uint32_t flags);
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