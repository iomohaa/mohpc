#pragma once

#include "../Exception.h"

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
class MSG;

namespace Network
{
	struct netField_t;

	namespace EntityField
	{
		void ReadNumberPlayerStateField(MSG& msg, intptr_t bits, void* toF, size_t size);
		void WriteNumberPlayerStateField(MSG& msg, intptr_t bits, const void* toF, size_t size);

		void ReadRegular(MSG& msg, intptr_t bits, void* toF, size_t size);
		void WriteNumberEntityField(MSG& msg, intptr_t bits, const void* toF, size_t size);
		void ReadRegular2(MSG& msg, intptr_t bits, void* toF, size_t size);
		void WriteRegular2(MSG& msg, intptr_t bits, const void* toF, size_t size);

		void ReadSimple(MSG& msg, intptr_t bits, void* toF, size_t size);
		void WriteSimple(MSG& msg, intptr_t bits, const void* toF, size_t size);

		float ReadAngleField(MSG& msg, size_t bits);
		void WriteAngleField(MSG& msg, size_t bits, float angle);

		float ReadTimeField(MSG& msg, size_t bits);
		void WriteTimeField(MSG& msg, float time);

		float ReadScaleField(MSG& msg, size_t bits);
		void WriteScaleField(MSG& msg, float time);

		uint32_t PackAngle(float angle, intptr_t bits);
		float UnpackAngle(int result, intptr_t bits, bool isNeg);
		uint32_t PackAngle2(float angle, intptr_t bits);
		float UnpackAngle2(uint32_t result, intptr_t bits);

		uint32_t PackAnimTime(float time, size_t bits);
		float UnpackAnimTime(int result);

		uint32_t PackAnimWeight(float weight, size_t bits);
		float UnpackAnimWeight(int result, intptr_t bits);

		uint32_t PackScale(float scale, size_t bits);
		float UnpackScale(int16_t result);

		uint32_t PackAlpha(float alpha, size_t bits);
		float UnpackAlpha(int result, intptr_t bits);

		int32_t PackCoord(float val);
		float UnpackCoord(int32_t val);

		int32_t PackCoordExtra(float val);
		float UnpackCoordExtra(int32_t val);

		bool DeltaNeeded(const void* fromField, const void* toField, const netField_t* field);
		bool DeltaNeeded_ver15(const void* fromField, const void* toField, const netField_t* field);
	}

	namespace SerializableErrors
	{
		class Base : public Network::NetworkException {};

		class BadEntityNumberException : public Base
		{
		public:
			BadEntityNumberException(const char* name, size_t inBadNumber);

			MOHPC_NET_EXPORTS const char* getName() const;
			MOHPC_NET_EXPORTS size_t getNumber() const;
			MOHPC_NET_EXPORTS str what() const override;

		private:
			const char* name;
			size_t badNumber;
		};

		class BadEntityFieldException : public Base
		{
		private:
			uint8_t fieldType;
			const char* fieldName;

		public:
			BadEntityFieldException(uint8_t inFieldType, const char* inFieldName);

			MOHPC_NET_EXPORTS uint8_t getFieldType() const;
			MOHPC_NET_EXPORTS const char* getFieldName() const;
			MOHPC_NET_EXPORTS str what() const override;
		};

		class BadEntityFieldCountException : public Base
		{
		private:
			uint8_t count;

		public:
			BadEntityFieldCountException(uint8_t inCount);

			MOHPC_NET_EXPORTS uint8_t getCount() const;
			MOHPC_NET_EXPORTS str what() const override;
		};
	}
}
}
