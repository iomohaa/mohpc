#pragma once

#include <MOHPC/Formats/TIKI.h>
#include <MOHPC/Vector.h>
#include "TIKI_Script.h"

// skb skeletor
#define TIKI_SKB_HEADER_IDENT		( *( int * )"SKL " )
#define TIKI_SKB_HEADER_VER_3		3
#define TIKI_SKB_HEADER_VERSION		4

// skd skeletor
#define TIKI_SKD_HEADER_IDENT		( *( int * )"SKMD" )
#define TIKI_SKD_HEADER_OLD_VERSION	5
#define TIKI_SKD_HEADER_VERSION		6

// skc skeletor animations
#define TIKI_SKC_HEADER_IDENT		( *( int * )"SKAN" )
#define TIKI_SKC_HEADER_OLD_VERSION	13
#define TIKI_SKC_HEADER_VERSION		14

// tiki limitations
#define TIKI_MAX_BONES				100
#define TIKI_MAX_VERTEXES			1000
#define TIKI_MAX_TRIANGLES			2000
#define TIKI_MAX_SKELCACHE			1024
#define TIKI_MAX_COMMANDS			128

namespace MOHPC
{
	struct TIKI::dtikicmd_t
	{
		int32_t frame_num;
		std::vector<std::string> args;
	};

	struct TIKI::dloadframecmd_t 
	{
		int frame_num;
		std::vector<std::string> args;
		std::string location;
	};

	struct TIKI::dloadinitcmd_t
	{
		std::vector<std::string> args;
	};

	struct TIKI::dloadsurface_t
	{
		std::string name;
		std::vector<std::string> shader;
		int flags;
		float damage_multiplier;
	};

	struct TIKI::dloadanim_t
	{
		std::string alias;
		std::string name;
		std::string location;
		float weight;
		float blendtime;
		int32_t flags;
		std::vector<dloadframecmd_t> loadservercmds;
		std::vector<dloadframecmd_t> loadclientcmds;
	};

	struct TIKI::dloaddef_t
	{
		std::string path;
		std::shared_ptr<TikiScript> tikiFile;

		std::map<std::string, std::string> keyvalues;
		std::vector<dloadanim_t> loadanims;
		std::vector<dloadinitcmd_t> loadserverinitcmds;
		std::vector<dloadinitcmd_t> loadclientinitcmds;

		int32_t skelIndex_ld[12];
		std::vector<std::string> headmodels;
		std::vector<std::string> headskins;
		bool bIsCharacter;

		struct
		{
			float load_scale;
			float lod_scale;
			float lod_bias;
			std::vector<std::string> skelmodel;
			Vector origin;
			Vector lightoffset;
			float radius;
			std::vector<dloadsurface_t> surfaces;
		} loaddata;

		bool bInIncludesSection;
	};
};
