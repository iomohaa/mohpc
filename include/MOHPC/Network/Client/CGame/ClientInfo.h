#pragma once

#include "../../NetGlobal.h"
#include "../../NetObject.h"
#include "../../Configstring.h"

#include "../../../Common/str.h"
#include "../../../Utility/Info.h"
#include "../../../Utility/PropertyMap.h"

#include "GameType.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		struct clientInfo_t
		{
		public:
			clientInfo_t();

			/** Name of the client. */
			MOHPC_NET_EXPORTS const char* getName() const;

			/** Client's current team. */
			MOHPC_NET_EXPORTS teamType_e getTeam() const;

			/** List of misc client properties. */
			MOHPC_NET_EXPORTS const PropertyObject& getProperties() const;

		public:
			str name;
			teamType_e team;
			PropertyObject properties;
		};

		class ClientInfoList
		{
			MOHPC_NET_OBJECT_DECLARATION(ClientInfoList);

		public:
			MOHPC_NET_EXPORTS ClientInfoList();
			MOHPC_NET_EXPORTS ~ClientInfoList();

			ClientInfoList(ClientInfoList&&) = delete;
			ClientInfoList(const ClientInfoList&) = delete;
			ClientInfoList& operator=(ClientInfoList&&) = delete;
			ClientInfoList& operator=(const ClientInfoList&) = delete;

			/** Get a client info in the interval of [0, MAX_CLIENTS]. */
			MOHPC_NET_EXPORTS const clientInfo_t& get(uint32_t clientNum) const;
			MOHPC_NET_EXPORTS const clientInfo_t& set(const ReadOnlyInfo& info, uint32_t clientNum);

		private:
			clientInfo_t* clientInfo;
		};
		using ClientInfoListPtr = SharedPtr<ClientInfoList>;
	}
}
}