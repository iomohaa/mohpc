#pragma once

#include "Listener.h"

namespace MOHPC
{
	class GameScript;
	class ScriptThread;
	class ScriptVM;
	class StateScript;

	class ScriptContainer : public Listener
	{
		friend GameScript;
		friend StateScript;
		friend ScriptManager;
		friend ScriptVM;

	private:
		/** Current game script. */
		GameScript* m_Script;

		/** The self variable. */
		SafePtr<Listener> m_Self;
		
		/** The first thread in list. */
		ScriptVM* m_Threads;

		ScriptContainer* Next;
		ScriptContainer* Prev;

	public:
		CLASS_PROTOTYPE(ScriptContainer);

		ScriptContainer(GameScript *gameScript, Listener *self);
		ScriptContainer();
		virtual ~ScriptContainer();

		virtual void Archive(Archiver& arc);
		void ArchiveInternal(Archiver& arc);
		static void ArchiveScript(Archiver& arc, ScriptContainer **obj);
		void ArchiveCodePos(Archiver& arc, unsigned char **codePos);

		virtual ScriptThread* CreateThreadInternal(const ScriptVariable& label);
		virtual ScriptThread* CreateScriptInternal(const ScriptVariable& label);

		void AddThread(ScriptVM * m_ScriptVM);
		void KillThreads(void);
		void RemoveThread(ScriptVM * m_ScriptVM);

		const str& Filename();
		unsigned char* FindLabel(const str& label);
		unsigned char* FindLabel(const_str label);
		const_str NearestLabel(unsigned char *pos);

		StateScript* GetCatchStateScript(unsigned char *in, unsigned char *&out);

		GameScript* GetScript();
		Listener* GetSelf();
	};
}
