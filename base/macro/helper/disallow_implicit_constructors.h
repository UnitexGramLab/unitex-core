/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      disallow_implicit_constructors.h
 * @brief     Disallow all the implicit constructors
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @note      This is from the Google C++ Style Guide
 * @see       http://google-styleguide.googlecode.com/svn/trunk/cppguide.html
 *
 *
 * @attention Do not include this file directly, rather include the
 *            base/compiler/common.h header file to gain this file's
 *            functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 disallow_implicit_constructors.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_MACRO_HELPER_DISALLOW_IMPLICIT_CONSTRUCTORS_H_  // NOLINT
#define UNITEX_BASE_MACRO_HELPER_DISALLOW_IMPLICIT_CONSTRUCTORS_H_  // NOLINT
/* ************************************************************************** */
#include "base/compiler/keyword/eq_delete.h"             // UNITEX_EQ_DELETE
#include "base/macro/helper/disallow_copy_and_assign.h"  // UNITEX_DISALLOW_COPY_AND_ASSIGN
/* ************************************************************************** */
/**
 * @brief  disallow all the implicit constructors, namely the default constructor,
 *         copy constructor and operator= functions
 *
 * This should be used in the private: declarations for a class that wants to
 * prevent anyone from instantiating it. This is especially useful for classes
 * containing only static methods.
 *
 * @see    UNITEX_DISALLOW_COPY_AND_ASSIGN
 */
# define UNITEX_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName)  \
    TypeName() UNITEX_EQ_DELETE;                          \
    UNITEX_DISALLOW_COPY_AND_ASSIGN(TypeName)
/* ************************************************************************** */
#endif  // UNITEX_BASE_MACRO_HELPER_DISALLOW_IMPLICIT_CONSTRUCTORS_H_  // NOLINT
