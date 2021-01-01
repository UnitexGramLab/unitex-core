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
#ifndef FUNC_DECL_MODIFIER_H
#define FUNC_DECL_MODIFIER_H 1

#if (((defined(_WIN32)) || defined(WIN32))) && (!(defined(NO_DLL_DECLARATION))) && (!(defined(NO_WIN32_DLLLIKE_FUNCSPEC)))
#define ULIB__EXPORT __declspec(dllexport)
#define ULIB__IMPORT __declspec(dllimport)
#define ULIB_CALL __stdcall


#else

#if defined(__GCC__)
#if __GCC__ >= 4
#define GCC_V4_or_upper
#endif
#endif


#if defined(__GNUC__) && (!defined(GCC_V4_or_upper))
#if __GNUC__ >= 4
#define GCC_V4_or_upper
#endif
#endif

#ifdef GCC_V4_or_upper
#define ULIB__EXPORT __attribute__ ((visibility("default")))
#define ULIB__IMPORT __attribute__ ((visibility("default")))
#define ULIB_CALL
#else
#define ULIB__EXPORT
#define ULIB__IMPORT
#define ULIB_CALL
#endif

#endif


#ifdef UNITEX_LIBRARY_VF
#define ULB_VFFUNC ULIB__EXPORT
#else
#ifdef UNITEX_LIBRARY_VF_IMPORT
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
