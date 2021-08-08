#pragma once

#include "../ProtocolSingleton.h"
#include "../Types/Entity.h"
#include "../../Utility/TickTypes.h"

namespace MOHPC
{
class MSG;

namespace Network
{
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

			virtual void readDeltaEntity(MSG& msg, const entityState_t* from, entityState_t* to, entityNum_t newNum, deltaTimeFloat_t deltaTime) const = 0;
			virtual void writeDeltaEntity(MSG& msg, const entityState_t* from, const entityState_t* to, entityNum_t newNum, deltaTimeFloat_t deltaTime) const = 0;
		};
	}
}
}
