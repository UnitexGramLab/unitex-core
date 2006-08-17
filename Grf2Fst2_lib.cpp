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
#include "Grf2Fst2_lib.h"
//---------------------------------------------------------------------------


struct donnees_comp *donnees;
unichar pckg_path[TAILLE_MOT_GRAND_COMP];
int nombre_graphes_comp;
int nombre_etiquettes_comp; /* attention: may be confused with macro
                               NOMBRE_ETIQUETTES_COMP */
struct noeud_g_comp *rac_graphe_comp; //racine de l'arbre des graphes
struct noeud_comp *rac_comp; //racine de l'arbre des étiquettes
FILE *fs_comp; //fichier de sortie
int EPSILON_comp;   // etiquette pour <E>
int n_malloc_comp=0;
int n_free_comp=0;
int compteur_char=0;
int compteur_free_char=0;
ensemble_det stock[NBRE_ETIQ_TRANSITION_COMP];
unsigned char final[NBRE_ETIQ_TRANSITION_COMP];
int hachage[NBRE_ETIQ_TRANSITION_COMP];
int hachageinv[NBRE_ETIQ_TRANSITION_COMP];
Etat_fst_det resultat[NBRE_ET];




/* structure of linked list added for representing sets for epsilon closure*/

// STRUCTURE

struct ll_i_cell_comp{
  unsigned int value;
  struct ll_i_cell_comp *next;
};

struct linked_list_i_comp{
  struct ll_i_cell_comp *head;
  struct ll_i_cell_comp *tail;
};

typedef struct linked_list_i_comp * Llist_i_comp;

// FONCTIONS

int ll_i_n_free = 0;
int ll_i_n_malloc = 0;

void util_error(char *function,char *message){
  fprintf(stderr,"%s: %s\n",function,message);
  exit(1);
}

void* ll_i_malloc(int n){
  ll_i_n_malloc++;
  return (void *)malloc(n);
}

void ll_i_pfree(void *ptr){
  if(ptr == NULL) return;
  ll_i_n_free++;
  free(ptr);
  ptr = NULL;
}

struct ll_i_cell_comp* ll_i_create_cell(unsigned int v){
  struct ll_i_cell_comp *ptr = (struct ll_i_cell_comp*)ll_i_malloc(sizeof(struct ll_i_cell_comp));
  ptr->value = v;
  ptr->next = NULL;
  return ptr;
}

Llist_i_comp ll_i_initialize(){
  Llist_i_comp ptr = (Llist_i_comp)ll_i_malloc(sizeof(struct linked_list_i_comp));
  ptr->head = NULL;
  ptr->tail = NULL;
  return ptr;
}

void ll_i_insert_at_tail(Llist_i_comp *l,unsigned int v){
  struct ll_i_cell_comp *ptr = ll_i_create_cell(v);

  if(ptr == NULL) util_error("ll_i_insert_at_tail","creating cell");
  if(*l == NULL){
    *l = ll_i_initialize();
  }
  if(*l == NULL)  util_error("ll_i_insert_at_tail","llist initialization");
  if((*l)->head == NULL){
    (*l)->head = ptr;
    (*l)->tail = (*l)->head;
    return;
  }
  (*l)->tail->next = ptr;
  (*l)->tail = (*l)->tail->next;
}


int ll_i_cell_i_compare(struct ll_i_cell_comp *c1,struct ll_i_cell_comp *c2){
  
  return (c1->value < c2->value);
}

int ll_i_cell_i_equals(struct ll_i_cell_comp *c1,struct ll_i_cell_comp *c2){  
  return (c1->value == c2->value);
}

// No tail


// insert dans l'ordre et pas de doublons

void ll_i_insert_sorted(Llist_i_comp *l,unsigned int v){
  struct ll_i_cell_comp* prev, *curr,*temp;
  int is_doublon = 0;

  temp = ll_i_create_cell(v);
  if(temp == NULL) util_error("ll_i_insert_sorted","creating cell");
  if(*l == NULL){
    *l = ll_i_initialize();
  }
  if(*l == NULL) util_error("ll_i_insert_sorted","llist initialization");

  if ( (*l)->head == NULL ){
    (*l)->head = temp;  
  }
  else{
    prev = NULL;
    curr = (*l)->head;
    
    /* traverse the list until the spot
       for insertion is found */
    is_doublon = 0;
    while (curr != NULL && ll_i_cell_i_compare(curr,temp)){
      prev = curr;     
      curr = curr -> next;
    }    
    if(curr != NULL && ll_i_cell_i_equals(curr,temp)) is_doublon = 1;
    /* insert the node, temp */
    if(!is_doublon){
      if(prev == NULL){
	temp -> next = (*l)->head;
	(*l)->head = temp;
      }
      else{
	prev -> next = temp;
	temp -> next = curr;
      } 
    }
    else{
      ll_i_pfree(temp);
    }
  }
}

//find an element in list

struct ll_i_cell_comp* ll_i_find_element(Llist_i_comp l,unsigned int elem){
  struct ll_i_cell_comp *ptr;
  if(l == NULL) return NULL;
  ptr = l->head;
  while(ptr != NULL){
    if(ptr->value == elem) return ptr;
     ptr = ptr->next;
  }
  return NULL;
}

void ll_i_print(Llist_i_comp l){
  struct ll_i_cell_comp *ptr;

  if(l == NULL) {
    printf("-- NULL LLIST --\n");
    return;
  }
  ptr = l->head;
  while(ptr != NULL){
    printf("%d ",ptr->value);
    ptr = ptr->next;
  }
}

void ll_i_println(Llist_i_comp l){
  ll_i_print(l);
  printf("\n");
}


void ll_i_free(Llist_i_comp *l){
  struct ll_i_cell_comp *ptr;

  if(*l != NULL){
    while((*l)->head != NULL){
      ptr = (*l)->head;
      (*l)->head = (*l)->head->next;
      ll_i_pfree(ptr);
    }
    ll_i_pfree(*l);
    *l = NULL;
  }
}


int ll_i_print_n_malloc(){
  printf("ll_i_malloc=%d\n",ll_i_n_malloc);
  return ll_i_n_malloc;
}

int ll_i_print_n_free(){
  printf("ll_i_free=%d\n",ll_i_n_free);
  return ll_i_n_free;
}



int ll_i_length(Llist_i_comp ll){
  struct ll_i_cell_comp *ptr;
  int i = 0;
  if(ll == NULL) return 0;
  ptr = ll->head;
  while(ptr != NULL){
    ptr = ptr->next;
    i++;
  }
  return i;
}

Llist_i_comp ll_i_copy(Llist_i_comp ll){
  Llist_i_comp temp = NULL;
  struct ll_i_cell_comp *ptr;
  if(ll == NULL) return NULL;
  ptr = ll->head;
  while(ptr != NULL){
    ll_i_insert_at_tail(&temp,ptr->value);
    ptr = ptr->next;
  }
 
  return temp;
}


/*------------------------------------------------------*/



unsigned char toupper(unsigned char c) {
  // called from "get_sous_noeud_g_comp"
  /* converts a filename (ascii only) to uppercase
     Maybe this function was introduced for backward compability
     to Unitex on DOS-based systems Win 95/98/ME and (accordingly) Intex.
     I think it should depend on the operating system, which
     file names point to the the same file. Unitex makes a difference
     between different cased filenames in the graph editor (depending on Java,
     which provides only an interface to the OS), so it should behave
     when compiling graphs.
     I.e.: THIS FUNCTION IS OBSOLETE!
  */
#ifdef _NOT_UNDER_WINDOWS
  /* Since on Unix OSs case in filenames has always been distinctive,
     this would be a "pragmatic" hack, but makes (for those who think
     "case.grf" and "CASE.GRF" are the same file) graphs not
     portable: */
  return c;
#endif
  /* Beside: toupper is also defined in <ctype.h>.
     Not very sophisticated to replace stdlib-functions.
     (Seb., Munich) */
  if (c>='a' && c<='z') return (unsigned char)(c-('a'-'A'));
  return c;
}



/* alloue n octets en mémoire et incremente n_malloc_comp*/

void *malloc_comp(int n)
{
  void *ptr;
  n_malloc_comp++;
  ptr = malloc(n);
  if(ptr == NULL) {
    exit(1);
  }
  return ptr;
}

/* libère espace memoire et incremente n_free_comp*/

void free_comp(void *ptr)
{
  n_free_comp++;
  free(ptr);
}



int is_letter_generic(unichar c,int mode,Alphabet* alph) {
switch(mode) {
case DEFAULT_TOKENIZATION: return u_is_letter(c);
case CHAR_BY_CHAR_TOKENIZATION: return 0; // by convention
case ALPHABET_TOKENIZATION: return is_letter(c,alph);
default: fprintf(stderr,"Internal error in is_letter_generic\n"); exit(1);
}
}




///////////////////////////////////////////////
// QUELQUES INITIALISATIONS                 //
//////////////////////////////////////////////

//
// initialise le graphe
//
void init_graphe_mat_det(Etat_fst_det resultat[]) {
long int i;
for (i=0;i<NBRE_ET;i++)
  resultat[i]=NULL;
}

///////////////////////////////////////////////////////////////
//////   GESTION DU BI-ARBRE DES ENSEMBLES  ///////////////////
/////////////////////////////////////////////////////////////


struct noeud_num_char_det* nouveau_noeud_num_char_det()
{
  struct noeud_num_char_det *n;
  n=(struct noeud_num_char_det*)malloc_comp(sizeof(struct noeud_num_char_det));
  n->num_char=-1;
  n->liste=NULL;
  return n;
}


struct noeud_valeur_det* nouveau_noeud_valeur_det()
{
  struct noeud_valeur_det *n;
  n = (struct noeud_valeur_det*)malloc_comp(sizeof(struct noeud_valeur_det));
  n->valeur = -1;
  n->indice = -1;
  n->liste = NULL;
  return n;
}



struct liste_branches_num_char_det* nouvelle_liste_branches_num_char_det()
{
  struct liste_branches_num_char_det *l;
  l=(struct liste_branches_num_char_det*)malloc_comp(sizeof(struct liste_branches_num_char_det));
  l->n=NULL;
  l->suivant=NULL;
  return l;
}



struct liste_branches_valeur_det* nouvelle_liste_branches_valeur_det()
{
  struct liste_branches_valeur_det *l;
  l=(struct liste_branches_valeur_det*)malloc_comp(sizeof(struct liste_branches_valeur_det));
  l->n=NULL;
  l->suivant=NULL;
  return l;
}




struct noeud_num_char_det* get_sous_noeud_num_char_det(int num_char,struct noeud_valeur_det *v)
{
  struct liste_branches_num_char_det *l;

  if (v->liste==NULL)
  {
    l=nouvelle_liste_branches_num_char_det();
    l->n=nouveau_noeud_num_char_det();
    (l->n)->num_char=num_char;
    v->liste=l;
    return l->n;
  }
  l=v->liste;
  while (l!=NULL)
  {
   if ((l->n)->num_char==num_char)
     return l->n;
   else l=l->suivant;
  }
  l=nouvelle_liste_branches_num_char_det();
  l->n=nouveau_noeud_num_char_det();
  (l->n)->num_char=num_char;
  l->suivant=v->liste;
  v->liste=l;
  return l->n;
}


struct noeud_valeur_det* get_sous_noeud_valeur_det(int valeur,struct noeud_num_char_det *nc)
{
  struct liste_branches_valeur_det *l;
   if (nc->liste==NULL)
   {
     l=nouvelle_liste_branches_valeur_det();
     l->n=nouveau_noeud_valeur_det();
     (l->n)->valeur=valeur;
     nc->liste=l;
     return l->n;
   }
  l=nc->liste;
  while (l!=NULL)
  {
    if ((l->n)->valeur==valeur)
      return l->n;
    else l=l->suivant;
  }
  l=nouvelle_liste_branches_valeur_det();
  l->n=nouveau_noeud_valeur_det();
  (l->n)->valeur=valeur;
  l->suivant=nc->liste;
  nc->liste=l;
  return l->n;
}



//
// recherche un ensemble dans l'arbre des ensembles
// s'il le trouve retourne son numero
// s'il ne le trouve pas l'ajoute a l'arbre, retourne dernier_etat_res+1;
//

int numero_ensemble_det(ensemble_det e,struct noeud_valeur_det *node,int dernier_etat_res)
{
  struct noeud_num_char_det *nc;
  struct noeud_valeur_det *v;
  if (e==NULL)
  {
    if (node->indice != -1) return node->indice;
    else
    {
      return (node->indice = dernier_etat_res+1);
    }
  }
  nc=get_sous_noeud_num_char_det(e->num_char,node);
  v=get_sous_noeud_valeur_det(e->valeur,nc);
  return numero_ensemble_det(e->suivant,v,dernier_etat_res);
}




//////////////////////////////////////////////////////////////////
/////// MANIPULATION DES ENSEMBLES D'ETATS //////////////////////
////////////////////////////////////////////////////////////////


//
// cree et retourne un nouveau char_etats
//

ensemble_det nouveau_char_etats_det()
{
  ensemble_det ce;
  ce=(ensemble_det)malloc_comp(sizeof(struct char_etats_det));
  ce->suivant=NULL;
  ce->valeur=0;
  ce->num_char=0xFFFF;
  return ce;
}



//
// ajoute un nouvel etat a un ensemble d'etats
//

void ajouter_etat_dans_ensemble_det(int netat,ensemble_det *e) {
  ensemble_det ptr,ptr1,pos;
  unsigned int numero=netat/32;
   if(*e == NULL)
   {
     *e=nouveau_char_etats_det();
     (*e)->num_char=numero;
     (*e)->valeur=1<<(netat%32);
     return;
   }
   ptr=(*e);
   if(ptr->num_char==numero)
   {
     ptr->valeur=(ptr->valeur)|(1<<(netat%32));
     return;
   }
   if(ptr->num_char>numero)
   {
     ptr1=nouveau_char_etats_det();
     ptr1->num_char=numero;
     ptr1->valeur=1<<(netat%32);
     (*e)=ptr1;
     (*e)->suivant=ptr;
     return;
   }
   if(ptr->suivant==NULL) {
       ptr->suivant=nouveau_char_etats_det();
       ptr=ptr->suivant;
       ptr->num_char=numero;
       ptr->valeur=1<<(netat%32);
       return;
     }
   pos=ptr;
   ptr=ptr->suivant;


   while(ptr!=NULL) {
       if(ptr->num_char==numero)
       {
	   ptr->valeur=(ptr->valeur)|(1<<(netat%32));
	   return;
	 }
       if(ptr->num_char>numero) {
	   ptr1=nouveau_char_etats_det();
	   ptr1->num_char=numero;
	   ptr1->valeur=1<<(netat%32);
	   ptr1->suivant=ptr;
	   pos->suivant=ptr1;
	   return;
	 }
       if(ptr->suivant==NULL) {
	   ptr->suivant=nouveau_char_etats_det();
	   ptr=ptr->suivant;
	   ptr->num_char=numero;
	   ptr->valeur=1<<(netat%32);
	   return;
	 }
       pos=ptr;
       ptr=ptr->suivant;
     }
}




//
// liberer un char_etats
//
void liberer_char_etat_det(ensemble_det ptr) {
   ensemble_det ptr1;
   while (ptr!=NULL)
   {
     ptr1=ptr;
     ptr=ptr->suivant;
     free_comp(ptr1);
   }
}






////////////////////////////////////////////////////
///  GESTION DE L'AUTOMATE (GRAPHE FST)          ///
///////////////////////////////////////////////////



void vider_noeud_valeur_det(struct noeud_valeur_det *v)
{
  struct liste_branches_num_char_det *old;
  struct liste_branches_num_char_det *ptr;

  ptr=v->liste;
   while (ptr!=NULL)
   {
     old=ptr;
     ptr=ptr->suivant;
     vider_noeud_num_char_det(old->n);
     free_comp(old->n);
     free_comp(old);
   }
}



void vider_noeud_num_char_det(struct noeud_num_char_det *nc) {
struct liste_branches_valeur_det *old;
struct liste_branches_valeur_det *ptr;
ptr=nc->liste;
 while (ptr!=NULL) {
   old=ptr;
   ptr=ptr->suivant;
   vider_noeud_valeur_det(old->n);
   free_comp(old->n);
   free_comp(old);
 }
}




void liberer_arbre_det(struct noeud_valeur_det *racine)
{
  vider_noeud_valeur_det(racine);
  free_comp(racine);
}




//
// cree et renvoie un etat vierge
//
Etat_fst_det nouvel_etat_mat_det()
{
  Etat_fst_det e;
  e = (Etat_fst_det)malloc_comp(sizeof(struct etat_fst_det));
  e->controle = 0;
  e->trans = NULL;
  e->ens = NULL;
  return e;
}



//
// cree et renvoie une nouvelle transition
//
liste_transition_det nouvelle_transition_mat_det()
{
  liste_transition_det t;
  t=(liste_transition_det)malloc_comp(sizeof(struct transition_fst_det));
  t->etiquette=-1;
  t->arr=-1;
  t->suivant=NULL;
  return t;
}


//
// ajoute une transition a l'etat courant
//

void ajouter_transition_mat_det(struct etat_fst_det *e,int etiq,int etarr)
{
  struct transition_fst_det *ptr;

  ptr=nouvelle_transition_mat_det();
  ptr->suivant=e->trans;
  ptr->etiquette=etiq;
  ptr->arr=etarr;
  e->trans=ptr;
}



//
//liberation de transtion dans etat i
//

void liberer_transition_mat_det(struct transition_fst_det *ptr)
{
  struct transition_fst_det *ptr1;
  while (ptr!=NULL)
  {
    ptr1=ptr;
    ptr=ptr->suivant;
    free_comp(ptr1);
  }
}



void liberer_etat_det(Etat_fst_det e)
{
  liberer_char_etat_det(e->ens);
  liberer_transition_mat_det(e->trans);
  free_comp(e);

}



///////////////////////////////////////////////////////////////
////// DETERMINISATION  //////////////////////////////////////
/////////////////////////////////////////////////////////////

void init_hachage_det(int h[])
{
  // time-critical function, called often
  register int i;
  // instead of
  //    for(i=0;i < NBRE_ETIQ_TRANSITION_COMP;i++) h[i]=-1;
  // initializing only used fields saves time:
  for (i=0; i < (nombre_etiquettes_comp+nombre_graphes_comp+1); i++)
    h[i]=-1;
}


void init_stock_det(ensemble_det s[])
{
  int i;
  for(i=0;i < NBRE_ETIQ_TRANSITION_COMP;i++) s[i]=NULL;
}



void init_resultat_det(Etat_fst_det resultat[],struct noeud_valeur_det *racine,int dernier_etat_res)
{
  resultat[0] = nouvel_etat_mat_det();
  ajouter_etat_dans_ensemble_det(0,&(resultat[0]->ens));
  numero_ensemble_det(resultat[0]->ens,racine,dernier_etat_res);
}


ensemble_det copie_det(ensemble_det e) {
  ensemble_det nouveau;
  if(e!=NULL) {
      nouveau=(ensemble_det)malloc_comp(sizeof(struct char_etats_det));
      nouveau->num_char=e->num_char;
      nouveau->valeur=e->valeur;
      nouveau->suivant=copie_det(e->suivant);
      return nouveau;
    }
  return NULL;
}



///////////////////////////////////////////////////////////////
////////////// AUTRES ////////////////////////////////////////
/////////////////////////////////////////////////////////////



void sauvegarder_etat_det(FILE *f,Etat_fst_det e)
{
 struct transition_fst_det *ptr;
 if(e->controle&1)
   u_fputc((unichar)'t',f);
 else u_fputc((unichar)':',f);
 ptr=e->trans;
 while(ptr!=NULL)
   {
     char s[1000];
     sprintf(s," %d %d",ptr->etiquette,ptr->arr);
     u_fprints_char(s,f);
     ptr=ptr->suivant;
   }
 u_fputc((unichar)' ',f);
 u_fputc((unichar)'\n',f);
}


int determinisation(Etat_comp graphe[]) {
  Transition_comp ptr;
  ensemble_det courant;
  unsigned long int q;  //etat courant ancien graphe
  int count;  //compteur pour savoir ou l'on se trouve dans notre int de 32 bits
  int compteur; //compteur pour savoir l'indice du dernier ensemble rentre dans stock;
  int num;
  int i,file_courant=0,dernier_etat_res,sous_graphe,temp2,temp,k;
  struct noeud_valeur_det *racine_det;

  dernier_etat_res = -1;
  racine_det = nouveau_noeud_valeur_det();
  init_graphe_mat_det(resultat);
  init_resultat_det(resultat,racine_det,dernier_etat_res);
  dernier_etat_res = 0;

  if ( (graphe[0]->controle) & 1)
    resultat[0]->controle = (unsigned char)(resultat[0]->controle | 1);
  init_stock_det(stock);
  temp2 = file_courant % NBRE_ET;
  while(resultat[temp2] != NULL)
  {
    courant = resultat[temp2]->ens;
    init_hachage_det(hachage);
    compteur = 0;
    while(courant != NULL)
    {
      count = 0;
      q = (courant->num_char*32) - 1;
      while(count < 32)
      {
        q++;
        if(((courant->valeur)&(1<<count))!=0)
        {
          ptr = graphe[q]->trans;
          while(ptr != NULL)
          {
            temp = ptr->etiq;
            if(temp < 0)
            {
              temp = nombre_etiquettes_comp - 1 - temp;
              sous_graphe = 1;
            }
            else sous_graphe = 0;
            if(hachage[temp] == -1)
            {
              hachage[temp] = compteur;
              liberer_char_etat_det(stock[compteur]);
              final[compteur] = 0;
              if(sous_graphe == 0)
                hachageinv[compteur] = temp;
              else
                hachageinv[compteur] = nombre_etiquettes_comp -1 - temp;

                stock[compteur] = NULL;
                compteur++;
              }
              ajouter_etat_dans_ensemble_det(ptr->arr,&stock[hachage[temp]]);
              if(((graphe[ptr->arr]->controle) & 1 ) != 0)
                final[hachage[temp]] = 1;    //test de finalite
              ptr = ptr->suivant;
            }
           }
           count++;
        }
        courant = courant->suivant;
      }
      for(i=0;i < compteur;i++)
      {
        num = numero_ensemble_det(stock[i],racine_det,dernier_etat_res);
        temp = num % NBRE_ET;
        ajouter_transition_mat_det(resultat[temp2],hachageinv[i],num);
        if(num > dernier_etat_res)
        {
          if (resultat[temp]!=NULL)
          {
                   /* m="Too many states in deterministic graph\n";
		    	    m=m+"Not enough memory to continue";
                    erreur(m.c_str());*/
             for(k=0;k<NBRE_ET;k++)liberer_etat_det(resultat[k]);
             liberer_arbre_det(racine_det);
             for (i=0;i < NBRE_ETIQ_TRANSITION_COMP;i++)
                if (stock[i]!=NULL)
                   //free_comp(stock[i]);
                   liberer_char_etat_det(stock[i]);
             return 0;
           }
           resultat[temp] = nouvel_etat_mat_det();
           dernier_etat_res++;
           resultat[temp]->ens = copie_det(stock[i]);
           resultat[temp]->controle = (unsigned char)((resultat[temp]->controle) | final[i]);
        }
      }
      sauvegarder_etat_det(fs_comp,resultat[temp2]);
      liberer_etat_det(resultat[temp2]);
      resultat[temp2] = NULL;
      file_courant++;
      temp2 = file_courant % NBRE_ET;
   }
  char s[10];
  sprintf(s,"f \n");
  u_fprints_char(s,fs_comp);
  liberer_arbre_det(racine_det);
  for (i=0;i < NBRE_ETIQ_TRANSITION_COMP;i++)
    {
      if (stock[i]!=NULL)
        {
          // free_comp(stock[i]);
          liberer_char_etat_det(stock[i]);
        }
    }
  return 1;
}



/////////////////////////////////////////////////////////////////////
////////// MANIPULATION DE LA STRUCTURE D'ETAT /////////////////////
////////////////////////////////////////////////////////////////////


//
// renvoie un Etat vierge
//

Etat_comp nouvel_etat_comp()
{
  Etat_comp e;

  e = (Etat_comp)malloc_comp(sizeof(struct etat_comp));
  e->controle = 0;
  e->trans = NULL;
  e->transinv = NULL;
  return e;
}

Transition_comp nouvelle_transition_comp()
{
  Transition_comp t;

  t = (Transition_comp)malloc_comp(sizeof(struct transition_comp));
  t->arr = -1;
  t->etiq = 0;
  t->suivant = NULL;

  return t;
}


void ajouter_transition_comp(Etat_comp *letats,int dep,int arr,int etiq)
{
  Transition_comp ptr;

  //transition
  ptr = nouvelle_transition_comp();
  ptr->arr = arr;
  ptr->etiq = etiq;
  ptr->suivant = letats[dep]->trans;
  letats[dep]->trans = ptr;

  //transition inverse
  ptr = nouvelle_transition_comp();
  ptr->arr = dep;
  ptr->etiq = etiq;

  ptr->suivant = letats[arr]->transinv;
  letats[arr]->transinv = ptr;

}


//
// vide la liste de transitions ptr
//
void vider_transitions_comp(Transition_comp ptr)
{
  if (ptr==NULL) return;
  vider_transitions_comp(ptr->suivant);
  free_comp(ptr);
}


void liberer_etat_graphe_comp(Etat_comp etat)
{
  if (etat!=NULL)
  {
    vider_transitions_comp(etat->trans);
    vider_transitions_comp(etat->transinv);
    free_comp(etat);
  }

}



//
// libere le graphe graphe
//
void liberer_graphe_comp(Etat_comp graphe[])
{
  int i;
   for (i=0;i<NOMBRE_ETATS_COMP;i++)
     if (graphe[i]!=NULL)
     {
       vider_transitions_comp(graphe[i]->trans);
       vider_transitions_comp(graphe[i]->transinv);
       free_comp(graphe[i]);
     }
}


int ajouter_etat_deliage_comp(Etat_comp letats[],int dep,int etiq,int *n_etats,int graphe_courant)
{
  if(((*n_etats)+1) >= NOMBRE_ETATS_COMP)
    {
      char err[1000];
      u_to_char(err,donnees->nom_graphe[graphe_courant]);
      fprintf(stderr,"ERROR in graph %s.grf :Too many states\n",err);
      return 0;
    }
  letats[*n_etats] = nouvel_etat_comp();
  ajouter_transition_comp(letats,dep,*n_etats,etiq);
 (*n_etats)++;
 return 1;
}


////////////////////////////////////////////////////////////
///// MANIPULATION DE L'ARBRE DES ETIQUETTES /////////////////////////
/////////////////////////////////////////////////////////


//
// renvoie un noeud vierge
//

struct noeud_comp* nouveau_noeud_comp()
{
  struct noeud_comp *n;

  n = (struct noeud_comp*)malloc_comp(sizeof(struct noeud_comp));
  n->fin = -1;
  n->lettre = 1;
  n->l = NULL;
  return n;
}


//
// retourne le sous-noeud correspondant au char c; le cree si absent
//

struct noeud_comp* get_sous_noeud_comp(struct noeud_comp *n,unichar c,int creer)
{
  struct liste_feuilles_comp *ptr;
  struct liste_feuilles_comp *ptr2;
  struct noeud_comp* res;

  ptr = n->l;
  while ((ptr != NULL) && ((ptr->node)->lettre != c))
    ptr = ptr->suivant;
  if (ptr == NULL)
    {        // si on veut juste savoir si le noeud existe
      if (!creer)           // et que le noeud n'existe pas, on renvoie NULL
	return NULL;
      res = nouveau_noeud_comp();
      res->lettre = c;
      ptr2 = (struct liste_feuilles_comp*)malloc_comp(sizeof(struct liste_feuilles_comp));
      ptr2->node = res;
      ptr2->suivant = n->l;
      n->l = ptr2;

      return res;
    }
  return ptr->node;
}



//
// ajoute une etiquette si absente de l'arbre
//

int ajouter_etiquette_comp(unichar *etiq, struct noeud_comp *ptr,int i)
{
  struct noeud_comp *ptr1;

  ptr1 = get_sous_noeud_comp(ptr,etiq[i],1);
  i++;
  if(etiq[i] == '\0')
    {
      if(ptr1->fin == -1)
	{
	  ptr1->fin = nombre_etiquettes_comp;
	  nombre_etiquettes_comp++;
	}
      return (ptr1->fin);
    }
  return ajouter_etiquette_comp(etiq,ptr1,i);
}



//
// vide sans la detruire, une feuille de l'arbre des etiquettes
//

void vider_feuille_comp(struct noeud_comp *n)
{
  struct liste_feuilles_comp *ptr;
  struct liste_feuilles_comp *ptr2;

  ptr = n->l;
  if (ptr != NULL)
    {
      ptr2 = ptr->suivant;
      vider_feuille_comp(ptr->node);
      free_comp(ptr->node);
      free_comp(ptr);
      while (ptr2 != NULL)
	{
	  ptr = ptr2;
	  ptr2 = ptr->suivant;
	  vider_feuille_comp(ptr->node);
	  free_comp(ptr->node);
	  free_comp(ptr);
	}
    }
}




//
// libere l'arbre des etiquettes
//

void libere_etiquettes_comp()
{
  vider_feuille_comp(rac_comp);
  free_comp(rac_comp);
}



////////////////////////////////////////////////////////////
///// MANIPULATION DES ARBRES DES GRAPHES /////////////////////////
/////////////////////////////////////////////////////////


//
// renvoie un noeud vierge
//

struct noeud_g_comp* nouveau_noeud_g_comp()
{
  struct noeud_g_comp *n;
  n = (struct noeud_g_comp*)malloc_comp(sizeof(struct noeud_g_comp));
  n->fin = -1;
  n->lettre = 1;
  n->l = NULL;

  return n;
}


//
// retourne le sous-noeud correspondant au char c; le cree si absent
//

struct noeud_g_comp* get_sous_noeud_g_comp(struct noeud_g_comp *n,unichar c,int creer)
{
  struct liste_feuilles_g_comp *ptr;
  struct liste_feuilles_g_comp *ptr2;
  struct noeud_g_comp* res;

  ptr=n->l;
  //  while ((ptr != NULL) && (toupper((ptr->node)->lettre) != toupper(c))) // see "toupper" above
  while ((ptr != NULL) && ((ptr->node)->lettre != c))
    ptr = ptr->suivant;
  if (ptr == NULL)
    {        // si on veut juste savoir si le noeud existe
      if (!creer)           // et que le noeud n'existe pas, on renvoie NULL
	return NULL;
      res=nouveau_noeud_g_comp();
      res->lettre = c;
      ptr2=(struct liste_feuilles_g_comp*)malloc_comp(sizeof(struct liste_feuilles_g_comp));
      ptr2->node = res;
      ptr2->suivant = n->l;
      n->l = ptr2;
      return res;
    }
  return ptr->node;
}


//
// ajoute un nom de graphe si absent de l'arbre
//

int ajouter_graphe_comp(unichar *etiq, struct noeud_g_comp *ptr,int i)
{
  struct noeud_g_comp *ptr1;

  ptr1=get_sous_noeud_g_comp(ptr,etiq[i],1);
  i++;
  if(etiq[i]=='\0')
    {
      if(ptr1->fin == -1)
	{
	  ptr1->fin=nombre_graphes_comp;
	  nombre_graphes_comp++;
	}
      return (ptr1->fin);
    }
  return ajouter_graphe_comp(etiq,ptr1,i);
}



//
// vide sans la detruire, une feuille de l'arbre des graphes
//

void vider_feuille_g_comp(struct noeud_g_comp *n)
{
  struct liste_feuilles_g_comp *ptr;
  struct liste_feuilles_g_comp *ptr2;

  ptr=n->l;
  if (ptr != NULL)
    {
      ptr2=ptr->suivant;
      vider_feuille_g_comp(ptr->node);
      free_comp(ptr->node);
      free_comp(ptr);
      while (ptr2 != NULL)
	{
	  ptr=ptr2;
	  ptr2=ptr->suivant;
	  vider_feuille_g_comp(ptr->node);
	  free_comp(ptr->node);
	  free_comp(ptr);
	}
    }
}




//
// libere l'arbre des graphes
//

void libere_graphes_comp()
{
  vider_feuille_g_comp(rac_graphe_comp);
  free_comp(rac_graphe_comp);
}


void libere_arbres_comp()
{
  libere_graphes_comp();
  libere_etiquettes_comp();
}


void init_arbres_comp()
{
  rac_comp = NULL;
  rac_comp = nouveau_noeud_comp();
  unichar s[10];
  u_strcpy_char(s,"<E>");
  EPSILON_comp = ajouter_etiquette_comp(s,rac_comp,0);
  u_strcpy(donnees->Etiquette_comp[EPSILON_comp],s);
  rac_graphe_comp = NULL;
  rac_graphe_comp = nouveau_noeud_g_comp();

}


///////////////////////////////////////////////////////////////////
/////// FONCTIONS CONCERNANT LE NETTOYAGE DU GRAPHE //////////////
//////////////////////////////////////////////////////////////////


//
// enleve la transition inverse allant vers origine de la liste ptr
//
Transition_comp degager_transition_inverse_comp(Transition_comp ptr,int origine) {
  Transition_comp tmp;
  if (ptr==NULL) return NULL;
  if (((ptr->arr)==origine)&&((ptr->etiq)==EPSILON_comp)) {
    tmp=ptr->suivant;
    free_comp(ptr);
    return tmp;
  }
  ptr->suivant=degager_transition_inverse_comp(ptr->suivant,origine);
  return ptr;
}



//
// enleve les epsilon transitions de la liste ptr
//
Transition_comp vider_epsilon_comp(Transition_comp ptr,Etat_comp *letats,int origine, int mark[]) {  
  Transition_comp liste,tmp;
  Etat_comp e,e2;  
  if (ptr == NULL) return NULL;
  if (ptr->etiq == EPSILON_comp) {
    e = letats[ptr->arr];
    mark[ptr->arr] = 1; // we mark the current state as already been a destination state
    //printf("mark %d\n",ptr->arr);
    //printf("%d:%d\n",origine,ptr->arr);
    if (ptr->arr == origine) {
      // if we have a loop by epsilon, we can ignore it, because (a|<E>)+ has
      // has already been turned into a+ | <E>
      liste=ptr;
      ptr=ptr->suivant;
      free(liste);
      return ptr;
    }
    if ((e->controle)&1) {
      // if we can reach a final state through an epsilon transition
      // then the current state must be final
      letats[origine]->controle=(unsigned char)((letats[origine]->controle)|1);
    }    
    liste=e->trans;
    while (liste!=NULL) {
      if(mark[liste->arr] == 0){ //if not marked
	tmp=nouvelle_transition_comp();
	tmp->etiq=liste->etiq;
	tmp->arr=liste->arr;
	tmp->suivant=ptr->suivant;
	ptr->suivant=tmp;
	//printf("%d,%d,%d\n",origine,tmp->etiq,tmp->arr);
	
	e2=letats[liste->arr];
	tmp=nouvelle_transition_comp();
	tmp->etiq=liste->etiq;
	tmp->arr=origine;
	tmp->suivant=e2->transinv;
	e2->transinv=tmp;
      }
      liste=liste->suivant;
    }
    tmp=ptr->suivant;
    e->transinv=degager_transition_inverse_comp(e->transinv,origine);   
    free_comp(ptr);
    return vider_epsilon_comp(tmp,letats,origine,mark);
  }
  ptr->suivant=vider_epsilon_comp(ptr->suivant,letats,origine,mark);
  return ptr;
}


int* new_marker(int n){
  int *res = new int[n];
  int i = 0;

  for(i = 0 ; i < n ; i++){
    res[i] = 0;
  }

  return res;
}


void add_closure(Llist_i_comp* closures,int q,Llist_i_comp nlist){
  if(nlist == NULL) return;

  struct ll_i_cell_comp *ptr = nlist->head;

  while(ptr != NULL){
    ll_i_insert_sorted(&(closures[q]),ptr->value);
    
    ptr = ptr->next;
  }

}



Llist_i_comp get_epsilon_closure_rec(Llist_i_comp* closures,Etat_comp *letats,int q, unsigned char mark[]){
  Transition_comp ptr;

  if(mark[q] == 0){ // si non marque
    mark[q] = 1;
    ll_i_insert_sorted(&(closures[q]),q);
    ptr = letats[q]->trans;
    while(ptr != NULL){
      if((ptr->etiq == EPSILON_comp) && (ptr->arr != q)){
	add_closure(closures,q,get_epsilon_closure_rec(closures,letats,ptr->arr,mark));
      }
      ptr = ptr->suivant;
    }
  }
  return closures[q];
}


Llist_i_comp * get_epsilon_closures(Etat_comp *letats,int n){
  Llist_i_comp * closures = (Llist_i_comp *)malloc(n*sizeof(Llist_i_comp));
  unsigned char mark[n];
  int i;  

  for(i = 0 ; i < n ; i++){
    mark[i] = 0;
    closures[i] = NULL;    
  }


  for(i = 0 ; i < n ; i++){ //pour chaque etat
    if(mark[i] == 0){
      get_epsilon_closure_rec(closures,letats,i,mark);
    }
  }

  return closures;
}


void remove_all_epsilon_transitions(Etat_comp *letats, int n){
  Transition_comp ptr,res,temp;  

  for(int i = 0 ; i < n ; i++){ // pour chq etat
    ptr = letats[i]->trans;
    res = NULL;
    while(ptr != NULL){ //pr chq transition     
      temp = ptr;
      ptr = ptr->suivant;
      
      if(temp->etiq == EPSILON_comp){ // on supprime transition
	free_comp(temp); 
      }
      else{
	temp->suivant = res;
	res = temp;
      }      
    }
    letats[i]->trans = res;

    ptr = letats[i]->transinv;
    res = NULL;
    while(ptr != NULL){ //pr chq transition inverse
      temp = ptr;
      ptr = ptr->suivant;
      
      if(temp->etiq == EPSILON_comp){ // on supprime transition inverse
	free_comp(temp); 
      }
      else{
	temp->suivant = res;
	res = temp;
      }      
    }
    letats[i]->transinv = res;


  }
}


void add_transitions_according_to_epsilon_closure(Llist_i_comp *closures,Etat_comp *letats,int n){
  unsigned int i;
  struct ll_i_cell_comp *ptr;
  Transition_comp tr;
  Etat_comp e;

  for(i = 0 ; i < (unsigned int)n ; i++){ //pour chq etat
    Llist_i_comp clos = closures[i];
    if(clos != NULL){
      ptr = clos->head;     
      while(ptr != NULL){
	if(ptr->value != i){
	  e = letats[ptr->value];
	  if((e->controle) & 1) { // etat final
	    letats[i]->controle = (unsigned char)(letats[i]->controle)|1; 
	  }
	  tr = e->trans;
	  while(tr != NULL){
	    ajouter_transition_comp(letats,i,tr->arr,tr->etiq);
	    tr = tr->suivant;
	  }
	}
	ptr = ptr->next;
      }
    }
  }
}


void free_closures(Llist_i_comp *closures,int n){
  int i;

  for(i = 0 ; i < n ; i++){
    ll_i_free(&(closures[i]));
  }
}



//
// enleve les epsilon transitions du graphe
//
void virer_epsilon_transitions_comp(Etat_comp *letats,int n){
  Llist_i_comp *closures = get_epsilon_closures(letats,n);
  remove_all_epsilon_transitions(letats,n);
  add_transitions_according_to_epsilon_closure(closures,letats,n);
  free_closures(closures,n);
  free_comp(closures);


  /*
  //OLD VERSION
  for (int i=0 ; i<n ; i++) {  
    letats[i]->trans=vider_epsilon_comp(letats[i]->trans,letats,i,mark); 
  }
  */
}


//
// Marque les etats accessibles
//

void accessibilite_comp(Etat_comp *e,int i)
{
  Transition_comp t;

  if((e[i]!=NULL) && ((e[i]->controle)&4)==0)
    {
      e[i]->controle=(unsigned char)((e[i]->controle)|4);
      t=e[i]->trans;
      while(t!=NULL)
	{
	  accessibilite_comp(e,t->arr);
	  t=t->suivant;
	}
    }
}


//
//Marque les etats co_accessibles
//

void co_accessibilite_comp(Etat_comp *e,int i)
{
  Transition_comp t;

  if((e[i]!=NULL) && ((e[i]->controle)&8)==0)
    {
      e[i]->controle=(unsigned char)((e[i]->controle)|8);
      t=e[i]->transinv;
      while(t!=NULL)
	{
	  co_accessibilite_comp(e,t->arr);
	  t=t->suivant;
	}
    }
}


Transition_comp supprimer_transition_comp(Etat_comp *letats,int i,Transition_comp ptr)
{
  Transition_comp tmp,t,tmp2;
  int j,etiq;

  j=ptr->arr;
  etiq=ptr->etiq;

  tmp=ptr->suivant;
  free_comp(ptr);

  /* Suppression de la transition inverse correspondante */
  t=letats[j]->transinv;
  while(t!=NULL)
    {
      if((t->arr==i) && (t->etiq==etiq))
	{
	  tmp2=t;
	  t=t->suivant;
	  free_comp(tmp2);
	}
      else
	t=t->suivant;
    }

  return tmp;
}


//
//Supprime une transition inverse + transition correspondante
//retourne transition suivante
//

Transition_comp supprimer_transitioninv_comp(Etat_comp *letats,int i,Transition_comp ptr)
{
  Transition_comp tmp,t,tmp2;
  int j,etiq;

  j=ptr->arr;
  etiq=ptr->etiq;

  tmp=ptr->suivant;
  free_comp(ptr);

  /* Suppression de la transition correspondante */
  t=letats[j]->trans;
  while(t!=NULL)
    {
      if((t->arr==i) && (t->etiq==etiq))
	{
	  tmp2=t;
	  t=t->suivant;
	  free_comp(tmp2);
	}
      else
	t=t->suivant;
    }

  return tmp;
}



//
// vire ptr si ptr pointe sur un etat a virer
//
// with complex graphs we got a stack overflow with this recursive function:
//// Transition_comp vider_trans_reciproques_comp(Transition_comp ptr,Etat_comp *letats)
//// {
////   Transition_comp tmp;
////   if (ptr==NULL) return NULL;
////   if((((letats[ptr->arr]->controle)&4)==0)||(((letats[ptr->arr]->controle)&8)==0))
////   {
////     tmp=ptr->suivant;
////     free_comp(ptr);
////     return vider_trans_reciproques_comp(tmp,letats);
////   }
////   ptr->suivant=vider_trans_reciproques_comp(ptr->suivant,letats);
////   return ptr;
//// }
// it's replaced now by an iterative one:
Transition_comp vider_trans_reciproques_comp(Transition_comp ptr,Etat_comp *letats)
{
  Transition_comp tmp, tmp2, tmp_old;
  tmp=ptr;
  while (tmp!=NULL)
    {
      if ((((letats[tmp->arr]->controle)&4)==0)||(((letats[tmp->arr]->controle)&8)==0))
        {
          tmp2=tmp->suivant;
          if (tmp == ptr)
            ptr = tmp2;
          else
            tmp_old->suivant = tmp2;
          free_comp(tmp);
          tmp=tmp2;
        }
      else
        {
          tmp_old = tmp;
          tmp=tmp->suivant;
        }
    }
  return ptr;
}



//
// renvoie 1 si l'etat e n'est ni accessible ni coaccessible
//
int est_a_virer_comp(Etat_comp e) {
return ((((e->controle)&4)==0)||(((e->controle)&8)==0));
}


//
// remplace ancien par nouveau dans les transitions sortant de l'etat e
//
void mettre_a_jour_sortie_comp(Etat_comp e,int ancien,int nouveau) {
Transition_comp ptr;
ptr=e->trans;
 while (ptr!=NULL) {
   if (ptr->arr==ancien) ptr->arr=nouveau;
   ptr=ptr->suivant;
 }
}




//
// remplace ancien par nouveau dans les transitions entrant dans l'etat e
//
void mettre_a_jour_entree_comp(Etat_comp e,int ancien,int nouveau) {
Transition_comp ptr;
ptr=e->transinv;
 while (ptr!=NULL) {
   if (ptr->arr==ancien) ptr->arr=nouveau;
   ptr=ptr->suivant;
 }
}




//
// renumerote les transitions vers ancien en transitions vers nouveau
//
void renumeroter_comp(Etat_comp *liste,int ancien,int nouveau) {
Transition_comp ptr;
ptr=liste[ancien]->trans;
 while (ptr!=NULL) {
   if (ptr->arr!=ancien) mettre_a_jour_entree_comp(liste[ptr->arr],ancien,nouveau);
   else ptr->arr=nouveau;
   ptr=ptr->suivant;
   }

ptr=liste[ancien]->transinv;
 while (ptr!=NULL) {
   mettre_a_jour_sortie_comp(liste[ptr->arr],ancien,nouveau);
   ptr=ptr->suivant;
   }
}




//
// Elimine les etats non accessibles et non co_accessibles
//

void eliminer_etats_comp(Etat_comp *letats,int *n_etats)
{
  int i,dernier;
  Etat_comp tmp;
  for (i=0;i<*n_etats;i++)
  {
    letats[i]->trans=vider_trans_reciproques_comp(letats[i]->trans,letats);
    letats[i]->transinv=vider_trans_reciproques_comp(letats[i]->transinv,letats);
  }

  dernier=(*n_etats)-1;
  i=0;
  do
  {
    while ((dernier>=0)&&(letats[dernier]==NULL))
      dernier--;
    while ((dernier>=0)&&est_a_virer_comp(letats[dernier]))
    {
      //free_comp(letats[dernier]);
      liberer_etat_graphe_comp(letats[dernier]);
      letats[dernier]=NULL;
      dernier--;
    }
    if (dernier==-1)
    {
      *n_etats=0;
      return;
    }
    while ((i<dernier)&&!est_a_virer_comp(letats[i]))
      i++;
    if (i==dernier)
    {
      *n_etats=dernier+1;
      return;
    }
    renumeroter_comp(letats,dernier,i);
    tmp=letats[i];
    letats[i]=letats[dernier];
    letats[dernier]=tmp;
    //free_comp(letats[dernier]);
    liberer_etat_graphe_comp(letats[dernier]);
    letats[dernier]=NULL;
    dernier--;
  }
  while (i<dernier);
  *n_etats=dernier+1;
}



///////////////////////////////////////////////////////////////
////////////////// FONCTIONS GENERALES/////////////////////////
///////////////////////////////////////////////////////////////

#ifndef _NOT_UNDER_WINDOWS
static int test4abs_windows_path_name(unichar* name) {
  if ( ((name[0] >= 'A' && name[0] <= 'Z') // A-Z
        ||
        (name[0] >= 'a' && name[0] <= 'z')) // a-z
       && (name[1] == ':')
       && ( (name[2] == '\\')
            || (name[2] == ':') )
       )
    return 1;
  return 0;
}
#endif

void conformer_nom_graphe_comp(char * NOM, int courant) {

  unichar nom[N_CAR_MAX_COMP];
  int offset;
  int abs_path_name_warning = 0; // 1 windows, 2 unix

  nom[0] = '\0'; // necessary if we have an absolute path name

  if (donnees->nom_graphe[courant][0] == ':')
    { /* the graph is located in the directory <pckg_path>.  If
         pckg_path is not defined, an (absolute) path is tried
         starting with '/' resp. '\\'. This enables absolute path
         names under Unixes. */

      u_strcpy(nom,pckg_path);
      offset = u_strlen(pckg_path);

      if (pckg_path[0] == '\0')
        abs_path_name_warning = 2;

    }

#ifndef _NOT_UNDER_WINDOWS
  else if ( test4abs_windows_path_name(donnees->nom_graphe[courant]) )
    { // we have an absolute windows path name ("c:\" but now "C::" )

      offset = 2; // "c::path:graph" -> "c:\path\graph.grf"
      abs_path_name_warning = 1;

    }
#endif

  else
    { // subgraph located relative to calling graph:
      //  relative path already calculated in lire_mot_comp


      // we add the path of the main graph in front of the path of the subgraph
      u_strcpy(nom,donnees->chemin_graphe_principal);

      offset = u_strlen(donnees->chemin_graphe_principal);

    }

  u_strcat(nom,donnees->nom_graphe[courant]);
  u_strcat_char(nom,".grf");

  u_to_char(NOM,nom); /* file name is now in iso-8859-1,
                         unicode characters not in iso-8859-1 are deleted */

  if (abs_path_name_warning != 0)
    {
      fprintf(stderr,
              "Absolute path name detected (%s):\n"
              "%s\n"
              "Absolute path names are not portable!\n",
              ((abs_path_name_warning == 1) ? "windows" :
               "unix: forgot to specify lib dir by -d <dir>?"),
              NOM);
    }

  NOM += offset;

  // transformation de ':' en '/' resp. '\'
  // but skip path to main graph (keep windows' "C:\" etc. alive!)
  while (*NOM)
    {
#ifdef _NOT_UNDER_WINDOWS
      if (*NOM == ':') { *NOM = '/'; }
#else
      if (*NOM == ':') { *NOM = '\\'; }
#endif
      ++NOM;
    }

}


//
// Sauvegarde du graphe compile
//


void sauvegarder_graphe_comp(Etat_comp *letats,int n_et,int n_gr)
{
  int i;
  Transition_comp t;
  char s[1000],s2[1000];

  u_to_char(s2,donnees->nom_graphe[n_gr]);
  sprintf(s,"%d %s\n",-n_gr-1,s2);
  u_fprints_char(s,fs_comp);
  if((donnees->statut_graphe[n_gr])==0 || n_et==0)
    {
      sprintf(s,": \nf \n");
      u_fprints_char(s,fs_comp);
    }
  else
    {
      for(i=0;i<n_et;i++)
	{
	  if(((letats[i]->controle)&1)==0) sprintf(s,":");
	  else sprintf(s,"t");
      u_fprints_char(s,fs_comp);
	  t=letats[i]->trans;
	  while(t!=NULL)
	    {
	      if (t->etiq>=0) sprintf(s," %d %d",t->etiq,t->arr);
	      else sprintf(s," %d %d",t->etiq,t->arr);
          u_fprints_char(s,fs_comp);
	      t=t->suivant;
	    }
      sprintf(s,"\n");
      u_fprints_char(s,fs_comp);
	}
      sprintf(s,"f \n");
      u_fprints_char(s,fs_comp);
    }
}





//
// initialisation locale
//

void init_locale_comp(Etat_comp *e, int n)
{
  int i;

  for(i=0;i<NOMBRE_ETATS_COMP;i++)
  {

   if(i >= n)
    e[i] = NULL;
   else
     e[i] = nouvel_etat_comp();
  }
}




//
// Sépare contenu (entrée de l'automate) et transduction (sortie)
//
void traitement_transduction_comp(unichar ligne[], unichar contenu[],unichar transduction[]) {
int i,j;
i=0;
//u_prints(ligne);
//getchar();
while (ligne[i]!='\0' && ligne[i]!='/') {
  if (ligne[i]=='\\') {
    // si on a un backslash et qu'on n'avait pas un backslash avant
    if (ligne[i+1]=='"' && (i==0 || (i>0 && ligne[i-1]!='\\'))) {
      // si on a un backslash-guillemet on cherche le prochain
      i=i+2;
      while (ligne[i]!='\0' && ligne[i]!='"') {
        //printf("XXXX ligne[%d]=%c",i,ligne[i]);
        //getchar();
        if (ligne[i]=='\\' && ligne[i+1]!='"') i++;
        i++;
      }
    }
    else {
       // we are in the case of a backslash, we must jump after the character
       i=i+2;
    }
  }
  else {
       //if (ligne[i]!='\0') i++;
       i++;
  }
}
contenu[0]='\0';
transduction[0]='\0';
if (ligne[i]=='\0') {
  // si pas de transduction...
  u_strcpy(contenu,ligne);
  return;
}
u_strcpy(contenu,ligne);
contenu[i]='\0';
j=0;
i++; // on saute le caractere /
while (ligne[i]!='\0') {
    if (ligne[i]=='\\') {
        i++;
        if (ligne[i]=='\0') {
                fatal_error("Unexpected backslash at end of line\n");
        }
    }
  transduction[j++]=ligne[i++];
}

}



void get_caractere_comp(unichar contenu[],int *pos,unichar mot[],int *pos_seq)
{
   mot[0] = contenu[*pos];
   mot[1] = '\0';
   (*pos)++;
   (*pos_seq)++;
}




// retourne -3 si pas bon
// retourne 1 sinon
int get_mot_comp_default(unichar contenu[],int *pos,unichar mot[],int *pos_seq)
{
   int i;
   i=0;
   do
   {
     mot[i] = contenu[*pos];
     (*pos)++;
     i++;
   }
   while(u_is_letter(contenu[(*pos)-1])&& (i<N_CAR_MAX_COMP));
  if(i>=N_CAR_MAX_COMP) return -3;
   mot[i-1] = '\0';
   (*pos)--;
   (*pos_seq)++;
   return 1;
}


// retourne -3 si pas bon
// retourne 1 sinon
int get_mot_comp_alphabet(unichar contenu[],int *pos,unichar mot[],int *pos_seq,Alphabet* alph)
{
   int i;
   i=0;
   do
   {
     mot[i] = contenu[*pos];
     (*pos)++;
     i++;
   }
   while(is_letter(contenu[(*pos)-1],alph)&& (i<N_CAR_MAX_COMP));
  if(i>=N_CAR_MAX_COMP) return -3;
   mot[i-1] = '\0';
   (*pos)--;
   (*pos_seq)++;
   return 1;
}



// retourne -3 si pas bon
// retourne 1 sinon
int get_mot_comp_char_by_char(unichar contenu[],int *pos,unichar mot[],int *pos_seq)
{
   mot[0]=contenu[*pos];
   mot[1]='\0';
   (*pos)++;
   (*pos_seq)++;
   return 1;
}


int get_mot_comp_generic(unichar contenu[],int *pos,unichar mot[],int *pos_seq,
                         int mode,Alphabet* alph)
{
switch(mode) {
case DEFAULT_TOKENIZATION: return get_mot_comp_default(contenu,pos,mot,pos_seq);
case CHAR_BY_CHAR_TOKENIZATION: return get_mot_comp_char_by_char(contenu,pos,mot,pos_seq);
case ALPHABET_TOKENIZATION: return get_mot_comp_alphabet(contenu,pos,mot,pos_seq,alph);
default: fprintf(stderr,"Internal error in get_mot_comp_generic\n"); exit(1);
}
}



int get_forme_canonique_comp(unichar contenu[],int *pos,unichar mot[],int *pos_seq)
{
   int i;

   i=0;
   do
   {
     mot[i] = contenu[*pos];
     (*pos)++;
     i++;
   }while((contenu[*pos] != '>') && (contenu[*pos] != '\0') && (i < N_CAR_MAX_COMP));
   if(i >= N_CAR_MAX_COMP) return -3;
   if((contenu[*pos] == '\0')) {
     fprintf(stderr,"WARNING : canonical form should be ended by >\n");
   }
   mot[i] = '>';

    /* $CD$ begin */
    if (contenu[(*pos)+1] == '>') {
        mot[++i] = contenu[++(*pos)];
        }
    else if ( (u_strlen(contenu)-1 - (*pos)) > 2 &&
              contenu[(*pos)+1] == '<' && contenu[(*pos)+2] == '<'
            ) {
            do {
                mot[++i] = contenu[++(*pos)];
                } while (contenu[*pos] != '>' && contenu[*pos] != '\0');
            if (contenu[*pos] == '\0' || contenu[(*pos)+1] == '\0' || contenu[(*pos)+1] != '>')
                return -3;
            mot[++i] = contenu[++(*pos)];
            }

    if (contenu[(*pos)+1] == '_') {
        mot[++i] = contenu[++(*pos)];
        do {
            mot[++i] = contenu[++(*pos)];
            } while (contenu[*pos] != '_' && contenu[*pos] != '\0');
        }
    /* $CD$ end   */

   mot[i+1] = '\0';
   (*pos)++;
   (*pos_seq)++;
   return 1;

}




int get_sequence_desambiguisee_comp(unichar contenu[],int *pos,unichar mot[],int *pos_seq)
{
   int i;

   i=0;
   do
   {
     mot[i] = contenu[*pos];
     (*pos)++;
     i++;
   }while((contenu[*pos] != '}') && (contenu[*pos] != '\0') && (i < N_CAR_MAX_COMP));
   if(i >= N_CAR_MAX_COMP) return -3;
   if(contenu[*pos] == '\0') {
     fprintf(stderr,"WARNING : desambiguised form should be ended by }\n");
   }
   mot[i] = '}';
   mot[i+1] = '\0';
   if (!u_strcmp_char(mot,"{STOP}")) {
      // if the graph contains the forbidden tag {STOP}, then
      // we raise a fatal error
      fatal_error("ERROR: a graph contains the forbidden tag {STOP}\n");
   }
   (*pos)++;
   (*pos_seq)++;
   return 1;
}






int lire_mot_entre_guillemet_comp(unichar contenu[],int *pos,unichar mot[],
                                  int *pos_seq,int mode,Alphabet* alph)
{


  unichar temp[N_CAR_MAX_COMP];

  mot[0]='\0';

    // cas du backslash
     if (contenu[*pos]=='\\') {
       (*pos)++;
       if (contenu[(*pos)]=='\\' && contenu[(*pos)+1]=='\\' && contenu[(*pos)+2]=='"') {
          // if we have $\\\"$ we must return the $"$ character
          (*pos)=(*pos)+2;
          get_caractere_comp(contenu,pos,mot,pos_seq);
          u_strcpy_char(temp,"@");
          u_strcat(temp,mot);
          u_strcpy(mot,temp);
          return 0;
       }
       else if (contenu[*pos] == '"') {
               // if we have $\"$ in the grf, not preceeded by $\\$,
               // it means that we have the final $"$ of the sequence
               (*pos)++;
               return 1;
            }
       else {
          // if we have $\x$ we must return the $x$ character
          get_caractere_comp(contenu,pos,mot,pos_seq);
          u_strcpy_char(temp,"@");
          u_strcat(temp,mot);
          u_strcpy(mot,temp);
          return 0;
       }

     }
     //Cas d'une lettre
     if (is_letter_generic(contenu[*pos],mode,alph))
     {
       if(get_mot_comp_generic(contenu,pos,mot,pos_seq,mode,alph)== -3) return -3;
       u_strcpy_char(temp,"@");
       u_strcat(temp,mot);
       u_strcpy(mot,temp);
       return 0;
     }


       // cas general d'un caractere seul
       get_caractere_comp(contenu,pos,mot,pos_seq);
       u_strcpy_char(temp,"@");
       u_strcat(temp,mot);
       u_strcpy(mot,temp);
       return 0;

}




int traitement_guillemet_comp(unichar contenu[],int *pos,
                unichar sequence[TAILLE_SEQUENCE_COMP][N_CAR_MAX_COMP],
                int *pos_seq,int graphe_courant,
                int mode,Alphabet* alph)
{
  int fin;

  (*pos)++;
  fin = 0;
  while((fin == 0) && (contenu[*pos] != '\0') && ((*pos_seq) < TAILLE_SEQUENCE_COMP))
  {
    fin = lire_mot_entre_guillemet_comp(contenu,pos,sequence[*pos_seq],pos_seq,mode,alph);
  }

  if(fin == -3) return -3;
  if((*pos_seq) >= TAILLE_SEQUENCE_COMP)
    return -1;
  return 0;
}




int get_sous_graphe_comp(unichar contenu[],int *pos,unichar mot[],int *pos_seq)
{
  int i = 0;
  while((contenu[*pos] != '+') && (contenu[*pos] != '\0')&& (i < N_CAR_MAX_COMP))
  {
    // if(contenu[*pos] == '\\') (*pos)++;
    /* the backslash shouldn't be used an escape character in graph
       names; under Unixes it is now obsolete: using the colon ':' as
       path separator, you don't need the construct '\/'. */
    mot[i]=contenu[*pos];
    // on normalise les separateurs de repertoires en ':'
    if (mot[i] == '/' || mot[i] == '\\') { mot[i] = ':'; }
    i++;
    (*pos)++;
  }
  if(i >= N_CAR_MAX_COMP) return 0;
  mot[i] = '\0';
  //i_fprintf(stderr, "mot=%S\n", mot);
  (*pos_seq)++;
  return 1;
}




//
// lit un mot et retourne 0 ou 1 si OK, 1 si PLUS, -1 si sequence trop longue,
// -2 si trop de graphes, -3 si mot trop long
int lire_mot_comp(unichar contenu[],int* pos,
                  unichar sequence[TAILLE_SEQUENCE_COMP][N_CAR_MAX_COMP],
                  int *pos_seq,int graphe_courant,
                  int mode,Alphabet* alph)
{
  int indice;

  sequence[*pos_seq][0]='\0';

  //Sous-graphe
  if (contenu[*pos]==':')
    {

      int l;
      unichar * seq = sequence[*pos_seq];
      seq[0] = ':';

      if (contenu[(*pos) + 1] != ':')
        { //cas d'un sous-graphe dans repertoire courant ou un de ses sous-repertoires

        // on recopie le chemin  relatif du graphe courant
        u_strcpy(seq + 1, donnees->nom_graphe[graphe_courant]);

        // on supprime le nom du graphe courant
        l = u_strlen(seq);
        // while ((l >= 0) && (seq[l] != '/') && (seq[l] != '\\')) { l--; }
        /* '/' and '\\' are already replaced by ':' in
           donnees->nom_graphe[graphe_courant] (see
           get_sous_graph_comp) */
        while (seq[l] != ':') { l--; } // l now >= 1

        }

#ifndef _NOT_UNDER_WINDOWS
      else if ( test4abs_windows_path_name((contenu+(*pos))) )
        {
          l = 0;
        }
#endif

      else
        { //cas d'un graphe se trouvant dans le repertoire <pckg_path> ou un de ses sous-repertoires
        l = 0;
        /* we keep the (second) colon in front of the graph name,
           i.e. we don't change the name. This indicates, that the path of
           the main graph won't be prefixed in
           conformer_nom_graphe_comp. */

        }

      // et on le remplace par l'appel au sous graphe
      if (get_sous_graphe_comp(contenu,pos, seq + l, pos_seq) == 0) return -3;

      //  i_fprintf(stderr, "lire_mot_comp: subgraph=%S\n", seq);

      indice = ajouter_graphe_comp(seq + 1,rac_graphe_comp,0); // + 1 pour omettre le ':'
      if (indice >= NOMBRE_GRAPHES_COMP)
        {
           fprintf(stderr,
                   "ERROR at top level: Too many graphes. "
                   "The number of graphs should be lower than %d\n",
                   NOMBRE_GRAPHES_COMP);
          return -2;
        }
      if (donnees->statut_graphe[indice] == -1)
        {
          u_strcpy(donnees->nom_graphe[indice], seq + 1);
        }

      return 0;
    }

  // le +
  if (contenu[*pos]=='+')
    {
      (*pos)++;
      return 1;
    }

  //espace
  if (contenu[*pos]==' ')
    {
      (*pos)++;
      return 0;
    }

  //Back-slash
  if (contenu[*pos]=='\\')
    {
      (*pos)++;
      if(contenu[*pos]!='\\')
        {
          if(contenu[*pos] == '"') // Cas d'une sequence entre guillemet
            return traitement_guillemet_comp(contenu,pos,sequence,pos_seq,graphe_courant,mode,alph);
          get_caractere_comp(contenu,pos,sequence[*pos_seq],pos_seq);
          return 0;
        }
      if(/*(contenu[(*pos)+1] != '\0')&& */(contenu[(*pos)+1] != '\\')) // caractere despecialise
        {
          get_caractere_comp(contenu,pos,sequence[*pos_seq],pos_seq);
          return 0;
        }
      if(contenu[(*pos)+2] == '"')   //caractere " seul
        {
          (*pos) += 2;
          get_caractere_comp(contenu,pos,sequence[*pos_seq],pos_seq);
          return 0;
        }
      get_caractere_comp(contenu,pos,sequence[*pos_seq],pos_seq);
      return 0;
    }



  //Cas des sequences desambiguises par Unitex au preprocessing
  if (contenu[*pos]=='{')
    {
      if(get_sequence_desambiguisee_comp(contenu,pos,sequence[*pos_seq],pos_seq)== -3) return -3;
      return 0;
    }

  // cas d'un debut de <...>
  if(contenu[*pos] == '<')
    {
      if(get_forme_canonique_comp(contenu,pos,sequence[*pos_seq],pos_seq) == -3) return -3;
      return 0;
    }

  // cas d'une lettre
  if (is_letter_generic(contenu[*pos],mode,alph)) {
    if(get_mot_comp_generic(contenu,pos,sequence[*pos_seq],pos_seq,mode,alph) == -3) return -3;
    return 0;
  }


  // par defaut
  get_caractere_comp(contenu,pos,sequence[*pos_seq],pos_seq);
  return 0;
}


int traitement_etiquettes_comp(int *indice,unichar mot[])
{
/*  unichar NB[10];
  u_strcpy_char(NB,"<NB>");

  if(u_strcmp(mot,NB) == 0) //CAS GRAPHE SPECIAL <NB>
  {
    (*indice) = ajouter_graphe_comp(NB,rac_graphe_comp,0);
    if((*indice) >= NOMBRE_GRAPHES_COMP)
    {
      fprintf(stderr,"ERROR at top level: Too many graphs (maximum %d)\n",NOMBRE_GRAPHES_COMP);
      return -1;
    }
    if(donnees->statut_graphe[(*indice)] == -1) u_strcpy(donnees->nom_graphe[(*indice)],NB);
    (*indice) = - (*indice) -1;
  }
  else*/    //CAS NORMAL
  {
    if(((*indice) = ajouter_etiquette_comp(mot,rac_comp,0)) >= NOMBRE_ETIQUETTES_COMP)
    {
      fprintf(stderr,"ERROR at top level: Too many tags (maximum %d)\n",NOMBRE_ETIQUETTES_COMP);
      return -1;
      }
      u_strcpy(donnees->Etiquette_comp[(*indice)],mot);
    }

  return 1;
}



//Transfo de la sequence en sequence d'entiers (negatif si graphe ou <NB>, positif si etiquette normale

int transfo_seq_carac_en_entiers(unichar sequence[TAILLE_SEQUENCE_COMP][N_CAR_MAX_COMP],unichar transduction[],int compteur_mots,int sequence_ent[],int *trans)
{
 int i,j,indice;
 unichar s[100];

 (*trans) = 0;
  j=0; i=0;
  if (transduction[0]!='\0') {  //CAS DE LA TRANSDUCTION
       j = 1;
       u_strcpy_char(s,"<NB>");
       if(((sequence[0][0] == ':') && (sequence[0][1] != '\0'))/* || (u_strcmp(sequence[0],s) == 0)*/)
       //CAS DU SOUS-GRAPHE OU <NB>
       {
        /* strcpy(temp,"<E>/");
         strcat(temp,transduction);
         if(traitement_etiquettes_comp(&indice,temp) == -1) return -1;*/
         //sequence_ent[0] = indice;
         //i = 0;
         //(*trans) = 1;
       }
       else
       {
          u_strcpy_char(s,"/");
          u_strcat(sequence[0],s);
          u_strcat(sequence[0],transduction);

          if(traitement_etiquettes_comp(&indice,sequence[0]) == -1) return -1;
          sequence_ent[0] = indice;
          i = 1;
       }
  }

  while(i < compteur_mots)
  {
     if((sequence[i][0] == ':') && (sequence[i][1] != '\0'))  //SOUS GRAPHE
     {
       /* inutile
        k = 1;
        do  // transfo :grf1 en grf1
          temp[k -1] = sequence[i][k++];
        while(sequence[i][k] != '\0');
        temp[k -1] = '\0';
        */
        indice = ajouter_graphe_comp(sequence[i] + 1,rac_graphe_comp,0);
        indice = - indice -1;
        sequence_ent[j] = indice;
     }
     else
     {
        u_strcpy_char(s,"/<E>");

        if(transduction[0]!='\0') u_strcat(sequence[i],s);
        if(traitement_etiquettes_comp(&indice,sequence[i]) == -1) return -1;
        sequence_ent[j] = indice;
     }
  i++;
  j++;
  }
  return 1;
}



int ecriture_transition_comp(Etat_comp letats[],int seq_ent[],int sortants[],int *n_et,int etat_courant,int longueur,int graphe_courant,int trans)
{
  int k,j;
  int dep;
  k=0;

   while(sortants[k] != -1)
   {
     dep=etat_courant;
     for(j=0;j<longueur -1+trans;j++)
     {
       if(ajouter_etat_deliage_comp(letats,dep,seq_ent[j],n_et,graphe_courant) == 0) return 0;
       dep=(*n_et)-1;
     }
     ajouter_transition_comp(letats,dep,sortants[k],seq_ent[longueur +trans -1]);
     k++;
   }

return 1;
}




int traitement_transition_comp(Etat_comp letats[],unichar contenu[],
                               unichar transduction[],int sortants[],
                               int *n_et, int *pos, int etat_courant,
                               int graphe_courant,
                               int mode,Alphabet* alph)
{
  int compteur_mots = 0;
  int plus = 0;
  int trans;
  unichar sequence[TAILLE_SEQUENCE_COMP][N_CAR_MAX_COMP];
  int sequence_ent[TAILLE_SEQUENCE_COMP];
  char err[1000];
  while((plus == 0) && (contenu[*pos] != '\0') && (compteur_mots < TAILLE_SEQUENCE_COMP)) {
    plus = lire_mot_comp(contenu,pos,sequence,&compteur_mots,graphe_courant,mode,alph);
  }

  if (plus == -2) return -1;
  if(plus == -3) {       // Cas trop de caractères dans un mot
   u_to_char(err,donnees->nom_graphe[graphe_courant]);
   if(graphe_courant == 0) {
      fprintf(stderr,"ERROR in main graph %s: The size of a word should be lower than %d characters\n",err,N_CAR_MAX_COMP);
    }
    else {
      fprintf(stderr,"WARNING in main graph %s: The size of a word should be lower than %d characters\nGraph has been emptied\n",err,N_CAR_MAX_COMP);
    }
    return 0;
  }
  if((plus == -1) || (compteur_mots >= TAILLE_SEQUENCE_COMP) )
  {
    u_to_char(err,donnees->nom_graphe[graphe_courant]);
    if(graphe_courant == 0) {
      fprintf(stderr,"ERROR in main graph %s: The size of a sequence between two + should be lower than %d words\n",err,TAILLE_SEQUENCE_COMP);
    }
    else {
      fprintf(stderr,"WARNING in main graph %s: The size of a sequence between two + should be lower than %d words\nGraph has been emptied\n",err,TAILLE_SEQUENCE_COMP);
    }
    return 0;
  }
  if (transfo_seq_carac_en_entiers(sequence,transduction,compteur_mots,sequence_ent,&trans) == -1) return -1;
  if (ecriture_transition_comp(letats,sequence_ent,sortants,n_et,etat_courant,compteur_mots,graphe_courant,trans) == 0) return 0;
  return 1;
 }



void traiter_debut_fin_variable(Etat_comp letats[],unichar contenu[],unichar transduction[],int sortants[], int *n_et, int *pos, int etat_courant,int graphe_courant) {
  unichar sequence[1][N_CAR_MAX_COMP];
  int sequence_ent[1];
  int trans;
  u_strcpy(sequence[0],contenu);
  if (transfo_seq_carac_en_entiers(sequence,transduction,1,sequence_ent,&trans) == -1) return;
  if (ecriture_transition_comp(letats,sequence_ent,sortants,n_et,etat_courant,1,graphe_courant,trans) == 0) return;
}



void traiter_context_mark(Etat_comp letats[],unichar contenu[],unichar transduction[],int sortants[], int *n_et, int *pos, int etat_courant,int graphe_courant) {
  unichar sequence[1][N_CAR_MAX_COMP];
  int sequence_ent[1];
  int trans;
  u_strcpy(sequence[0],contenu);
  if (transfo_seq_carac_en_entiers(sequence,transduction,1,sequence_ent,&trans) == -1) return;
  if (ecriture_transition_comp(letats,sequence_ent,sortants,n_et,etat_courant,1,graphe_courant,trans) == 0) return;
}





int traitement_ligne_comp(unichar ligne[],int sortants[],
                          Etat_comp letats[],int etat_courant,
                          int *n_et, int graphe_courant,
                          int mode,Alphabet* alph)
{
  unichar contenu[TAILLE_MOT_GRAND_COMP];
  unichar transduction[TAILLE_MOT_GRAND_COMP];
  int i;
  int res;

  if(sortants[0]!=-1)
  {
    i=0;
    if ((u_strlen(ligne)>2 && ligne[0]=='$' && (ligne[u_strlen(ligne)-1]=='(' || ligne[u_strlen(ligne)-1]==')'))
        // context marks are handled exactly as variable marks
        || (!u_strcmp_char(ligne,"$[") || !u_strcmp_char(ligne,"$![")
            || !u_strcmp_char(ligne,"$]"))) {
        u_strcpy(contenu,ligne);
        u_strcpy_char(transduction,"");
        traiter_debut_fin_variable(letats,contenu,transduction,sortants,n_et,&i,etat_courant,graphe_courant);
        return 1;
    }
    traitement_transduction_comp(ligne,contenu,transduction);
    while(contenu[i]!='\0')
    {
      res = traitement_transition_comp(letats,contenu,transduction,sortants,n_et,&i,etat_courant,graphe_courant,mode,alph);
      if((res == -1) || (res == 0)) return res;
    }
  }
  return 1;
}


 //lit le contenu d'une boite
//retourne 0 si erreur
//retourne 1 si OK
//retourne 2 si pas de transitions sortants de la boite

int lire_ligne_comp(FILE *f, unichar ligne[],int sortants[],Etat_comp *letats,int courant)
{
  unichar c;
  int i,n_sortantes;
  char err[1000];

  for(i=0;i<NOMBRE_TRANSITIONS_COMP;i++) sortants[i]=-1;

  i=0;
  while (u_fgetc(f)!='"');
  while (((c=(unichar)u_fgetc(f))!='"') && (i < TAILLE_MOT_GRAND_COMP))
    {
      ligne[i]=c;
      if ((ligne[i]=='\\') && (i < TAILLE_MOT_GRAND_COMP))
	{
	  i++;
	  ligne[i]=(unichar)u_fgetc(f);
	}
      i++;
    }
  if(i >= TAILLE_MOT_GRAND_COMP)
  {
    u_to_char(err,donnees->nom_graphe[courant]);
    if(courant == 0) {
      fprintf(stderr,
              "ERROR in main graph %s.grf: Too many characters in box. The number of characters per box should be lower than %d\n",
              err,TAILLE_MOT_GRAND_COMP);
    }
    else {
      fprintf(stderr,"WARNING in graph %s.grf: Too many characters in box. The number of characters per box should be lower than %d\n",err,TAILLE_MOT_GRAND_COMP);
    }
    return 0;
  }
  ligne[i]='\0';
  // we read the space char after the string
  u_fgetc(f);
  u_read_int(f);
  u_read_int(f);
  n_sortantes=u_read_int(f);

  if(n_sortantes >= NOMBRE_TRANSITIONS_COMP)
  {
    u_to_char(err,donnees->nom_graphe[courant]);
    if(courant == 0) {
      fprintf(stderr,"ERROR in main graph %s.grf: Too many transitions. The number of transitions per box should be lower than %d\n",err,NOMBRE_TRANSITIONS_COMP);
    }
    else {
      fprintf(stderr,"WARNING in graph %s.grf: Too many transitions. The number of transitions per box should be lower than %d\n",err,NOMBRE_TRANSITIONS_COMP);
    }
    return 0;
  }
  for (i = 0 ; i < n_sortantes ; i++)
    {
      sortants[i]=u_read_int(f);
    }
  // here we read the end of line char
  u_fgetc(f);
  if(n_sortantes == 0) return 2;
  return 1;
}





int traitement_graphe_special(int courant)
{
  unichar temp[3];
  int indice;
  unichar i;
  char s[1000];

  donnees->statut_graphe[courant]=1;
  courant = - courant - 1;

  sprintf(s,"%d <NB>\n:",courant);
  u_fprints_char(s,fs_comp);
  for(i='0';i<='9';i++)
    {
      temp[0]=i;
      temp[1]='\0';
      if((indice = ajouter_etiquette_comp(temp,rac_comp,0)) >= NOMBRE_ETIQUETTES_COMP)
	{
	  fprintf(stderr,"ERROR at top level: Too many tags (maximum %d)\n",NOMBRE_ETIQUETTES_COMP);
	  return 0;
	}
      u_strcpy(donnees->Etiquette_comp[indice],temp);
      sprintf(s," %d 1",indice);
      u_fprints_char(s,fs_comp);
    }
  unichar diese[10];
  u_strcpy_char(diese,"#");

  if((indice = ajouter_etiquette_comp(diese,rac_comp,0)) >= NOMBRE_ETIQUETTES_COMP)
    {
      fprintf(stderr,"ERROR at top level: Too many tags (maximum %d)\n",NOMBRE_ETIQUETTES_COMP);
      return 0;
    }
  u_strcpy(donnees->Etiquette_comp[indice],diese);
  sprintf(s," \nt %d 0 \nf \n",indice);
  u_fprints_char(s,fs_comp);
  return 1;
}


//////////////////////////////////////////////////////////
///////////////// COMPILER UN GRAPHE /////////////////////
//////////////////////////////////////////////////////////

int compiler_graphe_comp(int graphe_courant,int mode,Alphabet* alph)
{
   int i,det;
   //int courant;
   int traitement,lire;
   int n_etats_initial, n_etats_final;
   FILE *f;
   char nom[TAILLE_MOT_GRAND_COMP];
   Etat_comp letats[NOMBRE_ETATS_COMP];
   int sortants[NOMBRE_TRANSITIONS_COMP];
   unichar ligne[TAILLE_MOT_GRAND_COMP];
   char err[1000];
   /*unichar NB[10];
   u_strcpy_char(NB,"<NB>");
   if (u_strcmp(donnees->nom_graphe[graphe_courant],NB) == 0)
   {
     courant = ajouter_graphe_comp(NB,rac_graphe_comp,0);
     if(traitement_graphe_special(courant) == -1) return -1;
     return 1;
   }*/

   conformer_nom_graphe_comp(nom,graphe_courant); // DATE -> C:/Unitex/French/Graphs/Date/DATE.grf
   printf("Compiling graph ");
   u_prints(donnees->nom_graphe[graphe_courant]);
   printf("\n");
   // fprintf(stdout,"(%s)\n",nom);

   f=u_fopen(nom,U_READ);
   if(f == NULL)      // Ouverture du fichier
   {
     char s[1000];
     u_to_char(s,donnees->nom_graphe[graphe_courant]);
     fprintf(stderr,"Cannot open the graph %s.grf\n",s);
     fprintf(stderr,"(%s)\n",nom);
     sauvegarder_graphe_comp(letats,0,graphe_courant);
     donnees->statut_graphe[graphe_courant] = 0;
     if(graphe_courant == 0) return 0;
     return 1;
   }

   u_fgetc(f);
   while(u_fgetc(f)!='#');
   u_fgetc(f);
   n_etats_initial=u_read_int(f);

   if (n_etats_initial > NOMBRE_ETATS_COMP) //Trop de boites dans graphe
    {
      donnees->statut_graphe[graphe_courant] = 0;
      sauvegarder_graphe_comp(letats,0,graphe_courant);
      u_fclose(f);
      u_to_char(err,donnees->nom_graphe[graphe_courant]);
      if(graphe_courant == 0)
      {
        fprintf(stderr,"ERROR in main graph %s.grf: Too many boxes (%d). The number of boxes should be lower than %d\n",err,n_etats_initial,NOMBRE_ETATS_COMP);
        return 0;
      }
      fprintf(stderr,"WARNING in graph %s.grf: Too many boxes (%d). The number of boxes should be lower than %d\n",err,n_etats_initial,NOMBRE_ETATS_COMP);
      return 1;
    }

  init_locale_comp(letats, n_etats_initial);
  n_etats_final=n_etats_initial;
  for (i=0;i<n_etats_initial;i++)
    {
      //On lit le contenu d'une boite et ses sorties (transitions)
      lire = lire_ligne_comp(f,ligne,sortants,letats,graphe_courant);
      if (lire == 0)
      {
        donnees->statut_graphe[graphe_courant] = 0;
        sauvegarder_graphe_comp(letats,0,graphe_courant);
        liberer_graphe_comp(letats);
        u_fclose(f);
        if(graphe_courant == 0)
          return 0;
        return 1;
      }
      //On traite la ligne : ecriture en memoire
      if(lire == 1)
      {
        traitement = traitement_ligne_comp(ligne,sortants,letats,i,&n_etats_final,graphe_courant,mode,alph);
        if((traitement == 0) || (traitement == -1))
        {
          donnees->statut_graphe[graphe_courant] = 0;
          sauvegarder_graphe_comp(letats,0,graphe_courant);
          liberer_graphe_comp(letats);
          u_fclose(f);
          if(traitement == -1) return -1;
          if(graphe_courant == 0) return 0;
          return 1;
        }
        if (n_etats_final >= NOMBRE_ETATS_COMP) //Trop d'états dans l'automate
        {
          donnees->statut_graphe[graphe_courant] = 0;
          sauvegarder_graphe_comp(letats,0,graphe_courant);
          liberer_graphe_comp(letats);
          u_fclose(f);
          u_to_char(err,donnees->nom_graphe[graphe_courant]);
          if(graphe_courant == 0)
          {
            fprintf(stderr,"ERROR in main graph %s.grf: Too many states (%d)\n",err,n_etats_final);
            return 0;
          }
          fprintf(stderr,"WARNING in graph %s.grf: Too many states (%d)\n",err,n_etats_final);
          return 1;
        }
      }
    }
  fflush(f);
  u_fclose(f);

  //etat initial: bit 2 a 1
  letats[0]->controle=(unsigned char)((letats[0]->controle)|2);
  //etat final: bit 1 a 1
  letats[1]->controle=(unsigned char)((letats[1]->controle)|1);
  

  // ELAGAGE, SUPPRESSION DES EPSILONS TRANSITIONS
  co_accessibilite_comp(letats,1);  
  virer_epsilon_transitions_comp(letats,n_etats_final);
  accessibilite_comp(letats,0);
  eliminer_etats_comp(letats,&n_etats_final);
  if(letats[0] == NULL)
  {
    donnees->statut_graphe[graphe_courant] = 0;
    sauvegarder_graphe_comp(letats,0,graphe_courant);
    liberer_graphe_comp(letats);
    u_to_char(err,donnees->nom_graphe[graphe_courant]);
    if(graphe_courant == 0)
        {
          fprintf(stderr,"ERROR: Main graph %s.grf has been emptied\n",err);
          return 0;
        }
        fprintf(stderr,"WARNING: graph %s.grf has been emptied\n",err);
        return 1;
   }
  donnees->statut_graphe[graphe_courant] = 1;
  char s[1000];
  sprintf(s,"%d ",-(graphe_courant)-1);
  u_fprints_char(s,fs_comp);
  u_fprints(donnees->nom_graphe[graphe_courant],fs_comp);
  u_fputc('\n',fs_comp);
  det = determinisation(letats);
  //det = 1;
  liberer_graphe_comp(letats);
  if(det == 0)
  {
    u_to_char(err,donnees->nom_graphe[graphe_courant]);
    if(graphe_courant == 0)
    {
      fprintf(stderr,"ERROR in main graph %s.grf: Tore error. Please, contact Unitex programmers\n",err);
      return 0;
    }
    fprintf(stderr,"WARNING in graph %s.grf: Tore error. Please, contact Unitex programmers\n",err);
    return 0;
  }
  return 1;
}


// Extraction du nom du graphe et du chemin des graphes
// ex: E:/Unitex/French/date.grf -> date   et  E:/Unitex/French/

int extraire_nom_graphe_comp(char *s1,unichar* S2)
{
  char s2[TAILLE_MOT_GRAND_COMP];
  char temp[TAILLE_MOT_GRAND_COMP];
  int l,i,j;

  strcpy(temp,s1);
  l=strlen(temp);
  i=l-5;
  temp[i+1]='\0'; //On supprime le .grf

  while(i>=0 && temp[i]!='/' && temp[i]!='\\')i--;

  if (
#ifdef _NOT_UNDER_WINDOWS
      temp[i]=='/'
#else
      temp[i]=='\\'
#endif
      ) {
    // if we have an absolute path, we care for the slash or backslash
    for(j=i+1;j<l;j++) s2[j-i-1]=temp[j];
    for(j=0;j<i+1;j++) donnees->chemin_graphe_principal[j]=temp[j];
    donnees->chemin_graphe_principal[i+1]='\0';
    u_strcpy_char(S2,s2);
    return 1;
  }
  else {
    // if there is no path
    // we don't need to modify S2
    // and we put an an empty path
    donnees->chemin_graphe_principal[0]='\0';
    u_strcpy_char(S2,temp);
    return 1;
  }
}


/////////////////////////////////////////////////
///////// COMPILATION GENERALE //////////////////
////////////////////////////////////////////////

int compilation(char *nom_graphe_principal,int mode,Alphabet* alph)
{
 int compteur=0;
 int comp;

  if(extraire_nom_graphe_comp(nom_graphe_principal,donnees->nom_graphe[0]) == 0) return 0;
  ajouter_graphe_comp(donnees->nom_graphe[compteur],rac_graphe_comp,0);
  do
  {
   comp = compiler_graphe_comp(compteur,mode,alph);
   if((comp == 0) && (compteur == 0)) return 0;
   if(comp == -1) return 0;
   compteur++;
  }
  while((nombre_graphes_comp < NOMBRE_GRAPHES_COMP) && (compteur < nombre_graphes_comp));

  if(nombre_graphes_comp >= NOMBRE_GRAPHES_COMP)
  {
    fprintf(stderr,"There are too many graphs. The number of graphs should be lower than %d\n",NOMBRE_GRAPHES_COMP);
    return 0;
  }

  return 1;
}

//Initialisation generale

void init_generale_comp()
{
  int i;

  nombre_graphes_comp=0;
  nombre_etiquettes_comp=0;

  for(i=0;i<NOMBRE_GRAPHES_COMP;i++)
    {
      donnees->statut_graphe[i]=-1;
      donnees->nom_graphe[i][0]='\0';
    }
    nombre_graphes_comp=0;
}

int ouverture_fichier_sortie(char temp[])
{
  char temp1[TAILLE_MOT_GRAND_COMP];
  int l;

  strcpy(temp1,temp);
  l = strlen(temp1);
  temp1[l-4] = '\0';
  strcat(temp1,".fst2");
  fs_comp = u_fopen(temp1,U_WRITE);
  if(fs_comp == NULL) return 0;
  return 1;
}


void ecrire_fichier_sortie_nb_graphes(char name[])
{
  FILE *f;
  int i,n;
  i=2+9*2; // *2 because of unicode +2 because of FF FE at file start
  n=nombre_graphes_comp;
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


//
//Met dans fichier de sortie la liste des etiquettes
//
void sauvegarder_etiquettes_comp()
{
  int i;

  for(i=0;i<nombre_etiquettes_comp;i++) {
    if ((donnees->Etiquette_comp[i][0])=='@') {
      if ((donnees->Etiquette_comp[i][1])=='\0') {
        u_fprints_char("%%@\n",fs_comp);
        }
      else {
         u_fprints(donnees->Etiquette_comp[i],fs_comp);
         u_fputc('\n',fs_comp);
      }
    }
    else {
      u_fputc('%',fs_comp);
      u_fprints(donnees->Etiquette_comp[i],fs_comp);
      u_fputc('\n',fs_comp);
    }
  }
  u_fprints_char("f\n",fs_comp);
}



