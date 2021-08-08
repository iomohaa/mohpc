#include <MOHPC/Network/Client/CGame/Trace.h>
#include <MOHPC/Network/Client/CGame/Snapshot.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

TraceManager::TraceManager()
	: numSolidEntities(0)
	, numTriggerEntities(0)
{
	// Create predefined box hull
	boxHull = CollisionWorld::create();
	boxHull->InitBoxHull();
}

void TraceManager::clipMoveToEntities(CollisionWorld& cm, const_vec3r_t start, const_vec3r_t mins, const_vec3r_t maxs, const_vec3r_t end, uint16_t skipNumber, uint32_t mask, bool cylinder, trace_t& tr) const
{
	// iterate through entities and test their collision
	for (size_t i = 0; i < numSolidEntities; i++)
	{
		const EntityInfo* cent = solidEntities[i];
		const entityState_t* ent = &cent->currentState;

		if (ent->number == skipNumber) {
			continue;
		}

		CollisionWorld* world = &cm;
		clipHandle_t cmodel;
		vec3_t bmins, bmaxs;
		vec3_t origin, angles;

		if (ent->solid == SOLID_BMODEL)
		{
			// special value for bmodel
			cmodel = cm.inlineModel(ent->modelindex);
			if (!cmodel) continue;
			VectorCopy(cent->currentState.netangles, angles);
			VectorCopy(cent->currentState.netorigin, origin);
		}
		else
		{
			// encoded bbox
			IntegerToBoundingBox(ent->solid, bmins, bmaxs);
			cmodel = boxHull->TempBoxModel(bmins, bmaxs, ContentFlags::CONTENTS_BODY);
			VectorClear(angles);
			VectorCopy(cent->currentState.netorigin, origin);
			// trace to the boxhull instead
			// entities with a boundingbox use a boxhull
			world = boxHull.get();
		}

		trace_t trace;
		// trace through the entity's submodel
		world->TransformedBoxTrace(&trace, start, end,
			mins, maxs, cmodel, mask, origin, angles, cylinder);

		if (trace.allsolid || trace.fraction < tr.fraction) {
			trace.entityNum = ent->number;
			tr = trace;
		}
		else if (trace.startsolid) {
			tr.startsolid = true;
		}
	}
}

void TraceManager::trace(CollisionWorld& cm, trace_t& tr, const_vec3r_t start, const_vec3r_t mins, const_vec3r_t maxs, const_vec3r_t end, uint16_t skipNumber, uint32_t mask, bool cylinder, bool cliptoentities) const
{
	// if there is a loaded collision from the game, use it instead
	cm.BoxTrace(&tr, start, end, mins, maxs, 0, mask, cylinder);
	// collision defaults to world
	tr.entityNum = tr.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;

	if (tr.startsolid) {
		tr.entityNum = ENTITYNUM_WORLD;
	}

	if (cliptoentities)
	{
		// also trace through entities
		clipMoveToEntities(cm, start, mins, maxs, end, skipNumber, mask, cylinder, tr);
	}
}

uint32_t TraceManager::pointContents(CollisionWorld& cm, const_vec3r_t point, uintptr_t passEntityNum) const
{
	// get the contents in world
	uint32_t contents = cm.PointContents(point, 0);

	// also iterate through entities (that are using a submodel) to check if the point is inside
	for (size_t i = 0; i < numSolidEntities; i++)
	{
		const EntityInfo* cent = solidEntities[i];
		const entityState_t* ent = &cent->currentState;

		if (ent->number == passEntityNum) {
			continue;
		}

		// special value for bmodel
		if (ent->solid != SOLID_BMODEL) {
			continue;
		}

		clipHandle_t cmodel = cm.inlineModel(ent->modelindex);
		if (!cmodel) {
			continue;
		}

		contents |= cm.TransformedPointContents(point, cmodel, ent->netorigin, ent->netangles);
	}

	return contents;
}

void TraceManager::buildSolidList(const SnapshotProcessor& snapshotProcessor)
{
	numSolidEntities = 0;
	numTriggerEntities = 0;

	const SnapshotInfo* snap;

	if (snapshotProcessor.getNextSnap() && !snapshotProcessor.doesTeleportNextFrame() && !snapshotProcessor.doesTeleportThisFrame()) {
		snap = snapshotProcessor.getNextSnap();
	}
	else {
		snap = snapshotProcessor.getSnap();
	}

	for (uintptr_t i = 0; i < snap->numEntities; i++)
	{
		const EntityInfo* cent = snapshotProcessor.getEntity(snap->entities[i].number);
		const entityState_t* ent = &cent->currentState;

		// Ignore item/triggers, they're always non-solid
		if (ent->eType == entityType_e::item || ent->eType == entityType_e::push_trigger || ent->eType == entityType_e::teleport_trigger) {
			continue;
		}

		if (cent->nextState.solid)
		{
			// Add solid entities
			solidEntities[numSolidEntities] = cent;
			numSolidEntities++;
			continue;
		}
	}
}
