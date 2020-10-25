#pragma once

#include "ScriptClass.h"
#include "ContainerClass.h"
#include "../Common/Container.h"
#include "../Common/con_set.h"
#include "../Common/str.h"
#include "../Vector.h"

namespace MOHPC
{
	class ScriptVariable;
	class Archiver;
	class Listener;
	class SimpleEntity;
	class Entity;
	class Vector;

	enum variabletype
	{
		VARIABLE_NONE,
		VARIABLE_STRING,
		VARIABLE_INTEGER,
		VARIABLE_FLOAT,
		VARIABLE_CHAR,
		VARIABLE_CONSTSTRING,
		VARIABLE_LISTENER,
		VARIABLE_REF,
		VARIABLE_ARRAY,
		VARIABLE_CONSTARRAY,
		VARIABLE_CONTAINER,
		VARIABLE_SAFECONTAINER,
		VARIABLE_POINTER,
		VARIABLE_VECTOR,
		VARIABLE_MAX
	};

	static const char *typenames[] =
	{
		"none",
		"string",
		"int",
		"float",
		"char",
		"const string",
		"listener",
		"ref",
		"array",
		"const array",
		"array",
		"array",
		"pointer",
		"vector",
		"double"
	};

	class ScriptArrayHolder : public BaseScriptClass
	{
	public:
		con_map<ScriptVariable, ScriptVariable> arrayValue;
		unsigned int refCount;

	public:
		void			Archive(Archiver& arc);
		static void		Archive(Archiver& arc, ScriptArrayHolder *& arrayValue);
	};

	class ScriptConstArrayHolder : public BaseScriptClass
	{
	public:
		ScriptVariable * constArrayValue;
		unsigned int refCount;
		size_t size;

	public:
		void			Archive(Archiver& arc);
		static void		Archive(Archiver& arc, ScriptConstArrayHolder *& constArrayValue);

		ScriptConstArrayHolder(ScriptVariable *pVar, size_t size);
		ScriptConstArrayHolder(size_t size);
		ScriptConstArrayHolder();
		~ScriptConstArrayHolder();
	};

	class ScriptPointer : public BaseScriptClass
	{
	public:
		Container<ScriptVariable *> list;

	public:
		void			Archive(Archiver& arc);
		static void		Archive(Archiver& arc, ScriptPointer *& pointerValue);

		void		Clear();

		void		add(ScriptVariable *var);
		void		remove(ScriptVariable *var);
		void		setValue(const ScriptVariable& var);
	};

	class ScriptVariable : public BaseScriptClass {
	public:
		const_str		key;		// variable name
		unsigned char	type;		// variable type
		union
		{
		public:
			char						charValue;
			float						floatValue;
			int							intValue;
			SafePtr<Listener>			*listenerValue;
			const_str					constStringValue;
			str							*stringValue;
			float						*vectorValue;

			ScriptVariable				*refValue;

			ScriptArrayHolder			*arrayValue;
			ScriptConstArrayHolder		*constArrayValue;

			Container<SafePtr<Listener>>					*containerValue;
			SafePtr<ContainerClass<SafePtr<Listener>>>		*safeContainerValue;

			ScriptPointer								*pointerValue;
		} m_data;

	private:
		void					ClearInternal();
		void					ClearPointerInternal();

	public:
		ScriptVariable();
		ScriptVariable(const ScriptVariable& variable);

		~ScriptVariable();

		void					Archive(Archiver& arc);
		static void				Archive(Archiver& arc, ScriptVariable **obj);
		void					ArchiveInternal(Archiver& arc);

		void					CastBoolean(void);
		void					CastConstArrayValue(void);
		void					CastEntity(void);
		void					CastFloat(void);
		void					CastInteger(void);
		void					CastString(void);

		void					Clear();
		void					ClearPointer();

		const char				*GetTypeName(void) const;
		variabletype			GetType(void) const;

		bool				IsEntity(void);
		bool				IsListener(void);
		bool				IsNumeric(void);
		bool				IsSimpleEntity(void);
		bool				IsString(void);
		bool				IsVector(void);

		void					PrintValue(void);

		void					SetFalse(void);
		void					SetTrue(void);

		size_t					arraysize(void) const;
		size_t					size(void) const;

		bool					booleanNumericValue(void);
		bool					booleanValue(void) const;

		const str&				getName();

		const_str				GetKey();
		void					SetKey(const_str key);

		Entity					*entityValue(void);

		void					evalArrayAt(ScriptVariable &var);

		void					setArrayAt(ScriptVariable &index, ScriptVariable &value);
		void					setArrayAtRef(ScriptVariable &index, ScriptVariable &value);
		void					setArrayRefValue(ScriptVariable &var);

		char					charValue(void) const;
		void					setCharValue(char newvalue);

		ScriptVariable			*constArrayValue(void);
		void					setConstArrayValue(ScriptVariable *pVar, unsigned int size);

		int						constStringValue(void) const;
		void					setConstStringValue(const_str s);

		void					setContainerValue(Container<SafePtr<Listener>> *newvalue);
		void					setSafeContainerValue(ContainerClass< SafePtr< Listener > > *newvalue);

		float					floatValue(void) const;
		void					setFloatValue(float newvalue);

		int						intValue(void) const;
		void					setIntValue(int newvalue);

		Listener				*listenerValue(void) const;
		void					setListenerValue(Listener * newvalue);

		void					newPointer(void);
		void					setPointer(const ScriptVariable& newvalue);

		void					setRefValue(ScriptVariable * ref);

		//const char				*stringValue( void );
		str						stringValue(void) const;
		void					setStringValue(const str& newvalue);

		SimpleEntity			*simpleEntityValue(void) const;

		Vector					vectorValue(void) const;
		void					setVectorValue(const Vector &newvector);

		void					greaterthan(ScriptVariable &variable);
		void					greaterthanorequal(ScriptVariable &variable);
		void					lessthan(ScriptVariable &variable);
		void					lessthanorequal(ScriptVariable &variable);

		void					complement(void);
		void					minus(void);
		bool					operator=(const ScriptVariable& variable);
		ScriptVariable			&operator[](ScriptVariable& index);
		ScriptVariable			*operator[](uintptr_t index) const;
		ScriptVariable			*operator*();
		void					operator+=(const ScriptVariable& value);
		void					operator-=(const ScriptVariable& value);
		void					operator*=(const ScriptVariable& value);
		void					operator/=(const ScriptVariable& value);
		void					operator%=(const ScriptVariable& value);
		void					operator&=(const ScriptVariable& value);
		void					operator^=(const ScriptVariable& value);
		void					operator|=(const ScriptVariable& value);
		void					operator<<=(const ScriptVariable& value);
		void					operator>>=(const ScriptVariable& value);

		bool					operator!=(const ScriptVariable& value);
		bool					operator==(const ScriptVariable& value);

		ScriptVariable			operator++(int);
		ScriptVariable			operator--(int);
	};

	class ScriptVariableList : public BaseScriptClass
	{
	private:
		con_set<const_str, ScriptVariable> list;

	public:
		ScriptVariableList();

		virtual void	Archive(Archiver &arc);

		void			ClearList(void);

		ScriptVariable *GetOrCreateVariable(const str& name);
		ScriptVariable *GetOrCreateVariable(const_str name);

		ScriptVariable *GetVariable(const str& name);
		ScriptVariable *GetVariable(const_str name);

		ScriptVariable *SetVariable(const char *name, int value);
		ScriptVariable *SetVariable(const char *name, float value);
		ScriptVariable *SetVariable(const char *name, const char *value);
		ScriptVariable *SetVariable(const char *name, Entity *value);
		ScriptVariable *SetVariable(const char *name, Listener *value);
		ScriptVariable *SetVariable(const char *name, Vector &value);
		ScriptVariable *SetVariable(const char *name, ScriptVariable& value);
		ScriptVariable *SetVariable(const_str name, ScriptVariable& value);
	};
};
