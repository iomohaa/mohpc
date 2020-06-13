#include <Shared.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/ShaderManager.h>
#include <MOHPC/Script.h>
#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Formats/Image.h>
#include <MOHPC/Utilities/SharedPtr.h>

using namespace MOHPC;

static unsigned NameToAFunc(const char *funcname)
{
	if (!stricmp(funcname, "GT0"))
	{
		return GLS_ATEST_GT_0;
	}
	else if (!stricmp(funcname, "LT128"))
	{
		return GLS_ATEST_LT_80;
	}
	else if (!stricmp(funcname, "GE128"))
	{
		return GLS_ATEST_GE_80;
	}

	return 0;
}

static GenFunc NameToGenFunc(const char *funcname)
{
	if (!stricmp(funcname, "sin"))
	{
		return GF_SIN;
	}
	else if (!stricmp(funcname, "square"))
	{
		return GF_SQUARE;
	}
	else if (!stricmp(funcname, "triangle"))
	{
		return GF_TRIANGLE;
	}
	else if (!stricmp(funcname, "sawtooth"))
	{
		return GF_SAWTOOTH;
	}
	else if (!stricmp(funcname, "inversesawtooth"))
	{
		return GF_INVERSE_SAWTOOTH;
	}
	else if (!stricmp(funcname, "noise"))
	{
		return GF_NOISE;
	}

	return GF_SIN;
}


static int NameToSrcBlendMode(const char *name)
{
	if (!stricmp(name, "GL_ONE"))
	{
		return GLS_SRCBLEND_ONE;
	}
	else if (!stricmp(name, "GL_ZERO"))
	{
		return GLS_SRCBLEND_ZERO;
	}
	else if (!stricmp(name, "GL_DST_COLOR"))
	{
		return GLS_SRCBLEND_DST_COLOR;
	}
	else if (!stricmp(name, "GL_ONE_MINUS_DST_COLOR"))
	{
		return GLS_SRCBLEND_ONE_MINUS_DST_COLOR;
	}
	else if (!stricmp(name, "GL_SRC_ALPHA"))
	{
		return GLS_SRCBLEND_SRC_ALPHA;
	}
	else if (!stricmp(name, "GL_ONE_MINUS_SRC_ALPHA"))
	{
		return GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA;
	}
	else if (!stricmp(name, "GL_DST_ALPHA"))
	{
		return GLS_SRCBLEND_DST_ALPHA;
	}
	else if (!stricmp(name, "GL_ONE_MINUS_DST_ALPHA"))
	{
		return GLS_SRCBLEND_ONE_MINUS_DST_ALPHA;
	}
	else if (!stricmp(name, "GL_SRC_ALPHA_SATURATE"))
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
	if (!stricmp(name, "GL_ONE"))
	{
		return GLS_DSTBLEND_ONE;
	}
	else if (!stricmp(name, "GL_ZERO"))
	{
		return GLS_DSTBLEND_ZERO;
	}
	else if (!stricmp(name, "GL_SRC_ALPHA"))
	{
		return GLS_DSTBLEND_SRC_ALPHA;
	}
	else if (!stricmp(name, "GL_ONE_MINUS_SRC_ALPHA"))
	{
		return GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
	}
	else if (!stricmp(name, "GL_DST_ALPHA"))
	{
		return GLS_DSTBLEND_DST_ALPHA;
	}
	else if (!stricmp(name, "GL_ONE_MINUS_DST_ALPHA"))
	{
		return GLS_DSTBLEND_ONE_MINUS_DST_ALPHA;
	}
	else if (!stricmp(name, "GL_SRC_COLOR"))
	{
		return GLS_DSTBLEND_SRC_COLOR;
	}
	else if (!stricmp(name, "GL_ONE_MINUS_SRC_COLOR"))
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
	type = TextureMod::TMOD_NONE;
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

const str& Shader::GetFilename() const
{
	return shaderContainer->GetFilename();
}

const str& Shader::GetName() const
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

		if (!stricmp(token, "qer_editorimage"))
		{
			//m_editorimage = GetShaderManager()->GetFileManager()->GetFixedPath(script.GetToken(false));
			token = script.GetToken(false);
			m_editorimage = GetShaderManager()->FindImage(token);
		}
		else if (!stricmp(token, "deformVertexes"))
		{
			ParseDeform(script);
		}
		else if (!stricmp(token, "spriteGen"))
		{
			token = script.GetToken(false);

			if (!stricmp(token, "parallel"))
			{
				this->sprite.type = SPRITE_PARALLEL;
			}
			else if (!stricmp(token, "parallel_oriented"))
			{
				sprite.type = SPRITE_PARALLEL_ORIENTED;
			}
			else if (!stricmp(token, "parallel_upright"))
			{
				sprite.type = SPRITE_PARALLEL_UPRIGHT;
			}
			else if (!stricmp(token, "oriented"))
			{
				sprite.type = SPRITE_ORIENTED;
			}
		}
		else if (!stricmp(token, "spriteScale"))
		{
			token = script.GetToken(false);

			if (!token[0])
			{
				continue;
			}

			sprite.scale = (float)atof(token);
		}
		else if (!stricmp(token, "surfaceparm"))
		{
			ParseSurfaceParm(script);
		}
		// no mip maps
		else if (!stricmp(token, "nomipmaps"))
		{
			bNoMipMaps = true;
			bNoPicMip = true;
		}
		// no picmip adjustment
		else if (!stricmp(token, "nopicmip"))
		{
			bNoPicMip = true;
		}
		// portal
		else if (!stricmp(token, "portal"))
		{
			bIsPortal = true;
		}
		else if (!stricmp(token, "skyparms"))
		{
			//token = script.GetToken(false);
			//m_env = GetShaderManager()->GetFileManager()->GetFixedPath(string(token) + "_ft");
			//script.SkipToEOL();
			ParseSkyParms(script);
		}
		// cull <face>
		else if (!stricmp(token, "cull"))
		{
			token = script.GetToken(false);
			if (token[0] == 0)
			{
				//ri.Printf(PRINT_WARNING, "WARNING: missing cull parms in shader '%s'\n", shader.name);
				continue;
			}

			if (!stricmp(token, "none") || !stricmp(token, "twosided") || !stricmp(token, "disable"))
			{
				cullType = CT_TWO_SIDED;
			}
			else if (!stricmp(token, "back") || !stricmp(token, "backside") || !stricmp(token, "backsided"))
			{
				cullType = CT_BACK_SIDED;
			}
			else
			{
				//ri.Printf(PRINT_WARNING, "WARNING: invalid cull parm '%s' in shader '%s'\n", token, shader.name);
			}
		}
		else if (!stricmp(token, "portalsky"))
		{
			bIsPortalSky = true;
		}
		// stage definition
		else if (token[0] == '{')
		{
			//ShaderStage& shaderStage = *m_stages.insert(m_stages.end(), ShaderStage());
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
		//ri.Printf(PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name);
		return;
	}
	wave->func = NameToGenFunc(token);

	// BASE, AMP, PHASE, FREQ
	token = script.GetToken(false);
	if (token[0] == 0)
	{
		//ri.Printf(PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name);
		return;
	}
	wave->base = (float)atof(token);

	token = script.GetToken(false);
	if (token[0] == 0)
	{
		//ri.Printf(PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name);
		return;
	}
	wave->amplitude = (float)atof(token);

	token = script.GetToken(false);
	if (token[0] == 0)
	{
		//ri.Printf(PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name);
		return;
	}
	wave->phase = (float)atof(token);

	token = script.GetToken(false);
	if (token[0] == 0)
	{
		//ri.Printf(PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name);
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
	if (!stricmp(token, "turb"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing tcMod turb parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->wave.base = (float)atof(token);
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing tcMod turb in shader '%s'\n", shader.name);
			return;
		}
		tmi->wave.amplitude = (float)atof(token);
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing tcMod turb in shader '%s'\n", shader.name);
			return;
		}
		tmi->wave.phase = (float)atof(token);
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing tcMod turb in shader '%s'\n", shader.name);
			return;
		}
		tmi->wave.frequency = (float)atof(token);

		tmi->type = TMOD_TURBULENT;
	}
	//
	// scale
	//
	else if (!stricmp(token, "scale"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing scale parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->scale[0] = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing scale parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->scale[1] = (float)atof(token);
		tmi->type = TMOD_SCALE;
	}
	//
	// scroll
	//
	else if (!stricmp(token, "scroll"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing scale scroll parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->scroll[0] = (float)atof(token);
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing scale scroll parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->scroll[1] = (float)atof(token);
		tmi->type = TMOD_SCROLL;
	}
	//
	// stretch
	//
	else if (!stricmp(token, "stretch"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->wave.func = NameToGenFunc(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->wave.base = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->wave.amplitude = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->wave.phase = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->wave.frequency = (float)atof(token);

		tmi->type = TMOD_STRETCH;
	}
	//
	// transform
	//
	else if (!stricmp(token, "transform"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->matrix[0][0] = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->matrix[0][1] = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->matrix[1][0] = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->matrix[1][1] = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->translate[0] = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->translate[1] = (float)atof(token);

		tmi->type = TMOD_TRANSFORM;
	}
	//
	// rotate
	//
	else if (!stricmp(token, "rotate"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing tcMod rotate parms in shader '%s'\n", shader.name);
			return;
		}
		tmi->rotateSpeed = (float)atof(token);
		tmi->type = TMOD_ROTATE;
	}
	//
	// entityTranslate
	//
	else if (!stricmp(token, "entityTranslate"))
	{
		tmi->type = TMOD_ENTITY_TRANSLATE;
	}
	else
	{
		//ri.Printf(PRINT_WARNING, "WARNING: unknown tcMod '%s' in shader '%s'\n", token, shader.name);
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

		if (!stricmp(token, "map") || !stricmp(token, "clampmap"))
		{
			token = script.GetToken(false);
			stage->bundle[bundleNum].image.push_back(GetShaderManager()->FindImage(token)); // GetShaderManager()->GetFileManager()->GetFixedPath(token);
		}
		else if (!stricmp(token, "alphafunc"))
		{
			token = script.GetToken(false);
			atestBits = NameToAFunc(token);
		}
		else if (!stricmp(token, "blendfunc"))
		{
			token = script.GetToken(false);

			if (!stricmp(token, "add"))
			{
				blendSrcBits = GLS_SRCBLEND_ONE;
				blendDstBits = GLS_DSTBLEND_ONE;
			}
			else if (!stricmp(token, "filter"))
			{
				blendSrcBits = GLS_SRCBLEND_DST_COLOR;
				blendDstBits = GLS_DSTBLEND_ZERO;
			}
			else if (!stricmp(token, "blend"))
			{
				blendSrcBits = GLS_SRCBLEND_SRC_ALPHA;
				blendDstBits = GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
			}
			else if (!stricmp(token, "alphaadd"))
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
		else if (!stricmp(token, "rgbGen"))
		{
			token = script.GetToken(false);
			if (token[0] == 0)
			{
				//ri.Printf(PRINT_WARNING, "WARNING: missing parameters for rgbGen in shader '%s'\n", shader.name);
				continue;
			}

			if (!stricmp(token, "wave"))
			{
				ParseWaveForm(script, &stage->rgbWave);
				stage->rgbGen = CGEN_WAVEFORM;
			}
			else if (!stricmp(token, "colorwave"))
			{
				Vector color = script.GetVector(false);
				stage->constantColor[0] = color[0];
				stage->constantColor[1] = color[1];
				stage->constantColor[2] = color[2];
				ParseWaveForm(script, &stage->rgbWave);
				stage->rgbGen = CGEN_MULTIPLY_BY_WAVEFORM;
			}
			else if (!stricmp(token, "const") || !stricmp(token, "constant"))
			{
				Vector color = script.GetVector(false);
				stage->constantColor[0] = color[0];
				stage->constantColor[1] = color[1];
				stage->constantColor[2] = color[2];

				stage->rgbGen = CGEN_CONST;
			}
			else if (!stricmp(token, "identity"))
			{
				stage->rgbGen = CGEN_IDENTITY;
			}
			else if (!stricmp(token, "identityLighting"))
			{
				stage->rgbGen = CGEN_IDENTITY_LIGHTING;
			}
			else if (!stricmp(token, "entity") || !stricmp(token, "fromentity"))
			{
				stage->rgbGen = CGEN_ENTITY;
			}
			else if (!stricmp(token, "oneMinusEntity"))
			{
				stage->rgbGen = CGEN_ONE_MINUS_ENTITY;
			}
			else if (!stricmp(token, "vertex") || !stricmp(token, "fromclient"))
			{
				stage->rgbGen = CGEN_VERTEX;
				if (stage->alphaGen == 0) {
					stage->alphaGen = AGEN_VERTEX;
				}
			}
			else if (!stricmp(token, "exactVertex"))
			{
				stage->rgbGen = CGEN_EXACT_VERTEX;
			}
			else if (!stricmp(token, "vertexLit"))
			{
				stage->rgbGen = CGEN_VERTEX_LIT;
				if (stage->alphaGen == 0) {
					stage->alphaGen = AGEN_VERTEX;
				}
			}
			else if (!stricmp(token, "exactVertexLit"))
			{
				stage->rgbGen = CGEN_EXACT_VERTEX_LIT;
			}
			else if (!stricmp(token, "lightingGrid"))
			{
				stage->rgbGen = CGEN_LIGHTING_GRID;
			}
			else if (!stricmp(token, "lightingSpherical"))
			{
				//stage->rgbGen = CGEN_LIGHTING_SPHERICAL;
				stage->rgbGen = CGEN_LIGHTING_DIFFUSE;
			}
			else if (!stricmp(token, "lightingDiffuse"))
			{
				stage->rgbGen = CGEN_LIGHTING_DIFFUSE;
			}
			else if (!stricmp(token, "oneMinusVertex"))
			{
				stage->rgbGen = CGEN_ONE_MINUS_VERTEX;
			}
			else if (!stricmp(token, "global"))
			{
				stage->rgbGen = CGEN_GLOBAL_COLOR;
			}
			else if (!stricmp(token, "static"))
			{
				stage->rgbGen = CGEN_STATIC;

				if (stage->alphaGen = AGEN_IDENTITY) {
					stage->alphaGen = AGEN_VERTEX;
				}
			}
			else if (!stricmp(token, "sCoord") || !stricmp(token, "tCoord"))
			{
				if (!stricmp(token, "sCoord")) {
					stage->rgbGen = CGEN_SCOORD;
				}
				else if (!stricmp(token, "tCoord")) {
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
								//ri.Printf(PRINT_WARNING, "WARNING: missing rgbGen sCoord or tCoord parm 'max' in shader '%s'\n", shader.name);
							}
						}
					}
				}
			}
			else if (!stricmp(token, "dot"))
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
			else if (!stricmp(token, "oneminusdot"))
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
				//ri.Printf(PRINT_WARNING, "WARNING: unknown rgbGen parameter '%s' in shader '%s'\n", token, shader.name);
				continue;
			}
		}
		//
		// alphaGen 
		//
		else if (!stricmp(token, "alphaGen"))
		{
			token = script.GetToken(false);
			if (token[0] == 0)
			{
				//ri.Printf(PRINT_WARNING, "WARNING: missing parameters for alphaGen in shader '%s'\n", shader.name);
				continue;
			}

			if (!stricmp(token, "wave"))
			{
				ParseWaveForm(script, &stage->alphaWave);
				stage->alphaGen = AGEN_WAVEFORM;
			}
			else if (!stricmp(token, "const") || !stricmp(token, "constant"))
			{
				token = script.GetToken(false);
				stage->constantColor[3] = 255 * (float)atof(token);
				stage->alphaGen = AGEN_CONST;
			}
			else if (!stricmp(token, "identity"))
			{
				stage->alphaGen = AGEN_IDENTITY;
			}
			else if (!stricmp(token, "entity"))
			{
				stage->alphaGen = AGEN_ENTITY;
			}
			else if (!stricmp(token, "oneMinusEntity"))
			{
				stage->alphaGen = AGEN_ONE_MINUS_ENTITY;
			}
			else if (!stricmp(token, "vertex"))
			{
				stage->alphaGen = AGEN_VERTEX;
			}
			else if (!stricmp(token, "lightingSpecular"))
			{
				stage->alphaGen = AGEN_LIGHTING_SPECULAR;
			}
			else if (!stricmp(token, "oneMinusVertex"))
			{
				stage->alphaGen = AGEN_ONE_MINUS_VERTEX;
			}
			else if (!stricmp(token, "portal"))
			{
				stage->alphaGen = AGEN_PORTAL;
				token = script.GetToken(false);
				if (token[0] == 0)
				{
					portalRange = 256;
					//ri.Printf(PRINT_WARNING, "WARNING: missing range parameter for alphaGen portal in shader '%s', defaulting to 256\n", shader.name);
				}
				else
				{
					portalRange = (float)atof(token);
				}
			}
			else if (!stricmp(token, "distFade"))
			{
				stage->alphaGen = AGEN_DIST_FADE;
				fDistRange = 256;
				fDistNear = 256;

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
			else if (!stricmp(token, "oneMinusDistFade"))
			{
				stage->alphaGen = AGEN_ONE_MINUS_DIST_FADE;
				fDistRange = 256;
				fDistNear = 256;

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
			else if (!stricmp(token, "dot"))
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
			else if (!stricmp(token, "dotView"))
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
			else if (!stricmp(token, "oneMinusDotView"))
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
			else if (!stricmp(token, "oneMinusDot"))
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
			else if (!stricmp(token, "skyAlpha"))
			{
				stage->alphaGen = AGEN_SKYALPHA;
			}
			else if (!stricmp(token, "oneMinusSkyAlpha"))
			{
				stage->alphaGen = AGEN_ONE_MINUS_SKYALPHA;
			}
			else if (!stricmp(token, "sCoord") || !stricmp(token, "tCoord"))
			{
				if (!stricmp(token, "sCoord")) {
					stage->alphaGen = AGEN_SCOORD;
				}
				else if (!stricmp(token, "tCoord")) {
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
								//ri.Printf(PRINT_WARNING, "WARNING: missing rgbGen sCoord or tCoord parm 'max' in shader '%s'\n", shader.name);
							}
						}
					}
				}
			}
			else
			{
				//ri.Printf(PRINT_WARNING, "WARNING: unknown alphaGen parameter '%s' in shader '%s'\n", token, shader.name);
				continue;
			}
		}
		//
		// tcGen <function>
		//
		else if (!stricmp(token, "texgen") || !stricmp(token, "tcGen"))
		{
			token = script.GetToken(false);
			if (token[0] == 0)
			{
				//ri.Printf(PRINT_WARNING, "WARNING: missing texgen parm in shader '%s'\n", shader.name);
				continue;
			}

			if (!stricmp(token, "environment"))
			{
				//shader.needsNormal = qtrue;
				stage->bundle[bundleNum].tcGen = TCGEN_ENVIRONMENT_MAPPED;
			}
			else if (!stricmp(token, "environmentmodel"))
			{
				//shader.needsNormal = qtrue;
				stage->bundle[bundleNum].tcGen = TCGEN_ENVIRONMENT_MAPPED2;
			}
			else if (!stricmp(token, "sunreflection"))
			{
				//shader.needsNormal = qtrue;
				stage->bundle[bundleNum].tcGen = TCGEN_SUN_REFLECTION;
			}
			else if (!stricmp(token, "lightmap"))
			{
				stage->bundle[bundleNum].tcGen = TCGEN_LIGHTMAP;
			}
			else if (!stricmp(token, "texture") || !stricmp(token, "base"))
			{
				stage->bundle[bundleNum].tcGen = TCGEN_TEXTURE;
			}
			else if (!stricmp(token, "vector"))
			{
				stage->bundle[bundleNum].tcGenVectors[0] = script.GetVector(false);
				stage->bundle[bundleNum].tcGenVectors[1] = script.GetVector(false);

				stage->bundle[bundleNum].tcGen = TCGEN_VECTOR;
			}
			else
			{
				//ri.Printf(PRINT_WARNING, "WARNING: unknown texgen parm in shader '%s'\n", shader.name);
			}
		}
		//
		// tcMod <type> <...>
		//
		else if (!stricmp(token, "tcMod"))
		{
			ParseTexMod(script, stage);
		}
		else if (!stricmp(token, "nextbundle"))
		{
			bundleNum++;
			stage->bundle.push_back(TextureBundle());
		}
		else if (!stricmp(token, "animmap") || !stricmp(token, "animmaponce") || !stricmp(token, "animmapphase"))
		{
			if (!stricmp(token, "animmap") || !stricmp(token, "animmaponce"))
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
			else if (!stricmp(token, "animmapphase"))
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
		//ri.Printf(PRINT_WARNING, "WARNING: missing deform parm in shader '%s'\n", shader.name);
		return;
	}

	m_deforms.push_back(DeformStage());
	DeformStage* ds = m_deforms.Data();

	if (!stricmp(token, "projectionShadow"))
	{
		ds->deformation = DEFORM_PROJECTION_SHADOW;
	}
	else if (!stricmp(token, "autosprite"))
	{
		ds->deformation = DEFORM_AUTOSPRITE;
		sprite.type = SPRITE_ORIENTED;
	}
	else if (!stricmp(token, "autosprite2"))
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
	else if (!stricmp(token, "bulge"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing deformVertexes bulge parm in shader '%s'\n", shader.name);
			return;
		}
		ds->bulgeWidth = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing deformVertexes bulge parm in shader '%s'\n", shader.name);
			return;
		}
		ds->bulgeHeight = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing deformVertexes bulge parm in shader '%s'\n", shader.name);
			return;
		}
		ds->bulgeSpeed = (float)atof(token);

		ds->deformation = DEFORM_BULGE;
	}
	else if (!stricmp(token, "wave"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing deformVertexes parm in shader '%s'\n", shader.name);
			return;
		}

		if (atof(token) != 0)
		{
			ds->deformationSpread = (float)(1.0 / atof(token));
		}
		else
		{
			ds->deformationSpread = 100.0f;
			//ri.Printf(PRINT_WARNING, "WARNING: illegal div value of 0 in deformVertexes command for shader '%s'\n", shader.name);
		}

		ParseWaveForm(script, &ds->deformationWave);
		ds->deformation = DEFORM_WAVE;
	}
	else if (!stricmp(token, "normal"))
	{
		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing deformVertexes parm in shader '%s'\n", shader.name);
			return;
		}
		ds->deformationWave.amplitude = (float)atof(token);

		token = script.GetToken(false);
		if (token[0] == 0)
		{
			//ri.Printf(PRINT_WARNING, "WARNING: missing deformVertexes parm in shader '%s'\n", shader.name);
			return;
		}
		ds->deformationWave.frequency = (float)atof(token);

		ds->deformation = DEFORM_NORMALS;
	}
	else if (!stricmp(token, "move"))
	{
		int		i;

		for (i = 0; i < 3; i++)
		{
			token = script.GetToken(false);
			if (token[0] == 0)
			{
				//ri.Printf(PRINT_WARNING, "WARNING: missing deformVertexes parm in shader '%s'\n", shader.name);
				return;
			}
			ds->moveVector[i] = (float)atof(token);
		}

		ParseWaveForm(script, &ds->deformationWave);
		ds->deformation = DEFORM_MOVE;
	}
	else if (!stricmp(token, "flap"))
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
		//ri.Printf(PRINT_WARNING, "WARNING: 'skyParms' missing parameter in shader '%s'\n", shader.name);
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
		//ri.Printf(PRINT_WARNING, "WARNING: 'skyParms' missing parameter in shader '%s'\n", shader.name);
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
		//ri.Printf(PRINT_WARNING, "WARNING: 'skyParms' missing parameter in shader '%s'\n", shader.name);
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
		if (!stricmp(paramsToFlags[i].param, token))
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

	shader = shader;
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

	// Parse all shaders
	ParseShaders(entryList);

	//FS_FreeFileList( pszFiles );

	end = clock();

	seconds = ( float )( end - start ) / CLOCKS_PER_SEC;

	//bi.Printf( "%d shaders loaded in %.2f seconds\n", m_shaderlist.size(), seconds );
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

		std::streamsize length = file->ReadBuffer((void**)&buffer);

		ShaderContainerPtr shaderContainer = makeShared<ShaderContainer>(this, filename);

		// Parse the shader
		ParseShaderContainer(shaderContainer.get(), filename.c_str(), buffer, length);

		m_shaderContainers.AddObject(shaderContainer);
		m_fileShaderMap.addKeyValue(filename) = shaderContainer;
	}
}

void ShaderManager::ParseShaderContainer(ShaderContainer *shaderContainer, const char *name, const char *buffer, std::streamsize length)
{
	if (!length) length = strlen(buffer) + 1;

	Script script;
	script.Parse(buffer, length, "");

	str filename = name;

	while (script.TokenAvailable(true))
	{
		const char *token = script.GetToken(false);
		if (!token[0])
		{
			return;
		}

		if (token[0] == '/' && token[1] == '*')
		{
			bool bTokenAvailable;
			while (bTokenAvailable = script.TokenAvailable(true))
			{
				if (token[0] == '*' && token[1] == '/')
				{
					break;
				}
				script.GetToken(false);
			}
			if (!bTokenAvailable)
			{
				return;
			}
			continue;
		}

		const str shadername = token;

		token = script.GetToken(false);

		if (token[0] != '{')
		{
			return;
		}

		ShaderPtr shader = AllocShader(shaderContainer);
		shader->m_name = shadername;
		shader->m_name.tolower();
		//std::transform(shader->m_name.begin(), shader->m_name.end(), shader->m_name.begin(), &shader_tolower);

		shaderContainer->AddShader(shader);

		shader->ParseShader(script);
		//shader->Precache();

		AddShader(shader);
	}
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

ShaderRef ShaderManager::GetShader(const char *name) const
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

	return pShader->get();
}

ShaderRef ShaderManager::GetDefaultShader() const
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
