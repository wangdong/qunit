// ****************************************************************************
// ^FILE: options.h - option parsing classes
//
// ^DESCRIPTION:
//    This file defines classes used to parse command-line options.
//    Options may be parsed from an array of strings, or from any structure
//    for which a corresponding option-iterator exists.
//
// ^HISTORY:
//    03/06/92  Brad Appleton   <bradapp@enteract.com>   Created
//
//    03/23/93	Brad Appleton	<bradapp@enteract.com>
//    - Added OptIstreamIter class
//
//    03/08/94	Brad Appleton	<bradapp@enteract.com>
//    - Added Options::reset() member function
//
//    07/31/97	Brad Appleton	<bradapp@enteract.com>
//    - Added PARSE_POS control flag and POSITIONAL return value
// ^^**************************************************************************

#ifndef _options_h
#define _options_h

class  ostream;
class  istream;

// Abstract class to iterate through options/arguments
//
class OptIter {
public:
   OptIter(void) {}
   virtual ~OptIter(void);

      // curr() returns the current item in the iterator without
      // advancing on to the next item. If we are at the end of items
      // then NULL is returned.
   virtual const char *
   curr(void) = 0;

      // next() advances to the next item.
   virtual void
   next(void) = 0;

      // operator() returns the current item in the iterator and then
      // advances on to the next item. If we are at the end of items
      // then NULL is returned.
   virtual const char *
   operator()(void);
} ;

// Abstract class for a rewindable OptIter
//
class OptIterRwd : public OptIter {
public:
   OptIterRwd(void);

   virtual ~OptIterRwd(void);

   virtual const char *
   curr(void) = 0;

   virtual void
   next(void) = 0;

   virtual const char *
   operator()(void) = 0;

      // rewind() resets the "current-element" to the first one in the "list"
   virtual void
   rewind(void) = 0;
} ;

// Class to iterate through an array of tokens. The array may be terminated
// by NULL or a count containing the number of tokens may be given.
//
class OptArgvIter : public OptIterRwd {
private:
   int            ndx;   // index of current arg
   int            ac;    // arg count
   const char * const * av;  // arg vector

public:
   OptArgvIter(const char * const argv[])
      : av(argv), ac(-1), ndx(0) {}

   OptArgvIter(int argc, const char * const argv[])
      : av(argv), ac(argc), ndx(0) {}

   virtual
   ~OptArgvIter(void);

   virtual const char *
   curr(void);

   virtual void
   next(void);

   virtual const char *
   operator()(void);

   virtual void
   rewind(void);

      // index returns the current index to use for argv[]
   int index(void)  { return  ndx; }
} ;


// Class to iterate through a string containing delimiter-separated tokens
//
class OptStrTokIter : public OptIterRwd {
private:
   unsigned     len;        // length of token-string
   const char * str;        // the token-string
   const char * seps;       // delimiter-set (separator-characters)
   const char * cur;        // current token
   char       * tokstr;     // our copy of the token-string

public:
   OptStrTokIter(const char * tokens, const char * delimiters =0);

   virtual
   ~OptStrTokIter(void);

   virtual const char *
   curr(void);

   virtual void
   next(void);

   virtual const char *
   operator()(void);

   virtual void
   rewind(void);

      // delimiters() with NO arguments returns the current set of delimiters,
      // If an argument is given then it is used as the new set of delimiters.
   const char *
   delimiters(void)  { return  seps; }

   void
   delimiters(const char * delims);
} ;


   // OptIstreamIter is a class for iterating over arguments that come
   // from an input stream. Each line of the input stream is considered
   // to be a set of white-space separated tokens. If the the first
   // non-white character on a line is '#' ('!' for VMS systems) then
   // the line is considered a comment and is ignored.
   //
   // *Note:: If a line is more than 1022 characters in length then we
   // treat it as if it were several lines of length 1022 or less.
   //
   // *Note:: The string tokens returned by this iterator are pointers
   //         to temporary buffers which may not necessarily stick around
   //         for too long after the call to curr() or operator(), hence
   //         if you need the string value to persist - you will need to
   //         make a copy.
   //
class OptIstreamIter : public OptIter {
private:
   istream & is ;
   OptStrTokIter * tok_iter ;

   void
   fill(void);

public:
   OptIstreamIter(istream & input);

   virtual
   ~OptIstreamIter(void);

   virtual const char *
   curr(void);

   virtual void
   next(void);

   virtual const char *
   operator()(void);
} ;


// Now we are ready to define a class to declare and parse command-options
//
//
// CLASS
// =====
// Options  -- parse command-line options
//
// SYNOPSIS
// ========
//   #include <options.h>
//
//   Options opts(cmdname, optv);
//   char cmdname[], *optv[];
//
// DESCRIPTION
// ===========
// The Options constructor expects a command-name (usually argv[0]) and
// a pointer to an array of strings.  The last element in this array MUST
// be NULL. Each non-NULL string in the array must have the following format:
//
//   The 1st character must be the option-name ('c' for a -c option).
//
//   The 2nd character must be one of '|', '?', ':', '*', or '+'.
//      '|' -- indicates that the option takes NO argument;
//      '?' -- indicates that the option takes an OPTIONAL argument;
//      ':' -- indicates that the option takes a REQUIRED argument;
//      '*' -- indicates that the option takes 0 or more arguments;
//      '+' -- indicates that the option takes 1 or more arguments;
//
//   The remainder of the string must be the long-option name.
//
//   If desired, the long-option name may be followed by one or more
//   spaces and then by the name of the option value. This name will
//   be used when printing usage messages. If the option-value-name
//   is not given then the string "<value>" will be used in usage
//   messages.
//
//   One may use a space to indicate that a particular option does not
//   have a corresponding long-option.  For example, "c: " (or "c:")
//   means the -c option takes a value & has NO corresponding long-option.
//
//   To specify a long-option that has no corresponding single-character
//   option is a bit trickier: Options::operator() still needs an "option-
//   character" to return when that option is matched. One may use a whitespace
//   character or a non-printable character as the single-character option
//   in such a case. (hence " |hello" would only match "--hello").
//
//   EXCEPTIONS TO THE ABOVE:
//   ------------------------ 
//   If the 1st character of the string is '-', then the rest of the
//   string must correspond to the above format, and the option is
//   considered to be a hidden-option. This means it will be parsed
//   when actually matching options from the command-line, but will
//   NOT show-up if a usage message is printed using the usage() member
//   function. Such an example might be "-h|hidden". If you want to
//   use any "dummy" options (options that are not parsed, but that
//   to show up in the usage message), you can specify them along with
//   any positional parameters to the usage() member function.
//
//   If the 2nd character of the string is '\0' then it is assumed
//   that there is no corresponding long-option and that the option
//   takes no argument (hence "f", and "f| " are equivalent).
//
//   Examples:
//      const char * optv[] = {
//          "c:count   <number>",
//          "s?str     <string>",
//          "x",
//          " |hello",
//          "g+groups  <newsgroup>",
//          NULL
//      } ;
//
//      optv[] now corresponds to the following:
//
//            usage: cmdname [-c|--count <number>] [-s|--str [<string>]]
//                           [-x] [--hello] [-g|--groups <newsgroup> ...]
//
// Long-option names are matched case-insensitive and only a unique prefix
// of the name needs to be specified.
//
// Option-name characters are case-sensitive!
//
// CAVEAT
// ======
// Because of the way in which multi-valued options and options with optional
// values are handled, it is NOT possible to supply a value to an option in
// a separate argument (different argv[] element) if the value is OPTIONAL
// and begins with a '-'. What this means is that if an option "-s" takes an
// optional value value and you wish to supply a value of "-foo" then you must
// specify this on the command-line as "-s-foo" instead of "-s -foo" because
// "-s -foo" will be considered to be two separate sets of options.
//
// A multi-valued option is terminated by another option or by the end-of
// options. The following are all equivalent (if "-l" is a multi-valued
// option and "-x" is an option that takes no value):
//
//    cmdname -x -l item1 item2 item3 -- arg1 arg2 arg3
//    cmdname -x -litem1 -litem2 -litem3 -- arg1 arg2 arg3
//    cmdname -l item1 item2 item3 -x arg1 arg2 arg3
//
//
// EXAMPLE
// =======
//    #include <options.h>
//
//    static const char * optv[] = {
//       "H|help",
//       "c:count   <number>",
//       "s?str     <string>",
//       "x",
//       " |hello",
//       "g+groups  <newsgroup>",
//       NULL
//    } ;
//
//    main(int argc, char * argv[]) {
//       int  optchar;
//       const char * optarg;
//       const char * str = "default_string";
//       int  count = 0, xflag = 0, hello = 0;
//       int  errors = 0, ngroups = 0;
//    
//       Options  opts(*argv, optv);
//       OptArgvIter  iter(--argc, ++argv);
//    
//       while( optchar = opts(iter, optarg) ) {
//          switch (optchar) {
//          case 'H' :
//             opts.usage(cout, "files ...");
//             exit(0);
//             break;
//          case 'g' :
//             ++ngroups; break;  // the groupname is in "optarg"
//          case 's' :
//             str = optarg; break;
//          case 'x' :
//             ++xflag; break;
//          case ' ' :
//             ++hello; break;
//          case 'c' :
//             if (optarg == NULL)  ++errors;
//             else  count = (int) atol(optarg);
//             break;
//          default :  ++errors; break;
//          } //switch
//       }
//    
//       if (errors || (iter.index() == argc)) {
//          if (! errors) {
//             cerr << opts.name() << ": no filenames given." << endl ;
//          }
//          opts.usage(cerr, "files ...");
//          exit(1);
//       }
//    
//       cout << "xflag=" << ((xflag) ? "ON"  : "OFF") << endl
//            << "hello=" << ((hello) ? "YES" : "NO") << endl
//            << "count=" << count << endl
//            << "str=\"" << ((str) ? str : "No value given!") << "\"" << endl
//            << "ngroups=" << ngroups << endl ;
//    
//       if (iter.index() < argc) {
//          cout << "files=" ;
//          for (int i = iter.index() ; i < argc ; i++) {
//             cout << "\"" << argv[i] << "\" " ;
//          }
//          cout << endl ;
//       }
//    }
//
class Options {
private:
   unsigned       explicit_end : 1;  // were we terminated because of "--"?
   unsigned       optctrls : 7;  // control settings (a set of OptCtrl masks)
   const char  * const * optvec; // vector of option-specifications (last=NULL)
   const char   * nextchar;      // next option-character to process
   const char   * listopt;       // last list-option we matched
   const char   * cmdname;       // name of the command

   void
   check_syntax(void) const;

   const char *
   match_opt(char opt, int ignore_case =0) const;

   const char *
   match_longopt(const char * opt, int  len, int & ambiguous) const;

   int
   parse_opt(OptIter & iter, const char * & optarg);

   int
   parse_longopt(OptIter & iter, const char * & optarg);

public:
   enum OptCtrl {
      DEFAULT    = 0x00,  // Default setting
      ANYCASE    = 0x01,  // Ignore case when matching short-options
      QUIET      = 0x02,  // Dont print error messages
      PLUS       = 0x04,  // Allow "+" as a long-option prefix
      SHORT_ONLY = 0x08,  // Dont accept long-options
      LONG_ONLY  = 0x10,  // Dont accept short-options
                            // (also allows "-" as a long-option prefix).
      NOGUESSING = 0x20,  // Normally, when we see a short (long) option
                            // on the command line that doesnt match any
                            // known short (long) options, then we try to
                            // "guess" by seeing if it will match any known
                            // long (short) option. Setting this mask prevents
                            // this "guessing" from occurring.
      PARSE_POS = 0x40    // By default, Options will not present positional
                            // command-line arguments to the user and will
                            // instead stop parsing when the first positonal
                            // argument has been encountered. If this flag
                            // is given, Options will present positional
                            // arguments to the user with a return code of
                            // POSITIONAL; ENDOPTS will be returned only
                            // when the end of the argument list is reached.
   } ;

      // Error return values for operator()
      //
   enum OptRC {
      ENDOPTS    =  0,
      BADCHAR    = -1,
      BADKWD     = -2,
      AMBIGUOUS  = -3,
      POSITIONAL = -4
   } ;

   Options(const char * name, const char * const optv[]);

   virtual
   ~Options(void);

      // name() returns the command name
   const char *
   name(void) const { return  cmdname; }

      // ctrls() (with no arguments) returns the existing control settings
   unsigned
   ctrls(void) const { return  optctrls; }

      // ctrls() (with 1 argument) sets new control settings
   void
   ctrls(unsigned newctrls) { optctrls = newctrls; }

      // reset for another pass to parse for options
   void
   reset(void) { nextchar = listopt = NULL; }
  
      // usage() prints options usage (followed by any positional arguments
      // listed in the parameter "positionals") on the given outstream
   void
   usage(ostream & os, const char * positionals) const ;

      // operator() iterates through the arguments as necessary (using the
      // given iterator) and returns the character value of the option
      // (or long-option) that it matched. If the option has a value
      // then the value given may be found in optarg (otherwise optarg
      // will be NULL).
      //
      // 0 is returned upon end-of-options. At this point, "iter" may
      // be used to process any remaining positional parameters. If the
      // PARSE_POS control-flag is set then 0 is returned only when all
      // arguments in "iter" have been exhausted.
      //
      // If an invalid option is found then BADCHAR is returned and *optarg
      // is the unrecognized option character.
      //
      // If an invalid long-option is found then BADKWD is returned and optarg
      // points to the bad long-option.
      //
      // If an ambiguous long-option is found then AMBIGUOUS is returned and
      // optarg points to the ambiguous long-option.
      //
      // If the PARSE_POS control-flag is set then POSITIONAL is returned
      // when a positional argument is encountered and optarg points to
      // the positonal argument (and "iter" is advanced to the next argument
      // in the iterator).
      //
      // Unless Options::QUIET is used, missing option-arguments and
      // invalid options (and the like) will automatically cause error
      // messages to be issued to cerr.
   int
   operator()(OptIter & iter, const char * & optarg) ;

      // Call this member function after operator() has returned 0
      // if you want to know whether or not options were explicitly
      // terminated because "--" appeared on the command-line.
      //
   int
   explicit_endopts() const { return  explicit_end; }
} ;


#ifdef USE_STDIO
# include <stdio.h>
#else
# include <iostream.h>
#endif

#include <ctype.h>
#include <string.h>

extern "C" {
   void  exit(int);
}

static const char ident[] = "@(#)Options  1.05" ;
static const char NULL_spec[] = "\0\0\0" ;
static const char WHITESPACE[] = " \t\n\r\v\f" ;
static const char * default_delims = WHITESPACE ;
enum {MAX_LINE_LEN = 1024};


   // I need a portable version of "tolower" that does NOT modify
   // non-uppercase characters.
   //
#define  TOLOWER(c)  (isupper(c) ? tolower(c) : c)

   // Use this to shut the compiler up about NULL strings
#define  NULLSTR  (char *)NULL

// ******************************************************** insertion operators

  // If you are using <stdio.h> then you need this stuff!
  // If you are using <iostream.h> then #ifdef this stuff out
  //


#ifdef  USE_STDIO

   // Implement just enough of ostream to get this file to compile
   //

static const char endl = '\n' ;

class  ostream {
public:
   ostream(FILE * fileptr) : fp(fileptr) {}

   ostream &
   operator<<(char ch);

   ostream &
   operator<<(const char * str);

   ostream &
   write(const char * buf, unsigned bufsize);

private:
   FILE * fp;
} ;

inline ostream &
ostream::operator<<(char ch) {
   fputc(ch, fp);
   return *this;
}

inline ostream &
ostream::operator<<(const char * str) {
   fputs(str, fp);
   return *this;
}

inline ostream &
ostream::write(const char * buf, unsigned ) {
   fputs(buf, fp);
   return *this;
}

static  ostream  cerr(stderr);
static  ostream  cout(stdout);

#endif  /* USE_STDIO */

// ************************************************************** OptIter

inline OptIter::~OptIter(void) {}

inline const char *
OptIter::operator()(void)  {
   const char * elt = curr();
   (void) next();
   return  elt;
}

// ************************************************************** OptIterRwd

inline OptIterRwd::OptIterRwd(void) {}

inline OptIterRwd::~OptIterRwd(void) {}

// ************************************************************** OptArgvIter

inline OptArgvIter::~OptArgvIter(void) {}

inline const char *
OptArgvIter::curr(void) {
   return ((ndx == ac) || (av[ndx] == NULL)) ? NULLSTR : av[ndx];
}

inline void
OptArgvIter::next(void) {
   if ((ndx != ac) && av[ndx]) ++ndx;
}

inline const char *
OptArgvIter::operator()(void) {
   return ((ndx == ac) || (av[ndx] == NULL)) ? NULLSTR : av[ndx++];
}

inline void
OptArgvIter::rewind(void) { ndx = 0; }

// ************************************************************** OptStrTokIter

inline OptStrTokIter::OptStrTokIter(const char * tokens, const char * delimiters)
   : len(unsigned(strlen(tokens))), str(tokens), seps(delimiters),
     cur(NULLSTR), tokstr(NULLSTR)
{
   if (seps == NULL)  seps = default_delims;
   tokstr = new char[len + 1];
   (void) ::strcpy(tokstr, str);
   cur = ::strtok(tokstr, seps);
}


inline OptStrTokIter::~OptStrTokIter(void) { delete [] tokstr; }

inline void
OptStrTokIter::delimiters(const char * delims) {
  seps = (delims) ? delims : default_delims ;
}

inline const char *
OptStrTokIter::curr(void) { return cur; }

inline void
OptStrTokIter::next(void) { if (cur) cur = ::strtok(NULL, seps); }

inline const char *
OptStrTokIter::operator()(void) {
   const char * elt = cur;
   if (cur) cur = ::strtok(NULL, seps);
   return  elt;
}

inline void
OptStrTokIter::rewind(void) {
   (void) ::strcpy(tokstr, str);
   cur = ::strtok(tokstr, seps);
}

// ************************************************************* OptIstreamIter

#ifdef vms
   enum { c_COMMENT = '!' } ;
#else
   enum { c_COMMENT = '#' } ;
#endif


   // Constructor
inline OptIstreamIter::OptIstreamIter(istream & input) : is(input), tok_iter(NULL)
{
#ifdef  USE_STDIO
   fprintf(stderr, "%s: Can't use OptIstreamIter class:\n",
                   "OptIstreamIter::OptIstreamIter");
   fprintf(stderr, "\tOptions(3C++) was compiled with USE_STDIO #defined.\n");
   exit(-1);
#endif  /* USE_STDIO */
}

   // Destructor
inline OptIstreamIter::~OptIstreamIter(void) {
   delete  tok_iter;
}

inline const char *
OptIstreamIter::curr(void) {
#ifdef  USE_STDIO
   return  NULLSTR;
#else
   const char * result = NULLSTR;
   if (tok_iter)  result = tok_iter->curr();
   if (result)  return  result;
   fill();
   return (! is) ? NULLSTR : tok_iter->curr();
#endif  /* USE_STDIO */
}

inline void
OptIstreamIter::next(void) {
#ifdef  USE_STDIO
   return;
#else
   const char * result = NULLSTR;
   if (tok_iter)  result = tok_iter->operator()();
   if (result)  return;
   fill();
   if (! is) tok_iter->next();
#endif  /* USE_STDIO */
}

inline const char *
OptIstreamIter::operator()(void) {
#ifdef  USE_STDIO
   return  NULLSTR;
#else
   const char * result = NULLSTR;
   if (tok_iter)  result = tok_iter->operator()();
   if (result)  return  result;
   fill();
   return (! is) ? NULLSTR : tok_iter->operator()();
#endif  /* USE_STDIO */
}

   // What we do is this: for each line of text in the istream, we use
   // a OptStrTokIter to iterate over each token on the line.
   //
   // If the first non-white character on a line is c_COMMENT, then we
   // consider the line to be a comment and we ignore it.
   //
inline void
OptIstreamIter::fill(void) {
#ifdef USE_STDIO
   return;
#else
   char buf[MAX_LINE_LEN];
   do {
      *buf = '\0';
      is.getline(buf, sizeof(buf));
      char * ptr = buf;
      while (isspace(*ptr)) ++ptr;
      if (*ptr && (*ptr != c_COMMENT)) {
         delete  tok_iter;
         tok_iter = new OptStrTokIter(ptr);
         return;
      }
   } while (is);
#endif  /* USE_STDIO */
}

// **************************************************** Options class utilities

   // Is this option-char null?
inline static int
isNullOpt(char optchar) {
   return  ((! optchar) || isspace(optchar) || (! isprint(optchar)));
}
   
   // Check for explicit "end-of-options"
inline static int
isEndOpts(const char * token) {
   return ((token == NULL) || (! ::strcmp(token, "--"))) ;
}

   // See if an argument is an option
inline static int
isOption(unsigned  flags, const char * arg) {
   return  (((*arg != '\0') || (arg[1] != '\0')) &&
            ((*arg == '-')  || ((flags & Options::PLUS) && (*arg == '+')))) ;
}

   // See if we should be parsing only options or if we also need to
   // parse positional arguments
inline static int
isOptsOnly(unsigned  flags) {
   return  (flags & Options::PARSE_POS) ? 0 : 1;
}

   // return values for a keyword matching function
enum kwdmatch_t { NO_MATCH, PARTIAL_MATCH, EXACT_MATCH } ;

// ---------------------------------------------------------------------------
// ^FUNCTION: kwdmatch - match a keyword
//
// ^SYNOPSIS:
//    static kwdmatch_t kwdmatch(src, attempt, len)
//
// ^PARAMETERS:
//    char * src -- the actual keyword to match
//    char * attempt -- the possible keyword to compare against "src"
//    int len -- number of character of "attempt" to consider
//               (if 0 then we should use all of "attempt")
//
// ^DESCRIPTION:
//    See if "attempt" matches some prefix of "src" (case insensitive).
//
// ^REQUIREMENTS:
//    - attempt should be non-NULL and non-empty
//
// ^SIDE-EFFECTS:
//    None.
//
// ^RETURN-VALUE:
//    An enumeration value of type kwdmatch_t corresponding to whether
//    We had an exact match, a partial match, or no match.
//
// ^ALGORITHM:
//    Trivial
// ^^-------------------------------------------------------------------------
inline kwdmatch_t
kwdmatch(const char * src, const char * attempt, int len =0) {
   int  i;

   if (src == attempt)  return  EXACT_MATCH ;
   if ((src == NULL) || (attempt == NULL))  return  NO_MATCH ;
   if ((! *src) && (! *attempt))  return  EXACT_MATCH ;
   if ((! *src) || (! *attempt))  return  NO_MATCH ;

   for (i = 0 ; ((i < len) || (len == 0)) &&
                (attempt[i]) && (attempt[i] != ' ') ; i++) {
      if (TOLOWER(src[i]) != TOLOWER(attempt[i]))  return  NO_MATCH ;
   }

   return  (src[i]) ? PARTIAL_MATCH : EXACT_MATCH ;
}

// **************************************************************** OptionSpec

   // Class that represents an option-specification
   //    *NOTE*:: Assumes that the char-ptr given to the constructor points
   //             to storage that will NOT be modified and whose lifetime will
   //             be as least as long as the OptionSpec object we construct.
   //
class OptionSpec {
public:
   OptionSpec(const char * decl =NULLSTR)
      : hidden(0), spec(decl)
   {
      if (spec == NULL)  spec = NULL_spec;
      CheckHidden();
   }

   OptionSpec(const OptionSpec & cp) : hidden(cp.hidden), spec(cp.spec) {}

   // NOTE: use default destructor!

      // Assign to another OptionSpec
   OptionSpec &
   operator=(const OptionSpec & cp) {
      if (this != &cp) {
         spec = cp.spec;
         hidden = cp.hidden;
      }
      return *this;
   }

      // Assign to a string
   OptionSpec &
   operator=(const char * decl) {
      if (spec != decl) {
         spec = decl;
         hidden = 0;
         CheckHidden();
      }
      return *this;
   }

      // Convert to char-ptr by returning the original declaration-string
   operator const char*() { return  isHiddenOpt() ? (spec - 1) : spec; }

      // Is this option NULL?
   int
   isNULL(void) const { return ((spec == NULL) || (spec == NULL_spec)); }

      // Is this options incorrectly specified?
   int
   isSyntaxError(const char * name) const;

      // See if this is a Hidden option
   int
   isHiddenOpt(void) const { return  hidden; }

      // Get the corresponding option-character
   char
   OptChar(void) const { return  *spec; }

      // Get the corresponding long-option string
   const char *
   LongOpt(void) const {
       return  (spec[1] && spec[2] && (! isspace(spec[2]))) ? (spec + 2) : NULLSTR;
   }

      // Does this option require an argument?
   int
   isValRequired(void) const {
      return  ((spec[1] == ':') || (spec[1] == '+'));
   }

      // Does this option take an optional argument?
   int
   isValOptional(void) const {
      return  ((spec[1] == '?') || (spec[1] == '*'));
   }

      // Does this option take no arguments?
   int
   isNoArg(void) const {
      return  ((spec[1] == '|') || (! spec[1]));
   }

      // Can this option take more than one argument?
   int
   isList(void) const {
      return  ((spec[1] == '+') || (spec[1] == '*'));
   }

      // Does this option take any arguments?
   int
   isValTaken(void) const {
      return  (isValRequired() || isValOptional()) ;
   }

      // Format this option in the given buffer
   unsigned
   Format(char * buf, unsigned optctrls) const;

private:
   void
   CheckHidden(void) {
      if ((! hidden) && (*spec == '-')) {
         ++hidden;
         ++spec;
      }
   }

   unsigned     hidden : 1;  // hidden-flag
   const char * spec;        // string specification

} ;

inline int
OptionSpec::isSyntaxError(const char * name) const {
   int  error = 0;
   if ((! spec) || (! *spec)) {
      cerr << name << ": empty option specifier." << endl;
      cerr << "\tmust be at least 1 character long." << endl;
      ++error;
   } else if (spec[1] && (strchr("|?:*+", spec[1]) == NULL)) {
      cerr << name << ": bad option specifier \"" << spec << "\"." << endl;
      cerr << "\t2nd character must be in the set \"|?:*+\"." << endl;
      ++error;
   }
   return  error;
}

// ---------------------------------------------------------------------------
// ^FUNCTION: OptionSpec::Format - format an option-spec for a usage message
//
// ^SYNOPSIS:
//    unsigned OptionSpec::Format(buf, optctrls) const
//
// ^PARAMETERS:
//    char * buf -- where to print the formatted option
//    unsigned  optctrls -- option-parsing configuration flags
//
// ^DESCRIPTION:
//    Self-explanatory.
//
// ^REQUIREMENTS:
//    - buf must be large enough to hold the result
//
// ^SIDE-EFFECTS:
//    - writes to buf.
//
// ^RETURN-VALUE:
//    Number of characters written to buf.
//
// ^ALGORITHM:
//    Follow along in the source - it's not hard but it is tedious!
// ^^-------------------------------------------------------------------------
inline unsigned
OptionSpec::Format(char * buf, unsigned optctrls) const {
#ifdef NO_USAGE
   return  (*buf = '\0');
#else
   static  char default_value[] = "<value>";
   if (isHiddenOpt())  return (unsigned)(*buf = '\0');
   char optchar = OptChar();
   const char * longopt = LongOpt();
   char * p = buf ;

   const char * value = NULLSTR;
   int    longopt_len = 0;
   int    value_len = 0;

   if (longopt) {
      value = ::strchr(longopt, ' ');
      longopt_len = (value) ? (value - longopt) : ::strlen(longopt);
   } else {
      value = ::strchr(spec + 1, ' ');
   }
   while (value && (*value == ' '))  ++value;
   if (value && *value) {
      value_len = ::strlen(value);
   } else {
      value = default_value;
      value_len = sizeof(default_value) - 1;
   }

   if ((optctrls & Options::SHORT_ONLY) &&
       ((! isNullOpt(optchar)) || (optctrls & Options::NOGUESSING))) {
      longopt = NULLSTR;
   }
   if ((optctrls & Options::LONG_ONLY) &&
       (longopt || (optctrls & Options::NOGUESSING))) {
      optchar = '\0';
   }
   if (isNullOpt(optchar) && (longopt == NULL)) {
      *buf = '\0';
      return  0;
   }

   *(p++) = '[';

   // print the single character option
   if (! isNullOpt(optchar)) {
      *(p++) = '-';
      *(p++) = optchar;
   }

   if ((! isNullOpt(optchar)) && (longopt))  *(p++) = '|';

   // print the long option
   if (longopt) {
      *(p++) = '-';
      if (! (optctrls & (Options::LONG_ONLY | Options::SHORT_ONLY))) {
         *(p++) = '-';
      }
      strncpy(p, longopt, longopt_len);
      p += longopt_len;
   }

   // print any argument the option takes
   if (isValTaken()) {
      *(p++) = ' ' ;
      if (isValOptional())  *(p++) = '[' ;
      strcpy(p, value);
      p += value_len;
      if (isList()) {
         strcpy(p, " ...");
         p += 4;
      }
      if (isValOptional())  *(p++) = ']' ;
   }

   *(p++) = ']';
   *p = '\0';

   return  (unsigned) strlen(buf);
#endif  /* USE_STDIO */
}

// ******************************************************************* Options

#if (defined(MSWIN) || defined(OS2) || defined(MSDOS))
# define DIR_SEP_CHAR '\\'
#else
# define DIR_SEP_CHAR '/'
#endif

inline Options::Options(const char * name, const char * const optv[])
   : cmdname(name), optvec(optv), explicit_end(0), optctrls(DEFAULT),
     nextchar(NULLSTR), listopt(NULLSTR)
{
   const char * basename = ::strrchr(cmdname, DIR_SEP_CHAR);
   if (basename)  cmdname = basename + 1;
   check_syntax();
}

inline Options::~Options(void) {}

   // Make sure each option-specifier has correct syntax.
   //
   // If there is even one invalid specifier, then exit ungracefully!
   //
inline void
Options::check_syntax(void) const {
   int  errors = 0;
   if ((optvec == NULL) || (! *optvec))  return;

   for (const char * const * optv = optvec ; *optv ; optv++) {
      OptionSpec  optspec = *optv;
      errors += optspec.isSyntaxError(cmdname);
   }
   if (errors)  exit(127);
}

// ---------------------------------------------------------------------------
// ^FUNCTION: Options::match_opt - match an option
//
// ^SYNOPSIS:
//    const char * match_opt(opt, int  ignore_case) const
//
// ^PARAMETERS:
//    char opt -- the option-character to match
//    int  ignore_case -- should we ignore character-case?
//
// ^DESCRIPTION:
//    See if "opt" is found in "optvec"
//
// ^REQUIREMENTS:
//    - optvec should be non-NULL and terminated by a NULL pointer.
//
// ^SIDE-EFFECTS:
//    None.
//
// ^RETURN-VALUE:
//    NULL if no match is found,
//    otherwise a pointer to the matching option-spec.
//
// ^ALGORITHM:
//    foreach option-spec
//       - see if "opt" is a match, if so return option-spec
//    end-for
// ^^-------------------------------------------------------------------------
inline const char *
Options::match_opt(char opt, int ignore_case) const {
   if ((optvec == NULL) || (! *optvec))  return  NULLSTR;

   for (const char * const * optv = optvec ; *optv ; optv++) {
      OptionSpec  optspec = *optv;
      char optchar = optspec.OptChar();
      if (isNullOpt(optchar))  continue;
      if (opt == optchar) {
         return  optspec;
      } else if (ignore_case && (TOLOWER(opt) == TOLOWER(optchar))) {
         return  optspec;
      }
   }

   return  NULLSTR;  // not found
}

// ---------------------------------------------------------------------------
// ^FUNCTION: Options::match_longopt - match a long-option
//
// ^SYNOPSIS:
//   const char * Options::match_longopt(opt, len, ambiguous)
//
// ^PARAMETERS:
//    char * opt -- the long-option to match
//    int len -- the number of character of "opt" to match
//    int & ambiguous -- set by this routine before returning.
//
// ^DESCRIPTION:
//    Try to match "opt" against some unique prefix of a long-option
//    (case insensitive).
//
// ^REQUIREMENTS:
//    - optvec should be non-NULL and terminated by a NULL pointer.
//
// ^SIDE-EFFECTS:
//    - *ambiguous is set to '1' if "opt" matches >1 long-option
//      (otherwise it is set to 0).
//
// ^RETURN-VALUE:
//    NULL if no match is found,
//    otherwise a pointer to the matching option-spec.
//
// ^ALGORITHM:
//    ambiguous is FALSE
//    foreach option-spec
//       if we have an EXACT-MATCH, return the option-spec
//       if we have a partial-match then
//          if we already had a previous partial match then
//             set ambiguous = TRUE and return NULL
//          else
//             remember this options spec and continue matching
//          end-if
//       end-if
//    end-for
//    if we had exactly 1 partial match return it, else return NULL
// ^^-------------------------------------------------------------------------
inline const char *
Options::match_longopt(const char * opt, int  len, int & ambiguous) const {
   kwdmatch_t  result;
   const char * matched = NULLSTR ;

   ambiguous = 0;
   if ((optvec == NULL) || (! *optvec))  return  NULLSTR;

   for (const char * const * optv = optvec ; *optv ; optv++) {
      OptionSpec  optspec = *optv;
      const char * longopt = optspec.LongOpt();
      if (longopt == NULL)  continue;
      result = kwdmatch(longopt, opt, len);
      if (result == EXACT_MATCH) {
         return  optspec;
      } else if (result == PARTIAL_MATCH) {
         if (matched) {
            ++ambiguous;
            return  NULLSTR;
         } else {
            matched = optspec;
         }
      }
   }//for

   return  matched;
}

// ---------------------------------------------------------------------------
// ^FUNCTION: Options::parse_opt - parse an option
//
// ^SYNOPSIS:
//    int Options::parse_opt(iter, optarg)
//
// ^PARAMETERS:
//    OptIter & iter -- option iterator
//    const char * & optarg -- where to store any option-argument
//
// ^DESCRIPTION:
//    Parse the next option in iter (advancing as necessary).
//    Make sure we update the nextchar pointer along the way. Any option
//    we find should be returned and optarg should point to its argument.
//
// ^REQUIREMENTS:
//    - nextchar must point to the prospective option character
//
// ^SIDE-EFFECTS:
//    - iter is advanced when an argument completely parsed
//    - optarg is modified to point to any option argument
//    - if Options::QUIET is not set, error messages are printed on cerr
//
// ^RETURN-VALUE:
//    'c' if the -c option was matched (optarg points to its argument)
//    BADCHAR if the option is invalid (optarg points to the bad
//                                                   option-character).
//
// ^ALGORITHM:
//    It gets complicated -- follow the comments in the source.
// ^^-------------------------------------------------------------------------
inline int
Options::parse_opt(OptIter & iter, const char * & optarg) {
   listopt = NULLSTR;  // reset the list pointer

   if ((optvec == NULL) || (! *optvec))  return  Options::ENDOPTS;

      // Try to match a known option
   OptionSpec optspec = match_opt(*(nextchar++), (optctrls & Options::ANYCASE));

      // Check for an unknown option
   if (optspec.isNULL()) {
      // See if this was a long-option in disguise
      if (! (optctrls & Options::NOGUESSING)) {
         unsigned  save_ctrls = optctrls;
         const char * save_nextchar = nextchar;
         nextchar -= 1;
         optctrls |= (Options::QUIET | Options::NOGUESSING);
         int  optchar = parse_longopt(iter, optarg);
         optctrls = save_ctrls;
         if (optchar > 0) {
            return  optchar;
         } else {
            nextchar = save_nextchar;
         }
      }
      if (! (optctrls & Options::QUIET)) {
         cerr << cmdname << ": unknown option -"
              << *(nextchar - 1) << "." << endl ;
      }
      optarg = (nextchar - 1);  // record the bad option in optarg
      return  Options::BADCHAR;
   }

      // If no argument is taken, then leave now
   if (optspec.isNoArg()) {
      optarg = NULLSTR;
      return  optspec.OptChar();
   }

      // Check for argument in this arg
   if (*nextchar) {
      optarg = nextchar; // the argument is right here
      nextchar = NULLSTR;   // we've exhausted this arg
      if (optspec.isList())  listopt = optspec ;  // save the list-spec
      return  optspec.OptChar();
   }

      // Check for argument in next arg
   const char * nextarg = iter.curr();
   if ((nextarg != NULL)  &&
       (optspec.isValRequired() || (! isOption(optctrls, nextarg)))) {
      optarg = nextarg;    // the argument is here
      iter.next();         // end of arg - advance
      if (optspec.isList())  listopt = optspec ;  // save the list-spec
      return  optspec.OptChar();
   }

     // No argument given - if its required, thats an error
   optarg = NULLSTR;
   if (optspec.isValRequired() &&  !(optctrls & Options::QUIET)) {
      cerr << cmdname << ": argument required for -" << optspec.OptChar()
           << " option." << endl ;
   }
   return  optspec.OptChar();
}

// ---------------------------------------------------------------------------
// ^FUNCTION: Options::parse_longopt - parse a long-option
//
// ^SYNOPSIS:
//    int Options::parse_longopt(iter, optarg)
//
// ^PARAMETERS:
//    OptIter & iter -- option iterator
//    const char * & optarg -- where to store any option-argument
//
// ^DESCRIPTION:
//    Parse the next long-option in iter (advancing as necessary).
//    Make sure we update the nextchar pointer along the way. Any option
//    we find should be returned and optarg should point to its argument.
//
// ^REQUIREMENTS:
//    - nextchar must point to the prospective option character
//
// ^SIDE-EFFECTS:
//    - iter is advanced when an argument completely parsed
//    - optarg is modified to point to any option argument
//    - if Options::QUIET is not set, error messages are printed on cerr
//
// ^RETURN-VALUE:
//    'c' if the the long-option corresponding to the -c option was matched
//         (optarg points to its argument)
//    BADKWD if the option is invalid (optarg points to the bad long-option
//                                                                     name).
//
// ^ALGORITHM:
//    It gets complicated -- follow the comments in the source.
// ^^-------------------------------------------------------------------------
inline int
Options::parse_longopt(OptIter & iter, const char * & optarg) {
   int  len = 0, ambiguous = 0;

   listopt = NULLSTR ;  // reset the list-spec

   if ((optvec == NULL) || (! *optvec))  return  Options::ENDOPTS;

      // if a value is supplied in this argv element, get it now
   const char * val = strpbrk(nextchar, ":=") ;
   if (val) {
      len = val - nextchar ;
      ++val ;
   }

      // Try to match a known long-option
   OptionSpec  optspec = match_longopt(nextchar, len, ambiguous);

      // Check for an unknown long-option
   if (optspec.isNULL()) {
      // See if this was a short-option in disguise
      if ((! ambiguous) && (! (optctrls & Options::NOGUESSING))) {
         unsigned  save_ctrls = optctrls;
         const char * save_nextchar = nextchar;
         optctrls |= (Options::QUIET | Options::NOGUESSING);
         int  optchar = parse_opt(iter, optarg);
         optctrls = save_ctrls;
         if (optchar > 0) {
            return  optchar;
         } else {
            nextchar = save_nextchar;
         }
      }
      if (! (optctrls & Options::QUIET)) {
         cerr << cmdname << ": " << ((ambiguous) ? "ambiguous" : "unknown")
              << " option "
              << ((optctrls & Options::LONG_ONLY) ? "-" : "--")
              << nextchar << "." << endl ;
      }
      optarg = nextchar;  // record the bad option in optarg
      nextchar = NULLSTR;    // we've exhausted this argument
      return  (ambiguous) ? Options::AMBIGUOUS : Options::BADKWD;
   }

      // If no argument is taken, then leave now
   if (optspec.isNoArg()) {
      if ((val) && ! (optctrls & Options::QUIET)) {
         cerr << cmdname << ": option "
              << ((optctrls & Options::LONG_ONLY) ? "-" : "--")
              << optspec.LongOpt() << " does NOT take an argument." << endl ;
      }
      optarg = val;     // record the unexpected argument
      nextchar = NULLSTR;  // we've exhausted this argument
      return  optspec.OptChar();
   }

      // Check for argument in this arg
   if (val) {
      optarg = val;      // the argument is right here
      nextchar = NULLSTR;   // we exhausted the rest of this arg
      if (optspec.isList())  listopt = optspec ;  // save the list-spec
      return  optspec.OptChar();
   }

      // Check for argument in next arg
   const char * nextarg = iter.curr();  // find the next argument to parse
   if ((nextarg != NULL)  &&
       (optspec.isValRequired() || (! isOption(optctrls, nextarg)))) {
      optarg = nextarg;        // the argument is right here
      iter.next();             // end of arg - advance
      nextchar = NULLSTR;         // we exhausted the rest of this arg
      if (optspec.isList())  listopt = optspec ;  // save the list-spec
      return  optspec.OptChar();
   }

     // No argument given - if its required, thats an error
   optarg = NULLSTR;
   if (optspec.isValRequired() &&  !(optctrls & Options::QUIET)) {
      const char * longopt = optspec.LongOpt();
      const char * spc = ::strchr(longopt, ' ');
      int  longopt_len;
      if (spc) {
         longopt_len = spc - longopt;
      } else {
         longopt_len = ::strlen(longopt);
      }
      cerr << cmdname << ": argument required for "
           << ((optctrls & Options::LONG_ONLY) ? "-" : "--");
      cerr.write(longopt, longopt_len) << " option." << endl ;
   }
   nextchar = NULLSTR;           // we exhausted the rest of this arg
   return  optspec.OptChar();
}

// ---------------------------------------------------------------------------
// ^FUNCTION: Options::usage - print usage
//
// ^SYNOPSIS:
//    void Options::usage(os, positionals)
//
// ^PARAMETERS:
//    ostream & os -- where to print the usage
//    char * positionals -- command-line syntax for any positional args
//
// ^DESCRIPTION:
//    Print command-usage (using either option or long-option syntax) on os.
//
// ^REQUIREMENTS:
//    os should correspond to an open output file.
//
// ^SIDE-EFFECTS:
//    Prints on os
//
// ^RETURN-VALUE:
//    None.
//
// ^ALGORITHM:
//    Print usage on os, wrapping long lines where necessary.
// ^^-------------------------------------------------------------------------
inline void
Options::usage(ostream & os, const char * positionals) const {
#ifdef NO_USAGE
   return;
#else
   const char * const * optv = optvec;
   unsigned  cols = 79;
   int  first, nloop;
   char  buf[256] ;

   if ((optv == NULL) || (! *optv))  return;

      // print first portion "usage: progname"
   os << "usage: " << cmdname ;
   unsigned  ll = strlen(cmdname) + 7;

      // save the current length so we know how much space to skip for
      // subsequent lines.
      //
   unsigned  margin = ll + 1;

      // print the options and the positional arguments
   for (nloop = 0, first = 1 ; !nloop ; optv++, first = 0) {
      unsigned  len;
      OptionSpec   optspec = *optv;

         // figure out how wide this parameter is (for printing)
      if (! *optv) {
         len = strlen(positionals);
         ++nloop;  // terminate this loop
      } else {
         if (optspec.isHiddenOpt())  continue;
         len = optspec.Format(buf, optctrls);
      }

      //  Will this fit?
      if ((ll + len + 1) > (cols - first)) {
         os << '\n' ;  // No - start a new line;
#ifdef USE_STDIO
         for (int _i_ = 0; _i_ < margin; ++_i_)  os << " ";
#else
         os.width(margin); os << "" ;
#endif
         ll = margin;
      } else {
         os << ' ' ;  // Yes - just throw in a space
         ++ll;
      }
      ll += len;
      os << ((nloop) ? positionals : buf) ;
   }// for each ad

   os << endl ;
#endif  /* NO_USAGE */
}


// ---------------------------------------------------------------------------
// ^FUNCTION: Options::operator() - get options from the command-line
//
// ^SYNOPSIS:
//   int Options::operator()(iter, optarg)
//
// ^PARAMETERS:
//    OptIter & iter -- option iterator
//    const char * & optarg -- where to store any option-argument
//
// ^DESCRIPTION:
//    Parse the next option in iter (advancing as necessary).
//    Make sure we update the nextchar pointer along the way. Any option
//    we find should be returned and optarg should point to its argument.
//
// ^REQUIREMENTS:
//    None.
//
// ^SIDE-EFFECTS:
//    - iter is advanced when an argument is completely parsed
//    - optarg is modified to point to any option argument
//    - if Options::QUIET is not set, error messages are printed on cerr
//
// ^RETURN-VALUE:
//     0 if all options have been parsed.
//    'c' if the the option or long-option corresponding to the -c option was
//         matched (optarg points to its argument).
//    BADCHAR if the option is invalid (optarg points to the bad option char).
//    BADKWD if the option is invalid (optarg points to the bad long-opt name).
//    AMBIGUOUS if an ambiguous keyword name was given (optarg points to the
//         ambiguous keyword name).
//    POSITIONAL if PARSE_POS was set and the current argument is a positional
//         parameter (in which case optarg points to the positional argument).
//
// ^ALGORITHM:
//    It gets complicated -- follow the comments in the source.
// ^^-------------------------------------------------------------------------
inline int
Options::operator()(OptIter & iter, const char * & optarg) {
   int parse_opts_only = isOptsOnly(optctrls);
   if (parse_opts_only)  explicit_end = 0;

      // See if we have an option left over from before ...
   if ((nextchar) && *nextchar) {
      return  parse_opt(iter, optarg);
   }

      // Check for end-of-options ...
   const char * arg = NULLSTR;
   int get_next_arg = 0;
   do {
      arg = iter.curr();
      get_next_arg = 0;
      if (arg == NULL) {
         listopt = NULLSTR;
         return  Options::ENDOPTS;
      } else if ((! explicit_end) && isEndOpts(arg)) {
         iter.next();   // advance past end-of-options arg
         listopt = NULLSTR;
         explicit_end = 1;
         if (parse_opts_only)  return  Options::ENDOPTS;
         get_next_arg = 1;  // make sure we look at the next argument.
      }
   } while (get_next_arg);

      // Do we have a positional arg?
   if ( explicit_end || (! isOption(optctrls, arg)) ) {
      if (parse_opts_only) {
         return  Options::ENDOPTS;
      } else {
         optarg = arg;  // set optarg to the positional argument
         iter.next();   // advance iterator to the next argument
         return  Options::POSITIONAL;
      }
   }

   iter.next();  // pass the argument that arg already points to

      // See if we have a long option ...
   if (! (optctrls & Options::SHORT_ONLY)) {
      if ((*arg == '-') && (arg[1] == '-')) {
         nextchar = arg + 2;
         return  parse_longopt(iter, optarg);
      } else if ((optctrls & Options::PLUS) && (*arg == '+')) {
         nextchar = arg + 1;
         return  parse_longopt(iter, optarg);
      }
   }
   if (*arg == '-') {
      nextchar = arg + 1;
      if (optctrls & Options::LONG_ONLY) {
         return  parse_longopt(iter, optarg);
      } else {
         return  parse_opt(iter, optarg);
      }
   }

      // If we get here - it is because we have a list value
   OptionSpec  optspec = listopt;
   optarg = arg ;        // record the list value
   return  optspec.OptChar() ;
}

#endif /* _options_h */
