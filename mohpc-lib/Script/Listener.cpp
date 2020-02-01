#include <Shared.h>
#include <MOHPC/Script/Listener.h>
#include <MOHPC/Script/Level.h>
#include <MOHPC/Script/ScriptContainer.h>
#include <MOHPC/Script/ScriptThread.h>
#include <MOHPC/Script/ScriptVariable.h>
#include <MOHPC/Script/ScriptException.h>
#include <MOHPC/Script/ConstStr.h>
#include <MOHPC/Script/Archiver.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/EventManager.h>
#include <MOHPC/Managers/GameManager.h>
#include <MOHPC/Managers/ScriptManager.h>

namespace MOHPC
{
Event EV_Listener_CancelFor
(
	"cancelFor",
	EV_DEFAULT,
	"s",
	"name",
	"Cancel for event of type name.",
	EV_NORMAL
);

Event EV_Listener_CommandDelay
(
	"commanddelay",
	EV_DEFAULT,
	"fsSSSSSS",
	"delay command arg1 arg2 arg3 arg4 arg5 arg6",
	"executes a command after the given delay.",
	EV_NORMAL
);

Event EV_Listener_Classname
(
	"classname",
	EV_DEFAULT,
	NULL,
	NULL,
	"classname variable",
	EV_GETTER
);

Event EV_Listener_SetClassname
(
	"classname",
	EV_DEFAULT,
	"s",
	"classname",
	"classname variable",
	EV_NORMAL
);

Event EV_Listener_CreateReturnThread
(
	"thread",
	EV_DEFAULT,
	"s",
	"label",
	"Creates a thread starting at label.",
	EV_RETURN
);

Event EV_Listener_CreateThread
(
	"thread",
	EV_DEFAULT,
	"s",
	"label",
	"Creates a thread starting at label.",
	EV_NORMAL
);

Event EV_Listener_ExecuteReturnScript
(
	"exec",
	EV_DEFAULT,
	"s",
	"script",
	"Executes the specified script.",
	EV_RETURN
);

Event EV_Listener_ExecuteScript
(
	"exec",
	EV_DEFAULT,
	"s",
	"script",
	"Executes the specified script.",
	EV_NORMAL
);

Event EV_Delete
(
	"delete",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removes this listener immediately.",
	EV_NORMAL
);

Event EV_Remove
(
	"imediateremove",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removes this listener immediately.",
	EV_NORMAL
);

Event EV_ScriptRemove
(
	"remove",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removes this listener the next time events are processed.",
	EV_NORMAL
);

Event EV_Listener_EndOn
(
	"endon",
	EV_DEFAULT,
	"s",
	"name",
	"Ends the function when the specified event is triggered.",
	EV_NORMAL
);

Event EV_Listener_InheritsFrom
(
	"inheritsfrom",
	EV_DEFAULT,
	"s",
	"class",
	"Returns 1 if the class inherits from the specified class. 0 otherwise.",
	EV_RETURN
);

Event EV_Listener_IsInheritedBy
(
	"isinheritedby",
	EV_DEFAULT,
	"s",
	"class",
	"Returns 1 if the class is inherited by the specified class. 0 otherwise.",
	EV_RETURN
);

Event EV_Listener_Notify
(
	"notify",
	EV_DEFAULT,
	"s",
	"name",
	"Triggers an event. An undefined event will be automatically created by calling waittill or endon.",
	EV_NORMAL
);

Event EV_Listener_GetOwner
(
	"owner",
	EV_DEFAULT,
	NULL,
	NULL,
	"Returns the owner.",
	EV_GETTER
);

Event EV_DelayThrow
(
	"delaythrow",
	EV_DEFAULT,
	"s",
	"label",
	"Internal usage.",
	EV_NORMAL
);

Event EV_Throw
(
	"throw",
	EV_DEFAULT,
	"s",
	"label",
	"Throws to the specified label.",
	EV_NORMAL
);

Event EV_Listener_Unregister
(
	"unregister",
	EV_DEFAULT,
	"s",
	"label",
	"Unregisters the label from the event of the same name.",
	EV_NORMAL
);

Event EV_Listener_WaitCreateReturnThread
(
	"waitthread",
	EV_DEFAULT,
	"s",
	"label",
	"Creates a thread starting at label and waits until the called thread is finished.",
	EV_RETURN
);

Event EV_Listener_WaitCreateThread
(
	"waitthread",
	EV_DEFAULT,
	"s",
	"label",
	"Creates a thread starting at label and waits until the called thread is finished.",
	EV_NORMAL
);

Event EV_Listener_WaitExecuteReturnScript
(
	"waitexec",
	EV_DEFAULT,
	"s",
	"script",
	"Executes the specified script and waits until the called thread group is finished.",
	EV_RETURN
);

Event EV_Listener_WaitExecuteScript
(
	"waitexec",
	EV_DEFAULT,
	"s",
	"script",
	"Executes the specified script and waits until the called thread group is finished.",
	EV_NORMAL
);

Event EV_Listener_WaitTill
(
	"waitTill",
	EV_DEFAULT,
	"s",
	"name",
	"Wait until event of type name",
	EV_NORMAL
);

Event EV_Listener_WaitTillTimeout
(
	"waittill_timeout",
	EV_DEFAULT,
	"fs",
	"timeout_time name",
	"Wait until event of type name with a timeout time",
	EV_NORMAL
);

Event EV_Listener_WaitTillAny
(
	"waittill_any",
	EV_DEFAULT,
	"sS",
	"name1 ...",
	"Wait until any event of type name",
	EV_NORMAL
);

Event EV_Listener_WaitTillAnyTimeout
(
	"waittill_any_timeout",
	EV_DEFAULT,
	"fsS",
	"timeout_time name1 ...",
	"Wait until any event of type name with a timeout time",
	EV_NORMAL
);

static bool DisableListenerNotify = false;
};

using namespace MOHPC;

void ArchiveListenerPtr(Archiver& arc, SafePtr< Listener > *obj)
{
	//arc.ArchiveSafePointer(obj);
}

template<>
void ConList::Archive(Archiver& arc)
{
	value.Archive(arc, ArchiveListenerPtr);
}

template<>
void Entry< const_str, ConList >::Archive(Archiver& arc)
{
	/*
	Director.ArchiveString(arc, key);
	value.Archive(arc);
	*/
}

Listener::Listener()
{
	m_EndList = NULL;

	m_NotifyList = NULL;
	m_WaitForList = NULL;

	vars = NULL;
}

Listener::~Listener()
{
	CancelPendingEvents();

	Unregister("delete");
	Unregister("remove");

	UnregisterAll();
	CancelWaitingAll();

	if (vars) {
		delete vars;
	}
}

#define L_ARCHIVE_NOTIFYLIST		1
#define L_ARCHIVE_WAITFORLIST		2
#define L_ARCHIVE_VARLIST			4
#define L_ARCHIVE_ENDLIST			8

void Listener::Archive(Archiver &arc)
{
	ScriptClass::Archive(arc);

	/*
	byte flag = 0;

	if (!arc.Loading())
	{
		if (m_NotifyList)
			flag |= L_ARCHIVE_NOTIFYLIST;
		if (m_WaitForList)
			flag |= L_ARCHIVE_WAITFORLIST;
		if (vars)
			flag |= L_ARCHIVE_VARLIST;
		if (m_EndList)
			flag |= L_ARCHIVE_ENDLIST;
	}

	arc.ArchiveByte(&flag);

	// archive the notify list
	if (flag & L_ARCHIVE_NOTIFYLIST)
	{
		if (arc.Loading())
		{
			m_NotifyList = new con_set < const_str, ConList >;
		}

		m_NotifyList->Archive(arc);
	}

	// archive the waiting thread list
	if (flag & L_ARCHIVE_WAITFORLIST)
	{
		if (arc.Loading())
		{
			m_WaitForList = new con_set < const_str, ConList >;
		}

		m_WaitForList->Archive(arc);
	}

	// archive the variable list
	if (flag & L_ARCHIVE_VARLIST)
	{
		if (arc.Loading())
		{
			vars = new ScriptVariableList;
		}

		vars->Archive(arc);
	}

	// archive the end on event list
	if (flag & L_ARCHIVE_ENDLIST)
	{
		if (arc.Loading())
		{
			m_EndList = new con_set < const_str, ConList >;
		}

		m_EndList->Archive(arc);
	}
	*/
}

void Listener::CancelEventsOfType(Event *ev)
{
	EventQueueNode *node;
	EventQueueNode *next;
	uintptr_t eventnum;

	EventManager* eventManager = GetEventManager();

	node = eventManager->EventQueue.next;

	eventnum = ev->eventnum;
	while (node != &eventManager->EventQueue)
	{
		next = node->next;
		if ((node->GetSourceObject() == this) && (node->event->eventnum == eventnum))
		{
			LL_Remove(node, next, prev);
			delete node;
		}
		node = next;
	}
}

void Listener::CancelEventsOfType(Event &ev)
{
	this->CancelEventsOfType(&ev);
}

void Listener::CancelFlaggedEvents(int flags)
{
	EventQueueNode *node;
	EventQueueNode *next;

	EventManager* eventManager = GetEventManager();

	node = eventManager->EventQueue.next;

	while (node != &eventManager->EventQueue)
	{
		next = node->next;
		if ((node->GetSourceObject() == this) && (node->flags & flags))
		{
			LL_Remove(node, next, prev);
			delete node;
		}
		node = next;
	}
}

void Listener::CancelPendingEvents(void)
{
	EventQueueNode *node;
	EventQueueNode *next;

	EventManager* eventManager = GetEventManager();
	if (eventManager)
	{
		node = eventManager->EventQueue.next;

		while (node != &eventManager->EventQueue)
		{
			next = node->next;
			if (node->GetSourceObject() == this)
			{
				LL_Remove(node, next, prev);
				delete node;
			}
			node = next;
		}
	}
}

bool Listener::EventPending(Event &ev)
{
	EventQueueNode *event;
	uintptr_t eventnum;

	EventManager* eventManager = GetEventManager();

	event = eventManager->EventQueue.next;

	eventnum = ev.eventnum;

	while (event != &eventManager->EventQueue)
	{
		if ((event->GetSourceObject() == this) && (event->event->eventnum))
		{
			return true;
		}

		event = event->next;
	}

	return false;
}

EventQueueNode *Listener::PostEventInternal(Event *ev, float delay, int flags)
{
	EventQueueNode *node;
	EventQueueNode *i;
	float time;

	if (!classinfo()->responseLookup[ev->eventnum])
	{
		if (!ev->eventnum)
		{
			/*
#ifdef _DEBUG
			glbs.DPrintf("^~^~^ Failed execution of event '%s' for class '%s'\n", ev->name.c_str(), GetClassname());
#else
			glbs.DPrintf("^~^~^ Failed execution of event for class '%s'\n", GetClassname());
#endif
			*/
		}

		delete ev;
		return NULL;
	}

	node = new EventQueueNode;

	EventManager* eventManager = GetEventManager();

	i = eventManager->EventQueue.next;
	time = GetGameManager()->GetLevel()->GetTimeSeconds() + (delay + 0.0005f);

	while (i != &eventManager->EventQueue && time > i->time)
	{
		i = i->next;
	}

	node->time = time;
	node->event = ev;
	node->flags = flags;
	node->SetSourceObject(this);

#ifdef _DEBUG
	node->name = ev->name;
#endif

	LL_Add(i, node, next, prev);

	return node;
}

void Listener::PostEvent(Event *ev, float delay, int flags)
{
	PostEventInternal(ev, delay, flags);
}

void Listener::PostEvent(const Event &ev, float delay, int flags)
{
	Event *e = GetEventManager()->NewEvent(ev);

	PostEventInternal(e, delay, flags);
}

bool Listener::PostponeAllEvents(float time)
{
	EventQueueNode *event;
	EventQueueNode *node;
	
	EventManager* eventManager = GetEventManager();

	event = eventManager->EventQueue.next;
	while (event != &eventManager->EventQueue)
	{
		if (event->GetSourceObject() == this)
		{
			event->time += time + 0.0005f;

			node = event->next;
			while ((node != &eventManager->EventQueue) && (event->time >= node->time))
			{
				node = node->next;
			}

			LL_Remove(event, next, prev);
			LL_Add(node, event, next, prev);

			return true;
		}
		event = event->next;
	}

	return false;
}

bool Listener::PostponeEvent(Event &ev, float time)
{
	EventQueueNode *event;
	EventQueueNode *node;
	uintptr_t eventnum;

	eventnum = ev.eventnum;

	EventManager* eventManager = GetEventManager();

	event = eventManager->EventQueue.next;
	while (event != &eventManager->EventQueue)
	{
		if ((event->GetSourceObject() == this) && (event->event->eventnum == eventnum))
		{
			event->time += time + 0.0005f;

			node = event->next;
			while ((node != &eventManager->EventQueue) && (event->time >= node->time))
			{
				node = node->next;
			}

			LL_Remove(event, next, prev);
			LL_Add(node, event, next, prev);

			return true;
		}
		event = event->next;
	}

	return false;
}

bool Listener::ProcessEvent(Event *ev)
{
	ClassDef *c = classinfo();
	ResponseDef<Class> *responses = NULL;
	Response response = NULL;

	if (!ev->eventnum)
	{
		/*
#ifdef _DEBUG
		EVENT_DPrintf("^~^~^ Failed execution of event '%s' for class '%s'\n", ev->name.c_str(), c->classname);
#else
		EVENT_DPrintf("^~^~^ Failed execution of event for class '%s'\n", c->classname);
#endif
		*/

		delete ev;
		return false;
	}

	responses = c->responseLookup[ev->eventnum];

	if (responses == NULL)
	{
		delete ev;
		return true;
	}

	response = responses->response;

	try
	{
		if (response)
		{
			(this->*response)(ev);
		}
	}
	catch (ScriptException&)
	{
		//ev->ErrorInternal(this, exc.string);
		//glbs.DPrintf("%s\n", exc.string.c_str());
	}

	delete ev;
	return true;
}

bool Listener::ProcessEvent(Event &ev)
{
	return ProcessScriptEvent(ev);
}

ScriptVariable Listener::ProcessEventReturn(Event *ev)
{
	ClassDef *c = classinfo();
	ResponseDef<Class> *responses = NULL;
	Response response = NULL;
	ScriptVariable m_Return;

	if (!ev->eventnum)
	{
		/*
#ifdef _DEBUG
		EVENT_Printf("^~^~^ Failed execution of event '%s' for class '%s'\n", ev->name.c_str(), c->classname);
#else
		EVENT_Printf("^~^~^ Failed execution of event for class '%s'\n", c->classname);
#endif
		*/

		delete ev;
		return m_Return;
	}

	responses = c->responseLookup[ev->eventnum];

	if (responses == NULL)
	{

		//EVENT_Printf("^~^~^ Failed execution of command '%s' for class '%s'\n", Event::GetEventName(ev->eventnum).c_str(), c->classname);
		delete ev;
		return m_Return;
	}

	response = responses->response;

	size_t previousArgs = ev->NumArgs();

	if (response)
	{
		(this->*response)(ev);
	}

	if (previousArgs != ev->NumArgs() && ev->NumArgs() != 0)
	{
		m_Return = ev->GetValue(ev->NumArgs());
	}

	delete ev;
	return m_Return;
}

bool Listener::ProcessEvent(const Event &ev)
{
	Event *event = GetEventManager()->NewEvent(ev);
	return ProcessEvent(event);
}

bool Listener::ProcessScriptEvent(Event *ev)
{
	bool result = ProcessScriptEvent(*ev);
	delete ev;
	return result;
}

bool Listener::ProcessScriptEvent(Event &ev)
{
	ClassDef *c = classinfo();
	ResponseDef<Class> *responses = NULL;
	Response response = NULL;

	if (!ev.eventnum)
	{
		/*
#ifdef _DEBUG
		EVENT_Printf("^~^~^ Failed execution of event '%s' for class '%s'\n", ev.name.c_str(), c->classname);
#else
		EVENT_Printf("^~^~^ Failed execution of event for class '%s'\n", c->classname);
#endif
		*/

		return false;
	}

	responses = c->responseLookup[ev.eventnum];

	if (responses == NULL)
	{
		return true;
	}

	response = responses->response;

	if (response)
	{
		(this->*response)(&ev);
	}

	return true;
}

void Listener::ProcessContainerEvent(const Container< Event * >& conev)
{
	size_t num = conev.NumObjects();

	for (uintptr_t i = 1; i <= num; i++)
	{
		ProcessEvent(conev.ObjectAt(i));
	}
}

bool Listener::ProcessPendingEvents(void)
{
	EventQueueNode *event;
	bool processedEvents;
	float t;

	processedEvents = false;

	t = GetGameManager()->GetLevel()->GetTimeSeconds();

	EventManager* eventManager = GetEventManager();

	event = eventManager->EventQueue.next;
	while (event != &eventManager->EventQueue)
	{
		Listener *obj;

		assert(event);

		obj = event->GetSourceObject();

		if (event->time > t)
		{
			break;
		}

		if (obj != this)
		{
			// traverse normally
			event = event->next;
		}
		else
		{
			// the event is removed from its list and temporarily added to the active list
			LL_Remove(event, next, prev);

			// ProcessEvent will dispose of this event when it is done
			obj->ProcessEvent(event->event);

			// free up the node
			delete event;

			// start over, since can't guarantee that we didn't process any previous or following events
			event = eventManager->EventQueue.next;

			processedEvents = true;
		}
	}

	return processedEvents;
}

Listener *Listener::GetScriptOwner(void)
{
	return NULL;
}

void Listener::CreateVars(void)
{
	vars = new ScriptVariableList();
}

void Listener::ClearVars(void)
{
	if (vars)
	{
		delete vars;
		CreateVars();
	}
}

ScriptVariableList *Listener::Vars(void)
{
	if (!vars)
	{
		CreateVars();
	}

	return vars;
}

bool Listener::BroadcastEvent(str name, Event &event)
{
	return BroadcastEvent(GetScriptManager()->AddString(name), event);
}

bool Listener::BroadcastEvent(const_str name, Event &event)
{
	if (!m_NotifyList)
	{
		return false;
	}

	ConList *listeners = m_NotifyList->findKeyValue(name);

	if (!listeners)
	{
		return false;
	}

	return BroadcastEvent(event, listeners);
}

bool Listener::BroadcastEvent(Event &event, ConList *listeners)
{
	size_t num = listeners->NumObjects();

	if (!num)
	{
		return false;
	}

	if (num == 1)
	{
		Listener *listener = listeners->ObjectAt(1);

		if (listener)
		{
			listener->ProcessEvent(event);
		}

		return false;
	}

	ConList *listenersCopy = new ConList();
	*listenersCopy = *listeners;
	bool found = false;

	for (intptr_t i = listenersCopy->NumObjects(); i > 0; i--)
	{
		Listener *listener = listenersCopy->ObjectAt(i);

		if (listener)
		{
			listener->ProcessEvent(event);
			found = true;
		}
	}

	delete listenersCopy;

	return found;
}

void Listener::CancelWaiting(str name)
{
	CancelWaiting(GetScriptManager()->AddString(name));
}

void Listener::CancelWaiting(const_str name)
{
	if (!m_WaitForList)
	{
		return;
	}

	ConList *list = m_WaitForList->findKeyValue(name);
	ConList stoppedListeners;

	if (!list)
	{
		return;
	}

	CancelWaitingSources(name, *list, stoppedListeners);

	m_WaitForList->remove(name);

	if (m_WaitForList->isEmpty())
	{
		delete m_WaitForList;
		m_WaitForList = NULL;

		if (!DisableListenerNotify)
		{
			StoppedWaitFor(name, false);
		}
	}

	if (!DisableListenerNotify)
	{
		for (intptr_t i = stoppedListeners.NumObjects(); i > 0; i--)
		{
			Listener *listener = stoppedListeners.ObjectAt(i);

			if (listener)
			{
				listener->StoppedNotify();
			}
		}
	}
}

void Listener::CancelWaitingAll()
{
	CancelWaiting(0);

	if (!m_WaitForList)
	{
		return;
	}

	con_set_enum< const_str, ConList > en = *m_WaitForList;
	Entry< const_str, ConList > *e;
	ConList stoppedListeners;

	for (e = en.NextElement(); e != NULL; e = en.NextElement())
	{
		CancelWaitingSources(e->key, e->value, stoppedListeners);
	}

	delete m_WaitForList;
	m_WaitForList = NULL;

	if (!DisableListenerNotify)
	{
		StoppedWaitFor(0, false);

		for (intptr_t i = stoppedListeners.NumObjects(); i > 0; i--)
		{
			Listener *listener = stoppedListeners.ObjectAt(i);

			if (listener)
			{
				listener->StoppedNotify();
			}
		}
	}
}

void Listener::CancelWaitingSources(const_str name, ConList &listeners, ConList &stoppedListeners)
{
	for (intptr_t i = listeners.NumObjects(); i > 0; i--)
	{
		Listener *listener = listeners.ObjectAt(i);

		if (listener && listener->UnregisterSource(name, this))
		{
			stoppedListeners.AddObject(listener);
		}
	}
}

void Listener::Notify(const char *name)
{
	Unregister(name);
}

void Listener::StoppedNotify(void)
{
}

void Listener::StartedWaitFor(void)
{
}

void Listener::StoppedWaitFor(const_str name, bool bDeleting)
{
}

void Listener::EndOn(str name, Listener *listener)
{
	EndOn(GetScriptManager()->AddString(name), listener);
}

void Listener::EndOn(const_str name, Listener *listener)
{
	if (!m_EndList)
	{
		m_EndList = new con_set < const_str, ConList >;
	}

	ConList& list = m_EndList->addKeyValue(name);

	list.AddUniqueObject(listener);
}

void Listener::Register(str name, Listener *listener)
{
	Register(GetScriptManager()->AddString(name), listener);
}

void Listener::Register(const_str name, Listener *listener)
{
	RegisterSource(name, listener);
	listener->RegisterTarget(name, this);
}

void Listener::RegisterSource(const_str name, Listener *listener)
{
	if (!m_NotifyList)
	{
		m_NotifyList = new con_set < const_str, ConList >;
	}

	ConList& list = m_NotifyList->addKeyValue(name);

	list.AddObject(listener);
}

void Listener::RegisterTarget(const_str name, Listener *listener)
{
	if (!m_WaitForList)
	{
		StartedWaitFor();
		m_WaitForList = new con_set < const_str, ConList >;
	}

	ConList& list = m_WaitForList->addKeyValue(name);

	list.AddObject(listener);
}

void Listener::Unregister(const str& name)
{
	ScriptManager* scriptManager = GetScriptManager();
	if (scriptManager)
	{
		Unregister(scriptManager->AddString(name));
	}
}

void Listener::Unregister(const_str name)
{
	if (m_EndList)
	{
		ConList *list = m_EndList->findKeyValue(name);
		bool bDeleteSelf = false;

		if (list)
		{
			ConList listeners = *list;

			m_EndList->remove(name);

			if (m_EndList->isEmpty())
			{
				delete m_EndList;
				m_EndList = NULL;
			}

			for (intptr_t i = listeners.NumObjects(); i > 0; i--)
			{
				Listener *listener = listeners.ObjectAt(i);

				if (listener)
				{
					if (listener == this && (name == STRING_REMOVE || name == STRING_DELETE || bDeleteSelf))
					{
						continue;
					}

					if (listener == this)
					{
						bDeleteSelf = true;
					}

					delete listener;
				}
			}
		}

		if (bDeleteSelf)
		{
			return;
		}
	}

	if (!m_NotifyList)
	{
		return;
	}

	ConList *list = m_NotifyList->findKeyValue(name);
	ConList stoppedListeners;
	Container< const_str > stoppedNames;

	if (!list)
	{
		return;
	}

	UnregisterTargets(name, *list, stoppedListeners, stoppedNames);

	m_NotifyList->remove(name);

	if (m_NotifyList->isEmpty())
	{
		delete m_NotifyList;
		m_NotifyList = NULL;

		if (!DisableListenerNotify)
		{
			StoppedNotify();
		}
	}

	if (!DisableListenerNotify)
	{
		for (intptr_t i = stoppedListeners.NumObjects(); i > 0; i--)
		{
			Listener *listener = stoppedListeners.ObjectAt(i);

			if (listener)
			{
				listener->StoppedWaitFor(name, false);
			}
		}
	}
}

void Listener::Unregister(const str& name, Listener *listener)
{
	ScriptManager* scriptManager = GetScriptManager();
	if (scriptManager)
	{
		Unregister(scriptManager->AddString(name));
	}
}

void Listener::Unregister(const_str name, Listener *listener)
{
	if (UnregisterSource(name, listener))
	{
		if (!DisableListenerNotify) {
			StoppedNotify();
		}
	}

	if (listener->UnregisterTarget(name, this))
	{
		if (!DisableListenerNotify) {
			listener->StoppedWaitFor(name, false);
		}
	}
}

void Listener::UnregisterAll(void)
{
	Unregister(0);

	if (m_EndList)
	{
		delete m_EndList;
		m_EndList = NULL;
	}

	if (!m_NotifyList)
	{
		return;
	}

	con_set_enum < const_str, ConList > en = *m_NotifyList;
	Entry< const_str, ConList > *e;
	ConList stoppedListeners;
	Container< const_str > stoppedNames;

	en = *m_NotifyList;

	for (e = en.NextElement(); e != NULL; e = en.NextElement())
	{
		UnregisterTargets(e->key, e->value, stoppedListeners, stoppedNames);
	}

	delete m_NotifyList;
	m_NotifyList = NULL;

	if (!DisableListenerNotify) {
		StoppedNotify();
	}

	for (intptr_t i = stoppedListeners.NumObjects(); i > 0; i--)
	{
		Listener *listener = stoppedListeners.ObjectAt(i);

		if (listener)
		{
			if (!DisableListenerNotify) {
				listener->StoppedWaitFor(stoppedNames.ObjectAt(i), true);
			}
		}
	}
}

bool Listener::UnregisterSource(const_str name, Listener *listener)
{
	if (!m_NotifyList)
	{
		return false;
	}

	ConList *list = m_NotifyList->findKeyValue(name);
	bool found = false;

	if (!list)
	{
		return false;
	}

	for (intptr_t i = list->NumObjects(); i > 0; i--)
	{
		if (list->ObjectAt(i) == listener)
		{
			list->RemoveObjectAt(i);
			found = true;
		}
	}

	if (list->NumObjects() == 0)
	{
		m_NotifyList->remove(name);

		if (m_NotifyList->isEmpty())
		{
			delete m_NotifyList;
			m_NotifyList = NULL;
		}
	}

	return found;
}

bool Listener::UnregisterTarget(const_str name, Listener *listener)
{
	if (!m_WaitForList)
	{
		return false;
	}

	ConList *list = m_WaitForList->findKeyValue(name);
	bool found = false;

	if (!list)
	{
		return false;
	}

	for (intptr_t i = list->NumObjects(); i > 0; i--)
	{
		if (list->ObjectAt(i) == listener)
		{
			list->RemoveObjectAt(i);
			found = true;
		}
	}

	if (list->NumObjects() == 0)
	{
		m_WaitForList->remove(name);

		if (m_WaitForList->isEmpty())
		{
			delete m_WaitForList;
			m_WaitForList = NULL;
		}
	}

	return found;
}

void Listener::UnregisterTargets(const_str name, ConList &listeners, ConList &stoppedListeners, Container< const_str > &stoppedNames)
{
	for (intptr_t i = listeners.NumObjects(); i > 0; i--)
	{
		Listener *listener = listeners.ObjectAt(i);

		if (listener && listener->UnregisterTarget(name, this))
		{
			stoppedListeners.AddObject(listener);
			stoppedNames.AddObject(name);
		}
	}
}

void Listener::AbortRegistration(const_str name, Listener *l)
{
	UnregisterSource(name, l);
	l->UnregisterTarget(name, this);
}

size_t Listener::RegisterSize(const str& name) const
{
	return RegisterSize(GetScriptManager()->AddString(name));
}

size_t Listener::RegisterSize(const_str name) const
{
	ConList *listeners;

	if (!m_NotifyList) {
		return 0;
	}

	listeners = m_NotifyList->findKeyValue(name);

	// return the number of listeners waiting for this listener
	if (listeners)
	{
		return listeners->NumObjects();
	}
	else
	{
		return 0;
	}
}

size_t Listener::WaitingSize(const str& name) const
{
	return WaitingSize(GetScriptManager()->AddString(name));
}

size_t Listener::WaitingSize(const_str name) const
{
	ConList *listeners;

	if (!m_WaitForList) {
		return 0;
	}

	listeners = m_WaitForList->findKeyValue(name);

	// return the number of listeners this listener is waiting for
	if (listeners)
	{
		return listeners->NumObjects();
	}
	else
	{
		return 0;
	}
}

bool Listener::WaitTillDisabled(const str& s)
{
	return WaitTillDisabled(GetScriptManager()->AddString(s));
}

bool Listener::WaitTillDisabled(const_str s)
{
	for (ClassDef *c = classinfo(); c != NULL; c = c->super)
	{
		if (c->WaitTillDefined(s))
		{
			return true;
		}
	}

	return false;
}

int Listener::GetFlags(Event *event) const
{
	return classinfo()->GetFlags(event);
}

bool Listener::ValidEvent(const str& name) const
{
	uintptr_t num;
	EventDef *def;

	num = GetEventManager()->FindEventNum(name);
	if (!num)
		return false;

	def = classinfo()->GetDef(num);
	if (!def)
		return false;

	return true;
}

//==========================
// Listener's events
//==========================

void Listener::EventDelete(Event *ev)
{
	if (ev->NumArgs())
	{
		ScriptError("Arguments not allowed.");
	}

	delete this;
}

void Listener::EventInheritsFrom(Event *ev)
{
	ev->AddInteger(inheritsFrom(ev->GetString(1)));
}

void Listener::EventIsInheritedBy(Event *ev)
{
	ev->AddInteger(isInheritedBy(ev->GetString(1)));
}

void Listener::GetClassname(Event *ev)
{
	ev->AddString(ScriptClass::GetClassname());
}

void Listener::CommandDelay(Event *ev)
{
	if (ev->NumArgs() < 2)
	{
		ScriptError("Not enough arguments.");
	}

	Event *e = GetEventManager()->NewEvent(ev->GetString(2));

	for (size_t i = 3; i <= ev->NumArgs(); i++)
	{
		e->AddValue(ev->GetValue(i));
	}

	PostEvent(e, ev->GetFloat(1));
}

void Listener::CancelFor(Event *ev)
{
	BroadcastEvent(ev->GetConstString(1), EV_Remove);
}

void Listener::EventDelayThrow(Event *ev)
{
	BroadcastEvent(0, *ev);
}

void Listener::EventEndOn(Event *ev)
{
	const_str name = ev->GetConstString(1);

	if (GetScriptManager()->CurrentThread() == this)
	{
		ScriptError("cannot end for the current thread!");
	}

	EndOn(name, GetScriptManager()->CurrentThread());
}

void Listener::EventGetOwner(Event *ev)
{
	ev->AddListener(GetScriptOwner());
}

void Listener::EventNotify(Event *ev)
{
	str name = ev->GetString(1);

	Notify(name);
}

void Listener::EventThrow(Event *ev)
{
	BroadcastEvent(0, *ev);
}

void Listener::EventUnregister(Event *ev)
{
	Unregister(ev->GetConstString(1));
}

void Listener::WaitTill(Event *ev)
{
	const_str name;

	if (GetScriptManager()->CurrentThread() == this)
	{
		ScriptError("cannot waittill on the current thread!");
	}

	name = ev->GetConstString(1);

	if (WaitTillDisabled(name))
	{
		ScriptError("invalid waittill %s for '%s'", GetScriptManager()->GetString(name).c_str(), ScriptClass::GetClassname());
	}

	Register(name, GetScriptManager()->CurrentThread());
}

void Listener::WaitTillTimeout(Event *ev)
{
	const_str name;
	float timeout_time;

	if (GetScriptManager()->CurrentThread() == this)
	{
		ScriptError("cannot waittill on the current thread!");
	}

	timeout_time = ev->GetFloat(1);
	name = ev->GetConstString(2);

	if (WaitTillDisabled(name))
	{
		ScriptError("invalid waittill %s for '%s'", GetScriptManager()->GetString(name).c_str(), ScriptClass::GetClassname());
	}

	Register(name, GetScriptManager()->CurrentThread());
	GetScriptManager()->CurrentThread()->PostEvent(EV_ScriptThread_CancelWaiting, timeout_time);
}

void Listener::WaitTillAny(Event *ev)
{
	const_str name;

	if (GetScriptManager()->CurrentThread() == this)
	{
		ScriptError("cannot waittill any on the current thread!");
	}

	for (size_t i = 1; i <= ev->NumArgs(); i++)
	{
		name = ev->GetConstString(i);

		if (WaitTillDisabled(name))
		{
			ScriptError("invalid waittill %s for '%s'", GetScriptManager()->GetString(name).c_str(), ScriptClass::GetClassname());
		}

		Register(name, GetScriptManager()->CurrentThread());
	}
}

void Listener::WaitTillAnyTimeout(Event *ev)
{
	const_str name;
	float timeout_time;

	if (GetScriptManager()->CurrentThread() == this)
	{
		ScriptError("cannot waittill any on the current thread!");
	}

	timeout_time = ev->GetFloat(1);

	for (size_t i = 1; i <= ev->NumArgs(); i++)
	{
		name = ev->GetConstString(i);

		if (WaitTillDisabled(name))
		{
			ScriptError("invalid waittill %s for '%s'", GetScriptManager()->GetString(name).c_str(), ScriptClass::GetClassname());
		}

		Register(name, GetScriptManager()->CurrentThread());
	}

	GetScriptManager()->CurrentThread()->PostEvent(EV_ScriptThread_CancelWaiting, timeout_time);
}

void Listener::ExecuteScriptInternal(Event *ev, ScriptVariable& returnValue)
{
	SafePtr< ScriptThread > thread = CreateScriptInternal(ev->GetValue(1));
	ScriptThread *currentThread;
	bool fReturn = returnValue.GetType() != VARIABLE_NONE;

	thread->ScriptExecute(&ev->data[1], ev->dataSize - 1, returnValue);

	currentThread = GetScriptManager()->CurrentThread();

	// current thread should request a context switch because its waiting for the child's return value
	if (GetScriptManager()->IsContextSwitchAllowed() && currentThread && fReturn && thread)
	{
		currentThread->HandleContextSwitch(thread);
	}
}

void Listener::ExecuteThreadInternal(Event *ev, ScriptVariable& returnValue)
{
	SafePtr< ScriptThread > thread = CreateThreadInternal(ev->GetValue(1));
	ScriptThread *currentThread;
	bool fReturn = returnValue.GetType() != VARIABLE_NONE;

	thread->ScriptExecute(&ev->data[1], ev->dataSize - 1, returnValue);

	currentThread = GetScriptManager()->CurrentThread();

	if (GetScriptManager()->IsContextSwitchAllowed() && currentThread && fReturn && thread)
	{
		currentThread->HandleContextSwitch(thread);
	}
}

void Listener::WaitExecuteScriptInternal(Event *ev, ScriptVariable& returnValue)
{
	ScriptThread *thread = CreateScriptInternal(ev->GetValue(1));

	thread->GetScriptContainer()->Register(0, GetScriptManager()->CurrentThread());

	thread->ScriptExecute(&ev->data[1], ev->dataSize - 1, returnValue);
}

void Listener::WaitExecuteThreadInternal(Event *ev, ScriptVariable& returnValue)
{
	ScriptThread *thread = CreateThreadInternal(ev->GetValue(1));

	thread->GetScriptContainer()->Register(0, GetScriptManager()->CurrentThread());

	thread->ScriptExecute(&ev->data[1], ev->dataSize - 1, returnValue);
}

ScriptThread *Listener::CreateScriptInternal(const ScriptVariable& label)
{
	GameScript *scr;
	ScriptThread *thread = NULL;

	if (label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING)
	{
		if (label.GetType() == VARIABLE_CONSTSTRING)
			thread = GetScriptManager()->CreateScriptThread(GetScriptManager()->GetGameScript(label.stringValue()), this, "");
		else
			thread = GetScriptManager()->CreateScriptThread(GetScriptManager()->GetGameScript(label.constStringValue()), this, "");
	}
	else if (label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1)
	{
		ScriptVariable *script = label[1];
		ScriptVariable *labelname = label[2];

		if (script->GetType() == VARIABLE_CONSTSTRING)
			scr = GetScriptManager()->GetGameScript(script->constStringValue());
		else
			scr = GetScriptManager()->GetGameScript(script->stringValue());

		if (labelname->GetType() == VARIABLE_CONSTSTRING)
			thread = GetScriptManager()->CreateScriptThread(scr, this, labelname->constStringValue());
		else
			thread = GetScriptManager()->CreateScriptThread(scr, this, labelname->stringValue());
	}
	else
	{
		ScriptError("Listener::CreateScriptInternal: bad label type '%s'", label.GetTypeName());
	}

	return thread;
}

ScriptThread *Listener::CreateThreadInternal(const ScriptVariable& label)
{
	GameScript *scr;
	ScriptThread *thread = NULL;

	if (label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING)
	{
		ScriptContainer *scriptClass = GetScriptManager()->CurrentScriptContainer();
		scr = scriptClass->GetScript();

		if (label.GetType() == VARIABLE_CONSTSTRING)
			thread = GetScriptManager()->CreateScriptThread(scr, this, label.constStringValue());
		else
			thread = GetScriptManager()->CreateScriptThread(scr, this, label.stringValue());
	}
	else if (label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1)
	{
		ScriptVariable *script = label[1];
		ScriptVariable *labelname = label[2];

		if (script->GetType() == VARIABLE_CONSTSTRING)
			scr = GetScriptManager()->GetGameScript(script->constStringValue());
		else
			scr = GetScriptManager()->GetGameScript(script->stringValue());

		if (labelname->GetType() == VARIABLE_CONSTSTRING)
			thread = GetScriptManager()->CreateScriptThread(scr, this, labelname->constStringValue());
		else
			thread = GetScriptManager()->CreateScriptThread(scr, this, labelname->stringValue());
	}
	else
	{
		ScriptError("ScriptContainer::CreateThreadInternal: bad argument format");
	}

	return thread;
}

void Listener::CreateReturnThread(Event *ev)
{
	ScriptVariable returnValue;

	returnValue.newPointer();

	ExecuteThreadInternal(ev, returnValue);

	ev->AddValue(returnValue);
}

void Listener::CreateThread(Event *ev)
{
	if (!GetScriptManager()->IsContextSwitchAllowed())
	{
		ScriptVariable returnValue;

		ExecuteThreadInternal(ev, returnValue);
	}
	else
	{
		ScriptThread *pThread = CreateThreadInternal(ev->GetValue(1));
		Event event;

		for (size_t i = 2; i <= ev->NumArgs(); i++)
		{
			event.AddValue(ev->GetValue(i));
		}

		if (pThread)
		{
			pThread->DelayExecute(event);
		}
	}
}

void Listener::ExecuteReturnScript(Event *ev)
{
	ScriptVariable returnValue;

	returnValue.newPointer();

	ExecuteScriptInternal(ev, returnValue);

	ev->AddValue(returnValue);
}

void Listener::ExecuteScript(Event *ev)
{
	if (!GetScriptManager()->IsContextSwitchAllowed())
	{
		ScriptVariable returnValue;

		ExecuteScriptInternal(ev, returnValue);
	}
	else
	{
		ScriptThread *pThread = CreateScriptInternal(ev->GetValue(1));
		Event event;

		for (size_t i = 2; i <= ev->NumArgs(); i++)
		{
			event.AddValue(ev->GetValue(i));
		}

		if (pThread)
		{
			pThread->DelayExecute(event);
		}
	}
}

void Listener::WaitCreateReturnThread(Event *ev)
{
	ScriptVariable returnValue;

	returnValue.newPointer();

	WaitExecuteThreadInternal(ev, returnValue);

	ev->AddValue(returnValue);
}

void Listener::WaitCreateThread(Event *ev)
{
	ScriptVariable returnValue;

	WaitExecuteThreadInternal(ev, returnValue);
}

void Listener::WaitExecuteReturnScript(Event *ev)
{
	ScriptVariable returnValue;

	returnValue.newPointer();

	WaitExecuteScriptInternal(ev, returnValue);

	ev->AddValue(returnValue);
}

void Listener::WaitExecuteScript(Event *ev)
{
	ScriptVariable returnValue;

	WaitExecuteScriptInternal(ev, returnValue);
}

void Listener::ExecuteThread(const str& scriptName, const str& labelName, Event *params)
{
	ScriptThread *thread = GetScriptManager()->CreateThread(scriptName, labelName, this);

	try
	{
		if (!thread)
		{
			return;
		}

		thread->Execute(params);
	}
	catch (ScriptException&)
	{
		//glbs.DPrintf("Listener::ExecuteThread: %s\n", exc.string.c_str());
	}
}

void Listener::ExecuteThread(const str& scriptName, const str& labelName, Event& params)
{
	ScriptThread *thread = GetScriptManager()->CreateThread(scriptName, labelName, this);

	try
	{
		if (!thread)
		{
			return;
		}

		thread->Execute(params);
	}
	catch (ScriptException&)
	{
		//glbs.DPrintf("Listener::ExecuteThread: %s\n", exc.string.c_str());
	}
}

CLASS_DECLARATION(Class, Listener, NULL)
{
	{ &EV_Listener_CommandDelay,				&Listener::CommandDelay },
	{ &EV_Delete,								&Listener::EventDelete },
	{ &EV_Remove,								&Listener::EventDelete },
	{ &EV_ScriptRemove,							&Listener::EventDelete },
	{ &EV_Listener_Classname,					&Listener::GetClassname },
	{ &EV_Listener_InheritsFrom,				&Listener::EventInheritsFrom },
	{ &EV_Listener_IsInheritedBy,				&Listener::EventIsInheritedBy },
	{ &EV_Listener_CancelFor,					&Listener::CancelFor },
	{ &EV_Listener_CreateReturnThread,			&Listener::CreateReturnThread },
	{ &EV_Listener_CreateThread,				&Listener::CreateThread },
	{ &EV_Listener_ExecuteReturnScript,			&Listener::ExecuteReturnScript },
	{ &EV_Listener_ExecuteScript,				&Listener::ExecuteScript },
	{ &EV_Listener_EndOn,						&Listener::EventEndOn },
	{ &EV_Listener_GetOwner,					&Listener::EventGetOwner },
	{ &EV_Listener_Notify,						&Listener::EventNotify },
	{ &EV_DelayThrow,							&Listener::EventDelayThrow },
	{ &EV_Throw,								&Listener::EventThrow },
	{ &EV_Listener_Unregister,					&Listener::EventUnregister },
	{ &EV_Listener_WaitCreateReturnThread,		&Listener::WaitCreateReturnThread },
	{ &EV_Listener_WaitCreateThread,			&Listener::WaitCreateThread },
	{ &EV_Listener_WaitExecuteReturnScript,		&Listener::WaitExecuteReturnScript },
	{ &EV_Listener_WaitExecuteScript,			&Listener::WaitExecuteScript },
	{ &EV_Listener_WaitTill,					&Listener::WaitTill },
	{ &EV_Listener_WaitTillTimeout,				&Listener::WaitTillTimeout },
	{ &EV_Listener_WaitTillAny,					&Listener::WaitTillAny },
	{ &EV_Listener_WaitTillAnyTimeout,			&Listener::WaitTillAnyTimeout },
	{ NULL, NULL }
};


