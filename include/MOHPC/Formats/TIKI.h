#pragma once

#include "../Global.h"
#include "../Asset.h"
#include "../Vector.h"
#include "Skel.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

// animation flags
#define TAF_RANDOM					0x1		// random
#define TAF_NOREPEAT				0x2		// no loop
#define TAF_DONTREPEAT				( TAF_RANDOM | TAF_NOREPEAT )	// don't repeat
#define TAF_AUTOSTEPS_DOG			0x4		// footsteps for dogs
#define TAF_DEFAULT_ANGLES			0x8		// ?
#define TAF_NOTIMECHECK				0x10	// don't check for timing
#define TAF_AUTOSTEPS_WALK			( TAF_NOTIMECHECK | TAF_AUTOSTEPS_DOG )	// walk
#define TAF_AUTOSTEPS_RUN			( TAF_NOTIMECHECK | TAF_AUTOSTEPS_DOG | TAF_DEFAULT_ANGLES )	// run
#define TAF_DELTADRIVEN				0x20	// animation is based on skeletor animation's delta

// skeletor animation flags
#define TAF_LOOP				0x20
#define TAF_HASDELTA			0x40
#define TAF_HASMORPH			0x80

// tiki anim command frames
#define TIKI_FRAME_LAST				-5			// exec at last
#define TIKI_FRAME_END				-4			// exec at end
#define TIKI_FRAME_ENTRY			-3			// exec at entry
#define TIKI_FRAME_EXIT				-2			// exec at exit
#define TIKI_FRAME_EVERY			-1			// exec each frame
#define TIKI_FRAME_FIRST			0			// exec at first frame
#define TIKI_FRAME_MAXFRAMERATE		60

// tiki surface flags
#define TIKI_SURF_SKIN1				( 1 << 0 )
#define TIKI_SURF_SKIN2				( 1 << 1 )
#define TIKI_SURF_SKIN3				( TIKI_SURF_SKIN1 | TIKI_SURF_SKIN2 )
#define TIKI_SURF_NODRAW			( 1 << 2 )
#define TIKI_SURF_CROSSFADE			( 1 << 6 )
#define TIKI_SURF_NODAMAGE			( 1 << 7 )
#define TIKI_SURF_NOMIPMAPS			( 1 << 8 )
#define TIKI_SURF_NOPICMIP			( 1 << 9 )

namespace MOHPC
{
	class TIKIAnim
	{
	public:
		struct Command
		{
			intptr_t frame_num;
			std::vector<std::string> args;
		};

		struct AnimDef
		{
			std::string alias;
			float weight;
			float blendtime;
			int flags;
			std::vector<Command> client_cmds;
			std::vector<Command> server_cmds;
			std::shared_ptr<SkeletonAnimation> animData;
		};

	public:
		std::string name;
		std::vector<Command> client_initcmds;
		std::vector<Command> server_initcmds;
		Vector mins;
		Vector maxs;
		std::vector<std::string> headmodels;
		std::vector<std::string> headskins;
		bool bIsCharacter;
		std::vector<AnimDef> animdefs;
	};

	typedef std::shared_ptr<class TIKI> TIKIPtr;

	struct QuakedSection
	{
		std::string name;
		Vector color;
		Vector mins;
		Vector maxs;
		std::vector<std::string> spawnFlags;
	};

	class TIKI : public Asset
	{
		CLASS_BODY(TIKI);

	public:
		struct TIKISurface
		{
			std::string name;
			std::vector<std::string> shaders;
			int32_t flags;
			float damageMultiplier;
		};

	private:
		std::string name;
		class TIKIAnim* tikianim;
		std::vector<TIKISurface> surfaces;
		float loadScale;
		float lodScale;
		float lodBias;
		Vector lightOffset;
		Vector loadOrigin;
		float radius;
		SkeletonChannelList boneList;
		std::vector<std::shared_ptr<Skeleton>> meshes;
		QuakedSection quakedSection;

	public:
		struct dtikicmd_t;
		struct dloadframecmd_t;
		struct dloadinitcmd_t;
		struct dloadsurface_t;
		struct dloadanim_t;
		struct dloaddef_t;

	public:
		MOHPC_EXPORTS TIKI();
		~TIKI();

		virtual bool Load() override;

		// animations
		MOHPC_EXPORTS size_t GetNumAnimations() const;
		MOHPC_EXPORTS SkeletonAnimation* GetAnimation(size_t num) const;
		MOHPC_EXPORTS const TIKIAnim::AnimDef* GetAnimDef(size_t num) const;
		MOHPC_EXPORTS SkeletonAnimation* GetAnimationByName(const char *name) const;
		MOHPC_EXPORTS const TIKIAnim::AnimDef* GetAnimDefByName(const char *name) const;
		MOHPC_EXPORTS const TIKIAnim::AnimDef* GetRandomAnimation(const char *name) const;
		MOHPC_EXPORTS void GetAllAnimations(const char *name, std::vector<TIKIAnim::AnimDef*>& out) const;
		MOHPC_EXPORTS bool IsStaticModel() const;

		// commands
		MOHPC_EXPORTS size_t GetNumServerInitCommands() const;
		MOHPC_EXPORTS const TIKIAnim::Command* GetServerInitCommand(size_t index) const;
		MOHPC_EXPORTS size_t GetNumClientInitCommands() const;
		MOHPC_EXPORTS const TIKIAnim::Command* GetClientInitCommand(size_t index) const;

		// meshes
		MOHPC_EXPORTS size_t GetNumMeshes() const;
		MOHPC_EXPORTS Skeleton *GetMesh(size_t index) const;

		// surfaces
		MOHPC_EXPORTS size_t GetNumSurfaces() const;
		MOHPC_EXPORTS const TIKISurface *GetSurface(size_t index) const;

		// misc
		MOHPC_EXPORTS const SkeletonChannelList *GetBoneList() const;
		MOHPC_EXPORTS const char *GetBoneNameFromNum(int num) const;
		MOHPC_EXPORTS intptr_t GetBoneNumFromName(const char *name) const;
		MOHPC_EXPORTS float GetScale() const;
		MOHPC_EXPORTS const QuakedSection& GetQuakedSection() const;

	private:
		bool LoadTIKIAnim(const char* Filename, dloaddef_t* ld);
		bool LoadTIKIModel(const char* Filename, const dloaddef_t* ld);
		void FreeStorage(dloaddef_t* ld);

		bool ParseSetup(dloaddef_t* ld);
		void ParseInitCommands(dloaddef_t* ld, std::vector<dloadinitcmd_t>& cmdlist);
		void ParseInit(dloaddef_t* ld);
		bool ParseCase(dloaddef_t* ld);
		void ParseFrameCommands(dloaddef_t* ld, std::vector<dloadframecmd_t>& cmdlist);
		void ParseAnimationCommands(dloaddef_t* ld, dloadanim_t* anim);
		void ParseAnimationFlags(dloaddef_t* ld, dloadanim_t* anim);
		void ParseAnimationsFail(dloaddef_t* ld);
		bool ParseIncludes(dloaddef_t* ld);
		void ParseAnimations(dloaddef_t* ld);
		void ParseQuaked(dloaddef_t* ld);
		int32_t ParseSurfaceFlag(const char* token);
		void InitSetup(dloaddef_t* ld);
		bool LoadSetupCase(const char *filename, const dloaddef_t* ld, std::vector<dloadsurface_t>& loadsurfaces);
		bool LoadSetup(const char *filename, const dloaddef_t* ld, std::vector<dloadsurface_t>& loadsurfaces);

		void FixFrameNum(TIKIAnim *ptiki, SkeletonAnimation *animData, TIKIAnim::Command *cmd, const char *alias);
		void LoadAnim(TIKIAnim *ptiki);
		TIKIAnim* InitTiki(dloaddef_t *ld);
		void RemoveTiki(TIKIAnim *ptiki);

		// animations
		void GetAnimOrder(const dloaddef_t *ld, std::vector<size_t>& order) const;

		// main
		void SetupIndividualSurface(const char *filename, TIKISurface* surf, const char *name, const dloadsurface_t *loadsurf);
	};
};
