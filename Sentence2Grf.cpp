 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
void tfst_transitions_to_grf_states(Tfst*,int*,U_FILE*,int,int,int*,char*,int);
struct grf_state* new_grf_state(const char*,int,int);
struct grf_state* new_grf_state(unichar*,int,int);
void free_grf_state(struct grf_state*);
void add_transition_to_grf_state(struct grf_state*,int);
void save_grf_states(U_FILE*,struct grf_state**,int,int,char*,int,int);


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
void sentence_to_grf(Tfst* tfst,char* font,int fontsize,U_FILE* f) {
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
tfst_transitions_to_grf_states(tfst,rank,f,maximum_rank,width_max,pos_X,font,fontsize);
free(rank);
free(pos_X);
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
void remove_duplicate_grf_states(struct grf_state** states,int *N) {
int j;
/* 2 because the initial and final states are not concerned */
for (int i=2;i<*N;i++) {
   j=i+1;
   /* We look for a state that is equal to states[i] */
   while (j<*N) {
      if (!u_strcmp(states[i]->content,states[j]->content) &&
          equal_list_int(states[i]->l,states[j]->l)) {
         /* If we have such a state */
         if (states[i]->pos_X<states[j]->pos_X) {
            /* We take the rightmost coordinates */
            states[i]->pos_X=states[j]->pos_X;
            states[i]->rank=states[j]->rank;
         }
         /* Then we free the state #j and we swap it with the last one */
         free_grf_state(states[j]);
         states[j]=states[*N-1];
         /* We decrease the number of states */
         (*N)--;
         /* And we renumber all transitions to j into transitions to i */
         for (int k=0;k<*N;k++) {
            if (remove(j,&(states[k]->l))) {
               states[k]->l=sorted_insert(i,states[k]->l);
            }
            if (remove(*N,&(states[k]->l))) {
               states[k]->l=sorted_insert(j,states[k]->l);
            }
         }
      }
      j++;
   }
}
}


/**
 * This function creates the grf states that correspond to the given fst2
 * and saves them to the given file.
 */
void tfst_transitions_to_grf_states(Tfst* tfst,
                                    int* rank,U_FILE* f,int maximum_rank,
                                    int width_max,int* pos_X,char* font,int fontsize) {
int n_states=tfst->automaton->number_of_states;
int* n_transitions_before_state=get_n_transitions_before_state(tfst);
int max_transitions=get_maximum_difference(n_transitions_before_state,n_states);
int N_GRF_STATES=2;
int MAX_STATES=2+n_transitions_before_state[n_states];
Transition* trans;
struct grf_state** grf_states=(struct grf_state**)malloc(MAX_STATES*sizeof(struct grf_state));
if (grf_states==NULL) {
   fatal_alloc_error("tfst_transitions_to_grf_states");
}
/* We create initial state and set its transitions. We start at 2 because
 * 0 and 1 are respectively reserved for the initial and the final states. */
grf_states[0]=new_grf_state("\"<E>\"",50,0);
int j=2;
trans=tfst->automaton->states[0]->outgoing_transitions;
while (trans!=NULL) {
   add_transition_to_grf_state(grf_states[0],j++);
   trans=trans->next;
}
/* We create the final state */
grf_states[1]=new_grf_state("\"\"",(width_max+100),maximum_rank);
/* Then, we save all the other grf states */
unichar content[10000];
for (int i=0;i<n_states;i++) {
   trans=tfst->automaton->states[i]->outgoing_transitions;
   while (trans!=NULL) {
      TfstTag* t=(TfstTag*)tfst->tags->tab[trans->tag_number];
      if (!u_strcmp(t->content,"\"")) {
         /* If the box content is a double quote, we must protect it in a special
          * way since both \ and "  are special characters in grf files. */
         u_sprintf(content,"\"\\\\\\\"/%d %d %d %d %d %d\"",
               t->m.start_pos_in_token,t->m.start_pos_in_char,t->m.start_pos_in_letter,
               t->m.end_pos_in_token,t->m.end_pos_in_char,t->m.end_pos_in_letter);
      } else {
         /* Otherwise, we put the content between double quotes */
         content[0]='"';
         int length=1+escape(t->content,&content[1],P_DOUBLE_QUOTE);
         u_sprintf(content+length,"/%d %d %d %d %d %d\"",
               t->m.start_pos_in_token,t->m.start_pos_in_char,t->m.start_pos_in_letter,
               t->m.end_pos_in_token,t->m.end_pos_in_char,t->m.end_pos_in_letter);
      }
      grf_states[N_GRF_STATES]=new_grf_state(content,pos_X[rank[i]],rank[i]);
      /* Now that we have created the grf state, we set its outgoing transitions */
      if (tfst->automaton->states[trans->state_number]->outgoing_transitions==NULL) {
         /* If the current fst2 transition points on the final state,
          * we must put a transition for the current grf state to the
          * grf final state */
         add_transition_to_grf_state(grf_states[N_GRF_STATES],1);
      } else {
         /* Otherwise, we create transitions */
         Transition* tmp=tfst->automaton->states[trans->state_number]->outgoing_transitions;
         /* +2 because of the grf states 0 and 1 that are reserved */
         int j=2+n_transitions_before_state[trans->state_number];
         while (tmp!=NULL) {
            add_transition_to_grf_state(grf_states[N_GRF_STATES],j++);
            tmp=tmp->next;
         }
      }
      N_GRF_STATES++;
      trans=trans->next;
   }
}
free(n_transitions_before_state);
remove_duplicate_grf_states(grf_states,&N_GRF_STATES);
/* And we save the grf */
save_grf_states(f,grf_states,N_GRF_STATES,maximum_rank,font,max_transitions,fontsize);
/* Finally, we perform cleaning */
for (int i=0;i<N_GRF_STATES;i++) {
   free_grf_state(grf_states[i]);
}
free(grf_states);
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
 * Prints the header of the grf to the given file.
 */
void write_grf_header(int width,int height,int n_states,char* font,U_FILE* f,int fontsize) {
u_fprintf(f,"#Unigraph\n");
u_fprintf(f,"SIZE %d %d\n",width,height);
u_fprintf(f,"FONT %s:  %d\n",font,fontsize);
u_fprintf(f,"OFONT %s:B %d\n",font,fontsize);
u_fprintf(f,"BCOLOR 16777215\n");
u_fprintf(f,"FCOLOR 0\n");
u_fprintf(f,"ACOLOR 12632256\n");
u_fprintf(f,"SCOLOR 16711680\n");
u_fprintf(f,"CCOLOR 255\n");
u_fprintf(f,"DBOXES y\n");
u_fprintf(f,"DFRAME y\n");
u_fprintf(f,"DDATE y\n");
u_fprintf(f,"DFILE y\n");
u_fprintf(f,"DDIR y\n");
u_fprintf(f,"DRIG n\n");
u_fprintf(f,"DRST n\n");
u_fprintf(f,"FITS 100\n");
u_fprintf(f,"PORIENT L\n");
u_fprintf(f,"#\n");
u_fprintf(f,"%d\n",n_states);
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


/**
 * Allocates, initializes and returns a new grf state.
 */
struct grf_state* new_grf_state(unichar* content,int pos_X,int rank) {
struct grf_state* g=(struct grf_state*)malloc(sizeof(struct grf_state));
if (g==NULL) {
   fatal_alloc_error("new_grf_state");
}
g->content=u_strdup(content);
g->pos_X=pos_X;
g->rank=rank;
g->l=NULL;
return g;
}


/**
 * The same than the previous one, but with a char* parameter.
 */
struct grf_state* new_grf_state(const char* content,int pos_X,int rank) {
struct grf_state* g=(struct grf_state*)malloc(sizeof(struct grf_state));
if (g==NULL) {
   fatal_alloc_error("new_grf_state");
}
g->content=u_strdup(content);
g->pos_X=pos_X;
g->rank=rank;
g->l=NULL;
return g;
}


/**
 * Frees the memory associated the given grf state.
 */
void free_grf_state(struct grf_state* g) {
if (g==NULL) return;
if (g->content!=NULL) free(g->content);
free_list_int(g->l);
free(g);
}


/**
 * Adds a transition to the given grf state, if not already present.
 */
void add_transition_to_grf_state(struct grf_state* state,int dest_state) {
state->l=sorted_insert(dest_state,state->l);
}


/**
 * Saves the given grf states to the given file.
 */
void save_grf_states(U_FILE* f,struct grf_state** tab_grf_state,int N_GRF_STATES,
                     int maximum_rank,char* font,int height_indication,int fontsize) {
/* We count the number of boxes for each rank */
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
int *pos_Y=(int*)malloc(sizeof(int)*(maximum_rank+1));
#else
int pos_Y[maximum_rank+1];
#endif
for (int i=0;i<maximum_rank;i++) {
    pos_Y[i]=0;
}
for (int i=0;i<N_GRF_STATES;i++) {
   pos_Y[tab_grf_state[i]->rank]++;
}
/* We print the grf header and the initial state */
write_grf_header(tab_grf_state[1]->pos_X+300,200+height_indication*100,N_GRF_STATES,font,f,fontsize);
u_fprintf(f,"\"<E>\" 50 100 ");
int j=0;
struct list_int* l=tab_grf_state[0]->l;
while (l!=NULL) {
   j++;
   l=l->next;
}
u_fprintf(f,"%d ",j);
l=tab_grf_state[0]->l;
while (l!=NULL) {
   u_fprintf(f,"%d ",l->n);
   l=l->next;
}
/* We print the final state */
u_fprintf(f,"\n\"\" ");
u_fprintf(f,"%d 100 0 \n",tab_grf_state[1]->pos_X);
/* Then, we save all others states */
for (int i=2;i<N_GRF_STATES;i++) {
   u_fprintf(f,"%S ",tab_grf_state[i]->content);
   /* We compute the X coordinate of the box... */
   u_fprintf(f,"%d ",tab_grf_state[i]->pos_X);
   /* ...and the Y one */
   j=100-(50*(tab_grf_state[i]->rank%2))+100*(--pos_Y[tab_grf_state[i]->rank]);
   u_fprintf(f,"%d ",j);
   /* Then we save the transitions */
   j=0;
   l=tab_grf_state[i]->l;
   while (l!=NULL) {
      j++;
      l=l->next;
   }
   u_fprintf(f,"%d ",j);
   l=tab_grf_state[i]->l;
   while (l!=NULL) {
      u_fprintf(f,"%d ",l->n);
      l=l->next;
   }
   u_fprintf(f,"\n");
}
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
free(pos_Y);
#endif
}

