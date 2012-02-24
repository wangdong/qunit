#ifndef __QUNIT_RUNNER_CUI_H__
#define __QUNIT_RUNNER_CUI_H__

#include <time.h>

#ifdef X_OS_WIN32
#include <windows.h>
#endif

// -------------------------------------------------------------------------
namespace QUnit {

	struct QCUIOptParser
	{
		std::string testcase;
		std::string test;

		QCUIOptParser(int argc, char** argv)
		{
			testcase = ".*";
			test = ".*";
			if (argc > 0)
				test = argv[0];
			if (argc > 1)
				testcase = argv[1];
		}
	};

	struct QCUIRunnerHost : QHostBase
	{
		CRegexpT<char> rx_testcase;
		CRegexpT<char> rx_test;

		QCUIRunnerHost(const char* testcase, const char* test)
		{
			rx_testcase.Compile(testcase, IGNORECASE);
			rx_test.Compile(test, IGNORECASE);
		}
		bool is_excluded(const QTest& test)
		{
			MatchResult mtest = rx_test.Match(test.name.c_str());
			MatchResult mtestcase = rx_testcase.Match(test.testcase.c_str());
			return !(mtest.IsMatched() && mtestcase.IsMatched());
		}

		void done(const QResult& result)
		{
			switch(result.type)
			{
			case QResult::pass:
				printf(".");
				break;
			case QResult::failure:
				printf("F");
				break;
			case QResult::error:
				printf("E");
				break;
			}
			QHostBase::done(result);
		}

	};


	class QCUIRunner
	{
		std::string m_testcase;
		std::string m_test;
		QTests m_tests;

	public:
		QCUIRunner(
			const char* test_filter = "", 
			const char* testcase_filter = "",
			const QTests& tests = QModule::inst().tests()
		)
		{
			m_testcase = testcase_filter;
			m_test = test_filter;
			m_tests = tests;
		}

		QCUIRunner(
			int argc, char** argv,
			const QTests& tests = QModule::inst().tests())
		{
			m_tests = tests;
			QCUIOptParser parser(argc - 1, argv + 1);
			m_test = parser.test;
			m_testcase = parser.testcase;
		}
		~QCUIRunner()
		{
			QCUIRunnerHost host(m_testcase.c_str(), m_test.c_str());
			
			puts("Started");
			clock_t start_clock = clock();

			run(&host, m_tests);

			printf("\nFinished in %f seconds.\n\n", (clock() - start_clock) / 1000.0 );

			int index = 1;
			int failure_count = 0;
			int error_count = 0;
			int assertion_count = 0;
			QResults::const_iterator i = host.results.begin();
			for (; i < host.results.end(); ++i)
			{
				std::string test_name;
				if (i->testcase == "QDefaultCase")
					test_name = i->name;
				else
				{
					std::ostringstream o;
					o << i->name;
					o << "(";
					o << i->testcase;
					o << ")";
					test_name = o.str();
				}

				switch(i->type)
				{
				case QResult::failure:
					printf("  %d) Failure:\n", index);
					printf("%s [%s:%d]:\n%s\n\n", test_name.c_str(),
						i->fail.file.c_str(), i->fail.line, i->fail.condition.c_str());
					++failure_count;
					++index;
					break;
				case QResult::error:
					printf("  %d) Error:\n", index);
					printf("%s(%s):\n%s\n\n", i->name.c_str(), i->testcase.c_str(), i->msg.c_str());
					++error_count;
					++index;
					break;
				}
				assertion_count += i->assertion_count;
			}
			

#ifdef X_OS_WIN32
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbiInfo);

			if (error_count + failure_count)
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xf|BACKGROUND_RED);
			else
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xf|BACKGROUND_GREEN);
#endif
			
			printf("%d tests, %d assertions, %d failures, %d errors\n", 
				host.results.size(), assertion_count, failure_count, error_count);

#ifdef X_OS_WIN32
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), csbiInfo.wAttributes);
#endif
			exit(error_count + failure_count);
		}

	};

}

// -------------------------------------------------------------------------
//	$Log: cui.h,v $
//	Revision 1.1.2.2  2008/01/29 05:58:03  wangdong
//	*** empty log message ***
//	

#endif /* __QUNIT_RUNNER_CUI_H__ */
