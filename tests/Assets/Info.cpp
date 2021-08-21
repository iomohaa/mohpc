#include "Common/Common.h"

#include <MOHPC/Utility/Info.h>

#include <cassert>

using namespace MOHPC;

static constexpr char MOHPC_LOG_NAMESPACE[] = "test_info";

int main(int argc, const char* argv[])
{
	InitCommon(argc, argv);

	Info info;
	info.SetValueForKey("testKey1", "value");
	info.SetValueForKey("testKey2", "value2");
	info.SetValueForKey("somekey", "somevalue");
	info.SetValueForKey("keyToBe", "deleted");
	info.SetValueForKey("foo", "bar");
	info.SetValueForKey("fu", "boor");
	info.RemoveKey("keyToBe");
	info.SetValueForKey("afterkey", "deletion");

	str someKeyVal = info.ValueForKey("somekey");
	assert(someKeyVal == "somevalue");

	someKeyVal = info.ValueForKey("keyToBe");
	assert(someKeyVal.empty());

	ReadOnlyInfo rinfo(info.GetString(), info.GetInfoLength());
	assert(!strHelpers::cmp(rinfo.ValueForKey("testKey1").c_str(), "value"));
	assert(!strHelpers::cmp(rinfo.ValueForKey("testKey2").c_str(), "value2"));
	assert(!strHelpers::cmp(rinfo.ValueForKey("foo").c_str(), "bar"));
	assert(!strHelpers::cmp(rinfo.ValueForKey("fu").c_str(), "boor"));
	assert(!strHelpers::cmp(rinfo.ValueForKey("afterkey").c_str(), "deletion"));
	assert(rinfo.ValueForKey("keyToBe").length() == 0);

	const str json = InfoJson::toJson<JsonStyleBeautifier>(rinfo);
	assert(json.length());
	MOHPC_LOG(Info, json.c_str());
}
