#pragma once

#include "../NetGlobal.h"
#include "../ProtocolSingleton.h"
#include "../Exception.h"
#include "../../Utility/Misc/MSG/Serializable.h"

namespace MOHPC
{
namespace Network
{
	class entityState_t;
	struct rawSnapshot_t;
	struct gameState_t;
	class ICommandSequence;

	namespace Parsing
	{
		struct snapshotParm_t
		{
			const rawSnapshot_t* oldSnapshots;
			const rawSnapshot_t* oldSnap;
			entityState_t* parseEntities;
			size_t numOldSnapshots;
			size_t maxParseEntities;
			size_t maxSnapshotEntities;
			uint32_t parseEntitiesNum;
			float deltaTime;
		};

		class MOHPC_NET_EXPORTS ISnapshot : public IProtocolSingleton<ISnapshot>
		{
		public:
			virtual void parseSnapshot(
				uint64_t currentTime,
				MSG& msg,
				const gameState_t& gameState,
				ICommandSequence* serverCommands,
				rawSnapshot_t& rawSnapshot,
				snapshotParm_t& snapshotParm,
				uint32_t serverMessageSequence
			) const = 0;
		};
	}

	namespace SnapshotError
	{
		class Base : public NetworkException {};

		/**
		 * Bad area mask size while parsing snapshot.
		 */
		class AreaMaskBadSize : public Base
		{
		public:
			AreaMaskBadSize(uint8_t inSize);

			/** Return the size of the area mask. */
			MOHPC_NET_EXPORTS uint8_t getSize() const;
			MOHPC_NET_EXPORTS str what() const override;

		private:
			uint8_t size;
		};
	}
}
}