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
/**
 * @file      Unitex_revision.h
 * @brief     Defines the current revision of the Unitex Core
 * @date      November 06, 2015
 */
/* ************************************************************************** */
#ifndef UNITEX_CORE_UNITEX_REVISION_H                               // NOLINT
#define UNITEX_CORE_UNITEX_REVISION_H                               // NOLINT
/* ************************************************************************** */
#include "Version.h"
/* ************************************************************************** */
#define QUOTE(s) "\""#s"\""
/* ************************************************************************** */
#if defined(UNITEX_VERSION_BUILD_IS_ANONYMOUS) &&\
          (!UNITEX_VERSION_BUILD_IS_ANONYMOUS)
# define UNITEX_REVISION               UNITEX_VERSION_REVISION_NUMBER
# define UNITEXREVISION                UNITEX_VERSION_REVISION_NUMBER
# define UNITEX_REVISION_TEXT          QUOTE(UNITEX_VERSION_REVISION_NUMBER)
#else  // Anonymous Build
# define UNITEX_REVISION               (-1)
# define UNITEXREVISION                (-1)
# define UNITEX_REVISION_TEXT          "-1"
#endif  // defined(UNITEX_VERSION_BUILD_IS_ANONYMOUS)
/* ************************************************************************** */
#undef QUOTE
/* ************************************************************************** */
#endif  // UNITEX_CORE_UNITEX_REVISION_H                            // NOLINT
