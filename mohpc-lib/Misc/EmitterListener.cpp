#include <Shared.h>
#include "EmitterListener.h"
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/EmitterManager.h>
#include <MOHPC/Managers/EventManager.h>
#include <MOHPC/Managers/ShaderManager.h>
#include <MOHPC/Formats/TIKI.h>

using namespace MOHPC;

Event EV_Client_StartBlock
(
	"(",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Signals the beginning of a block of commands"
);
Event EV_Client_EndBlock
(
	")",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Signals the end of a block of commands"
);
Event EV_Client_EmitterStartOff
(
	"startoff",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Signals an emitter to start in the off state (no tempmodels are emitted)"
);
Event EV_Client_SetAlpha
(
	"alpha",
	EV_DEFAULT,
	"f",
	"alpha",
	"Set the alpha of the spawned tempmodel"
);
Event EV_Client_SetDieTouch
(
	"dietouch",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the spawned tempmodels to die when they touch a solid"
);
Event EV_Client_SetBounceFactor
(
	"bouncefactor",
	EV_DEFAULT,
	"f",
	"factor",
	"Set the bounciness of a tempmodel when it hits a solid.\n"
	"A factor > 1 will make the model bounce higher and higher on each hit"
);
Event EV_Client_SetBounceSound
(
	"bouncesound",
	EV_DEFAULT,
	"sF",
	"sound [delay]",
	"When bouncing, what sound to play on impact and an option delay (default is 1 second) between playing this sound"
);
Event EV_Client_SetBounceSoundOnce
(
	"bouncesoundonce",
	EV_DEFAULT,
	"s",
	"sound",
	"When bouncing, what sound to play on impact one time"
);
Event EV_Client_SetScale
(
	"scale",
	EV_DEFAULT,
	"f",
	"scale",
	"Set the scale of a spawned tempmodel"
);
Event EV_Client_SetScaleUpDown
(
	"scaleupdown",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodel to scale up to scale value and then down."
);
Event EV_Client_SetScaleMin
(
	"scalemin",
	EV_DEFAULT,
	"f",
	"scalemin",
	"Set the minimum scale of a spawned tempmodel"
);
Event EV_Client_SetScaleMax
(
	"scalemax",
	EV_DEFAULT,
	"f",
	"scalemax",
	"Set the maximum scale of a spawned tempmodel"
);
Event EV_Client_SetModel
(
	"model",
	EV_DEFAULT,
	"sSSSSS",
	"modelname1 modelname2 modelname3 modelname4 modelname5 modelname6",
	"Set the modelname of the tempmodel.  If more than 1 model is specified, it will\n"
	"be randomly chosen when spawned"
);
Event EV_Client_SetLife
(
	"life",
	EV_DEFAULT,
	"fF",
	"life randomlife",
	"Set the life (in seconds) of the spawned tempmodel"
);
Event EV_Client_SetColor
(
	"color",
	EV_DEFAULT,
	"fffF",
	"red green blue alpha",
	"Set the color (modulate) of the spawned tempmodel."
);
Event EV_Client_RadialVelocity
(
	"radialvelocity",
	EV_DEFAULT,
	"fff",
	"scale min_additional max_additional",
	"Subtracts the particle origin from origin and multiplies by scale, then adds additional velocity\n"
	"between min and max... negative values bring toward origin\n"
);
Event EV_Client_SetVelocity
(
	"velocity",
	EV_DEFAULT,
	"f",
	"forwardVelocity",
	"Set the forward velocity of the spawned tempmodel"
);
Event EV_Client_SetAngularVelocity
(
	"avelocity",
	EV_DEFAULT,
	"fff",
	"yawVel pitchVel rollVel",
	"Set the angular velocity of the spawned tempmodel"
);
Event EV_Client_SetColorVelocity
(
	"colorvelocity",
	EV_DEFAULT,
	"fff",
	"rVel gVel bVel",
	"Set the color velocity of the spawned dlight tempmodel"
);
Event EV_Client_SetRandomVelocity
(
	"randvel",
	EV_DEFAULT,
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
Event EV_Client_SetRandomVelocityAlongAxis
(
	"randvelaxis",
	EV_DEFAULT,
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
Event EV_Client_SetAccel
(
	"accel",
	EV_DEFAULT,
	"fff",
	"xAcc yAcc zAcc",
	"Set the acceleration of the spawned tempmodel.\nThis acceleration is applied using the world axis"
);
Event EV_Client_SetFriction
(
	"friction",
	EV_DEFAULT,
	"f",
	"friction",
	"Set the friction as a fraction of velocity per second... exact effect depends on physics rate:\n"
	"slowdown per second = [1 - (friction / physicsrate)] ^ physicsrate; physicsrate defaults to 10"
);
Event EV_Client_SetVaryColor
(
	"varycolor",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Sets the color to vary by 0 to -20% of specified color"
);
Event EV_Client_SetSmokeParms
(
	"smokeparms",
	EV_DEFAULT,
	"fff",
	"typeinfo fademult scalemult",
	"Sets some misc parms for smoke"
);
Event EV_Client_SetCount
(
	"count",
	EV_DEFAULT,
	"i",
	"count",
	"Set the number of tempmodels that are spawned\n"
	"This is only used for the originspawn and tagspawn commands,\n"
	"and not for emitters, use spawnrate instead"
);
Event EV_Client_SetFade
(
	"fade",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodel to fade out over it's life"
);
Event EV_Client_SetFadeDelay
(
	"fadedelay",
	EV_DEFAULT,
	"f",
	"time",
	"Set the amount of time to delay a fade"
);
Event EV_Client_SetFadeIn
(
	"fadein",
	EV_DEFAULT,
	"f",
	"time",
	"Set the tempmodel to fade in over the specified time"
);
Event EV_Client_SetTwinkle
(
	"twinkle",
	EV_DEFAULT,
	"ffff",
	"mintimeoff maxtimeoff mintimeon maxtimeon",
	"Set the tempmodel to twinkle with the specified settings"
);
Event EV_Client_SetTrail
(
	"trail",
	EV_DEFAULT,
	"sssf",
	"shader startTag endTag life",
	"Set the tempmodel to have a swipe that follows it"
);
Event EV_Client_SetSpawnRange
(
	"spawnrange",
	EV_DEFAULT,
	"iI",
	"range1 range2",
	"Sets the range in which this effect will spawn tempmodels.  If one number is specified, it is the max range\n"
	"and 0 is the min range; if two numbers are specified, the larger is the max range.\n"
);
Event EV_Client_SetSpawnRate
(
	"spawnrate",
	EV_DEFAULT,
	"f",
	"rate",
	"Set the spawnrate of the emitter (models per second).\n"
	"This is only used for emitters and not for the originspawn and tagspawn commands"
);
Event EV_Client_SetOriginOffset
(
	"offset",
	EV_DEFAULT,
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
Event EV_Client_SetScaleRate
(
	"scalerate",
	EV_DEFAULT,
	"f",
	"rate",
	"Set the scaling rate of the spawned tempmodel\nIf a negative rate is used, the model will shrink"
);
Event EV_Client_SetCircle
(
	"circle",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodels to be spawned in a circle around the origin\n"
	"This circle will be generated in the X/Y axis of the model"
);
Event EV_Client_SetSphere
(
	"sphere",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodels to spawn in a sphere around the origin.\n"
	"If sphereradius is set, the tempmodels will spawn at the radius distance from\n"
	"the origin"
);
Event EV_Client_SetInwardSphere
(
	"inwardsphere",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Create the tempmodels in a sphere around the origin, and adjust their\n"
	"angle so they point toward the center of the sphere.  This is best used with a\n"
	"spehere radius and some velocity so the models look like they're heading toward the\n"
	"center of the sphere."
);
Event EV_Client_SetRadius
(
	"radius",
	EV_DEFAULT,
	"f",
	"radius",
	"Set the radius of the sphere for the inwardsphere amd sphere settings"
);
Event EV_Client_SetSwarm
(
	"swarm",
	EV_DEFAULT,
	"iff",
	"frequency maxspeed delta",
	"Create a swarm like effect that the tempmodels follow when they are spawned\n"
	"frequency is how often they change direction\n"
	"maxspeed is how fast the tempmodel will move (it's randomly generated every\n"
	"time the frequency is hit)\n"
	"delta is how much the tempmodel moves toward the origin every frame"
);
Event EV_Client_SetAlign
(
	"align",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Align the tempmodels to the direction they are traveling"
);
Event EV_Client_SetAlignOnce
(
	"alignonce",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Align the tempmodels to the direction they are traveling at the time they are initialized"
);
Event EV_Client_SetFlickerAlpha
(
	"flicker",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodel to change it's alpha every frame.  Creates a flickering effect"
);
Event EV_Client_SetCollision
(
	"collision",
	EV_DEFAULT,
	"S",
	"water",
	"Turn on collision for the tempmodel.\n"
	"If the keyword water is specified, then the tempmodel will collide with water"
);
Event EV_Client_TagSpawn
(
	"tagspawn",
	EV_DEFAULT,
	"s",
	"tagName",
	"Spawn tempmodels from the specified tag.\n"
	"This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
Event EV_Client_TagSpawnLinked
(
	"tagspawnlinked",
	EV_DEFAULT,
	"s",
	"tagName",
	"Spawn tempmodels from the specified tag, linked to the entity at the tag.\n"
	"This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
Event EV_Client_OriginSpawn
(
	"originspawn",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Spawn tempmodels from the origin.\n"
	"This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
Event EV_Client_OriginBeamSpawn
(
	"originbeamspawn",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Spawn a beam from the origin.\n"
	"This command is followed by a ( to specify a block of commands that modify the beam"
);
Event EV_Client_OriginBeamEmitter
(
	"originbeamemitter",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Spawn beams from the origin.\n"
	"This command is followed by a ( to specify a block of commands that modify the beam"
);
Event EV_Client_TagEmitter
(
	"tagemitter",
	EV_DEFAULT,
	"ss",
	"tagName emitterName",
	"Create an emitter that spawns tempmodels from the specified tag.\n"
	"This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
Event EV_Client_OriginEmitter
(
	"originemitter",
	EV_DEFAULT,
	"s",
	"emitterName",
	"Create an emitter that spawns tempmodels from the origin.\n"
	"This command is followed by a ( to specify a block of commands that modify the tempmodels"
);
Event EV_Client_EmitterOn
(
	"emitteron",
	EV_DEFAULT,
	"s",
	"emitterName",
	"Turn the specified emitter on"
);
Event EV_Client_EmitterOff
(
	"emitteroff",
	EV_DEFAULT,
	"s",
	"emitterName",
	"Turn the specified emitter off"
);
Event EV_Client_Sound
(
	"sound",
	EV_DEFAULT,
	"sSFFFFS",
	"soundName channelName volume min_distance pitch randompitch randomvolume argstype",
	"Play the specified sound"
);
Event EV_Set_Current_Tiki
(
	"settiki",
	EV_DEFAULT,
	"s",
	"settiki tikiname - all subsequent events are applied to this tiki",
	"sets the tiki the aliases should be on in the sound uberfile"
);
Event EV_Client_StopSound
(
	"stopsound",
	EV_DEFAULT,
	"s",
	"channelName",
	"Stops the sound on the specified channel."
);
Event EV_Client_StopAliasChannel
(
	"stopaliaschannel",
	EV_DEFAULT,
	"s",
	"alias",
	"Stops the sound channel used by the specified alias."
);
Event EV_Client_LoopSound
(
	"loopsound",
	EV_DEFAULT,
	"sFFf",
	"soundName volume min_distance pitch",
	"Play the specifed sound as a looping sound"
);
Event EV_Client_Cache
(
	"cache",
	8,
	"s",
	"resourceName",
	"Cache the specified resource"
);
Event EV_Client_CacheImage
(
	"cacheimage",
	8,
	"s",
	"imageName",
	"Cache the specified image (For menu graphics that should never be picmiped)"
);
Event EV_Client_CacheFont
(
	"cachefont",
	8,
	"s",
	"fontName",
	"Cache the specified font (For menu graphics that should never be picmiped)"
);
Event EV_Client_AliasCache
(
	"aliascache",
	EV_DEFAULT,
	"ssSSSSSS",
	"alias realPath arg1 arg2 arg3 arg4 arg5 arg6",
	"Create an alias to the specified path and cache the resource"
);
Event EV_Client_Alias
(
	"alias",
	EV_DEFAULT,
	"ssSSSSSS",
	"alias realPath arg1 arg2 arg3 arg4 arg5 arg6",
	"Create an alias to the specified path"
);
Event EV_Client_CacheAlias
(
	"cachefromalias",
	EV_DEFAULT,
	"s",
	"alias",
	"Caches all data matching a previously specified alias"
);
Event EV_Client_Footstep
(
	"footstep",
	EV_DEFAULT,
	"ssI",
	"tag sRunning iEquipment",
	"Play a footstep sound that is appropriate to the surface we are currently stepping on\n"
	"sRunning should be set to run, walk, or ladder"
);
Event EV_Client_Landing
(
	"landing",
	EV_DEFAULT,
	"FI",
	"fVolume iEquipment",
	"Play a landing sound that is appropriate to the surface we are landing on\n"
);
Event EV_Client_BodyFall
(
	"bodyfall",
	EV_DEFAULT,
	"F",
	"fVolume",
	"Play a body fall sound that is appropriate to the surface we are falling on\n"
);
Event EV_Client_Client
(
	"client",
	EV_DEFAULT,
	"SSSSSS",
	"arg1 arg2 arg3 arg4 arg5 arg6",
	"Execute the specified command arg string"
);
Event EV_Client_OriginDynamicLight
(
	"origindlight",
	EV_DEFAULT,
	"fffffSS",
	"red green blue intensity life type1 type2",
	"Spawn a dynamic light from the origin of the model\n"
	"The red,green,blue parms are the color of the light\n"
	"The intensity is the radius of the light\n"
	"type is the type of light to create (lensflare,viewlensflare,additive)"
);
Event EV_Client_TagDynamicLight
(
	"tagdlight",
	EV_DEFAULT,
	"sfffffSS",
	"tagName red green blue intensity life intvel type1 type2",
	"Spawn a dynamic light from the specified tag\n"
	"The red,green,blue parms are the color of the light\n"
	"The intensity is the radius of the light\n"
	"type is the type of light to create (lensflare,viewlensflare,additive)"
);
Event EV_Client_DynamicLight
(
	"dlight",
	EV_DEFAULT,
	"ffffSS",
	"red green blue intensity type1 type2",
	"This makes the emitter itself a dynamic lightThe red,green,blue parms are the color of the light\n"
	"The intensity is the radius of the light\n"
	"type is the type of light to create (lensflare,viewlensflare,additive)"
);
Event EV_Client_BlockDynamicLight
(
	"blockdlight",
	EV_DEFAULT,
	"ffSS",
	"intensity life type1 type2",
	"Spawn a dynamic light from the origin of the model\n"
	"An additional commands block allows the specification of moving & varying dlights\n"
	"The intensity is the radius of the light\n"
	"type is the type of light to create (lensflare,viewlensflare,additive)"
);
Event EV_Client_SetEntityColor
(
	"entcolor",
	EV_DEFAULT,
	"fffF",
	"red green blue alpha",
	"Set the color(modulate) of this entity"
);
Event EV_Client_SetTexAnimTime
(
	"texanimtime",
	EV_DEFAULT,
	"f",
	"time",
	"Set the texture animation speed"
);
Event EV_Client_SetGlobalFade
(
	"globalfade",
	EV_DEFAULT,
	"S",
	"[in|out]",
	"Set the tempmodels to globally fade in or out together"
);
Event EV_Client_SetParentLink
(
	"parentlink",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodels linked to the parent, so they move with the parent model"
);
Event EV_Client_SetHardLink
(
	"hardlink",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodels linked to the model they are spawned from, so they move with it"
);
Event EV_Client_SetRandomRoll
(
	"randomroll",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodels so they pick a random roll value every frame"
);
Event EV_Client_ParentAngles
(
	"parentangles",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the tempmodels angles to that of its parent"
);
Event EV_Client_EmitterAngles
(
	"emitterangles",
	EV_DEFAULT,
	"FFF",
	"pitchofs yawofs rollofs",
	"Set the tempmodels angles to that of the emitter\n"
	"The three optional parameters are for setting an angle offset from the emitter"
);
Event EV_Client_SetAngles
(
	"angles",
	EV_DEFAULT,
	"SfFSfFSfF",
	"[random|crandom|range] pitch [pitch2] [random|crandom|range] yaw [yaw2] [random|crandom|range] roll [roll2]",
	"If random is specified, the component will range from 0 to +specified value.\n"
	"If crandom is specified, the component will range from -specified to +specified value.\n"
	"If range is specified, the component needs two values; it will randomly pick a number in the range\n"
	"from the first number to the first number plus the second number.\n"
	"If no keyword is explicitly specified, then the component will be just set\n"
	"without randomness."
);
Event EV_Client_Swipe
(
	"swipe",
	EV_DEFAULT,
	"V",
	"origin",
	"Do a swipe and add it on to the swipe rendering list."
);
Event EV_Client_SwipeOn
(
	"swipeon",
	EV_DEFAULT,
	"ssff",
	"shader startTagName endTagNamelife life",
	"Signal the start of a swipe from the current tag"
);
Event EV_Client_SwipeOff
(
	"swipeoff",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Signal the end of a swipe"
);
Event EV_Client_BeginTagBeamEmitter
(
	"tagbeamemitter",
	EV_DEFAULT,
	"sss",
	"tagstart tagend name",
	"Create a beam emitter that uses 2 tags to determine it's start and end position"
);
Event EV_Client_BeginTagBeamSpawn
(
	"tagbeamspawn",
	EV_DEFAULT,
	"sf",
	"tagstart name",
	"Create a beam emitter that uses the tag to determine it's starting position."
);
Event EV_Client_AnimateOnce
(
	"animateonce",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set a tempmodel to animate once and then get removed"
);
Event EV_Client_SetAnim
(
	"anim",
	EV_DEFAULT,
	"s",
	"animation",
	"Set a tempmodel the the specified animation"
);
Event EV_Client_SetSubdivisions
(
	"numsegments",
	EV_DEFAULT,
	"i",
	"numsegments",
	"Set the number of segments in a beam"
);
Event EV_Client_SetMinOffset
(
	"minoffset",
	EV_DEFAULT,
	"f",
	"minoffset",
	"Set the minimum offset from center in a beam"
);
Event EV_Client_SetMaxOffset
(
	"maxoffset",
	EV_DEFAULT,
	"f",
	"maxoffset",
	"Set the maximum offset from center in a beam"
);
Event EV_Client_SetBeamShader
(
	"beamshader",
	EV_DEFAULT,
	"s",
	"shadername",
	"Set the shader to use for the beam"
);
Event EV_Client_SetBeamLength
(
	"beamlength",
	EV_DEFAULT,
	"f",
	"length",
	"Set the length of the beam or trace length (for decals)"
);
Event EV_Client_SetBeamDelay
(
	"beamdelay",
	EV_DEFAULT,
	"SF",
	"[random] delay",
	"Set the delay time between creating new beams.\n"
	"If the keyword random is specified, the delay between beams will occur randomly between 0 and the time specified"
);
Event EV_Client_SetBeamToggleDelay
(
	"beamtoggledelay",
	EV_DEFAULT,
	"SF",
	"[random] delay",
	"Set a delay between toggling the beams on and off.\n"
	"If the keyword random is specified, the delay between toggling will occur randomly between 0 and the time specified"
);
Event EV_Client_SetBeamPersist
(
	"beampersist",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Make the beams persist instead of blinking out"
);
Event EV_Client_SetBeamOffsetEndpoints
(
	"beam_offset_endpoints",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Make the beams endpoints offset to reduce the bunching up effect"
);
Event EV_Client_BeamSphere
(
	"beamsphere",
	EV_DEFAULT,
	"i",
	"count",
	"Create a sphere shaped beam effect from the origin. Count is the number of beams"
);
Event EV_Client_Spread
(
	"spread",
	EV_DEFAULT,
	"ff",
	"spreadx spready",
	"Add a random variance in the spawned beam in the forward direction by the amound specified in spreadx and spready"
);
Event EV_Client_UseLastTraceEnd
(
	"uselasttraceend",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Makes this trace command use the end results of the last trace command"
);
Event EV_Client_OffsetAlongAxis
(
	"offsetalongaxis",
	EV_DEFAULT,
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
Event EV_Client_Cone
(
	"cone",
	EV_DEFAULT,
	"ff",
	"height radius",
	"Randomly spawns the particle somewhere inside a cone along the model's local x axis"
);
Event EV_Client_SetEndAlpha
(
	"endalpha",
	EV_DEFAULT,
	"f",
	"alpha",
	"Set the alpha of the beam's endpoint"
);
Event EV_Client_RandomChance
(
	"randomchance",
	EV_DEFAULT,
	"fSSSSSS",
	"percentage [arg1] [arg2] [arg3] [arg4] [arg5] [arg6]",
	"Set the percentage chance that command will occur"
);
Event EV_Client_DelayedRepeat
(
	"delayedrepeat",
	EV_DEFAULT,
	"fiSSSSSS",
	"time commandnumber [arg1] [arg2] [arg3] [arg4] [arg5] [arg6]",
	"Set the time delay between this command getting executed.  This requires a command number to be assigned here.\n"
	"This is internally used to keep track of the commands this entity executes and will resolve naming conflicts.\n"
	"This is only useful if the command gets called continuously but must execute at regular intervals.\n"
);
Event EV_Client_CommandDelay
(
	"commanddelay",
	EV_DEFAULT,
	"fsSSSSSS",
	"delay command [arg1] [arg2] [arg3] [arg4] [arg5] [arg6]",
	"executes a command after the given delay"
);
Event EV_Client_BounceDecal
(
	"bouncedecal",
	EV_DEFAULT,
	"iI",
	"maxamount temporary",
	"Put a mark when the tempmodel bounces and hits a surface\n"
	"maxamount = Max amount of decals to make when bouncing\n"
	"temporary = specify 1 for a temporary mark that only appears for a short time, 0 for a decal that stays aroung l"
	"onger (default is 0)\n"
);
Event EV_Client_SetDecalRadius
(
	"decalradius",
	EV_DEFAULT,
	"f",
	"radius",
	"Set the radius of the decal"
);
Event EV_Client_SetDecalOrientation
(
	"orientation",
	EV_DEFAULT,
	"f",
	"degrees",
	"Set the degrees to orient the decal.  Specify 'random' to use a random orientation"
);
Event EV_Client_SetDecalShader
(
	"decalshader",
	EV_DEFAULT,
	"s",
	"shadername",
	"Set the shader to use for the impact decal"
);
Event EV_Client_SetTraceLength
(
	"tracelength",
	EV_DEFAULT,
	"f",
	"length",
	"Set the length of the trace for the decal"
);
Event EV_Client_TraceSpawn
(
	"tracespawn",
	EV_DEFAULT,
	"s",
	"modelname",
	"Spawn the specified model when the trace hits a solid object"
);
Event EV_Client_TagList
(
	"taglist",
	EV_DEFAULT,
	"ssSSSSSS",
	"arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8",
	"Set the tag list to create a beam that travels between all the tags"
);
Event EV_Client_Lightstyle
(
	"lightstyle",
	EV_DEFAULT,
	"s",
	"nameOfImage",
	"Set a lightstyle to determine the color of this tempmodel, the image\n"
	"specified is used to determine the look of the light style"
);
Event EV_Client_PhysicsRate
(
	"physicsrate",
	EV_DEFAULT,
	"s",
	"rate",
	"Set the rate (in updates per second) for the tempmodel's physics to be updated"
);
Event EV_Client_Parallel
(
	"parallel",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set tempmodel to be parallel to the viewer"
);
Event EV_Client_AlwaysDraw
(
	"alwaysdraw",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set emitter/tempmodel to be alwaysdraw. Which can be turned off by alwaysdraw"
);
Event EV_Client_Detail
(
	"detail",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set emitter/tempmodel to be detail. Which can be turned off by detail"
);
Event EV_Client_SetVolumetric
(
	"volumetric",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Set the effect to spawn volumetric sources rather than tempmodels"
);
Event EV_Client_Wind
(
	"wind",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Makes the temp model be affected by wind"
);
Event EV_Client_SpriteGridLighting
(
	"spritegridlighting",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"Calculates grid lighting for a sprite"
);
Event EV_Client_WaterOnly
(
	"wateronly",
	EV_DEFAULT,
	nullptr,
	nullptr,
	"makes the temp model remove itself if it leaves water"
);
Event EV_Client_SetAlignStretch
(
	"alignstretch",
	EV_DEFAULT,
	"F",
	"scaleMultiplier",
	"Aligns the temp model to the direction of travel, and stretches it betwen the last and current positions.\n"
);
Event EV_Client_SetClampVelocity
(
	"clampvel",
	EV_DEFAULT,
	"ffffff",
	"minX maxX minY maxY minZ maxZ",
	"sets the valid range for velocities along global axes.  Cannot be used with clampvelaxis."
);
Event EV_Client_SetClampVelocityAxis
(
	"clampvelaxis",
	EV_DEFAULT,
	"ffffff",
	"minX maxX minY maxY minZ maxZ",
	"sets the valid range for velocities along oriented axes.  Cannot be used with clampvel."
);
Event EV_Client_Treads
(
	"treads",
	EV_DEFAULT,
	"ssi",
	"tagName shader localrefnumber",
	"Spawn treads from the specified tag using the specified tread type."
);
Event EV_Client_Treads_Off
(
	"treadsoff",
	EV_DEFAULT,
	"si",
	"tagName localrefnumber",
	"Stops spawning treads from the specified tag."
);
Event EV_Client_ViewKick
(
	"viewkick",
	EV_DEFAULT,
	"fffffsff",
	"pitchmin pitchmax yawmin yawmax recenterspeed patters pitchmax yawmax scatterpitchmax",
	"Adds kick to the view of the owner when fired."
);
Event EV_Client_EyeLimits
(
	"eyelimits",
	EV_DEFAULT,
	"fff",
	"fPitchmax fYawmax fRollmax",
	"Set the max angle offsets for the eyes from the model's head"
);
Event EV_Client_EyeMovement
(
	"eyemovement",
	EV_DEFAULT,
	"f",
	"fMovement",
	"Sets the amount of the head's movement to apply to the players view. 1 is full, 0 is none"
);
Event EV_Client_SFXStart
(
	"sfx",
	EV_DEFAULT,
	"sSSSSSSSS",
	"sCommand arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8",
	"Used for adding commands to a special effect"
);
Event EV_Client_SFXStartDelayed
(
	"delayedsfx",
	EV_DEFAULT,
	"fsSSSSSSSS",
	"fDelay sCommand arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8",
	"Used for adding commands to a special effect with a time delay"
);

CLASS_DECLARATION(Listener, EmitterListener, "")
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

void EmitterListener::BeginOriginSpawn(Event* ev)
{
	emitter->bIsEmitter = false;
	emitter->emitterName = ev->GetString(1);
}

void EmitterListener::BeginOriginEmitter(Event* ev)
{
	emitter->bIsEmitter = true;
	emitter->emitterName = ev->GetString(1);
}

void EmitterListener::StartBlock(Event* ev)
{
	bInBlock = true;
}

void EmitterListener::EndBlock(Event* ev)
{
	bInBlock = false;
	bProcessed = true;
}

void EmitterListener::StartSFX(Event* ev)
{
	StartSFXInternal(ev, false);
}

void EmitterListener::StartSFXDelayed(Event* ev)
{
	StartSFXInternal(ev, true);
}

void EmitterListener::StartSFXInternal(Event* ev, bool bDelayed)
{
	emitter->startTime = bDelayed ? ev->GetFloat(1) : 0.f;
	str commandName = ev->GetString(bDelayed + 1);

	Event* newEvent = GetEventManager()->NewEvent(commandName);
	for (int32_t i = bDelayed + 2; i < ev->NumArgs(); i++)
	{
		newEvent->AddToken(ev->GetToken(i));
	}

	ProcessEvent(newEvent);
}

void EmitterListener::SetBaseAndAmplitude(Event* ev, Vector& Base, Vector& Amplitude)
{
	int32_t i = 1;

	for (int32_t j = 0; j < 3; j++)
	{
		const str org = ev->GetString(i++);
		if (!str::cmp(org, "crandom"))
		{
			const float ampl = ev->GetFloat(i++);
			Amplitude[j] = ampl + ampl;
			Base[j] = -ampl;
		}
		else if (!str::cmp(org, "random"))
		{
			Base[j] = 0.f;
			Amplitude[j] = ev->GetFloat(i++);
		}
		else if (!str::cmp(org, "range"))
		{
			Base[j] = ev->GetFloat(i++);
			Amplitude[j] = ev->GetFloat(i++);
		}
		else
		{
			Base[j] = (float)atof(org);
			Amplitude[j] = 0.f;
		}
	}
}

void EmitterListener::SetSpawnRate(Event* ev)
{
	emitter->spawnRate = ev->GetFloat(1);
}

void EmitterListener::SetScaleRate(Event* ev)
{
	emitter->scaleRate = ev->GetFloat(1);
}

void EmitterListener::SetCount(Event* ev)
{
	emitter->count = ev->GetInteger(1);
}

void EmitterListener::SetScale(Event* ev)
{
	emitter->scale = ev->GetFloat(1);
}

void EmitterListener::SetScaleUpDown(Event* ev)
{
	emitter->flags |= Emitter::EF_UpDownScale;
}

void EmitterListener::SetScaleMin(Event* ev)
{
	emitter->scaleMin = ev->GetFloat(1);
}

void EmitterListener::SetScaleMax(Event* ev)
{
	emitter->scaleMax = ev->GetFloat(1);
}

void EmitterListener::SetModel(Event* ev)
{
	str resourceName = ev->GetString(1);
	if (resourceName.length() > 0)
	{
		const char* extension = resourceName.GetExtension();
		
		if (!str::icmp(extension, "spr"))
		{
			resourceName.StripExtension();
			emitter->sprite.spriteType = Sprite::ST_Shader;
			ShaderRef shader = GetManager<ShaderManager>()->GetShader(resourceName);
			if (shader)
			{
				emitter->sprite.Shader = new ShaderRef(std::move(shader));
			}
		}
		else if (!str::icmp(extension, "tik"))
		{
			emitter->sprite.spriteType = Sprite::ST_Tiki;
			auto Tiki = GetAssetManager()->LoadAsset<TIKI>(resourceName);
			if (Tiki)
			{
				emitter->sprite.Tiki = new std::shared_ptr<TIKI>(std::move(Tiki));
			}
		}
	}
}

void EmitterListener::SetOffset(Event* ev)
{
	SetBaseAndAmplitude(ev, emitter->originOffsetBase, emitter->originOffsetAmplitude);
}

void EmitterListener::SetOffsetAlongAxis(Event* ev)
{
	emitter->flags |= Emitter::EF_UseParentAxis;
	SetBaseAndAmplitude(ev, emitter->originOffsetBase, emitter->originOffsetAmplitude);
}

void EmitterListener::SetLife(Event* ev)
{
	emitter->life = ev->GetFloat(1);

	if (ev->NumArgs() > 1)
	{
		emitter->randomLife = ev->GetFloat(2);
	}
}

void EmitterListener::SetColor(Event* ev)
{
	emitter->color[0] = ev->GetFloat(1);
	emitter->color[1] = ev->GetFloat(2);
	emitter->color[2] = ev->GetFloat(3);

	if (ev->NumArgs() > 3)
	{
		emitter->alpha = ev->GetFloat(4);
	}
}

void EmitterListener::SetAlpha(Event* ev)
{
	emitter->alpha = ev->GetFloat(1);
}

void EmitterListener::SetAngles(Event* ev)
{
	SetBaseAndAmplitude(ev, emitter->anglesOffsetBase, emitter->anglesOffsetAmplitude);
}

void EmitterListener::SetRadialVelocity(Event* ev)
{
	emitter->radialVelocity[0] = ev->GetFloat(1);
	emitter->radialVelocity[1] = ev->GetFloat(2);
	emitter->radialVelocity[2] = ev->GetFloat(3);
}

void EmitterListener::SetVelocity(Event* ev)
{
	emitter->forwardVelocity = ev->GetFloat(1);
}

void EmitterListener::SetAngularVelocity(Event* ev)
{
	SetBaseAndAmplitude(ev, emitter->randAVelocityBase, emitter->randAVelocityAmplitude);
}

void EmitterListener::SetRandomVelocity(Event* ev)
{
	SetBaseAndAmplitude(ev, emitter->randVelocityBase, emitter->randVelocityAmplitude);
}

void EmitterListener::SetRandomVelocityAlongAxis(Event* ev)
{
	emitter->flags |= Emitter::EF_UseParentAxis;
	SetRandomVelocity(ev);
}

void EmitterListener::SetAccel(Event* ev)
{
	emitter->accel[0] = ev->GetFloat(1);
	emitter->accel[1] = ev->GetFloat(2);
	emitter->accel[2] = ev->GetFloat(3);
}

void EmitterListener::SetCone(Event* ev)
{
	emitter->spawnType = Emitter::EST_Cone;
	emitter->coneHeight = ev->GetFloat(1);
	emitter->sphereRadius = ev->GetFloat(2);
}

void EmitterListener::SetCircle(Event* ev)
{
	emitter->spawnType = Emitter::EST_Circle;
}

void EmitterListener::SetSphere(Event* ev)
{
	emitter->spawnType = Emitter::EST_Sphere;
}

void EmitterListener::SetInwardSphere(Event* ev)
{
	emitter->spawnType = Emitter::EST_InwardSphere;
}

void EmitterListener::SetRadius(Event* ev)
{
	emitter->sphereRadius = ev->GetFloat(1);
}

void EmitterListener::SetFade(Event* ev)
{
	emitter->flags |= Emitter::EF_Fade;
}

void EmitterListener::SetFadeDelay(Event* ev)
{
	emitter->flags |= Emitter::EF_Fade;
	emitter->fadeDelay = ev->GetFloat(1);
}

void EmitterListener::SetFadeIn(Event* ev)
{
	emitter->flags |= Emitter::EF_FadeIn;
	emitter->fadeInTime = ev->GetFloat(1);
}
