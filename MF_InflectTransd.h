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

/* Created by Agata Savary (savary@univ-tours.fr)
 */


#ifndef MF_InflectTransH
#define MF_InflectTransH



////////////////////////////////////////////////////////////
// Implementation of the management of inflectional tranducers
////////////////////////////////////////////////////////////
// Copy of some functions from Inflect.cpp

#include "Fst2.h"
#include "AbstractFst2Load.h"

#include "MF_InflectTransdBase.h"



///////////////////////////////
//Initiate the tree for inflection transducers' names
//On succes return 0, 1 otherwise
int init_transducer_tree(MultiFlex_ctx* p_multiFlex_ctx);

///////////////////////////////
// Free the transducer tree memory
void free_transducer_tree(MultiFlex_ctx* p_multiFlex_ctx);

///////////////////////////////
// Try to load the transducer flex and returns its position in the
// 'fst2' array. Returns -1 if the transducer cannot be loaded
int get_transducer(MultiFlex_ctx* p_multiFlex_ctx,char* flex,Encoding encoding_output,
		int bom_output,int mask_encoding_compatibility_input,const char* pkgdir);

#endif
