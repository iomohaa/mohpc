#include <MOHPC/Network/Serializable/PlayerState.h>
#include <MOHPC/Network/Serializable/NetField.h>
#include <MOHPC/Network/Serializable/EntityField.h>

#include <MOHPC/Network/Types/PlayerState.h>

#include <MOHPC/Utility/Misc/MSG/MSGCoord.h>

using namespace MOHPC;
using namespace MOHPC::Network;

#define	PSF(x) #x,(uint16_t)(size_t)&((playerState_t*)0)->x,sizeof(playerState_t::x)

const netField_template_t<fieldType_ver6_e> playerStateFields[] =
{
{ PSF(commandTime), 32, fieldType_ver6_e::regular },
{ PSF(origin[0]), 0, fieldType_ver6_e::coord },
{ PSF(origin[1]), 0, fieldType_ver6_e::coord },
{ PSF(viewangles[1]), 0, fieldType_ver6_e::regular },
{ PSF(velocity[1]), 0, fieldType_ver6_e::velocity },
{ PSF(velocity[0]), 0, fieldType_ver6_e::velocity },
{ PSF(viewangles[0]), 0, fieldType_ver6_e::regular },
{ PSF(pm_time), -16, fieldType_ver6_e::regular },
{ PSF(origin[2]), 0, fieldType_ver6_e::coord },
{ PSF(velocity[2]), 0, fieldType_ver6_e::velocity },
{ PSF(iViewModelAnimChanged), 2, fieldType_ver6_e::regular },
{ PSF(damage_angles[0]), -13, fieldType_ver6_e::angle },
{ PSF(damage_angles[1]), -13, fieldType_ver6_e::angle },
{ PSF(damage_angles[2]), -13, fieldType_ver6_e::angle },
{ PSF(speed), 16, fieldType_ver6_e::regular },
{ PSF(delta_angles[1]), 16, fieldType_ver6_e::regular },
{ PSF(viewheight), -8, fieldType_ver6_e::regular },
{ PSF(groundEntityNum), GENTITYNUM_BITS, fieldType_ver6_e::regular },
{ PSF(delta_angles[0]), 16, fieldType_ver6_e::regular },
{ PSF(iViewModelAnim), 4, fieldType_ver6_e::regular },
{ PSF(fov), 0, fieldType_ver6_e::regular },
{ PSF(current_music_mood), 8, fieldType_ver6_e::regular },
{ PSF(gravity), 16, fieldType_ver6_e::regular },
{ PSF(fallback_music_mood), 8, fieldType_ver6_e::regular },
{ PSF(music_volume), 0, fieldType_ver6_e::regular },
{ PSF(pm_flags), 16, fieldType_ver6_e::regular },
{ PSF(clientNum), 8, fieldType_ver6_e::regular },
{ PSF(fLeanAngle), 0, fieldType_ver6_e::regular },
{ PSF(blend[3]), 0, fieldType_ver6_e::regular },
{ PSF(blend[0]), 0, fieldType_ver6_e::regular },
{ PSF(pm_type), 8, fieldType_ver6_e::regular },
{ PSF(feetfalling), 8, fieldType_ver6_e::regular },
{ PSF(camera_angles[0]), 16, fieldType_ver6_e::angle },
{ PSF(camera_angles[1]), 16, fieldType_ver6_e::angle },
{ PSF(camera_angles[2]), 16, fieldType_ver6_e::angle },
{ PSF(camera_origin[0]), 0, fieldType_ver6_e::coord },
{ PSF(camera_origin[1]), 0, fieldType_ver6_e::coord },
{ PSF(camera_origin[2]), 0, fieldType_ver6_e::coord },
{ PSF(camera_posofs[0]), 0, fieldType_ver6_e::coord },
{ PSF(camera_posofs[2]), 0, fieldType_ver6_e::coord },
{ PSF(camera_time), 0, fieldType_ver6_e::regular },
{ PSF(bobCycle), 8, fieldType_ver6_e::regular },
{ PSF(delta_angles[2]), 16, fieldType_ver6_e::regular },
{ PSF(viewangles[2]), 0, fieldType_ver6_e::regular },
{ PSF(music_volume_fade_time), 0, fieldType_ver6_e::regular },
{ PSF(reverb_type), 6, fieldType_ver6_e::regular },
{ PSF(reverb_level), 0, fieldType_ver6_e::regular },
{ PSF(blend[1]), 0, fieldType_ver6_e::regular },
{ PSF(blend[2]), 0, fieldType_ver6_e::regular },
{ PSF(camera_offset[0]), 0, fieldType_ver6_e::regular },
{ PSF(camera_offset[1]), 0, fieldType_ver6_e::regular },
{ PSF(camera_offset[2]), 0, fieldType_ver6_e::regular },
{ PSF(camera_posofs[1]), 0, fieldType_ver6_e::coord },
{ PSF(camera_flags), 16, fieldType_ver6_e::regular }
};

static_assert(sizeof(playerStateFields) == sizeof(netField_template_t<fieldType_ver6_e>) * 54);

const netField_template_t<fieldType_ver15_e> playerStateFields_ver15[] =
{
{ PSF(commandTime), 32, fieldType_ver15_e::regular },
{ PSF(origin[0]), 0, fieldType_ver15_e::coordExtra },
{ PSF(origin[1]), 0, fieldType_ver15_e::coordExtra },
{ PSF(viewangles[1]), 0, fieldType_ver15_e::regular },
{ PSF(velocity[1]), 0, fieldType_ver15_e::velocity },
{ PSF(velocity[0]), 0, fieldType_ver15_e::velocity },
{ PSF(viewangles[0]), 0, fieldType_ver15_e::regular },
{ PSF(origin[2]), 0, fieldType_ver15_e::coordExtra },
{ PSF(velocity[2]), 0, fieldType_ver15_e::velocity },
{ PSF(iViewModelAnimChanged), 2, fieldType_ver15_e::regular },
{ PSF(damage_angles[0]), -13, fieldType_ver15_e::angle },
{ PSF(damage_angles[1]), -13, fieldType_ver15_e::angle },
{ PSF(damage_angles[2]), -13, fieldType_ver15_e::angle },
{ PSF(speed), 16, fieldType_ver15_e::regular },
{ PSF(delta_angles[1]), 16, fieldType_ver15_e::regular },
{ PSF(viewheight), -8, fieldType_ver15_e::regular },
{ PSF(groundEntityNum), GENTITYNUM_BITS, fieldType_ver15_e::regular },
{ PSF(delta_angles[0]), 16, fieldType_ver15_e::regular },
{ PSF(iViewModelAnim), 4, fieldType_ver15_e::regular },
{ PSF(fov), 0, fieldType_ver15_e::regular },
{ PSF(current_music_mood), 8, fieldType_ver15_e::regular },
{ PSF(gravity), 16, fieldType_ver15_e::regular },
{ PSF(fallback_music_mood), 8, fieldType_ver15_e::regular },
{ PSF(music_volume), 0, fieldType_ver15_e::regular },
{ PSF(pm_flags), 16, fieldType_ver15_e::regular },
{ PSF(clientNum), 8, fieldType_ver15_e::regular },
{ PSF(fLeanAngle), 0, fieldType_ver15_e::regular },
{ PSF(blend[3]), 0, fieldType_ver15_e::regular },
{ PSF(blend[0]), 0, fieldType_ver15_e::regular },
{ PSF(pm_type), 8, fieldType_ver15_e::regular },
{ PSF(feetfalling), 8, fieldType_ver15_e::regular },
{ PSF(radarInfo), 26, fieldType_ver15_e::regular },
{ PSF(camera_angles[0]), 16, fieldType_ver15_e::angle },
{ PSF(camera_angles[1]), 16, fieldType_ver15_e::angle },
{ PSF(camera_angles[2]), 16, fieldType_ver15_e::angle },
{ PSF(camera_origin[0]), 0, fieldType_ver15_e::coordExtra },
{ PSF(camera_origin[1]), 0, fieldType_ver15_e::coordExtra },
{ PSF(camera_origin[2]), 0, fieldType_ver15_e::coordExtra },
{ PSF(camera_posofs[0]), 0, fieldType_ver15_e::coordExtra },
{ PSF(camera_posofs[2]), 0, fieldType_ver15_e::coordExtra },
{ PSF(camera_time), 0, fieldType_ver15_e::regular },
{ PSF(bVoted), 1, fieldType_ver15_e::regular },
{ PSF(bobCycle), 8, fieldType_ver15_e::regular },
{ PSF(delta_angles[2]), 16, fieldType_ver15_e::regular },
{ PSF(viewangles[2]), 0, fieldType_ver15_e::regular },
{ PSF(music_volume_fade_time), 0, fieldType_ver15_e::regular },
{ PSF(reverb_type), 6, fieldType_ver15_e::regular },
{ PSF(reverb_level), 0, fieldType_ver15_e::regular },
{ PSF(blend[1]), 0, fieldType_ver15_e::regular },
{ PSF(blend[2]), 0, fieldType_ver15_e::regular },
{ PSF(camera_offset[0]), 0, fieldType_ver15_e::regular },
{ PSF(camera_offset[1]), 0, fieldType_ver15_e::regular },
{ PSF(camera_offset[2]), 0, fieldType_ver15_e::regular },
{ PSF(camera_posofs[1]), 0, fieldType_ver15_e::coordExtra },
{ PSF(camera_flags), 16, fieldType_ver15_e::regular }
};

static_assert(sizeof(playerStateFields_ver15) == sizeof(netField_template_t<fieldType_ver15_e>) * 55);

static const playerState_t nullPS;

SerializablePlayerStateBase::SerializablePlayerStateBase(playerState_t& inState)
	: state(inState)
{}

SerializablePlayerState::SerializablePlayerState(playerState_t& inState)
	: SerializablePlayerStateBase(inState)
{}

void SerializablePlayerState::NormalizePlayerState(playerState_t * ps) const
{
	const uint32_t pmFlags = ps->pm_flags;
	uint32_t newPmFlags = 0;

	// Convert AA PlayerMove flags to SH/BT flags
	newPmFlags |= pmFlags & PMF_DUCKED;
	for (size_t i = 2; i < 32; ++i)
	{
		if (pmFlags & (1 << (i + 2))) {
			newPmFlags |= (1 << i);
		}
	}

	// So that flags are normalized across modules
	ps->pm_flags = newPmFlags;
}

void SerializablePlayerState::UnNormalizePlayerState(playerState_t * ps) const
{
	const uint32_t pmFlags = ps->pm_flags;
	uint32_t newPmFlags = 0;

	// Convert new flags flag to old AA pmflags
	newPmFlags |= pmFlags & PMF_DUCKED;
	for (size_t i = 2; i < 32; ++i)
	{
		if (pmFlags & (1 << i)) {
			newPmFlags |= (1 << (i + 2));
		}
	}

	// So that flags are normalized across modules
	ps->pm_flags = newPmFlags;
}

void SerializablePlayerState::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{
	MsgCoordHelper msgHelper(msg);

	constexpr size_t numFields = sizeof(playerStateFields) / sizeof(playerStateFields[0]);

	const playerState_t* fromPS = from ? ((const SerializablePlayerState*)from)->GetState() : &nullPS;
	const netField_t* field;
	bool deltaNeededList[numFields];

	uint8_t lc = 0;
	uint8_t i;

	// Calculate the number of changes
	for (i = 0, field = playerStateFields; i < numFields; i++, field++)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->getOffset());
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)&state + field->getOffset());

		const bool deltaNeeded = EntityField::DeltaNeeded(fromF, toF, field);
		deltaNeededList[i] = deltaNeeded;
		if (deltaNeeded) {
			lc = i + 1;
		}
	}

	const uint32_t pm_flags = GetState()->pm_flags;
	UnNormalizePlayerState(GetState());

	// Serialize the number of changes
	msg.WriteByte(lc);

	for (i = 0, field = playerStateFields; i < lc; ++i, ++field)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->getOffset());
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)GetState() + field->getOffset());

		const bool hasChange = deltaNeededList[i];

		msg.WriteBool(hasChange);
		if (!hasChange) {
			continue;
		}

		switch (fieldType_ver6_e(field->getType()))
		{
		case fieldType_ver6_e::regular:
			EntityField::WriteNumberPlayerStateField(msg, field->getBits(), toF, field->getSize());
			break;
		case fieldType_ver6_e::angle:
			EntityField::WriteAngleField(msg, field->getBits(), *(float*)toF);
			break;
		case fieldType_ver6_e::coord:
			msgHelper.WriteCoord(*(float*)toF);
			break;
		case fieldType_ver6_e::velocity:
			msgHelper.WriteCoordSmall(*(float*)toF);
			break;
		default:
			break;
		}
	}

	GetState()->pm_flags = pm_flags;

	uint32_t statsBits = 0;
	uint32_t activeItemsBits = 0;
	uint32_t ammoAmountBits = 0;
	uint32_t ammoBits = 0;
	uint32_t maxAmmoAmountBits = 0;

	//
	// Serialize arrays
	//
	for (i = 0; i < playerState_t::MAX_STATS; ++i)
	{
		if (state.stats[i] != fromPS->stats[i]) {
			statsBits |= 1 << i;
		}
	}

	for (i = 0; i < playerState_t::MAX_ACTIVEITEMS; ++i)
	{
		if (state.activeItems[i] != fromPS->activeItems[i]) {
			activeItemsBits |= 1 << i;
		}
	}

	for (i = 0; i < playerState_t::MAX_AMMO_AMOUNT; ++i)
	{
		if (state.ammo_amount[i] != fromPS->ammo_amount[i]) {
			ammoAmountBits |= 1 << i;
		}
	}

	for (i = 0; i < playerState_t::MAX_AMMO; ++i)
	{
		if (state.ammo_name_index[i] != fromPS->ammo_name_index[i]) {
			ammoBits |= 1 << i;
		}
	}

	for (i = 0; i < playerState_t::MAX_MAX_AMMO_AMOUNT; ++i)
	{
		if (state.max_ammo_amount[i] != fromPS->max_ammo_amount[i]) {
			maxAmmoAmountBits |= 1 << i;
		}
	}

	const bool hasStatsChanges = statsBits || activeItemsBits || ammoBits || ammoAmountBits || maxAmmoAmountBits;

	msg.WriteBool(hasStatsChanges);
	if (!hasStatsChanges) {
		return;
	}

	// Serialize stats
	bool hasStatsChanged = statsBits ? true : false;
	msg.WriteBool(hasStatsChanged);
	if (hasStatsChanged)
	{
		msg.WriteUInteger(statsBits);
		for (i = 0; i < playerState_t::MAX_STATS; ++i)
		{
			if (statsBits & (1 << i)) {
				msg.WriteUShort(state.stats[i]);
			}
		}
	}

	// Serialize active items
	bool hasActiveItemsChanged = activeItemsBits ? true : false;
	msg.WriteBool(hasActiveItemsChanged);
	if (hasActiveItemsChanged)
	{
		msg.WriteByte(activeItemsBits);
		for (i = 0; i < playerState_t::MAX_ACTIVEITEMS; ++i)
		{
			if (activeItemsBits & (1 << i)) {
				msg.WriteUShort(state.activeItems[i]);
			}
		}
	}

	// Serialize ammo amount
	bool hasAmmoAmountChanges = ammoAmountBits ? true : false;
	msg.WriteBool(hasAmmoAmountChanges);
	if (hasAmmoAmountChanges)
	{
		msg.WriteUShort(ammoAmountBits);
		for (i = 0; i < playerState_t::MAX_AMMO_AMOUNT; ++i)
		{
			if (ammoAmountBits & (1 << i)) {
				msg.WriteUShort(state.ammo_amount[i]);
			}
		}
	}

	// Serialize ammo
	bool hasAmmoBitsChanges = ammoBits ? true : false;
	msg.WriteBool(hasAmmoBitsChanges);
	if (hasAmmoBitsChanges)
	{
		msg.WriteUShort(ammoBits);
		for (i = 0; i < playerState_t::MAX_AMMO; ++i)
		{
			if (ammoBits & (1 << i)) {
				msg.WriteUShort(state.ammo_name_index[i]);
			}
		}
	}

	// Serialize max ammo
	bool hasMaxAmmoAmountChanges = maxAmmoAmountBits ? true : false;
	msg.WriteBool(hasMaxAmmoAmountChanges);
	if (hasMaxAmmoAmountChanges)
	{
		msg.WriteUShort(maxAmmoAmountBits);
		for (i = 0; i < playerState_t::MAX_MAX_AMMO_AMOUNT; ++i)
		{
			if (maxAmmoAmountBits & (1 << i)) {
				msg.WriteUShort(state.max_ammo_amount[i]);
			}
		}
	}
}

void SerializablePlayerState::LoadDelta(MSG& msg, const ISerializableMessage* from)
{
	MsgCoordHelper msgHelper(msg);

	constexpr size_t numFields = sizeof(playerStateFields) / sizeof(playerStateFields[0]);

	const SerializablePlayerState* srFrom = (const SerializablePlayerState*)from;
	const playerState_t* fromPS = srFrom ? srFrom->GetState() : &nullPS;

	// Serialize the number of changes
	const uint8_t lc = msg.ReadByte();

	const uint32_t pm_flags = GetState()->pm_flags;

	size_t i;
	const netField_t* field;
	for (i = 0, field = playerStateFields; i < lc; ++i, ++field)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->getOffset());
		uint8_t* toF = (uint8_t*)((uint8_t*)GetState() + field->getOffset());
		bool hasChange;

		hasChange = msg.ReadBool();
		if (!hasChange)
		{
			// no change
			std::memcpy(toF, fromF, field->getSize());
			continue;
		}

		switch (fieldType_ver6_e(field->getType()))
		{
		case fieldType_ver6_e::regular:
			EntityField::ReadNumberPlayerStateField(msg, field->getBits(), toF, field->getSize());
			break;
		case fieldType_ver6_e::angle:
			*(float*)toF = EntityField::ReadAngleField(msg, field->getBits());
			break;
		case fieldType_ver6_e::coord:
			*(float*)toF = msgHelper.ReadCoord();
			break;
		case fieldType_ver6_e::velocity:
			*(float*)toF = msgHelper.ReadCoordSmall();
			break;
		default:
			break;
		}
	}

	if (GetState()->pm_flags != pm_flags)
	{
		// got a new flags so normalize it correctly
		NormalizePlayerState(GetState());
	}

	if (fromPS)
	{
		// assign unchanged fields accordingly
		for (i = lc, field = &playerStateFields[lc]; i < numFields; i++, field++)
		{
			const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->getOffset());
			uint8_t* toF = (uint8_t*)((uint8_t*)GetState() + field->getOffset());

			// no change
			std::memcpy(toF, fromF, field->getSize());
		}
	}

	uint32_t statsBits = 0;
	uint32_t activeItemsBits = 0;
	uint32_t ammoAmountBits = 0;
	uint32_t ammoBits = 0;
	uint32_t maxAmmoAmountBits = 0;

	const bool hasStatsChanges = msg.ReadBool();
	if (!hasStatsChanges) {
		return;
	}

	// Serialize stats
	const bool hasStatsChanged = msg.ReadBool();
	if (hasStatsChanged)
	{
		statsBits = msg.ReadUInteger();
		for (i = 0; i < playerState_t::MAX_STATS; ++i)
		{
			if (statsBits & (1 << i)) {
				state.stats[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize active items
	const bool hasActiveItemsChanged = msg.ReadBool();
	if (hasActiveItemsChanged)
	{
		activeItemsBits = msg.ReadByte();
		for (i = 0; i < playerState_t::MAX_ACTIVEITEMS; ++i)
		{
			if (activeItemsBits & (1 << i)) {
				state.activeItems[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize ammo amount
	const bool hasAmmoAmountChanges = msg.ReadBool();
	if (hasAmmoAmountChanges)
	{
		ammoAmountBits = msg.ReadUShort();
		for (i = 0; i < playerState_t::MAX_AMMO_AMOUNT; ++i)
		{
			if (ammoAmountBits & (1 << i)) {
				state.ammo_amount[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize ammo
	const bool hasAmmoBitsChanges = msg.ReadBool();
	if (hasAmmoBitsChanges)
	{
		ammoBits = msg.ReadUShort();
		for (i = 0; i < playerState_t::MAX_AMMO; ++i)
		{
			if (ammoBits & (1 << i)) {
				state.ammo_name_index[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize max ammo
	const bool hasMaxAmmoAmountChanges = msg.ReadBool();
	if (hasMaxAmmoAmountChanges)
	{
		maxAmmoAmountBits = msg.ReadUShort();
		for (i = 0; i < playerState_t::MAX_MAX_AMMO_AMOUNT; ++i)
		{
			if (maxAmmoAmountBits & (1 << i)) {
				state.max_ammo_amount[i] = msg.ReadUShort();
			}
		}
	}
}

SerializablePlayerState_ver15::SerializablePlayerState_ver15(playerState_t& inState)
	: SerializablePlayerStateBase(inState)
{}

void SerializablePlayerState_ver15::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{
	MsgCoordHelper msgHelper(msg);

	constexpr size_t numFields = sizeof(playerStateFields_ver15) / sizeof(playerStateFields_ver15[0]);

	const playerState_t* fromPS = from ? ((SerializablePlayerState*)from)->GetState() : &nullPS;
	const netField_t* field;
	bool deltaNeededList[numFields];

	uint8_t lc = 0;
	uint8_t i;

	// Calculate the number of changes
	for (i = 0, field = playerStateFields_ver15; i < numFields; i++, field++)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->getOffset());
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)&state + field->getOffset());

		const bool deltaNeeded = EntityField::DeltaNeeded(fromF, toF, field);
		deltaNeededList[i] = deltaNeeded;
		if (deltaNeeded) {
			lc = i + 1;
		}
	}

	// Serialize the number of changes
	msg.WriteByte(lc);

	for (i = 0, field = playerStateFields_ver15; i < lc; ++i, ++field)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->getOffset());
		const uint8_t* toF = (const uint8_t*)((const uint8_t*)GetState() + field->getOffset());

		const bool hasChange = deltaNeededList[i];

		msg.WriteBool(hasChange);
		if (!hasChange) {
			continue;
		}

		uint32_t packedValue;

		switch (fieldType_ver15_e(field->getType()))
		{
		case fieldType_ver15_e::regular:
			EntityField::WriteRegular2(msg, field->getBits(), toF, field->getSize());
			break;
		case fieldType_ver15_e::angle:
			packedValue = EntityField::PackAngle2(*(float*)toF, field->getBits());
			msg.WriteNumber<uint32_t>(packedValue, field->getBits());
			break;
		case fieldType_ver15_e::coord:
			msgHelper.WriteDeltaCoord(
				EntityField::PackCoord(*(float*)fromF),
				EntityField::PackCoord(*(float*)toF)
			);
			break;
		case fieldType_ver15_e::coordExtra:
			msgHelper.WriteDeltaCoordExtra(
				EntityField::PackCoordExtra(*(float*)fromF),
				EntityField::PackCoordExtra(*(float*)toF)
			);
			break;
		case fieldType_ver15_e::velocity:
			msgHelper.WriteCoordSmall(*(float*)toF);
			break;
		default:
			break;
		}
	}

	uint32_t statsBits = 0;
	uint32_t activeItemsBits = 0;
	uint32_t ammoAmountBits = 0;
	uint32_t ammoBits = 0;
	uint32_t maxAmmoAmountBits = 0;

	//
	// Serialize arrays
	//
	for (i = 0; i < playerState_t::MAX_STATS; ++i)
	{
		if (state.stats[i] != fromPS->stats[i]) {
			statsBits |= 1 << i;
		}
	}

	for (i = 0; i < playerState_t::MAX_ACTIVEITEMS; ++i)
	{
		if (state.activeItems[i] != fromPS->activeItems[i]) {
			activeItemsBits |= 1 << i;
		}
	}

	for (i = 0; i < playerState_t::MAX_AMMO_AMOUNT; ++i)
	{
		if (state.ammo_amount[i] != fromPS->ammo_amount[i]) {
			ammoAmountBits |= 1 << i;
		}
	}

	for (i = 0; i < playerState_t::MAX_AMMO; ++i)
	{
		if (state.ammo_name_index[i] != fromPS->ammo_name_index[i]) {
			ammoBits |= 1 << i;
		}
	}

	for (i = 0; i < playerState_t::MAX_MAX_AMMO_AMOUNT; ++i)
	{
		if (state.max_ammo_amount[i] != fromPS->max_ammo_amount[i]) {
			maxAmmoAmountBits |= 1 << i;
		}
	}

	const bool hasStatsChanges = statsBits || activeItemsBits || ammoBits || ammoAmountBits || maxAmmoAmountBits;

	msg.WriteBool(hasStatsChanges);
	if (!hasStatsChanges) {
		return;
	}

	// Serialize stats
	bool hasStatsChanged = statsBits ? true : false;
	msg.WriteBool(hasStatsChanged);
	if (hasStatsChanged)
	{
		msg.WriteUInteger(statsBits);
		for (i = 0; i < playerState_t::MAX_STATS; ++i)
		{
			if (statsBits & (1 << i)) {
				msg.WriteUShort(state.stats[i]);
			}
		}
	}

	// Serialize active items
	bool hasActiveItemsChanged = activeItemsBits ? true : false;
	msg.WriteBool(hasActiveItemsChanged);
	if (hasActiveItemsChanged)
	{
		msg.WriteByte(activeItemsBits);
		for (i = 0; i < playerState_t::MAX_ACTIVEITEMS; ++i)
		{
			if (activeItemsBits & (1 << i)) {
				msg.WriteUShort(state.activeItems[i]);
			}
		}
	}

	// Serialize ammo amount
	bool hasAmmoAmountChanges = ammoAmountBits ? true : false;
	msg.WriteBool(hasAmmoAmountChanges);
	if (hasAmmoAmountChanges)
	{
		msg.WriteUShort(ammoAmountBits);
		for (i = 0; i < playerState_t::MAX_AMMO_AMOUNT; ++i)
		{
			if (ammoAmountBits & (1 << i)) {
				msg.WriteUShort(state.ammo_amount[i]);
			}
		}
	}

	// Serialize ammo
	bool hasAmmoBitsChanges = ammoBits ? true : false;
	msg.WriteBool(hasAmmoBitsChanges);
	if (hasAmmoBitsChanges)
	{
		msg.WriteUShort(ammoBits);
		for (i = 0; i < playerState_t::MAX_AMMO; ++i)
		{
			if (ammoBits & (1 << i)) {
				msg.WriteUShort(state.ammo_name_index[i]);
			}
		}
	}

	// Serialize max ammo
	bool hasMaxAmmoAmountChanges = maxAmmoAmountBits ? true : false;
	msg.WriteBool(hasMaxAmmoAmountChanges);
	if (hasMaxAmmoAmountChanges)
	{
		msg.WriteUShort(maxAmmoAmountBits);
		for (i = 0; i < playerState_t::MAX_MAX_AMMO_AMOUNT; ++i)
		{
			if (maxAmmoAmountBits & (1 << i)) {
				msg.WriteUShort(state.max_ammo_amount[i]);
			}
		}
	}
}

void SerializablePlayerState_ver15::LoadDelta(MSG& msg, const ISerializableMessage* from)
{
	MsgCoordHelper msgHelper(msg);

	constexpr size_t numFields = sizeof(playerStateFields_ver15) / sizeof(playerStateFields_ver15[0]);

	const SerializablePlayerState* srFrom = (const SerializablePlayerState*)from;
	const playerState_t* fromPS = srFrom ? srFrom->GetState() : &nullPS;

	// Serialize the number of changes
	const uint8_t lc = msg.ReadByte();
	if (lc > numFields) {
		throw SerializableErrors::BadEntityFieldCountException(lc);
	}
	
	size_t i;
	const netField_t* field;
	for (i = 0, field = playerStateFields_ver15; i < lc; ++i, ++field)
	{
		const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->getOffset());
		uint8_t* toF = (uint8_t*)((uint8_t*)GetState() + field->getOffset());

		const bool hasChange = msg.ReadBool();
		if (!hasChange)
		{
			// no change
			std::memcpy(toF, fromF, field->getSize());
			continue;
		}

		uint32_t result;
		intptr_t bits = 0;
		int32_t coordOffset, coordVal;

		switch (fieldType_ver15_e(field->getType()))
		{
		case fieldType_ver15_e::regular:
			EntityField::ReadRegular2(msg, field->getBits(), toF, field->getSize());
			break;
		case fieldType_ver15_e::angle: // anglestmp = 1.0f;
			result = msg.ReadNumber<uint32_t>(field->getBits() < 0 ? -field->getBits() : field->getBits());
			*(float*)toF = EntityField::UnpackAngle2(result, field->getBits());
			break;
		case fieldType_ver15_e::coord:
			coordOffset = EntityField::PackCoord(*(float*)fromF);
			coordVal = msgHelper.ReadDeltaCoord(coordOffset);
			*(float*)toF = EntityField::UnpackCoord(coordVal);
			break;
		case fieldType_ver15_e::coordExtra:
			coordOffset = EntityField::PackCoordExtra(*(float*)fromF);
			coordVal = msgHelper.ReadDeltaCoordExtra(coordOffset);
			*(float*)toF = EntityField::UnpackCoordExtra(coordVal);
			break;
		case fieldType_ver15_e::velocity:
			*(float*)toF = msgHelper.ReadCoordSmall();
			break;
		default:
			break;
		}
	}

	if (fromPS)
	{
		// assign unchanged fields accordingly
		for (i = lc, field = &playerStateFields_ver15[lc]; i < numFields; i++, field++)
		{
			const uint8_t* fromF = (const uint8_t*)((const uint8_t*)fromPS + field->getOffset());
			uint8_t* toF = (uint8_t*)((uint8_t*)GetState() + field->getOffset());

			// no change
			std::memcpy(toF, fromF, field->getSize());
		}
	}

	uint32_t statsBits = 0;
	uint32_t activeItemsBits = 0;
	uint32_t ammoAmountBits = 0;
	uint32_t ammoBits = 0;
	uint32_t maxAmmoAmountBits = 0;

	const bool hasStatsChanges = msg.ReadBool();
	if (!hasStatsChanges) {
		return;
	}

	// Serialize stats
	const bool hasStatsChanged = msg.ReadBool();
	if (hasStatsChanged)
	{
		statsBits = msg.ReadUInteger();
		for (i = 0; i < playerState_t::MAX_STATS; ++i)
		{
			if (statsBits & (1 << i)) {
				state.stats[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize active items
	const bool hasActiveItemsChanged = msg.ReadBool();
	if (hasActiveItemsChanged)
	{
		activeItemsBits = msg.ReadByte();
		for (i = 0; i < playerState_t::MAX_ACTIVEITEMS; ++i)
		{
			if (activeItemsBits & (1 << i)) {
				state.activeItems[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize ammo amount
	const bool hasAmmoAmountChanges = msg.ReadBool();
	if (hasAmmoAmountChanges)
	{
		ammoAmountBits = msg.ReadUShort();
		for (i = 0; i < playerState_t::MAX_AMMO_AMOUNT; ++i)
		{
			if (ammoAmountBits & (1 << i)) {
				state.ammo_amount[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize ammo
	const bool hasAmmoBitsChanges = msg.ReadBool();
	if (hasAmmoBitsChanges)
	{
		ammoBits = msg.ReadUShort();
		for (i = 0; i < playerState_t::MAX_AMMO; ++i)
		{
			if (ammoBits & (1 << i)) {
				state.ammo_name_index[i] = msg.ReadUShort();
			}
		}
	}

	// Serialize max ammo
	const bool hasMaxAmmoAmountChanges = msg.ReadBool();
	if (hasMaxAmmoAmountChanges)
	{
		maxAmmoAmountBits = msg.ReadUShort();
		for (i = 0; i < playerState_t::MAX_MAX_AMMO_AMOUNT; ++i)
		{
			if (maxAmmoAmountBits & (1 << i)) {
				state.max_ammo_amount[i] = msg.ReadUShort();
			}
		}
	}
}
