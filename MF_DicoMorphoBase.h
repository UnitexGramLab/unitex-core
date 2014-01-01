/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


#ifndef DicoMorphoBaseH
#define DicoMorphoBaseH

#include "Unicode.h"
//#include "MF_MU_morpho.h"
#include "MF_FormMorpho.h"

/**
 *
 * This library is used to parse the "Equivalences" file that is
 * supposed to be in the same directory than the inflection graphs.
 *
 */


#define CONFIG_FILES_OK 0
#define CONFIG_FILES_ERROR 1

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

//maximum length of a line in the file containing equivalences between morphological and dictionary values
#define MAX_EQUIV_LINE 100
//maximum number of equivalences between morphological and dictionary values
#define MAX_MORPHO_EQUIV 500
//maximum number of equivalences between dictionary strings and classes
#define MAX_CLASS_EQUIV 500

////////////////////////////////////////////
//A list of equivalences between morphological features in the language file
//and their surface forms in dictionaries.
typedef struct{
  unichar dico_feat;   //a morphological feature in a dictionary is a single character, e.g. 's'
  f_category_T cat;    //category-value pair corresponding to dico_feat in the current language, e.g. Nb=sing <Nb,0>
} d_morpho_eq_T;

// list of all inflectional code equivalences
typedef struct {
  int no_equiv;       //number of equivalences
  d_morpho_eq_T equiv[MAX_MORPHO_EQUIV];  //set of equivalences
} d_morpho_equiv_T;


////////////////////////////////////////////
//A list of equivalences between class names in a dictionary (e.g. "N") and a language class (e.g. noun).
#ifdef __GNUC__
#warning Later this table should be replaced by an external file scanned at each initialisation.
#elif ((defined(__VISUALC__)) || defined(_MSC_VER))
#pragma message("warning : Later this table should be replaced by an external file scanned at each initialisation.")
#endif
typedef struct {
  unichar dico_class[MAX_CLASS_NAME];   //morphological class as it appears in a dictionary (e.g. "N")
  l_class_T* cl;                       //language class
} d_class_eq_T;

// list of all grammatical code equivalences
// For instance, "Noun" is represented by "N" in Unitex
typedef struct {
  int no_equiv;       //number of class equivalences
  d_class_eq_T equiv[MAX_CLASS_EQUIV];  //set of class equivalences
} d_class_equiv_T;


} // namespace unitex

#endif
