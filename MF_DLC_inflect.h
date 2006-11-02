/*
  * Unitex 
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
  *
  */

/* Created by Agata Savary (agata.savary@univ-tours.fr)
 * Last modification on Sept 08 2005
 */
//---------------------------------------------------------------------------

/********************************************************************************/
/* INFLECTION OF A DELAC FILE INTO A DELACF                                     */
/********************************************************************************/

#ifndef DlcInflectH
#define DlcInflectH

#include "unicode.h"
#include "MF_LangMorpho.h"
#include "MF_MU_morpho.h"

/////////////////////////////////////////////////
//Maximum number of semantic or syntactic codes
#define MAX_CODES 20

/////////////////////////////////////////////////
//Maximum length of a DELAC line
#define MAX_DLC_LINE 500
//Maximum length of a DELACF line
#define MAX_DLCF_LINE 500

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
int DLC_line2entry(unichar* line, DLC_entry_T* entry);

/////////////////////////////////////////////////////////////////////////////////
// Inflects a DELAC into a DELACF.
// On error returns 1, 0 otherwise.
int DLC_inflect(char* DLC, char* DLCF);

/////////////////////////////////////////////////////////////////////////////////
// Prints a DELAC entry.
int DLC_print_entry(DLC_entry_T* entry);

/////////////////////////////////////////////////////////////////////////////////
// Liberates the memory allocated for a DELAC entry.
void DLC_delete_entry(DLC_entry_T* entry);

#endif
