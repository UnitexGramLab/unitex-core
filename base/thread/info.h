/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      info.h
 * @brief     Thread identification routines
 *
 * @see       base/thread/model.h
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 info.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_THREAD_INFO_H_                                  // NOLINT
#define UNITEX_BASE_THREAD_INFO_H_                                  // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/api/callback.h"               // UNITEX_WIN32_CALLBACK
#include "base/compiler/keyword/nullptr.h"   // UNITEX_NULLPTR
#include "base/compiler/attribute/inline.h"  // UNITEX_FORCE_INLINE
#include "base/integer/wordsize.h"           // UNITEX_WORDSIZE_IS
#include "base/macro/helpers.h"              // UNITEX_DISCARD_UNUSED_PARAMETER
#include "base/thread/model.h"               // ISO, BOOST, POSIX, WIN32, SINGLE
#include "base/thread/types.h"               // thread_id_t, thread_name_t
/* ************************************************************************** */
// get_current_thread_id()    : Portable current thread identifier
// get_current_thread_name()  : Portable current thread string identifier
/* ************************************************************************** */
// C system files
// For getpid() (UNIX) or _getpid() (Windows)
// The getpid() function shall return the process ID of the calling process
#if UNITEX_OS_IS(CYGWIN)
# include <cygwin/process.h>
# define unitex_getpi()      _getpid()
#elif UNITEX_OS_IS(WINDOWS)
# include <process.h>
# define unitex_getpi()      _getpid()
#elif   UNITEX_OS_IS(UNIX)
#ifndef __SYSCALL
# include <unistd.h>
#endif  // __SYSCALL
# define unitex_getpi()       getpid()
#endif  // UNITEX_OS_IS(UNIX)
/* ************************************************************************** */
// Unitex's .h files              (try to order the includes alphabetically)
#include "Unicode.h"               // u_* functions            // NOLINT
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
// Define thread name u_sprintf format
// How many bits per long
#if   UNITEX_WORDSIZE_IS(64)
// (64/8)*2 long (l) hexadecimals (X)
# define UNITEX_THREAD_NAME_FORMAT "0x%016lX"
#elif UNITEX_WORDSIZE_IS(32)
// (32/8)*2 long long (ll) hexadecimals (X)
// FIXME(martinec) check why ll format in unitex::u_sprintf is buggy ("0x%08llX")
#define UNITEX_THREAD_NAME_FORMAT  "0x%08lX"
#endif
/* ************************************************************************** */
/**
 * @def    get_current_thread_id
 * @brief  Returns the id of the current thread
 * @see    get_current_thread_name
 */
/**
 * @def    get_current_thread_name
 * @brief  Returns the stringize id of the current thread
 * @see    get_current_thread_id
 */
#if   UNITEX_USE(ISO_THREADS)
// get_current_thread_id (ISO threads)
UNITEX_FORCE_INLINE
thread_id_t get_current_thread_id() {
 return std::this_thread::get_id();
}
// get_current_thread_name (ISO threads)
UNITEX_FORCE_INLINE
void get_current_thread_name(unitex::unichar* thread_name_buffer) {
  size_t numeric_thread_id = std::hash<thread_id_t>()(get_current_thread_id());
  unitex::u_sprintf(thread_name_buffer, UNITEX_THREAD_NAME_FORMAT,
                   (uintptr_t) numeric_thread_id);
}
/* ************************************************************************** */
#elif UNITEX_USE(BOOST_THREADS)
// get_current_thread_id (BOOST threads)
UNITEX_FORCE_INLINE
thread_id_t get_current_thread_id() {
 return boost::this_thread::get_id();
}
// get_current_thread_name (BOOST threads)
UNITEX_FORCE_INLINE
void get_current_thread_name(unitex::unichar* thread_name_buffer) {
  std::stringstream stream;
  void* numeric_thread_id;
  stream << get_current_thread_id();
  stream >> numeric_thread_id;
  unitex::u_sprintf(thread_name_buffer, UNITEX_THREAD_NAME_FORMAT,
                   (uintptr_t) numeric_thread_id);
}
/* ************************************************************************** */
#elif UNITEX_USE(POSIX_THREADS)
// get_current_thread_id (POSIX threads)
UNITEX_FORCE_INLINE
thread_id_t get_current_thread_id() {
 return pthread_self();
}
// get_current_thread_name (POSIX threads)
UNITEX_FORCE_INLINE
void get_current_thread_name(unitex::unichar* thread_name_buffer) {
  unitex::u_sprintf(thread_name_buffer, UNITEX_THREAD_NAME_FORMAT,
                   (uintptr_t) get_current_thread_id());
}
/* ************************************************************************** */
#elif UNITEX_USE(WIN32_THREADS)
// get_current_thread_id (WIN32 threads)
UNITEX_FORCE_INLINE
thread_id_t get_current_thread_id() {
  return GetCurrentThreadId();
}
// get_current_thread_name (WIN32 threads)
UNITEX_FORCE_INLINE
void get_current_thread_name(unitex::unichar* thread_name_buffer) {
  unitex::u_sprintf(thread_name_buffer, UNITEX_THREAD_NAME_FORMAT,
                   (uintptr_t) get_current_thread_id());
}
/* ************************************************************************** */
#elif UNITEX_USE(SINGLE_THREADED)
// get_current_thread_id (Single threaded)
UNITEX_FORCE_INLINE
thread_id_t get_current_thread_id() {
#if   UNITEX_OS_IS(CYGWIN)  ||\
      UNITEX_OS_IS(WINDOWS) ||\
      UNITEX_OS_IS(UNIX)
    return static_cast<thread_id_t> (unitex_getpi());
#else /* this is not supposed to happen */
    return UINT64_C(1);
    // return static_cast<thread_id_t> syscall(SYS_gettid);
#endif
}

// get_current_thread_name (Single threaded)
UNITEX_FORCE_INLINE
void get_current_thread_name(unitex::unichar* thread_name_buffer) {
  unitex::u_sprintf(thread_name_buffer, UNITEX_THREAD_NAME_FORMAT,
                   (uintptr_t) get_current_thread_id());
}
#endif  // UNITEX_USE(ISO_THREADS)
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_THREAD_INFO_H_                               // NOLINT
