/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @brief     Macro to test when the elg build mode is debug or not
 *
 * @author    Cristian Martinez
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 build_mode.h`
 *
 * @date      July 2021
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_ELG_BUILD_MODE_H_                               // NOLINT
#define UNITEX_BASE_ELG_BUILD_MODE_H_                               // NOLINT
/* ************************************************************************** */
#include "base/debug/check.h"      // DEBUG consistency over NDEBUG and _DEBUG
/* ************************************************************************** */
/**
 * @def    ELG_BUILD_MODE_DEBUG
 * @brief  true if ELG_DEBUG is defined and true
 * @see    ELG_BUILD_MODE_STRING
 */
/**
 * @def    ELG_BUILD_MODE_STRING
 * @return "ELG Engine Debug"     if  ELG_DEBUG
 * @return "ELG Engine Release"   if !ELG_DEBUG
 * @see    ELG_BUILD_MODE_DEBUG
 */
#if defined(ELG_DEBUG) && ELG_DEBUG
# define ELG_BUILD_MODE_DEBUG                1
# define ELG_BUILD_MODE_STRING               "ELG Engine Debug"
#else     // !ELG_DEBUG
# define ELG_BUILD_MODE_RELEASE              1
# define ELG_BUILD_MODE_STRING               "ELG Engine Release"
#endif  // ELG_DEBUG
/* ************************************************************************** */
/**
 * @brief  Test when the build mode is debug or not
 *
 * @code{.cpp}
 *          ELG_BUILD_MODE(DEBUG)   // true if ELG_DEBUG
 *          ELG_BUILD_MODE(RELEASE) // true if !ELG_DEBUG
 * @endcode
 */
#define ELG_BUILD_MODE(BuildMode)   \
        (ELG_BUILD_MODE_##BuildMode == 1)
/* ************************************************************************** */
#endif  // UNITEX_BASE_ELG_BUILD_MODE_H_                            // NOLINT
