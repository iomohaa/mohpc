#include "UnitTest.h"
#include <stddef.h>
#include <iostream>
#include <cassert>

IUnitTest* IUnitTest::head = NULL;
IUnitTest* IUnitTest::last = NULL;

IUnitTest::IUnitTest()
{
	next = NULL;
	prev = last;

	if (!head) head = this;
	if (last) last->next = this;
	last = this;
}

IUnitTest::~IUnitTest()
{
	assert(!next || next->prev == this);
	assert(!prev || prev->next == this);
	assert(prev || head == this);
	assert(next || last == this);
	if (prev) prev->next = next;
	if (next) next->prev = prev;
	if (head == this) head = next;
	if (last == this) last = prev;
}

void IUnitTest::runAll(const MOHPC::AssetManagerPtr& AM)
{
	std::cout << "--- Running unit testing" << std::endl;

	unsigned int higher = 0;

	// Get the highest priority
	for (IUnitTest* test = head; test; test = test->next)
	{
		unsigned int pri = test->priority();
		if (pri > higher) {
			higher = pri;
		}
	}

	bool remaining;
	do
	{
		unsigned int nextPri = 0;
		remaining = false;

		// Check for next tests below higher priority
		for (IUnitTest* test = head; test; test = test->next)
		{
			unsigned int pri = test->priority();
			if (pri >= nextPri && pri < higher)
			{
				nextPri = pri;
				remaining = true;
			}
		}

		// Run tests with this priority
		for (IUnitTest* test = head; test; test = test->next)
		{
			if (test->priority() == higher)
			{
				std::cout << "=== Testing " << test->name() << std::endl;
				test->run(AM);
			}
		}

		// Decrement the highest priority to the highest below
		higher = nextPri;
	} while (remaining);

	std::cout << "--- End of unit testing" << std::endl;
}
