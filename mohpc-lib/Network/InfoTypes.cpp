#include <MOHPC/Network/InfoTypes.h>

MOHPC::usercmd_t::usercmd_t()
	: serverTime(0)
	, msec(0)
	, buttons(0)
	, angles{ 0 }
	, forwardmove(0)
	, rightmove(0)
	, upmove(0)
{

}

MOHPC::usereyes_t::usereyes_t()
	: angles{ 0 }
	, ofs{ 0 }
{

}

MOHPC::playerState_t::playerState_t()
	: commandTime(0)
	, pm_type(0)
	, bobCycle(0)
	, pm_flags(0)
	, pm_time(0)
	, gravity(0)
	, speed(0)
	, delta_angles{ 0 }
	, groundEntityNum(0)
	, walking(false)
	, groundPlane(false)
	, feetfalling(0)
	, radarInfo(0)
	, clientNum(0)
	, viewheight(0)
	, fLeanAngle(0.f)
	, iViewModelAnim(0)
	, iViewModelAnimChanged(0)
	, bVoted(false)
	, stats{ 0 }
	, activeItems{ 0 }
	, ammo_name_index{ 0 }
	, ammo_amount{ 0 }
	, max_ammo_amount{ 0 }
	, current_music_mood(0)
	, fallback_music_mood(0)
	, music_volume(0.f)
	, music_volume_fade_time(0.f)
	, reverb_type(0)
	, reverb_level(0)
	, blend{ 0 }
	, fov(0.f)
	, camera_time(0.f)
	, camera_flags(0)
{
}

MOHPC::trajectory_t::trajectory_t()
	: trTime(0)
{}

MOHPC::frameInfo_t::frameInfo_t()
	: index(0)
	, time(0.f)
	, weight(0.f)
{}

MOHPC::entityState_t::entityState_t()
	: number(0)
	, eType(0)
	, eFlags(0)
	, constantLight(0)
	, loopSound(0)
	, loopSoundVolume(0.f)
	, loopSoundMinDist(0.f)
	, loopSoundMaxDist(0.f)
	, loopSoundPitch(0.f)
	, loopSoundFlags(0)
	, parent(0)
	, tag_num(0)
	, attach_use_angles(false)
	, beam_entnum(0)
	, modelindex(0)
	, usageIndex(0)
	, skinNum(0)
	, wasframe(0)
	, actionWeight(0.f)
	, bone_tag{ 0 }
	, bone_quat{ 0.f }
	, surfaces{ 0 }
	, clientNum(0)
	, groundEntityNum(0)
	, solid(0)
	, scale(0.f)
	, alpha(0.f)
	, renderfx(0)
	, shader_data{ 0.f }
	, shader_time(0.f)
	, quat{ 0.f }

{
}
