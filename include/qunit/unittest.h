#ifndef __QUNIT_H__
#define __QUNIT_H__

#pragma warning(disable: 4996)
// ----------------------------------------------------------------------------

#include <string>
#include <vector>
#include <map>

#include "private/platform.h"
#include "private/helper.h"
#include "private/deelx.h"

// ----------------------------------------------------------------------------
namespace QUnit {

	class QRun : public PrivateHelper::QRunCookie
	{
	public:
		virtual void run() = 0;
	};
	
	struct QDefaultCase
	{
	};
	
	struct QFailure
	{
		QFailure(const char* f = "", 
			int l = 0, const char* c = "")
		{
			file = f;
			line = l;
			condition = c;
		}

		std::string file;
		int line;
		std::string condition;
	};

	
	struct QTest
	{
		std::string testcase;
		std::string name;
		QRun* run;
	};
	typedef std::vector<QTest> QTests;

	struct QResult
	{
		QResult(const QUnit::QTest& test)
		{
			testcase = test.testcase;
			name = test.name;
			type = pass;
			assertion_count = 0;
		}

		std::string testcase;
		std::string name;
		int assertion_count;

		enum {pass, failure, error};
		int type;

		std::string msg;
		QFailure fail;

	};
	typedef std::vector<QResult> QResults;
	

	class QModule
	{
		QTests m_tests;
	public:
		void add_test(const char* testcase, const char* name, QRun* run)
		{
			QTest test;
			test.testcase = testcase;
			test.name = name;
			test.run = run;
			m_tests.push_back(test);
		}
		const QTests& tests() const
		{
			return m_tests;
		}

	public:
		static inline 
		QModule& inst();
	};

	inline
	QModule& QModule::inst()
	{
		static QModule module;
		return module;
	}

	struct QRunHost
	{
		virtual bool is_excluded(const QTest& test) = 0;
		virtual void done(const QResult& result) = 0;
	};

	struct QHostBase : QRunHost
	{
		QResults results;

		bool is_excluded(const QTest& test)
		{
			return false;
		}
		void done(const QResult& result)
		{
			results.push_back(result);
		}
	};

	inline 
	void run(QRunHost* host = NULL, 
		const QTests& tests = QModule::inst().tests()) throw()
	{
		using namespace QUnit;

		QHostBase defHost;
		if (host == NULL)
			host = &defHost;

		QTests::const_iterator i = tests.begin();
		for (;i != tests.end(); ++i)
		{
			if (strncmp("test", i->name.c_str(), 4) != 0)
				continue;
			if (host->is_excluded(*i))
				continue;

			QResult result(*i);
			i->run->__initialize_cookie();

			try
			{
				i->run->run();
			}
			catch (const QFailure& e)
			{
				result.type = QResult::failure;
				result.fail = e;
				result.msg = "";
			}
			catch (const std::exception& e)
			{
				result.type = QResult::error;
				result.msg = e.what();
			}
			catch (const char* e)
			{
				result.type = QResult::error;
				result.msg = e;
			}
			catch (const std::string& e)
			{
				result.type = QResult::error;
				result.msg = e;
			}
			catch (...)
			{
				result.type = QResult::error;
				result.msg = "Unknown exception";
			}

			result.assertion_count = i->run->__assertion_counter;
			host->done(result);
		}
	}


}

#include "runner/cui.h"


// ----------------------------------------------------------------------------
/*1*/#define qtest(test, testcase)											\
	class __qhelper_gen_name(test, testcase, test) : public testcase		\
	{																		\
	public:																	\
		void run(QUnit::QRun* __qunit_runner_inst);							\
		virtual ~__qhelper_gen_name(test, testcase, test)()					\
		{																	\
		}																	\
	};																		\
	static struct __qhelper_gen_name(test, testcase, runner) : QUnit::QRun	\
	{																		\
		__qhelper_gen_name(test, testcase, runner)()						\
		{																	\
			QUnit::QModule::inst().add_test(#testcase, #test, this);		\
		}																	\
		void run()															\
		{																	\
			__qhelper_gen_name(test, testcase, test) inst;					\
			inst.run(this);													\
		}																	\
	}  __qhelper_gen_name(test, testcase, runner_inst);						\
																			\
	inline void __qhelper_gen_name(test, testcase, test)::run(				\
		QUnit::QRun* __qunit_runner_inst)


// ----------------------------------------------------------------------------
using QUnit::QDefaultCase;
// 宏__qhelper_gen_name决定了在使用QDefaultCase时是不能带这个::符号的，
// 也就是说：__qhelper_gen_name(testFoo, QUnit::QDefaultCase)
// 会生成不合法的标识符。所以只好在此将之using了以支持qtest(test, QDefaultCase)。
//
// 虽然这里诡异，但是不得以而为之，若有更佳方案当然更好，
// 这么做起码会保护用户价值#6，而并不带来实际的害处。
//

/*2*/#define qcase(test) qtest(test, QDefaultCase)



// ----------------------------------------------------------------------------
/*3*/#define qassert(exp)														\
	do {																		\
		__qhelper_assertion_called();											\
		if (!(exp))																\
			throw QUnit::QFailure(__FILE__, __LINE__,  #exp "应为true");		\
	} while(0)
	
/*4*/#define qassert_equal(expect, exp)											\
	do {																		\
		using namespace QUnit::PrivateHelper;									\
		__qhelper_assertion_called();											\
		if (!equal(expect, exp))												\
		{																		\
			char msg[1024];														\
			sprintf(msg, #exp "不等于" #expect ", 期望值是%s，结果是%s",		\
				to_s(expect).c_str(), to_s(exp).c_str());						\
			throw QUnit::QFailure(__FILE__, __LINE__, msg);						\
		}																		\
	} while(0)

/*5*/#define qassert_not_equal(expect, exp)										\
	do {																		\
		using namespace QUnit::PrivateHelper;									\
		__qhelper_assertion_called();											\
		if (equal(expect, exp))													\
		{																		\
			char msg[1024];														\
			sprintf(msg, #exp "等于" #expect ", 期望不等于%s，结果与之相等",	\
				to_s(expect).c_str(), to_s(exp).c_str());						\
			throw QUnit::QFailure(__FILE__, __LINE__, msg);						\
		}																		\
	} while(0)

/*6*/#define qassert_not(exp)													\
	do {																		\
		__qhelper_assertion_called();											\
		if (exp)																\
		{																		\
			throw QUnit::QFailure(__FILE__, __LINE__, #exp "应为false");		\
		}																		\
	} while(0)

/*7*/#define qassert_null(exp)													\
	do {																		\
		__qhelper_assertion_called();											\
		if (exp)																\
		{																		\
			throw QUnit::QFailure(__FILE__, __LINE__, #exp "应为空");			\
		}																		\
	} while(0)

/*8*/#define qassert_not_null(exp)												\
	do {																		\
		__qhelper_assertion_called();											\
		if (!(exp))																\
		{																		\
			throw QUnit::QFailure(__FILE__, __LINE__, #exp "应为非空");			\
		}																		\
	} while(0)

/*9*/#define qassert_match(regex, exp)											\
	do {																		\
		__qhelper_assertion_called();											\
		CRegexpT<char> rx(regex);												\
		MatchResult result = rx.Match(exp);										\
		if (!result.IsMatched())												\
		{																		\
			throw QUnit::QFailure(__FILE__, __LINE__, #exp "应该匹配" #regex);	\
		}																		\
	} while(0)

/*10*/#define qassert_not_match(regex, exp)										\
	do {																		\
		__qhelper_assertion_called();											\
		CRegexpT<char> rx(regex);												\
		MatchResult result = rx.Match(exp);										\
		if (result.IsMatched())													\
		{																		\
			throw QUnit::QFailure(__FILE__, __LINE__, #exp "应该不匹配" #regex);\
		}																		\
	} while(0)


// ----------------------------------------------------------------------------
// 定义QUNIT_ANYTIME即可在非_DEBUG时做测试
#if !defined(_DEBUG) && !defined(QUNIT_ANYTIME)

/*1*/ #undef qtest
/*2*/ #undef qcase
/*3*/ #undef qassert
/*4*/ #undef qassert_not
/*5*/ #undef qassert_not_equal
/*6*/ #undef qassert_equal
/*7*/ #undef qassert_null
/*8*/ #undef qassert_not_null
/*9*/ #undef qassert_match
/*10*/#undef qassert_not_match

/*1*/ #define qtest(test, testcase) template<class T> static void __qhelper_gen_name(test, testcase, null)()
/*2*/ #define qcase(test) qtest(test, QDefaultCase)
/*3*/ #define qassert(x) (void*)0
/*4*/ #define qassert_not(x) qassert(0)
/*5*/ #define qassert_not_equal(x, y) qassert(0)
/*6*/ #define qassert_equal(x, y) qassert(0)
/*7*/ #define qassert_null(x) qassert(0)
/*8*/ #define qassert_not_null(x) qassert(0)
/*9*/ #define qassert_match(x, y) qassert(0)
/*10*/#define qassert_not_match(x, y) qassert(0)

#endif

// ----------------------------------------------------------------------------
__QUNIT_DLLEXPORT inline
QUnit::QModule& qunit_module_inst()
{
	return QUnit::QModule::inst();
}

// ----------------------------------------------------------------------------
#endif // __QUNIT_H__
