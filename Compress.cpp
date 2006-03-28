 /*
  * Unitex
  *
  * Copyright (C) 2001-2005 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unicode.h"
#include "DELA.h"
#include "DictionaryTree.h"
#include "String_hash.h"
#include "Arbre_to_bin.h"
#include "FileName.h"
#include "Minimize_tree.h"
#include "Copyright.h"
#include "IOBuffer.h"

//---------------------------------------------------------------------------

//
// e:\test_dico.dic
//


//---------------------------------------------------------------------------


void usage() {
printf("%s",COPYRIGHT);
printf("Usage: Compress <dictionary> [-flip]\n");
printf("   <dictionary> : any unicode DELAF or DELACF dictionary\n");
printf("   -flip : this optional parameter specifies that the inflected and lemma\n");
printf("           forms must be swapped\n\n");
printf("Compresses a dictionary into an finite state automaton. This automaton\n");
printf("is stored is a .bin file, and the associated flexional codes are\n");
printf("written in a .inf file.\n\n");
}



void ecrire_nombre_lignes_INF(char name[],int n)
{
  FILE *f;
  int i;
  i=2+9*2; // *2 because of unicode +2 because of FF FE at file start
  f=fopen((char*)name,"r+b");
  do
    {
      fseek(f,i,0);
      i=i-2;
      u_fputc((unichar)((n%10)+'0'),f);
      n=n/10;
    }
  while (n);
  fclose(f);
}




int main(int argc, char **argv) {
setBufferMode();
FILE *f;
FILE *INF;
unichar s[DIC_WORD_SIZE];
dic_entry *e;
char bin[DIC_WORD_SIZE];
char inf[DIC_WORD_SIZE];
struct dictionary_node* racine;
struct string_hash* hash;
int i=0;

if (argc!=2 && argc!=3) {
   usage();
   return 0;
}
f=u_fopen(argv[1],U_READ);
if (f==NULL) {
   fprintf(stderr,"Cannot open %s\n",argv[1]);
   return 1;
}
int FLIP=0;
if (argc==3) {
   if (!strcmp(argv[2],"-flip")) {
        FLIP=1;
   } else {
      fprintf(stderr,"Invalid parameter: %s\n",argv[2]);
   }
}

name_without_extension(argv[1],bin);
strcat(bin,".bin");
name_without_extension(argv[1],inf);
strcat(inf,".inf");
INF=u_fopen(inf,U_WRITE);
if (INF==NULL) {
   fprintf(stderr,"Cannot create %s\n",inf);
   u_fclose(f);
   return 1;
}
u_fprints_char("0000000000\n",INF);
racine=new_arbre_dico();
hash=new_string_hash();
unichar tmp[DIC_WORD_SIZE];
printf("Compressing...\n");
while(read_DELA_line(f,s)) {
  if (s[0]=='\0') {
     fprintf(stderr,"Line %d: empty line\n",i);
  }
  else if (s[0]=='/') {
     // do nothing because the line is considered as a comment line
  }
  else {
     e=tokenize_DELA_line(s);
     if (FLIP) {
        unichar* o=e->inflected;
        e->inflected=e->lemma;
        e->lemma=o;
     }
     if (e!=NULL) {
        if (contains_unprotected_equal_sign(e->inflected)
            || contains_unprotected_equal_sign(e->lemma)) {
           // if the inflected form or lemma contains any unprotected = sign,
           // we must insert the space entry and the - entry:
           // pomme=de=terre,.N  ->  pomme de terre,pomme de terre.N
           //                        pomme-de-terre,pomme-de-terre.N
           unichar inf_tmp[DIC_WORD_SIZE];
           unichar lem_tmp[DIC_WORD_SIZE];
           u_strcpy(inf_tmp,e->inflected);
           u_strcpy(lem_tmp,e->lemma);
           replace_unprotected_equal_sign(e->inflected,(unichar)' ');
           replace_unprotected_equal_sign(e->lemma,(unichar)' ');
           // we insert pomme de terre,pomme de terre.N
           get_compressed_line(e,tmp);
           inserer_entree(e->inflected,tmp,racine,hash);
           // and pomme-de-terre,pomme-de-terre.N
           u_strcpy(e->inflected,inf_tmp);
           u_strcpy(e->lemma,lem_tmp);
           replace_unprotected_equal_sign(e->inflected,(unichar)'-');
           replace_unprotected_equal_sign(e->lemma,(unichar)'-');
           get_compressed_line(e,tmp);
           inserer_entree(e->inflected,tmp,racine,hash);
        }
        else {
           // normal case
           unprotect_equal_signs(e->inflected);
           unprotect_equal_signs(e->lemma);
           get_compressed_line(e,tmp);
           inserer_entree(e->inflected,tmp,racine,hash);
        }
        /* and last, but not least: don't forget to free your memory
           or it would be impossible to compress large dictionaries */
        free_dic_entry(e);
     }
  }
  if (i%10000==0) {
     printf("%d line%s read...       \r",i,(i>1)?"s":"");
  }
  i++;


}
u_fclose(f);
sauver_lignes_hash(INF,hash);
u_fclose(INF);
minimize_tree(racine);
creer_et_sauver_bin(racine,bin);
printf("%d line%s read            \n"
       "%d INF entr%s created\n",
       i,
       (i!=1)?"s":"",
       hash->N,
       (hash->N!=1)?"ies":"y");
printf("%d states, %d transitions\n",N_STATES,N_TRANSITIONS);
ecrire_nombre_lignes_INF(inf,hash->N);
// the following line had been removed because of a slowness problem with
// very large INF lines
//free_string_hash(hash);
return 0;
}

