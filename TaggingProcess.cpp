/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/*
 * author : Anthony Sigogne
 */

#include "TaggingProcess.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Computes the tag code of a token according to its tokenized tag.
 * For example if we have a dela_entry with semantic code = N
 * and inflectional codes = mp, the tag code will be N:mp.
 */
void compute_tag_code(struct dela_entry* tag,unichar* tag_code,int form_type){
if(form_type == 0){
    /* we use simple forms, we extract only the first semantic code */
    unichar* tmp = u_strcpy(tag_code,tag->semantic_codes[0]);
    for(int i=1;i<tag->n_semantic_codes;i++){
        tmp = u_strcat(tmp,"+");
        tmp = u_strcat(tmp,tag->semantic_codes[i]);
    }
    u_strcat(tmp,"\0");
}
else{
    /* we use compound forms, we extract the first semantic code
     * and possible inflectional codes */
    unichar* tmp = u_strcpy(tag_code,tag->semantic_codes[0]);
    for(int i=1;i<tag->n_semantic_codes;i++){
        tmp = u_strcat(tmp,"+");
        tmp = u_strcat(tmp,tag->semantic_codes[i]);
    }
    if(tag->n_inflectional_codes > 0){
        tmp = u_strcat(tmp,":");
        for(int i=0;i<tag->n_inflectional_codes;i++){
            tmp = u_strcat(tmp,tag->inflectional_codes[i]);
        }
        tmp = u_strcat(tmp,"\0");
    }
}
}

/**
 * identify the semantic code of an unknown inflected token
 */
unichar* get_pos_unknown(const unichar* inflected){
unichar* pos = (unichar*)malloc(DIC_LINE_SIZE*sizeof(unichar));
if(pos == NULL){
    fatal_alloc_error("get_pos_unknown");
}
if(u_are_digits(inflected) == 1 || u_is_word(inflected) == 1){
    u_sprintf(pos,"N\0");//peut être séparer et faire suffix pour le word
}
else{
/* inflected is a punctuation, we have to identify if it's either a APOS like
 * "'" or a PONCT for all other punctuations */
    unichar apos[2] = {'\'','\0'};
    if(apos == inflected){
        u_sprintf(pos,"APOS\0");
    }
    else{
        u_sprintf(pos,"PONCT\0");
    }
}
return pos;
}

/**
 * Creates a matrix entry for a token tag. This tag is associated
 * to a transition in the automata.
 */
int create_matrix_entry(const unichar* tag,struct matrix_entry** mx,int form_type,int tag_number,int state_number){
*mx = (struct matrix_entry*)malloc(sizeof(struct matrix_entry));
if(mx == NULL){
    fatal_alloc_error("create_matrix_entry");
}
(*mx)->predecessor = -1;
(*mx)->partial_prob = (double)0.0;
(*mx)->tag_number = tag_number;
(*mx)->state_number = state_number;
int verbose = 0;
struct dela_entry* tmp = tokenize_DELAF_line(tag,1,&verbose);
free_dela_entry(tmp);
if(verbose == 0){
    (*mx)->tag = tokenize_tag_token(tag,1);
    (*mx)->tag_code = (unichar*)malloc(sizeof(unichar)*DIC_LINE_SIZE);
    if((*mx)->tag_code == NULL){
        fatal_alloc_error("create_matrix_entry");
    }
    compute_tag_code((*mx)->tag,(*mx)->tag_code,form_type);
}
else {
    /* we create a new inflected entry for unknown words (not present in lexicons).
     * POS of the entry is determined by a suffix way. */
    unichar* code = get_pos_unknown(tag);
    unichar* inflected = (unichar*)malloc(sizeof(unichar)*(u_strlen(tag)+1));
    if(inflected == NULL){
        fatal_alloc_error("create_matrix_entry");
    }
    u_strcpy(inflected,tag);
    (*mx)->tag = new_dela_entry(inflected,tag,code);
    (*mx)->tag_code = (unichar*)malloc(sizeof(unichar)*DIC_LINE_SIZE);
    if((*mx)->tag_code == NULL){
        fatal_alloc_error("create_matrix_entry");
    }
    compute_tag_code((*mx)->tag,(*mx)->tag_code,form_type);
    free(code);
    free(inflected);
    return -1;
}
return 0;
}

/**
 * Allocates a matrix of matrix_entry*.
 */
struct matrix_entry** allocate_matrix(int size){
struct matrix_entry** matrix = (struct matrix_entry**)malloc(size*sizeof(struct matrix_entry));
if(matrix == NULL){
    fatal_alloc_error("allocate_matrix");
}
return matrix;
}

/**
 * Initializes a matrix by determining its size (number of transitions
 * in the automata.
 */
struct matrix_entry** initialize_viterbi_matrix(SingleGraph automaton,int form_type){
int nb_transitions = 0;
for(int i=0;i<automaton->number_of_states;i++){
    SingleGraphState state = automaton->states[i];
    for(Transition* transO=state->outgoing_transitions;transO!=NULL;transO=transO->next){
        nb_transitions++;
    }
}
/* we allocate the matrix with a size of nb_transitions+2 because
 * we will add two entries "#" at start of this matrix representing
 * the start of the automata. */
struct matrix_entry** matrix = allocate_matrix(nb_transitions+2);
/* we add the two entries "#" */
for(int i=0;i<2;i++){
    unichar* token = (unichar*)malloc(sizeof(unichar)*DIC_LINE_SIZE);
    if(token == NULL){
        fatal_alloc_error("initialize_viterbi_matrix");
    }
    u_sprintf(token,"{#,#.#}");
    create_matrix_entry(token,&matrix[i],form_type,-1,i);
    free(token);
}
/* the second # points on the first entry */
matrix[1]->predecessor = 0;
return matrix;
}

/**
 * Liberates memory used by a matrix_entry.
 */
void free_matrix_entry(struct matrix_entry* entry){
    free_dela_entry(entry->tag);
    free(entry->tag_code);
    free(entry);
}

/**
 * Liberates memory used by a viterbi matrix.
 */
void free_viterbi_matrix(struct matrix_entry** matrix,int size){
    for(int i=0;i<size;i++){
        free_matrix_entry(matrix[i]);
    }
    free(matrix);
}

/**
 * Return the index of the tag in matrix.
 */
int search_matrix_predecessor(struct matrix_entry** matrix,unichar* tag,int start,
                              int tag_number,int state_number){
struct dela_entry* entry = tokenize_tag_token(tag,1);
for(int i=start;i>=0;i--){
    if(equal(entry,matrix[i]->tag) == 1 && matrix[i]->tag_number == tag_number && matrix[i]->state_number == state_number){
        free_dela_entry(entry);
        return i;
    }
}
free_dela_entry(entry);
return -1;
}

/**
 * Extracts INF_code of a given token in the dictionary.
 */
void get_INF_code(Dictionary* d,const unichar* token,
                  int case_sensitive,int index,int offset,
                  const Alphabet* alphabet,int* inf_index,Ustring* ustr) {
int final,n_transitions,inf_number;
if (d->type!=BIN_CLASSIC) {
    fatal_error("get_INF_code: unsupported dictionary type\n");
}
offset=read_dictionary_state(d,offset,&final,&n_transitions,&inf_number);
if (token[index]=='\0') {
   /* If we are at end of the token */
   if (final) {
      /* If the node is final */
       *inf_index=inf_number;
   }
   return;
}
unichar c;
int adr;
int z=save_output(ustr);
for(int i=0;i<n_transitions;i++) {
   /* For each outgoing transition, we look if the transition character is
    * compatible with the token's one */
    offset=read_dictionary_transition(d,offset,&c,&adr,ustr);
   if(case_sensitive == 1 && c == token[index]){
       get_INF_code(d,token,case_sensitive,index+1,adr,alphabet,inf_index,ustr);
   }
   else if(case_sensitive == 0 && is_equal_ignore_case(token[index],c,alphabet)){
       get_INF_code(d,token,case_sensitive,index+1,adr,alphabet,inf_index,ustr);
   }
   restore_output(z,ustr);
}
}

/*
 * Returns the value pointed by inf_code in the INF_codes structure.
 */
long int get_inf_value(const struct INF_codes* inf,int inf_index){
struct list_ustring* tmp=inf->codes[inf_index];
/* we work only with integer values in all dictionnaries
 * so we convert the string found into long value. */
char* code = (char*)malloc(sizeof(char)*u_strlen(tmp->string));
if(code == NULL){
    fatal_alloc_error("get_inf_value");
}
unsigned int l=u_strlen(tmp->string);
for(unsigned int i=0;i<l;i++){
    code[i] = (char)tmp->string[i+1];
}
/* conversion string to int value */
long int value = strtol(code,NULL,10);
free(code);
return value;
}

/*
 * Returns the long integer value of a string in the dictionnary
 * according to the extracted INF_codes index.
 * Returns -1 if the string is not in the dictionnary.
 */
long int get_sequence_integer(const unichar* sequence,Dictionary* d,const Alphabet* alphabet){
int inf_index = -1;
Ustring* ustr=new_Ustring();
get_INF_code(d,sequence,1,0,d->initial_state_offset,alphabet,&inf_index,ustr);
free_Ustring(ustr);
if(inf_index == -1){
    /* sequence is not in the dictionary */
    return -1;
}
return get_inf_value(d->inf,inf_index);
}

/**
 * Creates a new sequence token based on the combining of two tokens.
 * In our case, the tokens could be "N" and "dog". The result is a
 * new unichar "N\dog".
 */
unichar* create_bigram_sequence(const unichar* first_token,const unichar* second_token,int tabulation){
unichar* sequence = (unichar*)malloc(sizeof(unichar)*(2+u_strlen(first_token)+u_strlen(second_token)));
if(sequence == NULL){
    fatal_alloc_error("create_bigram_sequence");
}
unichar* tmp = u_strcpy_sized(sequence,u_strlen(first_token)+1,first_token);
if(tabulation == 1){
    tmp = u_strcat(tmp,"\t");
}
u_strcat(tmp,second_token);
return sequence;
}

/**
 * Same as create_bigram_sequence but the first token is there a char*.
 */
unichar* create_bigram_sequence(const char* first_token,const unichar* second_token,int tabulation){
/* we convert first token in a unichar* string sequence */
unichar* tmp_first = u_strdup(first_token);
unichar* sequence = create_bigram_sequence(tmp_first,second_token,tabulation);
free(tmp_first);
return sequence;
}

/**
 * Creates a new sequence token based on the combining of three tokens.
 * In our case, the tokens could be "N", "A" and "blue". The result is a
 * new unichar "N\tA\tblue".
 */
unichar* create_trigram_sequence(const unichar* first_token,const unichar* second_token,const unichar* third_token){
unichar* bigram = create_bigram_sequence(first_token,second_token,1);
unichar* sequence = (unichar*)malloc(sizeof(unichar)*(2+u_strlen(bigram)+u_strlen(third_token)));
if(sequence == NULL){
    fatal_alloc_error("create_trigram_sequence");
}
unichar* tmp = u_strcpy_sized(sequence,u_strlen(bigram)+1,bigram);
free(bigram);
tmp = u_strcat(tmp,"\t");
u_strcat(tmp,third_token);
return sequence;
}

/**
 * Computes the suffix of size n of a token.
 */
unichar* u_strnsuffix(const unichar* s,int n){
unichar* suffix = (unichar*)malloc(sizeof(unichar)*(n+1));
if(suffix == NULL){
    fatal_alloc_error("u_strnsuffix");
}
for(int i=0;i<=n;i++){
    suffix[i] = s[(u_strlen(s)-n)+i];
}
return suffix;
}

/**
 * Compute emit probability according to an inflected token associated
 * with a code (semantic and sometimes inflectional codes).
 * This probability is a double value between 0 and 1.
 */
double compute_emit_probability(Dictionary* d,const Alphabet* alphabet,
        const unichar* tag,const unichar* inflected){
const char prefix1[] = "word_";
unichar* new_inflected = create_bigram_sequence(prefix1,inflected,0);
unichar* sequence1 = create_bigram_sequence(tag,new_inflected,1);
long int N1 = get_sequence_integer(new_inflected,d,alphabet);
long int N2 = get_sequence_integer(sequence1,d,alphabet);
free(sequence1);
free(new_inflected);
if(N1 == -1){
    /* current inflected token is unknown, we apply
     * a suffix-based algorithm to determine its part of speech tag*/
    if(u_strlen(inflected) < 3){
        /* the word is too short to be treated */
        return 0;
    }
    int suffix_length = 4;
    if(u_strlen(inflected) < 6){
        suffix_length = u_strlen(inflected) - 2;
    }
    unichar* suffix = u_strnsuffix(inflected,suffix_length);
    const char prefix2[] = "suff_";
    unichar* seq_suff = create_bigram_sequence(prefix2,suffix,0);
    N1 = get_sequence_integer(seq_suff,d,alphabet);
    unichar* sequence2 = create_bigram_sequence(tag,seq_suff,1);
    N2 = get_sequence_integer(sequence2,d,alphabet);
    free(sequence2);
    free(suffix);
    free(seq_suff);
}
if(N1 == -1){
    N1 = 0;
}
if(N2 == -1){
    N2 = 0;
}
/* we compute the emit probability thanks to this smoothed formula */
return (double)(((double)N2)/(1+((double)(N1))));
}

/**
 * Compute transition probability according to three following codes
 * (semantic and sometimes inflectional codes). This probability is
 * a double value between 0 and 1.
 */
double compute_transition_probability(Dictionary* d,const Alphabet* alphabet,
                                     const unichar* ancestor,const unichar* predecessor,const unichar* current){
unichar* tri_sequence = create_trigram_sequence(ancestor,predecessor,current);
unichar* bi_sequence = create_bigram_sequence(ancestor,predecessor,1);
long int C1 = get_sequence_integer(tri_sequence,d,alphabet);
long int C2 = get_sequence_integer(bi_sequence,d,alphabet);
free(bi_sequence);
free(tri_sequence);
if(C1 == -1){
    C1 = 0;
}
if(C2 == -1){
    C2 = 1;
}
return (double)(((double)C1)/((double)(C2)));
}

/**
 * check whether the compound word contains the '-' character.
 * If true, replace '-' by '_'.
 */
void check_compound(unichar* inflected){
    for(unsigned int i=0;i<u_strlen(inflected);i++){
        if(inflected[i] == '-'){
            inflected[i] = '_';
        }
    }
}

/**
 * Computes partial probability of a outgoing transition of a state (for compounds words only).
 * The system used here is BIO.
 */
double compute_partial_probability_compounds(Dictionary* d,const Alphabet* alphabet,
          unichar* ancestor,unichar* predecessor,unichar* tag_code,unichar* inflected){
    check_compound(inflected);
    unichar* word = u_strchr(inflected,'_');
    int old_value=0,nb_words=0;
    double score = 0.0;
    while(word != NULL) {
        unichar* simple_word = u_strdup(inflected+old_value,u_strlen(inflected)-old_value-u_strlen(word));
        unichar* new_tag_code = (unichar*)malloc(sizeof(unichar)*(u_strlen(tag_code)+3));
        unichar* tmp = u_strcpy_sized(new_tag_code,u_strlen(tag_code)+1,tag_code);
        if(nb_words == 0){
            u_strcat(tmp,"+B\0");
        }
        else{
            u_strcat(tmp,"+I\0");
        }
        score += compute_emit_probability(d,alphabet,new_tag_code,simple_word);
        score += compute_transition_probability(d,alphabet,ancestor,predecessor,new_tag_code);
        nb_words+=1;
        old_value += u_strlen(simple_word)+1;
        word = u_strchr(inflected+old_value,'_');
        if(nb_words>2){
            free(ancestor);
        }
        ancestor = predecessor;
        predecessor = new_tag_code;
        free(simple_word);
        if(word == NULL){
            if(u_strlen(inflected)-old_value != 0){
                word = inflected+old_value;
                unichar* new_tag_code_2 = (unichar*)malloc(sizeof(unichar)*(u_strlen(tag_code)+3));
                unichar* tmp_2 = u_strcpy_sized(new_tag_code_2,u_strlen(tag_code)+1,tag_code);
                u_strcat(tmp_2,"+I\0");
                score += compute_emit_probability(d,alphabet,new_tag_code_2,word);
                score += compute_transition_probability(d,alphabet,ancestor,predecessor,new_tag_code_2);
                free(new_tag_code_2);
            }
            if(nb_words>=2){
                free(ancestor);
            }
            if(nb_words>=1){
                free(predecessor);
            }
            break;
        }
    }
    free(inflected);
    return score;
}

/**
 * Computes partial probability of a outgoing transition of a state.
 * This probability is the product of emit and transition probabilities.
 */
double compute_partial_probability(Dictionary* d,const Alphabet* alphabet,
                                  struct matrix_entry* ancestor,struct matrix_entry* predecessor,
                                  struct matrix_entry* current){
unichar* inflected = compound_to_simple(current->tag->inflected);
/* case : a transition tagged by a compound */
if((u_strchr(inflected,'_') != NULL || (u_strchr(inflected,'-') != NULL && inflected[0]!='-'))&& u_strlen(inflected)>2){
    return compute_partial_probability_compounds(d,alphabet,ancestor->tag_code,predecessor->tag_code,current->tag_code,inflected);
}
double emit_prob = compute_emit_probability(d,alphabet,current->tag_code,inflected);
double trans_prob = compute_transition_probability(d,alphabet,ancestor->tag_code,predecessor->tag_code,current->tag_code);
free(inflected);
return emit_prob+trans_prob;
}

int u_find_char(const unichar* s,unichar t){
for(int i=u_strlen(s)-1;i>0;i--){
    if(s[i]==t){
        return i;
    }
}
return -1;
}

/**
 * Calculates partial probability for a transition and if this probability
 * is better than the previous best transition, we replace this one by the new.
 */
void compute_best_probability(Dictionary* d,const Alphabet* alphabet,
                              struct matrix_entry** matrix,int index_matrix,int indexI,int cover_span){
double score = cover_span==1?0:compute_partial_probability(d,alphabet,matrix[matrix[indexI]->predecessor],
                                          matrix[indexI],matrix[index_matrix])+matrix[indexI]->partial_prob;
if(score > 0 && u_find_char(matrix[index_matrix]->tag->inflected,'_') != -1){
    score +=2;
}
/* best predecessor is saved for the current output transition*/
if(score >= matrix[index_matrix]->partial_prob){
    matrix[index_matrix]->predecessor = indexI;
    matrix[index_matrix]->partial_prob = (float)score;
}
}

/**
 * Reconstructs by a backtracking way and returns the sequence of transition
 * corresponding to the best path in the automata.
 */
int* get_state_sequence(struct matrix_entry** matrix,int index){
/* the sequence is just a table of matrix index */
int* state_sequence = (int*)malloc(index*sizeof(int));
if(state_sequence == NULL){
    fatal_alloc_error("get_state_sequence");
}
for(int i=index;;){
    matrix_entry* me = matrix[i];
    if(me->predecessor == -1){
        break;
    }
    state_sequence[me->predecessor] = i;
    i = me->predecessor;
}
return state_sequence;
}

/**
 * Returns 1 if the token is a compound, i.e. contains whitespace(s);
 * returns 0 otherwise.
 */
int is_compound_word(const unichar* token){
unsigned int l=u_strlen(token);
for(unsigned int i=0;i<l;i++){
    if((token[i] == ' ') || (token[i] == '-')) {
        return 1;
    }
}
return 0;
}

/**
 * Converts a compound word to a simple word.
 * Returns the new token.
 */
unichar* compound_to_simple(const unichar* token){
unichar* word = (unichar*)malloc(sizeof(unichar)*(u_strlen(token)+1));
for(unsigned int i=0;i<=u_strlen(token);i++){
    if(token[i] == ' '){
        word[i] = '_';
    }
    else{
        word[i] = token[i];
    }
}

return word;
}

/**
 * Prunes the automata according to the best path found by
 * the Viterbi Path algorithm. We keep transitions tagged by
 * compound words.
 */
vector_ptr* do_backtracking(struct matrix_entry** matrix,int index,SingleGraph graph,vector_ptr* tags,int form_type){
/* we extract the best sequence of transitions (indexes) from the viterbi matrix */
int* state_sequence = get_state_sequence(matrix,index);
/* we initialize the new vector of tags (extracted from the pruning automaton) */
vector_ptr* new_tags = new_vector_ptr(index);
/* the first tag token in the vector must be a Epsilon token <E> (by convention) */
vector_ptr_add(new_tags,u_strdup("@<E>\n.\n"));
if(index == 1){
    /* sentence has been rejected by elag tagset.def, so we return the empty token */
    free(state_sequence);
    return new_tags;
}
/* trans_table is used to save indexes of best transitions */
int* trans_table = (int*)malloc((index+1)*sizeof(int));
for(int i=0;i<=index;i++){
    trans_table[i] = -1;
}
SingleGraphState state = graph->states[get_initial_state(graph)];
for(int i=state_sequence[1];i<=index;i=state_sequence[i]){
    int next_state = -1;
    Transition *first = new_Transition(-1,-1),*list = first;
    for(Transition* transO=state->outgoing_transitions;transO!=NULL;transO=transO->next){
        TfstTag* tag = (TfstTag*)tags->tab[transO->tag_number];
        unichar* content = compound_to_simple(tag->content);
        struct dela_entry* entry = tokenize_tag_token(content,1);
        free(content);
        if(((same_codes(entry,matrix[i]->tag) == 1 && form_type == 1) ||
                (form_type == 0 && (u_strcmp(entry->semantic_codes[0],matrix[i]->tag->semantic_codes[0]) == 0))) &&
                (u_strcmp(entry->inflected,matrix[i]->tag->inflected) == 0) &&
                transO->tag_number == matrix[i]->tag_number){
            /* this outgoing transition is the good one (best partial probability at this state) */
            next_state = transO->state_number;
            list->next = transO;
            list = list->next;
            if(trans_table[transO->tag_number] == -1){
                /* we add a new tag in the tags table */
                unichar* tmp = (unichar*)malloc(sizeof(unichar)*4096);
                TfstTag_to_string(tag,tmp);
                vector_ptr_add(new_tags,u_strdup(tmp));
                free(tmp);
                trans_table[transO->tag_number] = new_tags->nbelems-1;
                transO->tag_number = new_tags->nbelems-1;
            }
            else{
                transO->tag_number = trans_table[transO->tag_number];
            }
        }
        else{
            /* this transition is not the best at this state so we delete it */
            list->next = transO->next;
            free_Transition(transO);
            transO = list;
        }
        free_dela_entry(entry);
    }
    state->outgoing_transitions = first->next;
    free(first);
    if(next_state == -1){
        fatal_error("Invalid state value in do_backtracking\n");
    }
    state = graph->states[next_state];
    if(is_final_state(state) == 1){
        break;
    }
}
free(trans_table);
free(state_sequence);
/* "trim" aims at deleting non accessible nodes */
trim(graph,NULL);
/* we return the new vector of tags of the pruned automata */
return new_tags;
}

/**
 * Computes the Viterbi Path algorithm to find the best path in
 * the automata and then this path is used to prune transitions.
 */
vector_ptr* do_viterbi(Dictionary* d,const Alphabet* alphabet,Tfst* input_tfst,int form_type){
SingleGraph automaton = input_tfst->automaton;
int index_matrix = 2;
topological_sort(automaton,NULL);
compute_reverse_transitions(automaton);
struct matrix_entry** matrix = initialize_viterbi_matrix(automaton,form_type);
for(int i=0;i<automaton->number_of_states;i++){
    SingleGraphState state = automaton->states[i];
    for(Transition* transO=state->outgoing_transitions;transO!=NULL;transO=transO->next){
        TfstTag* tag = (TfstTag*)input_tfst->tags->tab[transO->tag_number];
        unichar* content = compound_to_simple(tag->content);
        int value = create_matrix_entry(content,&matrix[index_matrix],form_type,transO->tag_number,i);
        free(content);
        if(value == -1){
            free(tag->content);
            tag->content = (unichar*)malloc(sizeof(unichar)*DIC_LINE_SIZE);
            if(tag->content == NULL){
                fatal_alloc_error("do_viterbi");
            }
            build_tag(matrix[index_matrix]->tag,NULL,tag->content);
        }
        if(is_initial_state(state) != 0){
            /* initial state has no incoming transitions so we
             * calculate probabilities in a separate process */
            compute_best_probability(d,alphabet,matrix,index_matrix,1,0);
        }
        for(Transition* transI=state->reverted_incoming_transitions;transI!=NULL;transI=transI->next){
            TfstTag* tagI = (TfstTag*)input_tfst->tags->tab[transI->tag_number];
            unichar* content_2 = compound_to_simple(tagI->content);
            int indexI = search_matrix_predecessor(matrix,content_2,index_matrix-1,
                         transI->tag_number,transI->state_number);
            free(content_2);
            int cover_span = same_positions(&tagI->m,&tag->m);
            compute_best_probability(d,alphabet,matrix,index_matrix,indexI,cover_span);
        }
        index_matrix++;
    }
}
/* we compute the backtracking part of the process to prune the automata */
vector_ptr* new_tags = do_backtracking(matrix,index_matrix-1,automaton,input_tfst->tags,form_type);
/* we liberate all structures allocated during the process */
free_viterbi_matrix(matrix,index_matrix);
return new_tags;
}

/*
 * Returns 1 if the data file contains 'inflected' forms;
 * this information is encoded in the dictionary
 * at the line "CODE\tFEATURES";returns 0 otherwise.
 */
int get_form_type(Dictionary* d,const Alphabet* alphabet){
unichar code_type[DIC_LINE_SIZE];
u_sprintf(code_type,"CODE\tFEATURES");
long int value = get_sequence_integer(code_type,d,alphabet);
if(value == -1){
    fatal_error("Bad value in get_form_type\n");
}
return (int)value;
}

/**
 * Computes Viterbi Path algorithm on each sentence of the tfst.
 * This algorithm aims at pruning tokens of the automata in order to
 * obtain a linear path (the most probable path).
 */
void do_tagging(Tfst* input_tfst,Tfst* result_tfst,Dictionary* d,
                const Alphabet* alphabet,int form_type,
                struct hash_table* form_frequencies){
/* we write the number of sentences in the result tfst file */
u_fprintf(result_tfst->tfst,"%010d\n",input_tfst->N);
/* for each sentence we compute Viterbi Path algorithm */
for(int i=1;i<=input_tfst->N;i++){
    load_sentence(input_tfst,i);
    vector_ptr* new_tags = do_viterbi(d,alphabet,input_tfst,form_type);
    save_current_sentence(input_tfst,result_tfst->tfst,result_tfst->tind,
            (unichar**)new_tags->tab,new_tags->nbelems,form_frequencies);
    free_vector_ptr(new_tags,free);
    if(i%100 == 0){
        u_printf("Sentence %d/%d...\r",i,input_tfst->N);
    }
}
u_printf("\n");
}

} // namespace unitex
