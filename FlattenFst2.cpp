/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "FlattenFst2.h"
#include "LocateConstants.h"
#include "List_int.h"
#include "Grf2Fst2_lib.h"
#include "SingleGraph.h"
#include "Error.h"
#include "Transitions.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * During the flatten operation, some graphs may be kept and some
 * may not. We use the 'new_graph_number' array to know for each graph
 * if it will be kept. new_graph_number[k]==0 means that the graph number k
 * won't be kept; otherwise, new_graph_number[k]==i means that the graph
 * number k will have the number i in the flattened fst2.
 */

struct list_int** compute_dependencies(Fst2*);
void compute_dependencies_for_subgraph(Fst2*,int,struct list_int**);
void print_dependencies(Fst2*,struct list_int**);
int* check_for_graphs_to_keep(Fst2*,int,struct list_int**);
int renumber_graphs_to_keep(Fst2*,int*);
int flatten_graph(Fst2*,int,int,int,
                  SingleGraph,
                  int,int,int*,int*,int*);
void save_graphs_to_keep(Fst2*,U_FILE*,int*);
void save_graph_to_keep(int,Fst2*,U_FILE*,int*);


/**
 * Flattens the grammar 'origin' according to the 'depth' parameter
 * and stores the resulting grammar in a file named 'temp'. If
 * 'RTN' is non null, we want to get a strictly equivalent grammar,
 * even if it is not a strict FST.
 */
int flatten_fst2(Fst2* origin,int depth,char* temp, const VersatileEncodingConfig* vec,int RTN) {
U_FILE* res=u_fopen(vec,temp,U_WRITE);
if (res==NULL) {
   error("Cannot create %s\n",temp);
   return FLATTEN_ERROR;
}

u_printf("Original fst2: %d graphs, ",origin->number_of_graphs);
u_printf("%d states, ",origin->number_of_states);
int n=0;
for (int i=0;i<origin->number_of_states;i++) {
Transition* t=origin->states[i]->transitions; while (t!=NULL) {
n++;
t=t->next;
}
}
u_printf("%d transitions\n",n);

u_printf("Computing grammar dependencies...\n");
/* We build the dependency tree of the grammar */
struct list_int** dependencies;
dependencies=compute_dependencies(origin);
/* And we use it in order to know which graphs will be kept */
int* new_graph_number=check_for_graphs_to_keep(origin,depth,dependencies);
int n_graphs_to_keep=renumber_graphs_to_keep(origin,new_graph_number);
#ifdef DEBUG
  print_dependencies(origin,dependencies);
#endif
/* Now we can free liberation of the dependency structures */
for (int i=1;i<=origin->number_of_graphs;i++) {
   free_list_int(dependencies[i]);
}
free(dependencies);
u_printf("Flattening...\n");
/* We create the new main graph structure */
SingleGraph new_fst2=new_SingleGraph();
/* And we do the flattening job */
int SUBGRAPH_CALL_IGNORED=0;
int SUBGRAPH_CALL=0;
int result=flatten_graph(origin,
                           1, /* start with graph number 1 (main graph) */
                           0, /* start with depth 0 */
                           depth, /* maximal depth */
                           new_fst2,
                           0,
                           RTN,
                           &SUBGRAPH_CALL_IGNORED,
                           &SUBGRAPH_CALL,
                           new_graph_number);
/* Now, we clean the new main graph, i.e.we remove epsilon transitions
 * and unreachable states */
u_printf("Cleaning graph...\n");
compute_reverse_transitions(new_fst2);
for (int h=0;h<new_fst2->number_of_states;h++) {
   if (is_final_state(new_fst2->states[h])) {
      /* We start the co_accessibility check from every final state */
      check_co_accessibility(new_fst2->states,h);
   }
}
check_accessibility(new_fst2->states,0);
remove_epsilon_transitions(new_fst2,1);
remove_useless_states(new_fst2,NULL);
/* We minimize the new main graph */
u_printf("Minimization...\n");
minimize(new_fst2,0);

int number_keep_graphs=0;
int number_keep_states=0;
int number_keep_transitions=0;
if ((new_graph_number!=NULL) && RTN && (result == EQUIVALENT_RTN))
{
int i;
for (i=2;i<=origin->number_of_graphs;i++)
  if (new_graph_number[i]!=0) {
      number_keep_graphs++;
      number_keep_states+=origin->number_of_states_per_graphs[i];
      if (origin->number_of_states_per_graphs[i]<0) u_printf("_%d,%d,%u_",origin->number_of_states_per_graphs[i],origin->initial_states[i],i);
      int limit=origin->initial_states[i]+origin->number_of_states_per_graphs[i];
      for (int k=origin->initial_states[i];k<limit;k++) {
        Transition* t=origin->states[k]->transitions; while (t!=NULL) {
        number_keep_transitions++;
        t=t->next;
        }
      }
  }
}
u_printf("Resulting fst2: %d+%d=%d graphs, ",1,number_keep_graphs,1+number_keep_graphs);
u_printf("%d+%d=%d states, ",new_fst2->number_of_states,number_keep_states,new_fst2->number_of_states+number_keep_states);
int n3=0;
for (int i=0;i<new_fst2->number_of_states;i++) {
  Transition* t=new_fst2->states[i]->outgoing_transitions; while (t!=NULL) {
    n3++;
    t=t->next;
  }
}
u_printf("%d+%d=%d transitions\n",n3,number_keep_transitions,n3+number_keep_transitions);


/* Now, we can start saving the grammar, so we print the header of the .fst2,
 * which is the number of graphs it contains. */
u_fprintf(res,"%C%09d\n",(origin->debug?'d':'0'),(RTN?n_graphs_to_keep:1));
/* We save the new main graph */
u_printf("Writing grammar...\n");
save_fst2_subgraph(res,new_fst2,-1,origin->graph_names[1]);
free_SingleGraph(new_fst2,NULL);
/* Then, we save the subgraphs, if we have to */
if (RTN && (result == EQUIVALENT_RTN)) {
   u_printf("Saving remaining subgraphs...\n");
   save_graphs_to_keep(origin,res,new_graph_number);
}
/* We don't forget to free the new_graph_number array */
free(new_graph_number);
/* Finally, we save the tags */
u_printf("Saving tags...\n");
write_fst2_tags(res,origin);
u_fclose(res);
return result;
}


/**
 * This function computes for each subgraph of a grammar its subgraph list.
 */
struct list_int** compute_dependencies(Fst2* grammar) {
struct list_int** dependencies_cur=(struct list_int**)malloc(sizeof(struct list_int*)*(1+grammar->number_of_graphs));
if (dependencies_cur==NULL) {
   fatal_alloc_error("compute_dependencies");
}
for (int i=1;i<=grammar->number_of_graphs;i++) {
   dependencies_cur[i]=NULL;
   compute_dependencies_for_subgraph(grammar,i,&dependencies_cur[i]);
}
return dependencies_cur;
}


/**
 * This function computes for the subgraph 'n' of the grammar its subgraph list.
 */
void compute_dependencies_for_subgraph(Fst2* grammar,int n,struct list_int** L) {
int last_state=grammar->initial_states[n]+grammar->number_of_states_per_graphs[n];
for (int state=grammar->initial_states[n];state<last_state;state++) {
   Transition* trans=grammar->states[state]->transitions;
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
void print_dependencies(Fst2* grammar,struct list_int** dependencies) {
for (int i=1;i<=grammar->number_of_graphs;i++) {
   if (dependencies[i]!=NULL) {
      u_printf("graph %d %S calls:\n",i,grammar->graph_names[i]);
      struct list_int* l=dependencies[i];
      while (l!=NULL) {
         u_printf("   graph %d %S\n",l->n,grammar->graph_names[l->n]);
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
void check_if_subgraphs_must_be_kept(int* new_graph_number,int N,int depth,int max_depth,struct list_int** dependencies) {
/* If we have not overpassed the maximum flattening depth,
 * we just go on */
if (depth<=max_depth) {
   struct list_int* l=dependencies[N];
   while (l!=NULL) {
      check_if_subgraphs_must_be_kept(new_graph_number,l->n,depth+1,max_depth,dependencies);
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
         check_if_subgraphs_must_be_kept(new_graph_number,l->n,depth+1,max_depth,dependencies);
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
int* check_for_graphs_to_keep(Fst2* grammar,int depth,struct list_int** dependencies) {
int* new_graph_number=(int*)malloc((1+grammar->number_of_graphs)*sizeof(int));
if (new_graph_number==NULL) {
   fatal_alloc_error("check_for_graphs_to_keep");
}
/* The main graph must always be kept */
new_graph_number[1]=1;
for (int i=2;i<=grammar->number_of_graphs;i++) {
   new_graph_number[i]=0;
}
check_if_subgraphs_must_be_kept(new_graph_number,1,0,depth,dependencies);
return new_graph_number;
}


/**
 * This function renumbers the graphs to be kept: if the graph 17 must
 * be renumbered to 8 then new_graph_number[17]=8.
 * 0 means that the graph won't be kept. The fucntion
 */
int renumber_graphs_to_keep(Fst2* grammar,int* new_graph_number) {
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
 *  Builds an array of states that represents the flattened
 *  version of the main graph. The return value indicates if the result is
 *  an equivalent RTN, an equivalent FST or an FST approximation.
 * Recursive function: called by itself for subgraphs.
 * @param grammar the fst2 grammar to be flattened
 * @param n_graph number of actually treated graph
 * @param depth actual depth
 * @param max_depth maximal depth until which the grammar should be flattened
 * @param new_graph the main graph of the resulting grammar (only the main graph is flattened)
 * @param destination_state the state to which a subgraph (to be incorporated) leds
 *        for instance, if we call a transition like X --call to graph G--> Y,
 *        the state Y will be the destination state while flattening graph G
 * @param SUBGRAPH_CALL_IGNORED will be > 0 if there are subgraphs ignored (passed by)
 *  in the grammar, i.e. the grammar is a finite-state approximization
 * @param SUBGRAPH_CALL will be > 0 if there are still subgraphs in the grammar,
 *  i.e. the grammar is a RTN
 * @return int : - returns starting position for subgraphs;
 *               - type of resulting graph (RTN, FST, FST-approximation) for main graph
 */
int flatten_graph(Fst2* grammar,int n_graph,
                  int depth,int max_depth,
                  SingleGraph new_main_graph,
                  int destination_state,int RTN,
                  int *SUBGRAPH_CALL_IGNORED,int *SUBGRAPH_CALL,
                  int* new_graph_number) {
/* The new states will be appended at the end of the old graph's state array */
int initial_position_for_new_states=new_main_graph->number_of_states;
/* The following array contains the of transitions that correspond
 * to subgraph calls to be flattened. We arbitrary set its capacity to 2048 */
int trans_to_flatten_capacity=2048;
Transition** transitions_to_flatten=(Transition**)malloc(trans_to_flatten_capacity*sizeof(Transition*));
if (transitions_to_flatten==NULL) {
   fatal_alloc_error("flatten_graph");
}
int trans_to_flatten_size=0;
/* First, we copy the original states into the destination graph */
int limit=grammar->initial_states[n_graph]+grammar->number_of_states_per_graphs[n_graph];
for (int i=grammar->initial_states[n_graph];i<limit;i++) {
   Fst2State original_state = grammar->states[i];
   SingleGraphState new_state=add_state(new_main_graph);
   if (is_final_state(original_state)) {
      /* If the original state is terminal */
      if (depth==0) {
         /* If we are in the main graph, at the first step, i.e. not inside a
          * recursive call to the main graph, we say that the new state must
          * be also terminal */
         set_final_state(new_state);
      }
      else {
         /* In a subgraph, we add an epsilon transition pointing to the
          * destination state specified in parameter. By convention,
          * epsilon has the tag number 0 */
         add_outgoing_transition(new_state,0,destination_state);
      }
   }
   /* Now, we deal with the transitions */
   Transition* original_transitions=original_state->transitions;
   while (original_transitions!=NULL) {
      if (!RTN && (original_transitions->tag_number<0) && depth>=max_depth) {
         /* If we have a subgraph call while 1) we have overpassed the maximum
          * depth and 2) we must produce a strict FST, then we have to ignore it.
          * We just signal the fact by increasing a counter */
         (*SUBGRAPH_CALL_IGNORED)++;
      }
      else {
         /* Otherwise, we deal with the transition. First of all, we compute the
          * number of its destination state in the new graph. The point is that
          * original_transitions->state_number is a global state number in the original
          * fst2. So, we compute its relative number in the current graph,
          * which is original_transitions->state_number-grammar->initial_states[n_graph],
          * and we add to it the starting position of the current graph.
          */
         int destination_state_number=initial_position_for_new_states+original_transitions->state_number-grammar->initial_states[n_graph];
         add_outgoing_transition(new_state,original_transitions->tag_number,destination_state_number);
         /* We get a pointer on the transition we have just created */
         Transition* temp=new_state->outgoing_transitions;
         if ((temp->tag_number) < 0) {
            /* If the transition is a subgraph call */
            if (depth<max_depth) {
               /* And if we must flatten, we note it in order the modify it later */
               if (trans_to_flatten_size>=trans_to_flatten_capacity) {
                  /* We resize the array if needed */
                  trans_to_flatten_capacity=2*trans_to_flatten_capacity;
                  transitions_to_flatten=(Transition**)realloc(transitions_to_flatten,trans_to_flatten_capacity*sizeof(Transition*));
                  if (transitions_to_flatten==NULL) {
                     fatal_alloc_error("flatten_graph");
                  }
               }
               transitions_to_flatten[trans_to_flatten_size++]=temp;
            }
            else {
               /* If we have overpassed the maximum depth, we know that we can produce
                * a subgraph call, since, if not, the condition tested above:
                *
                *   (!RTN && (original_transitions->tag_number<0) && depth>=max_depth)
                *
                * would have been true and we would not be here. So, we just produce
                * a call to the subgraph taking care of the graph renumerotation. */
               temp->tag_number=-(new_graph_number[-(original_transitions->tag_number)]);
               (*SUBGRAPH_CALL)++;
            }
         }
      }
      /* Finally, we go on with the next transition in the list */
      original_transitions=original_transitions->next;
   }
}
if (depth==0) {
   /* If we are in the main graph, at the first step, i.e. not inside a
    * recursive call to the main graph, we say that its first state is initial */
   set_initial_state(new_main_graph->states[0]);
}
/* Then, if there were some calls to subgraphs, we flatten them */
for (int i=0;i<trans_to_flatten_size;i++) {
   /* We flatten recursively the subgraph */
   int starting_pos=flatten_graph(grammar,-(transitions_to_flatten[i]->tag_number),
                                  depth+1,max_depth,
                                  new_main_graph,
                                  transitions_to_flatten[i]->state_number,RTN,
                                  SUBGRAPH_CALL_IGNORED,SUBGRAPH_CALL,
                                  new_graph_number);
   /* And we replace the subgraph call by an epsilon transition to the initial state
    * of the flattened subgraph */
   transitions_to_flatten[i]->tag_number = 0;
   transitions_to_flatten[i]->state_number = starting_pos;
}
/* Clean up */
free(transitions_to_flatten);
/* And finally... */
if (depth==0) {
   /* If we are in the main graph, at the first step, i.e. not inside a
    * recursive call to the main graph */
   if (*SUBGRAPH_CALL) {
      /* If some subgraph calls remains, then we have an equivalent RTN */
      return EQUIVALENT_RTN;
   }
   else {
      /* Otherwise, we test if we have removed some subgraph calls */
      if (*SUBGRAPH_CALL_IGNORED) {
         /* In that case, we  have a FST that is just an approximation of
          * the original grammar */
         return APPROXIMATIVE_FST;
      }
      else {
         /* If there is no subgraph call and if no subgraph call was removed,
          * then we have a FST that is strictly equivalent to the original
          * grammar */
         return EQUIVALENT_FST;
      }
  }
}
/* If we are in a subgraph, we return the number of the initial state of
 * the current graph that we have just flattened */
return initial_position_for_new_states;
}


/**
 * This function dumps a graph from a Fst2 that had been marked to be kept,
 * taking into account renumbering of graphs.
 */
void save_graph_to_keep(int graph_number,Fst2* grammar,U_FILE* f,int* new_graph_number) {
int limit=grammar->initial_states[graph_number]+grammar->number_of_states_per_graphs[graph_number];
/* We save the graph header (number+name) */
u_fprintf(f,"%d ",-new_graph_number[graph_number]);
u_fprintf(f,"%S\n",grammar->graph_names[graph_number]);
/* Then, we dump all the states */
for (int i=grammar->initial_states[graph_number];i<limit;i++) {
   /* We print the symbol that corresponds to the finality of the state */
   if (is_final_state(grammar->states[i])) {
      u_fprintf(f,"t ");
   }
   else {
      u_fprintf(f,": ");
   }
   /* And we print all the outgoing transitions */
   Transition* transition=grammar->states[i]->transitions;
   while (transition!=NULL) {
      if (transition->tag_number < 0) {
         /* If we have a subgraph call, we renumber it */
         u_fprintf(f,"%d %d ",-new_graph_number[-(transition->tag_number)],(transition->state_number)-grammar->initial_states[graph_number]);
      }
      else {
         u_fprintf(f,"%d %d ",transition->tag_number,(transition->state_number)-grammar->initial_states[graph_number]);
      }
      transition = transition->next;
   }
   u_fprintf(f,"\n");
}
u_fprintf(f,"f \n");
}


/**
 * This function dumps to the given file each graph of the
 * given grammar that has been marked to be kept.
 */
void save_graphs_to_keep(Fst2* grammar,U_FILE* f,int* new_graph_number) {
for (int i=2;i<=grammar->number_of_graphs;i++) {
   if (new_graph_number[i]!=0) {
      save_graph_to_keep(i,grammar,f,new_graph_number);
   }
}
}

} // namespace unitex

