#pragma once

#include "../ProtocolSingleton.h"
#include "../Types/PlayerState.h"

namespace MOHPC
{
class MSG;

namespace Network
{
	namespace Parsing
	{
		/**
		 * Abstract player state class for reading and parsing player states.
		 */
		class MOHPC_NET_EXPORTS IPlayerState : public IProtocolSingleton<IPlayerState>
		{
		public:
			virtual void readDeltaPlayerState(MSG& msg, const playerState_t* from, playerState_t* to) const = 0;
			virtual void writeDeltaPlayerState(MSG& msg, const playerState_t* from, const playerState_t* to) const = 0;
		};
	}
}
}