#include <MOHPC/Network/Event.h>
#include <MOHPC/Common/str.h>

using namespace MOHPC;
using namespace Network;

Event::Event()
{
}

void Event::AddInteger(int32_t value)
{
	new(data) EventData(value);
}

void Event::AddString(const char* value)
{
	new(data) EventData(value);
}

int32_t Event::GetInteger(size_t index) const
{
	return data[index - 1].GetInteger();
}

str Event::GetString(size_t index) const
{
	return data[index - 1].GetString();
}

size_t MOHPC::Network::Event::numArgs() const
{
	return data.NumObjects();
}

EventData::EventData(const char* value)
	: dataType(dataType_e::String)
{
	data.stringValue = new str(value);
}

EventData::EventData(int32_t value)
	: dataType(dataType_e::Integer)
{
	data.intValue = value;
}

EventData::EventData(EventData&& other)
	: dataType(other.dataType)
{
	data.voidValue = other.data.voidValue;
	other.dataType = dataType_e::None;
	other.data.voidValue = nullptr;
}

EventData::~EventData()
{
	switch (dataType)
	{
	case dataType_e::String:
		delete data.stringValue;
		break;
	default:
		break;
	}
}

EventData& EventData::operator=(EventData&& other)
{
	dataType = other.dataType;
	data.voidValue = other.data.voidValue;
	other.dataType = dataType_e::None;
	other.data.voidValue = nullptr;

	return *this;
}

int32_t EventData::GetInteger() const
{
	switch (dataType)
	{
	case dataType_e::Integer:
		return data.intValue;
	case dataType_e::String:
		assert(data.stringValue);
		return atoi(data.stringValue->c_str());
	default:
		return 0;
	}
}

str EventData::GetString() const
{
	switch (dataType)
	{
	case dataType_e::String:
		assert(data.stringValue);
		return *data.stringValue;
	case dataType_e::Integer:
		return data.intValue;
	default:
		return 0;
	}
}
