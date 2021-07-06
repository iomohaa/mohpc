#pragma once

#include "../../ProtocolSingleton.h"
#include "Effect.h"

namespace MOHPC
{
class MSG;

namespace Network
{
	namespace CGame
	{
		namespace Messages
		{
			class IBullet;
			class IEffect;
			class IHUD;
			class IImpact;
			class IEvent;
		}

		struct MessageInterfaces
		{
			Messages::IBullet* bullet;
			Messages::IEffect* effect;
			Messages::IHUD* hud;
			Messages::IImpact* impact;
			Messages::IEvent* event;
		};

		class IMessageParser : public IProtocolSingleton<IMessageParser>
		{
		public:
			/**
			 * Parse all game messages.
			 *
			 * @param msg Message to read.
			 * @param interfaces List of interfaces callback for each messages.
			 */
			virtual void parseGameMessage(MSG& msg, const MessageInterfaces& interfaces) const = 0;

		protected:
			/**
			 * Handle a single client game message.
			 *
			 * @param msg Message to read.
			 * @param interfaces List of interfaces callback for each messages.
			 * @param msgId The message number to process.
			 */
			virtual void handleCGMessage(MSG& msg, const MessageInterfaces& interfaces, uint32_t msgId) const = 0;

			/**
			 * Return
			 */
			virtual effects_e getEffectId(uint32_t effectId) const = 0;
		};
	}
}
}