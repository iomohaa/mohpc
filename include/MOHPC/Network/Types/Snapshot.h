#pragma once

#include "../InfoTypes.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	struct rawSnapshot_t
	{
	public:
		int32_t deltaNum;
		uint32_t ping;
		uint32_t serverTime;
		uint32_t messageNum;
		uint32_t numEntities;
		uint32_t parseEntitiesNum;
		uint32_t serverCommandNum;
		uint32_t numSounds;
		uint8_t snapFlags;
		uint8_t serverTimeResidual;
		uint8_t areamask[MAX_MAP_AREA_BYTES];
		uint8_t cmdNum;
		playerState_t ps;
		bool valid;
		sound_t sounds[MAX_SERVER_SOUNDS];

	public:
		rawSnapshot_t();
	};
}
}