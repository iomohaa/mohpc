#pragma once

#include "ScriptClass.h"
#include "Container.h"

namespace MOHPC
{
	// Event flags
#define EV_CONSOLE				(1<<0)		// Allow entry from console
#define EV_CHEAT				(1<<1)		// Only allow entry from console if cheats are enabled
#define EV_CODEONLY				(1<<2)		// Hide from eventlist
#define EV_CACHE				(1<<3)		// This event is used to cache data in
#define EV_TIKIONLY				(1<<4)		// This command only applies to TIKI files
#define EV_SCRIPTONLY			(1<<5)		// This command only applies to SCRIPT files
#define EV_SERVERCMD			(1<<6)		// Client : server command
#define EV_DEFAULT				-1			// default flag

// Event types
#define EV_NORMAL				0	// Normal command
#define EV_RETURN				1	// Return as a function (local.var = local ReturnCommand)
#define EV_GETTER				2	// Return as a variable (local.var = local.listener.some_getter)
#define EV_SETTER				3	// Set as a variable (local.listener.some_setter = "value")

// times for posting events
// Even though negative times technically don't make sense, the effect is to 
// sort events that take place at the start of a map so that they are executed
// in the proper order.  For example, spawnargs must occur before any script
// commands take place, while unused entities must be removed before the spawnargs
// are parsed.

#define EV_REMOVE					-12.0f		// remove any unused entities before spawnargs are parsed
#define EV_PRIORITY_SPAWNARG		-11.0f		// for priority spawn args passed in by the bsp file
#define EV_SPAWNARG					-10.0f		// for spawn args passed in by the bsp file
#define EV_PROCESS_INIT				-6.0f
#define EV_POSTSPAWN				-5.0f		// for any processing that must occur after all objects are spawned
#define EV_SPAWNENTITIES			-4.0f

	class Listener;
	class SimpleEntity;
	class Entity;
	class ScriptVariable;
	class Vector;
	class EventQueueNode;

	typedef enum
	{
		IS_STRING,
		IS_VECTOR,
		IS_BOOLEAN,
		IS_INTEGER,
		IS_FLOAT,
		IS_ENTITY,
		IS_LISTENER
	} vartype;

	class EventArgDef : public Class
	{
	private:
		vartype type;
		str name;
		float minRange[3];
		bool minRangeDefault[3];
		float maxRange[3];
		bool maxRangeDefault[3];
		bool optional;
	public:

		EventArgDef()
		{
			type = vartype::IS_INTEGER;
			//name        = "undefined";
			optional = false;
		};
		void              Setup(const char * eventName, const char *argName, const char *argType, const char *argRange);
		void              PrintArgument(FILE *event_file = NULL);
		void              PrintRange(FILE *event_file = NULL);
		int               getType(void);
		const char        *getName(void);
		bool          isOptional(void);

		float GetMinRange(int index)
		{
			if (index < 3)
				return minRange[index];
			return 0.0;
		}

		bool GetMinRangeDefault(int index)
		{
			if (index < 3)
				return minRangeDefault[index];
			return false;
		}

		float GetMaxRange(int index)
		{
			if (index < 3)
				return maxRange[index];
			return 0.0;
		}

		bool GetMaxRangeDefault(int index)
		{
			if (index < 3)
				return maxRangeDefault[index];
			return false;
		}
	};

	inline int EventArgDef::getType
	(
		void
	)

	{
		return type;
	}

	inline const char *EventArgDef::getName
	(
		void
	)

	{
		return name.c_str();
	}

	inline bool EventArgDef::isOptional
	(
		void
	)

	{
		return optional;
	}

	class EventDef
	{
	public:
		Event* ev;
		str command;
		int flags;
		const char *formatspec;
		const char *argument_names;
		const char *documentation;
		uint8_t type;
		Container<EventArgDef> *definition;
		EventDef* next;

		EventDef() { definition = NULL; }

		void Error(const char *format, ...);

		void PrintDocumentation(FILE *event_file, bool html);
		void PrintEventDocumentation(FILE *event_file, bool html);

		void DeleteDocumentation(void);
		void SetupDocumentation(void);
	};

	class command_t {
	public:
		str command;
		int flags;
		uint8_t type;
		uintptr_t index;

		friend bool		operator==(const str &name, const command_t &command);
		friend bool		operator==(const command_t &cmd1, const command_t &cmd2);
	};

	inline bool operator==(const str &name, const command_t &command)
	{
		return command.command == name;
	}

	inline bool operator==(const command_t &cmd1, const command_t &cmd2)
	{
		return (cmd2.command == cmd1.command && (cmd2.type == (uint8_t)-1 || cmd2.type == cmd1.type));
	}

	class Event : public ScriptClass
	{
		friend class ScriptVM;

	public:
		bool fromScript;
		uintptr_t eventnum;
		short unsigned int dataSize;
		ScriptVariable *data;

#ifdef _DEBUG
		// should be used only for debugging purposes
		str name;
#endif

	public:
		CLASS_PROTOTYPE(Event);

		bool operator==(Event ev) { return eventnum == ev.eventnum; }
		bool operator!=(Event ev) { return eventnum != ev.eventnum; }

		Event();
		Event(uintptr_t eventnum);
		Event
		(
			const char *command,
			unsigned int flags,
			const char *formatspec,		// Arguments are : 'e' (Entity) 'v' (Vector) 'i' (Integer) 'f' (Float) 's' (String) 'b' (Boolean).
										// Uppercase arguments means optional.
			const char *argument_names,
			const char *documentation,
			uint8_t type = EV_NORMAL
		);

		virtual ~Event();

		virtual void Archive(Archiver &arc);

		EventDef *GetInfo();
		const char *GetName();

		void AddContainer(Container<SafePtr<Listener>> *container);
		void AddEntity(Entity* ent);
		void AddFloat(float number);
		void AddInteger(int number);
		void AddListener(Listener* listener);
		void AddNil();
		void AddConstString(const_str string);
		void AddString(const str& string);
		void AddToken(const str& token);
		void AddTokens(int argc, const char **argv);
		void AddValue(const ScriptVariable& value);
		void AddVector(const Vector& vector);

		void Clear(void);

		void CheckPos(uintptr_t pos);

		bool GetBoolean(uintptr_t pos);

		int GetConstString(uintptr_t pos);

		Entity *GetEntity(uintptr_t pos);

		float GetFloat(uintptr_t pos);
		int GetInteger(uintptr_t pos);
		Listener *GetListener(uintptr_t pos);

		SimpleEntity *GetSimpleEntity(uintptr_t pos);

		str GetString(uintptr_t pos);
		str GetToken(uintptr_t pos);
		ScriptVariable&	GetValue(uintptr_t pos);
		ScriptVariable&	GetValue(void);
		Vector GetVector(uintptr_t pos);

		bool IsEntityAt(uintptr_t pos);
		bool IsListenerAt(uintptr_t pos);
		bool IsNilAt(uintptr_t pos);
		bool IsNumericAt(uintptr_t pos);
		bool IsSimpleEntityAt(uintptr_t pos);
		bool IsStringAt(uintptr_t pos);
		bool IsVectorAt(uintptr_t pos);

		bool IsFromScript(void);

		const char *GetFormat();
		int NumArgs();
	};

#define					NODE_CANCEL				1
#define					NODE_FIXED_EVENT		2

	class EventQueueNode {
	public:
		Event * event;
		double time;
		int flags;
		SafePtr<Listener> m_sourceobject;

		EventQueueNode		*prev;
		EventQueueNode		*next;

#ifdef _DEBUG
		str name;
#endif

		EventQueueNode() { prev = this; next = this; }
		Listener	*GetSourceObject(void) { return m_sourceobject; }
		void		SetSourceObject(Listener *obj) { m_sourceobject = obj; }
	};
};
