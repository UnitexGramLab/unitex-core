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

#include <stdlib.h>
#include "Grf_lib.h"
#include "GrfSvn_lib.h"
#include "Unicode.h"


static int cmp_grf_states(Grf*,Grf*);


static DiffOp* new_DiffOp(DiffOpType type) {
DiffOp* d=(DiffOp*)malloc(sizeof(DiffOp));
if (d==NULL) fatal_alloc_error("new_DiffOp");
d->op_type=type;
return d;
}


static DiffOp* new_DiffOp(const char* property) {
DiffOp* d=new_DiffOp(DIFF_PROPERTY);
strcpy(d->property,property);
return d;
}


static void free_DiffOp(DiffOp* op) {
free(op);
}


/**
 * Prints the given diff op to the given file.
 */
static void print_diff_op(U_FILE* f,DiffOp* d) {
u_fprintf(f,"%c",d->op_type);
switch (d->op_type) {
case DIFF_PROPERTY: u_fprintf(f," %s\n",d->property); break;
case DIFF_BOX_REMOVED: u_fprintf(f," %d\n",d->box_base); break;
case DIFF_BOX_ADDED: u_fprintf(f," %d\n",d->box_dest); break;
case DIFF_BOX_MOVED: u_fprintf(f," %d %d\n",d->box_base,d->box_dest); break;
case DIFF_BOX_CONTENT_CHANGED: u_fprintf(f," %d %d\n",d->box_base,d->box_dest); break;
case DIFF_TRANSITION_REMOVED: u_fprintf(f," %d %d %d %d\n",d->box_base,d->box_dest,d->dest_base,d->dest_dest); break;
case DIFF_TRANSITION_ADDED: u_fprintf(f," %d %d %d %d\n",d->box_base,d->box_dest,d->dest_base,d->dest_dest); break;
}
}


static void add_diff_op_property(GrfDiff* diff,const char* prop) {
vector_ptr_add(diff->diff_ops,new_DiffOp(prop));
}


static void add_diff_box_removed(GrfDiff* diff,int n) {
DiffOp* d=new_DiffOp(DIFF_BOX_REMOVED);
d->box_base=n;
vector_ptr_add(diff->diff_ops,d);
}


static void add_diff_box_added(GrfDiff* diff,int n) {
DiffOp* d=new_DiffOp(DIFF_BOX_ADDED);
d->box_dest=n;
vector_ptr_add(diff->diff_ops,d);
}


static void add_diff_box_moved(GrfDiff* diff,int base,int dest) {
DiffOp* d=new_DiffOp(DIFF_BOX_MOVED);
d->box_base=base;
d->box_dest=dest;
vector_ptr_add(diff->diff_ops,d);
}


static void add_diff_transition_removed(GrfDiff* diff,int src_base,int src_dest,int dst_base,int dst_dest) {
DiffOp* d=new_DiffOp(DIFF_TRANSITION_REMOVED);
d->box_base=src_base;
d->box_dest=src_dest;
d->dest_base=dst_base;
d->dest_dest=dst_dest;
vector_ptr_add(diff->diff_ops,d);
}


static void add_diff_transition_added(GrfDiff* diff,int src_base,int src_dest,int dst_base,int dst_dest) {
DiffOp* d=new_DiffOp(DIFF_TRANSITION_ADDED);
d->box_base=src_base;
d->box_dest=src_dest;
d->dest_base=dst_base;
d->dest_dest=dst_dest;
vector_ptr_add(diff->diff_ops,d);
}


static void add_diff_box_content_changed(GrfDiff* diff,int base,int dest) {
DiffOp* d=new_DiffOp(DIFF_BOX_CONTENT_CHANGED);
d->box_base=base;
d->box_dest=dest;
vector_ptr_add(diff->diff_ops,d);
}


/**
 * Performs a diff3 on strings. Returns 0 in case of success, 1 otherwise.
 */
static int diff3_grf_strings(unichar* mine,unichar* base,unichar* other,unichar* result) {
if (!u_strcmp(mine,base)) {
	/* If I didn't modify the string, then the other one version can be taken into account */
	u_strcpy(result,other);
	return 0;
}
if (!u_strcmp(other,base)) {
	/* If my version differs but not the other's one, we take my version */
	u_strcpy(result,mine);
	return 0;
}
if (!u_strcmp(mine,other)) {
	/* If both versions differ from the base one but are the same, there is no conflict */
	u_strcpy(result,mine);
	return 0;
}
/* Conflict */
return 1;
}


/**
 * Performs a diff3 on the given graph headers. In case of success,
 * it returns 0 and save the resulting header in 'result'. In case of
 * conflicts, it returns 1.
 */
static int diff3_grf_headers(Grf* mine,Grf* base,Grf* other,Grf* result) {
if (diff3_grf_strings(mine->size,base->size,other->size,result->size)) return 1;
if (diff3_grf_strings(mine->font,base->font,other->font,result->font)) return 1;
if (diff3_grf_strings(mine->ofont,base->ofont,other->ofont,result->ofont)) return 1;
if (diff3_grf_strings(mine->bcolor,base->bcolor,other->bcolor,result->bcolor)) return 1;
if (diff3_grf_strings(mine->fcolor,base->fcolor,other->fcolor,result->fcolor)) return 1;
if (diff3_grf_strings(mine->acolor,base->acolor,other->acolor,result->acolor)) return 1;
if (diff3_grf_strings(mine->scolor,base->scolor,other->scolor,result->scolor)) return 1;
if (diff3_grf_strings(mine->ccolor,base->ccolor,other->ccolor,result->ccolor)) return 1;
if (diff3_grf_strings(mine->dboxes,base->dboxes,other->dboxes,result->dboxes)) return 1;
if (diff3_grf_strings(mine->dframe,base->dframe,other->dframe,result->dframe)) return 1;
if (diff3_grf_strings(mine->ddate,base->ddate,other->ddate,result->ddate)) return 1;
if (diff3_grf_strings(mine->dfile,base->dfile,other->dfile,result->dfile)) return 1;
if (diff3_grf_strings(mine->ddir,base->ddir,other->ddir,result->ddir)) return 1;
if (diff3_grf_strings(mine->drig,base->drig,other->drig,result->drig)) return 1;
if (diff3_grf_strings(mine->drst,base->drst,other->drst,result->drst)) return 1;
if (diff3_grf_strings(mine->fits,base->fits,other->fits,result->fits)) return 1;
if (diff3_grf_strings(mine->porient,base->porient,other->porient,result->porient)) return 1;
return 0;
}


/**
 * Performs a diff3 on the given normalized graph states. In case of success,
 * it returns 0 and save the resulting states in 'result'. In case of
 * conflicts, it returns 1.
 */
static int diff3_grf_states(Grf* mine,Grf* base,Grf* other,Grf* result) {
if (!cmp_grf_states(mine,base)) {
	/* If I didn't modify the string, then the other one version can be taken into account */
	cpy_grf_states(result,other);
	return 0;
}
if (!cmp_grf_states(other,base)) {
	/* If my version differs but not the other's one, we take my version */
	cpy_grf_states(result,mine);
	return 0;
}
if (!cmp_grf_states(mine,other)) {
	/* If both versions differ from the base one but are the same, there is no conflict */
	cpy_grf_states(result,mine);
	return 0;
}
/* Conflict */
return 1;
}


/**
 * Returns the merged grf or NULL in case of unresolved conflicts. In such a case,
 * conflicts are described in f.
 */
static Grf* diff3_internal(U_FILE* f,Grf* mine,Grf* base,Grf* other) {
GrfDiff* base_mine=grf_diff(base,mine);
if (base_mine->diff_ops->nbelems==0) {
	/* base==mine ? => return other */
	free_GrfDiff(base_mine);
	return dup_Grf(other);
}
GrfDiff* base_other=grf_diff(base,other);
if (base_other->diff_ops->nbelems==0) {
	/* base==other ? => return mine */
	free_GrfDiff(base_mine);
	free_GrfDiff(base_other);
	return dup_Grf(mine);
}
GrfDiff* mine_other=grf_diff(mine,other);
if (mine_other->diff_ops->nbelems==0) {
	/* mine==other ? => return mine */
	free_GrfDiff(base_mine);
	free_GrfDiff(base_other);
	free_GrfDiff(mine_other);
	return dup_Grf(mine);
}


free_GrfDiff(base_mine);
free_GrfDiff(base_other);
free_GrfDiff(mine_other);
return NULL;
#if 0
Grf* result=new_Grf();
if (1==diff3_grf_headers(mine,base,other,result)) {
	free_Grf(result);
	return NULL;
}
/*normalize_grf(mine);
normalize_grf(base);
normalize_grf(other);*/
if (0==diff3_grf_states(mine,base,other,result)) {
	/* First, we try a raw comparison. If either mine
	 * or other's version is the same as the base one, we
	 * simply consider the differing version as being
	 * the new one. This simple operation prevents from
	 * fake conflicts that only occur because identical graph state sets
	 * may not be numbered the same in different .grf files */
	return result;
}
/* If there is no simple answer to the diff3, we look for a more complicated one */
GrfDiff* diff1=grf_diff(base,mine);

free_GrfDiff(diff1);
free_Grf(result);
return NULL;
#endif
}


/**
 * Performs a diff3 on the given graphs. In case of success,
 * it returns 0 and prints on the standard output the UTF16LE
 * graph that results from the fusion. In case of conflicts,
 * it returns 1 and prints nothing.
 */
int diff3(U_FILE* f,Grf* mine,Grf* base,Grf* other) {
Grf* result=diff3_internal(f,mine,base,other);
if (result==NULL) return 1;
save_Grf(f,result);
free_Grf(result);
return 0;
}


/**
 * Comparison function for grf states:
 * - compare box contents
 * - if equality, compare number of outgoing transitions
 * - if equality, compare x
 * - if equality, compare y
 */
static int cmp_states(GrfState* x,GrfState* y) {
int ret=u_strcmp(x->box_content,y->box_content);
if (ret) return ret;
ret=x->n_transitions-y->n_transitions;
if (ret) return ret;
ret=x->x-y->x;
if (ret) return ret;
return x->y-y->y;
}


/**
 * Sorts the outgoing state array of state #n.
 */
static void sort_states(Grf* grf,int* t,int size) {
for (int i=0;i<size;i++) {
	int index_min=i;
	for (int j=i;j<size;j++) {
		if (cmp_states(grf->states[t[j]],grf->states[t[index_min]])<0) index_min=j;
	}
	int tmp=t[i];
	t[i]=t[index_min];
	t[index_min]=tmp;
}
}


/**
 * This function is called on state #n that is supposed to have already
 * been renumbered. Then, it renumbers all outgoing states that have not
 * already be renumbered themselves. *current_pos_in_mark is next free cell
 * in the virtual renumbering array.
 */
static void renumber_outgoing_states(Grf* grf,int* mark,int n,int *current_pos_in_mark) {
if (n==1) {
	/* Nothing to do for the final state */
	return;
}
int old_mark=0;
if (n==0) {
	/* If we have already explored the initial state, we do nothing */
	if (mark[0]==0) return;
	/* If we are already exploring the initial state, we do nothing */
	if (mark[0]!=-2) return;
	/* We mark the initial so that we know we already are exploring it */
	mark[0]=1;
} else {
	/* If we have already normalized a normal state, we do nothing */
	if (mark[n]>2) return;
	/* If we are already normalizing a normal state, we do nothing */
	if (mark[n]==0) return;
	old_mark=-mark[n];
	mark[n]=0;
}
if (n>1 && mark[n]>1) {
	/* If we have already normalized a regular state */
	return;
}
sort_states(grf,grf->states[n]->transitions,grf->states[n]->n_transitions);
int* t=grf->states[n]->transitions;
for (int i=0;i<grf->states[n]->n_transitions;i++) {
	if (mark[t[i]]==-1) {
		/* If the state has not already been renumbered, we do it now */
		mark[t[i]]=-(*current_pos_in_mark);
		(*current_pos_in_mark)++;
	}
}
/* Then, we normalize outgoing states */
for (int i=0;i<grf->states[n]->n_transitions;i++) {
	renumber_outgoing_states(grf,mark,t[i],current_pos_in_mark);
}
/* Finally, we mark the current state has normalized */
if (n==0) mark[0]=0;
else {
	mark[n]=old_mark;
}
}


/**
 * This function normalizes the given grf, with a kind of state sort.
 * This is useful to match graph identity for graphs that only differ
 * with state numbering as it may occur if a graph has been saved after
 * doing and then undoing something in the graph editor.
 */
void normalize_grf(Grf* grf) {
int* mark=(int*)malloc(grf->n_states*sizeof(int));
if (mark==NULL) fatal_alloc_error("normalize_grf");
/* States 0 and 1 must never be renumbered */
mark[0]=-2;
mark[1]=1;
int n=2;
for (int i=2;i<grf->n_states;i++) mark[i]=-1;
renumber_outgoing_states(grf,mark,0,&n);
/* Then, we deal with unreachable states that have not been renumbered */
int* unreachable=(int*)malloc(grf->n_states*sizeof(int));
if (unreachable==NULL) fatal_alloc_error("normalize_grf");
int n_unreachable=0;
for (int i=0;i<grf->n_states;i++) {
	if (mark[i]<0) {
		/* We mark the unreachable states */
		unreachable[n_unreachable++]=i;
	}
}
/* We sort those unreachable states */
sort_states(grf,unreachable,n_unreachable);
/* And we renumber them */
for (int i=0;i<n_unreachable;i++) {
	mark[unreachable[i]]=n++;
}
free(unreachable);
/* Now, we swap states to sort them according to the renumbering */
GrfState** tmp=(GrfState**)malloc(grf->n_states*sizeof(GrfState*));
for (int i=0;i<grf->n_states;i++) {
	tmp[mark[i]]=grf->states[i];
}
for (int i=0;i<grf->n_states;i++) {
	grf->states[i]=tmp[i];
}
free(tmp);
/* And finally, we have to renumber transitions */
for (int i=0;i<grf->n_states;i++) {
	int* t=grf->states[i]->transitions;
	for (int j=0;j<grf->states[i]->n_transitions;j++) {
		t[j]=mark[t[j]];
	}
}

free(mark);
}


/**
 * Comparing two graph state sets. Returns 0 if equality; 1 otherwise.
 */
static int cmp_grf_states(Grf* a,Grf* b) {
if (a->n_states!=b->n_states) return 1;
for (int i=0;i<a->n_states;i++) {
	if (0!=cmp_states(a->states[i],b->states[i])) return 1;
}
return 0;
}


/**
 * Fills the given array with g's reverse transitions.
 */
static void compute_reverse_transitions(Grf* g,vector_int** t) {
for (int i=0;i<g->n_states;i++) {
	t[i]=new_vector_int(1);
}
for (int i=0;i<g->n_states;i++) {
	GrfState* s=g->states[i];
	for (int j=0;j<s->n_transitions;j++) {
		vector_int_add(t[s->transitions[j]],i);
	}
}
}


/**
 * Creates a GrfDiff object. Renumbering arrays
 * are initialized with 0 and 1 for states 0 and 1, and -1
 * for all other states. Reverse transitions are computed.
 */
static GrfDiff* new_GrfDiff(Grf* base,Grf* dest) {
int n_base=base->n_states;
int n_dest=dest->n_states;
GrfDiff* d=(GrfDiff*)malloc(sizeof(GrfDiff));
if (d==NULL) fatal_alloc_error("new_GrfDiff");
d->diff_ops=new_vector_ptr();
d->size_base_to_dest=n_base;
d->size_dest_to_base=n_dest;
/* Initializing renumber arrays */
d->base_to_dest=(int*)malloc(n_base*sizeof(int));
if (d->base_to_dest==NULL) fatal_alloc_error("new_GrfDiff");
d->base_to_dest[0]=0;
d->base_to_dest[1]=1;
for (int i=2;i<n_base;i++) {
	d->base_to_dest[i]=-1;
}
d->dest_to_base=(int*)malloc(n_dest*sizeof(int));
if (d->dest_to_base==NULL) fatal_alloc_error("new_GrfDiff");
d->dest_to_base[0]=0;
d->dest_to_base[1]=1;
for (int i=2;i<n_dest;i++) {
	d->dest_to_base[i]=-1;
}
/* Initializing reverse transition arrays */
d->reverse_transitions_base=(vector_int**)malloc(n_base*sizeof(vector_int*));
if (d->reverse_transitions_base==NULL) fatal_alloc_error("new_GrfDiff");
compute_reverse_transitions(base,d->reverse_transitions_base);
d->reverse_transitions_dest=(vector_int**)malloc(n_dest*sizeof(vector_int*));
if (d->reverse_transitions_dest==NULL) fatal_alloc_error("new_GrfDiff");
compute_reverse_transitions(dest,d->reverse_transitions_dest);
return d;
}


/**
 * Frees all the memory associated to the given GrfDiff object.
 */
void free_GrfDiff(GrfDiff* g) {
if (g==NULL) return;
free_vector_ptr(g->diff_ops,(void(*)(void*))free_DiffOp);
free(g->base_to_dest);
free(g->dest_to_base);
for (int i=0;i<g->size_base_to_dest;i++) {
	free_vector_int(g->reverse_transitions_base[i]);
}
free(g->reverse_transitions_base);
for (int i=0;i<g->size_dest_to_base;i++) {
	free_vector_int(g->reverse_transitions_dest[i]);
}
free(g->reverse_transitions_dest);
free(g);
}


/**
 * Returns 1 if the given states seem to have the same incoming transitions.
 * The comparison ignore loop transitions, and takes renumbering into account.
 */
static int same_incoming_transitions(GrfDiff* diff,int base,int dest) {
vector_int* v_base=diff->reverse_transitions_base[base];
vector_int* v_dest=diff->reverse_transitions_dest[dest];
/* First, we test if all base transitions are included in dest ones,
 * ignoring the loop transition, if any */
for (int i=0;i<v_base->nbelems;i++) {
	if (v_base->tab[i]==base) {
		/* We skip loop transitions */
		continue;
	}
	int renumbered_incoming_state=diff->base_to_dest[v_base->tab[i]];
	if (renumbered_incoming_state==-1) {
		/* If we have an incoming transition from an unknown box,
		 * we have no hope to succeed */
		return 0;
	}
	if (!vector_int_contains(v_dest,renumbered_incoming_state)) {
		return 0;
	}
}
/* Then we test if all dest transitions are included in base ones,
 * ignoring the loop transition, if any */
for (int i=0;i<v_dest->nbelems;i++) {
	if (v_dest->tab[i]==dest) {
		/* We skip loop transitions */
		continue;
	}
	int renumbered_incoming_state=diff->dest_to_base[v_dest->tab[i]];
	if (renumbered_incoming_state==-1) {
		/* If we have an incoming transition from an unknown box,
		 * we have no hope to succeed */
		return 0;
	}
	if (!vector_int_contains(v_base,renumbered_incoming_state)) {
		return 0;
	}
}
if (base==15 && dest==6) fatal_error("ooops\n");
return 1;
}


/**
 * Returns 1 if the given states seem to have the same outgoing transitions.
 * The comparison ignore loop transitions, and takes renumbering into account.
 */
static int same_outgoing_transitions(GrfDiff* diff,GrfState* base,GrfState* dest,
		int base_index,int dest_index) {
/* First, we test if all base transitions are included in dest ones,
 * ignoring the loop transition, if any */
for (int i=0;i<base->n_transitions;i++) {
	if (base->transitions[i]==base_index) {
		/* We skip loop transitions */
		continue;
	}
	int renumbered_outgoing_state=diff->base_to_dest[base->transitions[i]];
	if (renumbered_outgoing_state==-1) {
		/* If we have an outgoing transition to an unknown box,
		 * we have no hope to succeed */
		return 0;
	}
	int j;
	for (j=0;j<dest->n_transitions;j++) {
		if (renumbered_outgoing_state==dest->transitions[j]) break;
	}
	if (j==dest->n_transitions) {
		/* Fail to find ? */
		return 0;
	}
}
/* Then, we test if all dest transitions are included in base ones,
 * ignoring the loop transition, if any */
for (int i=0;i<dest->n_transitions;i++) {
	if (dest->transitions[i]==dest_index) {
		/* We skip loop transitions */
		continue;
	}
	int renumbered_outgoing_state=diff->dest_to_base[dest->transitions[i]];
	if (renumbered_outgoing_state==-1) {
		/* If we have an outgoing transition to an unknown box,
		 * we have no hope to succeed */
		return 0;
	}
	int j;
	for (j=0;j<base->n_transitions;j++) {
		if (renumbered_outgoing_state==base->transitions[j]) break;
	}
	if (j==base->n_transitions) {
		/* Fail to find ? */
		return 0;
	}
}
return 1;
}


/**
 * This function tries to match boxes on the basis of the criteria described by
 * the parameters. This function only consider boxes that have not already been
 * matched.
 */
static void find_correspondance(GrfDiff* diff,Grf* base,Grf* dest,
		int coord,     /* compare coordinates */
		int content,   /* compare box content */
		int incoming,  /* compare incoming transitions */
		int outgoing,  /* compare outgoing transitions */
		int index,     /* compare box indices in .grf files */
		int ignore_comment_boxes
		) {
for (int i=2;i<diff->size_base_to_dest;i++) {
	if (diff->base_to_dest[i]!=-1) {
		/* If the base state has already been matched, we ignore it */
		continue;
	}
	GrfState* base_state=base->states[i];
	for (int j=2;j<diff->size_dest_to_base;j++) {
		if (diff->dest_to_base[j]!=-1) {
			/* If the dest box has already been matched, we ignore it */
			continue;
		}
		GrfState* dest_state=dest->states[j];
		int base_is_comment_box=base_state->n_transitions==0
				&& diff->reverse_transitions_base[i]->nbelems==0;
		int dest_is_comment_box=dest_state->n_transitions==0
				&& diff->reverse_transitions_dest[j]->nbelems==0;
		if (ignore_comment_boxes && (base_is_comment_box || dest_is_comment_box)) continue;
		if (coord && (base_state->x!=dest_state->x || base_state->y!=dest_state->y)) {
			continue;
		}
		if (content && u_strcmp(base_state->box_content,dest_state->box_content)) {
			continue;
		}
		if (incoming && !same_incoming_transitions(diff,i,j)) {
			continue;
		}
		if (outgoing && !same_outgoing_transitions(diff,base_state,dest_state,i,j)) {
			continue;
		}
		if (index && i!=j) {
			continue;
		}
		/* If we get there, we have matched all the required criteria,
		 * so we have matching boxes */
		diff->base_to_dest[i]=j;
		diff->dest_to_base[j]=i;
		break;
	}
}
}



/**
 * This function produces the diff ops corresponding to the given matching boxes, if any.
 * Note that when comparing outgoing transitions, we only consider transitions going to
 * matched states, since differences induced by transitions pointing to added/removed
 * states are already represented by the fact that states have been added/removed, so
 * there is no need no explicit thoses changes any more if the diff result.
 */
static void compare_matching_boxes(GrfDiff* diff,Grf* base,Grf* dest,int base_index,int dest_index) {
GrfState* base_state=base->states[base_index];
GrfState* dest_state=dest->states[dest_index];
if (base_state->x!=dest_state->x || base_state->y!=dest_state->y) {
	add_diff_box_moved(diff,base_index,dest_index);
}
if (u_strcmp(base_state->box_content,dest_state->box_content)) {
	add_diff_box_content_changed(diff,base_index,dest_index);
}
/* We look for all transitions that are in base and not in dest */
for (int i=0;i<base_state->n_transitions;i++) {
	int renumbered_dest_state=diff->base_to_dest[base_state->transitions[i]];
	if (renumbered_dest_state==-1) {
		/* Transitions to unmatched states are ignored here */
		continue;
	}
	int j;
	for (j=0;j<dest_state->n_transitions;j++) {
		if (renumbered_dest_state==dest_state->transitions[j]) break;
	}
	if (j==dest_state->n_transitions) {
		/* Transition not found ? */
		add_diff_transition_removed(diff,base_index,base_state->transitions[i],
				dest_index,renumbered_dest_state);
	}
}
/* And we look for all transitions that are in dest and not in base */
for (int i=0;i<dest_state->n_transitions;i++) {
	int renumbered_base_state=diff->dest_to_base[dest_state->transitions[i]];
	if (renumbered_base_state==-1) {
		/* Transitions to unmatched states are ignored here */
		continue;
	}
	int j;
	for (j=0;j<base_state->n_transitions;j++) {
		if (renumbered_base_state==base_state->transitions[j]) break;
	}
	if (j==base_state->n_transitions) {
		/* Transition not found ? */
		add_diff_transition_added(diff,base_index,renumbered_base_state,
				dest_index,dest_state->transitions[i]);
	}
}
}


static void show_matching_boxes(GrfDiff* diff,Grf* base,Grf* dest) {
error("-------------------------------------\n");
for (int i=0;i<diff->size_base_to_dest;i++) {
	if (diff->base_to_dest[i]!=-1) {
		error("match %d/%S => %d/%S\n",i,base->states[i]->box_content,
				diff->base_to_dest[i],dest->states[diff->base_to_dest[i]]->box_content);
	}
}
}


/**
 * This function computes a correspondance between base states and dest states.
 * In the worst case, the graphs are too different, so that a correspondance
 * would be made of removing all base states and adding all dest ones.
 */
static void grf_diff_states(GrfDiff* diff,Grf* base,Grf* dest) {
/* coord+content */
find_correspondance(diff,base,dest,1,1,0,0,0,0);
/* coord */
find_correspondance(diff,base,dest,1,0,0,0,0,0);
/* content+transitions+index */
find_correspondance(diff,base,dest,0,1,1,1,1,1);
/* content+transitions */
find_correspondance(diff,base,dest,0,1,1,1,0,1);
/* transitions */
find_correspondance(diff,base,dest,0,0,1,1,0,1);
/* content+incoming transitions */
find_correspondance(diff,base,dest,0,1,1,0,0,1);
/* content+outgoing transitions */
find_correspondance(diff,base,dest,0,1,0,1,0,1);
/* incoming transitions+index */
find_correspondance(diff,base,dest,0,0,1,0,1,1);
/* outgoing transitions+index */
find_correspondance(diff,base,dest,0,0,0,1,1,1);
/* Last chance, content+index, but only on non comment boxes */
find_correspondance(diff,base,dest,0,1,0,0,1,1);
for (int i=0;i<diff->size_base_to_dest;i++) {
	if (diff->base_to_dest[i]==-1) {
		add_diff_box_removed(diff,i);
	}
}
for (int i=0;i<diff->size_dest_to_base;i++) {
	if (diff->dest_to_base[i]==-1) {
		add_diff_box_added(diff,i);
	}
}
/* Finally, we compare boxes that match */
for (int i=0;i<diff->size_base_to_dest;i++) {
	if (diff->base_to_dest[i]!=-1) {
		compare_matching_boxes(diff,base,dest,i,diff->base_to_dest[i]);
	}
}
}


/**
 * Looks for differences in the headers.
 */
static void grf_diff_headers(GrfDiff* diff,Grf* base,Grf* dest) {
if (u_strcmp(base->size,dest->size)) add_diff_op_property(diff,"SIZE");
if (u_strcmp(base->font,dest->font)) add_diff_op_property(diff,"FONT");
if (u_strcmp(base->ofont,dest->ofont)) add_diff_op_property(diff,"OFONT");
if (u_strcmp(base->bcolor,dest->bcolor)) add_diff_op_property(diff,"BCOLOR");
if (u_strcmp(base->fcolor,dest->fcolor)) add_diff_op_property(diff,"FCOLOR");
if (u_strcmp(base->acolor,dest->acolor)) add_diff_op_property(diff,"ACOLOR");
if (u_strcmp(base->scolor,dest->scolor)) add_diff_op_property(diff,"SCOLOR");
if (u_strcmp(base->ccolor,dest->ccolor)) add_diff_op_property(diff,"CCOLOR");
if (u_strcmp(base->dboxes,dest->dboxes)) add_diff_op_property(diff,"DBOXES");
if (u_strcmp(base->dframe,dest->dframe)) add_diff_op_property(diff,"DFRAME");
if (u_strcmp(base->ddate,dest->ddate)) add_diff_op_property(diff,"DDATE");
if (u_strcmp(base->dfile,dest->dfile)) add_diff_op_property(diff,"DFILE");
if (u_strcmp(base->ddir,dest->ddir)) add_diff_op_property(diff,"DDIR");
if (u_strcmp(base->drig,dest->drig)) add_diff_op_property(diff,"DRIG");
if (u_strcmp(base->drst,dest->drst)) add_diff_op_property(diff,"DRST");
if (u_strcmp(base->fits,dest->fits)) add_diff_op_property(diff,"FITS");
if (u_strcmp(base->porient,dest->porient)) add_diff_op_property(diff,"PORIENT");
}


/**
 * Performs a diff operation on the given grfs.
 */
GrfDiff* grf_diff(Grf* base,Grf* dest) {
GrfDiff* diff=new_GrfDiff(base,dest);
grf_diff_headers(diff,base,dest);
/* We look for a correspondance between base states and dest states */
grf_diff_states(diff,base,dest);
return diff;
}


/**
 * Prints the given diff to the given file.
 */
void print_diff(U_FILE* f,GrfDiff* diff) {
for (int i=0;i<diff->diff_ops->nbelems;i++) {
	print_diff_op(f,(DiffOp*)(diff->diff_ops->tab[i]));
}
}

