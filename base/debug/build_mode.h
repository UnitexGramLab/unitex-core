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
 * @file      build_mode.h
 * @brief     Macro to test when the build mode is debug or not
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 build_mode.h`
 *
 * @date      December 2014
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_BUILD_MODE_H_                                   // NOLINT
#define UNITEX_BASE_BUILD_MODE_H_                                   // NOLINT
/* ************************************************************************** */
#include "base/debug/check.h"      // DEBUG consistency over NDEBUG and _DEBUG
/* ************************************************************************** */
/**
 * @def    UNITEX_BUILD_MODE_DEBUG
 * @brief  true if DEBUG is defined and true
 * @see    UNITEX_BUILD_MODE_STRING
 */
/**
 * @def    UNITEX_BUILD_MODE_STRING
 * @return "debug"     if  DEBUG
 * @return "release"   if !DEBUG
 * @see    UNITEX_BUILD_MODE_DEBUG
 */
#if defined(DEBUG) && DEBUG
# define UNITEX_BUILD_MODE_DEBUG                1
# define UNITEX_BUILD_MODE_STRING               "Debug"
#else     // !DEBUG
# define UNITEX_BUILD_MODE_RELEASE              1
# define UNITEX_BUILD_MODE_STRING               "Release"
#endif  // DEBUG
/* ************************************************************************** */
/**
 * @brief  Test when the build mode is debug or not
 *
 * @code{.cpp}
 *          UNITEX_BUILD_MODE(DEBUG)   // true if DEBUG
 *          UNITEX_BUILD_MODE(RELEASE) // true if !DEBUG
 * @endcode
 */
#define UNITEX_BUILD_MODE(BuildMode)   \
        (defined(UNITEX_BUILD_MODE_##BuildMode) &&\
                 UNITEX_BUILD_MODE_##BuildMode)
/* ************************************************************************** */
#endif  // UNITEX_BASE_BUILD_MODE_H_                                // NOLINT
