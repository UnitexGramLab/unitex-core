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
                               MAX_FST2_TAGS */
struct noeud_g_comp *rac_graphe_comp; //racine de l'arbre des graphes
struct noeud_comp *rac_comp; //racine de l'arbre des étiquettes
int EPSILON_comp;   // etiquette pour <E>
int compteur_char=0;
int compteur_free_char=0;


// to clean up??: also defined in Fst2.cpp for Fst2State
#define FST2_FINAL_STATE_BIT_MASK 1
#define FST2_INITIAL_STATE_BIT_MASK 2
int is_final_state (Etat_comp e) {
  return e->controle & FST2_FINAL_STATE_BIT_MASK;
}
int is_final_state (Etat_fst_det e) {
  return e->controle & FST2_FINAL_STATE_BIT_MASK;
}
void set_final_state(Etat_comp e,int finality) {
  if (e==NULL) {
    fatal_error("NULL error in set_final_state\n");
  }
  /* First the compute the controle byte without the finality bit */
  e->controle=e->controle & (0xFF-FST2_FINAL_STATE_BIT_MASK);
  /* And we add it if necessary*/
  if (finality) {
    e->controle=e->controle | FST2_FINAL_STATE_BIT_MASK;
  }
}
// end to clean up


/**
 *  renvoie un Etat vierge
 */
Etat_comp nouvel_etat_comp()
{
  Etat_comp e;

  e = (Etat_comp)malloc(sizeof(struct etat_comp));
  e->controle = 0;
  e->trans = NULL;
  e->transinv = NULL;
  return e;
}

void liberer_etat_comp(Etat_comp etat)
{
  if (etat!=NULL)
  {
    free_Fst2Transition(etat->trans);
    free_Fst2Transition(etat->transinv);
    free(etat);
  }

}

/**
 * Creates and adds a transition to the given state.
 */
void add_transition_to_etat_comp(Etat_comp state, int tag_number, int state_number) {
  Fst2Transition transition = new_Fst2Transition();
  transition->tag_number = tag_number;
  transition->state_number = state_number;
  transition->next = state->trans;
  state->trans = transition;
}


/**
 * creates a new graph_comp object
 */
Graph_comp new_graph_comp() {
  Graph_comp g
    = (Graph_comp) malloc(sizeof(struct graph_comp));
  if ( g == NULL )
    fatal_error("Not enough memory at %s, line %s\n",__FILE__,__LINE__);
  g->n_states = 0;
  g->size = 0;
  g->states = NULL;
  resize_graph_comp(g);
  return g;
}


/**
 * resize a new graph_comp object (malloc room for more states)
 */
bool resize_graph_comp(Graph_comp g) {
  return resize_graph_comp(g,0x2000);
}
/**
 * resize a new graph_comp object by n states
 */
bool resize_graph_comp(Graph_comp g, int n) {
  n += g->size;
  return resize_graph_comp_to(g,n);
}
/**
 * resize a new graph_comp object to n states
 */
bool resize_graph_comp_to(Graph_comp g, int n) {
  Etat_comp* gg = (Etat_comp*) realloc(g->states,
                                        (n*sizeof(Etat_comp)));
  if ( gg == NULL )
    {
      error("Not enough memory at %s, line %s\n",__FILE__,__LINE__);
      return false;
    }
  else
    {
      g->size = n;
      g->states = gg;
      return true;
    }
}


/**
 * add a new state to a graph_comp object
 */
Etat_comp add_state(Graph_comp g) {
  Etat_comp e = nouvel_etat_comp();
  if ( g->size < (1+g->n_states) )
    resize_graph_comp(g);
  g->states[g->n_states] = e;
  (g->n_states)++;
  return e;
}
/**
 * add a new state to a graph_comp object, write state number to n
 */
Etat_comp add_state(Graph_comp g, int* n) {
  *n = g->n_states;
  return add_state(g);
}

/**
 * Move a graph_comp src to graph_comp dest.
 *
 * dest is free'd before copying content of src to it,
 * src is free'd after.
 */
void move_graph_comp(Graph_comp dest, Graph_comp src) {
  /* free dest */
  if ( dest->states != NULL )
    {
      for (register int i=0; i < dest->n_states; i++)
        liberer_etat_comp(dest->states[i]);
      free(dest->states);
    }
  /* copy src -> dest */
  dest->size = src->size;
  dest->n_states = src->n_states;
  dest->states = src->states;
  /* free src */
  src->states = NULL; /* make sure that the states array isn't accidentally free'd */
  free(src);
}


/**
 * frees a graph_comp object
 */
void free_graph_comp(Graph_comp g) {
  if (g == NULL) return;
  for (register int i=0; i < g->n_states; i++)
    liberer_etat_comp(g->states[i]);
  free(g->states);
  free(g);
}


/* structure of linked list added for representing sets for epsilon closure */

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

void util_error(char *function,char *message) {
fatal_error("%s: %s\n",function,message);
}

void* ll_i_malloc(int n) {
ll_i_n_malloc++;
return (void *)malloc(n);
}

void ll_i_pfree(void *ptr) {
  if (ptr == NULL) return;
  ll_i_n_free++;
  free(ptr);
  ptr = NULL;
}

struct ll_i_cell_comp* ll_i_create_cell(unsigned int v) {
  struct ll_i_cell_comp *ptr = (struct ll_i_cell_comp*)ll_i_malloc(sizeof(struct ll_i_cell_comp));
  ptr->value = v;
  ptr->next = NULL;
  return ptr;
}

Llist_i_comp ll_i_initialize() {
  Llist_i_comp ptr = (Llist_i_comp)ll_i_malloc(sizeof(struct linked_list_i_comp));
  ptr->head = NULL;
  ptr->tail = NULL;
  return ptr;
}

void ll_i_insert_at_tail(Llist_i_comp *l,unsigned int v) {
  struct ll_i_cell_comp *ptr = ll_i_create_cell(v);

  if (ptr == NULL) util_error("ll_i_insert_at_tail","creating cell");
  if (*l == NULL) {
    *l = ll_i_initialize();
  }
  if (*l == NULL)  util_error("ll_i_insert_at_tail","llist initialization");
  if ((*l)->head == NULL) {
    (*l)->head = ptr;
    (*l)->tail = (*l)->head;
    return;
  }
  (*l)->tail->next = ptr;
  (*l)->tail = (*l)->tail->next;
}


int ll_i_cell_i_compare(struct ll_i_cell_comp *c1,struct ll_i_cell_comp *c2) {
  
  return (c1->value < c2->value);
}

int ll_i_cell_i_equals(struct ll_i_cell_comp *c1,struct ll_i_cell_comp *c2) {  
  return (c1->value == c2->value);
}

// No tail


// insert dans l'ordre et pas de doublons

void ll_i_insert_sorted(Llist_i_comp *l,unsigned int v) {
  struct ll_i_cell_comp* prev, *curr,*temp;
  int is_doublon = 0;

  temp = ll_i_create_cell(v);
  if (temp == NULL) util_error("ll_i_insert_sorted","creating cell");
  if (*l == NULL) {
    *l = ll_i_initialize();
  }
  if (*l == NULL) util_error("ll_i_insert_sorted","llist initialization");

  if ( (*l)->head == NULL ) {
    (*l)->head = temp;  
  }
  else{
    prev = NULL;
    curr = (*l)->head;
    
    /* traverse the list until the spot
       for insertion is found */
    is_doublon = 0;
    while (curr != NULL && ll_i_cell_i_compare(curr,temp)) {
      prev = curr;     
      curr = curr -> next;
    }    
    if (curr != NULL && ll_i_cell_i_equals(curr,temp)) is_doublon = 1;
    /* insert the node, temp */
    if (!is_doublon) {
      if (prev == NULL) {
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

struct ll_i_cell_comp* ll_i_find_element(Llist_i_comp l,unsigned int elem) {
  struct ll_i_cell_comp *ptr;
  if (l == NULL) return NULL;
  ptr = l->head;
  while (ptr != NULL) {
    if (ptr->value == elem) return ptr;
     ptr = ptr->next;
  }
  return NULL;
}

void ll_i_print(Llist_i_comp l) {
  struct ll_i_cell_comp *ptr;

  if (l == NULL) {
    printf("-- NULL LLIST --\n");
    return;
  }
  ptr = l->head;
  while (ptr != NULL) {
    printf("%d ",ptr->value);
    ptr = ptr->next;
  }
}

void ll_i_println(Llist_i_comp l) {
  ll_i_print(l);
  printf("\n");
}


void ll_i_free(Llist_i_comp *l) {
  struct ll_i_cell_comp *ptr;

  if (*l != NULL) {
    while ((*l)->head != NULL) {
      ptr = (*l)->head;
      (*l)->head = (*l)->head->next;
      ll_i_pfree(ptr);
    }
    ll_i_pfree(*l);
    *l = NULL;
  }
}


int ll_i_print_n_malloc() {
  printf("ll_i_malloc=%d\n",ll_i_n_malloc);
  return ll_i_n_malloc;
}

int ll_i_print_n_free() {
  printf("ll_i_free=%d\n",ll_i_n_free);
  return ll_i_n_free;
}



int ll_i_length(Llist_i_comp ll) {
  struct ll_i_cell_comp *ptr;
  int i = 0;
  if (ll == NULL) return 0;
  ptr = ll->head;
  while (ptr != NULL) {
    ptr = ptr->next;
    i++;
  }
  return i;
}

Llist_i_comp ll_i_copy(Llist_i_comp ll) {
  Llist_i_comp temp = NULL;
  struct ll_i_cell_comp *ptr;
  if (ll == NULL) return NULL;
  ptr = ll->head;
  while (ptr != NULL) {
    ll_i_insert_at_tail(&temp,ptr->value);
    ptr = ptr->next;
  }
 
  return temp;
}








int is_letter_generic(unichar c,int mode,Alphabet* alph) {
switch(mode) {
case DEFAULT_TOKENIZATION: return u_is_letter(c);
case CHAR_BY_CHAR_TOKENIZATION: return 0; // by convention
case ALPHABET_TOKENIZATION: return is_letter(c,alph);
default: fatal_error("Internal error in is_letter_generic\n");
}
return 0;
}




///////////////////////////////////////////////
// QUELQUES INITIALISATIONS                 //
//////////////////////////////////////////////

/**
 *  initialise le graphe
 */
void init_graphe_mat_det(Etat_fst_det resultat[]) {
register int i;
for (i=0;i<NBRE_ET;i++)
  resultat[i]=NULL;
}

///////////////////////////////////////////////////////////////
//////   GESTION DU BI-ARBRE DES ENSEMBLES  ///////////////////
/////////////////////////////////////////////////////////////


struct noeud_num_char_det* nouveau_noeud_num_char_det()
{
  struct noeud_num_char_det *n;
  n=(struct noeud_num_char_det*)malloc(sizeof(struct noeud_num_char_det));
  n->num_char=-1;
  n->liste=NULL;
  return n;
}


struct noeud_valeur_det* nouveau_noeud_valeur_det()
{
  struct noeud_valeur_det *n;
  n = (struct noeud_valeur_det*)malloc(sizeof(struct noeud_valeur_det));
  n->valeur = -1;
  n->indice = -1;
  n->liste = NULL;
  return n;
}



struct liste_branches_num_char_det* nouvelle_liste_branches_num_char_det()
{
  struct liste_branches_num_char_det *l;
  l=(struct liste_branches_num_char_det*)malloc(sizeof(struct liste_branches_num_char_det));
  l->n=NULL;
  l->suivant=NULL;
  return l;
}



struct liste_branches_valeur_det* nouvelle_liste_branches_valeur_det()
{
  struct liste_branches_valeur_det *l;
  l=(struct liste_branches_valeur_det*)malloc(sizeof(struct liste_branches_valeur_det));
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


/**
 *  cree et retourne un nouveau char_etats
 */
ensemble_det nouveau_char_etats_det()
{
  ensemble_det ce;
  ce = (ensemble_det) malloc(sizeof(struct char_etats_det));
  ce->suivant = NULL;
  ce->valeur = 0;
  ce->num_char = (unsigned int) UINT_MAX; /* UINT_MAX = 0xffff on 32-bit machines,
                                             0xffffffff on 64-bit: defined in
                                             "Limits.h" */
  return ce;
}



/**
 *  ajoute un nouvel etat a un ensemble d'etats
 */
void ajouter_etat_dans_ensemble_det(int netat,ensemble_det *e) {
  ensemble_det ptr,ptr1,pos;
  unsigned int numero=netat/INT_BITS;
   if (*e == NULL)
   {
     *e=nouveau_char_etats_det();
     (*e)->num_char=numero;
     (*e)->valeur=1<<(netat%INT_BITS);
     return;
   }
   ptr=(*e);
   if (ptr->num_char==numero)
   {
     ptr->valeur=(ptr->valeur)|(1<<(netat%INT_BITS));
     return;
   }
   if (ptr->num_char>numero)
   {
     ptr1=nouveau_char_etats_det();
     ptr1->num_char=numero;
     ptr1->valeur=1<<(netat%INT_BITS);
     (*e)=ptr1;
     (*e)->suivant=ptr;
     return;
   }
   if (ptr->suivant==NULL) {
       ptr->suivant=nouveau_char_etats_det();
       ptr=ptr->suivant;
       ptr->num_char=numero;
       ptr->valeur=1<<(netat%INT_BITS);
       return;
     }
   pos=ptr;
   ptr=ptr->suivant;


   while (ptr!=NULL) {
       if (ptr->num_char==numero)
       {
	   ptr->valeur=(ptr->valeur)|(1<<(netat%INT_BITS));
	   return;
	 }
       if (ptr->num_char>numero) {
	   ptr1=nouveau_char_etats_det();
	   ptr1->num_char=numero;
	   ptr1->valeur=1<<(netat%INT_BITS);
	   ptr1->suivant=ptr;
	   pos->suivant=ptr1;
	   return;
	 }
       if (ptr->suivant==NULL) {
	   ptr->suivant=nouveau_char_etats_det();
	   ptr=ptr->suivant;
	   ptr->num_char=numero;
	   ptr->valeur=1<<(netat%INT_BITS);
	   return;
	 }
       pos=ptr;
       ptr=ptr->suivant;
     }
}




/**
 *  liberer un char_etats
 */
void liberer_char_etat_det(ensemble_det ptr) {
   ensemble_det ptr1;
   while (ptr!=NULL)
   {
     ptr1=ptr;
     ptr=ptr->suivant;
     free(ptr1);
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
     free(old->n);
     free(old);
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
   free(old->n);
   free(old);
 }
}




void liberer_arbre_det(struct noeud_valeur_det *racine)
{
  vider_noeud_valeur_det(racine);
  free(racine);
}




/**
 *  cree et renvoie un etat vierge
 */
Etat_fst_det nouvel_etat_mat_det()
{
  Etat_fst_det e = (Etat_fst_det) malloc(sizeof(struct etat_fst_det));
  e->controle = 0;
  e->trans = NULL;
  e->ens = NULL;
  return e;
}



/**
 *  ajoute une transition a l'etat courant
 */
void ajouter_transition_mat_det(Etat_fst_det e,int etiq,int etarr)
{
  Fst2Transition ptr;

  ptr=new_Fst2Transition();
  ptr->next=e->trans;
  ptr->tag_number=etiq;
  ptr->state_number=etarr;
  e->trans=ptr;
}


void liberer_etat_det(Etat_fst_det e)
{
  liberer_char_etat_det(e->ens);
  free_Fst2Transition(e->trans);
  free(e);
}


/**
 * this function adds a reverse transition to a state
 */
void add_reverse_transition_to_state(int etiq,int dest,Etat_comp e) {
  Fst2Transition l = new_Fst2Transition();
  l->tag_number = etiq;
  l->state_number = dest;
  if (e==NULL) {
     fatal_error("Internal problem in add_reverse_transition_to_state\n");
  }
  l->next = e->transinv;
  e->transinv = l;
}



/**
 * this function creates for each transition of the graph
 * the corresponding reverse one
 */
void compute_reverse_transitions(Etat_comp* graph, int n_states) {
  register int i;
  for (i=0; i<n_states; i++) {
    if ( graph[i] == NULL )
      continue;
    Fst2Transition l = graph[i]->trans;
    while (l != NULL) {
      add_reverse_transition_to_state(l->tag_number,i,graph[l->state_number]);
      l = l->next;
    }
  }
}



///////////////////////////////////////////////////////////////
////// DETERMINISATION  //////////////////////////////////////
/////////////////////////////////////////////////////////////

void init_hachage_det(int *h, int n)
{
  // time-critical function, called often
  register int i;
  // instead of
  //    for (i=0;i < NBRE_ETIQ_TRANSITION_COMP;i++) h[i]=-1;
  // initializing only used fields saves time:
  //  for (i=0; i < (nombre_etiquettes_comp+nombre_graphes_comp+1); i++)
  // even better until it is really necessary
  for (i=0; i <= n; i++)
    h[i]=-1;
}


void init_stock_det(ensemble_det s[])
{
  register int i;
  for (i=0;i < NBRE_ETIQ_TRANSITION_COMP;i++) s[i]=NULL;
}



void init_resultat_det(Etat_fst_det resultat[],struct noeud_valeur_det *racine,int dernier_etat_res)
{
  resultat[0] = nouvel_etat_mat_det();
  ajouter_etat_dans_ensemble_det(0,&(resultat[0]->ens));
  numero_ensemble_det(resultat[0]->ens,racine,dernier_etat_res);
}


ensemble_det copie_det(ensemble_det e) {
  ensemble_det nouveau;
  if (e!=NULL) {
      nouveau=(ensemble_det)malloc(sizeof(struct char_etats_det));
      nouveau->num_char=e->num_char;
      nouveau->valeur=e->valeur;
      nouveau->suivant=copie_det(e->suivant);
      return nouveau;
    }
  return NULL;
}


void store_etat_det(Graph_comp g,Etat_fst_det e) {
  Etat_comp ee = add_state(g);
  if ( is_final_state(e) )
    ee->controle = (unsigned char) (ee->controle | 1);
  ee->trans = e->trans;
  e->trans = NULL; /* destroy old pointer to transitions to avoid double freeing */
}

void sauvegarder_etat_det(FILE *f,Etat_comp e)
{
 Fst2Transition ptr;
 if ( is_final_state(e) )
   u_fputc((unichar)'t',f);
 else u_fputc((unichar)':',f);
 ptr=e->trans;
 while (ptr!=NULL)
   {
     u_fprintf(f," %d %d",ptr->tag_number,ptr->state_number);
     ptr=ptr->next;
   }
 u_fputc((unichar)' ',f);
 u_fputc((unichar)'\n',f);
}


/**
 * determinize the graph/automaton "graph",
 * @param graph the graph to be determinized
 * @return 1 if successful, 0 if not
 */
int determinisation(Graph_comp graph) {

  if ((graph->n_states == 0) || (graph->states[0] == NULL)) {
    // do not segfault on empty automaton
    error("warning: resulting automaton is empty\n");
    return 1;
  }

  Etat_comp *states = graph->states;

  Etat_fst_det resultat[NBRE_ET];
  ensemble_det stock[NBRE_ETIQ_TRANSITION_COMP];
  unsigned char final[NBRE_ETIQ_TRANSITION_COMP];
  int hachage[NBRE_ETIQ_TRANSITION_COMP];
  int hachageinv[NBRE_ETIQ_TRANSITION_COMP];
  init_hachage_det(hachage,(NBRE_ETIQ_TRANSITION_COMP-1));

  Graph_comp new_graph = new_graph_comp();

  Fst2Transition ptr;
  ensemble_det courant;
  unsigned int q;  //etat courant ancien graph
  int count;  //compteur pour savoir ou l'on se trouve dans notre int de INT_BITS bits
  int compteur; //compteur pour savoir l'indice du dernier ensemble rentre dans stock;
  int num;
  int i, file_courant, k;
  int temp, max_temp, dernier_etat_res;
  int temp2, sous_graphe;
  struct noeud_valeur_det *racine_det;

  dernier_etat_res = -1;
  racine_det = nouveau_noeud_valeur_det();
  init_graphe_mat_det(resultat);
  init_resultat_det(resultat,racine_det,dernier_etat_res);
  dernier_etat_res = 0;
  max_temp = NBRE_ETIQ_TRANSITION_COMP-1;

  if ( is_final_state(states[0]) )
    resultat[0]->controle = (unsigned char)(resultat[0]->controle | 1);
  init_stock_det(stock);

  file_courant = 0;
  temp2 = file_courant % NBRE_ET;
  while (resultat[temp2] != NULL)
    {
      courant = resultat[temp2]->ens;
      init_hachage_det(hachage,max_temp);
      max_temp = 0;
      compteur = 0;
      while (courant != NULL)
        {
          count = 0;
          q = (courant->num_char*INT_BITS) - 1;
          while (count < INT_BITS)
            {
              q++;
              if (((courant->valeur)&(1<<count))!=0)
                {
                  ptr = states[q]->trans;
                  while (ptr != NULL)
                    {
                      temp = ptr->tag_number;

                      if (temp < 0)
                        {
                          temp = nombre_etiquettes_comp - 1 - temp;
                          sous_graphe = 1;
                        }
                      else
                        sous_graphe = 0;
                      
                      if (temp > max_temp)
                        max_temp = temp;

                      if (hachage[temp] == -1)
                        {
                          hachage[temp] = compteur;
                          liberer_char_etat_det(stock[compteur]);
                          final[compteur] = 0;
                          if (sous_graphe == 0)
                            hachageinv[compteur] = temp;
                          else
                            {
                              hachageinv[compteur] = nombre_etiquettes_comp - 1 - temp;
                            }
                          stock[compteur] = NULL;
                          compteur++;
                        }
                      ajouter_etat_dans_ensemble_det(ptr->state_number,&stock[hachage[temp]]);
                      if ( is_final_state(states[ptr->state_number]) )
                        final[hachage[temp]] = 1;    //test de finalite
                      ptr = ptr->next;
                    }
                }
              count++;
            }
          courant = courant->suivant;
        }
      for (i=0; i < compteur; i++)
        {
          num = numero_ensemble_det(stock[i],racine_det,dernier_etat_res);
          temp = num % NBRE_ET;
          ajouter_transition_mat_det(resultat[temp2],hachageinv[i],num);
          if (num > dernier_etat_res)
            {
              if (resultat[temp]!=NULL)
                {
                  /* m="Too many states in deterministic graph\n";
                     m=m+"Not enough memory to continue";
                     erreur(m.c_str());*/
                  for (k=0;k<NBRE_ET;k++)
                    liberer_etat_det(resultat[k]);
                  liberer_arbre_det(racine_det);
                  for (i=0; i < NBRE_ETIQ_TRANSITION_COMP; i++) {
                    if (stock[i]!=NULL) {
                      liberer_char_etat_det(stock[i]);
                    }
                  }
                  fatal_error("Too many states in automaton: cannot determinize.\n"
                        "Max. number of states per automaton/subgraph: NBRE_ET = %i\n",
                        NBRE_ET);
                }
              resultat[temp] = nouvel_etat_mat_det();
              dernier_etat_res++;
              resultat[temp]->ens = copie_det(stock[i]);

              resultat[temp]->controle = (unsigned char)((resultat[temp]->controle) | final[i]);
            }
        }
      store_etat_det(new_graph,resultat[temp2]);
      liberer_etat_det(resultat[temp2]);
      resultat[temp2] = NULL;
      file_courant++;
      temp2 = file_courant % NBRE_ET;
    }
  liberer_arbre_det(racine_det);
  for (i=0;i < NBRE_ETIQ_TRANSITION_COMP;i++) {
    if (stock[i]!=NULL) {
      liberer_char_etat_det(stock[i]);
    }
  }

  /* Finally we copy the content of new_graph to graph
     freeing old content of graph */
  move_graph_comp(graph,new_graph);
  
  return 1;
}



/**
 * Reverse the graph "graph", so that all edges are inverted.
 * "graph" is also a deterministic automaton.
 *
 * Because an deterministic automaton ("Graph_comp") has only one initial
 * state, but may have many final states, reversing is complicated or
 * must be done by introducing <E> transitions, which requires removing
 * them in the end.
 */
int reverse (Graph_comp graph) {

  register int i;

  /* make sure that reverse transitions of the automaton are
     calculated */
  compute_reverse_transitions(graph->states,graph->n_states);

  /* table for mapping between states of old and reversed automaton */
  int* map_table = (int*) malloc(graph->n_states*sizeof(int));
  for ( i = 0; i < graph->n_states; i++ )
    map_table[i] = -1;

  /* new reversed graph, state 0 as initial state */
  Graph_comp reversed = new_graph_comp();
  add_state(reversed);

  for ( i = ((graph->n_states)-1); i >= 0; i-- )
    {

      /* create a new state if necessary */
      if ( map_table[i] == -1 )
        add_state(reversed,&(map_table[i]));
      
      Fst2Transition l = graph->states[i]->transinv;
      for (   ; l != NULL ; l = l->next )
        {
          if ( map_table[(l->state_number)] == -1 )
            add_state(reversed,&(map_table[(l->state_number)]));
          add_transition_to_etat_comp(reversed->states[(map_table[i])],
                                      l->tag_number,map_table[(l->state_number)]);
#ifdef DEBUG                                                                /* DEBUG */
          error("%i --%i--> %i\n",                                 /* DEBUG */
                  map_table[i],sons->tag_number,map_table[(sons->state_number)]); /* DEBUG */
#endif                                                                      /* DEBUG */
        }

      /* all final states have <E> connections from the initial state
         in the reversed automaton: this requires to remove epsilon
         transitions again; should be changed for efficiency
         reasons */
      if ( is_final_state(graph->states[i]) )
          add_transition_to_etat_comp(reversed->states[0],
                                      0,map_table[i]); 

      /* initial state becomes final state */
      if ( i == 0 ) // is_initial_state(graph->states[i])
        set_final_state(reversed->states[(map_table[i])],1);

      /* clean memory (peu-a-peu to ever have enough!) */
      //   maybe better: reuse transition
      //   structures to save malloc time !? */
      liberer_etat_comp(graph->states[i]);
      graph->states[i] = NULL;

    }

  /* finally: remove epsilon transitions, make the automaton
     deterministic */
  compute_reverse_transitions(reversed->states,reversed->n_states);
  for (int h=0;h<reversed->n_states;h++) {
    if (is_final_state(reversed->states[h])) {
      // we start the co_accessibility check from every final state
      co_accessibilite_comp(reversed->states,h);
    }
  }
  accessibilite_comp(reversed->states,0);
  virer_epsilon_transitions_comp(reversed->states,reversed->n_states);
  eliminer_etats_comp(reversed->states,&(reversed->n_states));
  determinisation(reversed);

  /* move the reversed graph to "graph" */
  move_graph_comp(graph,reversed);

  /* clean up */
  free(map_table);

  return 1;

}



/**
 * Minimize the graph/automaton "graphe".
 * Uses Brzozowskis algorithm: reverse, determinize, reverse.
 */
int minimisation(Graph_comp graph) {
  
  reverse(graph);
  /* determinization is performed in reverse(Graph_comp) */
  reverse(graph);

  return 1;
}


/**
 * write the graph/automaton "graphe" to file "fs_comp"
 */
int write_graph_comp(FILE* f,
                     Graph_comp graph,
                     int number,
                     unichar *name) {

  u_fprintf(f, "%d %S\n", number, name);

  if (graph->states[0] == NULL) { /* do not segfault on empty automaton */
    error("warning: resulting automaton is empty\n");
    u_fprintf(f, ": \nf \n");
    return 1;
  }
  
  /* print all states */
  int i;
  for ( i=0; i < graph->n_states; i++ )
    {
      if ( graph->states[i] == NULL )
        continue;
      sauvegarder_etat_det(f,graph->states[i]);
    }

  /* mark end of graph */
  u_fprintf(f,"f \n");
  
  return 0;
}




/////////////////////////////////////////////////////////////////////
////////// MANIPULATION DE LA STRUCTURE D'ETAT /////////////////////
////////////////////////////////////////////////////////////////////


void ajouter_transition_comp(Etat_comp* states,int dep,int arr,int etiq)
{
  Fst2Transition ptr;

  //transition
  ptr = new_Fst2Transition();
  ptr->state_number = arr;
  ptr->tag_number = etiq;
  ptr->next = states[dep]->trans;
  states[dep]->trans = ptr;

  //transition inverse
  ptr = new_Fst2Transition();
  ptr->state_number = dep;
  ptr->tag_number = etiq;

  ptr->next = states[arr]->transinv;
  states[arr]->transinv = ptr;

}





int ajouter_etat_deliage_comp(Graph_comp graph,int dep,int etiq,int graphe_courant)
{
  int state_n;
  add_state(graph,&state_n);
  ajouter_transition_comp(graph->states,dep,state_n,etiq);
  return 1;
}


////////////////////////////////////////////////////////////
///// MANIPULATION DE L'ARBRE DES ETIQUETTES /////////////////////////
/////////////////////////////////////////////////////////


/**
 *  renvoie un noeud vierge
 */
struct noeud_comp* nouveau_noeud_comp()
{
  struct noeud_comp *n;

  n = (struct noeud_comp*)malloc(sizeof(struct noeud_comp));
  n->fin = -1;
  n->lettre = 1;
  n->l = NULL;
  return n;
}


/**
 *  retourne le sous-noeud correspondant au char c; le cree si absent
 */
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
      ptr2 = (struct liste_feuilles_comp*)malloc(sizeof(struct liste_feuilles_comp));
      ptr2->node = res;
      ptr2->suivant = n->l;
      n->l = ptr2;

      return res;
    }
  return ptr->node;
}



/**
 *  ajoute une etiquette si absente de l'arbre
 */
int ajouter_etiquette_comp(unichar *etiq, struct noeud_comp *ptr,int i)
{
  struct noeud_comp *ptr1;

  ptr1 = get_sous_noeud_comp(ptr,etiq[i],1);
  i++;
  if (etiq[i] == '\0')
    {
      if (ptr1->fin == -1)
	{
	  ptr1->fin = nombre_etiquettes_comp;
	  nombre_etiquettes_comp++;
	}
      return (ptr1->fin);
    }
  return ajouter_etiquette_comp(etiq,ptr1,i);
}



/**
 *  vide sans la detruire, une feuille de l'arbre des etiquettes
 */
void vider_feuille_comp(struct noeud_comp *n)
{
  struct liste_feuilles_comp *ptr;
  struct liste_feuilles_comp *ptr2;

  ptr = n->l;
  if (ptr != NULL)
    {
      ptr2 = ptr->suivant;
      vider_feuille_comp(ptr->node);
      free(ptr->node);
      free(ptr);
      while (ptr2 != NULL)
	{
	  ptr = ptr2;
	  ptr2 = ptr->suivant;
	  vider_feuille_comp(ptr->node);
	  free(ptr->node);
	  free(ptr);
	}
    }
}




/**
 *  libere l'arbre des etiquettes
 */
void libere_etiquettes_comp()
{
  vider_feuille_comp(rac_comp);
  free(rac_comp);
}



////////////////////////////////////////////////////////////
///// MANIPULATION DES ARBRES DES GRAPHES /////////////////////////
/////////////////////////////////////////////////////////


/**
 *  renvoie un noeud vierge
 */
struct noeud_g_comp* nouveau_noeud_g_comp()
{
  struct noeud_g_comp *n;
  n = (struct noeud_g_comp*)malloc(sizeof(struct noeud_g_comp));
  n->fin = -1;
  n->lettre = 1;
  n->l = NULL;

  return n;
}


/**
 *  retourne le sous-noeud correspondant au char c; le cree si absent
 */
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
      ptr2=(struct liste_feuilles_g_comp*)malloc(sizeof(struct liste_feuilles_g_comp));
      ptr2->node = res;
      ptr2->suivant = n->l;
      n->l = ptr2;
      return res;
    }
  return ptr->node;
}


/**
 *  ajoute un nom de graphe si absent de l'arbre
 */
int ajouter_graphe_comp(unichar *etiq, struct noeud_g_comp *ptr,int i)
{
  struct noeud_g_comp *ptr1;

  ptr1=get_sous_noeud_g_comp(ptr,etiq[i],1);
  i++;
  if (etiq[i]=='\0')
    {
      if (ptr1->fin == -1)
	{
	  ptr1->fin=nombre_graphes_comp;
	  nombre_graphes_comp++;
	}
      return (ptr1->fin);
    }
  return ajouter_graphe_comp(etiq,ptr1,i);
}



/**
 *  vide sans la detruire, une feuille de l'arbre des graphes
 */
void vider_feuille_g_comp(struct noeud_g_comp *n)
{
  struct liste_feuilles_g_comp *ptr;
  struct liste_feuilles_g_comp *ptr2;

  ptr=n->l;
  if (ptr != NULL)
    {
      ptr2=ptr->suivant;
      vider_feuille_g_comp(ptr->node);
      free(ptr->node);
      free(ptr);
      while (ptr2 != NULL)
	{
	  ptr=ptr2;
	  ptr2=ptr->suivant;
	  vider_feuille_g_comp(ptr->node);
	  free(ptr->node);
	  free(ptr);
	}
    }
}




/**
 *  libere l'arbre des graphes
 */
void libere_graphes_comp()
{
  vider_feuille_g_comp(rac_graphe_comp);
  free(rac_graphe_comp);
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


/**
 *  enleve la transition inverse allant vers origine de la liste ptr
 */
Fst2Transition degager_transition_inverse_comp(Fst2Transition ptr,int origine) {
  Fst2Transition tmp;
  if (ptr==NULL) return NULL;
  if (((ptr->state_number)==origine)&&((ptr->tag_number)==EPSILON_comp)) {
    tmp=ptr->next;
    free(ptr);
    return tmp;
  }
  ptr->next=degager_transition_inverse_comp(ptr->next,origine);
  return ptr;
}



/**
 *  enleve les epsilon transitions de la liste ptr
 */
Fst2Transition vider_epsilon_comp(Fst2Transition ptr,Etat_comp *letats,int origine, int mark[]) {  
  Fst2Transition liste,tmp;
  Etat_comp e,e2;  
  if (ptr == NULL) return NULL;
  if (ptr->tag_number == EPSILON_comp) {
    e = letats[ptr->state_number];
    mark[ptr->state_number] = 1; // we mark the current state as already been a destination state
    //printf("mark %d\n",ptr->state_number);
    //printf("%d:%d\n",origine,ptr->state_number);
    if (ptr->state_number == origine) {
      // if we have a loop by epsilon, we can ignore it, because (a|<E>)+ has
      // has already been turned into a+ | <E>
      liste=ptr;
      ptr=ptr->next;
      free(liste);
      return ptr;
    }
    if ( is_final_state(e) ) {
      // if we can reach a final state through an epsilon transition
      // then the current state must be final
      letats[origine]->controle=(unsigned char)((letats[origine]->controle)|1);
    }    
    liste=e->trans;
    while (liste!=NULL) {
      if (mark[liste->state_number] == 0) { //if not marked
	tmp=new_Fst2Transition();
	tmp->tag_number=liste->tag_number;
	tmp->state_number=liste->state_number;
	tmp->next=ptr->next;
	ptr->next=tmp;
	//printf("%d,%d,%d\n",origine,tmp->tag_number,tmp->state_number);
	
	e2=letats[liste->state_number];
	tmp=new_Fst2Transition();
	tmp->tag_number=liste->tag_number;
	tmp->state_number=origine;
	tmp->next=e2->transinv;
	e2->transinv=tmp;
      }
      liste=liste->next;
    }
    tmp=ptr->next;
    e->transinv=degager_transition_inverse_comp(e->transinv,origine);   
    free(ptr);
    return vider_epsilon_comp(tmp,letats,origine,mark);
  }
  ptr->next=vider_epsilon_comp(ptr->next,letats,origine,mark);
  return ptr;
}


int* new_marker(int n) {
  int *res = new int[n];
  int i = 0;

  for (i = 0 ; i < n ; i++) {
    res[i] = 0;
  }

  return res;
}


void add_closure(Llist_i_comp* closures,int q,Llist_i_comp nlist) {
  if (nlist == NULL) return;

  struct ll_i_cell_comp *ptr = nlist->head;

  while (ptr != NULL) {
    ll_i_insert_sorted(&(closures[q]),ptr->value);
    
    ptr = ptr->next;
  }

}



Llist_i_comp get_epsilon_closure_rec(Llist_i_comp* closures,Etat_comp *letats,int q, unsigned char mark[]) {
  Fst2Transition ptr;

  if (mark[q] == 0) { // si non marque
    mark[q] = 1;
    ll_i_insert_sorted(&(closures[q]),q);
    ptr = letats[q]->trans;
    while (ptr != NULL) {
      if ((ptr->tag_number == EPSILON_comp) && (ptr->state_number != q)) {
	add_closure(closures,q,get_epsilon_closure_rec(closures,letats,ptr->state_number,mark));
      }
      ptr = ptr->next;
    }
  }
  return closures[q];
}


Llist_i_comp * get_epsilon_closures(Etat_comp *letats,int n) {
  Llist_i_comp * closures = (Llist_i_comp *)malloc(n*sizeof(Llist_i_comp));
  unsigned char mark[n];
  int i;  

  for (i = 0 ; i < n ; i++) {
    mark[i] = 0;
    closures[i] = NULL;    
  }


  for (i = 0 ; i < n ; i++) { //pour chaque etat
    if (mark[i] == 0) {
      get_epsilon_closure_rec(closures,letats,i,mark);
    }
  }

  return closures;
}


void remove_all_epsilon_transitions(Etat_comp *letats, int n) {
  Fst2Transition ptr,res,temp;  

  for (int i = 0 ; i < n ; i++) { // pour chq etat
    ptr = letats[i]->trans;
    res = NULL;
    while (ptr != NULL) { //pr chq transition     
      temp = ptr;
      ptr = ptr->next;
      
      if (temp->tag_number == EPSILON_comp) { // on supprime transition
	free(temp); 
      }
      else{
	temp->next = res;
	res = temp;
      }      
    }
    letats[i]->trans = res;

    ptr = letats[i]->transinv;
    res = NULL;
    while (ptr != NULL) { //pr chq transition inverse
      temp = ptr;
      ptr = ptr->next;
      
      if (temp->tag_number == EPSILON_comp) { // on supprime transition inverse
	free(temp); 
      }
      else{
	temp->next = res;
	res = temp;
      }      
    }
    letats[i]->transinv = res;


  }
}


void add_transitions_according_to_epsilon_closure(Llist_i_comp *closures,Etat_comp* states,int n) {
  unsigned int i;
  struct ll_i_cell_comp *ptr;
  Fst2Transition tr;
  Etat_comp e;

  for (i = 0 ; i < (unsigned int)n ; i++) { //pour chq etat
    Llist_i_comp clos = closures[i];
    if (clos != NULL) {
      ptr = clos->head;     
      while (ptr != NULL) {
	if (ptr->value != i) {
	  e = states[ptr->value];
	  if ( is_final_state(e) ) { // etat final
	    states[i]->controle = (unsigned char)(states[i]->controle)|1; 
	  }
	  tr = e->trans;
	  while (tr != NULL) {
	    ajouter_transition_comp(states,i,tr->state_number,tr->tag_number);
	    tr = tr->next;
	  }
	}
	ptr = ptr->next;
      }
    }
  }
}


void free_closures(Llist_i_comp *closures,int n) {
  int i;

  for (i = 0 ; i < n ; i++) {
    ll_i_free(&(closures[i]));
  }
}



/**
 *  enleve les epsilon transitions du graphe
 */
void virer_epsilon_transitions_comp(Etat_comp *letats,int n) {
  Llist_i_comp *closures = get_epsilon_closures(letats,n);
  remove_all_epsilon_transitions(letats,n);
  add_transitions_according_to_epsilon_closure(closures,letats,n);
  free_closures(closures,n);
  free(closures);
}


/**
 *  Marque les etats accessibles
 */
void accessibilite_comp(Etat_comp *e,int i)
{
  Fst2Transition t;

  if ((e[i]!=NULL) && ((e[i]->controle)&4)==0)
    {
      e[i]->controle=(unsigned char)((e[i]->controle)|4);
      t=e[i]->trans;
      while (t!=NULL)
	{
	  accessibilite_comp(e,t->state_number);
	  t=t->next;
	}
    }
}


/**
 * Marque les etats co_accessibles
 */
void co_accessibilite_comp(Etat_comp *e,int i)
{
  Fst2Transition t;

  if ( (e[i] != NULL) && (((e[i]->controle)&8) == 0) )
    {
      e[i]->controle = (unsigned char) ((e[i]->controle)|8);
      t = e[i]->transinv;
      while ( t != NULL )
	{
	  co_accessibilite_comp(e,t->state_number);
	  t = t->next;
	}
    }
}


Fst2Transition supprimer_transition_comp(Etat_comp *letats,int i,Fst2Transition ptr)
{
  Fst2Transition tmp,t,tmp2;
  int j,etiq;

  j=ptr->state_number;
  etiq=ptr->tag_number;

  tmp=ptr->next;
  free(ptr);

  /* Suppression de la transition inverse correspondante */
  t=letats[j]->transinv;
  while (t!=NULL)
    {
      if ((t->state_number==i) && (t->tag_number==etiq))
	{
	  tmp2=t;
	  t=t->next;
	  free(tmp2);
	}
      else
	t=t->next;
    }

  return tmp;
}


//
//Supprime une transition inverse + transition correspondante
//retourne transition suivante
//

Fst2Transition supprimer_transitioninv_comp(Etat_comp *letats,int i,Fst2Transition ptr)
{
  Fst2Transition tmp,t,tmp2;
  int j,etiq;

  j=ptr->state_number;
  etiq=ptr->tag_number;

  tmp=ptr->next;
  free(ptr);

  /* Suppression de la transition correspondante */
  t=letats[j]->trans;
  while (t!=NULL)
    {
      if ((t->state_number==i) && (t->tag_number==etiq))
	{
	  tmp2=t;
	  t=t->next;
	  free(tmp2);
	}
      else
	t=t->next;
    }

  return tmp;
}



/**
 *  vire ptr si ptr pointe sur un etat a virer
 */
// with complex graphs we got a stack overflow with this recursive function:
//// Fst2Transition vider_trans_reciproques_comp(Fst2Transition ptr,Etat_comp *letats)
//// {
////   Fst2Transition tmp;
////   if (ptr==NULL) return NULL;
////   if ((((letats[ptr->state_number]->controle)&4)==0)||(((letats[ptr->state_number]->controle)&8)==0))
////   {
////     tmp=ptr->next;
////     free(ptr);
////     return vider_trans_reciproques_comp(tmp,letats);
////   }
////   ptr->next=vider_trans_reciproques_comp(ptr->next,letats);
////   return ptr;
//// }
// it's replaced now by an iterative one:
Fst2Transition vider_trans_reciproques_comp(Fst2Transition ptr,Etat_comp *letats)
{
  Fst2Transition tmp, tmp2, tmp_old;
  tmp=ptr;
  while (tmp!=NULL)
    {
      if ((((letats[tmp->state_number]->controle)&4)==0)||(((letats[tmp->state_number]->controle)&8)==0))
        {
          tmp2=tmp->next;
          if (tmp == ptr)
            ptr = tmp2;
          else
            tmp_old->next = tmp2;
          free(tmp);
          tmp=tmp2;
        }
      else
        {
          tmp_old = tmp;
          tmp=tmp->next;
        }
    }
  return ptr;
}



/**
 *  renvoie 1 si l'etat e n'est ni accessible ni coaccessible
 */
int est_a_virer_comp(Etat_comp e) {
return ((((e->controle)&4)==0)||(((e->controle)&8)==0));
}


/**
 *  remplace ancien par nouveau dans les transitions sortant de l'etat e
 */
void mettre_a_jour_sortie_comp(Etat_comp e,int ancien,int nouveau) {
Fst2Transition ptr;
ptr=e->trans;
 while (ptr!=NULL) {
   if (ptr->state_number==ancien) ptr->state_number=nouveau;
   ptr=ptr->next;
 }
}




/**
 *  remplace ancien par nouveau dans les transitions entrant dans l'etat e
 */
void mettre_a_jour_entree_comp(Etat_comp e,int ancien,int nouveau) {
Fst2Transition ptr;
ptr=e->transinv;
 while (ptr!=NULL) {
   if (ptr->state_number==ancien) ptr->state_number=nouveau;
   ptr=ptr->next;
 }
}




/**
 *  renumerote les transitions vers ancien en transitions vers nouveau
 */
void renumeroter_comp(Etat_comp *liste,int ancien,int nouveau) {
Fst2Transition ptr;
ptr=liste[ancien]->trans;
 while (ptr!=NULL) {
   if (ptr->state_number!=ancien) mettre_a_jour_entree_comp(liste[ptr->state_number],ancien,nouveau);
   else ptr->state_number=nouveau;
   ptr=ptr->next;
   }

ptr=liste[ancien]->transinv;
 while (ptr!=NULL) {
   mettre_a_jour_sortie_comp(liste[ptr->state_number],ancien,nouveau);
   ptr=ptr->next;
   }
}




/**
 *  Elimine les etats non accessibles et non co_accessibles
 */
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
      //free(letats[dernier]);
      liberer_etat_comp(letats[dernier]);
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
    //free(letats[dernier]);
    liberer_etat_comp(letats[dernier]);
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
      error(
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
  replace_colon_by_pathseparator(NOM);

}









/**
 *  Sépare contenu (entrée de l'automate) et transduction (sortie)
 */
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
do
  {
    if (ligne[i]=='\\') {
      i++;
      if (ligne[i]=='\0') {
        fatal_error("Unexpected backslash at end of line\n");
      }
    }
    transduction[j] = ligne[i];
    j++;
 }
 while (ligne[i++] != '\0'); /* make sure that the closing '\0' is also copied to transduction */

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
   while (u_is_letter(contenu[(*pos)-1])&& (i<N_CAR_MAX_COMP));
  if (i>=N_CAR_MAX_COMP) return -3;
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
   while (is_letter(contenu[(*pos)-1],alph)&& (i<N_CAR_MAX_COMP));
  if (i>=N_CAR_MAX_COMP) return -3;
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
default: fatal_error("Internal error in get_mot_comp_generic\n");
}
return 0;
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
   }while ((contenu[*pos] != '>') && (contenu[*pos] != '\0') && (i < N_CAR_MAX_COMP));
   if (i >= N_CAR_MAX_COMP) return -3;
   if ((contenu[*pos] == '\0')) {
      error("WARNING : canonical form should be ended by >\n");
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
   }while ((contenu[*pos] != '}') && (contenu[*pos] != '\0') && (i < N_CAR_MAX_COMP));
   if (i >= N_CAR_MAX_COMP) return -3;
   if (contenu[*pos] == '\0') {
      error("WARNING : desambiguised form should be ended by }\n");
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
       if (get_mot_comp_generic(contenu,pos,mot,pos_seq,mode,alph)== -3) return -3;
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
  while ((fin == 0) && (contenu[*pos] != '\0') && ((*pos_seq) < TAILLE_SEQUENCE_COMP))
  {
    fin = lire_mot_entre_guillemet_comp(contenu,pos,sequence[*pos_seq],pos_seq,mode,alph);
  }

  if (fin == -3) return -3;
  if ((*pos_seq) >= TAILLE_SEQUENCE_COMP)
    return -1;
  return 0;
}




int get_sous_graphe_comp(unichar contenu[],int *pos,unichar mot[],int *pos_seq)
{
  int i = 0;
  while ((contenu[*pos] != '+') && (contenu[*pos] != '\0')&& (i < N_CAR_MAX_COMP))
  {
    // if (contenu[*pos] == '\\') (*pos)++;
    /* the backslash shouldn't be used an escape character in graph
       names; under Unixes it is now obsolete: using the colon ':' as
       path separator, you don't need the construct '\/'. */
    mot[i]=contenu[*pos];
    // on normalise les separateurs de repertoires en ':'
    if (mot[i] == '/' || mot[i] == '\\') { mot[i] = ':'; }
    i++;
    (*pos)++;
  }
  if (i >= N_CAR_MAX_COMP) return 0;
  mot[i] = '\0';
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
           get_sous_graphe_comp) */
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

      indice = ajouter_graphe_comp(seq + 1,rac_graphe_comp,0); // + 1 pour omettre le ':'
      if (indice >= NOMBRE_GRAPHES_COMP)
        {
           error(
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
      if (contenu[*pos]!='\\')
        {
          if (contenu[*pos] == '"') // Cas d'une sequence entre guillemet
            return traitement_guillemet_comp(contenu,pos,sequence,pos_seq,graphe_courant,mode,alph);
          get_caractere_comp(contenu,pos,sequence[*pos_seq],pos_seq);
          return 0;
        }
      if (/*(contenu[(*pos)+1] != '\0')&& */(contenu[(*pos)+1] != '\\')) // caractere despecialise
        {
          get_caractere_comp(contenu,pos,sequence[*pos_seq],pos_seq);
          return 0;
        }
      if (contenu[(*pos)+2] == '"')   //caractere " seul
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
      if (get_sequence_desambiguisee_comp(contenu,pos,sequence[*pos_seq],pos_seq)== -3) return -3;
      return 0;
    }

  // cas d'un debut de <...>
  if (contenu[*pos] == '<')
    {
      if (get_forme_canonique_comp(contenu,pos,sequence[*pos_seq],pos_seq) == -3) return -3;
      return 0;
    }

  // cas d'une lettre
  if (is_letter_generic(contenu[*pos],mode,alph)) {
    if (get_mot_comp_generic(contenu,pos,sequence[*pos_seq],pos_seq,mode,alph) == -3) return -3;
    return 0;
  }


  // par defaut
  get_caractere_comp(contenu,pos,sequence[*pos_seq],pos_seq);
  return 0;
}


int traitement_etiquettes_comp(int *indice,unichar mot[])
{
  if (((*indice) = ajouter_etiquette_comp(mot,rac_comp,0)) >= MAX_FST2_TAGS)
    {
      error("ERROR at top level: Too many tags (maximum %d)\n",MAX_FST2_TAGS);
      return -1;
    }
  u_strcpy(donnees->Etiquette_comp[(*indice)],mot);

  return 1;
}



//Transfo de la sequence en sequence d'entiers (negatif si graphe, positif si etiquette normale

int transfo_seq_carac_en_entiers(unichar sequence[TAILLE_SEQUENCE_COMP][N_CAR_MAX_COMP],unichar transduction[],int compteur_mots,int sequence_ent[],int *trans)
{
 int i,j,indice;
 unichar s[100];

 (*trans) = 0;
  j=0; i=0;
  if (transduction[0]!='\0') {  //CAS DE LA TRANSDUCTION
       j = 1;
       if (((sequence[0][0] == ':') && (sequence[0][1] != '\0')))
       //CAS DU SOUS-GRAPHE
       {
        /* strcpy(temp,"<E>/");
         strcat(temp,transduction);
         if (traitement_etiquettes_comp(&indice,temp) == -1) return -1;*/
         //sequence_ent[0] = indice;
         //i = 0;
         //(*trans) = 1;
       }
       else
       {
          u_strcpy_char(s,"/");
          u_strcat(sequence[0],s);
          u_strcat(sequence[0],transduction);

          if (traitement_etiquettes_comp(&indice,sequence[0]) == -1) return -1;
          sequence_ent[0] = indice;
          i = 1;
       }
  }

  while (i < compteur_mots)
  {
     if ((sequence[i][0] == ':') && (sequence[i][1] != '\0'))  //SOUS GRAPHE
     {
       /* inutile
        k = 1;
        do  // transfo :grf1 en grf1
          temp[k -1] = sequence[i][k++];
        while (sequence[i][k] != '\0');
        temp[k -1] = '\0';
        */
        indice = ajouter_graphe_comp(sequence[i] + 1,rac_graphe_comp,0);
        indice = - indice -1;
        sequence_ent[j] = indice;
     }
     else
     {
        u_strcpy_char(s,"/<E>");

        if (transduction[0]!='\0') u_strcat(sequence[i],s);
        if (traitement_etiquettes_comp(&indice,sequence[i]) == -1) return -1;
        sequence_ent[j] = indice;
     }
  i++;
  j++;
  }
  return 1;
}



int ecriture_transition_comp(Graph_comp graph,int seq_ent[],int sortants[],int etat_courant,int longueur,int graphe_courant,int trans)
{
  int k,j;
  int dep;
  k=0;

   while (sortants[k] != -1)
   {
     dep=etat_courant;
     for (j=0;j<longueur -1+trans;j++)
     {
       if (ajouter_etat_deliage_comp(graph,dep,seq_ent[j],graphe_courant) == 0) return 0;
       dep = (graph->n_states - 1);
     }
     ajouter_transition_comp(graph->states,dep,sortants[k],seq_ent[longueur +trans -1]);
     k++;
   }

return 1;
}




int traitement_transition_comp(Graph_comp graph,unichar contenu[],
                               unichar transduction[],int sortants[],
                               int *pos, int etat_courant,
                               int graphe_courant,
                               int mode,Alphabet* alph)
{
  int compteur_mots = 0;
  int plus = 0;
  int trans;
  unichar sequence[TAILLE_SEQUENCE_COMP][N_CAR_MAX_COMP];
  int sequence_ent[TAILLE_SEQUENCE_COMP];
  char err[1000];
  while ((plus == 0) && (contenu[*pos] != '\0') && (compteur_mots < TAILLE_SEQUENCE_COMP)) {
    plus = lire_mot_comp(contenu,pos,sequence,&compteur_mots,graphe_courant,mode,alph);
  }

  if (plus == -2) return -1;
  if (plus == -3) {       // Cas trop de caractères dans un mot
   u_to_char(err,donnees->nom_graphe[graphe_courant]);
   if (graphe_courant == 0) {
      error("ERROR in main graph %s: The size of a word should be lower than %d characters\n",err,N_CAR_MAX_COMP);
    }
    else {
      error("WARNING in main graph %s: The size of a word should be lower than %d characters\nGraph has been emptied\n",err,N_CAR_MAX_COMP);
    }
    return 0;
  }
  if ((plus == -1) || (compteur_mots >= TAILLE_SEQUENCE_COMP) )
  {
    u_to_char(err,donnees->nom_graphe[graphe_courant]);
    if (graphe_courant == 0) {
      error("ERROR in main graph %s: The size of a sequence between two + should be lower than %d words\n",err,TAILLE_SEQUENCE_COMP);
    }
    else {
      error("WARNING in main graph %s: The size of a sequence between two + should be lower than %d words\nGraph has been emptied\n",err,TAILLE_SEQUENCE_COMP);
    }
    return 0;
  }
  if (transfo_seq_carac_en_entiers(sequence,transduction,compteur_mots,sequence_ent,&trans) == -1) return -1;
  if (ecriture_transition_comp(graph,sequence_ent,sortants,etat_courant,compteur_mots,graphe_courant,trans) == 0) return 0;
  return 1;
 }



void traiter_debut_fin_variable(Graph_comp graph,unichar contenu[],unichar transduction[],int sortants[], int *pos, int etat_courant,int graphe_courant) {
  unichar sequence[1][N_CAR_MAX_COMP];
  int sequence_ent[1];
  int trans;
  u_strcpy(sequence[0],contenu);
  if (transfo_seq_carac_en_entiers(sequence,transduction,1,sequence_ent,&trans) == -1) return;
  if (ecriture_transition_comp(graph,sequence_ent,sortants,etat_courant,1,graphe_courant,trans) == 0) return;
}



void traiter_context_mark(Graph_comp graph,unichar contenu[],unichar transduction[],int sortants[], int *pos, int etat_courant,int graphe_courant) {
  unichar sequence[1][N_CAR_MAX_COMP];
  int sequence_ent[1];
  int trans;
  u_strcpy(sequence[0],contenu);
  if (transfo_seq_carac_en_entiers(sequence,transduction,1,sequence_ent,&trans) == -1) return;
  if (ecriture_transition_comp(graph,sequence_ent,sortants,etat_courant,1,graphe_courant,trans) == 0) return;
}





int traitement_ligne_comp(unichar ligne[],int sortants[],
                          Graph_comp graph,int etat_courant,
                          int graphe_courant,
                          int mode,Alphabet* alph)
{
  unichar contenu[TAILLE_MOT_GRAND_COMP];
  unichar transduction[TAILLE_MOT_GRAND_COMP];
  int i;
  int res;

  if (sortants[0]!=-1)
  {
    i=0;
    if ((u_strlen(ligne)>2
         && ligne[0]=='$'
         && ( ligne[u_strlen(ligne)-1]=='('
              || ligne[u_strlen(ligne)-1]==')'))
        // context marks are handled exactly as variable marks
        || (!u_strcmp_char(ligne,"$[") || !u_strcmp_char(ligne,"$![")
            || !u_strcmp_char(ligne,"$]"))) {
        u_strcpy(contenu,ligne);
        u_strcpy_char(transduction,"");
        traiter_debut_fin_variable(graph,contenu,transduction,sortants,&i,etat_courant,graphe_courant);
        return 1;
    }
    traitement_transduction_comp(ligne,contenu,transduction);
    while (contenu[i]!='\0')
    {
      res = traitement_transition_comp(graph,contenu,transduction,sortants,&i,etat_courant,graphe_courant,mode,alph);
      if ((res == -1) || (res == 0)) return res;
    }
  }
  return 1;
}



/**
 * read one graph state: 
 * - store box entry to ligne
 * - store transitions in sortants
 * retourne 0 si erreur
 * retourne 1 si OK
 * retourne 2 si pas de transitions sortants de la boite
*/
int lire_ligne_comp(FILE *f, unichar *ligne, int *sortants, int courant)
{
  unichar c;
  int i, n_sortantes;
  char err[TAILLE_MOT_GRAND_COMP];

  for (i=0;i<NOMBRE_TRANSITIONS_COMP;i++) sortants[i]=-1;

  i=0;
  while (u_fgetc(f) != '"');  /* skip all chars including first '"' */

  /* reescape the box content, store it in ligne */
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

  /* error: box content to long */
  if ( i >= TAILLE_MOT_GRAND_COMP )
    {
      u_to_char(err,donnees->nom_graphe[courant]);
      error(
              "ERROR in main graph %s.grf:\n"
              "Too many characters in box. The number of characters\n"
              "per box should be lower than %d\n",
              err,TAILLE_MOT_GRAND_COMP);
      return 0;
    }

  ligne[i] = '\0'; /* box entry read */

  u_fgetc(f); /* read the space char after the string */

  /* skip the values (x,y) for box positioning */
  u_read_int(f);
  u_read_int(f);

  /* read the number of transitions */
  n_sortantes = u_read_int(f);

  /* error: to many transitions */
  if ( n_sortantes >= NOMBRE_TRANSITIONS_COMP )
    {
      u_to_char(err,donnees->nom_graphe[courant]);
      error("WARNING in graph %s.grf:\n"
              "to many transitions. The number of transitions\n"
              "per box should be lower than %d\n",
              err, NOMBRE_TRANSITIONS_COMP);
      return 0;
    }

  /* read the transitions */
  for (i = 0 ; i < n_sortantes ; i++)
    {
      sortants[i] = u_read_int(f);
    }

  /* read the end of line char */
  u_fgetc(f);

  if (n_sortantes == 0)
    return 2;

  return 1;
}






//////////////////////////////////////////////////////////
///////////////// COMPILER UN GRAPHE /////////////////////
//////////////////////////////////////////////////////////

int compiler_graphe_comp (int graphe_courant, int mode, Alphabet* alph, FILE* fs_comp)
{

  int i;
  //int courant;
  int traitement,lire;
  int n_etats_initial;
  FILE *f;
  char nom[TAILLE_MOT_GRAND_COMP];
  int sortants[NOMBRE_TRANSITIONS_COMP];
  unichar ligne[TAILLE_MOT_GRAND_COMP];
  char err[1000];

  Graph_comp graph = new_graph_comp();


  /* status message */
  printf("Compiling graph ");
  u_prints(donnees->nom_graphe[graphe_courant]);
  printf("\n");
  // fprintf(stdout,"(%s)\n",nom);


  /* get name with path, e.g.
   * DATE -> C:/Unitex/French/Graphs/Date/DATE.grf */
  conformer_nom_graphe_comp(nom,graphe_courant); 

  /* open graph file */
  f=u_fopen(nom,U_READ);
  if (f == NULL) /* cannot open */
    {
      char s[TAILLE_MOT_GRAND_COMP];
      u_to_char(s,donnees->nom_graphe[graphe_courant]);
      error("Cannot open the graph %s.grf\n",s);
      error("(%s)\n",nom);
      write_graph_comp(fs_comp, graph,
                       (-(graphe_courant)-1),
                       donnees->nom_graphe[graphe_courant]);
      donnees->statut_graphe[graphe_courant] = 0;
      if (graphe_courant == 0) return 0;
      return 1;
    }

  /* read header */
  u_fgetc(f);                       /* skip BOM */
  while ( u_fgetc(f) != '#' );      /* skip header with formatting instructions */
  u_fgetc(f);                       /* skip newline */
  n_etats_initial = u_read_int(f);  /* read number of states */

  /* resize graph that it can hold all states */
  if ( graph->size < n_etats_initial )
    {
      if ( ! resize_graph_comp_to(graph,n_etats_initial) )
        { /* too many states - trop de boites dans graphe */
          donnees->statut_graphe[graphe_courant] = 0;
          write_graph_comp(fs_comp, graph,
                           (-(graphe_courant)-1),
                           donnees->nom_graphe[graphe_courant]);
          u_fclose(f);
          u_to_char(err,donnees->nom_graphe[graphe_courant]);
          error(
                  "ERROR in graph %s.grf: Too many boxes (%d).\n"
                  "The number of boxes should be lower than %d\n",
                  err,n_etats_initial,MAX_FST2_STATES);
          if (graphe_courant == 0)
            return 0;
          return 1;
        }
    }

  /* we start with every line being one state of the automaton */
  for (i=0; i < n_etats_initial; i++)
    {
      graph->states[i] = nouvel_etat_comp();
    }
  graph->n_states = n_etats_initial;

  for (i=0; i<n_etats_initial; i++)
    {

      /* read one line in file f */
      //On lit le contenu d'une boite et ses sorties (transitions)
      lire = lire_ligne_comp(f,ligne,sortants,graphe_courant);

      /* error reading line */
      if (lire == 0)
        {
          donnees->statut_graphe[graphe_courant] = 0;
          write_graph_comp(fs_comp, graph,
                           (-(graphe_courant)-1),
                           donnees->nom_graphe[graphe_courant]);
          free_graph_comp(graph);
          u_fclose(f);
          if (graphe_courant == 0)
            return 0;
          return 1;
        }

      /* box contains transitions: process them */
      //On traite la ligne : ecriture en memoire
      if (lire == 1)
        {
          traitement = traitement_ligne_comp(ligne,sortants,graph,i,graphe_courant,mode,alph);
          if ((traitement == 0) || (traitement == -1))
            {
              donnees->statut_graphe[graphe_courant] = 0;
              write_graph_comp(fs_comp, graph,
                               (-(graphe_courant)-1),
                               donnees->nom_graphe[graphe_courant]);
              free_graph_comp(graph);
              u_fclose(f);
              if (traitement == -1) return -1;
              if (graphe_courant == 0) return 0;
              return 1;
            }
        }
    }
  fflush(f);
  u_fclose(f);
  /* end of reading file */


  //etat initial: bit 2 a 1
  graph->states[0]->controle=(unsigned char)((graph->states[0]->controle)|2);
  //etat final: bit 1 a 1
  graph->states[1]->controle=(unsigned char)((graph->states[1]->controle)|1);
  

  // ELAGAGE, SUPPRESSION DES EPSILONS TRANSITIONS
  co_accessibilite_comp(graph->states,1);  
  virer_epsilon_transitions_comp(graph->states,graph->n_states);
  accessibilite_comp(graph->states,0);
  eliminer_etats_comp(graph->states,&(graph->n_states));

  if (graph->states[0] == NULL)
    {
      donnees->statut_graphe[graphe_courant] = 0;
      write_graph_comp(fs_comp, graph,
                       (-(graphe_courant)-1),
                       donnees->nom_graphe[graphe_courant]);

      free_graph_comp(graph);
      u_to_char(err,donnees->nom_graphe[graphe_courant]);
      if (graphe_courant == 0)
        {
          error("ERROR: Main graph %s.grf has been emptied\n",err);
          return 0;
        }
      error("WARNING: graph %s.grf has been emptied\n",err);
      return 1;
    }
  donnees->statut_graphe[graphe_courant] = 1;

  int det = determinisation(graph);
  int min = minimisation(graph);
  write_graph_comp(fs_comp, graph,
                   (-(graphe_courant)-1),
                   donnees->nom_graphe[graphe_courant]);
  free_graph_comp(graph);
  if ( (det == 0) || (min == 0) )
    {
      u_to_char(err,donnees->nom_graphe[graphe_courant]);
      if (graphe_courant == 0)
        {
          error("ERROR in main graph %s.grf: Tore error. Please, contact Unitex programmers\n",err);
          return 0;
        }
      error("WARNING in graph %s.grf: Tore error. Please, contact Unitex programmers\n",err);
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

  while (i>=0 && temp[i]!='/' && temp[i]!='\\') i--;

  if (
#ifdef _NOT_UNDER_WINDOWS
      temp[i]=='/'
#else
      temp[i]=='\\'
#endif
      ) {
    // if we have an absolute path, we care for the slash or backslash
    for (j=i+1;j<l;j++) s2[j-i-1]=temp[j];
    for (j=0;j<i+1;j++) donnees->chemin_graphe_principal[j]=temp[j];
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

int compilation(char *nom_graphe_principal,int mode,Alphabet* alph,FILE* fs_comp)
{
 int compteur=0;
 int comp;

  if (extraire_nom_graphe_comp(nom_graphe_principal,donnees->nom_graphe[0]) == 0) return 0;
  
  ajouter_graphe_comp(donnees->nom_graphe[compteur],rac_graphe_comp,0);
  do
  {
    comp = compiler_graphe_comp(compteur,mode,alph,fs_comp);
   if ((comp == 0) && (compteur == 0)) return 0;
   if (comp == -1) return 0;
   compteur++;
  }
  while ((nombre_graphes_comp < NOMBRE_GRAPHES_COMP) && (compteur < nombre_graphes_comp));

  if (nombre_graphes_comp >= NOMBRE_GRAPHES_COMP)
  {
    error("There are too many graphs. The number of graphs should be lower than %d\n",NOMBRE_GRAPHES_COMP);
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

  for (i=0;i<NOMBRE_GRAPHES_COMP;i++)
    {
      donnees->statut_graphe[i]=-1;
      donnees->nom_graphe[i][0]='\0';
    }
    nombre_graphes_comp=0;
}


/**
 * print the final number of graphes
 * to the beginning of .fst2 file
 */
void ecrire_fichier_sortie_nb_graphes(char name[],FILE* fs_comp)
{
  FILE *f;
  int i,n;
  i=2+9*2; // *2 because of unicode, +2 because of FF FE at file start
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


/**
 * Met dans fichier de sortie la liste des etiquettes
 */
void sauvegarder_etiquettes_comp(FILE* fs_comp)
{
  int i;

  for (i=0;i<nombre_etiquettes_comp;i++) {
    if ((donnees->Etiquette_comp[i][0])=='@') {
      if ((donnees->Etiquette_comp[i][1])=='\0') {
        u_fprints_char("%@\n",fs_comp);
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



