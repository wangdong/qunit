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
// ��__qhelper_gen_name��������ʹ��QDefaultCaseʱ�ǲ��ܴ����::���ŵģ�
// Ҳ����˵��__qhelper_gen_name(testFoo, QUnit::QDefaultCase)
// �����ɲ��Ϸ��ı�ʶ��������ֻ���ڴ˽�֮using����֧��qtest(test, QDefaultCase)��
//
// ��Ȼ������죬���ǲ����Զ�Ϊ֮�����и��ѷ�����Ȼ���ã�
// ��ô������ᱣ���û���ֵ#6������������ʵ�ʵĺ�����
//

/*2*/#define qcase(test) qtest(test, QDefaultCase)



// ----------------------------------------------------------------------------
/*3*/#define qassert(exp)														\
	do {																		\
		__qhelper_assertion_called();											\
		if (!(exp))																\
			throw QUnit::QFailure(__FILE__, __LINE__,  #exp "ӦΪtrue");		\
	} while(0)
	
/*4*/#define qassert_equal(expect, exp)											\
	do {																		\
		using namespace QUnit::PrivateHelper;									\
		__qhelper_assertion_called();											\
		if (!equal(expect, exp))												\
		{																		\
			char msg[1024];														\
			sprintf(msg, #exp "������" #expect ", ����ֵ��%s�������%s",		\
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
			sprintf(msg, #exp "����" #expect ", ����������%s�������֮���",	\
				to_s(expect).c_str(), to_s(exp).c_str());						\
			throw QUnit::QFailure(__FILE__, __LINE__, msg);						\
		}																		\
	} while(0)

/*6*/#define qassert_not(exp)													\
	do {																		\
		__qhelper_assertion_called();											\
		if (exp)																\
		{																		\
			throw QUnit::QFailure(__FILE__, __LINE__, #exp "ӦΪfalse");		\
		}																		\
	} while(0)

/*7*/#define qassert_null(exp)													\
	do {																		\
		__qhelper_assertion_called();											\
		if (exp)																\
		{																		\
			throw QUnit::QFailure(__FILE__, __LINE__, #exp "ӦΪ��");			\
		}																		\
	} while(0)

/*8*/#define qassert_not_null(exp)												\
	do {																		\
		__qhelper_assertion_called();											\
		if (!(exp))																\
		{																		\
			throw QUnit::QFailure(__FILE__, __LINE__, #exp "ӦΪ�ǿ�");			\
		}																		\
	} while(0)

/*9*/#define qassert_match(regex, exp)											\
	do {																		\
		__qhelper_assertion_called();											\
		CRegexpT<char> rx(regex);												\
		MatchResult result = rx.Match(exp);										\
		if (!result.IsMatched())												\
		{																		\
			throw QUnit::QFailure(__FILE__, __LINE__, #exp "Ӧ��ƥ��" #regex);	\
		}																		\
	} while(0)

/*10*/#define qassert_not_match(regex, exp)										\
	do {																		\
		__qhelper_assertion_called();											\
		CRegexpT<char> rx(regex);												\
		MatchResult result = rx.Match(exp);										\
		if (result.IsMatched())													\
		{																		\
			throw QUnit::QFailure(__FILE__, __LINE__, #exp "Ӧ�ò�ƥ��" #regex);\
		}																		\
	} while(0)


// ----------------------------------------------------------------------------
// ����QUNIT_ANYTIME�����ڷ�_DEBUGʱ������
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
