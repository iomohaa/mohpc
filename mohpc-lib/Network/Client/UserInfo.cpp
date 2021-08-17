#include <MOHPC/Network/Client/UserInfo.h>
#include <MOHPC/Utility/Info.h>

using namespace MOHPC;
using namespace MOHPC::Network;

MOHPC_OBJECT_DEFINITION(UserInfo);

UserInfo::UserInfo()
	: snaps(20)
	, rate(5000)
{
}

UserInfo::UserInfo(UserInfo&& other)
	: name(std::move(other.name))
	, rate(other.rate)
	, snaps(other.snaps)
	, properties(std::move(other.properties))
{
}

UserInfo& UserInfo::operator=(UserInfo&& other)
{
	name = std::move(other.name);
	rate = other.rate;
	snaps = other.snaps;
	properties = std::move(other.properties);
	return *this;
}

UserInfo::~UserInfo()
{
}

void UserInfo::setRate(uint32_t rateVal)
{
	rate = rateVal;
}

uint32_t UserInfo::getRate() const
{
	return rate;
}

void UserInfo::setSnaps(uint32_t snapsVal)
{
	snaps = snapsVal;
}

uint32_t UserInfo::getSnaps() const
{
	return snaps;
}

void UserInfo::setName(const char* newName)
{
	name = newName;
}

const char* UserInfo::getName() const
{
	return name.c_str();
}

void UserInfo::setUserKeyValue(const char* key, const char* value)
{
	properties.SetPropertyValue(key, value);
}

const char* UserInfo::getUserKeyValue(const char* key) const
{
	return properties.GetPropertyRawValue(key);
}

const PropertyObject& UserInfo::getPropertyObject() const
{
	return properties;
}

void UserInfoHelpers::fillInfoString(const UserInfo& userInfo, Info& info)
{
	// Build mandatory variables
	info.SetValueForKey("rate", std::to_string(userInfo.getRate()).c_str());
	info.SetValueForKey("snaps", std::to_string(userInfo.getSnaps()).c_str());
	info.SetValueForKey("name", userInfo.getName());

	// Build miscellaneous values
	for (PropertyMapIterator it = userInfo.getPropertyObject().GetIterator(); it; ++it)
	{
		info.SetValueForKey(
			it.key().GetFullPropertyName(),
			it.value().c_str()
		);
	}
}
