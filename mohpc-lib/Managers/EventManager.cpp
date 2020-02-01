#include <Shared.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/EventManager.h>
#include <MOHPC/Managers/GameManager.h>
#include <MOHPC/Managers/ScriptManager.h>
#include <MOHPC/Script/Level.h>
#include <MOHPC/Script/Listener.h>
#include <MOHPC/Script/ScriptVariable.h>
#include <algorithm>

using namespace MOHPC;

EventDef* EventManager::lastEventDef = nullptr;
size_t EventManager::totalEvents;

template<>
intptr_t MOHPC::HashCode< Event * >(Event * const& key)
{
	// can't use key->eventnum because eventnum will be assigned from con_set
	return (intptr_t)key;
}

template<>
intptr_t MOHPC::HashCode< command_t >(const command_t& key)
{
	return HashCode< str >(key.command);
}

CLASS_DEFINITION(EventManager)
EventManager::EventManager()
{

}

EventManager::~EventManager()
{
	if (EventSystemStarted())
	{
		ClassDef::ClearEventResponses();
		ClearEventList();
		UnloadEvents();
	}
}

void EventManager::ClearEventList()
{
	EventQueueNode *node = EventQueue.next, *tmp;

	while (node != &EventQueue)
	{
		tmp = node->next;

		delete node->event;
		delete node;

		node = tmp;
	}

	LL_Reset(&EventQueue, next, prev);

	//Event_allocator.FreeAll();
}

bool EventManager::EventSystemStarted()
{
	return bEventSystemStarted;
}

void EventManager::Init()
{
	InitEvents();
}

void EventManager::InitEvents()
{
	LoadEvents();
	ClassDef::BuildEventResponses();
	ClearEventList();
	bEventSystemStarted = true;
}

void EventManager::LoadEvents()
{
	EventDef *e = lastEventDef;
	str lower;
	EventDef *cmd;
	uintptr_t index;
	command_t c;
	const command_t *i;
	Entry< command_t, command_t > *entry;
	con_set_enum< command_t, command_t > en;

	while (e)
	{
		e->ev->InitAssetManager(this);

		cmd = &eventDefList[e->ev];

		cmd->command = e->command;
		cmd->flags = ((e->flags == EV_DEFAULT) - 1) & e->flags;
		cmd->formatspec = e->formatspec;
		cmd->argument_names = e->argument_names;
		cmd->documentation = e->documentation;
		cmd->type = e->type;

#ifdef _DEBUG
		//e->ev->name = e->command;
#endif

		lower = str(e->command);
		lower.tolower();

		en = commandList;

		index = 0;

		for (entry = en.NextElement(); entry != NULL; entry = en.NextElement())
		{
			i = &entry->value;

			if (i->command == lower && i->type == cmd->type)
			{
				index = entry->index;
				break;
			}
		}

		if (!index)
		{
			c.command = lower;
			c.flags = e->flags;
			c.type = cmd->type;

			index = commandList.addKeyIndex(c);
		}

		e->ev->eventnum = index;

		e = e->next;

		totalEvents++;
	}
}

void EventManager::UnloadEvents()
{
	EventDef* tmp;
	for (EventDef *e = lastEventDef; e; e = tmp)
	{
		tmp = e->next;
		delete e;
	}

	lastEventDef = nullptr;
}

size_t EventManager::NumEventCommands()
{
	return totalEvents;
}

void EventManager::ProcessPendingEvents()
{
	EventQueueNode *node;

	while (!LL_Empty(&EventQueue, next, prev))
	{
		Listener *obj;

		node = EventQueue.next;

		assert(node);

		obj = node->GetSourceObject();

		assert(obj);

		if (node->time > GetManager<GameManager>()->GetLevel()->GetTimeSeconds())
		{
			break;
		}

		// the event is removed from its list
		LL_Remove(node, next, prev);

		// ProcessEvent will dispose of this event when it is done
		obj->ProcessEvent(node->event);

		delete node;
	}
}

void EventManager::ShutdownEvents(void)
{
	if (!bEventSystemStarted)
	{
		return;
	}

	ClearEventList();

	EventDef *e, *prev;
	for (e = lastEventDef; e != NULL; e = prev)
	{
		prev = e->next;
		delete e;
	}

	commandList.clear();
	eventDefList.clear();

	bEventSystemStarted = false;
}

/*
void EventManager::ArchiveEvents(Archiver &arc)
{
	EventQueueNode *event;
	int num;

	num = 0;
	for (event = EventQueue.next; event != &EventQueue; event = event->next)
	{
		Listener * obj;

		assert(event);

		obj = event->GetSourceObject();

		assert(obj);

#if defined ( GAME_DLL )
		if (obj->isSubclassOf(Entity) &&
			(((Entity *)obj)->flags & FL_DONTSAVE))
		{
			continue;
		}
#endif

		num++;
	}

	arc.ArchiveInteger(&num);
	for (event = EventQueue.next; event != &EventQueue; event = event->next)
	{
		Listener * obj;

		assert(event);

		obj = event->GetSourceObject();

		assert(obj);

#if defined ( GAME_DLL )
		if (obj->isSubclassOf(Entity) &&
			(((Entity *)obj)->flags & FL_DONTSAVE))
		{
			continue;
		}
#endif

		event->event->Archive(arc);
		arc.ArchiveInteger(&event->inttime);
		arc.ArchiveInteger(&event->flags);
		arc.ArchiveSafePointer(&event->m_sourceobject);
	}
}

void EventManager::UnarchiveEvents(Archiver &arc)
{
	EventQueueNode *node;
	Event *e;
	int i, numEvents;

	// the FreeEvents list would already be allocated at this point
	// clear out any events that may exist
	L_ClearEventList();

	arc.ArchiveInteger(&numEvents);
	for (i = 0; i < numEvents; i++)
	{
		node = new EventQueueNode();
		e = new Event();
		e->Archive(arc);

		arc.ArchiveInteger(&node->inttime);
		arc.ArchiveInteger(&node->flags);
		arc.ArchiveSafePointer(&node->m_sourceobject);

		LL_Add(&EventQueue, node, next, prev);
	}
}
*/

uintptr_t EventManager::GetEvent(str name, uint8_t type)
{
	return GetEventWithFlags(name, EV_DEFAULT, type);
}

uintptr_t EventManager::GetEventWithFlags(str name, int flags, uint8_t type)
{
	uintptr_t *index;
	con_map<const_str, uintptr_t> *cmdList;

	if (type == EV_NORMAL)
	{
		cmdList = &normalCommandList;
	}
	else if (type == EV_RETURN)
	{
		cmdList = &returnCommandList;
	}
	else if (type == EV_GETTER)
	{
		cmdList = &getterCommandList;
	}
	else if (type == EV_SETTER)
	{
		cmdList = &setterCommandList;
	}
	else
	{
		return 0;
	}

	name.tolower();

	index = cmdList->find(GetManager<ScriptManager>()->GetString(name));

	if (!index || !(GetEventFlags(*index) & flags))
	{
		return 0;
	}
	else
	{
		return *index;
	}
}

command_t *EventManager::GetEventInfo(int eventnum)
{
	return &commandList[eventnum];
}

unsigned int EventManager::GetEventFlags(uintptr_t eventnum)
{
	command_t *cmd = &commandList[eventnum];

	if (cmd)
	{
		return cmd->flags;
	}
	else
	{
		return 0;
	}

	return 0;
}

const char *EventManager::GetEventName(uintptr_t eventnum)
{
	if (eventnum <= 0 || eventnum > commandList.size())
	{
		return "";
	}

	const command_t *cmd = &commandList[eventnum];

	if (cmd)
	{
		return cmd->command.c_str();
	}
	else
	{
		return "";
	}

	return "";
}

uintptr_t EventManager::FindEventNum(const str& s)
{
	command_t cmd;

	cmd.command = s;
	cmd.flags = 0;
	cmd.type = -1;

	return commandList.findKeyIndex(cmd);
}

uintptr_t EventManager::FindNormalEventNum(const_str s)
{
	uintptr_t* eventnum = normalCommandList.find(s);
	if (eventnum)
	{
		return *eventnum;
	}
	else
	{
		return 0;
	}
}

uintptr_t EventManager::FindNormalEventNum(const str& s)
{
	str lowered = s;
	lowered.tolower();
	return FindNormalEventNum(GetManager<ScriptManager>()->AddString(lowered));
}

uintptr_t EventManager::FindReturnEventNum(const_str s)
{
	uintptr_t* eventnum = returnCommandList.find(s);
	if (eventnum)
	{
		return *eventnum;
	}
	else
	{
		return 0;
	}
}

uintptr_t EventManager::FindReturnEventNum(const str& s)
{
	str lowered = s;
	lowered.tolower();
	return FindReturnEventNum(GetManager<ScriptManager>()->AddString(lowered));
}

uintptr_t EventManager::FindSetterEventNum(const_str s)
{
	uintptr_t* eventnum = setterCommandList.find(s);
	if (eventnum)
	{
		return *eventnum;
	}
	else
	{
		return 0;
	}
}

uintptr_t EventManager::FindSetterEventNum(const str& s)
{
	str lowered = s;
	lowered.tolower();
	return FindSetterEventNum(GetManager<ScriptManager>()->AddString(lowered));
}

uintptr_t EventManager::FindGetterEventNum(const_str s)
{
	uintptr_t* eventnum = getterCommandList.find(s);
	if (eventnum)
	{
		return *eventnum;
	}
	else
	{
		return 0;
	}
}

uintptr_t EventManager::FindGetterEventNum(const str& s)
{
	str lowered = s;
	lowered.tolower();
	return FindGetterEventNum(GetManager<ScriptManager>()->AddString(lowered));
}

Event *EventManager::NewEvent()
{
	return new Event();
}

Event *EventManager::NewEvent(const Event* ev)
{
	Event* newEvent = new Event();

	newEvent->fromScript = ev->fromScript;
	newEvent->eventnum = ev->eventnum;
	newEvent->dataSize = ev->dataSize;

	if (newEvent->dataSize)
	{
		newEvent->data = new ScriptVariable[newEvent->dataSize];

		for (uintptr_t i = 0; i < newEvent->dataSize; i++)
		{
			newEvent->data[i] = ev->data[i];
		}
	}
	else
	{
		newEvent->data = NULL;
	}

#ifdef _DEBUG
	newEvent->name = ev->name;
#endif

	return newEvent;
}

Event *EventManager::NewEvent(const Event& ev)
{
	return NewEvent(&ev);
}

Event *EventManager::NewEvent(uintptr_t eventnum)
{
	Event* ev = new Event(eventnum);
	
#ifdef _DEBUG
	ev->name = GetEventName(eventnum);
#endif

	return ev;
}

Event *EventManager::NewEvent(const str& command)
{
	Event* ev = new Event();

	command_t c;

	c.command = command.c_str();
	c.flags = 0;
	c.type = EV_NORMAL;

	c.command.tolower();

	ev->eventnum = commandList.findKeyIndex(c);
	if(!ev->eventnum)
	{
		//CLASS_DPrintf("^~^~^ Event '%s' does not exist.\n", command.c_str());
	}

#ifdef _DEBUG
	ev->name = command;
#endif

	return ev;
}

Event *EventManager::NewEvent(const str& command, uint8_t type)
{
	uintptr_t *index;

	ScriptManager* Director = GetManager<ScriptManager>();

	if (type == EV_NORMAL)
	{
		index = normalCommandList.find(Director->AddString(command));
	}
	else if (type == EV_RETURN)
	{
		index = returnCommandList.find(Director->AddString(command));
	}
	else if (type == EV_GETTER)
	{
		index = getterCommandList.find(Director->AddString(command));
	}
	else if (type == EV_SETTER)
	{
		index = setterCommandList.find(Director->AddString(command));
	}
	else
	{
		index = NULL;
	}

	Event* ev = new Event();

	if (index)
	{
		ev->eventnum = *index;
	}
	else
	{
		ev->eventnum = 0;
	}

#ifdef _DEBUG
	ev->name = command;
#endif

	return ev;
}

#ifdef _WIN32
int EventManager::compareEvents(void *context, const void *arg1, const void *arg2)
#else
int EventManager::compareEvents(const void *arg1, const void *arg2, void *context)
#endif
{
	EventManager* manager = (EventManager*)context;
	uintptr_t num1 = *(uintptr_t *)arg1;
	uintptr_t num2 = *(uintptr_t *)arg2;
	command_t *cmd1 = &manager->commandList[num1];
	command_t *cmd2 = &manager->commandList[num2];

	return stricmp(cmd1->command.c_str(), cmd2->command.c_str());
}

void EventManager::SortEventList(Container<uintptr_t> *sortedList)
{
	command_t *cmd;

	sortedList->Resize(commandList.size());

	for (uintptr_t i = 1; i <= commandList.size(); i++)
	{
		cmd = &commandList[i];

		if (cmd != NULL)
		{
			sortedList->AddObject(i);
		}
	}

	qsort2((void *)sortedList->AddressOfObjectAt(1),
		(size_t)sortedList->NumObjects(),
		sizeof(uintptr_t), &EventManager::compareEvents, this);
}

#define MAX_INHERITANCE 64
void EventManager::ClassEvents(const char *classname, bool print_to_disk)
{
	ClassDef *c;
	ResponseDef<Class> *r;
	uintptr_t ev;
	uintptr_t i;
	intptr_t j;
	bool *set;
	size_t num;
	uint8_t orderNum;
	Event **events;
	uint8_t *order;
	FILE *class_file;
	std::string classNames[MAX_INHERITANCE];
	std::string class_filename;

	c = GetClass(classname);
	if (!c)
	{
		//CLASS_DPrintf("Unknown class: %s\n", classname);
		return;
	}

	class_file = NULL;

	if (print_to_disk)
	{
		class_filename = std::string(classname) + ".txt";
		class_file = fopen(class_filename.c_str(), "w");
		if (class_file == NULL)
			return;
	}

	num = NumEventCommands();

	set = new bool[num];
	memset(set, 0, sizeof(bool) * num);

	events = new Event *[num];
	memset(events, 0, sizeof(Event *) * num);

	order = new uint8_t[num];
	memset(order, 0, sizeof(uint8_t) * num);

	orderNum = 0;
	for (; c != NULL; c = c->super)
	{
		if (orderNum < MAX_INHERITANCE)
		{
			classNames[orderNum] = c->classname;
		}
		r = c->responses;
		if (r)
		{
			for (i = 0; r[i].event != NULL; i++)
			{
				ev = (int)r[i].event->eventnum;
				if (!set[ev])
				{
					set[ev] = true;

					if (r[i].response)
					{
						events[ev] = r[i].event;
						order[ev] = orderNum;
					}
				}
			}
		}
		orderNum++;
	}

	CLASS_Print(class_file, "********************************************************\n");
	CLASS_Print(class_file, "********************************************************\n");
	CLASS_Print(class_file, "* All Events For Class: %s\n", classname);
	CLASS_Print(class_file, "********************************************************\n");
	CLASS_Print(class_file, "********************************************************\n\n");

	for (j = orderNum - 1; j >= 0; j--)
	{
		CLASS_Print(class_file, "\n********************************************************\n");
		CLASS_Print(class_file, "* Class: %s\n", classNames[j].c_str());
		CLASS_Print(class_file, "********************************************************\n\n");
		for (i = 0; i < num; i++)
		{
			intptr_t index;

			index = ClassDef::sortedList.at(i);
			if (events[index] && (order[index] == j))
			{
				eventDefList[events[index]].PrintEventDocumentation(class_file, false);
			}
		}
	}

	if (class_file != NULL)
	{
		//CLASS_DPrintf("Printed class info to file %s\n", class_filename.c_str());
		fclose(class_file);
	}

	delete[] events;
	delete[] order;
	delete[] set;
}

void EventManager::DumpClass(FILE * class_file, const char * className)
{
	ClassDef		*c;
	ResponseDef<Class> *r;
	int			ev;
	int			i;
	size_t		num, num2;
	Event       **events;

	c = GetClass(className);
	if (!c)
	{
		return;
	}

	num = commandList.size();
	num2 = NumEventCommands();

	events = new Event *[num2];
	memset(events, 0, sizeof(Event *) * num2);

	// gather event responses for this class
	r = c->responses;
	if (r)
	{
		for (i = 0; r[i].event != NULL; i++)
		{
			ev = (int)r[i].event->eventnum;
			if (r[i].response)
			{
				events[ev] = r[i].event;
			}
		}
	}

	CLASS_Print(class_file, "\n");
	if (c->classID[0])
	{
		CLASS_Print(class_file, "<h2> <a name=\"%s\">%s (<i>%s</i>)</a>", c->classname, c->classname, c->classID);
	}
	else
	{
		CLASS_Print(class_file, "<h2> <a name=\"%s\">%s</a>", c->classname, c->classname);
	}

	// print out lineage
	for (c = c->super; c != NULL; c = c->super)
	{
		CLASS_Print(class_file, " -> <a href=\"#%s\">%s</a>", c->classname, c->classname);
	}
	CLASS_Print(class_file, "</h2>\n");

	ClassDef::dump_numclasses++;

	CLASS_Print(class_file, "<BLOCKQUOTE>\n");
	for (i = 0; i < num; i++)
	{
		intptr_t index;

		index = ClassDef::sortedList.at(i);
		if (events[index])
		{
			eventDefList[events[index]].PrintEventDocumentation(class_file, true);
			ClassDef::dump_numevents++;
		}
	}
	CLASS_Print(class_file, "</BLOCKQUOTE>\n");
	delete[] events;
}


#define MAX_CLASSES 1024
void EventManager::DumpAllClasses()
{
	size_t i, num;
	ClassDef *c;
	FILE * class_file;
	std::string class_filename;
	std::string class_title;
	std::string classes[MAX_CLASSES];

#if defined( GAME_DLL )
	class_filename = "g_allclasses.html";
	class_title = "Game Module";
#elif defined( CGAME_DLL )
	class_filename = "cg_allclasses.html";
	class_title = "Client Game Module";
#else
	class_filename = "cl_allclasses.html";
	class_title = "Client Module";
#endif

	class_file = fopen(class_filename.c_str(), "w");
	if (class_file == NULL)
		return;

	// construct the HTML header for the document
	CLASS_Print(class_file, "<HTML>\n");
	CLASS_Print(class_file, "<HEAD>\n");
	CLASS_Print(class_file, "<Title>%s Classes</Title>\n", class_title.c_str());
	CLASS_Print(class_file, "</HEAD>\n");
	CLASS_Print(class_file, "<BODY>\n");
	CLASS_Print(class_file, "<H1>\n");
	CLASS_Print(class_file, "<center>%s Classes</center>\n", class_title.c_str());
	CLASS_Print(class_file, "</H1>\n");
#if defined( GAME_DLL )
	//
	// print out some commonly used classnames
	//
	CLASS_Print(class_file, "<h2>");
	CLASS_Print(class_file, "<a href=\"#Actor\">Actor</a>, ");
	CLASS_Print(class_file, "<a href=\"#Animate\">Animate</a>, ");
	CLASS_Print(class_file, "<a href=\"#Entity\">Entity</a>, ");
	CLASS_Print(class_file, "<a href=\"#ScriptSlave\">ScriptSlave</a>, ");
	CLASS_Print(class_file, "<a href=\"#ScriptThread\">ScriptThread</a>, ");
	CLASS_Print(class_file, "<a href=\"#Sentient\">Sentient</a>, ");
	CLASS_Print(class_file, "<a href=\"#StateMap\">StateMap</a>, ");
	CLASS_Print(class_file, "<a href=\"#Trigger\">Trigger</a>, ");
	CLASS_Print(class_file, "<a href=\"#World\">World</a>");
	CLASS_Print(class_file, "</h2>");
#endif

	ClassDef::dump_numclasses = 0;
	ClassDef::dump_numevents = 0;

	Container<uintptr_t> sortedList;
	Container<ClassDef*> sortedClassList;

	ClassDef::sortedList.clear();
	ClassDef::sortedClassList.clear();

	SortEventList(&sortedList);
	ClassDef::SortClassList(&sortedClassList);

	num = ClassDef::sortedClassList.size();

	// go through and process each class from smallest to greatest
	for (i = 0; i < num; i++)
	{
		c = ClassDef::sortedClassList.at(i);
		DumpClass(class_file, c->classname);
	}

	if (class_file != NULL)
	{
		CLASS_Print(class_file, "<H2>\n");
		CLASS_Print(class_file, "%d %s Classes.<BR>%d %s Events.\n", ClassDef::dump_numclasses, class_title.c_str(), ClassDef::dump_numevents, class_title.c_str());
		CLASS_Print(class_file, "</H2>\n");
		CLASS_Print(class_file, "</BODY>\n");
		CLASS_Print(class_file, "</HTML>\n");
		//CLASS_DPrintf("Dumped all classes to file %s\n", class_filename.c_str());
		fclose(class_file);
	}
}
