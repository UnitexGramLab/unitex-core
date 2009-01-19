/*
  * Unitex 
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * Last modification on Jul 10 2005
 */
//---------------------------------------------------------------------------

/********************************************************************************/
/********************************************************************************/

#ifndef FormMorphoH
#define FormMorphoH

#include "Unicode.h"
#include "MF_LangMorpho.h"

// Structure for a category-value couple, e.g. Gen=fem
typedef struct {
  l_category_T* cat;   //category, e.g. Gen
  int val;             //category's value (e.g. fem): index of val in the domain of cat
} f_category_T;

/////////////////////////////////////////////////////////
// Morphology of a word form.
// A set of category-value couples, e.g. {Gen=fem, Nb=sing}
typedef struct {
  int no_cats;     //number of category-value couples
  f_category_T cats[MAX_CATS];  //collection of category-value couples
} f_morpho_T;

////////////////////////////////////////////
// Initializes the morphology of a form.
int f_init_morpho(f_morpho_T *feat);

////////////////////////////////////////////
// Liberates the morphology of a form.
void f_delete_morpho(f_morpho_T *feat);

////////////////////////////////////////////
// Modifies the morphology of a form.
// Each category-value appearing in 'new_feat' replaces the corresponding category in 'old_feat',
// e.g. if 'old_feat'={Gen=fem, Nb=sing} and 'new_feat'={Nb=pl} then 'old_feat' becomes {Gen=fem, Nb=pl}
// Returns 0 on success, 1 otherwise.
int f_change_morpho(f_morpho_T *old_feat, f_morpho_T *new_feat);

////////////////////////////////////////////
// Compare two morphological descriptions
// Return 0 if they are identical, 1 otherwise.
int f_morpho_cmp(f_morpho_T* m1, f_morpho_T* m2);

////////////////////////////////////////////
// Enlarges the morphology of a form.
// The category-value pair created from 'cat' and 'val' is added to 'feat',
// e.g. if 'feat'={Gen=fem}, 'cat'=Nb, nb=(sing,pl) and 'val'=1 then 'feat' becomes {Gen=fem, Nb=pl}.
// We assume that 'val' is a valid index in the domain of 'cat.
// Returns 0 on success, returns 1 if 'cat' already appears in 'feat'.
int f_add_morpho(f_morpho_T *feat, l_category_T *cat, int val);

////////////////////////////////////////////
// Enlarges the morphology of a form on a basis of char data.
// The category-value pair created from 'cat' and 'val' is added to 'feat',
// e.g. if 'feat'={Gen=fem}, 'cat'="Nb" and 'val'="pl" then 'feat' becomes {Gen=fem, Nb=pl}.
// Returns 0 on success, returns 1 if 'cat' already appears in 'feat'.
// Returns -1 if 'cat' or 'val' invalid category or value names in the current language.
int f_add_morpho_unichar(f_morpho_T *feat, unichar *cat, unichar* val);

////////////////////////////////////////////
// Reduces the morphology of a form.
// Category-value corresponding to 'cat' is deleted from 'feat',
// e.g. if 'feat'={Gen=fem,Nb=pl} and 'cat'=Gen then 'feat' becomes {Nb=pl}
// Returns 0 on success, 1 otherwise.
int f_del_one_morpho(f_morpho_T *feat, l_category_T* cat);

////////////////////////////////////////////
// Reads the morphology of a form.
// Returns the value of the category 'cat' in 'feat' (i.e. the index of the value in the domain of 'cat'),
// e.g. if 'feat'={Gen=neu,Nb=pl}, 'cat'=Gen, and Gen={masc,fem,neu} then return 2.
// If 'cat' does not appear in 'feat' but it admits an emmpty value, then returns the index of the empty value.
// Returns -1 if 'cat' not found in 'feat' and 'cat' does not admit an empty value.
int f_get_value(f_morpho_T *feat, l_category_T* cat);

/////////////////////////////////////////////////
// Prints the contents of a form's morphology.
// Returns 0.
int f_print_morpho(f_morpho_T *feat);

////////////////////////////////////////////
// Copies the form morphology feat2 into feat1
// If feat1 has its space allocated its contents is replaced by a copy of feat2
// If feat1 does not have its space allocated it is allocated first and then filled with a copy feat2
// Returns 0 on success, -1 on error
int f_copy_morpho(f_morpho_T *feat1, f_morpho_T *feat2);
  
#endif
