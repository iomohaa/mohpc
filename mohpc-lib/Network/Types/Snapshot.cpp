#include <MOHPC/Network/Types/Snapshot.h>

using namespace MOHPC;
using namespace MOHPC::Network;

rawSnapshot_t::rawSnapshot_t()
	: valid(false)
	, snapFlags(0)
	, serverTime(0)
	, serverTimeResidual(0)
	, messageNum(0)
	, deltaNum(0)
	, areamask{ 0 }
	, cmdNum(0)
	, numEntities(0)
	, parseEntitiesNum(0)
	, serverCommandNum(0)
	, numSounds(0)
{
}
