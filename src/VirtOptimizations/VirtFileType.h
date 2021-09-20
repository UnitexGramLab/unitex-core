/*
 * Unitex - Performance optimization code 
 *
 * File created and contributed by Gilles Vollant, working with François Liger
 * as part of an UNITEX optimization and reliability effort, first descibed at
 * http://www.smartversion.com/unitex-contribution/Unitex_A_NLP_engine_from_the_lab_to_the_iPhone.pdf
 *
 * Free software when used with Unitex 3.2 or later
 *
 * Copyright (C) 2021-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#ifndef VIRTUAL_FILE_TYPE_DEFINED
#define VIRTUAL_FILE_TYPE_DEFINED 1

#include "FuncDeclModifier.h"

#ifndef BOOL

  #if (((defined(__llvm__)) || defined(__clang__)))
    // if we are in Apple world and BOOL not already defined, we copy the apple definition
    #ifndef OBJC_BOOL_DEFINED

      /// Type to represent a boolean value.
      #if !defined(OBJC_HIDE_64) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE && __LP64__
        typedef bool BOOL; // introduced only for iOS 64 bits
      #else
        typedef signed char BOOL; // for Apple 32 bits and MacOsx 64 bits...
      #endif
    #endif

  #else
  #if (((defined(_WIN32)) || defined(WIN32)))
  typedef int BOOL;
  #else
  typedef signed char BOOL;
  #endif
#endif
#endif


#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#endif
