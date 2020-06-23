/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      check.h
 * @brief     Macro to test DEBUG consistency over NDEBUG and _DEBUG
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 check.h`
 *
 * @date      December 2014
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_DEBUG_CHECK_H_                                  // NOLINT
#define UNITEX_BASE_DEBUG_CHECK_H_                                  // NOLINT
/* ************************************************************************** */
/**
 * @brief    Sanity check DEBUG consistency over NDEBUG and _DEBUG
 *
 * @details  _DEBUG is defined whether /LDd, /MDd, or /MTd are passed to
 *           compiler (MSVC), NDEBUG controls whether assert() are active
 *           or not (assert.h or <cassert> headers)
 */
#if defined(DEBUG) && DEBUG
// If DEBUG is 1 (actived) you need also to  :
//  #undef   NDEBUG
//  #define _DEBUG 1
# if defined(NDEBUG)
#  error "DEBUG is 1 (debug actived) but NDEBUG (debug disabled) was defined"
# endif  // defined(NDEBUG)
# if defined(_DEBUG) && !_DEBUG
#  error "DEBUG is 1 (debug actived) but _DEBUG is 0 (debug disabled)"
# endif  // defined(_DEBUG) && !_DEBUG
#elif defined(DEBUG) && !DEBUG
// If DEBUG is 0 (disabled) you need also to :
//  #define  NDEBUG
//  #define _DEBUG  0
# if !defined(NDEBUG)
#  error "DEBUG is 0 (debug disabled) but NDEBUG (debug disabled) wasn't defined"
# endif  // !defined(NDEBUG)
# if defined(_DEBUG) && _DEBUG
#  error "DEBUG is 0 (debug disabled) but _DEBUG is 1 (debug actived)"
# endif  // defined(_DEBUG) && _DEBUG
#endif // defined(DEBUG) && DEBUG
/* ************************************************************************** */
#endif  // UNITEX_BASE_DEBUG_CHECK_H_                               // NOLINT
