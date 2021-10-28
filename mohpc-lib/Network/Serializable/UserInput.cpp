#include <MOHPC/Network/Serializable/UserInput.h>
#include <MOHPC/Network/Types/UserInput.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>

using namespace MOHPC;
using namespace MOHPC::Network;

SerializableUsercmd::SerializableUsercmd(usercmd_t& inCmd)
	: ucmd(&inCmd)
	, time32(0)
{}

void SerializableUsercmd::LoadDelta(MSG& msg, const ISerializableMessage* from, intptr_t key)
{
	using namespace std::chrono;

	const usercmd_t* fromCmd = static_cast<const SerializableUsercmd*>(from)->ucmd;

	const bool isByteTime = msg.ReadBool();
	if (isByteTime)
	{
		const uint8_t deltaTime = msg.ReadByte();
		*ucmd = usercmd_t(fromCmd->getServerTime() + milliseconds(deltaTime));
	}
	else
	{
		const uint32_t serverTime = msg.ReadUInteger();
		*ucmd = usercmd_t(tickTime_t(milliseconds(serverTime)));
	}

	const bool hasChanges = msg.ReadBool();
	if (hasChanges)
	{
		key = (uint32_t)((uint32_t)key ^ duration_cast<milliseconds>(ucmd->getServerTime().time_since_epoch()).count());
		const UserMovementInput& mFromInput = fromCmd->getMovement();
		UserMovementInput& mInput = ucmd->getMovement();
		const UserActionInput& aFromInput = fromCmd->getAction();
		UserActionInput& aInput = ucmd->getAction();

		const netAngles_t& fromAngles = mFromInput.getAngles();

		netAngles_t angles;
		angles[0] = msg.ReadDeltaTypeKey(fromAngles[0], key);
		angles[1] = msg.ReadDeltaTypeKey(fromAngles[1], key);
		angles[2] = msg.ReadDeltaTypeKey(fromAngles[2], key);

		mInput.setAngles(angles);

		mInput.moveForward(msg.ReadDeltaTypeKey(mFromInput.getForwardValue(), key));
		mInput.moveRight(msg.ReadDeltaTypeKey(mFromInput.getRightValue(), key));
		mInput.moveUp(msg.ReadDeltaTypeKey(mFromInput.getUpValue(), key));
		aInput.setFlags(msg.ReadDeltaTypeKey(aFromInput.getFlags(), key));
	}
}

void SerializableUsercmd::SaveDelta(MSG& msg, const ISerializableMessage* from, intptr_t key)
{
	const SerializableUsercmd* fromCmdSer = static_cast<const SerializableUsercmd*>(from);
	const usercmd_t* fromCmd = fromCmdSer->ucmd;

	using namespace ticks;
	// use exact same values that are sent over the network (milliseconds)
	time32 = (uint32_t)duration_cast<milliseconds>(ucmd->getServerTime().time_since_epoch()).count();

	// use milliseconds to be able to send the correct value over the network
	const uint32_t deltaTime = time32 - fromCmdSer->time32;
	if (deltaTime < 256)
	{
		msg.WriteBool(true);
		msg.WriteByte(deltaTime);
	}
	else
	{
		msg.WriteBool(false);
		// write the 32-bit server time
		msg.WriteUInteger(time32);
	}

	const UserMovementInput& mFromInput = fromCmd->getMovement();
	UserMovementInput& mInput = ucmd->getMovement();
	const UserActionInput& aFromInput = fromCmd->getAction();
	UserActionInput& aInput = ucmd->getAction();

	const netAngles_t& fromAngles = mFromInput.getAngles();
	const netAngles_t& angles = mInput.getAngles();

	const bool hasChanges =
		fromAngles[0] != angles[0] ||
		fromAngles[1] != angles[1] ||
		fromAngles[2] != angles[2] ||
		mFromInput.getForwardValue() != mInput.getForwardValue() ||
		mFromInput.getRightValue() != mInput.getRightValue() ||
		mFromInput.getUpValue() != mInput.getUpValue() ||
		aFromInput.getFlags() != aInput.getFlags();

	msg.WriteBool(hasChanges);
	if (hasChanges)
	{
		const uint32_t keyTime = (uint32_t)key ^ time32;
		msg.WriteDeltaTypeKey(fromAngles[0], angles[0], keyTime, 16);
		msg.WriteDeltaTypeKey(fromAngles[1], angles[1], keyTime, 16);
		msg.WriteDeltaTypeKey(fromAngles[2], angles[2], keyTime, 16);
		msg.WriteDeltaTypeKey(mFromInput.getForwardValue(), mInput.getForwardValue(), keyTime, 8);
		msg.WriteDeltaTypeKey(mFromInput.getRightValue(), mInput.getRightValue(), keyTime, 8);
		msg.WriteDeltaTypeKey(mFromInput.getUpValue(), mInput.getUpValue(), keyTime, 8);
		msg.WriteDeltaTypeKey(aFromInput.getFlags(), aInput.getFlags(), keyTime, 16);
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

void SerializableUserEyes::SaveDelta(MSG& msg, const ISerializableMessage* from)
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

