#include <Shared.h>
#include <MOHPC/Script/World.h>
#include <MOHPC/Script/Level.h>
#include <MOHPC/Managers/GameManager.h>
#include <MOHPC/Managers/ScriptManager.h>
#include <MOHPC/Script/ScriptException.h>

using namespace MOHPC;

Event EV_World_SetSoundtrack
(
	"soundtrack",
	EV_DEFAULT,
	"s",
	"MusicFile",
	"Set music soundtrack for this level."
);
Event EV_World_SetGravity
(
	"gravity",
	EV_DEFAULT,
	"f",
	"worldGravity",
	"Set the gravity for the whole world."
);
Event EV_World_SetNextMap
(
	"nextmap",
	EV_DEFAULT,
	"s",
	"nextMap",
	"Set the next map to change to"
);
Event EV_World_SetMessage
(
	"message",
	EV_DEFAULT,
	"s",
	"worldMessage",
	"Set a message for the world"
);
Event EV_World_SetWaterColor
(
	"watercolor",
	EV_DEFAULT,
	"v",
	"waterColor",
	"Set the watercolor screen blend"
);
Event EV_World_SetWaterAlpha
(
	"wateralpha",
	EV_DEFAULT,
	"f",
	"waterAlpha",
	"Set the alpha of the water screen blend"
);
Event EV_World_SetLavaColor
(
	"lavacolor",
	EV_DEFAULT,
	"v",
	"lavaColor",
	"Set the color of lava screen blend"
);
Event EV_World_SetLavaAlpha
(
	"lavaalpha",
	EV_DEFAULT,
	"f",
	"lavaAlpha",
	"Set the alpha of lava screen blend"
);
Event EV_World_SetFarPlane_Color
(
	"farplane_color",
	EV_DEFAULT,
	"v",
	"farplaneColor",
	"Set the color of the far clipping plane fog"
);
Event EV_World_SetFarPlane_Cull
(
	"farplane_cull",
	EV_DEFAULT,
	"b",
	"farplaneCull",
	"Whether or not the far clipping plane should cull things out of the world"
);
Event EV_World_SetFarPlane
(
	"farplane",
	EV_DEFAULT,
	"f",
	"farplaneDistance",
	"Set the distance of the far clipping plane"
);
Event EV_World_SetAmbientLight
(
	"ambientlight",
	EV_DEFAULT,
	"b",
	"ambientLight",
	"Set whether or not ambient light should be used"
);
Event EV_World_SetAmbientIntensity
(
	"ambient",
	EV_DEFAULT,
	"f",
	"ambientIntensity",
	"Set the intensity of the ambient light"
);
Event EV_World_SetSunColor
(
	"suncolor",
	EV_DEFAULT,
	"v",
	"sunColor",
	"Set the color of the sun"
);
Event EV_World_SetSunLight
(
	"sunlight",
	EV_DEFAULT,
	"b",
	"sunlight",
	"Set whether or not there should be sunlight"
);
Event EV_World_SetSunDirection
(
	"sundirection",
	EV_DEFAULT,
	"v",
	"sunlightDirection",
	"Set the direction of the sunlight"
);
Event EV_World_LightmapDensity
(
	"lightmapdensity",
	EV_DEFAULT,
	"f",
	"density",
	"Set the default lightmap density for all world surfaces"
);
Event EV_World_SunFlare
(
	"sunflare",
	EV_DEFAULT,
	"v",
	"position_of_sun",
	"Set the position of the sun for the purposes of the sunflare"
);
Event EV_World_SunFlareInPortalSky
(
	"sunflare_inportalsky",
	EV_DEFAULT,
	NULL,
	NULL,
	"Let the renderer know that the sun is in the portal sky"
);
Event EV_World_SetSkyAlpha
(
	"skyalpha",
	EV_DEFAULT,
	"f",
	"newAlphaForPortalSky",
	"Set the alpha on the sky"
);
Event EV_World_SetSkyPortal
(
	"skyportal",
	EV_DEFAULT,
	"b",
	"newSkyPortalState",
	"Whether or not to use the sky portal at all"
);
Event EV_World_SetNorthYaw
(
	"northyaw",
	EV_DEFAULT,
	"f",
	"yaw",
	"Sets the yaw direction that is considered to be north"
);

void World::AddTargetEntity( SimpleEntity *ent )
{
	str targetname = ent->TargetName();

	if( !targetname.length() )
	{
		return;
	}

	ConSimple* list = GetTargetList( targetname );

	list->AddObject( ent );
}

void World::AddTargetEntityAt( SimpleEntity *ent, int index )
{
	str targetname = ent->TargetName();

	if( !targetname.length() )
	{
		return;
	}

	ConSimple* list = GetTargetList( targetname );

	list->AddObjectAt( index, ent );
}

void World::RemoveTargetEntity( SimpleEntity *ent )
{
	str targetname = ent->TargetName();

	if( !targetname.length() )
	{
		return;
	}

	ConSimple* list = GetExistingTargetList( targetname );

	if( list )
	{
		list->RemoveObject( ent );

		if( list->NumObjects() <= 0 )
		{
			m_targetList.remove(GetScriptManager()->AddString( targetname ) );
		}
	}
}

void World::FreeTargetList()
{
	m_targetList.clear();
}

SimpleEntity *World::GetNextEntity( str targetname, SimpleEntity *ent )
{
	return GetNextEntity( GetScriptManager()->AddString( targetname ), ent );
}

SimpleEntity *World::GetNextEntity( const_str targetname, SimpleEntity *ent )
{
	ConSimple* list = GetTargetList( targetname );
	uintptr_t index;

	if( ent )
	{
		index = list->IndexOfObject( ent ) + 1;
	}
	else
	{
		index = 1;
	}

	if( list->NumObjects() >= index )
	{
		return list->ObjectAt( index );
	}
	else
	{
		return NULL;
	}
}

SimpleEntity *World::GetScriptTarget( str targetname )
{
	return GetScriptTarget( GetScriptManager()->AddString( targetname ) );
}

SimpleEntity *World::GetScriptTarget( const_str targetname )
{
	ConSimple* list = GetTargetList( targetname );

	if( list->NumObjects() == 1 )
	{
		return list->ObjectAt( 1 );
	}
	else if( list->NumObjects() > 1 )
	{
		ScriptError( "There are %d entities with targetname '%s'. You are using a command that requires exactly one.", list->NumObjects(), GetScriptManager()->GetString( targetname ).c_str() );
	}

	return NULL;
}

SimpleEntity *World::GetTarget( str targetname, bool quiet )
{
	return GetTarget( GetScriptManager()->AddString( targetname ), quiet );
}

SimpleEntity *World::GetTarget( const_str targetname, bool quiet )
{
	ConSimple* list = GetTargetList( targetname );

	if( list->NumObjects() == 1 )
	{
		return list->ObjectAt( 1 );
	}
	else if( list->NumObjects() > 1 )
	{
		if( !quiet ) {
			warning( "World::GetTarget", "There are %d entities with targetname '%s'. You are using a command that requires exactly one.", list->NumObjects(), GetScriptManager()->GetString( targetname ).c_str() );
		}
	}

	return NULL;
}

uintptr_t World::GetTargetnameIndex( SimpleEntity *ent )
{
	ConSimple* list = GetTargetList( ent->TargetName() );

	return list->IndexOfObject( ent );
}

ConSimple *World::GetExistingTargetList( const str& targetname )
{
	return GetExistingTargetList( GetScriptManager()->AddString( targetname ) );
}

ConSimple *World::GetExistingTargetList( const_str targetname )
{
	return m_targetList.findKeyValue( targetname );
}

ConSimple *World::GetTargetList( const str& targetname )
{
	return GetTargetList( GetScriptManager()->AddString( targetname ) );
}

ConSimple *World::GetTargetList( const_str targetname )
{
	return &m_targetList.addKeyValue( targetname );
}

World::World()
{
	GetGameManager()->world = this;
	world_dying = false;

	// set the default gravity
	m_fGravity = 800;

	// set the default farplane parameters
	farplane_distance = 0;
	farplane_color = "0 0 0";
	farplane_cull = true;

	UpdateFog();

	sky_portal = true;
	UpdateSky();

	m_fAIVisionDistance = 2048.0f;

	//
	// set the targetname of the world
	//
	SetTargetName( "world" );

	m_fNorth = 0;
}

void World::UpdateConfigStrings( void )
{
}

void World::UpdateFog( void )
{
}

void World::UpdateSky( void )
{
}


void World::SetSoundtrack( Event *ev )
{
	soundtrack = ev->GetString(1);
}

void World::SetGravity( Event *ev )
{
	m_fGravity = ev->GetFloat(1);
}

void World::SetFarPlane( Event *ev )
{
	farplane_distance = ev->GetFloat( 1 );
	UpdateFog();
}

void World::SetFarPlane_Color( Event *ev )
{
	farplane_color = ev->GetVector( 1 );
	UpdateFog();
}

void World::SetFarPlane_Cull( Event *ev )
{
	farplane_cull = ev->GetBoolean( 1 );
	UpdateFog();
}

void World::SetSkyAlpha( Event *ev )
{
	sky_alpha = ev->GetFloat( 1 );
	UpdateSky();
}

void World::SetSkyPortal( Event *ev )
{
	sky_portal = ev->GetBoolean( 1 );
	UpdateSky();
}

void World::SetNextMap( Event *ev )
{
	GetLevel()->nextmap = ev->GetString( 1 );
}

void World::SetMessage( Event *ev )
{
	message = ev->GetString(1);
}

void World::SetWaterColor( Event *ev )
{
	GetLevel()->water_color = ev->GetVector( 1 );
}

void World::SetWaterAlpha( Event *ev )
{
	GetLevel()->water_alpha = ev->GetFloat( 1 );
}
void World::SetLavaColor( Event *ev )
{
	GetLevel()->lava_color = ev->GetVector( 1 );
}

void World::SetLavaAlpha( Event *ev )
{
	GetLevel()->lava_alpha = ev->GetFloat( 1 );
}

void World::SetNorthYaw( Event *ev )
{
	m_fNorth = AngleMod(ev->GetFloat(1));
}

CLASS_DECLARATION(SimpleEntity, World, "worldspawn")
{
	{ &EV_World_SetSoundtrack,			&World::SetSoundtrack },
	{ &EV_World_SetGravity,				&World::SetGravity },
	{ &EV_World_SetNextMap,				&World::SetNextMap },
	{ &EV_World_SetMessage,				&World::SetMessage },
	{ &EV_World_SetWaterColor,			&World::SetWaterColor },
	{ &EV_World_SetWaterAlpha,			&World::SetWaterAlpha },
	{ &EV_World_SetLavaColor,			&World::SetLavaColor },
	{ &EV_World_SetLavaAlpha,			&World::SetLavaAlpha },
	{ &EV_World_SetFarPlane_Color,		&World::SetFarPlane_Color },
	{ &EV_World_SetFarPlane_Cull,		&World::SetFarPlane_Cull },
	{ &EV_World_SetFarPlane,			&World::SetFarPlane },
	{ &EV_World_SetSkyAlpha,			&World::SetSkyAlpha },
	{ &EV_World_SetSkyPortal,			&World::SetSkyPortal },
	{ &EV_World_SetNorthYaw,			&World::SetNorthYaw },
	{ &EV_World_SetAmbientLight,		NULL },
	{ &EV_World_SetAmbientIntensity,	NULL },
	{ &EV_World_SetSunColor,			NULL },
	{ &EV_World_SetSunLight,			NULL },
	{ &EV_World_SetSunDirection,		NULL },
	{ &EV_World_LightmapDensity,		NULL },
	{ &EV_World_SunFlare,				NULL },
	{ &EV_World_SunFlareInPortalSky,	NULL },
	{ NULL,								NULL }
};
