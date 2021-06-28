#pragma once

#include "../NetGlobal.h"
#include "../../Common/Vector.h"
#include "../../Utility/Misc/MSG/MSG.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	using entityNum_t = uint16_t;

	/**
	 * renderfx flags(entityState_t::renderfx)
	 * render flags are set by "renderEffects" event
	 * and sent to cgame trough entityState_t
	 */
	static constexpr unsigned int RF_THIRD_PERSON		= (1<<0);   	// don't draw through eyes, only mirrors = (player bodies, chat sprites)
	static constexpr unsigned int RF_FIRST_PERSON		= (1<<1);		// only draw through eyes = (view weapon, damage blood blob)
	static constexpr unsigned int RF_DEPTHHACK			= (1<<2);		// hack the z-depth so that view weapons do not clip into walls
	static constexpr unsigned int RF_VIEWLENSFLARE		= (1<<3);		// View dependent lensflare
	static constexpr unsigned int RF_FRAMELERP			= (1<<4);		// interpolate between current and next state
	static constexpr unsigned int RF_BEAM				= (1<<5);		// draw a beam between origin and origin2
	static constexpr unsigned int RF_DONTDRAW			= (1<<7);		// don't draw this entity but send it over
	static constexpr unsigned int RF_LENSFLARE			= (1<<8);		// add a lens flare to this
	static constexpr unsigned int RF_EXTRALIGHT			= (1<<9);		// use good lighting on this entity
	static constexpr unsigned int RF_DETAIL				= (1<<10);		// Culls a model based on the distance away from you
	static constexpr unsigned int RF_SHADOW				= (1<<11);		// whether or not to draw a shadow
	static constexpr unsigned int RF_PORTALSURFACE		= (1<<12);   	// don't draw, but use to set portal views
	static constexpr unsigned int RF_SKYORIGIN   		= (1<<13);   	// don't draw, but use to set sky portal origin and coordinate system
	static constexpr unsigned int RF_SKYENTITY   		= (1<<14);   	// this entity is only visible through a skyportal
	static constexpr unsigned int RF_LIGHTOFFSET   		= (1<<15);   	// this entity has a light offset
	static constexpr unsigned int RF_CUSTOMSHADERPASS	= (1<<16);		// draw the custom shader on top of the base geometry
	static constexpr unsigned int RF_MINLIGHT			= (1<<17);		// allways have some light = (viewmodel, some items)
	static constexpr unsigned int RF_FULLBRIGHT			= (1<<18);		// allways have full lighting
	static constexpr unsigned int RF_LIGHTING_ORIGIN	= (1<<19);		// use refEntity->lightingOrigin instead of refEntity->origin
																		// for lighting.  This allows entities to sink into the floor
																		// with their origin going solid, and allows all parts of a
																		// player to get the same lighting

	static constexpr unsigned int RF_SHADOW_PLANE		= (1<<20);		// use refEntity->shadowPlane
	static constexpr unsigned int RF_WRAP_FRAMES		= (1<<21);		// mod the model frames by the maxframes to allow continuous animation without needing to know the frame count
	static constexpr unsigned int RF_ADDITIVE_DLIGHT	= (1<<22);		// this entity has an additive dynamic light
	static constexpr unsigned int RF_LIGHTSTYLE_DLIGHT	= (1<<23);		// this entity has a dynamic light that uses a light style
	static constexpr unsigned int RF_SHADOW_PRECISE		= (1<<24);		// this entity can have a precise shadow applied to it
	static constexpr unsigned int RF_INVISIBLE			= (1<<25);		// This entity is invisible, and only negative lights will light it up
	static constexpr unsigned int RF_VIEWMODEL			= (1<<26);		// This entity is invisible, and only negative lights will light it up
	static constexpr unsigned int RF_PRECISESHADOW		= (1<<28);		// This entity is invisible, and only negative lights will light it up
	static constexpr unsigned int RF_ALWAYSDRAW			= (1<<30);
	/**
	 * use this mask when propagating renderfx from one entity to another
	 */
	static constexpr unsigned int RF_FLAGS_NOT_INHERITED = ( RF_LENSFLARE | RF_VIEWLENSFLARE | RF_BEAM | RF_EXTRALIGHT | RF_SKYORIGIN | RF_SHADOW | RF_SHADOW_PRECISE | RF_SHADOW_PLANE );

	static constexpr unsigned int RF_FORCENOLOD				= 1024;
	static constexpr unsigned int BEAM_LIGHTNING_EFFECT		= (1<<0);
	static constexpr unsigned int BEAM_USEMODEL				= (1<<1);
	static constexpr unsigned int BEAM_PERSIST_EFFECT		= (1<<2);
	static constexpr unsigned int BEAM_SPHERE_EFFECT		= (1<<3);
	static constexpr unsigned int BEAM_RANDOM_DELAY			= (1<<4);
	static constexpr unsigned int BEAM_TOGGLE				= (1<<5);
	static constexpr unsigned int BEAM_RANDOM_TOGGLEDELAY	= (1<<6);
	static constexpr unsigned int BEAM_WAVE_EFFECT			= (1<<7);
	static constexpr unsigned int BEAM_USE_NOISE			= (1<<8);
	static constexpr unsigned int BEAM_PARENT				= (1<<9);
	static constexpr unsigned int BEAM_TILESHADER			= (1<<10);
	static constexpr unsigned int BEAM_OFFSET_ENDPOINTS		= (1<<11);

	/** don't need to send any more */
	static constexpr size_t GENTITYNUM_BITS = 10;
	static constexpr size_t	MAX_GENTITIES = (1 << GENTITYNUM_BITS);

	static constexpr size_t ENTITYNUM_NONE = (MAX_GENTITIES - 1);
	static constexpr size_t ENTITYNUM_WORLD = (MAX_GENTITIES - 2);
	static constexpr size_t ENTITYNUM_MAX_NORMAL = (MAX_GENTITIES - 2);

	
	struct MOHPC_NET_EXPORTS trajectory_t
	{
	public:
		Vector trDelta;
		uint32_t trTime;

	public:
		trajectory_t();

		const Vector& getDelta() const;
		uint32_t getTime() const;
	};

	struct MOHPC_NET_EXPORTS frameInfo_t
	{
	public:
		uint32_t index;
		float time;
		float weight;

	public:
		frameInfo_t();

		uint32_t getIndex() const;
		float getTime() const;
		float getWeight() const;
	};

	static constexpr unsigned int EF_TELEPORT_BIT	= (1 << 2);
	static constexpr unsigned int EF_CLIMBWALL		= (1 << 4);
	/** This player entity has his weapon holstered. */
	static constexpr unsigned int EF_UNARMED		= (1 << 5);
	static constexpr unsigned int EF_LINKANGLES		= (1 << 6);
	/** This player entity is in allies team. */
	static constexpr unsigned int EF_ALLIES			= (1 << 7);
	/** This player entity is in axis team. */
	static constexpr unsigned int EF_AXIS			= (1 << 8);
	/** This player entity is actually dead. Don't draw a foe marker over players with EF_DEAD. */
	static constexpr unsigned int EF_DEAD			= (1 << 9);

	enum class entityType_e : unsigned char {
		modelanim_skel,
		modelanim,
		vehicle,
		player,
		item,
		general,
		missile,
		mover,
		beam,
		multibeam,
		portal,
		event_only,
		rain,
		leaf,
		speaker,
		push_trigger,
		teleport_trigger,
		decal,
		emitter,
		rope,
		events,
		exec_commands
	};

	/** flip the togglebit every time an animation. */
	/** changes so a restart of the same anim can be detected. */
	static constexpr unsigned int ANIM_TOGGLEBIT	= (1 << 9);
	static constexpr unsigned int ANIM_BLEND		= (1 << 10);
	static constexpr unsigned int ANIM_NUMBITS		= 11;

	/** server side anim bits */
	static constexpr unsigned int ANIM_SERVER_EXITCOMMANDS_PROCESSED = (1 << 12);

	static constexpr unsigned int ANIM_MASK = (~(ANIM_TOGGLEBIT | ANIM_BLEND | ANIM_SERVER_EXITCOMMANDS_PROCESSED));

	/** if FRAME_EXPLICIT is set, don't auto animate */
	static constexpr unsigned int FRAME_EXPLICIT = 512;
	static constexpr unsigned int FRAME_MASK = (~FRAME_EXPLICIT);

	/**
	 * Tag specific flags
	 */
	/** number of bits required to send over network */
	static constexpr unsigned int TAG_NUMBITS	= 10;
	static constexpr unsigned int TAG_MASK		= ((1 << 10) - 1);

	/** bone_tag values for players */
	enum class playerBoneTag_e : unsigned char {
		head,
		torso,
		arms,
		pelvis,
		mouth
	};

	class MOHPC_NET_EXPORTS entityState_t
	{
	public:
		static constexpr unsigned long MAX_FRAMEINFOS = 16;
		static constexpr unsigned long NUM_BONE_CONTROLLERS = 5;

	public:
		entityState_t();

		/** Entity number. */
		entityNum_t getNumber() const;
		/** Type of the entity. */
		entityType_e getType() const;
		/** Flags of the entity (see EF_* values). */
		uint16_t getEntityFlags() const;

		/** Entity's trajectory (delta movement and time). */
		const trajectory_t& getTrajectory() const;

		/** Network origin. */
		const Vector& getNetOrigin() const;
		/** Network secondary origin. */
		const Vector& getAlternateOrigin() const;
		/** Network angles. */
		const Vector& getNetAngles() const;

		/** Entity's RGBA values from constant light. */
		uint32_t getConstantLight() const;
		/** Extract RGBA values from constant light. */
		void getConstantLight(uint8_t& red, uint8_t& green, uint8_t& blue, uint8_t& intensity);

		/** Whether or not entity is looping sound. */
		bool doesLoopsound() const;
		/** Looping sound volume. */
		float getLoopsoundVolume() const;
		/** Looping sound minimum distance. */
		float getLoopsoundMinDist() const;
		/** Looping sound maximum distance. */
		float getLoopsoundMaxDist() const;
		/** Looping sound flags. */
		uint32_t getLoopsoundFlags() const;

		/** Entity's parent if it has one. None = ENTITYNUM_NONE. */
		entityNum_t getParent() const;
		/** Tag num of the entity's parent (usually 0). */
		uint16_t getTagNum() const;

		/** Whether or not to also bind the entity's angles. */
		bool doesAttachUseAngle() const;
		/** Attachment offset (relative location). */
		const Vector& getAttachOffset() const;

		/** Beam number of the entity. */
		uint16_t getBeamEntityNum() const;

		/** Model index (starting from CS_MODELS)*/
		uint16_t getModelIndex() const;
		/** Usage index of the entity. */
		uint16_t getUsageIndex() const;
		/** Skin number. */
		uint16_t getSkinNum() const;
		/** This field doesn't seem to have a reason to exist. */
		uint16_t getWasFrame() const;
		/** Frame info at the specified index. */
		const frameInfo_t& getFrameInfo(uint8_t frameIndex) const;
		/** Get the torso action weight if it's a player. */
		float getActionWeight() const;

		/** Get bone number for index. */
		uint8_t getBoneTag(uint8_t boneIndex) const;
		/** Bone angles at index. */
		const Vector& getBoneAngles(uint8_t boneIndex);
		/** Surface flags index. */
		uint8_t getSurface(uint8_t surfaceIndex) const;

		/** Client number of the entity if it's a player. */
		uint8_t getClientNum() const;
		/** Number of the entity it is landing on. */
		uint16_t getGroundEntityNum() const;

		/** Contains bounding box values. SOLID_BMODEL for brushmodel collision. */
		uint32_t getSolid() const;

		/** Scale multiplier. */
		float getScale() const;
		/** Render alpha. */
		float getAlpha() const;
		/** Render flags of the entity (see RF_* values). */
		uint32_t getRenderFlags() const;
		/** Shader data. */
		float getShaderData(uint8_t shaderIndex) const;
		/** Shader time. */
		float getShaderTime() const;

	public:
		trajectory_t pos;
		Vector netorigin;
		Vector origin2;
		Vector netangles;
		Vector attach_offset;
		Vector eyeVector;
		Vector bone_angles[NUM_BONE_CONTROLLERS];
		quat_t bone_quat[NUM_BONE_CONTROLLERS];

		uint32_t constantLight;	// r + (g<<8) + (b<<16) + (intensity<<24)
		uint32_t loopSound;		// constantly loop this sound

		// for client side prediction, trap_linkentity sets this properly
		uint32_t solid;
		// Render flags (see RF_*)
		uint32_t renderfx;

		float loopSoundVolume;
		float loopSoundMinDist;
		float loopSoundMaxDist;
		float loopSoundPitch;
		float actionWeight;
		float scale;
		float alpha;
		float shader_data[2];
		float shader_time;
		frameInfo_t frameInfo[MAX_FRAMEINFOS];

		// entity index
		entityNum_t number;
		uint16_t eFlags;

		uint16_t parent;
		uint16_t tag_num;

		uint16_t beam_entnum;

		uint16_t modelindex;
		uint16_t usageIndex;
		uint16_t skinNum;
		uint16_t wasframe;
		// -1 = in air
		uint16_t groundEntityNum;

		// entityType_t
		entityType_e eType;
		uint8_t loopSoundFlags;

		// 0 to (MAX_CLIENTS - 1), for players and corpses
		uint8_t clientNum;
		uint8_t bone_tag[NUM_BONE_CONTROLLERS];
		uint8_t	surfaces[32];
		bool attach_use_angles;
	};

	entityState_t getNullEntityState();

	class MOHPC_NET_EXPORTS MsgTypesEntityHelper : public MsgBaseHelper
	{
	public:
		MsgTypesEntityHelper(MSG& inMsg);

		/** Read an entity number. */
		uint32_t ReadEntityNum();

		/** Read an entity number (protocol >= 15). */
		uint32_t ReadEntityNum2();

		/** Read an entity number. */
		void WriteEntityNum(uint32_t num);

		/** Read an entity number (protocol >= 15). */
		void WriteEntityNum2(uint32_t num);
	};
}
}