#include <Shared.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/ShaderManager.h>
#include <MOHPC/Script.h>
#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Formats/Image.h>
#include <MOHPC/Utilities/SharedPtr.h>
#include <MOHPC/Log.h>

using namespace MOHPC;

#define MOHPC_LOG_NAMESPACE "shaderman"

static unsigned NameToAFunc(const char *funcname)
{
	if (!str::icmp(funcname, "GT0"))
	{
		return GLS_ATEST_GT_0;
	}
	else if (!str::icmp(funcname, "LT128"))
	{
		return GLS_ATEST_LT_80;
	}
	else if (!str::icmp(funcname, "GE128"))
	{
		return GLS_ATEST_GE_80;
	}

	return 0;
}

static GenFunc NameToGenFunc(const char *funcname)
{
	if (!str::icmp(funcname, "sin"))
	{
		return GF_SIN;
	}
	else if (!str::icmp(funcname, "square"))
	{
		return GF_SQUARE;
	}
	else if (!str::icmp(funcname, "triangle"))
	{
		return GF_TRIANGLE;
	}
	else if (!str::icmp(funcname, "sawtooth"))
	{
		return GF_SAWTOOTH;
	}
	else if (!str::icmp(funcname, "inversesawtooth"))
	{
		return GF_INVERSE_SAWTOOTH;
	}
	else if (!str::icmp(funcname, "noise"))
	{
		return GF_NOISE;
	}

	return GF_SIN;
}


static int NameToSrcBlendMode(const char *name)
{
	if (!str::icmp(name, "GL_ONE"))
	{
		return GLS_SRCBLEND_ONE;
	}
	else if (!str::icmp(name, "GL_ZERO"))
	{
		return GLS_SRCBLEND_ZERO;
	}
	else if (!str::icmp(name, "GL_DST_COLOR"))
	{
		return GLS_SRCBLEND_DST_COLOR;
	}
	else if (!str::icmp(name, "GL_ONE_MINUS_DST_COLOR"))
	{
		return GLS_SRCBLEND_ONE_MINUS_DST_COLOR;
	}
	else if (!str::icmp(name, "GL_SRC_ALPHA"))
	{
		return GLS_SRCBLEND_SRC_ALPHA;
	}
	else if (!str::icmp(name, "GL_ONE_MINUS_SRC_ALPHA"))
	{
		return GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA;
	}
	else if (!str::icmp(name, "GL_DST_ALPHA"))
	{
		return GLS_SRCBLEND_DST_ALPHA;
	}
	else if (!str::icmp(name, "GL_ONE_MINUS_DST_ALPHA"))
	{
		return GLS_SRCBLEND_ONE_MINUS_DST_ALPHA;
	}
	else if (!str::icmp(name, "GL_SRC_ALPHA_SATURATE"))
	{
		return GLS_SRCBLEND_ALPHA_SATURATE;
	}

	return GLS_SRCBLEND_ONE;
}

/*
===============
NameToDstBlendMode
===============
*/
static int NameToDstBlendMode(const char *name)
{
	if (!str::icmp(name, "GL_ONE"))
	{
		return GLS_DSTBLEND_ONE;
	}
	else if (!str::icmp(name, "GL_ZERO"))
	{
		return GLS_DSTBLEND_ZERO;
	}
	else if (!str::icmp(name, "GL_SRC_ALPHA"))
	{
		return GLS_DSTBLEND_SRC_ALPHA;
	}
	else if (!str::icmp(name, "GL_ONE_MINUS_SRC_ALPHA"))
	{
		return GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
	}
	else if (!str::icmp(name, "GL_DST_ALPHA"))
	{
		return GLS_DSTBLEND_DST_ALPHA;
	}
	else if (!str::icmp(name, "GL_ONE_MINUS_DST_ALPHA"))
	{
		return GLS_DSTBLEND_ONE_MINUS_DST_ALPHA;
	}
	else if (!str::icmp(name, "GL_SRC_COLOR"))
	{
		return GLS_DSTBLEND_SRC_COLOR;
	}
	else if (!str::icmp(name, "GL_ONE_MINUS_SRC_COLOR"))
	{
		return GLS_DSTBLEND_ONE_MINUS_SRC_COLOR;
	}

	return GLS_DSTBLEND_ONE;
}

static char shader_tolower(char c)
{
	return tolower(c);
}

ImageCache::ImageCache()
{
	shaderManager = nullptr;
	cachedImage = nullptr;
	bCached = false;
}

ImageCache::ImageCache(ShaderManager *s, const str& i)
	: ImageCache()
{
	shaderManager = s;
	imageName = i;
}

const char *ImageCache::GetImageName() const
{
	return imageName.c_str();
}

Image* ImageCache::GetImage() const
{
	return cachedImage.get();
}

void ImageCache::CacheImage()
{
	if (!bCached)
	{
		imageName = shaderManager->GetFileManager()->GetDefaultFileExtension(imageName.c_str(), "tga");

		if (!shaderManager->GetFileManager()->FileExists(imageName.c_str()))
		{
			imageName = shaderManager->GetFileManager()->SetFileExtension(imageName.c_str(), "jpg");
		}

		cachedImage = shaderManager->GetAssetManager()->LoadAsset<Image>(imageName.c_str());
		bCached = true;
	}
}

void ImageCache::ClearCachedImage()
{
	if (bCached)
	{
		cachedImage = nullptr;
		bCached = false;
	}
}

TextureModInfo::TextureModInfo()
{
	type = textureMod_e::TMOD_NONE;
	matrix[0][0] = matrix[0][1] = 0;
	matrix[1][0] = matrix[1][1] = 0;
	translate[0] = translate[1] = 0;
	scale[0] = scale[1] = 0;
	scroll[0] = scroll[1] = 0;
	rotateSpeed = 0;
}

WaveForm::WaveForm()
{
	func = GenFunc::GF_NONE;
	base = 0;
	amplitude = 0;
	phase = 0;
	frequency = 0;
}

SkyParms::SkyParms()
{
	cloudHeight = 0;
	outerbox[0] = nullptr;
	outerbox[1] = nullptr;
	outerbox[2] = nullptr;
	outerbox[3] = nullptr;
	outerbox[4] = nullptr;
	outerbox[5] = nullptr;
	innerbox[0] = nullptr;
	innerbox[1] = nullptr;
	innerbox[2] = nullptr;
	innerbox[3] = nullptr;
	innerbox[4] = nullptr;
	innerbox[5] = nullptr;
}

SpriteParms::SpriteParms()
{
	type = SPRITE_NONE;
	scale = 0.f;
}

DeformStage::DeformStage()
{
	deformationSpread = 0;

	bulgeWidth = 0;
	bulgeHeight = 0;
	bulgeSpeed = 0;
}

TextureBundle::TextureBundle()
{
	imageAnimationSpeed = 0.f;
	imageAnimationPhase = 0.f;
	tcGen = TextureCoordGen::TCGEN_IDENTITY;
	videoMapHandle = -1;
	isLightmap = false;
	isVideoMap = false;
	flags = 0;
}

TextureBundle::~TextureBundle()
{
}

ShaderStage::ShaderStage()
{
	rgbGen = ColorGen::CGEN_IDENTITY_LIGHTING;
	alphaGen = AlphaGen::AGEN_IDENTITY;

	constantColor[0] = 0;
	constantColor[1] = 0;
	constantColor[2] = 0;
	constantColor[3] = 0;

	distFadeRange = 0.f;
	distFadeNear = 0.f;

	stateBits = 0;

	alphaMin = 0.f;
	alphaMax = 1.f;
	alphaConst = 0;
	alphaConstMin = -1;

	bundle.push_back(TextureBundle());
}

ShaderStage::~ShaderStage()
{
}

Shader::Shader(ShaderContainer* Container)
	: shaderContainer(Container)
{
	assert(Container);

	bCached = false;
	numRef = 0;
	m_editorimage = nullptr;
	contents = 0;
	surfaceFlags = 0;
	bIsPortalSky = false;
	fDistRange = 0;
	fDistNear = 0;
	bIsSky = false;
	portalRange = 0;
	bIsPortal = false;
	cullType = CullType::CT_FRONT_SIDED;
	bNoMipMaps = false;
	bNoPicMip = false;

}

Shader::~Shader()
{
}

const char* Shader::GetFilename() const
{
	return shaderContainer->GetFilename();
}

const char* Shader::GetName() const
{
	return m_name;
}

int32_t Shader::GetContents() const
{
	return contents;
}

int32_t Shader::GetSurfaceFlags() const
{
	return surfaceFlags;
}
float Shader::GetDistRange() const
{
	return fDistRange;
}

float Shader::GetDistNear() const
{
	return fDistNear;
}

bool Shader::IsSky() const
{
	return bIsSky;
}

const SkyParms* Shader::GetSkyParms() const
{
	return &sky;
}

bool Shader::IsPortalSky() const
{
	return bIsPortalSky;
}

SpriteType Shader::GetSpriteType() const
{
	return sprite.type;
}

float Shader::GetPortalRange() const
{
	return portalRange;
}

CullType Shader::GetCullType() const
{
	return cullType;
}

bool Shader::HasMipMaps() const
{
	return !bNoMipMaps;
}

bool Shader::HasPicMip() const
{
	return !bNoPicMip;
}

bool Shader::HasTextures() const
{
	//return m_stages.size() || !m_editorimage.empty() || !m_env.empty();
	return m_stages.size();
}

const DeformStage* Shader::GetDeform(size_t index) const
{
	if (m_deforms.size())
	{
		return &m_deforms[index];
	}
	else
	{
		return nullptr;
	}
}

size_t Shader::GetNumDeforms() const
{
	return m_deforms.size();
}

const ShaderStage* Shader::GetStage(size_t index) const
{
	if (m_stages.size())
	{
		return &m_stages[index];
	}
	else
	{
		return nullptr;
	}
}

size_t Shader::GetNumStages() const
{
	return m_stages.size();
}

void Shader::AddReference()
{
	numRef++;
}

void Shader::ClearReference()
{
	assert(numRef > 0);
	numRef--;
	if (numRef == 0)
	{
		ClearCache();
	}
}

void Shader::ParseShader(Script& script)
{
	while (script.TokenAvailable(true))
	{
		const char *token = script.GetToken(false);

		if (!str::icmp(token, "qer_editorimage"))
		{
			//m_editorimage = GetShaderManager()->GetFileManager()->GetFixedPath(script.GetToken(false));
			token = script.GetToken(false);
			m_editorimage = GetShaderManager()->FindImage(token);
		}
		else if (!str::icmp(token, "deformVertexes"))
		{
			ParseDeform(script);
		}
		else if (!str::icmp(token, "spriteGen"))
		{
			token = script.GetToken(false);

			if (!str::icmp(token, "parallel"))
			{
				this->sprite.type = SPRITE_PARALLEL;
			}
			else if (!str::icmp(token, "parallel_oriented"))
			{
				sprite.type = SPRITE_PARALLEL_ORIENTED;
			}
			else if (!str::icmp(token, "parallel_upright"))
			{
				sprite.type = SPRITE_PARALLEL_UPRIGHT;
			}
			else if (!str::icmp(token, "oriented"))
			{
				sprite.type = SPRITE_ORIENTED;
			}
		}
		else if (!str::icmp(token, "spriteScale"))
		{
			token = script.GetToken(false);

			if (!token[0]) {
				continue;
			}

			sprite.scale = (float)atof(token);
		}
		else if (!str::icmp(token, "surfaceparm"))
		{
			ParseSurfaceParm(script);
		}
		// no mip maps
		else if (!str::icmp(token, "nomipmaps"))
		{
			bNoMipMaps = true;
			bNoPicMip = true;
		}
		// no picmip adjustment
		else if (!str::icmp(token, "nopicmip"))
		{
			bNoPicMip = true;
		}
		// portal
		else if (!str::icmp(token, "portal"))
		{
			bIsPortal = true;
		}
		else if (!str::icmp(token, "skyparms"))
		{
			//token = script.GetToken(false);
			//m_env = GetShaderManager()->GetFileManager()->GetFixedPath(string(token) + "_ft");
			//script.SkipToEOL();
			ParseSkyParms(script);
		}
		// cull <face>
		else if (!str::icmp(token, "cull"))
		{
			token = script.GetToken(false);
			if (token[0] == 0)
			{
				MOHPC_LOG(Warn, "missing cull parms in shader '%s' (file %s)", GetName(), GetFilename());
				continue;
			}

			if (!str::icmp(token, "none") || !str::icmp(token, "twosided") || !str::icmp(token, "disable"))
			{
				cullType = CT_TWO_SIDED;
			}
			else if (!str::icmp(token, "back") || !str::icmp(token, "backside") || !str::icmp(token, "backsided")) {
				cullType = CT_BACK_SIDED;
			}
			else if (!str::icmp(token, "front") || !str::icmp(token, "frontside") || !str::icmp(token, "frontsided")) {
				cullType = CT_FRONT_SIDED;
			}
			else {
				MOHPC_LOG(Warn, "invalid cull parm '%s' in shader '%s' (file %s)", token, GetName(), GetFilename());
			}
		}
		else if (!str::icmp(token, "portalsky"))
		{
			bIsPortalSky = true;
		}
		// stage definition
		else if (token[0] == '{')
		{
			ShaderStage* shaderStage = new(m_stages) ShaderStage();

			ParseStage(script, shaderStage);
		}
		// end of shader definition
		else if (token[0] == '}')
		{
			break;
		}
	}
}

void Shader::ParseWaveForm(Script& script, WaveForm *wave)
{
	const char *token;

	token = script.GetToken(false);
	if (token[0] == 0)
	{
		MOHPC_LOG(Warn, "missing waveform func parm in shader '%s' (file %s)", GetName(), GetFilename());
		return;
	}
	wave->func = NameToGenFunc(token);

	// BASE, AMP, PHASE, FREQ
	token = script.GetToken(false);
	if (token[0] == 0)
	{
		MOHPC_LOG(Warn, "missing waveform base parm in shader '%s' (file %s)", GetName(), GetFilename());
		return;
	}
	wave->base = (float)atof(token);

	token = script.GetToken(false);
	if (token[0] == 0)
	{
		MOHPC_LOG(Warn, "missing waveform amplitude parm in shader '%s' (file %s)", GetName(), GetFilename());
		return;
	}
	wave->amplitude = (float)atof(token);

	token = script.GetToken(false);
	if (token[0] == 0)
	{
		MOHPC_LOG(Warn, "missing waveform phase parm in shader '%s' (file %s)", GetName(), GetFilename());
		return;
	}
	wave->phase = (float)atof(token);

	token = script.GetToken(false);
	if (token[0] == 0)
	{
		MOHPC_LOG(Warn, "missing waveform frequency parm in shader '%s' (file %s)", GetName(), GetFilename());
		return;
	}
	wave->frequency = (float)atof(token);
}

void Shader::ParseTexMod(Script& script, ShaderStage *stage)
{
	stage->bundle[0].texMods.push_back(TextureModInfo());
	TextureModInfo* tmi = stage->bundle[0].texMods.Data();

	const char *token = script.GetToken(false);

	//
	// turb
	//
	if (!str::icmp(token, "turb"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing tcMod turb parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->wave.base = (float)atof(token);
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing tcMod turb in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->wave.amplitude = (float)atof(token);
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing tcMod turb in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->wave.phase = (float)atof(token);
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing tcMod turb in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->wave.frequency = (float)atof(token);

		tmi->type = TMOD_TURBULENT;
	}
	//
	// scale
	//
	else if (!str::icmp(token, "scale"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing tcMod turb in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->scale[0] = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing tcMod turb in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->scale[1] = (float)atof(token);
		tmi->type = TMOD_SCALE;
	}
	//
	// scroll
	//
	else if (!str::icmp(token, "scroll"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing scale scroll parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->scroll[0] = (float)atof(token);
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing scale scroll parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->scroll[1] = (float)atof(token);
		tmi->type = TMOD_SCROLL;
	}
	//
	// stretch
	//
	else if (!str::icmp(token, "stretch"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing stretch parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		ParseWaveForm(script, &tmi->wave);
		tmi->type = TMOD_STRETCH;
	}
	//
	// transform
	//
	else if (!str::icmp(token, "transform"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing transform parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->matrix[0][0] = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing transform parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->matrix[0][1] = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing transform parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->matrix[1][0] = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing transform parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->matrix[1][1] = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing transform parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->translate[0] = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing transform parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->translate[1] = (float)atof(token);

		tmi->type = TMOD_TRANSFORM;
	}
	//
	// rotate
	//
	else if (!str::icmp(token, "rotate"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing tcMod rotate parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->rotateSpeed = (float)atof(token);
		tmi->type = TMOD_ROTATE;
	}
	//
	// entityTranslate
	//
	else if (!str::icmp(token, "entityTranslate")) {
		tmi->type = TMOD_ENTITY_TRANSLATE;
	}
	//
	// wavetrant
	//
	else if (!str::icmp(token, "wavetrant"))
	{
		ParseWaveForm(script, &tmi->wave);
		tmi->type = TMOD_WAVETRANT;
	}
	//
	// parallax
	//
	else if(!str::icmp(token, "parallax"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing parallax parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->parallax[0] = (float)atof(token);
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing parallax parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->parallax[1] = (float)atof(token);
		tmi->type = TMOD_PARALLAX;
	}
	//
	// offset
	//
	else if(!str::icmp(token, "offset"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing offset parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->offset[0] = (float)atof(token);
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing offset parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->offset[1] = (float)atof(token);
		tmi->type = TMOD_OFFSET;
	}
	//
	// bulge
	//
	else if(!str::icmp(token, "bulge"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing bulge parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->bulge[0] = (float)atof(token);
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing bulge parms in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		tmi->bulge[1] = (float)atof(token);
		tmi->type = TMOD_BULGE;
	}
	else {
		MOHPC_LOG(Warn, "unknown tcMod '%s' in shader '%s' (file %s)", token, GetName(), GetFilename());
	}
}

bool Shader::ParseStage(Script& script, ShaderStage *stage)
{
	int32_t bundleNum = 0;
	int32_t atestBits = 0;
	int32_t blendSrcBits = 0, blendDstBits = 0;

	while (script.TokenAvailable(true))
	{
		const char *token = script.GetToken(false);
		if (token[0] == '}')
		{
			break;
		}

		if (!str::icmp(token, "map") || !str::icmp(token, "clampmap"))
		{
			token = script.GetToken(false);
			stage->bundle[bundleNum].image.push_back(GetShaderManager()->FindImage(token)); // GetShaderManager()->GetFileManager()->GetFixedPath(token);
		}
		else if (!str::icmp(token, "alphafunc"))
		{
			token = script.GetToken(false);
			atestBits = NameToAFunc(token);
		}
		else if (!str::icmp(token, "blendfunc"))
		{
			token = script.GetToken(false);

			if (!str::icmp(token, "add"))
			{
				blendSrcBits = GLS_SRCBLEND_ONE;
				blendDstBits = GLS_DSTBLEND_ONE;
			}
			else if (!str::icmp(token, "filter"))
			{
				blendSrcBits = GLS_SRCBLEND_DST_COLOR;
				blendDstBits = GLS_DSTBLEND_ZERO;
			}
			else if (!str::icmp(token, "blend"))
			{
				blendSrcBits = GLS_SRCBLEND_SRC_ALPHA;
				blendDstBits = GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
			}
			else if (!str::icmp(token, "alphaadd"))
			{
				blendSrcBits = GLS_SRCBLEND_SRC_ALPHA;
				blendDstBits = GLS_DSTBLEND_ONE;
			}
			else
			{
				// complex double blends
				blendSrcBits = NameToSrcBlendMode(token);
				token = script.GetToken(false);
				blendDstBits = NameToDstBlendMode(token);
			}
		}
		//
		// rgbGen
		//
		else if (!str::icmp(token, "rgbGen"))
		{
			token = script.GetToken(false);
			if (token[0] == 0)
			{
				MOHPC_LOG(Warn, "missing parameters for rgbGen in shader '%s' (file %s)", GetName(), GetFilename());
				continue;
			}

			if (!str::icmp(token, "wave"))
			{
				ParseWaveForm(script, &stage->rgbWave);
				stage->rgbGen = CGEN_WAVEFORM;
			}
			else if (!str::icmp(token, "colorwave"))
			{
				Vector color = script.GetVector(false);
				stage->constantColor[0] = color[0];
				stage->constantColor[1] = color[1];
				stage->constantColor[2] = color[2];
				ParseWaveForm(script, &stage->rgbWave);
				stage->rgbGen = CGEN_MULTIPLY_BY_WAVEFORM;
			}
			else if (!str::icmp(token, "const") || !str::icmp(token, "constant"))
			{
				Vector color = script.GetVector(false);
				stage->constantColor[0] = color[0];
				stage->constantColor[1] = color[1];
				stage->constantColor[2] = color[2];

				stage->rgbGen = CGEN_CONST;
			}
			else if (!str::icmp(token, "identity"))
			{
				stage->rgbGen = CGEN_IDENTITY;
			}
			else if (!str::icmp(token, "identityLighting"))
			{
				stage->rgbGen = CGEN_IDENTITY_LIGHTING;
			}
			else if (!str::icmp(token, "entity") || !str::icmp(token, "fromentity"))
			{
				stage->rgbGen = CGEN_ENTITY;
			}
			else if (!str::icmp(token, "oneMinusEntity"))
			{
				stage->rgbGen = CGEN_ONE_MINUS_ENTITY;
			}
			else if (!str::icmp(token, "vertex") || !str::icmp(token, "fromclient"))
			{
				stage->rgbGen = CGEN_VERTEX;
				if (stage->alphaGen == AGEN_IDENTITY) {
					stage->alphaGen = AGEN_VERTEX;
				}
			}
			else if (!str::icmp(token, "exactVertex"))
			{
				stage->rgbGen = CGEN_EXACT_VERTEX;
			}
			else if (!str::icmp(token, "vertexLit"))
			{
				stage->rgbGen = CGEN_VERTEX_LIT;
				if (stage->alphaGen == AGEN_IDENTITY) {
					stage->alphaGen = AGEN_VERTEX;
				}
			}
			else if (!str::icmp(token, "exactVertexLit"))
			{
				stage->rgbGen = CGEN_EXACT_VERTEX_LIT;
			}
			else if (!str::icmp(token, "lightingGrid"))
			{
				stage->rgbGen = CGEN_LIGHTING_GRID;
			}
			else if (!str::icmp(token, "lightingSpherical"))
			{
				//stage->rgbGen = CGEN_LIGHTING_SPHERICAL;
				stage->rgbGen = CGEN_LIGHTING_DIFFUSE;
			}
			else if (!str::icmp(token, "lightingDiffuse"))
			{
				stage->rgbGen = CGEN_LIGHTING_DIFFUSE;
			}
			else if (!str::icmp(token, "oneMinusVertex"))
			{
				stage->rgbGen = CGEN_ONE_MINUS_VERTEX;
			}
			else if (!str::icmp(token, "global"))
			{
				stage->rgbGen = CGEN_GLOBAL_COLOR;
			}
			else if (!str::icmp(token, "static"))
			{
				stage->rgbGen = CGEN_STATIC;

				if (stage->alphaGen == AGEN_IDENTITY) {
					stage->alphaGen = AGEN_VERTEX;
				}
			}
			else if (!str::icmp(token, "sCoord") || !str::icmp(token, "tCoord"))
			{
				if (!str::icmp(token, "sCoord")) {
					stage->rgbGen = CGEN_SCOORD;
				}
				else if (!str::icmp(token, "tCoord")) {
					stage->rgbGen = CGEN_TCOORD;
				}

				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->alphaConstMin = 0;
				stage->alphaConst = -1;

				token = script.GetToken(false);
				if (token[0])
				{
					stage->alphaMin = (float)atof(token);

					token = script.GetToken(false);
					if (token[0])
					{
						stage->alphaMax = (float)atof(token);

						token = script.GetToken(false);
						if (token[0])
						{
							stage->alphaConstMin = (uint8_t)(atof(token) * 255.0);

							token = script.GetToken(false);
							if (token[0])
							{
								stage->alphaConst = (uint8_t)(atof(token) * 255.0);
							}
							else
							{
								MOHPC_LOG(Warn, "missing rgbGen sCoord or tCoord parm 'max' in shader '%s' (file %s)", GetName(), GetFilename());
							}
						}
					}
				}
			}
			else if (!str::icmp(token, "dot"))
			{
				//shader.needsNormal = qtrue;
				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->rgbGen = CGEN_DOT;

				token = script.GetToken(false);
				if (token[0])
				{
					stage->alphaMin = (float)atof(token);

					token = script.GetToken(false);
					if (token[0])
					{
						stage->alphaMax = (float)atof(token);
					}
				}
			}
			else if (!str::icmp(token, "oneminusdot"))
			{
				//shader.needsNormal = qtrue;
				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->rgbGen = CGEN_ONE_MINUS_DOT;

				token = script.GetToken(false);
				if (token[0])
				{
					stage->alphaMin = (float)atof(token);

					token = script.GetToken(false);
					if (token[0])
					{
						stage->alphaMax = (float)atof(token);
					}
				}
			}
			else
			{
				MOHPC_LOG(Warn, "unknown rgbGen parameter '%s' in shader '%s' (file %s)", token, GetName(), GetFilename());
				continue;
			}
		}
		//
		// alphaGen 
		//
		else if (!str::icmp(token, "alphaGen"))
		{
			token = script.GetToken(false);
			if (token[0] == 0)
			{
				MOHPC_LOG(Warn, "missing parameters for alphaGen in shader '%s' (file %s)", GetName(), GetFilename());
				continue;
			}

			if (!str::icmp(token, "wave"))
			{
				ParseWaveForm(script, &stage->alphaWave);
				stage->alphaGen = AGEN_WAVEFORM;
			}
			else if (!str::icmp(token, "const") || !str::icmp(token, "constant"))
			{
				token = script.GetToken(false);
				stage->constantColor[3] = 255 * (float)atof(token);
				stage->alphaGen = AGEN_CONST;
			}
			else if (!str::icmp(token, "identity"))
			{
				stage->alphaGen = AGEN_IDENTITY;
			}
			else if (!str::icmp(token, "entity") || !str::icmp(token, "fromentity"))
			{
				stage->alphaGen = AGEN_ENTITY;
			}
			else if (!str::icmp(token, "oneMinusEntity"))
			{
				stage->alphaGen = AGEN_ONE_MINUS_ENTITY;
			}
			else if (!str::icmp(token, "vertex"))
			{
				stage->alphaGen = AGEN_VERTEX;
			}
			else if (!str::icmp(token, "lightingSpecular"))
			{
				stage->alphaGen = AGEN_LIGHTING_SPECULAR;
			}
			else if (!str::icmp(token, "oneMinusVertex"))
			{
				stage->alphaGen = AGEN_ONE_MINUS_VERTEX;
			}
			else if (!str::icmp(token, "portal"))
			{
				stage->alphaGen = AGEN_PORTAL;
				token = script.GetToken(false);
				if (token[0] == 0)
				{
					portalRange = 256;
					MOHPC_LOG(Warn, "missing range parameter for alphaGen portal in shader '%s', defaulting to 256 (file %s)", GetName(), GetFilename());
				}
				else
				{
					portalRange = (float)atof(token);
				}
			}
			else if (!str::icmp(token, "distFade") || !str::icmp(token, "oneMinusDistFade")
				|| !str::icmp(token, "tikiDistFade") || !str::icmp(token, "oneMinusTikiDistFade"))
			{
				if (!str::icmp(token, "distFade")) {
					stage->alphaGen = AGEN_DIST_FADE;
				} else if (!str::icmp(token, "oneMinusDistFade")) {
					stage->alphaGen = AGEN_ONE_MINUS_DIST_FADE;
				} else if (!str::icmp(token, "oneMinusDistFade")) {
					stage->alphaGen = AGEN_TIKI_DIST_FADE;
				} else if (!str::icmp(token, "oneMinusDistFade")) {
					stage->alphaGen = AGEN_ONE_MINUS_TIKI_DIST_FADE;
				}

				stage->distFadeNear = 256;
				stage->distFadeRange = 256;

				token = script.GetToken(false);
				if (token[0])
				{
					fDistNear = (float)atof(token);

					token = script.GetToken(false);
					if (token[0])
					{
						fDistRange = (float)atof(token);
					}
				}
			}
			else if (!str::icmp(token, "heightFade"))
			{
				stage->alphaGen = AGEN_HEIGHT_FADE;
				stage->distFadeNear = 256.f;
				stage->distFadeRange = 512.f;

				token = script.GetToken(false);
				if (token[0])
				{
					stage->distFadeNear = (float)atof(token);

					token = script.GetToken(false);
					if (token[0])
					{
						stage->distFadeRange = (float)atof(token);
					}
				}
			}
			else if (!str::icmp(token, "dot"))
			{
				//shader.needsNormal = qtrue;
				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->alphaGen = AGEN_DOT;

				token = script.GetToken(false);
				if (token[0])
				{
					stage->alphaMin = (float)atof(token);

					token = script.GetToken(false);
					if (token[0])
					{
						stage->alphaMax = (float)atof(token);
					}
				}
			}
			else if (!str::icmp(token, "dotView"))
			{
				//shader.needsNormal = qtrue;
				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->alphaGen = AGEN_DOT_VIEW;

				token = script.GetToken(false);
				if (token[0])
				{
					stage->alphaMin = (float)atof(token);

					token = script.GetToken(false);
					if (token[0])
					{
						stage->alphaMax = (float)atof(token);
					}
				}
			}
			else if (!str::icmp(token, "oneMinusDotView"))
			{
				//shader.needsNormal = qtrue;
				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->alphaGen = AGEN_ONE_MINUS_DOT_VIEW;

				token = script.GetToken(false);
				if (token[0])
				{
					stage->alphaMin = (float)atof(token);

					token = script.GetToken(false);
					if (token[0])
					{
						stage->alphaMax = (float)atof(token);
					}
				}
			}
			else if (!str::icmp(token, "oneMinusDot"))
			{
				//shader.needsNormal = qtrue;
				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->alphaGen = AGEN_ONE_MINUS_DOT;

				token = script.GetToken(false);
				if (token[0])
				{
					stage->alphaMin = (float)atof(token);

					token = script.GetToken(false);
					if (token[0])
					{
						stage->alphaMax = (float)atof(token);
					}
				}
			}
			else if (!str::icmp(token, "skyAlpha"))
			{
				stage->alphaGen = AGEN_SKYALPHA;
			}
			else if (!str::icmp(token, "oneMinusSkyAlpha"))
			{
				stage->alphaGen = AGEN_ONE_MINUS_SKYALPHA;
			}
			else if (!str::icmp(token, "sCoord") || !str::icmp(token, "tCoord"))
			{
				if (!str::icmp(token, "sCoord")) {
					stage->alphaGen = AGEN_SCOORD;
				}
				else if (!str::icmp(token, "tCoord")) {
					stage->alphaGen = AGEN_TCOORD;
				}

				stage->alphaMin = 0.0;
				stage->alphaMax = 1.0;
				stage->alphaConstMin = 0;
				stage->alphaConst = -1;

				token = script.GetToken(false);
				if (token[0])
				{
					stage->alphaMin = (float)atof(token);

					token = script.GetToken(false);
					if (token[0])
					{
						stage->alphaMax = (float)atof(token);

						token = script.GetToken(false);
						if (token[0])
						{
							stage->alphaConstMin = (uint8_t)(atof(token) * 255.0);

							token = script.GetToken(false);
							if (token[0])
							{
								stage->alphaConst = (uint8_t)(atof(token) * 255.0);
							}
							else
							{
								MOHPC_LOG(Warn, "missing rgbGen sCoord or tCoord parm 'max' in shader '%s' (file %s)", GetName(), GetFilename());
							}
						}
					}
				}
			}
			else if(!str::icmp(token, "alphaGen")) {
				stage->alphaGen = AGEN_FROM_ENTITY;
			}
			else if (!str::icmp(token, "fromClient")) {
				stage->alphaGen = AGEN_FROM_CLIENT;
			}
			else
			{
				MOHPC_LOG(Warn, "unknown alphaGen parameter '%s' in shader '%s' (file %s)", token, GetName(), GetFilename());
				continue;
			}
		}
		//
		// tcGen <function>
		//
		else if (!str::icmp(token, "texgen") || !str::icmp(token, "tcGen"))
		{
			token = script.GetToken(false);
			if (token[0] == 0)
			{
				MOHPC_LOG(Warn, "missing texgen parm in shader '%s' (file %s)", GetName(), GetFilename());
				continue;
			}

			if (!str::icmp(token, "environment"))
			{
				//shader.needsNormal = qtrue;
				stage->bundle[bundleNum].tcGen = TCGEN_ENVIRONMENT_MAPPED;
			}
			else if (!str::icmp(token, "environmentmodel"))
			{
				//shader.needsNormal = qtrue;
				stage->bundle[bundleNum].tcGen = TCGEN_ENVIRONMENT_MAPPED2;
			}
			else if (!str::icmp(token, "sunreflection"))
			{
				//shader.needsNormal = qtrue;
				stage->bundle[bundleNum].tcGen = TCGEN_SUN_REFLECTION;
			}
			else if (!str::icmp(token, "lightmap"))
			{
				stage->bundle[bundleNum].tcGen = TCGEN_LIGHTMAP;
			}
			else if (!str::icmp(token, "texture") || !str::icmp(token, "base"))
			{
				stage->bundle[bundleNum].tcGen = TCGEN_TEXTURE;
			}
			else if (!str::icmp(token, "vector"))
			{
				stage->bundle[bundleNum].tcGenVectors[0] = script.GetVector(false);
				stage->bundle[bundleNum].tcGenVectors[1] = script.GetVector(false);

				stage->bundle[bundleNum].tcGen = TCGEN_VECTOR;
			}
			else
			{
				MOHPC_LOG(Warn, "unknown texgen parm '%s' in shader '%s' (file %s)", token, GetName(), GetFilename());
			}
		}
		//
		// tcMod <type> <...>
		//
		else if (!str::icmp(token, "tcMod"))
		{
			ParseTexMod(script, stage);
		}
		else if (!str::icmp(token, "nextbundle"))
		{
			bundleNum++;
			stage->bundle.push_back(TextureBundle());
		}
		else if (!str::icmp(token, "animmap") || !str::icmp(token, "animmaponce") || !str::icmp(token, "animmapphase"))
		{
			if (!str::icmp(token, "animmap") || !str::icmp(token, "animmaponce"))
			{
				token = script.GetToken(false);

				bool bHasFrequency = true;

				const char* p = token;
				while (*p)
				{
					if (!isdigit(*p))
					{
						bHasFrequency = false;
						break;
					}
					p++;
				}

				if (bHasFrequency)
				{
					stage->bundle[bundleNum].imageAnimationSpeed = (float)atof(token);
				}
			}
			else if (!str::icmp(token, "animmapphase"))
			{
				stage->bundle[bundleNum].imageAnimationSpeed = script.GetFloat(false);
				stage->bundle[bundleNum].imageAnimationPhase = script.GetFloat(false);
			}

			stage->bundle[bundleNum].image.clear();
			while (script.TokenAvailable(false))
			{
				token = script.GetToken(false);
				ImageCache* image = GetShaderManager()->FindImage(token);
				stage->bundle[bundleNum].image.push_back(image);
			}
		}
	}

	//
	// if cgen isn't explicitly specified, use either identity or identitylighting
	//
	if (stage->rgbGen == CGEN_BAD)
	{
		if (blendSrcBits == 0 ||
			blendSrcBits == GLS_SRCBLEND_ONE ||
			blendSrcBits == GLS_SRCBLEND_SRC_ALPHA)
		{
			stage->rgbGen = CGEN_IDENTITY_LIGHTING;
		}
		else
		{
			stage->rgbGen = CGEN_IDENTITY;
		}
	}

	//
	// implicitly assume that a GL_ONE GL_ZERO blend mask disables blending
	//
	if ((blendSrcBits == GLS_SRCBLEND_ONE) &&
		(blendDstBits == GLS_DSTBLEND_ZERO))
	{
		blendDstBits = blendSrcBits = 0;
	}

	// decide which agens we can skip
	if (stage->alphaGen == AGEN_IDENTITY) {
		if (stage->rgbGen == CGEN_IDENTITY
			|| stage->rgbGen == CGEN_LIGHTING_DIFFUSE) {
			stage->alphaGen = AGEN_SKIP;
		}
	}

	stage->stateBits |= atestBits | blendSrcBits | blendDstBits;

	return true;
}

void Shader::ParseDeform(Script& script)
{
	const char *token = script.GetToken(false);
	if (token[0] == 0)
	{
		MOHPC_LOG(Warn, "missing deform parm in shader '%s' (file %s)", GetName(), GetFilename());
		return;
	}

	m_deforms.push_back(DeformStage());
	DeformStage* ds = m_deforms.Data();

	if (!str::icmp(token, "projectionShadow"))
	{
		ds->deformation = DEFORM_PROJECTION_SHADOW;
	}
	else if (!str::icmp(token, "autosprite"))
	{
		ds->deformation = DEFORM_AUTOSPRITE;
		sprite.type = SPRITE_ORIENTED;
	}
	else if (!str::icmp(token, "autosprite2"))
	{
		ds->deformation = DEFORM_AUTOSPRITE2;
		sprite.type = SPRITE_ORIENTED;
	}
	else if (!strnicmp(token, "text", 4))
	{
		int		n;

		n = token[4] - '0';
		if (n < 0 || n > 7) {
			n = 0;
		}
		ds->deformation = (DeformMod)(DEFORM_TEXT0 + n);
	}
	else if (!str::icmp(token, "bulge"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing deformVertexes bulge parm in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		ds->bulgeWidth = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing deformVertexes bulge parm in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		ds->bulgeHeight = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing deformVertexes bulge parm in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		ds->bulgeSpeed = (float)atof(token);

		ds->deformation = DEFORM_BULGE;
	}
	else if (!str::icmp(token, "wave"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing deformVertexes parm in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}

		if (atof(token) != 0)
		{
			ds->deformationSpread = (float)(1.0 / atof(token));
		}
		else
		{
			ds->deformationSpread = 100.0f;
			MOHPC_LOG(Warn, "illegal div value of 0 in deformVertexes command for shader '%s' (file %s)", GetName(), GetFilename());
		}

		ParseWaveForm(script, &ds->deformationWave);
		ds->deformation = DEFORM_WAVE;
	}
	else if (!str::icmp(token, "normal"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing deformVertexes parm in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		ds->deformationWave.amplitude = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			MOHPC_LOG(Warn, "missing deformVertexes parm in shader '%s' (file %s)", GetName(), GetFilename());
			return;
		}
		ds->deformationWave.frequency = (float)atof(token);

		ds->deformation = DEFORM_NORMALS;
	}
	else if (!str::icmp(token, "move"))
	{
		int		i;

		for (i = 0; i < 3; i++)
		{
			token = script.GetToken(false);
			if (token[0] == 0)
			{
				MOHPC_LOG(Warn, "missing deformVertexes parm in shader '%s' (file %s)", GetName(), GetFilename());
				return;
			}
			ds->moveVector[i] = (float)atof(token);
		}

		ParseWaveForm(script, &ds->deformationWave);
		ds->deformation = DEFORM_MOVE;
	}
	else if (!str::icmp(token, "flap"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			return;
		}

		if (*token == 's')
		{
			ds->deformation = DEFORM_FLAP_S;
		}
		else if (*token == 't')
		{
			ds->deformation = DEFORM_FLAP_T;
		}

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			return;
		}

		if (atof(token) != 0)
		{
			ds->deformationSpread = (float)(1.0 / atof(token));
		}
		else
		{
			ds->deformationSpread = 100.0f;
		}

		ParseWaveForm(script, &ds->deformationWave);
	}
}

void Shader::ParseSkyParms(Script& script)
{
	static const char *suf[6] = { "rt", "bk", "lf", "ft", "up", "dn" };

	// outerbox
	const char *token = script.GetToken(false);
	if (token[0] == 0)
	{
		MOHPC_LOG(Warn, "'skyParms' missing parameter in shader '%s'", GetName(), GetFilename());
		return;
	}
	if (strcmp(token, "-"))
	{
		for (int32_t i = 0; i < 6; i++)
		{
			const str imagename = str(token) + "_" + str(suf[i]) + str(".tga");
			sky.outerbox[i] = GetShaderManager()->FindImage(imagename.c_str());

			/*
			if (!sky.outerbox[i])
			{
				sky.outerbox[i] = tr.defaultImage;
			}
			*/
		}
	}

	// cloudheight
	token = script.GetToken(false);
	if (token[0] == 0)
	{
		MOHPC_LOG(Warn, "'skyParms' missing parameter in shader '%s'", GetName(), GetFilename());
		return;
	}

	sky.cloudHeight = (float)atof(token);
	if (!sky.cloudHeight)
	{
		sky.cloudHeight = 512;
	}

	//R_InitSkyTexCoords(shader.sky.cloudHeight);

	// innerbox
	token = script.GetToken(false);
	if (token[0] == 0)
	{
		MOHPC_LOG(Warn, "'skyParms' missing parameter in shader '%s'", GetName(), GetFilename());
		return;
	}
	if (strcmp(token, "-"))
	{
		for (int32_t i = 0; i < 6; i++)
		{
			const str imagename = str(token) + "_" + str(suf[i]) + str(".tga");
			sky.innerbox[i] = GetShaderManager()->FindImage(imagename.c_str());

			/*
			if (!sky.innerbox[i])
			{
				sky.innerbox[i] = tr.defaultImage;
			}
			*/
		}
	}

	bIsSky = true;
}

void Shader::ParseSurfaceParm(Script& script)
{
	struct paramToFlags
	{
		const char *param;
		uint32_t surfaceFlags;
		uint32_t contents;
	};
	static paramToFlags paramsToFlags[] =
	{
		{ "water",			0,					CONTENTS_WATER },
		{ "slime",			0,					CONTENTS_SLIME },
		{ "lava",			0,					CONTENTS_LAVA },
		{ "playerclip",		0,					CONTENTS_PLAYERCLIP },
		{ "monsterclip",	0,					CONTENTS_MONSTERCLIP },
		{ "nodrop",			0,					(uint32_t)CONTENTS_NODROP },
		{ "nonsolid",		SURF_NONSOLID,		0 },

		{ "origin",			0,					CONTENTS_ORIGIN },
		{ "ladder",			SURF_LADDER,		CONTENTS_LADDER },
		{ "hint",			SURF_HINT,			0 },
		{ "trans",			0,					CONTENTS_TRANSLUCENT },
		{ "detail",			0,					CONTENTS_DETAIL },
		{ "structural",		0,					CONTENTS_STRUCTURAL },
		{ "areaportal",		0,					CONTENTS_AREAPORTAL },
		{ "clusterportal",	0,					CONTENTS_CLUSTERPORTAL },

		{ "fog",			0,					CONTENTS_FOG },
		{ "sky",			SURF_SKY,			0 },
		{ "alphashadow",	SURF_ALPHASHADOW,	0 },
		{ "hint",			SURF_HINT,			0 },

		{ "slick",			SURF_SLICK,			0 },
		{ "noimpact",		SURF_NOIMPACT,		0 },
		{ "nomarks",		SURF_NOMARKS,		0 },
		{ "ladder",			SURF_LADDER,		0 },
		{ "nodamage",		SURF_NODAMAGE,		0 },
		{ "nosteps",		SURF_NOSTEPS,		0 },

		{ "nodraw",			SURF_NODRAW,		0 },
		{ "nolightmap",		SURF_NOLIGHTMAP,	0 },
		{ "nodlight",		SURF_NODLIGHT,		0 },

		{ "paper",			SURF_PAPER,			0 },
		{ "wood",			SURF_WOOD,			0 },
		{ "metal",			SURF_METAL,			0 },
		{ "rock",			SURF_ROCK,			0 },
		{ "dirt",			SURF_DIRT,			0 },
		{ "grill",			SURF_GRILL,			0 },
		{ "grass",			SURF_GRASS,			0 },
		{ "mud",			SURF_MUD,			0 },
		{ "puddle",			SURF_PUDDLE,		0 },
		{ "glass",			SURF_GLASS,			0 },
		{ "gravel",			SURF_GRAVEL,		0 },
		{ "sand",			SURF_SAND,			0 },
		{ "foliage",		SURF_FOLIAGE,		0 },
		{ "snow",			SURF_SNOW,			0 },
		{ "carpet",			SURF_CARPET,		0 },
	};

	const char *token = script.GetToken(false);

	for (intptr_t i = 0; i < sizeof(paramsToFlags) / sizeof(paramsToFlags[0]); i++)
	{
		if (!str::icmp(paramsToFlags[i].param, token))
		{
			surfaceFlags |= paramsToFlags[i].surfaceFlags;
			contents |= paramsToFlags[i].contents;
			break;
		}
	}
}

void Shader::Precache()
{
	if (!bCached)
	{
		if (m_editorimage)
		{
			m_editorimage->CacheImage();
		}

		for (size_t i = 0; i < m_stages.size(); i++)
		{
			ShaderStage* stage = &m_stages[i];
			for (size_t j = 0; j < stage->bundle.size(); j++)
			{
				TextureBundle* bundle = &stage->bundle[j];
				for (size_t k = 0; k < bundle->image.size(); k++)
				{
					ImageCache* cachedImage = bundle->image[k];
					if (cachedImage)
					{
						// Load the image
						cachedImage->CacheImage();
					}
				}
			}
		}

		for (size_t i = 0; i < 6; i++)
		{
			ImageCache* cachedImage = sky.outerbox[i];
			if (cachedImage)
			{
				// Load the image
				cachedImage->CacheImage();
			}
		}

		for (size_t i = 0; i < 6; i++)
		{
			ImageCache* cachedImage = sky.innerbox[i];
			if (cachedImage)
			{
				// Load the image
				cachedImage->CacheImage();
			}
		}
	}
}

void Shader::ClearCache()
{
	if (bCached)
	{
		if (m_editorimage)
		{
			m_editorimage->ClearCachedImage();
		}

		for (size_t i = 0; i < m_stages.size(); i++)
		{
			ShaderStage* stage = &m_stages[i];
			for (size_t j = 0; j < stage->bundle.size(); j++)
			{
				TextureBundle* bundle = &stage->bundle[j];
				for (size_t k = 0; k < bundle->image.size(); k++)
				{
					ImageCache* cachedImage = bundle->image[k];
					if (cachedImage)
					{
						// Load the image
						cachedImage->ClearCachedImage();
					}
				}
			}
		}

		for (size_t i = 0; i < 6; i++)
		{
			ImageCache* cachedImage = sky.outerbox[i];
			if (cachedImage)
			{
				// Load the image
				cachedImage->ClearCachedImage();
			}
		}

		for (size_t i = 0; i < 6; i++)
		{
			ImageCache* cachedImage = sky.innerbox[i];
			if (cachedImage)
			{
				// Load the image
				cachedImage->ClearCachedImage();
			}
		}

		bCached = false;
	}
}

ShaderRef::ShaderRef()
	: shader(NULL)
{
}

ShaderRef::ShaderRef(Shader* inShader)
	: shader(inShader)
{
	if (shader)
	{
		shader->AddReference();
	}
}

ShaderRef::ShaderRef(const ShaderRef& shaderRef)
{
	shader = shaderRef.shader;
	if (shader)
	{
		shader->AddReference();
	}
}

ShaderRef::ShaderRef(ShaderRef&& shaderRef)
{
	shader = shaderRef.shader;
	shaderRef.shader = NULL;
}

ShaderRef::~ShaderRef()
{
	if (shader)
	{
		shader->ClearReference();
	}
}

ShaderRef::operator bool() const
{
	return shader != nullptr;
}

ShaderRef::operator Shader*() const
{
	return shader;
}

ShaderRef& ShaderRef::operator=(Shader* other)
{
	if (shader)
	{
		shader->ClearReference();
	}

	shader = other;
	if (shader)
	{
		shader->AddReference();
	}

	return *this;
}

ShaderRef& ShaderRef::operator=(const ShaderRef& other)
{
	if (shader)
	{
		shader->ClearReference();
	}

	shader = other.shader;
	if (shader)
	{
		shader->AddReference();
	}

	return *this;
}

ShaderRef& ShaderRef::operator=(ShaderRef&& other)
{
	shader = other.shader;
	other.shader = NULL;
	return *this;
}

bool ShaderRef::operator==(const Shader* other) const
{
	return shader == other;
}

bool ShaderRef::operator!=(const Shader* other) const
{
	return shader != other;
}

Shader* ShaderRef::operator->() const
{
	return shader;
}

ShaderManager* Shader::GetShaderManager() const
{
	return shaderContainer->GetShaderManager();
}

ShaderContainer* Shader::GetShaderContainer()
{
	return shaderContainer;
}

const ShaderContainer* Shader::GetShaderContainer() const
{
	return shaderContainer;
}

ShaderContainer::ShaderContainer(ShaderManager* shaderManager, const str& filename)
	: m_filename(filename)
	, m_shaderManager(shaderManager)
{
}

void ShaderContainer::AddShader(const ShaderPtr& Shader)
{
	m_shaderList.AddObject(Shader);
}

void ShaderContainer::RemoveShader(const ShaderPtr& Shader)
{
	/*
	for (auto it = m_shaderList.begin(); it != m_shaderList.end(); ++it)
	{
		if (*it == Shader)
		{
			m_shaderList.erase(it);
			break;
		}
	}
	*/
	m_shaderList.RemoveObject(Shader);
}

size_t ShaderContainer::GetNumShaders() const
{
	return m_shaderList.NumObjects();
}

const Shader* ShaderContainer::GetShader(size_t num) const
{
	if (num >= m_shaderList.NumObjects())
	{
		return nullptr;
	}

	return m_shaderList.ObjectAt(num + 1).get();
}

ShaderManager *ShaderContainer::GetShaderManager() const
{
	return m_shaderManager;
}

const str& ShaderContainer::GetFilename() const
{
	return m_filename;
}

CLASS_DEFINITION(ShaderManager);
ShaderManager::ShaderManager()
	: m_defaultShaderContainer(this, "")
	, m_defaultshader(&m_defaultShaderContainer)
{
}

ShaderManager::~ShaderManager(void)
{
	/*
	for (auto it : m_nametoshader)
	{
		Shader* shader = it.second;
		delete shader;
	}

	for (auto it : m_images)
	{
		ImageCache* cachedImage = it.second;
		delete cachedImage;
	}
	*/
}

void ShaderManager::Init()
{
	//char **pszFiles;
	FileEntryList entryList;
	clock_t start, end;
	double seconds;

	start = clock();

	//pszFiles = FS_ListFilteredFiles( "scripts", "shader", NULL, false, &iNumFiles );
	entryList = GetFileManager()->ListFilteredFiles("/scripts", "shader", false);

	//g_defaultshader = AllocShader();

	// reserve at least 7000 shader objects
	m_nametoshader.resize(7000);
	m_images.Resize(7000);

	MOHPC_LOG(Info, "loading shaders");

	// Parse all shaders
	ParseShaders(entryList);

	//FS_FreeFileList( pszFiles );

	end = clock();

	seconds = (float)(end - start) / CLOCKS_PER_SEC;

	MOHPC_LOG(Info, "%d shaders, loaded in %lf", m_nametoshader.size(), seconds);
}

void ShaderManager::ParseShaders(const FileEntryList& files)
{
	const char *buffer;

	const size_t numFiles = files.GetNumFiles();
	m_shaderContainers.Resize(numFiles);

	for (size_t i = 0; i < numFiles; i++)
	{
		//FS_ReadFile(string("scripts/") + files[i], (void **)&buffer);
		const str& filename = files.GetFileEntry(i)->GetStr();

		FilePtr file = GetFileManager()->OpenFile(filename.c_str());
		if (!file) {
			continue;
		}

		const uint64_t length = file->ReadBuffer((void**)&buffer);

		// Parse the shader
		const ShaderContainerPtr shaderContainer = ParseShaderContainer(filename, buffer, length);
		if(shaderContainer)
		{
			m_shaderContainers.AddObject(shaderContainer);
			m_fileShaderMap.addKeyValue(filename) = shaderContainer;
		}
	}
}

ShaderContainerPtr ShaderManager::ParseShaderContainer(const str& fileName, const char *buffer, uint64_t length)
{
	if (!length) length = strlen(buffer) + 1;

	Script script;
	script.Parse(buffer, length, "");

	if(!length || !script.TokenAvailable(true))
	{
		MOHPC_LOG(Info, "Nothing to parse in %s.", fileName.c_str());
		return nullptr;
	}

	ShaderContainerPtr shaderContainer = makeShared<ShaderContainer>(this, fileName);

	while (script.TokenAvailable(true))
	{
		const char *token = script.GetToken(false);
		if (!token[0])
		{
			MOHPC_LOG(Warn, "Empty token, file %s.", fileName.c_str());
			return nullptr;
		}

		if (token[0] == '/' && token[1] == '*')
		{
			bool bTokenAvailable;
			while ((bTokenAvailable = script.TokenAvailable(true)))
			{
				if (token[0] == '*' && token[1] == '/') {
					break;
				}
				script.GetToken(false);
			}
			if (!bTokenAvailable)
			{
				MOHPC_LOG(Warn, "Unexpected end of file found while parsing a comment for file %s.", fileName.c_str());
				return nullptr;
			}
			continue;
		}

		const str shadername = token;

		token = script.GetToken(false);

		if (token[0] != '{')
		{
			MOHPC_LOG(Warn, "A beginning brace was expected for file %s.", fileName.c_str());
			return nullptr;
		}

		const ShaderPtr shader = AllocShader(shaderContainer.get());
		shader->m_name = shadername;
		shader->m_name.tolower();
		//std::transform(shader->m_name.begin(), shader->m_name.end(), shader->m_name.begin(), &shader_tolower);

		shaderContainer->AddShader(shader);

		shader->ParseShader(script);
		//shader->Precache();

		AddShader(shader);
	}

	return shaderContainer;
}

/*
string ShaderManager::ParseTextureExtension(const string& name)
{
	// try with jpg
	string filename = name + ".jpg";
	FilePtr file = GetFileManager()->OpenFile(filename.c_str());
	if(!file)
	{
		// try with tga
		filename = name + ".tga";
		file = GetFileManager()->OpenFile(filename.c_str());
		if(!file)
		{
			return "";
		}
	}

	return filename;
}
*/

ShaderPtr ShaderManager::AllocShader(ShaderContainer *shaderContainer)
{
	return makeShared<Shader>(shaderContainer);
}

void ShaderManager::FreeShader(const ShaderPtr& shader)
{
	//m_shaders.RemoveObject(shader);
	m_nametoshader.remove(shader->GetName());
}

void ShaderManager::AddShader(const ShaderPtr &shader)
{
	//bool bHasTexture = true;

	/*
	if (!shader->m_editorimage.length())
	{
		if (shader->m_stages.size())
		{
			//shader->m_editorimage = shader->m_stages[0].bundle[0].image[0];
		}
		else
		{
			bHasTexture = false;
		}
	}
	*/

	if (!shader->m_stages.size() && !shader->bIsSky && !shader->m_editorimage) {
		shader->m_editorimage = FindImage(shader->m_name.c_str());
	}

	/*
	if (!shader->m_stages.size())
	{
		if (shader->m_editorimage.length())
		{
			shader->m_texture.push_back(shader->m_editorimage);
		}
		else
		{
			bHasTexture = false;
		}
	}

	if (!bHasTexture)
	{
		string filename;

		if (shader->m_env.length())
		{
			filename = ParseTextureExtension(shader->m_env);
		}

		if (!filename.length())
		{
			filename = ParseTextureExtension(shader->m_name);

			if (!filename.length())
			{
				delete shader;
				return;
			}
		}

		// finally found a texture
		shader->m_editorimage = filename;
		shader->m_texture.push_back(filename);
	}
	*/

	ShaderPtr* pExistingShader = m_nametoshader.findKeyValue(shader->m_name);
	if (!pExistingShader)
	{
		// don't remove shaders in other containers
		m_nametoshader.addNewKeyValue(shader->m_name) = shader;
	}
	/*
	if (pExistingShader) {
		pExistingShader->get()->GetShaderContainer()->RemoveShader(*pExistingShader);
	}
	else {
		m_nametoshader.addNewKeyValue(shader->m_name) = shader;
	}
	*/

	/*
	auto it = m_nametoshader.find(shader->m_name);
	if (it != m_nametoshader.end())
	{
		Shader* s = it->second;
		s->GetShaderContainer()->RemoveShader(s);
		delete s;
		it->second = shader;
	}
	else
	{
		m_nametoshader.emplace(shader->m_name, shader);
	}
	*/
}

ShaderPtr ShaderManager::GetShader(const char *name) const
{
	str newname;

	if( !name || !*name )
	{
		return NULL;
	}

	newname = name;
	newname.tolower();
	//std::transform(newname.begin(), newname.end(), newname.begin(), &shader_tolower);

	const ShaderPtr* pShader = m_nametoshader.findKeyValue(newname);
	if (!pShader) {
		return NULL;
	}

	/*
	auto it = m_nametoshader.find(newname);
	if(it == m_nametoshader.end())
	{
		return NULL;
	}
	*/

	return *pShader;
}

Shader* ShaderManager::GetDefaultShader() const
{
	return &m_defaultshader;
}

ImageCache* ShaderManager::FindImage(const char *name)
{
	ImageCachePtr* pCachedImage = m_nametoimage.findKeyValue(name);
	if (pCachedImage) {
		return pCachedImage->get();
	}

	ImageCachePtr cachedImage = makeShared<ImageCache>(this, name);
	//Image* image = GetAssetManager()->LoadAsset<Image>(imagename.c_str());

	m_nametoimage.addNewKeyValue(name) = cachedImage;
	m_images.AddObject(cachedImage);

	return cachedImage.get();
}

size_t ShaderManager::GetNumShaderContainers() const
{
	return m_shaderContainers.NumObjects();
}

const ShaderContainer* ShaderManager::GetShaderContainer(size_t num) const
{
	if (num >= m_shaderContainers.NumObjects())
	{
		return nullptr;
	}

	return m_shaderContainers.ObjectAt(num + 1).get();
}

const ShaderContainer* ShaderManager::GetShaderContainer(const char* Filename) const
{
	const ShaderContainerPtr* pContainer = m_fileShaderMap.findKeyValue(Filename);
	if (pContainer) {
		return pContainer->get();
	}
	else {
		return nullptr;
	}

	/*
	auto it = m_fileShaderMap.find(Filename);
	if (it != m_fileShaderMap.end())
	{
		return it->second;
	}
	else
	{
		return nullptr;
	}
	*/
}
