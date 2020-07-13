#pragma once

#include "../Global.h"
#include "Manager.h"
#include "../Script/ScriptClass.h"
#include "../Script/ScriptThread.h"
#include "../Script/con_set.h"
#include "../Script/con_arrayset.h"
#include "../Script/str.h"
#include "../Script/con_timer.h"
#include "../Script/Parm.h"

namespace MOHPC
{
	class Listener;
	class ScriptContainer;
	class ScriptThread;
	class GameScript;
	class FlagList;

	class ScriptManager : public Manager
	{
		friend class ScriptContainer;
		friend class ScriptThread;
		friend class Flag;

		CLASS_BODY(ScriptManager);

	private:
		/** VM recursions */
		int stackCount;

		/** The previous thread. */
		SafePtr<ScriptThread> m_PreviousThread;

		/** The current playing thread. */
		SafePtr<ScriptThread> m_CurrentThread;

		/** The list of compiled game scripts. */
		con_map<const_str, GameScript *> m_GameScripts;

		/** Threads that are currently waiting */
		con_timer timerList;

		/** The string dictionary, used to cache strings into a number. */
		con_arrayset<str, str> StringDict;

		/** The head of the script container. */
		ScriptContainer* ContainerHead;

		/** Parm instance for scripts. */
		Parm parm;

	public:
		FlagList flags;

	private:
		GameScript* GetGameScriptInternal(const str& filename);
		void InitConstStrings();

	public:
		MOHPC_EXPORTS ScriptManager();

		MOHPC_EXPORTS const_str AddString(const char* s);
		MOHPC_EXPORTS const_str AddString(const str& s);
		MOHPC_EXPORTS const_str GetString(const char* s);
		MOHPC_EXPORTS const_str GetString(const str& s);

		MOHPC_EXPORTS const str& GetString(const_str s);

		MOHPC_EXPORTS void AddTiming(ScriptThread* Thread, float Time);
		MOHPC_EXPORTS void RemoveTiming(ScriptThread* Thread);

		MOHPC_EXPORTS uintptr_t GetStackCount() const;
		MOHPC_EXPORTS void AddStack();
		MOHPC_EXPORTS void RemoveStack();

		MOHPC_EXPORTS void AddContextSwitch(ScriptThread* thread);
		MOHPC_EXPORTS bool IsContextSwitchAllowed() const;
		MOHPC_EXPORTS bool HasLoopDrop() const;
		MOHPC_EXPORTS bool HasLoopProtection() const;
		MOHPC_EXPORTS ScriptContainer* GetHeadContainer() const;

		MOHPC_EXPORTS ScriptThread* CreateScriptThread(GameScript *scr, Listener *self, const_str label);
		MOHPC_EXPORTS ScriptThread* CreateScriptThread(GameScript *scr, Listener *self, const str& label);
		MOHPC_EXPORTS ScriptThread* CreateScriptThread(ScriptContainer *ScriptContainer, const_str label);
		MOHPC_EXPORTS ScriptThread* CreateScriptThread(ScriptContainer *ScriptContainer, const str& label);
		MOHPC_EXPORTS ScriptThread* CreateScriptThread(ScriptContainer *ScriptContainer, unsigned char *m_pCodePos);
		MOHPC_EXPORTS ScriptThread* CreateThread(GameScript *scr, const str& label, Listener *self = NULL);
		MOHPC_EXPORTS ScriptThread* CreateThread(const str& filename, const str& label, Listener *self = NULL);
		MOHPC_EXPORTS ScriptContainer* CurrentScriptContainer();

		MOHPC_EXPORTS ScriptThread* CurrentThread();
		MOHPC_EXPORTS ScriptThread* PreviousThread();

		MOHPC_EXPORTS void ExecuteThread(GameScript *scr, const str& label = "");
		MOHPC_EXPORTS void ExecuteThread(const str& filename, const str& label = "");

		MOHPC_EXPORTS void ExecuteThread(GameScript *scr, const str& label, Event &parms);
		MOHPC_EXPORTS void ExecuteThread(const str& filename, const str& label, Event &parms);

		MOHPC_EXPORTS GameScript* GetTempScript(const char* data);
		MOHPC_EXPORTS GameScript* GetGameScript(const str& filename, bool recompile = false);
		MOHPC_EXPORTS GameScript* GetGameScript(const_str filename, bool recompile = false);
		MOHPC_EXPORTS GameScript* GetScript(const str& filename, bool recompile = false);
		MOHPC_EXPORTS GameScript* GetScript(const_str filename, bool recompile = false);

		MOHPC_EXPORTS void CloseGameScript();
		MOHPC_EXPORTS void Reset(bool samemap = false);

		MOHPC_EXPORTS void ExecuteRunning();
		MOHPC_EXPORTS void SetTime(float time);

		MOHPC_EXPORTS Parm* GetParm();

	protected:
		virtual void Init() override;
	};
	using ScriptManagerPtr = SharedPtr<ScriptManager>;
};
