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

LevelEntity::LevelEntity(size_t num)
{
	entnum = num;
	spawnflags = 0;
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
	return propertyObject.HasProperty(Key);
}

const char* LevelEntity::GetPropertyRawValue(const char* Key) const
{
	return propertyObject.GetPropertyRawValue(Key);
}

const str& LevelEntity::GetPropertyStringValue(const char* Key, const str& defaultValue) const
{
	return propertyObject.GetPropertyStringValue(Key, defaultValue);
}

int8_t LevelEntity::GetPropertyCharValue(const char* Key, int8_t defaultValue) const
{
	return propertyObject.GetPropertyCharValue(Key, defaultValue);
}

int16_t LevelEntity::GetPropertyShortValue(const char* Key, int16_t defaultValue) const
{
	return propertyObject.GetPropertyShortValue(Key, defaultValue);
}

int32_t LevelEntity::GetPropertyIntegerValue(const char* Key, int32_t defaultValue) const
{
	return propertyObject.GetPropertyIntegerValue(Key, defaultValue);
}

int64_t LevelEntity::GetPropertyLongValue(const char* Key, int64_t defaultValue) const
{
	return propertyObject.GetPropertyLongValue(Key, defaultValue);
}

uint8_t LevelEntity::GetPropertyByteValue(const char* Key, uint8_t defaultValue) const
{
	return propertyObject.GetPropertyByteValue(Key, defaultValue);
}

uint16_t LevelEntity::GetPropertyUnsignedShortValue(const char* Key, uint16_t defaultValue) const
{
	return propertyObject.GetPropertyUnsignedShortValue(Key, defaultValue);
}

uint32_t LevelEntity::GetPropertyUnsignedIntegerValue(const char* Key, uint32_t defaultValue) const
{
	return propertyObject.GetPropertyUnsignedIntegerValue(Key, defaultValue);
}

uint64_t LevelEntity::GetPropertyUnsignedLongValue(const char* Key, uint64_t defaultValue) const
{
	return propertyObject.GetPropertyUnsignedLongValue(Key, defaultValue);
}

float LevelEntity::GetPropertyFloatValue(const char* Key, float defaultValue) const
{
	return propertyObject.GetPropertyFloatValue(Key, defaultValue);
}

double LevelEntity::GetPropertyDoubleValue(const char* Key, double defaultValue) const
{
	return propertyObject.GetPropertyDoubleValue(Key, defaultValue);
}

long double LevelEntity::GetPropertyLongDoubleValue(const char* Key, long double defaultValue) const
{
	return propertyObject.GetPropertyLongDoubleValue(Key, defaultValue);
}

Vector LevelEntity::GetPropertyVectorValue(const char* Key, const Vector& defaultValue) const
{
	return propertyObject.GetPropertyVectorValue(Key, defaultValue);
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
		return propertyObject.SetPropertyValue(Key, Value);
	}
}

void MOHPC::LevelEntity::SetPropertyDef(const PropertyDef& Key, str&& Value)
{
	if (!TrySetMemberValue(Key.GetPropertyName(), Value.c_str()))
	{
		return propertyObject.SetPropertyDef(Key, std::move(Value));
	}
}

PropertyMapIterator MOHPC::LevelEntity::GetIterator() const
{
	return propertyObject.GetIterator();
}

const MOHPC::PropertyObject& LevelEntity::GetPropertyObject() const
{
	return propertyObject;
}
