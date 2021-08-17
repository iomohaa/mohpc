#include <Shared.h>
#include "EmitterListener.h"
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Utility/Managers/EmitterManager.h>
#include <MOHPC/Files/Managers/FileManager.h>
#include <MOHPC/Files/FileHelpers.h>
#include <morfuse/Script/EventSystem.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>
#include <MOHPC/Assets/Formats/TIKI.h>

using namespace MOHPC;

mfuse::EventDef EV_Client_StartBlock
(
	"(",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Signals the beginning of a block of commands"
);
mfuse::EventDef EV_Client_EndBlock
(
	")",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Signals the end of a block of commands"
);
mfuse::EventDef EV_Client_EmitterStartOff
(
	"startoff",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Signals an emitter to start in the off state (no tempmodels are emitted)"
);
mfuse::EventDef EV_Client_SetAlpha
(
	"alpha",
	mfuse::EV_DEFAULT,
	"f",
	"alpha",
	"Set the alpha of the spawned tempmodel"
);
mfuse::EventDef EV_Client_SetDieTouch
(
	"dietouch",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the spawned tempmodels to die when they touch a solid"
);
mfuse::EventDef EV_Client_SetBounceFactor
(
	"bouncefactor",
	mfuse::EV_DEFAULT,
	"f",
	"factor",
	"Set the bounciness of a tempmodel when it hits a solid.\n"
	"A factor > 1 will make the model bounce higher and higher on each hit"
);
mfuse::EventDef EV_Client_SetBounceSound
(
	"bouncesound",
	mfuse::EV_DEFAULT,
	"sF",
	"sound [delay]",
	"When bouncing, what sound to play on impact and an option delay (default is 1 second) between playing this sound"
);
mfuse::EventDef EV_Client_SetBounceSoundOnce
(
	"bouncesoundonce",
	mfuse::EV_DEFAULT,
	"s",
	"sound",
	"When bouncing, what sound to play on impact one time"
);
mfuse::EventDef EV_Client_SetScale
(
	"scale",
	mfuse::EV_DEFAULT,
	"f",
	"scale",
	"Set the scale of a spawned tempmodel"
);
mfuse::EventDef EV_Client_SetScaleUpDown
(
	"scaleupdown",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodel to scale up to scale value and then down."
);
mfuse::EventDef EV_Client_SetScaleMin
(
	"scalemin",
	mfuse::EV_DEFAULT,
	"f",
	"scalemin",
	"Set the minimum scale of a spawned tempmodel"
);
mfuse::EventDef EV_Client_SetScaleMax
(
	"scalemax",
	mfuse::EV_DEFAULT,
	"f",
	"scalemax",
	"Set the maximum scale of a spawned tempmodel"
);
mfuse::EventDef EV_Client_SetModel
(
	"model",
	mfuse::EV_DEFAULT,
	"sSSSSS",
	"modelname1 modelname2 modelname3 modelname4 modelname5 modelname6",
	"Set the modelname of the tempmodel.  If more than 1 model is specified, it will\n"
	"be randomly chosen when spawned"
);
mfuse::EventDef EV_Client_SetLife
(
	"life",
	mfuse::EV_DEFAULT,
	"fF",
	"life randomlife",
	"Set the life (in seconds) of the spawned tempmodel"
);
mfuse::EventDef EV_Client_SetColor
(
	"color",
	mfuse::EV_DEFAULT,
	"fffF",
	"red green blue alpha",
	"Set the color (modulate) of the spawned tempmodel."
);
mfuse::EventDef EV_Client_RadialVelocity
(
	"radialvelocity",
	mfuse::EV_DEFAULT,
	"fff",
	"scale min_additional max_additional",
	"Subtracts the particle origin from origin and multiplies by scale, then adds additional velocity\n"
	"between min and max... negative values bring toward origin\n"
);
mfuse::EventDef EV_Client_SetVelocity
(
	"velocity",
	mfuse::EV_DEFAULT,
	"f",
	"forwardVelocity",
	"Set the forward velocity of the spawned tempmodel"
);
mfuse::EventDef EV_Client_SetAngularVelocity
(
	"avelocity",
	mfuse::EV_DEFAULT,
	"fff",
	"yawVel pitchVel rollVel",
	"Set the angular velocity of the spawned tempmodel"
);
mfuse::EventDef EV_Client_SetColorVelocity
(
	"colorvelocity",
	mfuse::EV_DEFAULT,
	"fff",
	"rVel gVel bVel",
	"Set the color velocity of the spawned dlight tempmodel"
);
mfuse::EventDef EV_Client_SetRandomVelocity
(
	"randvel",
	mfuse::EV_DEFAULT,
	"SfFSfFSfF",
	"[random|crandom|range] xVel [xVel2] [random|crandom|range] yVel [yVel2] [random|crandom|range] zVel [zVel2]",
	"Add a random component to the regular velocity.\n"
	"If random is specified, the component will range from 0 to specified velocity.\n"
	"If crandom is specified, the component will range from -specified to +specified velocity.\n"
	"If range is specified, the component needs two values; it will randomly pick a number in the range\n"
	"from the first number to the first number plus the second number.\n"
	"If no keyword is explicitly specified, then the component will just be added on\n"
	"without randomness.\n"
	"This velocity is applied using the world axis"
);
mfuse::EventDef EV_Client_SetRandomVelocityAlongAxis
(
	"randvelaxis",
	mfuse::EV_DEFAULT,
	"SfFSfFSfF",
	"[random|crandom|range] forwardVel [forwardVel2] [random|crandom|range] rightVel [rightVel2] [random|crandom|rang"
	"e] upVel [upVel2]",
	"Add a random component to the regular velocity.\n"
	"If random is specified, the component will range from 0 to specified velocity.\n"
	"If crandom is specified, the component will range from -specified to +specified velocity.\n"
	"If range is specified, the component needs two values; it will randomly pick a number in the range\n"
	"from the first number to the first number plus the second number.\n"
	"If no keyword is explicitly specified, then the component will just be added on\n"
	"without randomness.\n"
	"This velocity is applied using the parent axis"
);
mfuse::EventDef EV_Client_SetAccel
(
	"accel",
	mfuse::EV_DEFAULT,
	"fff",
	"xAcc yAcc zAcc",
	"Set the acceleration of the spawned tempmodel.\nThis acceleration is applied using the world axis"
);
mfuse::EventDef EV_Client_SetFriction
(
	"friction",
	mfuse::EV_DEFAULT,
	"f",
	"friction",
	"Set the friction as a fraction of velocity per second... exact effect depends on physics rate:\n"
	"slowdown per second = [1 - (friction / physicsrate)] ^ physicsrate; physicsrate defaults to 10"
);
mfuse::EventDef EV_Client_SetVaryColor
(
	"varycolor",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Sets the color to vary by 0 to -20% of specified color"
);
mfuse::EventDef EV_Client_SetSmokeParms
(
	"smokeparms",
	mfuse::EV_DEFAULT,
	"fff",
	"typeinfo fademult scalemult",
	"Sets some misc parms for smoke"
);
mfuse::EventDef EV_Client_SetCount
(
	"count",
	mfuse::EV_DEFAULT,
	"i",
	"count",
	"Set the number of tempmodels that are spawned\n"
	"This is only used for the originspawn and tagspawn commands,\n"
	"and not for emitters, use spawnrate instead"
);
mfuse::EventDef EV_Client_SetFade
(
	"fade",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodel to fade out over it's life"
);
mfuse::EventDef EV_Client_SetFadeDelay
(
	"fadedelay",
	mfuse::EV_DEFAULT,
	"f",
	"time",
	"Set the amount of time to delay a fade"
);
mfuse::EventDef EV_Client_SetFadeIn
(
	"fadein",
	mfuse::EV_DEFAULT,
	"f",
	"time",
	"Set the tempmodel to fade in over the specified time"
);
mfuse::EventDef EV_Client_SetTwinkle
(
	"twinkle",
	mfuse::EV_DEFAULT,
	"ffff",
	"mintimeoff maxtimeoff mintimeon maxtimeon",
	"Set the tempmodel to twinkle with the specified settings"
);
mfuse::EventDef EV_Client_SetTrail
(
	"trail",
	mfuse::EV_DEFAULT,
	"sssf",
	"shader startTag endTag life",
	"Set the tempmodel to have a swipe that follows it"
);
mfuse::EventDef EV_Client_SetSpawnRange
(
	"spawnrange",
	mfuse::EV_DEFAULT,
	"iI",
	"range1 range2",
	"Sets the range in which this effect will spawn tempmodels.  If one number is specified, it is the max range\n"
	"and 0 is the min range; if two numbers are specified, the larger is the max range.\n"
);
mfuse::EventDef EV_Client_SetSpawnRate
(
	"spawnrate",
	mfuse::EV_DEFAULT,
	"f",
	"rate",
	"Set the spawnrate of the emitter (models per second).\n"
	"This is only used for emitters and not for the originspawn and tagspawn commands"
);
mfuse::EventDef EV_Client_SetOriginOffset
(
	"offset",
	mfuse::EV_DEFAULT,
	"SfFSfFSfF",
	"[random|crandom|range] offsetX [offsetX2] [random|crandom|range] offsetY [offsetY2] [random|crandom|range] offsetZ [offsetZ2]",
	"If random is specified, the component will range from 0 to +specified offset.\n"
	"If crandom is specified, the component will range from -specified to +specified offset.\n"
	"If range is specified, the component needs two values; it will randomly pick a number in the range\n"
	"from the first number to the first number plus the second number.\n"
	"If no keyword is explicitly specified, then the component will just be added on\n"
	"without randomness.\n"
	"This offset is applied using the world axis."
);
mfuse::EventDef EV_Client_SetScaleRate
(
	"scalerate",
	mfuse::EV_DEFAULT,
	"f",
	"rate",
	"Set the scaling rate of the spawned tempmodel\nIf a negative rate is used, the model will shrink"
);
mfuse::EventDef EV_Client_SetCircle
(
	"circle",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodels to be spawned in a circle around the origin\n"
	"This circle will be generated in the X/Y axis of the model"
);
mfuse::EventDef EV_Client_SetSphere
(
	"sphere",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodels to spawn in a sphere around the origin.\n"
	"If sphereradius is set, the tempmodels will spawn at the radius distance from\n"
	"the origin"
);
mfuse::EventDef EV_Client_SetInwardSphere
(
	"inwardsphere",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Create the tempmodels in a sphere around the origin, and adjust their\n"
	"angle so they point toward the center of the sphere.  This is best used with a\n"
	"spehere radius and some velocity so the models look like they're heading toward the\n"
	"center of the sphere."
);
mfuse::EventDef EV_Client_SetRadius
(
	"radius",
	mfuse::EV_DEFAULT,
	"f",
	"radius",
	"Set the radius of the sphere for the inwardsphere amd sphere settings"
);
mfuse::EventDef EV_Client_SetSwarm
(
	"swarm",
	mfuse::EV_DEFAULT,
	"iff",
	"frequency maxspeed delta",
	"Create a swarm like effect that the tempmodels follow when they are spawned\n"
	"frequency is how often they change direction\n"
	"maxspeed is how fast the tempmodel will move (it's randomly generated every\n"
	"time the frequency is hit)\n"
	"delta is how much the tempmodel moves toward the origin every frame"
);
mfuse::EventDef EV_Client_SetAlign
(
	"align",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Align the tempmodels to the direction they are traveling"
);
mfuse::EventDef EV_Client_SetAlignOnce
(
	"alignonce",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Align the tempmodels to the direction they are traveling at the time they are initialized"
);
mfuse::EventDef EV_Client_SetFlickerAlpha
(
	"flicker",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodel to change it's alpha every frame.  Creates a flickering effect"
);
mfuse::EventDef EV_Client_SetCollision
(
	"collision",
	mfuse::EV_DEFAULT,
	"S",
	"water",
	"Turn on collision for the tempmodel.\n"
	"If the keyword water is specified, then the tempmodel will collide with water"
);
mfuse::EventDef EV_Client_TagSpawn
(
	"tagspawn",
	mfuse::EV_DEFAULT,
	"s",
	"tagName",
	"Spawn tempmodels from the specified tag.\n"
	"This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
mfuse::EventDef EV_Client_TagSpawnLinked
(
	"tagspawnlinked",
	mfuse::EV_DEFAULT,
	"s",
	"tagName",
	"Spawn tempmodels from the specified tag, linked to the entity at the tag.\n"
	"This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
mfuse::EventDef EV_Client_OriginSpawn
(
	"originspawn",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Spawn tempmodels from the origin.\n"
	"This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
mfuse::EventDef EV_Client_OriginBeamSpawn
(
	"originbeamspawn",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Spawn a beam from the origin.\n"
	"This command is followed by a ( to specify a block of commands that modify the beam"
);
mfuse::EventDef EV_Client_OriginBeamEmitter
(
	"originbeamemitter",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Spawn beams from the origin.\n"
	"This command is followed by a ( to specify a block of commands that modify the beam"
);
mfuse::EventDef EV_Client_TagEmitter
(
	"tagemitter",
	mfuse::EV_DEFAULT,
	"ss",
	"tagName emitterName",
	"Create an emitter that spawns tempmodels from the specified tag.\n"
	"This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
mfuse::EventDef EV_Client_OriginEmitter
(
	"originemitter",
	mfuse::EV_DEFAULT,
	"s",
	"emitterName",
	"Create an emitter that spawns tempmodels from the origin.\n"
	"This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
mfuse::EventDef EV_Client_EmitterOn
(
	"emitteron",
	mfuse::EV_DEFAULT,
	"s",
	"emitterName",
	"Turn the specified emitter on"
);
mfuse::EventDef EV_Client_EmitterOff
(
	"emitteroff",
	mfuse::EV_DEFAULT,
	"s",
	"emitterName",
	"Turn the specified emitter off"
);
mfuse::EventDef EV_Client_Sound
(
	"sound",
	mfuse::EV_DEFAULT,
	"sSFFFFS",
	"soundName channelName volume min_distance pitch randompitch randomvolume argstype",
	"Play the specified sound"
);
mfuse::EventDef EV_Set_Current_Tiki
(
	"settiki",
	mfuse::EV_DEFAULT,
	"s",
	"settiki tikiname - all subsequent mfuse::Events are applied to this tiki",
	"sets the tiki the aliases should be on in the sound uberfile"
);
mfuse::EventDef EV_Client_StopSound
(
	"stopsound",
	mfuse::EV_DEFAULT,
	"s",
	"channelName",
	"Stops the sound on the specified channel."
);
mfuse::EventDef EV_Client_StopAliasChannel
(
	"stopaliaschannel",
	mfuse::EV_DEFAULT,
	"s",
	"alias",
	"Stops the sound channel used by the specified alias."
);
mfuse::EventDef EV_Client_LoopSound
(
	"loopsound",
	mfuse::EV_DEFAULT,
	"sFFf",
	"soundName volume min_distance pitch",
	"Play the specifed sound as a looping sound"
);
mfuse::EventDef EV_Client_Cache
(
	"cache",
	8,
	"s",
	"resourceName",
	"Cache the specified resource"
);
mfuse::EventDef EV_Client_CacheImage
(
	"cacheimage",
	8,
	"s",
	"imageName",
	"Cache the specified image (For menu graphics that should never be picmiped)"
);
mfuse::EventDef EV_Client_CacheFont
(
	"cachefont",
	8,
	"s",
	"fontName",
	"Cache the specified font (For menu graphics that should never be picmiped)"
);
mfuse::EventDef EV_Client_AliasCache
(
	"aliascache",
	mfuse::EV_DEFAULT,
	"ssSSSSSS",
	"alias realPath arg1 arg2 arg3 arg4 arg5 arg6",
	"Create an alias to the specified path and cache the resource"
);
mfuse::EventDef EV_Client_Alias
(
	"alias",
	mfuse::EV_DEFAULT,
	"ssSSSSSS",
	"alias realPath arg1 arg2 arg3 arg4 arg5 arg6",
	"Create an alias to the specified path"
);
mfuse::EventDef EV_Client_CacheAlias
(
	"cachefromalias",
	mfuse::EV_DEFAULT,
	"s",
	"alias",
	"Caches all data matching a previously specified alias"
);
mfuse::EventDef EV_Client_Footstep
(
	"footstep",
	mfuse::EV_DEFAULT,
	"ssI",
	"tag sRunning iEquipment",
	"Play a footstep sound that is appropriate to the surface we are currently stepping on\n"
	"sRunning should be set to run, walk, or ladder"
);
mfuse::EventDef EV_Client_Landing
(
	"landing",
	mfuse::EV_DEFAULT,
	"FI",
	"fVolume iEquipment",
	"Play a landing sound that is appropriate to the surface we are landing on\n"
);
mfuse::EventDef EV_Client_BodyFall
(
	"bodyfall",
	mfuse::EV_DEFAULT,
	"F",
	"fVolume",
	"Play a body fall sound that is appropriate to the surface we are falling on\n"
);
mfuse::EventDef EV_Client_Client
(
	"client",
	mfuse::EV_DEFAULT,
	"SSSSSS",
	"arg1 arg2 arg3 arg4 arg5 arg6",
	"Execute the specified command arg string"
);
mfuse::EventDef EV_Client_OriginDynamicLight
(
	"origindlight",
	mfuse::EV_DEFAULT,
	"fffffSS",
	"red green blue intensity life type1 type2",
	"Spawn a dynamic light from the origin of the model\n"
	"The red,green,blue parms are the color of the light\n"
	"The intensity is the radius of the light\n"
	"type is the type of light to create (lensflare,viewlensflare,additive)"
);
mfuse::EventDef EV_Client_TagDynamicLight
(
	"tagdlight",
	mfuse::EV_DEFAULT,
	"sfffffSS",
	"tagName red green blue intensity life intvel type1 type2",
	"Spawn a dynamic light from the specified tag\n"
	"The red,green,blue parms are the color of the light\n"
	"The intensity is the radius of the light\n"
	"type is the type of light to create (lensflare,viewlensflare,additive)"
);
mfuse::EventDef EV_Client_DynamicLight
(
	"dlight",
	mfuse::EV_DEFAULT,
	"ffffSS",
	"red green blue intensity type1 type2",
	"This makes the emitter itself a dynamic lightThe red,green,blue parms are the color of the light\n"
	"The intensity is the radius of the light\n"
	"type is the type of light to create (lensflare,viewlensflare,additive)"
);
mfuse::EventDef EV_Client_BlockDynamicLight
(
	"blockdlight",
	mfuse::EV_DEFAULT,
	"ffSS",
	"intensity life type1 type2",
	"Spawn a dynamic light from the origin of the model\n"
	"An additional commands block allows the specification of moving & varying dlights\n"
	"The intensity is the radius of the light\n"
	"type is the type of light to create (lensflare,viewlensflare,additive)"
);
mfuse::EventDef EV_Client_SetEntityColor
(
	"entcolor",
	mfuse::EV_DEFAULT,
	"fffF",
	"red green blue alpha",
	"Set the color(modulate) of this entity"
);
mfuse::EventDef EV_Client_SetTexAnimTime
(
	"texanimtime",
	mfuse::EV_DEFAULT,
	"f",
	"time",
	"Set the texture animation speed"
);
mfuse::EventDef EV_Client_SetGlobalFade
(
	"globalfade",
	mfuse::EV_DEFAULT,
	"S",
	"[in|out]",
	"Set the tempmodels to globally fade in or out together"
);
mfuse::EventDef EV_Client_SetParentLink
(
	"parentlink",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodels linked to the parent, so they move with the parent model"
);
mfuse::EventDef EV_Client_SetHardLink
(
	"hardlink",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodels linked to the model they are spawned from, so they move with it"
);
mfuse::EventDef EV_Client_SetRandomRoll
(
	"randomroll",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodels so they pick a random roll value every frame"
);
mfuse::EventDef EV_Client_ParentAngles
(
	"parentangles",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodels angles to that of its parent"
);
mfuse::EventDef EV_Client_EmitterAngles
(
	"emitterangles",
	mfuse::EV_DEFAULT,
	"FFF",
	"pitchofs yawofs rollofs",
	"Set the tempmodels angles to that of the emitter\n"
	"The three optional parameters are for setting an angle offset from the emitter"
);
mfuse::EventDef EV_Client_SetAngles
(
	"angles",
	mfuse::EV_DEFAULT,
	"SfFSfFSfF",
	"[random|crandom|range] pitch [pitch2] [random|crandom|range] yaw [yaw2] [random|crandom|range] roll [roll2]",
	"If random is specified, the component will range from 0 to +specified value.\n"
	"If crandom is specified, the component will range from -specified to +specified value.\n"
	"If range is specified, the component needs two values; it will randomly pick a number in the range\n"
	"from the first number to the first number plus the second number.\n"
	"If no keyword is explicitly specified, then the component will be just set\n"
	"without randomness."
);
mfuse::EventDef EV_Client_Swipe
(
	"swipe",
	mfuse::EV_DEFAULT,
	"V",
	"origin",
	"Do a swipe and add it on to the swipe rendering list."
);
mfuse::EventDef EV_Client_SwipeOn
(
	"swipeon",
	mfuse::EV_DEFAULT,
	"ssff",
	"shader startTagName endTagNamelife life",
	"Signal the start of a swipe from the current tag"
);
mfuse::EventDef EV_Client_SwipeOff
(
	"swipeoff",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Signal the end of a swipe"
);
mfuse::EventDef EV_Client_BeginTagBeamEmitter
(
	"tagbeamemitter",
	mfuse::EV_DEFAULT,
	"sss",
	"tagstart tagend name",
	"Create a beam emitter that uses 2 tags to determine it's start and end position"
);
mfuse::EventDef EV_Client_BeginTagBeamSpawn
(
	"tagbeamspawn",
	mfuse::EV_DEFAULT,
	"sf",
	"tagstart name",
	"Create a beam emitter that uses the tag to determine it's starting position."
);
mfuse::EventDef EV_Client_AnimateOnce
(
	"animateonce",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set a tempmodel to animate once and then get removed"
);
mfuse::EventDef EV_Client_SetAnim
(
	"anim",
	mfuse::EV_DEFAULT,
	"s",
	"animation",
	"Set a tempmodel the the specified animation"
);
mfuse::EventDef EV_Client_SetSubdivisions
(
	"numsegments",
	mfuse::EV_DEFAULT,
	"i",
	"numsegments",
	"Set the number of segments in a beam"
);
mfuse::EventDef EV_Client_SetMinOffset
(
	"minoffset",
	mfuse::EV_DEFAULT,
	"f",
	"minoffset",
	"Set the minimum offset from center in a beam"
);
mfuse::EventDef EV_Client_SetMaxOffset
(
	"maxoffset",
	mfuse::EV_DEFAULT,
	"f",
	"maxoffset",
	"Set the maximum offset from center in a beam"
);
mfuse::EventDef EV_Client_SetBeamShader
(
	"beamshader",
	mfuse::EV_DEFAULT,
	"s",
	"shadername",
	"Set the shader to use for the beam"
);
mfuse::EventDef EV_Client_SetBeamLength
(
	"beamlength",
	mfuse::EV_DEFAULT,
	"f",
	"length",
	"Set the length of the beam or trace length (for decals)"
);
mfuse::EventDef EV_Client_SetBeamDelay
(
	"beamdelay",
	mfuse::EV_DEFAULT,
	"SF",
	"[random] delay",
	"Set the delay time between creating new beams.\n"
	"If the keyword random is specified, the delay between beams will occur randomly between 0 and the time specified"
);
mfuse::EventDef EV_Client_SetBeamToggleDelay
(
	"beamtoggledelay",
	mfuse::EV_DEFAULT,
	"SF",
	"[random] delay",
	"Set a delay between toggling the beams on and off.\n"
	"If the keyword random is specified, the delay between toggling will occur randomly between 0 and the time specified"
);
mfuse::EventDef EV_Client_SetBeamPersist
(
	"beampersist",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Make the beams persist instead of blinking out"
);
mfuse::EventDef EV_Client_SetBeamOffsetEndpoints
(
	"beam_offset_endpoints",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Make the beams endpoints offset to reduce the bunching up effect"
);
mfuse::EventDef EV_Client_BeamSphere
(
	"beamsphere",
	mfuse::EV_DEFAULT,
	"i",
	"count",
	"Create a sphere shaped beam effect from the origin. Count is the number of beams"
);
mfuse::EventDef EV_Client_Spread
(
	"spread",
	mfuse::EV_DEFAULT,
	"ff",
	"spreadx spready",
	"Add a random variance in the spawned beam in the forward direction by the amound specified in spreadx and spready"
);
mfuse::EventDef EV_Client_UseLastTraceEnd
(
	"uselasttraceend",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Makes this trace command use the end results of the last trace command"
);
mfuse::EventDef EV_Client_OffsetAlongAxis
(
	"offsetalongaxis",
	mfuse::EV_DEFAULT,
	"SfFSfFSfF",
	"[crandom|random|range] offsetx [offsetx2] [crandom|random|range] offsety [offsety2] [crandom|random|range] offsetz [offsetz2]",
	"If random is specified, the component will range from 0 to specified offset.\n"
	"If crandom is specified, the component will range from -specified to +specified offset.\n"
	"If range is specified, the component needs two values; it will randomly pick a number in the range\n"
	"from the first number to the first number plus the second number.\n"
	"If no keyword is explicitly specified, then the component will just be added on\n"
	"without randomness.\n"
	"This offset is applied using the model's local axis"
);
mfuse::EventDef EV_Client_Cone
(
	"cone",
	mfuse::EV_DEFAULT,
	"ff",
	"height radius",
	"Randomly spawns the particle somewhere inside a cone along the model's local x axis"
);
mfuse::EventDef EV_Client_SetEndAlpha
(
	"endalpha",
	mfuse::EV_DEFAULT,
	"f",
	"alpha",
	"Set the alpha of the beam's endpoint"
);
mfuse::EventDef EV_Client_RandomChance
(
	"randomchance",
	mfuse::EV_DEFAULT,
	"fSSSSSS",
	"percentage [arg1] [arg2] [arg3] [arg4] [arg5] [arg6]",
	"Set the percentage chance that command will occur"
);
mfuse::EventDef EV_Client_DelayedRepeat
(
	"delayedrepeat",
	mfuse::EV_DEFAULT,
	"fiSSSSSS",
	"time commandnumber [arg1] [arg2] [arg3] [arg4] [arg5] [arg6]",
	"Set the time delay between this command getting executed.  This requires a command number to be assigned here.\n"
	"This is internally used to keep track of the commands this entity executes and will resolve naming conflicts.\n"
	"This is only useful if the command gets called continuously but must execute at regular intervals.\n"
);
mfuse::EventDef EV_Client_CommandDelay
(
	"commanddelay",
	mfuse::EV_DEFAULT,
	"fsSSSSSS",
	"delay command [arg1] [arg2] [arg3] [arg4] [arg5] [arg6]",
	"executes a command after the given delay"
);
mfuse::EventDef EV_Client_BounceDecal
(
	"bouncedecal",
	mfuse::EV_DEFAULT,
	"iI",
	"maxamount temporary",
	"Put a mark when the tempmodel bounces and hits a surface\n"
	"maxamount = Max amount of decals to make when bouncing\n"
	"temporary = specify 1 for a temporary mark that only appears for a short time, 0 for a decal that stays aroung l"
	"onger (default is 0)\n"
);
mfuse::EventDef EV_Client_SetDecalRadius
(
	"decalradius",
	mfuse::EV_DEFAULT,
	"f",
	"radius",
	"Set the radius of the decal"
);
mfuse::EventDef EV_Client_SetDecalOrientation
(
	"orientation",
	mfuse::EV_DEFAULT,
	"f",
	"degrees",
	"Set the degrees to orient the decal.  Specify 'random' to use a random orientation"
);
mfuse::EventDef EV_Client_SetDecalShader
(
	"decalshader",
	mfuse::EV_DEFAULT,
	"s",
	"shadername",
	"Set the shader to use for the impact decal"
);
mfuse::EventDef EV_Client_SetTraceLength
(
	"tracelength",
	mfuse::EV_DEFAULT,
	"f",
	"length",
	"Set the length of the trace for the decal"
);
mfuse::EventDef EV_Client_TraceSpawn
(
	"tracespawn",
	mfuse::EV_DEFAULT,
	"s",
	"modelname",
	"Spawn the specified model when the trace hits a solid object"
);
mfuse::EventDef EV_Client_TagList
(
	"taglist",
	mfuse::EV_DEFAULT,
	"ssSSSSSS",
	"arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8",
	"Set the tag list to create a beam that travels between all the tags"
);
mfuse::EventDef EV_Client_Lightstyle
(
	"lightstyle",
	mfuse::EV_DEFAULT,
	"s",
	"nameOfImage",
	"Set a lightstyle to determine the color of this tempmodel, the image\n"
	"specified is used to determine the look of the light style"
);
mfuse::EventDef EV_Client_PhysicsRate
(
	"physicsrate",
	mfuse::EV_DEFAULT,
	"s",
	"rate",
	"Set the rate (in updates per second) for the tempmodel's physics to be updated"
);
mfuse::EventDef EV_Client_Parallel
(
	"parallel",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set tempmodel to be parallel to the viewer"
);
mfuse::EventDef EV_Client_AlwaysDraw
(
	"alwaysdraw",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set emitter/tempmodel to be alwaysdraw. Which can be turned off by alwaysdraw"
);
mfuse::EventDef EV_Client_Detail
(
	"detail",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set emitter/tempmodel to be detail. Which can be turned off by detail"
);
mfuse::EventDef EV_Client_SetVolumetric
(
	"volumetric",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the effect to spawn volumetric sources rather than tempmodels"
);
mfuse::EventDef EV_Client_Wind
(
	"wind",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Makes the temp model be affected by wind"
);
mfuse::EventDef EV_Client_SpriteGridLighting
(
	"spritegridlighting",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"Calculates grid lighting for a sprite"
);
mfuse::EventDef EV_Client_WaterOnly
(
	"wateronly",
	mfuse::EV_DEFAULT,
	nullptr,
	nullptr,
	"makes the temp model remove itself if it leaves water"
);
mfuse::EventDef EV_Client_SetAlignStretch
(
	"alignstretch",
	mfuse::EV_DEFAULT,
	"F",
	"scaleMultiplier",
	"Aligns the temp model to the direction of travel, and stretches it betwen the last and current positions.\n"
);
mfuse::EventDef EV_Client_SetClampVelocity
(
	"clampvel",
	mfuse::EV_DEFAULT,
	"ffffff",
	"minX maxX minY maxY minZ maxZ",
	"sets the valid range for velocities along global axes.  Cannot be used with clampvelaxis."
);
mfuse::EventDef EV_Client_SetClampVelocityAxis
(
	"clampvelaxis",
	mfuse::EV_DEFAULT,
	"ffffff",
	"minX maxX minY maxY minZ maxZ",
	"sets the valid range for velocities along oriented axes.  Cannot be used with clampvel."
);
mfuse::EventDef EV_Client_Treads
(
	"treads",
	mfuse::EV_DEFAULT,
	"ssi",
	"tagName shader localrefnumber",
	"Spawn treads from the specified tag using the specified tread type."
);
mfuse::EventDef EV_Client_Treads_Off
(
	"treadsoff",
	mfuse::EV_DEFAULT,
	"si",
	"tagName localrefnumber",
	"Stops spawning treads from the specified tag."
);
mfuse::EventDef EV_Client_ViewKick
(
	"viewkick",
	mfuse::EV_DEFAULT,
	"fffffsff",
	"pitchmin pitchmax yawmin yawmax recenterspeed patters pitchmax yawmax scatterpitchmax",
	"Adds kick to the view of the owner when fired."
);
mfuse::EventDef EV_Client_EyeLimits
(
	"eyelimits",
	mfuse::EV_DEFAULT,
	"fff",
	"fPitchmax fYawmax fRollmax",
	"Set the max angle offsets for the eyes from the model's head"
);
mfuse::EventDef EV_Client_EyeMovement
(
	"eyemovement",
	mfuse::EV_DEFAULT,
	"f",
	"fMovement",
	"Sets the amount of the head's movement to apply to the players view. 1 is full, 0 is none"
);
mfuse::EventDef EV_Client_SFXStart
(
	"sfx",
	mfuse::EV_DEFAULT,
	"sSSSSSSSS",
	"sCommand arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8",
	"Used for adding commands to a special effect"
);
mfuse::EventDef EV_Client_SFXStartDelayed
(
	"delayedsfx",
	mfuse::EV_DEFAULT,
	"fsSSSSSSSS",
	"fDelay sCommand arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8",
	"Used for adding commands to a special effect with a time delay"
);

MFUS_CLASS_DECLARATION(Listener, EmitterListener, "")
{
	{ &EV_Client_StartBlock,					&EmitterListener::StartBlock },
	{ &EV_Client_EndBlock,						&EmitterListener::EndBlock },
	{ &EV_Client_EmitterStartOff,				NULL },
	{ &EV_Client_SetAlpha,						&EmitterListener::SetAlpha },
	{ &EV_Client_SetDieTouch,					NULL },
	{ &EV_Client_SetBounceFactor,				NULL },
	{ &EV_Client_SetBounceSound,				NULL },
	{ &EV_Client_SetBounceSoundOnce,			NULL },
	{ &EV_Client_SetScale,						&EmitterListener::SetScale },
	{ &EV_Client_SetScaleUpDown,				&EmitterListener::SetScaleUpDown },
	{ &EV_Client_SetScaleMin,					&EmitterListener::SetScaleMin },
	{ &EV_Client_SetScaleMax,					&EmitterListener::SetScaleMax },
	{ &EV_Client_SetModel,						&EmitterListener::SetModel },
	{ &EV_Client_SetLife,						&EmitterListener::SetLife },
	{ &EV_Client_SetColor,						&EmitterListener::SetColor },
	{ &EV_Client_RadialVelocity,				&EmitterListener::SetRadialVelocity },
	{ &EV_Client_SetVelocity,					&EmitterListener::SetVelocity },
	{ &EV_Client_SetAngularVelocity,			&EmitterListener::SetAngularVelocity },
	{ &EV_Client_SetColorVelocity,				&EmitterListener::SetAngularVelocity },
	{ &EV_Client_SetRandomVelocity,				&EmitterListener::SetRandomVelocity },
	{ &EV_Client_SetRandomVelocityAlongAxis,	&EmitterListener::SetRandomVelocityAlongAxis },
	{ &EV_Client_SetAccel,						&EmitterListener::SetAccel },
	{ &EV_Client_SetFriction,					NULL },
	{ &EV_Client_SetVaryColor,					NULL },
	{ &EV_Client_SetSmokeParms,					NULL },
	{ &EV_Client_SetCount,						&EmitterListener::SetCount },
	{ &EV_Client_SetFade,						&EmitterListener::SetFade },
	{ &EV_Client_SetFadeDelay,					&EmitterListener::SetFadeDelay },
	{ &EV_Client_SetFadeIn,						&EmitterListener::SetFadeIn },
	{ &EV_Client_SetTwinkle,					NULL },
	{ &EV_Client_SetTrail,						NULL },
	{ &EV_Client_SetSpawnRange,					NULL },
	{ &EV_Client_SetSpawnRate,					&EmitterListener::SetSpawnRate },
	{ &EV_Client_SetOriginOffset,				&EmitterListener::SetOffset },
	{ &EV_Client_SetScaleRate,					&EmitterListener::SetScaleRate },
	{ &EV_Client_SetCircle,						&EmitterListener::SetCircle },
	{ &EV_Client_SetSphere,						&EmitterListener::SetSphere },
	{ &EV_Client_SetInwardSphere,				&EmitterListener::SetInwardSphere },
	{ &EV_Client_SetRadius,						&EmitterListener::SetRadius },
	{ &EV_Client_SetSwarm,						NULL },
	{ &EV_Client_SetAlign,						NULL },
	{ &EV_Client_SetAlignOnce,					NULL },
	{ &EV_Client_SetFlickerAlpha,				NULL },
	{ &EV_Client_SetCollision,					NULL },
	{ &EV_Client_TagSpawn,						NULL },
	{ &EV_Client_TagSpawnLinked,				NULL },
	{ &EV_Client_OriginSpawn,					&EmitterListener::BeginOriginSpawn },
	{ &EV_Client_OriginBeamSpawn,				NULL },
	{ &EV_Client_OriginBeamEmitter,				NULL },
	{ &EV_Client_TagEmitter,					NULL },
	{ &EV_Client_OriginEmitter,					&EmitterListener::BeginOriginEmitter },
	{ &EV_Client_EmitterOn,						NULL },
	{ &EV_Client_EmitterOff,					NULL },
	{ &EV_Client_Sound,							NULL },
	{ &EV_Set_Current_Tiki,						NULL },
	{ &EV_Client_StopSound,						NULL },
	{ &EV_Client_StopAliasChannel,				NULL },
	{ &EV_Client_LoopSound,						NULL },
	{ &EV_Client_Cache,							NULL },
	{ &EV_Client_CacheImage,					NULL },
	{ &EV_Client_CacheFont,						NULL },
	{ &EV_Client_AliasCache,					NULL },
	{ &EV_Client_Alias,							NULL },
	{ &EV_Client_CacheAlias,					NULL },
	{ &EV_Client_Footstep,						NULL },
	{ &EV_Client_Landing,						NULL },
	{ &EV_Client_BodyFall,						NULL },
	{ &EV_Client_Client,						NULL },
	{ &EV_Client_OriginDynamicLight,			NULL },
	{ &EV_Client_TagDynamicLight,				NULL },
	{ &EV_Client_DynamicLight,					NULL },
	{ &EV_Client_BlockDynamicLight,				NULL },
	{ &EV_Client_SetEntityColor,				NULL },
	{ &EV_Client_SetTexAnimTime,				NULL },
	{ &EV_Client_SetGlobalFade,					NULL },
	{ &EV_Client_SetParentLink,					NULL },
	{ &EV_Client_SetHardLink,					NULL },
	{ &EV_Client_SetRandomRoll,					NULL },
	{ &EV_Client_ParentAngles,					NULL },
	{ &EV_Client_EmitterAngles,					NULL },
	{ &EV_Client_SetAngles,						&EmitterListener::SetAngles },
	{ &EV_Client_Swipe,							NULL },
	{ &EV_Client_SwipeOn,						NULL },
	{ &EV_Client_SwipeOff,						NULL },
	{ &EV_Client_BeginTagBeamEmitter,			NULL },
	{ &EV_Client_BeginTagBeamSpawn,				NULL },
	{ &EV_Client_AnimateOnce,					NULL },
	{ &EV_Client_SetAnim,						NULL },
	{ &EV_Client_SetSubdivisions,				NULL },
	{ &EV_Client_SetMinOffset,					NULL },
	{ &EV_Client_SetMaxOffset,					NULL },
	{ &EV_Client_SetBeamShader,					NULL },
	{ &EV_Client_SetBeamLength,					NULL },
	{ &EV_Client_SetBeamDelay,					NULL },
	{ &EV_Client_SetBeamToggleDelay,			NULL },
	{ &EV_Client_SetBeamPersist,				NULL },
	{ &EV_Client_SetBeamOffsetEndpoints,		NULL },
	{ &EV_Client_BeamSphere,					NULL },
	{ &EV_Client_Spread,						NULL },
	{ &EV_Client_UseLastTraceEnd,				NULL },
	{ &EV_Client_OffsetAlongAxis,				&EmitterListener::SetOffsetAlongAxis },
	{ &EV_Client_Cone,							&EmitterListener::SetCone },
	{ &EV_Client_SetEndAlpha,					NULL },
	{ &EV_Client_RandomChance,					NULL },
	{ &EV_Client_DelayedRepeat,					NULL },
	{ &EV_Client_CommandDelay,					NULL },
	{ &EV_Client_BounceDecal,					NULL },
	{ &EV_Client_SetDecalRadius,				NULL },
	{ &EV_Client_SetDecalOrientation,			NULL },
	{ &EV_Client_SetDecalShader,				NULL },
	{ &EV_Client_SetTraceLength,				NULL },
	{ &EV_Client_TraceSpawn,					NULL },
	{ &EV_Client_TagList,						NULL },
	{ &EV_Client_Lightstyle,					NULL },
	{ &EV_Client_PhysicsRate,					NULL },
	{ &EV_Client_Parallel,						NULL },
	{ &EV_Client_AlwaysDraw,					NULL },
	{ &EV_Client_Detail,						NULL },
	{ &EV_Client_SetVolumetric,					NULL },
	{ &EV_Client_Wind,							NULL },
	{ &EV_Client_SpriteGridLighting,			NULL },
	{ &EV_Client_WaterOnly,						NULL },
	{ &EV_Client_SetAlignStretch,				NULL },
	{ &EV_Client_SetClampVelocity,				NULL },
	{ &EV_Client_SetClampVelocityAxis,			NULL },
	{ &EV_Client_Treads,						NULL },
	{ &EV_Client_Treads_Off,					NULL },
	{ &EV_Client_SFXStart,						&EmitterListener::StartSFX },
	{ &EV_Client_SFXStartDelayed,				&EmitterListener::StartSFXDelayed },
	{ NULL, NULL }
};

EmitterListener::EmitterListener()
{
	emitter = nullptr;
	bProcessed = false;
}

EmitterListener::EmitterListener(Emitter* OutEmitter)
	: EmitterListener()
{
	emitter = OutEmitter;
}

bool EmitterListener::FinishedParsing() const
{
	return bProcessed;
}

void EmitterListener::BeginOriginSpawn(mfuse::Event* ev)
{
	emitter->bIsEmitter = false;
	emitter->emitterName = ev->GetString(1);
}

void EmitterListener::BeginOriginEmitter(mfuse::Event* ev)
{
	emitter->bIsEmitter = true;
	emitter->emitterName = ev->GetString(1);
}

void EmitterListener::StartBlock(mfuse::Event* ev)
{
	bInBlock = true;
}

void EmitterListener::EndBlock(mfuse::Event* ev)
{
	bInBlock = false;
	bProcessed = true;
}

void EmitterListener::StartSFX(mfuse::Event* ev)
{
	StartSFXInternal(ev, false);
}

void EmitterListener::StartSFXDelayed(mfuse::Event* ev)
{
	StartSFXInternal(ev, true);
}

void EmitterListener::StartSFXInternal(mfuse::Event* ev, bool bDelayed)
{
	emitter->startTime = bDelayed ? ev->GetFloat(1) : 0.f;
	str commandName = ev->GetString(bDelayed + 1);

	mfuse::Event newEvent(mfuse::EventSystem::Get().FindNormalEventNum(commandName.c_str()));
	for (size_t i = bDelayed + 2; i < ev->NumArgs(); i++)
	{
		newEvent.AddString(ev->GetString(i));
	}

	ProcessEvent(newEvent);
}

void EmitterListener::SetBaseAndAmplitude(mfuse::Event* ev, vec3r_t Base, vec3r_t Amplitude)
{
	int32_t i = 1;

	for (int32_t j = 0; j < 3; j++)
	{
		const str org = ev->GetString(i++);
		if (!strHelpers::cmp(org.c_str(), "crandom"))
		{
			const float ampl = ev->GetFloat(i++);
			Amplitude[j] = ampl + ampl;
			Base[j] = -ampl;
		}
		else if (!strHelpers::cmp(org.c_str(), "random"))
		{
			Base[j] = 0.f;
			Amplitude[j] = ev->GetFloat(i++);
		}
		else if (!strHelpers::cmp(org.c_str(), "range"))
		{
			Base[j] = ev->GetFloat(i++);
			Amplitude[j] = ev->GetFloat(i++);
		}
		else
		{
			Base[j] = (float)atof(org.c_str());
			Amplitude[j] = 0.f;
		}
	}
}

void EmitterListener::SetSpawnRate(mfuse::Event* ev)
{
	emitter->spawnRate = ev->GetFloat(1);
}

void EmitterListener::SetScaleRate(mfuse::Event* ev)
{
	emitter->scaleRate = ev->GetFloat(1);
}

void EmitterListener::SetCount(mfuse::Event* ev)
{
	emitter->count = ev->GetInteger(1);
}

void EmitterListener::SetScale(mfuse::Event* ev)
{
	emitter->scale = ev->GetFloat(1);
}

void EmitterListener::SetScaleUpDown(mfuse::Event* ev)
{
	emitter->flags |= Emitter::EF_UpDownScale;
}

void EmitterListener::SetScaleMin(mfuse::Event* ev)
{
	emitter->scaleMin = ev->GetFloat(1);
}

void EmitterListener::SetScaleMax(mfuse::Event* ev)
{
	emitter->scaleMax = ev->GetFloat(1);
}

void EmitterListener::SetModel(mfuse::Event* ev)
{
	str resourceName = ev->GetString(1);
	if (resourceName.length() > 0)
	{
		const char* extension = FileHelpers::getExtension(resourceName.c_str());
		
		if (!strHelpers::icmp(extension, "spr"))
		{
			resourceName = str(resourceName, extension - resourceName.c_str());
			emitter->sprite.spriteType = Sprite::ST_Shader;
			const ShaderManagerPtr shaderManager = assetManager->getManager<ShaderManager>();
			if(shaderManager)
			{
				ShaderPtr shader = shaderManager->GetShader(resourceName.c_str());
				if (shader)
				{
					emitter->sprite.Shader = new ShaderPtr(std::move(shader));
				}
			}
		}
		else if (!strHelpers::icmp(extension, "tik"))
		{
			emitter->sprite.spriteType = Sprite::ST_Tiki;
			auto Tiki = assetManager->readAsset<TIKIReader>(resourceName.c_str());
			if (Tiki)
			{
				emitter->sprite.Tiki = new SharedPtr<TIKI>(std::move(Tiki));
			}
		}
	}
}

void EmitterListener::SetOffset(mfuse::Event* ev)
{
	SetBaseAndAmplitude(ev, emitter->originOffsetBase, emitter->originOffsetAmplitude);
}

void EmitterListener::SetOffsetAlongAxis(mfuse::Event* ev)
{
	emitter->flags |= Emitter::EF_UseParentAxis;
	SetBaseAndAmplitude(ev, emitter->originOffsetBase, emitter->originOffsetAmplitude);
}

void EmitterListener::SetLife(mfuse::Event* ev)
{
	emitter->life = ev->GetFloat(1);

	if (ev->NumArgs() > 1)
	{
		emitter->randomLife = ev->GetFloat(2);
	}
}

void EmitterListener::SetColor(mfuse::Event* ev)
{
	emitter->color[0] = ev->GetFloat(1);
	emitter->color[1] = ev->GetFloat(2);
	emitter->color[2] = ev->GetFloat(3);

	if (ev->NumArgs() > 3)
	{
		emitter->alpha = ev->GetFloat(4);
	}
}

void EmitterListener::SetAlpha(mfuse::Event* ev)
{
	emitter->alpha = ev->GetFloat(1);
}

void EmitterListener::SetAngles(mfuse::Event* ev)
{
	SetBaseAndAmplitude(ev, emitter->anglesOffsetBase, emitter->anglesOffsetAmplitude);
}

void EmitterListener::SetRadialVelocity(mfuse::Event* ev)
{
	emitter->radialVelocity[0] = ev->GetFloat(1);
	emitter->radialVelocity[1] = ev->GetFloat(2);
	emitter->radialVelocity[2] = ev->GetFloat(3);
}

void EmitterListener::SetVelocity(mfuse::Event* ev)
{
	emitter->forwardVelocity = ev->GetFloat(1);
}

void EmitterListener::SetAngularVelocity(mfuse::Event* ev)
{
	SetBaseAndAmplitude(ev, emitter->randAVelocityBase, emitter->randAVelocityAmplitude);
}

void EmitterListener::SetRandomVelocity(mfuse::Event* ev)
{
	SetBaseAndAmplitude(ev, emitter->randVelocityBase, emitter->randVelocityAmplitude);
}

void EmitterListener::SetRandomVelocityAlongAxis(mfuse::Event* ev)
{
	emitter->flags |= Emitter::EF_UseParentAxis;
	SetRandomVelocity(ev);
}

void EmitterListener::SetAccel(mfuse::Event* ev)
{
	emitter->accel[0] = ev->GetFloat(1);
	emitter->accel[1] = ev->GetFloat(2);
	emitter->accel[2] = ev->GetFloat(3);
}

void EmitterListener::SetCone(mfuse::Event* ev)
{
	emitter->spawnType = Emitter::EST_Cone;
	emitter->coneHeight = ev->GetFloat(1);
	emitter->sphereRadius = ev->GetFloat(2);
}

void EmitterListener::SetCircle(mfuse::Event* ev)
{
	emitter->spawnType = Emitter::EST_Circle;
}

void EmitterListener::SetSphere(mfuse::Event* ev)
{
	emitter->spawnType = Emitter::EST_Sphere;
}

void EmitterListener::SetInwardSphere(mfuse::Event* ev)
{
	emitter->spawnType = Emitter::EST_InwardSphere;
}

void EmitterListener::SetRadius(mfuse::Event* ev)
{
	emitter->sphereRadius = ev->GetFloat(1);
}

void EmitterListener::SetFade(mfuse::Event* ev)
{
	emitter->flags |= Emitter::EF_Fade;
}

void EmitterListener::SetFadeDelay(mfuse::Event* ev)
{
	emitter->flags |= Emitter::EF_Fade;
	emitter->fadeDelay = ev->GetFloat(1);
}

void EmitterListener::SetFadeIn(mfuse::Event* ev)
{
	emitter->flags |= Emitter::EF_FadeIn;
	emitter->fadeInTime = ev->GetFloat(1);
}

void EmitterListener::SetAssetManager(const AssetManagerPtr& inAssetManager)
{
	assetManager = inAssetManager;
}
