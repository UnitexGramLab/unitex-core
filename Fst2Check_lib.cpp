/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Fst2Check_lib.h"
#include "Error.h"
#include "BitMasks.h"
#include "Transitions.h"
#include "SingleGraph.h"
#include "List_int.h"
#include "List_pointer.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/* see http://en.wikipedia.org/wiki/Variable_Length_Array . MSVC did not support it 
   see http://msdn.microsoft.com/en-us/library/zb1574zs(VS.80).aspx */
#if defined(_MSC_VER) && (!(defined(NO_C99_VARIABLE_LENGTH_ARRAY)))
#define NO_C99_VARIABLE_LENGTH_ARRAY 1
#endif


typedef enum {CHK_MATCHES_E,CHK_DOES_NOT_MATCH_E,CHK_DONT_KNOW} E_MATCHING_STATUS;

typedef struct {
	Fst2* fst2;
	E_MATCHING_STATUS* graphs_matching_E;
	SingleGraph* condition_graphs;
} GrfCheckInfo;


/**
 * Returns 1 if the given tag number corresponds to a $[ or $![ tag;
 * 0 otherwise.
 */
static int is_right_context_beginning(Fst2* fst2,int n) {
Fst2Tag tag=fst2->tags[n];
return tag->type==BEGIN_POSITIVE_CONTEXT_TAG || tag->type==BEGIN_NEGATIVE_CONTEXT_TAG;
}


static int get_end_of_context(Fst2* fst2,int state);

/**
 * Returns the number of the state that is pointed by the $] transition that
 * closes the current context or -1 if not found.
 * Note that nested contexts are taken into account.
 * 'visited_states' is used to avoid exploring loops, which would lead to
 * a stack overflow.
 */
static int get_end_of_context__(Fst2* fst2,int state,struct list_int* *visited_states) {
int res;
if (is_in_list(state,*visited_states)) {
	/* No need to visit twice a state */
	return -1;
}
(*visited_states)=new_list_int(state,*visited_states);
for (Transition* t=fst2->states[state]->transitions;t!=NULL;t=t->next) {
	if (t->tag_number<=0) {
		/* If we have a graph call or a E transition, we look after it */
		res=get_end_of_context__(fst2,t->state_number,visited_states);
		if (res!=-1) {
			return res;
		}
		continue;
	}
	Fst2Tag tag=fst2->tags[t->tag_number];
	if (tag->type==END_CONTEXT_TAG) {
		/* We found it! */
		return t->state_number;
	}
	if (tag->type==BEGIN_POSITIVE_CONTEXT_TAG || tag->type==BEGIN_NEGATIVE_CONTEXT_TAG) {
		/* If we have a nested context, we deal with it */
		int end=get_end_of_context(fst2,t->state_number);
		if (end==-1) {
			/* No end for this nested context ? There is nothing more to be done
			 * with this transition */
			continue;
		} else {
			/* Now, we can continue to explore from the end of the nested context */
			res=get_end_of_context__(fst2,end,visited_states);
			if (res!=-1) {
				return res;
			}
			continue;
		}
	}
	/* If we have a normal transition, we explore it */
	res=get_end_of_context__(fst2,t->state_number,visited_states);
	if (res!=-1) {
		return res;
	}
	continue;
}
return -1;
}


/**
 * Returns the number of the state that is pointed by the $] transition that
 * closes the current context or -1 if not found.
 * Note that nested contexts are taken into account.
 */
static int get_end_of_context(Fst2* fst2,int state) {
struct list_int* visited_states=NULL;
int res=get_end_of_context__(fst2,state,&visited_states);
free_list_int(visited_states);
return res;
}


/**
 * Returns 1 if the given tag does not match anything in the text.
 *
 */
static int matches_E(Fst2* fst2,int tag_number) {
if (tag_number<0) {
	return 0;
}
if (tag_number==0) {
	return 1;
}
Fst2Tag tag=fst2->tags[tag_number];
switch (tag->type) {
/* WARNING: this is important not to use a default clause here!
 *          By enumerating all values instead, we make sure that there will
 *          be a compiler warning if one day a new value is added to the enum tag_type
 *          that is not taken into account here.
 */
case UNDEFINED_TAG: // used at initialization of a tag
case META_TAG:      // <MOT>, <MIN>, etc.
case PATTERN_TAG:   // <be.V>
case PATTERN_NUMBER_TAG: // used when patterns have been numbered
case TOKEN_LIST_TAG: break;  // used when the tag matches a list of tokens. This will
/* The following matches E */
case BEGIN_VAR_TAG: // $a(
case END_VAR_TAG:   // $a)
case BEGIN_OUTPUT_VAR_TAG: // $|a(
case END_OUTPUT_VAR_TAG:   // $|a)
case BEGIN_POSITIVE_CONTEXT_TAG: // $[
case BEGIN_NEGATIVE_CONTEXT_TAG: // $![
case END_CONTEXT_TAG:            // $]
case LEFT_CONTEXT_TAG:           // $*
case BEGIN_MORPHO_TAG: // $<
case END_MORPHO_TAG:    // $>
case TEXT_START_TAG: // {^}
case TEXT_END_TAG: return 1;   // {$}
}
/* Finally, we test if we have a <E> transition with an output */
if (!u_strcmp(tag->input,"<E>")) {
	return 1;
}
return 0;
}


static void clean_condition_graph(SingleGraph g) {
remove_epsilon_transitions(g,0);
trim(g,NULL);
if (g->number_of_states!=0) {
	determinize(g);
}
/* And we print the resulting graph */
/*save_fst2_subgraph(U_STDERR,g,graph,NULL);
if (graph==1) {
	for (int i=0;i<fst2->number_of_tags;i++) {
		error("%d: %S",i,fst2->tags[i]->input);
		if (fst2->tags[i]->output!=NULL && fst2->tags[i]->output[0]!='\0') {
			error("/%S",fst2->tags[i]->output);
		}
		error("\n");
}
}*/
}


/**
 * Deals with the given transition, for full simplification,
 * i.e. for E-matching detection.
 */
static void deal_with_transition_v1(Fst2* fst2,Transition* t,SingleGraphState dst,int initial_state) {
if (t->tag_number<=0) {
	/* For graphs and <E> we keep the transition as is, except for
	 * the state number that we have to adjust */
	add_outgoing_transition(dst,t->tag_number,t->state_number-initial_state);
} else if (is_right_context_beginning(fst2,t->tag_number)) {
	/* Right contexts are a special case: we skip the whole context by
	 * using an E transition that points to the end of the context
	 */
	int dst_state=get_end_of_context(fst2,t->state_number);
	if (dst_state!=-1) {
		add_outgoing_transition(dst,0,dst_state-initial_state);
	} else {
		/* If we cannot reach the end of the context, then this transition cannot
		 * match anything anyway, so we can just ignore it */
	}
} else if (matches_E(fst2,t->tag_number)) {
	/* Tags like $*, variable tags, etc. can be considered
	 * like E transitions, but they must be kept, because they
	 * could be involved into an infinite E loop. However, we also have to
	 * add a real E transition, so that the final state can be reached by the
	 * regular E removal algorithm. */
	add_outgoing_transition(dst,0,t->state_number-initial_state);
	add_outgoing_transition(dst,t->tag_number,t->state_number-initial_state);
} else {
	/* If we have a transition does actually match something in the text, we just
	 * ignore it */
}
}


/**
 * Deals with the given transition, for light simplification, i.e. for
 * E loop/left recursion detection.
 */
static void deal_with_transition_v2(Fst2* fst2,Transition* t,SingleGraphState dst,int initial_state) {
/* We always add the original transition */
add_outgoing_transition(dst,t->tag_number,t->state_number-initial_state);
if (t->tag_number>0 && is_right_context_beginning(fst2,t->tag_number)) {
	/* Right contexts are a special case: we allow to skip the whole context by
	 * adding an E transition that points to the end of the context
	 */
	int dst_state=get_end_of_context(fst2,t->state_number);
	if (dst_state!=-1) {
		add_outgoing_transition(dst,0,dst_state-initial_state);
	} else {
		/* If we cannot reach the end of the context, then this transition cannot
		 * match anything anyway, so we can just ignore it */
	}
}
}

/**
 * We create a copy of the given graph using the following rules if full_simplification is not null:
 * - <E> transitions and graph calls are kept
 * - all right contexts are ignored, replaced by an epsilon transition
 * - all tags that don't match anything in the text (like $* $< and $>) are kept,
 *   because they can be involved into a E loop. We also add a real E transition.
 * - all other transitions that matches something from the text are removed
 *
 * As a consequence, the resulting graph is only made of real E transitions,
 * pseudo-E transitions, and graph calls and we can use it as follows:
 * - if no final state is accessible, it means that the graph cannot match E
 * - if the initial state is final, it means that the graph match E
 * - otherwise, we don't know yet
 *
 *
 * If full_simplification is null, we have to create a condition graph suitable for
 * E loop and left recursion detection. For that purpose, we keep the graph as is,
 * with only one modification: adding an E transition to skip right contexts. But still,
 * we keep the context, because we also have to look at it for E loops and left recursions.
 *
 */
static SingleGraph create_condition_graph(Fst2* fst2,int graph,int full_simplification) {
SingleGraph g=new_SingleGraph(INT_TAGS);
int initial_state=fst2->initial_states[graph];
int n_states=fst2->number_of_states_per_graphs[graph];
for (int i=initial_state;i<initial_state+n_states;i++) {
	SingleGraphState dst=add_state(g);
	Fst2State src=fst2->states[i];
	if (is_initial_state(src)) {
		set_initial_state(dst);
	}
	if (is_final_state(src)) {
		set_final_state(dst);
	}
	Transition* t=src->transitions;
	while (t!=NULL) {
		if (full_simplification) {
			deal_with_transition_v1(fst2,t,dst,initial_state);
		} else {
			deal_with_transition_v2(fst2,t,dst,initial_state);
		}
		t=t->next;
	}
}
clean_condition_graph(g);
return g;
}


static E_MATCHING_STATUS get_status(SingleGraph g) {
if (g->number_of_states==0) {
	return CHK_DOES_NOT_MATCH_E;
}
int i=get_initial_state(g);
if (i<0) {
	fatal_error("Internal error in get_status: invalid negative initial state %d\n",i);
}
SingleGraphState s=g->states[i];
if (is_final_state(s)) {
	/* If the initial state is final, it means that we can reach it without matching
	 * anything in the text */
	return CHK_MATCHES_E;
}
return CHK_DONT_KNOW;
}


/**
 * We initialize the structure that we will need to look for graphs
 * that matches E, E loops and left recursions.
 */
static GrfCheckInfo* new_GrfCheckInfo(Fst2* fst2) {
GrfCheckInfo* res=(GrfCheckInfo*)malloc(sizeof(GrfCheckInfo));
if (res==NULL) {
	fatal_alloc_error("new_GrfCheckInfo");
}
res->fst2=fst2;
res->graphs_matching_E=(E_MATCHING_STATUS*)malloc(sizeof(E_MATCHING_STATUS)*(fst2->number_of_graphs+1));
if (res->graphs_matching_E==NULL) {
	fatal_alloc_error("new_GrfCheckInfo");
}
res->condition_graphs=(SingleGraph*)calloc(fst2->number_of_graphs+1,sizeof(SingleGraph));
if (res->condition_graphs==NULL) {
	fatal_alloc_error("new_GrfCheckInfo");
}
for (int i=1;i<fst2->number_of_graphs+1;i++) {
	SingleGraph g=create_condition_graph(fst2,i,1);
	res->condition_graphs[i]=g;
	res->graphs_matching_E[i]=get_status(g);
}
return res;
}


/**
 * Note that we don't free the fst2 field, since we did not create it.
 */
static void free_GrfCheckInfo(GrfCheckInfo* info) {
if (info==NULL) {
	return;
}
free(info->graphs_matching_E);
if (info->condition_graphs!=NULL) {
	for (int i=1;i<info->fst2->number_of_graphs+1;i++) {
		free_SingleGraph(info->condition_graphs[i],NULL);
	}
	free(info->condition_graphs);
}
free(info);
}


/**
 * We will update the given graph with the given updated graph for which we
 * have now reliable information. We make sure that the graph is clean after that
 * (E removal+trim+determinization).
 */
static void resolve_conditions(GrfCheckInfo* chk,int graph,struct list_int* updated_graphs) {
SingleGraph g=chk->condition_graphs[graph];
for (int i=0;i<g->number_of_states;i++) {
	SingleGraphState state=g->states[i];
	Transition** t=&(state->outgoing_transitions);
	while ((*t)!=NULL) {
		if ((*t)->tag_number<0) {
			if (is_in_list(-(*t)->tag_number,updated_graphs)) {
				/* We only look at graphs that have been updated */
				E_MATCHING_STATUS status=chk->graphs_matching_E[-(*t)->tag_number];
				switch(status) {
					case CHK_DONT_KNOW: fatal_error("Unexpected CHK_DONT_KNOW value in resolve_conditions\n"); break;
					case CHK_MATCHES_E: {
						/* The graph matches E, we can add an E transition */
						Transition* new_E_transition=new_Transition(0,(*t)->state_number,(*t)->next);
						(*t)->next=new_E_transition;
						t=&(new_E_transition->next);
						break;
					}
					case CHK_DOES_NOT_MATCH_E: {
						/* We have to remove this transition */
						Transition* next=(*t)->next;
						free_Transition(*t);
						(*t)=next;
						break;
					}
				}
				continue;
			}
		}
		/* Not a transition we are concerned about */
		t=&((*t)->next);
	}
}
clean_condition_graph(g);
}


/**
 * Returns 1 if there is something more to do after this call or 0 if:
 * - no new information was found
 * - the main graph matches E
 */
static int resolve_all_conditions(GrfCheckInfo* chk,struct list_int* *list,int *unknown) {
*unknown=0;
struct list_int* new_list=NULL;
for (int i=1;i<chk->fst2->number_of_graphs+1;i++) {
	if (chk->graphs_matching_E[i]==CHK_DONT_KNOW) {
		/* We only need to look at the graphs we are not sure about yet */
		resolve_conditions(chk,i,*list);
		chk->graphs_matching_E[i]=get_status(chk->condition_graphs[i]);
		if (chk->graphs_matching_E[i]!=CHK_DONT_KNOW) {
			/* If we have found an answer, we note that graph #i must be
			 * looked at on the next loop */
			new_list=new_list_int(i,new_list);
		} else {
			/* The graph is still unknown */
			(*unknown)++;
		}
	}
}
/* Now we can use the new list */
free_list_int(*list);
*list=new_list;
if (chk->graphs_matching_E[1]==CHK_MATCHES_E) {
	error("Main graph matches epsilon!\n");
	return 0;
}
return ((*list)!=NULL && (*unknown)!=0);
}



static void rebuild_condition_graphs(GrfCheckInfo* chk) {
for (int i=1;i<chk->fst2->number_of_graphs+1;i++) {
	free_SingleGraph(chk->condition_graphs[i],NULL);
	chk->condition_graphs[i]=create_condition_graph(chk->fst2,i,0);
}
}


static int transition_can_match_E(int tag_number,GrfCheckInfo* chk) {
if (tag_number==0) {
	return 1;
}
if (tag_number<0) {
	return chk->graphs_matching_E[-tag_number]==CHK_MATCHES_E;
}
return matches_E(chk->fst2,tag_number);
}


/**
 * By convention, if stop is >=0 it represents a state number, and if <0, it represents
 * a graph call. We use it as a stop condition.
 */
static void print_reversed_list(struct list_pointer* list,Fst2* fst2,int stop,int depth) {
if (list==NULL) {
	return;
}
Transition* t=(Transition*)list->pointer;
if (depth!=0) {
	if ((stop>=0 && t->state_number==stop) || (stop<0 && t->tag_number==stop)) {
		return;
	}
}
print_reversed_list(list->next,fst2,stop,depth+1);
if (t->tag_number<0) {
	error("   :%S",fst2->graph_names[-t->tag_number]);
} else {
	error("   %S",fst2->tags[t->tag_number]->input);
	if (fst2->tags[t->tag_number]->output!=NULL && fst2->tags[t->tag_number]->output[0]!='\0') {
		error("/%S",fst2->tags[t->tag_number]->output);
	}
}
error("\n");
}


/**
 * Looks for a loop. To do that, we only follow transitions that can match E.
 * Every time we follow such a transition, we add it to the 'transitions' list.
 * This list is used to print the E loop if we find any. The function returns
 * 1 if a loop is found; 0 otherwise.
 */
static int find_an_E_loop(int* mark,int current_state,int graph,GrfCheckInfo* chk,
		struct list_pointer* transitions) {
if (mark[current_state]==1) {
	/* The state has been visited, nothing to do */
	return 0;
}
if (mark[current_state]==2) {
	/* The state is being visited, we have a loop */
	error("E loop in graph %S, made of the following tags:\n",chk->fst2->graph_names[graph]);
	print_reversed_list(transitions,chk->fst2,current_state,0);
	return 1;
}
/* We start visiting the state */
mark[current_state]=2;
SingleGraphState s=chk->condition_graphs[graph]->states[current_state];
Transition* t=s->outgoing_transitions;
while (t!=NULL) {
	if (transition_can_match_E(t->tag_number,chk)) {
		struct list_pointer* new_head=new_list_pointer(t,transitions);
		int res=find_an_E_loop(mark,t->state_number,graph,chk,new_head);
		new_head->next=NULL;
		free_list_pointer(new_head);
		if (res==1) {
			return 1;
		}
	}
	t=t->next;
}
/* The state has been fully visited */
mark[current_state]=1;
return 0;
}


/**
 * Returns 0 if no E loop is found. Otherwise, prints a message that
 * describes the loop and returns 1.
 */
static int is_E_loop(GrfCheckInfo* chk,int graph) {
SingleGraph g=chk->condition_graphs[graph];
/* 0 means that the state has not been visited at all
 * 1 means that the state has already been fully visited
 * 2 means that the state is being visited now, so finding such
 *   a state means that there is an E loop
 */
int* mark=(int*)calloc(g->number_of_states,sizeof(int));
int loop=0;
for (int i=0;loop==0 && i<g->number_of_states;i++) {
	if (mark[i]==1) {
		/* No need to examine twice a state */
		continue;
	}
	if (find_an_E_loop(mark,i,graph,chk,NULL)) {
		loop=1;
	}
	mark[i]=1;
}
free(mark);
return loop;
}


/**
 * Tests all graphs for E loops, and returns 1 if there is any. In such a case,
 * a message is printed on the error output stream for every graph containing such
 * a loop. Note that 'rebuild_condition_graphs' must have been invoked before,
 * to ensure that the condition graphs are adequate.
 */
static int is_any_E_loop(GrfCheckInfo* chk) {
int loop=0;
for (int i=1;i<chk->fst2->number_of_graphs+1;i++) {
	if (is_E_loop(chk,i)) {
		loop=1;
	}
}
return loop;
}



static int is_left_recursion(GrfCheckInfo* chk,int graph,int* mark_graph,struct list_pointer* transitions);

static int find_a_left_recursion(int* mark_graph,int* mark_state,int current_state,int graph,
									GrfCheckInfo* chk,struct list_pointer* transitions) {
if (mark_state[current_state]==1) {
	/* The state has been visited, nothing to do */
	return 0;
}
if (mark_state[current_state]==2) {
	/* The state is being visited, we have a loop, but it should have been detected before */
	error("E loop in graph %S, made of the following tags:\n",chk->fst2->graph_names[graph]);
	print_reversed_list(transitions,chk->fst2,current_state,0);
	return 1;
}
/* We start visiting the state */
mark_state[current_state]=2;
SingleGraphState s=chk->condition_graphs[graph]->states[current_state];
Transition* t=s->outgoing_transitions;
while (t!=NULL) {
	if (t->tag_number<0) {
		/* As we look for left recursions, we always test recursively
		 * graph calls, regardless the fact that they may match E
		 */
		struct list_pointer* new_head=new_list_pointer(t,transitions);
		int res=is_left_recursion(chk,-(t->tag_number),mark_graph,new_head);
		new_head->next=NULL;
		free_list_pointer(new_head);
		if (res==1) {
			return 1;
		}
	}
	if (transition_can_match_E(t->tag_number,chk)) {
		struct list_pointer* new_head=new_list_pointer(t,transitions);
		int res=find_a_left_recursion(mark_graph,mark_state,t->state_number,graph,chk,new_head);
		new_head->next=NULL;
		free_list_pointer(new_head);
		if (res==1) {
			return 1;
		}
	}
	t=t->next;
}
/* The state has been fully visited */
mark_state[current_state]=1;
return 0;
}


/**
 * Returns 0 if no left recursion is found. Otherwise, prints a message that
 * describes the loop and returns 1.
 */
static int is_left_recursion(GrfCheckInfo* chk,int graph,int* mark_graph,struct list_pointer* transitions) {
if (mark_graph[graph]==1) {
	/* The graph has already been tested for left recursions */
	return 0;
}
if (mark_graph[graph]==2) {
	/* We found a left recursion */
	error("Left recursion found in graph %S, made of the following tags:\n",chk->fst2->graph_names[graph]);
	print_reversed_list(transitions,chk->fst2,-graph,0);
	return 1;
}

mark_graph[graph]=2;
SingleGraph g=chk->condition_graphs[graph];
/* 0 means that the state has not been visited at all
 * 1 means that the state has already been fully visited
 * 2 means that the state is being visited now, so finding such
 *   a state means that there is an E loop
 */
int* mark_state=(int*)calloc(g->number_of_states,sizeof(int));
int recursion=0;
int initial_state=get_initial_state(g);
if (initial_state==-2) {
	fatal_error("Internal error: several initial states in graph %S\n",chk->fst2->graph_names[graph]);
}
if (initial_state==-1) {
	/* If the graph could not be loaded, we just ignore */
	mark_graph[graph]=1;
	free(mark_state);
	return 0;
}
if (find_a_left_recursion(mark_graph,mark_state,initial_state,graph,chk,transitions)) {
	recursion=1;
}
free(mark_state);
mark_graph[graph]=1;
return recursion;
}


/**
 * Tests all graphs for left recursion, and returns 1 if there is any. In such a case,
 * a message is printed on the error output stream for every graph call sequence
 * leading to such a recursion. Note that 'rebuild_condition_graphs' must have been invoked before,
 * to ensure that the condition graphs are adequate.
 */
static int is_any_left_recursion(GrfCheckInfo* chk) {
int recursion=0;
int* mark_graph=(int*)calloc(chk->fst2->number_of_graphs+1,sizeof(int));

for (int i=1;recursion==0 && i<chk->fst2->number_of_graphs+1;i++) {
	if (mark_graph[i]==0) {
		/* No need to look a graph twice */
		if (is_left_recursion(chk,i,mark_graph,NULL)) {
			recursion=1;
		}
	}
}
free(mark_graph);
return recursion;
}


/**
 * Returns 1 if the given .fst2 is OK to be used by the Locate program; 0 otherwise.
 * Conditions are:
 *
 * 1) no left recursion
 * 2) no loop that can recognize the empty word (<E> with an output or subgraph
 *    that can match the empty word).
 */
int OK_for_Locate_write_error(const VersatileEncodingConfig* vec,const char* name,char no_empty_graph_warning,U_FILE* ferr) {
int RESULT=1;
struct FST2_free_info fst2_free;
Fst2* fst2=load_abstract_fst2(vec,name,1,&fst2_free);
if (fst2==NULL) {
	fatal_error("Cannot load graph %s\n",name);
}
u_printf("Creating condition sets...\n");
GrfCheckInfo* chk=new_GrfCheckInfo(fst2);
/* Now, we look for a fix point in the condition graphs */
struct list_int* list=NULL;
/* To do that, we start by creating a list of all the graphs we are sure about */
int unknown=0;
for (int i=1;i<fst2->number_of_graphs+1;i++) {
	if (chk->graphs_matching_E[i]!=CHK_DONT_KNOW) {
		list=new_list_int(i,list);
	} else {
		unknown++;
	}
}
/* While there is something to do for E matching */
u_printf("Checking empty word matching...\n");
while (resolve_all_conditions(chk,&list,&unknown)) {}
if (chk->graphs_matching_E[1]==CHK_MATCHES_E) {
	if (!no_empty_graph_warning) {
       error("ERROR: the main graph %S recognizes <E>\n",fst2->graph_names[1]);
       if (ferr!=NULL) {
    	   u_fprintf(ferr,"ERROR: the main graph %S recognizes <E>\n",fst2->graph_names[1]);
	   }
	}
	goto evil_goto;
}
if (!no_empty_graph_warning) {
	for (int i=2;i<fst2->number_of_graphs+1;i++) {
		if (chk->graphs_matching_E[i]==CHK_MATCHES_E) {
			error("WARNING: the graph %S recognizes <E>\n",fst2->graph_names[i]);
			if (ferr!=NULL) {
				u_fprintf(ferr,"WARNING: the graph %S recognizes <E>\n",fst2->graph_names[i]);
			}
		}
	}
}
/* Now, we look for E loops and left recursions. And to do that, we need a new version
 * of the condition graphs, because a graph that does not match E would have been emptied.
 * And obviously, we can not deduce anything from an empty graph. */
rebuild_condition_graphs(chk);
u_printf("Checking E loops...\n");
if (is_any_E_loop(chk)) {
	/* Error messages have already been printed */
	goto evil_goto;
}
u_printf("Checking left recursions...\n");
if (is_any_left_recursion(chk)) {
	/* Error messages have already been printed */
	goto evil_goto;
}
evil_goto:
/* There may be something unused in the list that we need to free */
free_list_int(list);
free_GrfCheckInfo(chk);
free_abstract_Fst2(fst2,&fst2_free);
return RESULT;
}




int OK_for_Locate(const VersatileEncodingConfig* vec,const char* name,char no_empty_graph_warning) {
    return OK_for_Locate_write_error(vec,name,no_empty_graph_warning,NULL);
}

#define NOT_SEEN_YET 0
#define SEEN 1
#define BEING_EXPLORED 2
/**
 * Returns 1 if the exploration of the current state leads to the conclusion that
 * the automaton is acyclic; 0 otherwise.
 */
static int is_acyclic(Fst2* fst2,char* mark,int current_state_index,int shift) {
if (mark[current_state_index-shift]==BEING_EXPLORED) {
   /* If we find a state that is currently being explored, then we found
    * a cycle */
   return 0;
}
if (mark[current_state_index-shift]==SEEN) {
   return 1;
}
/* If the state is unseen, we mark it as being explored */
mark[current_state_index-shift]=BEING_EXPLORED;
Fst2State state=fst2->states[current_state_index];
Transition* t=state->transitions;
while (t!=NULL) {
   if (!is_acyclic(fst2,mark,t->state_number,shift)) {
      return 0;
   }
   t=t->next;
}
/* Finally, we mark the state as seen, and we return the success value */
mark[current_state_index-shift]=SEEN;
return 1;
}


/**
 * Returns 1 if the given sentence automaton is acylic; 0 otherwise.
 */
static int is_acyclic(Fst2* fst2,int graph_number) {
if (graph_number<1 || graph_number>fst2->number_of_graphs) {
   fatal_error("Invalid graph number in is_acyclic\n");
}
int N=fst2->number_of_states_per_graphs[graph_number];
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
char *mark=(char*)malloc(sizeof(char)*N);
#else
char mark[N];
#endif
for (int i=0;i<N;i++) {
   mark[i]=NOT_SEEN_YET;
}
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
free(mark);
#endif
return is_acyclic(fst2,mark,fst2->initial_states[graph_number],fst2->initial_states[graph_number]);
}


/**
 * Returns 1 if all tags have valid sentence automaton outputs; 0 otherwise.
 */
static int valid_outputs(Fst2* fst2) {
if (u_strcmp(fst2->tags[0]->input,"<E>") || fst2->tags[0]->output!=NULL) {
   /* Should never happen */
   fatal_error("valid_outputs: the first tag of the .fst2 should be <E>\n");
}
for (int i=1;i<fst2->number_of_tags;i++) {
   /* Condition 3: no tag of the form <E>/XYZ */
   if (!u_strcmp(fst2->tags[i]->input,"<E>") && fst2->tags[i]->output!=NULL) {
      return 0;
   }
   if (fst2->tags[i]->output==NULL) {
      /* Condition 4: <E> must the only tag without output */
      return 0;
   }
   int w,x,y,z,f,g;
   char foo;
   /* Condition 5 */
   if (6!=u_sscanf(fst2->tags[i]->output,"%d %d %d %d %d %d%c",&w,&x,&y,&z,&f,&g,&foo)) {
      /* If the output is not made of 6 integers */
      return 0;
   }
   if (w<0 || x<-1 || y<0 || z<-1) {
      return 0;
   }
}
return 1;
}


/**
 * Returns 1 if the given .fst2 corresponds to a valid sentence automaton; 0
 * otherwise. Following conditions must be true:
 * 
 * 1) there must be only one graph
 * 2) it must be acyclic
 * 3) there must not be any <E> transition with an ouput
 * 4) <E> must the only tag without output
 * 5) all other tags must have an ouput of the form w x y z f g, with
 *    w and y being integers >=0, and x, z, f and g being integers >=-1 
 */
int valid_sentence_automaton_write_error(const VersatileEncodingConfig* vec,const char* name,U_FILE*) {
struct FST2_free_info fst2_free;
Fst2* fst2=load_abstract_fst2(vec,name,0,&fst2_free);
if (fst2==NULL) return 0;
/* Condition 1 */
if (fst2->number_of_graphs!=1) {
   free_abstract_Fst2(fst2,&fst2_free);
   return 0;
}
/* Condition 2 */
if (!is_acyclic(fst2,1)) {
   free_abstract_Fst2(fst2,&fst2_free);
   return 0;
}
/* Conditions 3, 4 & 5 */
if (!valid_outputs(fst2)) {
   free_abstract_Fst2(fst2,&fst2_free);
   return 0;
}
/* Victory! */
return 1;
}

int valid_sentence_automaton(const VersatileEncodingConfig* vec,const char* name) {
    return valid_sentence_automaton_write_error(vec,name,NULL);
}


int OK_for_Fst2Txt(Fst2* fst2) {
if (fst2==NULL) return 0;
int ret=1;
int mark,a,b,c;
for (int i=1;i<=fst2->number_of_graphs;i++) {
	mark=a=b=c=0;
	for (int j=0;j<fst2->number_of_states_per_graphs[i];j++) {
		Transition* t=fst2->states[fst2->initial_states[i]+j]->transitions;
		while (t!=NULL) {
			if (t->tag_number<0) {
				t=t->next;
				continue;
			}
			Fst2Tag tag=fst2->tags[t->tag_number];
			switch (tag->type) {
				case BEGIN_POSITIVE_CONTEXT_TAG:
				case BEGIN_NEGATIVE_CONTEXT_TAG:
				case END_CONTEXT_TAG:
				case LEFT_CONTEXT_TAG: {
					if (!mark) {
						/* Don't modify this line!! This exact wording is expected
						 * by the Gramlab console in order to display a clickable
						 * link that the user can use to open the faulty graph
						 */
						error("Error in graph %S:\n",fst2->graph_names[i]);
						mark=1;
					}
					if (!a) error("- unsupported context\n");
					ret=0;
					a=1;
					break;
				}
				case BEGIN_MORPHO_TAG:
				case END_MORPHO_TAG: {
					if (!mark) {
						/* Don't modify this line!! This exact wording is expected
						 * by the Gramlab console in order to display a clickable
						 * link that the user can use to open the faulty graph
						 */
						error("Error in graph %S:\n",fst2->graph_names[i]);
						mark=1;
					}
					if (!b) error("- unsupported morphological mode\n");
					ret=0;
					b=1;
					break;
				}
				default: {
					if (tag->morphological_filter!=NULL && tag->morphological_filter[0]!='\0') {
						if (!mark) {
							/* Don't modify this line!! This exact wording is expected
							 * by the Gramlab console in order to display a clickable
							 * link that the user can use to open the faulty graph
							 */
							error("Error in graph %S:\n",fst2->graph_names[i]);
							mark=1;
						}
						if (!c) error("- unsupported morphological filter\n");
						ret=0;
						c=1;
						break;
					}
				}
			}
			t=t->next;
		}
	}
}
return ret;
}


} // namespace unitex
