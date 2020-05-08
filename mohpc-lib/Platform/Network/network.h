#include <MOHPC/Network/Socket.h>

namespace MOHPC
{
	namespace Platform
	{
		class Network
		{
		public:
			static MOHPC::Network::ISocketFactoryPtr createSockFactory();
		};
	}
}