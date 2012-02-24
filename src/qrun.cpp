#pragma warning(disable: 4786)

#include <qunit.h>
#include <map>
#include <string>


#ifdef X_OS_WIN32
bool load_module(
	const char* name, QUnit::QTests& tests)
{
	typedef QUnit::QModule& (*InstFuncType)(void);

	HMODULE h = LoadLibraryA(name);
	if (h == NULL)
	{
		printf("%s can't load.", name);
		return false;
	}

	const char* func_name = "qunit_module_inst";
	InstFuncType qunit_module_inst = 
		(InstFuncType)GetProcAddress(h, func_name);

	if (qunit_module_inst == NULL)
	{
		printf("it's not a qunit module.", name);
		return false;
	}

	tests.insert(
		tests.end(),
		qunit_module_inst().tests().begin(),
		qunit_module_inst().tests().end()
		);

	return true;
}
#endif

#ifdef X_OS_LINUX
bool load_module(
	const char* name, QUnit::QTests& tests)
{
	return false;
}
#endif



int main(int argc, char** argv)
{
	QUnit::QTests tests;

	if (argc == 1)
	{
		puts("usage: qrun module [test] [fixture]");
		return 0;
	}
	if (!load_module(argv[1], tests))
		return 1;

	QUnit::QCUIRunner runner(
		argc - 2, argv + 2,
		tests
		);

	return 0;
}

