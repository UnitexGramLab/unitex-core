/*
  * Unitex 
  *
  * Copyright (C) 2001-2003 Universit<E9> de Marne-la-Vall<E9>e <unitex@univ-mlv.fr>
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
 * Last modification on June 23 2005
 */
////////////////////////////////////////////////////////////////////
// TESTING MODULE


#include <stdio.h>
#include <string.h>
#include "MF_LangMorpho.h"
#include "unicode.h"
#include "MF_Unif.h"   //debug
#include "MF_FormMorpho.h"   //debug
#include "MF_SU_morpho.h"   //debug
#include "MF_Util.h"   //debug
#include "MF_MU_graph.h"   //debug
#include "MF_MU_morpho.h"   //debug
#include "Alphabet.h"
#include "MF_DicoMorpho.h"  //debug
#include "MF_DLC_inflect.h"  //debug

//Current language's alphabet
Alphabet* alph;

//Classes of the current language 
extern l_classes_T L_CLASSES;

int main() {
  //  unichar tmp[50];
  //Path and name of the alphabet file 
  char alphabet[1000];
  strcpy(alphabet,"/home/agata/UNITEX1.2beta_avec_flex_comp/UNITEX1.2beta_avec_flex_comp_22062005/Polish/Alphabet.txt");
  
  //N=Beginning of test of MF_LangMorho
  read_language_morpho("Morphology", "/home/agata/UNITEX1.2beta_avec_flex_comp/UNITEX1.2beta_avec_flex_comp_22062005/Polish/Inflection/");
  print_language_morpho();
  
  //Load alphabet
  alph = load_alphabet(alphabet);  //To be done once at the beginning of the DELAC inflection
  if (alph==NULL) {
    fprintf(stderr,"Cannot open alphabet file %s\n",alphabet);
    return 1;
  }
  
  //Init equivalence files
  d_init_morpho_equiv("Equivalences","/home/agata/UNITEX1.2beta_avec_flex_comp/UNITEX1.2beta_avec_flex_comp_22062005/Polish/Inflection/");
  d_print_morpho_equiv();
  d_init_class_equiv();
  
  //Initialise the structure for inflection transducers
  if (MU_graph_init_graphs())
    return 1;
  
 /*  
     unichar var[10], val[10], cat[10];
     unichar var1[10], val1[10], cat1[10];
     unichar var2[10], val2[10], cat2[10];
     unichar var3[10], val3[10], cat3[10];
     l_category_T *c, *c1, *c2, *c3;
     //  unif_init_vars();
     
     u_strcpy_char(cat,"Gen");
     //  u_strcpy_char(var,"g1");
     u_strcpy_char(val,"neu");
     
     u_strcpy_char(cat1,"Nb");
     //  u_strcpy_char(var1,"n37");
     u_strcpy_char(val1,"pl");
     
     u_strcpy_char(cat2,"Case");
     //  u_strcpy_char(var2,"c");
     u_strcpy_char(val2,"Nom");
     
     
     u_strcpy_char(cat3,"Nb");
     //  u_strcpy_char(var3,"n");
     u_strcpy_char(val3,"sing");
     
     c = is_valid_cat(cat);
     c1 = is_valid_cat(cat1);
     c2 = is_valid_cat(cat2);
     c3 = is_valid_cat(cat3);
     
     //Beginning of test of MF_FormMorpho
     f_morpho_T feat;
     
     //Initialise features
     f_init_morpho(&feat);
     //f_add_morpho(&feat,c,is_valid_val(c,val));
     //f_add_morpho(&feat,c1,is_valid_val(c1,val1));  
     f_add_morpho_char(&feat,cat,val);
     f_add_morpho_char(&feat,cat1,val1);
 */
  //Print features
  /*  u_strcpy_char(tmp,"\n");
      u_prints(tmp);
      f_print_morpho(&feat);
      u_strcpy_char(tmp,"\n");
      u_prints(tmp);
  */
  //Check features
  /*  u_prints(feat.cats[1].cat->name);    //Print the category
      u_strcpy_char(tmp,"=");
      u_prints(tmp);
      int i;
      i = f_get_value(&feat,c1);
      u_prints(feat.cats[1].cat->values[i]);    //Print the value
      u_strcpy_char(tmp,"\n\n");
      u_prints(tmp);
  */

/*//Change features
  f_morpho_T new_feat;
  //new_feat.no_cats = 1;
  //new_feat.cats[0].cat = c3;
  //new_feat.cats[0].val = is_valid_val(c3,val3);
  f_init_morpho(&new_feat);
  f_add_morpho_char(&new_feat,cat3,val3);
  u_strcpy_char(tmp,"New features : \n");
  u_prints(tmp);
  f_print_morpho(&new_feat);
  f_change_morpho(&feat, &new_feat);
  u_strcpy_char(tmp,"Changed features : \n");
  u_prints(tmp);
  f_print_morpho(&feat);
  u_strcpy_char(tmp,"\n");
  u_prints(tmp);
*/ 
  //Add features
  /*  f_add_morpho(&feat, c2, is_valid_val(c2,val2));
      u_strcpy_char(tmp,"Enlarged features : \n");
      u_prints(tmp);
      f_print_morpho(&feat);
      u_strcpy_char(tmp,"\n");
      u_prints(tmp);
  */
  //Reduce features
  /*  u_strcpy_char(tmp,"Feature to be eliminated : ");
      u_prints(tmp);
      u_prints(c->name);
      u_strcpy_char(tmp,"\n");
      u_prints(tmp);
      f_del_morpho(&feat, c);
      u_strcpy_char(tmp,"Reduced features : \n");
      u_prints(tmp);
      f_print_morpho(&feat);
      u_strcpy_char(tmp,"\n");
      u_prints(tmp);
  */
  
  //End of test of MF_FormMorpho
  
  /*
  //Beginning of test of MF_SU_morpho
  f_morpho_T feat1;
  u_strcpy_char(cat,"Gen");
  u_strcpy_char(val,"fem");
  u_strcpy_char(cat1,"Nb");
  u_strcpy_char(val1,"sing");
  u_strcpy_char(cat2,"Case");
  u_strcpy_char(val2,"Inst");
  f_init_morpho(&feat1);
  f_add_morpho_char(&feat1,cat,val);
  f_add_morpho_char(&feat1,cat1,val1);
  f_add_morpho_char(&feat1,cat2,val2);
  //reka
  SU_lemma_T l_reka;
  SU_id_T id_reka;
  SU_init_lemma(&l_reka,"reka","noun","N41");
  SU_print_lemma(&l_reka);
  id_reka.id.word_id.SU_lemma = &l_reka;
  id_reka.id.word_id.form_nr = 5;
  SU_forms_T forms_reka;
  SU_inflect(&id_reka,&feat1,&forms_reka);
  SU_print_forms(&forms_reka);
  f_morpho_T* feat2;
  feat2 = SU_get_features(&id_reka);
  f_print_morpho(feat2);
  SU_delete_features(feat2);
  //zlamany
  SU_lemma_T l_zl;
  SU_id_T id_zl;
  SU_init_lemma(&l_zl,"zlamany","adj","A6");
  SU_print_lemma(&l_zl);
  id_zl.id.word_id.SU_lemma = &l_zl;
  id_zl.id.word_id.form_nr = 5;
  SU_forms_T forms_zl;
  SU_inflect(&id_zl,&feat1,&forms_zl);
  SU_print_forms(&forms_zl);
  //separator
  SU_id_T id_sep;
  id_sep.id.sep_id = (unichar) '-';
  SU_forms_T forms_sep;
  SU_inflect(&id_sep,&feat1,&forms_sep);
  SU_print_forms(&forms_sep);
  //test of SU_get_id
  SU_id_T *id_zl1;
  id_zl1 = SU_get_id(NULL,&l_zl);
  SU_print_lemma(id_zl1->id.word_id.SU_lemma);
  printf("form_nr = %d\n",id_zl1->id.word_id.form_nr);
  

  SU_delete_lemma(&l_reka);
  SU_delete_inflection(&forms_reka);
  SU_delete_lemma(&l_zl);
  SU_delete_inflection(&forms_zl);
  SU_delete_inflection(&forms_sep);
  SU_free_id(id_zl1);
  //End of test of MF_SU_morpho
  */

  /*  if (c2)
    printf("Valid category 2\n");
  else
    printf("INVALID CATEGORY 2\n");
  
  if (is_valid_val(c,val))
    printf("Valid value\n");
  else
    printf("INVALID VALUE\n");
  */
  //End of test of MF_LangMorpho
  
  //Beginning of test of MF_Unif
  /*
  int res;
  if ((res = unif_instantiate(var, c, val)) != -1)
      printf("Instantiation 0 succeeded\n");
  else
    printf("INSTANTIATION 0 DIDN'T SUCCEED\n");

  if ((res = unif_instantiate(var1, c1, val1)) != -1)
    printf("Instantiation 1 succeeded\n");
  else
    printf("INSTANTIATION 1 DIDN'T SUCCEED\n");

  if ((res = unif_instantiate(var2, c2, val2)) != -1)
    printf("Instantiation 2 succeeded\n");
  else
    printf("INSTANTIATION 2 DIDN'T SUCCEED\n");

  if((res = unif_instantiate(var3, c3, val3)) != -1)
    printf("Instantiation 3 succeeded\n");
  else
    printf("INSTANTIATION 3 DIDN'T SUCCEED\n");

  unif_print_vars();

  unif_desinstantiate(var);
  unif_print_vars();

  u_prints(var3);
  printf(" instantiated to value ");
  u_prints(unif_get_val(var3));
  printf(" of category ");
  u_prints(unif_get_cat(var3)->name);
  printf("\n");
 
  unif_free_vars();
  */ 
  //End of test of MF_Unif

  /*  //Test of u_scan_while
  unichar tmp4[50], tmp5[50], tmp6[50];
  unichar *p;
  int l4;
  u_strcpy_char(tmp4,"  \t\n cde  ");
  p = tmp4;
  u_strcpy_char(tmp5," \t\n");
  l4 = u_scan_while_char(tmp6,tmp4,49," \t\n");X
  p += l4;
  printf("\nInitial string: ");
  u_prints(tmp4);
  printf("\nString after omitting void characters:\n");
  u_prints(p);
  printf("\n");
  // end of test of u_scan_while
  */

  /*
  //Test of MF_MU_graph
  printf("\n\n");
  MU_graph_label_T lab;
  unichar in[100], out[100];

  u_strcpy_char(in,"<$1 : Gen=$g; Nb=$n; Case=Inst>");
  //  u_strcpy_char(out,"<E>");
  u_strcpy_char(out,"<Gen=$g; Case = Acc ; Nb=$n1 >\t");

  MU_graph_scan_label(in,out,&lab);
  MU_graph_print_label(&lab);
  MU_graph_free_label(&lab);
  //End of test of MF_MU_graph
  */  

  /*
 /////////////////
  //Initialize the lemma "zlamana reka"
  ///////////////
  // Initialize "zlamana"
  SU_lemma_T* l_zl;
  l_zl = (SU_lemma_T*) malloc(sizeof(SU_lemma_T));
  SU_id_T* id_zl;
  id_zl = (SU_id_T*) malloc(sizeof(SU_id_T));
  SU_init_lemma(l_zl,"zlamany","adj","A6");
  SU_print_lemma(l_zl);
  id_zl->form = (unichar*) malloc(9*sizeof(unichar));
  u_strcpy_char(id_zl->form,"zlamanym");
  id_zl->lemma = l_zl;
  //  id_zl->form_nr = 5;
  
  //Initialize a space
  SU_id_T* id_space;
  id_space = (SU_id_T*) malloc(sizeof(SU_id_T));
  id_space->form = (unichar*) malloc(2*sizeof(unichar));
  u_strcpy_char(id_space->form," ");
  id_space->lemma = NULL;
  //  id_space->form_nr=-1;
  
  // Initialize "reka"
  SU_lemma_T* l_reka;
  l_reka = (SU_lemma_T*) malloc(sizeof(SU_lemma_T));
  SU_id_T* id_reka;
  id_reka = (SU_id_T*) malloc(sizeof(SU_id_T));
  SU_init_lemma(l_reka,"reka","noun","N41");
  SU_print_lemma(l_reka);
  id_reka->form = (unichar*) malloc(5*sizeof(unichar));
  u_strcpy_char(id_reka->form,"reka");
  id_reka->lemma = l_reka;
  //  id_reka->form_nr = 5;

  // Initialize "okno"
  SU_lemma_T* l_okno;
  l_okno = (SU_lemma_T*) malloc(sizeof(SU_lemma_T));
  SU_id_T* id_okno;
  id_okno = (SU_id_T*) malloc(sizeof(SU_id_T));
  SU_init_lemma(l_okno,"okno","noun","N42");
  SU_print_lemma(l_okno);
  id_okno->form = (unichar*) malloc(6*sizeof(unichar));
  u_strcpy_char(id_okno->form,"oknem");
  id_okno->lemma = l_okno;
  //  id_okno->form_nr = 5;
  */
  /*
  //Test of SU_inflect
  unichar cat1[10], cat2[10], val1[10], val2[10];
  l_category_T *c1, *c2;
  u_strcpy_char(cat1,"Gen");
  u_strcpy_char(val1,"f");
  u_strcpy_char(cat2,"Case");
  u_strcpy_char(val2,"I");
  f_morpho_T feat;
  f_init_morpho(&feat);
  f_add_morpho_unichar(&feat,cat1,val1);
  f_add_morpho_unichar(&feat,cat2,val2);

  SU_forms_T* F;
  F = (SU_forms_T*) malloc(sizeof(SU_forms_T));

  SU_inflect(id_zl,&feat,F);
  SU_print_forms(F);
  SU_delete_inflection(F);

  SU_delete_id(id_zl);
  //  SU_delete_id(id_space);
  SU_delete_id(id_reka);
  //End of test of SU_inflect
  */

  /*
  //Test of MU_inflect
  //Initialize the single units
  MU_lemma_T* MU_lemma;
  MU_lemma = (MU_lemma_T*) malloc(sizeof(MU_lemma_T));
  MU_forms_T* MU_forms;
  MU_forms = (MU_forms_T*) malloc(sizeof(MU_forms_T));

  MU_lemma->units[0] = id_zl;
  MU_lemma->units[1] = id_space;
  MU_lemma->units[2] = id_okno;
  MU_lemma->no_units = 3;
  MU_lemma->cl =  &(L_CLASSES.classes[0]);    //class: noun
  MU_lemma->paradigm = (char*) malloc(30*sizeof(char));  
  strcpy(MU_lemma->paradigm,"NC4");   //paradigm: "ZlamanaReka_instr"
  MU_lemma->codes[0] = NULL;   //No codes after '+'
  MU_lemma->comment = NULL;    //No comment after '\'
  MU_print_lemma(MU_lemma);
 
  MU_inflect(MU_lemma, MU_forms);
  
  //Print the generated forms
  MU_print_forms(MU_forms);
  
  MU_delete_inflection(MU_forms);
  MU_delete_lemma(MU_lemma);  
  //End of test of MU_inflect
  */

  //Test of DLC_line2entry

  unichar line[1000];
  /*
  u_strcpy_char(line,"z\\,l,a.mkl\\\\oiuy;p");
  char delim[10];
  strcpy(delim,";\\");
  unichar test[100];
  int l = u_scan_until_char(test,line,100,delim,0);
  u_prints(test);
  printf(" %d\n",l);
  */
  
  /*
  //Test of DLC_line2entry
  DLC_entry_T entry;
  //  u_strcpy_char(line,"zlamana(zlamany.A6:Ifs) reka(reka.N41:fIs),NC5+Conc/med");
  u_strcpy_char(line,"zlamanie(zlamanie.N42:Mns) reki(reka.N41:fDs),NC7+Conc/med");
  int err = DLC_line2entry(line, &entry);
  if (!err) {
    printf("%d constituents\n",entry.lemma->no_units);
    DLC_print_entry(&entry);
  }
  //End of test of DLC_line2entry

  MU_forms_T* MU_forms;
  MU_forms = (MU_forms_T*) malloc(sizeof(MU_forms_T));

  MU_inflect(entry.lemma, MU_forms);
  
  //Print the generated forms
  MU_print_forms(MU_forms);
  
  MU_delete_inflection(MU_forms);
  */


  //Test of DLC_inflect
  DLC_inflect("/home/agata/UNITEX1.2beta_avec_flex_comp/UNITEX1.2beta_avec_flex_comp_22062005/Polish/Dela/Delac/test.dlc.uni","/home/agata/UNITEX1.2beta_avec_flex_comp/UNITEX1.2beta_avec_flex_comp_22062005/Polish/Dela/Delacf/test.dlcf.uni");
  //End of test of DLC_inflect

  MU_graph_free_graphs();
  
  free_alphabet(alph);
  free_language_morpho();
  return 0;
}
