#pragma once

#include "../Asset.h"
#include "../../Common/Vector.h"
#include "Skel.h"

//=
// animation flags
//=

/** Random animation play. */
static constexpr unsigned int TAF_RANDOM				= 0x1;
/** Prevent looping. */
static constexpr unsigned int TAF_NOREPEAT				= 0x2;
/** Don't repeat the animation. */
static constexpr unsigned int TAF_DONTREPEAT			= ( TAF_RANDOM | TAF_NOREPEAT );
/** Footsteps for dogs. */
static constexpr unsigned int TAF_AUTOSTEPS_DOG			= 0x4;
static constexpr unsigned int TAF_DEFAULT_ANGLES		= 0x8;
/** Don't check for timing. */
static constexpr unsigned int TAF_NOTIMECHECK			= 0x10;
/** Automatically play sound when the owning entity is walking. */
static constexpr unsigned int TAF_AUTOSTEPS_WALK		= ( TAF_NOTIMECHECK | TAF_AUTOSTEPS_DOG );
/** Automatically play sound when the owning entity is running. */
static constexpr unsigned int TAF_AUTOSTEPS_RUN			= ( TAF_NOTIMECHECK | TAF_AUTOSTEPS_DOG | TAF_DEFAULT_ANGLES );
/** Animation is based on skeletor animation's delta. */
static constexpr unsigned int TAF_DELTADRIVEN			= 0x20;

//=
// skeletor animation flags
//=

/** The animation should loop. */
static constexpr unsigned int TAF_LOOP					= 0x20;
/** Contains delta data. */
static constexpr unsigned int TAF_HASDELTA				= 0x40;
/** Contains morph data. */
static constexpr unsigned int TAF_HASMORPH				= 0x80;

//=
// tiki anim command frames
//=

/** Execute command last. */
static constexpr unsigned int TIKI_FRAME_LAST			= -5;
/** Execute command at end. */
static constexpr unsigned int TIKI_FRAME_END			= -4;
/** Execute command at entry. */
static constexpr unsigned int TIKI_FRAME_ENTRY			= -3;
/** Execute command at exit. */
static constexpr unsigned int TIKI_FRAME_EXIT			= -2;
/** Execute command each frame. */
static constexpr unsigned int TIKI_FRAME_EVERY			= -1;
/** Execute command on first frame. */
static constexpr unsigned int TIKI_FRAME_FIRST			= 0;
static constexpr unsigned int TIKI_FRAME_MAXFRAMERATE	= 60;

using frameInt_t = unsigned int;

//=
// tiki surface flags
//=

static constexpr unsigned int TIKI_SURF_SKIN1				= ( 1 << 0 );
static constexpr unsigned int TIKI_SURF_SKIN2				= ( 1 << 1 );
static constexpr unsigned int TIKI_SURF_SKIN3				= ( TIKI_SURF_SKIN1 | TIKI_SURF_SKIN2 );
static constexpr unsigned int TIKI_SURF_NODRAW				= ( 1 << 2 );
static constexpr unsigned int TIKI_SURF_CROSSFADE			= ( 1 << 6 );
static constexpr unsigned int TIKI_SURF_NODAMAGE			= ( 1 << 7 );
static constexpr unsigned int TIKI_SURF_NOMIPMAPS			= ( 1 << 8 );
static constexpr unsigned int TIKI_SURF_NOPICMIP			= ( 1 << 9 );

static constexpr char TIKI_HEADER[] = "TIKI";

namespace MOHPC
{
	class TIKIAnim
	{
	public:
		struct Command
		{
			frameInt_t frame_num;
			mfuse::con::Container<str> args;
		};

		struct AnimDef
		{
			str alias;
			float weight;
			float blendtime;
			uint32_t flags;
			mfuse::con::Container<Command> client_cmds;
			mfuse::con::Container<Command> server_cmds;
			SharedPtr<SkeletonAnimation> animData;
		};

	public:
		str name;
		mfuse::con::Container<Command> client_initcmds;
		mfuse::con::Container<Command> server_initcmds;
		Vector mins;
		Vector maxs;
		mfuse::con::Container<str> headmodels;
		mfuse::con::Container<str> headskins;
		bool bIsCharacter;
		mfuse::con::Container<AnimDef> animdefs;
	};

	typedef SharedPtr<class TIKI> TIKIPtr;

	struct QuakedSection
	{
		str name;
		Vector color;
		Vector mins;
		Vector maxs;
		mfuse::con::Container<str> spawnFlags;
	};

	class TIKI : public Asset
	{
		MOHPC_ASSET_OBJECT_DECLARATION(TIKI);

	public:
		struct TIKISurface
		{
			str name;
			mfuse::con::Container<str> shaders;
			uint32_t flags;
			float damageMultiplier;
		};

	private:
		str name;
		class TIKIAnim* tikianim;
		mfuse::con::Container<TIKISurface> surfaces;
		float loadScale;
		float lodScale;
		float lodBias;
		Vector lightOffset;
		Vector loadOrigin;
		float radius;
		SkeletonChannelList boneList;
		mfuse::con::Container<SharedPtr<Skeleton>> meshes;
		QuakedSection quakedSection;

	public:
		struct dtikicmd_t;
		struct dloadframecmd_t;
		struct dloadinitcmd_t;
		struct dloadsurface_t;
		struct dloadanim_t;
		struct dloaddef_t;

	public:
		MOHPC_ASSETS_EXPORTS TIKI();
		~TIKI();

		void Load() override;

		// animations
		MOHPC_ASSETS_EXPORTS size_t GetNumAnimations() const;
		MOHPC_ASSETS_EXPORTS SkeletonAnimationPtr GetAnimation(size_t num) const;
		MOHPC_ASSETS_EXPORTS const TIKIAnim::AnimDef* GetAnimDef(size_t num) const;
		MOHPC_ASSETS_EXPORTS SkeletonAnimationPtr GetAnimationByName(const char *name) const;
		MOHPC_ASSETS_EXPORTS const TIKIAnim::AnimDef* GetAnimDefByName(const char *name) const;
		MOHPC_ASSETS_EXPORTS const TIKIAnim::AnimDef* GetRandomAnimation(const char *name) const;
		MOHPC_ASSETS_EXPORTS void GetAllAnimations(const char *name, mfuse::con::Container<TIKIAnim::AnimDef*>& out) const;
		MOHPC_ASSETS_EXPORTS bool IsStaticModel() const;

		// commands
		MOHPC_ASSETS_EXPORTS size_t GetNumServerInitCommands() const;
		MOHPC_ASSETS_EXPORTS const TIKIAnim::Command* GetServerInitCommand(size_t index) const;
		MOHPC_ASSETS_EXPORTS size_t GetNumClientInitCommands() const;
		MOHPC_ASSETS_EXPORTS const TIKIAnim::Command* GetClientInitCommand(size_t index) const;

		// meshes
		MOHPC_ASSETS_EXPORTS size_t GetNumMeshes() const;
		MOHPC_ASSETS_EXPORTS SkeletonPtr GetMesh(size_t index) const;

		// surfaces
		MOHPC_ASSETS_EXPORTS size_t GetNumSurfaces() const;
		MOHPC_ASSETS_EXPORTS const TIKISurface *GetSurface(size_t index) const;

		// misc
		MOHPC_ASSETS_EXPORTS const SkeletonChannelList *GetBoneList() const;
		MOHPC_ASSETS_EXPORTS const char *GetBoneNameFromNum(int num) const;
		MOHPC_ASSETS_EXPORTS intptr_t GetBoneNumFromName(const char *name) const;
		MOHPC_ASSETS_EXPORTS float GetScale() const;
		MOHPC_ASSETS_EXPORTS const QuakedSection& GetQuakedSection() const;

	private:
		bool LoadTIKIAnim(const char* Filename, dloaddef_t* ld);
		bool LoadTIKIModel(const char* Filename, const dloaddef_t* ld);
		void FreeStorage(dloaddef_t* ld);

		bool ParseSetup(dloaddef_t* ld);
		void ParseInitCommands(dloaddef_t* ld, mfuse::con::Container<dloadinitcmd_t>& cmdlist);
		void ParseInit(dloaddef_t* ld);
		bool ParseCase(dloaddef_t* ld);
		void ParseFrameCommands(dloaddef_t* ld, mfuse::con::Container<dloadframecmd_t>& cmdlist);
		void ParseAnimationCommands(dloaddef_t* ld, dloadanim_t* anim);
		void ParseAnimationFlags(dloaddef_t* ld, dloadanim_t* anim);
		void ParseAnimationsFail(dloaddef_t* ld);
		bool ParseIncludes(dloaddef_t* ld);
		void ParseAnimations(dloaddef_t* ld);
		void ParseQuaked(dloaddef_t* ld);
		int32_t ParseSurfaceFlag(const char* token);
		void InitSetup(dloaddef_t* ld);
		bool LoadSetupCase(const char *filename, const dloaddef_t* ld, mfuse::con::Container<dloadsurface_t>& loadsurfaces);
		bool LoadSetup(const char *filename, const dloaddef_t* ld, mfuse::con::Container<dloadsurface_t>& loadsurfaces);

		bool IsValidFrame(size_t maxFrames, frameInt_t frameNum) const;
		void FixFrameNum(const TIKIAnim *ptiki, const SkeletonAnimation *animData, TIKIAnim::Command *cmd, const char *alias);
		void LoadAnim(const TIKIAnim *ptiki);
		TIKIAnim* InitTiki(dloaddef_t *ld);

		// animations
		void GetAnimOrder(const dloaddef_t *ld, mfuse::con::Container<size_t>& order) const;

		// main
		void SetupIndividualSurface(const char *filename, TIKISurface* surf, const char *name, const dloadsurface_t *loadsurf);
	};

	namespace TIKIError
	{
		class Base : public std::exception {};

		class BadSetup : public Base
		{
		public:
			const char* what() const noexcept override;
		};
	}
};
