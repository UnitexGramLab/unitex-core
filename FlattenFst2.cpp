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
int flatten_fst2(Fst2* origin,int depth,char* temp,int RTN) {
FILE* res=u_fopen(temp,U_WRITE);
if (res==NULL) {
   fprintf(stderr,"Cannot create %s\n",temp);
   return FLATTEN_ERROR;
}

// set globals used in Grf2Fst2lib.cpp !!!
nombre_etiquettes_comp = origin->number_of_tags;
nombre_graphes_comp = origin->number_of_graphs;


dependences=(struct liste_nombres**)malloc((1+origin->number_of_graphs)*sizeof(struct liste_nombres*));
new_graph_number=(int*)malloc((1+origin->number_of_graphs)*sizeof(int));
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
u_fprints(origin->graph_names[1],res);
u_fprints_char("\n",res);
printf("Determinisation...\n");
determinisation(res,new_main_graph->states); /* determize and write the new main graph */
if (RTN) {
  save_graphs_to_keep(origin,res); // write the still remaining subgraphs
}
printf("Saving tags...\n");
copy_tags_into_file(origin,res); // copy the terminal symbols
u_fclose(res);
// liberation of the dependence structures
for (int i=1;i<=origin->number_of_graphs;i++) {
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
void compute_dependences(Fst2* grammar) {
for (int i=1;i<=grammar->number_of_graphs;i++) {
   dependences[i]=NULL;
   compute_dependences_for_subgraph(grammar,i,&dependences[i]);
}
}



//
// this function compute for the subgraph n of the grammar its subgraph list
//
void compute_dependences_for_subgraph(Fst2* grammar,int n,struct liste_nombres** L) {
int limite=grammar->initial_states[n]+grammar->number_of_states_per_graphs[n];
for (int etat=grammar->initial_states[n];etat<limite;etat++) {
   struct fst2Transition* trans=grammar->states[etat]->transitions;
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
void print_dependences(Fst2* grammar) {
for (int i=1;i<=grammar->number_of_graphs;i++) {
   if (dependences[i]!=NULL) {
      printf("graph %d ",i);
      u_prints(grammar->graph_names[i]);
      printf(" calls:\n");
      struct liste_nombres* l=dependences[i];
      while (l!=NULL) {
         printf("   graph %d ",l->n);
         u_prints(grammar->graph_names[l->n]);
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
void check_for_graphs_to_keep(Fst2* grammar,int depth) {
// the main graph must always be kept
new_graph_number[1]=1;
for (int i=2;i<=grammar->number_of_graphs;i++) {
   new_graph_number[i]=0;
}
check_if_subgraphs_must_be_kept(1,0,depth);
}



//
// this function renumerote the graphs to be kept: if the graph 17 must
// be renumbered to 8 then new_graph_number[17]=8.
// 0 means that the graph won't be kept 
//
int renumber_graphs_to_keep(Fst2* grammar) {
int j=2;
int N=1;
for (int i=2;i<=grammar->number_of_graphs;i++) {
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
int flatten_main_graph(Fst2* grammar,int max_depth,
                        struct flattened_main_graph_info* new_main_graph,
                        int RTN) {
struct transition_comp* tab[5000]; // array for subgraphs
int pos_in_tab=0;

// first, we copy the original states
int limite=grammar->initial_states[1]+grammar->number_of_states_per_graphs[1];
for (int i=grammar->initial_states[1]; i<limite; i++) {
    new_main_graph->states[new_main_graph->current_pos]=nouvel_etat_comp();
    Etat_comp etat=new_main_graph->states[new_main_graph->current_pos];
    Fst2State E=grammar->states[i];
    (new_main_graph->current_pos)++;
    if (is_final_state(E)) {
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
       temp->arr=0+(l->state_number)-grammar->initial_states[1];
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
for (int i=grammar->initial_states[1];i<limite;i++) {
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
int flatten_sub_graph_recursively(Fst2* grammar,int N,int depth,int max_depth,
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
int limite=grammar->initial_states[N]+grammar->number_of_states_per_graphs[N];
for (int i=grammar->initial_states[N];i<limite;i++) {
    new_main_graph->states[new_main_graph->current_pos]=nouvel_etat_comp();
    Etat_comp etat=new_main_graph->states[new_main_graph->current_pos];
    Fst2State E=grammar->states[i];
    (new_main_graph->current_pos)++;
    if (is_final_state(E)) {
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
          temp->arr=initial_pos+(l->state_number)-grammar->initial_states[N];
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
// this function saves each graph that have been marked to be kept
//
void save_graphs_to_keep(Fst2* grammar,FILE* f) {
for (int i=2;i<=grammar->number_of_graphs;i++) {
   if (new_graph_number[i]!=0) {
      save_graph_to_be_kept(i,grammar,f);
   }
}
}



//
// this function saves a graph that had been marked to be kept,
// taking into account graph renumerotation
//
void save_graph_to_be_kept(int N,Fst2* grammar,FILE* f) {
int limite=grammar->initial_states[N]+grammar->number_of_states_per_graphs[N];
char temp[1000];
sprintf(temp,"%d ",-new_graph_number[N]);
u_fprints_char(temp,f);
u_fprints(grammar->graph_names[N],f);
u_fprints_char("\n",f);
for (int i=grammar->initial_states[N];i<limite;i++) {
   if (is_final_state(grammar->states[i])) {
      u_fprints_char("t ",f);
   }
   else {
      u_fprints_char(": ",f);
   }
   struct fst2Transition* l=(grammar->states[i])->transitions;
   while (l!=NULL) {
      if (l->tag_number < 0) {
         sprintf(temp,"%d %d ",-new_graph_number[-(l->tag_number)],(l->state_number)-grammar->initial_states[N]);
      }
      else {
         sprintf(temp,"%d %d ",l->tag_number,(l->state_number)-grammar->initial_states[N]);
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
void copy_tags_into_file(Fst2* grammar, FILE* f) {
for (int i=0; i<grammar->number_of_tags; i++) {
   if (grammar->tags[i]->control & RESPECT_CASE_TAG_BIT_MASK) {
      u_fprints_char("@",f);
   }
   else {
      u_fprints_char("%",f);
   }
   // if the tag is a variable, print '$'
   if (grammar->tags[i]->control & (START_VAR_TAG_BIT_MASK|END_VAR_TAG_BIT_MASK)) {
     u_fprints_char("$",f);
   }
   // print the content (label) of the tag
   u_fprints(grammar->tags[i]->input,f);
   // if any, we add the morphological filter: <A><<^pre>>
   if (grammar->tags[i]->contentGF!=NULL &&
       grammar->tags[i]->contentGF[0]!='\0') {
     u_fprints(grammar->tags[i]->contentGF,f);
   }
   // if any, we add transitions
   if (grammar->tags[i]->output!=NULL &&
       grammar->tags[i]->output[0]!='\0') {
     u_fprints_char("/",f);
     u_fprints(grammar->tags[i]->output,f);
   }
   // print closing '(' for variables
   else if (grammar->tags[i]->control & START_VAR_TAG_BIT_MASK) {
     u_fprints_char("(",f);
   }
   // or ')' resp.
   else if (grammar->tags[i]->control & END_VAR_TAG_BIT_MASK) {
     u_fprints_char(")",f);
   }
   u_fprints_char("\n",f);
}
u_fprints_char("f\n",f);
}

