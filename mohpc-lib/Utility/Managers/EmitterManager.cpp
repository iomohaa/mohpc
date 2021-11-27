#include <Shared.h>
#include <MOHPC/Utility/Managers/EmitterManager.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>
#include <MOHPC/Assets/Formats/TIKI.h>
#include "../Misc/EmitterListener.h"

#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/EventContext.h>

using namespace MOHPC;

MOHPC_OBJECT_DEFINITION(EmitterManager);
EmitterManager::EmitterManager()
{
	// initialize the event system
	mfuse::EventSystem::Get();
}

Sprite::Sprite()
{
	spriteType = ST_None;
	pointerValue = nullptr;
}

MOHPC::Sprite::Sprite(const Sprite& sprite)
{
	*this = sprite;
}

MOHPC::Sprite& MOHPC::Sprite::operator=(const Sprite& sprite)
{
	spriteType = sprite.spriteType;
	pointerValue = sprite.pointerValue;

	if (pointerValue)
	{
		if (spriteType == ST_Tiki)
		{
			Tiki = new SharedPtr<class TIKI>(*sprite.Tiki);
		}
		else if (spriteType == ST_Shader)
		{
			Shader = new ShaderPtr(*sprite.Shader);
		}
	}

	return *this;
}

MOHPC::Sprite::Sprite(Sprite&& sprite)
{
	spriteType = sprite.spriteType;
	pointerValue = sprite.pointerValue;
	sprite.spriteType = ST_None;
	sprite.pointerValue = nullptr;
}

Sprite::~Sprite()
{
	if (pointerValue)
	{
		if (spriteType == ST_Tiki)
		{
			delete Tiki;
		}
		else if (spriteType == ST_Shader)
		{
			delete Shader;
		}
		pointerValue = nullptr;
	}
}

Emitter::Emitter()
	: bIsEmitter(false)
	, flags((emitterFlags_e)0)
	, spawnType(EST_None)
	, accel{ 0.f }
	, radialVelocity{ 0.f }
	, originOffsetBase{ 0.f }
	, originOffsetAmplitude{ 0.f }
	, anglesOffsetBase{ 0.f }
	, anglesOffsetAmplitude{ 0.f }
	, randVelocityBase{ 0.f }
	, randVelocityAmplitude{ 0.f }
	, randAVelocityBase{ 0.f }
	, randAVelocityAmplitude{ 0.f }
	, count(1)
	, life(0.f)
	, randomLife(0.f)
	, color{ 1.f }
	, alpha(1.f)
	, scaleRate(0.f)
	, scaleMin(0.f)
	, scaleMax(0.f)
	, scale(1.f)
	, fadeInTime(0.f)
	, fadeDelay(0.f)
	, startTime(0.f)
	, sphereRadius(0.f)
	, coneHeight(0.f)
	, spawnRate(0.f)
	, forwardVelocity(0.f)
	, dlight{ 0.f }
	, lightRadius(0.f)
{
}

const char* MOHPC::Emitter::GetEmitterName() const
{
	return emitterName.c_str();
}

EmitterResults::EmitterResults()
{
	animNum = -1;
}

size_t MOHPC::EmitterResults::GetNumEmitters() const
{
	return Emitters.size();
}

const MOHPC::Emitter* MOHPC::EmitterResults::GetEmitterAt(size_t Index) const
{
	return Index < Emitters.size() ? &Emitters[Index] : nullptr;
}

const char* MOHPC::EmitterResults::GetAnimName() const
{
	return animName.c_str();
}

bool EmitterManager::ParseEmitters(const TIKI* Tiki, EmitterResults& Results)
{
	mfuse::EventContext context;

	size_t cursor = 0;

	uint32_t animNum = Results.animNum;
	Results = EmitterResults();

	bool bProcessedCommands = true;
	bool bAnimHasCommands = false;
	while(bProcessedCommands || !bAnimHasCommands)
	{
		Emitter Emitter;

		EmitterListener listener(&Emitter);
		listener.SetAssetManager(GetAssetManager());

		bProcessedCommands = false;

		if (animNum == -1)
		{
			// Parse init commands

			const size_t numCmds = Tiki->GetNumClientInitCommands();
			for (; cursor < numCmds; cursor++)
			{
				const TIKICommand* cmd = Tiki->GetClientInitCommand(cursor);

				if(ProcessCommand(cmd->args, cmd->num_args, listener))
				{
					bProcessedCommands = true;
					bAnimHasCommands = true;
					cursor++;
					break;
				}
			}

			if (cursor == numCmds)
			{
				Results.animName = "";
			}
		}
		else if (animNum < Tiki->GetNumAnimations())
		{
			const TIKIAnimDef* animDef = Tiki->GetAnimDef(animNum);

			// Parse animation commands
			const size_t numCmds = animDef->num_client_cmds;
			for (; cursor < numCmds; cursor++)
			{
				const TIKICommand* cmd = &animDef->client_cmds[cursor];

				if (cmd->num_args)
				{
					if (ProcessCommand(cmd->args, cmd->num_args, listener))
					{
						bProcessedCommands = true;
						bAnimHasCommands = true;
						cursor++;
						break;
					}
				}
			}

			if (cursor == numCmds)
			{
				Results.animName = animDef->alias;
			}
		}
		else
		{
			break;
		}

		if (bProcessedCommands)
		{
			Results.Emitters.push_back(std::move(Emitter));
		}
		else if (!bAnimHasCommands)
		{
			animNum++;
		}
	}

	cursor = 0;
	Results.animNum = animNum + 1;

	return bAnimHasCommands;
}

bool EmitterManager::ProcessCommand(const char* const* Arguments, size_t numArgs, EmitterListener& Listener)
{
	mfuse::Event ev(mfuse::EventSystem::Get().FindNormalEventNum(Arguments[0]));

	for (size_t j = 1; j < numArgs; j++)
	{
		ev.AddString(Arguments[j]);
	}

	Listener.ProcessEvent(ev);

	if (Listener.FinishedParsing())
	{
		return true;
	}

	return false;
}
