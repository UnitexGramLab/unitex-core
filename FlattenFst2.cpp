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
#include "FlattenFst2.h"
#include "LocateConstants.h"
//---------------------------------------------------------------------------


struct liste_nombres** dependences;
int* new_graph_number;

//
// this function flattens the grammar origin according to the depth parameter
// and stores the resulting grammar in a file named temp
//
int flatten_fst2(Automate_fst2* origin,int depth,char* temp,int RTN) {
FILE* res=u_fopen(temp,U_WRITE);
if (res==NULL) {
   fprintf(stderr,"Cannot create %s\n",temp);
   return FLATTEN_ERROR;
}

// set globals used in Grf2Fst2lib.cpp !!!
nombre_etiquettes_comp = origin->nombre_etiquettes;
nombre_graphes_comp = origin->nombre_graphes;


dependences=(struct liste_nombres**)malloc((1+origin->nombre_graphes)*sizeof(struct liste_nombres*));
new_graph_number=(int*)malloc((1+origin->nombre_graphes)*sizeof(int));
printf("Computing grammar dependences...\n");
compute_dependences(origin); // make dependency tree of the grammar
check_for_graphs_to_keep(origin,depth);
int n_graphs_to_keep=renumber_graphs_to_keep(origin);
#ifdef DEBUG
print_dependences(origin); // DEBUG
#endif // DEBUG
struct flattened_main_graph_info* new_main_graph=new_flattened_main_graph_info();
printf("Flattening...\n");
int result=flatten_main_graph(origin,depth,new_main_graph,RTN); /* incorporate subgraphs
                                                                   into main graph */
printf("Cleaning graph...\n");
compute_reverse_transitions_of_main_graph(new_main_graph);
for (int h=0;h<new_main_graph->current_pos;h++) {
   if (new_main_graph->states[h]->controle & 1) {
      // we start the co_accessibility check from every final state
      co_accessibilite_comp(new_main_graph->states,h);
   }
}
accessibilite_comp(new_main_graph->states,0);
virer_epsilon_transitions_comp(new_main_graph->states,new_main_graph->current_pos);
eliminer_etats_comp(new_main_graph->states,&(new_main_graph->current_pos));

char TEMP[1000];
sprintf(TEMP,"%010d\n-1 flattened version of graph ",RTN?n_graphs_to_keep:1);
u_fprints_char(TEMP,res);
u_fprints(origin->nom_graphe[1],res);
u_fprints_char("\n",res);
printf("Determinisation...\n");
determinisation_new_main_graph(res,new_main_graph->states); /* determize and
                                                               write the new main graph */
if (RTN) {
  save_graphs_to_keep(origin,res); // write the still remaining subgraphs
}
printf("Saving tags...\n");
copy_tags_into_file(origin,res); // copy the terminal symbols
u_fclose(res);
// liberation of the dependence structures
for (int i=1;i<=origin->nombre_graphes;i++) {
    free_liste_nombres(dependences[i]);
}
free(dependences);
free(new_graph_number);
free_flattened_main_graph_info(new_main_graph);
return result;
}



//
// this function compute for each subgraph of a grammar its subgraph list
//
void compute_dependences(Automate_fst2* grammar) {
for (int i=1;i<=grammar->nombre_graphes;i++) {
   dependences[i]=NULL;
   compute_dependences_for_subgraph(grammar,i,&dependences[i]);
}
}



//
// this function compute for the subgraph n of the grammar its subgraph list
//
void compute_dependences_for_subgraph(Automate_fst2* grammar,int n,struct liste_nombres** L) {
int limite=grammar->debut_graphe_fst2[n]+grammar->nombre_etats_par_grf[n];
for (int etat=grammar->debut_graphe_fst2[n];etat<limite;etat++) {
   struct fst2Transition* trans=grammar->etat[etat]->transitions;
   while (trans!=NULL) {
      if (trans->tag_number<0) {
         // if we find a reference to a subgraph, we store it in the list
         *L=inserer_dans_liste_nombres(-(trans->tag_number),*L);
      }
      trans=trans->next;
   }
}
}



//
// this function prints the grammar dependences. n is the number of subgraphs
//
void print_dependences(Automate_fst2* grammar) {
for (int i=1;i<=grammar->nombre_graphes;i++) {
   if (dependences[i]!=NULL) {
      printf("graph %d ",i);
      u_prints(grammar->nom_graphe[i]);
      printf(" calls:\n");
      struct liste_nombres* l=dependences[i];
      while (l!=NULL) {
         printf("   graph %d ",l->n);
         u_prints(grammar->nom_graphe[l->n]);
         printf("\n");
         l=l->suivant;
      }
   }
}
}



//
// this function checks if the subgraphs of the graph N must be kept
// in the resulting fst2
//
void check_if_subgraphs_must_be_kept(int N,int depth,int max_depth) {
// if we have not overpassed the maximum flattening depth,
// we just go on
if (depth<=max_depth) {
   struct liste_nombres* l=dependences[N];
   while (l!=NULL) {
      check_if_subgraphs_must_be_kept(l->n,depth+1,max_depth);
      l=l->suivant;
   }
}
// if we have reached the maximum flattening depth,
else {
   if (new_graph_number[N]==0) {
      // we look there only if we hadn't computed this graph yet
      new_graph_number[N]=1;
      struct liste_nombres* l=dependences[N];
      while (l!=NULL) {
         check_if_subgraphs_must_be_kept(l->n,depth+1,max_depth);
         l=l->suivant;
      }
   }
}
}



//
// this function explores the dependences to determine the graphs which
// will remain in the resulting fst2
//
void check_for_graphs_to_keep(Automate_fst2* grammar,int depth) {
// the main graph must always be kept
new_graph_number[1]=1;
for (int i=2;i<=grammar->nombre_graphes;i++) {
   new_graph_number[i]=0;
}
check_if_subgraphs_must_be_kept(1,0,depth);
}



//
// this function renumerote the graphs to be kept: if the graph 17 must
// be renumbered to 8 then new_graph_number[17]=8.
// 0 means that the graph won't be kept 
//
int renumber_graphs_to_keep(Automate_fst2* grammar) {
int j=2;
int N=1;
for (int i=2;i<=grammar->nombre_graphes;i++) {
   if (new_graph_number[i]!=0) {
      new_graph_number[i]=j++;
      N++;
   }
}
return N;
}



//
// creates a new flattened_main_graph_info object
//
struct flattened_main_graph_info* new_flattened_main_graph_info() {
struct flattened_main_graph_info* tmp=(struct flattened_main_graph_info*)malloc(sizeof(struct flattened_main_graph_info));
tmp->current_pos=0;
tmp->size=100000;
tmp->states=(Etat_comp*)malloc(tmp->size*sizeof(Etat_comp));
return tmp;
}



//
// frees a flattened_main_graph_info object
//
void free_flattened_main_graph_info(struct flattened_main_graph_info* tmp) {
if (tmp==NULL) return;
for (int i=0;i<tmp->current_pos;i++) {
   if (tmp->states[i]!=NULL) {
       liberer_etat_graphe_comp(tmp->states[i]);
   }
}
free(tmp);
}



//
// this function build an array of states that represents the flattened
// version of the main graph. The return value indicates if the result is
// an equivalent RTN, an equivalent FST or an FST approximation
//
int flatten_main_graph(Automate_fst2* grammar,int max_depth,
                        struct flattened_main_graph_info* new_main_graph,
                        int RTN) {
struct transition_comp* tab[1000]; // array for subgraphs
int pos_in_tab=0;

// first, we copy the original states
int limite=grammar->debut_graphe_fst2[1]+grammar->nombre_etats_par_grf[1];
for (int i=grammar->debut_graphe_fst2[1]; i<limite; i++) {
    new_main_graph->states[new_main_graph->current_pos]=nouvel_etat_comp();
    Etat_comp etat=new_main_graph->states[new_main_graph->current_pos];
    Fst2State E=grammar->etat[i];
    (new_main_graph->current_pos)++;
    if ((E->control) & FST2_FINAL_STATE_BIT_MASK) {
       // if the original state is terminal, then the new state must be so
       etat->controle=(unsigned char)((etat->controle) | 1);
    }
    struct fst2Transition* l=E->transitions;
    while (l!=NULL) {
       struct transition_comp* temp=nouvelle_transition_comp();
       temp->etiq=l->tag_number;
       // to compute the arr value, we must consider the relative value
       // of l->arr which is (l->arr)-grammar->debut_graphe_fst2[1]
       // and add to it the starting position of the current graph
       // which is 0 for the main graph
       temp->arr=0+(l->state_number)-grammar->debut_graphe_fst2[1];
       temp->suivant=etat->trans;
       if ((temp->etiq) < 0) {
          // if the transition is a reference to a sub-graph
          // we note it in order the modify it later
          tab[pos_in_tab++]=temp;
       }
       etat->trans=temp;
       l=l->next;
    }
}
new_main_graph->states[0]->controle=(unsigned char)((new_main_graph->states[0]->controle) | 2);
int SUBGRAPH_CALL_IGNORED=0;
int SUBGRAPH_CALL=0;
// then, if there were some calls to subgraphs, we copy them
for (int i=0;i<pos_in_tab;i++) {
   // we flatten recursively the subgraph
   int starting_pos=flatten_sub_graph_recursively(grammar,-(tab[i]->etiq),1,max_depth,
                                                  new_main_graph,tab[i]->arr,RTN,
                                                  &SUBGRAPH_CALL_IGNORED,&SUBGRAPH_CALL);
   // and we replace the subgraph call by an epsilon transition to the initial state
   // of the flattened subgraph
   tab[i]->etiq=0;
   tab[i]->arr=starting_pos;
}


#ifdef DEBUG
printf("graphe 1:\n");
for (int i=grammar->debut_graphe_fst2[1];i<limite;i++) {
    if (new_main_graph->states[i]->control & 1) printf("t ");
    else printf(": ");
    struct transition_comp* l=new_main_graph->states[i]->transitions;
    while (l!=NULL) {
        printf("%d %d ",l->etiq,l->state_number);
        l=l->next;
    }
    printf("\n");
}
#endif // DEBUG

if (SUBGRAPH_CALL) {
   return EQUIVALENT_RTN;
}
else {
   if (SUBGRAPH_CALL_IGNORED) {
      return APPROXIMATIVE_FST;
   }
   else {
      return EQUIVALENT_FST;
   }
}
}



//
// this function copies a subgraph. If necessary, it goes on recursively
// in the subgraphs called the graph N
//
int flatten_sub_graph_recursively(Automate_fst2* grammar,int N,int depth,int max_depth,
                                  struct flattened_main_graph_info* new_main_graph,
                                  int arr,int RTN,int* SUBGRAPH_CALL_IGNORED,int* SUBGRAPH_CALL) {
int initial_pos=new_main_graph->current_pos;
if ((new_main_graph->size)-(new_main_graph->current_pos) < 10000) {
   // if necessary, we reallocate the states array
   new_main_graph->size=new_main_graph->size+10000;
   new_main_graph->states=(Etat_comp*)realloc(new_main_graph->states,
                                             new_main_graph->size*sizeof(Etat_comp));
}

struct transition_comp* tab[1000];
int pos_in_tab=0;

// first, we copy the original states
int limite=grammar->debut_graphe_fst2[N]+grammar->nombre_etats_par_grf[N];
for (int i=grammar->debut_graphe_fst2[N];i<limite;i++) {
    new_main_graph->states[new_main_graph->current_pos]=nouvel_etat_comp();
    Etat_comp etat=new_main_graph->states[new_main_graph->current_pos];
    Fst2State E=grammar->etat[i];
    (new_main_graph->current_pos)++;
    if ((E->control) & FST2_FINAL_STATE_BIT_MASK) {
       // if the original state is terminal, then we must add an epsilon transition
       // pointing to the arr specified in parameter
       struct transition_comp* temp=nouvelle_transition_comp();
       temp->etiq=0;
       temp->arr=arr;
       temp->suivant=etat->trans;
       etat->trans=temp;
    }
    struct fst2Transition* l=E->transitions;
    while (l!=NULL) {
       if (RTN || ((l->tag_number>=0) || (depth<max_depth))) {
          // if we must produce a FST and if we have a subgraph call
          struct transition_comp* temp=nouvelle_transition_comp();
          temp->etiq=l->tag_number;
          // to compute the arr value, we must consider the relative value
          // of l->arr which is (l->arr)-grammar->debut_graphe_fst2[1]
          // and add to it the starting position of the current graph
          // which is 0 for the main graph
          temp->arr=initial_pos+(l->state_number)-grammar->debut_graphe_fst2[N];
          temp->suivant=etat->trans;
          if ((temp->etiq) < 0) {
             // if the transition is a reference to a sub-graph
             if (depth<max_depth) {
                // if we must flatten:
                // we note it in order the modify it later
                tab[pos_in_tab++]=temp;
             }
             else {
                // if we have overpassed the maximum depth,
                // we just produce a call to the subgraph taking
                // care of the graph renumerotation
                temp->etiq=-(new_graph_number[-(l->tag_number)]);
                (*SUBGRAPH_CALL)++;
             }
          }
          etat->trans=temp;
       }
       else {
          (*SUBGRAPH_CALL_IGNORED)++;
       }
       l=l->next;
    }
}
// then, if there were some calls to subgraphs, we copy them
for (int i=0;i<pos_in_tab;i++) {
   // we flatten recursively the subgraph
   int starting_pos=flatten_sub_graph_recursively(grammar,-(tab[i]->etiq),depth+1,max_depth,new_main_graph,tab[i]->arr,RTN,SUBGRAPH_CALL_IGNORED,SUBGRAPH_CALL);
   // and we replace the subgraph call by an epsilon transition to the initial state
   // of the flattened subgraph
   tab[i]->etiq=0;
   tab[i]->arr=starting_pos;
}

// DEBUG
/*
printf("-------------------------------\n");
printf("graphe %d aplati:\n",N);
for (int i=initial_pos;i<initial_pos+grammar->nombre_etats_par_grf[N];i++) {
    if (new_main_graph->states[i]->controle & 1) printf("t ");
    else printf(": ");
    struct transition_comp* l=new_main_graph->states[i]->trans;
    while (l!=NULL) {
        printf("%d %d ",l->etiq,l->arr);
        l=l->suivant;
    }
    printf("\n");
}
*/

return initial_pos;
}



void remove_epsilon_transitions_in_flattened_graph(struct flattened_main_graph_info* graph) {
}



//
// this function add a transition to a state
//
void add_reverse_transition_to_state(int etiq,int dest,Etat_comp e) {
struct transition_comp* l=nouvelle_transition_comp();
l->etiq=etiq;
l->arr=dest;
if (e==NULL) {
   fprintf(stderr,"Internal problem in add_reverse_transition_to_state\n");
   exit(1);
}
l->suivant=e->transinv;
e->transinv=l;
}



//
// this function creates for each transition of the graph the corresponding
// reverse one
//
void compute_reverse_transitions_of_main_graph(struct flattened_main_graph_info* graph) {
for (int i=0;i<graph->current_pos;i++) {
   struct transition_comp* l=graph->states[i]->trans;
   while (l!=NULL) {
      add_reverse_transition_to_state(l->etiq,i,graph->states[l->arr]);
      l=l->suivant;
   }
}
}



//
// this function makes the graph/automaton "graphe" deterministic and saves it to file "fs_comp"
//
int determinisation_new_main_graph(FILE* fs_comp,Etat_comp graphe[]) {

  if (graphe[0] == NULL) { // do not segfault on empty automaton
    fprintf(stderr, "warning: resulting automaton is empty\n");
    u_fprintf(fs_comp, ": \nf \n");
    return 1;
  }

  Transition_comp ptr;
  ensemble_det courant;
  unsigned long int q;  //etat courant ancien graphe
  int count;  //compteur pour savoir ou l'on se trouve dans notre int de 32 bits
  int compteur; //compteur pour savoir l'indice du dernier ensemble rentre dans stock;
  int num;
  int i, file_courant=0, k;
  int temp, dernier_etat_res;
  int temp2, sous_graphe;
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
  while (resultat[temp2] != NULL)
    {
      courant = resultat[temp2]->ens;
      init_hachage_det(hachage);
      compteur = 0;
      while (courant != NULL)
        {
          count = 0;
          q = (courant->num_char*32) - 1;
          while (count < 32)
            {
              q++;
              if (((courant->valeur)&(1<<count))!=0)
                {
                  ptr = graphe[q]->trans;
                  while (ptr != NULL)
                    {
                      temp = ptr->etiq;

                      if (temp < 0)
                        {
                          temp = nombre_etiquettes_comp - 1 - temp;
                          sous_graphe = 1;
                        }
                      else
                        sous_graphe = 0;

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
                      ajouter_etat_dans_ensemble_det(ptr->arr,&stock[hachage[temp]]);
                      if (((graphe[ptr->arr]->controle) & 1 ) != 0)
                        final[hachage[temp]] = 1;    //test de finalite
                      ptr = ptr->suivant;
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
                      //free_comp(stock[i]);
                      liberer_char_etat_det(stock[i]);
                    }
                  }
                  fprintf(stderr,"Too many states in main automaton: cannot determinize.\n");
                  exit(1);
                }
              resultat[temp] = nouvel_etat_mat_det();
              dernier_etat_res++;
              resultat[temp]->ens = copie_det(stock[i]);

              resultat[temp]->controle = (unsigned char)((resultat[temp]->controle) | final[i]);
            }
        }
      sauvegarder_etat_det(fs_comp,resultat[temp2]); // print all transitions of one state
      liberer_etat_det(resultat[temp2]);
      resultat[temp2] = NULL;
      file_courant++;
      temp2 = file_courant % NBRE_ET;
    }
  char s[10];
  sprintf(s,"f \n");
  u_fprints_char(s,fs_comp);
  liberer_arbre_det(racine_det);
  for (i=0;i < NBRE_ETIQ_TRANSITION_COMP;i++) {
    if (stock[i]!=NULL) {
      // free_comp(stock[i]);
      liberer_char_etat_det(stock[i]);
    }
  }
  return 1;
}




//
// this function saves each graph that have been marked to be kept
//
void save_graphs_to_keep(Automate_fst2* grammar,FILE* f) {
for (int i=2;i<=grammar->nombre_graphes;i++) {
   if (new_graph_number[i]!=0) {
      save_graph_to_be_kept(i,grammar,f);
   }
}
}



//
// this function saves a graph that had been marked to be kept,
// taking into account graph renumerotation
//
void save_graph_to_be_kept(int N,Automate_fst2* grammar,FILE* f) {
int limite=grammar->debut_graphe_fst2[N]+grammar->nombre_etats_par_grf[N];
char temp[1000];
sprintf(temp,"%d ",-new_graph_number[N]);
u_fprints_char(temp,f);
u_fprints(grammar->nom_graphe[N],f);
u_fprints_char("\n",f);
for (int i=grammar->debut_graphe_fst2[N];i<limite;i++) {
   if (grammar->etat[i]->control & FST2_FINAL_STATE_BIT_MASK) {
      u_fprints_char("t ",f);
   }
   else {
      u_fprints_char(": ",f);
   }
   struct fst2Transition* l=(grammar->etat[i])->transitions;
   while (l!=NULL) {
      if (l->tag_number < 0) {
         sprintf(temp,"%d %d ",-new_graph_number[-(l->tag_number)],(l->state_number)-grammar->debut_graphe_fst2[N]);
      }
      else {
         sprintf(temp,"%d %d ",l->tag_number,(l->state_number)-grammar->debut_graphe_fst2[N]);
      }
      u_fprints_char(temp,f);
      l=l->next;
   }
   u_fprints_char("\n",f);
}
u_fprints_char("f \n",f);
}



//
// copy the tag list of the grammar into the file f
//
void copy_tags_into_file(Automate_fst2* grammar, FILE* f) {
for (int i=0; i<grammar->nombre_etiquettes; i++) {
   if (grammar->etiquette[i]->control & RESPECT_CASE_TAG_BIT_MASK) {
      u_fprints_char("@",f);
   }
   else {
      u_fprints_char("%",f);
   }
   // if the tag is a variable, print '$'
   if (grammar->etiquette[i]->control & (START_VAR_TAG_BIT_MASK|END_VAR_TAG_BIT_MASK)) {
     u_fprints_char("$",f);
   }
   // print the content (label) of the tag
   u_fprints(grammar->etiquette[i]->input,f);
   // if any, we add the morphological filter: <A><<^pre>>
   if (grammar->etiquette[i]->contentGF!=NULL &&
       grammar->etiquette[i]->contentGF[0]!='\0') {
     u_fprints(grammar->etiquette[i]->contentGF,f);
   }
   // if any, we add transitions
   if (grammar->etiquette[i]->output!=NULL &&
       grammar->etiquette[i]->output[0]!='\0') {
     u_fprints_char("/",f);
     u_fprints(grammar->etiquette[i]->output,f);
   }
   // print closing '(' for variables
   else if (grammar->etiquette[i]->control & START_VAR_TAG_BIT_MASK) {
     u_fprints_char("(",f);
   }
   // or ')' resp.
   else if (grammar->etiquette[i]->control & END_VAR_TAG_BIT_MASK) {
     u_fprints_char(")",f);
   }
   u_fprints_char("\n",f);
}
u_fprints_char("f\n",f);
}

