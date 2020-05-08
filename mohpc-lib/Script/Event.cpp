#include <Shared.h>
#include <MOHPC/Script/Event.h>
#include <MOHPC/Script/ScriptException.h>
#include <MOHPC/Script/ScriptVariable.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/EventManager.h>
#include <stdarg.h>
#include <string>

using namespace MOHPC;

class EventDefDeconstructor
{
public:
	~EventDefDeconstructor()
	{
		EventManager::UnloadEvents();
	}
};

EventDefDeconstructor Deconstructor;

void EventArgDef::Setup(const char *eventName, const char *argName, const char *argType, const char *argRange)
{
	char        scratch[256];
	const char  *ptr;
	char        *tokptr;
	const char  *endptr;
	int         index;

	// set name
	name = argName;

	// set optionality
	if (isupper(argType[0]))
	{
		optional = true;
	}
	else
	{
		optional = false;
	}

	// grab the ranges
	index = 0;
	memset(minRangeDefault, true, sizeof(minRangeDefault));
	memset(minRange, 0, sizeof(minRange));
	memset(maxRangeDefault, true, sizeof(maxRangeDefault));
	memset(maxRange, 0, sizeof(maxRange));

	if (argRange && argRange[0])
	{
		ptr = argRange;
		while (1)
		{
			// find opening '['
			tokptr = (char *)strchr(ptr, '[');
			if (!tokptr)
			{
				break;
			}
			// find closing ']'
			endptr = strchr(tokptr, ']');
			if (!endptr)
			{
				assert(0);
				printf("Argument defintion %s, no matching ']' found for range spec in event %s.\n", name.c_str(), eventName);
				break;
			}
			// point to the next range
			ptr = endptr;
			// skip the '['
			tokptr++;
			// copy off the range spec
			// skip the ']'
			strncpy(scratch, tokptr, endptr - tokptr);
			// terminate the range
			scratch[endptr - tokptr] = 0;
			// see if there is one or two parameters here
			tokptr = strchr(scratch, ',');
			if (!tokptr)
			{
				// just one parameter
				minRange[index >> 1] = (float)atof(scratch);
				minRangeDefault[index >> 1] = false;
				index++;
				// skip the second parameter
				index++;
			}
			else if (tokptr == scratch)
			{
				// just second parameter
				// skip the first paremeter
				index++;
				tokptr++;
				maxRange[index >> 1] = (float)atof(scratch);
				maxRangeDefault[index >> 1] = false;
				index++;
			}
			else
			{
				bool second;
				// one or two parameters
				// see if there is anything behind the ','
				if (strlen(scratch) > size_t(tokptr - scratch + 1))
					second = true;
				else
					second = false;
				// zero out the ','
				*tokptr = 0;
				minRange[index >> 1] = (float)atof(scratch);
				minRangeDefault[index >> 1] = false;
				index++;
				// skip over the nul character
				tokptr++;
				if (second)
				{
					maxRange[index >> 1] = (float)atof(tokptr);
					maxRangeDefault[index >> 1] = false;
				}
				index++;
			}
		}
	}

	// figure out the type of variable it is
	switch (tolower(argType[0]))
	{
	case 'e':
		type = IS_ENTITY;
		break;
	case 'v':
		type = IS_VECTOR;
		break;
	case 'i':
		type = IS_INTEGER;
		break;
	case 'f':
		type = IS_FLOAT;
		break;
	case 's':
		type = IS_STRING;
		break;
	case 'b':
		type = IS_BOOLEAN;
		break;
	case 'l':
		type = IS_LISTENER;
		break;
	}
}

void EV_Print(FILE *stream, const char *format, ...)
{
	char buffer[1000];
	va_list va;

	va_start(va, format);

	vsprintf(buffer, format, va);

	if (stream)
	{
		fprintf(stream, buffer);
	}
	else
	{
		//glbs.DPrintf(buffer);
	}

	va_end(va);
}

void EventArgDef::PrintRange(FILE *event_file)
{
	bool integer;
	bool single;
	int numRanges;
	int i;

	single = false;
	integer = true;
	numRanges = 1;
	switch (type)
	{
	case IS_VECTOR:
		integer = false;
		numRanges = 3;
		break;
	case IS_FLOAT:
		integer = false;
		break;
	case IS_STRING:
		single = true;
		break;
	}
	for (i = 0; i < numRanges; i++)
	{
		if (single)
		{
			if (!minRangeDefault[i])
			{
				if (integer)
				{
					EV_Print(event_file, "<%d>", (int)minRange[i]);
				}
				else
				{
					EV_Print(event_file, "<%.2f>", minRange[i]);
				}
			}
		}
		else
		{
			// both non-default
			if (!minRangeDefault[i] && !maxRangeDefault[i])
			{
				if (integer)
				{
					EV_Print(event_file, "<%d...%d>", (int)minRange[i], (int)maxRange[i]);
				}
				else
				{
					EV_Print(event_file, "<%.2f...%.2f>", minRange[i], maxRange[i]);
				}
			}
			// max default
			else if (!minRangeDefault[i] && maxRangeDefault[i])
			{
				if (integer)
				{
					EV_Print(event_file, "<%d...max_integer>", (int)minRange[i]);
				}
				else
				{
					EV_Print(event_file, "<%.2f...max_float>", minRange[i]);
				}
			}
			// min default
			else if (minRangeDefault[i] && !maxRangeDefault[i])
			{
				if (integer)
				{
					EV_Print(event_file, "<min_integer...%d>", (int)maxRange[i]);
				}
				else
				{
					EV_Print(event_file, "<min_float...%.2f>", maxRange[i]);
				}
			}
		}
	}
}

void EventArgDef::PrintArgument(FILE *event_file)
{
	if (optional)
	{
		EV_Print(event_file, "[ ");
	}

	switch (type)
	{
	case IS_ENTITY:
		EV_Print(event_file, "Entity ");
		break;
	case IS_VECTOR:
		EV_Print(event_file, "Vector ");
		break;
	case IS_INTEGER:
		EV_Print(event_file, "Integer ");
		break;
	case IS_FLOAT:
		EV_Print(event_file, "Float ");
		break;
	case IS_STRING:
		EV_Print(event_file, "String ");
		break;
	case IS_BOOLEAN:
		EV_Print(event_file, "Boolean ");
		break;
	case IS_LISTENER:
		EV_Print(event_file, "Listener ");
		break;
	}
	EV_Print(event_file, "%s", name.c_str());

	PrintRange(event_file);

	if (optional)
	{
		EV_Print(event_file, " ]");
	}
}

void EventDef::Error(const char *format, ...)
{
	char buffer[1000];
	va_list va;

	va_start(va, format);
	vsprintf(buffer, format, va);
	va_end(va);

	printf("^~^~^ Game: '%s' : %s\n", command.c_str(), buffer);
}

void EventDef::PrintDocumentation(FILE *event_file, bool html)
{
	int p;
	str text;
	const char *name = command.c_str();

	if (!html)
	{
		text = "   ";
		p = 0;

		if (flags & EV_CONSOLE)
		{
			text[p++] = '*';
		}
		if (flags & EV_CHEAT)
		{
			text[p++] = 'C';
		}
		if (flags & EV_CACHE)
		{
			text[p++] = '%';
		}
	}

	if (html)
	{
		EV_Print(event_file, "\n<P><tt><B>%s</B>", name);
	}
	else
	{
		if (text[0] != ' ')
		{
			EV_Print(event_file, "%s %s", text.c_str(), name);
		}
		else
		{
			EV_Print(event_file, "%s %s", text.c_str(), name);
		}
	}

	SetupDocumentation();

	if (definition)
	{
		if (html)
		{
			EV_Print(event_file, "( <i>");
		}
		else
		{
			EV_Print(event_file, "( ");
		}

		for (size_t i = 1; i <= definition->NumObjects(); i++)
		{
			definition->ObjectAt(i).PrintArgument(event_file);

			if (i < definition->NumObjects())
			{
				EV_Print(event_file, ", ");
			}
		}

		if (html)
		{
			EV_Print(event_file, " </i>)</tt><BR>\n");
		}
		else
		{
			EV_Print(event_file, " )\n");
		}

		DeleteDocumentation();
	}
	else
	{
		if (html)
		{
			EV_Print(event_file, "</tt><BR>\n");
		}
		else
		{
			EV_Print(event_file, "\n");
		}
	}

	if (documentation)
	{
		char new_doc[1024];
		int new_index = 0;

		for (size_t old_index = 0; old_index < strlen(documentation); old_index++)
		{
			if (documentation[old_index] == '\n')
			{
				if (html)
				{
					new_doc[new_index] = '<';
					new_doc[new_index + 1] = 'B';
					new_doc[new_index + 2] = 'R';
					new_doc[new_index + 3] = '>';
					new_doc[new_index + 4] = '\n';
					new_index += 5;
				}
				else
				{
					new_doc[new_index] = '\n';
					new_doc[new_index + 1] = '\t';
					new_doc[new_index + 2] = '\t';
					new_index += 3;
				}
			}
			else
			{
				new_doc[new_index] = documentation[old_index];
				new_index++;
			}

		}

		new_doc[new_index] = 0;

		if (html)
		{
			EV_Print(event_file, "<ul>%s</ul>\n", new_doc);
		}
		else
		{
			EV_Print(event_file, "\t\t- %s\n", new_doc);
		}
	}
}

void EventDef::PrintEventDocumentation(FILE *event_file, bool html)
{
	if (flags & EV_CODEONLY) {
		return;
	}

	// purposely suppressed
	if (command[0] == '_')
	{
		return;
	}

	PrintDocumentation(event_file, html);
}

void EventDef::DeleteDocumentation(void)
{
	if (formatspec)
	{
		if (argument_names)
		{
			definition->FreeObjectList();

			delete definition;
			definition = NULL;
		}
	}
}

void EventDef::SetupDocumentation(void)
{
	const char *name = command.c_str();

	// setup documentation
	if (formatspec)
	{
		if (argument_names)
		{
			char        argumentNames[256];
			str         argSpec;
			str         rangeSpec;
			str         argName;
			EventArgDef argDef;
			const char  *namePtr;
			const char  *specPtr;
			size_t       specLength;
			size_t       index;
			Container<str> argNames;

			specLength = strlen(formatspec);
			specPtr = formatspec;
			//
			// store off all the names
			//
			strcpy(argumentNames, argument_names);
			namePtr = strtok(argumentNames, " ");
			while (namePtr != NULL)
			{
				argNames.AddObject(str(namePtr));
				namePtr = strtok(NULL, " ");
			}

			index = 0;

			//
			// create the definition container
			//
			definition = new Container<EventArgDef>;
			definition->Resize(argNames.NumObjects());

			// go throught he formatspec
			while (specLength)
			{
				// clear the rangeSpec
				rangeSpec = "";
				// get the argSpec
				argSpec = "";
				argSpec += *specPtr;
				specPtr++;
				specLength--;
				// see if there is a range specified
				while (*specPtr == '[')
				{
					// add in all the characters until NULL or ']'
					while (specLength && (*specPtr != ']'))
					{
						rangeSpec += *specPtr;
						specPtr++;
						specLength--;
					}
					if (specLength && (*specPtr == ']'))
					{
						rangeSpec += *specPtr;
						specPtr++;
						specLength--;
					}
				}
				if (index < argNames.NumObjects())
				{
					argName = argNames.ObjectAt(index + 1);
					argDef.Setup(name, argName, argSpec, rangeSpec);
					definition->AddObject(argDef);
				}
				else
				{
					assert(0);
					Error("More format specifiers than argument names for event %s\n", name);
				}
				index++;
			}
			if (index < argNames.NumObjects())
			{
				assert(0);
				Error("More argument names than format specifiers for event %s\n", name);
			}
		}
	}
}

CLASS_DECLARATION(ScriptClass, Event, NULL)
{
	{ NULL, NULL }
};

void Event::Archive(Archiver &arc)
{
	/*
	if (arc.Loading())
	{
		fromScript = false;
	}

	Class::Archive(arc);

	arc.ArchiveUnsignedShort(&eventnum);
	arc.ArchiveUnsignedShort(&dataSize);

	if (arc.Loading())
	{
		data = new ScriptVariable[dataSize + 1];
	}

	for (int i = dataSize; i > 0; i--)
	{
		data[i].ArchiveInternal(arc);
	}
	*/
}

Event::Event()
{
	fromScript = false;
	eventnum = 0;
	dataSize = 0;
	data = nullptr;
}

Event::Event(uintptr_t eventnum)
{
	fromScript = false;
	this->eventnum = eventnum;
	dataSize = 0;
	data = nullptr;
}

Event::Event(const char *command, unsigned int flags, const char *formatspec, const char *argument_names, const char *documentation, uint8_t type)
{
	EventDef* e = new EventDef;

	e->ev = this;
	e->command = command;
	e->flags = flags;
	e->formatspec = formatspec;
	e->argument_names = argument_names;
	e->documentation = documentation;
	e->type = type;

	e->next = EventManager::lastEventDef;

	EventManager::lastEventDef = e;

	fromScript = false;
	dataSize = 0;
	data = NULL;
	eventnum = 0;
}

Event::~Event()
{
	Clear();
}

EventDef* Event::GetInfo()
{
	return &GetEventManager()->eventDefList[this];
}

const char* Event::GetName()
{
	return GetEventManager()->GetEventName(eventnum);
}

void Event::AddContainer(Container<SafePtr<Listener>> *container)
{
	ScriptVariable& variable = GetValue();
	variable.setContainerValue(container);
}

void Event::AddEntity(Entity* ent)
{
	ScriptVariable& variable = GetValue();
	//variable.setEntityValue(ent);
}

void Event::AddFloat(float number)
{
	ScriptVariable& variable = GetValue();
	variable.setFloatValue(number);
}

void Event::AddInteger(int number)
{
	ScriptVariable& variable = GetValue();
	variable.setIntValue(number);
}

void Event::AddListener(Listener* listener)
{
	ScriptVariable& variable = GetValue();
	variable.setListenerValue(listener);
}

void Event::AddNil()
{
	ScriptVariable& variable = GetValue();
	variable.Clear();
}

void Event::AddConstString(const_str string)
{
	ScriptVariable& variable = GetValue();
	variable.setConstStringValue(string);
}

void Event::AddString(const str& string)
{
	ScriptVariable& variable = GetValue();
	variable.setStringValue(string);
}

void Event::AddToken(const str& token)
{
	ScriptVariable& variable = GetValue();
	variable.setStringValue(token);
}

void Event::AddTokens(int argc, const char **argv)
{
	for (int i = 0; i < argc; i++)
	{
		AddToken(argv[i]);
	}
}

void Event::AddValue(const ScriptVariable& value)
{
	ScriptVariable& variable = GetValue();
	variable = value;
}

void Event::AddVector(const Vector& vector)
{
	ScriptVariable& variable = GetValue();
	variable.setVectorValue(vector);
}

void Event::Clear(void)
{
	if (data)
	{
		delete[] data;

		data = NULL;
		dataSize = 0;
	}
}

void Event::CheckPos(uintptr_t pos)
{
	if (pos > NumArgs())
	{
		ScriptError("Index %d out of range.", pos);
	}
}

bool Event::GetBoolean(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.booleanNumericValue();
}

int Event::GetConstString(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.constStringValue();
}

Entity* Event::GetEntity(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.entityValue();
}

float Event::GetFloat(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.floatValue();
}

int Event::GetInteger(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.intValue();
}

Listener* Event::GetListener(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.listenerValue();
}

SimpleEntity* Event::GetSimpleEntity(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.simpleEntityValue();
}

str Event::GetString(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.stringValue();
}

str Event::GetToken(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.stringValue();
}

ScriptVariable&	Event::GetValue(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1];
}

ScriptVariable&	Event::GetValue()
{
	ScriptVariable *tmp = data;

	AssetManager* AM = GetAssetManager();

	data = new ScriptVariable[dataSize + 1];

	if (tmp != NULL)
	{
		for (int i = 0; i < dataSize; i++)
		{
			data[i] = tmp[i];
		}

		delete[] tmp;
	}

	dataSize++;

	return data[dataSize - 1];
}

Vector Event::GetVector(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);

	return variable.vectorValue();
}

bool Event::IsEntityAt(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1].IsEntity();
}

bool Event::IsListenerAt(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1].IsListener();
}

bool Event::IsNilAt(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1].GetType() == VARIABLE_NONE;
}

bool Event::IsNumericAt(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1].IsNumeric();
}

bool Event::IsSimpleEntityAt(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1].IsSimpleEntity();
}

bool Event::IsStringAt(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1].IsString();
}

bool Event::IsVectorAt(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1].IsVector();
}

bool Event::IsFromScript(void)
{
	return fromScript;
}

const char* Event::GetFormat()
{
	for (EventDef* e = EventManager::lastEventDef; e != nullptr; e = e->next)
	{
		if (e->ev->eventnum == eventnum)
		{
			return e->formatspec;
		}
	}

	return nullptr;
}

size_t Event::NumArgs()
{
	return dataSize;
}

