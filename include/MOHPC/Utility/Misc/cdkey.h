#pragma once

#include "../UtilityGlobal.h"
#include <cstdint>

namespace MOHPC
{
	namespace CDKey
	{
		enum class productType_e : uint16_t
		{
			/** Medal of Honor: Allied Assault. */
			MOHAA = 0x6149,
			/** Medal of Honor: Allied Assault Spearhead. */
			MOHAAS = 0x3B2F
		};

		enum class productTypeExtended_e : uint32_t
		{
			/** Medal of Honor: Allied Assault Breakthrough. */
			MOHAAB = 0x1D387
		};

		/**
		 * Product key generator.
		 * This generates a product key consisting of digits.
		 */
		class MOHPC_UTILITY_EXPORTS ProductKeyGenerator
		{
		public:
			ProductKeyGenerator(productType_e productTypeValue);

			/**
			 * Generate a product key.
			 *
			 * @param out 23 bytes buffer.
			 */
			void generate(char* out);

			/**
			 * Generate a product key from a starting base.
			 *
			 * @param base containing 18 characters (digits only).
			 * @param out 23 bytes buffer.
			 */
			void generateFromBase(const char* base, char* out);

		private:
			productType_e productType;
		};

		/**
		 * Extended product key generator.
		 * This generates a product key consisting of capital letters and digits.
		 */
		class MOHPC_UTILITY_EXPORTS ProductKeyExGenerator
		{
		public:
			ProductKeyExGenerator(productTypeExtended_e productTypeValue);

			/**
			 * Generate a product key.
			 *
			 * @param out 20 bytes buffer.
			 */
			void generate(char* out);

			/**
			 * Generate a product key from a starting base.
			 *
			 * @param base containing 14 characters (capital letters and digits).
			 * @param out 20 bytes buffer.
			 */
			void generateFromBase(const char* base, char* out);

		private:
			productTypeExtended_e productType;
		};

		/**
		 * Class for product key verification.
		 */
		class MOHPC_UTILITY_EXPORTS Verifier
		{
		public:
			Verifier(productType_e productTypeValue);

			/**
			 * Verify an input product key.
			 *
			 * @param key Product key of 23 characters (digits).
			 */
			bool verifyKey(const char* key);

		private:
			productType_e productType;
		};


		/**
		 * Class for extended product key verification.
		 */
		class MOHPC_UTILITY_EXPORTS VerifierEx
		{
		public:
			VerifierEx(productTypeExtended_e productTypeValue);

			/**
			 * Verify an input product key.
			 *
			 * @param key Product key of 20 characters (capital letters and digits).
			 */
			bool verifyKey(const char* key);

		private:
			productTypeExtended_e productType;
		};
	}
};
