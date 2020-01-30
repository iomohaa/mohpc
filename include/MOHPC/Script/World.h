#pragma once

#include "SimpleEntity.h"

namespace MOHPC
{
#define WORLD_CINEMATIC		1

	typedef Container< SafePtr< SimpleEntity > > ConSimple;

	class World : public SimpleEntity
	{
		con_set < const_str, ConSimple > m_targetList; // moh could have used con_set instead of TargetList
		bool world_dying;

	public:
		// farplane variables
		float farplane_distance;
		Vector farplane_color;
		bool farplane_cull;

		// sky variables
		float sky_alpha;
		bool sky_portal;

		// orientation variables
		float m_fAIVisionDistance;
		float m_fNorth;
		float m_fGravity;

		str soundtrack;
		str message;

	public:
		CLASS_PROTOTYPE(World);

		World();

		void AddTargetEntity(SimpleEntity *ent);
		void AddTargetEntityAt(SimpleEntity *ent, int index);
		void RemoveTargetEntity(SimpleEntity *ent);

		void FreeTargetList();

		SimpleEntity* GetNextEntity(str targetname, SimpleEntity *ent);
		SimpleEntity* GetNextEntity(const_str targetname, SimpleEntity *ent);
		SimpleEntity* GetScriptTarget(str targetname);
		SimpleEntity* GetScriptTarget(const_str targetname);
		SimpleEntity* GetTarget(str targetname, bool quiet);
		SimpleEntity* GetTarget(const_str targetname, bool quiet);
		uintptr_t GetTargetnameIndex(SimpleEntity *ent);

		ConSimple* GetExistingTargetList(const str& targetname);
		ConSimple* GetExistingTargetList(const_str targetname);
		ConSimple* GetTargetList(const str& targetname);
		ConSimple* GetTargetList(const_str targetname);

		virtual void SetSoundtrack(Event *ev);
		virtual void SetGravity(Event *ev);
		virtual void SetNextMap(Event *ev);
		virtual void SetMessage(Event *ev);
		virtual void SetWaterColor(Event *ev);
		virtual void SetWaterAlpha(Event *ev);
		virtual void SetLavaColor(Event *ev);
		virtual void SetLavaAlpha(Event *ev);
		virtual void SetFarPlane_Color(Event *ev);
		virtual void SetFarPlane_Cull(Event *ev);
		virtual void SetFarPlane(Event *ev);
		virtual void SetSkyAlpha(Event *ev);
		virtual void SetSkyPortal(Event *ev);
		virtual void SetNorthYaw(Event *ev);
		virtual void UpdateConfigStrings(void);
		virtual void UpdateFog(void);
		virtual void UpdateSky(void);
	};

	typedef SafePtr<World> WorldPtr;
};

