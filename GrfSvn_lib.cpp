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
#include "Ustring.h"


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
 * Performs a diff3 on property values. Returns 0 in case of success, 1 otherwise.
 */
static int diff3_grf_property(const char* property,unichar* mine,unichar* base,unichar* other,
		unichar* result,U_FILE* f_conflicts,int *conflict) {
int res;
if (1==(res=diff3_grf_strings(mine,base,other,result))) {
	*conflict=1;
	if (f_conflicts!=NULL) u_fprintf(f_conflicts,"PROPERTY %s\n",property);
}
return res;
}


/**
 * Performs a diff3 on the given graph headers. In case of success,
 * it returns 0 and save the resulting header in 'result'. In case of
 * conflicts, it returns 1 and conflict descriptions are written to f.
 */
static void merge_grf_headers(Grf* mine,Grf* base,Grf* other,Grf* result,
		U_FILE* f_conflicts,int *conflict) {
diff3_grf_property("SIZE",mine->size,base->size,other->size,result->size,f_conflicts,conflict);
diff3_grf_property("FONT",mine->font,base->font,other->font,result->font,f_conflicts,conflict);
diff3_grf_property("OFONT",mine->ofont,base->ofont,other->ofont,result->ofont,f_conflicts,conflict);
diff3_grf_property("BCOLOR",mine->bcolor,base->bcolor,other->bcolor,result->bcolor,f_conflicts,conflict);
diff3_grf_property("FCOLOR",mine->fcolor,base->fcolor,other->fcolor,result->fcolor,f_conflicts,conflict);
diff3_grf_property("ACOLOR",mine->acolor,base->acolor,other->acolor,result->acolor,f_conflicts,conflict);
diff3_grf_property("SCOLOR",mine->scolor,base->scolor,other->scolor,result->scolor,f_conflicts,conflict);
diff3_grf_property("CCOLOR",mine->ccolor,base->ccolor,other->ccolor,result->ccolor,f_conflicts,conflict);
diff3_grf_property("DBOXES",mine->dboxes,base->dboxes,other->dboxes,result->dboxes,f_conflicts,conflict);
diff3_grf_property("DFRAME",mine->dframe,base->dframe,other->dframe,result->dframe,f_conflicts,conflict);
diff3_grf_property("DDATE",mine->ddate,base->ddate,other->ddate,result->ddate,f_conflicts,conflict);
diff3_grf_property("DFILE",mine->dfile,base->dfile,other->dfile,result->dfile,f_conflicts,conflict);
diff3_grf_property("DDIR",mine->ddir,base->ddir,other->ddir,result->ddir,f_conflicts,conflict);
diff3_grf_property("DRIG",mine->drig,base->drig,other->drig,result->drig,f_conflicts,conflict);
diff3_grf_property("DRST",mine->drst,base->drst,other->drst,result->drst,f_conflicts,conflict);
diff3_grf_property("FITS",mine->fits,base->fits,other->fits,result->fits,f_conflicts,conflict);
diff3_grf_property("PORIENT",mine->porient,base->porient,other->porient,result->porient,f_conflicts,conflict);
}


/**
 * This function is called when a move has been detected on base box #base_index.
 * It looks for a conflicting box move in other.
 */
static void test_conflicting_move(GrfDiff* base_other,Grf* mine,Grf* other,int base_index,
		int mine_index,U_FILE* f_conflicts,int *conflict) {
for (int i=0;i<base_other->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)base_other->diff_ops->tab[i];
	if (op->op_type!=DIFF_BOX_MOVED || op->box_base!=base_index) continue;
	/* We have found a box move, but it may (miraculously) be the same in
	 * mine and other */
	GrfState* mine_box=mine->states[mine_index];
	GrfState* other_box=other->states[op->box_dest];
	if (mine_box->x==other_box->x && mine_box->y==other_box->y) return;
	*conflict=1;
	if (f_conflicts!=NULL) {
		u_fprintf(f_conflicts,"MOVE %d %d %d\n",mine_index,base_index,op->box_dest);
		return;
	}
}
}


/**
 * This function is called when base box #base_index has been removed in a graph.
 * It tests if the same box has been used in the other graph, either by a move,
 * a content change, or a transition add.
 */
static void test_conflicting_removal(int removed_in_mine,int base_index,GrfDiff* diff,
		U_FILE* f_conflicts,int *conflict) {
for (int i=0;i<diff->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)diff->diff_ops->tab[i];
	switch (op->op_type) {
	case DIFF_BOX_CONTENT_CHANGED:
	case DIFF_BOX_MOVED:
	case DIFF_TRANSITION_ADDED: {
		if (op->box_base==base_index) {
			*conflict=1;
			if (f_conflicts!=NULL) {
				u_fprintf(f_conflicts,"REMOVAL_IN_%s %d\n",removed_in_mine?"MINE":"OTHER",base_index);
			}
		}
		break;
	}
	default: break;
	}
}
}


/**
 * This function applies to result all box moves described either in
 * base_mine or base_other.
 */
static void process_box_moves(Grf* mine,Grf* other,
		Grf* result,GrfDiff* base_mine,GrfDiff* base_other) {
/* Box moves in mine */
for (int i=0;i<base_mine->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)base_mine->diff_ops->tab[i];
	if (op->op_type!=DIFF_BOX_MOVED) continue;
	GrfState* dest=result->states[op->box_base];
	GrfState* src=mine->states[op->box_dest];
	dest->x=src->x;
	dest->y=src->y;
}
/* Box moves in other */
for (int i=0;i<base_other->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)base_other->diff_ops->tab[i];
	if (op->op_type!=DIFF_BOX_MOVED) continue;
	GrfState* dest=result->states[op->box_base];
	GrfState* src=other->states[op->box_dest];
	dest->x=src->x;
	dest->y=src->y;
}
}


/**
 * Returns 1 if the given vector contains the given string; 0 otherwise.
 */
static int contains(vector_ptr* v,unichar* s) {
for (int i=0;i<v->nbelems;i++) {
	if (!u_strcmp(s,(unichar*)v->tab[i])) return 1;
}
return 0;
}


/**
 * Replaces by NULL the first occurrence of the given string, if any.
 */
static void replace_by_NULL(vector_ptr* v,unichar* s) {
for (int i=0;i<v->nbelems;i++) {
	if (!u_strcmp(s,(unichar*)v->tab[i])) {
		free(v->tab[i]);
		v->tab[i]=NULL;
		return;
	}
}
}


/**
 * This function tries to merge the given box line vectors. The result is
 * stored in base. It returns 0 in case of success or 1 if a conflict was
 * found on the box output. Note that mine and other may be modified as a side effect.
 */
static int merge_box_lines(vector_ptr* mine,vector_ptr* base,vector_ptr* other) {
unichar* last_mine=(unichar*)mine->tab[mine->nbelems-1];
unichar* last_other=(unichar*)other->tab[other->nbelems-1];
if (last_mine[0]=='/' && last_other[0]=='/' && u_strcmp(last_mine,last_other)) {
	return 1;
}
/* First, we remove from all three vectors strings that have been removed
 * either in mine or other */
for (int i=0;i<base->nbelems;i++) {
	unichar* s=(unichar*)base->tab[i];
	if (!contains(mine,s)) {
		replace_by_NULL(other,s);
		free(s);
		base->tab[i]=NULL;
	} else if (!contains(other,s)) {
		replace_by_NULL(mine,s);
		free(s);
		base->tab[i]=NULL;
	}
}
/* Then, we add all new strings from mine and other */
for (int i=0;i<mine->nbelems;i++) {
	unichar* s=(unichar*)mine->tab[i];
	if (s!=NULL && !contains(base,s)) {
		vector_ptr_add(base,u_strdup(s));
	}
}
for (int i=0;i<other->nbelems;i++) {
	unichar* s=(unichar*)other->tab[i];
	if (s!=NULL && !contains(base,s)) {
		vector_ptr_add(base,u_strdup(s));
	}
}
/* Finally, we look for an output. If any, we place it at the end of the vector */
for (int i=0;i<base->nbelems;i++) {
	unichar* s=(unichar*)base->tab[i];
	if (s!=NULL && s[0]=='/') {
		base->tab[i]=NULL;
		vector_ptr_add(base,s);
		break;
	}
}
return 0;
}


/**
 * This function merges two box contents into dest. It must be called
 * with dest containing the base box content. Returns 0 in case of success,
 * 1 in case of conflict.
 */
static int merge_box_contents(unichar* *dest,unichar* mine,unichar* other) {
if (!u_strcmp(mine,other)) {
	/* If (miraculously) mine and other are the same, there is no need
	 * to perform a complicated merge */
	free(*dest);
	*dest=u_strdup(mine);
	return 0;
}
vector_ptr* v_base=tokenize_box_content(*dest);
if (v_base==NULL) {
	fatal_error(2,"Invalid box content in 'base' file:\n%S\n",*dest);
}
vector_ptr* v_mine=tokenize_box_content(mine);
if (v_mine==NULL) {
	free_vector_ptr(v_base,free);
	fatal_error(2,"Invalid box content in 'mine' file:\n%S\n",mine);
}
vector_ptr* v_other=tokenize_box_content(other);
if (v_other==NULL) {
	free_vector_ptr(v_base,free);
	free_vector_ptr(v_mine,free);
	fatal_error(2,"Invalid box content in 'other' file:\n%S\n",other);
}
free(*dest);
*dest=NULL;
int ret=merge_box_lines(v_mine,v_base,v_other);
if (ret==0) {
	Ustring* s=new_Ustring();
	u_strcat(s,'"');
	for (int i=0;i<v_base->nbelems;i++) {
		unichar* line=(unichar*)v_base->tab[i];
		if (line==NULL) continue;
		if (s->len!=1 && line[0]!='/') {
			u_strcat(s,'+');
		}
		u_strcat(s,line);
	}
	u_strcat(s,'"');
	*dest=u_strdup(s->str);
	free_Ustring(s);
}
free_vector_ptr(v_base,free);
free_vector_ptr(v_mine,free);
free_vector_ptr(v_other,free);
return ret;
}


/**
 * This function applies to result all box content changes described either in
 * base_mine or base_other. If two content changes apply on the same box, they are
 * merged, unless there is a conflict on the box output.
 */
static void process_box_content_changes(Grf* mine,Grf* other,
		Grf* result,GrfDiff* base_mine,GrfDiff* base_other,
		U_FILE* f_conflict,int *conflict) {
/* Box content changes in mine */
for (int i=0;i<base_mine->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)base_mine->diff_ops->tab[i];
	if (op->op_type!=DIFF_BOX_CONTENT_CHANGED) continue;
	int j;
	DiffOp* op2=NULL;
	/* We look for a content change on the same box in other */
	for (j=0;j<base_other->diff_ops->nbelems;j++) {
		op2=(DiffOp*)base_other->diff_ops->tab[j];
		if (op2->op_type!=DIFF_BOX_CONTENT_CHANGED
				|| op2->box_base!=op->box_base) {
			op2=NULL;
			continue;
		}
		break;
	}
	GrfState* dest=result->states[op->box_base];
	GrfState* mine_state=mine->states[op->box_dest];
	if (op2!=NULL) {
		GrfState* other_state=other->states[op2->box_dest];
		if (1==merge_box_contents(&(dest->box_content),mine_state->box_content,other_state->box_content)) {
			*conflict=1;
			if (f_conflict!=NULL) u_fprintf(f_conflict,"CONTENT %d %d %d\n",op->box_dest,op->box_base,op2->box_dest);
		}
	} else {
		free(dest->box_content);
		dest->box_content=u_strdup(mine_state->box_content);
	}
}
}


static int contains(int t[],int size,int n) {
for (int i=0;i<size;i++) {
	if (t[i]==n) return i;
}
return -1;
}


static void add_transition(int* *t,int *size,int n) {
(*size)++;
*t=(int*)realloc(*t,(*size)*sizeof(int));
if (*t==NULL) {
	fatal_alloc_error("add_transition");
}
(*t)[(*size)-1]=n;
}


static void remove_transition(int* *t,int *size,int index) {
(*t)[index]=(*t)[(*size)-1];
(*size)--;
*t=(int*)realloc(*t,(*size)*sizeof(int));
if (*t==NULL) {
	fatal_alloc_error("remove_transition");
}
}


static void add_grf_state(GrfState** *t,int *size,GrfState* s) {
(*size)++;
*t=(GrfState**)realloc(*t,(*size)*sizeof(GrfState*));
if (*t==NULL) {
	fatal_alloc_error("add_grf_state");
}
(*t)[(*size)-1]=s;
}


/**
 * This function applies to result all transition additions described either in
 * base_mine or base_other.
 */
static void process_added_transitions(Grf* result,GrfDiff* base_mine,GrfDiff* base_other) {
/* Transitions added in mine */
for (int i=0;i<base_mine->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)base_mine->diff_ops->tab[i];
	if (op->op_type!=DIFF_TRANSITION_ADDED) continue;
	GrfState* dest=result->states[op->box_base];
	if (-1==contains(dest->transitions,dest->n_transitions,op->box_dest)) {
		add_transition(&(dest->transitions),&(dest->n_transitions),op->box_dest);
	}
}
/* Transitions added in other */
for (int i=0;i<base_other->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)base_other->diff_ops->tab[i];
	if (op->op_type!=DIFF_TRANSITION_ADDED) continue;
	GrfState* dest=result->states[op->box_base];
	if (-1==contains(dest->transitions,dest->n_transitions,op->box_dest)) {
		add_transition(&(dest->transitions),&(dest->n_transitions),op->box_dest);
	}
}
}


/**
 * This function applies to result all transition removals described either in
 * base_mine or base_other.
 */
static void process_removed_transitions(Grf* result,GrfDiff* base_mine,GrfDiff* base_other) {
/* Transitions removed in mine */
for (int i=0;i<base_mine->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)base_mine->diff_ops->tab[i];
	if (op->op_type!=DIFF_TRANSITION_REMOVED) continue;
	GrfState* dest=result->states[op->box_base];
	int n;
	if (-1!=(n=contains(dest->transitions,dest->n_transitions,op->box_dest))) {
		remove_transition(&(dest->transitions),&(dest->n_transitions),n);
	}
}
/* Transitions removed in other */
for (int i=0;i<base_other->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)base_other->diff_ops->tab[i];
	if (op->op_type!=DIFF_TRANSITION_REMOVED) continue;
	GrfState* dest=result->states[op->box_base];
	int n;
	if (-1!=(n=contains(dest->transitions,dest->n_transitions,op->box_dest))) {
		remove_transition(&(dest->transitions),&(dest->n_transitions),n);
	}
}
}


/**
 * This function applies to result all box additions described either in
 * diff.
 *
 * WARNING: note that this function must be called BEFORE any state copied from
 *          the base graph to result was removed. It neither must be called if
 *          an added state has a transition to state that has been removed in another
 *          graph, but this should not happen, since those conflicts have already been
 *          detected.
 */
static void process_added_states(Grf* src,Grf* result,GrfDiff* diff) {
int n_states_before=result->n_states;
/* added_states_new_indices[i]==-1 => state #i was already in base
 *                            ==n  => added state #i has number n in result */
int* added_states_new_indices=(int*)malloc(src->n_states*sizeof(int));
if (added_states_new_indices==NULL) {
	fatal_alloc_error("process_added_states");
}
for (int i=0;i<src->n_states;i++) {
	added_states_new_indices[i]=-1;
}
for (int i=0;i<diff->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)diff->diff_ops->tab[i];
	if (op->op_type!=DIFF_BOX_ADDED) continue;
	/* The index of an added box is stored in the box_dest field */
	GrfState* added_state=src->states[op->box_dest];
	add_grf_state(&(result->states),&(result->n_states),cpy_grf_state(added_state));
	added_states_new_indices[op->box_dest]=result->n_states-1;
}
/* Now, we renumber transitions outgoing from added states */
for (int i=n_states_before;i<result->n_states;i++) {
	GrfState* s=result->states[i];
	for (int j=0;j<s->n_transitions;j++) {
		int n=s->transitions[j];
		if (added_states_new_indices[n]!=-1) {
			/* If the transition points to an added state, we replace
			 * the state number by the new one, relative to result */
			s->transitions[j]=added_states_new_indices[n];
		} else {
			/* If the transition did point to a state that was copied
			 * into result, we just have to use the state index as
			 * numbered in the base graph, that is supposed to have been
			 * copied into result */
			s->transitions[j]=diff->dest_to_base[n];
		}
	}
}
/* Finally, we explore src transitions outgoing from non added states, since
 * they may have transitions pointing to added states that must be taken
 * into account */
for (int i=0;i<src->n_states;i++) {
	if (added_states_new_indices[i]!=-1) {
		/* Added states have already been handled */
		continue;
	}
	GrfState* s=src->states[i];
	for (int j=0;j<s->n_transitions;j++) {
		if (added_states_new_indices[s->transitions[j]]!=-1) {
			/* If the transition points from a state #i to an
			 * added state #j, we have to find the result
			 * equivalent of i and j */
			int result_src_index=diff->dest_to_base[i];
			int result_dst_index=added_states_new_indices[s->transitions[j]];
			GrfState* result_state=result->states[result_src_index];
			add_transition(&(result_state->transitions),&(result_state->n_transitions),result_dst_index);
		}
	}
}

free(added_states_new_indices);
}


/**
 * This function merge state removals from mine and other. It must be called last,
 * after all other merge operations, because it may induce state renumbering.
 */
static void process_removed_states(Grf* result,GrfDiff* base_mine,GrfDiff* base_other) {
int* renumber=(int*)malloc(result->n_states*sizeof(int));
if (renumber==NULL) {
	fatal_alloc_error("process_removed_states");
}
for (int i=0;i<result->n_states;i++) {
	renumber[i]=i;
}
/* We mark and free boxes removed in mine */
for (int i=0;i<base_mine->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)base_mine->diff_ops->tab[i];
	if (op->op_type!=DIFF_BOX_REMOVED) continue;
	free_GrfState(result->states[op->box_base]);
	result->states[op->box_base]=NULL;
	renumber[op->box_base]=-1;
}
/* The same with boxes removed in other */
for (int i=0;i<base_other->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)base_other->diff_ops->tab[i];
	if (op->op_type!=DIFF_BOX_REMOVED) continue;
	free_GrfState(result->states[op->box_base]);
	result->states[op->box_base]=NULL;
	renumber[op->box_base]=-1;
}
/* Now we have to swap states in order to avoid NULL holes in
 * result's state array */
for (int i=0;i<result->n_states;i++) {
	if (result->states[i]!=NULL) continue;
	/* If we have a NULL state, we must swap it with a non NULL
	 * one found at the end of the array, if any */
	int j;
	for (j=result->n_states-1;j>i;j--) {
		if (result->states[j]!=NULL) break;
	}
	if (j==-1 || j==i) {
		/* If we have found no non NULL state in the end of the array,
		 * it means that we have finished */
		break;
	}
	/* Now, we can switch state #j in position i */
	result->states[i]=result->states[j];
	result->states[j]=NULL;
	renumber[j]=i;
}
/* Finally, we have to renumber or remove transitions */
for (int i=0;i<result->n_states;i++) {
	GrfState* s=result->states[i];
	if (s==NULL) {
		/* We update the number of states */
		result->n_states=i;
		break;
	}
	for (int j=0;j<s->n_transitions;/* no j++ here, it's not an error */) {
		if (renumber[s->transitions[j]]!=-1) {
			/* Just have to renumber */
			s->transitions[j]=renumber[s->transitions[j]];
			/* Don't forget to move on */
			j++;
		} else {
			/* We have to removal transition #j */
			remove_transition(&(s->transitions),&(s->n_transitions),j);
			/* No move on, since the new transition #j must have to be considered
			 * as well */
		}
	}
}
free(renumber);
}


/**
 * This function must be called only when there is no unresolvable conflict
 * between base and other about box moves and deleted boxes. It merges those
 * graphs into result. Conflicts still may occur when merging box content
 * changes.
 */
static void merge_grf_states(Grf* mine,Grf* base,Grf* other,
		Grf* result,GrfDiff* base_mine,GrfDiff* base_other,
		U_FILE* f_conflict,int *conflict) {
/* First, we copy the base graph, and then we will apply all transformations to it */
cpy_grf_states(result,base);
process_box_moves(mine,other,result,base_mine,base_other);
process_box_content_changes(mine,other,result,base_mine,base_other,f_conflict,conflict);
if (*conflict) return;
process_added_transitions(result,base_mine,base_other);
process_removed_transitions(result,base_mine,base_other);
process_added_states(mine,result,base_mine);
process_added_states(other,result,base_other);
/* This function must called last, otherwise, it would perturb previous
 * operations because of unexpected states renumbering */
process_removed_states(result,base_mine,base_other);
}


/**
 * Performs a diff3 on the given graph states. In case of success,
 * it returns 0 and save the resulting header in 'result'. In case of
 * conflicts, it returns 1 and conflict descriptions are written to f.
 */
static void try_to_merge_grf_states(GrfDiff* base_mine,GrfDiff* base_other,Grf* mine,Grf* base,Grf* other,
		Grf* result,U_FILE* f_conflicts,int *conflict) {
/* First, we try to detect unresolvable conflict cases:
 * 1) a box that has moved differently in base and other
 * 2) a box that has been removed from a graph and modified in the other
 *    (content change, move, transition(s) added)
 */
for (int i=0;i<base_mine->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)base_mine->diff_ops->tab[i];
	if (op->op_type==DIFF_BOX_MOVED) {
		test_conflicting_move(base_other,mine,other,op->box_base,op->box_dest,f_conflicts,conflict);
	} else if (op->op_type==DIFF_BOX_REMOVED) {
		test_conflicting_removal(1,op->box_base,base_other,f_conflicts,conflict);
	}
}
/* Box move conflicts can only occur when there are box moves in both base_mine
 * and base_other, but in order to detect conflicts dued to removed boxes, we have
 * now to consider boxes that have been removed in other, since in the previous loop,
 * we only looked at boxes removed in mine. */
for (int i=0;i<base_other->diff_ops->nbelems;i++) {
	DiffOp* op=(DiffOp*)base_other->diff_ops->tab[i];
	if (op->op_type==DIFF_BOX_REMOVED) {
		test_conflicting_removal(0,op->box_base,base_mine,f_conflicts,conflict);
	}
}
if (*conflict) {
	/* If there are unresolvable conflicts, we can't merge graphs so we have finished */
	return;
}
merge_grf_states(mine,base,other,result,base_mine,base_other,f_conflicts,conflict);
}


/**
 * Returns the merged grf or NULL in case of unresolved conflicts. In such a case,
 * conflicts are described in f_conflicts.
 */
static Grf* diff3_internal(U_FILE* f_conflicts,Grf* mine,Grf* base,Grf* other) {
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
/* If we have to compare and merge different grf, we only need
 * the diffs obtained from base */
free_GrfDiff(mine_other);
Grf* result=new_Grf();
int conflict=0;
merge_grf_headers(base,mine,other,result,f_conflicts,&conflict);
try_to_merge_grf_states(base_mine,base_other,mine,base,other,result,f_conflicts,&conflict);
free_GrfDiff(base_mine);
free_GrfDiff(base_other);
if (conflict) {
	free_Grf(result);
	result=NULL;
}
return result;
}


/**
 * Performs a diff3 on the given graphs. In case of success,
 * it returns 0 and prints on f the
 * graph that results from the fusion. In case of conflicts,
 * it returns 1 and prints nothing.
 */
int diff3(U_FILE* f,U_FILE* f_conflicts,Grf* mine,Grf* base,Grf* other) {
Grf* result=diff3_internal(f_conflicts,mine,base,other);
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
 * matched. Returns the number of matching box pairs.
 */
static int find_correspondance(GrfDiff* diff,Grf* base,Grf* dest,
		int coord,     /* compare coordinates */
		int content,   /* compare box content */
		int incoming,  /* compare incoming transitions */
		int outgoing,  /* compare outgoing transitions */
		int index,     /* compare box indices in .grf files */
		int ignore_comment_boxes
		) {
int n=0;
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
		n++;
		break;
	}
}
return n;
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
int n;
/* We do a loop, because all match tests that depends on transitions may
 * fail at one time and succeed later, since transitions to unmatched boxes
 * are ignored */
do {
	n=0;
	/* content+transitions+index */
	n+=find_correspondance(diff,base,dest,0,1,1,1,1,1);
	/* content+transitions */
	n+=find_correspondance(diff,base,dest,0,1,1,1,0,1);
	/* transitions */
	n+=find_correspondance(diff,base,dest,0,0,1,1,0,1);
	/* content+incoming transitions */
	n+=find_correspondance(diff,base,dest,0,1,1,0,0,1);
	/* content+outgoing transitions */
	n+=find_correspondance(diff,base,dest,0,1,0,1,0,1);
	/* incoming transitions+index */
	n+=find_correspondance(diff,base,dest,0,0,1,0,1,1);
	/* outgoing transitions+index */
	n+=find_correspondance(diff,base,dest,0,0,0,1,1,1);
	/* Last chance, content+index, but only on non comment boxes */
	n+=find_correspondance(diff,base,dest,0,1,0,0,1,1);
} while (n!=0);

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

