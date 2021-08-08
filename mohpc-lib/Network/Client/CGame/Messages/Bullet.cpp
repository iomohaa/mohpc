#include <MOHPC/Network/Client/CGame/Messages/Bullet.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;
using namespace MOHPC::Network::CGame::Messages;

void BulletImpl::CreateBulletTracer(const vec3r_t barrel, const vec3r_t start, const vec3r_t end, uint32_t numBullets, uint32_t iLarge, uint32_t numTracersVisible, float bulletSize)
{
	createBulletTracerHandler.broadcast(barrel, start, end, numBullets, iLarge, numTracersVisible, bulletSize);
}

void BulletImpl::CreateBubbleTrail(const vec3r_t start, const vec3r_t end, uint32_t iLarge, float bulletSize)
{
	createBubbleTrailHandler.broadcast(start, end, iLarge, bulletSize);
}
