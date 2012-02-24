#include <qunit.h>

qcase(testFoo)
{
	qassert_not(true);
}

struct Bar2Case
{
	int i;
	Bar2Case()
	{
		i = 0;
	}
	~Bar2Case()
	{
		i = 0;
	}

};

qtest(testFoo2, Bar2Case)
{
	qassert(i != 0);
}

qtest(testFoo3, Bar2Case)
{
	qassert(true);
}