#include <MOHPC/Misc/MSG/MSG.h>
#include <MOHPC/Misc/MSG/Codec.h>
#include <MOHPC/Misc/MSG/Stream.h>
#include <MOHPC/Misc/MSG/Serializable.h>
#include <MOHPC/Network/InfoTypes.h>
#include <MOHPC/Network/SerializableTypes.h>
#include "UnitTest.h"
#include <vector>
#include <cassert>
#include <stdlib.h>
#include <time.h>

class CMSGUnitTest : public IUnitTest, public TAutoInst<CMSGUnitTest>
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

	virtual void run(MOHPC::AssetManager& AM) override
	{
		std::vector<uint8_t> msgBuffer(65536, 0);
		std::vector<uint8_t> msgBuffer2(16, 0);

		// Writing data
		MOHPC::FixedDataMessageStream stream1(msgBuffer.data(), msgBuffer.size());
		{
			MOHPC::MSG msg1(stream1, MOHPC::msgMode_e::Writing);

			uint16_t shortVal = 3;
			uint32_t intVal = 4;
			float floatVal = 5.f;
			uint32_t deltaVal1 = 6;
			uint32_t deltaVal2 = 7;

			for (int i = 0; i < 100; i++)
			{
				int32_t bitVal = 1;
				uint8_t byteVal = 250;
				msg1.SerializeBits(&bitVal, 1);
				bitVal = 655;
				msg1.SerializeBits(&bitVal, 12);
				msg1.SerializeBits(&bitVal, 13);
				msg1.SerializeBits(&bitVal, 31);
				msg1.SerializeByte(byteVal);
				msg1.SerializeUShort(shortVal);
				msg1.SerializeUInteger(intVal);
				msg1.SerializeFloat(floatVal);

				MOHPC::StringMessage strval = "testString";
				msg1.SerializeString(strval);

				for (int j = 0; j < 256; ++j)
				{
					byteVal = j;
					msg1.SerializeByte(byteVal);
				}

				for (int j = 1; j <= 32; ++j)
				{
					uint32_t byteVal = 1 << (j - 1);
					msg1.SerializeBits(&byteVal, j);
				}

				for (int j = 1; j <= 64; ++j)
				{
					uint64_t byteVal = 1ull << (j - 1);
					msg1.SerializeBits(&byteVal, j);
				}

				msg1.SerializeClass(MOHPC::SerializableAngle16(180.f));
				msg1.SerializeDelta(&deltaVal1, &deltaVal2, 32);
			}

			MOHPC::FixedDataMessageStream stream1bit(msgBuffer2.data(), msgBuffer2.size());
			MOHPC::MSG msg1bit(stream1bit, MOHPC::msgMode_e::Writing);
			uint8_t sb = 1;
			msg1bit.SerializeBits(&sb, 1);
		}

		// Data compression and decompression
		/*
		{
			MOHPC::FixedDataMessageStream instream(msgBuffer.data(), stream1.GetPosition());

			std::vector<uint8_t> compressedBuffer(32768, 0);
			MOHPC::FixedDataMessageStream outstream(compressedBuffer.data(), compressedBuffer.size());

			MOHPC::CompressedMessage compressed(instream, outstream);
			compressed.Compress(0);

			MOHPC::CompressedMessage decompressed(outstream, instream);
			decompressed.Decompress(0);
		}
		*/

		// Reading data
		{
			uint8_t byteVal = 0;
			uint16_t shortVal = 0;
			uint32_t intVal = 0;
			float floatVal = 0;
			uint32_t deltaVal1 = 0;
			uint32_t deltaVal2 = 0;

			MOHPC::StringMessage testString;

			MOHPC::FixedDataMessageStream stream2(msgBuffer.data(), msgBuffer.size());
			MOHPC::MSG msg2(stream2, MOHPC::msgMode_e::Reading);
			//msg2.SetCodec(MOHPC::MessageCodecs::OOB);

			for (int i = 0; i < 100; i++)
			{
				int32_t bitVal = 0;
				msg2.SerializeBits(&bitVal, 1); assert(bitVal == 1);
				msg2.SerializeBits(&bitVal, 12); assert(bitVal == 655);
				msg2.SerializeBits(&bitVal, 13); assert(bitVal == 655);
				msg2.SerializeBits(&bitVal, 31); assert(bitVal == 655);
				msg2.SerializeByte(byteVal); assert(byteVal == 250);
				msg2.SerializeUShort(shortVal); assert(shortVal == 3);
				msg2.SerializeUInteger(intVal); assert(intVal == 4);
				msg2.SerializeFloat(floatVal); assert(floatVal == 5.f);
				msg2.SerializeString(testString); assert(!strcmp(testString, "testString"));

				for (int j = 0; j < 256; ++j)
				{
					msg2.SerializeByte(byteVal);
					assert(byteVal == j);
				}

				for (int j = 1; j <= 32; ++j)
				{
					uint32_t byteVal = 0;
					msg2.SerializeBits(&byteVal, j);
					assert(byteVal == (1 << (j - 1)));
				}

				for (int j = 1; j <= 64; ++j)
				{
					uint64_t byteVal = 0;
					msg2.SerializeBits(&byteVal, j);
					assert(byteVal == (1ull << (j - 1)));
				}

				MOHPC::SerializableAngle16 ang;
				msg2.SerializeClass(ang); assert(ang == 180.f);
				msg2.SerializeDelta(&deltaVal1, &deltaVal2, 32);
			}

			MOHPC::FixedDataMessageStream stream2bit(msgBuffer2.data(), msgBuffer2.size());
			MOHPC::MSG msg2bit(stream2bit, MOHPC::msgMode_e::Reading);
			uint8_t sb = 0;
			msg2bit.SerializeBits(&sb, 1); assert(sb == 1);
		}

		TestPlayerState();
		TestEntityState();
	}

	void TestPlayerState()
	{
		MOHPC::playerState_t ps1, ps2;
		std::vector<uint8_t> msgBuffer(32768, 0);

		// Writing
		{
			MOHPC::FixedDataMessageStream streamWriter(msgBuffer.data(), msgBuffer.size());
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

			writer.SerializeDeltaClass(&MOHPC::SerializablePlayerState(ps1), &MOHPC::SerializablePlayerState(ps2));
		}

		// Reading
		{
			MOHPC::FixedDataMessageStream streamReader(msgBuffer.data(), msgBuffer.size());
			MOHPC::MSG reader(streamReader, MOHPC::msgMode_e::Reading);

			MOHPC::playerState_t serializedPS;
			reader.SerializeDeltaClass(&MOHPC::SerializablePlayerState(ps1), &MOHPC::SerializablePlayerState(serializedPS));

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
		std::vector<uint8_t> msgBuffer(32768, 0);

		// Writing
		{
			MOHPC::FixedDataMessageStream streamWriter(msgBuffer.data(), msgBuffer.size());
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
				en2.frameInfo[i].time = float(rand() % 65536) / 16387.f;
				en2.frameInfo[i].weight = float(rand() % 65536) / 16387.f;
			}

			writer.SerializeDeltaClass(&MOHPC::SerializableEntityState(en1), &MOHPC::SerializableEntityState(en2));
		}

		// Reading
		{
			MOHPC::FixedDataMessageStream streamReader(msgBuffer.data(), msgBuffer.size());
			MOHPC::MSG reader(streamReader, MOHPC::msgMode_e::Reading);

			MOHPC::entityState_t sEnt;
			reader.SerializeDeltaClass(&MOHPC::SerializableEntityState(en1), &MOHPC::SerializableEntityState(sEnt));

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
