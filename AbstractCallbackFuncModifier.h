 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef _ABSTRACT_CALLBACK_FUNC_MODIFIER_INCLUDED
#define _ABSTRACT_CALLBACK_FUNC_MODIFIER_INCLUDED 1



#if (((defined(_WIN32)) || defined(WIN32)) && (!(defined(NO_WIN32_DLLLIKE_FUNCSPEC))))


#ifndef ABSTRACT_CALLBACK_UNITEX_DEFINED
#define ABSTRACT_CALLBACK_UNITEX_DEFINED
#define ABSTRACT_CALLBACK_UNITEX __stdcall
#endif

#ifndef UNITEX_FUNC_DEFINED
#define UNITEX_FUNC_DEFINED

#ifdef UNITEX_FUNC_EXPORT_DLL
#define UNITEX_FUNC __declspec(dllexport)
#else
#ifdef UNITEX_FUNC_IMPORT_DLL
#define UNITEX_FUNC __declspec(dllimport)
#else
#define UNITEX_FUNC
#endif
#endif
#endif


#ifndef UNITEX_CALL_DEFINED
#define UNITEX_CALL_DEFINED
#define UNITEX_CALL __stdcall
#endif


#else



#ifndef ABSTRACT_CALLBACK_UNITEX_DEFINED
#define ABSTRACT_CALLBACK_UNITEX_DEFINED
#define ABSTRACT_CALLBACK_UNITEX
#endif

#ifndef UNITEX_FUNC_DEFINED
#define UNITEX_FUNC_DEFINED
#define UNITEX_FUNC
#endif

#ifndef UNITEX_CALL_DEFINED
#define UNITEX_CALL_DEFINED
#define UNITEX_CALL
#endif


#endif


#endif
