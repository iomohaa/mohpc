#pragma once

#include "ScriptClass.h"
#include "ContainerClass.h"
#include "con_set.h"
#include "Event.h"

namespace MOHPC
{
	class Listener;
	class Event;
	class EventDef;
	class ScriptThread;
	class ScriptVariableList;

	typedef ContainerClass<SafePtr<Listener>> ConList;
	typedef con_map<Event *, EventDef *> eventMap;

	typedef SafePtr<Listener> ListenerPtr;

	extern Event EV_Listener_WaitCreateReturnThread;
	extern Event EV_DelayThrow;
	extern Event EV_Throw;
	extern Event EV_Listener_CreateReturnThread;
	extern Event EV_Listener_CreateThread;
	extern Event EV_Listener_ExecuteReturnScript;
	extern Event EV_Listener_ExecuteScript;
	extern Event EV_Remove;

	class Listener : public ScriptClass
	{
	public:
		con_set<const_str, ConList> *m_NotifyList;
		con_set<const_str, ConList> *m_WaitForList;
		con_set<const_str, ConList> *m_EndList;
		ScriptVariableList *vars;

	private:
		void ExecuteScriptInternal(Event *ev, ScriptVariable& scriptVariable);
		void ExecuteThreadInternal(Event *ev, ScriptVariable& returnValue);
		void WaitExecuteScriptInternal(Event *ev, ScriptVariable& returnValue);
		void WaitExecuteThreadInternal(Event *ev, ScriptVariable& returnValue);

		EventQueueNode* PostEventInternal(Event *ev, float delay, int flags);

	public:
		CLASS_PROTOTYPE(Listener);

		/* Game functions */
		virtual ScriptThread* CreateThreadInternal(const ScriptVariable& label);
		virtual ScriptThread* CreateScriptInternal(const ScriptVariable& label);
		virtual void StoppedNotify();
		virtual void StartedWaitFor();
		virtual void StoppedWaitFor(const_str name, bool bDeleting);

		virtual Listener* GetScriptOwner(void);

		Listener();
		virtual ~Listener();

		virtual void Archive(Archiver &arc);

		void CancelEventsOfType(Event *ev);
		void CancelEventsOfType(Event &ev);
		void CancelFlaggedEvents(int flags);
		void CancelPendingEvents(void);

		bool EventPending(Event &ev);

		void PostEvent(Event *ev, float delay, int flags = 0);
		void PostEvent(const Event &ev, float delay, int flags = 0);

		bool PostponeAllEvents(float time);
		bool PostponeEvent(Event& ev, float time);

		bool ProcessEvent(const Event &ev);
		bool ProcessEvent(Event *ev);
		bool ProcessEvent(Event &ev);
		ScriptVariable ProcessEventReturn(Event * ev);

		void ProcessContainerEvent(const Container< Event * >& conev);

		bool ProcessPendingEvents(void);

		bool ProcessScriptEvent(Event &ev);
		bool ProcessScriptEvent(Event *ev);

		void CreateVars(void);
		void ClearVars(void);
		ScriptVariableList* Vars(void);

		bool BroadcastEvent(Event &event, ConList *listeners);
		bool BroadcastEvent(str name, Event &event);
		bool BroadcastEvent(const_str name, Event &event);
		void CancelWaiting(str name);
		void CancelWaiting(const_str name);
		void CancelWaitingAll(void);
		void CancelWaitingSources(const_str name, ConList &listeners, ConList &stoppedListeners);

		void ExecuteThread(const str& scriptName, const str& label, Event *params = NULL);
		void ExecuteThread(const str& scriptName, const str& label, Event &params);

		void EndOn(str name, Listener *listener);
		void EndOn(const_str name, Listener *listener);
		void Notify(const char *name);
		void Register(str name, Listener *listener);
		void Register(const_str name, Listener *listener);
		void RegisterSource(const_str name, Listener *listener);
		void RegisterTarget(const_str name, Listener *listener);
		void Unregister(const str& name);
		void Unregister(const_str name);
		void Unregister(const str& name, Listener *listener);
		void Unregister(const_str name, Listener *listener);
		void UnregisterAll(void);
		bool UnregisterSource(const_str name, Listener *listener);
		bool UnregisterTarget(const_str name, Listener *listener);
		void UnregisterTargets(const_str name, ConList &listeners, ConList &stoppedListeners, Container< const_str > &stoppedNames);
		void AbortRegistration(const_str name, Listener *l);

		size_t RegisterSize(const_str name) const;
		size_t RegisterSize(const str& name) const;
		size_t WaitingSize(const_str name) const;
		size_t WaitingSize(const str& name) const;

		bool WaitTillDisabled(const str& s);
		bool WaitTillDisabled(const_str s);

		int GetFlags(Event *event) const;
		bool ValidEvent(const str& name) const;

		//
		// Scripting functions
		//
		void CommandDelay(Event *ev);
		void EventDelete(Event *ev);
		void EventInheritsFrom(Event *ev);
		void EventIsInheritedBy(Event *ev);
		void GetClassname(Event *ev);

		void CancelFor(Event *ev);
		void CreateReturnThread(Event *ev);
		void CreateThread(Event *ev);
		void ExecuteReturnScript(Event *ev);
		void ExecuteScript(Event *ev);
		void EventDelayThrow(Event *ev);
		void EventEndOn(Event *ev);
		void EventGetOwner(Event *ev);
		void EventNotify(Event *ev);
		void EventThrow(Event *ev);
		void EventUnregister(Event *ev);
		void WaitCreateReturnThread(Event *ev);
		void WaitCreateThread(Event *ev);
		void WaitExecuteReturnScript(Event *ev);
		void WaitExecuteScript(Event *ev);
		void WaitTill(Event *ev);
		void WaitTillTimeout(Event *ev);
		void WaitTillAny(Event *ev);
		void WaitTillAnyTimeout(Event *ev);
	};
};
