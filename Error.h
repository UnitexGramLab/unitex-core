/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
/* ************************************************************************** */ 
#ifndef ErrorH
#define ErrorH
/* ************************************************************************** */
#include <cstdlib>                // EXIT_FAILURE, EXIT_SUCCESS
/* ************************************************************************** */
#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Like EXIT_SUCCESS: expands to a system-dependent integral expression that, 
 * when used as the argument for function return, signifies that the 
 * function was successful
 */
#define SUCCESS_RETURN_CODE EXIT_SUCCESS  

/**
 * Like EXIT_FAILURE: expands to a system-dependent integral expression that,
 * when used as the argument for function return, signifies that the
 * function or program failed
 */
#define DEFAULT_ERROR_CODE  EXIT_FAILURE


#define ALLOC_ERROR_CODE    2

/**
 * Like EX_USAGE: The command was used incorrectly, e.g., with the wrong number
 * of arguments, a bad flag, a bad syntax in a parameter, or whatever
 */
#define USAGE_ERROR_CODE    64

/**
 * Codes between 65 and 78 are reserved for future use
 */

/**
 * RunLog tool custom exit codes
 */
#define RUNLOG_COMPARE_WARNING_CODE     79
#define RUNLOG_COMPARE_ERROR_CODE       80


void fatal_error(int);
void fatal_error(int,const char*,...);
void fatal_error(const char*,...);
void fatal_alloc_error(const char*);
void alloc_error(const char*);
void error(const char*,...);
void debug(const char*,...);
void set_debug(char);

#ifdef IGNORE_FATAL_ASSERT
static inline void fatal_assert(int, const char*,...) {
}
#else
void fatal_assert(int condition, const char*, ...);
#endif


} // namespace unitex
/* ************************************************************************** */ 
#endif  // ErrorH
