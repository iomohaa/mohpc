#pragma once

#include "Listener.h"
#include "../Vector.h"

namespace MOHPC
{
	class SimpleArchivedEntity;

	class Level : public Listener
	{
	private:
		// Level time
		int framenum;
		int inttime;
		int intframetime;

		float time;
		float frametime;

		Container< SimpleArchivedEntity * > m_SimpleArchivedEntities;

	public:
		str	current_map;
		str nextmap;

		// Water variables
		Vector water_color;
		Vector lava_color;
		float water_alpha;
		float lava_alpha;

	public:
		CLASS_PROTOTYPE(Level);

		Level();

		virtual void setTime(int _svsTime_);
		virtual void setFrametime(int frameTime);
		const str& GetMapName() const;
		float GetFrameTime() const;
		float GetTimeSeconds() const;
		void AddSimpleArchivedEntity(SimpleArchivedEntity* Entity);
		void RemoveSimpleArchivedEntity(SimpleArchivedEntity* Entity);
		size_t GetNumSimpleArchivedEntities() const;
		SimpleArchivedEntity* GetSimpleArchivedEntity(uintptr_t num) const;
	};
};
