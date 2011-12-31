/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * Last modification on Jul 10 2005
 */
//---------------------------------------------------------------------------

/********************************************************************************/
/********************************************************************************/

#include "MF_FormMorpho.h"
#include "MF_LangMorpho.h"
#include "Error.h"

//extern l_cats_T L_CATS;

////////////////////////////////////////////
// Initializes the morphology of a form.
int f_init_morpho(f_morpho_T *feat) {
  feat->no_cats = 0;
  return 0;
}

////////////////////////////////////////////
// Liberates the morphology of a form.
void f_delete_morpho(f_morpho_T *feat) {
  free(feat);
}

////////////////////////////////////////////
// Modifies the morphology of a form.
// Each category-value appearing in 'new_feat' replaces the corresponding category in 'old_feat',
// e.g. if 'old_feat'={Gen=fem, Nb=sing} and 'new_feat'={Nb=pl} then 'old_feat' becomes {Gen=fem, Nb=pl}.
// If a category does not appear in 'old_feat' but it appears in 'new_feat' an error occurs, except when
// this category may admit an empty value. In this case the category is added to 'old_feat'.
// e.g. if 'old_feat'={Gen=fem, Nb=sing} and 'new_feat'={Nb=pl, Gr=D} and Gr:<E>,D,A then
// 'old_feat' becomes {Gen=fem, Nb=pl, Gr=D}. But if Gr:B,D,A than an error appears.
// Returns 0 on success, 1 otherwise  
int f_change_morpho(struct l_morpho_t* pL_MORPHO,f_morpho_T *old_feat, f_morpho_T *new_feat) {
  int c_old, c_new;  //category indices in old_feat and in new_feat
  int found;

  for (c_new=0 ; c_new<new_feat->no_cats; c_new++) {
    found = 0;
    for (c_old=0; c_old<old_feat->no_cats && !found; c_old++)
      if (old_feat->cats[c_old].cat == new_feat->cats[c_new].cat) {
	old_feat->cats[c_old].val = new_feat->cats[c_new].val;
	found = 1;
      }
    //If a category was not found in 'old_feat' but is admits and empty value
    //then it is added to 'old_feat' with the current value
    if (!found && admits_empty_val(pL_MORPHO,new_feat->cats[c_new].cat)) {
      f_add_morpho(old_feat, new_feat->cats[c_new].cat, new_feat->cats[c_new].val);
    }

    /*
    if (!found)   //Error if the category to be changed does not appear in 'old_feat'
      return 1;
    */
    
  }
  return 0;
} 

////////////////////////////////////////////
// Compare two morphological descriptions
// Return 0 if they are identical, 1 otherwise.
int f_morpho_cmp(f_morpho_T* m1, f_morpho_T* m2) {
  if (!m1 && !m2)
    return 0;
  if (!m1 || !m2)
    return 1;
  if (m1->no_cats != m2->no_cats)
    return 1;
  int c1;  //index of the current category-value pair in m1
  int c2;  //index of the current category-value pair in m2
  int found;  //Boolean saying if the current category-value in m1 has been found in m2
  for (c1=0; c1<m1->no_cats; c1++) {
    found = 0;
    c2=0; 
    while (c2<m2->no_cats && !found) {
      if ((m1->cats[c1].cat == m2->cats[c2].cat) && (m1->cats[c1].val == m2->cats[c2].val))
	  found = 1;
      c2++;
    }
    //If the current category-value pair in m1 does not exist in m2 then m1 and m2 are not identical
    if (!found)
      return 1;
  }
  return 0;
}

////////////////////////////////////////////
// Enlarges the morphology of a form.
// The category-value pair created from 'cat' and 'val' is added to 'feat',
// e.g. if 'feat'={Gen=fem}, 'cat'=Nb, nb=(sing,pl) and 'val'=1 then 'feat' becomes {Gen=fem, Nb=pl}.
// We assume that 'val' is a valid index in the domain of 'cat.
// Returns 0 on success, returns 1 if 'cat' already appears in 'feat', returns -1 if no space for a new category.
int f_add_morpho(f_morpho_T *feat, l_category_T *cat, int val) {
  int c;  //category index in feat

  for (c=0; c<feat->no_cats; c++) 
      if (feat->cats[c].cat == cat) 
        return 1;      //Error if the category to be added already appears in old_cat

  //Check if there is enough space for a new category
  if (feat->no_cats >= MAX_CATS) {
    error("Too many inflection categories required");
    return 1;
  }
  feat->cats[feat->no_cats].cat = cat;  //Add new category-value pair
  feat->cats[feat->no_cats].val = val;  //Add new category-value pair
  feat->no_cats++;
  return 0;
}

////////////////////////////////////////////
// Enlarges the morphology of a form on a basis of char data.
// The category-value pair created from 'cat' and 'val' is added to 'feat',
// e.g. if 'feat'={Gen=fem}, 'cat'="Nb" and 'val'="pl" then 'feat' becomes {Gen=fem, Nb=pl}.
// Returns 0 on success, returns 1 if 'cat' already appears in 'feat'.
// Returns -1 if 'cat' or 'val' invalid category or value names in the current language.
int f_add_morpho_unichar(struct l_morpho_t* pL_MORPHO,f_morpho_T *feat, unichar *cat, unichar* val) {
  l_category_T *c;
  int v;  //category index in feat

  c = is_valid_cat(pL_MORPHO,cat);    //Checks if 'cat' is a valid category name in the current language
  if (!c) {
    error("Invalid category: %S",cat);
    //    error("\n");
    return -1;
  }
  v = is_valid_val(c,val);    //Checks if 'val' is a valid category name in the current language
  if (v == -1) {
    error("Invalid value: %S",val);
    //    error("\n");
    return -1;
  }
  return (f_add_morpho(feat,c,v));
}

////////////////////////////////////////////
// Reduces the morphology of a form.
// Category-value corresponding to 'cat' is deleted from 'feat',
// e.g. if 'feat'={Gen=fem,Nb=pl} and 'cat'=Gen then 'feat' becomes {Nb=pl}
// Returns 0 on success, returns 1 if 'cat' not found in 'old_feat'.
int f_del_one_morpho(f_morpho_T *feat, l_category_T* cat) {
  int c;  //category index in feat

  for (c=0; c<feat->no_cats; c++)
    if (feat->cats[c].cat == cat) {   //Category 'cat' found in 'feat'.
      while (c < feat->no_cats - 1) {  //Shift the remaining cat-val pairs to the left.
	feat->cats[c] = feat->cats[c+1];
	c++;
      }
      feat->no_cats--;
      return 0;
    }
  return 1;  //Error if category 'cat' not found in 'old_feat'.
}

////////////////////////////////////////////
// Reads the morphology of a form.
// Returns the value of the category 'cat' in 'feat' (i.e. the index of the value in the domain of 'cat'),
// e.g. if 'feat'={Gen=neu,Nb=pl}, 'cat'=Gen, and Gen={masc,fem,neu} then return 2.
// If 'cat' does not appear in 'feat' but it admits an empty value, then returns the index of the empty value.
// Returns -1 if 'cat' not found in 'feat' and 'cat' does not admit an empty value.
int f_get_value(struct l_morpho_t* pL_MORPHO,f_morpho_T *feat, l_category_T* cat) {
  int c; //category index in feat
  for (c=0; c<feat->no_cats; c++)
    if (feat->cats[c].cat == cat)    //Category 'cat' found in 'feat'.
      return feat->cats[c].val;
  //If the category 'cat' not found in 'feat' check if 'cat' admits an empty value
  return get_empty_val(pL_MORPHO,cat);
}

/////////////////////////////////////////////////
// Prints the contents of a form's morphology.
// Returns 0.
int f_print_morpho(f_morpho_T *feat) {
int c; //category index in feat
int i; //index of the current value in the domain of the current category
u_printf("{");
for (c=0; c<feat->no_cats; c++) {
   u_printf("%S=",feat->cats[c].cat->name);    //Print the category
   i=feat->cats[c].val;
   u_printf("%S",feat->cats[c].cat->values[i]);    //Print the value
   if (c<feat->no_cats-1)  {
      u_printf(";");    
   }
}
u_printf("}\n");
return 0;
}

////////////////////////////////////////////
// Copies the form morphology feat2 into feat1
// If feat1 has its space allocated its contents is replaced by a copy of feat2
// If feat1 does not have its space allocated it is allocated first and then filled with a copy feat2
// Returns 0 on success, -1 on error
int f_copy_morpho(f_morpho_T *feat1, f_morpho_T *feat2) {
  int c;  //category index in feat1 and feat2
  
  if (!feat1) {
    feat1 = (f_morpho_T*) malloc(sizeof(f_morpho_T));
    if (!feat1)
      fatal_alloc_error("f_copy_morpho");
  }

  if (!feat2)
    return -1;

  for (c=0; c<feat2->no_cats; c++)  
    feat1->cats[c] = feat2->cats[c];  //Add new category-value pair

  feat1->no_cats = feat2->no_cats;

  return 0;
}


