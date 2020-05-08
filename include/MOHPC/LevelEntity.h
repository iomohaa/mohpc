#pragma once

#include "Vector.h"
#include "Script/str.h"
#include <map>

namespace MOHPC
{
	MOHPC_EXPORTS str CanonicalModelName(const char* ModelName);

	class PropertyDef
	{
	private:
		str propertyName;

	public:
		PropertyDef(const char* inPropertyName);
		PropertyDef(str&& inPropertyName);
		PropertyDef(const str& inPropertyName);

		bool operator<(const PropertyDef& right) const;

		MOHPC_EXPORTS const char* GetPropertyName() const;
		MOHPC_EXPORTS const char* GetFullPropertyName() const;

	private:
		bool IsKeyed() const;
	};
	using PropertyMap = std::map<PropertyDef, str>;

	class LevelEntity
	{
	private:
		size_t entnum;
		str classname;
		int32_t spawnflags;
		str targetname;
		str target;
		PropertyMap keyValues;

	public:
		str model;
		Vector origin;
		Vector angles;

	public:
		LevelEntity(size_t entnum);

		MOHPC_EXPORTS size_t GetEntNum() const;
		MOHPC_EXPORTS const char* GetClassName() const;
		MOHPC_EXPORTS int32_t GetSpawnflags() const;
		MOHPC_EXPORTS const char* GetTargetName() const;
		void SetTargetName(const char* newTargetName);
		MOHPC_EXPORTS const char* GetTarget() const;
		MOHPC_EXPORTS const char* GetModel() const;
		MOHPC_EXPORTS bool IsClassOf(const char* classname) const;
		MOHPC_EXPORTS bool HasSubmodel() const;

		MOHPC_EXPORTS const Vector& GetOrigin() const;
		MOHPC_EXPORTS const Vector& GetAngles() const;

		MOHPC_EXPORTS bool HasProperty(const char* Key) const;
		MOHPC_EXPORTS const char* GetPropertyRawValue(const char* Key) const;
		MOHPC_EXPORTS const str& GetPropertyStringValue(const char* Key, const str& defaultValue = "") const;
		MOHPC_EXPORTS int8_t GetPropertyCharValue(const char* Key, int8_t defaultValue = 0) const;
		MOHPC_EXPORTS int16_t GetPropertyShortValue(const char* Key, int16_t defaultValue = 0) const;
		MOHPC_EXPORTS int32_t GetPropertyIntegerValue(const char* Key, int32_t defaultValue = 0) const;
		MOHPC_EXPORTS int64_t GetPropertyLongValue(const char* Key, int64_t defaultValue = 0) const;
		MOHPC_EXPORTS uint8_t GetPropertyByteValue(const char* Key, uint8_t defaultValue = 0) const;
		MOHPC_EXPORTS uint16_t GetPropertyUnsignedShortValue(const char* Key, uint16_t defaultValue = 0) const;
		MOHPC_EXPORTS uint32_t GetPropertyUnsignedIntegerValue(const char* Key, uint32_t defaultValue = 0) const;
		MOHPC_EXPORTS uint64_t GetPropertyUnsignedLongValue(const char* Key, uint64_t defaultValue = 0) const;
		MOHPC_EXPORTS float GetPropertyFloatValue(const char* Key, float defaultValue = 0) const;
		MOHPC_EXPORTS double GetPropertyDoubleValue(const char* Key, double defaultValue = 0) const;
		MOHPC_EXPORTS long double GetPropertyLongDoubleValue(const char* Key, long double defaultValue = 0) const;
		MOHPC_EXPORTS Vector GetPropertyVectorValue(const char* Key, const Vector& defaultValue = Vector(0, 0, 0)) const;
		MOHPC_EXPORTS bool TrySetMemberValue(const char* Key, const char* Value);
		MOHPC_EXPORTS void SetPropertyValue(const char* Key, const char* Value);
		void SetPropertyDef(const PropertyDef& Key, str&& Value);
		MOHPC_EXPORTS const PropertyMap& GetPropertyMap() const;

	private:
		const str* GetPropertyValuePointer(const char* Key) const;
	};
}
