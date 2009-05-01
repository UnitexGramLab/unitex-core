 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef FUNC_DECL_MODIFIER_H
#define FUNC_DECL_MODIFIER_H 1

#if (((defined(_WIN32)) || defined(WIN32)))
#define ULIB__EXPORT __declspec(dllexport)
#define ULIB__IMPORT __declspec(dllimport)
#define ULIB_CALL __stdcall

#define ABSTRACT_CALLBACK_UNITEX __stdcall


#else

#define ABSTRACT_CALLBACK_UNITEX



#define ULIB__EXPORT
#define ULIB__IMPORT
#define ULIB_CALL

#endif


#ifdef UNITEX_LIBRARY_VF
#define ULB_VFFUNC ULIB__EXPORT
#else
#ifdef UNITEX_LIBRARY_VF__IMPORT
#define ULB_VFFUNC ULIB__IMPORT
#else
#define ULB_VFFUNC
#endif
#endif


#ifdef UNITEX_LIBRARY_CORE
#define ULB_FUNC ULIB__EXPORT
#else
#ifdef UNITEX_LIBRARY_CORE_IMPORT
#define ULB_FUNC ULIB__IMPORT
#else
#define ULB_FUNC
#endif
#endif

#endif

