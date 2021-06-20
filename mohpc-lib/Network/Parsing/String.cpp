#include <MOHPC/Network/Parsing/String.h>
#include <MOHPC/Utility/Misc/MSG/MSG.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::Parsing;

/**
 * Map of char to bytes (scramble)
 */
constexpr uint8_t charByteMapping[256] =
{
	254, 120, 89, 13, 27, 73, 103, 78, 74, 102, 21, 117, 76, 86, 238, 96, 88, 62, 59, 60,
	40, 84, 52, 119, 251, 51, 75, 121, 192, 85, 44, 54, 114, 87, 25, 53, 35, 224, 67, 31,
	82, 41, 45, 99, 233, 112, 255, 11, 46, 115, 8, 32, 19, 100, 110, 95, 116, 48, 58, 107,
	70, 91, 104, 81, 118, 109, 36, 24, 17, 39, 43, 65, 49, 83, 56, 57, 33, 64, 80, 28,
	184, 160, 18, 105, 42, 20, 194, 38, 29, 26, 61, 50, 9, 90, 37, 128, 79, 2, 108, 34,
	4, 0, 47, 12, 101, 10, 92, 15, 5, 7, 22, 55, 23, 14, 3, 1, 66, 16, 63, 30,
	6, 97, 111, 248, 72, 197, 191, 122, 176, 245, 250, 68, 195, 77, 232, 106, 228, 93, 240, 98,
	208, 69, 164, 144, 186, 222, 94, 246, 148, 170, 244, 190, 205, 234, 252, 202, 230, 239, 174, 225,
	226, 209, 236, 216, 237, 151, 149, 231, 129, 188, 200, 172, 204, 154, 168, 71, 133, 217, 196, 223,
	134, 253, 173, 177, 219, 235, 214, 182, 132, 227, 183, 175, 137, 152, 158, 221, 243, 150, 210, 136,
	167, 211, 179, 193, 218, 124, 140, 178, 213, 249, 185, 113, 127, 220, 180, 145, 138, 198, 123, 162,
	189, 203, 166, 126, 159, 156, 212, 207, 146, 181, 247, 139, 142, 169, 242, 241, 171, 187, 153, 135,
	201, 155, 161, 125, 163, 130, 229, 206, 165, 157, 141, 147, 143, 199, 215, 131
};

namespace MOHPC
{
	class byteCharMapping_c
	{
	private:
		char mapping[256];

	public:
		constexpr byteCharMapping_c()
			: mapping{ 0 }
		{
			for (uint16_t i = 0; i < 256; ++i)
			{
				for (uint16_t j = 0; j < 256; ++j)
				{
					if (charByteMapping[j] == i)
					{
						mapping[i] = (char)j;
						break;
					}
				}
			}
		}

		constexpr operator const char* () const { return mapping; }
	};
	static byteCharMapping_c byteCharMapping;
}

class String8 : public Parsing::IString
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 5;
		maxRange = 8;
	}

	StringMessage readString(MSG& msg) const override
	{
		return msg.ReadString();
	}

	void writeString(MSG& msg, const char* s) const override
	{
		msg.WriteString(s);
	}
};

class String17 : public String8
{
public:
	void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override
	{
		minRange = 15;
		maxRange = 17;
	}

	StringMessage readString(MSG& msg) const override
	{
		return msg.ReadScrambledString(byteCharMapping);
	}

	void writeString(MSG& msg, const char* s) const override
	{
		msg.WriteScrambledString(s, charByteMapping);
	}
};

using StringDefault = ProtocolSingletonInherit<String17, 0>;

static String8 stringParser8;
static String17 stringParser17;
static StringDefault stringParserDefault;

