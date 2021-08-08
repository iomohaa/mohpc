#pragma once

#include "Base.h"
#include "../MessageInterfaces/IEvent.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		namespace Handlers
		{
			//=== Deathmatch functions
			/** Called to notify the client of an enemy hit in DM game. */
			struct HitNotify : public HandlerNotifyBase<void()> {};

			/** Called to notify the client of an enemy kill in DM game. */
			struct KillNotify : public HandlerNotifyBase<void()> {};

			/**
			 * Called to play a DM voice message.
			 *
			 * @param	origin		Origin of the message
			 * @param	local		True to play the sound locally. False if it should be spatialized.
			 * @param	clientNum	Player responsible for the voice.
			 * @param	soundName	Sound to play.
			 */
			struct VoiceMessage : public HandlerNotifyBase<void(const vec3r_t origin, bool local, uint8_t clientNum, const char* soundName)> {};
		}

		namespace Messages
		{
			class EventImpl : public IEvent
			{
			public:
				void Hit() override;
				void GotKill() override;
				void PlayVoice(const vec3r_t origin, bool local, uint8_t clientNum, const char* soundName) override;

			public:
				FunctionList<Handlers::HitNotify> hitHandler;
				FunctionList<Handlers::KillNotify> gotKillHandler;
				FunctionList<Handlers::VoiceMessage> voiceMessageHandler;
			};
		}
	}
}
}
