#pragma once

#include "../ProtocolSingleton.h"
#include "../Types/PlayerState.h"

namespace MOHPC
{
namespace Network
{
	namespace Parsing
	{
		struct MOHPC_NET_EXPORTS pvsParm_t
		{
		public:
			pvsParm_t();

		public:
			Vector origin;
			uint32_t clientNum;
			float radarRange;
			float radarFarMult;
		};

		class MOHPC_NET_EXPORTS IPVS : public IProtocolSingleton<IPVS>
		{
		public:
			virtual bool readNonPVSClient(radarInfo_t radarInfo, const pvsParm_t& parm, radarUnpacked_t& unpacked) const = 0;
			virtual radarInfo_t writeNonPVSClient(const pvsParm_t& parm, const radarUnpacked_t& unpacked) const = 0;
		};
	}
}
}
