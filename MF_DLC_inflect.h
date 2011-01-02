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

/* Created by Agata Savary (agata.savary@univ-tours.fr)
 */


/********************************************************************************/
/* INFLECTION OF A DELAC FILE INTO A DELACF                                     */
/********************************************************************************/

#ifndef DlcInflectH
#define DlcInflectH

#include "Unicode.h"
#include "MF_LangMorpho.h"
#include "MF_MU_morpho.h"
#include "MF_DicoMorpho.h"
#include "Korean.h"

#define ONLY_SIMPLE_WORDS 0
#define ONLY_COMPOUND_WORDS 1
#define SIMPLE_AND_COMPOUND_WORDS 2


/////////////////////////////////////////////////
//Maximum number of semantic or syntactic codes
#define MAX_CODES 20

////////////////////////////////////////////
// A DELAC entry.
typedef struct {
  MU_lemma_T* lemma;           //lemma, with its class (e.g. noun)  and padaradigm (e.g. "N41")
  unichar* codes[MAX_CODES];   //semantic or syntactic codes, e.g. ("Hum","z1"), possibly void
  unichar* comment;            //e.g. "electricity", possibly void
} DLC_entry_T;

/////////////////////////////////////////////////////////////////////////////////
// Converts a DELAC line ('line') into a structured DELAC entry ('entry').
// Initially, entry has its space allocated but is empty.
// Return 1 if 'line' is empty, -1 if its format is incorrect, 0 otherwise.
int DLC_line2entry(Alphabet* alph,struct l_morpho_t* pL_MORPHO,unichar* line, DLC_entry_T* entry,d_class_equiv_T* D_CLASS_EQUIV);

/////////////////////////////////////////////////////////////////////////////////
// Inflects a DELAS/DELAC into a DELAC/DELACF.
// On error returns 1, 0 otherwise.
int inflect(char*,char*,MultiFlex_ctx*,struct l_morpho_t*,Alphabet* alph,Encoding,int,int,int,d_class_equiv_T* D_CLASS_EQUIV,int error_check_status,
		      Korean* korean,const char* pkgdir);

/////////////////////////////////////////////////////////////////////////////////
// Prints a DELAC entry.
int DLC_print_entry(struct l_morpho_t* pL_MORPHO, DLC_entry_T* entry);

/////////////////////////////////////////////////////////////////////////////////
// Liberates the memory allocated for a DELAC entry.
void DLC_delete_entry(DLC_entry_T* entry);

#endif
