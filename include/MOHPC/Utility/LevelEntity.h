#pragma once

#include "UtilityGlobal.h"
#include "../Common/Vector.h"
#include "PropertyMap.h"

#include <MOHPC/Common/str.h>
#include <map>

namespace MOHPC
{
	MOHPC_UTILITY_EXPORTS str CanonicalModelName(const char* ModelName);

	class LevelEntity
	{
	public:
		LevelEntity(size_t entnum);

		MOHPC_UTILITY_EXPORTS size_t GetEntNum() const;
		MOHPC_UTILITY_EXPORTS const char* GetClassName() const;
		MOHPC_UTILITY_EXPORTS int32_t GetSpawnflags() const;
		MOHPC_UTILITY_EXPORTS const char* GetTargetName() const;
		void SetTargetName(const char* newTargetName);
		MOHPC_UTILITY_EXPORTS const char* GetTarget() const;
		MOHPC_UTILITY_EXPORTS const char* GetModel() const;
		MOHPC_UTILITY_EXPORTS bool IsClassOf(const char* classname) const;
		MOHPC_UTILITY_EXPORTS bool HasSubmodel() const;

		MOHPC_UTILITY_EXPORTS const_vec3p_t GetOrigin() const;
		MOHPC_UTILITY_EXPORTS const_vec3p_t GetAngles() const;

		MOHPC_UTILITY_EXPORTS bool HasProperty(const char* Key) const;
		MOHPC_UTILITY_EXPORTS const char* GetPropertyRawValue(const char* Key) const;
		MOHPC_UTILITY_EXPORTS const str& GetPropertyStringValue(const char* Key, const str& defaultValue = "") const;
		MOHPC_UTILITY_EXPORTS int8_t GetPropertyCharValue(const char* Key, int8_t defaultValue = 0) const;
		MOHPC_UTILITY_EXPORTS int16_t GetPropertyShortValue(const char* Key, int16_t defaultValue = 0) const;
		MOHPC_UTILITY_EXPORTS int32_t GetPropertyIntegerValue(const char* Key, int32_t defaultValue = 0) const;
		MOHPC_UTILITY_EXPORTS int64_t GetPropertyLongValue(const char* Key, int64_t defaultValue = 0) const;
		MOHPC_UTILITY_EXPORTS uint8_t GetPropertyByteValue(const char* Key, uint8_t defaultValue = 0) const;
		MOHPC_UTILITY_EXPORTS uint16_t GetPropertyUnsignedShortValue(const char* Key, uint16_t defaultValue = 0) const;
		MOHPC_UTILITY_EXPORTS uint32_t GetPropertyUnsignedIntegerValue(const char* Key, uint32_t defaultValue = 0) const;
		MOHPC_UTILITY_EXPORTS uint64_t GetPropertyUnsignedLongValue(const char* Key, uint64_t defaultValue = 0) const;
		MOHPC_UTILITY_EXPORTS float GetPropertyFloatValue(const char* Key, float defaultValue = 0) const;
		MOHPC_UTILITY_EXPORTS double GetPropertyDoubleValue(const char* Key, double defaultValue = 0) const;
		MOHPC_UTILITY_EXPORTS long double GetPropertyLongDoubleValue(const char* Key, long double defaultValue = 0) const;
		MOHPC_UTILITY_EXPORTS void GetPropertyVectorValue(const char* Key, vec3r_t out, const vec3r_t defaultValue = vec3_zero) const;
		MOHPC_UTILITY_EXPORTS bool TrySetMemberValue(const char* Key, const char* Value);
		MOHPC_UTILITY_EXPORTS void SetPropertyValue(const char* Key, const char* Value);
		void SetPropertyDef(const PropertyDef& Key, str&& Value);

		MOHPC_UTILITY_EXPORTS PropertyMapIterator GetIterator() const;

		MOHPC_UTILITY_EXPORTS const PropertyObject& GetPropertyObject() const;

	private:
		str model;
		vec3_t origin;
		vec3_t angles;
		size_t entnum;
		str classname;
		int32_t spawnflags;
		str targetname;
		str target;
		PropertyObject propertyObject;
	};
}
