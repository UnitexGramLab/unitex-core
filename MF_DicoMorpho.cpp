/*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
 */

#include <stdio.h>

#include "MF_Global.h"
#include "MF_LangMorpho.h"
#include "MF_FormMorpho.h"
#include "MF_DicoMorpho.h"
#include "MF_Util.h"
#include "Error.h"


///////////////////////////////////////
//All functions defined in this file
int d_init_morpho_equiv(struct l_morpho_t* pL_MORPHO,char* equiv_file, char *di);
int d_read_line(struct l_morpho_t* pL_MORPHO,unichar* line,int line_no);
void d_init_class_equiv();
void d_print_morpho_equiv();
f_morpho_T* d_get_feat_str(struct l_morpho_t* pL_MORPHO,unichar* feat_str);
unichar* d_get_str_feat(struct l_morpho_t* pL_MORPHO,f_morpho_T* feat);
l_class_T* d_get_class_str(unichar* cl_str);
unichar* d_get_str_class(l_class_T* cl);

/**************************************************************************************/
/* Initialises the set of equivalences between morphological and dictionary features. */
/* For instance in Unitex the dictionary features are represented by single characters*/
/* e.g. 's', with no precision of the relevant category. In the compound inflection   */
/* the features may be strings, e.g. "sing", and have to refer to a category, e.g. Nb */
/* 'equiv_file' is a file describing these equivalences for a given language          */
/* 'dir' is the directory containing 'equiv_file'                                     */
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
/* The function fills out pL_MORPHO->D_MORPHO_EQUIV.                                             */
/* Returns 0 on success, 1 otherwise.                                                 */
int d_init_morpho_equiv(struct l_morpho_t* pL_MORPHO,char* equiv_file) {

  U_FILE* ef; //equivalence file
  int line_no;  //number of the current line
  unichar line[MAX_EQUIV_LINE];  //current line of the Equivalence file

  //Opening the equivalence file
  ef = u_fopen_existing_unitex_text_format(equiv_file,U_READ);
  if (!ef)  {
    error("Unable to open equivalence file %s\n",equiv_file);
    return 1;
  }

  //Initialize pL_MORPHO->D_MORPHO_EQUIV
  pL_MORPHO->D_MORPHO_EQUIV.no_equiv = 0;

  line_no = 0;

  //Omit the first line (language name)
  if (u_fgets(line,MAX_EQUIV_LINE-1,ef)!=EOF) {
    line_no++;
  } else {
   u_fclose(ef);
   return 1;
  }

  while (u_fgets(line,MAX_EQUIV_LINE-1,ef)!=EOF) {
    line_no++;
    int l=u_strlen(line);
    if (l>0 && line[l-1]=='\n') {
       /* If necessary, we remove the final \n */
       line[l-1]='\0';
    }
    if (line[0]!='\0' && d_read_line(pL_MORPHO,line,line_no)) return 1;
  }
  u_fclose(ef);
  return 0;
}

/**************************************************************************************/
/* Read a line of the morphology-dictionary equivalence file, e.g. s:Nb=sing          */
/* Fill out a line of pL_MORPHO->D_MORPHO_EQUIV.                                                 */
/* Return 0 on success, 1 otherwise.                                                  */
int d_read_line(struct l_morpho_t* pL_MORPHO,unichar* line, int line_no) {
  int l;   //Length of a scanned sequence
  unichar* line_pos;   //current position in the input line
  unichar tmp[MAX_MORPHO_NAME];  //buffer for line elements
  unichar tmp_void[MAX_MORPHO_NAME];  //buffer for void characters
  l_category_T* cat;

  line_pos = line;

  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters

  //Read the dictionary value
  l = u_scan_until_char(tmp,line_pos,MAX_MORPHO_NAME-1,": \t",1);
  line_pos = line_pos + l;
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters

  if (line_pos[0] != (unichar) ':') {
    error("Bad format in \'Equivalence\' file:\n");
    error("Line %d: a \':\' missing.\n",line_no);
    return 1;
  }
  if (u_strlen(tmp) != 1) {
    error("Bad format in \'Equivalence\' file:\n");
    error("Line %d: s morphological values in a dictionary must be of one caracter.\n", line_no);
    return 1;
  }
  pL_MORPHO->D_MORPHO_EQUIV.equiv[pL_MORPHO->D_MORPHO_EQUIV.no_equiv].dico_feat = tmp[0];
  line_pos++;  //Skip the ':'

  //Read the category
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
  l = u_scan_until_char(tmp,line_pos,MAX_MORPHO_NAME-1,"= \t",1);
  line_pos = line_pos + l;
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
  if (!u_strlen(tmp)) {
    error("Bad format in \'Equivalence\' file:\n");
    error("Line %d: category missing.\n",line_no);
    return 1;
  }
  if (line_pos[0] != (unichar) '=') {
    error("Bad format in \'Equivalence\' file:\n");
    error("Line %d: a \'=\' missing.\n",line_no);
    return 1;
  }
  cat = is_valid_cat(pL_MORPHO,tmp);
  if (!cat) {
    error("In \'Equivalence\' file:\n");
    error("%S is not a valid category in line %d.\n",tmp,line_no);
    return 1;
  };
  pL_MORPHO->D_MORPHO_EQUIV.equiv[pL_MORPHO->D_MORPHO_EQUIV.no_equiv].cat.cat = cat;
  line_pos++;  //Skip the '='

  //Read the value
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
  l = u_scan_until_char(tmp,line_pos,MAX_MORPHO_NAME-1," \t",1);
  line_pos = line_pos + l;
  line_pos = line_pos + u_scan_while_char(tmp_void, line_pos, MAX_MORPHO_NAME-1," \t");  //Omit void characters
  if (!u_strlen(tmp)) {
    error("Bad format in \'Equivalence\' file\n");
    error("Line %d: value missing.\n",line_no);
    return 1;
  }
  if ((pL_MORPHO->D_MORPHO_EQUIV.equiv[pL_MORPHO->D_MORPHO_EQUIV.no_equiv].cat.val = is_valid_val(cat,tmp)) == -1) {
    error("In \'Equivalence\' file\n");
    error("%S is not a valid value in line %d.\n",tmp,line_no);
    return 1;
  }
  if (line_pos[0]) {
    error("Bad format in \'Equivalence\' file\n");
    error("Line %d: unnecessary string:%S.:\n",line_no,line_pos);
    return 1;
  }

  pL_MORPHO->D_MORPHO_EQUIV.no_equiv++;
  return 0;
}

/**************************************************************************************/
/* Prints to the standard output the equivalences between dictionary qnd morphology   */
/* features.                                                                          */
void d_print_morpho_equiv(struct l_morpho_t* pL_MORPHO) {
int e;  //index of the current equivalence
unichar tmp[MAX_MORPHO_NAME+1];
for (e=0; e<pL_MORPHO->D_MORPHO_EQUIV.no_equiv; e++) {
   u_printf("%C:",pL_MORPHO->D_MORPHO_EQUIV.equiv[e].dico_feat);
   copy_cat_str(tmp,pL_MORPHO->D_MORPHO_EQUIV.equiv[e].cat.cat);
   u_printf("%S=",tmp);
   copy_val_str(tmp,pL_MORPHO->D_MORPHO_EQUIV.equiv[e].cat.cat,pL_MORPHO->D_MORPHO_EQUIV.equiv[e].cat.val);
   u_printf("%S\n",tmp);
}
}


/**************************************************************************************/
/* Initialises the set of equivalences between class names in a dictionary (e.g. "N") */
/* and language classes (e.g. noun)                                                   */
/* This function is temporarily done for Polish. In future it has to be replaced by   */
/* a function scanning an external equivalence file for the given language.           */
void d_init_class_equiv(struct l_morpho_t* pL_MORPHO,d_class_equiv_T *D_CLASS_EQUIV) {
  //Noun
  u_strcpy(D_CLASS_EQUIV->equiv[0].dico_class,"N");
  D_CLASS_EQUIV->equiv[0].cl = &(pL_MORPHO->L_CLASSES.classes[0]);
  u_strcpy(D_CLASS_EQUIV->equiv[1].dico_class,"NC");
  D_CLASS_EQUIV->equiv[1].cl = &(pL_MORPHO->L_CLASSES.classes[0]);

  //Adjectif
  u_strcpy(D_CLASS_EQUIV->equiv[2].dico_class,"A");
  D_CLASS_EQUIV->equiv[2].cl = &(pL_MORPHO->L_CLASSES.classes[1]);
  u_strcpy(D_CLASS_EQUIV->equiv[3].dico_class,"AC");
  D_CLASS_EQUIV->equiv[3].cl = &(pL_MORPHO->L_CLASSES.classes[1]);

  //Adverb
  u_strcpy(D_CLASS_EQUIV->equiv[4].dico_class,"ADV");
  D_CLASS_EQUIV->equiv[4].cl = &(pL_MORPHO->L_CLASSES.classes[2]);

  //Verb
  u_strcpy(D_CLASS_EQUIV->equiv[5].dico_class,"V");
  D_CLASS_EQUIV->equiv[5].cl = &(pL_MORPHO->L_CLASSES.classes[3]);
  u_strcpy(D_CLASS_EQUIV->equiv[6].dico_class,"VC");
  D_CLASS_EQUIV->equiv[6].cl = &(pL_MORPHO->L_CLASSES.classes[3]);

  D_CLASS_EQUIV->no_equiv = 7;
}

/**************************************************************************************/
/* Produces a set of structured inflection features (e.g. <Gen=f;Case=I;Nb=s>) from   */
/* a string (e.g. "fIs").                                                             */
/* If a string component is not equivalent to a morphological feature, returns NULL.  */
/* The return structure is allocated in the function. The liberation has to take place*/
/* in the calling function (by f_delete_morpho).                                                           */
f_morpho_T* d_get_feat_str(struct l_morpho_t* pL_MORPHO,unichar* feat_str) {
  int e;  //index of the current equivalence
  int f;  //index of the current feature in the string 'feat_str'
  int found;
  f_morpho_T* feat;
  feat = (f_morpho_T*) malloc(sizeof(f_morpho_T));
  if (!feat) {
    fatal_alloc_error("d_get_feat_str");
  }
  f_init_morpho(feat);
  for (f=0; f<u_strlen(feat_str); f++) {
    found = 0;
    for (e=0; e<pL_MORPHO->D_MORPHO_EQUIV.no_equiv && !found; e++)
      if (pL_MORPHO->D_MORPHO_EQUIV.equiv[e].dico_feat == feat_str[f]) {
	f_add_morpho(feat, pL_MORPHO->D_MORPHO_EQUIV.equiv[e].cat.cat, pL_MORPHO->D_MORPHO_EQUIV.equiv[e].cat.val);
	found = 1;
      }
    if (!found)
      return NULL;
  }
  return feat;
}

/**************************************************************************************/
/* Produces a feature string (e.g. "fIs") from a set of structured inflection features*/
/* (e.g. <Gen=f;Case=I;Nb=s>).                                                        */
/* The return string is allocated in the function. The liberation has to take place   */
/* in the calling function.                                                           */
/* If 'feat' empty or a morphological feature has no corresponding character value, returns NULL.   */
unichar* d_get_str_feat(struct l_morpho_t* pL_MORPHO,f_morpho_T* feat) {
  int f;  //index of the current category-value pair in 'feat'
  int ef; //index of the current category-value pair in pL_MORPHO->D_MORPHO_EQUIV
  unichar* tmp;
  int c;   //index of the current character in tmp;
  tmp = (unichar*) malloc((MAX_CATS+1) * sizeof(unichar));
  if (!tmp) {
    fatal_alloc_error("d_get_str_feat");
  }
  tmp[0] = (unichar) '\0';
  c = 0;
  int found;

  if (!feat)
    return NULL;

  for (f=0; f<feat->no_cats; f++) {
    found = 0;
    for (ef=0; ef<pL_MORPHO->D_MORPHO_EQUIV.no_equiv && !found; ef++) {
      if ((feat->cats[f].cat == pL_MORPHO->D_MORPHO_EQUIV.equiv[ef].cat.cat) && (feat->cats[f].val == pL_MORPHO->D_MORPHO_EQUIV.equiv[ef].cat.val) ) {
	tmp[c] = pL_MORPHO->D_MORPHO_EQUIV.equiv[ef].dico_feat;
	c++;
	found = 1;
      }
    }
    //If a feature was not found in equivalences and it is not empty
    //no string corresponds to the desired morphology
    if (!found && !is_empty_val(pL_MORPHO,feat->cats[f].cat,feat->cats[f].val))
      return NULL;
    //If the feature is empty, it is simply omitted from the string
  }
  tmp[c] = (unichar) '\0';
  return tmp;
}

/**************************************************************************************/
/* Returns the class (e.g. noun) corresponding to a class string as it appears in a   */
/* dictionary (e.g. "N"). If no class corresponds to the string, returns NULL.        */
/* The return structure is NOT allocated in the function.                             */
l_class_T* d_get_class_str(unichar* cl_str,d_class_equiv_T* D_CLASS_EQUIV) {
  int c;  //index of the current class in the equivalence set
  for (c=0; c<D_CLASS_EQUIV->no_equiv; c++)
    if (!u_strcmp(cl_str,D_CLASS_EQUIV->equiv[c].dico_class))
      return D_CLASS_EQUIV->equiv[c].cl;
  return NULL;

}

/**************************************************************************************/
/* Returns the class string (e.g. 'N') corresponding to a class (e.g. noun)           */
/*If no string corresponds to the class, returns NULL.                                */
/* The return structure is NOT allocated in the function.                             */
unichar* d_get_str_class(l_class_T* cl,d_class_equiv_T* D_CLASS_EQUIV) {
  int c;  //index of the current class in the equivalence set
  for (c=0; c<D_CLASS_EQUIV->no_equiv; c++)
    if (cl == D_CLASS_EQUIV->equiv[c].cl)
      return D_CLASS_EQUIV->equiv[c].dico_class;
  return NULL;

}
