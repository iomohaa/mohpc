#pragma once

#include "../Asset.h"
#include "../../Common/Vector.h"
#include "Skel.h"

#include <list>

namespace MOHPC
{
	using tikiFlag_t = uint32_t;

	/**
	 * Animation flags.
	 */
	namespace TAF
	{
		/** Random animation play. */
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t RANDOM;
		/** Prevent looping. */
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t NOREPEAT;
		/** Don't repeat the animation. */
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t DONTREPEAT;
		/** Footsteps for dogs. */
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t AUTOSTEPS_DOG;
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t DEFAULT_ANGLES;
		/** Don't check for timing. */
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t NOTIMECHECK;
		/** Automatically play sound when the owning entity is walking. */
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t AUTOSTEPS_WALK;
		/** Automatically play sound when the owning entity is running. */
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t AUTOSTEPS_RUN;
		/** Animation is based on skeletor animation's delta. */
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t DELTADRIVEN;
	}

	/**
	 * Pseudo animation command frames.
	 */
	namespace TF
	{

		/** Execute command last. */
		MOHPC_ASSETS_EXPORTS extern const unsigned long LAST;
		/** Execute command at end. */
		MOHPC_ASSETS_EXPORTS extern const unsigned long END;
		/** Execute command at entry. */
		MOHPC_ASSETS_EXPORTS extern const unsigned long ENTRY;
		/** Execute command at exit. */
		MOHPC_ASSETS_EXPORTS extern const unsigned long EXIT;
		/** Execute command each frame. */
		MOHPC_ASSETS_EXPORTS extern const unsigned long EVERY;
		/** Execute command on first frame. */
		MOHPC_ASSETS_EXPORTS extern const unsigned long FIRST;
	}

	using frameInt_t = uint32_t;

	/**
	 * Surface flags.
	 */
	namespace TS
	{
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t SKIN1;
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t SKIN2;
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t SKIN3;
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t NODRAW;
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t CROSSFADE;
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t NODAMAGE;
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t NOMIPMAPS;
		MOHPC_ASSETS_EXPORTS extern const tikiFlag_t NOPICMIP;
	}

	MOHPC_ASSETS_EXPORTS extern const unsigned long TIKI_FRAME_MAXFRAMERATE;
	MOHPC_ASSETS_EXPORTS extern const char TIKI_HEADER[];

	struct dtikicmd_t;
	struct dloadframecmd_t;
	struct dloadinitcmd_t;
	struct dloadsurface_t;
	struct dloadanim_t;
	struct dloaddef_t;
	class TIKIAnim;

	using tiki_str_list_t = std::list<str>;
	using tiki_frame_cmd_list_t = std::list<dloadframecmd_t>;
	using tiki_init_cmd_list_t = std::list<dloadinitcmd_t>;
	using tiki_surface_list_t = std::list<dloadsurface_t>;
	using tiki_skel_list_t = std::list<SkeletonPtr>;

	class SkeletorManager;

	struct TIKICommand
	{
	public:
		TIKICommand() noexcept;

		/** Return the number of arguments this command contains.*/
		MOHPC_ASSETS_EXPORTS size_t getNumArgs() const;
		/** Return the argument at the specified index. */
		MOHPC_ASSETS_EXPORTS const char* getArg(uintptr_t index) const;
		/** Return the associated frame number for this command. */
		MOHPC_ASSETS_EXPORTS frameInt_t getFrameNumber() const;

	public:
		size_t num_args;
		char** args;
		frameInt_t frame_num;
	};

	struct TIKIAnimDef
	{
	public:
		TIKIAnimDef() noexcept;

		/** Return the alias name for this definition. */
		MOHPC_ASSETS_EXPORTS const char* getAlias() const;
		/** Return the number of client commands. */
		MOHPC_ASSETS_EXPORTS size_t getNumClientCommands() const;
		/** Return the client command at the specified index. */
		MOHPC_ASSETS_EXPORTS const TIKICommand* getClientCommand(uintptr_t index) const;
		/** Return the number of server commands. */
		MOHPC_ASSETS_EXPORTS size_t getNumServerCommands() const;
		/** Return the server command at the specified index. */
		MOHPC_ASSETS_EXPORTS const TIKICommand* getServerCommand(uintptr_t index) const;
		/** Return a pointer to the animation data. */
		MOHPC_ASSETS_EXPORTS const SharedPtr<SkeletonAnimation>& getAnimData() const;
		/** Return the animation's weight. */
		MOHPC_ASSETS_EXPORTS float getWeight() const;
		/** Return the animation's blend time. */
		MOHPC_ASSETS_EXPORTS float getBlendTime() const;
		/** Return the flag for the animation. */
		MOHPC_ASSETS_EXPORTS uint32_t getFlags() const;

	public:
		char* alias;
		TIKICommand* client_cmds;
		TIKICommand* server_cmds;
		size_t num_client_cmds;
		size_t num_server_cmds;
		SharedPtr<SkeletonAnimation> animData;
		float weight;
		float blendtime;
		uint32_t flags;
	};

	class TIKIAnim
	{
	public:
		TIKIAnim() noexcept;

		/** Return the number of animation definitions. */
		MOHPC_ASSETS_EXPORTS size_t getNumAnimDefs() const;
		/** Return the animation definition at the specified index. */
		MOHPC_ASSETS_EXPORTS const TIKIAnimDef* getAnimDef(uintptr_t index) const;
		/** Return the number of client initialization commands. */
		MOHPC_ASSETS_EXPORTS size_t getNumClientInitCommands() const;
		/** Return the client initialization command at the specified index. */
		MOHPC_ASSETS_EXPORTS const TIKICommand* getClientInitCommands(uintptr_t index) const;
		/** Return the number of server initialization commands. */
		MOHPC_ASSETS_EXPORTS size_t getNumServerInitCommands() const;
		/** Return the server initialization command at the specified index. */
		MOHPC_ASSETS_EXPORTS const TIKICommand* getServerInitCommands(uintptr_t index) const;
		/** Return the number of head models. */
		MOHPC_ASSETS_EXPORTS size_t getNumHeadModels() const;
		/** Return the head model at the specified index. */
		MOHPC_ASSETS_EXPORTS const char* getHeadModel(uintptr_t index) const;
		/** Return the number of head skins. */
		MOHPC_ASSETS_EXPORTS size_t getNumHeadSkins() const;
		/** Return the head skin at the specified index. */
		MOHPC_ASSETS_EXPORTS const char* getHeadSkin(uintptr_t index) const;
		/** Return the minimum bounds for this anim def (idle animation's bounds). */
		MOHPC_ASSETS_EXPORTS void getMins(vec3r_t out) const;
		/** Return the maximum bounds for this anim def (idle animation's bounds). */
		MOHPC_ASSETS_EXPORTS void getMaxs(vec3r_t out) const;
		/** Whether or not this animation deifnition matches a character. */
		MOHPC_ASSETS_EXPORTS bool isCharacter() const;

	public:
		TIKIAnimDef* animdefs;
		TIKICommand* client_initcmds;
		TIKICommand* server_initcmds;
		char** headmodels;
		char** headskins;
		size_t num_headmodels;
		size_t num_headskins;
		size_t num_client_initcmds;
		size_t num_server_initcmds;
		size_t num_animdefs;
		vec3_t mins;
		vec3_t maxs;
		bool bIsCharacter;
	};
	typedef SharedPtr<class TIKI> TIKIPtr;

	struct QuakedSection
	{
		str name;
		vec3_t color;
		vec3_t mins;
		vec3_t maxs;
		std::vector<str> spawnFlags;
	};

	struct TIKISurface
	{
		str name;
		std::vector<str> shaders;
		uint32_t flags;
		float damageMultiplier;
	};

	class TIKI : public Asset
	{
		MOHPC_ASSET_OBJECT_DECLARATION(TIKI);

	public:
		MOHPC_ASSETS_EXPORTS TIKI(const fs::path& path, const SharedPtr<SkeletorManager>& skeletorManPtr, TIKIAnim* panim);
		~TIKI();

		// animations
		MOHPC_ASSETS_EXPORTS size_t GetNumAnimations() const;
		MOHPC_ASSETS_EXPORTS SkeletonAnimationPtr GetAnimation(size_t num) const;
		MOHPC_ASSETS_EXPORTS const TIKIAnimDef* GetAnimDef(size_t num) const;
		MOHPC_ASSETS_EXPORTS SkeletonAnimationPtr GetAnimationByName(const char *name) const;
		MOHPC_ASSETS_EXPORTS const TIKIAnimDef* GetAnimDefByName(const char *name) const;
		MOHPC_ASSETS_EXPORTS const TIKIAnimDef* GetRandomAnimation(const char *name) const;
		MOHPC_ASSETS_EXPORTS void GetAllAnimations(const char *name, std::vector<TIKIAnimDef*>& out) const;
		MOHPC_ASSETS_EXPORTS bool IsStaticModel() const;

		// commands
		MOHPC_ASSETS_EXPORTS size_t GetNumServerInitCommands() const;
		MOHPC_ASSETS_EXPORTS const TIKICommand* GetServerInitCommand(size_t index) const;
		MOHPC_ASSETS_EXPORTS size_t GetNumClientInitCommands() const;
		MOHPC_ASSETS_EXPORTS const TIKICommand* GetClientInitCommand(size_t index) const;

		// meshes
		MOHPC_ASSETS_EXPORTS size_t GetNumMeshes() const;
		MOHPC_ASSETS_EXPORTS SkeletonPtr GetMesh(size_t index) const;
		std::vector<SharedPtr<Skeleton>>& getMeshes();

		// surfaces
		MOHPC_ASSETS_EXPORTS size_t GetNumSurfaces() const;
		MOHPC_ASSETS_EXPORTS const TIKISurface *GetSurface(size_t index) const;
		std::vector<TIKISurface>& getSurfaces();

		// misc
		MOHPC_ASSETS_EXPORTS const SkeletonChannelList *GetBoneList() const;
		SkeletonChannelList& getBoneList();
		MOHPC_ASSETS_EXPORTS const char *GetBoneNameFromNum(int num) const;
		MOHPC_ASSETS_EXPORTS intptr_t GetBoneNumFromName(const char *name) const;
		MOHPC_ASSETS_EXPORTS float GetScale() const;
		MOHPC_ASSETS_EXPORTS const QuakedSection& GetQuakedSection() const;
		QuakedSection& GetQuakedSection();

		const TIKIAnim* getTikiAnim() const;
		void setLoadScale(float loadScaleValue);
		void setLodScale(float lodScaleValue);
		void setLodBias(float lodBiasValue);
		void setLightOffset(const_vec3r_t lightOffsetValue);
		void setLoadOrigin(const_vec3r_t loadOriginValue);
		void setRadius(float radiusValue);

	private:
		TIKIAnim* tikianim;
		std::vector<TIKISurface> surfaces;
		std::vector<SharedPtr<Skeleton>> meshes;
		SkeletonChannelList boneList;
		QuakedSection quakedSection;
		SharedPtr<SkeletorManager> skeletorManager;
		float loadScale;
		float lodScale;
		float lodBias;
		vec3_t lightOffset;
		vec3_t loadOrigin;
		float radius;
	};

	class TIKIReader : public AssetReader
	{
		MOHPC_ASSET_OBJECT_DECLARATION(TIKIReader);

	public:
		using AssetType = TIKI;

	public:
		/**
		 * The default constructor allows include of every file regardless of the condition.
		 */
		MOHPC_ASSETS_EXPORTS TIKIReader();

		/**
		 * Instantiate a TIKI reader with specific include conditions.
		 *
		 * @param allowedIncludes array of allowed include names (case-insensitive).
		 * @param numElements number of elements in the array.
		 */
		MOHPC_ASSETS_EXPORTS TIKIReader(const char* allowedIncludes[], size_t numElements);
		MOHPC_ASSETS_EXPORTS ~TIKIReader();
		MOHPC_ASSETS_EXPORTS AssetPtr read(const IFilePtr& file) override;

	private:
		TIKIAnim* LoadTIKIAnim(const fs::path& Filename, dloaddef_t* ld);
		TIKIPtr LoadTIKIModel(const fs::path& Filename, TIKIAnim* tikiAnim, const dloaddef_t* ld);
		void FreeStorage(dloaddef_t* ld);

		bool ParseSetup(dloaddef_t* ld);
		void ParseInitCommands(dloaddef_t* ld, tiki_init_cmd_list_t& cmdlist);
		void ParseInit(dloaddef_t* ld);
		bool ParseCase(dloaddef_t* ld);
		void ParseFrameCommands(dloaddef_t* ld, tiki_frame_cmd_list_t& cmdlist);
		void ParseAnimationCommands(dloaddef_t* ld, dloadanim_t* anim);
		void ParseAnimationFlags(dloaddef_t* ld, dloadanim_t* anim);
		void ParseAnimationsFail(dloaddef_t* ld);
		bool ParseIncludes(dloaddef_t* ld);
		void ParseAnimations(dloaddef_t* ld);
		void ParseQuaked(dloaddef_t* ld);
		int32_t ParseSurfaceFlag(const char* token);
		void InitSetup(dloaddef_t* ld);
		bool LoadSetupCase(TIKI& tiki, const dloaddef_t* ld, tiki_surface_list_t& loadsurfaces);
		bool LoadSetup(TIKI& tiki, const dloaddef_t* ld, tiki_surface_list_t& loadsurfaces);

		bool IsPredefinedFrame(frameInt_t frameNum) const;
		bool IsValidFrame(size_t maxFrames, frameInt_t frameNum) const;
		void FixFrameNum(dloaddef_t* ld, const TIKIAnim* ptiki, const SkeletonAnimation* animData, TIKICommand* cmd, const char* alias);
		void LoadAnim(dloaddef_t* ld, const TIKIAnim* ptiki);
		TIKIAnim* InitTiki(dloaddef_t* ld);

		// main
		void SetupIndividualSurface(TIKISurface* surf, const char* name, const dloadsurface_t& loadsurf);

		bool isIncludeAllowed(const char* includeName);

	private:
		const char** allowedIncludes;
		size_t numAllowedIncludes;
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
