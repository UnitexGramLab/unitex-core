/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
	int ok=1;
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
 * This function takes a grf generated from a sequence list, and tries
 * to make it more beautiful for a human reader.
 */
void beautify(Grf* grf,Alphabet* alph) {
if (grf==NULL) return;
merge_case_equivalent_boxes(grf,alph);
merge_pseudo_case_equivalent_boxes(grf,alph);
merge_box_pair(grf,alph);
merge_boxes_with_same_transition(grf);
replace_transition_by_E(grf);
/* 6: si A, B et C sont toutes reliées à X, Y et Z, utiliser une boîte <E> intermédiaire pour
 *    réduire le nombre de transitions
 *
 * en dernier: positionner joliment les boîtes
 *  => il faudra détecter les plus grands sous-groupes
 */
remove_isolated_boxes(grf);
}
