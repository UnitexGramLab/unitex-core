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
 * @file      logic_binary.h
 * @brief     Preprocessor macros helpers
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      For a full preprocessor library, use instead the Boost Preprocessor
 *            standalone library. A basic introduction is available from:
 *            http://www.boost.org/doc/libs/1_57_0/libs/preprocessor/doc/index.html
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 logic_binary.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_PREPROCESSOR_LOGIC_BINARY_H_                    // NOLINT
#define UNITEX_BASE_PREPROCESSOR_LOGIC_BINARY_H_                    // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/preprocessor/control.h"
#include "base/preprocessor/logic_unary.h"
/* ************************************************************************** */
/**
 * @def    UNITEX_PP_BITOR
 */
#define UNITEX_PP_BITOR(x,y)    UNITEX_PP_TEST(x)(1,UNITEX_PP_TEST(y)(1,0))
#define UNITEX_PP_BITOR_00      UNITEX_PP_BITOR(0,0)
#define UNITEX_PP_BITOR_01      UNITEX_PP_BITOR(0,1)
#define UNITEX_PP_BITOR_10      UNITEX_PP_BITOR(1,0)
#define UNITEX_PP_BITOR_11      UNITEX_PP_BITOR(1,1)

/**
 * @def    UNITEX_PP_BITAND
 */
#define UNITEX_PP_BITAND(x,y)   UNITEX_PP_TEST(x)(UNITEX_PP_TEST(y)(1,0),0)
#define UNITEX_PP_BITAND_00     UNITEX_PP_BITAND(0,0)
#define UNITEX_PP_BITAND_01     UNITEX_PP_BITAND(0,1)
#define UNITEX_PP_BITAND_10     UNITEX_PP_BITAND(1,0)
#define UNITEX_PP_BITAND_11     UNITEX_PP_BITAND(1,1)

/**
 * @def    UNITEX_PP_BITNOR
 */
#define UNITEX_PP_BITNOR(x,y)   UNITEX_PP_COMPL(UNITEX_PP_BITOR(x,y))
#define UNITEX_PP_BITNOR_00     UNITEX_PP_BITNOR(0,0)
#define UNITEX_PP_BITNOR_01     UNITEX_PP_BITNOR(0,1)
#define UNITEX_PP_BITNOR_10     UNITEX_PP_BITNOR(1,0)
#define UNITEX_PP_BITNOR_11     UNITEX_PP_BITNOR(1,1)

/**
 * @def    UNITEX_PP_BITNAND
 */
#define UNITEX_PP_BITNAND(x,y)  UNITEX_PP_COMPL(UNITEX_PP_BITAND(x,y))
#define UNITEX_PP_BITNAND_00    UNITEX_PP_BITNAND(0,0)
#define UNITEX_PP_BITNAND_01    UNITEX_PP_BITNAND(0,1)
#define UNITEX_PP_BITNAND_10    UNITEX_PP_BITNAND(1,0)
#define UNITEX_PP_BITNAND_11    UNITEX_PP_BITNAND(1,1)

/**
 * @def    UNITEX_PP_BITXOR
 */
#define UNITEX_PP_BITXOR(x,y)   UNITEX_PP_BITNOR(UNITEX_PP_BITAND(x,y),UNITEX_PP_BITNOR(x,y))
#define UNITEX_PP_BITXOR_00     UNITEX_PP_BITXOR(0,0)
#define UNITEX_PP_BITXOR_01     UNITEX_PP_BITXOR(0,1)
#define UNITEX_PP_BITXOR_10     UNITEX_PP_BITXOR(1,0)
#define UNITEX_PP_BITXOR_11     UNITEX_PP_BITXOR(1,1)

/**
 * @def    UNITEX_PP_BITXNOR
 */
#define UNITEX_PP_BITXNOR(x,y)  UNITEX_PP_COMPL(UNITEX_PP_BITXOR(x,y))
#define UNITEX_PP_BITXNOR_00    UNITEX_PP_BITXNOR(0,0)
#define UNITEX_PP_BITXNOR_01    UNITEX_PP_BITXNOR(0,1)
#define UNITEX_PP_BITXNOR_10    UNITEX_PP_BITXNOR(1,0)
#define UNITEX_PP_BITXNOR_11    UNITEX_PP_BITXNOR(1,1)

/**
 * @def    UNITEX_PP_OR
 */
#define UNITEX_PP_OR(x,y)       UNITEX_PP_IF(x)(1,UNITEX_PP_IF(y)(1,0))

/**
 * @def    UNITEX_PP_AND
 */
#define UNITEX_PP_AND(x,y)      UNITEX_PP_IF(x)(UNITEX_PP_IF(y)(1,0),0)

/**
 * @def    UNITEX_PP_NOR
 */
#define UNITEX_PP_NOR(x,y)      UNITEX_PP_COMPL(UNITEX_PP_OR(x,y))

/**
 * @def    UNITEX_PP_NAND
 */
#define UNITEX_PP_NAND(x,y)     UNITEX_PP_COMPL(UNITEX_PP_AND(x,y))

/**
 * @def    UNITEX_PP_XOR
 */
#define UNITEX_PP_XOR(x,y)      UNITEX_PP_NOR(UNITEX_PP_AND(x,y),UNITEX_PP_NOR(x,y))

/**
 * @def    UNITEX_PP_XNOR
 */
#define UNITEX_PP_XNOR(x,y)     UNITEX_PP_COMPL(UNITEX_PP_XOR(x,y))
/* ************************************************************************** */
#endif  // UNITEX_BASE_PREPROCESSOR_LOGIC_BINARY_H_                 // NOLINT
