#include <MOHPC/Utility/Misc/MSG/MSG.h>
#include <MOHPC/Utility/Misc/MSG/Codec.h>
#include <MOHPC/Utility/Misc/MSG/Stream.h>
#include <MOHPC/Utility/Misc/MSG/Serializable.h>
#include <MOHPC/Utility/Misc/Endian.h>
#include <MOHPC/Network/Types/Entity.h>
#include <MOHPC/Network/Types/PlayerState.h>
#include <MOHPC/Network/Serializable/Entity.h>
#include <MOHPC/Network/Serializable/PlayerState.h>
#include <MOHPC/Network/Serializable/Angles.h>
#include "Common/Common.h"

#include <vector>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <random>

using namespace MOHPC;
using namespace Network;

void TestShift();
void TestMSG();
void TestCompression();
void TestPlayerState();
void TestEntityState();
void AssertEntity(const entityState_t& to, const entityState_t& from);
void AssertPlayerState(const playerState_t& to, const playerState_t& from);

bool approximation(const_vec3r_t a, const_vec3r_t b, float tolerance)
{
	return fabs(a[0] - b[0]) < tolerance
		&& fabs(a[1] - b[1]) < tolerance
		&& fabs(a[2] - b[2]) < tolerance;
}

int main(int argc, const char* argv[])
{
	TestShift();
	TestMSG();
	TestCompression();

	for (size_t i = 0; i < 5000; ++i)
	{
		TestPlayerState();
		TestEntityState();
	}

	return 0;
}

template<typename T>
void TestShift(uint64_t max)
{
	using namespace MOHPC;

	for (size_t i = 0; i < max; ++i)
	{
		const T value = (T)i;
		const T shifted = shiftValue(value);

		T tmp = value;
		shiftValue(&tmp, sizeof(tmp));
		assert(shifted == tmp);

		const T unshifted = unshiftValue(shifted);

		T tmp2 = shifted;
		unshiftValue(&tmp2, sizeof(tmp2));
		assert(unshifted == value);
		assert(tmp2 == value);
	}
}

void TestShift()
{
	TestShift<uint8_t>(1 << 7);
	TestShift<uint16_t>(1 << 15);
	TestShift<uint32_t>(1 << 20);
}

void TestMSG()
{
	using namespace MOHPC;

	uint8_t msgBuffer[65536];
	uint8_t msgBuffer2[16];

	// Writing data
	{
		FixedDataMessageStream stream1(msgBuffer, sizeof(msgBuffer));
		MSG msg1(stream1, msgMode_e::Writing);

		const uint8_t byteVal = 250;
		const uint16_t shortVal = 3;
		const uint32_t intVal = 4;
		const float floatVal = 5.f;
		const uint32_t deltaVal1 = 6;
		const uint32_t deltaVal2 = 7;

		for (int i = 0; i < 100; i++)
		{
			union {
				uint32_t bitVal;
				uint8_t bytes[4];
			};

			msg1.WriteNumber(1, 1);
			msg1.WriteNumber(655u, 12);
			msg1.WriteNumber(655u, 13);
			msg1.WriteNumber(655u, 31);
			msg1.WriteByte(byteVal);
			msg1.WriteUShort(shortVal);
			msg1.WriteUInteger(intVal);
			msg1.WriteFloat(floatVal);
			msg1.WriteString("testString");

			for (int j = 0; j < 256; ++j) {
				msg1.WriteByte(j);
			}

			for (int j = 1; j <= 32; ++j) {
				msg1.WriteNumber(1 << (j - 1), j);
			}

			for (int j = 1; j <= 64; ++j) {
				msg1.WriteNumber<uint64_t>(1ull << (j - 1), j);
			}

			const SerializableAngle16 serializableAngle(180.f);
			msg1.WriteClass(serializableAngle);
			msg1.WriteDeltaType(deltaVal1, deltaVal2);
		}

		FixedDataMessageStream stream1bit(msgBuffer2, sizeof(msgBuffer2));
		MSG msg1bit(stream1bit, msgMode_e::Writing);
		const uint8_t sb = 1;
		msg1bit.WriteBits(&sb, 1);
	}

	// Reading data
	{
		uint8_t byteVal = 0;
		uint16_t shortVal = 0;
		uint32_t intVal = 0;
		float floatVal = 0;
		uint32_t deltaVal1 = 6;
		uint32_t deltaVal2 = 0;

		StringMessage testString;

		FixedDataMessageStream stream2(msgBuffer, sizeof(msgBuffer));
		MSG msg2(stream2, msgMode_e::Reading);

		for (int i = 0; i < 100; i++)
		{
			union {
				uint32_t bitVal;
				uint8_t bytes[4];
			};
			bitVal = msg2.ReadNumber<uint32_t>(1); assert(bitVal == 1);
			bitVal = msg2.ReadNumber<uint32_t>(12); assert(bitVal == 655);
			bitVal = msg2.ReadNumber<uint32_t>(13); assert(bitVal == 655);
			bitVal = msg2.ReadNumber<uint32_t>(31); assert(bitVal == 655);
			byteVal = msg2.ReadByte(); assert(byteVal == 250);
			shortVal = msg2.ReadUShort(); assert(shortVal == 3);
			intVal = msg2.ReadUInteger(); assert(intVal == 4);
			floatVal = msg2.ReadFloat(); assert(floatVal == 5.f);
			testString = msg2.ReadString(); assert(!strcmp(testString, "testString"));

			for (int j = 0; j < 256; ++j)
			{
				byteVal = msg2.ReadByte();
				assert(byteVal == j);
			}

			for (int j = 1; j <= 32; ++j)
			{
				const uint32_t byteVal = msg2.ReadNumber<uint32_t>(j);
				assert(byteVal == (1 << (j - 1)));
			}

			for (int j = 1; j <= 64; ++j)
			{
				const uint64_t byteVal = msg2.ReadNumber<uint64_t>(j);
				assert(byteVal == (1ull << (j - 1)));
			}

			SerializableAngle16 ang;
			msg2.ReadClass(ang); assert(ang == 180.f);
			deltaVal2 = msg2.ReadDeltaType(deltaVal1);
		}

		FixedDataMessageStream stream2bit(msgBuffer2, sizeof(msgBuffer2));
		MSG msg2bit(stream2bit, msgMode_e::Reading);
		uint8_t sb = 0;
		msg2bit.ReadBits(&sb, 1); assert(sb == 1);
	}
}

void TestCompression()
{
	// Data compression and decompression
	uint8_t rawBuffer[32768]{ 0 };
	uint8_t compressedBuffer[32768]{ 0 };
	uint8_t decompressedBuffer[32768]{ 0 };

	size_t compressedLen = 0;
	{
		FixedDataMessageStream instream(rawBuffer, sizeof(rawBuffer));
		FixedDataMessageStream outstream(compressedBuffer, sizeof(compressedBuffer));

		instream.Write("Hello, world", 12);

		CompressedMessage compressed(instream, outstream);
		compressed.Compress(0, 13);

		compressedLen = outstream.GetPosition();
	}

	{
		FixedDataMessageStream instream(compressedBuffer, sizeof(compressedBuffer), compressedLen);
		FixedDataMessageStream outstream(decompressedBuffer, sizeof(decompressedBuffer));

		CompressedMessage decompressed(instream, outstream);
		decompressed.Decompress(0, instream.GetLength());

		assert(!memcmp(decompressedBuffer, rawBuffer, sizeof(decompressedBuffer)));
	}
}

void TestPlayerState()
{
	playerState_t ps1, ps2;
	uint8_t msgBuffer[32768];

	// Writing
	{
		using namespace std::chrono;

		FixedDataMessageStream streamWriter(msgBuffer, sizeof(msgBuffer));
		MSG writer(streamWriter, msgMode_e::Writing);


		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution originDistrib(-8192.f, 8192.f);
		std::uniform_real_distribution angleDistrib(0.f, 360.f);
		std::uniform_real_distribution alphaDistrib(-1.f, 1.f);
		std::uniform_int_distribution<> entityNumDistrib(0, MAX_GENTITIES - 1);
		std::uniform_int_distribution<> miscDistrib(0, 255);

		ps2.origin[0] = originDistrib(gen);
		ps2.origin[1] = originDistrib(gen);
		ps2.origin[2] = originDistrib(gen);
		ps2.gravity = miscDistrib(gen);
		ps2.speed = miscDistrib(gen);
		ps2.groundEntityNum = entityNumDistrib(gen);
		ps2.commandTime = netTime_t(milliseconds(miscDistrib(gen)));
		ps2.viewheight = uint8_t(miscDistrib(gen));
		ps2.feetfalling = uint8_t(miscDistrib(gen));
		ps2.delta_angles[0] = short(miscDistrib(gen));
		ps2.delta_angles[1] = short(miscDistrib(gen));
		ps2.delta_angles[2] = short(miscDistrib(gen));
		ps2.damage_angles[0] = angleDistrib(gen);
		ps2.damage_angles[1] = angleDistrib(gen);
		ps2.damage_angles[2] = angleDistrib(gen);
		ps2.fLeanAngle = alphaDistrib(gen) * 16.f;
		ps2.camera_flags = uint16_t(miscDistrib(gen));

		for (size_t i = 0; i < sizeof(ps2.activeItems) / sizeof(ps2.activeItems[0]); ++i) {
			ps2.activeItems[i] = short(miscDistrib(gen));
		}

		{
			SerializablePlayerState sps1(ps1);
			SerializablePlayerState sps2(ps2);
			writer.WriteDeltaClass(&sps1, &sps2);
		}
		{
			SerializablePlayerState_ver15 sps1(ps1);
			SerializablePlayerState_ver15 sps2(ps2);
			writer.WriteDeltaClass(&sps1, &sps2);
		}
	}

	// Reading
	{
		FixedDataMessageStream streamReader(msgBuffer, sizeof(msgBuffer));
		MSG reader(streamReader, msgMode_e::Reading);

		playerState_t serializedPS;

		{
			SerializablePlayerState sps1(ps1);
			SerializablePlayerState sps2(serializedPS);
			reader.ReadDeltaClass(&sps1, &sps2);

			AssertPlayerState(serializedPS, ps2);
		}
		{
			SerializablePlayerState_ver15 sps1(ps1);
			SerializablePlayerState_ver15 sps2(serializedPS);
			reader.ReadDeltaClass(&sps1, &sps2);

			AssertPlayerState(serializedPS, ps2);
		}
	}
}

void TestEntityState()
{
	entityState_t en1, en2;
	uint8_t msgBuffer[32768];

	// Writing
	{
		FixedDataMessageStream streamWriter(msgBuffer, sizeof(msgBuffer));
		MSG writer(streamWriter, msgMode_e::Writing);

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution originDistrib(-8192.f, 8192.f);
		std::uniform_real_distribution anglesDistrib(0.f, 360.f);
		std::uniform_real_distribution alphaDistrib(0.f, 1.f);
		std::uniform_int_distribution<> entityNumDistrib(0, MAX_GENTITIES - 1);
		std::uniform_int_distribution<> miscDistrib(0, 255);

		en2.alpha = alphaDistrib(gen);
		en2.netorigin[0] = originDistrib(gen);
		en2.netorigin[1] = originDistrib(gen);
		en2.netorigin[2] = originDistrib(gen);
		en2.netangles[0] = anglesDistrib(gen);
		en2.netangles[1] = anglesDistrib(gen);
		en2.netangles[2] = anglesDistrib(gen);
		en2.bone_angles[4][0] = anglesDistrib(gen);
		en2.bone_angles[4][1] = anglesDistrib(gen);
		en2.bone_angles[4][2] = anglesDistrib(gen);
		en2.loopSoundVolume = alphaDistrib(gen);
		en2.loopSoundMinDist = (float)miscDistrib(gen);
		en2.wasframe = miscDistrib(gen);
		en2.clientNum = miscDistrib(gen);
		en2.groundEntityNum = entityNumDistrib(gen);
		en2.surfaces[31] = miscDistrib(gen);
		en2.pos.trDelta[0] = (float)miscDistrib(gen);
		en2.pos.trDelta[1] = (float)miscDistrib(gen);
		en2.pos.trDelta[2] = (float)miscDistrib(gen);

		for (size_t i = 0; i < entityState_t::MAX_FRAMEINFOS; ++i)
		{
			en2.frameInfo[i].index = uint8_t(rand());
			en2.frameInfo[i].time = float(rand() % 65535) / 16387.f;
			en2.frameInfo[i].weight = float(rand() % 16388) / 16387.f;
		}

		{
			SerializableEntityState sen1(en1, 0);
			SerializableEntityState sen2(en2, 0);
			writer.WriteDeltaClass(&sen1, &sen2);
		}
		{
			SerializableEntityState_ver15 sen1(en1, 0, deltaTimeFloat_t(0.f));
			SerializableEntityState_ver15 sen2(en2, 0, deltaTimeFloat_t(0.f));
			writer.WriteDeltaClass(&sen1, &sen2);
		}
	}

	// Reading
	{
		FixedDataMessageStream streamReader(msgBuffer, sizeof(msgBuffer));
		MSG reader(streamReader, msgMode_e::Reading);

		{
			entityState_t sEnt;
			SerializableEntityState sen1(en1, 0);
			SerializableEntityState sen2(sEnt, 0);
			reader.ReadDeltaClass(&sen1, &sen2);

			AssertEntity(sEnt, en2);
		}
		{
			entityState_t sEnt;
			SerializableEntityState_ver15 sen1(en1, 0, deltaTimeFloat_t(0.f));
			SerializableEntityState_ver15 sen2(sEnt, 0, deltaTimeFloat_t(0.f));
			reader.ReadDeltaClass(&sen1, &sen2);

			// FIXME: Implement serialization
			AssertEntity(sEnt, en2);
		}
	}
}

void AssertEntity(const entityState_t& to, const entityState_t& from)
{
	assert(to.alpha >= from.alpha - 0.01f && to.alpha <= from.alpha + 0.01f);
	assert(approximation(to.netorigin, from.netorigin, 0.25f));
	assert(approximation(to.netangles, from.netangles, 0.1f));
	assert(approximation(to.pos.trDelta, from.pos.trDelta, 0.1f));
	assert(to.loopSoundVolume == from.loopSoundVolume);
	assert(to.loopSoundMinDist == from.loopSoundMinDist);
	assert(to.wasframe == from.wasframe);
	assert(to.clientNum == from.clientNum);
	assert(to.groundEntityNum == from.groundEntityNum);
	assert(to.surfaces[31] == from.surfaces[31]);

	for (size_t i = 0; i < entityState_t::MAX_FRAMEINFOS; ++i)
	{
		const frameInfo_t& toFrameInfo = to.frameInfo[i];
		const frameInfo_t& fromFrameInfo = from.frameInfo[i];

		assert(toFrameInfo.index == fromFrameInfo.index);
		assert(fabsf(toFrameInfo.time - fromFrameInfo.time) <= 0.05);
		assert(fabsf(toFrameInfo.weight - fromFrameInfo.weight) <= 0.05);
	}
}

void AssertPlayerState(const playerState_t& to, const playerState_t& from)
{
	assert(approximation(to.origin, from.origin, 0.25f));
	assert(to.commandTime == from.commandTime);
	assert(to.gravity == from.gravity);
	assert(to.speed == from.speed);
	assert(to.feetfalling == from.feetfalling);
	assert(to.delta_angles[0] == from.delta_angles[0]
		&& to.delta_angles[1] == from.delta_angles[1]
		&& to.delta_angles[2] == from.delta_angles[2]
	);
	assert(int(to.fLeanAngle) == int(from.fLeanAngle));
	assert(to.camera_flags == from.camera_flags);

	for (size_t i = 0; i < sizeof(from.activeItems) / sizeof(from.activeItems[0]); ++i) {
		assert(to.activeItems[i] == from.activeItems[i]);
	}
}
