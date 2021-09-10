/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      functions.cpp
 * @brief     Simple Portable Time Class
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @ingroup   Helper Classes
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `curl -L https://git.io/vV1N8 -o cpplint.py ; chmod +x cpplint.py
 *            ; cpplint.py --linelength=120 functions.h`
 *
 * @date      August 2021
 */
/* ************************************************************************** */
// Header for this file
#include "base/time/functions.h"
/* ************************************************************************** */
// Cobalto headers
// nothing
/* ************************************************************************** */
// C system files                  (try to order the includes alphabetically)
// nothing
/* ************************************************************************** */
// C++ system files                (try to order the includes alphabetically)
// nothing
/* ************************************************************************** */
// Other libraries' .h files       (try to order the includes alphabetically)
// nothing
/* ************************************************************************** */
// Project's .h files              (try to order the includes alphabetically)
// nothing
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
# if defined(UNITEX_UNIX_FEATURE_MACH_CLOCK_GETTIME)
/* ************************************************************************** */
namespace helper {   // unitex::helper
/* ************************************************************************** */
uint32_t MachClockService::count_ = 0;
clock_serv_t MachClockService::clock_service_ = 0;
/* ************************************************************************** */
}  // namespace unitex::helper                                      // NOLINT
/* ************************************************************************** */
#endif  // defined(UNITEX_UNIX_FEATURE_MACH_CLOCK_GETTIME)
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
