/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * @file      encode.h
 * @brief     Functions to encode/decode unitex unicode strings
 *
 * @author    cristian.martinez@unitexgramlab.org (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/common.h header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors
 *
 * @date      September 2016, October 2021
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_UNICODE_ENCODE_H_                                // NOLINT
#define UNITEX_BASE_UNICODE_ENCODE_H_                                // NOLINT
/* ************************************************************************** */
#include "base/unicode/utf8.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
/**
 *
 *
 * @param source
 * @param destination
 * @return the length of the destination string
 */
size_t u_encode_utf8(const unichar* source, char* destination) UNITEX_PARAMS_NON_NULL;

/**
 *
 *
 * @param source
 * @param destination
 * @return
 */
size_t u_decode_utf8(const char* source, unichar* destination) UNITEX_PARAMS_NON_NULL;

/**
 *
 * @param source
 * @param destination
 * @param n
 * @return
 */
size_t u_decode_utf8_n(const char* source, unichar* destination, size_t n) UNITEX_PARAMS_NON_NULL;
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_UNICODE_ENCODE_H_                              // NOLINT
