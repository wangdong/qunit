#include <qunit.h>
#include <assert.h>


class FooCase
{	
};

qtest(testSucc, FooCase)
{
	qassert_equal(2, 2);
}

qtest(testSucc2, FooCase)
{
	qassert_equal(2, 2);
	throw -1;
}

qcase(testBar)
{
	int i = 3;
	qassert_equal(4, i);
}

qtest(testSucc3, FooCase)
{
	qassert_equal(2, 2);
	throw "นามหฃก";
}

qtest(testSucc4, FooCase)
{
	qassert_equal(2, 2);
	qassert(1);
}

qtest(testMatch, FooCase)
{
	qassert_match("AB+", "ABB");
}

qtest(testMatch2, FooCase)
{
	qassert_not_match("AB+", "ABB");
}


qcase(testToS)
{
	using namespace QUnit::PrivateHelper;
	qassert_equal("2", to_s(3));
}

qcase(testEqual)
{
	using namespace QUnit::PrivateHelper;
	qassert(equal(1, 1));
	qassert(equal("A", std::string("A")));
	qassert(equal(std::string("A"), "A"));
	qassert(equal("ABC", "ABC"));
	qassert(equal(L"ABC", L"ABC"));
}


void testRunAll()
{
;
}


void testRegex() 
{
	CRegexpT<char> regexp("test");
	MatchResult result = regexp.Match("testBar");

	assert(result.IsMatched());
}


int main(int argc, char** argv)
{
	testRegex();
	QUnit::QCUIRunner runner(argc, argv);
	return 0;
}
