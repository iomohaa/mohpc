#include <Shared.h>
#include <MOHPC/LevelEntity.h>
#include <MOHPC/Managers/FileManager.h>
#include <string.h>

using namespace MOHPC;

str MOHPC::CanonicalModelName(const char* ModelName)
{
	if (*ModelName != '*')
	{
		if (!strnicmp(ModelName, "models/", 7))
		{
			return FileManager::CanonicalFilename(ModelName);
		}
		else
		{
			return "models/" + FileManager::CanonicalFilename(ModelName);
		}
	}
	else
	{
		return ModelName;
	}
}

PropertyDef::PropertyDef(const char* inPropertyName)
{
	propertyName = inPropertyName;
}

PropertyDef::PropertyDef(str&& inPropertyName)
	: propertyName(std::move(inPropertyName))
{

}

PropertyDef::PropertyDef(const str& inPropertyName)
	: propertyName(inPropertyName)
{
}

bool PropertyDef::operator<(const PropertyDef& right) const
{
	return propertyName < right.propertyName;
}

const char* PropertyDef::GetPropertyName() const
{
	return IsKeyed() ? propertyName.c_str() + 1 : propertyName.c_str();
}

const char* PropertyDef::GetFullPropertyName() const
{
	return propertyName.c_str();
}

bool MOHPC::PropertyDef::IsKeyed() const
{
	return propertyName[0] == '#' || propertyName[0] == '$';
}

LevelEntity::LevelEntity(size_t num)
{
	entnum = num;
}

size_t LevelEntity::GetEntNum() const
{
	return entnum;
}

const char* LevelEntity::GetClassName() const
{
	return !classname.isEmpty() ? classname.c_str() : "";
}

int32_t LevelEntity::GetSpawnflags() const
{
	return spawnflags;
}

const char* LevelEntity::GetTargetName() const
{
	return !targetname.isEmpty() ? targetname.c_str() : "";
}

const char* LevelEntity::GetTarget() const
{
	return !target.isEmpty() ? target.c_str() : "";
}

void MOHPC::LevelEntity::SetTargetName(const char* newTargetName)
{
	targetname = newTargetName;
}

const char* MOHPC::LevelEntity::GetModel() const
{
	return !model.isEmpty() ? model.c_str() : "";
}

const Vector& MOHPC::LevelEntity::GetOrigin() const
{
	return origin;
}

const Vector& MOHPC::LevelEntity::GetAngles() const
{
	return angles;
}

bool LevelEntity::IsClassOf(const char* c) const
{
	return !stricmp(classname.c_str(), c);
}

bool LevelEntity::HasSubmodel() const
{
	return model.length() > 1 && *model.c_str() == '*';
}

bool LevelEntity::HasProperty(const char* Key) const
{
	return GetPropertyValuePointer(Key) != nullptr;
}

const char* LevelEntity::GetPropertyRawValue(const char* Key) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return nullptr;
	}
	return val->c_str();
}

const str& LevelEntity::GetPropertyStringValue(const char* Key, const str& defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return *val;
}

int8_t LevelEntity::GetPropertyCharValue(const char* Key, int8_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return *val->c_str();
}

int16_t LevelEntity::GetPropertyShortValue(const char* Key, int16_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return (int16_t)std::stoi(val->c_str());
}

int32_t LevelEntity::GetPropertyIntegerValue(const char* Key, int32_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stoi(val->c_str());
}

int64_t LevelEntity::GetPropertyLongValue(const char* Key, int64_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stoll(val->c_str());
}

uint8_t LevelEntity::GetPropertyByteValue(const char* Key, uint8_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return (uint8_t)*val->c_str();
}

uint16_t LevelEntity::GetPropertyUnsignedShortValue(const char* Key, uint16_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return (uint16_t)std::stoul(val->c_str());
}

uint32_t LevelEntity::GetPropertyUnsignedIntegerValue(const char* Key, uint32_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stoul(val->c_str());
}

uint64_t LevelEntity::GetPropertyUnsignedLongValue(const char* Key, uint64_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stoull(val->c_str());
}

float LevelEntity::GetPropertyFloatValue(const char* Key, float defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stof(val->c_str());
}

double LevelEntity::GetPropertyDoubleValue(const char* Key, double defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stod(val->c_str());
}

long double LevelEntity::GetPropertyLongDoubleValue(const char* Key, long double defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stold(val->c_str());
}

Vector LevelEntity::GetPropertyVectorValue(const char* Key, const Vector& defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return Vector(val->c_str());
}

const str* LevelEntity::GetPropertyValuePointer(const char* Key) const
{
	PropertyMap::const_iterator it = keyValues.find(Key);
	if (it != keyValues.end())
	{
		return &it->second;
	}
	else
	{
		return nullptr;
	}
}

MOHPC_EXPORTS bool MOHPC::LevelEntity::TrySetMemberValue(const char* Key, const char* Value)
{
	if (!stricmp(Key, "model"))
	{
		model = CanonicalModelName(Value);
	}
	else if (!stricmp(Key, "origin"))
	{
		origin = Vector(Value);
	}
	else if (!stricmp(Key, "angles"))
	{
		angles = Vector(Value);
	}
	else if (!stricmp(Key, "angle"))
	{
		try
		{
			angles = Vector(0, (float)atof(Value), 0);
		}
		catch (const std::exception&)
		{
			angles = vec_zero;
		}
	}
	else if (!stricmp(Key, "classname"))
	{
		if (classname.isEmpty())
		{
			// Only set if not set already
			classname = Value;
		}
	}
	else if (!stricmp(Key, "spawnflags"))
	{
		spawnflags = atoi(Value);
	}
	else if (!stricmp(Key, "targetname"))
	{
		targetname = Value;
	}
	else if (!stricmp(Key, "target"))
	{
		target = Value;
	}
	else
	{
		return false;
	}

	return true;
}

void LevelEntity::SetPropertyValue(const char* Key, const char* Value)
{
	if(!TrySetMemberValue(Key, Value))
	{
		keyValues.insert_or_assign(Key, Value);
	}
}

void MOHPC::LevelEntity::SetPropertyDef(const PropertyDef& Key, str&& Value)
{
	if (!TrySetMemberValue(Key.GetPropertyName(), Value.c_str()))
	{
		keyValues.insert_or_assign(Key, std::move(Value));
	}
}

const PropertyMap& LevelEntity::GetPropertyMap() const
{
	return keyValues;
}

