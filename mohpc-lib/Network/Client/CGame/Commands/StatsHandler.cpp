#include <MOHPC/Network/Client/CGame/Commands/StatsHandler.h>
#include <MOHPC/Network/Client/CGame/Stats.h>
#include <MOHPC/Utility/TokenParser.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(StatsCommandHandler);

StatsCommandHandler::StatsCommandHandler(const SnapshotProcessorPtr& snapshotProcessor)
	: SingleCommandHandlerBase(snapshotProcessor, "stats")
{
}

StatsCommandHandler::~StatsCommandHandler()
{
}

void StatsCommandHandler::handle(TokenParser& args)
{
	stats_t stats;

	stats.numObjectives = args.GetInteger(false);
	stats.numComplete = args.GetInteger(false);
	stats.numShotsFired = args.GetInteger(false);
	stats.numHits = args.GetInteger(false);
	stats.accuracy = args.GetInteger(false);
	stats.preferredWeapon = args.GetToken(false);
	stats.numHitsTaken = args.GetInteger(false);
	stats.numObjectsDestroyed = args.GetInteger(false);
	stats.numEnemysKilled = args.GetInteger(false);
	stats.headshots = args.GetInteger(false);
	stats.torsoShots = args.GetInteger(false);
	stats.leftLegShots = args.GetInteger(false);
	stats.rightLegShots = args.GetInteger(false);
	stats.groinShots = args.GetInteger(false);
	stats.leftArmShots = args.GetInteger(false);
	stats.rightArmShots = args.GetInteger(false);
	stats.gunneryEvaluation = args.GetInteger(false);
	stats.gotMedal = args.GetInteger(false);
	stats.success = args.GetInteger(false);
	stats.failed = args.GetInteger(false);

	getHandler().broadcast(stats);
}