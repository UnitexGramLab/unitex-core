/*
 * Unitex
 *
 * Copyright (C) 2001-2023 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      util.h
 * @brief     String macros helpers
 *
 * @author    cristian.martinez (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 util.h`
 *
 * @date      December 2023
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_STRING_UTIL_H_                            // NOLINT
#define UNITEX_BASE_STRING_UTIL_H_                            // NOLINT
/* ************************************************************************** */
// Unitex headers
/* ************************************************************************** */
/**
 * @brief Calculate the length of a string at compile time.
 *
 * This macro calculates the length of a string literal, defined by another macro,
 * at compile time. It works by using the sizeof operator to get the size of the
 * string literal (including the null terminator) and then subtracts 1 to exclude
 * the null terminator from the count.
 *
 * e.g.
 * @code{.cpp}
 *         #define MY_STRING "Hello, world!"
 *         #define MY_STRING_LENGTH UNITEX_PP_STRLEN(MY_STRING)
 * @endcode
 * MY_STRING_LENGTH will be evaluated at compile time to the length of "Hello, world!"
 * (excluding the null terminator).
 */
#define UNITEX_PP_STRLEN(s) (sizeof(s) - 1)

/**
 * @brief Converts a character to lowercase if it is uppercase, otherwise leaves it unchanged.
 *
 * This macro checks if a character is an uppercase letter (A-Z). If it is,
 * it converts it to its lowercase equivalent (a-z). If the character is
 * already lowercase or not a letter, it remains unchanged.
 *
 * e.g.
 * @code{.cpp}
 *         char lower = UNITEX_PP_CHAR_LOWERCASE('A');     // lower will be 'a'
 *         char unchanged = UNITEX_PP_CHAR_LOWERCASE('b'); // unchanged will still be 'b'
 * @endcode
 */
#define UNITEX_PP_CHAR_LOWERCASE(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) - 'A' + 'a') : (c))

/**
 * @brief Converts a character to uppercase if it is lowercase, otherwise leaves it unchanged.
 *
 * This macro checks if a character is a lowercase letter (a-z). If it is,
 * it converts it to its uppercase equivalent (A-Z). If the character is
 * already uppercase or not a letter, it remains unchanged.
 *
 * e.g.
 * @code{.cpp}
 *         char upper = UNITEX_PP_CHAR_UPPERCASE('b');     // upper will be 'B'
 *         char unchanged = UNITEX_PP_CHAR_UPPERCASE('C'); // unchanged will still be 'C'
 * @endcode
 */
#define UNITEX_PP_CHAR_UPPERCASE(c) (((c) >= 'a' && (c) <= 'z') ? ((c) - 'a' + 'A') : (c))

/* ************************************************************************** */
#endif  // UNITEX_BASE_STRING_UTIL_H_                         // NOLINT
