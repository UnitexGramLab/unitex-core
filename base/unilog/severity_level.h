/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 */
/**
 * @file      severity_level.h
 * @brief     Logger severity levels class
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 severity_level.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_UNILOG_SEVERITY_LEVEL_H_                        // NOLINT
#define UNITEX_BASE_UNILOG_SEVERITY_LEVEL_H_                        // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/compiler/compiler.h"
/* ************************************************************************** */
// C system files                  (try to order the includes alphabetically)
#include <cstring>                 // strcmp
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
struct SeverityLevel {
  // Typedefs and Enums
  /**
   * Wrapper enum for SeverityLevels
   * Levels 0 to 7 correspond to the priority levels used by syslog
   * @see http://tools.ietf.org/html/rfc5424
   */
  struct constant {
    enum level {
      invalid_ = -1,  // invalid level assignment
      Panic,          // 0 : system is unusable (not to be used)
      Alert,          // 1 : action must be taken immediately
      Critical,       // 2 : critical condition
      Error,          // 3 : error condition
      Warning,        // 4 : warning condition
      Notice,         // 5 : normal but significant condition
      Info,           // 6 : purely informational message
      Debug,          // 7 : messages to debug application/plugins
      Trace,          // 8 : detailed trace of execution
      None,           // 9 : not a message logging
    };
  };

  // Constants (static const data members)
  /**
   * Number of logging levels
   */
  static const int kNumberofLevels = 10;

  /**
   * Default message logging level
   */
  // Initialize the default log level, this value is used in SeverityLevel
  // construction
  static const constant::level kDefaultLevel = constant::Info;

  /**
   * Interface SeverityLevels
   */
  static const SeverityLevel Panic;     // unusable condition
  static const SeverityLevel Alert;     // action must be taken immediately
  static const SeverityLevel Critical;  // critical condition
  static const SeverityLevel Error;     // error condition
  static const SeverityLevel Warning;   // warning condition
  static const SeverityLevel Notice;    // normal but significant condition
  static const SeverityLevel Info;      // purely informational message
  static const SeverityLevel Debug;     // messages to debug application/plugins
  static const SeverityLevel Trace;     // detailed trace of execution
  static const SeverityLevel None;      // not a message logging

  // Constructors

  /**
   * Constructor without parameters
   * SeverityLevel is Info by default
   */
  SeverityLevel()
    :    level_(kDefaultLevel) { }

  /**
   * Constructor from integer
   */
  explicit SeverityLevel(int level_number) {
    *this = level_number;
  }

  /**
   * Constructor from string
   */
  explicit SeverityLevel(const char* level_name) {
    *this = level_name;
  }

  /**
   * Constructor from SeverityLevel
   * Note that this disallow implicit copy constructor
   * use "const SeverityLevel& level" to pass as parameter
   */
  explicit SeverityLevel(const SeverityLevel& level) {
    *this = level;
  }

  /**
   * Specifies a conversion from SeverityLevel to the underline level
   * using a conversion-type-id function
   */
  UNITEX_EXPLICIT_CONVERSIONS
  operator constant::level () const {
    return level_;
  }

  /**
   * Assignement operator from SeverityLevel
   */
  SeverityLevel& operator=(const SeverityLevel& rhs) {
    if (this != &rhs) {
      // TODO(martinec) lock(guard)
      level_ = rhs.level_;
    }
    return *this;
  }

  /**
   * Assignement operator from integer
   * Valid input : {0, 1, ..., kNumberofLevels - 1}
   */
  SeverityLevel& operator =(int level_number) {
    // TODO(martinec) lock
    level_ = (level_number < 0 || level_number >= kNumberofLevels) ?
              constant::invalid_ :
              static_cast<constant::level>(level_number);

    return *this;
  }

  // Operator overloads

  /**
   * Assignement operator from string
   * Valid input : {"debug", "info", ... , "panic"}
   */
  SeverityLevel& operator =(const char* level_name) {
    // TODO(martinec) lock
    level_ = string_to_level(level_name);

    return *this;
  }

  // Overload some operators, note that this explicit allows comparisons
  // between different instances of this class

  /**
   * Equality operator over SeverityLevel
   */
  bool operator ==(const SeverityLevel& rhs) const {
    return level_ == rhs.level_;
  }

  /**
   * Inequality operator over SeverityLevel
   */
  bool operator !=(const SeverityLevel& rhs) const {
    return !(*this == rhs);
  }

  /**
   * Less than operator over SeverityLevel
   */
  bool operator <(const SeverityLevel& rhs) const {
    return level_ < rhs.level_;
  }

  /**
   * Greater than operator over SeverityLevel
   */
  bool operator >(const SeverityLevel& rhs) const {
    return rhs < *this;
  }

  /**
   * Less than or equal to operator over SeverityLevel
   */
  bool operator <=(const SeverityLevel& rhs) const {
    return !(rhs < *this);
  }

  /**
   * Greater than or equal to operator over SeverityLevel
   */
  bool operator >=(const SeverityLevel& rhs) const {
    return !(*this < rhs);
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
    typedef void (SeverityLevel::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}

  public :
    /**
     * Returns true if state_ is constant::True, false otherwise
     */
    operator bool_type() const {
      return is_valid() ?
        &SeverityLevel::this_type_does_not_support_comparisons : 0;
    }
#endif

  // Methods, including static

  /**
   * Return a constant char pointer with the stringify name of the current
   * level
   */
  const char* name() const {
    return names(*this);
  }

  /**
   * Return a constant char pointer with the stringify alias of the current
   * level
   */
  const char* alias() const {
    return aliases(*this);
  }

  /**
   * Return an integer with the number of the current level
   */
  int number() const {
    return static_cast<int>(level_);
  }

  /**
   * Test if SeverityLevel is valid
   * @return true if level_ is  beetween 0 and kNumberofLevels - 1
   */
  bool is_valid() const {
    return level_ != constant::invalid_;
  }

 private:
  // Typedefs and Enums

  /**
   * Namea and Alias encapsulation for logging levels
   */
#define Declare__MsgLogLevel__(MsgLogLevelName)  \
  struct MsgLogLevelName  {                      \
    static const char* name;                     \
    static const char* alias;                    \
    static const constant::level value =         \
                 constant::MsgLogLevelName;      \
  }

  Declare__MsgLogLevel__(Panic);      // 0
  Declare__MsgLogLevel__(Alert);      // 1
  Declare__MsgLogLevel__(Critical);   // 2
  Declare__MsgLogLevel__(Error);      // 3
  Declare__MsgLogLevel__(Warning);    // 4
  Declare__MsgLogLevel__(Notice);     // 5
  Declare__MsgLogLevel__(Info);       // 6
  Declare__MsgLogLevel__(Debug);      // 7
  Declare__MsgLogLevel__(Trace);      // 8
  Declare__MsgLogLevel__(None);       // 9

#undef Declare__MsgLogLevel__ /* undef */

  // Constants (static const data members)


  // Constructors

  /**
   * Explicit constructor from log level
   */
  explicit SeverityLevel(constant::level level)
  : level_(level) {}

  // Methods, including static

  /**
   * Return a constant char pointer with the stringify name of the current
   * level
   */
  static const char* names(const SeverityLevel& level) {
    switch (level.level_) {
      case constant::Panic:    return Panic::name;
      case constant::Alert:    return Alert::name;
      case constant::Critical: return Critical::name;
      case constant::Error:    return Error::name;
      case constant::Warning:  return Warning::name;
      case constant::Notice:   return Notice::name;
      case constant::Info:     return Info::name;
      case constant::Debug:    return Debug::name;
      case constant::Trace:    return Trace::name;
      case constant::None:     return None::name;
      default:                 return ((const char *) 0);
    }
  }

  /**
   * Return a constant char pointer with the stringify alias of the current
   * Level
   */
  static const char* aliases(const SeverityLevel& level) {
    switch (level.level_) {
      case constant::Panic:    return Panic::alias;
      case constant::Alert:    return Alert::alias;
      case constant::Critical: return Critical::alias;
      case constant::Error:    return Error::alias;
      case constant::Warning:  return Warning::alias;
      case constant::Notice:   return Notice::alias;
      case constant::Info:     return Info::alias;
      case constant::Debug:    return Debug::alias;
      case constant::Trace:    return Trace::alias;
      case constant::None:     return None::alias;
      default:                 return ((const char *) 0);
    }
  }

  /**
   * Return a constant::level from the stringify name (or alias)
   * passed as argument
   */
  static constant::level string_to_level(const char* level_name) {
    //  "Premature optimization is the root of all evil
    //  (or at least most of it) in programming" --Donald Knuth
    if (level_name) {
      if (strcmp(level_name, Panic::name)     == 0) return constant::Panic;
      if (strcmp(level_name, Panic::alias)    == 0) return constant::Panic;
      if (strcmp(level_name, Alert::name)     == 0) return constant::Alert;
      if (strcmp(level_name, Alert::alias)    == 0) return constant::Alert;
      if (strcmp(level_name, Critical::name)  == 0) return constant::Critical;
      if (strcmp(level_name, Critical::alias) == 0) return constant::Critical;
      if (strcmp(level_name, Error::name)     == 0) return constant::Error;
      if (strcmp(level_name, Error::alias)    == 0) return constant::Error;
      if (strcmp(level_name, Warning::name)   == 0) return constant::Warning;
      if (strcmp(level_name, Warning::alias)  == 0) return constant::Warning;
      if (strcmp(level_name, Notice::name)    == 0) return constant::Notice;
      if (strcmp(level_name, Notice::alias)   == 0) return constant::Notice;
      if (strcmp(level_name, Info::name)      == 0) return constant::Info;
      if (strcmp(level_name, Info::alias)     == 0) return constant::Info;
      if (strcmp(level_name, Debug::name)     == 0) return constant::Debug;
      if (strcmp(level_name, Debug::alias)    == 0) return constant::Debug;
      if (strcmp(level_name, Trace::name)     == 0) return constant::Trace;
      if (strcmp(level_name, Trace::alias)    == 0) return constant::Trace;
      if (strcmp(level_name, None::name)      == 0) return constant::None;
      if (strcmp(level_name, None::alias)     == 0) return constant::None;
    }
    return constant::invalid_;
  }

  // Data Members

  /**
   * underline raw log level
   */
  constant::level level_;
};
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_UNILOG_SEVERITY_LEVEL                        // NOLINT
