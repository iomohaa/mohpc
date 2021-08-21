#pragma once

#include <MOHPC/Assets/Formats/TIKI.h>
#include <MOHPC/Common/Vector.h>
#include <MOHPC/Utility/SharedPtr.h>
#include "TIKI_Script.h"

#include <vector>
#include <set>
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
	struct dtikicmd_t
	{
		frameInt_t frame_num;
		tiki_str_list_t args;
	};

	struct dloadframecmd_t 
	{
		frameInt_t frame_num;
		tiki_str_list_t args;
		str location;
	};

	struct dloadinitcmd_t
	{
		tiki_str_list_t args;
	};

	struct dloadsurface_t
	{
		str name;
		tiki_str_list_t shader;
		int flags;
		float damage_multiplier;
	};

	struct dloadanim_t
	{
	public:
		dloadanim_t();
		dloadanim_t(const char* aliasValue);

		void init();

		operator const char* () const;

	public:
		str alias;
		str name;
		str location;
		float weight;
		float blendtime;
		int32_t flags;
		tiki_frame_cmd_list_t loadservercmds;
		tiki_frame_cmd_list_t loadclientcmds;
	};

	struct danimcomp_t
	{
	public:
		bool operator ()(const char* left, const char* right) const
		{
			return strHelpers::icmp(left, right) < 0;
		}

		/*
		bool operator ()(const dloadanim_t& left, const dloadanim_t& right) const
		{
			return strHelpers::icmp(left.alias.c_str(), right.alias.c_str()) < 0;
		}
		*/
	};

	using tiki_anim_list_t = std::set<dloadanim_t, danimcomp_t>;

	struct dloadquaked_t
	{
		str name;
		vec3_t color;
		vec3_t mins;
		vec3_t maxs;
		tiki_str_list_t spawnFlags;
	};

	struct dloaddef_t
	{
		SharedPtr<TikiScript> tikiFile;

		tiki_anim_list_t loadanims;
		tiki_init_cmd_list_t loadserverinitcmds;
		tiki_init_cmd_list_t loadclientinitcmds;

		tiki_str_list_t headmodels;
		tiki_str_list_t headskins;
		int32_t skelIndex_ld[12];
		bool bIsCharacter;

		struct
		{
			float load_scale;
			float lod_scale;
			float lod_bias;
			tiki_str_list_t skelmodel;
			vec3_t origin;
			vec3_t lightoffset;
			float radius;
			std::list<dloadsurface_t> surfaces;
			dloadquaked_t quaked;
		} loaddata;

		bool bInIncludesSection;
	};
};
