#pragma once

#include <MOHPC/Formats/TIKI.h>
#include <MOHPC/Vector.h>
#include <MOHPC/Utilities/SharedPtr.h>
#include <MOHPC/Common/con_set.h>
#include "TIKI_Script.h"

// skb skeletor
static constexpr unsigned char	TIKI_SKB_HEADER_IDENT[]			= "SKL ";
static constexpr unsigned int	TIKI_SKB_HEADER_VER_3			= 3;
static constexpr unsigned int	TIKI_SKB_HEADER_VERSION			= 4;

// skd skeletor
static constexpr unsigned char	TIKI_SKD_HEADER_IDENT[]			= "SKMD";
static constexpr unsigned int	TIKI_SKD_HEADER_OLD_VERSION		= 5;
static constexpr unsigned int	TIKI_SKD_HEADER_VERSION			= 6;

// skc skeletor animations
static constexpr unsigned char	TIKI_SKC_HEADER_IDENT[]			= "SKAN";
static constexpr unsigned int	TIKI_SKC_HEADER_OLD_VERSION		= 13;
static constexpr unsigned int	TIKI_SKC_HEADER_VERSION			= 14;

// tiki limitations
static constexpr unsigned int TIKI_MAX_BONES		= 100;
static constexpr unsigned int TIKI_MAX_VERTEXES		= 1000;
static constexpr unsigned int TIKI_MAX_TRIANGLES	= 2000;
static constexpr unsigned int TIKI_MAX_SKELCACHE	= 1024;
static constexpr unsigned int TIKI_MAX_COMMANDS		= 128;

namespace MOHPC
{
	struct TIKI::dtikicmd_t
	{
		int32_t frame_num;
		MOHPC::Container<MOHPC::str> args;
	};

	struct TIKI::dloadframecmd_t 
	{
		int frame_num;
		MOHPC::Container<MOHPC::str> args;
		MOHPC::str location;
	};

	struct TIKI::dloadinitcmd_t
	{
		MOHPC::Container<MOHPC::str> args;
	};

	struct TIKI::dloadsurface_t
	{
		MOHPC::str name;
		MOHPC::Container<MOHPC::str> shader;
		int flags;
		float damage_multiplier;
	};

	struct TIKI::dloadanim_t
	{
		MOHPC::str alias;
		MOHPC::str name;
		MOHPC::str location;
		float weight;
		float blendtime;
		int32_t flags;
		MOHPC::Container<dloadframecmd_t> loadservercmds;
		MOHPC::Container<dloadframecmd_t> loadclientcmds;
	};

	struct TIKI::dloaddef_t
	{
		MOHPC::str path;
		SharedPtr<TikiScript> tikiFile;

		con_set<MOHPC::str, MOHPC::str> keyvalues;
		MOHPC::Container<dloadanim_t> loadanims;
		MOHPC::Container<dloadinitcmd_t> loadserverinitcmds;
		MOHPC::Container<dloadinitcmd_t> loadclientinitcmds;

		int32_t skelIndex_ld[12];
		MOHPC::Container<MOHPC::str> headmodels;
		MOHPC::Container<MOHPC::str> headskins;
		bool bIsCharacter;

		struct
		{
			float load_scale;
			float lod_scale;
			float lod_bias;
			MOHPC::Container<MOHPC::str> skelmodel;
			Vector origin;
			Vector lightoffset;
			float radius;
			MOHPC::Container<dloadsurface_t> surfaces;
		} loaddata;

		bool bInIncludesSection;
	};
};
