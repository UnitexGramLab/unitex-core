/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "TfstStats.h"
#include "BitArray.h"
#include "Tfst.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/* see http://en.wikipedia.org/wiki/Variable_Length_Array . MSVC did not support it
   see http://msdn.microsoft.com/en-us/library/zb1574zs(VS.80).aspx */
#if defined(_MSC_VER) && (!(defined(NO_C99_VARIABLE_LENGTH_ARRAY)))
#define NO_C99_VARIABLE_LENGTH_ARRAY 1
#endif


static void compute_form_frequencies(SingleGraph g,void** tags,int tfst_tags,struct hash_table* hash);
static int explore_for_form_frequencies(SingleGraph g,int state,char* factorizing,double freq,
                                    void** tags,int tfst_tags,struct hash_table* form_frequencies);

/**
 * Computes the form frequencies for the given sentence automaton as follows:
 *
 * if there are n concurrent paths, then the frequency of each tag of each path
 * is increased by 1/n.
 *
 * 'hash' is supposed to be a hash table associating double values to strings.
 */
void compute_form_frequencies(SingleGraph g,TfstTag** tags,
                                struct hash_table* form_frequencies) {
  compute_form_frequencies(g,(void**)tags,1,form_frequencies);
}


/**
 * Computes the form frequencies for the given sentence automaton as follows:
 *
 * if there are n concurrent paths, then the frequency of each tag of each path
 * is increased by 1/n.
 *
 * 'hash' is supposed to be a hash table associating double values to strings.
 * 'string_tags' is supposed to contain complete tfst tags as strings
 * like "@STD.....". As a consequence, we first have to a construct a new array
 * that only contains the tag contents.
 */
void compute_form_frequencies(SingleGraph g,const unichar* const* string_tags,int n_string_tags,
                                struct hash_table* form_frequencies) {
unichar** tags=(unichar**)calloc(n_string_tags,sizeof(unichar*));
if (tags==NULL) {
    fatal_alloc_error("compute_form_frequencies");
}

Ustring* foo=new_Ustring(64);
tags[0]=u_strdup("<E>");

for (int i=1;i<n_string_tags;i++) {
    const unichar* s=string_tags[i];
    int n_at=0;
    /* We look for the second '@' */
    while ((*s)!='\0' && n_at!=2) {
        if ((*s)=='@') n_at++;
        s++;
    }
    if ((*s)=='\0') {
        fatal_error("Invalid tfst tag %S in compute_form_frequencies\n",string_tags[i]);
    }
    empty(foo);
    while ((*s)!='\0' && (*s)!='\n') {
        u_strcat(foo,(*s));
        s++;
    }
    if ((*s)=='\0') {
        fatal_error("Invalid tfst tag %S in compute_form_frequencies\n",string_tags[i]);
    }
    tags[i]=u_strdup(foo->str);
}
free_Ustring(foo);
compute_form_frequencies(g,(void**)tags,0,form_frequencies);
for (int i=0;i<n_string_tags;i++) {
    free(tags[i]);
}
free(tags);
}


/**
 * The same than above, but it takes only tag contents like "{be,.V:W}"
 */
static void compute_form_frequencies(SingleGraph g,void** tags,int tfst_tags,struct hash_table* form_frequencies) {
topological_sort(g,NULL);
/* We create and initialize a matrix to know, for each couple of state
 * (x,y) if there is a direct transition from x to y. */
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
struct bit_array** direct=(struct bit_array**)malloc(sizeof(struct bit_array*)*g->number_of_states);
#else
struct bit_array* direct[g->number_of_states];
#endif
for (int i=0;i<g->number_of_states;i++) {
   direct[i]=new_bit_array(g->number_of_states,ONE_BIT);
}
int q;
for (q=0;q<g->number_of_states;q++) {
   for (Transition* t=g->states[q]->outgoing_transitions;t!=NULL;t=t->next) {
      set_value(direct[q],t->state_number,1);
   }
   if (g->states[q]->default_state!=-1) {
      set_value(direct[q],g->states[q]->default_state,1);
   }
}
/* Now, we look for factorizing states, i.e. states so that
 * every path of the automaton cross them */
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
char *factorizing=(char*)malloc(sizeof(char)*g->number_of_states);
#else
char factorizing[g->number_of_states];
#endif
for (q=0;q<g->number_of_states;q++) {
   factorizing[q]=1;
}
for (int i=0;i<g->number_of_states;i++) {
   for (int j=1;j<g->number_of_states;j++) {
      if (get_value(direct[i],j)) {
         for (int k=i+1;k<j;k++) {
            /* We can do this only because we have performed a
             * topological sort before */
            factorizing[k]=0;
         }
      }
   }
}
/* We can free the matrix */
for (int i=0;i<g->number_of_states;i++) {
   free_bit_array(direct[i]);
}
/* Now, we will count the number of paths between all the couple
 * of states q_i and q_i+1, where q_i is the factorizing state #i */
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
int* number_of_paths=(int*)malloc(sizeof(int)*(g->number_of_states));
#else
int number_of_paths[g->number_of_states];
#endif
for (int i=0;i<g->number_of_states;i++) {
   number_of_paths[i]=-1;
}
int q1,q2;
/* By definition, the initial state #0 is a factorizing one */
q2=0;
while (q2<g->number_of_states-1) {
   q1=q2;
   q2++;
   while (q2<g->number_of_states-1 && !factorizing[q2]) {
      q2++;
   }
   number_of_paths[q1]=-1;
   count_paths(g,q1,q2,NULL,NULL,number_of_paths);
}
/* Now, we know how many paths there are between factorizing states, so we
 * can explore the subautomata between factorizing states in order to compute
 * form frequencies */
for (int i=0;i<g->number_of_states;i++) {
    if (factorizing[i]) {
        explore_for_form_frequencies(g,i,factorizing,(double)(1./number_of_paths[i]),tags,tfst_tags,form_frequencies);
    }
}
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
free(direct);
free(factorizing);
free(number_of_paths);
#endif
}


/**
 * Exploring a subautomaton until we find a factorizing state. For each
 * transition we go through, we add 'freq' to its tag frequency.
 */
static int explore_for_form_frequencies(SingleGraph g,int state,char* factorizing,double freq,
                                    void** tags,int tfst_tags,struct hash_table* form_frequencies) {
Transition* t=g->states[state]->outgoing_transitions;
int n=0,foo;
while (t!=NULL) {
    int ret;
    unichar* content;
    if (tfst_tags) {
        TfstTag* tmp=(TfstTag*)(tags[t->tag_number]);
        content=tmp->content;
    } else {
        content=(unichar*)(tags[t->tag_number]);
    }
    struct any* value=get_value(form_frequencies,content,HT_INSERT_IF_NEEDED,&ret);
    if (ret!=HT_KEY_ALREADY_THERE) {
        value->_double=0;
    }
    if (!factorizing[t->state_number]) {
        foo=explore_for_form_frequencies(g,t->state_number,factorizing,freq,tags,tfst_tags,form_frequencies);
        n=n+foo;
        value->_double=value->_double+freq*foo;
    } else {
        value->_double=value->_double+freq;
        n++;
    }
    t=t->next;
}
return n;
}

// this strange code force compiler to do operation on double to avoir equality miss
// two separated constant again optimizer removing...
static const double cte_double_a = (double)atof("0.66666\x00\na");
static const double cte_double_b = (double)atof("0.66666\x00\nb");

static inline int compare_freq_then_name(double freq_a,  const unichar * name_a, double freq_b,const unichar* name_b)
{
    if ((freq_a*cte_double_a) < (freq_b*cte_double_b))
        return -1;
    if ((freq_a*cte_double_a) > (freq_b*cte_double_b))
        return 1;
    return -u_strcmp(name_a, name_b);
}

static int partition_for_quicksort_by_frequence(int m, int n,vector_ptr* tags,vector_double* freq) {
double pivot_freq;
const unichar* pivot_name;
double tmp;
unichar* tmp_char;
int i = m-1;
int j = n+1; // final pivot index
pivot_freq=freq->tab[(m+n)/2];
pivot_name=(const unichar*)tags->tab[(m+n)/2];
for (;;) {
  do j--;
  while ((j>(m-1)) && (compare_freq_then_name(pivot_freq, pivot_name, freq->tab[j], (const unichar*)tags->tab[j])>0));

  do i++;
  while ((i<n+1) && (compare_freq_then_name(freq->tab[i], (const unichar*)tags->tab[i], pivot_freq, pivot_name)>0));
  if (i<j) {
    tmp=freq->tab[i];
    freq->tab[i]=freq->tab[j];
    freq->tab[j]=tmp;

    tmp_char=(unichar*)tags->tab[i];
    tags->tab[i]=tags->tab[j];
    tags->tab[j]=tmp_char;
  } else return j;
}
}

static void quicksort_by_frequence(int first,int last,vector_ptr* tags,vector_double* freq) {
int p;
if (first<last) {
  p=partition_for_quicksort_by_frequence(first,last,tags,freq);
  quicksort_by_frequence(first,p,tags,freq);
  quicksort_by_frequence(p+1,last,tags,freq);
}
}

static int partition_for_quicksort_by_alph_order(int m, int n,vector_ptr* tags,vector_double* freq) {
const unichar* pivot;
unichar* tmp;
double tmp_double;
int i = m-1;
int j = n+1; // final pivot index
pivot=(const unichar*)tags->tab[(m+n)/2];
for (;;) {
  do j--;
  while ((j>(m-1))&&(u_strcmp(pivot,(const unichar*)tags->tab[j])<0));
  do i++;
  while ((i<n+1)&&(u_strcmp((const unichar*)tags->tab[i],pivot)<0));
  if (i<j) {
    tmp_double=freq->tab[i];
    freq->tab[i]=freq->tab[j];
    freq->tab[j]=tmp_double;

    tmp=(unichar*)tags->tab[i];
    tags->tab[i]=tags->tab[j];
    tags->tab[j]=tmp;
  } else return j;
}
}

static void quicksort_by_alph_order(int first,int last,vector_ptr* tags,vector_double* freq) {
int p;
if (first<last) {
  p=partition_for_quicksort_by_alph_order(first,last,tags,freq);
  quicksort_by_alph_order(first,p,tags,freq);
  quicksort_by_alph_order(p+1,last,tags,freq);
}
}

static void sort_and_save_by_freq(U_FILE *f,vector_ptr* tags,vector_double* freq) {
quicksort_by_frequence(0,tags->nbelems - 1,tags,freq);
for (int i=0;i<tags->nbelems;i++) {
   u_fprintf(f,"%lf\t%S\n",freq->tab[i],tags->tab[i]);
}
}

static void sort_and_save_by_alph(U_FILE *f,vector_ptr* tags,vector_double* freq) {
quicksort_by_alph_order(0,tags->nbelems - 1,tags,freq);
for (int i=0;i<tags->nbelems;i++) {
   u_fprintf(f,"%S\t%lf\n",tags->tab[i],freq->tab[i]);
}
}


/**
 * Produces the sorted version of the tfst tags frequencies.
 */
void sort_and_save_tfst_stats(struct hash_table* form_frequencies,U_FILE* by_freq,U_FILE* by_alph) {
vector_ptr* tags=new_vector_ptr(4096);
vector_double* freq=new_vector_double(4096);
/* First, we build vectors from the hash table content */
for (unsigned int i=0;i<form_frequencies->capacity;i++) {
    struct hash_list* l=form_frequencies->table[i];
    while (l!=NULL) {
        vector_ptr_add(tags,l->ptr_key);
        vector_double_add(freq,l->value._double);
        l=l->next;
    }
}
if (by_freq!=NULL) {
    sort_and_save_by_freq(by_freq,tags,freq);
}
if (by_alph!=NULL) {
    sort_and_save_by_alph(by_alph,tags,freq);
}
free_vector_ptr(tags,NULL);
free_vector_double(freq);
}

} // namespace unitex
