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
 * @file      config.h
 * @brief     Unitex Base Config
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 config.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_CONFIG_H_                                       // NOLINT
#define UNITEX_BASE_CONFIG_H_                                       // NOLINT
/* ************************************************************************** */
// <signal.h> is available
#define HAVE_SIGNAL_H                           1

// clock_gettime function is available
#define HAVE_CLOCK_GETTIME                      1

// localtime_r function is available
//  HAVE_LOCALTIME_R

// UNITEX_WIN32_FEATURE_TIME_PRECISE

// boost thread library is installed
//  HAVE_BOOST_THREAD

// C++ Technical Report 1 libraries
//  HAVE_STD_TR1

// syscall function is available
//  HAVE_SYSCALL

// __MMX__    MMX:   MultiMedia eXtensions
// __SSE__    SSE:   Streaming SIMD Extensions instructions
// __SSE2__   SSE2:  Streaming SIMD Extensions 2 instructions
// __SSE3__   SSE3:  Streaming SIMD Extensions 3 instructions
// __SSSE3__  SSSE3: Supplemental Streaming SIMD Extensions 3 instructions
// __SSE4_1__ SSE41: Streaming SIMD Extensions 4.1 instructions
// __SSE4_2__ SSE42: Streaming SIMD Extensions 4.2 instructions
// __SSE5__   SSE5:  Streaming SIMD Extensions 5 instructions

// <linux/version.h> is available
#define HAVE_LINUX_VERSION_H                    1

// Use threads
#define USE_THREADS                             1
/* ************************************************************************** */
#endif  // UNITEX_BASE_CONFIG_H_                                    // NOLINT
