#include <Shared.h>
#include <MOHPC/Script/ScriptClass.h>
#include <MOHPC/Script/Event.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/EventManager.h>
#include <MOHPC/Managers/GameManager.h>
#include <MOHPC/Managers/ScriptManager.h>
#include <stdarg.h>

using namespace MOHPC;

ClassDef *ClassDef::classlist = nullptr;
size_t ClassDef::numclasses;

size_t ClassDef::dump_numclasses;
size_t ClassDef::dump_numevents;
Container<intptr_t> ClassDef::sortedList;
Container<ClassDef *> ClassDef::sortedClassList;

static void CLASS_Printf(const char *format, ...)
{
	va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va);
}

static void CLASS_DPrintf(const char *format, ...)
{
#ifdef _DEBUG
	va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va);
#endif
}

static void CLASS_Error(const char *format, ...)
{
	va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va);
}
int ClassDef::compareClasses(const void *arg1, const void *arg2)
{
	ClassDef *c1 = *(ClassDef **)arg1;
	ClassDef *c2 = *(ClassDef **)arg2;

	return stricmp(c1->classname, c2->classname);
}

void ClassDef::SortClassList(Container<ClassDef *> *sortedList)
{
	ClassDef *c;

	sortedList->Resize(numclasses);

	for (c = classlist; c != nullptr; c = c->next)
	{
		sortedList->AddObject(c);
	}

	qsort((void *)sortedList->AddressOfObjectAt(1),
		(size_t)sortedList->NumObjects(),
		sizeof(ClassDef *), compareClasses);
}

ClassDef *MOHPC::GetClassForID(const char *name)
{
	ClassDef *classlist = ClassDef::classlist;
	ClassDef *c;

	for (c = classlist; c != nullptr; c = c->next)
	{
		if (c->classID && !stricmp(c->classID, name))
		{
			return c;
		}
	}

	return NULL;
}

ClassDef *MOHPC::GetClass(const char *name)
{
	if (name == NULL || name == "") {
		return NULL;
	}

	ClassDef *c;

	for (c = GetClassList(); c != nullptr; c = c->next)
	{
		if (stricmp(c->classname, name) == 0) {
			return c;
		}
	}

	return NULL;
}

ClassDef *MOHPC::GetClassList(void)
{
	return ClassDef::classlist;
}

void MOHPC::listAllClasses(void)
{
	ClassDef *c;
	for (c = GetClassList(); c != nullptr; c = c->next)
	{
		CLASS_DPrintf("%s\n", c->classname);
	}
}

void MOHPC::listInheritanceOrder(const char *classname)
{
	ClassDef *cls;
	ClassDef *c;

	cls = GetClass(classname);
	if (!cls)
	{
		CLASS_DPrintf("Unknown class: %s\n", classname);
		return;
	}
	for (c = cls; c != NULL; c = c->super)
	{
		CLASS_DPrintf("%s\n", c->classname);
	}
}

bool MOHPC::checkInheritance(const ClassDef *superclass, const ClassDef *subclass)
{
	const ClassDef *c;

	for (c = subclass; c != NULL; c = c->super)
	{
		if (c == superclass)
		{
			return true;
		}
	}
	return false;
}

bool MOHPC::checkInheritance(ClassDef *superclass, const char *subclass)
{
	ClassDef *c;

	c = GetClass(subclass);

	if (c == NULL)
	{
		CLASS_DPrintf("Unknown class: %s\n", subclass);
		return false;
	}

	return checkInheritance(superclass, c);
}

bool MOHPC::checkInheritance(const char *superclass, const char *subclass)
{
	ClassDef *c1;
	ClassDef *c2;

	c1 = GetClass(superclass);
	c2 = GetClass(subclass);

	if (c1 == NULL)
	{
		CLASS_DPrintf("Unknown class: %s\n", superclass);
		return false;
	}

	if (c2 == NULL)
	{
		CLASS_DPrintf("Unknown class: %s\n", subclass);
		return false;
	}

	return checkInheritance(c1, c2);
}

void MOHPC::CLASS_Print(FILE *class_file, const char *fmt, ...)
{
	va_list	argptr;
	char		text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	if (class_file)
		fprintf(class_file, text);
	else
		CLASS_DPrintf(text);
}

size_t totalmemallocated = 0;
unsigned int numclassesallocated = 0;

bool classInited = false;

void ScriptClass::Archive(Archiver& arc)
{

}

void ScriptClass::warning(const char *function, const char *format, ...)
{
	const char *classname;
	va_list va;

	va_start(va, format);
	int len = vsprintf(nullptr, format, va);
	char *buffer = new char[len + 1];
	vsprintf(buffer, format, va);
	va_end(va);

	classname = classinfo()->classname;

	printf("%s::%s : %s\n", classname, function, buffer);

	delete[] buffer;
}

void ScriptClass::error(const char *function, const char *format, ...)
{
	va_list	va;

	va_start(va, format);
	int len = vsprintf(nullptr, format, va);
	char *buffer = new char[len + 1];
	vsprintf(buffer, format, va);
	va_end(va);

	if (GetClassID())
	{
		CLASS_Error("%s::%s : %s\n", GetClassID(), function, buffer);
	}
	else
	{
		CLASS_Error("%s::%s : %s\n", GetClassname(), function, buffer);
	}

	delete[] buffer;
}

ClassDef::ClassDef()
{
	this->classname = NULL;
	this->classID = NULL;
	this->superclass = NULL;
	this->responses = NULL;
	this->numEvents = 0;
	this->responseLookup = NULL;
	this->newInstance = NULL;
	this->classSize = 0;
	this->super = NULL;
	this->prev = this;
	this->next = this;

	this->waitTillSet = NULL;
}

ClassDef::ClassDef(const char *classname, const char *classID, const char *superclass, ResponseDef<Class> *responses,
	void *(*newInstance)(void), int classSize)
{
	ClassDef *node;

	this->classname = classname;
	this->classID = classID;
	this->superclass = superclass;
	this->responses = responses;
	this->numEvents = 0;
	this->responseLookup = NULL;
	this->newInstance = newInstance;
	this->classSize = classSize;
	this->super = GetClass(superclass);

#ifndef NO_SCRIPTENGINE
	this->waitTillSet = NULL;
#endif

	if (!classID)
	{
		this->classID = "";
	}

	for (node = classlist; node != nullptr; node = node->next)
	{
		if ((node->super == NULL) && (!stricmp(node->superclass, this->classname)) &&
			(stricmp(node->classname, "Class")))
		{
			node->super = this;
		}
	}

	// Add to front of list
	if (classlist)
	{
		classlist->prev = this;
	}

	prev = nullptr;
	next = classlist;
	classlist = this;

	numclasses++;
}

ClassDef::~ClassDef()
{
	ClassDef *node;

	if (classlist)
	{
		if (prev)
		{
			prev->next = next;
		}
		if (next)
		{
			next->prev = prev;
		}

		// Check if any subclasses were initialized before their superclass
		for (node = classlist; node != nullptr; node = node->next)
		{
			if (node->super == this)
			{
				node->super = NULL;
			}
		}

		if (classlist == this)
		{
			classlist = next;
		}
	}
	else
	{
		// If the head of the list is deleted before the list is cleared, then we may have problems
		assert(this->next == this->prev);
	}

	if (responseLookup)
	{
		delete[] responseLookup;
		responseLookup = NULL;
	}
}

void ClassDef::AddWaitTill(const str& s)
{
	//return AddWaitTill(Director.AddString(s));
}

void ClassDef::AddWaitTill(const_str s)
{
	if (!waitTillSet)
	{
		waitTillSet = new con_arrayset<const_str, const_str>;
	}

	waitTillSet->addKeyIndex(s);
}

void ClassDef::RemoveWaitTill(const str& s)
{
	//return RemoveWaitTill(Director.AddString(s));
}

void ClassDef::RemoveWaitTill(const_str s)
{
	if (waitTillSet)
	{
		waitTillSet->remove(s);
	}
}

bool ClassDef::WaitTillDefined(const str& s)
{
	return false; // return WaitTillDefined(Director.AddString(s));
}

bool ClassDef::WaitTillDefined(const_str s)
{
	if (!waitTillSet)
	{
		return false;
	}

	return waitTillSet->findKeyIndex(s) != 0;
}

EventDef *ClassDef::GetDef(uintptr_t eventnum)
{
	ResponseDef< Class > *r = responseLookup[eventnum];

	if (r)
	{
		return r->def;
	}
	else
	{
		return NULL;
	}
}

int ClassDef::GetFlags(Event *event)
{
	EventDef *def = GetDef(event->eventnum);

	if (def)
	{
		return def->flags;
	}
	else
	{
		return 0;
	}
}

void ClassDef::BuildResponseList(void)
{
	ClassDef *c;
	ResponseDef<Class> *r;
	int ev;
	int i;
	bool *set;
	size_t num;

	if (responseLookup)
	{
		delete[] responseLookup;
		responseLookup = NULL;
	}

	num = EventManager::NumEventCommands();
	responseLookup = (ResponseDef< Class > **)new char[sizeof(ResponseDef< Class > *) * num];
	memset(responseLookup, 0, sizeof(ResponseDef< Class > *) * num);

	set = new bool[num];
	memset(set, 0, sizeof(bool) * num);

	this->numEvents = num;

	for (c = this; c != NULL; c = c->super)
	{
		r = c->responses;

		if (r)
		{
			for (i = 0; r[i].event != NULL; i++)
			{
				ev = (int)r[i].event->eventnum;
				r[i].def = r[i].event->GetInfo();

				if (!set[ev])
				{
					set[ev] = true;

					if (r[i].response)
					{
						responseLookup[ev] = &r[i];
					}
					else
					{
						responseLookup[ev] = NULL;
					}
				}
			}
		}
	}

	delete[] set;
}

void MOHPC::ClassDef::ClearResponseList()
{
	assert(responseLookup);
	if (responseLookup)
	{
		delete[] responseLookup;
		responseLookup = nullptr;
	}
}

void ClassDef::BuildEventResponses(void)
{
	ClassDef *c;
	size_t amount;
	size_t numclasses;

	amount = 0;
	numclasses = 0;

	for (c = classlist; c != nullptr; c = c->next)
	{
		c->BuildResponseList();

		amount += c->numEvents * sizeof(Response *);
		numclasses++;
	}

	CLASS_DPrintf("\n------------------\nEvent system initialized: "
		"%d classes %d events %d total memory in response list\n\n", numclasses, EventManager::NumEventCommands(), amount);
}

void ClassDef::ClearEventResponses()
{
	for (ClassDef* c = classlist; c != nullptr; c = c->next)
	{
		c->ClearResponseList();
	}
}

const char *ScriptClass::GetClassID(void) const
{
	return classinfo()->classID;
}

const char *ScriptClass::GetClassname(void) const
{
	return classinfo()->classname;
}

const char *ScriptClass::getSuperclass(void) const
{
	return classinfo()->superclass;
}

GameManagerPtr BaseScriptClass::GetGameManager() const
{
	return GetManager<GameManager>();
}

EventManagerPtr BaseScriptClass::GetEventManager() const
{
	return GetManager<EventManager>();
}

ScriptManagerPtr BaseScriptClass::GetScriptManager() const
{
	return GetManager<ScriptManager>();
}

bool ScriptClass::inheritsFrom(ClassDef *c) const
{
	return checkInheritance(c, classinfo());
}

bool ScriptClass::inheritsFrom(const char * name) const
{
	ClassDef *c;

	c = GetClass(name);

	if (c == NULL)
	{
		CLASS_Printf("Unknown class: %s\n", name);
		return false;
	}

	return checkInheritance(c, classinfo());
}

bool ScriptClass::isInheritedBy(const char * name) const
{
	ClassDef *c;

	c = GetClass(name);

	if (c == NULL)
	{
		CLASS_DPrintf("Unknown class: %s\n", name);
		return false;
	}

	return checkInheritance(classinfo(), c);
}

bool ScriptClass::isInheritedBy(ClassDef *c) const
{
	return checkInheritance(classinfo(), c);
}

ClassDefHook::ClassDefHook()
{
	this->classdef = NULL;
}

CLASS_DECLARATION(NULL, ScriptClass, NULL)
{
	{ NULL, NULL }
};


