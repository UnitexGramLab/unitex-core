/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "GrfBeauty.h"
#include "Error.h"
#include "Vector.h"
#include "Grf_lib.h"
#include "Ustring.h"
#include "List_int.h"
#include "FIFO.h"
#include "BitArray.h"

#define WIDTH_PER_CHAR 8
#define HEIGHT_PER_CHAR 12

#define WIDTH_MARGIN 40
#define HEIGHT_MARGIN 20

/**
 * Possible configurations for groups:
 *
 * SINGLE: one state
 * PAIR: two states: start --> end
 * LEMON: two states with no factorizing state between them: start --> (subgroup) --> end
 * SEQUENTIAL: concatenation of other group types:
 *
 *   start --> ... --> (subgroup #i) --> factorizing state #X --> (subgroup #j) --> ... --> end
 */
typedef enum {SINGLE,PAIR,LEMON,SEQUENTIAL} GroupType;

typedef struct {
	int parent;
	int width;
	int height;
	int start;
	int end;
	GroupType type;
	vector_int* subgroups;
} BoxGroup;


typedef struct {
	int n;
	struct list_int** to_restore;
	struct list_int** to_remove;
} TransitionModifs;


static TransitionModifs* new_TransitionModifs(int n) {
TransitionModifs* t=(TransitionModifs*)malloc(sizeof(TransitionModifs));
if (t==NULL) {
	fatal_alloc_error("new_TransitionModifs");
}
t->n=n;
t->to_restore=(struct list_int**)calloc(n,sizeof(struct list_int*));
t->to_remove=(struct list_int**)calloc(n,sizeof(struct list_int*));
if (t->to_restore==NULL || t->to_remove==NULL) {
	fatal_alloc_error("new_TransitionModifs");
}
return t;
}


static void free_TransitionModifs(TransitionModifs* t) {
if (t==NULL) return;
for (int i=0;i<t->n;i++) {
	free_list_int(t->to_restore[i]);
	free_list_int(t->to_remove[i]);
}
free(t->to_restore);
free(t->to_remove);
free(t);
}


static void resize(TransitionModifs* t,int n) {
t->to_restore=(struct list_int**)realloc(t->to_restore,n*sizeof(struct list_int*));
t->to_remove=(struct list_int**)realloc(t->to_remove,n*sizeof(struct list_int*));
if (t->to_restore==NULL || t->to_remove==NULL) {
	fatal_alloc_error("resize");
}
for (int i=t->n;i<n;i++) {
	t->to_restore[i]=NULL;
	t->to_remove[i]=NULL;
}
t->n=n;
}


static void to_remove(TransitionModifs* t,int src,int dst) {
t->to_remove[src]=sorted_insert(dst,t->to_remove[src]);
}

static void to_restore(TransitionModifs* t,int src,int dst) {
t->to_restore[src]=sorted_insert(dst,t->to_restore[src]);
}


static int was_one_to_remove(TransitionModifs* t,int src,int dst) {
return remove(dst,&(t->to_remove[src]));
}


/**
 * Makes state #n an empty comment box, removing all its incoming and
 * outgoing transitions, and setting its content to the empty string.
 *
 * Note that once this function has been called, remove_isolated_boxes
 * should be called to clean those boxes before saving the .grf
 */
static void isolate_state(Grf* grf,int n,ReverseTransitions* r) {
GrfState* s=grf->states[n];
s->box_content[0]='\0';
/* To remove outgoing transitions, we have to remove
 * incoming transition from n in other states */
for (int i=0;i<s->transitions->nbelems;i++) {
	int dest=s->transitions->tab[i];
	vector_int_remove(r->t[dest],n);
}
/* We indicate that the state has no outgoing transition anymore */
s->transitions->nbelems=0;
/* And we do the same for incoming transitions */
for (int i=0;i<r->t[n]->nbelems;i++) {
	int src=r->t[n]->tab[i];
	vector_int_remove(grf->states[src]->transitions,n);
}
r->t[n]->nbelems=0;
}


/**
 * Looks for all isolated boxes, i.e. boxes with empty box content,
 * and removes them from the graph.
 */
static void remove_isolated_boxes(Grf* grf) {
int* renumber=(int*)malloc(grf->n_states*sizeof(int));
if (renumber==NULL) {
	fatal_alloc_error("remove_isolated_boxes");
}
for (int i=0;i<grf->n_states;i++) {
	renumber[i]=i;
}
for (int i=0;i<grf->n_states;/* No i++ here, not a mistake */) {
	if (grf->states[i]->box_content[0]=='\0') {
		free_GrfState(grf->states[i]);
		grf->states[i]=NULL;
		(grf->n_states)--;
		if (i!=grf->n_states) {
			/* If there is a state to swap with */
			grf->states[i]=grf->states[grf->n_states];
			grf->states[grf->n_states]=NULL;
			renumber[grf->n_states]=i;
			continue;
		}
	}
	i++;
}
for (int i=0;i<grf->n_states;i++) {
	vector_int* v=grf->states[i]->transitions;
	for (int j=0;j<v->nbelems;j++) {
		v->tab[j]=renumber[v->tab[j]];
	}
}
free(renumber);
}


/**
 * Look for boxes that have same transitions and a content that
 * is equivalent when ignoring case. If such boxes are found, they
 * are merged, so that one will be made an empty comment box.
 */
static void merge_case_equivalent_boxes(Grf* grf,Alphabet* alph) {
ReverseTransitions* reverse_transitions=compute_reverse_transitions(grf);
for (int i=2;i<grf->n_states-1;i++) {
	GrfState* a=grf->states[i];
	if (a->box_content[0]=='\0') {
		continue;
	}
	for (int j=i+1;j<grf->n_states;j++) {
		GrfState* b=grf->states[j];
		if (b->box_content[0]=='\0') {
			continue;
		}
		int isolate_j=0;
		if (have_same_transitions(grf,i,j,reverse_transitions)) {
			if (is_equal_or_uppercase(a->box_content,b->box_content,alph)) {
				/* We have to remove state b */
				isolate_j=1;
			} else if (is_equal_or_uppercase(b->box_content,a->box_content,alph)) {
				/* We have to remove state a, but not to perturb the algorithm,
				 * we will swap a and b's content, and actually remove b */
				unichar* tmp=a->box_content;
				a->box_content=b->box_content;
				b->box_content=tmp;
				isolate_j=1;
			}
			if (isolate_j) {
				isolate_state(grf,j,reverse_transitions);
			}
		}
	}
}
free_ReverseTransitions(reverse_transitions);
}


/**
 * Look for boxes that have same incoming OR outgoing transitions and a
 * content that is equivalent when ignoring case. If such boxes are found, they
 * are merged, so that one will be made a comment box.
 */
static void merge_pseudo_case_equivalent_boxes(Grf* grf,Alphabet* alph) {
ReverseTransitions* reverse_transitions=compute_reverse_transitions(grf);
for (int i=2;i<grf->n_states-1;i++) {
	GrfState* a=grf->states[i];
	if (a->box_content[0]=='\0') {
		continue;
	}
	for (int j=i+1;j<grf->n_states;j++) {
		GrfState* b=grf->states[j];
		if (b->box_content[0]=='\0') {
			continue;
		}
		int isolate_j=0;
		if (have_same_incoming_transitions(i,j,reverse_transitions)) {
			isolate_j=1; /* 1 means incoming */
		} else if (have_same_outgoing_transitions(a,b)) {
			isolate_j=2; /* 2 means outgoing */
		}
		if (isolate_j) {
			if (is_equal_or_uppercase(a->box_content,b->box_content,alph)) {
				/* We have to remove state b */
			} else if (is_equal_or_uppercase(b->box_content,a->box_content,alph)) {
				/* We have to remove state a, but not to perturb the algorithm,
				 * we will swap a and b's content, and actually remove b */
				unichar* tmp=a->box_content;
				a->box_content=b->box_content;
				b->box_content=tmp;
			} else {
				/* We won't merge a and b after all */
				isolate_j=0;
			}
			if (isolate_j) {
				if (isolate_j==1) {
					/* a and b have same incoming transitions, so we merge the
					 * outgoing ones */
					for (int k=0;k<b->transitions->nbelems;k++) {
						vector_int_add_if_absent(a->transitions,b->transitions->tab[k]);
						int dest=b->transitions->tab[k];
						vector_int_add_if_absent(reverse_transitions->t[dest],i);
					}
				} else {
					/* We have to merge the incoming transitions */
					for (int k=0;k<reverse_transitions->t[j]->nbelems;k++) {
						GrfState* src=grf->states[reverse_transitions->t[j]->tab[k]];
						vector_int_add_if_absent(src->transitions,i);
					}
				}
				isolate_state(grf,j,reverse_transitions);
			}
		}
	}
}
free_ReverseTransitions(reverse_transitions);
}


/**
 * This function returns 1 if a space is needed between a and b, i.e.
 * if:
 * - both a and b are letters
 * - one is a letter, the other is a digit
 */
static int need_a_space(unichar a,unichar b,Alphabet* alph) {
if (is_letter(a,alph)) {
	return is_letter(b,alph) || u_is_digit(b);
}
return u_is_digit(a) && is_letter(b,alph);
}


/**
 * Returns 1 if the given grf box contains more than one line; 0 otherwise.
 */
static int just_one_line(GrfState* s) {
vector_ptr* v=tokenize_box_content(s->box_content);
if (v==NULL) return 0;
int ret=v->nbelems==1;
free_vector_ptr(v,free);
return ret;
}


/**
 * This function looks for boxes a and b of only one line each so that
 * a only points to b and b is only pointed by a. If such boxes are found,
 * they are merged by concatenating their contents, separated with a space.
 */
static void merge_box_pair(Grf* grf,Alphabet* alph) {
ReverseTransitions* reverse_transitions=compute_reverse_transitions(grf);
int merge;
do {
	/* We have to loop, since a merge may introduce new possibilities */
	merge=0;
	for (int i=2;i<grf->n_states-1;i++) {
		GrfState* a=grf->states[i];
		if (a->box_content[0]=='\0') {
			continue;
		}
		for (int j=2;j<grf->n_states;j++) {
			if (i==j) continue;
			GrfState* b=grf->states[j];
			if (b->box_content[0]=='\0') {
				continue;
			}
			if (a->transitions->nbelems!=1 || a->transitions->tab[0]!=j) continue;
			if (reverse_transitions->t[j]->nbelems!=1 || reverse_transitions->t[j]->tab[0]!=i) continue;
			if (!just_one_line(a) || !just_one_line(b)) continue;
			merge=1;
			/* Remember that the contents are double quoted, so we want
			 * to have "abc" + "def" => "abc def"
			 */
			int size_a=u_strlen(a->box_content);
			a->box_content=(unichar*)realloc(a->box_content,sizeof(unichar)*(size_a+u_strlen(b->box_content)));
			if (a->box_content==NULL) {
				fatal_alloc_error("merge_box_pair");
			}
			if (need_a_space(a->box_content[size_a-2],b->box_content[1],alph)) {
				a->box_content[size_a-1]=' ';
			} else {
				size_a--;
			}
			u_strcpy(a->box_content+size_a,b->box_content+1);
			/* Now we have to remove a->b (done by isolate_state)
			 * and give b's outgoing transitions to a */
			for (int k=0;k<b->transitions->nbelems;k++) {
				vector_int_add_if_absent(a->transitions,b->transitions->tab[k]);
				vector_int_add_if_absent(reverse_transitions->t[b->transitions->tab[k]],i);
			}
			isolate_state(grf,j,reverse_transitions);

		}
	}
} while (merge);
free_ReverseTransitions(reverse_transitions);
}



/**
 * This function looks for boxes a and b that have same transitions. If such
 * boxes are found, their contents are merged.
 */
static void merge_boxes_with_same_transition(Grf* grf) {
ReverseTransitions* reverse_transitions=compute_reverse_transitions(grf);
for (int i=2;i<grf->n_states-1;i++) {
	GrfState* a=grf->states[i];
	if (a->box_content[0]=='\0') {
		continue;
	}
	for (int j=i+1;j<grf->n_states;j++) {
		GrfState* b=grf->states[j];
		if (b->box_content[0]=='\0') {
			continue;
		}
		if (have_same_transitions(grf,i,j,reverse_transitions)) {
			int size_a=u_strlen(a->box_content);
			a->box_content=(unichar*)realloc(a->box_content,sizeof(unichar)*(size_a+u_strlen(b->box_content)));
			if (a->box_content==NULL) {
				fatal_alloc_error("merge_boxes_with_same_transition");
			}
			a->box_content[size_a-1]='+';
			u_strcpy(a->box_content+size_a,b->box_content+1);
			isolate_state(grf,j,reverse_transitions);
		}
	}
}
free_ReverseTransitions(reverse_transitions);
}


/**
 * If there are transitions from A1,An to C1,Cm and if there is a box B
 * whose transitions are only Ai->B and B->Cj, then we can remove all Ai->Cj
 * and add <E> to B's content.
 */
static void replace_transition_by_E(Grf* grf) {
ReverseTransitions* reverse_transitions=compute_reverse_transitions(grf);
for (int i=2;i<grf->n_states-1;i++) {
	GrfState* B=grf->states[i];
	if (B->box_content[0]=='\0') {
		continue;
	}
	int ok=(B->transitions->nbelems!=0 && reverse_transitions->t[i]->nbelems!=0);
	for (int x=0;x<reverse_transitions->t[i]->nbelems;x++) {
		int A_x=reverse_transitions->t[i]->tab[x];
		for (int y=0;y<B->transitions->nbelems;y++) {
			int C_j=B->transitions->tab[y];
			if (-1==vector_int_contains(grf->states[A_x]->transitions,C_j)) {
				ok=0;
				goto out;
			}
		}
	}
	out:
	if (ok) {
		/* We can add <E> to B's content */
		Ustring* tmp=new_Ustring();
		u_strcpy(tmp,"\"<E>+");
		u_strcat(tmp,B->box_content+1);
		free(B->box_content);
		B->box_content=tmp->str;
		tmp->str=NULL;
		free_Ustring(tmp);
		for (int x=0;x<reverse_transitions->t[i]->nbelems;x++) {
			int A_x=reverse_transitions->t[i]->tab[x];
			for (int y=0;y<B->transitions->nbelems;y++) {
				int C_j=B->transitions->tab[y];
				vector_int_remove(grf->states[A_x]->transitions,C_j);
				vector_int_remove(reverse_transitions->t[C_j],A_x);
			}
		}
	}
}
free_ReverseTransitions(reverse_transitions);
}


/**
 * This function looks for back transitions and removes them. It
 * returns 1 if the visited state was ok or 0 if it was already being
 * visited, indicating thus that the transition to it was a back one.
 *
 * mark: 0=not visited
 *       1=visited
 *       2=visit in progress
 */
static int remove_back_transitions(Grf* grf,int current_state,int* mark,TransitionModifs* t) {
if (mark[current_state]==1) return 1;
if (mark[current_state]==2) return 0;
mark[current_state]=2;
vector_int* transitions=grf->states[current_state]->transitions;
vector_int* copy=vector_int_dup(transitions);
for (int i=0;i<copy->nbelems;i++) {
	int dest_state=copy->tab[i];
	if (!remove_back_transitions(grf,dest_state,mark,t)) {
		vector_int_remove(transitions,dest_state);
		to_restore(t,current_state,dest_state);
	}
}
free_vector_int(copy);
mark[current_state]=1;
return 1;
}


/**
 * Every accessible state x is marked with mark[x]==1.
 */
static void explore_accessibility(Grf* grf,int state,int* mark) {
if (mark[state]==1) return;
mark[state]=1;
vector_int* transitions=grf->states[state]->transitions;
for (int i=0;i<transitions->nbelems;i++) {
	int dest=transitions->tab[i];
	explore_accessibility(grf,dest,mark);
}
}


/**
 * Every coaccessible state x is marked with mark[x]==1.
 */
static void explore_coaccessibility(ReverseTransitions* reverse,int state,int* mark) {
if (mark[state]==1) return;
mark[state]=1;
vector_int* transitions=reverse->t[state];
for (int i=0;i<transitions->nbelems;i++) {
	int dest=transitions->tab[i];
	explore_coaccessibility(reverse,dest,mark);
}
}


/**
 * We remove transitions that lead to non-coaccessible states.
 */
static void deal_with_non_coaccessibility(Grf* grf,int* mark,TransitionModifs* t) {
ReverseTransitions* reverse=compute_reverse_transitions(grf);
for (int i=0;i<grf->n_states;i++) {
	mark[i]=0;
}
explore_coaccessibility(reverse,1,mark);
free_ReverseTransitions(reverse);
/* Now we can remove transitions */
for (int i=0;i<grf->n_states;i++) {
	vector_int* transitions=grf->states[i]->transitions;
	for (int i=transitions->nbelems-1;i>=0;i--) {
		int dest=transitions->tab[i];
		if (!mark[dest]) {
			to_restore(t,i,dest);
			vector_int_remove(transitions,dest);
		}
	}
}
}


static void compute_ranks(Grf* grf,int* rank) {
rank[0]=0;
for (int i=1;i<grf->n_states;i++) {
	rank[i]=-1;
}
struct fifo* f=new_fifo();
put_int(f,0);
int state;
while (!is_empty(f)) {
	state=take_int(f);
	vector_int* transitions=grf->states[state]->transitions;
	for (int i=0;i<transitions->nbelems;i++) {
		int dest=transitions->tab[i];
		if (rank[dest]!=-1) continue;
		rank[dest]=rank[state]+1;
		put_int(f,dest);
	}
}
free_fifo(f);
}

/**
 * This function computes state ranks and then uses the rank information
 * to detect and remove transverse transitions. For convenience reason,
 * we reuse an existing int array for the rank.
 */
static void remove_transverse_transitions(Grf* grf,int* rank,TransitionModifs* t) {
/* First, we compute ranks and reverse transitions */
compute_ranks(grf,rank);
ReverseTransitions* reverse=compute_reverse_transitions(grf);
/* Then, we use ranks to detect transverse transitions, since a transverse
 * transition is one from x to y where rank[y]!=rank[x]+1
 *
 * Case 1: a transition to a lesser ranked state
 */
for (int state=0;state<grf->n_states;state++) {
	vector_int* transitions=grf->states[state]->transitions;
	if (transitions->nbelems==1) {
		/* If there is only one outgoing transition, there is nothing to do */
		continue;
	}
	for (int i=transitions->nbelems-1;transitions->nbelems>1 && i>=0;i--) {
		/* transitions->nbelems>1: we want to keep at least one transition */
		int dest=transitions->tab[i];
		if (rank[dest]<(rank[state]+1)) {
			//error("pb 1 between %S and %S\n",grf->states[state]->box_content,grf->states[dest]->box_content);
			to_restore(t,state,dest);
			vector_int_remove(transitions,dest);
			vector_int_remove(reverse->t[dest],state);
			continue;
		}
	}
}
/**
 * Before going on, we have to update ranks.
 */
compute_ranks(grf,rank);
/**
 * Case 2: good rank. We have to check if dest has other incoming transitions
 */
for (int state=0;state<grf->n_states;state++) {
	vector_int* transitions=grf->states[state]->transitions;
	if (transitions->nbelems==1) {
		continue;
	}
	for (int i=transitions->nbelems-1;i>=0;i--) {
		int dest=transitions->tab[i];
		if (rank[dest]==(rank[state]+1)) {
			if (reverse->t[dest]->nbelems>1) {
				//error("pb 2 between %S and %S\n",grf->states[state]->box_content,grf->states[dest]->box_content);
				/* For every state X that has a transition to the dest one, we remove this
				 * transition if W has at least one other outgoing transition
				 */
				for (int j=0;j<reverse->t[dest]->nbelems;j++) {
					int Xindex=reverse->t[dest]->tab[j];
					GrfState* X=grf->states[Xindex];
					if (X->transitions->nbelems>1) {
						//t[Xindex]=sorted_insert(dest,t[Xindex]);
						to_restore(t,Xindex,dest);
						vector_int_remove(X->transitions,dest);
						vector_int_remove(reverse->t[dest],Xindex);
					}
				}
			}
		}
	}
}
free_ReverseTransitions(reverse);
}


static int* compute_factorizing_states(Grf* grf,int start,int end,ReverseTransitions* reverse,
									int *n_factorizing_states);


/**
 * Returns 0 if the given state is already marked globally; 1 otherwise.
 */
enum {UNVISITED,BEING_VISITED,VISITED};

static int check_states(Grf* grf,TransitionModifs* t,int state,char* mark_global,
						char* mark_local,int end,ReverseTransitions* reverse) {
if (state==end) return 1;
if (mark_global[state]) return 0;
if (mark_local[state]==BEING_VISITED || mark_local[state]==VISITED) return 1;
mark_local[state]=BEING_VISITED;
vector_int* transitions=grf->states[state]->transitions;
struct list_int* to_be_removed=NULL;
for (int i=0;i<transitions->nbelems;i++) {
	int dest=transitions->tab[i];
	if (!check_states(grf,t,dest,mark_global,mark_local,end,reverse)) {
		/* If we find a transition to a state globally marked, then
		 * we have found a transition to be removed, and replaced by a fake
		 * one to the end state of the current group.
		 */
		to_be_removed=head_insert(dest,to_be_removed);
	}
}
/* Finally, we actually remove the transitions */
if (to_be_removed!=NULL) {
	/* If there is at least one transition to remove, then we have
	 * to add a fake transition to the end of the subgroup
	 */
	int already_a_real_transition_to_end=(-1!=vector_int_contains(transitions,end));
	vector_int_add_if_absent(transitions,end);
	vector_int_add_if_absent(reverse->t[end],state);
	if (!already_a_real_transition_to_end) {
		to_remove(t,state,end);
	}
}
while (to_be_removed!=NULL) {
	vector_int_remove(transitions,to_be_removed->n);
	vector_int_remove(reverse->t[to_be_removed->n],state);
	//(*t)[state]=sorted_insert(to_be_removed->n,(*t)[state]);
	to_restore(t,state,to_be_removed->n);
	struct list_int* tmp=to_be_removed;
	to_be_removed=to_be_removed->next;
	tmp->next=NULL;
	free_list_int(tmp);
}
mark_local[state]=VISITED;
return 1;
}


static void get_last_states_before_end(Grf* grf,int current,int end,struct list_int* *list);
static void ensure_graph_form_rec(Grf* grf,TransitionModifs* t,ReverseTransitions* reverse,
		                          int start,int end);

/**
 * This function considers a list of states that all have a transition to state #end. It
 * replaces those transitions by paths through a new state #n so that finally there is
 * only this single state that will have a transition to state #end.
 */
static int lemonize(Grf* grf,struct list_int* subends,int end,ReverseTransitions* reverse,
					TransitionModifs* t) {
int n=grf->n_states;
(grf->n_states)++;
/* We enlarge the state array */
grf->states=(GrfState**)realloc(grf->states,grf->n_states*sizeof(GrfState*));
if (grf->states==NULL) {
	fatal_alloc_error("lemonize");
}
/* We also have to enlarge reverse */
reverse->n++;
reverse->t=(vector_int**)realloc(reverse->t,reverse->n*sizeof(vector_int*));
if (reverse->t==NULL) {
	fatal_alloc_error("lemonize");
}
reverse->t[n]=new_vector_int();
/* And also *t */
resize(t,grf->n_states);
/* Now, we create the new state with content "<E>" */
GrfState* state=new_GrfState("\"<E>\"",0,0,0,0);
grf->states[n]=state;
/* We add the transition to state #end */
vector_int_add(state->transitions,end);
vector_int_add(reverse->t[end],n);
/* And we replace transitions to state #end by transitions to state #n */
while (subends!=NULL) {
	/* We remove the old transition */
	vector_int_remove(grf->states[subends->n]->transitions,end);
	vector_int_remove(reverse->t[end],subends->n);
	/*error("lemonizing between %S and %S\n",grf->states[subends->n]->box_content,
			grf->states[end]->box_content);
	*/
	int was_a_t_one=was_one_to_remove(t,subends->n,end);
	//error("was a t one=%d\n",was_a_t_one);
	/* Replacing it by the new one */
	vector_int_add(grf->states[subends->n]->transitions,n);
	vector_int_add(reverse->t[n],subends->n);
	if (was_a_t_one) {
		//(*t)[subends->n]=sorted_insert(n,(*t)[subends->n]);
		to_remove(t,subends->n,n);
	}
	subends=subends->next;
}
return n;
}


/**
 * Here, we are sure, that there is no factorizing state between start and end.
 */
static void ensure_graph_form_lemon(Grf* grf,TransitionModifs* t,ReverseTransitions* reverse,
		                          int start,int end) {
char* mark_global=(char*)calloc(grf->n_states,sizeof(char));
if (mark_global==NULL) {
	fatal_alloc_error("ensure_graph_form_lemon");
}
char* mark_local=(char*)calloc(grf->n_states,sizeof(char));
if (mark_local==NULL) {
	fatal_alloc_error("ensure_graph_form_lemon");
}
vector_int* transitions=grf->states[start]->transitions;
for (int i=0;i<transitions->nbelems;i++) {
	memset(mark_local,0,grf->n_states);
	check_states(grf,t,transitions->tab[i],mark_global,mark_local,end,reverse);
	for (int j=0;j<grf->n_states;j++) {
		if (mark_local[j]) {
			mark_local[j]=0;
			mark_global[j]=1;
		}
	}
}
free(mark_global);
free(mark_local);

/* We do recursively the same on subgroups */
for (int i=0;i<transitions->nbelems;i++) {
	int substart=transitions->tab[i];
	struct list_int* subends=NULL;
	get_last_states_before_end(grf,substart,end,&subends);
	if (subends==NULL) {
		fatal_error("Unexpected empty list in ensure_graph_form_lemon\n");
	}
	int subend;
	if (subends->next==NULL) {
		subend=subends->n;
	} else {
		subend=lemonize(grf,subends,end,reverse,t);
	}
	free_list_int(subends);
	ensure_graph_form_rec(grf,t,reverse,substart,subend);
}
}

/**
 * General case.
 */
static void ensure_graph_form_rec(Grf* grf,TransitionModifs* t,ReverseTransitions* reverse,
		                          int start,int end) {
int n_factorizing_states;
if (start==end) return;
int* factorizing=compute_factorizing_states(grf,start,end,reverse,&n_factorizing_states);
if (factorizing==NULL) {
	fatal_error("Cannot compute factorizing states in ensure_graph_form_rec\n");
}
for (int i=0;i<n_factorizing_states-1;i++) {
	ensure_graph_form_lemon(grf,t,reverse,factorizing[i],factorizing[i+1]);
}
free(factorizing);
}


/**
 * This function considers an acyclic graph and makes sure
 * that it is in the form expected by process_groups. This function
 * may remove transitions and also insert some fake ones.
 */
static void ensure_graph_form(Grf* grf,TransitionModifs* t) {
ReverseTransitions* reverse=compute_reverse_transitions(grf);
ensure_graph_form_rec(grf,t,reverse,0,1);
free_ReverseTransitions(reverse);
}


/**
 * This function looks for every transition that must be ignored in order
 * to make the given grf acyclic. The grf is modified by removing such transitions,
 * and the function returns a list array, so that t[x] containing
 * y means that transition from x to y was ignored, so that we will now which
 * transitions must be restored before saving the grf. We also ignore transitions
 * to states that are not co-accessible.
 *
 * The function may also insert some fake transitions that should be removed
 * at the end of the whole processing. So, the rule is:
 *
 *   for any transition listed in the result of this function:
 *      - if the transition exists, then remove it
 *      - otherwise, add it
 */
static TransitionModifs* compute_ignorable_or_artificial_transitions(Grf* grf) {
TransitionModifs* t=new_TransitionModifs(grf->n_states);
int* mark=(int*)calloc(grf->n_states,sizeof(int));
if (mark==NULL) {
	fatal_alloc_error("compute_ignorable_transitions");
}
/* First, we remove back transitions */
remove_back_transitions(grf,0,mark,t);
/* Then we remove transitions to states that are not co-accessible */
deal_with_non_coaccessibility(grf,mark,t);
/* Then, we use state ranks to detect transverse transitions */
remove_transverse_transitions(grf,mark,t);
free(mark);
ensure_graph_form(grf,t);
return t;
}


static void free_ignorable_or_artificial_transitions(struct list_int** t,int n_states) {
if (t==NULL) return;
for (int i=0;i<n_states;i++) {
	free_list_int(t[i]);
}
free(t);
}



/**
 * Returns an array indicating which states are part of the subgraph from
 * state #start to state #end or NULL if there are some states that are not connected to
 * end.
 */
static int* get_concerned_states(Grf* grf,int start,int end,ReverseTransitions* reverse,int *n) {
int* accessible=(int*)calloc(grf->n_states,sizeof(int));
if (accessible==NULL) {
	fatal_alloc_error("compute_factorizing_states");
}
int* coaccessible=(int*)calloc(grf->n_states,sizeof(int));
if (coaccessible==NULL) {
	fatal_alloc_error("compute_factorizing_states");
}
int* concerned=(int*)malloc(grf->n_states*sizeof(int));
if (concerned==NULL) {
	fatal_alloc_error("compute_factorizing_states");
}
explore_accessibility(grf,start,accessible);
explore_coaccessibility(reverse,end,coaccessible);
if (!accessible[end] || !coaccessible[start]) {
	free(accessible);
	free(coaccessible);
	if (!accessible[end]) error("accessibility ooops on state %S\n",grf->states[end]->box_content);
	if (!coaccessible[start]) error("coaccessibility ooops on state %S\n",grf->states[start]->box_content);
	return NULL;
}
/* If there is no problem, we reuse the accessible array to return the result.
 * By convention, we place start and end at positions 0 and 1 */
int j=2;
concerned[0]=start;
concerned[1]=end;
for (int i=0;i<grf->n_states;i++) {
	if (i==start || i==end) continue;
	if (accessible[i] && coaccessible[i]) {
		concerned[j++]=i;
	}
}
*n=j;
free(accessible);
free(coaccessible);
return concerned;
}


/**
 * Reorganizes the content of the subgraph array according to
 * a topological sort.
 */
static void topological_sort(int* subgraph,int n,Grf* grf) {
int* incoming=(int*)calloc(grf->n_states,sizeof(int));
if (incoming==NULL) {
	fatal_alloc_error("topological_sort");
}
for (int i=0;i<n;i++) {
	/* We must only take into account transitions from concerned states */
	int state=subgraph[i];
	vector_int* transitions=grf->states[state]->transitions;
	for (int j=0;j<transitions->nbelems;j++) {
		incoming[transitions->tab[j]]++;
	}
}
int* renumber=(int*)malloc(n*sizeof(int));
if (renumber==NULL) {
	fatal_alloc_error("topological_sort");
}
int q;
for (q=0;q<n;q++) {
   int old=0;
   /* At each step #q of the algorithm, we look for the first state with no
    * incoming transition, and we renumber it as the new state #q */
   while (old<n && incoming[subgraph[old]]!=0) {
      old++;
   }
   if (old==n) {
	   fatal_error("old>n should not happen in topological_sort\n");
   }
   renumber[old]=q;
   incoming[subgraph[old]]=-1;
   /* Then we decrease the number of incoming transitions of the
    * states that can directly be reached from the current one */
   vector_int* transitions=grf->states[subgraph[old]]->transitions;
   for (int i=0;i<transitions->nbelems;i++) {
      incoming[transitions->tab[i]]--;
   }
}
for (int i=0;i<n;i++) {
	incoming[renumber[i]]=subgraph[i];
}
for (int i=0;i<n;i++) {
	subgraph[i]=incoming[i];
}
free(renumber);
free(incoming);
}


/**
 * Returns an array indicating which states are factorizing between
 * states #start and #end, or NULL if there are some states that are not connected to
 * end.
 */
static int* compute_factorizing_states(Grf* grf,int start,int end,ReverseTransitions* reverse,
										int *n_factorizing_states) {
int n;
int* subgraph=get_concerned_states(grf,start,end,reverse,&n);
if (subgraph==NULL) {
	error("Cannot compute concerned states between _%S_ and _%S_\n",grf->states[start]->box_content,
				grf->states[end]->box_content);
	return NULL;
}
topological_sort(subgraph,n,grf);
int* factorizing=(int*)malloc(grf->n_states*sizeof(int));
if (factorizing==NULL) {
	fatal_alloc_error("compute_factorizing_states");
}
for (int i=0;i<grf->n_states;i++) {
	factorizing[i]=1;
}
for (int i=0;i<n;i++) {
   for (int j=1;j<n;j++) {
	   int src_state=subgraph[i];
	   int dest_state=subgraph[j];
	   int direct_transition=(-1!=vector_int_contains(grf->states[src_state]->transitions,dest_state));
      if (direct_transition) {
         for (int k=i+1;k<j;k++) {
            /* We can do this only because we have performed a
             * topological sort before */
            factorizing[subgraph[k]]=0;
         }
      }
   }
}
int* result=(int*)malloc(grf->n_states*sizeof(int));
*n_factorizing_states=0;
for (int i=0;i<n;i++) {
	if (factorizing[subgraph[i]]) {
		result[(*n_factorizing_states)++]=subgraph[i];
	}
}
free(factorizing);
free(subgraph);
return result;
}


static BoxGroup* new_BoxGroup(GroupType type,int start,int end) {
BoxGroup* g=(BoxGroup*)malloc(sizeof(BoxGroup));
if (g==NULL) {
	fatal_alloc_error("new_BoxGroup");
}
g->type=type;
g->start=start;
g->end=end;
g->subgroups=new_vector_int();
return g;
}


static void free_BoxGroup(BoxGroup* g) {
free_vector_int(g->subgroups);
free(g);
}


static int process_group(Grf* grf,int parent_group,int start,int end,TransitionModifs* t,
					vector_ptr* groups,ReverseTransitions* reverse,
					int* box_width,int* box_height);

/**
 * This function explores the graph from state #current until transitions
 * to state #end are found.
 */
static void get_last_states_before_end(Grf* grf,int current,int end,struct list_int* *list) {
if (current==end) {
	*list=sorted_insert(end,*list);
	return;
}
vector_int* t=grf->states[current]->transitions;
if (vector_int_contains(t,end)!=-1) {
	*list=sorted_insert(current,*list);
	return;
}
for (int i=0;i<t->nbelems;i++) {
	get_last_states_before_end(grf,t->tab[i],end,list);
}
}


/**
 * The same as get_last_states_before_end, but raises a fatal error
 * if there is more than one answer.
 */
static int get_last_state_before_end(Grf* grf,int current,int end) {
struct list_int* l=NULL;
get_last_states_before_end(grf,current,end,&l);
if (l==NULL) {
	fatal_error("Unexpected empty list in get_last_state_before_end\n");
}
if (l->next!=NULL) {
	fatal_error("Unexpected list size >1 in get_last_state_before_end\n");
}
int n=l->n;
free_list_int(l);
return n;
}


static int max(int a,int b) {
return (a>b)?a:b;
}


/**
 * This function takes a subgraph containing no factorizing states,
 * except states #start and #end and it divides it into a conjunction
 * of several subgroups.
 */
static int process_lemon_group(Grf* grf,int parent_group,int start,int end,TransitionModifs* t,
					vector_ptr* groups,ReverseTransitions* reverse,
					int* box_width,int* box_height) {
/* First case, a subgraph made of only two linked states: start --> end */
if (-1!=vector_int_contains(grf->states[start]->transitions,end)) {
	int current_group=groups->nbelems;
	BoxGroup* group=new_BoxGroup(PAIR,start,end);
	group->parent=parent_group;
	group->width=box_width[start]+WIDTH_MARGIN+box_width[end];
	group->height=max(box_height[start],box_height[end]);
	vector_ptr_add(groups,group);
	return current_group;
}
int current_group=groups->nbelems;
BoxGroup* group=new_BoxGroup(LEMON,start,end);
group->parent=parent_group;
group->width=box_width[start]+2*WIDTH_MARGIN+box_width[end];
group->height=max(box_height[start],box_height[end]);
vector_ptr_add_if_absent(groups,group);
vector_int* transitions=grf->states[start]->transitions;
int max_width=0;
int height=0;
for (int i=0;i<transitions->nbelems;i++) {
	int subend=get_last_state_before_end(grf,transitions->tab[i],end);
	int subgroup_index=process_group(grf,current_group,transitions->tab[i],subend,t,groups,
									reverse,box_width,box_height);
	if (subgroup_index==-1) return -1;
	vector_int_add(group->subgroups,subgroup_index);
	BoxGroup* subgroup=(BoxGroup*)groups->tab[subgroup_index];
	if (subgroup->width>max_width) max_width=subgroup->width;
	height=height+subgroup->height+HEIGHT_MARGIN;
}
height=height-HEIGHT_MARGIN;
group->width=group->width+max_width;
group->height=max(group->height,height);
return current_group;
}


/**
 * This function divides boxes between parent_start and parent_end into
 * subgroups Gi, so that parent_start -> G0 -> G1 -> ... -> Gn -> parent_end is
 * equivalent to the subgraph between parent_start and parent_end, modulo
 * the transitions to be ignored. Each Gi is recursively given a width and
 * a height, and finally parent group is given a width and height as well.
 *
 * Returns the number of the main group 1 in case of success, or -1 if something went wrong, which may
 * happen if the given grf has not been fully cleaned by compute_ignorable_transitions.
 */
static int process_group(Grf* grf,int parent_group,int start,int end,TransitionModifs* t,
					vector_ptr* groups,ReverseTransitions* reverse,
					int* box_width,int* box_height) {
if (start==end) {
	/* If we have a single state, then it is a group of size 1 for whose
	 * we can set a width and a height */
	int current_group=groups->nbelems;
	BoxGroup* group=new_BoxGroup(SINGLE,start,end);
	group->parent=parent_group;
	group->width=box_width[start];
	group->height=box_height[start];
	vector_ptr_add(groups,group);
	return current_group;
}
/* Otherwise, we compute factorizing states between parent_start and parent_end */
int n_factorizing;
int* factorizing=compute_factorizing_states(grf,start,end,reverse,&n_factorizing);
if (factorizing==NULL) {
	error("Cannot compute factorizing states between _%S_ and _%S_\n",grf->states[start]->box_content,
			grf->states[end]->box_content);
	return -1;
}
if (n_factorizing==2) {
	/* If there are none other factorizing states that the start and end ones,
	 * then we can directly call process_lemon_group */
	free(factorizing);
	return process_lemon_group(grf,parent_group,start,end,t,groups,reverse,
								box_width,box_height);
}
int current_group=groups->nbelems;
BoxGroup* group=new_BoxGroup(SEQUENTIAL,start,end);
group->parent=parent_group;
group->width=0;
group->height=0;
vector_ptr_add(groups,group);
for (int i=1;i<n_factorizing;i++) {
	int BB=factorizing[i];
	int subgroup_index=process_lemon_group(grf,current_group,factorizing[i-1],factorizing[i],t,
									groups,reverse,box_width,box_height);
	vector_int_add(group->subgroups,subgroup_index);
	if (subgroup_index==-1) {
		free(factorizing);
		return 0;
	}
	BoxGroup* subgroup=(BoxGroup*)groups->tab[subgroup_index];
	/* In a group made of successive lemon ones, some factorizing states
	 * are included in two lemons. For instance, if we have
	 *
	 * A --lemon--> B --lemon--> C
	 *
	 * the width of A->C should be width(A->B)+width(B->C)-width(B), because
	 * B's width is taken twice into account in both A->B and B->C
	 */
	group->width=group->width+subgroup->width;
	if (BB!=end) {
		group->width=group->width-box_width[BB];
	}
	if (subgroup->height>group->height) {
		group->height=subgroup->height;
	}
}
free(factorizing);
return current_group;
}


/**
 * This function places boxes of the group #n within the rectangle
 *  whose upper left corner is shiftX,shiftY and whose dimensions
 *  are the width and height of the given group.
 */
static void place_groups(Grf* grf,int n,int shiftX,int shiftY,vector_ptr* groups,
						int* is_placed,int* box_width,int* box_height) {
BoxGroup* bg=(BoxGroup*)groups->tab[n];
GrfState* box_start=grf->states[bg->start];
GrfState* box_end=grf->states[bg->end];
/* Whatever happen, we must place the start box on the left, at mid-height */
if (!is_placed[bg->start]) {
	is_placed[bg->start]=1;
	box_start->x=shiftX;
	box_start->y=shiftY+bg->height/2;
}
if (bg->type==SINGLE) {
	/* A single box is a group with start==end, so we're done */
	return;
}
/* If we have a multi-box group, we must place the end box
 * on the right, at mid-height */
if (!is_placed[bg->end]) {
	is_placed[bg->end]=1;
	box_end->x=shiftX+bg->width-box_width[bg->end];
	box_end->y=shiftY+bg->height/2;
}
if (bg->type==PAIR) {
	/* Once we have placed start and end, we're done if the subgroup is a pair one */
	return;
}
/* Now, we have to place the content of the group */
if (bg->type==SEQUENTIAL) {
	/* If we have a group sequence, we first align all the factorizing states */
	int currentX=box_start->x;
	for (int i=0;i<bg->subgroups->nbelems;i++) {
		int subgroup_index=bg->subgroups->tab[i];
		BoxGroup* subgroup=(BoxGroup*)groups->tab[subgroup_index];
		if (!is_placed[subgroup->start]) {
			GrfState* sub_start=grf->states[subgroup->start];
			sub_start->x=currentX;
			sub_start->y=box_start->y;
			is_placed[subgroup->start]=1;
		}
		currentX=currentX+subgroup->width-box_width[subgroup->end];
		if (!is_placed[subgroup->end]) {
			GrfState* sub_end=grf->states[subgroup->end];
			sub_end->x=currentX;
			sub_end->y=box_start->y;
			is_placed[subgroup->end]=1;
		}
	}
	/* Now, we recursively place all the subgroups */
	currentX=shiftX;
	for (int i=0;i<bg->subgroups->nbelems;i++) {
		int subgroup_index=bg->subgroups->tab[i];
		BoxGroup* subgroup=(BoxGroup*)groups->tab[subgroup_index];
		int y=shiftY+(bg->height-subgroup->height)/2;
		place_groups(grf,subgroup_index,currentX,y,groups,is_placed,box_width,box_height);
		currentX=currentX+subgroup->width-box_width[subgroup->end];
	}
	return;
}
/* If we have a lemon group, we place all the subgroups */
int currentY=shiftY;
shiftX=shiftX+box_width[bg->start]+WIDTH_MARGIN;
for (int i=0;i<bg->subgroups->nbelems;i++) {
	int subgroup_index=bg->subgroups->tab[i];
	BoxGroup* subgroup=(BoxGroup*)groups->tab[subgroup_index];
	place_groups(grf,subgroup_index,shiftX,currentY,groups,is_placed,box_width,box_height);
	currentY=currentY+subgroup->height+HEIGHT_MARGIN;
}
}


static void compute_box_dimensions(Grf* grf,int* box_width,int* box_height) {
for (int i=0;i<grf->n_states;i++) {
	vector_ptr* lines=tokenize_box_content(grf->states[i]->box_content);
	if (lines==NULL) {
		fatal_error("Unexpected NULL lines in compute_box_dimensions for box content %S\n",
				grf->states[i]->box_content);
	}
	int max_line=0;
	for (int j=0;j<lines->nbelems;j++) {
		int tmp=u_strlen((unichar*)(lines->tab[j]));
		if (tmp>max_line) max_line=tmp;
	}
	if (i==1) {
		box_width[i]=20;
		box_height[i]=20;
	} else {
		box_width[i]=WIDTH_PER_CHAR*max_line;
		box_height[i]=HEIGHT_PER_CHAR*lines->nbelems;
	}
	free_vector_ptr(lines,free);
}
}



/**
 * This function tries to optimize the location of each box to make
 * the grf more readable. The principle is to subdivide boxes into box
 * disjunct groups.
 */
static void organize_boxes(Grf* grf) {
TransitionModifs * t=compute_ignorable_or_artificial_transitions(grf);
vector_ptr* groups=new_vector_ptr(grf->n_states);
int* box_width=(int*)malloc(grf->n_states*sizeof(int));
if (box_width==NULL) {
	fatal_alloc_error("organize_boxes");
}
int* box_height=(int*)malloc(grf->n_states*sizeof(int));
if (box_height==NULL) {
	fatal_alloc_error("organize_boxes");
}
compute_box_dimensions(grf,box_width,box_height);
/* Now we do the job */
ReverseTransitions* reverse=compute_reverse_transitions(grf);
process_group(grf,-1,0,1,t,groups,reverse,box_width,box_height);
free_ReverseTransitions(reverse);
int* is_placed=(int*)calloc(grf->n_states,sizeof(int));
if (is_placed==NULL) {
	fatal_alloc_error("organize_boxes");
}
place_groups(grf,0,40,40,groups,is_placed,box_width,box_height);
u_sprintf(grf->size,"SIZE %d %d",((BoxGroup*)groups->tab[0])->width+4*WIDTH_MARGIN,
		((BoxGroup*)groups->tab[0])->height+4*HEIGHT_MARGIN);
free(is_placed);
free(box_width);
free(box_height);
/* We restore the ignored transitions */
for (int i=0;i<grf->n_states;i++) {
	vector_int* transitions=grf->states[i]->transitions;
	struct list_int* tmp=t->to_restore[i];
	while (tmp!=NULL) {
		if (-1==vector_int_contains(transitions,tmp->n)) {
			/* If the transition does not exist, we add it */
			vector_int_add(transitions,tmp->n);
		}
		tmp=tmp->next;
	}
}
/* And remove the added fake ones */
for (int i=0;i<grf->n_states;i++) {
	vector_int* transitions=grf->states[i]->transitions;
	struct list_int* tmp=t->to_remove[i];
	while (tmp!=NULL) {
		/* If the transition exists, we remove it */
		vector_int_remove(transitions,tmp->n);
		tmp=tmp->next;
	}
}
/* And we clean our stuffs */
free_vector_ptr(groups,(void(*)(void*))free_BoxGroup);
free_TransitionModifs(t);
}


/**
 * This function takes a grf generated from a sequence list, and tries
 * to make it more beautiful for a human reader.
 */
void beautify(Grf* grf,Alphabet* alph) {
if (grf==NULL) return;
merge_case_equivalent_boxes(grf,alph);
merge_pseudo_case_equivalent_boxes(grf,alph);
//merge_box_pair(grf,alph);
merge_boxes_with_same_transition(grf);
replace_transition_by_E(grf);
/* 6: si A, B et C sont toutes reliées à X, Y et Z, utiliser une boîte <E> intermédiaire pour
 *    réduire le nombre de transitions
 */
remove_isolated_boxes(grf);
organize_boxes(grf);
}
