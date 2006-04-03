 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//---------------------------------------------------------------------------
#ifndef LocateConstantsH
#define LocateConstantsH
//---------------------------------------------------------------------------

// bit masks used in the control byte of each token of the text
#define MOT_TOKEN_BIT_MASK 1
#define DIC_TOKEN_BIT_MASK 2
#define MAJ_TOKEN_BIT_MASK 4
#define MIN_TOKEN_BIT_MASK 8
#define PRE_TOKEN_BIT_MASK 16
#define CDIC_TOKEN_BIT_MASK 32
#define NOT_DIC_TOKEN_BIT_MASK 64

//---------------------------------------------------------------------------

// constants used to define tags found in the fst2
#define MOT -10
#define DIC -11
#define MAJ -12
#define MIN -13
#define PRE	-14
#define DIESE -15
#define EPSILON -16
#define SPACE_TAG -17
#define VAR_START -18
#define VAR_END -19
#define NB -20
#define CDIC -21
#define SDIC -22
/* $CD$ begin */
#define TOKEN -23
/* $CD$ end   */
#define LEXICAL_TAG -5
#define NOTHING_TAG -1

#define POSITIVE_CONTEXT_MARK -24
#define NEGATIVE_CONTEXT_MARK -25
#define CONTEXT_END_MARK -26

//---------------------------------------------------------------------------

// tag bit maks
// this first bit mask set is used when a fst2 is loaded
#define TRANSDUCTION_TAG_BIT_MASK 1
#define NEGATION_TAG_BIT_MASK 2
#define RESPECT_CASE_TAG_BIT_MASK 4 // to 1 if case variants are not allowed
#define START_VAR_TAG_BIT_MASK 64
#define END_VAR_TAG_BIT_MASK 128

#define POSITIVE_CONTEXT_MASK 8
#define NEGATIVE_CONTEXT_MASK 16
#define CONTEXT_END_MASK 32

// this second tag set is used in the "numerote_tags(...)" function
#define GRAMM_CODE_TAG_BIT_MASK 8 // used when a grammatical code is present in a tag: <V>, <manger.V>
#define CONTROL_TAG_BIT_MASK 16 // #, ESPACE, <E>, <MOT>, <DIC>, <SDIC>, <CDIC>, <MAJ>, <MIN>, <PRE>, $a(, $a), <NB>
#define TOKEN_TAG_BIT_MASK 32 // lundi, <manger> (<manger> is there because it will be replaced by a token list)
#define LEMMA_TAG_BIT_MASK 64 // used when a lemma is present a tag: <manger>, <manger.V>, <mange,manger.V>

//---------------------------------------------------------------------------

#define NO_COMPOUND_PATTERN -1


//---------------------------------------------------------------------------
#endif

