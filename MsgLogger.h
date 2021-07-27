/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 *
 * cristian.martinez@univ-paris-est.fr (martinec)
 *
 */
/**
 * Unitex MsgLogger
 *
 * The purpose of this header is to ...
 *
 * Severity Levels :
 *
 *  A : Level number   ; B : Level alias    ; C : Level name
 *  D : Related system ; E : Default output ; E : Brief description
 *  > : Default level
 *
 *  A   B    C        D        E        F
 *  0   ++   trace    [unitex] [stdout] detailed trace of execution
 *  1   %%   debug    [unitex] [stdout] messages to debug application/plugins
 * >2   II   info     [unitex] [stdout] purely informational message
 *  3   !!   notice   [unitex] [stdout] normal but significant condition
 *  4   WW   warning  [unitex] [stdout] warning condition
 *  5   EE   error    [unitex] [stderr] error condition
 *  6   CC   critical [unitex] [stderr] critical condition
 *  7   ^^   alert    [system] [stderr] action must be taken immediately
 *  8   @@   panic    [system] [stderr] host system unusable condition
 *  9   --   none     [      ] [stdout] not a logging message
 *
 *  trace (0) is the lowest level and none (9) is the highest
 *
 *
 *  Configuration parameters
 *
 *  A : Parameter name ; B : Default value  ; C : Brief description
 *
 *  A               B       C
 *  print_level     false   prefix each message with level alias name
 *  redirect_output false   if output_file != stdout, redirect also to stdout
 *  redirect_error  true    if error_file  != stderr, redirect also to sterr
 *  severity_level  info    select severity level
 *  output_file     STDOUT  append output messages in output_file
 *  error_file      STDERR  append error messages in error_file
 *
 *  Silent logging
 *
 *  First alternative (recommended) :
 *
 *  --severity_level=none
 *
 *  Second alternative :
 *
 *  Unix-like               Windows-like
 *  --output_file=/dev/null --output_file=nul
 *  --error_file=/dev/null  --error_file=nul
 *
 *  Typical output :
 *
 *  The first field is the .The second field is the . The third field is the
 *  The fourth field is the . fifth,
 *
 *
 *  =External configuration macros=
 *  UNITEX_LIBRARY                     : disable ALI and export MsgLogger C API
 *
 *  =Config features macros=
 *  HAVE_BOOST_THREAD                  : boost thread library is installed
 *  HAVE_CLOCK_GETTIME                 : clock_gettime function is available
 *  HAVE_LOCALTIME_R                   : localtime_r function is available
 *  HAVE_STD_TR1                       : C++ Technical Report 1 libraries
 *  HAVE_SYSCALL                       : syscall function is available
 *  HAVE_LINUX_VERSION_H               : <linux/version.h> is available
 *  HAVE_SIGNAL_H                      : <signal.h> is available
 *
 *  =Compilation Configuration macros=
 *  UNITEX_EXPERIMENTAL_MSGLOGGER      : enable experimental message logger
 *  UNITEX_MANUAL_INIT       : disable automatic logger initialization
 *
 *  msglogger_config.h macros
 *
 *  Thread models
 *  UNITEX_USE_ISO_THREADS   : C++11 threads
 *  UNITEX_USE_BOOST_THREADS : BOOST threads
 *  UNITEX_USE_POSIX_THREADS : POSIX threads
 *  UNITEX_USE_WIN32_THREADS : Win32 threads
 *  UNITEX_SINGLE_THREADED   : disable threading support
 *
 *  UNITEX_SYSCALL_THREADID  : use syscall for threadid (Unix-like)
 *
 */
#ifndef UNITEX_H_  // NOLINT
#define UNITEX_H_  // NOLINT
/* ************************************************************************** */
#undef  UNITEX_EXPERIMENTAL_MSGLOGGER
#define UNITEX_EXPERIMENTAL_MSGLOGGER

#undef DEBUG
#define DEBUG 1
/* ************************************************************************** */
#if defined(UNITEX_EXPERIMENTAL_MSGLOGGER)
/* ************************************************************************** */
// UNITEX_LIBRARY compilation flag is passed when unitex dynamic library
// generation is requested
#if defined(UNITEX_LIBRARY)
# define UNITEX_BUILD_LIBRARY                        /* nothing */
// UNITEX_STATIC_LIBRARY compilation flag is passed when static library
// generation is requested
# if defined(UNITEX_STATIC_LIBRARY)
#  undef UNITEX_SHARED_LIB
# endif  // defined(UNITEX_STATIC_LIBRARY)
#endif // defined(UNITEX_LIBRARY)
/* ************************************************************************** */
#include "base/common.h"
#include "base/unilog/severity_level.h"
#include "base/thread/thread.h"
#include "base/boolean/boolean.h"
#include "base/bits/bits.h"
#include "base/string/strtok_r.h"
#include "base/file/path_separator.h"
/* ************************************************************************** */
//PLATFORM_API
//OpenGL
//QuickTime
//QT
//Tcl/Tk
//wxWidgets
//X11R6
/* ************************************************************************** */
// C system files                  (try to order the includes alphabetically)
#include <ctype.h>                 // isspace
#include <limits.h>                // CHAR_BIT
/* ************************************************************************** */
// Unitex .h files                 (try to order the includes alphabetically)
#include "File.h"                   // remove_path              // NOLINT
#include "Unicode.h"                // u_* functions            // NOLINT
#include "UnitexRevisionInfo.h"     // get_unitex_version       // NOLINT
#include "Ustring.h"                // u_* functions            // NOLINT
#include "UnitexString.h"           // UnitexString class       // NOLINT
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
#ifndef  HAS_UNITEX_NAMESPACE
# define HAS_UNITEX_NAMESPACE 1
#endif  // !defined(HAS_UNITEX_NAMESPACE)
/* ************************************************************************** */

/* ************************************************************************** */
/**
 * Resource Acquisition Is Initialization
 * Allows only one thread to use MsgLogger at a time
 */
class MsgLogGuard {
 public:
  // Typedefs and Enums
  // Constants (including static const data members)
  // Constructor
  MsgLogGuard() {
    unitex::mutex_lock(&mutex_);
  }

  // Destructor
  ~MsgLogGuard() {
    unitex::mutex_unlock(&mutex_);
  }
  // Methods, including static
  // Data Members (except static const data members)
 private:
  // Typedefs and Enums
  // Constants (including static const data members)
  // Constructors
  // Destructor
  // Methods, including static
  // Data Members (except static const data members)
  static unitex::mutex_t mutex_;

  /**
   * This class disallow implicit copy constructor and assignment
   */
  UNITEX_DISALLOW_COPY_AND_ASSIGN(MsgLogGuard);
};
/* ************************************************************************** */
namespace helper {  // helper
/* ************************************************************************** */
/**
 * Schwarz Counter (Nifty Counter) initialiser
 *
 * @see "The Annotated C++ Reference Manual", by Margaret A. Ellis and
 * Bjarne Stroustrup, Addison-Wesley Publishing Company, ISBN 0-201-51259-1,
 * Section 3.4, page 20-21.
 */
template<class T> class ObjectInitializer {
  public:
    /**
     *
     */
    ObjectInitializer() {
      if (!count_++) {
        // Note that we use init_once() and not init()
        T::init_once();
      }
    }

    /**
     *
     */
    ~ObjectInitializer() {
      if (!--count_) {
        T::shutdown();
      }
    }

  private:

    /**
     *
     */
    static uint32_t count_;

    /**
     * This class disallow implicit copy constructor and assignment
     */
    UNITEX_DISALLOW_COPY_AND_ASSIGN(ObjectInitializer);
};

template<class T> uint32_t ObjectInitializer<T>::count_ = 0;
/* ************************************************************************** */
}  // namespace helper
/* ************************************************************************** */
// forward declaration
// class MsgLogger;
/* ************************************************************************** */
//MsgLogColor::Black
//MsgLogColor::Red
//MsgLogColor::Green
//MsgLogColor::Yellow
//MsgLogColor::Blue
//MsgLogColor::Magenta
//MsgLogColor::Cyan
//MsgLogColor::White
//enum color
//{
//  COLOR_BLACK = 0,
//  COLOR_BLUE = 1,
//  COLOR_GREEN = 2,
//  COLOR_CYAN = 3,
//  COLOR_RED = 4,
//  COLOR_MAGENTA = 5,
//  COLOR_BROWN = 6,
//  COLOR_LIGHT_GREY = 7,
//  COLOR_DARK_GREY = 8,
//  COLOR_LIGHT_BLUE = 9,
//  COLOR_LIGHT_GREEN = 10,
//  COLOR_LIGHT_CYAN = 11,
//  COLOR_LIGHT_RED = 12,
//  COLOR_LIGHT_MAGENTA = 13,
//  COLOR_LIGHT_BROWN = 14,
//  COLOR_WHITE = 15
//};
// color foreg | backg << 4;
/* ************************************************************************** */

#if UNITEX_OS_IS(WINDOWS)
# if UNITEX_COMPILER_IS(MINGW)
// Let MINGW ignore X_OK flag and
// defines access() as __mingw_access()
#  define __USE_MINGW_ACCESS 1
# endif  // UNITEX_COMPILER_IS(MINGW)

# include <io.h>  // access()

// only if access isn't already defined
# if !defined(__USE_MINGW_ACCESS)
#  define access(file, mode) _access(file,mode)
# endif  // !defined(__USE_MINGW_ACCESS)

// R_OK
# if !defined(R_OK)
#  define R_OK  4  // Read-only
# endif  // !defined(R_OK)
// W_OK
# if !defined(W_OK)
#  define W_OK  2  // Write-only
# endif  // !defined(W_OK)
// X_OK
# if !defined(X_OK)
#  define X_OK  1  // Execute only
# endif  // !defined(X_OK)
// F_OK
# if !defined(F_OK)
#  define F_OK  0  // Existence only
# endif  // !defined(F_OK)
#endif  // UNITEX_OS_IS(WINDOWS)
/* ************************************************************************** */
namespace helper {
/* ************************************************************************** */
namespace file {
/* ************************************************************************** */
namespace {   // namespace helper::file::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/**
 * Test if a file exists
 */
UNITEX_FORCE_INLINE
UNITEX_PARAMS_NON_NULL
bool exists(const char* filename) {
  // we don't use unitex::fexists function declared in @see File.cpp
  // This is for two reasons :
  // (i)   fexists uses af_fopen logged call ! and even if not
  // (ii)  if a file exist and we don't have reading permissions,
  //       fexists returns false. This isn't the desired behavior.
  return (access(filename, F_OK) != -1) ? true : false;
}
/* ************************************************************************** */
}  // namespace helper::file::{unnamed}
/* ************************************************************************** */
}  // namespace helper::file
/* ************************************************************************** */
}  // namespace helper
/* ************************************************************************** */
struct MsgLogFile {
 public:
  // Typedefs and Enums
  typedef const VersatileEncodingConfig* VersatileEncodingConfigPtr;

  // Constants (static const data members)
  /**
   * Interface Files
   */
  static const MsgLogFile STDOUT;
  static const MsgLogFile STDERR;

  /**
   *
   */
  static const VersatileEncodingConfig kDefaultEncoding;  // = VEC_DEFAULT

  /**
   *
   */
  static const char kPathSeparator = UNITEX_FILE_PATH_SEPARATOR;

  // Constructors
  /**
   * Default constructor
   * filename and file are NULL
   * encoding is kDefaultEncoding
   */
  // TODO(martinec) append mode is actually unreachable outside of this class
  MsgLogFile()
   : name_(),
     file_(NULL),
     encoding_(&kDefaultEncoding),
     append_(true) {
   }

  /**
   * Explicit constructor from string
   */
  explicit MsgLogFile(const char* filename)
  : name_(),
    file_(NULL),
    encoding_(&kDefaultEncoding),
    append_(true) {
    *this = filename;
  }


  /**
   * Constructor from string and encoding
   */
  MsgLogFile(const char* filename,
             const VersatileEncodingConfigPtr& encoding)
  : name_(),
    file_(NULL),
    encoding_(encoding),
    append_(true) {
    *this = filename;
  }

  /**
   * Explicit copy constructor
   */
  explicit MsgLogFile(const MsgLogFile& param)
  : name_(),
    file_(param.file_),
    encoding_(param.encoding_),
    append_(param.append_) {
    strcpy(name_, param.name_);  // NOLINT
  }

  // Destructor

  /**
   * Destructor try to close an already opened file
   */
  ~MsgLogFile(){
    close();
  }

  // Operator overloads

  /**
   * Assignement operator from string
   */
  MsgLogFile& operator =(const char* filename) {
    // TODO(martinec) lock
    // first try to close the current file
    if ( close() ) {
      // then open a new file pointed by filename
      open(filename);
    }
    return *this;
  }

  // Overload some operators, note that this explicit allows comparisons
  // between different instances of this class

  /**
   * Equality operator over MsgLogFile
   */
  bool operator ==(const MsgLogFile& rhs) const {
    return file_ == rhs.file_;
  }

  /**
   * Inequality operator over MsgLogFile
   */
  bool operator !=(const MsgLogFile& rhs) const {
    return !(*this == rhs);
  }

  /**
   * Equality operator over U_FILE
   */
  bool operator ==(U_FILE* const file) const {
    return file_ == file;
  }

  /**
   * Inequality operator over U_FILE
   */
  bool operator !=(U_FILE* const file) const {
    return !(*this == file);
  }

#if UNITEX_COMPILER_COMPLIANT(CXX11)
  /**
   * C++11 support explicit type conversion operators
   * This is a ready to use version of the Safe Bool Idiom
   */
  explicit operator bool() const  {
    return (is_valid());
  }
#else
  /**
   * The next block is part of "The Safe Bool Idiom"
   * note that we allows comparisons between different
   * instances of this class. For more details please visit
   * @see http://www.artima.com/cppsource/safeboolP.html
   */
  private :
    typedef void (MsgLogFile::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}

  public :
    /**
     * Returns true if state_ is constant::True, false otherwise
     */
    operator bool_type() const {
     return is_valid() ?
       &MsgLogFile::this_type_does_not_support_comparisons : 0;
    }
#endif

  // Methods, including static methods
  /**
   *
   */
  UNITEX_PARAMS_NON_NULL
  static UnitexString basename(const char* filename) {
    char buffer[FILENAME_MAX];
    unitex::remove_path(filename,buffer);
    return UnitexString(buffer);
  }

  /**
   *
   */
  UNITEX_PARAMS_NON_NULL
  static bool exists(const char* filename) {
    return helper::file::exists(filename);
  }


  void write(const char* format,...) {
    //flush,message
  }

  /**
   * May not be accurate
   */
  off_t size() const {
   // TODO(martinec) lock(local)
   return 0; //size_;
  }

  /**
   *
   */
  bool is_stdout() const {
    return *this == MsgLogFile::STDOUT;
  }

  /**
   *
   */
  bool is_stderr() const {
    return *this == MsgLogFile::STDERR;
  }

  /**
   * Check if file is open
   */
  bool is_open() const {
    return file_ != NULL;
  }

  /**
   * Check if file is assigned
   */
  bool is_valid() const {
    return is_open();
  }

  // FileInfo get_file_info

    /**
     * flush file
     */
  //   void flush() {
     // TODO(martinec) lock(guard)
     // TODO(martinec) lock(local)
  //   if (file_ != NULL) {
  //       fflush(file_);
  //     }
  //   }

  /**
   * set file output encoding
   */
  void set_encoding(const VersatileEncodingConfigPtr& encoding) {
    encoding_ = encoding;
  }

  /**
   * Return a constant char pointer with the stringify name of the current
   * file
   */
  const char* name() const {
    return name_;
  }

  // Data Members (except static const data members)

 private:
  // Typedefs and Enums
  // Constants (static const data members)

  // Constructors

  /**
   * Setup file and filename
   * Only use in explicit copy constructor
   */
  MsgLogFile(U_FILE* const file, const char* name,
            const VersatileEncodingConfigPtr& encoding = &kDefaultEncoding)
  : name_(),
    file_(file),
    encoding_(encoding),
    append_(true) {
    strcpy(name_, name);  // NOLINT
  }

  // Destructor

  // Operator overloads

  // Methods, including static

  /**
   * Close a file that's already open
   */
  bool close(){
    // TODO(martinec) lock(local)
    bool success = true;
    // Only close a file that is already open
    if (is_open() && !is_stdout() && !is_stderr()) {
       // u_fclose returns 0 if successful
       success = unitex::u_fclose(file_) == 0 ? true : false ;
       file_   = NULL;
       *name_  = '\0';
    }
    return success;
  }

  /**
   * Open the file pointed by filename
   * current file descriptor needs to be null
   */
  void open(const char* filename) {
    // TODO(martinec) lock
    // Only open a new file when no other is open
    if(!is_open() || is_stdout() || is_stderr()){
      // try to open filename
      if (*filename != '\0' &&
          encoding_ != NULL) {
          file_ = unitex::u_fopen(encoding_, filename, U_APPEND);
      }

      // and save the filename
      if (file_ != NULL) {
          strcpy(name_, filename);  // NOLINT
      }
    }
  }

  // Data Members
  /**
   *
   */
  char name_[FILENAME_MAX];

  /**
   *
   */
  U_FILE* file_;

  /**
   *
   */
  VersatileEncodingConfigPtr encoding_;

  /**
   *
   */
  bool append_;

  /**
   * This struct disallow implicit assignment
   */
  void operator=(const MsgLogFile&);

};
/* ************************************************************************** */
struct MsgLogChannel {

};
/* ************************************************************************** */

/* ************************************************************************** */
struct MsgLogStats {
  // Typedefs and Enums
  // Constants (including static const data members)

  // Constructors
  /**
   *
   */
  MsgLogStats() : message_level_count_(),                  // zero initialized
                  timestamp_(unitex::Chrono::gettimeofday()) { // now() initialized
  }

  // Destructor
  /**
   *
   */
  ~MsgLogStats() {

  }

  // Methods, including static
  /**
   *
   */
  int64_t get_message_count(const unitex::SeverityLevel& level) const {
    // TODO(martinec) lock(guard)
    return message_level_count_[level.number()];
  }

  // Data Members (except static const data members)

 private:
  // Typedefs and Enums
  // Constants (including static const data members)

  // Constructors
  // Destructor
  // Methods, including static
  /**
   *
   */
//  void inc_message_count(const unitex::SeverityLevel& level) {
//    // TODO(martinec) lock(guard)
//    assert ( level.is_valid() );
//    ++message_level_count_[level.number()];
//  }

  // Data Members (except static const data members)

  /**
   *
   */
  int64_t message_level_count_[unitex::SeverityLevel::kNumberofLevels];

  /**
   *
   */
  unitex::Chrono timestamp_;

  /**
   *
   */
  friend class MsgLogger;

  /**
   * This class disallow implicit copy constructor and assignment
   * use "const MsgLogStats& stats" to pass as parameter
   */
  UNITEX_DISALLOW_COPY_AND_ASSIGN(MsgLogStats);
};
/* ************************************************************************** */
struct MsgLogFilter {
    //set_filter
    //level >= level_severity
};
/* ************************************************************************** */
namespace helper {  // helper
/* ************************************************************************** */

/* ************************************************************************** */
}  // namespace helper
/* ************************************************************************** */

/* ************************************************************************** */
// TODO(martinec) Prefix UINT64_C
/* ************************************************************************** */

/* ************************************************************************** */
struct MsgLogMessageDetails {
  public :
   // Typedefs and Enums

   /**
    * flag_t type, always unsigned
    */
   typedef uint32_t flag_t;

 private :
   // Constants (static const data members)
  /**
   * All n-bit of flag_t set 11111111...
   */
  static const flag_t kAllBitsSet = static_cast<flag_t>(-1);

  /**
   * Number of detail flags
   */
  static const size_t kNumberofFlags = 8;

  /**
   * Maximun flag value
   */
  static const flag_t kMaxFlagValue  = (1UL << kNumberofFlags) - 1;

 public  :
  // Constants (static const data members)

  /**
   * Underline MsgLogMessageDetails flags
   */
  struct constant {
    enum flag {
      None           = binary(00000000),
      ThreadName     = binary(00000001),   // flag 1
      SeverityAlias  = binary(00000010),   // flag 2
      SeverityName   = binary(00000100),   // flag 3
      TimeElapsed    = binary(00001000),   // flag 4
      TimeStamp      = binary(00010000),   // flag 5
      FileName       = binary(00100000),   // flag 6
      FileLine       = binary(01000000),   // flag 7
      UserMessage    = binary(10000000),   // flag 8
      All            = MsgLogMessageDetails::kAllBitsSet
    };
  };

  /**
   * Default format flags
   */
  static const flag_t kDefaultFlags = constant::UserMessage;

  /**
   * Interface Flags
   */
  static const MsgLogMessageDetails None;           //        00000000
  static const MsgLogMessageDetails ThreadName;     //        00000001
  static const MsgLogMessageDetails SeverityAlias;  //        00000010
  static const MsgLogMessageDetails SeverityName;   //        00000100
  static const MsgLogMessageDetails TimeElapsed;    //        00001000
  static const MsgLogMessageDetails TimeStamp;      //        00010000
  static const MsgLogMessageDetails FileName;       //        00100000
  static const MsgLogMessageDetails FileLine;       //        01000000
  static const MsgLogMessageDetails UserMessage;    //        10000000
  static const MsgLogMessageDetails All;            // ...111111111111

  /**
   *                          min  max  avg   format
   *  00010000 _threadname_   10   10   10    0xffffffff
   *  00000001 _xx_            2    2    2    CC
   *  00000010 _levelname_     4    8    7    CCCCCCC
   *  00000100 _timestamp_     15  15   15    hh:mm:ss.uuuuuu
   *  00001000 _elapsed_       1   16    -    uuuuuuuuuuuuuuuu
   *  00100000 _file_          5   32   15    cccccccccccccccccc
   *  01000000 _line_          4    4    4    0000
   *  10000000 _message_       -    -    -    cccccccccccccccccccccc ...
   */

  /**
   * Simple details : SeverityAlias + UserMessage
   * format : (_xx_) _message_
   * e.g    : (II) this is a message
   * binary : 10000010
   */
  static const flag_t  kFlagSimpleDetails  =  constant::SeverityAlias |
                                              constant::UserMessage;

  /**
   * More details   : SeverityAlias + UserMessage + TimeStamp
   * format : (_xx_) [_timestamp_] _message_
   * e.g    : (II) [16:09:55.100163] this is a message
   * binary : 10010010
   */
  static const flag_t  kFlagMoreDetails    =  kFlagSimpleDetails |
                                              constant::TimeStamp;

  /**
   * Extra details  : SeverityAlias + UserMessage + TimeStamp + ThreadName
   * format : ==_threadname_== (_xx_) [_timestamp_] _message_
   * e.g    : ==0x04370080== (II) [16:09:55.100163] this is a message
   * binary : 10010011
   */
  static const flag_t  kFlagExtraDetails   =  kFlagMoreDetails |
                                              constant::ThreadName;


  /**
   * Full details   : SeverityAlias + UserMessage + TimeStamp + ThreadName +
   *                  FileName      + FileLine
   * format : ==_threadname_== (_xx_) [_timestamp_ _file_:_line_] _message_
   * e.g    : ==0x04370080== (II) [16:09:55.100163 Locate.cpp:0611] a message
   * binary : 11110011
   */
  static const flag_t  kFlagFullDetails   =  kFlagExtraDetails   |
                                             constant::FileName  |
                                             constant::FileLine;
  // Constructors

  /**
   * Constructor without parameters
   * MsgLogMessageDetails is UserMessage (10000000) by default
   */
  MsgLogMessageDetails()
    :    flags_(kDefaultFlags) {
    }

  /**
   * Constructor from flag_t integer
   * Note that this disallows implicit copy construction
   */
  explicit MsgLogMessageDetails(flag_t flag)
      :   flags_((flag > kMaxFlagValue) ? constant::None : flag) {
  }

  /**
   * Assignement operator from MsgLogMessageDetails
   */
  MsgLogMessageDetails& operator=(const MsgLogMessageDetails& rhs) {
    // TODO(martinec) lock
    if (this != &rhs) {
      flags_ = rhs.flags_;
    }
    return *this;
  }

  /**
   * Assignement operator from flag_t
   * Valid input : {kFlagSimpleDetails, kFlagMoreDetails, kFlagExtraDetails ...}
   */
  MsgLogMessageDetails& operator=(flag_t flag) {
    flags_ = (flag > kMaxFlagValue) ? constant::None : flag;
    return *this;
  }

  /**
   * Assignement operator from string
   * Valid input : {"none", "nn", "severity_alias", "sa", ... , "panic"}
   */
  MsgLogMessageDetails& operator=(const char* flags) {
    flags_ = string_to_flags(flags);

    return *this;
  }

  /**
   * Specifies a conversion from MsgLogMessageDetails to the underline flag
   * using a conversion-type-id function
   */
  /*UNITEX_EXPLICIT_CONVERSIONS*/
  operator flag_t () const {
    return value();
  }

  // Overload some operators, note that this explicity allows comparisons
  // between different instances of same class

  /**
   * Equality operator over MsgLogMessageDetails
   */
  bool operator ==(const MsgLogMessageDetails& rhs) const {
    return flags_ == rhs.flags_;
  }

  /**
   * Inequality operator over MsgLogMessageDetails
   */
  bool operator !=(const MsgLogMessageDetails& rhs) const {
    return !this->operator==(rhs);
  }


  /**
   * Less than operator over MsgLogMessageDetails
   */
  bool operator <(const MsgLogMessageDetails& rhs) const {
    return flags_ < rhs.flags_;
  }

  /**
   * Greater than operator over MsgLogMessageDetails
   */
  bool operator >(const MsgLogMessageDetails& rhs) const {
    return rhs.operator<(*this);
  }

  /**
   * Less than or equal to operator over MsgLogMessageDetails
   */
  bool operator <=(const MsgLogMessageDetails& rhs) const {
    return !rhs.operator<(*this);
  }

  /**
   * Greater than or equal to operator over MsgLogMessageDetails
   */
  bool operator >=(const MsgLogMessageDetails& rhs) const {
    return !this->operator<(rhs);
  }

  // on self

  MsgLogMessageDetails& operator |= (const MsgLogMessageDetails& rhs) {
    flags_ |= rhs.flags_;
    return *this;
  }

  MsgLogMessageDetails& operator &= (const MsgLogMessageDetails& rhs) {
    flags_ &= rhs.flags_;
    return *this;
  }

  MsgLogMessageDetails& operator -= (const MsgLogMessageDetails& rhs) {
    flags_ -= rhs.flags_;
    return *this;
  }

  MsgLogMessageDetails&  operator ^= (const MsgLogMessageDetails& rhs) {
   flags_ ^= rhs.flags_;
   return *this;
  }

  MsgLogMessageDetails&  operator ~() {
   flags_ = ~flags_;
   return *this;
  }

#if UNITEX_COMPILER_COMPLIANT(CXX11)
  /**
   * C++11 support explicit type conversion operators
   * This is a ready to use version of the Safe Bool Idiom
   */
  explicit operator bool() const  {
    return (is_any());
  }
#else
  /**
   * The next block is part of "The Safe Bool Idiom"
   * note that we allows comparisons between different
   * instances of this class. For more details please visit
   * @see http://www.artima.com/cppsource/safeboolP.html
   */
  private :
    typedef void (MsgLogMessageDetails::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}

  public :
    /**
     * Returns true if state_ is constant::True, false otherwise
     */
    operator bool_type() const {
      return is_any() ?
        &MsgLogMessageDetails::this_type_does_not_support_comparisons : 0;
    }
#endif

  // Methods, including static
  /**
   * Test if all bits are unset
   * @return true if flags_ is  beetween 0 and kNumberofFlagss - 1
   */
  bool is_none() const {
    return flags_ == constant::None;
  }

  /**
   * Test if at least 1 bit is set
   */
  bool is_any() const {
    return !is_none();
  }

  /**
   * Test if all bits are set
   */
  bool is_all() const {
    return flags_ == constant::All;
  }

  /**
   * Clear all
   */
  MsgLogMessageDetails&  reset() {
    flags_ = constant::None;
    return *this;
  }

  /**
   * Set all
   */
  MsgLogMessageDetails& set() {
    flags_ = constant::All;
    return *this;
  }


  /**
   *
   */
  MsgLogMessageDetails& set(const MsgLogMessageDetails& format) {
    flags_ |= format.flags_;
    return *this;
  }

  /**
   *
   */
  MsgLogMessageDetails& clear(const MsgLogMessageDetails& format) {
    flags_ &= ~format.flags_;
    return *this;
  }


  /**
   *
   */
  bool  test(const MsgLogMessageDetails& format) const {
    return flags_ & format.flags_;
  }

  /**
   * Return a numeric bitset representing the current flag
   */
  flag_t value() const {
    return flags_;
  }

  /**
   *
   */
  static size_t size() {
    return kFlagSizeInBits;
  }

  /**
   *
   * @return
   */
  size_t count() const {
    return unitex::util::popcount(flags_);
  }

  // some friends in unnamed namespace

  //
  friend bool operator == (const MsgLogMessageDetails& lhs,
                           const MsgLogMessageDetails& rhs) {
    return MsgLogMessageDetails(lhs).operator ==(rhs);
  }

  //
  friend bool operator != (const MsgLogMessageDetails& lhs,
                           const MsgLogMessageDetails& rhs) {
    return MsgLogMessageDetails(lhs).operator !=(rhs);
  }

  //
  friend bool operator <  (const MsgLogMessageDetails& lhs,
                           const MsgLogMessageDetails& rhs) {
    return MsgLogMessageDetails(lhs).operator <(rhs);
  }

  //
  friend bool operator <= (const MsgLogMessageDetails& lhs,
                           const MsgLogMessageDetails& rhs) {
    return MsgLogMessageDetails(lhs).operator <=(rhs);
  }

  //
  friend bool operator >  (const MsgLogMessageDetails& lhs,
                           const MsgLogMessageDetails& rhs) {
    return MsgLogMessageDetails(lhs).operator >(rhs);
  }

  //
  friend bool operator >= (const MsgLogMessageDetails& lhs,
                           const MsgLogMessageDetails& rhs) {
    return MsgLogMessageDetails(lhs).operator >=(rhs);
  }

  //
  friend MsgLogMessageDetails& operator |(const MsgLogMessageDetails& lhs,
                                  const MsgLogMessageDetails& rhs) {
    return MsgLogMessageDetails(lhs) |= rhs;
  }

  //
  friend MsgLogMessageDetails& operator &(const MsgLogMessageDetails& lhs,
                                  const MsgLogMessageDetails& rhs) {
    return MsgLogMessageDetails(lhs) &= rhs;
  }

  //
  friend MsgLogMessageDetails& operator -(const MsgLogMessageDetails& lhs,
                                  const MsgLogMessageDetails& rhs) {
    return MsgLogMessageDetails(lhs) -= rhs;
  }

  //
  friend MsgLogMessageDetails& operator ^(const MsgLogMessageDetails& lhs,
                                  const MsgLogMessageDetails& rhs) {
    return MsgLogMessageDetails(lhs) ^= rhs;
  }


 private :
  // Typedefs and Enums
  /**
   * Name encapsulation for format flags
   */
#define Declare__MsgLogFormat__(MsgLogFormatName)           \
  struct MsgLogFormatName  {                                \
    static const char* name;                                \
    static const char* alias;                               \
    static const constant::flag value  = \
                 constant::MsgLogFormatName; \
  }

  Declare__MsgLogFormat__(None);
  Declare__MsgLogFormat__(ThreadName);
  Declare__MsgLogFormat__(SeverityAlias);
  Declare__MsgLogFormat__(SeverityName);
  Declare__MsgLogFormat__(TimeElapsed);
  Declare__MsgLogFormat__(TimeStamp);
  Declare__MsgLogFormat__(FileName);
  Declare__MsgLogFormat__(FileLine);
  Declare__MsgLogFormat__(UserMessage);
  Declare__MsgLogFormat__(All);

#undef Declare__MsgLogFormat__ /* undef */

  // Constants (static const data members)

  /**
  * Numbers of bits of flag_t data type
  */
  static const size_t kFlagSizeInBits = unitex::util::size_in_bits<flag_t>::value;

  /**
  * Numbers of bits of flag_t data type minus 1
  * This constant is used in mask(index) function
  */
  static const size_t kFlagSizeMinusOne =
                      kFlagSizeInBits - static_cast<size_t>(1);

  // Constructors

  /**
   * Explicit constructor from log flag
   */
  explicit MsgLogMessageDetails(constant::flag flag) :
      flags_(flag) {}

  // Methods, including static
  /**
   * Return a flag_t from a stringify combination of name or alias attributes
   * separated by a "+" character
   * e.g. "um+sa"                           == UserMessage | SeverityAlias
   * e.g. "user_message + severity_alias"   == UserMessage | SeverityAlias
   */
  UNITEX_PARAMS_NON_NULL
  static flag_t string_to_flags(const char* flags) {
    // stores the final flag
    flag_t result = constant::None;

    if(flags != NULL) {
      // flags_buffer will store a copy of flags
      size_t flags_buffer_length = 1 + strlen(flags);
      char*  flags_buffer = static_cast<char*>(malloc(flags_buffer_length));

      // do a copy of input flags parameter into flags_buffer
      if(flags_buffer) {
        memcpy(flags_buffer, flags, flags_buffer_length);
      } else {
        return constant::None;
      }

      char* flag_token;
      char* end_iterator;
      char* remainder;

      const char separator[2] = "+";

      // parse flags_buffer
      flag_token = strtok_r(flags_buffer, separator, &remainder);

      // until there aren't more tokens to parsing
      while(flag_token != NULL) {
        // left trim
        while(isspace(*flag_token)) {
          ++flag_token;
        }

        // flag is more than only left spaces
        if(*flag_token != '\0') {
          // end iterator pointed at the end of flag_token
          end_iterator = flag_token + strlen(flag_token) - 1;

          // right trim
          while(end_iterator > flag_token && isspace(*end_iterator)) {
            --end_iterator;
          }

          // mark the end of the string
          *(end_iterator+1) = '\0';

          // try to convert flag_token in a numeric constant flag and then
          // combine it with result
          result |= string_to_flag(flag_token);
        }

        // parse the next token
        flag_token = strtok_r(NULL, separator, &remainder);
      }

      free(flags_buffer);
    }

    return result;
  }


  /**
   * Return a constant::flag from the stringify name or alias
   * passed as argument
   */
  static constant::flag string_to_flag(const char* f) {
    //  "Premature optimization is the root of all evil
    //  (or at least most of it) in programming" --Donald Knuth
    if (f) {
      if (strcmp(f, None::name)           == 0) return constant::None;
      if (strcmp(f, None::alias)          == 0) return constant::None;
      if (strcmp(f, ThreadName::name)     == 0) return constant::ThreadName;
      if (strcmp(f, ThreadName::alias)    == 0) return constant::ThreadName;
      if (strcmp(f, SeverityAlias::name)  == 0) return constant::SeverityAlias;
      if (strcmp(f, SeverityAlias::alias) == 0) return constant::SeverityAlias;
      if (strcmp(f, SeverityName::name)   == 0) return constant::SeverityName;
      if (strcmp(f, SeverityName::alias)  == 0) return constant::SeverityName;
      if (strcmp(f, TimeElapsed::name)    == 0) return constant::TimeElapsed;
      if (strcmp(f, TimeElapsed::alias)   == 0) return constant::TimeElapsed;
      if (strcmp(f, TimeStamp::name)      == 0) return constant::TimeStamp;
      if (strcmp(f, TimeStamp::alias)     == 0) return constant::TimeStamp;
      if (strcmp(f, FileName::name)       == 0) return constant::FileName;
      if (strcmp(f, FileName::alias)      == 0) return constant::FileName;
      if (strcmp(f, FileLine::name)       == 0) return constant::FileLine;
      if (strcmp(f, FileLine::alias)      == 0) return constant::FileLine;
      if (strcmp(f, UserMessage::name)    == 0) return constant::UserMessage;
      if (strcmp(f, UserMessage::alias)   == 0) return constant::UserMessage;
      if (strcmp(f, All::name)            == 0) return constant::All;
      if (strcmp(f, All::alias)           == 0) return constant::All;
    }
    return constant::None;
  }

  /**
   * Create a mask
   */
  static flag_t mask(size_t index) {
    // FIXME(martinec) 1U literal depends of flag_t size
    return UINT32_C(1) << (index & kFlagSizeMinusOne);
  }

  // Data Members

  /**
   * underline raw flag
   */
  flag_t flags_;

  /**
   * This class disallow implicit copy constructor
   */
  MsgLogMessageDetails(const MsgLogMessageDetails&);
};
/* ************************************************************************** */
struct MsgLogParams {
  // Typedefs and Enums
  // Constants (including static const data members)
  // Constructors
  /**
   *
   */
  MsgLogParams() :    append_header(false),  // don't print info header
                       append_stats(false),  // don't print footer stats
                    redirect_output(false),  // don't redirect to stdout
                      redirect_error(true),  // redirect errors to stderr
                     colorful_output(true),  // show colorful console output
                          severity_level(),  // kDefaultLevel by default
                         message_details(),  // kDefaultFlags by default
                                    file()   // logging file isn't set
  {
  }

  // Destructor
  /**
   * Destructor does nothing
   */
  ~MsgLogParams(){

  }
  // Methods, including static
  // Data Members (except static const data members)

  /**
   *
   */
  unitex::Bool append_header;


  /**
   *
   */
  unitex::Bool append_stats;

  /**
   *
   */
  unitex::Bool redirect_output;

  /**
   *
   */
  unitex::Bool redirect_error;

  /**
   *
   */
  unitex::Bool colorful_output;

  /**
   *
   */
  unitex::SeverityLevel severity_level;

  /**
   *
   */
  MsgLogMessageDetails message_details;

  /**
   *
   */
  MsgLogFile file;

 private:

  // Typedefs and Enums
  // Constants (including static const data members)
  // Constructors
  // Destructor
  // Methods, including static
  // Data Members (except static const data members)

  /**
   *
   */
  friend class MsgLogger;

  /**
   *
   */
  friend class MsgLogMessage;

  /**
   * This class disallow implicit copy constructor and assignment
   * use "const MsgLogParams& params" to pass as parameter
   */
  UNITEX_DISALLOW_COPY_AND_ASSIGN(MsgLogParams);
};
/* ************************************************************************** */
//struct MsgLogTargetSettings {
//  // Typedefs and Enums
//  // Constants (including static const data members)
//  // Constructors
//  /**
//   *
//   */
//  MsgLogTargetSettings() :
//       append_header(false),  // don't print header
//       append_footer(false),  // don't print footer
//       append_message(true),  // print message
//       message_details(),     // kDefaultFlags by default
//       header_details(),      // kDefault...
//       footer_details(),      // kDefault...
//  {
//  }
//
//  // Destructor
//  /**
//   * Destructor does nothing
//   */
//  ~MsgLogTargetSettings(){
//
//  }
//  // Methods, including static
//  // Data Members (except static const data members)
//
//  /**
//   *
//   */
//  unitex::Bool append_header;
//
//
//  /**
//   *
//   */
//  unitex::Bool append_stats;
//
//  /**
//   *
//   */
//  unitex::SeverityLevel severity_level;
//
//  /**
//   *
//   */
//  MsgLogMessageDetails message_details;
//
//
// private:
//
//  // Typedefs and Enums
//  // Constants (including static const data members)
//  // Constructors
//  // Destructor
//  // Methods, including static
//  // Data Members (except static const data members)
//
//  /**
//   *
//   */
//  friend class MsgLogger;
//
//  /**
//   *
//   */
//  friend class MsgLogMessage;
//
//  /**
//   * This class disallow implicit copy constructor and assignment
//   * use "const MsgLogTargetSettings& params" to pass as parameter
//   */
//  MsgLogTargetSettings(const MsgLogTargetSettings&);
//  void operator=(const MsgLogTargetSettings&);
//};
/* ************************************************************************** */
namespace info  {
/* ************************************************************************** */
namespace process {  // namespace info::process
/* ************************************************************************** */
namespace {  // namespace info::process::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
/**
 * Stringize thread_name
 * @return UnitexString
 */
UNITEX_FORCE_INLINE
UnitexString thread_name() {
  unitex::thread_name_t thread_name = {/* zero initialized */};
  unitex::get_current_thread_name(thread_name);
  return UnitexString(thread_name);
}

// thread_identifier
// therad_model
// therad_model_identifier info.processs.thread.model:single
// process_name
// process_identifier info.process.xxxx
//

/* ************************************************************************** */
}  // namespace info::process::{unnamed}
/* ************************************************************************** */
}  // namespace info::process
/* ************************************************************************** */
// The target platform is the operating system where this program is supposed
// to run
namespace target {  // namespace info::target
/* ************************************************************************** */
namespace {  // namespace info::target::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
#if   UNITEX_OS_IS(CYGWIN)

UNITEX_FORCE_INLINE
UnitexString operating_system_release_name() {
  return UnitexString("TODO");
}

UNITEX_FORCE_INLINE
int32_t operating_system_release_version() {
  return -1;
}

#elif UNITEX_OS_IS(UNIX)

UnitexString operating_system_release_name() {
  return UnitexString("TODO");
}

UNITEX_FORCE_INLINE
int32_t operating_system_release_version() {
  return -1;
}

#elif UNITEX_OS_IS(WINDOWS)

UnitexString operating_system_release_name() {
  return UnitexString("TODO");
}

UNITEX_FORCE_INLINE
int32_t operating_system_release_version() {
  return -1;
}

#endif  // UNITEX_OS_IS(CYGWIN)

/**
 * Gets the name of the target operating system (compile time value)
 */
UNITEX_FORCE_INLINE
UnitexString operating_system_name() {
  return UnitexString(UNITEX_OS_NAME);
}

/**
 *
 */
UNITEX_FORCE_INLINE
UnitexString operating_system_identifier() {
  return UnitexString::format("target.os.%S.%S:%09d",
                              operating_system_name().c_unichar(),
                              operating_system_release_name().c_unichar(),
                              operating_system_release_version());
}


// @see http://en.wikipedia.org/wiki/Android_version_history
//  Android Version | __ANDROID_API__
//  1.0             | 1
//  1.1             | 2
//  1.5             | 3
//  1.6             | 4
//  2.0             | 5
//  2.0.1           | 6
//  2.1             | 7
//  2.2             | 8
//  2.3             | 9
//  2.3.3           | 10
//  3.0             | 11
//  3.1             | 12
//  3.2             | 13
//  4.0             | 14
//  4.0.3           | 15
//  4.1             | 16
//  4.2             | 17
//  4.3             | 18
//  4.4             | 19
/* ************************************************************************** */
}  // namespace info::target::{unnamed}
/* ************************************************************************** */
}  // namespace info::target
/* ************************************************************************** */
// The build platform is the operating system where code will be built
namespace build {  // namespace info::build
/* ************************************************************************** */
namespace {  // namespace info::build::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
/**
 * Get compiler name
 */
UNITEX_FORCE_INLINE
const char* compiler_name() {
  //return compiler_name_version_table[0].name;
  return UNITEX_COMPILER_NAME;
}

/**
 * Get compiler version as integer [VVRRPPPPP]
 */
UNITEX_FORCE_INLINE
int32_t compiler_version() {
  return UNITEX_COMPILER_VERSION;
}


/**
 * Get compiler string identifier as name:version
 */
UNITEX_FORCE_INLINE
UnitexString compiler_identifier() {
  return UnitexString::format("build.compiler.%s:%09d",
                               compiler_name(),
                               compiler_version());
}

// system_api_version

// cpu_name
// cpu_version

/**
 * Get unitex version number
 * version minor revision [vvmmrrrrr]
 */
UNITEX_FORCE_INLINE
int32_t unitex_version() {
  // major and minor version number
  uint32_t major_version_number =  0u;
  uint32_t minor_version_number =  0u;
  get_unitex_version(&major_version_number, &minor_version_number);

  // revision number
  int32_t revision_number = get_unitex_revision();

  // if revision_number is -1 (i.e an anonymous build without SVN revision number)
  // set revison_number to 0
  revision_number         = revision_number < 0 ? 0 : revision_number;

  // build an integer version number (major minor revision) [vvmmrrrrr]
  int32_t unitex_version  =  (major_version_number * 10000000  \
                            + minor_version_number * 100000   \
                            + revision_number);

  return unitex_version;
}


/**
 *  Get unitex name
 */
UNITEX_FORCE_INLINE
const char* unitex_name() {
  int version = unitex_version();
  if(version < 30000000){
     return "unitex";
  }
  return    "unitex_gramlab";
}


/**
 * Get compiler string identifier as name:version
 */
UNITEX_FORCE_INLINE
UnitexString unitex_identifier() {
  return UnitexString::format("build.%s:%09d",
                               unitex_name(),
                               unitex_version());
}

/**
 * Gets msglogger version
 * version minor revision [vvmmrrrrr]
 */
UNITEX_FORCE_INLINE
int32_t msglogger_version() {
  // major and minor version number
  int32_t major_version_number =  0;
  int32_t minor_version_number =  0;
  int32_t revision_number      =  0;

  // build an integer version minor revision [vvmmrrrrr]
  int32_t msglogger_version =  (major_version_number * 10000000  \
                              + minor_version_number * 100000    \
                              + revision_number);
  return msglogger_version;
}

UNITEX_FORCE_INLINE
UnitexString msglogger_identifier() {
  return UnitexString::format("build.%s.msglogger:%09d",
                               UNITEX_BUILD_MODE_STRING,
                               msglogger_version());
}
/* ************************************************************************** */
}  // namespace info::build::{unnamed}
/* ************************************************************************** */
}  // namespace info::build
/* ************************************************************************** */
// The host platform is user current operating system
// i.e this program is builded to run on HOST
namespace host {  // namespace info::host
/* ************************************************************************** */
#if   UNITEX_OS_IS(CYGWIN)

#elif UNITEX_OS_IS(UNIX)
# include <sys/utsname.h>          // uname()
#elif UNITEX_OS_IS(WINDOWS)
# include <tchar.h>  // _T()
#endif  // UNITEX_OS_IS(CYGWIN)
/* ************************************************************************** */
#if UNITEX_OS_IS(UNIX)
/* ************************************************************************** */
namespace os_linux {  // namespace info::host::os_linux
/* ************************************************************************** */
namespace os_release {  //  namespace info::host::os_linux::os_release
/* ************************************************************************** */
// Notice that all definitions are const pointer and hence have internal
// linkage
const char* const kFileName       = "/etc/os-release";
/* ************************************************************************** */
namespace symbol {  // namespace info::host::os_linux::os_release::symbol
/* ************************************************************************** */
const char kNewLine              = 0x0A; // New line char
const char kEqual                = '=';  // Key value separator char
const char kDoubleQuote          = '\"'; // Double quoted values char
const char kSingleQuote          = '\''; // Single quoted values char
const char kComment              = '#';  // Comment char
const char kEOS                  = '\0'; // End of string char
const char kEscape               = '\\'; // Escape char for double or
                                         // single quotes or backslashes
/* ************************************************************************** */
}  // namespace info::host::os_linux::os_release::symbol
/* ************************************************************************** */
namespace key {  // namespace info::host::os_linux::os_release::key
/* ************************************************************************** */
const char* const NAME           = "NAME";
const char* const VERSION        = "VERSION";
const char* const ID             = "ID";
const char* const ID_LIKE        = "ID_LIKE";
const char* const VERSION_ID     = "VERSION_ID";
const char* const PRETTY_NAME    = "PRETTY_NAME";
const char* const ANSI_COLOR     = "ANSI_COLOR";
const char* const CPE_NAME       = "CPE_NAME";
const char* const HOME_URL       = "HOME_URL";
const char* const SUPPORT_URL    = "SUPPORT_URL";
const char* const BUG_REPORT_URL = "BUG_REPORT_URL";
const char* const BUILD_ID       = "BUILD_ID";
/* ************************************************************************** */
}  // namespace info::host::os_linux::os_release::key
/* ************************************************************************** */
  // at files scope this is the same that static const struct
  const struct /* file_release_table_t */ {
    const char* file_name;
    const char* release_name;
  } file_release_table[] = {
    // InXi
    // do not change the definition order
    {"/etc/antix-version",        "antix"                 },
    {"/etc/aptosid-version",      "aptosid"               },
    {"/etc/kanotix-version",      "kanotix"               },
    {"/etc/knoppix-version",      "knoppix"               },
    {"/etc/mandrake-release",     "mandrake"              },
    {"/etc/pardus-release",       "pardus"                },
    {"/etc/porteus-version",      "porteus"               },
    {"/etc/sabayon-release",      "sabayon"               },
    {"/etc/siduction-version",    "siduction"             },
    {"/etc/sidux-version",        "sidux"                 },
    {"/etc/slitaz-release",       "slitaz"                },
    {"/etc/solusos-release",      "solusos"               },
    {"/etc/turbolinux-release",   "turbolinux"            },
    {"/etc/zenwalk-version",      "zenwalk"               },
    // this list was partially build from
    // @see http://linuxmafia.com/faq/Admin/release-files.html
    {"/etc/debian_version",       "debian"                },
    {"/etc/debian_release",       "debian"                },
    {"/etc/arch-release",         "arch"                  },
    {"/etc/gentoo-release",       "gentoo"                },
    {"/etc/SuSE-release",         "suse"                  },
    {"/etc/sles-release",         "sles"                  },
    {"/etc/slackware-release",    "slackware"             },
    {"/etc/slackware-version",    "slackware"             },
    {"/etc/fedora-release",       "fedora"                },
    {"/etc/altlinux-release",     "altlinux"              },
    {"/etc/angstrom-version",     "angstrom"              },
    {"/etc/frugalware-release",   "frugalware"            },
    {"/etc/oracle-release",       "oracle_linux"          },
    {"/etc/vmware-release",       "vmware"                },
    {"/etc/mageia-release",       "mageia"                },
    {"/etc/mandriva-release",     "mandriva"              },
    {"/etc/meego-release",        "meego"                 },
    {"/etc/yellowdog-release"     "yellowdog"             },
    {"/etc/UnitedLinux-release",  "unitedlinux"           },
    {"/etc/annvix-release",       "annvix"                },
    {"/etc/arklinux-release",     "arklinux"              },
    {"/etc/aurox-release",        "aurox"                 },
    {"/etc/blackcat-release",     "blackcat"              },
    {"/etc/cobalt-release",       "cobalt"                },
    {"/etc/conectiva-release",    "conectiva"             },
    {"/etc/eos-version",          "freeeos"               },
    {"/etc/hlfs-release",         "hlfs"                  },
    {"/etc/hlfs_version",         "hlfs"                  },
    {"/etc/immunix-release",      "immunix"               },
    {"/etc/lfs_version",          "linux_from_scratch"    },
    {"/etc/lfs-release",          "linux_from_scratch"    },
    {"/etc/mklinux-release",      "mklinux"               },
    {"/etc/nld-release",          "novell_linux_desktop"  },
    {"/etc/pld-release",          "pld_linux"             },
    {"/etc/rubix-version",        "rubix"                 },
    {"/etc/e-smith-release",      "sme_server"            },
    {"/etc/synoinfo.conf",        "synology"              },
    {"/etc/tinysofa-release",     "tinysofa"              },
    {"/etc/trustix-release",      "trustix"               },
    {"/etc/trustix-version",      "trustix"               },
    {"/etc/ultrapenguin-release", "ultrapenguin"          },
    {"/etc/va-release",           "va_linux"              },
    {"/etc/redhat-release",       "redhat"                },
    {"/etc/redhat_version",       "redhat"                },
    {"/etc/lsb-release",          "ubuntu"                },
    {NULL,                        NULL                    }
  };  // file_release_table[]
}  // namespace info::host::os_linux::os_release
/* ************************************************************************** */
namespace {  // namespace info::host::os_linux::{unnamed}, to enforce the ODR
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
/**
 *
 * Alternative os_release name get method used when /etc/os-release file
 * doesn't exists
 */
UNITEX_FORCE_INLINE
UnitexString os_release_from_file() {
  int32_t i = 0;
  while(os_release::file_release_table[i].file_name != NULL) {
    if(MsgLogFile::exists(os_release::file_release_table[i].file_name)) {
      return UnitexString(os_release::file_release_table[i].release_name);
    }
    ++i;
  }
  return UnitexString();
}

/**
 * Minimal KEY="value" parser for /etc/os-release
 * os-release is a newline-separated list of environment-like
 * shell-compatible variable assignments.
 * @see http://www.freedesktop.org/software/systemd/man/os-release.html
 * @return
 */
// List of keys to parse in /etc/os-release :
// os_release::key::NAME           OS name
// os_release::key::VERSION        OS version
// os_release::key::ID             OS lower-case string identifier
// os_release::key::ID_LIKE        OS closely related to the local os
// os_release::key::VERSION_ID     OS lower-case string version identifier
// os_release::key::PRETTY_NAME    OS pretty string name
// os_release::key::ANSI_COLOR     OS name  suggested presentation color
// os_release::key::CPE_NAME       OS Common Platform Enumeration Specificator
// os_release::key::HOME_URL       OS homepage
// os_release::key::SUPPORT_URL    OS main support page
// os_release::key::BUG_REPORT_URL OS main bug reporting page
// os_release::key::BUILD_ID       OS version build unique identifier
UNITEX_FORCE_INLINE
UNITEX_PARAMS_NON_NULL
UnitexString os_release_get_value(const char* const key) {
  ABSTRACTFILE* file_descriptor = NULL;
  file_descriptor = af_fopen_unlogged(os_linux::os_release::kFileName, "rb");

  // if open fails
  if (!file_descriptor) {
    return UnitexString();
  }

  static const size_t  kMaxBufferLineSize = 128;  // Max buffer size used

  const char* const_iterator = key;
  // FIXME(Unitex) unichar is uint16_t (unsigned) and u_fgetc_raw returns int
  // hence unichar can never be equal to EOF (-1)
  int32_t u_char;

  unichar string_buffer[kMaxBufferLineSize];
  size_t size = 0;

  // All strings in /etc/os-release should be in UTF-8 format,
  // and non-printable characters should not be used -- freedesktop.org
  while (EOF != (u_char = u_fgetc_UTF8(file_descriptor))) {
    if(u_char == os_release::symbol::kComment) {
     // ignore comments until the end of the line
      while((EOF != (u_char = u_fgetc_UTF8(file_descriptor))) &&
                     u_char != os_release::symbol::kNewLine) continue;
    } else if (u_char == *const_iterator) {
      ++const_iterator;
      // until key isn't found
      while ((*const_iterator != os_release::symbol::kEOS) &&
             (EOF != (u_char = u_fgetc_UTF8(file_descriptor)))) {
        if (*const_iterator == u_char) {
          ++const_iterator;
        }  // (*const_iterator == u_char)
      }  // ((*const_iterator != '\0')
      // key was found
      if(*const_iterator == os_release::symbol::kEOS) {
        // ignore spaces
        while((u_char != os_release::symbol::kEqual)              &&
              (EOF != (u_char = u_fgetc_UTF8(file_descriptor)))   &&
              isspace(u_char)) continue;
        if(u_char == os_release::symbol::kEqual) {
          // ignore initial spaces
          while((EOF != (u_char = u_fgetc_UTF8(file_descriptor))) &&
                isspace(u_char)) continue;
          // ignore initial quote
          u_char = u_char == os_release::symbol::kDoubleQuote ?
                             u_fgetc_UTF8 (file_descriptor) :
                             u_char;
          // until the end of the line or till buffer has space
          do {
            string_buffer[size++] = u_char;
          } while((u_char != os_release::symbol::kNewLine)  &&
                  (size < kMaxBufferLineSize)               &&
                  (EOF != (u_char = u_fgetc_UTF8(file_descriptor))));

          // ignore new line char
          size = (size > 0 && string_buffer[size-1] ==
                  os_release::symbol::kNewLine) ?
                  size - 1 : // true
                  size;      // false

          // ignore final spaces
          while(size > 0 && isspace(string_buffer[size-1]) && --size) continue;

          // ignore final quote
          size = (size > 0 && string_buffer[size-1] ==
                  os_release::symbol::kDoubleQuote) ?
                  size - 1 : // true
                  size;      // false

          // close file
          af_fclose_unlogged(file_descriptor);

          // return string
          return UnitexString(string_buffer, size);
        }  // if(*const_iterator == '=')
      }  // if(*const_iterator == '\0')
    }  //  else if (u_char == *const_iterator)
  }  // while (EOF != (u_char = u_fgetc_UTF8(file_descriptor)))

  // close file
  af_fclose_unlogged(file_descriptor);

  // if key not found returns an empty string
  return UnitexString();
}

/**
 * Get Unix OS version
 * @return returns -1 if fails
 */
UNITEX_FORCE_INLINE
UnitexString release_name() {
  UnitexString os_release_name;

  // First try to read release name from /etc/os-release
  if (MsgLogFile::exists(os_release::kFileName)) {
    // try to read ID key
    os_release_name = os_release_get_value(os_release::key::ID);
    // if ID key wasn't found, try to read ID_LIKE as fallback for ID
    if (os_release_name.is_empty()) {
      os_release_name = os_release_get_value(os_release::key::ID_LIKE);
    }  // (os_release_name.is_empty())
  }  // (MsgLogFile::exists(os_release::kFileName))

  // if /etc/os-release file doesn't exist or ID/ID_LIKE keys were not found
  // try to find the OS * info file
  if(os_release_name.is_empty()) {
     os_release_name = os_release_from_file();
  }

  // uname
  // last try to read /proc/version


  // if all else fails, OS release name is unknown
  if(os_release_name.is_empty()) {
     os_release_name = UnitexString("<unknown>");
  }

  // return release name in lowercase format
  return os_release_name.lower();
}  // release_name()
/* ************************************************************************** */
}  // namespace info::host::os_linux::{unnamed}
/* ************************************************************************** */
}  // namespace info::host::os_linux
/* ************************************************************************** */
#endif  // UNITEX_OS_IS(UNIX)
/* ************************************************************************** */
namespace {  // namespace info::host::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
#if  UNITEX_OS_IS(CYGWIN)
/* ************************************************************************** */
/**
 * Get Cygwin OS version
 * @return returns -1 if fails
 */
UNITEX_FORCE_INLINE
int operating_system_version() {
  return -1;
}
/* ************************************************************************** */
#elif UNITEX_OS_IS(UNIX)
/* ************************************************************************** */
/**
 * Gets the name of the operating system (compile time value)
 */
UNITEX_FORCE_INLINE
UnitexString operating_system_name() {
  return UnitexString(UNITEX_OS_NAME);
}

/**
 * Get Unix OS version
 * @return returns -1 if fails
 */
UNITEX_FORCE_INLINE
int32_t operating_system_release_version() {
  return 60000000;
}

/**
 * Get Unix OS version
 * @return returns -1 if fails
 */
UNITEX_FORCE_INLINE
UnitexString kernel_name() {
  // utsname structure in <sys/utsname.h>
  // sysname  : uname -s : the kernel name
  // release  : uname -r : the kernel release
  // version  : uname -v : the kernel version
  // nodename : uname -n : the network node hostname
  // machine  : uname -m : the machine hardware name
  struct utsname info;
  int fail = uname(&info);
  if(fail){
     return UnitexString("<unknown>");
  }

  return UnitexString(info.sysname).lower();
}

/**
 * Get Unix OS kernel version
 */
UNITEX_FORCE_INLINE
UnitexString kernel_version() {
  struct utsname info;
  // sysname  : uname -s : the kernel name
  // release  : uname -r : the kernel release
  // version  : uname -v : the kernel version
  // nodename : uname -n : the network node hostname
  // machine  : uname -m : the machine hardware name
  int fail = uname(&info);
  if(fail){
     // this is VVRRPPPPP
     return UnitexString("?????????");
  }

  // uname -v is all numbers ?
  // uname -v VV "%[0-9]"
  // uname -r RR "%[0-9]"

  // uname -r[0] is a letter
  //  VV "%*[^.].%[0-9]"

  // uname -r is all numbers with one or two dots
  //  VV "%[0-9]"
  //  RR "%*d.%[0-9]"
  //  PP "%*d.%*d.%[0-9]"

  return UnitexString(info.release);
}

UNITEX_FORCE_INLINE
UnitexString operating_system_release_name() {
#if UNITEX_OS_UNIX_ENVIRONMENT_IS(LINUX)
  return os_linux::release_name();
#endif  // UNITEX_OS_UNIX_ENVIRONMENT_IS(LINUX)
  return UnitexString();
}

// VERSION_ID




/* ************************************************************************** */
#elif UNITEX_OS_IS(WINDOWS)
/* ************************************************************************** */
UNITEX_FORCE_INLINE
UnitexString operating_system_name() {
  return UnitexString::format("TODO");
}

UNITEX_FORCE_INLINE
UnitexString operating_system_release_name() {
  return UnitexString::format("TODO");
}

UNITEX_FORCE_INLINE
UnitexString kernel_name() {
  return UnitexString::format("TODO");
}

UNITEX_FORCE_INLINE
UnitexString kernel_version() {
  return UnitexString::format("TODO");
}

/**
 * Get Unix OS version
 * @return returns -1 if fails
 */
UNITEX_FORCE_INLINE
int32_t operating_system_release_version() {
  return 0;
}


/**
 * Get Windows OS version number
 * >= Windows 8.1
 *  [vvrrbbbb] major_version minor_version kernel32.dll_build_version
 * >= Windows 2000 && < Windows 8.1
 *  [vvrrmmii] major_version minor_version service_pack_major service_pack_minor
 *  < Windows 2000
 *  [vvrrbbbb] major_version minor_version build_number
 * @return returns -1 if fails
 */
UNITEX_FORCE_INLINE
int windows_version_number() {
// GetVersionEx was deprecated starting Windows 8.1 (Blue)
// We need only a full version number as information for debugging or user
// support purposes but using the new <VersionHelpers.h> header no help at all!
// We will start to using GetFileVersionInfo as temporary workaround
# if UNITEX_OS_WINDOWS_API_AT_LEAST(WINBLUE)
  int windows_version = -1;
  const LPCTSTR szVersionFile = _T("kernel32.dll");
  DWORD verHandle;
  DWORD verSize = GetFileVersionInfoSize(szVersionFile, &verHandle);

  if (verSize) {
    LPSTR lpVersionInfo = (LPSTR) malloc(verSize);
    if (lpVersionInfo) {
      if (GetFileVersionInfo(szVersionFile,
                             verHandle,
                             verSize,
                             lpVersionInfo)) {
        UINT uLen;
        VS_FIXEDFILEINFO* lpFileInfo;
        if(VerQueryValue(lpVersionInfo ,
                         _T("\\"),
                         (LPVOID *)&lpFileInfo ,
                         (PUINT)&uLen )) {
          // major    : H(lpFileInfo->dwFileVersionMS)
          // minor    : L(lpFileInfo->dwFileVersionMS)
          // build    : H(lpFileInfo->dwFileVersionLS)
          // release  : not used
          windows_version =
              (((lpFileInfo->dwFileVersionMS >> 16 ) & 0xffff) * 0x1000000 \
             + ((lpFileInfo->dwFileVersionMS >>  0 ) & 0xffff) * 0x10000   \
             + ((lpFileInfo->dwFileVersionLS >> 16 ) & 0xffff) * 0x1);
        }  // VerQueryValue()
      }  // GetFileVersionInfo()
    }  // lpVersionInfo
    free(lpVersionInfo);
  }  // verSize

  return windows_version;

# else  // !UNITEX_OS_WINDOWS_API_AT_LEAST(WINBLUE)
#  if UNITEX_OS_WINDOWS_API_AT_LEAST(WIN2K)
     OSVERSIONINFOEX version_info;
     ZeroMemory(&version_info, sizeof(OSVERSIONINFOEX));
     version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

     if(!(GetVersionEx((OSVERSIONINFO*) &version_info))) {
       return -1;
     }

     int windows_version =  (version_info.dwMajorVersion    * 1000000  \
                           + version_info.dwMinorVersion    * 10000    \
                           + version_info.wServicePackMajor * 100      \
                           + version_info.wServicePackMinor * 1);
     return windows_version;
#  else   // !UNITEX_OS_WINDOWS_API_AT_LEAST(WIN2K)
     // @see http://support.microsoft.com/kb/189249/en-us
     OSVERSIONINFO version_info;
     ZeroMemory(&version_info, sizeof(OSVERSIONINFO));
     version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

     if(!(GetVersionEx((OSVERSIONINFO*) &version_info))) {
       return -1;
     }

     int windows_version =  (version_info.dwMajorVersion    * 1000000  \
                           + version_info.dwMinorVersion    * 10000    \
                           + version_info.dwBuildNumber     * 1);
     return windows_version;
#  endif  //  UNITEX_OS_WINDOWS_API_AT_LEAST(WIN2K)
# endif  //  UNITEX_OS_WINDOWS_API_VERSION_AT_LEAST(WIN8,1)
}
#endif  // UNITEX_OS_IS(CYGWIN)

/**
 *
 */
UNITEX_FORCE_INLINE
UnitexString operating_system_identifier() {
//  u_printf("==%s==\n",UNITEX_FUNCTION_NAME);
  return UnitexString::format("host.os.%S.%S:%09d",
                              operating_system_name().c_unichar(),
                              operating_system_release_name().c_unichar(),
                              operating_system_release_version());
}

/**
 *
 */
UNITEX_FORCE_INLINE
UnitexString kernel_identifier() {
  return UnitexString::format("host.kernel.%S:%S",
                               kernel_name().c_unichar(),
                               kernel_version().c_unichar());
}

// kernel_name
// kernel_version

// cpu_name
// cpu_version

// thread_model
/* ************************************************************************** */
}  // namespace info::host::{unnamed}
/* ************************************************************************** */
}  // namespace info::host
/* ************************************************************************** */
}  // namespace info
/* ************************************************************************** */
namespace detail {
/* ************************************************************************** */
template <typename T>
struct pointer_t {
    typedef T* type;
};
/* ************************************************************************** */
namespace {   // namespace detail::{unnamed}, to enforce the one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */

/* ************************************************************************** */
}  // namespace detail::{unnamed}
/* ************************************************************************** */
}  // namespace detail
/* ************************************************************************** */
// MsgLogTarget : AUTO (DEFAULT), ALL

// CONSOLE      : is STDOUT, STDERR,
// SYSLOG       :
// FILE         : -> mode {w+}... maxBytes, backupCount,
// NULL         : (isn't  /dev/null)
// FUNCTION     :
//
// add_target() // (addObserver)
//
//
// Target
//  - MsgLogFilter
//  - MsgLogMessageDetails
//  -
//  -
class MsgLogTarget {
 public :
  // Public typedefs and enums
  // Public static constants
  // Public constants

  // Public constructors
  MsgLogTarget() {

  }

  // Public destructor
  virtual ~MsgLogTarget() {

  }

  // Public operator overloading
  // Public static methods
  // Public methods

  virtual void init() = 0;


  virtual void shutdown() = 0;

  virtual void set_identifier(const UnitexString& identifier) {
    identifier_ = identifier;
  }

  virtual UnitexString get_identifier() {
    return identifier_;
  }

  // Public data members

 protected :
  // Protected typedefs and enums
  // Protected static constants
  // Protected constants
  // Protected constructors
  // Protected destructor
  // Protected operator overloading
  // Protected static methods
  // Protected methods
  // Protected data members
  bool disabled_;

  UnitexString identifier_;

 private :
  /**
   * This class disallow implicit copy constructor and assignment
   * use "const MsgLogTarget& message" to pass as parameter
   */
  UNITEX_DISALLOW_COPY_AND_ASSIGN(MsgLogTarget);
};
/* ************************************************************************** */
//template <typename T>
//struct MsgLogTypeWatcher {
//  // Public constructors
//
//  // default constructor
//  MsgLogTypeWatcher() : t_() {
//  // t_() if T is a POD type then value initializes to zero [8.5/5]
//  }
//
//  // explicit constructor from underline type
//  // if type is POD
//  explicit MsgLogTypeWatcher(const T t) : t_(t) {
//  }
//
//  // if type isnt POD
//  // const T& t
//
//  // copy constructor
//  MsgLogTypeWatcher(const MsgLogTypeWatcher& other) : t_(other.t_) {
//
//  }
//
//  MsgLogTypeWatcher& operator=(const MsgLogTypeWatcher& rhs) {
//    t_ = rhs.t_;
//    return* this;
//  }
//
//  // if type is POD const T rhs
//
//  MsgLogTypeWatcher& operator=(const T& rhs) {
//    t_ = rhs;
//    return* this;
//  }
//
//  /**
//   * Specifies a conversion from MsgLogTypeWatcher to the underline type
//   * using a conversion-type-id function
//   */
//  UNITEX_EXPLICIT_CONVERSIONS
//  operator T& () {
//    return t_;
//  }
//
//  UNITEX_EXPLICIT_CONVERSIONS
//  operator T&() const {
//    return t_;
//  }
//
//  // if ispod
//  // operator T
//
//  // Public destructor
//  // Does nothing
//  ~MsgLogTypeWatcher() {
//  }
//
//  // Public operator overloading
//  bool operator==(const MsgLogTypeWatcher& rhs) const {
//    return t_ == rhs.t_;
//  }
//
//  // Public data members
//  T t_;
//};
/* ************************************************************************** */
//class MsgLogMessageFormat {
//UNITEX_PARAMS_NON_NULL
//static UnitexString as_string(const MsgLogMessage& message) {
//  use format
//  return UnitexString();
//}
// const char* format;
// console_format_
//};

/* ************************************************************************** */
class MsgLogMessage {
 public :
  // Public typedefs and enums
  // Public static constants
  // Public constants
  // Public constructors
  // Public destructor
  // Public operator overloading
  // Public static methods
  // Public methods
  // Public data members
 private :
  // Private typedefs and enums
  // Private static constants
  // Private constants
  // Private constructors
  // Private destructor
  // Private operator overloading
  // Private static methods
  // Private methods
  // Private data members
  int32_t*      file_line_;
  Chrono*       time_stamp_;
  Chrono*       time_elapsed_;
  UnitexString* user_message_;
  UnitexString* thread_name_;
  UnitexString* function_name_;
  UnitexString* file_basename_;
  unitex::SeverityLevel*  message_level_;

  /**
   * This class disallow implicit copy constructor and assignment
   * use "const MsgLogMessage& message" to pass as parameter
   */
  UNITEX_DISALLOW_COPY_AND_ASSIGN(MsgLogMessage);
};
/* ************************************************************************** */
/**
 * This is the main message logging class
 */
class MsgLogger {
 public :
  // Typedefs and Enums
  // Constants (static const data members)
  // Constructors

  // Destructor
  ~MsgLogger(){
    u_printf("destroy\n");
    //params.output_file.flush();
    //params.error_file.flush();
  }

  // Operator overloads

  // Methods, including static methods
  /**
   *
   */
  static MsgLogger& get() {
    init_once();
    assert( instance_ != NULL );
    return *instance_;
  }

  /**
   *
   */
  static void enable() {
    // TODO(martinec) lock(guard)
    disabled_ = false;
  }

  /**
   *
   */
  static void disable() {
    // TODO(martinec) lock(guard)
    disabled_ = true;
  }

  //void write()
  //Lock
  //output_file!=null && error_file!=null
  //generate log_header :

  /**
   *
   */
  // const unitex::SeverityLevel& level, const char* file, size_t line, const char* message
  /* ************************************************************************** */
  //  //# Unitex Message Loggger Fri Mar 22 02:26:35 CET 2013
  //  //# Logging host  [hostname]
  //  //# Logging file  [console]
  //  //# Markers:
  //  //# (!!) notice,  (II) inform
  //  //# (WW) warning, (EE) error
  //  //# XX : Marker
  //  //# _threadid_ processid
  //  //# hh:mm:ss.uuuuuu
  //  //# hh : hour ; mm : minute ; ss : seconds ; uuuuuu : microseconds
  //  //#
  //  //;Format:
  //  //; (_xx_) [_elapsed_ _threadid_ _hh_:_mm_:_ss_._uuuuuu_ _file_:_line_] _message_
  //  //if ( !disabled_ )
  //  //u_fprintf and disk full ? (ENOSPC)
  //  //flush every x bytes
  //  //setw,setfill
  //  //(!!) Logging started   : Sun Jun  2 13:55:27 CEST 2013
  //  //(!!) ...
  //  //(!!) 1 warning generated
  //  //(!!) 1 error   generated
  //  //(!!) Elapsed time      : 00:00:04
  //  //(!!) Logging finished  : Sun Jun  2 13:55:31 CEST 2013

//  va_list args;
//  va_start(args, format);
//  vfprintf(stderr, format, args);
//  va_end(args);

  void log(const unitex::SeverityLevel& level,
           const UnitexString& message,
           const char* file_name,
           int32_t file_line,
           const char* function_name) const {

    if(disabled_) {
      return;
    }

    // Only messages with a level equal or higher than than logger's
    // severity level are propagated
    if(level >= params.severity_level) {
      // TODO(martinec) lock(guard)
      // Create message
      // Send message to targets
      // }

      unitex::Chrono time = unitex::Chrono::now();
      UnitexString file_basename = MsgLogFile::basename(file_name);

//      const MsgLogMessageDetails::flag_t option = MsgLogMessageDetails::UserMessage.to_number();

      switch(params.message_details) {
        case  MsgLogMessageDetails::constant::UserMessage :
          u_printf("%S\n",message.c_unichar());
          break;

        case MsgLogMessageDetails::kFlagSimpleDetails :
          u_printf("(%s) %S\n",level.alias(),
                                message.c_unichar());
          break;

        case MsgLogMessageDetails::kFlagMoreDetails :
          u_printf("(%s) [%S] %S\n",level.alias(),
                  time.as_timestamp().c_unichar(),
                             message.c_unichar());
          break;

        case MsgLogMessageDetails::kFlagExtraDetails :
          u_printf("==%S== (%s) [%S] %S\n",
                   info::process::thread_name().c_unichar(),
                                       level.alias(),
                     time.as_timestamp().c_unichar(),
                                 message.c_unichar());
          break;

        case MsgLogMessageDetails::kFlagFullDetails :
          u_printf("==%S== (%s) [%S | %S:%0.4d] %S\n",
                   info::process::thread_name().c_unichar(),
                                       level.alias(),
                     time.as_timestamp().c_unichar(),
                     file_basename.c_unichar(),
                                           file_line,
                                 message.c_unichar());
          break;


        case MsgLogMessageDetails::kFlagFullDetails |
             MsgLogMessageDetails::constant::TimeElapsed:
          u_printf("==%S== (%s) [%S # %12.0f | %s %S:%0.4d] %S\n",
                   info::process::thread_name().c_unichar(),
                                       level.alias(),
                     time.as_timestamp().c_unichar(),
                     stats.timestamp_.elapsed(time).as_microseconds(),
                     function_name,
                     file_basename.c_unichar(),
                                           file_line,
                                 message.c_unichar());
          break;

//        case MsgLogMessageDetails::constant::All :
//          u_printf("==%S== (%s) [%S | %s:%0.4d] %S\n",
//                   detail::thread_name().c_unichar(),
//                                       level.alias(),
//                     time.as_timestamp().c_unichar(),
//                                            filename,
//                                           file_line,
//                                message.c_unichar());
//          break;

        default :
          u_printf("todo\n");
          break;
      }

    }

  }

//  void trace(const char* format, ...)
//  void debug(const char* format, ...)
//  void info(const char* format, ...)
//  void notice(const char* format, ...)
//  void warning(const char* format, ...)
//  void error(const char* format, ...)
//  void critical(const char* format, ...)
//  void alert(const char* format, ...)
//  void panic(const char* format, ...)


  /**
   *
   * @return
   */

  static UnitexString thread_name() {
    return info::process::thread_name();
  }

  //print_stats (loggins started, elapsed, finished, message count)


  /**
   *
   * @return
   */
  static bool is_enabled() {
    return !is_disabled();
  }

  /**
   *
   * @return
   */
  static bool is_disabled() {
    return disabled_;
  }

  // Data Members

  /**
   *
   */
  MsgLogParams params;

  /**
   *
   */
  MsgLogStats stats;

 private :
  // Typedefs and Enums

  // Constants (static const data members)

  /**
   *
   */
  static bool disabled_;

  /**
   *
   */
  static detail::pointer_t<MsgLogger>::type instance_;

  /**
   *
   */
  static unitex::once_flag_t flag_;

  // Constructors
  /**
   * Private constructor
   */
  MsgLogger() : params(), // params default construction
                stats() {  // stats default construction

  }

  // channels() -> filter.
  //

  // Methods, including static methods

  /**
   * init_once allow only a single instance creation
   */
  static bool init_once() {
    // one_time_initialization is thread-safe
    return unitex::one_time_initialization(flag_, init);
  }

  /**
   * Initialize the instance member
   * this method will be once called by the init_once() function
   */
  static void init() {
    assert( instance_ == NULL );
    // TODO(martinec) change by malloc, test not null or fatal_alloc_error
    instance_ = new MsgLogger();
  }

  /**
   *
   */
  static void shutdown() {
    // TODO(martinec) lock(guard)
    // params.output_file.flush();
    // params.error_file.flush();
    assert( instance_ != NULL );
    delete instance_;
    instance_ = 0;
    // TODO(martinec) change by free
  }

  // Destructor
  // Operator overloads

  // Data Members

  /**
   *
   */
  friend class helper::ObjectInitializer<MsgLogger>;

  /**
   * This class disallow implicit copy constructor and assignment
   * use "const unitex::SeverityLevel& level" to pass as parameter
   */
  UNITEX_DISALLOW_COPY_AND_ASSIGN(MsgLogger);
};
/* ************************************************************************** */

/* ************************************************************************** */
namespace detail {
/* ************************************************************************** */
namespace {   // namespace detail::{unnamed}, to enforce the one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
//#pragma init_seg(
//__cxa_atexit
//__attribute__((constructor))
///*static*/ void initialize_() {
//  u_printf("hello");
//}
//
//__attribute__((destructor))
///*static*/ void destroy_() {
//  u_printf("world");
//}



///**
// *
// * @return
// */
//MsgLogger& unitex__msglogger__get() {
//      return MsgLogger::get();
//}
/* ************************************************************************** */
}  // namespace detail::{unnamed}
/* ************************************************************************** */
}  // namespace detail
/* ************************************************************************** */
//#define TDEBUG(fmt, args...)
//    alog::Logger::getRootLogger()->log(alog::LOG_LEVEL_DEBUG,
//            __FILE__, __LINE__, __func__, fmt, ##args)
///** Log a message using the @c pLogFunction passed **/
//#define LOG(level, message, ...)
//  pLogFunction(level, false, g_basename(__FILE__), __LINE__, __func__, message,## __VA_ARGS__);
//#define WARN(...) do{__VALIDATE_FROMAT_SPECIFIERS(__VA_ARGS__);Logger::Log(_WARNING_, E__FILE__, __LINE__OVERRIDE, __FUNC__OVERRIDE, __VA_ARGS__);}while(0)

// #define logerr(  x, ...) log_logerr(  __FILE__, __LINE__, __func__, x, ##__VA_ARGS__)
/*
122 #define ATH5K_DBG_UNLIMIT(_sc, _m, _fmt, ...) do { \
23         if (unlikely((_sc)->debug.level & (_m))) \
124                 ATH5K_PRINTK(_sc, KERN_DEBUG, "(%s:%d): " _fmt, \
125                         __func__, __LINE__, ##__VA_ARGS__); \
126         } while (0)
*/
//unitex::Msg
//msglogger_trace()
//msglogger_debug()
//msglogger_info()
//msglogger_notice()
//msglogger_warning()
//msglogger_error()
//msglogger_critical()
//msglogger_alert()
//msglogger_panic()

//MSGLOGGER_TRACE()
//MSGLOGGER_DEBUG()
//MSGLOGGER_INFO()
//MSGLOGGER_NOTICE()
//MSGLOGGER_WARNING()
//MSGLOGGER_ERROR()
//MSGLOGGER_CRITICAL()
//MSGLOGGER_ALERT()
//MSGLOGGER_PANIC()
/* ************************************************************************** */
#if  !defined(UNITEX_BUILD_LIBRARY)
# if !defined(UNITEX_MANUAL_INIT)
/* ************************************************************************** */
namespace {  // {unnamed}, internal linkage
/* ************************************************************************** */
  // Every file that includes this header also creates a MsgLogger object
  // and try to initialize it (only once for all tries). This is done using
  // a Schwarz Counter within an one_time_initialization call
  helper::ObjectInitializer<MsgLogger> unitex__msglogger__initializer;
/* ************************************************************************** */
}  // {unnamed}
/* ************************************************************************** */
# endif  // !defined(UNITEX_MANUAL_INIT)
#else    //  defined(UNITEX_LIBRARY)
/* ************************************************************************** */
UNITEX_UNMANGLE_DECLS_BEGIN
/* ************************************************************************** */
// TODO(martinec) C API interface :
//UNITEX_API int msglogger_init()
//UNITEX_API int msglogger_set_level()

//UNITEX_API int msglogger_write()
//UNITEX_API int msglogger_print()
//UNITEX_API int msglogger_vprint()

//UNITEX_API int msglogger_enable()
//UNITEX_API int msglogger_disable()
//UNITEX_API int msglogger_shutdown()
//
/* ************************************************************************** */
UNITEX_UNMANGLE_DECLS_END
/* ************************************************************************** */
#endif   // !defined(UNITEX_LIBRARY)
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // defined(UNITEX_EXPERIMENTAL_MSGLOGGER)
/* ************************************************************************** */
#endif  // UNITEX_H_   // NOLINT
