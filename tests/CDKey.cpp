#include <MOHPC/Misc/cdkey.h>
#include "UnitTest.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

class CCDKeyUnitTest : public IUnitTest, public TAutoInst<CCDKeyUnitTest>
{
public:
	virtual const char* name() override
	{
		return "CD-Key";
	}

	virtual void run(const MOHPC::AssetManagerPtr& AM) override
	{
		srand((unsigned int)time(NULL));

		char CDKey[60];
		FILE* file = fopen("keys.txt", "wb");

		char* buf = (char*)malloc(10000000 * 59);
		size_t len = 0;
		char* p = buf;

		for (int i = 0; i < 100000; i++)
		{
			//char Base[20];
			//sprintf(Base, "%0.18d", i);
			MOHPC::CDKey::GenerateCDKey(CDKey, MOHPC::CDKey::GP_MOHAA);
			//MOHPC::CDKey::VerifyCDKey(CDKey, MOHPC::CDKey::GP_MOHAA);

			//memcpy(p, "AA: ", 4);
			*(int*)p = *(int*)"AA: ";
			p += 4;
			*(int*)p = *(int*)&CDKey[0];
			//memcpy(p, &CDKey[0], 4);
			p += 4;
			*p++ = '-';
			memcpy(p, &CDKey[4], 7);
			p += 7;
			*p++ = '-';
			memcpy(p, &CDKey[11], 7);
			p += 7;
			*p++ = '-';
			*(int*)p = *(int*)&CDKey[18];
			//memcpy(p, &CDKey[18], 4);
			p += 4;
			*p++ = '\n';
			len += 30;

			//sprintf(Base, "%0.13d", i);
			MOHPC::CDKey::GenerateCDKey2(CDKey, MOHPC::CDKey::GP_MOHAAB);

			//memcpy(p, "BT: ", 4);
			*(int*)p = *(int*)"BT: ";
			p += 4;
			*(int*)p = *(int*)&CDKey[0];
			//memcpy(p, &CDKey[0], 4);
			p += 4;
			*p++ = '-';
			*(int*)p = *(int*)&CDKey[4];
			//memcpy(p, &CDKey[4], 4);
			p += 4;
			*p++ = '-';
			*(int*)p = *(int*)&CDKey[8];
			//memcpy(p, &CDKey[8], 4);
			p += 4;
			*p++ = '-';
			*(int*)p = *(int*)&CDKey[12];
			//memcpy(p, &CDKey[12], 4);
			p += 4;
			*p++ = '-';
			*(int*)p = *(int*)&CDKey[16];
			//memcpy(p, &CDKey[16], 4);
			p += 4;
			*p++ = '\n';
			len += 29;
		}

		fwrite(buf, len, 1, file);
		fclose(file);
		free(buf);
	}
};