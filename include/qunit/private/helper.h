#ifndef __QUNIT_PRIVATE_HELPER__
#define __QUNIT_PRIVATE_HELPER__

#ifdef X_OS_LINUX
#include <alloca.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <sstream>


	
#ifdef X_CC_VC
	#define __QUNIT_DLLEXPORT extern "C" __declspec(dllexport)
#else
	#define __QUNIT_DLLEXPORT extern "C" 
#endif


namespace QUnit { namespace PrivateHelper
{
	class QRunCookie
	{
	public:
		int __assertion_counter;
	public:
		QRunCookie()
		{
			__initialize_cookie();
		}
		void __initialize_cookie()
		{
			__assertion_counter = 0;
		}
	};
	
	template<class T> inline
	std::string to_s(const T& value)
	{
		std::ostringstream o;
		o << value;
		return o.str();
	}
	inline
	std::string to_s(const std::string value)
	{
		std::ostringstream o;
		o << "\"" << value << "\"";
		return o.str();
	}
	inline
	std::string to_s(const char* value)
	{
		return to_s(std::string(value));
	}
	inline
	std::string to_s(const std::wstring value)
	{
		char* buf = (char*)alloca( (value.size()*2));
		wcstombs(buf, value.c_str(), value.size());
		return to_s(buf);
	}
	inline
	std::string to_s(const wchar_t* value)
	{
		return to_s(std::wstring(value));
	}



	template<class L, class R> inline 
	bool equal(const L& l, const R& r)
	{
		return l == r;
	}
	
	inline 
	bool equal(const char* l, const char* r)
	{
		return strcmp(l, r) == 0;
	}
	inline 
	bool equal(char* l, const char* r)
	{
		return strcmp(l, r) == 0;
	}
	inline 
	bool equal(const char* l, char* r)
	{
		return strcmp(l, r) == 0;
	}
	inline 
	bool equal(char* l, char* r)
	{
		return strcmp(l, r) == 0;
	}

	inline 
	bool equal(const wchar_t* l, const wchar_t* r)
	{
		return wcscmp(l, r) == 0;
	}
	inline 
	bool equal(wchar_t* l, const wchar_t* r)
	{
		return wcscmp(l, r) == 0;
	}
	inline 
	bool equal(const wchar_t* l, wchar_t* r)
	{
		return wcscmp(l, r) == 0;
	}
	inline 
	bool equal(wchar_t* l, wchar_t* r)
	{
		return wcscmp(l, r) == 0;
	}

}}


// ----------------------------------------------------------------------------
#define __qhelper_gen_name(test, fixture, suffix) qtest__##test##_of_##fixture_##suffix
#define __qhelper_assertion_called() ++__qunit_runner_inst->__assertion_counter


// ----------------------------------------------------------------------------
#endif // __QUNIT_PRIVATE_HELPER__
