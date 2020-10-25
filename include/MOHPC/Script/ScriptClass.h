#pragma once

#include "../Class.h"
#include "../Common/Container.h"
#include "../Common/str.h"
#include "../Common/con_arrayset.h"

namespace MOHPC
{
	class ScriptClass;
	class Event;

	typedef uint32_t const_str;

#define isSubclassOf(classname) inheritsFrom(&classname::ClassInfo)
#define isSuperclassOf(classname) isInheritedBy(&classname::ClassInfo)

#define CLASS_DECLARATION(parentclass, classname, classid)		\
	ClassDef classname::ClassInfo								\
	(															\
		#classname, classid, #parentclass,						\
		(ResponseDef<Class> *)classname::Responses,				\
		classname::_newInstance, sizeof( classname )			\
	);															\
	void *classname::_newInstance( void )						\
	{															\
		return new classname;									\
	}															\
	ClassDef *classname::classinfo( void ) const				\
	{															\
		return &(classname::ClassInfo);							\
	}															\
	ClassDef *classname::classinfostatic( void )				\
	{															\
		return &(classname::ClassInfo);							\
	}															\
	void classname::AddWaitTill( const str& s )			\
	{															\
		classname::ClassInfo.AddWaitTill( s );					\
	}															\
	void classname::AddWaitTill( const_str s )					\
	{															\
		classname::ClassInfo.AddWaitTill( s );					\
	}															\
	void classname::RemoveWaitTill( const str& s )		\
	{															\
		classname::ClassInfo.RemoveWaitTill( s );				\
	}															\
	void classname::RemoveWaitTill( const_str s )				\
	{															\
		classname::ClassInfo.RemoveWaitTill( s );				\
	}															\
	bool classname::WaitTillDefined( const str& s )		\
	{															\
		return classname::ClassInfo.WaitTillDefined( s );		\
	}															\
	bool classname::WaitTillDefined( const_str s )				\
	{															\
		return classname::ClassInfo.WaitTillDefined( s );		\
	}															\
	ResponseDef<classname> classname::Responses[] =

#define CLASS_PROTOTYPE( classname )											\
	public:																		\
	static	ClassDef				ClassInfo;									\
	static	void					*_newInstance( void );						\
	static	ClassDef				*classinfostatic(void);						\
	virtual	ClassDef				*classinfo(void) const;						\
	static void						AddWaitTill( const str& s );		\
	static void						AddWaitTill( const_str s );					\
	static void						RemoveWaitTill( const str& s );		\
	static void						RemoveWaitTill( const_str s );				\
	static bool						WaitTillDefined( const str& s );	\
	static bool						WaitTillDefined( const_str s );				\
	static	ResponseDef<classname>	Responses[]

	typedef void (Class::*Response)(Event *ev);

	class EventDef;

	template< class Type >
	struct ResponseDef
	{
		Event			*event;
		void			(Type::*response)(Event *ev);
		EventDef		*def;
	};

	class ClassDef
	{
	public:
		const char *classname;
		const char *classID;
		const char *superclass;
		void *(*newInstance)(void);
		int classSize;
		ResponseDef<Class> *responses;
		ResponseDef<Class> **responseLookup;
		ClassDef *super;
		ClassDef *next;
		ClassDef *prev;

		con_arrayset<const_str, const_str> *waitTillSet;

		size_t numEvents;

		static ClassDef *classlist;
		static size_t numclasses;

		static void BuildEventResponses();
		static void ClearEventResponses();

		void BuildResponseList();
		void ClearResponseList();

		static size_t dump_numclasses;
		static size_t dump_numevents;
		static Container<intptr_t> sortedList;
		static Container<ClassDef *> sortedClassList;

		ClassDef();
		~ClassDef();

		static int compareClasses(const void *arg1, const void *arg2);
		static void SortClassList(Container<ClassDef *> *sortedList);

		void AddWaitTill(const str& s);
		void AddWaitTill(const_str s);
		void RemoveWaitTill(const str& s);
		void RemoveWaitTill(const_str s);
		bool WaitTillDefined(const str& s);
		bool WaitTillDefined(const_str s);

		/* Create-a-class function */
		ClassDef(const char *classname, const char *classID, const char *superclass, ResponseDef<Class> *responses,
			void *(*newInstance)(void), int classSize);

		EventDef *GetDef(uintptr_t eventnum);
		int GetFlags(Event *event);

		void Destroy();
	};

	bool checkInheritance(const ClassDef *superclass, const ClassDef *subclass);
	bool checkInheritance(ClassDef *superclass, const char *subclass);
	bool checkInheritance(const char *superclass, const char *subclass);
	void CLASS_Print(FILE *class_file, const char *fmt, ...);
	void ClassEvents(const char *classname, bool print_to_disk);
	void DumpClass(FILE * class_file, const char * className);
	void DumpAllClasses(void);

	class ClassDefHook
	{
	private:
		ClassDef * classdef;

	public:
		//void * operator new( size_t );
		//void operator delete( void * );

		ClassDefHook();
		~ClassDefHook();

		/* Hook-a-class function */
		ClassDefHook(ClassDef * classdef, ResponseDef<Class> *responses);
	};

	ClassDef *GetClassForID(const char *name);
	ClassDef *GetClass(const char * name);
	ClassDef *GetClassList(void);
	void listAllClasses(void);
	void listInheritanceOrder(const char *classname);

	class Archiver;

	class BaseScriptClass : public Class
	{
	public:
		SharedPtr<class GameManager> GetGameManager() const;
		SharedPtr<class EventManager> GetEventManager() const;
		SharedPtr<class ScriptManager> GetScriptManager() const;
	};

	class ScriptClass : public BaseScriptClass
	{
	public:
		CLASS_PROTOTYPE(ScriptClass);

		virtual void Archive(Archiver& arc);

		const char *GetClassID(void) const;
		const char *GetClassname(void) const;
		const char *getSuperclass(void) const;

		bool inheritsFrom(ClassDef *c) const;
		bool inheritsFrom(const char * name) const;
		bool isInheritedBy(const char * name) const;
		bool isInheritedBy(ClassDef *c) const;

		void warning(const char *function, const char *format, ...);
		void error(const char *function, const char *format, ...);
	};
};
