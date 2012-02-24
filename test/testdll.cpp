#include <qunit.h>

qcase(testFoo)
{
	qassert_not(true);
}

struct BarCase
{
	int i;
	BarCase()
	{
		i = 0;
	}
	~BarCase()
	{
		i = 0;
	}

};

qtest(testFoo, BarCase)
{
	qassert(i != 0);
}

qtest(testFoo2, BarCase)
{
	qassert(true);
}