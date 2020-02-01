#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/EventManager.h>
#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Managers/GameManager.h>
#include <MOHPC/Managers/ScriptManager.h>
#include <MOHPC/Script/ConstStr.h>
#include <MOHPC/Script/Parm.h>
#include <MOHPC/Script/Listener.h>
#include <MOHPC/Script/Level.h>
#include <MOHPC/Script/ScriptThread.h>
#include <MOHPC/Script/ScriptContainer.h>
#include <MOHPC/Script/ScriptException.h>
#include <MOHPC/Script/GameScript.h>

using namespace MOHPC;

CLASS_DEFINITION(ScriptManager);
ScriptManager::ScriptManager()
{

}

void ScriptManager::Init()
{
	parm.InitAssetManager(GetAssetManager());
	InitConstStrings();
}

void ScriptManager::InitConstStrings()
{
	EventDef *eventDef;
	const_str name;
	uintptr_t eventnum;
	con_map_enum<Event *, EventDef> en;

	/*
	for (int i = 0; i < sizeof(ConstStrings) / sizeof(ConstStrings[0]); i++)
	{
		AddString(ConstStrings[i]);
	}
	*/

	EventManager* eventManager = GetManager<EventManager>();

	eventManager->normalCommandList.clear();
	eventManager->returnCommandList.clear();
	eventManager->getterCommandList.clear();
	eventManager->setterCommandList.clear();

	en = eventManager->eventDefList;

	for (en.NextValue(); en.CurrentValue() != NULL; en.NextValue())
	{
		eventDef = en.CurrentValue();
		eventnum = (*en.CurrentKey())->eventnum;
		str command = eventDef->command.c_str();
		command.tolower();

		name = AddString(command);

		if (eventDef->type == EV_NORMAL)
		{
			eventManager->normalCommandList[name] = eventnum;
		}
		else if (eventDef->type == EV_RETURN)
		{
			eventManager->returnCommandList[name] = eventnum;
		}
		else if (eventDef->type == EV_GETTER)
		{
			eventManager->getterCommandList[name] = eventnum;
		}
		else if (eventDef->type == EV_SETTER)
		{
			eventManager->setterCommandList[name] = eventnum;
		}
	}
}

const_str ScriptManager::AddString(const char *s)
{
	return (const_str)StringDict.addKeyIndex(s);
}

const_str ScriptManager::AddString(const str& s)
{
	return (const_str)StringDict.addKeyIndex(s);
}

const_str ScriptManager::GetString(const char *s)
{
	const_str cs = (const_str)StringDict.findKeyIndex(s);
	return cs ? cs : STRING_EMPTY;
}

const_str ScriptManager::GetString(const str& s)
{
	return GetString(s.c_str());
}

const str& ScriptManager::GetString(const_str s)
{
	return StringDict[s];
}

void ScriptManager::AddTiming(ScriptThread* Thread, float Time)
{
	timerList.AddElement(Thread, GetManager<GameManager>()->GetLevel()->GetTimeSeconds() + Time + 0.0005f);
}

void ScriptManager::RemoveTiming(ScriptThread* Thread)
{
	timerList.RemoveElement(Thread);
}

uintptr_t ScriptManager::GetStackCount() const
{
	return stackCount;
}

void ScriptManager::AddStack()
{
	stackCount++;
}

void ScriptManager::RemoveStack()
{
	stackCount--;
	assert(stackCount >= 0);
}

void ScriptManager::AddContextSwitch(ScriptThread *Thread)
{

}

bool ScriptManager::IsContextSwitchAllowed() const
{
	return false;
}


bool ScriptManager::HasLoopDrop() const
{
	return false;
}

bool ScriptManager::HasLoopProtection() const
{
	return true;
}

ScriptContainer* ScriptManager::GetHeadContainer() const
{
	return ContainerHead;
}

ScriptThread* ScriptManager::CreateScriptThread(GameScript *scr, Listener *self, const_str label)
{
	ScriptContainer* scriptContainer = new ScriptContainer(scr, self);

	return CreateScriptThread(scriptContainer, label);
}

ScriptThread* ScriptManager::CreateScriptThread(GameScript *scr, Listener *self, const str& label)
{
	return CreateScriptThread(scr, self, AddString(label));
}

ScriptThread* ScriptManager::CreateScriptThread(ScriptContainer* scriptContainer, const_str label)
{
	unsigned char *m_pCodePos = scriptContainer->FindLabel(label);

	if (!m_pCodePos)
	{
		ScriptError("ScriptMaster::CreateScriptThread: label '%s' does not exist in '%s'.", GetString(label).c_str(), scriptContainer->Filename().c_str());
	}

	return CreateScriptThread(scriptContainer, m_pCodePos);
}

ScriptThread* ScriptManager::CreateScriptThread(ScriptContainer* scriptContainer, const str& label)
{
	if (label.length() && *label)
	{
		return CreateScriptThread(scriptContainer, AddString(label));
	}
	else
	{
		return CreateScriptThread(scriptContainer, STRING_EMPTY);
	}
}

ScriptThread* ScriptManager::CreateScriptThread(ScriptContainer* scriptContainer, unsigned char *m_pCodePos)
{
	return new ScriptThread(scriptContainer, m_pCodePos);
}

ScriptThread* ScriptManager::CreateThread(GameScript *scr, const str& label, Listener *self)
{
	try
	{
		return CreateScriptThread(scr, self, label);
	}
	catch (ScriptException&)
	{
		//glbs.DPrintf("ScriptMaster::CreateThread: %s\n", exc.string.c_str());
		return NULL;
	}
}

ScriptThread* ScriptManager::CreateThread(const str& filename, const str& label, Listener *self)
{
	GameScript *scr = GetScript(filename);

	if (!scr)
	{
		return NULL;
	}

	return CreateThread(scr, label, self);
}

ScriptContainer* ScriptManager::CurrentScriptContainer()
{
	return CurrentThread()->GetScriptContainer();
}

ScriptThread* ScriptManager::CurrentThread()
{
	return m_CurrentThread;
}

ScriptThread* ScriptManager::PreviousThread()
{
	return m_PreviousThread;
}

void ScriptManager::ExecuteThread(GameScript *scr, const str& label)
{
	ScriptThread *Thread = CreateThread(scr, label);

	try
	{
		if (Thread)
		{
			Thread->Execute();
		}
	}
	catch (ScriptException&)
	{
		//glbs.DPrintf("ScriptMaster::ExecuteThread: %s\n", exc.string.c_str());
	}
}

void ScriptManager::ExecuteThread(const str& filename, const str& label)
{
	GameScript *scr = GetScript(filename);

	if (!scr)
	{
		return;
	}

	ExecuteThread(scr, label);
}

void ScriptManager::ExecuteThread(GameScript *scr, const str& label, Event &parms)
{
	ScriptThread *Thread = CreateThread(scr, label);

	try
	{
		Thread->Execute(parms);
	}
	catch (ScriptException&)
	{
		//glbs.DPrintf("ScriptMaster::ExecuteThread: %s\n", exc.string.c_str());
	}
}

void ScriptManager::ExecuteThread(const str& filename, const str& label, Event &parms)
{
	GameScript *scr = GetScript(filename);

	if (!scr)
	{
		return;
	}

	ExecuteThread(scr, label, parms);
}

GameScript* ScriptManager::GetTempScript(const char *data)
{
	GameScript *scr = new GameScript;

	scr->Load((void *)data, strlen(data));

	if (!scr->successCompile)
	{
		return NULL;
	}

	return scr;
}

GameScript* ScriptManager::GetGameScriptInternal(const str& filename)
{
	const_str constString = AddString(filename);

	GameScript* scr = m_GameScripts[constString];

	if (scr != NULL)
	{
		return scr;
	}

	scr = new GameScript(filename);

	m_GameScripts[constString] = scr;

	/*
	if (GetCompiledScript(scr))
	{
		scr->m_Filename = AddString(filename);
		return scr;
	}
	*/

	FilePtr File = GetFileManager()->OpenFile(filename);
	if (File)
	{
		void* sourceBuffer;
		std::streamsize sourceLength = File->ReadBuffer(&sourceBuffer);

		scr->Load(sourceBuffer, sourceLength);

		if (!scr->successCompile)
		{
			ScriptError("Script '%s' was not properly loaded", filename.c_str());
		}
	}
	else
	{
		ScriptError("Can't find '%s'\n", filename.c_str());
	}

	return scr;
}

GameScript* ScriptManager::GetGameScript(const str& filename, bool recompile)
{
	const_str s = (const_str)StringDict.findKeyIndex(filename);
	GameScript *scr = m_GameScripts[s];

	if (scr != NULL && !recompile)
	{
		if (!scr->successCompile)
		{
			ScriptError("Script '%s' was not properly loaded\n", filename.c_str());
		}

		return scr;
	}
	else
	{
		if (scr && recompile)
		{
			Container< ScriptContainer * > list;
			ScriptContainer *scriptClass;
			m_GameScripts[s] = NULL;

			for (scriptClass = GetHeadContainer(); scriptClass != nullptr; scriptClass = scriptClass->Next)
			{
				if (scriptClass->GetScript() == scr)
				{
					list.AddObject(scriptClass);
				}
			}

			AssetManager* AM = GetAssetManager();

			for (uintptr_t i = 1; i <= list.NumObjects(); i++)
			{
				delete list.ObjectAt(i);
			}

			delete scr;
		}

		return GetGameScriptInternal(filename);
	}
}

GameScript* ScriptManager::GetGameScript(const_str filename, bool recompile)
{
	return GetGameScript(GetString(filename), recompile);
}

GameScript* ScriptManager::GetScript(const str& filename, bool recompile)
{
	try
	{
		return GetGameScript(filename, recompile);
	}
	catch (ScriptException&)
	{
		//glbs.Printf("ScriptMaster::GetScript: %s\n", exc.string.c_str());
	}

	return NULL;
}

GameScript* ScriptManager::GetScript(const_str filename, bool recompile)
{
	try
	{
		return GetGameScript(filename, recompile);
	}
	catch (ScriptException&)
	{
		//glbs.Printf("ScriptMaster::GetScript: %s\n", exc.string.c_str());
	}

	return NULL;
}

void ScriptManager::CloseGameScript()
{
}

void ScriptManager::Reset(bool samemap)
{
}

void ScriptManager::ExecuteRunning()
{

}

void ScriptManager::SetTime(float time)
{

}

Parm* ScriptManager::GetParm()
{
	return &parm;
}

