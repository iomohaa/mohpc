#include <MOHPC/Network/Parsing/PVS.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::Parsing;

static constexpr char MOHPC_LOG_NAMESPACE[] = "parsing_pvs";

pvsParm_t::pvsParm_t()
	: clientNum(0)
	, radarRange(256.f)
	, radarFarMult(1024.f)
{

}

class PVS8 : public Parsing::IPVS
{
public:
	virtual void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 5;
		maxRange = 8;
	}

	virtual bool readNonPVSClient(radarInfo_t radarInfo, const pvsParm_t& parm, radarUnpacked_t& unpacked) const override
	{
		return false;
	}

	radarInfo_t writeNonPVSClient(const pvsParm_t& parm, const radarUnpacked_t& unpacked) const override
	{
		return radarInfo_t();
	}
};

class PVS17 : public PVS8
{
public:
	virtual void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 15;
		maxRange = 17;
	}

	virtual bool readNonPVSClient(radarInfo_t radarInfo, const pvsParm_t& parm, radarUnpacked_t& unpacked) const override
	{
		if (unpackNonPVSClient(radarInfo, unpacked, parm))
		{
			unpacked.x += parm.origin[0];
			unpacked.y += parm.origin[1];

			return true;
		}

		return false;
	}

	bool unpackNonPVSClient(radarInfo_t radarInfo, radarUnpacked_t& unpacked, const pvsParm_t& parm) const
	{
		unpacked.clientNum = radarInfo.clientNum();

		if (unpacked.clientNum == parm.clientNum) {
			return false;
		}

		const float radarScaled = parm.radarRange / radarInfo.getCoordPrecision();

		unpacked.x = radarInfo.x() * radarScaled;
		unpacked.y = radarInfo.y() * radarScaled;

		if (radarInfo.flags() & RADAR_PLAYER_FAR)
		{
			// when it's too far it needs to be scaled to make it look very far away
			unpacked.x = unpacked.x * parm.radarFarMult;
			unpacked.y = unpacked.y * parm.radarFarMult;
		}

		// retrieve the yaw from 5-bits value
		unpacked.yaw = radarInfo.yaw() * radarInfo.getYawPrecision();

		return true;
	}

	radarInfo_t writeNonPVSClient(const pvsParm_t& parm, const radarUnpacked_t& unpacked) const override
	{
		const float relx = unpacked.x - parm.origin[0];
		const float rely = unpacked.y - parm.origin[1];

		const int8_t yaw = int8_t(unpacked.yaw / radarInfo_t::getYawPrecision());
		int8_t flags = 0;
		int8_t x = 0, y = 0;

		const int8_t cmin = radarInfo_t::getMinCoord();
		const int8_t cmax = radarInfo_t::getMaxCoord();

		if (relx < cmin || rely < cmin
		 || rely > cmax || rely > cmax)
		{
			flags |= RADAR_PLAYER_FAR;
		}

		if (relx < cmin) x = cmin;
		else if (relx > cmax) x = cmax;
		else x = (int8_t)relx;

		if (rely < cmin) y = cmin;
		else if (rely > cmax) y = cmax;
		else y = (int8_t)rely;

		return radarInfo_t(
			unpacked.clientNum,
			x,
			y,
			yaw,
			flags
		);
	}
};

static PVS8 PVSVersion8;
static PVS17 PVSVersion17;
