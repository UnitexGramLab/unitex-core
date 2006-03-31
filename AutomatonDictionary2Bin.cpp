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

//---------------------------------------------------------------------------
#include "Arbre_to_bin.h"
#include "DictionaryTree.h"
//---------------------------------------------------------------------------



int N_STATES=0;
int N_TRANSITIONS=0;

static unsigned char* bin;
static int taille;


void numeroter_noeuds(struct dictionary_node* a) {
if (a==NULL) return;
if (a->offset!=-1) {
   return;
}
a->offset=taille;
a->n_trans=0;
taille=taille+2;  // 2 bytes for the number of transitions
if (a->single_INF_code_list!=NULL) {
   // if the node is a final one, we count 3 bytes for the adress of the INF line
   taille=taille+3;
}
struct dictionary_node_transition* tmp;
tmp=a->trans;
while (tmp!=NULL) {
  taille=taille+5; // for each transition, we count 2 bytes for the unichar and 3 bytes for the dest adress
  tmp=tmp->next;
  (a->n_trans)++;  // we also count the number of transitions
}
tmp=a->trans;
while (tmp!=NULL) {
  numeroter_noeuds(tmp->node);
  tmp=tmp->next;
}
}



void remplir_tableau_bin(struct dictionary_node* a) {
if (a==NULL) return;
if (a->INF_code==-1) {
   return;
}
N_STATES++;
unichar n=(unichar)a->n_trans;
if (a->single_INF_code_list==NULL) {
   // if the node is not a final one, we put to 1 the the heaviest bit
   n=(unichar)(n|32768);
}
int pos=a->offset;
// we write the 2 bytes info about the node
bin[pos]=(unsigned char)(n/256);
bin[pos+1]=(unsigned char)(n%256);
pos=pos+2;
if (a->single_INF_code_list!=NULL) {
   int adr=a->INF_code;
   bin[pos++]=(unsigned char)(adr/(256*256));
   adr=adr%(256*256);
   bin[pos++]=(unsigned char)(adr/256);
   adr=adr%256;
   bin[pos++]=(unsigned char)(adr);
}
a->INF_code=-1;
struct dictionary_node_transition* tmp;
tmp=a->trans;
while (tmp!=NULL) {
   N_TRANSITIONS++;
   bin[pos++]=(unsigned char)((tmp->letter)/256);
   bin[pos++]=(unsigned char)((tmp->letter)%256);
   int adr=tmp->node->offset;
   bin[pos++]=(unsigned char)(adr/(256*256));
   adr=adr%(256*256);
   bin[pos++]=(unsigned char)(adr/256);
   adr=adr%256;
   bin[pos++]=(unsigned char)(adr);
   remplir_tableau_bin(tmp->node);
   tmp=tmp->next;
}
}



void creer_et_sauver_bin(struct dictionary_node* a,char* nom) {
FILE *f;
f=fopen(nom,"wb");
if (f==NULL) {
  fprintf(stderr,"Cannot write automaton file %s\n",nom);
  exit(1);
}
taille=4; // we jump after the 4 bytes describing the automaton size
numeroter_noeuds(a);
bin=(unsigned char*)malloc(taille*sizeof(unsigned char));
if (bin==NULL) {
   fprintf(stderr,"Not enough memory to create the dictionary automaton\n");
   exit(1);
}
int n=taille;
printf("Binary file: %d bytes\n",n);
bin[0]=(unsigned char)(n/(256*256*256));
n=n%(256*256*256);
bin[1]=(unsigned char)(n/(256*256));
n=n%(256*256);
bin[2]=(unsigned char)(n/256);
n=n%256;
bin[3]=(unsigned char)(n);
remplir_tableau_bin(a);
if (fwrite(bin,1,taille,f)!=(unsigned)taille) {
  fprintf(stderr,"Error while writing file %s\n",nom);
}
fclose(f);
free(bin);
}
