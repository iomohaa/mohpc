#include <Shared.h>
#include <MOHPC/Script/ScriptVariable.h>
#include <MOHPC/Script/ScriptException.h>
#include <MOHPC/Script/Listener.h>
#include <MOHPC/Script/World.h>
#include <MOHPC/Script/SimpleEntity.h>
#include <MOHPC/Script/ConstStr.h>
#include <MOHPC/Script/Archiver.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/GameManager.h>
#include <MOHPC/Managers/ScriptManager.h>

using namespace MOHPC;

template<>
intptr_t MOHPC::HashCode< ScriptVariable >(const ScriptVariable& key)
{
	Listener *l;

	switch (key.GetType())
	{
	case VARIABLE_STRING:
	case VARIABLE_CONSTSTRING:
		return HashCode< str >(key.stringValue());

	case VARIABLE_INTEGER:
		return key.m_data.intValue;

	case VARIABLE_LISTENER:
		l = key.listenerValue();
		return (intptr_t)l;

	default:
		ScriptError("Bad hash code value: %s", key.stringValue().c_str());
	}
}

void ScriptArrayHolder::Archive(Archiver& arc)
{
	/*
	arc.ArchiveUnsigned(&refCount);
	arrayValue.Archive(arc);
	*/
}

void ScriptArrayHolder::Archive(Archiver& arc, ScriptArrayHolder *& arrayValue)
{
	/*
	bool newRef;

	if (arc.Loading())
	{
		arc.ArchiveBoolean(&newRef);
	}
	else
	{
		newRef = !arc.ObjectPositionExists(arrayValue);
		arc.ArchiveBoolean(&newRef);
	}

	if (newRef)
	{
		if (arc.Loading())
		{
			arrayValue = GetAssetManager()->NewObject<ScriptArrayHolder>();
		}

		arc.ArchiveObjectPosition(arrayValue);
		arrayValue->Archive(arc);
		return;
	}
	else
	{
		arc.ArchiveObjectPointer((Class **)&arrayValue);
	}
	*/
}


void ScriptConstArrayHolder::Archive(Archiver& arc)
{
	/*
	arc.ArchiveUnsigned(&refCount);
	arc.ArchiveUnsigned(&size);

	if (arc.Loading())
	{
		constArrayValue = new ScriptVariable[size + 1] - 1;
	}

	for (int i = 1; i <= size; i++)
	{
		constArrayValue[i].ArchiveInternal(arc);
	}
	*/
}

void ScriptConstArrayHolder::Archive(Archiver& arc, ScriptConstArrayHolder *& constArrayValue)
{
	/*
	bool newRef;

	if (arc.Loading())
	{
		arc.ArchiveBoolean(&newRef);
	}
	else
	{
		newRef = !arc.ObjectPositionExists(constArrayValue);
		arc.ArchiveBoolean(&newRef);
	}

	if (newRef)
	{
		if (arc.Loading())
		{
			constArrayValue = GetAssetManager()->NewObject<ScriptConstArrayHolder>();
		}

		arc.ArchiveObjectPosition(constArrayValue);
		constArrayValue->Archive(arc);
		return;
	}
	else
	{
		arc.ArchiveObjectPointer((Class **)&constArrayValue);
	}
	*/
}

ScriptConstArrayHolder::ScriptConstArrayHolder(ScriptVariable *pVar, size_t size)
{
	refCount = 0;
	this->size = size;

	constArrayValue = new ScriptVariable[size] - 1;

	for (size_t i = 1; i <= size; i++)
	{
		constArrayValue[i] = pVar[i];
	}
}

ScriptConstArrayHolder::ScriptConstArrayHolder(size_t size)
{
	refCount = 0;
	this->size = size;

	constArrayValue = new ScriptVariable[size] - 1;
}

ScriptConstArrayHolder::ScriptConstArrayHolder()
{
	refCount = 0;
	size = 0;
	constArrayValue = NULL;
}

ScriptConstArrayHolder::~ScriptConstArrayHolder()
{
	if (constArrayValue)
	{
		delete[] (constArrayValue + 1);
	}
}


void ScriptPointer::Archive(Archiver& arc)
{
	//list.Archive(arc, ScriptVariable::Archive);
}

void ScriptPointer::Archive(Archiver& arc, ScriptPointer *& pointerValue)
{
	/*
	bool newRef;

	if (arc.Loading())
	{
		arc.ArchiveBoolean(&newRef);
	}
	else
	{
		newRef = !arc.ObjectPositionExists(pointerValue);
		arc.ArchiveBoolean(&newRef);
	}

	if (newRef)
	{
		if (arc.Loading())
		{
			pointerValue = GetAssetManager()->NewObject<ScriptPointer>();
		}

		arc.ArchiveObjectPosition(pointerValue);
		pointerValue->Archive(arc);
		return;
	}
	else
	{
		arc.ArchiveObjectPointer((Class **)&pointerValue);
	}
	*/
}

void ScriptPointer::Clear()
{
	for (int i = 0; i < list.NumObjects(); i++)
	{
		ScriptVariable *variable = list[i];

		variable->type = 0;
	}

	delete this;
}

void ScriptPointer::add(ScriptVariable *var)
{
	list.AddObject(var);
}

void ScriptPointer::remove(ScriptVariable *var)
{
	list.RemoveObject(var);

	if (!list.NumObjects())
	{
		delete this;
	}
}

void ScriptPointer::setValue(const ScriptVariable& var)
{
	ScriptVariable *pVar;

	for (uintptr_t i = list.NumObjects(); i > 0; i--)
	{
		pVar = list.ObjectAt(i);

		pVar->type = 0;
		*pVar = var;
	}

	delete this;
}

ScriptVariable::ScriptVariable()
{
	key = 0;
	type = 0;
	m_data.pointerValue = NULL;
}

ScriptVariable::ScriptVariable(const ScriptVariable& variable)
	: BaseScriptClass(variable)
{
	type = 0;
	m_data.pointerValue = NULL;

	*this = variable;
}

ScriptVariable::~ScriptVariable()
{
	ClearInternal();
}

void ScriptVariable::Archive(Archiver& arc)
{
	/*
	const_str s;

	if (arc.Loading())
	{
		GetScriptManager()->ArchiveString(arc, s);
		key = s;
	}
	else
	{
		s = key;
		GetScriptManager()->ArchiveString(arc, s);
	}
	*/

	ArchiveInternal(arc);
}

void ScriptVariable::Archive(Archiver& arc, ScriptVariable **obj)
{
	//arc.ArchiveObjectPointer((Class **)obj);
}

void ScriptVariable::ArchiveInternal(Archiver& arc)
{
	/*
	arc.ArchiveObjectPosition(this);

	arc.ArchiveByte(&type);
	switch (type)
	{
	case VARIABLE_STRING:
		if (arc.Loading())
		{
			m_data.stringValue = new str(4);
		}

		arc.ArchiveString(m_data.stringValue);
		break;

	case VARIABLE_INTEGER:
		arc.ArchiveInteger(&m_data.intValue);
		break;

	case VARIABLE_FLOAT:
		arc.ArchiveFloat(&m_data.floatValue);
		break;

	case VARIABLE_CHAR:
		arc.ArchiveChar(&m_data.charValue);
		break;

	case VARIABLE_CONSTSTRING:
		if (arc.Loading())
		{
			str s;
			arc.ArchiveString(&s);
			m_data.intValue = GetScriptManager()->AddString(s);
		}
		else
		{
			str s = GetScriptManager()->GetString(m_data.intValue);
			arc.ArchiveString(&s);
		}
		break;

	case VARIABLE_LISTENER:
		if (arc.Loading())
		{
			m_data.listenerValue = new SafePtr < Listener >;
		}

		arc.ArchiveSafePointer(m_data.listenerValue);
		break;

	case VARIABLE_ARRAY:
		ScriptArrayHolder::Archive(arc, m_data.arrayValue);
		break;

	case VARIABLE_CONSTARRAY:
		ScriptConstArrayHolder::Archive(arc, m_data.constArrayValue);
		break;

	case VARIABLE_REF:
	case VARIABLE_CONTAINER:
		arc.ArchiveObjectPointer((Class **)&m_data.refValue);
		break;

	case VARIABLE_SAFECONTAINER:
		if (arc.Loading())
		{
			m_data.safeContainerValue = new SafePtr < ConList >;
		}

		arc.ArchiveSafePointer(m_data.safeContainerValue);
		break;

	case VARIABLE_POINTER:
		ScriptPointer::Archive(arc, m_data.pointerValue);
		break;

	case VARIABLE_VECTOR:
		if (arc.Loading())
		{
			m_data.vectorValue = new float[3];
		}

		arc.ArchiveVec3(m_data.vectorValue);
		break;

	default:
		break;
	}
	*/
}

void ScriptVariable::CastBoolean(void)
{
	int newvalue = booleanValue();

	ClearInternal();

	type = VARIABLE_INTEGER;
	m_data.intValue = newvalue;
}

void ScriptVariable::CastConstArrayValue(void)
{
	con_map_enum< ScriptVariable, ScriptVariable > en;
	ScriptConstArrayHolder *constArrayValue;
	ScriptVariable *value;
	int i;
	ConList *listeners;

	switch (GetType())
	{
	case VARIABLE_POINTER:
		ClearPointerInternal();
	case VARIABLE_NONE:
		ScriptError("cannot cast NIL to an array");

	case VARIABLE_CONSTARRAY:
		return;

	case VARIABLE_ARRAY:
		constArrayValue = new ScriptConstArrayHolder(m_data.arrayValue->arrayValue.size());

		en = m_data.arrayValue->arrayValue;

		i = 0;

		for (value = en.NextValue(); value != NULL; value = en.NextValue())
		{
			i++;
			constArrayValue->constArrayValue[i] = *value;
		}

		break;

	case VARIABLE_CONTAINER:
		constArrayValue = new ScriptConstArrayHolder(m_data.containerValue->NumObjects());

		for (uintptr_t i = m_data.containerValue->NumObjects(); i > 0; i--)
		{
			constArrayValue->constArrayValue[i].setListenerValue(m_data.containerValue->ObjectAt(i));
		}
		break;

	case VARIABLE_SAFECONTAINER:
		listeners = *m_data.safeContainerValue;

		if (listeners)
		{
			constArrayValue = new ScriptConstArrayHolder(listeners->NumObjects());

			for (intptr_t i = listeners->NumObjects(); i > 0; i--)
			{
				constArrayValue->constArrayValue[i].setListenerValue(listeners->ObjectAt(i));
			}
		}
		else
		{
			constArrayValue = new ScriptConstArrayHolder(0);
		}
		break;

	default:
		constArrayValue = new ScriptConstArrayHolder(1);
		constArrayValue->constArrayValue[1] = *this;

		break;
	}

	ClearInternal();
	type = VARIABLE_CONSTARRAY;
	m_data.constArrayValue = constArrayValue;
}

void ScriptVariable::CastEntity(void)
{
	setListenerValue((Listener *)entityValue());
}

void ScriptVariable::CastFloat(void)
{
	setFloatValue(floatValue());
}

void ScriptVariable::CastInteger(void)
{
	setIntValue(intValue());
}

void ScriptVariable::CastString(void)
{
	setStringValue(stringValue());
}

void ScriptVariable::Clear()
{
	ClearInternal();
	type = 0;
}

void ScriptVariable::ClearInternal()
{
	switch (GetType())
	{
	case VARIABLE_STRING:
		if (m_data.stringValue)
		{
			delete m_data.stringValue;
			m_data.stringValue = NULL;
		}

		break;

	case VARIABLE_ARRAY:
		if (m_data.arrayValue->refCount)
		{
			m_data.arrayValue->refCount--;
		}
		else
		{
			delete m_data.arrayValue;
		}

		m_data.arrayValue = NULL;
		break;

	case VARIABLE_CONSTARRAY:
		if (m_data.constArrayValue->refCount)
		{
			m_data.constArrayValue->refCount--;
		}
		else
		{
			delete m_data.constArrayValue;
		}

		m_data.constArrayValue = NULL;
		break;

	case VARIABLE_LISTENER:
	case VARIABLE_SAFECONTAINER:
		if (m_data.listenerValue)
		{
			delete m_data.listenerValue;
			m_data.listenerValue = NULL;
		}

		break;

	case VARIABLE_POINTER:
		m_data.pointerValue->remove(this);
		m_data.pointerValue = NULL;
		break;

	case VARIABLE_VECTOR:
		delete[] m_data.vectorValue;
		m_data.vectorValue = NULL;
		break;

	default:
		break;
	}
}

void ScriptVariable::ClearPointer()
{
	if (type == VARIABLE_POINTER) {
		return ClearPointerInternal();
	}
}

void ScriptVariable::ClearPointerInternal()
{
	type = VARIABLE_NONE;

	m_data.pointerValue->Clear();
	m_data.pointerValue = NULL;
}

const char *ScriptVariable::GetTypeName() const
{
	return typenames[type];
}

variabletype ScriptVariable::GetType() const
{
	return (variabletype)type;
}

bool ScriptVariable::IsEntity(void)
{
	if (type == VARIABLE_LISTENER)
	{
		if (!m_data.listenerValue->Pointer() ||
#if defined ( CGAME_DLL ) || defined ( GAME_DLL )
			checkInheritance(Entity::classinfostatic(), m_data.listenerValue->Pointer()->classinfo())
#else
			0
#endif
			)
		{
			return true;
		}
	}

	return false;
}

bool ScriptVariable::IsListener(void)
{
	return type == VARIABLE_LISTENER;
}

bool ScriptVariable::IsNumeric(void)
{
	return type == VARIABLE_INTEGER || type == VARIABLE_FLOAT;
}

bool ScriptVariable::IsSimpleEntity(void)
{
	if (type == VARIABLE_LISTENER)
	{
		/*
		if (!m_data.listenerValue->Pointer() ||
			checkInheritance(&SimpleEntity::ClassInfo, m_data.listenerValue->Pointer()->classinfo())
			)
		{
			return true;
		}
		*/
	}

	return false;
}

bool ScriptVariable::IsString(void)
{
	return (type == VARIABLE_STRING || type == VARIABLE_CONSTSTRING);
}

bool ScriptVariable::IsVector(void)
{
	return type == VARIABLE_VECTOR;
}

void ScriptVariable::PrintValue(void)
{
	switch (GetType())
	{
	case VARIABLE_NONE:
		printf("");
		break;

	case VARIABLE_CONSTSTRING:
		printf(GetScriptManager()->GetString(m_data.constStringValue));
		break;

	case VARIABLE_STRING:
		printf(m_data.stringValue->c_str());
		break;

	case VARIABLE_INTEGER:
		printf("%d", m_data.intValue);
		break;

	case VARIABLE_FLOAT:
		printf("%f", m_data.floatValue);
		break;

	case VARIABLE_CHAR:
		printf("%c", m_data.charValue);
		break;

	case VARIABLE_LISTENER:
		printf("<Listener>%p", m_data.listenerValue->Pointer());
		break;

	case VARIABLE_REF:
	case VARIABLE_ARRAY:
	case VARIABLE_CONSTARRAY:
	case VARIABLE_CONTAINER:
	case VARIABLE_SAFECONTAINER:
	case VARIABLE_POINTER:
		printf("type: %s", GetTypeName());
		break;

	case VARIABLE_VECTOR:
		printf("( %f %f %f )", m_data.vectorValue[0], m_data.vectorValue[1], m_data.vectorValue[2]);
		break;
	}
}

void ScriptVariable::SetFalse(void)
{
	setIntValue(0);
}

void ScriptVariable::SetTrue(void)
{
	setIntValue(1);
}

size_t ScriptVariable::arraysize(void) const
{
	switch (GetType())
	{
	case VARIABLE_POINTER:
		m_data.pointerValue->Clear();
		delete m_data.pointerValue;

		return -1;

	case VARIABLE_NONE:
		return -1;

	case VARIABLE_ARRAY:
		return m_data.arrayValue->arrayValue.size();

	case VARIABLE_CONSTARRAY:
		return m_data.constArrayValue->size;

	case VARIABLE_CONTAINER:
		return m_data.containerValue->NumObjects();

	case VARIABLE_SAFECONTAINER:
		if (*m_data.safeContainerValue)
		{
			return (*m_data.safeContainerValue)->NumObjects();
		}
		else
		{
			return 0;
		}

	default:
		return 1;
	}

	return 0;
}

size_t ScriptVariable::size(void) const
{
	switch (GetType())
	{
	case VARIABLE_POINTER:
		m_data.pointerValue->Clear();
		delete m_data.pointerValue;

		return -1;

	case VARIABLE_CONSTSTRING:
	case VARIABLE_STRING:
		return stringValue().length();

	case VARIABLE_LISTENER:
		return *m_data.listenerValue != NULL;

	case VARIABLE_ARRAY:
		return m_data.arrayValue->arrayValue.size();

	case VARIABLE_CONSTARRAY:
		return m_data.constArrayValue->size;

	case VARIABLE_CONTAINER:
		return m_data.containerValue->NumObjects();

	case VARIABLE_SAFECONTAINER:
		if (*m_data.safeContainerValue)
		{
			return (*m_data.safeContainerValue)->NumObjects();
		}
		else
		{
			return 0;
		}

	default:
		return 1;
	}

	return 0;
}

bool ScriptVariable::booleanNumericValue(void)
{
	str value;

	switch (GetType())
	{
	case VARIABLE_STRING:
	case VARIABLE_CONSTSTRING:
		value = stringValue();

		return atoi(value.c_str()) ? true : false;

	case VARIABLE_INTEGER:
		return m_data.intValue != 0;

	case VARIABLE_FLOAT:
		return ::fabs(m_data.floatValue) >= 0.00009999999747378752;

	case VARIABLE_LISTENER:
		return (*m_data.listenerValue) != NULL;

	default:
		ScriptError("Cannot cast '%s' to boolean numeric", GetTypeName());
	}

	return true;
}

bool ScriptVariable::booleanValue(void) const
{
	switch (GetType())
	{
	case VARIABLE_NONE:
		return false;

	case VARIABLE_STRING:
		if (m_data.stringValue)
		{
			return m_data.stringValue->length() != 0;
		}

		return false;

	case VARIABLE_INTEGER:
		return m_data.intValue != 0;

	case VARIABLE_FLOAT:
		return ::fabs(m_data.floatValue) >= 0.00009999999747378752;

	case VARIABLE_CONSTSTRING:
		return m_data.intValue != STRING_EMPTY;

	case VARIABLE_LISTENER:
		return (*m_data.listenerValue) != NULL;

	default:
		return true;
	}
}

char ScriptVariable::charValue(void) const
{
	str value;

	switch (GetType())
	{
	case VARIABLE_CHAR:
		return m_data.charValue;

	case VARIABLE_CONSTSTRING:
	case VARIABLE_STRING:
		value = stringValue();

		if (value.length() != 1)
		{
			ScriptError("Cannot cast string not of length 1 to char");
		}

		return *value;

	default:
		ScriptError("Cannot cast '%s' to char", GetTypeName());
	}

	return 0;
}

ScriptVariable *ScriptVariable::constArrayValue(void)
{
	return m_data.constArrayValue->constArrayValue;
}

#ifndef NO_SCRIPTENGINE

str getname_null = "";

const str& ScriptVariable::getName(void)
{
	//return GetScriptManager()->GetString(GetKey());
	return getname_null;
}

const_str ScriptVariable::GetKey()
{
	return key;
}

void ScriptVariable::SetKey(const_str key)
{
	this->key = key;
}

#endif

Entity *ScriptVariable::entityValue(void)
{
#if defined ( GAME_DLL ) || defined ( CGAME_DLL )
	return (Entity *)listenerValue();
#else
	return NULL;
#endif
}

void ScriptVariable::evalArrayAt(ScriptVariable &var)
{
	size_t index;
	str string;
	ScriptVariable *array;

	switch (GetType())
	{
	case VARIABLE_VECTOR:
		index = var.intValue();

		if (index > 2)
		{
			Clear();
			ScriptError("Vector index '%d' out of range", index);
		}

		return setFloatValue(m_data.vectorValue[index]);

	case VARIABLE_NONE:
		break;

	case VARIABLE_CONSTSTRING:
	case VARIABLE_STRING:
		index = var.intValue();
		string = stringValue();

		if (index >= string.length())
		{
			Clear();
			ScriptError("String index %d out of range", index);
		}

		return setCharValue(string[index]);

	case VARIABLE_LISTENER:
		index = var.intValue();

		if (index != 1)
		{
			Clear();
			ScriptError("array index %d out of range", index);
		}

		break;

	case VARIABLE_ARRAY:
		array = m_data.arrayValue->arrayValue.find(var);

		if (array)
		{
			*this = *array;
		}
		else
		{
			Clear();
		}

		break;

	case VARIABLE_CONSTARRAY:
		index = var.intValue();

		if (!index || index > m_data.constArrayValue->size)
		{
			ScriptError("array index %d out of range", index);
		}

		*this = m_data.constArrayValue->constArrayValue[index];
		break;

	case VARIABLE_CONTAINER:
		index = var.intValue();

		if (!index || index > m_data.constArrayValue->size)
		{
			ScriptError("array index %d out of range", index);
		}

		setListenerValue(m_data.containerValue->ObjectAt(index));
		break;

	case VARIABLE_SAFECONTAINER:
		index = var.intValue();

		if (!*m_data.safeContainerValue || !index || index > m_data.constArrayValue->size)
		{
			ScriptError("array index %d out of range", index);
		}

		setListenerValue((*m_data.safeContainerValue)->ObjectAt(index));
		break;

	default:
		Clear();
		ScriptError("[] applied to invalid type '%s'", typenames[GetType()]);
		break;
	}
}

float ScriptVariable::floatValue(void) const
{
	const char *string;
	float val;

	switch (type)
	{
	case VARIABLE_FLOAT:
		return m_data.floatValue;

	case VARIABLE_INTEGER:
		return (float)m_data.intValue;

		/* Transform the string into an integer if possible */
	case VARIABLE_STRING:
	case VARIABLE_CONSTSTRING:
		string = stringValue();
		val = (float)atof((const char *)string);

		return val;

	default:
		ScriptError("Cannot cast '%s' to float", typenames[type]);
	}
}

int ScriptVariable::intValue(void) const
{
	str string;
	int val;

	switch (type)
	{
	case VARIABLE_INTEGER:
		return m_data.intValue;

	case VARIABLE_FLOAT:
		return (int)m_data.floatValue;

	case VARIABLE_STRING:
	case VARIABLE_CONSTSTRING:
		string = stringValue();
		val = atoi(string);

		return val;

	default:
		ScriptError("Cannot cast '%s' to int", typenames[type]);
	}
}

Listener *ScriptVariable::listenerValue(void) const
{
	switch (type)
	{
	case VARIABLE_CONSTSTRING:
		return GetGameManager()->GetWorld()->GetScriptTarget(GetScriptManager()->GetString(m_data.constStringValue));

	case VARIABLE_STRING:
		return GetGameManager()->GetWorld()->GetScriptTarget(stringValue());

	case VARIABLE_LISTENER:
		return (Listener *)m_data.listenerValue->Pointer();

	default:
		ScriptError("Cannot cast '%s' to listener", typenames[type]);
	}

	return NULL;
}

void ScriptVariable::newPointer(void)
{
	type = VARIABLE_POINTER;

	m_data.pointerValue = new ScriptPointer();
	m_data.pointerValue->add(this);
}

#ifndef NO_SCRIPTENGINE

SimpleEntity *ScriptVariable::simpleEntityValue(void) const
{
	return (SimpleEntity *)listenerValue();
}

#endif

str ScriptVariable::stringValue() const
{
	str string;

	switch (GetType())
	{
	case VARIABLE_NONE:
		return "NIL";

	case VARIABLE_CONSTSTRING:
		return GetScriptManager()->GetString(m_data.intValue);

	case VARIABLE_STRING:
		return *m_data.stringValue;

	case VARIABLE_INTEGER:
		return str(m_data.intValue);

	case VARIABLE_FLOAT:
		return str(m_data.floatValue);

	case VARIABLE_CHAR:
		return str(m_data.charValue);

	case VARIABLE_LISTENER:
		if (m_data.listenerValue->Pointer())
		{
			/*
			if (m_data.listenerValue->Pointer()->isSubclassOf(SimpleEntity))
			{
				SimpleEntity *s = (SimpleEntity *)m_data.listenerValue->Pointer();
				return s->targetname;
			}
			else
			*/
			{
				string = "class '" + str(m_data.listenerValue->Pointer()->ScriptClass::GetClassname()) + "'";
				return string;
			}
		}
		else
		{
			return "NULL";
		}

	case VARIABLE_VECTOR:
		return str("( ") + str(m_data.vectorValue[0]) + str(" ") + str(m_data.vectorValue[1]) + str(" ") + str(m_data.vectorValue[2] + str(" )"));

	default:
		ScriptError("Cannot cast '%s' to string", typenames[GetType()]);
		break;
	}

	return "";
}

Vector ScriptVariable::vectorValue(void) const
{
	const char *string;
	float x = 0.f, y = 0.f, z = 0.f;

	switch (type)
	{
	case VARIABLE_VECTOR:
		return Vector(m_data.vectorValue);

	case VARIABLE_CONSTSTRING:
	case VARIABLE_STRING:
		string = stringValue();

		if (strcmp(string, "") == 0) {
			ScriptError("cannot cast empty string to vector");
		}

		if (*string == '(')
		{
			if (sscanf(string, "(%f %f %f)", &x, &y, &z) != 3)
			{
				if (sscanf(string, "(%f, %f, %f)", &x, &y, &z) != 3) {
					ScriptError("Couldn't convert string to vector - malformed string '%s'", string);
				}
			}
		}
		else
		{
			if (sscanf(string, "%f %f %f", &x, &y, &z) != 3)
			{
				if (sscanf(string, "%f, %f, %f", &x, &y, &z) != 3) {
					ScriptError("Couldn't convert string to vector - malformed string '%s'", string);
				}
			}
		}

		return Vector(x, y, z);
	case VARIABLE_LISTENER:
	{
		if (!m_data.listenerValue->Pointer()) {
			ScriptError("Cannot cast NULL to vector");
		}

		if (!checkInheritance(&SimpleEntity::ClassInfo, m_data.listenerValue->Pointer()->classinfo()))
		{
			ScriptError("Cannot cast '%s' to vector", GetTypeName());
		}

		SimpleEntity *ent = (SimpleEntity *)m_data.listenerValue->Pointer();

		return Vector(ent->origin[0], ent->origin[1], ent->origin[2]);
	}

	default:
		ScriptError("Cannot cast '%s' to vector", GetTypeName());
	}
}

void ScriptVariable::setArrayAt(ScriptVariable& index, ScriptVariable& value)
{
	return m_data.refValue->setArrayAtRef(index, value);
}

void ScriptVariable::setArrayAtRef(ScriptVariable& index, ScriptVariable& value)
{
	int intValue;
	unsigned int uintValue;
	str string;

	switch (type)
	{
	case VARIABLE_VECTOR:
		intValue = index.intValue();

		if (intValue > 2) {
			ScriptError("Vector index '%d' out of range", intValue);
		}

		m_data.vectorValue[intValue] = value.floatValue();
		break;

	case VARIABLE_REF:
		return;

	case VARIABLE_NONE:
		type = VARIABLE_ARRAY;

		m_data.arrayValue = new ScriptArrayHolder();

		if (value.GetType() != VARIABLE_NONE)
		{
			m_data.arrayValue->arrayValue[index] = value;
		}

		break;

	case VARIABLE_ARRAY:
		if (value.GetType() == VARIABLE_NONE)
		{
			m_data.arrayValue->arrayValue.remove(index);
		}
		else
		{
			m_data.arrayValue->arrayValue[index] = value;
		}
		break;

	case VARIABLE_STRING:
	case VARIABLE_CONSTSTRING:
		intValue = index.intValue();
		string = stringValue();

		if (intValue >= strlen(string)) {
			ScriptError("String index '%d' out of range", intValue);
		}

		string[intValue] = value.charValue();

		setStringValue(string);

		break;

	case VARIABLE_CONSTARRAY:
		uintValue = index.intValue();

		if (!uintValue || uintValue > m_data.constArrayValue->size)
		{
			ScriptError("array index %d out of range", uintValue);
		}

		if (value.GetType())
		{
			m_data.constArrayValue->constArrayValue[uintValue] = value;
		}
		else
		{
			m_data.constArrayValue->constArrayValue[uintValue].Clear();
		}

		break;

	default:
		ScriptError("[] applied to invalid type '%s'\n", typenames[GetType()]);
		break;
	}
}

void ScriptVariable::setArrayRefValue(ScriptVariable &var)
{
	setRefValue(&(*m_data.refValue)[var]);
}

void ScriptVariable::setCharValue(char newvalue)
{
	ClearInternal();

	type = VARIABLE_CHAR;
	m_data.charValue = newvalue;
}

void ScriptVariable::setContainerValue(Container< SafePtr< Listener > > *newvalue)
{
	ClearInternal();

	type = VARIABLE_CONTAINER;
	m_data.containerValue = newvalue;
}

void ScriptVariable::setSafeContainerValue(ConList *newvalue)
{
	ClearInternal();

	if (newvalue)
	{
		type = VARIABLE_SAFECONTAINER;
		m_data.safeContainerValue = new SafePtr< ConList >(newvalue);
	}
	else
	{
		type = VARIABLE_NONE;
	}
}

void ScriptVariable::setConstArrayValue(ScriptVariable *pVar, unsigned int size)
{
	ScriptConstArrayHolder *constArray = new ScriptConstArrayHolder(pVar - 1, size);

	ClearInternal();
	type = VARIABLE_CONSTARRAY;

	m_data.constArrayValue = constArray;
}

int ScriptVariable::constStringValue(void) const
{
	if (GetType() == VARIABLE_CONSTSTRING)
	{
		return m_data.constStringValue;
	}
	else
	{
		return GetScriptManager()->AddString(stringValue());
	}
}

void ScriptVariable::setConstStringValue(const_str s)
{
	ClearInternal();
	type = VARIABLE_CONSTSTRING;
	m_data.constStringValue = s;
}

void ScriptVariable::setFloatValue(float newvalue)
{
	ClearInternal();
	type = VARIABLE_FLOAT;
	m_data.floatValue = newvalue;
}

void ScriptVariable::setIntValue(int newvalue)
{
	ClearInternal();
	type = VARIABLE_INTEGER;
	m_data.intValue = newvalue;
}

void ScriptVariable::setListenerValue(Listener *newvalue)
{
	ClearInternal();

	type = VARIABLE_LISTENER;

	m_data.listenerValue = new SafePtr< Listener >(newvalue);
}

void ScriptVariable::setPointer(const ScriptVariable& newvalue)
{
	if (GetType() == VARIABLE_POINTER) {
		m_data.pointerValue->setValue(newvalue);
	}
}

void ScriptVariable::setRefValue(ScriptVariable *ref)
{
	ClearInternal();

	type = VARIABLE_REF;
	m_data.refValue = ref;
}

void ScriptVariable::setStringValue(const str& newvalue)
{
	str *s;

	ClearInternal();
	type = VARIABLE_STRING;

	s = new str(newvalue);

	m_data.stringValue = s;
}

void ScriptVariable::setVectorValue(const Vector &newvector)
{
	ClearInternal();

	type = VARIABLE_VECTOR;
	m_data.vectorValue = new float[3];
	VecCopy(newvector, m_data.vectorValue);
}

void ScriptVariable::operator+=(const ScriptVariable& value)
{
	int type = GetType();

	switch (type + value.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '+' applied to incompatible types '%s' and '%s'", typenames[type], typenames[value.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) + ( int )
		m_data.intValue = m_data.intValue + value.m_data.intValue;
		break;

	case VARIABLE_INTEGER + VARIABLE_FLOAT * VARIABLE_MAX: // ( int ) + ( float )
		setFloatValue((float)m_data.intValue + value.m_data.floatValue);
		break;

	case VARIABLE_FLOAT + VARIABLE_FLOAT * VARIABLE_MAX: // ( float ) + ( float )
		m_data.floatValue = m_data.floatValue + value.m_data.floatValue;
		break;

	case VARIABLE_FLOAT + VARIABLE_INTEGER * VARIABLE_MAX: // ( float ) + ( int )
		m_data.floatValue = m_data.floatValue + value.m_data.intValue;
		break;

	case VARIABLE_STRING + VARIABLE_STRING * VARIABLE_MAX:				// ( string )			+		( string )
	case VARIABLE_INTEGER + VARIABLE_STRING * VARIABLE_MAX:				// ( int )				+		( string )
	case VARIABLE_FLOAT + VARIABLE_STRING * VARIABLE_MAX:				// ( float )			+		( string )
	case VARIABLE_CHAR + VARIABLE_STRING * VARIABLE_MAX:				// ( char )				+		( string )
	case VARIABLE_CONSTSTRING + VARIABLE_STRING * VARIABLE_MAX:			// ( const string )		+		( string )
	case VARIABLE_LISTENER + VARIABLE_STRING * VARIABLE_MAX:			// ( listener )			+		( string )
	case VARIABLE_VECTOR + VARIABLE_STRING * VARIABLE_MAX:				// ( vector )			+		( string )
	case VARIABLE_STRING + VARIABLE_INTEGER * VARIABLE_MAX:				// ( string )			+		( int )
	case VARIABLE_CONSTSTRING + VARIABLE_INTEGER * VARIABLE_MAX:		// ( const string )		+		( int )
	case VARIABLE_STRING + VARIABLE_FLOAT * VARIABLE_MAX:				// ( string )			+		( float )
	case VARIABLE_CONSTSTRING + VARIABLE_FLOAT * VARIABLE_MAX:			// ( const string )		+		( float )
	case VARIABLE_STRING + VARIABLE_CHAR * VARIABLE_MAX:				// ( string )			+		( char )
	case VARIABLE_CONSTSTRING + VARIABLE_CHAR * VARIABLE_MAX:			// ( const string )		+		( char )
	case VARIABLE_STRING + VARIABLE_CONSTSTRING * VARIABLE_MAX:			// ( string )			+		( const string )
	case VARIABLE_INTEGER + VARIABLE_CONSTSTRING * VARIABLE_MAX:		// ( int )				+		( const string )
	case VARIABLE_FLOAT + VARIABLE_CONSTSTRING * VARIABLE_MAX:			// ( float )			+		( const string )
	case VARIABLE_CHAR + VARIABLE_CONSTSTRING * VARIABLE_MAX:			// ( char )				+		( const string )
	case VARIABLE_CONSTSTRING + VARIABLE_CONSTSTRING * VARIABLE_MAX:	// ( const string )		+		( const string )
	case VARIABLE_LISTENER + VARIABLE_CONSTSTRING * VARIABLE_MAX:		// ( listener )			+		( const string )
	case VARIABLE_VECTOR + VARIABLE_CONSTSTRING * VARIABLE_MAX:			// ( vector )			+		( const string )
	case VARIABLE_STRING + VARIABLE_LISTENER * VARIABLE_MAX:			// ( string )			+		( listener )
	case VARIABLE_CONSTSTRING + VARIABLE_LISTENER * VARIABLE_MAX:		// ( const string )		+		( listener )
	case VARIABLE_STRING + VARIABLE_VECTOR * VARIABLE_MAX:				// ( string )			+		( vector )
	case VARIABLE_CONSTSTRING + VARIABLE_VECTOR * VARIABLE_MAX:			// ( const string )		+		( vector )
		setStringValue(stringValue() + value.stringValue());
		break;

	case VARIABLE_VECTOR + VARIABLE_VECTOR * VARIABLE_MAX:
		VecAdd(m_data.vectorValue, value.m_data.vectorValue, m_data.vectorValue);
		break;
	}
}

void ScriptVariable::operator-=(const ScriptVariable& value)
{
	switch (GetType() + value.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '-' applied to incompatible types '%s' and '%s'", typenames[GetType()], typenames[value.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) - ( int )
		m_data.intValue = m_data.intValue - value.m_data.intValue;
		break;

	case VARIABLE_INTEGER + VARIABLE_FLOAT * VARIABLE_MAX: // ( int ) - ( float )
		setFloatValue((float)m_data.intValue - value.m_data.floatValue);
		break;

	case VARIABLE_FLOAT + VARIABLE_FLOAT * VARIABLE_MAX: // ( float ) - ( float )
		m_data.floatValue = m_data.floatValue - value.m_data.floatValue;
		break;

	case VARIABLE_FLOAT + VARIABLE_INTEGER * VARIABLE_MAX: // ( float ) - ( int )
		m_data.floatValue = m_data.floatValue - value.m_data.intValue;
		break;

	case VARIABLE_VECTOR + VARIABLE_VECTOR * VARIABLE_MAX: // ( vector ) - ( vector )
		VecSubtract(m_data.vectorValue, value.m_data.vectorValue, m_data.vectorValue);
		break;
	}
}

void ScriptVariable::operator*=(const ScriptVariable& value)
{
	switch (GetType() + value.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '*' applied to incompatible types '%s' and '%s'", typenames[GetType()], typenames[value.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) * ( int )
		m_data.intValue = m_data.intValue * value.m_data.intValue;
		break;

	case VARIABLE_VECTOR + VARIABLE_INTEGER * VARIABLE_MAX: // ( vector ) * ( int )
		VectorScale(m_data.vectorValue, (float)value.m_data.intValue, m_data.vectorValue);
		break;

	case VARIABLE_VECTOR + VARIABLE_FLOAT * VARIABLE_MAX: // ( vector ) * ( float )
		VectorScale(m_data.vectorValue, value.m_data.floatValue, m_data.vectorValue);
		break;

	case VARIABLE_INTEGER + VARIABLE_FLOAT * VARIABLE_MAX: // ( int ) * ( float )
		setFloatValue((float)m_data.intValue * value.m_data.floatValue);
		break;

	case VARIABLE_FLOAT + VARIABLE_FLOAT * VARIABLE_MAX: // ( float ) * ( float )
		m_data.floatValue = m_data.floatValue * value.m_data.floatValue;
		break;

	case VARIABLE_FLOAT + VARIABLE_INTEGER * VARIABLE_MAX: // ( float ) * ( int )
		m_data.floatValue = m_data.floatValue * value.m_data.intValue;
		break;

	case VARIABLE_INTEGER + VARIABLE_VECTOR * VARIABLE_MAX: // ( int ) * ( vector )
		setVectorValue((float)m_data.intValue * Vector(value.m_data.vectorValue));
		break;

	case VARIABLE_FLOAT + VARIABLE_VECTOR * VARIABLE_MAX: // ( float ) * ( vector )
		setVectorValue(m_data.floatValue * Vector(value.m_data.vectorValue));
		break;

	case VARIABLE_VECTOR + VARIABLE_VECTOR * VARIABLE_MAX: // ( vector ) * ( vector )
		m_data.vectorValue[0] = m_data.vectorValue[0] * value.m_data.vectorValue[0];
		m_data.vectorValue[1] = m_data.vectorValue[1] * value.m_data.vectorValue[1];
		m_data.vectorValue[2] = m_data.vectorValue[2] * value.m_data.vectorValue[2];
		break;
	}
}

void ScriptVariable::operator/=(const ScriptVariable& value)
{
	switch (GetType() + value.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '/' applied to incompatible types '%s' and '%s'", typenames[GetType()], typenames[value.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) / ( int )
		if (value.m_data.intValue == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.intValue = m_data.intValue / value.m_data.intValue;
		break;

	case VARIABLE_VECTOR + VARIABLE_INTEGER * VARIABLE_MAX: // ( vector ) / ( int )
		if (value.m_data.intValue == 0) {
			ScriptError("Division by zero error\n");
		}

		(Vector)m_data.vectorValue = (Vector)m_data.vectorValue / (float)value.m_data.intValue;
		break;

	case VARIABLE_VECTOR + VARIABLE_FLOAT * VARIABLE_MAX: // ( vector ) / ( float )
		if (value.m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.vectorValue[0] = m_data.vectorValue[0] / value.m_data.floatValue;
		m_data.vectorValue[1] = m_data.vectorValue[1] / value.m_data.floatValue;
		m_data.vectorValue[2] = m_data.vectorValue[2] / value.m_data.floatValue;
		break;

	case VARIABLE_INTEGER + VARIABLE_FLOAT * VARIABLE_MAX: // ( int ) / ( float )
		if (value.m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		setFloatValue((float)m_data.intValue / value.m_data.floatValue);
		break;

	case VARIABLE_FLOAT + VARIABLE_FLOAT * VARIABLE_MAX: // ( float ) / ( float )
		if (value.m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.floatValue = m_data.floatValue / value.m_data.floatValue;
		break;

	case VARIABLE_FLOAT + VARIABLE_INTEGER * VARIABLE_MAX: // ( float ) / ( int )
		if (value.m_data.intValue == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.floatValue = m_data.floatValue / value.m_data.intValue;
		break;

	case VARIABLE_INTEGER + VARIABLE_VECTOR * VARIABLE_MAX: // ( int ) / ( vector )
		if (m_data.intValue == 0) {
			ScriptError("Division by zero error\n");
		}

		setVectorValue((float)m_data.intValue / Vector(value.m_data.vectorValue));
		break;

	case VARIABLE_FLOAT + VARIABLE_VECTOR * VARIABLE_MAX: // ( float ) / ( vector )
		if (m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		setVectorValue(m_data.floatValue / Vector(value.m_data.vectorValue));
		break;

	case VARIABLE_VECTOR + VARIABLE_VECTOR * VARIABLE_MAX: // ( vector ) / ( vector )
		m_data.vectorValue = vec_zero;

		if (value.m_data.vectorValue[0] != 0) {
			m_data.vectorValue[0] = m_data.vectorValue[0] / value.m_data.vectorValue[0];
		}

		if (value.m_data.vectorValue[1] != 0) {
			m_data.vectorValue[1] = m_data.vectorValue[1] / value.m_data.vectorValue[1];
		}

		if (value.m_data.vectorValue[2] != 0) {
			m_data.vectorValue[2] = m_data.vectorValue[2] / value.m_data.vectorValue[2];
		}
		break;
	}
}

void ScriptVariable::operator%=(const ScriptVariable& value)
{
	float mult = 0.0f;

	switch (GetType() + value.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '%' applied to incompatible types '%s' and '%s'", typenames[GetType()], typenames[value.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) % ( int )
		if (value.m_data.intValue == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.intValue = m_data.intValue % value.m_data.intValue;
		break;

	case VARIABLE_VECTOR + VARIABLE_INTEGER * VARIABLE_MAX: // ( vector ) % ( int )
		if (value.m_data.intValue == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.vectorValue[0] = (float)fmod(m_data.vectorValue[0], value.m_data.intValue);
		m_data.vectorValue[1] = (float)fmod(m_data.vectorValue[1], value.m_data.intValue);
		m_data.vectorValue[2] = (float)fmod(m_data.vectorValue[2], value.m_data.intValue);
		break;

	case VARIABLE_VECTOR + VARIABLE_FLOAT * VARIABLE_MAX: // ( vector ) % ( float )
		if (value.m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.vectorValue[0] = (float)fmod(m_data.vectorValue[0], value.m_data.floatValue);
		m_data.vectorValue[1] = (float)fmod(m_data.vectorValue[1], value.m_data.floatValue);
		m_data.vectorValue[2] = (float)fmod(m_data.vectorValue[2], value.m_data.floatValue);
		break;

	case VARIABLE_INTEGER + VARIABLE_FLOAT * VARIABLE_MAX: // ( int ) % ( float )
		if (value.m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		setFloatValue(fmod((float)m_data.intValue, value.m_data.floatValue));
		break;

	case VARIABLE_FLOAT + VARIABLE_FLOAT * VARIABLE_MAX: // ( float ) % ( float )
		if (value.m_data.floatValue == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.floatValue = fmod(m_data.floatValue, value.m_data.floatValue);
		break;

	case VARIABLE_FLOAT + VARIABLE_INTEGER * VARIABLE_MAX: // ( float ) % ( int )
		if (value.m_data.intValue == 0) {
			ScriptError("Division by zero error\n");
		}

		m_data.floatValue = fmod(m_data.floatValue, (float)value.m_data.intValue);
		break;

	case VARIABLE_INTEGER + VARIABLE_VECTOR * VARIABLE_MAX: // ( int ) % ( vector )
		mult = (float)m_data.intValue;

		if (mult == 0) {
			ScriptError("Division by zero error\n");
		}

		setVectorValue(vec_zero);

		m_data.vectorValue[0] = fmod(value.m_data.vectorValue[0], mult);
		m_data.vectorValue[1] = fmod(value.m_data.vectorValue[1], mult);
		m_data.vectorValue[2] = fmod(value.m_data.vectorValue[2], mult);
		break;

	case VARIABLE_FLOAT + VARIABLE_VECTOR * VARIABLE_MAX: // ( float ) % ( vector )
		mult = m_data.floatValue;

		if (mult == 0) {
			ScriptError("Division by zero error\n");
		}

		setVectorValue(vec_zero);

		m_data.vectorValue[0] = fmod(m_data.vectorValue[0], mult);
		m_data.vectorValue[1] = fmod(m_data.vectorValue[1], mult);
		m_data.vectorValue[2] = fmod(m_data.vectorValue[2], mult);
		break;

	case VARIABLE_VECTOR + VARIABLE_VECTOR * VARIABLE_MAX: // ( vector ) % ( vector )
		m_data.vectorValue = vec_zero;

		if (value.m_data.vectorValue[0] != 0) {
			m_data.vectorValue[0] = fmod(m_data.vectorValue[0], value.m_data.vectorValue[0]);
		}

		if (value.m_data.vectorValue[1] != 0) {
			m_data.vectorValue[1] = fmod(m_data.vectorValue[1], value.m_data.vectorValue[1]);
		}

		if (value.m_data.vectorValue[2] != 0) {
			m_data.vectorValue[2] = fmod(m_data.vectorValue[2], value.m_data.vectorValue[2]);
		}

		break;
	}
}

void ScriptVariable::operator&=(const ScriptVariable& value)
{
	int type = GetType();

	switch (type + value.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '&' applied to incompatible types '%s' and '%s'", typenames[type], typenames[value.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) &= ( int )
		m_data.intValue &= value.m_data.intValue;
		break;
	}
}

void ScriptVariable::operator^=(const ScriptVariable& value)
{
	int type = GetType();

	switch (type + value.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '^' applied to incompatible types '%s' and '%s'", typenames[type], typenames[value.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) ^= ( int )
		m_data.intValue ^= value.m_data.intValue;
		break;
	}
}

void ScriptVariable::operator|=(const ScriptVariable& value)
{
	int type = GetType();

	switch (type + value.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '|' applied to incompatible types '%s' and '%s'", typenames[type], typenames[value.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) |= ( int )
		m_data.intValue |= value.m_data.intValue;
		break;
	}
}

void ScriptVariable::operator<<=(const ScriptVariable& value)
{
	int type = GetType();

	switch (type + value.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '<<' applied to incompatible types '%s' and '%s'", typenames[type], typenames[value.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) <<= ( int )
		m_data.intValue <<= value.m_data.intValue;
		break;
	}
}

void ScriptVariable::operator>>=(const ScriptVariable& value)
{
	int type = GetType();

	switch (type + value.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '>>' applied to incompatible types '%s' and '%s'", typenames[type], typenames[value.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) >>= ( int )
		m_data.intValue >>= value.m_data.intValue;
		break;
	}
}

bool ScriptVariable::operator!=(const ScriptVariable &value)
{
	return !(*this == value);
}

bool ScriptVariable::operator==(const ScriptVariable &value)
{
	int type = GetType();

	switch (type + value.GetType() * VARIABLE_MAX)
	{
	default:		// ( lval )	==	( nil )
					// ( nil )	==	( rval )
		Clear();
		return false;

	case VARIABLE_NONE + VARIABLE_NONE * VARIABLE_MAX: // ( nil ) == ( nil )
		return true;

	case VARIABLE_LISTENER + VARIABLE_LISTENER * VARIABLE_MAX: // ( listener ) == ( listener )
	{
		SafePtrClass *lval = NULL;
		SafePtrClass *rval = NULL;

		if (m_data.listenerValue)
		{
			lval = m_data.listenerValue->Pointer();
		}

		if (value.m_data.listenerValue)
		{
			rval = value.m_data.listenerValue->Pointer();
		}

		return lval == rval;
	}

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) == ( int )
		return m_data.intValue == value.m_data.intValue;

	case VARIABLE_INTEGER + VARIABLE_FLOAT * VARIABLE_MAX: // ( int ) == ( float )
		return m_data.intValue == value.m_data.floatValue;

	case VARIABLE_FLOAT + VARIABLE_FLOAT * VARIABLE_MAX: // ( float ) == ( float )
		return m_data.floatValue == value.m_data.floatValue;

	case VARIABLE_FLOAT + VARIABLE_INTEGER * VARIABLE_MAX: // ( float ) == ( int )
		return m_data.floatValue == value.m_data.intValue;

	case VARIABLE_CONSTSTRING + VARIABLE_CONSTSTRING * VARIABLE_MAX:	// ( const string )		==		( const string )
		return m_data.intValue == value.m_data.intValue;

	case VARIABLE_STRING + VARIABLE_STRING * VARIABLE_MAX:				// ( string )			==		( string )
	case VARIABLE_INTEGER + VARIABLE_STRING * VARIABLE_MAX:				// ( int )				==		( string )
	case VARIABLE_FLOAT + VARIABLE_STRING * VARIABLE_MAX:				// ( float )			==		( string )
	case VARIABLE_CHAR + VARIABLE_STRING * VARIABLE_MAX:				// ( char )				==		( string )
	case VARIABLE_CONSTSTRING + VARIABLE_STRING * VARIABLE_MAX:			// ( const string )		==		( string )
	case VARIABLE_LISTENER + VARIABLE_STRING * VARIABLE_MAX:			// ( listener )			==		( string )
	case VARIABLE_VECTOR + VARIABLE_STRING * VARIABLE_MAX:				// ( vector )			==		( string )
	case VARIABLE_STRING + VARIABLE_CONSTSTRING * VARIABLE_MAX:			// ( string )			==		( const string )
	case VARIABLE_INTEGER + VARIABLE_CONSTSTRING * VARIABLE_MAX:		// ( int )				==		( const string )
	case VARIABLE_FLOAT + VARIABLE_CONSTSTRING * VARIABLE_MAX:			// ( float )			==		( const string )
	case VARIABLE_CHAR + VARIABLE_CONSTSTRING * VARIABLE_MAX:			// ( char )				==		( const string )
	case VARIABLE_LISTENER + VARIABLE_CONSTSTRING * VARIABLE_MAX:		// ( listener )			==		( const string )
	case VARIABLE_VECTOR + VARIABLE_CONSTSTRING * VARIABLE_MAX:			// ( vector )			==		( const string )
	case VARIABLE_STRING + VARIABLE_INTEGER * VARIABLE_MAX:				// ( string )			==		( int )
	case VARIABLE_CONSTSTRING + VARIABLE_INTEGER * VARIABLE_MAX:		// ( const string )		==		( int )
	case VARIABLE_STRING + VARIABLE_FLOAT * VARIABLE_MAX:				// ( string )			==		( float )
	case VARIABLE_CONSTSTRING + VARIABLE_FLOAT * VARIABLE_MAX:			// ( const string )		==		( float )
	case VARIABLE_STRING + VARIABLE_CHAR * VARIABLE_MAX:				// ( string )			==		( char )
	case VARIABLE_CONSTSTRING + VARIABLE_CHAR * VARIABLE_MAX:			// ( const string )		==		( char )
	case VARIABLE_STRING + VARIABLE_LISTENER * VARIABLE_MAX:			// ( string )			==		( listener )
	case VARIABLE_CONSTSTRING + VARIABLE_LISTENER * VARIABLE_MAX:		// ( const string )		==		( listener )
	case VARIABLE_STRING + VARIABLE_VECTOR * VARIABLE_MAX:				// ( string )			==		( vector )
	case VARIABLE_CONSTSTRING + VARIABLE_VECTOR * VARIABLE_MAX:			// ( const string )		==		( vector )
	{
		str lval = stringValue();
		str rval = value.stringValue();

		return (!lval.length() && !rval.length()) || (lval == rval);
	}

	case VARIABLE_VECTOR + VARIABLE_VECTOR * VARIABLE_MAX: // ( vector ) == ( vector )
		return VecCompare(m_data.vectorValue, value.m_data.vectorValue) ? true : false;
	}
}

bool ScriptVariable::operator=(const ScriptVariable &variable)
{
	ClearInternal();

	type = variable.GetType();

	switch (type)
	{
	case VARIABLE_NONE:
		return false;

	case VARIABLE_CONSTSTRING:
		m_data.intValue = variable.m_data.intValue;
		return true;

	case VARIABLE_STRING:
		m_data.stringValue = new str(variable.stringValue());
		return true;

	case VARIABLE_FLOAT:
		m_data.floatValue = variable.m_data.floatValue;
		return true;

	case VARIABLE_CHAR:
		m_data.charValue = variable.m_data.charValue;
		return true;

	case VARIABLE_INTEGER:
		m_data.intValue = variable.m_data.intValue;
		return true;

	case VARIABLE_LISTENER:
		m_data.listenerValue = new SafePtr< Listener >(*variable.m_data.listenerValue);
		return true;

	case VARIABLE_ARRAY:
		m_data.arrayValue = variable.m_data.arrayValue;
		m_data.arrayValue->refCount++;
		return true;

	case VARIABLE_CONSTARRAY:
		m_data.constArrayValue = variable.m_data.constArrayValue;
		m_data.constArrayValue->refCount++;
		return true;

	case VARIABLE_CONTAINER:
		m_data.containerValue = new Container< SafePtr< Listener > >(*variable.m_data.containerValue);
		return true;

	case VARIABLE_SAFECONTAINER:
		m_data.safeContainerValue = new SafePtr< ConList >(*variable.m_data.safeContainerValue);
		return true;

	case VARIABLE_POINTER:
		m_data.pointerValue = variable.m_data.pointerValue;
		m_data.pointerValue->add(this);
		return true;

	case VARIABLE_VECTOR:
		m_data.vectorValue = (float *)new float[3];
		VecCopy(variable.m_data.vectorValue, m_data.vectorValue);
		return true;
	}

	return true;
}

ScriptVariable &ScriptVariable::operator[](ScriptVariable& index)
{
	size_t i;

	switch (GetType())
	{
	case VARIABLE_NONE:
		type = VARIABLE_ARRAY;

		m_data.arrayValue = new ScriptArrayHolder();
		return m_data.arrayValue->arrayValue[index];

	case VARIABLE_ARRAY:
		return m_data.arrayValue->arrayValue[index];

	case VARIABLE_CONSTARRAY:
		i = index.intValue();

		if (i == 0 || i > m_data.constArrayValue->size)
		{
			ScriptError("array index %d out of range", i);
		}

		return m_data.constArrayValue->constArrayValue[i];

	default:
		ScriptError("[] applied to invalid type '%s'", typenames[GetType()]);
	}
}

ScriptVariable *ScriptVariable::operator[](uintptr_t index) const
{
	return &m_data.constArrayValue->constArrayValue[index];
}

ScriptVariable *ScriptVariable::operator*()
{
	return m_data.refValue;
}

void ScriptVariable::complement(void)
{
	if (type == VARIABLE_INTEGER)
	{
		m_data.intValue = ~m_data.intValue;
	}
	else
	{
		float value = floatValue();
		int i = ~*(int *)&value; // ley0k: evil floating point hack

		setFloatValue(*(float *)&i);
	}
}

void ScriptVariable::greaterthan(ScriptVariable &variable)
{
	switch (GetType() + variable.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '>' applied to incompatible types '%s' and '%s'", typenames[GetType()], typenames[variable.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) > ( int )
		m_data.intValue = m_data.intValue > variable.m_data.intValue;
		break;

	case VARIABLE_INTEGER + VARIABLE_FLOAT * VARIABLE_MAX: // ( int ) > ( float )
		m_data.intValue = m_data.intValue > variable.m_data.floatValue;
		break;

	case VARIABLE_FLOAT + VARIABLE_FLOAT * VARIABLE_MAX: // ( float ) > ( float )
		type = VARIABLE_INTEGER;
		m_data.intValue = m_data.floatValue > variable.m_data.floatValue;
		break;

	case VARIABLE_FLOAT + VARIABLE_INTEGER * VARIABLE_MAX: // ( float ) > ( int )
		type = VARIABLE_INTEGER;
		m_data.intValue = m_data.floatValue > variable.m_data.intValue;
		break;
	}
}

void ScriptVariable::greaterthanorequal(ScriptVariable &variable)
{
	switch (GetType() + variable.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '>=' applied to incompatible types '%s' and '%s'", typenames[GetType()], typenames[variable.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) >= ( int )
		m_data.intValue = m_data.intValue >= variable.m_data.intValue;
		break;

	case VARIABLE_INTEGER + VARIABLE_FLOAT * VARIABLE_MAX: // ( int ) >= ( float )
		m_data.intValue = m_data.intValue >= variable.m_data.floatValue;
		break;

	case VARIABLE_FLOAT + VARIABLE_FLOAT * VARIABLE_MAX: // ( float ) >= ( float )
		type = VARIABLE_INTEGER;
		m_data.intValue = m_data.floatValue >= variable.m_data.floatValue;
		break;

	case VARIABLE_FLOAT + VARIABLE_INTEGER * VARIABLE_MAX: // ( float ) >= ( int )
		type = VARIABLE_INTEGER;
		m_data.intValue = m_data.floatValue >= variable.m_data.intValue;
		break;
	}
}

void ScriptVariable::lessthan(ScriptVariable &variable)
{
	switch (GetType() + variable.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '<' applied to incompatible types '%s' and '%s'", typenames[GetType()], typenames[variable.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) < ( int )
		m_data.intValue = m_data.intValue < variable.m_data.intValue;
		break;

	case VARIABLE_INTEGER + VARIABLE_FLOAT * VARIABLE_MAX: // ( int ) < ( float )
		m_data.intValue = m_data.intValue < variable.m_data.floatValue;
		break;

	case VARIABLE_FLOAT + VARIABLE_FLOAT * VARIABLE_MAX: // ( float ) < ( float )
		type = VARIABLE_INTEGER;
		m_data.intValue = m_data.floatValue < variable.m_data.floatValue;
		break;

	case VARIABLE_FLOAT + VARIABLE_INTEGER * VARIABLE_MAX: // ( float ) < ( int )
		type = VARIABLE_INTEGER;
		m_data.intValue = m_data.floatValue < variable.m_data.intValue;
		break;
	}
}

void ScriptVariable::lessthanorequal(ScriptVariable &variable)
{
	switch (GetType() + variable.GetType() * VARIABLE_MAX)
	{
	default:
		Clear();

		ScriptError("binary '<=' applied to incompatible types '%s' and '%s'", typenames[GetType()], typenames[variable.GetType()]);

		break;

	case VARIABLE_INTEGER + VARIABLE_INTEGER * VARIABLE_MAX: // ( int ) <= ( int )
		m_data.intValue = m_data.intValue <= variable.m_data.intValue;
		break;

	case VARIABLE_INTEGER + VARIABLE_FLOAT * VARIABLE_MAX: // ( int ) <= ( float )
		m_data.intValue = m_data.intValue <= variable.m_data.floatValue;
		break;

	case VARIABLE_FLOAT + VARIABLE_FLOAT * VARIABLE_MAX: // ( float ) <= ( float )
		type = VARIABLE_INTEGER;
		m_data.intValue = m_data.floatValue <= variable.m_data.floatValue;
		break;

	case VARIABLE_FLOAT + VARIABLE_INTEGER * VARIABLE_MAX: // ( float ) <= ( int )
		type = VARIABLE_INTEGER;
		m_data.intValue = m_data.floatValue <= variable.m_data.intValue;
		break;
	}
}

void ScriptVariable::minus(void)
{
	if (GetType() == VARIABLE_INTEGER)
	{
		m_data.intValue = -m_data.intValue;
	}
	else if (GetType() == VARIABLE_FLOAT)
	{
		m_data.floatValue = -m_data.floatValue;
	}
	else
	{
		setIntValue(-intValue());
	}
}

ScriptVariable ScriptVariable::operator++(int)
{
	switch (type)
	{
	case VARIABLE_NONE:
		return *this;

	case VARIABLE_INTEGER:
		setIntValue(intValue() + 1);
		break;

	case VARIABLE_POINTER:
		ClearPointerInternal();
		break;

	case VARIABLE_FLOAT:
		setFloatValue(floatValue() + 1.0f);
		break;

	default:
		int newvalue = intValue();

		ClearInternal();

		type = VARIABLE_INTEGER;
		m_data.intValue = newvalue + 1;

		break;
	}

	return *this;
}

ScriptVariable ScriptVariable::operator--(int)
{
	switch (type)
	{
	case VARIABLE_NONE:
		return *this;

	case VARIABLE_INTEGER:
		setIntValue(intValue() - 1);
		break;

	case VARIABLE_POINTER:
		ClearPointerInternal();
		break;

	case VARIABLE_FLOAT:
		setFloatValue(floatValue() - 1.0f);
		break;

	default:
		int newvalue = intValue();

		ClearInternal();

		type = VARIABLE_INTEGER;
		m_data.intValue = newvalue - 1;

		break;
	}

	return *this;
}

ScriptVariableList::ScriptVariableList()
{

}

void ScriptVariableList::ClearList(void)
{
	list.clear();
}

ScriptVariable *ScriptVariableList::GetOrCreateVariable(const str& name)
{
	return GetOrCreateVariable(GetScriptManager()->AddString(name));
}

ScriptVariable *ScriptVariableList::GetOrCreateVariable(const_str name)
{
	return &list.addKeyValue(name);
}

ScriptVariable *ScriptVariableList::GetVariable(const str& name)
{
	return GetVariable(GetScriptManager()->AddString(name));
}

ScriptVariable *ScriptVariableList::GetVariable(const_str name)
{
	return list.findKeyValue(name);
}

ScriptVariable * ScriptVariableList::SetVariable(const char *name, int value)
{
	ScriptVariable *variable = GetOrCreateVariable(name);

	variable->setIntValue(value);

	return variable;
}

ScriptVariable * ScriptVariableList::SetVariable(const char *name, float value)
{
	ScriptVariable *variable = GetOrCreateVariable(name);

	variable->setFloatValue(value);

	return variable;
}

ScriptVariable * ScriptVariableList::SetVariable(const char *name, const char * value)
{
	ScriptVariable *variable = GetOrCreateVariable(name);

	variable->setStringValue(value);

	return variable;
}

ScriptVariable *ScriptVariableList::SetVariable(const char *name, Entity *value)
{
	ScriptVariable *variable = GetOrCreateVariable(name);

	variable->setListenerValue((Listener *)value);

	return variable;
}

ScriptVariable *ScriptVariableList::SetVariable(const char *name, Listener *value)
{
	ScriptVariable *variable = GetOrCreateVariable(name);

	variable->setListenerValue(value);

	return variable;
}

ScriptVariable *ScriptVariableList::SetVariable(const char *name, ScriptVariable& value)
{
	ScriptVariable *variable = GetOrCreateVariable(name);

	*variable = value;

	return variable;
}

ScriptVariable *ScriptVariableList::SetVariable(const char *name, Vector &value)
{
	ScriptVariable *variable = GetOrCreateVariable(name);

	variable->setVectorValue(value);

	return variable;
}

ScriptVariable *ScriptVariableList::SetVariable(const_str name, ScriptVariable& value)
{
	ScriptVariable *variable = GetOrCreateVariable(name);

	*variable = value;
	variable->SetKey(name);

	return variable;
}

void ScriptVariableList::Archive(Archiver &arc)
{
	//BaseScriptClass::Archive(arc);
	list.Archive(arc);
}
