 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "BinList.h"
#include "FileName.h"



// loads the .bin and .inf files of a dictionary
// the parameter bin is supposed to be the name of the .bin file
struct bin_dic* load_dictionary(char* bin) {
unsigned char* b=load_BIN_file(bin);
if (b==NULL) {
   fprintf(stderr,"Cannot load %s\n",bin);
   return NULL;
}
char temp[2000];
name_without_extension(bin,temp);
int priority;
switch(temp[strlen(temp)-1]) {
  case '-': priority=HIGH_PRIORITY_DICTIONARY; break;
  case '+': priority=LOW_PRIORITY_DICTIONARY; break;
  default : priority=NORMAL_PRIORITY_DICTIONARY; break;
}
strcat(temp,".inf");
struct INF_codes* inf=load_INF_file(temp);
if (inf==NULL) {
   fprintf(stderr,"Cannot load %s\n",inf);
   free(b);
   return NULL;
}
struct bin_dic* res=(struct bin_dic*)malloc(sizeof(struct bin_dic));
if (res==NULL) {
   return NULL;
}
res->bin=b;
res->inf=inf;
res->priority=priority;
return res;
}


void free_dictionary(struct bin_dic* dic) {
if (dic==NULL) {
   return;
}
if (dic->bin!=NULL) {
   free(dic->bin);
} else {
  fprintf(stderr,"Inconsistency 1 in free_dictionary\n");
}
if (dic->inf!=NULL) {
   free_INF_codes(dic->inf);
} else {
  fprintf(stderr,"Inconsistency 2 in free_dictionary\n");
}
free(dic);
}


//
// tab is an array that contains .bin file names
// these dictionaries are loaded in a (struct bin_list*) structure,
// ordered by descending priority
//
struct bin_list* load_dictionary_list(int n,char** tab) {
struct bin_list* res=(struct bin_list*)malloc(sizeof(struct bin_list));
res->n=n;
res->tab=(struct bin_dic**)malloc(n*sizeof(struct bin_dic*));
int z=0;
char temp[2000];

// we load first high priority dictionaries
for (int i=0;i<n;i++) {
   name_without_extension(tab[i],temp);
   if (temp[strlen(temp)-1]=='-') {
      // if the dictionary has the high priority mark, we load it
      printf("Loading %s\n",tab[i]);
      res->tab[z]=load_dictionary(tab[i]);
      if (res->tab[z]!=NULL) {
         // we do that in order to avoid NULL holes in the dictionary list
         z++;
      } else {
        (res->n)--;
      }
   }
}

// then, we load normal priority dictionaries
for (int i=0;i<n;i++) {
   name_without_extension(tab[i],temp);
   if (temp[strlen(temp)-1]!='-' && temp[strlen(temp)-1]!='+') {
      // if the dictionary has the high priority mark, we load it
      printf("Loading %s\n",tab[i]);
      res->tab[z]=load_dictionary(tab[i]);
      if (res->tab[z]!=NULL) {
         // we do that in order to avoid NULL holes in the dictionary list
         z++;
      } else {
        (res->n)--;
      }
   }
}

// finally, we load low priority dictionaries
for (int i=0;i<n;i++) {
   name_without_extension(tab[i],temp);
   if (temp[strlen(temp)-1]=='+') {
      // if the dictionary has the high priority mark, we load it
      printf("Loading %s\n",tab[i]);
      res->tab[z]=load_dictionary(tab[i]);
      if (res->tab[z]!=NULL) {
         // we do that in order to avoid NULL holes in the dictionary list
         z++;
      } else {
        (res->n)--;
      }
   }
}
printf("%d/%d dictionar%s successfully loaded\n",res->n,n,(res->n<=1)?"y":"ies");
if (res->n==0) {
   free_dictionary_list(res);
   return NULL;
}
return res;
}


void free_dictionary_list(struct bin_list* dic_list) {
if (dic_list==NULL) return;
for (int i=0;i<dic_list->n;i++) {
   free_dictionary(dic_list->tab[i]);
}
free(dic_list);
}

