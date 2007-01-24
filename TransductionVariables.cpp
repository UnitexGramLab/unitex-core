 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "TransductionVariables.h"
#include "Error.h"
//---------------------------------------------------------------------------


struct transduction_variable* tab_transduction_variable[N_MAX_TRANSDUCTION_VARIABLES];
struct string_hash* transduction_variable_index=NULL;


struct transduction_variable* new_transduction_variable() {
struct transduction_variable* v;
v=(struct transduction_variable*)malloc(sizeof(struct transduction_variable));
v->start=-1;
v->end=-1;
return v;
}



void free_transduction_variable(struct transduction_variable* v) {
free(v);
}



int get_transduction_variable_indice(unichar* s) {
return get_value_index(s,transduction_variable_index,DONT_INSERT);
}



struct transduction_variable* get_transduction_variable(unichar* s) {
int n=get_transduction_variable_indice(s);
if (n==-1) {
   return NULL;
}
return tab_transduction_variable[n];
}



void init_transduction_variable_index(struct variable_list* l) {
transduction_variable_index=new_string_hash();
int i=0;
while (l!=NULL) {
   get_value_index(l->name,transduction_variable_index);
   tab_transduction_variable[i++]=new_transduction_variable();
   l=l->next;
}
}


void free_transduction_variable_index() {
for (int i=0;i<transduction_variable_index->size;i++) {
  free_transduction_variable(tab_transduction_variable[i]);
}
free_string_hash(transduction_variable_index);
}



void set_variable_start(int n,int pos) {
tab_transduction_variable[n]->start=pos;
}



void set_variable_end(int n,int pos) {
tab_transduction_variable[n]->end=pos;
}



int get_variable_start(int n) {
return tab_transduction_variable[n]->start;
}



int get_variable_end(int n) {
return tab_transduction_variable[n]->end;
}


int* create_variable_backup() {
if (transduction_variable_index==NULL) return NULL;
int* tab=(int*)malloc(sizeof(int)*2*(transduction_variable_index->size));
int j=0;
for (int i=0;i<transduction_variable_index->size;i++) {
   tab[j++]=tab_transduction_variable[i]->start;
   tab[j++]=tab_transduction_variable[i]->end;
}
return tab;
}


void free_variable_backup(int* tab) {
if (tab!=NULL) free(tab);
}


void install_variable_backup(int* tab) {
if (tab==NULL) {
	fatal_error("NULL error in install_variable_backup\n");
}
int j=0;
for (int i=0;i<transduction_variable_index->size;i++) {
   tab_transduction_variable[i]->start=tab[j++];
   tab_transduction_variable[i]->end=tab[j++];
}
}


