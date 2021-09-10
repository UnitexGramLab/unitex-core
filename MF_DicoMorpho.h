/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


#ifndef DicoMorphoH
#define DicoMorphoH

#include "Unicode.h"
#include "MF_MU_morpho.h"


/**
 *
 * This library is used to parse the "Equivalences" file that is
 * supposed to be in the same directory than the inflection graphs.
 *
 */
#include "MF_DicoMorphoBase.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {



/**************************************************************************************/
/* Initialises the set of equivalences between morphological and dictionary features. */
/* For instance in Unitex the dictionary features are represented by single characters*/
/* e.g. 's', with no precision of the relevant category. In the compound inflection   */
/* the features may be strings, e.g. "sing", and have to refer to a category, e.g. Nb */
/* 'equiv_file' is a file describing these equivalences for a given language          */
/* Each line of the file is of the form:                                              */
/*      <df>:<cat>=<val>                                                              */
/* meaning that in the morphological dictionaries of the given language the feature   */
/* 'df' corresponds to category 'cat' taking value 'val'. Each 'cat' and 'val' has to */
/* has to appear in the 'Morphology' file of the given language.                      */
/* E.g. for Polish:                                                                   */
/*                    Polish                                                          */
/*                    s:Nb=sing                                                       */
/*                    p:Nb=pl                                                         */
/*                    N:Case=Nom                                                      */
/*                    G:Case=Gen                                                      */
/*                    D:Case=Dat                                                      */
/*                    A:Case=Acc                                                      */
/*                    I:Case=Inst                                                     */
/*                    L:Case=Loc                                                      */
/*                    V:Case=Voc                                                      */
/*                    o:Gen=masc_pers                                                 */
/*                    z:Gen=masc_anim                                                 */
/*                    r:Gen=masc_inanim                                               */
/*                    f:Gen=fem                                                       */
/*                    n:Gen=neu                                                       */
/* The function fills out D_MORPHO_EQUIV.                                             */
/* Returns 0 on success, 1 otherwise.                                                 */
int d_init_morpho_equiv(const VersatileEncodingConfig*,struct l_morpho_t* pL_MORPHO,const char* equiv_file);

/**************************************************************************************/
/* Initialises the set of equivalences between class names in a dictionary (e.g. "N") */
/* and language classes (e.g. noun)                                                   */
/* This function is temporarily done for Polish. In future it has to be replaced by   */
/* a function scanning an external equivalence file for the given language.           */
void d_init_class_equiv(struct l_morpho_t* pL_MORPHO,d_class_equiv_T *D_CLASS_EQUIV);

/**************************************************************************************/
/* Prints to the standard output the equivalences between dictionary and morphology   */
/* features.                                                                          */
void d_print_morpho_equiv();

/**************************************************************************************/
/* Produces a set of structured inflection features (e.g. <Gen=f;Case=I;Nb=s>) from   */
/* a string (e.g. "fIs").                                                             */
/* If the string component is not equivalent to a morphological feature, returns NULL.*/
/* The return structure is allocated in the function. The liberation has to take place*/
/* in the calling function (by f_delete_morpho).                                      */
f_morpho_T* d_get_feat_str(struct l_morpho_t* pL_MORPHO,unichar* feat_str);

/**************************************************************************************/
/* Produces a feature string (e.g. "fIs") from a set of structured inflection features*/
/* (e.g. <Gen=f;Case=I;Nb=s>).                                                        */
/* The return string is allocated in the function. The liberation has to take place   */
/* in the calling function.                                                           */
/* If 'feat' is empty or a morphological feature has no corresponding character value,*/
/* returns NULL.                                                                      */
unichar* d_get_str_feat(struct l_morpho_t* pL_MORPHO,f_morpho_T* feat);

/**************************************************************************************/
/* Returns the class (e.g. noun) corresponding to a class string as it appears in a   */
/* dictionary (e.g. "N"). If no class corresponds to the string, returns NULL.        */
/* The returned structure is NOT allocated in the function.                           */
l_class_T* d_get_class_str(unichar* cl_str,d_class_equiv_T* D_CLASS_EQUIV);

/**************************************************************************************/
/* Returns the class string (e.g. 'N') corresponding to a class (e.g. noun)           */
/*If no string corresponds to the class, returns NULL.                                */
/* The return structure is NOT allocated in the function.                             */
unichar* d_get_str_class(l_class_T* cl,d_class_equiv_T* D_CLASS_EQUIV);

} // namespace unitex

#endif
