#pragma once

#include "../../NetGlobal.h"
#include "../../Configstring.h"

#include "../../../Common/str.h"
#include "../../../Utility/Info.h"
#include "../../../Utility/PropertyMap.h"
#include "../UserInfo.h"

#include "GameType.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	class ServerGameState;

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
		public:
			ClientInfoList();

			/** Get a client info in the interval of [0, MAX_CLIENTS]. */
			MOHPC_NET_EXPORTS const clientInfo_t& get(uint32_t clientNum) const;
			const clientInfo_t& set(const ReadOnlyInfo& info, uint32_t clientNum);

			void setPtrs(
				const ServerGameState* gameStatePtr,
				const UserInfoPtr& userInfoPtr
			);

		private:
			void reflectLocalClient(const clientInfo_t& client);

		private:
			const ServerGameState* gameState;
			UserInfoPtr userInfo;
			clientInfo_t clientInfo[MAX_CLIENTS];
		};
	}
}
}