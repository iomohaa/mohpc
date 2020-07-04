#include <MOHPC/Utilities/PropertyMap.h>
#include <string>

using namespace MOHPC;

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
	return str::icmp(propertyName, right.propertyName) < 0;
}

const char* PropertyDef::GetPropertyName() const
{
	return IsKeyed() ? propertyName.c_str() + 1 : propertyName.c_str();
}

const char* PropertyDef::GetFullPropertyName() const
{
	return propertyName.c_str();
}

bool PropertyDef::IsKeyed() const
{
	return propertyName[0] == '#' || propertyName[0] == '$';
}

bool PropertyObject::HasProperty(const char* Key) const
{
	return GetPropertyValuePointer(Key) != nullptr;
}

const char* PropertyObject::GetPropertyRawValue(const char* Key) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return nullptr;
	}
	return val->c_str();
}

const str& PropertyObject::GetPropertyStringValue(const char* Key, const str& defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return *val;
}

int8_t PropertyObject::GetPropertyCharValue(const char* Key, int8_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return *val->c_str();
}

int16_t PropertyObject::GetPropertyShortValue(const char* Key, int16_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return (int16_t)std::stoi(val->c_str());
}

int32_t PropertyObject::GetPropertyIntegerValue(const char* Key, int32_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stoi(val->c_str());
}

int64_t PropertyObject::GetPropertyLongValue(const char* Key, int64_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stoll(val->c_str());
}

uint8_t PropertyObject::GetPropertyByteValue(const char* Key, uint8_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return (uint8_t)*val->c_str();
}

uint16_t PropertyObject::GetPropertyUnsignedShortValue(const char* Key, uint16_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return (uint16_t)std::stoul(val->c_str());
}

uint32_t PropertyObject::GetPropertyUnsignedIntegerValue(const char* Key, uint32_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stoul(val->c_str());
}

uint64_t PropertyObject::GetPropertyUnsignedLongValue(const char* Key, uint64_t defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stoull(val->c_str());
}

float PropertyObject::GetPropertyFloatValue(const char* Key, float defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stof(val->c_str());
}

double PropertyObject::GetPropertyDoubleValue(const char* Key, double defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stod(val->c_str());
}

long double PropertyObject::GetPropertyLongDoubleValue(const char* Key, long double defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return std::stold(val->c_str());
}

Vector PropertyObject::GetPropertyVectorValue(const char* Key, const Vector& defaultValue) const
{
	const str* val = GetPropertyValuePointer(Key);
	if (!val)
	{
		return defaultValue;
	}
	return Vector(val->c_str());
}

const str* PropertyObject::GetPropertyValuePointer(const char* Key) const
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

void PropertyObject::SetPropertyValue(const char* Key, const char* Value)
{
	keyValues.insert_or_assign(Key, Value);
}

void PropertyObject::SetPropertyDef(const PropertyDef& Key, str&& Value)
{
	keyValues.insert_or_assign(Key, std::move(Value));
}

PropertyMapIterator PropertyObject::GetIterator() const
{
	return PropertyMapIterator(keyValues);
}

const PropertyMap& PropertyObject::GetPropertyMap() const
{
	return keyValues;
}

void MOHPC::PropertyObject::clear()
{
	keyValues.clear();
}

bool PropertyObject::hasAnyProperty() const
{
	return keyValues.size() > 0;
}

size_t PropertyObject::count() const
{
	return keyValues.size();
}

PropertyMapIterator::PropertyMapIterator(const PropertyMap& inMap)
	: map(&inMap)
{
	it = new PropertyMap::const_iterator(map->begin());
}

PropertyMapIterator::PropertyMapIterator(PropertyMapIterator&& other)
{
	it = other.it;
	map = other.map;
	other.it = nullptr;
	other.map = nullptr;
}

PropertyMapIterator::PropertyMapIterator()
{
	it = nullptr;
	map = nullptr;
}

PropertyMapIterator& PropertyMapIterator::operator=(PropertyMapIterator&& other)
{
	it = other.it;
	map = other.map;
	other.it = nullptr;
	other.map = nullptr;
	return *this;
}

PropertyMapIterator::~PropertyMapIterator()
{
	delete it;
}

PropertyMapIterator::operator bool() const
{
	return it && *it != map->end();
}

const PropertyDef& PropertyMapIterator::key() const
{
	return (*it)->first;
}

const str& PropertyMapIterator::value() const
{
	return (*it)->second;
}

PropertyMapIterator& PropertyMapIterator::operator++()
{
	++(*it);
	return *this;
}

const PropertyMap* PropertyMapIterator::getMap() const
{
	return map;
}
