 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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


#define DEFAULT_ERROR_CODE 1


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
void fatal_error(int error_code,char* fmt,...) {
  va_list plist;
  va_start(plist,fmt);
  vfprintf(stderr,fmt,plist);
  va_end(plist);
  fatal_error(error_code);
}


/**
 * Prints the given message and
 * exits the program with the default exit code.
 */
void fatal_error(char* fmt,...) {
  va_list plist;
  va_start(plist,fmt);
  vfprintf(stderr,fmt,plist);
  va_end(plist);
  fatal_error(DEFAULT_ERROR_CODE);
}
