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
 * @file      types.h
 * @brief     Thread-related types
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 types.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_THREAD_TYPES_H_                                 // NOLINT
#define UNITEX_BASE_THREAD_TYPES_H_                                 // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/thread/model.h"       // UNITEX_USE_*_THREADS
#include "base/macro/helpers.h"      // UNITEX_DISALLOW_COPY_AND_ASSIGN
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
/**
 * @def    thread_id_t
 * @brief  An opaque type which acts as a unique identifier of threads
 * @see    get_current_thread_id
 */
#if   UNITEX_USE(ISO_THREADS)                       // ISO_THREADS
typedef std::thread::id thread_id_t;
/* ************************************************************************** */
#elif UNITEX_USE(BOOST_THREADS)                     // BOOST_THREADS
typedef boost::thread::id thread_id_t;
/* ************************************************************************** */
#elif UNITEX_USE(POSIX_THREADS)                     // POSIX_THREADS
typedef pthread_t thread_id_t;
/* ************************************************************************** */
#elif UNITEX_USE(WIN32_THREADS)                     // WIN32_THREADS
typedef DWORD     thread_id_t;
/* ************************************************************************** */
#elif UNITEX_USE(SINGLE_THREADED)                   // SINGLE_THREADED
typedef uint64_t  thread_id_t;
/* ************************************************************************** */
#endif  // UNITEX_USE(ISO_THREADS)
/* ************************************************************************** */
/**
 * @def    mutex_t
 * @brief  Thread mutex type
 *
 * UNITEX_USE(ISO_THREADS)     : std::mutex
 * UNITEX_USE(BOOST_THREADS)   : boost::mutex
 * UNITEX_USE(POSIX_THREADS)   : pthread_mutex_t
 * UNITEX_USE(WIN32_THREADS)   : wthread_mutex_t (custom class)
 * UNITEX_USE(SINGLE_THREADED) : sthread_mutex_t (dummy struct)
 *
 * @see    mutex_lock
 * @see    mutex_unlock
 */
/* ************************************************************************** */
#if   UNITEX_USE(ISO_THREADS)
typedef std::mutex mutex_t;
// C++11 support brace-enclosed initializer-lists
#define UNITEX_MUTEX_INIT      { }
#define UNITEX_EQ_MUTEX_INIT = { }
/* ************************************************************************** */
#elif UNITEX_USE(BOOST_THREADS)
typedef boost::mutex mutex_t;
#define UNITEX_MUTEX_INIT     /* nothing */
#define UNITEX_EQ_MUTEX_INIT  /* nothing */
/* ************************************************************************** */
#elif UNITEX_USE(POSIX_THREADS)
typedef pthread_mutex_t mutex_t;
#define UNITEX_MUTEX_INIT      PTHREAD_MUTEX_INITIALIZER
#define UNITEX_EQ_MUTEX_INIT = PTHREAD_MUTEX_INITIALIZER
/* ************************************************************************** */
#elif UNITEX_USE(WIN32_THREADS)
/* ************************************************************************** */
namespace thread_details {
/* ************************************************************************** */
// Windows mutex wrapper
class wthread_mutex_t {
 public:

  // Constructor
  wthread_mutex_t() {
    InitializeCriticalSection(&critical_section_);
  }

  // Destructor
  ~wthread_mutex_t() {
    DeleteCriticalSection(&critical_section_);
  }

  // Handle
  LPCRITICAL_SECTION handle() {
    return &critical_section_;
  }

 private:

  // Constants (including static const data members)
  static CRITICAL_SECTION critical_section_;

  // This class disallow implicit copy constructor and assignment
  UNITEX_DISALLOW_COPY_AND_ASSIGN(wthread_mutex_t);
};
/* ************************************************************************** */
}  // namespace thread_details
/* ************************************************************************** */
typedef thread_details::wthread_mutex_t mutex_t;
#define UNITEX_MUTEX_INIT     /* nothing */
#define UNITEX_EQ_MUTEX_INIT  /* nothing */
/* ************************************************************************** */
#elif UNITEX_USE(SINGLE_THREADED)
/* ************************************************************************** */
namespace thread_details {   // thread_details
/* ************************************************************************** */
// Single thread mutex wrapper
struct sthread_mutex_t {};
/* ************************************************************************** */
}  // namespace thread_details
/* ************************************************************************** */
typedef thread_details::sthread_mutex_t mutex_t;
// A class with no constructors, no private or protected members,
// no base classes, and no virtual functions can be initialized with
// a brace-enclosed initializer-list (C++98)
// @see http://en.cppreference.com/w/cpp/language/aggregate_initialization
#define UNITEX_MUTEX_INIT      { }
#define UNITEX_EQ_MUTEX_INIT = { }
/* ************************************************************************** */
#endif  // UNITEX_USE(ISO_THREADS)
/* ************************************************************************** */
/**
 * @def    once_flag_t
 * @brief  statically initialized to UNITEX_ONCE_INIT
 * @see    one_time_initialization
 */
/**
 * @def    UNITEX_ONCE_INIT
 * @brief  Constant value used to initialize once_flag_t instances to indicate
 *         that the logically associated routine has not been run yet
 * @see    one_time_initialization
 */
#if   UNITEX_USE(ISO_THREADS)
// 1. std::call_once
typedef std::once_flag once_flag_t;
// C++11 support brace-enclosed initializer-lists
#define UNITEX_ONCE_INIT { }
/* ************************************************************************** */
#elif UNITEX_USE(BOOST_THREADS)
// 2. boost::call_once
typedef boost::once_flag once_flag_t;
#define UNITEX_ONCE_INIT BOOST_ONCE_INIT
/* ************************************************************************** */
#elif UNITEX_USE(POSIX_THREADS)
// 3. pthread_once
typedef pthread_once_t once_flag_t;
#define UNITEX_ONCE_INIT PTHREAD_ONCE_INIT
/* ************************************************************************** */
#elif UNITEX_USE(WIN32_THREADS)
// According to MSDN minimum supported clients for SwitchToThread
// are Windows XP and Windows Server 2003 (both 0x0501).
// TODO(martinec) check SwitchToThread in a _WIN32_WINNT >= 0x0400
// environment
#if UNITEX_OS_WINDOWS_API_AT_LEAST(WINXP)
# define UNITEX_WIN32_FEATURE_SWITCH_TO_THREAD
#endif

// WIN32 One-Time Initialization (InitOnceExecuteOnce) capabilities
// were introduced with Windows Vista and Windows Server 2008 (both 0x0600)
#if UNITEX_OS_WINDOWS_API_AT_LEAST(VISTA)
# define UNITEX_WIN32_FEATURE_INIT_ONCE
#endif

#if defined(UNITEX_WIN32_FEATURE_INIT_ONCE)
  // 4. InitOnceExecuteOnce windows one-time initialization
  typedef INIT_ONCE once_flag_t;
# define UNITEX_ONCE_INIT INIT_ONCE_STATIC_INIT
#else
  // 5. InterlockedExchange windows interlocked functions
  struct once_flag_t_ {
      volatile LONG done_;  // 1 if once-time-call is done, 0 otherwise
      volatile LONG lock_;  // 1 if once-time-call is being done, 0 otherwise
  };
  typedef struct once_flag_t_ once_flag_t;
# define UNITEX_ONCE_INIT {0, 0}
#endif  // defined(UNITEX_WIN32_FEATURE_INIT_ONCE)
/* ************************************************************************** */
#elif UNITEX_USE(SINGLE_THREADED)
// 6. No thread-safe support
  typedef volatile int once_flag_t;
# define UNITEX_ONCE_INIT 0
#endif  // UNITEX_USE(ISO_THREADS)
/* ************************************************************************** */
/**
 * @def    thread_name_t
 * @brief  stringize id of a thread
 * @see    base/thread/info.h
 */
typedef unitex::unichar  thread_name_t[20];
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_THREAD_TYPES_H_                              // NOLINT
