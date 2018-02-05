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
 * @file      once.h
 * @brief     Thread-safe one time initialization
 *
 * Several implementation approaches relying in the selected thread model:
 *
 * (a) UNITEX_USE(ISO_THREADS)
 *     std::call_once         : C++11 (not experimental) or superior compilers
 *
 * (b) UNITEX_USE(BOOST_THREADS)
 *     boost::call_once       : Boost thread library
 *
 * (c) UNITEX_USE(POSIX_THREADS)
 *     pthread_once           : Unix-like mostly POSIX operating systems
 *
 * (d) UNITEX_USE(WIN32_THREADS)
 *     InterlockedExchange    : Windows XP, Server 2003 or earlier systems
 *     InitOnceExecuteOnce    : Windows Vista, Server 2008 or later systems
 *
 * (e) UNITEX_USE(SINGLE_THREADED)
 *     no thread-safe support : Use an eager initialization strategy within a Schwarz counter
 *
 * @see       base/thread/model.h
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 once.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_THREAD_ONCE_H_                                  // NOLINT
#define UNITEX_BASE_THREAD_ONCE_H_                                  // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/api/callback.h"               // UNITEX_WIN32_CALLBACK
#include "base/macro/helpers.h"              // UNITEX_DISCARD_UNUSED_PARAMETER
#include "base/compiler/keyword/nullptr.h"   // UNITEX_NULLPTR
#include "base/compiler/attribute/inline.h"  // UNITEX_FORCE_INLINE
#include "base/thread/model.h"               // ISO, BOOST, POSIX, WIN32, SINGLE
#include "base/thread/types.h"               // once_flag_t
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
/**
 * @def    one_time_initialization
 * @brief  Invokes a function only once even if called from multiple threads
 *
 * @code{.cpp}
 *         unitex::once_flag_t flag = UNITEX_ONCE_INIT;
 *
 *         void init() {
 *           printf("one_time_initialization\n");
 *         }
 *
 *         bool init_once() {
 *          return unitex::one_time_initialization(flag, init);
 *         }
 * @endcode
 *
 * @see    once_flag_t
 */
#if   UNITEX_USE(ISO_THREADS)
// 1. std::call_once
UNITEX_FORCE_INLINE
bool one_time_initialization(once_flag_t& once_control,
                             void (*init_routine)(void)){
  std::call_once(once_control, init_routine);
  return true;
}
/* ************************************************************************** */
#elif UNITEX_USE(BOOST_THREADS)
// 2. boost::call_once
UNITEX_FORCE_INLINE
bool one_time_initialization(once_flag_t& once_control,
                             void (*init_routine)(void)){
  boost::call_once(init_routine, once_control);
  return true;
}
/* ************************************************************************** */
#elif UNITEX_USE(POSIX_THREADS)
// 3. pthread_once
UNITEX_FORCE_INLINE
bool one_time_initialization(once_flag_t& once_control,
                             void (*init_routine)(void)){
  pthread_once(&once_control, init_routine);
  return true;
}
/* ************************************************************************** */
#elif UNITEX_USE(WIN32_THREADS)
# if  defined(UNITEX_WIN32_FEATURE_INIT_ONCE)
  // 4. InitOnceExecuteOnce windows one-time initialization
  namespace thread_details {
    // to store the init_routine function pointer
    struct init_routine_param {
        void (*init_routine)(void);
    };

    BOOL UNITEX_WIN32_CALLBACK call_once(PINIT_ONCE InitOnce,
                                          PVOID Parameter,
                                          PVOID *Context) {
      struct init_routine_param* param = (struct init_routine_param*) Parameter;
      (param->init_routine)();
      UNITEX_DISCARD_UNUSED_PARAMETER(InitOnce);
      UNITEX_DISCARD_UNUSED_PARAMETER(Context);
      return TRUE;
    }  // call_once
  }  //  thread_details

  UNITEX_FORCE_INLINE
  bool one_time_initialization(once_flag_t& once_control,
                               void (*init_routine)(void)) {
    struct thread_details::init_routine_param param = { init_routine };
    InitOnceExecuteOnce(&once_control,   // one-time init structure
            thread_details::call_once,   // pointer to callback function
                        (PVOID)&param,   // init routine param to callback
                     UNITEX_NULLPTR);   // doesn't receives pointer to event
    return true;
  }
# else  // !defined(UNITEX_WIN32_FEATURE_INIT_ONCE)
  // 5. InterlockedExchange windows interlocked functions
  UNITEX_FORCE_INLINE
  bool one_time_initialization(once_flag_t& once_control,
                               void (*init_routine)(void)) {

    // Do nothing if once_control or init_routine are null
    if (&once_control == UNITEX_NULLPTR ||
         init_routine == UNITEX_NULLPTR) {
      return false;
    }

    // InterlockedCompareExchange related to  __sync_val_compare_and_swap (GCC)
    // InterlockedExchange related to         __sync_lock_test_and_set    (GCC)

    // Only if init_routine hasn't already been called
    if (once_control.done_ == 0) {
      // If actual lock_ value is 0 (unlock) then atomically copy in it a 1
      // (lock) in this case InterlockedCompareExchange returns 0, otherwise 1
      if(InterlockedCompareExchange(&(once_control.lock_), 1, 0) == 0){
        // If once_control.done_ is 1 (already done),
        // then init_routine has already been called
        if (once_control.done_ == 0) {
          // This function will be called once per instance
          (*init_routine)();
          // Set 1 in done_ to indicate that init_routine has already
          // been called
          InterlockedExchange(&(once_control.done_), 1);
        }
        // Unlock control
        InterlockedExchange(&(once_control.lock_), 0);
      } else {
          // If while condition is true, once_control.lock_ is 1 (already lock)
          // and another thread is calling init_routine
          while (once_control.lock_ == 1) {
#          if defined(UNITEX_WIN32_FEATURE_SWITCH_TO_THREAD)
              SwitchToThread();
#          else
              // Sleep(0) is as an ineffective yield : "Consequences of the
              // scheduling algorithm: Sleeping doesn't always help" (MSDN)
              Sleep(1);
#          endif
          }  // while (&(once_control.lock_) == 1)
      }      // if (InterlockedCompareExchange(&(once_control.lock_), 1, 0)== 0)
    }        // if (!(once_control.done_))

    return true;
  }
# endif  // #if defined(UNITEX_WIN32_FEATURE_INIT_ONCE)
/* ************************************************************************** */
#elif UNITEX_USE(SINGLE_THREADED)
// 6. No thread-safe support
// Naive implementation which has a race condition
// Use an eager initialization to avoid problems
UNITEX_FORCE_INLINE
bool one_time_initialization(once_flag_t& once_control,
                             void (*init_routine)(void)){
  // Do nothing if once_control or init_routine are null
  if (&once_control == UNITEX_NULLPTR ||
       init_routine == UNITEX_NULLPTR) {
    return false;
  }

  // Only if init_routine hasn't already been called
  if (once_control == 0) {
      once_control = 1;
     (*init_routine)();
  }

  return true;
}
#endif  // UNITEX_USE(ISO_THREADS)
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_THREAD_ONCE_H_                               // NOLINT
