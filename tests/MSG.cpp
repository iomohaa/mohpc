#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Misc/MSG/Serializable.h>
#include <MOHPC/Network/InfoTypes.h>
#include <MOHPC/Network/SerializableTypes.h>
#include <MOHPC/Misc/Endian.h>
#include "UnitTest.h"
#include <vector>
#include <cassert>
#include <cstdlib>
#include <ctime>

class CMSGUnitTest : public IUnitTest
{
public:
	virtual unsigned int priority()
	{
		return 2;
	}

	virtual const char* name() override
	{
		return "MSG";
	}

	virtual void run(const MOHPC::AssetManagerPtr& AM) override
	{
		TestMSG();
		TestCompression();
		TestPlayerState();
		TestEntityState();
	}

	void TestMSG()
	{
		using namespace MOHPC;

		uint8_t msgBuffer[65536];
		uint8_t msgBuffer2[16];

		// Writing data
		{
			MOHPC::FixedDataMessageStream stream1(msgBuffer, sizeof(msgBuffer));
			MOHPC::MSG msg1(stream1, MOHPC::msgMode_e::Writing);

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
					msg1.WriteNumber(1ull << (j - 1), j);
				}

				const MOHPC::SerializableAngle16 serializableAngle(180.f);
				msg1.WriteClass(serializableAngle);
				msg1.WriteDeltaType(deltaVal1, deltaVal2);
			}

			MOHPC::FixedDataMessageStream stream1bit(msgBuffer2, sizeof(msgBuffer2));
			MOHPC::MSG msg1bit(stream1bit, MOHPC::msgMode_e::Writing);
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

			MOHPC::StringMessage testString;

			MOHPC::FixedDataMessageStream stream2(msgBuffer, sizeof(msgBuffer));
			MOHPC::MSG msg2(stream2, MOHPC::msgMode_e::Reading);

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

				MOHPC::SerializableAngle16 ang;
				msg2.ReadClass(ang); assert(ang == 180.f);
				deltaVal2 = msg2.ReadDeltaType(deltaVal1);
			}

			MOHPC::FixedDataMessageStream stream2bit(msgBuffer2, sizeof(msgBuffer2));
			MOHPC::MSG msg2bit(stream2bit, MOHPC::msgMode_e::Reading);
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
			MOHPC::FixedDataMessageStream instream(rawBuffer, sizeof(rawBuffer));
			MOHPC::FixedDataMessageStream outstream(compressedBuffer, sizeof(compressedBuffer));

			instream.Write("Hello, world", 12);

			MOHPC::CompressedMessage compressed(instream, outstream);
			compressed.Compress(0, 13);

			compressedLen = outstream.GetPosition();
		}

		{
			MOHPC::FixedDataMessageStream instream(compressedBuffer, sizeof(compressedBuffer), compressedLen);
			MOHPC::FixedDataMessageStream outstream(decompressedBuffer, sizeof(decompressedBuffer));

			MOHPC::CompressedMessage decompressed(instream, outstream);
			decompressed.Decompress(0, instream.GetLength());

			assert(!memcmp(decompressedBuffer, rawBuffer, sizeof(decompressedBuffer)));
		}
	}

	void TestPlayerState()
	{
		MOHPC::playerState_t ps1, ps2;
		uint8_t msgBuffer[32768];

		// Writing
		{
			MOHPC::FixedDataMessageStream streamWriter(msgBuffer, sizeof(msgBuffer));
			MOHPC::MSG writer(streamWriter, MOHPC::msgMode_e::Writing);

			srand(1000);

			ps2.origin[0] = (float)((rand() % 16384) - 8192);
			ps2.origin[1] = (float)((rand() % 16384) - 8192);
			ps2.origin[2] = (float)((rand() % 16384) - 8192);
			ps2.gravity = rand();
			ps2.speed = rand();
			ps2.groundEntityNum = rand() & (MOHPC::MAX_GENTITIES - 1);
			ps2.pm_time = short(rand());
			ps2.viewheight = uint8_t(rand());
			ps2.feetfalling = uint8_t(rand());
			ps2.delta_angles[0] = short(rand());
			ps2.delta_angles[1] = short(rand());
			ps2.delta_angles[2] = short(rand());
			ps2.damage_angles[0] = (float(rand() % 65535) / 65535.f) * 360.f;
			ps2.damage_angles[1] = (float(rand() % 65535) / 65535.f) * 360.f;
			ps2.damage_angles[2] = (float(rand() % 65535) / 65535.f) * 360.f;
			ps2.fLeanAngle = (float(rand() % 65535) / 65535.f) * 32.f - 16.f;
			ps2.camera_flags = uint16_t(rand());

			for (size_t i = 0; i < sizeof(ps2.activeItems) / sizeof(ps2.activeItems[0]); ++i) {
				ps2.activeItems[i] = short(rand());
			}

			MOHPC::SerializablePlayerState sps1(ps1);
			MOHPC::SerializablePlayerState sps2(ps2);
			writer.WriteDeltaClass(&sps1, &sps2);
		}

		// Reading
		{
			MOHPC::FixedDataMessageStream streamReader(msgBuffer, sizeof(msgBuffer));
			MOHPC::MSG reader(streamReader, MOHPC::msgMode_e::Reading);

			MOHPC::playerState_t serializedPS;

			MOHPC::SerializablePlayerState sps1(ps1);
			MOHPC::SerializablePlayerState sps2(serializedPS);
			reader.ReadDeltaClass(&sps1, &sps2);

			assert(serializedPS.origin == ps2.origin);
			assert(serializedPS.pm_time == ps2.pm_time);
			assert(serializedPS.gravity == ps2.gravity);
			assert(serializedPS.speed == ps2.speed);
			assert(serializedPS.feetfalling == ps2.feetfalling);
			assert(serializedPS.delta_angles[0] == ps2.delta_angles[0]
				&& serializedPS.delta_angles[1] == ps2.delta_angles[1]
				&& serializedPS.delta_angles[2] == ps2.delta_angles[2]
			);
			assert(serializedPS.fLeanAngle == (int)ps2.fLeanAngle);
			assert(serializedPS.camera_flags == ps2.camera_flags);

			for (size_t i = 0; i < sizeof(ps2.activeItems) / sizeof(ps2.activeItems[0]); ++i) {
				assert(serializedPS.activeItems[i] == ps2.activeItems[i]);
			}
		}
	}

	void TestEntityState()
	{
		MOHPC::entityState_t en1, en2;
		uint8_t msgBuffer[32768];

		// Writing
		{
			MOHPC::FixedDataMessageStream streamWriter(msgBuffer, sizeof(msgBuffer));
			MOHPC::MSG writer(streamWriter, MOHPC::msgMode_e::Writing);

			srand(2000);

			en2.alpha = float(rand() % 65536) / 65536.f;
			en2.netorigin[0] = (float)((rand() % 16384) - 8192);
			en2.netorigin[1] = (float)((rand() % 16384) - 8192);
			en2.netorigin[2] = (float)((rand() % 16384) - 8192);
			en2.netangles[0] = (float)(((rand() % 65536) - 32768) / 65536.f) * 180.f;
			en2.netangles[1] = (float)(((rand() % 65536) - 32768) / 65536.f) * 180.f;
			en2.netangles[2] = (float)(((rand() % 65536) - 32768) / 65536.f) * 180.f;
			en2.bone_angles[4][0] = (float)(((rand() % 65536) - 32768) / 65536.f) * 180.f;
			en2.bone_angles[4][1] = (float)(((rand() % 65536) - 32768) / 65536.f) * 180.f;
			en2.bone_angles[4][2] = (float)(((rand() % 65536) - 32768) / 65536.f) * 180.f;
			en2.wasframe = uint8_t(rand());
			en2.clientNum = uint8_t(rand());
			en2.groundEntityNum = rand() % (MOHPC::MAX_GENTITIES - 1);
			en2.surfaces[31] = uint8_t(rand());

			for (size_t i = 0; i < MOHPC::entityState_t::MAX_FRAMEINFOS; ++i)
			{
				en2.frameInfo[i].index = uint8_t(rand());
				en2.frameInfo[i].time = float(rand() % 65535) / 16387.f;
				en2.frameInfo[i].weight = float(rand() % 16388) / 16387.f;
			}

			MOHPC::SerializableEntityState sen1(en1, 0);
			MOHPC::SerializableEntityState sen2(en2, 0);
			writer.WriteDeltaClass(&sen1, &sen2);
		}

		// Reading
		{
			MOHPC::FixedDataMessageStream streamReader(msgBuffer, sizeof(msgBuffer));
			MOHPC::MSG reader(streamReader, MOHPC::msgMode_e::Reading);

			MOHPC::entityState_t sEnt;
			MOHPC::SerializableEntityState sen1(en1, 0);
			MOHPC::SerializableEntityState sen2(sEnt, 0);
			reader.ReadDeltaClass(&sen1, &sen2);

			assert(sEnt.alpha >= en2.alpha - 0.01f && sEnt.alpha <= en2.alpha + 0.01f);
			assert(sEnt.netorigin == en2.netorigin);
			assert(sEnt.wasframe == en2.wasframe);
			assert(sEnt.clientNum == en2.clientNum);
			assert(sEnt.groundEntityNum == en2.groundEntityNum);
			assert(sEnt.surfaces[31] == en2.surfaces[31]);

			for (size_t i = 0; i < MOHPC::entityState_t::MAX_FRAMEINFOS; ++i)
			{
				assert(sEnt.frameInfo[i].index == en2.frameInfo[i].index);
				assert((int)sEnt.frameInfo[i].time == (int)en2.frameInfo[i].time);
				assert((int)sEnt.frameInfo[i].weight == (int)en2.frameInfo[i].weight);
			}
		}
	}
};
static CMSGUnitTest unitTest;
