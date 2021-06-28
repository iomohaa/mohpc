#include <MOHPC/Network/Serializable/UserInput.h>
#include <MOHPC/Network/Types/UserInput.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>

using namespace MOHPC;
using namespace MOHPC::Network;

void SerializableUsercmd::LoadDelta(MSG& msg, const ISerializableMessage* from, intptr_t key)
{
	const usercmd_t* fromCmd = &((SerializableUsercmd*)from)->ucmd;

	const bool isByteTime = msg.ReadBool();
	if (isByteTime)
	{
		const uint8_t deltaTime = msg.ReadByte();
	}
	else {
		ucmd.serverTime = msg.ReadUInteger();
	}

	const bool hasChanges = msg.ReadBool();
	if (hasChanges)
	{
		key = (uint32_t)((uint32_t)key ^ ucmd.serverTime);
		ucmd.angles[0] = msg.ReadDeltaTypeKey(fromCmd->angles[0], key);
		ucmd.angles[1] = msg.ReadDeltaTypeKey(fromCmd->angles[1], key);
		ucmd.angles[2] = msg.ReadDeltaTypeKey(fromCmd->angles[2], key);
		ucmd.forwardmove = msg.ReadDeltaTypeKey(fromCmd->forwardmove, key);
		ucmd.rightmove = msg.ReadDeltaTypeKey(fromCmd->rightmove, key);
		ucmd.upmove = msg.ReadDeltaTypeKey(fromCmd->upmove, key);
		ucmd.buttons.flags = msg.ReadDeltaTypeKey(fromCmd->buttons.flags, key);
	}
}

void SerializableUsercmd::SaveDelta(MSG& msg, const ISerializableMessage* from, intptr_t key) const
{
	const usercmd_t* fromCmd = &((SerializableUsercmd*)from)->ucmd;

	const uint32_t deltaTime = ucmd.serverTime - fromCmd->serverTime;
	const bool isByteTime = deltaTime < 256;

	msg.WriteBool(isByteTime);
	if (isByteTime) {
		msg.WriteByte(deltaTime);
	}
	else {
		msg.WriteUInteger(ucmd.serverTime);
	}

	const bool hasChanges =
		fromCmd->angles[0] != ucmd.angles[0] ||
		fromCmd->angles[1] != ucmd.angles[1] ||
		fromCmd->angles[2] != ucmd.angles[2] ||
		fromCmd->forwardmove != ucmd.forwardmove ||
		fromCmd->rightmove != ucmd.rightmove ||
		fromCmd->upmove != ucmd.upmove ||
		fromCmd->buttons.flags != ucmd.buttons.flags;

	msg.WriteBool(hasChanges);
	if (hasChanges)
	{
		const uint32_t keyTime = (uint32_t)key ^ ucmd.serverTime;
		msg.WriteDeltaTypeKey(fromCmd->angles[0], ucmd.angles[0], keyTime, 16);
		msg.WriteDeltaTypeKey(fromCmd->angles[1], ucmd.angles[1], keyTime, 16);
		msg.WriteDeltaTypeKey(fromCmd->angles[2], ucmd.angles[2], keyTime, 16);
		msg.WriteDeltaTypeKey(fromCmd->forwardmove, ucmd.forwardmove, keyTime, 8);
		msg.WriteDeltaTypeKey(fromCmd->rightmove, ucmd.rightmove, keyTime, 8);
		msg.WriteDeltaTypeKey(fromCmd->upmove, ucmd.upmove, keyTime, 8);
		msg.WriteDeltaTypeKey(fromCmd->buttons.flags, ucmd.buttons.flags, keyTime, 16);
	}
}

void SerializableUserEyes::LoadDelta(MSG& msg, const ISerializableMessage* from)
{
	const usereyes_t& fromEye = ((SerializableUserEyes*)from)->eyesInfo;

	const bool hasChanges = msg.ReadBool();
	if (hasChanges)
	{
		eyesInfo.ofs[0] = msg.ReadDeltaType(fromEye.ofs[0]);
		eyesInfo.ofs[1] = msg.ReadDeltaType(fromEye.ofs[1]);
		eyesInfo.ofs[2] = msg.ReadDeltaType(fromEye.ofs[2]);
		eyesInfo.angles[0] = msg.ReadDeltaType(fromEye.angles[0]);
		eyesInfo.angles[1] = msg.ReadDeltaType(fromEye.angles[1]);
	}
}

void SerializableUserEyes::SaveDelta(MSG& msg, const ISerializableMessage* from) const
{
	const usereyes_t& fromEye = ((SerializableUserEyes*)from)->eyesInfo;

	const bool hasChanges =
		fromEye.angles[0] != eyesInfo.angles[0]
		|| fromEye.angles[1] != eyesInfo.angles[1]
		|| fromEye.ofs[0] != eyesInfo.ofs[0]
		|| fromEye.ofs[1] != eyesInfo.ofs[1]
		|| fromEye.ofs[2] != eyesInfo.ofs[2];

	msg.WriteBool(hasChanges);
	if (hasChanges)
	{
		msg.WriteDeltaType(fromEye.ofs[0], eyesInfo.ofs[0]);
		msg.WriteDeltaType(fromEye.ofs[1], eyesInfo.ofs[1]);
		msg.WriteDeltaType(fromEye.ofs[2], eyesInfo.ofs[2]);
		msg.WriteDeltaType(fromEye.angles[0], eyesInfo.angles[0]);
		msg.WriteDeltaType(fromEye.angles[1], eyesInfo.angles[1]);
	}
}

