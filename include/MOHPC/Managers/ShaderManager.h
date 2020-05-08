#pragma once

#include "../Global.h"
#include "../Script/Container.h"
#include "../Script/con_set.h"
#include "../Script/str.h"
#include "../Utilities/SharedPtr.h"
#include "Manager.h"

#include <MOHPC/Vector.h>

namespace MOHPC
{
	static const unsigned GLS_SRCBLEND_ZERO = 0x00000001;
	static const unsigned GLS_SRCBLEND_ONE = 0x00000002;
	static const unsigned GLS_SRCBLEND_DST_COLOR = 0x00000003;
	static const unsigned GLS_SRCBLEND_ONE_MINUS_DST_COLOR = 0x00000004;
	static const unsigned GLS_SRCBLEND_SRC_ALPHA = 0x00000005;
	static const unsigned GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA = 0x00000006;
	static const unsigned GLS_SRCBLEND_DST_ALPHA = 0x00000007;
	static const unsigned GLS_SRCBLEND_ONE_MINUS_DST_ALPHA = 0x00000008;
	static const unsigned GLS_SRCBLEND_ALPHA_SATURATE = 0x00000009;
	static const unsigned GLS_SRCBLEND_BITS = 0x0000000f;

	static const unsigned GLS_DSTBLEND_ZERO = 0x00000010;
	static const unsigned GLS_DSTBLEND_ONE = 0x00000020;
	static const unsigned GLS_DSTBLEND_SRC_COLOR = 0x00000030;
	static const unsigned GLS_DSTBLEND_ONE_MINUS_SRC_COLOR = 0x00000040;
	static const unsigned GLS_DSTBLEND_SRC_ALPHA = 0x00000050;
	static const unsigned GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA = 0x00000060;
	static const unsigned GLS_DSTBLEND_DST_ALPHA = 0x00000070;
	static const unsigned GLS_DSTBLEND_ONE_MINUS_DST_ALPHA = 0x00000080;
	static const unsigned GLS_DSTBLEND_BITS = 0x000000f0;

	static const unsigned GLS_DEPTHMASK_TRUE = 0x00000100;

	static const unsigned GLS_POLYMODE_LINE = 0x00001000;

	static const unsigned GLS_DEPTHTEST_DISABLE = 0x00010000;
	static const unsigned GLS_DEPTHFUNC_EQUAL = 0x00020000;
	static const unsigned GLS_DEPTHFUNC_GREATER = 0x00040000;
	static const unsigned GLS_DEPTHFUNC_BITS = 0x00060000;

	static const unsigned GLS_ATEST_GT_0 = 0x10000000;
	static const unsigned GLS_ATEST_LT_80 = 0x20000000;
	static const unsigned GLS_ATEST_GE_80 = 0x40000000;

	class ShaderManager;
	class Script;
	class Image;

	enum ContentFlags
	{
		CONTENTS_SOLID	= 1,		// an eye is never valid in a solid
		CONTENTS_LADDER	= 2,
		CONTENTS_LAVA	= 0x0008,
		CONTENTS_SLIME	= 0x0010,
		CONTENTS_WATER	= 0x0020,
		CONTENTS_FOG	= 0x0040,

		CONTENTS_BBOX		= 0x0100,
		CONTENTS_NOBOTCLIP	= 0x0200,
		CONTENTS_UNKNOWN1	= 0x0400,
		CONTENTS_UNKNOWN2	= 0x0800,

		CONTENTS_UNKNOWN3	= 0x1000,
		CONTENTS_FENCE		= 0x2000,
		CONTENTS_UNKNOWN4	= 0x4000,
		CONTENTS_AREAPORTAL	= 0x8000,

		CONTENTS_CLUSTERPORTAL = CONTENTS_AREAPORTAL,

		CONTENTS_PLAYERCLIP		= 0x10000,
		CONTENTS_MONSTERCLIP	= 0x20000,
		CONTENTS_WEAPONCLIP		= 0x40000,
		CONTENTS_VEHICLECLIP	= 0x80000,
		MASK_CLIP				= (CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP | CONTENTS_WEAPONCLIP | CONTENTS_VEHICLECLIP),

		CONTENTS_SHOOTONLY	= 0x100000,
		CONTENTS_DONOTENTER	= 0x200000,
		CONTENTS_BOTCLIP	= 0x400000,
		CONTENTS_MOVER		= 0x800000,

		CONTENTS_ORIGIN		= 0x1000000,	// removed before bsping an entity
		CONTENTS_BODY		= 0x2000000,	// should never be on a brush, only in game
		CONTENTS_CORPSE		= 0x4000000,
		CONTENTS_DETAIL		= 0x8000000,	// brushes not used for the bsp

		CONTENTS_STRUCTURAL		= 0x10000000,	// brushes used for the bsp
		CONTENTS_TRANSLUCENT	= 0x20000000,	// don't consume surface fragments inside
		CONTENTS_TRIGGER		= 0x40000000,
		CONTENTS_NODROP			= 0x80000000,	// don't leave bodies or items (death fog, lava)

		MASK_FOOTSTEP	= 0x42022901,
		MASK_TREADMARK	= 0x42012B01,
	};

	enum SurfaceFlags
	{
		SURF_NODAMAGE		= 0x1, // never give falling damage
		SURF_SLICK			= 0x2, // effects game physics
		SURF_SKY			= 0x4, // lighting from environment map
		SURF_LADDER			= 0x8,
		SURF_NOIMPACT		= 0x10, // don't make missile explosions
		SURF_NOMARKS		= 0x20, // don't leave missile marks
		SURF_CASTSHADOW		= 0x40,
		SURF_NODRAW			= 0x80, // don't generate a drawsurface at all
		SURF_HINT			= 0x100, // make a primary bsp splitter
		SURF_ALPHASHADOW	= 0x200, // do per-pixel light shadow casting in q3map
		SURF_NOSTEPS		= 0x400, // no footstep sounds
		SURF_NONSOLID		= 0x800, // don't collide against curves with this set
		SURF_UNKNOWN1		= 0x1000,
		SURF_PAPER			= 0x2000, // paper effects
		SURF_WOOD			= 0x4000, // wood effects
		SURF_METAL			= 0x8000, // metal effects
		SURF_ROCK			= 0x10000, // rock effects
		SURF_DIRT			= 0x20000, // dirt effects
		SURF_GRILL			= 0x40000, // metal grill (grate)
		SURF_GRASS			= 0x80000, // grass effects
		SURF_MUD			= 0x100000, // mud effects
		SURF_PUDDLE			= 0x200000, // puddle effects
		SURF_GLASS			= 0x400000, // glass effects
		SURF_GRAVEL			= 0x800000, // gravel effects
		SURF_SAND			= 0x1000000, // sand effects
		SURF_FOLIAGE		= 0x2000000, // foliage effects
		SURF_SNOW			= 0x4000000, // snow effects
		SURF_CARPET			= 0x8000000, // carpet effects
		SURF_BACKSIDE		= 0x10000000, // backside
		SURF_NODLIGHT		= 0x20000000, // don't dlight even if solid (solid lava, skies)
		SURF_NOLIGHTMAP		= 0x40000000, // surface doesn't need a lightmap
		SURF_UNKNOWN3		= 0x80000000,
	};

	enum CullType
	{
		CT_FRONT_SIDED,
		CT_BACK_SIDED,
		CT_TWO_SIDED
	};

	enum GenFunc
	{
		GF_NONE,

		GF_SIN,
		GF_SQUARE,
		GF_TRIANGLE,
		GF_SAWTOOTH,
		GF_INVERSE_SAWTOOTH,

		GF_NOISE
	};

	enum DeformMod
	{
		DEFORM_NONE,
		DEFORM_WAVE,
		DEFORM_NORMALS,
		DEFORM_BULGE,
		DEFORM_MOVE,
		DEFORM_PROJECTION_SHADOW,
		DEFORM_AUTOSPRITE,
		DEFORM_AUTOSPRITE2,
		DEFORM_FLAP_S,
		DEFORM_FLAP_T,
		DEFORM_TEXT0,
		DEFORM_TEXT1,
		DEFORM_TEXT2,
		DEFORM_TEXT3,
		DEFORM_TEXT4,
		DEFORM_TEXT5,
		DEFORM_TEXT6,
		DEFORM_TEXT7
	};

	enum AlphaGen
	{
		AGEN_IDENTITY,
		AGEN_SKIP,
		AGEN_ENTITY,
		AGEN_ONE_MINUS_ENTITY,
		AGEN_VERTEX,
		AGEN_ONE_MINUS_VERTEX,
		AGEN_LIGHTING_SPECULAR,
		AGEN_WAVEFORM,
		AGEN_PORTAL,
		AGEN_CONST,
		AGEN_NOISE,
		AGEN_DOT,
		AGEN_ONE_MINUS_DOT,
		AGEN_CONSTANT,
		AGEN_GLOBAL_ALPHA,
		AGEN_SKYALPHA,
		AGEN_ONE_MINUS_SKYALPHA,
		AGEN_SCOORD,
		AGEN_TCOORD,
		AGEN_DIST_FADE,
		AGEN_ONE_MINUS_DIST_FADE,
		AGEN_DOT_VIEW,
		AGEN_ONE_MINUS_DOT_VIEW
	};

	enum ColorGen
	{
		CGEN_BAD,
		CGEN_IDENTITY_LIGHTING,
		CGEN_IDENTITY,
		CGEN_ENTITY,
		CGEN_ONE_MINUS_ENTITY,
		CGEN_EXACT_VERTEX,
		CGEN_VERTEX,
		CGEN_ONE_MINUS_VERTEX,
		CGEN_EXACT_VERTEX_LIT,
		CGEN_VERTEX_LIT,
		CGEN_WAVEFORM,
		CGEN_MULTIPLY_BY_WAVEFORM,
		CGEN_LIGHTING_GRID,
		CGEN_LIGHTING_SPHERICAL,
		CGEN_LIGHTING_DIFFUSE,
		CGEN_CONST,
		CGEN_NOISE,
		CGEN_GLOBAL_COLOR,
		CGEN_STATIC,
		CGEN_SCOORD,
		CGEN_TCOORD,
		CGEN_DOT,
		CGEN_ONE_MINUS_DOT,
		CGEN_FOG,
	};

	enum TextureCoordGen
	{
		TCGEN_BAD,
		TCGEN_IDENTITY,			// clear to 0,0
		TCGEN_LIGHTMAP,
		TCGEN_TEXTURE,
		TCGEN_ENVIRONMENT_MAPPED,
		TCGEN_VECTOR,			// S and T from world coordinates
		TCGEN_ENVIRONMENT_MAPPED2,
		TCGEN_SUN_REFLECTION,
		TCGEN_FOG,
	};

	enum SpriteType
	{
		SPRITE_NONE,
		SPRITE_PARALLEL,
		SPRITE_PARALLEL_ORIENTED,
		SPRITE_ORIENTED,
		SPRITE_PARALLEL_UPRIGHT
	};

	enum TextureMod
	{
		TMOD_NONE,
		TMOD_TRANSFORM,
		TMOD_TURBULENT,
		TMOD_SCROLL,
		TMOD_SCALE,
		TMOD_STRETCH,
		TMOD_ROTATE,
		TMOD_ENTITY_TRANSLATE
	};

	class ImageCache
	{
	private:
		str imageName;
		SharedPtr<Image> cachedImage;
		bool bCached;
		ShaderManager* shaderManager;

	public:
		ImageCache(ShaderManager *shaderManager, const str& imageName);

	private:
		ImageCache();

	public:
		MOHPC_EXPORTS const char* GetImageName() const;
		MOHPC_EXPORTS Image* GetImage() const;

		MOHPC_EXPORTS void CacheImage();
		MOHPC_EXPORTS void ClearCachedImage();
	};

	using ImageCachePtr = SharedPtr<ImageCache>;

	struct WaveForm
	{
		GenFunc func;

		float base;
		float amplitude;
		float phase;
		float frequency;

		WaveForm();
	};

	struct SkyParms
	{
		float cloudHeight;
		ImageCache *outerbox[6];
		ImageCache *innerbox[6];

		SkyParms();
	};

	struct SpriteParms
	{
		SpriteType type;
		float scale;

		SpriteParms();
	};

	struct DeformStage
	{
		DeformMod deformation;

		Vector moveVector;
		WaveForm deformationWave;
		float deformationSpread;

		float bulgeWidth;
		float bulgeHeight;
		float bulgeSpeed;

		DeformStage();
	};

	struct TextureModInfo
	{
		TextureMod type;

		// used for TMOD_TURBULENT and TMOD_STRETCH
		WaveForm wave;

		// used for TMOD_TRANSFORM
		float matrix[2][2];		// s' = s * m[0][0] + t * m[1][0] + trans[0]
		float translate[2];		// t' = s * m[0][1] + t * m[0][1] + trans[1]

											// used for TMOD_SCALE
		float scale[2];			// s *= scale[0]
											// t *= scale[1]

											// used for TMOD_SCROLL
		float scroll[2];			// s' = s + scroll[0] * time
											// t' = t + scroll[1] * time

											// + = clockwise
											// - = counterclockwise
		float rotateSpeed;

		TextureModInfo();
	};

	struct TextureBundle
	{
		Container<ImageCache*> image;
		float imageAnimationSpeed;
		float imageAnimationPhase;

		TextureCoordGen tcGen;
		Vector tcGenVectors[2];

		Container<TextureModInfo> texMods;

		int32_t videoMapHandle;
		bool isLightmap;
		bool isVideoMap;
		int flags;

		TextureBundle();
		~TextureBundle();
	};

	struct ShaderStage
	{
		Container<TextureBundle> bundle;

		WaveForm rgbWave;
		ColorGen rgbGen;

		WaveForm alphaWave;
		AlphaGen alphaGen;

		float constantColor[4];

		unsigned stateBits;

		float alphaMin;
		float alphaMax;
		uint8_t alphaConst;
		uint8_t alphaConstMin;

		ShaderStage();
		~ShaderStage();
	};

	class Shader
	{
		friend class ShaderManager;
		friend class ShaderRef;

	private:
		class ShaderContainer* shaderContainer;
		str m_name;
		int32_t numRef;

		bool bCached;

		ImageCache* m_editorimage;

		int32_t surfaceFlags;
		int32_t contents;

		float fDistRange;
		float fDistNear;
		bool bIsSky;
		SkyParms sky;
		bool bIsPortalSky;
		SpriteParms sprite;

		float portalRange;
		bool bIsPortal;

		CullType cullType;
		bool bNoMipMaps;
		bool bNoPicMip;

		Container<DeformStage> m_deforms;
		Container<ShaderStage> m_stages;

	public:
		Shader(class ShaderContainer* Container);
		~Shader();

		MOHPC_EXPORTS ShaderManager* GetShaderManager() const;
		MOHPC_EXPORTS ShaderContainer* GetShaderContainer();
		MOHPC_EXPORTS const ShaderContainer* GetShaderContainer() const;
		MOHPC_EXPORTS const str& GetFilename() const;
		MOHPC_EXPORTS const str& GetName() const;
		MOHPC_EXPORTS int32_t GetContents() const;
		MOHPC_EXPORTS int32_t GetSurfaceFlags() const;
		MOHPC_EXPORTS float GetDistRange() const;
		MOHPC_EXPORTS float GetDistNear() const;
		MOHPC_EXPORTS bool IsSky() const;
		MOHPC_EXPORTS const SkyParms* GetSkyParms() const;
		MOHPC_EXPORTS bool IsPortalSky() const;
		MOHPC_EXPORTS SpriteType GetSpriteType() const;
		MOHPC_EXPORTS float GetPortalRange() const;
		MOHPC_EXPORTS CullType GetCullType() const;
		MOHPC_EXPORTS bool HasMipMaps() const;
		MOHPC_EXPORTS bool HasPicMip() const;
		MOHPC_EXPORTS bool HasTextures() const;
		MOHPC_EXPORTS const DeformStage* GetDeform(size_t index) const;
		MOHPC_EXPORTS size_t GetNumDeforms() const;
		MOHPC_EXPORTS const ShaderStage* GetStage(size_t index) const;
		MOHPC_EXPORTS size_t GetNumStages() const;

		void ParseShader(Script& script);

	private:
		void AddReference();
		void ClearReference();
		void ParseWaveForm(Script& script, WaveForm *wave);
		void ParseTexMod(Script& script, ShaderStage *stage);
		bool ParseStage(Script& script, ShaderStage*stage);
		void ParseDeform(Script& script);
		void ParseSkyParms(Script& script);
		void ParseSurfaceParm(Script& script);

		void Precache();
		void ClearCache();
	};

	using ShaderPtr = SharedPtr<Shader>;

	class MOHPC_EXPORTS ShaderRef
	{
	private:
		Shader* shader;

	public:
		ShaderRef();
		ShaderRef(Shader* inShader);
		ShaderRef(const ShaderRef& shaderRef);
		ShaderRef(ShaderRef&& shaderRef);
		~ShaderRef();

		operator bool() const;
		explicit operator Shader*() const;
		ShaderRef& operator=(Shader* other);
		ShaderRef& operator=(const ShaderRef& other);
		ShaderRef& operator=(ShaderRef&& other);
		bool operator==(const Shader* other) const;
		bool operator!=(const Shader* other) const;
		Shader* operator->() const;
	};

	class ShaderContainer
	{
	private:
		ShaderManager *m_shaderManager;
		str m_filename;
		Container<ShaderPtr> m_shaderList;

	public:
		ShaderContainer(ShaderManager* shaderManager, const str& filename);

		void AddShader(const ShaderPtr& Shader);
		void RemoveShader(const ShaderPtr& Shader);

		MOHPC_EXPORTS size_t GetNumShaders() const;
		MOHPC_EXPORTS const Shader* GetShader(size_t num) const;

		MOHPC_EXPORTS ShaderManager *GetShaderManager() const;
		MOHPC_EXPORTS const str& GetFilename() const;
	};

	using ShaderContainerPtr = SharedPtr<ShaderContainer>;

	class ShaderManager : public Manager
	{
		CLASS_BODY(ShaderManager);

	private:
		//std::unordered_map<str, Shader *> m_nametoshader;
		//std::unordered_map<str, ImageCache *> m_images;
		//std::unordered_map<str, ShaderContainer*> m_fileShaderMap;
		//Container<ShaderContainer*> m_shaderContainers;
		MOHPC::con_set<str, ShaderPtr> m_nametoshader;
		MOHPC::con_set<str, ImageCachePtr> m_nametoimage;
		MOHPC::con_set<str, ShaderContainerPtr> m_fileShaderMap;
		//Container<ShaderPtr> m_shaders;
		Container<ImageCachePtr> m_images;
		Container<ShaderContainerPtr> m_shaderContainers;
		ShaderContainer m_defaultShaderContainer;
		mutable Shader m_defaultshader;

	public:
		MOHPC_EXPORTS ShaderManager();
		MOHPC_EXPORTS ~ShaderManager();

		MOHPC_EXPORTS void Init();
		MOHPC_EXPORTS ShaderPtr AllocShader(ShaderContainer *shaderContainer);
		MOHPC_EXPORTS void FreeShader(const ShaderPtr& shader);
		MOHPC_EXPORTS void AddShader(const ShaderPtr& shader);
		MOHPC_EXPORTS ShaderRef GetShader(const char *name) const;
		MOHPC_EXPORTS ShaderRef GetDefaultShader() const;
		MOHPC_EXPORTS ImageCache* FindImage(const char *name);
		MOHPC_EXPORTS size_t GetNumShaderContainers() const;
		MOHPC_EXPORTS const ShaderContainer* GetShaderContainer(size_t num) const;
		MOHPC_EXPORTS const ShaderContainer* GetShaderContainer(const char* Filename) const;

	private:
		void ParseShaders(const class FileEntryList& files);
		void ParseShaderContainer(ShaderContainer *shaderContainer, const char *name, const char *buffer, std::streamsize length = 0);
		//string ParseTextureExtension(const string& name);
	};
};
