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
 * @file      common.h
 * @brief     Unitex Base common header
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 common.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMMON_H_                                       // NOLINT
#define UNITEX_BASE_COMMON_H_                                       // NOLINT
/* ************************************************************************** */
#include <assert.h>                     // assert
#include <stdlib.h>                     // NULL, size_t
#include <cstring>                      // strcpy, strtok_r, strlen
/* ************************************************************************** */
#include "base/config.h"                // base configuration
/* ************************************************************************** */
#include "api/api.h"                    // API-related macros
#include "bits/bits.h"                  // Bit-related operations
#include "boolean/boolean.h"            // Boolean-related types and operations
#include "compiler/compiler.h"          // Compiler-related macros
#include "cpu/cpu.h"                    // CPU-related macros
#include "debug/debug.h"                // Debug-related macros and functions
#include "file/file.h"                  // File-related types and functions
#include "integer/integer.h"            // Integer-related literals and types
#include "macro/macro.h"                // Macro helpers
#include "os/os.h"                      // OS-related macros
#include "preprocessor/preprocessor.h"  // Minimalist preprocessor library
#include "string/string.h"              // String-related types and functions
#include "thread/thread.h"              // Portable thread types and functions
#include "time/time.h"                  // Portable time-related functions
#include "unilog/unilog.h"              // Unicode-aware message logging
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMMON_H_                                    // NOLINT
