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
 * Last modification on Jul 10 2005
 */
//---------------------------------------------------------------------------

/********************************************************************************/
/********************************************************************************/

#include "MF_FormMorpho.h"

extern l_cats_T L_CATS;

////////////////////////////////////////////
// Initializes the morphology of a form.
int f_init_morpho(f_morpho_T *feat) {
  feat->no_cats = 0;
  return 0;
}

////////////////////////////////////////////
// Initializes the morphology of a form.
void f_delete_morpho(f_morpho_T *feat) {
  free(feat);
}

////////////////////////////////////////////
// Modifies the morphology of a form.
// Each category-value appearing in 'new_feat' replaces the corresponding category in 'old_feat',
// e.g. if 'old_feat'={Gen=fem, Nb=sing} and 'new_feat'={Nb=pl} then 'old_feat' becomes {Gen=fem, Nb=pl}.
// Returns 0 on success, returns 1 if a category from 'new_feat' does not appear in 'old_feat'.
int f_change_morpho(f_morpho_T *old_feat, f_morpho_T *new_feat) {
  int c_old, c_new;  //category indices in old_feat and in new_feat
  int found;

  for (c_new=0 ; c_new<new_feat->no_cats; c_new++) {
    found = 0;
    for (c_old=0; c_old<old_feat->no_cats && !found; c_old++)
      if (old_feat->cats[c_old].cat == new_feat->cats[c_new].cat) {
	old_feat->cats[c_old].val = new_feat->cats[c_new].val;
	found = 1;
      }
    if (!found)   //Error if the category to be changed does not appear in 'old_feat'
      return 1;
  }
  return 0;
} 
////////////////////////////////////////////
// Enlarges the morphology of a form.
// The category-value pair created from 'cat' and 'val' is added to 'feat',
// e.g. if 'feat'={Gen=fem}, 'cat'=Nb, nb=(sing,pl) and 'val'=1 then 'feat' becomes {Gen=fem, Nb=pl}.
// We assume that 'val' is a valid index in the domain of 'cat.
// Returns 0 on success, returns 1 if 'cat' already appears in 'feat'.
int f_add_morpho(f_morpho_T *feat, l_category_T *cat, int val) {
  int c;  //category index in feat

  for (c=0; c<feat->no_cats; c++)
      if (feat->cats[c].cat == cat) 
        return 1;      //Error if the category to be added already appears in old_cat
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
int f_add_morpho_unichar(f_morpho_T *feat, unichar *cat, unichar* val) {
  l_category_T *c;
  int v;  //category index in feat

  c = is_valid_cat(cat);    //Checks if 'cat' is a valid category name in the current language
  if (!c) {
    fprintf(stderr,"Invalid category: ");
    u_fprints(cat,stderr);
    //    fprintf(stderr,"\n");
    return -1;
  }
  v = is_valid_val(c,val);    //Checks if 'val' is a valid category name in the current language
  if (v == -1) {
    fprintf(stderr,"Invalid value: ");
    u_fprints(val,stderr);
    //    fprintf(stderr,"\n");
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
// Returns -1 if 'cat' not found in 'feat'.
int f_get_value(f_morpho_T *feat, l_category_T* cat) {
  int c; //category index in feat
  for (c=0; c<feat->no_cats; c++)
    if (feat->cats[c].cat == cat)    //Category 'cat' found in 'feat'.
      return feat->cats[c].val;
  return -1;
}

/////////////////////////////////////////////////
// Prints the contents of a form's morphology.
// Returns 0.
int f_print_morpho(f_morpho_T *feat) {
  int c; //category index in feat
  int i; //index of the current value in the domain of the current category
  unichar tmp[3];
  u_strcpy_char(tmp,"{");
  u_prints(tmp);
  for (c=0; c<feat->no_cats; c++) {
    u_prints(feat->cats[c].cat->name);    //Print the category
    u_strcpy_char(tmp,"=");
    u_prints(tmp);
    i = feat->cats[c].val;
    u_prints(feat->cats[c].cat->values[i]);    //Print the value
    if (c<feat->no_cats-1)  {
      u_strcpy_char(tmp,";");
      u_prints(tmp);    
    }
  }
  u_strcpy_char(tmp,"}\n");
  u_prints(tmp);    
  return 0;

}

