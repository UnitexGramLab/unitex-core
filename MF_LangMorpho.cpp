/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdio.h>
#include "MF_LangMorpho.h"
#include "MF_Util.h"
#include "Unicode.h"
#include "MF_Unif.h"   //debug
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

///////////////////////////////////////
//All functions defined in this file
int read_language_morpho(struct l_morpho_t*,char *lan_file, const char *dir);
int read_cats(struct l_morpho_t*);
int read_cat_line(struct l_morpho_t*,int cat_no);
int read_classes(struct l_morpho_t*);
int read_class_line(struct l_morpho_t*,int class_no);
int print_language_morpho(struct l_morpho_t*);
struct l_morpho_t* init_language_morpho();
int free_language_morpho(struct l_morpho_t*);
l_category_T* is_valid_cat(struct l_morpho_t*,const unichar* cat);
int is_valid_val(const l_category_T* cat, const unichar* val);
l_category_T* get_cat(struct l_morpho_t*,const unichar* val);

//////////////////////////////////////

/**************************************************************************************/
/* Read the language file "file".                                                     */
/* This file contains lists of all classes (nous, verb, etc.) of the language,        */
/* with their inflection categories (number, case, gender, etc.) and values           */
/* (e.g. sing, pl, masc, etc.).                                                       */
/* <E> is a special character meaning that the feature may have an empty value, e.g.  */
/* the base form in gradation                                                         */
/* E.g. for Polish:								      */
/* 			Polish							      */
/*                      <CATEGORIES>                                                  */
/* 			Nb:sing,pl		                 		      */
/* 			Case:Nom,Gen,Dat,Acc,Inst,Loc,Voc			      */
/* 			Gen:masc_pers,masc_anim,masc_inanim,fem,neu                   */
/*                      Gr:<E>,aug,sup                                                */
/*                      <CLASSES>                                                     */
/*                      noun: (Nb,<var>),(Case,<var>),(Gen,<fixed>)                   */
/*                      adj: (Nb,<var>),(Case,<var>),(Gen,<var>),(Gr,<var>)           */
/*                      adv: (Gr,<var>)                                               */
/* Fills out pL_MORPHO->L_CLASSES and pL_MORPHO->L_CATS.						      */
/* Returns 0 if success, 1 otherwise                                                  */
int read_language_morpho(const VersatileEncodingConfig* vec,struct l_morpho_t* pL_MORPHO, const char *file) {
  //Initialise the symbol representing an empty morphological value
  u_strcpy(pL_MORPHO->EMPTY_VAL,"<E>");

  //Open the Morphology file
  pL_MORPHO->lf = u_fopen(vec,file,U_READ);
  if ( !(pL_MORPHO->lf))  {
    error("Unable to open language morphology file %s\n",file);
    return 1;
  }

  //Omit the first pL_MORPHO->line (language name)
  if (! u_feof(pL_MORPHO->lf)) {
    u_fgets(pL_MORPHO->line,MAX_LANG_MORPHO_LINE-1,pL_MORPHO->lf);
    pL_MORPHO->line_no++;
  }

  //scan the following pL_MORPHO->line
  u_fgets(pL_MORPHO->line,MAX_LANG_MORPHO_LINE-1,pL_MORPHO->lf);
  u_to_char(pL_MORPHO->line_ch,pL_MORPHO->line);
  sscanf(pL_MORPHO->line_ch,"%s",pL_MORPHO->word_ch);
  pL_MORPHO->line_no++;

  if (! u_feof(pL_MORPHO->lf))
    if (read_cats(pL_MORPHO))
      return 1;
  if (! u_feof(pL_MORPHO->lf))
    if (read_classes(pL_MORPHO))
      return 1;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Read the category block of the language morphology file.
// It should begin with <CATEGORIES>
// Before the beginning and after the end of the function "pL_MORPHO->line" (global) contains
// the current pL_MORPHO->line of the morpho file.
int read_cats(struct l_morpho_t* pL_MORPHO) {

  int cat_no; //category's number
  int l;   //lenght of the scanned pL_MORPHO->line

  //Current pL_MORPHO->line should contain <CATEGORIES>

  if (u_feof(pL_MORPHO->lf) || strcmp(pL_MORPHO->word_ch,"<CATEGORIES>")) {
    error("Language morphology file format incorrect in pL_MORPHO->line %d!\n",pL_MORPHO->line_no);
    return 1;
  }

  //Scan categories
  l = u_fgets(pL_MORPHO->line,MAX_LANG_MORPHO_LINE-1,pL_MORPHO->lf);
  u_to_char(pL_MORPHO->line_ch,pL_MORPHO->line);
  sscanf(pL_MORPHO->line_ch,"%s",pL_MORPHO->word_ch);
  pL_MORPHO->line_no++;
  cat_no = 0;
  while (l && strcmp(pL_MORPHO->word_ch,"<CLASSES>")) {
    if (read_cat_line(pL_MORPHO,cat_no))
      return 1;
    l = u_fgets(pL_MORPHO->line,MAX_LANG_MORPHO_LINE-1,pL_MORPHO->lf);
    u_to_char(pL_MORPHO->line_ch,pL_MORPHO->line);
    sscanf(pL_MORPHO->line_ch,"%s",pL_MORPHO->word_ch);
    pL_MORPHO->line_no++;
    cat_no++;
  }
  pL_MORPHO->L_CATS.no_cats = cat_no;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Read a category pL_MORPHO->line (having number cat_no) of the language morphology file.
// Before the beginning of the function "pL_MORPHO->line" (global) contains
// the current pL_MORPHO->line of the morpho file.
int read_cat_line(struct l_morpho_t* pL_MORPHO,int cat_no) {

  int v_cnt;  //counter of values for the present category
  unichar* cat_name;   //category's name
  unichar* cat_val;    //category's value
  unichar tmp[MAX_MORPHO_NAME];  //buffer for pL_MORPHO->line elements
  unichar tmp_void[MAX_MORPHO_NAME];  //buffer for void characters
  unichar* line_pos; //current position in the input pL_MORPHO->line
  int l;  //length of a scanned sequence
  int done;

  line_pos = pL_MORPHO->line;

  //Read category name
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
  l = u_scan_until_char(tmp,line_pos,MAX_MORPHO_NAME-1,": \t",1);
  line_pos = line_pos + l;
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
  if (*line_pos != (char) ':') {
    error("Language morphology file format incorrect: ':' missing in pL_MORPHO->line %d!\n", pL_MORPHO->line_no);
    return 1;
  }
  cat_name=u_strdup(tmp);
  pL_MORPHO->L_CATS.cats[cat_no].name = cat_name;
  line_pos++;   //Omit the ':'

  //Read category values
  v_cnt = 0;
  done = 0;
  do {
    line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
    l = u_scan_until_char(tmp,line_pos,MAX_MORPHO_NAME-1,", \t\n",1);
    line_pos = line_pos + l;
    line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
    cat_val=u_strdup(tmp);
    pL_MORPHO->L_CATS.cats[cat_no].values[v_cnt] = cat_val;
    v_cnt++;
    if (*line_pos == (char) '\n')
      done = 1;
    else
      line_pos++;  //Omit the ',' or the newpL_MORPHO->line
  } while (!done);
  pL_MORPHO->L_CATS.cats[cat_no].no_values = v_cnt;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Read that class block of the language morphology file.
// It should begin with <CLASSES>.
// Before the beginning and after the end of the function "pL_MORPHO->line" (global) contains
// the current pL_MORPHO->line of the morpho file.
int read_classes(struct l_morpho_t* pL_MORPHO) {

  int class_no; //class number
  int l;   //lenght of the scanned pL_MORPHO->line

  //Current pL_MORPHO->line should contain <CLASSES>
  if (u_feof(pL_MORPHO->lf) || strcmp(pL_MORPHO->word_ch,"<CLASSES>")) {
    error("Language morphology file format incorrect: <CLASSES> missing in pL_MORPHO->line %d!\n", pL_MORPHO->line_no);
    return 1;
  }

 //Scan classes
  l = u_fgets(pL_MORPHO->line,MAX_LANG_MORPHO_LINE-1,pL_MORPHO->lf);
  u_to_char(pL_MORPHO->line_ch,pL_MORPHO->line);
  sscanf(pL_MORPHO->line_ch,"%s",pL_MORPHO->word_ch);
  pL_MORPHO->line_no++;

  class_no = 0;
  while (l>1) {
    if (read_class_line(pL_MORPHO,class_no))
      return 1;
    l = u_fgets(pL_MORPHO->line,MAX_LANG_MORPHO_LINE-1,pL_MORPHO->lf);
    u_to_char(pL_MORPHO->line_ch,pL_MORPHO->line);
    sscanf(pL_MORPHO->line_ch,"%s",pL_MORPHO->word_ch);
    pL_MORPHO->line_no++;
    class_no++;
  }
  pL_MORPHO->L_CLASSES.no_classes = class_no;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Read a class pL_MORPHO->line (having number class_no) of the language morphology file.
// Before the beginning and after the end of the function "pL_MORPHO->line" (global) contains
// the current pL_MORPHO->line of the morpho file.
int read_class_line(struct l_morpho_t* pL_MORPHO,int class_no) {

  int c_cnt;  //counter of categories for the present class
  unichar* class_name;   //class' name
  unichar tmp[MAX_MORPHO_NAME];  //buffer for pL_MORPHO->line elements
  unichar tmp_void[MAX_MORPHO_NAME];  //buffer for void characters
  unichar* line_pos; //current position in the input pL_MORPHO->line
  int l;  //length of a scanned sequence
  int done;
  int c;
  int found;

  line_pos = pL_MORPHO->line;

  //Read class name
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
  l = u_scan_until_char(tmp,line_pos,MAX_MORPHO_NAME-1,": \t",1);
  line_pos = line_pos + l;
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
  class_name=u_strdup(tmp);
  pL_MORPHO->L_CLASSES.classes[class_no].name = class_name;

  //Read class' categories
  c_cnt = 0;
  line_pos++;   //Omit the ':'
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
  if (*line_pos != (char) '\n') {
    done = 0;
    do {

      //Read category
      line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
      line_pos++; //Omit the '('
      line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
      l = u_scan_until_char(tmp,line_pos,MAX_MORPHO_NAME-1,", \t",1);
      line_pos = line_pos + l;
      line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
      //Check if tmp contains an existing category
      for (c=0, found=0; c<pL_MORPHO->L_CATS.no_cats && !found; c++)
	if (! u_strcmp(pL_MORPHO->L_CATS.cats[c].name,tmp))
	  found = 1;
      if (!found) {
	      error("Undefined category in language morphology file: pL_MORPHO->line %d!\n", pL_MORPHO->line_no);
	      return 1;
      }
      else
	pL_MORPHO->L_CLASSES.classes[class_no].cats[c_cnt].cat = &(pL_MORPHO->L_CATS.cats[c-1]);

      //Read the fixedness
      line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
      line_pos ++; //Omit the ','
      line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
      line_pos ++; //Omit the '<'
      line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
      l = u_scan_until_char(tmp,line_pos,MAX_MORPHO_NAME-1,"> \t",1);
      line_pos = line_pos + l;
      line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
      if (!u_strcmp(tmp,"fixed"))
	pL_MORPHO->L_CLASSES.classes[class_no].cats[c_cnt].fixed = 1;
      else
	if (!u_strcmp(tmp,"var"))
	  pL_MORPHO->L_CLASSES.classes[class_no].cats[c_cnt].fixed = 0;
	else {
	  error("Undefined fixedness symbol in language morphology file: pL_MORPHO->line %d!\n", pL_MORPHO->line_no);
	  return 1;
      }
      c_cnt++;
      line_pos++; //Omit the '>'
      line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
      line_pos++; //Omit the ')'
      line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
      if (*line_pos == (char) '\n')
	done = 1;
      else
	line_pos++;  //Omit the ','
    } while (!done);
  }
  pL_MORPHO->L_CLASSES.classes[class_no].no_cats = c_cnt;
  return 0;
}

/**************************************************************************************/
/* Prints to the standard output the morphological system of the language             */
/* as defined by pL_MORPHO->L_CLASSES.       		    			              */
/* Returns 0 on success, 1 otherwise.                                                 */
int print_language_morpho(struct l_morpho_t* pL_MORPHO) {
int c,v, cl;
//Print categories
u_printf("<CATEGORIES>\n");
for (c=0; c<pL_MORPHO->L_CATS.no_cats; c++) {
   //Print category
   u_printf("%S:",pL_MORPHO->L_CATS.cats[c].name);
   //Print values
   for (v=0; v<pL_MORPHO->L_CATS.cats[c].no_values; v++) {
      u_printf("%S",pL_MORPHO->L_CATS.cats[c].values[v]);
      if (v != pL_MORPHO->L_CATS.cats[c].no_values-1) {
         u_printf(",");
      }
   }
   u_printf("\n");
}

//Print classes
u_printf("<CLASSES>\n");
for (cl=0; cl<pL_MORPHO->L_CLASSES.no_classes; cl++) {
   //print class
   u_printf("%S:",pL_MORPHO->L_CLASSES.classes[cl].name);
   //print relevant categories
   for (c=0; c<pL_MORPHO->L_CLASSES.classes[cl].no_cats; c++) {
      //print category
      u_printf("(%S,<",pL_MORPHO->L_CLASSES.classes[cl].cats[c].cat->name);
      //print fixedness
      if (pL_MORPHO->L_CLASSES.classes[cl].cats[c].fixed) u_printf("fixed");
      else u_printf("var");
      u_printf(">)");
      if (c != pL_MORPHO->L_CLASSES.classes[cl].no_cats-1) {
         u_printf(",");
      }
   }
   u_printf("\n");
}
return 0;
}

/**************************************************************************************/
/* Init the space allocated for the language morphology description.                  */
struct l_morpho_t* init_langage_morph()
{
    l_morpho_t* pL_MORPHO;
    pL_MORPHO = (struct l_morpho_t*)malloc(sizeof(struct l_morpho_t));
    if (pL_MORPHO != NULL) {
        pL_MORPHO->line_no = 0;
        pL_MORPHO->L_CATS.no_cats = 0;
        pL_MORPHO->L_CLASSES.no_classes = 0;
        pL_MORPHO->lf = NULL;
        pL_MORPHO->CATS_OR_CLASSES = 0;
    }
    return pL_MORPHO;
}

/**************************************************************************************/
/* Liberates the space allocated for the language morphology description.             */
int free_language_morpho(struct l_morpho_t* pL_MORPHO) {

  if (pL_MORPHO != NULL) {
    int c, v;

    //Liberate pL_MORPHO->L_CATS
    for (c=0; c<pL_MORPHO->L_CATS.no_cats; c++) {
      free(pL_MORPHO->L_CATS.cats[c].name);
      for (v=0; v<pL_MORPHO->L_CATS.cats[c].no_values; v++)
        free(pL_MORPHO->L_CATS.cats[c].values[v]);
    }

    //Liberate pL_MORPHO->L_CLASSES
    for (c=0; c<pL_MORPHO->L_CLASSES.no_classes; c++)
      free(pL_MORPHO->L_CLASSES.classes[c].name);

    if (pL_MORPHO->lf != NULL)
        u_fclose(pL_MORPHO->lf);

    free(pL_MORPHO);
  }
  return 0;
}

/**************************************************************************************/
/* If cat is a valid category name, returns a pointer to this category.               */
/* Otherwise returns NULL.                                                            */
l_category_T* is_valid_cat(struct l_morpho_t* pL_MORPHO,const unichar* cat) {
  int c;
  for (c=0; c<pL_MORPHO->L_CATS.no_cats; c++)
    if (!u_strcmp(cat,pL_MORPHO->L_CATS.cats[c].name))
      return &(pL_MORPHO->L_CATS.cats[c]);
  return NULL;
}

/**************************************************************************************/
/* If val is a valid value in the domain of category cat, returns the index of val    */
/* in cat. Otherwise returns -1.                                                      */
int is_valid_val(const l_category_T* cat, const unichar* val) {
  int v;
  for (v=0; v<cat->no_values; v++)
    if (!u_strcmp(val,cat->values[v]))
      return v;
  return -1;
}

/**************************************************************************************/
/* If val is an empty value in the domain of category cat, returns 1,                 */
/* otherwise returns 0.                                                               */
/* val is the ordinal number of the value in 'cat'                                    */
int is_empty_val(struct l_morpho_t* pL_MORPHO,l_category_T* cat, int val) {
  if (! u_strcmp(cat->values[val],pL_MORPHO->EMPTY_VAL))
    return 1;
  else
    return 0;
}

/**************************************************************************************/
/* If category 'cat' admits an empty value returns 1, otherwise returns 0.                                                               */
/* val is the ordinal number of the value in 'cat'                                    */
int admits_empty_val(struct l_morpho_t* pL_MORPHO,l_category_T* cat) {
  if (get_empty_val(pL_MORPHO,cat) >= 0)
    return 1;
  else
    return 0;
}

/**************************************************************************************/
/* If category 'cat' admits an empty value returns the ordinal number of this value   */
/* in 'cat'. Otherwise returns -1.                                                    */
int get_empty_val(struct l_morpho_t* pL_MORPHO,l_category_T* cat) {
  int v;  //Current value
  for (v=0; v<cat->no_values; v++)
    if (! u_strcmp(cat->values[v],pL_MORPHO->EMPTY_VAL))
      return v;
  return 0;
}

/**************************************************************************************/
/* If val is a valid value, returns the pointer to its (first) category.             */
/* Otherwise returns NULL.                                                            */
l_category_T* get_cat(struct l_morpho_t* pL_MORPHO,const unichar* val) {
  int c;
  int v;
  for (c=0; c<pL_MORPHO->L_CATS.no_cats; c++)
    for (v=0; v<pL_MORPHO->L_CATS.cats[c].no_values; v++)
      if (!u_strcmp(val,pL_MORPHO->L_CATS.cats[c].values[v]))
	return &(pL_MORPHO->L_CATS.cats[c]);
  return NULL;
}

/**************************************************************************************/
/* If 'cat' is a valid category, copies its name to 'cat_str' which should have its   */
/* space allocated, and returns 0. Otherwise returns 1.                               */
int copy_cat_str(unichar* cat_str,const l_category_T* cat) {
  if (!cat)
    return 1;
  u_strcpy(cat_str,cat->name);
  return 0;
}

/**************************************************************************************/
/* If 'cat' is a valid category, copies its name of its vale number 'val' to 'val_str'*/
/* which should have its space allocated, and returns 0. Otherwise returns 1.         */
int copy_val_str(unichar* val_str, l_category_T* cat, int val) {
  if (!cat)
    return 1;
  u_strcpy(val_str,cat->values[val]);
  return 0;
}

} // namespace unitex
