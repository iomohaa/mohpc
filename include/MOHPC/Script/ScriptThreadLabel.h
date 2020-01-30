#pragma once

#include "ScriptClass.h"

namespace MOHPC
{
	class Listener;
	class ScriptThread;
	class ScriptVariable;
	class GameScript;

	class ScriptThreadLabel : public BaseScriptClass
	{
	private:
		GameScript* m_Script;
		const_str m_Label;

	public:
		ScriptThreadLabel();

		ScriptThread* Create(Listener* listener);
		void Execute(Listener* listener = NULL);
		void Execute(Listener* listener, Event &ev);
		void Execute(Listener* listener, Event *ev);

		void Set(const char* label);
		void SetScript(const ScriptVariable& label);
		void SetScript(const char* label);
		void SetThread(const ScriptVariable& label);

		bool TrySet(const_str label);
		bool TrySet(const char* label);
		bool TrySetScript(const_str label);
		bool TrySetScript(const char* label);

		bool IsSet();

		void Archive(Archiver& arc);
	};
}
