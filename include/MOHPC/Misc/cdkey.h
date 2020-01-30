#pragma once

#include "../Global.h"
#include <stdint.h>

namespace MOHPC
{
	namespace CDKey
	{
		enum GameProduct
		{
			GP_MOHAA = 0x6149,
			GP_MOHAAS = 0x3B2F,
			GP_MOHAAB = 0x1D387
		};

		MOHPC_EXPORTS void GenerateCDKey(char* out, int32_t gameProduct);
		MOHPC_EXPORTS void GenerateCDKey2(char* out, int32_t gameProduct);
		MOHPC_EXPORTS void GenerateCDKeyFromBase(char* out, int32_t gameProduct, const char* base);
		MOHPC_EXPORTS void GenerateCDKeyFromBase2(char* out, int32_t gameProduct, const char* base);
		MOHPC_EXPORTS bool VerifyCDKey(const char* key, int32_t gameProduct);
		MOHPC_EXPORTS bool VerifyCDKey2(const char* key, int32_t gameProduct);
	}
};
