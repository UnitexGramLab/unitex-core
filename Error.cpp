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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "Unicode.h"
#include "UnusedParameter.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#ifdef DEBUG
static char DEBUG_ON=0;
#endif

/**
 * Exits the program with the given exit code.
 */
void fatal_error(int error_code) {
exit(error_code);
}


/**
 * Prints the given message and
 * exits the program with the given exit code.
 */
void fatal_error(int error_code,const char* format,...) {
va_list list;
va_start(list,format);
u_vfprintf(U_STDERR,format,list);
va_end(list);
fatal_error(error_code);
}


/**
 * Prints the given message and
 * exits the program with the default exit code.
 */
void fatal_error(const char* format,...) {
va_list list;
va_start(list,format);
u_vfprintf(U_STDERR,format,list);
va_end(list);
fatal_error(DEFAULT_ERROR_CODE);
}


/**
* If assert_condition is not 0, prints the given message and
* exits the program with the default exit code.
*/
#ifndef IGNORE_FATAL_ASSERT
void fatal_assert(int assert_condition, const char* format, ...) {
if (assert_condition) {
  va_list list;
  va_start(list, format);
  u_vfprintf(U_STDERR, format, list);
  va_end(list);
  fatal_error(DEFAULT_ERROR_CODE);
}
}
#endif


/**
 * Prints the given message on the error stream.
 */
void error(const char* format,...) {
va_list list;
va_start(list,format);
u_vfprintf(U_STDERR,format,list);
va_end(list);
}


/**
 * Prints the given message on the error stream, but only if
 * we are in debug mode.
 */
void debug(const char* format,...) {
#ifdef DEBUG
if (!DEBUG_ON) return;
va_list list;
va_start(list,format);
u_vfprintf(U_STDERR,format,list);
va_end(list);
#else
DISCARD_UNUSED_PARAMETER(format)
#endif
}


/**
 * Sets debug mode.
 */
void set_debug(char mode) {
#ifdef DEBUG
DEBUG_ON=mode;
#else
DISCARD_UNUSED_PARAMETER(mode)
#endif
}

/**
 * Raises a memory allocation error. 'function' is the name
 * of the function where the error occurred
 */
void alloc_error(const char* function) {
  error("Not enough memory in %s\n",function);
}

/**
 * Raises a fatal memory allocation error. 'function' is the name
 * of the function where the error occurred.
 */
void fatal_alloc_error(const char* function) {
fatal_error(ALLOC_ERROR_CODE,"Not enough memory in %s\n",function);
}

} // namespace unitex
