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

#include "Sentence2Grf.h"
#include "StringParsing.h"
#include "Error.h"
#include "BitArray.h"
#include "DELA.h"
#include "Transitions.h"


/* see http://en.wikipedia.org/wiki/Variable_Length_Array . MSVC did not support it
   see http://msdn.microsoft.com/en-us/library/zb1574zs(VS.80).aspx */
#if defined(_MSC_VER) && (!(defined(NO_C99_VARIABLE_LENGTH_ARRAY)))
#define NO_C99_VARIABLE_LENGTH_ARRAY 1
#endif


#define EMPTY_AUTOMATON_DISCLAIMER_TEXT "THIS SENTENCE AUTOMATON HAS BEEN EMPTIED"


int compute_state_ranks(Tfst*,int*);
int get_max_width_for_ranks(Tfst*,int*,int*,int,int);
Grf* tfst_transitions_to_grf_states(Tfst*,int*,int,int,int*,char*,int,int);



/**
 * If the given .tfst is empty (only one state with no outgoing transition),
 * this function modifies it in order to have a single path containing
 * the 'EMPTIED' disclaimer.
 */
void check_automaton_is_empty(Tfst* t) {
	static const char* EMPTY_AUTOMATON_DISCLAIMER=EMPTY_AUTOMATON_DISCLAIMER_TEXT;
	SingleGraph g=t->automaton;
	if (g==NULL) {
		fatal_error("NULL automaton error in check_automaton_is_empty\n");
	}
	if (g->number_of_states==0) {
		fatal_error("Unexpected null number of states in check_automaton_is_empty\n");
	}
	if (g->number_of_states>1) {
		return;
	}
	if (g->states[0]->outgoing_transitions!=NULL) {
		fatal_error("check_automaton_is_empty: unexpected transitions from single state\n");
	}
	SingleGraphState s=add_state(g);
	set_final_state(s);
	TfstTag* tag=new_TfstTag(T_STD);
	tag->content=u_strdup(EMPTY_AUTOMATON_DISCLAIMER);
	int tag_number=vector_ptr_add(t->tags,tag);
	add_outgoing_transition(g->states[0],tag_number,1);
}


/**
 * This function takes a Tfst that represents a text automaton and
 * it build in 'f' the .grf that corresponds to its current sentence.
 *
 * WARNING: a sentence automaton is supposed to have the following properties:
 *           1) being acyclic
 *           2) being minimal
 *           2) having no outgoing transition from the final state
 */
Grf* sentence_to_grf(Tfst* tfst,char* font,int fontsize,int sequence) {
	check_automaton_is_empty(tfst);
	/* The rank array will be used to store the rank of each state */
	int* rank=(int*)malloc(sizeof(int)*tfst->automaton->number_of_states);
	if (rank==NULL) {
		fatal_alloc_error("sentence_to_grf");
	}
	int maximum_rank=compute_state_ranks(tfst,rank);
	/* The pos_X array will be used to store the X coordinate of the grf
	 * boxes that correspond to a given rank. We add +1 to the maximum rank,
	 * because the rank has been computed on the fst2 states and the
	 * X positions concerns fst2 transitions. */
	int* pos_X=(int*)malloc(sizeof(int)*(maximum_rank+1));
	if (pos_X==NULL) {
		fatal_alloc_error("sentence_to_grf");
	}
	int width_max=get_max_width_for_ranks(tfst,pos_X,rank,maximum_rank,fontsize);
	Grf* grf=tfst_transitions_to_grf_states(tfst,rank,maximum_rank,width_max,pos_X,font,fontsize,sequence);
	free(rank);
	free(pos_X);
	return grf;
}



/**
 * This function computes for each state of the fst2 the number of transitions
 * that have been declared before it. These values will be used to know which number
 * must be associated to a given transition of a given state. For instance, if we have
 * the following automaton:
 *
 * state 0: (4 1)
 * state 1: (5 2) (6 2)
 * state 2: (7 3)
 * state 3: (8 4) (9 5)
 * state 4: (10 6) (11 7) (12 8)
 * state 5: (13 9)
 * ...
 *
 * we will have the following array:
 *
 * state 0: 0
 * state 1: 1
 * state 2: 3
 * state 3: 4
 * state 4: 6
 * state 5: 9
 * ...
 *
 * Note that the array has an extra cell at its end. We use it to store the
 * total number of transitions in the automaton.
 */
int* get_n_transitions_before_state(Tfst* tfst) {
	int max=tfst->automaton->number_of_states;
	int* n_transitions_par_state=(int*)malloc((1+max)*sizeof(int));
	if (n_transitions_par_state==NULL) {
		fatal_alloc_error("get_n_transitions_par_state");
	}
	Transition* trans;
	n_transitions_par_state[0]=0;
	for (int i=0;i<max;i++) {
		n_transitions_par_state[i+1]=n_transitions_par_state[i];
		trans=tfst->automaton->states[i]->outgoing_transitions;
		while (trans!=NULL) {
			n_transitions_par_state[i+1]++;
			trans=trans->next;
		}
	}
	return n_transitions_par_state;
}


/**
 * This function takes the array computed by 'get_n_transitions_before_state'
 * and returns the maximum difference of number of transition between
 * neighbor states. This value represents the maximum number of transitions
 * that outgo from a state of the fst2. We will use this value to determine
 * the height of the resulting graph.
 */
int get_maximum_difference(int* t,int size) {
	int m=0;
	for (int i=1;i<size;i++) {
		if ((t[i]-t[i-1])>m) {
			m=t[i]-t[i-1];
		}
	}
	return m;
}


/**
 * This function removes the duplicates grf states, if any. We
 * consider that states are equal when they have the same content
 * and transition list. When we remove a state, the one we keep
 * keeps the coordinates of the rightmost state of the two.
 */
void remove_duplicate_grf_states(Grf* grf) {
	int j;
	/* 2 because the initial and final states are not concerned */
	for (int i=2;i<grf->n_states;i++) {
		j=i+1;
		/* We look for a state that is equal to states[i] */
		while (j<grf->n_states) {
			if (!u_strcmp(grf->states[i]->box_content,grf->states[j]->box_content) &&
					vector_int_equals(grf->states[i]->transitions,grf->states[j]->transitions)) {
				/* If we have such a state */
				if (grf->states[i]->x<grf->states[j]->x) {
					/* We take the rightmost coordinates */
					grf->states[i]->x=grf->states[j]->x;
					grf->states[i]->rank=grf->states[j]->rank;
				}
				/* Then we free the state #j and we swap it with the last one */
				free_GrfState(grf->states[j]);
				grf->states[j]=grf->states[grf->n_states-1];
				/* We decrease the number of states */
				(grf->n_states)--;
				grf->states=(GrfState**)realloc(grf->states,grf->n_states*sizeof(GrfState*));
				/* And we renumber all transitions to j into transitions to i */
				for (int k=0;k<grf->n_states;k++) {
					if (vector_int_remove(grf->states[k]->transitions,j)) {
						vector_int_add(grf->states[k]->transitions,i);
					}
					if (vector_int_remove(grf->states[k]->transitions,grf->n_states)) {
						vector_int_add(grf->states[k]->transitions,j);
					}
				}
			}
			j++;
		}
	}
}


static void prepare_grf_header(Grf* grf,int width,int height,char* font,int fontsize) {
	u_sprintf(grf->size,"SIZE %d %d",width,height);
	u_sprintf(grf->font,"FONT %s:  %d",font,fontsize);
	u_sprintf(grf->ofont,"OFONT %s:B %d",font,fontsize);
	u_sprintf(grf->bcolor,"BCOLOR 16777215");
	u_sprintf(grf->fcolor,"FCOLOR 0");
	u_sprintf(grf->acolor,"ACOLOR 12632256");
	u_sprintf(grf->scolor,"SCOLOR 16711680");
	u_sprintf(grf->ccolor,"CCOLOR 255");
	u_sprintf(grf->dboxes,"DBOXES y");
	u_sprintf(grf->dframe,"DFRAME y");
	u_sprintf(grf->ddate,"DDATE y");
	u_sprintf(grf->dfile,"DFILE y");
	u_sprintf(grf->ddir,"DDIR y");
	u_sprintf(grf->drig,"DRIG n");
	u_sprintf(grf->drst,"DRST n");
	u_sprintf(grf->fits,"FITS 100");
	u_sprintf(grf->porient,"PORIENT L");
}


/**
 * Saves the given grf states to the given file.
 */
static void prepare_grf_for_saving(Grf* grf,
		int maximum_rank,char* font,int height_indication,int fontsize) {
	/* We count the number of boxes for each rank */
	int* pos_Y=(int*)calloc(maximum_rank+1,sizeof(int));
	if (pos_Y==NULL) {
		fatal_alloc_error("prepare_grf_for_saving");
	}
	for (int i=0;i<grf->n_states;i++) {
		if (grf->states[i]->rank<0) error("%d\n",grf->states[i]->rank);
		pos_Y[grf->states[i]->rank]++;
	}
	prepare_grf_header(grf,grf->states[1]->x+300,200+height_indication*100,font,fontsize);
	/* We set the Y coordinates of all boxes */
	grf->states[0]->y=100;
	grf->states[1]->y=100;
	for (int i=2;i<grf->n_states;i++) {
		grf->states[i]->y=100-(50*(grf->states[i]->rank%2))+100*(--pos_Y[grf->states[i]->rank]);
	}
	free(pos_Y);
}




/**
 * This function creates the grf states that correspond to the given fst2
 * and saves them to the given file.
 */
Grf* tfst_transitions_to_grf_states(Tfst* tfst,
		int* rank,int maximum_rank,
		int width_max,int* pos_X,char* font,int fontsize, int sequences) {
	Grf* grf=new_Grf();
	int n_states=tfst->automaton->number_of_states;
	int* n_transitions_before_state=get_n_transitions_before_state(tfst);
	int max_transitions=get_maximum_difference(n_transitions_before_state,n_states);
	Transition* trans;
	/* We create initial state and set its transitions. We start at 2 because
	 * 0 and 1 are respectively reserved for the initial and the final states. */
	add_GrfState(grf,new_GrfState("\"<E>\"",50,0,0,0));
	int j=2;
	trans=tfst->automaton->states[0]->outgoing_transitions;
	while (trans!=NULL) {
		vector_int_add_if_absent(grf->states[0]->transitions,j++);
		trans=trans->next;
	}
	/* We create the final state */
	add_GrfState(grf,new_GrfState("\"\"",(width_max+100),0,maximum_rank,1));
	/* Then, we save all the other grf states */
	Ustring*  content=new_Ustring();
	for (int i=0;i<n_states;i++) {
		trans=tfst->automaton->states[i]->outgoing_transitions;
		while (trans!=NULL) {
			TfstTag* t=(TfstTag*)tfst->tags->tab[trans->tag_number];
			empty(content);
			if (sequences==1){
				if (!u_strcmp(t->content,"\"")) {
					/* If the box content is a double quote, we must protect it in a special
					 * way since both \ and "  are special characters in grf files. */
					u_sprintf(content,"\"\\\\\\\"\"");
				} else if(!u_strcmp(t->content,"<")){
					u_strcpy(content,"\"\\<\"");
				} else if(!u_strcmp(t->content,"\\")){
					u_strcpy(content,"\"\\\\\"");
				} else if(!u_strcmp(t->content,"+")){
					u_strcpy(content,"\"\\+\"");
				} else if(!u_strcmp(t->content,"-")){
					u_strcpy(content,"\"\\-\"");
				} else if(!u_strcmp(t->content,":")){
					u_strcpy(content,"\"\\:\"");
				} else if(!u_strcmp(t->content,"/")){
					u_strcpy(content,"\"\\/\"");
				}	else {

					/* Otherwise, we put the content between double quotes */
					u_strcpy(content,"\"");
					escape(t->content,content,P_DOUBLE_QUOTE);
					u_strcatf(content,"\"");
				}
			}else{

				if (!u_strcmp(t->content,"\"")) {
					/* If the box content is a double quote, we must protect it in a special
					 * way since both \ and "  are special characters in grf files. */
					u_sprintf(content,"\"\\\\\\\"/%d %d %d %d %d %d\"",
							t->m.start_pos_in_token,t->m.start_pos_in_char,t->m.start_pos_in_letter,
							t->m.end_pos_in_token,t->m.end_pos_in_char,t->m.end_pos_in_letter);
				} else {
					/* Otherwise, we put the content between double quotes */
					u_strcpy(content,"\"");
					escape(t->content,content,P_DOUBLE_QUOTE);
					u_strcatf(content,"/%d %d %d %d %d %d\"",
							t->m.start_pos_in_token,t->m.start_pos_in_char,t->m.start_pos_in_letter,
							t->m.end_pos_in_token,t->m.end_pos_in_char,t->m.end_pos_in_letter);
				}
			}
			int index=add_GrfState(grf,new_GrfState(content->str,pos_X[rank[i]],0,rank[i],grf->n_states));
			/* Now that we have created the grf state, we set its outgoing transitions */
			if (tfst->automaton->states[trans->state_number]->outgoing_transitions==NULL) {
				/* If the current fst2 transition points on the final state,
				 * we must put a transition for the current grf state to the
				 * grf final state */
				vector_int_add(grf->states[index]->transitions,1);
			} else {
				/* Otherwise, we create transitions */
				Transition* tmp=tfst->automaton->states[trans->state_number]->outgoing_transitions;
				/* +2 because of the grf states 0 and 1 that are reserved */
				int k=2+n_transitions_before_state[trans->state_number];
				while (tmp!=NULL) {
					vector_int_add(grf->states[index]->transitions,k++);
					tmp=tmp->next;
				}
			}
			trans=trans->next;
		}
	}
	free_Ustring(content);
	free(n_transitions_before_state);
	remove_duplicate_grf_states(grf);
	/* And we prepare the grf so that it will be ready to be saved on disk */
	prepare_grf_for_saving(grf,maximum_rank,font,max_transitions,fontsize);
	return grf;
}


/**
 * This function takes a state A whose current rank is X. Then, for
 * each outgoing transitions A-->B, it tests if X+1>rank(B). If it
 * is the case, then we increase the rank of B and we mark B as updated.
 * Finally, we explore recursively all the states that have been updated.
 */
void explore_states_for_ranks(int current_state,int initial_state,Tfst* tfst,
		int* rank,struct bit_array* modified,int* maximum_rank) {
	Transition* trans=tfst->automaton->states[current_state]->outgoing_transitions;
	int current_rank=rank[current_state-initial_state];
	while (trans!=NULL) {
		if (current_rank+1>rank[trans->state_number-initial_state]) {
			/* If we must increase the rank of the destination state */
			rank[trans->state_number-initial_state]=current_rank+1;
			if ((current_rank+1)>(*maximum_rank)) {
				(*maximum_rank)=current_rank+1;
			}
			set_value(modified,(trans->state_number)-initial_state,1);
		}
		trans=trans->next;
	}
	/* Then, we process all the states we have modified */
	trans=tfst->automaton->states[current_state]->outgoing_transitions;
	while (trans!=NULL) {
		if (get_value(modified,trans->state_number-initial_state)) {
			explore_states_for_ranks(trans->state_number,initial_state,tfst,rank,modified,maximum_rank);
			/* After we have processed the state, we remove the modification mark */
			set_value(modified,(trans->state_number)-initial_state,0);
		}
		trans=trans->next;
	}
}


/**
 * Computes the rank of each state of a sentence automaton.
 * All ranks are stored into the 'rank' array, and the
 * maximum rank is returned.
 */
int compute_state_ranks(Tfst* tfst,int* rank) {
	int n_states=tfst->automaton->number_of_states;
	int maximum_rank=0;
	for (int i=0;i<n_states;i++) {
		rank[i]=0;
	}
	struct bit_array* modified=new_bit_array(n_states,ONE_BIT);
	explore_states_for_ranks(0,0,tfst,rank,modified,&maximum_rank);
	free_bit_array(modified);
	return maximum_rank;
}


/**
 * This is a raw approximation of the width of a tag.
 */
int width_of_tag(TfstTag* e) {
	if (e->content[0]!='{' || !u_strcmp(e->content,"{S}")) {
		/* Note that the {S} should not appear in a sentence automaton */
		return u_strlen(e->content);
	}
	/* If the tag is a tag token like {today,.ADV}, we take the maximum
	 * of the lengths of the inflected form, the lemma and the codes */
	struct dela_entry* entry=tokenize_tag_token(e->content);
	int width=u_strlen(entry->inflected);
	int tmp=u_strlen(entry->lemma);
	if (tmp>width) width=tmp;
	tmp=u_strlen(entry->semantic_codes[0]);
	for (int i=1;i<entry->n_semantic_codes;i++) {
		tmp=tmp+1+u_strlen(entry->semantic_codes[i]);
	}
	for (int i=0;i<entry->n_inflectional_codes;i++) {
		tmp=tmp+1+u_strlen(entry->inflectional_codes[i]);
	}
	if (tmp>width) width=tmp;
	free_dela_entry(entry);
	return width;
}


/**
 * Computes the maximum width in characters associated to each rank and
 * returns the maximum width. These values will be used to generate
 * the X coordinates of the grf boxes so that the graph will be readable.
 * The function returns the X position of the last rank boxes.
 */
int get_max_width_for_ranks(Tfst* tfst,int* pos_X,int* rank,int maximum_rank,int fontsize) {
	int n_states=tfst->automaton->number_of_states;
	int i;
	Transition* trans;
	for (i=0;i<=maximum_rank;i++) {
		pos_X[i]=0;
	}
	/* First, we compute the maximum width for the boxes of each rank */
	for (i=0;i<n_states;i++) {
		trans=tfst->automaton->states[i]->outgoing_transitions;
		while (trans!=NULL) {
			TfstTag* t=(TfstTag*)(tfst->tags->tab[trans->tag_number]);
			int v=fontsize*(5+width_of_tag(t));
			if (pos_X[rank[i]+1]<v) {
				pos_X[rank[i]+1]=v;
			}
			trans=trans->next;
		}
	}
	/* Now that we have the width for each rank, we compute the absolute X
	 * for each rank. We arbitrary set a distance of 100 pixels between boxes
	 * of consecutive ranks. */
	int tmp=100;
	for (i=0;i<=maximum_rank;i++) {
		pos_X[i]=tmp+pos_X[i];
		tmp=pos_X[i];
	}
	return pos_X[maximum_rank];
}


