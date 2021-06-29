#pragma once

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		/**
		 * List of valid game types.
		 */
		enum class gameType_e : unsigned char
		{
			SinglePlayer = 0,
			FreeForAll,
			TeamDeathmatch,
			RoundBasedMatch,
			Objective,
			TugOfWar,
			Liberation
		};

		enum class teamType_e : unsigned char {
			None,
			Spectator,
			FreeForAll,
			Allies,
			Axis
		};
	}
}
}