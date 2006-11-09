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
#include "List_int.h"
#include "Grf2Fst2_lib.h"
//---------------------------------------------------------------------------


static struct list_int** dependencies;

/**
 * During the flatten operation, some graphs may be kept and some
 * may not. We use the 'new_graph_number' array to know for each graph
 * if it will be kept. new_graph_number[k]==0 means that the graph number k
 * won't be kept; otherwise, new_graph_number[k]==i means that the graph 
 * number k will have the number i in the flattened fst2.
 */
static int* new_graph_number;

struct list_int** compute_dependencies(Fst2*);
void compute_dependencies_for_subgraph(Fst2*,int,struct list_int**);
void print_dependencies(Fst2*);
int* check_for_graphs_to_keep(Fst2*,int);
int renumber_graphs_to_keep(Fst2*);
int flatten_graph(Fst2*,int,int,int,
                  Graph_comp,
                  int,int,int*,int*);
void remove_epsilon_transitions_in_flattened_graph(Graph_comp);
void save_graphs_to_keep(Fst2*,FILE*);
void save_graph_to_be_kept(int,Fst2*,FILE*);
void copy_tags_into_file(Fst2*,FILE*);


/**
 * Flattens the grammar 'origin' according to the 'depth' parameter
 * and stores the resulting grammar in a file named 'temp'.
 */
int flatten_fst2(Fst2* origin,int depth,char* temp,int RTN) {
FILE* res = u_fopen(temp,U_WRITE);
if (res==NULL) {
   error("Cannot create %s\n",temp);
   return FLATTEN_ERROR;
}
/* We set globals used in Grf2Fst2lib.cpp !!! */
nombre_etiquettes_comp = origin->number_of_tags;
nombre_graphes_comp = origin->number_of_graphs;
printf("Computing grammar dependencies...\n");
/* We build the dependency tree of the grammar */
dependencies=compute_dependencies(origin);
new_graph_number=check_for_graphs_to_keep(origin,depth);
int n_graphs_to_keep = renumber_graphs_to_keep(origin);
#ifdef DEBUG
  print_dependencies(origin);
#endif
/* We do the flattening job */
printf("Flattening...\n");
Graph_comp new_main_graph=new_graph_comp();
int SUBGRAPH_CALL_IGNORED=0,SUBGRAPH_CALL=0;
int result=flatten_graph(origin,
                           1, // start with graph number 1 (main graph)
                           0, // start with depth 0
                           depth, // maximal depth
                           new_main_graph,
                           0,
                           RTN,
                           &SUBGRAPH_CALL_IGNORED,
                           &SUBGRAPH_CALL); 
/* And we free liberation of the dependency structures */
for (int i=1;i<=origin->number_of_graphs;i++) {
   free_list_int(dependencies[i]);
}
free(dependencies);
/* clean graph, i.e. remove epsilon transitions and unreachable states */
printf("Cleaning graph...\n");
compute_reverse_transitions(new_main_graph->states,new_main_graph->n_states);
for (int h=0;h<new_main_graph->n_states;h++) {
   if (is_final_state(new_main_graph->states[h])) {
      /* We start the co_accessibility check from every final state */
      co_accessibilite_comp(new_main_graph->states,h);
   }
}
accessibilite_comp(new_main_graph->states,0);
virer_epsilon_transitions_comp(new_main_graph->states,new_main_graph->n_states);
eliminer_etats_comp(new_main_graph->states,&(new_main_graph->n_states));
/* Print header (number of graphs) */
char tmpstr[256];
snprintf(tmpstr,256,"%010d\n",(RTN?n_graphs_to_keep:1));
u_fprints_char(tmpstr,res);
/* Determinize and minimize the new main graph */
printf("Determinisation...\n");
determinisation(new_main_graph);
printf("Minimisation...\n");
minimisation(new_main_graph);
/* Write the new main graph */
printf("Writing grammar...\n");
write_graph_comp(res,new_main_graph,-1,origin->graph_names[1]);
free_graph_comp(new_main_graph);

if (RTN && (result == EQUIVALENT_RTN)) {
   printf("Saving remaining subgraphs...\n");
   save_graphs_to_keep(origin,res); // write the still remaining subgraphs
}
free(new_graph_number);
printf("Saving tags...\n");
write_fst2_tags(res,origin); // copy the terminal symbols to the resulting file
u_fclose(res);
return result;
}


/**
 * This function computes for each subgraph of a grammar its subgraph list.
 */
struct list_int** compute_dependencies(Fst2* grammar) {
struct list_int** dependencies=(struct list_int**)malloc(sizeof(struct list_int*)*(1+grammar->number_of_graphs));
if (dependencies==NULL) {
   fatal_error("Not enough memory in compute_dependencies\n");
}
for (int i=1;i<=grammar->number_of_graphs;i++) {
   dependencies[i]=NULL;
   compute_dependencies_for_subgraph(grammar,i,&dependencies[i]);
}
return dependencies;
}


/**
 * This function computes for the subgraph 'n' of the grammar its subgraph list.
 */
void compute_dependencies_for_subgraph(Fst2* grammar,int n,struct list_int** L) {
int last_state=grammar->initial_states[n]+grammar->number_of_states_per_graphs[n];
for (int state=grammar->initial_states[n];state<last_state;state++) {
   struct fst2Transition* trans=grammar->states[state]->transitions;
   while (trans!=NULL) {
      if (trans->tag_number<0) {
         /* If we find a reference to a subgraph, we store it in the list */
         *L=sorted_insert(-(trans->tag_number),*L);
      }
      trans = trans->next;
   }
}
}


/**
 * This function prints the grammar dependencies.
 */
void print_dependencies(Fst2* grammar) {
for (int i=1;i<=grammar->number_of_graphs;i++) {
   if (dependencies[i]!=NULL) {
      printf("graph %d ",i);
      u_prints(grammar->graph_names[i]);
      printf(" calls:\n");
      struct list_int* l=dependencies[i];
      while (l!=NULL) {
         printf("   graph %d ",l->n);
         u_prints(grammar->graph_names[l->n]);
         printf("\n");
         l=l->next;
      }
   }
}
}


/**
 * This function checks if the subgraphs of the graph 'N' must be kept
 * in the resulting fst2. Its effect is to set new_graph_number[k] to
 * 1 if the graph number k must be kept.
 */
void check_if_subgraphs_must_be_kept(int N,int depth,int max_depth) {
/* If we have not overpassed the maximum flattening depth,
 * we just go on */
if (depth<=max_depth) {
   struct list_int* l=dependencies[N];
   while (l!=NULL) {
      check_if_subgraphs_must_be_kept(l->n,depth+1,max_depth);
      l=l->next;
   }
}
/* If we have reached the maximum flattening depth */
else {
   if (new_graph_number[N]==0) {
      /* We look there only if we hadn't computed this graph yet */
      new_graph_number[N]=1;
      struct list_int* l=dependencies[N];
      while (l!=NULL) {
         check_if_subgraphs_must_be_kept(l->n,depth+1,max_depth);
         l=l->next;
      }
   }
}
}


/**
 * This function explores the dependencies to determine the graphs which
 * will remain in the resulting fst2. It returns an array indicating for
 * each graph if it must be kept (value=1) or not (value=0).
 */
int* check_for_graphs_to_keep(Fst2* grammar,int depth) {
int* new_graph_number=(int*)malloc((1+grammar->number_of_graphs)*sizeof(int));
if (new_graph_number==NULL) {
   fatal_error("Not enough memory in check_for_graphs_to_keep\n");
}
/* The main graph must always be kept */
new_graph_number[1]=1;
for (int i=2;i<=grammar->number_of_graphs;i++) {
   new_graph_number[i]=0;
}
check_if_subgraphs_must_be_kept(1,0,depth);
return new_graph_number;
}


/**
 * This function renumbers the graphs to be kept: if the graph 17 must
 * be renumbered to 8 then new_graph_number[17]=8.
 * 0 means that the graph won't be kept.
 */
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


/**
 * This function flattens the main graph:
 *  Build an array of states that represents the flattened
 *  version of the main graph. The return value indicates if the result is
 *  an equivalent RTN, an equivalent FST or an FST approximation.
 * Recursive function: called by itself for subgraphs.
 * @param grammar the fst2 grammar to be flattened
 * @param n_graph number of actually treated graph 
 * @param depth actual depth
 * @param max_depth maximal depth until which the grammar should be flattened
 * @param new_main_graph the resulting new grammar
 * @param arr arrival states to which a subgraph (to be incorporated) leds
 * @param SUBGRAPH_CALL_IGNORED will be > 0 if there are subgraphs ignored (passed by)
 *  in the grammar, i.e. the grammar is a finite-state approximization
 * @param SUBGRAPH_CALL will be > 0 if there are still subgraphs in the grammar,
 *  i.e. the grammar is a RTN
 * @return int : returns starting position for subgraphs;
 *               type of resulting graph (RTN, FST, FST-approximation) for main graph
 */
int flatten_graph (Fst2* grammar, int n_graph,
                   int depth, int max_depth,
                   Graph_comp new_main_graph,
                   int arr, int RTN,
                   int* SUBGRAPH_CALL_IGNORED, int* SUBGRAPH_CALL) {

int initial_pos = new_main_graph->n_states;

/* tab contains list of all subgraphs to be incorporated */
int resize_of_tab_step = 0x400;
int size_of_tab = resize_of_tab_step;
fst2Transition** tab = 
  (struct fst2Transition**) malloc(size_of_tab * sizeof(fst2Transition));
int pos_in_tab = 0;

// first, we copy the original states
int limite = grammar->initial_states[n_graph]+grammar->number_of_states_per_graphs[n_graph];
while ( new_main_graph->size < (new_main_graph->n_states + limite) ) {
  // if necessary, resize states array
  resize_graph_comp(new_main_graph);
}
for (int i=grammar->initial_states[n_graph];i<limite;i++) {
    new_main_graph->states[new_main_graph->n_states] = nouvel_etat_comp();
    Etat_comp etat = new_main_graph->states[new_main_graph->n_states];
    Fst2State E = grammar->states[i];
    (new_main_graph->n_states)++;
    if (is_final_state(E)) {
      // if the original state is terminal
      if (n_graph == 1) {
        // in the main graph: new state must be also terminal
       etat->controle = (unsigned char)((etat->controle) | 1);
      } else {
        // in a subgraph:
        // we add an epsilon transition pointing to the arr specified in parameter
        Fst2Transition temp = new_Fst2Transition();
        temp->tag_number = 0;
        temp->state_number = arr;
        temp->next = etat->trans;
        etat->trans = temp;
      }
    }
    struct fst2Transition* l = E->transitions;
    while (l!=NULL) {
       if (RTN || ((l->tag_number>=0) || (depth<max_depth))) {
          // if we must produce a FST and if we have a subgraph call
          Fst2Transition temp = new_Fst2Transition();
          temp->tag_number = l->tag_number;
          // to compute the arr value, we must consider the relative value
          // of l->state_number which is (l->state_number)-grammar->debut_graphe_fst2[1]
          // and add to it the starting position of the current graph
          // which is 0 for the main graph
          temp->state_number = initial_pos + (l->state_number) - grammar->initial_states[n_graph];
          temp->next = etat->trans;
          if ((temp->tag_number) < 0) {
             // if the transition is a reference to a sub-graph
             if (depth<max_depth) {
                // if we must flatten:
                // we note it in order the modify it later
               if (pos_in_tab >= size_of_tab) { // resize tab
                 size_of_tab += resize_of_tab_step;
                 tab = (fst2Transition**) realloc(tab, (size_of_tab * sizeof(fst2Transition*)));
               }
               tab[pos_in_tab++]=temp;
             }
             else {
                // if we have overpassed the maximum depth,
                // we just produce a call to the subgraph taking
                // care of the graph renumerotation
                temp->tag_number = -(new_graph_number[-(l->tag_number)]);
                (*SUBGRAPH_CALL)++;
             }
          }
          etat->trans = temp;
       }
       else {
          (*SUBGRAPH_CALL_IGNORED)++;
       }
       l = l->next;
    }
}

 if (n_graph == 1) // in main graph
   new_main_graph->states[0]->controle
     = (unsigned char) ((new_main_graph->states[0]->controle) | 2);


// then, if there were some calls to subgraphs, we copy them
for (int i=0;i<pos_in_tab;i++) {
   // we flatten recursively the subgraph
   int starting_pos = flatten_graph(grammar,-(tab[i]->tag_number),
                                    depth+1,max_depth,
                                    new_main_graph,
                                    tab[i]->state_number,RTN,
                                    SUBGRAPH_CALL_IGNORED,SUBGRAPH_CALL);
   // and we replace the subgraph call by an epsilon transition to the initial state
   // of the flattened subgraph
   tab[i]->tag_number = 0;
   tab[i]->state_number = starting_pos;
}

/* clean up */
free(tab);

if (n_graph == 1) { // in main graph
  if (*SUBGRAPH_CALL)
    return EQUIVALENT_RTN;
  else {
    if (*SUBGRAPH_CALL_IGNORED)
      return APPROXIMATIVE_FST;
    else
      return EQUIVALENT_FST;
  }
}
else // in a subgraph
  return initial_pos;

}




void remove_epsilon_transitions_in_flattened_graph(Graph_comp graph) {
}








/**
 * this function saves each graph that have been marked to be kept
 */
void save_graphs_to_keep(Fst2* grammar,FILE* f) {
for (int i=2;i<=grammar->number_of_graphs;i++) {
   if (new_graph_number[i]!=0) {
      save_graph_to_be_kept(i,grammar,f);
   }
}
}



/**
 * this function saves a graph that had been marked to be kept,
 * taking into account renumbering of graphs
 */
void save_graph_to_be_kept(int N,Fst2* grammar,FILE* f) {
int limite = grammar->initial_states[N]+grammar->number_of_states_per_graphs[N];
char temp[1000];
sprintf(temp,"%d ",-new_graph_number[N]);
u_fprints_char(temp,f);
u_fprints(grammar->graph_names[N],f);
u_fprints_char("\n",f);
for (int i = grammar->initial_states[N];i<limite;i++) {
   if (is_final_state(grammar->states[i])) {
      u_fprints_char("t ",f);
   }
   else {
      u_fprints_char(": ",f);
   }
   struct fst2Transition* l = (grammar->states[i])->transitions;
   while (l!=NULL) {
      if (l->tag_number < 0) {
         sprintf(temp,"%d %d ",-new_graph_number[-(l->tag_number)],(l->state_number)-grammar->initial_states[N]);
      }
      else {
         sprintf(temp,"%d %d ",l->tag_number,(l->state_number)-grammar->initial_states[N]);
      }
      u_fprints_char(temp,f);
      l = l->next;
   }
   u_fprints_char("\n",f);
}
u_fprints_char("f \n",f);
}




