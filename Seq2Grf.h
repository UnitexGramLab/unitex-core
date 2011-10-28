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
  
#ifndef Seq2GrfH
#define Seq2GrfH

#include "UnitexGetOpt.h"
#include "Alphabet.h"
#include "DELA_tree.h"
#include "Tfst.h"

extern const char* optstring_Seq2Grf;
extern const struct option_TS lopts_Seq2Grf[];
extern const char* usage_Seq2Grf;

int main_Seq2Grf(int argc,char* const argv[]);
void build_sequences_automaton(
		U_FILE*, const struct text_tokens*,
		const Alphabet*, U_FILE*, U_FILE*, int, struct hash_table*,
		int,int,int,int);
#endif

