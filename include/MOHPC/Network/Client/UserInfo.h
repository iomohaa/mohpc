#pragma once

#include "../../Common/str.h"
#include "../../Utilities/PropertyMap.h"

namespace MOHPC
{
	namespace Network
	{
		/**
		 * Handle client specific settings.
		 *
		 * It reflects userinfo from server.
		 */
		class ClientInfo
		{
			MOHPC_OBJECT_DECLARATION(ClientInfo);

		private:
			str name;
			uint32_t rate;
			uint32_t snaps;
			PropertyObject properties;

		private:
			MOHPC_EXPORTS ClientInfo();
			ClientInfo(ClientInfo&& other) = default;
			ClientInfo& operator=(ClientInfo&& other) = default;
			ClientInfo(const ClientInfo& other) = delete;
			ClientInfo& operator=(const ClientInfo& other) = delete;
			~ClientInfo() = default;

		public:
			/**
			 * Set/get the client rate, in kbps. Common rates are :
			 * - 2500 : 28.8k modem
			 * - 3000 : 33.6k modem
			 * - 4000 : 56k modem
			 * - 5000 : ISDN
			 * - 20000 : Cable
			 * - 25000 : xDSL
			 * - 30000 : LAN
			 */
			MOHPC_EXPORTS void setRate(uint32_t inRate);
			MOHPC_EXPORTS uint32_t getRate() const;

			/** Set/get the number of processed snapshots per second. */
			MOHPC_EXPORTS void setSnaps(uint32_t inSnaps);
			MOHPC_EXPORTS uint32_t getSnaps() const;

			/** Set/get the client name. */
			MOHPC_EXPORTS void setName(const char* newName);
			MOHPC_EXPORTS const char* getName() const;

			/** Set/get the client deathmatch allied model. */
			MOHPC_EXPORTS void setPlayerAlliedModel(const char* newModel);
			MOHPC_EXPORTS const char* getPlayerAlliedModel() const;

			/** Set/get the client deathmatch german model. */
			MOHPC_EXPORTS void setPlayerGermanModel(const char* newModel);
			MOHPC_EXPORTS const char* getPlayerGermanModel() const;

			/** Set/get an user value. */
			MOHPC_EXPORTS void setUserKeyValue(const char* key, const char* value);
			MOHPC_EXPORTS const char* getUserKeyValue(const char* key) const;

			/** Return the list of properties. */
			MOHPC_EXPORTS const PropertyObject& getPropertyObject() const;
		};
		using ClientInfoPtr = SharedPtr<ClientInfo>;
		using ConstClientInfoPtr = SharedPtr<const ClientInfo>;
	}
}