/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      size.h
 * @brief     Calculate the size in bits of a data type T
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 size.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_BITS_SIZE_H_                                   // NOLINT
#define UNITEX_BASE_BITS_SIZE_H_                                   // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/integer/integer.h"    // uint32_t, uint64_t, UINT32_C, UINT64_C
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace helper {  // helper
/* ************************************************************************** */
/**
 * @brief  is_bitshiftable
 */
template <typename T>
struct is_bitshiftable {
  static const bool value = (T) 1.5 == 1;
};

/**
 * @brief  is_bitshiftable
 */
template <typename T>
struct is_bitshiftable<T*> {
  static const bool value = false;
};
/* ************************************************************************** */
}  // namespace helper
/* ************************************************************************** */
namespace details {  // details
/* ************************************************************************** */
/**
 * @brief  size_in_bits_impl
 */
template<bool, typename T, uint64_t flag = UINT64_C(1)>
struct size_in_bits_impl {
    static const uint32_t value = UINT32_C(1) +
        size_in_bits_impl<true, T, (uint64_t)(T)(flag << UINT64_C(1))>::value;
};
/**
 * @brief  size in bits 0 specialization
 */
template<typename T>
struct size_in_bits_impl<true, T, UINT64_C(0)> {
    static const uint32_t value = UINT32_C(0);
};

/**
 * @brief  size in bits false specialization
 */
template<typename T>
struct size_in_bits_impl<false, T> {
    static const uint32_t value = sizeof(T) * CHAR_BIT;
};
/* ************************************************************************** */
}  // namespace details
/* ************************************************************************** */
namespace util {  // util
/* ************************************************************************** */
/**
 * @brief  Calculate the size in bits of a data type T
 *
 * @code{.cpp}
 *         size_in_bits<int8_t>::value   //  8
 *         size_in_bits<int32_t>::value  // 32
 *         size_in_bits<int64_t>::value  // 64
 * @endcode
 */
template<typename T>
struct size_in_bits {
    static const uint32_t value = details::size_in_bits_impl<
                                           helper::is_bitshiftable<T>::value,
                                           T>::value;
};
/* ************************************************************************** */
}  // namespace util
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_BITS_SIZE_H_                                // NOLINT

