#ifndef __QUNIT_RUNNER_XML_H__
#define __QUNIT_RUNNER_XML_H__

#include <iosfwd>

// -------------------------------------------------------------------------
namespace QUnit {

#define _A(name, value) #name "=" #value " "

	class XmlWr
	{
		FILE* m_wr;
		void _W(const char* s)
		{
			fwrite(s, strlen(s), 1, m_wr);
		}

	public:
		void putWr(FILE* wr)
		{
			m_wr = wr;
			_W("<?xml version='1.0' encoding='UTF-8'?>");
		}
		void beginElem(const char* name, const char* attr = NULL)
		{
			_W("<");
			_W(name);
			if (attr)
			{
				_W(" ");
				_W(attr);
			}
			_W(">");
		}
		void addText(const char* text)
		{
			_W(text);
		}
		void endElem(const char* name)
		{
			_W("</");
			_W(name);
			_W(">");
			_W("\n");
		}
		void close()
		{
			fclose(m_wr);
		}
	};


	class QNUnitXmlReporter : public QReporter
	{
		FILE* m_file;
	public:
		QNUnitXmlReporter(FILE* file)
		{
			m_file = file;
		}

	public:
		void end(const QResults& results, clock_t duration)
		{
			XmlWr wr;
			wr.putWr(m_file);

			{
			std::ostringstream o;
			o << _A(name,		'');
			o << _A(total,		'2' );
			o << _A(failures,	'2' );
			o << _A(not-run,	'2' );
			o << _A(date,		'0');
			o << _A(time,		'0');
			wr.beginElem("test-results", o.str().c_str());
			}

			{
			std::ostringstream o;
			o << _A(name,		''		);
			o << _A(success,	'True'	);
			o << _A(asserts,	'0' );
			wr.beginElem("test-suite", o.str().c_str());
			}
			wr.beginElem("results");


			printf("\nFinished in %f seconds.\n\n", duration / 1000.0);

			int index = 1;
			int failure_count = 0;
			int error_count = 0;
			int assertion_count = 0;
			QResults::const_iterator i = results.begin();
			for (; i < results.end(); ++i)
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
					{
					std::ostringstream o;
					o << _A(name,		''		);
					o << _A(executed,	'True'	);
					o << _A(success,	'False'	);
					o << _A(time,		'0'		);
					o << _A(asserts,	'3'		);
					wr.beginElem("test-case", o.str().c_str());
					wr.beginElem("failure", o.str().c_str());
					wr.beginElem("message", o.str().c_str());
					wr.addText("<![CDATA[   who am i ? - ]]>");
					wr.endElem("message");
					wr.endElem("failure");
					wr.endElem("test-case");
					}
					
					printf("  %d) Failure:\n", index);
					printf("%s [%s:%d]:\n%s\n\n", test_name.c_str(),
						i->fail.file.c_str(), i->fail.line, i->fail.condition.c_str());
					++failure_count;
					++index;
					break;
				case QResult::error:
					{
					std::ostringstream o;
					o << _A(name,		''		);
					o << _A(executed,	'True'	);
					o << _A(success,	'True'	);
					o << _A(time,		'0'		);
					o << _A(asserts,	'3'		);
					wr.beginElem("test-case", o.str().c_str());
					wr.endElem("test-case");
					}

					printf("  %d) Error:\n", index);
					printf("%s(%s):\n%s\n\n", i->name.c_str(), i->testcase.c_str(), i->msg.c_str());
					++error_count;
					++index;
					break;
				}
				assertion_count += i->assertion_count;
			}
			

			CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbiInfo);

			if (error_count + failure_count)
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xf|BACKGROUND_RED);
			else
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0xf|BACKGROUND_GREEN);
			
			printf("%d tests, %d assertions, %d failures, %d errors\n", 
				results.size(), assertion_count, failure_count, error_count);

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), csbiInfo.wAttributes);

			wr.endElem("results");
			wr.endElem("test-suite");
			wr.endElem("test-results");
		}
	};

}

// -------------------------------------------------------------------------
//	

#endif /* __QUNIT_RUNNER_XML_H__ */
