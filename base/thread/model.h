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
 * @file      model.h
 * @brief     Thread model selection
 *
 * @code{.cpp}
 *       UNITEX_USE(ISO_THREADS)    // C++11 (not experimental) compilers
 *       UNITEX_USE(BOOST_THREADS)  // Boost thread library
 *       UNITEX_USE(POSIX_THREADS)  // Unix-like mostly POSIX operating systems
 *       UNITEX_USE(WIN32_THREADS)  // Windows operating systems
 *       UNITEX_USE(SINGLE_THREADED)  // Single thread support
 * @endcode
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 model.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_THREAD_MODEL_H_                                // NOLINT
#define UNITEX_BASE_THREAD_MODEL_H_                                // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/config.h"             // USE_*_THREADS, HAVE_BOOST_THREAD
#include "base/compiler/compiler.h"  // UNITEX_COMPILER_COMPLIANT
#include "base/integer/types.h"      // Portable integer types: uint64_t
#include "base/macro/helper/test.h"  // UNITEX_HAVE, UNITEX_USE
#include "base/os/os.h"              // UNITEX_OS_IS
/* ************************************************************************** */
// Select a threading model
#if  UNITEX_USE(THREADS)
# if   UNITEX_USE(ISO_THREADS)                      // ISO_THREADS
#  if  UNITEX_COMPILER_COMPLIANT(CXX11)
#       include <mutex>
#       include <sstream>
#       include <thread>
# else    // !UNITEX_COMPILER_COMPLIANT(CX11)
#       error "[Co] UNITEX_USE_ISO_THREADS without UNITEX_COMPILER_COMPLIANT_CXX11"
# endif  // UNITEX_COMPILER_COMPLIANT(CXX11)
# elif UNITEX_USE(BOOST_THREADS)                    // BOOST_THREADS
#  if   UNITEX_HAVE(BOOST_THREAD)
#       include <boost/thread.hpp>
#  else    // !UNITEX_HAVE(BOOST_THREAD)
#       error "[Co] UNITEX_USE_BOOST_THREADS without HAVE_BOOST_THREAD"
#  endif  // UNITEX_HAVE(BOOST_THREAD)
# elif UNITEX_USE(POSIX_THREADS)                    // POSIX_THREADS
#  if  UNITEX_OS_IS(UNIX)
#       include <pthread.h>
#  else     //UNITEX_OS_IS(UNIX)
#       error "[Co] UNITEX_USE_POSIX_THREADS without UNITEX_OS_IS_UNIX"
#  endif  // UNITEX_OS_IS(UNIX)
# elif UNITEX_USE(WIN32_THREADS)                    // WIN32_THREADS
#  if  UNITEX_OS_IS(WINDOWS)
#   include <windows.h>
#  else    // !UNITEX_OS_IS(WINDOWS)
#       error "[Co] UNITEX_USE_WIN32_THREADS without UNITEX_OS_IS_WINDOWS"
#  endif  // UNITEX_OS_IS(WINDOWS)
# else  // No preselect threading model detected
#  if   UNITEX_COMPILER_COMPLIANT(CXX11)
#        include <mutex>
#        include <sstream>
#        include <thread>
#        define UNITEX_USE_ISO_THREADS           1  // ISO_THREADS
#  elif UNITEX_HAVE(BOOST_THREAD)
#        include <boost/thread.hpp>
#        define UNITEX_USE_BOOST_THREADS         1  // BOOST_THREADS
#  elif UNITEX_OS_IS(UNIX)
#        include <pthread.h>
#        define UNITEX_USE_POSIX_THREADS         1  // POSIX_THREADS
#  elif UNITEX_OS_IS(WINDOWS)
#        include <windows.h>
#        define UNITEX_USE_WIN32_THREADS         1  // WIN32_THREADS
#  else     // !UNITEX_COMPILER_COMPLIANT(CXX11)
#        define UNITEX_USE_SINGLE_THREADED       1  // SINGLE_THREADED
#  endif   // UNITEX_COMPILER_COMPLIANT(CXX11)
# endif   // UNITEX_USE(ISO_THREADS) ...
# else   // ! UNITEX_USE(THREADS)
#        define UNITEX_USE_SINGLE_THREADED       1  // SINGLE_THREADED
#endif  // UNITEX_USE(THREADS)
/* ************************************************************************** */
#endif  // UNITEX_BASE_THREAD_MODEL_H_                             // NOLINT
