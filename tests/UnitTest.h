#pragma once

#include <MOHPC/Managers/AssetManager.h>

class IUnitTest
{
private:
	IUnitTest* prev;
	IUnitTest* next;
	static IUnitTest* head;
	static IUnitTest* last;

public:
	IUnitTest();
	virtual ~IUnitTest();

	/** Priority of the test, for faster testing. Higher = first */
	virtual unsigned int priority() { return 0; }

	/** Define the name of the unit test. */
	virtual const char* name() = 0;

	/** Function for running code after everything has initialized. */
	virtual void run(const MOHPC::AssetManagerPtr& AM) = 0;

	/** Run all unit tests. */
	static void runAll(const MOHPC::AssetManagerPtr& AM);
};

//#define DEFINE_UNIT_TEST(classname) \
//static classname instance = classname()

template<typename T>
class TAutoInst
{
private:
	class Instance
	{
	private:
		T* instance;

	public:
		Instance()
			: instance(new T())
		{
		}
		~Instance()
		{
			delete instance;
		}
	};

	static Instance instance;
};

template<typename T>
typename TAutoInst<T>::Instance TAutoInst<T>::instance;
