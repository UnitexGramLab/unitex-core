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
 */

#ifndef LangMorphoBaseH
#define LangMorphoBaseH

#include "Unicode.h"
//#include "MF_DicoMorpho.h" //add
//#include "MF_DicoMorphoBase.h" //add

/**
 * 
 * This library is used to parse the "Morphology" file that is 
 * supposed to be in the same directory than the inflection graphs.
 * 
 */




//maximum number of inflection categories (number, gender etc.)
#define MAX_CATS 100
//maximum number of inflection classes (noun, adj, etc.)
#define MAX_CLASSES 50
//maximum length of a class name in a dictionary
#define MAX_CLASS_NAME 10
//maximum number of values for an inflection category
#define MAX_CAT_VALUES 500
//maximum length of a line in the language morhology file
#define MAX_LANG_MORPHO_LINE 500
//maximum length of a class, category or value name
#define MAX_MORPHO_NAME 20

//Structure for an inflection category (cf Inflection/Morphology file)
//e.g. for English 
//          name = "Nb"  (number inflection)
//          no_values = 2 
//          values = ["sing","pl"] (singular, plural)
typedef struct {
  unichar* name;
  int no_values;
  unichar* values[MAX_CAT_VALUES];
} l_category_T;

//Structure for all inflection categories
typedef struct {
  int no_cats;     //number of categories
  l_category_T cats[MAX_CATS];
} l_cats_T;

//Structure for an inflection class (cf Inflection/Morphology file)
//e.g. for Polish 
//          name = "noun"
//          no_cats = 3 
//          cats = ((Nb,0),(Case,0),(Gen,1))  // a Polish noun HAS a gender and INFLECTS IN number and case
typedef struct {
  unichar* name;   
  int no_cats; 
  struct {
    l_category_T* cat;
    int fixed;
  } cats[MAX_CATS];
} l_class_T;

//From the viewpoint of morphology a language is a list of classes (noun, verb, etc.)
//which are lists of categories (number, case, gender, etc.) and their domains.
typedef struct{
  int no_classes;      //e.g. 7
  l_class_T classes[MAX_CLASSES];  //e.g. (noun, verb, etc.)
} l_classes_T;


#endif
