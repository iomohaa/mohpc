#pragma once

#include "../ProtocolSingleton.h"
#include "../InfoTypes.h"

namespace MOHPC
{
class MSG;

namespace Network
{
	class IGameState;

	namespace Parsing
	{
		/**
		 * Abstract entity class for reading and parsing protocol entity.
		 */
		class MOHPC_NET_EXPORTS IEntity : public IProtocolSingleton<IEntity>
		{
		public:
			virtual entityNum_t readEntityNum(MSG& msg) const = 0;
			virtual void writeEntityNum(MSG& msg, entityNum_t num) const = 0;

			virtual void readDeltaEntity(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum, float deltaTime) const = 0;
			virtual void writeDeltaEntity(MSG& msg, const entityState_t* from, const entityState_t* to, entityNum_t newNum, float deltaTime) const = 0;
		};

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