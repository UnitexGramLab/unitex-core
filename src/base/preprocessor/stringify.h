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
 * @file      stringify.h
 * @brief     Preprocessor macros to convert macro names or values into string
 *            constants
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      For more details about the stringification process in C
 *            visit http://gcc.gnu.org/onlinedocs/cpp/Stringification.html
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 stringify.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_PREPROCESSOR_STRINGIFY_H_                       // NOLINT
#define UNITEX_BASE_PREPROCESSOR_STRINGIFY_H_                       // NOLINT
/* ************************************************************************** */
/**
 * @brief  Convert a single macro value into a string constant (surrounded by quotes)
 *
 * e.g.
 * @code{.cpp}
 *         #define FOO UNITEX_PPSTRINGIFY_NAME(__TIME__)
 * @endcode
 * FOO will be literally `"__TIME__"`
 */
#define UNITEX_PP_STRINGIFY_HELPER(s) #s
#define UNITEX_PP_STRINGIFY(s) UNITEX_PP_STRINGIFY_HELPER(s)

/**
 * @brief  Convert a macro name into a string constant (surrounded by quotes)
 *
 * e.g.
 * @code{.cpp}
 *         #define FOO UNITEX_PPSTRINGIFY_NAME(__TIME__)
 * @endcode
 * FOO will be literally `"__TIME__"`
 */
#define UNITEX_PP_STRINGIFY_NAME(...) #__VA_ARGS__

/**
 * @brief  Convert a macro value into a string constant (surrounded by quotes)
 *
 * e.g.
 * @code{.cpp}
 *         #define FOO UNITEX_PPSTRINGIFY_VALUE(__TIME__)
 * @endcode
 * FOO will be literally `"15:17:05"`
 */
#define UNITEX_PP_STRINGIFY_VALUE(...) \
        UNITEX_PP_STRINGIFY_NAME(__VA_ARGS__)
/* ************************************************************************** */
#endif  // UNITEX_BASE_PREPROCESSOR_STRINGIFY_H_                    // NOLINT
