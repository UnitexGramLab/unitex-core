/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      do_nothing.h
 * @brief     Tells the compiler that some variables would be not used
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 *
 * @attention Do not include this file directly, rather include the
 *            base/compiler/attributes.h header file to gain this file's
 *            functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 do_nothing.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_MACRO_HELPER_DO_NOTHING_H_                      // NOLINT
#define UNITEX_BASE_MACRO_HELPER_DO_NOTHING_H_                      // NOLINT
/* ************************************************************************** */
#include "base/macro/helper/decls.h"  // UNITEX_MACRO_DECLS_BEGIN
/* ************************************************************************** */
/**
 * @brief  Does nothing statement
 *
 * @code{.cpp}
 *  UNITEX_DO_NOTHING;  // Does nothing statement
 * @endcode
 */
# define UNITEX_DO_NOTHING   UNITEX_MACRO_DECLS_BEGIN \
                             UNITEX_MACRO_DECLS_END
/* ************************************************************************** */
#endif  // UNITEX_BASE_MACRO_HELPER_DO_NOTHING_H_                   // NOLINT
