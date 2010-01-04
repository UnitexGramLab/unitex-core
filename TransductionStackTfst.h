 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef TransductionStackH
#define TransductionStackH

#include "Unicode.h"
#include "TransductionVariables.h"
#include "Ustring.h"
#include "LocateTfst_lib.h"

/* Every character or string that comes from the input text must be
 * pushed with the following functions, because some characters like dots
 * and commas may have to be protected when they come from the input. This
 * is useful when LocateTfst is invoked (one day maybe) from the Dico program 
 * in order to avoid producing bad lines like:
 *
 *    3,14,PI.NUM   ==>  should be:  3\,14,PI.NUM
 */
void push_input_char_tfst(Ustring*,unichar,int);
void push_input_string_tfst(Ustring*,unichar*,int);
void push_input_substring_tfst(Ustring*,unichar* s,int length,int);

void push_output_char_tfst(Ustring*,unichar);
void push_output_string_tfst(Ustring*,unichar*);
void insert_text_interval_tfst(struct locate_tfst_infos*,Ustring*,int,int,int,int);
int process_output_tfst(Ustring*,unichar*,struct locate_tfst_infos*);

#endif

