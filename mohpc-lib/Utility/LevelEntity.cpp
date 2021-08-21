#include <Shared.h>
#include <MOHPC/Utility/LevelEntity.h>
#include <MOHPC/Files/FileHelpers.h>

#include <cstring>

using namespace MOHPC;

str MOHPC::CanonicalModelName(const char* ModelName)
{
	if (*ModelName != '*')
	{
		if (!strHelpers::icmpn(ModelName, "models/", 7))
		{
			return FileHelpers::CanonicalFilename(ModelName);
		}
		else
		{
			return "models/" + FileHelpers::CanonicalFilename(ModelName);
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
	VectorClear(origin);
	VectorClear(angles);
}

size_t LevelEntity::GetEntNum() const
{
	return entnum;
}

const char* LevelEntity::GetClassName() const
{
	return !classname.empty() ? classname.c_str() : "";
}

int32_t LevelEntity::GetSpawnflags() const
{
	return spawnflags;
}

const char* LevelEntity::GetTargetName() const
{
	return !targetname.empty() ? targetname.c_str() : "";
}

const char* LevelEntity::GetTarget() const
{
	return !target.empty() ? target.c_str() : "";
}

void MOHPC::LevelEntity::SetTargetName(const char* newTargetName)
{
	targetname = newTargetName;
}

const char* MOHPC::LevelEntity::GetModel() const
{
	return !model.empty() ? model.c_str() : "";
}

const_vec3p_t MOHPC::LevelEntity::GetOrigin() const
{
	return origin;
}

const_vec3p_t MOHPC::LevelEntity::GetAngles() const
{
	return angles;
}

bool LevelEntity::IsClassOf(const char* c) const
{
	return !strHelpers::icmp(classname.c_str(), c);
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

void LevelEntity::GetPropertyVectorValue(const char* Key, vec3r_t out, const vec3r_t defaultValue) const
{
	propertyObject.GetPropertyVectorValue(Key, out, defaultValue);
}

bool MOHPC::LevelEntity::TrySetMemberValue(const char* Key, const char* Value)
{
	if (!strHelpers::icmp(Key, "model"))
	{
		model = CanonicalModelName(Value);
	}
	else if (!strHelpers::icmp(Key, "origin"))
	{
		VectorFromString(Value, origin);
	}
	else if (!strHelpers::icmp(Key, "angles"))
	{
		VectorFromString(Value, angles);
	}
	else if (!strHelpers::icmp(Key, "angle"))
	{
		try
		{
			angles[0] = 0.f;
			angles[1] = (float)atof(Key);
			angles[2] = 0.f;
		}
		catch (const std::exception&)
		{
			VectorClear(angles);
		}
	}
	else if (!strHelpers::icmp(Key, "classname"))
	{
		if (classname.empty())
		{
			// Only set if not set already
			classname = Value;
		}
	}
	else if (!strHelpers::icmp(Key, "spawnflags"))
	{
		spawnflags = atoi(Value);
	}
	else if (!strHelpers::icmp(Key, "targetname"))
	{
		targetname = Value;
	}
	else if (!strHelpers::icmp(Key, "target"))
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
