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
 * @file      chrono.cpp
 * @brief     Simple Portable Time Class
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @ingroup   Helper Classes
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      For more details about the stringification process in C
 *            visit http://gcc.gnu.org/onlinedocs/cpp/Stringification.html
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `curl -L https://git.io/vV1N8 -o cpplint.py ; chmod +x cpplint.py
 *            ; cpplint.py --linelength=120 chrono.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://unitex.amabis.fr)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
// Header for this file
#include "base/time/chrono.h"
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
const Chrono Chrono::Zero;
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
