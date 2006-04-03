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
#include "Sentence_to_grf.h"
//---------------------------------------------------------------------------


void sentence_to_grf(Automate_fst2* automate,int SENTENCE,char* font,FILE* f) {
if (SENTENCE>automate->nombre_graphes) {
   // if the index is out of bound
   fprintf(stderr,"Sentence number too long\n");
   return;
}
int nombre_etats=automate->nombre_etats_par_grf[SENTENCE];
int* rang=(int*)malloc(sizeof(int)*nombre_etats);
int* pos_X=(int*)malloc(sizeof(int)*nombre_etats);
int rang_max=calculer_rang(automate,SENTENCE,rang,nombre_etats);
int width_max=calculer_largeur_max_pour_chaque_rang(automate,SENTENCE,pos_X,nombre_etats,rang);
int nombre_etats_res=numeroter_etiquettes_sur_octets_forts(automate,SENTENCE,nombre_etats);
convertir_transitions_en_etats(automate,SENTENCE,nombre_etats,rang,f,nombre_etats_res,rang_max,width_max,pos_X,font);
free(rang);
free(pos_X);
}



int numeroter_etiquettes_sur_octets_forts(Automate_fst2* automate,int SENTENCE,int nombre_etats) {
int debut=automate->debut_graphe_fst2[SENTENCE];
int N=2;
for (int i=0;i<nombre_etats;i++) {
   struct transition_fst* trans=automate->etat[i+debut]->trans;
   while (trans!=NULL) {
      // we put the value in the 2 upper bytes
      trans->etiquette=(int)(trans->etiquette | (N<<NBRE_BITS_DE_DECALAGE));
      N++;
      trans=trans->suivant;
   }
}
return N;
}



void convertir_transitions_en_etats(Automate_fst2* automate,int SENTENCE,
                                    int nombre_etats,int* rang,FILE* f,int N,int rang_max,
                                    int width_max,int* pos_X,char* font) {
int N_GRF_STATES=2;
int MAX_STATES=10000;
int debut=automate->debut_graphe_fst2[SENTENCE];
struct transition_fst* trans;
struct grf_state* tab_grf_state[MAX_STATES];
for (int i=0;i<MAX_STATES;i++) {
   tab_grf_state[i]=NULL;
}
tab_grf_state[0]=new_grf_state("\"<E>\"",50,0);
// we process the initial state
int j;
trans=automate->etat[debut]->trans;
while (trans!=NULL) {
  j=get_numero_de_la_transition(trans->etiquette);
  add_transition_to_grf_state(tab_grf_state[0],j);
  trans=trans->suivant;
}
// and the final state
tab_grf_state[1]=new_grf_state("\"\"",(width_max+100),10000);

// then, we save all others states
for (int i=0;i<nombre_etats;i++) {
   trans=automate->etat[i+debut]->trans;
   while (trans!=NULL) {
      // we put the value in the 2 upper bytes
      // normal line:
      if (!u_strcmp_char(automate->etiquette[get_etiquette_reelle(trans->etiquette)]->input,"\"")) {
         //u_fprints_char("\\\"",f);
         tab_grf_state[N_GRF_STATES]=new_grf_state("\"\\\"\"",pos_X[rang[i]],rang[i]);
      }
      else {
         unichar temp[10000];
         u_strcpy_char(temp,"\"");
         u_strcat(temp,automate->etiquette[get_etiquette_reelle(trans->etiquette)]->input);
         u_strcat_char(temp,"\"");
         //u_fprints(automate->etiquette[get_etiquette_reelle(trans->etiquette)]->contenu,f);
         tab_grf_state[N_GRF_STATES]=new_grf_state(temp,pos_X[rang[i]],rang[i]);
      }
      j=0;
      struct transition_fst* TMP=automate->etat[trans->arr]->trans;
      while (TMP!=NULL) {
         j++;
         TMP=TMP->suivant;
      }
      if (j==0) {
         // if we arrive on the final state
         add_transition_to_grf_state(tab_grf_state[N_GRF_STATES],1);
      } else {
         TMP=automate->etat[trans->arr]->trans;
         while (TMP!=NULL) {
            j=get_numero_de_la_transition(TMP->etiquette);
            add_transition_to_grf_state(tab_grf_state[N_GRF_STATES],j);
            TMP=TMP->suivant;
         }
      }
      N_GRF_STATES++;
      trans=trans->suivant;
   }
}
// now, we look for duplicate states
remove_duplicates_grf_states(tab_grf_state,&N_GRF_STATES);
// and we save the grf
save_grf_states(f,tab_grf_state,N_GRF_STATES,rang_max,font);
}



void convertir_transitions_en_etats_old(Automate_fst2* automate,int SENTENCE,
                                    int nombre_etats,int* rang,FILE* f,int N,int rang_max,
                                    int width_max,int* pos_X,char* font) {
unichar z[100];
int position_verticale[2000];
int debut=automate->debut_graphe_fst2[SENTENCE];
struct transition_fst* trans;
// we counts the number of boxes for each horizontal position
for (int i=0;i<rang_max;i++) {
    position_verticale[i]=0;
}
for (int i=0;i<nombre_etats;i++) {
   trans=automate->etat[i+debut]->trans;
   while (trans!=NULL) {
      position_verticale[rang[i]]++;
      trans=trans->suivant;
   }
}
write_grf_header(width_max+300,800,N,font,f);
u_fprints_char("\"<E>\" 50 100 ",f);
trans=automate->etat[debut]->trans;
// we save the initial state
int j=0;
while (trans!=NULL) {
  j++;
  trans=trans->suivant;
}
u_int_to_string(j,z);
u_strcat_char(z," ");
u_fprints(z,f);
trans=automate->etat[debut]->trans;
while (trans!=NULL) {
  j=get_numero_de_la_transition(trans->etiquette);
  u_int_to_string(j,z);
  u_strcat_char(z," ");
  u_fprints(z,f);
  trans=trans->suivant;
}
// and the final state
u_fprints_char("\n\"\" ",f);
u_int_to_string((width_max+100),z);
u_strcat_char(z," 100 0 \n");
u_fprints(z,f);

// then, we save all others states
for (int i=0;i<nombre_etats;i++) {
   trans=automate->etat[i+debut]->trans;
   while (trans!=NULL) {
      // we put the value in the 2 upper bytes
      u_fprints_char("\"",f);
      // normal line:
      if (!u_strcmp_char(automate->etiquette[get_etiquette_reelle(trans->etiquette)]->input,"\"")) {
         u_fprints_char("\\\"",f);
      }
      else {
           u_fprints(automate->etiquette[get_etiquette_reelle(trans->etiquette)]->input,f);
      }
      /*if (automate->etiquette[get_etiquette_reelle(trans->etiquette)]->contenu[0]!='{') {
         u_fprints(automate->etiquette[get_etiquette_reelle(trans->etiquette)]->contenu,f);
      } else {
         u_fprints(automate->etiquette[get_etiquette_reelle(trans->etiquette)]->flechi,f);
         u_fprints_char("+",f);
         u_fprints(automate->etiquette[get_etiquette_reelle(trans->etiquette)]->canonique,f);
         u_fprints_char("/",f);
         u_fprints(automate->etiquette[get_etiquette_reelle(trans->etiquette)]->infos_gramm,f);
      }*/
      u_fprints_char("\" ",f);
      // we compute the x coordinate of the box
      u_int_to_string(pos_X[rang[i]],z);
      u_strcat_char(z," ");
      u_fprints(z,f);
      // and the y one
      j=100-(50*(rang[i]%2))+100*(--position_verticale[rang[i]]);
      //position_verticale[rang[i]];
      u_int_to_string(j,z);
      u_strcat_char(z," ");
      u_fprints(z,f);
      j=0;
      struct transition_fst* TMP=automate->etat[trans->arr]->trans;
      while (TMP!=NULL) {
         j++;
         TMP=TMP->suivant;
      }
      if (j==0) {
         // if we arrive on the final state
         u_fprints_char("1 1 \n",f);
      } else {
         u_int_to_string(j,z);
         u_strcat_char(z," ");
         u_fprints(z,f);
         TMP=automate->etat[trans->arr]->trans;
         while (TMP!=NULL) {
            j=get_numero_de_la_transition(TMP->etiquette);
            u_int_to_string(j,z);
            u_strcat_char(z," ");
            u_fprints(z,f);
            TMP=TMP->suivant;
         }
         u_fprints_char("\n",f);
      }
      trans=trans->suivant;
   }
}
}



int get_etiquette_reelle(int etiquette) {
return (etiquette & ((1<<NBRE_BITS_DE_DECALAGE) -1));
}



int get_numero_de_la_transition(int etiquette) {
return (etiquette >> NBRE_BITS_DE_DECALAGE);
}



int calculer_rang(Automate_fst2* automate,int SENTENCE,int* rang,int nombre_etats) {
char modif[3000];
int RANG=0;
for (int i=0;i<nombre_etats;i++) {
   rang[i]=0;
   modif[i]=0;
}
explorer_rang_etat(automate->debut_graphe_fst2[SENTENCE],automate->debut_graphe_fst2[SENTENCE],
                   automate,rang,modif,&RANG);
return RANG;
}



void explorer_rang_etat(int etat_courant,int debut,Automate_fst2* automate,
                        int* rang,char* modif,int* RANG) {
struct transition_fst* trans=automate->etat[etat_courant]->trans;
int RANG_COURANT=rang[etat_courant-debut];
while (trans!=NULL) {
   if (RANG_COURANT+1>rang[trans->arr-debut]) {
      // if we must increase the path length
      rang[trans->arr-debut]=RANG_COURANT+1;
      if ((RANG_COURANT+1)> (*RANG)) {
         (*RANG)=RANG_COURANT+1;
      }
      modif[(trans->arr)-debut]=1;
   }
   trans=trans->suivant;
}
// then, we process all the nodes we have modified
trans=automate->etat[etat_courant]->trans;
while (trans!=NULL) {
   if (modif[trans->arr-debut]==1) {
      // if we must increase the path length
      explorer_rang_etat(trans->arr,debut,automate,rang,modif,RANG);
      modif[(trans->arr)-debut]=0;
   }
   trans=trans->suivant;
}
}



void explorer_position_horizontale_etat(int etat_courant,int debut,Automate_fst2* automate,
                                        int* pos_X,char* modif,int* WIDTH) {
struct transition_fst* trans=automate->etat[etat_courant]->trans;
int POS_COURANTE=pos_X[etat_courant-debut];
while (trans!=NULL) {
   int VAL=POS_COURANTE+(WIDTH_OF_A_CHAR*(width_of_tag(automate->etiquette[trans->etiquette])));
   if (VAL>pos_X[(trans->arr)-debut]) {
      // if we must increase the path length
      pos_X[(trans->arr)-debut]=VAL;
      if (VAL > (*WIDTH)) {
         (*WIDTH)=VAL;
      }
      modif[(trans->arr)-debut]=1;
   }
   trans=trans->suivant;
}
// then, we process all the nodes we have modified
trans=automate->etat[etat_courant]->trans;
while (trans!=NULL) {
   if (modif[(trans->arr)-debut]==1) {
      // if we must increase the path length
      explorer_position_horizontale_etat(trans->arr,debut,automate,pos_X,modif,WIDTH);
      modif[(trans->arr)-debut]=0;
   }
   trans=trans->suivant;
}
}



void write_grf_header(int width,int height,int N,char* font,FILE* f) {
unichar z[10];
u_fprints_char("#Unigraph\n",f);
u_fprints_char("SIZE ",f);
u_int_to_string(width,z);
u_strcat_char(z," ");
u_fprints(z,f);
u_int_to_string(height,z);
u_strcat_char(z,"\n");
u_fprints(z,f);
if (font==NULL) {
   u_fprints_char("FONT Times New Roman:  10\n",f);
   u_fprints_char("OFONT Times New Roman:B 10\n",f);
}
else {
   u_fprints_char("FONT ",f);
   u_fprints_char(font,f);
   u_fprints_char(":  10\n",f);
   u_fprints_char("OFONT ",f);
   u_fprints_char(font,f);
   u_fprints_char(":B 10\n",f);
}

u_fprints_char("BCOLOR 16777215\n",f);
u_fprints_char("FCOLOR 0\n",f);
u_fprints_char("ACOLOR 12632256\n",f);
u_fprints_char("SCOLOR 16711680\n",f);
u_fprints_char("CCOLOR 255\n",f);
u_fprints_char("DBOXES y\n",f);
u_fprints_char("DFRAME y\n",f);
u_fprints_char("DDATE y\n",f);
u_fprints_char("DFILE y\n",f);
u_fprints_char("DDIR y\n",f);
u_fprints_char("DRIG n\n",f);
u_fprints_char("DRST n\n",f);
u_fprints_char("FITS 100\n",f);
u_fprints_char("PORIENT L\n",f);
u_fprints_char("#\n",f);
u_int_to_string(N,z);
u_strcat_char(z,"\n");
u_fprints(z,f);
}



//
// this function computes the width of the box for the tag s in the sentence GRF
// if s is a not a tag, we return its length
// else, the width is the max of length(inflected), length(lemma), length(code)
//
int width_of_tag(Fst2Tag e) {
if (e->input[0]!='{') {
   return u_strlen(e->input);
}
int i=u_strlen(e->inflected);
int j;
if (i<(j=u_strlen(e->lemma))) i=j;
if (i<(j=u_strlen(e->codes))) i=j;
return i;
}



int calculer_largeur_max_pour_chaque_rang(Automate_fst2* automate,int SENTENCE,
                                          int* pos_X,int nombre_etats,int* rang) {
int i;
struct transition_fst* trans;
for (i=0;i<nombre_etats;i++) {
   pos_X[i]=0;
}
int debut=automate->debut_graphe_fst2[SENTENCE];
// first, we compute the maximum length for this column
for (i=0;i<nombre_etats;i++) {
   trans=automate->etat[i+debut]->trans;
   while (trans!=NULL) {
      int VAL=(WIDTH_OF_A_CHAR*(5+width_of_tag(automate->etiquette[trans->etiquette])));
      if (pos_X[rang[i]+1]<VAL) {
         pos_X[rang[i]+1]=VAL;
      }
      trans=trans->suivant;
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
u_strcpy_char(temp,content);
return new_grf_state(temp,pos_X,rang);
}



void free_grf_state(struct grf_state* g) {
if (g==NULL) return;
if (g->content!=NULL) free(g->content);
free_liste_nombres(g->l);
free(g);
}



void add_transition_to_grf_state(struct grf_state* g,int arr) {
g->l=inserer_dans_liste_nombres(arr,g->l);
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
   struct liste_nombres* l=tab_grf_state[i]->l;
   while (l!=NULL) {
      if (l->n==a_virer) {
         l->n=a_garder;
      }
      else if (l->n==(*N)-1) {
         l->n=a_virer;
      }
      l=l->suivant;
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
return are_equivalent_liste_nombres(a->l,b->l);
}



void save_grf_states(FILE* f,struct grf_state** tab_grf_state,int N_GRF_STATES,
                     int rang_max,char* font) {
unichar z[100];
int position_verticale[2000];
struct liste_nombres* l;
// we counts the number of boxes for each horizontal position
for (int i=0;i<rang_max;i++) {
    position_verticale[i]=0;
}
for (int i=0;i<N_GRF_STATES;i++) {
   position_verticale[tab_grf_state[i]->rang]++;
}
write_grf_header(tab_grf_state[1]->pos_X+300,800,N_GRF_STATES,font,f);
u_fprints_char("\"<E>\" 50 100 ",f);
// we save the initial state
int j=0;
l=tab_grf_state[0]->l;
while (l!=NULL) {
  j++;
  l=l->suivant;
}
u_int_to_string(j,z);
u_strcat_char(z," ");
u_fprints(z,f);
l=tab_grf_state[0]->l;
while (l!=NULL) {
  u_int_to_string(l->n,z);
  u_strcat_char(z," ");
  u_fprints(z,f);
  l=l->suivant;
}
// and the final state
u_fprints_char("\n\"\" ",f);
u_int_to_string(tab_grf_state[1]->pos_X,z);
u_strcat_char(z," 100 0 \n");
u_fprints(z,f);

// then, we save all others states
for (int i=2;i<N_GRF_STATES;i++) {
   u_fprints(tab_grf_state[i]->content,f);
   u_fprints_char(" ",f);
   // we compute the x coordinate of the box
   u_int_to_string(tab_grf_state[i]->pos_X,z);
   u_strcat_char(z," ");
   u_fprints(z,f);
   // and the y one
   j=100-(50*(tab_grf_state[i]->rang%2))+100*(--position_verticale[tab_grf_state[i]->rang]);
   //position_verticale[rang[i]];
   u_int_to_string(j,z);
   u_strcat_char(z," ");
   u_fprints(z,f);
   j=0;
   l=tab_grf_state[i]->l;
   while (l!=NULL) {
      j++;
      l=l->suivant;
   }
   u_int_to_string(j,z);
   u_strcat_char(z," ");
   u_fprints(z,f);
   l=tab_grf_state[i]->l;
   while (l!=NULL) {
      u_int_to_string(l->n,z);
      u_strcat_char(z," ");
      u_fprints(z,f);
      l=l->suivant;
   }
   u_fprints_char("\n",f);
}
}
