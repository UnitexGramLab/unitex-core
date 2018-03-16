/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
// Header for this file
#include "MsgLogger.h"  // NOLINT
/* ************************************************************************** */
#if defined(UNITEX_EXPERIMENTAL_MSGLOGGER)
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
// Initializes MsgLogGuard static mutex with default attributes
#if   UNITEX_USE(ISO_THREADS)
mutex_t MsgLogGuard::mutex_ UNITEX_EQ_MUTEX_INIT;
/* ************************************************************************** */
#elif UNITEX_USE(BOOST_THREADS)
mutex_t MsgLogGuard::mutex_ UNITEX_EQ_MUTEX_INIT;
/* ************************************************************************** */
#elif UNITEX_USE(POSIX_THREADS)
mutex_t MsgLogGuard::mutex_ UNITEX_EQ_MUTEX_INIT;
/* ************************************************************************** */
#elif UNITEX_USE(WIN32_THREADS)
// FIXME(martinec) error: definition of ‘thread_details::wthread_mutex_t::critical_section_’
// is not in namespace enclosing ‘thread_details::wthread_mutex_t’ [-fpermissive]
CRITICAL_SECTION thread_details::wthread_mutex_t::critical_section_;
mutex_t MsgLogGuard::mutex_ UNITEX_EQ_MUTEX_INIT;
/* ************************************************************************** */
#elif UNITEX_USE(SINGLE_THREAD)
mutex_t MsgLogGuard::mutex_ UNITEX_EQ_MUTEX_INIT;
#endif  // UNITEX_USE(ISO_THREADS)
/* ************************************************************************** */
#define Initialize__MsgLogFormat__(MsgLogFormatName, StrName, StrAlias)  \
const MsgLogMessageDetails MsgLogMessageDetails::MsgLogFormatName(                       \
      MsgLogMessageDetails::constant::MsgLogFormatName);                         \
const char*  MsgLogMessageDetails::MsgLogFormatName::name  = StrName;            \
const char*  MsgLogMessageDetails::MsgLogFormatName::alias = StrAlias
/* ************************************************************************** */
Initialize__MsgLogFormat__(None,          "none",           "nn");
Initialize__MsgLogFormat__(ThreadName,    "thread_name",    "tn");
Initialize__MsgLogFormat__(SeverityAlias, "severity_alias", "sa");
Initialize__MsgLogFormat__(SeverityName,  "severity_name",  "sn");
Initialize__MsgLogFormat__(TimeElapsed,   "time_elapsed",   "te");
Initialize__MsgLogFormat__(TimeStamp,     "time_stamp",     "ts");
Initialize__MsgLogFormat__(FileName,      "file_name",      "fn");
Initialize__MsgLogFormat__(FileLine,      "file_line",      "fl");
Initialize__MsgLogFormat__(UserMessage,   "user_message",   "um");
Initialize__MsgLogFormat__(All,           "all",            "al");
/* ************************************************************************** */
#undef Initialize__MsgLogFormatDetails__
/* ************************************************************************** */
// Initialize default enconding config
const VersatileEncodingConfig MsgLogFile::kDefaultEncoding = {
DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT,    // encodings that are supported
                                    UTF8,     // default encoding output
                                       0};    // don't write BOM

// Initialize STDOUT and STDERR default Files
const MsgLogFile MsgLogFile::STDOUT(U_STDOUT, "stdout");
const MsgLogFile MsgLogFile::STDERR(U_STDERR, "stderr");
/* ************************************************************************** */
bool MsgLogger::disabled_             = false;
once_flag_t MsgLogger::flag_ = UNITEX_ONCE_INIT;
detail::pointer_t<MsgLogger>::type MsgLogger::instance_;
/* ************************************************************************** */
//
/* ************************************************************************** */
}  // unitex
/* ************************************************************************** */

// namespace __cxxabiv1
// {
//  /* guard variables */
//
//  /* The ABI requires a 64-bit type.  */
//  __extension__ typedef int __guard __attribute__((mode(__DI__)));
//
//  extern "C" int __cxa_guard_acquire (__guard *);
//  extern "C" void __cxa_guard_release (__guard *);
//  extern "C" void __cxa_guard_abort (__guard *);
//
//  extern "C" int __cxa_guard_acquire (__guard *g)
//  {
//    return !*(char *)(g);
//  }
//
//  extern "C" void __cxa_guard_release (__guard *g)
//  {
//    *(char *)g = 1;
//  }
//
//  extern "C" void __cxa_guard_abort (__guard *)
//  {
//
//  }
// }

/* ************************************************************************** */
#endif  // defined(UNITEX_EXPERIMENTAL_MSGLOGGER)
/* ************************************************************************** */
