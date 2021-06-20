#include <MOHPC/Network/Client/GamespyRequest.h>
#include <MOHPC/Utility/Info.h>

using namespace MOHPC;
using namespace Network;

void IGamespyRequest::generateOutput(IMessageStream& output)
{
	Info info;
	generateInfo(info);

	// Send the type of query
	const char* queryName = queryId();

	const char* infoStr = info.GetString();
	if (infoStr)
	{
		output.Write(infoStr, strlen(infoStr));
		output.Write("\\final\\", 7);

		if (queryName) {
			output.Write("\\queryid\\", 9);
			output.Write(queryName, strlen(queryName));
			output.Write("\\", 1);
		}
	}
}

void IGamespyServerRequest::generateOutput(IMessageStream& output)
{
	// Send the type of query
	const char* queryName = generateQuery();

	output.Write("\\", 1);
	output.Write(queryName, strlen(queryName));
	output.Write("\\", 1);
}
