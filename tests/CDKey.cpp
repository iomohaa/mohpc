#include <MOHPC/Utility/Misc/cdkey.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>

void copyKey(const char* start, size_t length, std::ostream& outStream, bool dash = true);

int main(int argc, const char* argv[])
{
	std::ios::sync_with_stdio(false);

	char CDKey[60];
	std::ostream& outStream = std::cout;

	size_t len = 0;

	for (int i = 0; i < 100000; i++)
	{
		{
			MOHPC::CDKey::ProductKeyGenerator generator(MOHPC::CDKey::productType_e::MOHAA);
			generator.generate(CDKey);
			//generator.generateFromBase("199165941647931721", CDKey);

			//memcpy(p, "AA: ", 4);
			outStream << "AA: ";
			copyKey(CDKey, 4, outStream);
			copyKey(CDKey + 4, 7, outStream);
			copyKey(CDKey + 11, 7, outStream);
			copyKey(CDKey + 18, 4, outStream, false);
			outStream << std::endl;

			MOHPC::CDKey::Verifier verifier(MOHPC::CDKey::productType_e::MOHAA);
			assert(verifier.verifyKey(CDKey));
		}

		{
			MOHPC::CDKey::ProductKeyExGenerator generator(MOHPC::CDKey::productTypeExtended_e::MOHAAB);
			generator.generate(CDKey);

			outStream << "BT: ";
			copyKey(CDKey, 4, outStream);
			copyKey(CDKey + 4, 4, outStream);
			copyKey(CDKey + 8, 4, outStream);
			copyKey(CDKey + 12, 4, outStream);
			copyKey(CDKey + 16, 4, outStream, false);
			outStream << std::endl;

			MOHPC::CDKey::VerifierEx verifier(MOHPC::CDKey::productTypeExtended_e::MOHAAB);
			assert(verifier.verifyKey(CDKey));
		}
	}
}

void copyKey(const char* start, size_t length, std::ostream& outStream, bool dash)
{
	std::copy(start, start + length, std::ostream_iterator<char>(outStream, ""));
	if (dash) outStream << "-";
}
