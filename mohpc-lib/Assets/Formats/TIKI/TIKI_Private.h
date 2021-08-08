#pragma once

#include <MOHPC/Assets/Formats/TIKI.h>
#include <MOHPC/Common/Vector.h>
#include <MOHPC/Utility/SharedPtr.h>
#include "TIKI_Script.h"

#include <vector>
#include <unordered_map>

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
		frameInt_t frame_num;
		std::vector<str> args;
	};

	struct TIKI::dloadframecmd_t 
	{
		frameInt_t frame_num;
		std::vector<str> args;
		str location;
	};

	struct TIKI::dloadinitcmd_t
	{
		std::vector<str> args;
	};

	struct TIKI::dloadsurface_t
	{
		str name;
		std::vector<str> shader;
		int flags;
		float damage_multiplier;
	};

	struct TIKI::dloadanim_t
	{
		str alias;
		str name;
		str location;
		float weight;
		float blendtime;
		int32_t flags;
		std::vector<dloadframecmd_t> loadservercmds;
		std::vector<dloadframecmd_t> loadclientcmds;
	};

	struct TIKI::dloaddef_t
	{
		str path;
		SharedPtr<TikiScript> tikiFile;

		std::unordered_map<str, str> keyvalues;
		std::vector<dloadanim_t> loadanims;
		std::vector<dloadinitcmd_t> loadserverinitcmds;
		std::vector<dloadinitcmd_t> loadclientinitcmds;

		int32_t skelIndex_ld[12];
		std::vector<str> headmodels;
		std::vector<str> headskins;
		bool bIsCharacter;

		struct
		{
			float load_scale;
			float lod_scale;
			float lod_bias;
			std::vector<str> skelmodel;
			vec3_t origin;
			vec3_t lightoffset;
			float radius;
			std::vector<dloadsurface_t> surfaces;
		} loaddata;

		bool bInIncludesSection;
	};
};
