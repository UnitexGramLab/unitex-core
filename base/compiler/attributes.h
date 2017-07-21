/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      attributes.h
 * @brief     Standard compilers attributes
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 attributes.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_ATTRIBUTES_H_                          // NOLINT
#define UNITEX_BASE_COMPILER_ATTRIBUTES_H_                          // NOLINT
/* ************************************************************************** */
// Project's .h files              (try to order the includes alphabetically)
#include "base/compiler/attribute/alignas.h"
#include "base/compiler/attribute/const.h"
#include "base/compiler/attribute/dll_visibility.h"
#include "base/compiler/attribute/inline.h"
#include "base/compiler/attribute/nonnull.h"
#include "base/compiler/attribute/noreturn.h"
#include "base/compiler/attribute/malloc.h"
#include "base/compiler/attribute/printf_format.h"
#include "base/compiler/attribute/pure.h"
#include "base/compiler/attribute/sentinel.h"
#include "base/compiler/attribute/unused.h"
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_ATTRIBUTES_H_                       // NOLINT
