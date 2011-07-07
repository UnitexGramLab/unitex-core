/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef KrMwuDicH
#define KrMwuDicH

#include "Unicode.h"
#include "Alphabet.h"
#include "Korean.h"
#include "MF_Global.h"
#include "MF_LangMorpho.h"
#include "DELA.h"
#include "LoadInf.h"
#include "CompressedDic.h"

void create_mwu_dictionary(U_FILE* delas,U_FILE* grf,MultiFlex_ctx* ctx,
                           Korean* korean,struct l_morpho_t* morpho,
                           const VersatileEncodingConfig*,
                           Dictionary* d);

#endif

