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
 * @file      literal.h
 * @brief     Binary Constants
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @see       Binary Literals in the C++ Core Language (C++14/n3472)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 literal.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_BITS_LITERAL_H_                                 // NOLINT
#define UNITEX_BASE_BITS_LITERAL_H_                                 // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/compiler/compiler.h"
#include "base/integer/integer.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace details {  // details
/* ************************************************************************** */
/**
 * @brief  binary literal
 * @see    binary
 */
template<uint64_t N>
struct binary_ {
    static const uint64_t
      value = (N % UINT64_C(8)) + UINT64_C(2) * binary_<N / UINT64_C(8)>::value;
    static const uint32_t
      size  =                     UINT32_C(1) + binary_<N / UINT64_C(8)>::size;
};

/**
 * @brief  binary literal zero specialization
 * @see    binary
 */
template<>
struct binary_<0> {
    static const uint64_t value = UINT64_C(0);
    static const uint32_t size  = UINT32_C(0);
};

/**
 * @brief  mono-byte binary literal
 * @note   byte1 need to be always in octal notation (0n)
 * @see    binary1
 */
template <uint64_t byte1>
struct binary1_ {
    static const uint8_t  value = binary_<byte1>::value << UINT8_C( 0);
    static const uint32_t size  = UINT32_C(8);
};

/**
 * @brief  dual-byte binary literal
 * @note   byte1 and byte2 need to be always in octal notation (0n)
 * @see    binary2
 */
template <uint64_t byte1,
          uint64_t byte2>
struct binary2_ {
    static const uint16_t value = binary_<byte1>::value << UINT16_C( 8) |
                                  binary_<byte2>::value << UINT16_C( 0);
    static const uint32_t size  = UINT32_C(16);
};

/**
 * @brief  quad-byte binary literal
 *
 * @note   binary4 struct was adapted from Ofek Shilon's binary4 snnipet
 * @see    http://ofekshilon.com/2009/04/10/coding-binary-as-binary/
 *
 * @note   byte1 ... byte4 need to be always in octal notation (0n)
 * @see    binary4
 */
template <uint64_t byte1,
          uint64_t byte2,
          uint64_t byte3,
          uint64_t byte4>
struct binary4_ {
    static const uint32_t value = binary_<byte1>::value << UINT32_C(24) |
                                  binary_<byte2>::value << UINT32_C(16) |
                                  binary_<byte3>::value << UINT32_C( 8) |
                                  binary_<byte4>::value << UINT32_C( 0);
    static const uint32_t size  = UINT32_C(32);
};

/**
 * @brief  octo-byte binary literal
 * @note   byte1 ... byte8 need to be always in octal notation (0n)
 * @see    binary8
 */
template <uint64_t byte1,
          uint64_t byte2,
          uint64_t byte3,
          uint64_t byte4,
          uint64_t byte5,
          uint64_t byte6,
          uint64_t byte7,
          uint64_t byte8>
struct binary8_ {
    static const uint64_t value = binary_<byte1>::value << UINT64_C(56) |
                                  binary_<byte2>::value << UINT64_C(48) |
                                  binary_<byte3>::value << UINT64_C(40) |
                                  binary_<byte4>::value << UINT64_C(32) |
                                  binary_<byte5>::value << UINT64_C(24) |
                                  binary_<byte6>::value << UINT64_C(16) |
                                  binary_<byte7>::value << UINT64_C( 8) |
                                  binary_<byte8>::value << UINT64_C( 0);
    static const uint32_t size  = UINT32_C(64);
};

#if UNITEX_HAVE(UINT128_T)
/**
 * @brief  hexa-byte binary literal
 * @note   byte1 ... byte16 need to be always in octal notation (0n)
 * @see    binary16
 */
template <uint64_t byte1,
          uint64_t byte2,
          uint64_t byte3,
          uint64_t byte4,
          uint64_t byte5,
          uint64_t byte6,
          uint64_t byte7,
          uint64_t byte8,
          uint64_t byte9,
          uint64_t byte10,
          uint64_t byte11,
          uint64_t byte12,
          uint64_t byte13,
          uint64_t byte14,
          uint64_t byte15,
          uint64_t byte16>
struct binary16_ {
    static const uint64_t h_value =  binary_<byte1>::value  << UINT64_C(56)  |
                                     binary_<byte2>::value  << UINT64_C(48)  |
                                     binary_<byte3>::value  << UINT64_C(40)  |
                                     binary_<byte4>::value  << UINT64_C(32)  |
                                     binary_<byte5>::value  << UINT64_C(24)  |
                                     binary_<byte6>::value  << UINT64_C(16)  |
                                     binary_<byte7>::value  << UINT64_C( 8)  |
                                     binary_<byte8>::value  << UINT64_C( 0);
    static const uint64_t l_value =  binary_<byte9>::value  << UINT64_C( 56) |
                                     binary_<byte10>::value << UINT64_C( 48) |
                                     binary_<byte11>::value << UINT64_C( 40) |
                                     binary_<byte12>::value << UINT64_C( 32) |
                                     binary_<byte13>::value << UINT64_C( 24) |
                                     binary_<byte14>::value << UINT64_C( 16) |
                                     binary_<byte15>::value << UINT64_C(  8) |
                                     binary_<byte16>::value << UINT64_C(  0);


    static const uint128_t value  =  h_value << UINT64_C(64) | l_value;
    static const uint32_t  size   =  UINT32_C(128);
};
#endif  // UNITEX_HAVE(UINT128_T)
/* ************************************************************************** */
}  // namespace details
/* ************************************************************************** */
/**
 * @brief  binary literal
 *
 * @code{.cpp}
 *         binary(1111111111111111111111)
 * @endcode
 *
 * @note   The max possible binary(n) representation, using a uint64_t(n)
 *         is 2^22 - 1, i.e. `binary(1111111111111111111111)`, after this
 *         the compiler will throw the message "warning: integer constant
 *          is too large for its type". In this case, you need to use
 *         `quad_byte`, `octo_byte` or `hexa_byte` macros
 *
 * @see    mono_byte
 * @see    dual_byte
 * @see    quad_byte
 * @see    octo_byte
 * @see    hexa_byte
 */
#define binary(n)   unitex::details::binary_<0##n>::value

/**
 * @brief  mono-byte binary literal
 *
 * @code{.cpp}
 *         binary1(11111111)
 * @endcode
 *
 * @see    mono_byte
 */
#define binary1(byte1)                                            \
                    unitex::details::binary1_<0##byte1>::value

/**
 * @brief  mono-byte binary literal
 *
 * @code{.cpp}
 *         mono_byte(11111111)
 * @endcode
 *
 * @note  This is an alias for `binary1`
 */
#define mono_byte(byte1)                                          \
                    unitex::details::binary1_<0##byte1>::value


/**
 * @brief  dual-byte binary literal
 *
 * @code{.cpp}
 *         binary2(11111111,11111111)
 * @endcode
 *
 * @see    dual_byte
 */
#define binary2(byte1, byte2)                                     \
                    unitex::details::binary2_<0##byte1,           \
                                               0##byte2>::value

/**
 * @brief  dual-byte binary literal
 *
 * @code{.cpp}
 *         dual_byte(11111111,11111111)
 * @endcode
 *
 * @note  This is an alias for `binary2`
 */
#define dual_byte(byte1, byte2)                                   \
                    unitex::details::binary2_<0##byte1,           \
                                               0##byte2>::value

/**
 * @brief  quad-byte binary literal
 *
 * @code{.cpp}
 *         binary4(11111111,11111111,11111111,11111111)
 * @endcode
 *
 * @see    quad_byte
 */
#define binary4(byte1, byte2, byte3, byte4)                       \
                    unitex::details::binary4_<0##byte1,           \
                                               0##byte2,          \
                                               0##byte3,          \
                                               0##byte4>::value

/**
 * @brief  quad-byte binary literal
 *
 * @code{.cpp}
 *         quad_byte(11111111,11111111,11111111,11111111)
 * @endcode
 *
 * @note  This is an alias for `binary4`
 */
#define quad_byte(byte1, byte2, byte3, byte4)                     \
                    unitex::details::binary4_<0##byte1,           \
                                               0##byte2,          \
                                               0##byte3,          \
                                               0##byte4>::value

/**
 * @brief  octo-byte binary literal
 *
 * @code{.cpp}
 *         binary8(11111111,11111111,11111111,11111111,
 *                 11111111,11111111,11111111,11111111)
 * @endcode
 *
 * @see    octo_byte
 */
#define binary8(byte1, byte2, byte3, byte4,                      \
                byte5, byte6, byte7, byte8)                      \
                   unitex::details::binary8_<0##byte1,           \
                                              0##byte2,          \
                                              0##byte3,          \
                                              0##byte4,          \
                                              0##byte5,          \
                                              0##byte6,          \
                                              0##byte7,          \
                                              0##byte8>::value

/**
 * @brief  octo-byte binary literal
 *
 * @code{.cpp}
 *         octo_byte(11111111,11111111,11111111,11111111,
 *                   11111111,11111111,11111111,11111111)
 * @endcode
 *
 * @note  This is an alias for `binary8`
 */
#define octo_byte(byte1, byte2, byte3, byte4,                     \
                  byte5, byte6, byte7, byte8)                     \
                    unitex::details::binary8_<0##byte1,           \
                                               0##byte2,          \
                                               0##byte3,          \
                                               0##byte4,          \
                                               0##byte5,          \
                                               0##byte6,          \
                                               0##byte7,          \
                                               0##byte8>::value

/**
 * @brief  hexa-byte binary literal
 *
 * @code{.cpp}
 *         binary16(11111111,11111111,11111111,11111111,
 *                  11111111,11111111,11111111,11111111,
 *                  11111111,11111111,11111111,11111111,
 *                  11111111,11111111,11111111,11111111)
 * @endcode
 *
 * @see    hexa_byte
 */
#define binary16(byte1,  byte2,  byte3,  byte4,                   \
                 byte5,  byte6,  byte7,  byte8,                   \
                 byte9,  byte10, byte11, byte12,                  \
                 byte13, byte14, byte15, byte16)                  \
                   unitex::details::binary16_<0##byte1,           \
                                               0##byte2,          \
                                               0##byte3,          \
                                               0##byte4,          \
                                               0##byte5,          \
                                               0##byte6,          \
                                               0##byte7,          \
                                               0##byte8,          \
                                               0##byte9,          \
                                               0##byte10,         \
                                               0##byte11,         \
                                               0##byte12,         \
                                               0##byte13,         \
                                               0##byte14,         \
                                               0##byte15,         \
                                               0##byte16>::value

#if UNITEX_HAVE(UINT128_T)
/**
 * @brief  hexa-byte binary literal
 *
 * @code{.cpp}
 *         hexa_byte(11111111,11111111,11111111,11111111,
 *                   11111111,11111111,11111111,11111111,
 *                   11111111,11111111,11111111,11111111,
 *                   11111111,11111111,11111111,11111111)
 * @endcode
 *
 * @note  This is an alias for `binary16`
 */
#define hexa_byte(byte1,  byte2,  byte3,  byte4,                  \
                  byte5,  byte6,  byte7,  byte8,                  \
                  byte9,  byte10, byte11, byte12,                 \
                  byte13, byte14, byte15, byte16)                 \
                   unitex::details::binary16_<0##byte1,           \
                                               0##byte2,          \
                                               0##byte3,          \
                                               0##byte4,          \
                                               0##byte5,          \
                                               0##byte6,          \
                                               0##byte7,          \
                                               0##byte8,          \
                                               0##byte9,          \
                                               0##byte10,         \
                                               0##byte11,         \
                                               0##byte12,         \
                                               0##byte13,         \
                                               0##byte14,         \
                                               0##byte15,         \
                                               0##byte16>::value
#endif  // UNITEX_HAVE(UINT128_T)
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_BITS_LITERAL_H_                              // NOLINT
