#pragma once

#include "../NetGlobal.h"
#include "../NetObject.h"
#include "../../Utility/PropertyMap.h"
#include "../../Common/str.h"

namespace MOHPC
{
	class Info;

	namespace Network
	{
		/**
		 * Keep track of user-info locally and remotely.
		 *
		 * It reflects user information from server.
		 * Movable and non-copyable.
		 */
		class UserInfo
		{
			MOHPC_NET_OBJECT_DECLARATION(UserInfo);

		public:
			/**
			 * Set the client rate, in kbps. Common rates are :
			 * - 2500 : 28.8k modem
			 * - 3000 : 33.6k modem
			 * - 4000 : 56k modem
			 * - 5000 : ISDN
			 * - 20000 : Cable
			 * - 25000 : xDSL
			 * - 30000 : LAN
			 */
			MOHPC_NET_EXPORTS void setRate(uint32_t rateVal);
			/** Get the client rate. */
			MOHPC_NET_EXPORTS uint32_t getRate() const;

			/** Set the number of processed snapshots per second. */
			MOHPC_NET_EXPORTS void setSnaps(uint32_t snapsVal);
			/** Get the number of processed snapshots per second. */
			MOHPC_NET_EXPORTS uint32_t getSnaps() const;

			/** Set the client name. */
			MOHPC_NET_EXPORTS void setName(const char* newName);
			/** Get the client name. */
			MOHPC_NET_EXPORTS const char* getName() const;

			/** Set an user value. */
			MOHPC_NET_EXPORTS void setUserKeyValue(const char* key, const char* value);
			/** Get an user value. */
			MOHPC_NET_EXPORTS const char* getUserKeyValue(const char* key) const;

			/** Return the list of properties. */
			MOHPC_NET_EXPORTS const PropertyObject& getPropertyObject() const;

		private:
			MOHPC_NET_EXPORTS UserInfo();
			MOHPC_NET_EXPORTS UserInfo(UserInfo&& other);
			MOHPC_NET_EXPORTS UserInfo& operator=(UserInfo&& other);
			UserInfo(const UserInfo & other) = delete;
			UserInfo& operator=(const UserInfo & other) = delete;
			MOHPC_NET_EXPORTS ~UserInfo();

		private:
			str name;
			uint32_t rate;
			uint32_t snaps;
			PropertyObject properties;
		};

		using UserInfoPtr = SharedPtr<UserInfo>;
		using ConstUserInfoPtr = SharedPtr<const UserInfo>;

		namespace UserInfoHelpers
		{
			MOHPC_NET_EXPORTS void fillInfoString(const UserInfo& userInfo, Info& info);
		};
	}
}