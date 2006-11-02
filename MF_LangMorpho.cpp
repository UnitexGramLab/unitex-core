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

/* Created by Agata Savary (savary@univ-tours.fr)
 * Last modification on June 2232005
 */
//---------------------------------------------------------------------------

#include <stdio.h>
#include "MF_LangMorpho.h"
#include "MF_Util.h"
#include "unicode.h"
#include "MF_Unif.h"   //debug
#include "Error.h"

///////////////////////////////////////////////////////////////////////////////////////
//Global structure describing the morphological categories of a language
l_cats_T L_CATS;
///////////////////////////////////////////////////////////////////////////////////////
//Global structure describing the morphological system of a language
l_classes_T L_CLASSES;

//Morphology file
FILE* lf;
//Current line of the morphology file
unichar line[MAX_LANG_MORPHO_LINE];
//Curent line number of the morphology file
int line_no = 0;
//Current character (not unichar) line, and current character word of the morphology file
char line_ch[MAX_LANG_MORPHO_LINE], word_ch[MAX_LANG_MORPHO_LINE];

//Says if we are in a category block (0) or in a class block (1) in the language morphology file
int CATS_OR_CLASSES;

///////////////////////////////////////
//All functions defined in this file
int read_language_morpho(char *lan_file, char *dir);
int read_cats();
int read_cat_line(int cat_no);
int read_classes();
int read_class_line(int class_no);
int print_language_morpho();
int free_language_morpho();
l_category_T* is_valid_cat(unichar* cat);
int is_valid_val(l_category_T* cat, unichar* val);
l_category_T* get_cat(unichar* val);

//////////////////////////////////////

/**************************************************************************************/
/* Read the language file "lan_file" in directory "dir".                              */
/* This file contains lists of all classes (nous, verb, etc.) of the language,        */
/* with their inflection categories (number, case, gender, etc.) and values           */
/* (e.g. sing, pl, masc, etc.).                                                       */
/* Categories must appear before classes.                                             */
/* E.g. for Polish:								      */
/* 			Polish							      */
/*                      <CATEGORIES>                                                  */
/* 			Nb:sing,pl		                 		      */
/* 			Case:Nom,Gen,Dat,Acc,Inst,Loc,Voc			      */
/* 			Gen:masc_pers,masc_anim,masc_inanim,fem,neu                   */
/*                      <CLASSES>                                                     */
/*                      noun: (Nb,<var>),(Case,<var>),(Gen,<fixed>)                   */
/*                      adj: (Nb,<var>),(Case,<var>),(Gen,<var>)                      */
/*                      adv:                                                          */
/* Fills out L_CLASSES								      */
/* Returns 0 if success, 1 otherwise                                                  */
int read_language_morpho(char *file) {
  if ( !(lf = u_fopen(file, "r")))  {
    error("Unable to open language morphology file %s !\n",file);
    return 1;
  }
  
  //Omit the first line (language name)
  if (! feof(lf)) {
    u_fgets(line,MAX_LANG_MORPHO_LINE-1,lf);
    line_no++;
  }

  //scan the following line
  u_fgets(line,MAX_LANG_MORPHO_LINE-1,lf);
  u_to_char(line_ch,line);
  sscanf(line_ch,"%s",word_ch);
  line_no++;

  if (! feof(lf))
    if (read_cats())
      return 1;
  if (! feof(lf))
    if (read_classes())
      return 1;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Read the category block of the language morphology file.                                     
// It should begin with <CATEGORIES>
// Before the beginning and after the end of the function "line" (global) contains 
// the current line of the morpho file.
int read_cats() {

  int cat_no; //category's number
  int l;   //lenght of the scanned line
  
  //Current line should contain <CATEGORIES>
  
  if (feof(lf) || strcmp(word_ch,"<CATEGORIES>")) {
    error("Language morphology file format incorrect in line %d!\n",line_no);
    return 1;
  }
  
  //Scan categories
  l = u_fgets(line,MAX_LANG_MORPHO_LINE-1,lf);
  u_to_char(line_ch,line);
  sscanf(line_ch,"%s",word_ch);
  line_no++;
  cat_no = 0;
  while (l && strcmp(word_ch,"<CLASSES>")) {
    if (read_cat_line(cat_no))
      return 1;
    l = u_fgets(line,MAX_LANG_MORPHO_LINE-1,lf);
    u_to_char(line_ch,line);
    sscanf(line_ch,"%s",word_ch);
    line_no++;
    cat_no++;
  }
  L_CATS.no_cats = cat_no;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Read a category line (having number cat_no) of the language morphology file.                                     
// Before the beginning of the function "line" (global) contains 
// the current line of the morpho file.
int read_cat_line(int cat_no) {
  
  int v_cnt;  //counter of values for the present category
  unichar* cat_name;   //category's name
  unichar* cat_val;    //category's value
  unichar tmp[MAX_MORPHO_NAME];  //buffer for line elements
  unichar tmp_void[MAX_MORPHO_NAME];  //buffer for void characters
  unichar* line_pos; //current position in the input line
  int l;  //length of a scanned sequence
  int done;
  
  line_pos = line;
  
  //Read category name
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
  l = u_scan_until_char(tmp,line_pos,MAX_MORPHO_NAME-1,": \t",1);
  line_pos = line_pos + l;
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
  if (*line_pos != (char) ':') {
    error("Language morphology file format incorrect: ':' missing in line %d!\n", line_no);
    return 1;
  }
  cat_name=u_strdup(tmp);
  L_CATS.cats[cat_no].name = cat_name;
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
    L_CATS.cats[cat_no].values[v_cnt] = cat_val;
    v_cnt++;
    if (*line_pos == (char) '\n')
      done = 1;
    else      line_pos++;  //Omit the ',' or the newline
  } while (!done);
  L_CATS.cats[cat_no].no_values = v_cnt;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Read that class block of the language morphology file.                                     
// It should begin with <CLASSES>.
// Before the beginning and after the end of the function "line" (global) contains 
// the current line of the morpho file.
int read_classes() {
  
  int class_no; //class number
  int l;   //lenght of the scanned line
  
  //Current line should contain <CLASSES>
  if (feof(lf) || strcmp(word_ch,"<CLASSES>")) {
    error("Language morphology file format incorrect: <CLASSES> missing in line %d!\n", line_no);
    return 1;
  }

 //Scan classes
  l = u_fgets(line,MAX_LANG_MORPHO_LINE-1,lf);
  u_to_char(line_ch,line);
  sscanf(line_ch,"%s",word_ch);
  line_no++;

  class_no = 0;
  while (l>1) {
    if (read_class_line(class_no))
      return 1;
    l = u_fgets(line,MAX_LANG_MORPHO_LINE-1,lf);
    u_to_char(line_ch,line);
    sscanf(line_ch,"%s",word_ch);
    line_no++;
    class_no++;
  }
  L_CLASSES.no_classes = class_no;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Read a class line (having number class_no) of the language morphology file.                                     
// Before the beginning and after the end of the function "line" (global) contains 
// the current line of the morpho file.
int read_class_line(int class_no) {

  int c_cnt;  //counter of categories for the present class
  unichar* class_name;   //class' name
  unichar tmp[MAX_MORPHO_NAME];  //buffer for line elements
  unichar tmp_void[MAX_MORPHO_NAME];  //buffer for void characters
  unichar* line_pos; //current position in the input line
  int l;  //length of a scanned sequence
  int done;
  int c;
  int found;

  line_pos = line;

  //Read class name
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
  l = u_scan_until_char(tmp,line_pos,MAX_MORPHO_NAME-1,": \t",1);
  line_pos = line_pos + l;
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
  class_name=u_strdup(tmp);
  L_CLASSES.classes[class_no].name = class_name;

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
      for (c=0, found=0; c<L_CATS.no_cats && !found; c++)
	if (! u_strcmp(L_CATS.cats[c].name,tmp))
	  found = 1;
      if (!found) {
	      error("Undefined category in language morphology file: line %d!\n", line_no);
	      return 1;
      }
      else
	L_CLASSES.classes[class_no].cats[c_cnt].cat = &(L_CATS.cats[c-1]);
      
      //Read the fixedness
      line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
      line_pos ++; //Omit the ','
      line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
      line_pos ++; //Omit the '<'
      line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
      l = u_scan_until_char(tmp,line_pos,MAX_MORPHO_NAME-1,"> \t",1);
      line_pos = line_pos + l;
      line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
      if (!u_strcmp_char(tmp,"fixed"))
	L_CLASSES.classes[class_no].cats[c_cnt].fixed = 1;
      else
	if (!u_strcmp_char(tmp,"var"))
	  L_CLASSES.classes[class_no].cats[c_cnt].fixed = 0;
	else {
	  error("Undefined fixedness symbol in language morphology file: line %d!\n", line_no);
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
  L_CLASSES.classes[class_no].no_cats = c_cnt;
  return 0;
}

/**************************************************************************************/
/* Prints to the standard output the morphological system of the language             */
/* as defined by L_CLASSES.       		    			              */
/* Returns 0 on success, 1 otherwise.                                                 */
int print_language_morpho() {
  unichar tmp[MAX_LANG_MORPHO_LINE];
  int c,v, cl;

  //Print categories
  u_strcpy_char(tmp, "<CATEGORIES>\n");
  u_prints(tmp);
  for (c=0; c<L_CATS.no_cats; c++) {
    //Print category
    u_prints(L_CATS.cats[c].name);
    u_strcpy_char(tmp,":");
    u_prints(tmp);
    //Print values
    for (v=0; v<L_CATS.cats[c].no_values; v++) {
      u_prints(L_CATS.cats[c].values[v]);
      if (v != L_CATS.cats[c].no_values-1) {
	u_strcpy_char(tmp,",");
	u_prints(tmp);
      }
    }
    u_strcpy_char(tmp,"\n");
    u_prints(tmp);
  }

  //Print classes
  u_strcpy_char(tmp, "<CLASSES>\n");
  u_prints(tmp);
  for (cl=0; cl<L_CLASSES.no_classes; cl++) {
    //print class
    u_prints(L_CLASSES.classes[cl].name);
    u_strcpy_char(tmp,":");
    u_prints(tmp);
    //print relevant categories
    for (c=0; c<L_CLASSES.classes[cl].no_cats; c++) {
      u_strcpy_char(tmp,"(");
      u_prints(tmp);
      //print category
      u_prints(L_CLASSES.classes[cl].cats[c].cat->name);
      u_strcpy_char(tmp,",<");
      u_prints(tmp);
      //print fixedness
      if (L_CLASSES.classes[cl].cats[c].fixed)
	u_strcpy_char(tmp,"fixed");
      else
	u_strcpy_char(tmp,"var");
      u_prints(tmp);
      u_strcpy_char(tmp,">)");
      u_prints(tmp);

      if (c != L_CLASSES.classes[cl].no_cats-1) {
	u_strcpy_char(tmp,",");
	u_prints(tmp);
      }	
    }
    u_strcpy_char(tmp,"\n");
    u_prints(tmp);
  }
  return 0;
}


/**************************************************************************************/
/* Liberates the space allocated for the language morphology description.             */
int free_language_morpho() {

  int c, v;

  //Liberate L_CATS
  for (c=0; c<L_CATS.no_cats; c++) {
    free(L_CATS.cats[c].name);
    for (v=0; v<L_CATS.cats[c].no_values; v++)
      free(L_CATS.cats[c].values[v]);
  }
	
  //Liberate L_CLASSES
  for (c=0; c<L_CLASSES.no_classes; c++)
    free(L_CLASSES.classes[c].name);

  return 0;
}

/**************************************************************************************/
/* If cat is a valid category name, returns a pointer to this category.               */
/* Otherwise returns NULL.                                                            */
l_category_T* is_valid_cat(unichar* cat) {
  int c;
  for (c=0; c<L_CATS.no_cats; c++)
    if (!u_strcmp(cat,L_CATS.cats[c].name))
      return &(L_CATS.cats[c]);
  return NULL;
}

/**************************************************************************************/
/* If val is a valid value in the domain of category cat, returns the index of val    */
/* in cat. Otherwise returns -1.                                                      */
int is_valid_val(l_category_T* cat, unichar* val) {
  int v;
  for (v=0; v<cat->no_values; v++)
    if (!u_strcmp(val,cat->values[v]))
      return v;
  return -1;
}

/**************************************************************************************/
/* If val is a valid value, returns the pointer to its (first) category.             */
/* Otherwise returns NULL.                                                            */
l_category_T* get_cat(unichar* val) {
  int c;
  int v;
  for (c=0; c<L_CATS.no_cats; c++)
    for (v=0; v<L_CATS.cats[c].no_values; v++)
      if (!u_strcmp(val,L_CATS.cats[c].values[v]))
	return &(L_CATS.cats[c]);
  return NULL;
}

/**************************************************************************************/
/* If 'cat' is a valid category, copies its name to 'cat_str' which should have its   */
/* space allocated, and returns 0. Otherwise returns 1.                               */
int copy_cat_str(unichar* cat_str,l_category_T* cat) {
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
