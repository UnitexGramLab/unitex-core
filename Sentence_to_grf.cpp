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

#include "Sentence_to_grf.h"
#include "StringParsing.h"
#include "Error.h"


void sentence_to_grf(Fst2* automate,int SENTENCE,char* font,FILE* f) {
if (SENTENCE>automate->number_of_graphs) {
   // if the index is out of bound
   error("Sentence number too long\n");
   return;
}
int nombre_etats=automate->number_of_states_per_graphs[SENTENCE];
int* rang=(int*)malloc(sizeof(int)*nombre_etats);
int* pos_X=(int*)malloc(sizeof(int)*nombre_etats);
int rang_max=calculer_rang(automate,SENTENCE,rang,nombre_etats);
int width_max=calculer_largeur_max_pour_chaque_rang(automate,SENTENCE,pos_X,nombre_etats,rang);
int nombre_etats_res=numeroter_etiquettes_sur_octets_forts(automate,SENTENCE,nombre_etats);
convertir_transitions_en_etats(automate,SENTENCE,nombre_etats,rang,f,nombre_etats_res,rang_max,width_max,pos_X,font);
free(rang);
free(pos_X);
}



int numeroter_etiquettes_sur_octets_forts(Fst2* automate,int SENTENCE,int nombre_etats) {
int debut=automate->initial_states[SENTENCE];
int N=2;
for (int i=0;i<nombre_etats;i++) {
   struct fst2Transition* trans=automate->states[i+debut]->transitions;
   while (trans!=NULL) {
      // we put the value in the 2 upper bytes
      trans->tag_number=(int)(trans->tag_number | (N<<NBRE_BITS_DE_DECALAGE));
      N++;
      trans=trans->next;
   }
}
return N;
}



void convertir_transitions_en_etats(Fst2* automate,int SENTENCE,
                                    int nombre_etats,int* rang,FILE* f,int N,int rang_max,
                                    int width_max,int* pos_X,char* font) {
int N_GRF_STATES=2;
int MAX_STATES=10000;
int debut=automate->initial_states[SENTENCE];
struct fst2Transition* trans;
struct grf_state* tab_grf_state[MAX_STATES];
for (int i=0;i<MAX_STATES;i++) {
   tab_grf_state[i]=NULL;
}
tab_grf_state[0]=new_grf_state("\"<E>\"",50,0);
// we process the initial state
int j;
trans=automate->states[debut]->transitions;
while (trans!=NULL) {
  j=get_numero_de_la_transition(trans->tag_number);
  add_transition_to_grf_state(tab_grf_state[0],j);
  trans=trans->next;
}
// and the final state
tab_grf_state[1]=new_grf_state("\"\"",(width_max+100),10000);

// then, we save all others states
for (int i=0;i<nombre_etats;i++) {
   trans=automate->states[i+debut]->transitions;
   while (trans!=NULL) {
      // we put the value in the 2 upper bytes
      // normal line:
      if (!u_strcmp(automate->tags[get_etiquette_reelle(trans->tag_number)]->input,"\"")) {
         //u_fprints_char("\\\"",f);
         tab_grf_state[N_GRF_STATES]=new_grf_state("\"\\\"\"",pos_X[rang[i]],rang[i]);
      }
      else {
         unichar temp[10000];
         u_strcpy(temp,"\"");
         escape(automate->tags[get_etiquette_reelle(trans->tag_number)]->input,&temp[1],P_DOUBLE_QUOTE);
         u_strcat(temp,"\"");
         tab_grf_state[N_GRF_STATES]=new_grf_state(temp,pos_X[rang[i]],rang[i]);
      }
      j=0;
      struct fst2Transition* TMP=automate->states[trans->state_number]->transitions;
      while (TMP!=NULL) {
         j++;
         TMP=TMP->next;
      }
      if (j==0) {
         // if we arrive on the final state
         add_transition_to_grf_state(tab_grf_state[N_GRF_STATES],1);
      } else {
         TMP=automate->states[trans->state_number]->transitions;
         while (TMP!=NULL) {
            j=get_numero_de_la_transition(TMP->tag_number);
            add_transition_to_grf_state(tab_grf_state[N_GRF_STATES],j);
            TMP=TMP->next;
         }
      }
      N_GRF_STATES++;
      trans=trans->next;
   }
}
// now, we look for duplicate states
remove_duplicates_grf_states(tab_grf_state,&N_GRF_STATES);
// and we save the grf
save_grf_states(f,tab_grf_state,N_GRF_STATES,rang_max,font);
}



void convertir_transitions_en_etats_old(Fst2* automate,int SENTENCE,
                                    int nombre_etats,int* rang,FILE* f,int N,int rang_max,
                                    int width_max,int* pos_X,char* font) {
int position_verticale[2000];
int debut=automate->initial_states[SENTENCE];
struct fst2Transition* trans;
// we counts the number of boxes for each horizontal position
for (int i=0;i<rang_max;i++) {
    position_verticale[i]=0;
}
for (int i=0;i<nombre_etats;i++) {
   trans=automate->states[i+debut]->transitions;
   while (trans!=NULL) {
      position_verticale[rang[i]]++;
      trans=trans->next;
   }
}
write_grf_header(width_max+300,800,N,font,f);
u_fprintf(f,"\"<E>\" 50 100 ");
trans=automate->states[debut]->transitions;
// we save the initial state
int j=0;
while (trans!=NULL) {
   j++;
   trans=trans->next;
}
u_fprintf(f,"%d ",j);
trans=automate->states[debut]->transitions;
while (trans!=NULL) {
   j=get_numero_de_la_transition(trans->tag_number);
   u_fprintf(f,"%d ",j);
   trans=trans->next;
}
// and the final state
u_fprintf(f,"\n\"\" %d 100 0 \n",(width_max+100));

// then, we save all others states
for (int i=0;i<nombre_etats;i++) {
   trans=automate->states[i+debut]->transitions;
   while (trans!=NULL) {
      // we put the value in the 2 upper bytes
      u_fprintf(f,"\"");
      // normal line:
      if (!u_strcmp(automate->tags[get_etiquette_reelle(trans->tag_number)]->input,"\"")) {
         u_fprintf(f,"\\\"");
      }
      else {
           u_fprintf(f,"%S",automate->tags[get_etiquette_reelle(trans->tag_number)]->input);
      }
      u_fprintf(f,"\" ");
      // we compute the x coordinate of the box
      u_fprintf(f,"%d ",pos_X[rang[i]]);
      // and the y one
      j=100-(50*(rang[i]%2))+100*(--position_verticale[rang[i]]);
      u_fprintf(f,"%d ",j);
      j=0;
      struct fst2Transition* TMP=automate->states[trans->state_number]->transitions;
      while (TMP!=NULL) {
         j++;
         TMP=TMP->next;
      }
      if (j==0) {
         // if we arrive on the final state
         u_fprintf(f,"1 1 \n");
      } else {
         u_fprintf(f,"%d ",j);
         TMP=automate->states[trans->state_number]->transitions;
         while (TMP!=NULL) {
            j=get_numero_de_la_transition(TMP->tag_number);
            u_fprintf(f,"%d ",j);
            TMP=TMP->next;
         }
         u_fprintf(f,"\n");
      }
      trans=trans->next;
   }
}
}



int get_etiquette_reelle(int etiquette) {
return (etiquette & ((1<<NBRE_BITS_DE_DECALAGE) -1));
}



int get_numero_de_la_transition(int etiquette) {
return (etiquette >> NBRE_BITS_DE_DECALAGE);
}



int calculer_rang(Fst2* automate,int SENTENCE,int* rang,int nombre_etats) {
char modif[3000];
int RANG=0;
for (int i=0;i<nombre_etats;i++) {
   rang[i]=0;
   modif[i]=0;
}
explorer_rang_etat(automate->initial_states[SENTENCE],automate->initial_states[SENTENCE],
                   automate,rang,modif,&RANG);
return RANG;
}



void explorer_rang_etat(int etat_courant,int debut,Fst2* automate,
                        int* rang,char* modif,int* RANG) {
struct fst2Transition* trans=automate->states[etat_courant]->transitions;
int RANG_COURANT=rang[etat_courant-debut];
while (trans!=NULL) {
   if (RANG_COURANT+1>rang[trans->state_number-debut]) {
      // if we must increase the path length
      rang[trans->state_number-debut]=RANG_COURANT+1;
      if ((RANG_COURANT+1)> (*RANG)) {
         (*RANG)=RANG_COURANT+1;
      }
      modif[(trans->state_number)-debut]=1;
   }
   trans=trans->next;
}
// then, we process all the nodes we have modified
trans=automate->states[etat_courant]->transitions;
while (trans!=NULL) {
   if (modif[trans->state_number-debut]==1) {
      // if we must increase the path length
      explorer_rang_etat(trans->state_number,debut,automate,rang,modif,RANG);
      modif[(trans->state_number)-debut]=0;
   }
   trans=trans->next;
}
}



void explorer_position_horizontale_etat(int etat_courant,int debut,Fst2* automate,
                                        int* pos_X,char* modif,int* WIDTH) {
struct fst2Transition* trans=automate->states[etat_courant]->transitions;
int POS_COURANTE=pos_X[etat_courant-debut];
while (trans!=NULL) {
   int VAL=POS_COURANTE+(WIDTH_OF_A_CHAR*(width_of_tag(automate->tags[trans->tag_number])));
   if (VAL>pos_X[(trans->state_number)-debut]) {
      // if we must increase the path length
      pos_X[(trans->state_number)-debut]=VAL;
      if (VAL > (*WIDTH)) {
         (*WIDTH)=VAL;
      }
      modif[(trans->state_number)-debut]=1;
   }
   trans=trans->next;
}
// then, we process all the nodes we have modified
trans=automate->states[etat_courant]->transitions;
while (trans!=NULL) {
   if (modif[(trans->state_number)-debut]==1) {
      // if we must increase the path length
      explorer_position_horizontale_etat(trans->state_number,debut,automate,pos_X,modif,WIDTH);
      modif[(trans->state_number)-debut]=0;
   }
   trans=trans->next;
}
}



void write_grf_header(int width,int height,int N,char* font,FILE* f) {
u_fprintf(f,"#Unigraph\n");
u_fprintf(f,"SIZE %d %d\n",width,height);
if (font==NULL) {
   u_fprintf(f,"FONT Times New Roman:  10\n");
   u_fprintf(f,"OFONT Times New Roman:B 10\n");
}
else {
   u_fprintf(f,"FONT %s:  10\n",font);
   u_fprintf(f,"OFONT %s:B 10\n",font);
}
u_fprintf(f,"BCOLOR 16777215\n");
u_fprintf(f,"FCOLOR 0\n");
u_fprintf(f,"ACOLOR 12632256\n");
u_fprintf(f,"SCOLOR 16711680\n");
u_fprintf(f,"CCOLOR 255\n");
u_fprintf(f,"DBOXES y\n");
u_fprintf(f,"DFRAME y\n");
u_fprintf(f,"DDATE y\n");
u_fprintf(f,"DFILE y\n");
u_fprintf(f,"DDIR y\n");
u_fprintf(f,"DRIG n\n");
u_fprintf(f,"DRST n\n");
u_fprintf(f,"FITS 100\n");
u_fprintf(f,"PORIENT L\n");
u_fprintf(f,"#\n");
u_fprintf(f,"%d\n",N);
}



//
// this function computes the width of the box for the tag s in the sentence GRF
// if s is a not a tag, we return its length
// else, the width is the max of length(inflected), length(lemma), length(code)
//
int width_of_tag(Fst2Tag e) {
return u_strlen(e->input);
/*
 * To modify since fst2 tags do not contain anymore such info. We must use now
 * e->pattern instead
if (e->input[0]!='{') {
   return u_strlen(e->input);
}
int i=u_strlen(e->inflected);
int j;
if (i<(j=u_strlen(e->lemma))) i=j;
if (i<(j=u_strlen(e->codes))) i=j;
return i;*/
}



int calculer_largeur_max_pour_chaque_rang(Fst2* automate,int SENTENCE,
                                          int* pos_X,int nombre_etats,int* rang) {
int i;
struct fst2Transition* trans;
for (i=0;i<nombre_etats;i++) {
   pos_X[i]=0;
}
int debut=automate->initial_states[SENTENCE];
// first, we compute the maximum length for this column
for (i=0;i<nombre_etats;i++) {
   trans=automate->states[i+debut]->transitions;
   while (trans!=NULL) {
      int VAL=(WIDTH_OF_A_CHAR*(5+width_of_tag(automate->tags[trans->tag_number])));
      if (pos_X[rang[i]+1]<VAL) {
         pos_X[rang[i]+1]=VAL;
      }
      trans=trans->next;
   }
}
// then, we compute the position
int tmp=100;
for (i=0;i<nombre_etats;i++) {
   pos_X[i]=tmp+pos_X[i];
   tmp=pos_X[i];
}
return pos_X[nombre_etats-1];
}




struct grf_state* new_grf_state(unichar* content,int pos_X,int rang) {
struct grf_state* g=(struct grf_state*)malloc(sizeof(struct grf_state));
g->content=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(content)));
u_strcpy(g->content,content);
g->pos_X=pos_X;
g->rang=rang;
g->l=NULL;
return g;
}



struct grf_state* new_grf_state(char* content,int pos_X,int rang) {
unichar temp[10000];
u_strcpy(temp,content);
return new_grf_state(temp,pos_X,rang);
}



void free_grf_state(struct grf_state* g) {
if (g==NULL) return;
if (g->content!=NULL) free(g->content);
free_list_int(g->l);
free(g);
}



void add_transition_to_grf_state(struct grf_state* g,int arr) {
g->l=sorted_insert(arr,g->l);
}



void remove_grf_state(struct grf_state** tab_grf_state,int a_garder,int a_virer,int *N) {
// we keep the right most position
if (tab_grf_state[a_garder]->pos_X < tab_grf_state[a_virer]->pos_X) {
   tab_grf_state[a_garder]->pos_X=tab_grf_state[a_virer]->pos_X;
}
free_grf_state(tab_grf_state[a_virer]);
tab_grf_state[a_virer]=tab_grf_state[(*N)-1];
tab_grf_state[(*N)-1]=NULL;
for (int i=0;i<(*N)-1;i++) {
   struct list_int* l=tab_grf_state[i]->l;
   while (l!=NULL) {
      if (l->n==a_virer) {
         l->n=a_garder;
      }
      else if (l->n==(*N)-1) {
         l->n=a_virer;
      }
      l=l->next;
   }
}
(*N)--;
}


//
// this function takes an array of grf_state and remove duplicate states
// returns 1 if modification occurred, 0 else
//
int actually_remove_duplicate_grf_states(struct grf_state** tab_grf_state,int *N) {
for (int i=0;i<(*N);i++) {
   for (int j=i+1;j<(*N);j++) {
      if (are_equivalent_grf_states(tab_grf_state[i],tab_grf_state[j])) {
         remove_grf_state(tab_grf_state,i,j,N);
         return 1;
      }
   }
}
return 0;
}


//
// this function takes an array of grf_state and remove duplicate states
//
void remove_duplicates_grf_states(struct grf_state** tab_grf_state,int *N) {
while (actually_remove_duplicate_grf_states(tab_grf_state,N));
}



//
// returns 1 if the grf_state are equivalent, 0 else
//
int are_equivalent_grf_states(struct grf_state* a,struct grf_state* b) {
if (a==NULL) {
   if (b==NULL) return 1;
   else return 0;
}
if (a->content==NULL) {
   if (b->content==NULL) return 1;
   else return 0;
}
if (u_strcmp(a->content,b->content)) return 0;
return equal_list_int(a->l,b->l);
}



void save_grf_states(FILE* f,struct grf_state** tab_grf_state,int N_GRF_STATES,
                     int rang_max,char* font) {
int position_verticale[2000];
struct list_int* l;
// we counts the number of boxes for each horizontal position
for (int i=0;i<rang_max;i++) {
    position_verticale[i]=0;
}
for (int i=0;i<N_GRF_STATES;i++) {
   position_verticale[tab_grf_state[i]->rang]++;
}
write_grf_header(tab_grf_state[1]->pos_X+300,800,N_GRF_STATES,font,f);
u_fprintf(f,"\"<E>\" 50 100 ");
// we save the initial state
int j=0;
l=tab_grf_state[0]->l;
while (l!=NULL) {
   j++;
   l=l->next;
}
u_fprintf(f,"%d ",j);
l=tab_grf_state[0]->l;
while (l!=NULL) {
   u_fprintf(f,"%d ",l->n);
   l=l->next;
}
// and the final state
u_fprintf(f,"\n\"\" ");
u_fprintf(f,"%d 100 0 \n",tab_grf_state[1]->pos_X);

// then, we save all others states
for (int i=2;i<N_GRF_STATES;i++) {
   u_fprintf(f,"%S ",tab_grf_state[i]->content);
   // we compute the x coordinate of the box
   u_fprintf(f,"%d ",tab_grf_state[i]->pos_X);
   // and the y one
   j=100-(50*(tab_grf_state[i]->rang%2))+100*(--position_verticale[tab_grf_state[i]->rang]);
   u_fprintf(f,"%d ",j);
   j=0;
   l=tab_grf_state[i]->l;
   while (l!=NULL) {
      j++;
      l=l->next;
   }
   u_fprintf(f,"%d ",j);
   l=tab_grf_state[i]->l;
   while (l!=NULL) {
      u_fprintf(f,"%d ",l->n);
      l=l->next;
   }
   u_fprintf(f,"\n");
}
}
