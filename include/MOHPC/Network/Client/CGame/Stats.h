#pragma once

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		struct stats_t
		{
			const char* preferredWeapon;
			uint32_t numObjectives;
			uint32_t numComplete;
			uint32_t numShotsFired;
			uint32_t numHits;
			uint32_t accuracy;
			uint32_t numHitsTaken;
			uint32_t numObjectsDestroyed;
			uint32_t numEnemysKilled;
			uint32_t headshots;
			uint32_t torsoShots;
			uint32_t leftLegShots;
			uint32_t rightLegShots;
			uint32_t groinShots;
			uint32_t leftArmShots;
			uint32_t rightArmShots;
			uint32_t gunneryEvaluation;
			uint32_t gotMedal;
			uint32_t success;
			uint32_t failed;
		};
	}
}
}