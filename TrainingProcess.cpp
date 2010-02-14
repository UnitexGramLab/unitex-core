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

/*
 * author : Anthony Sigogne
 */

#include "TrainingProcess.h"

/**
 * Frees all the memory associated to the given corpus_entry structure.
 */
void free_corpus_entry(struct corpus_entry* entry){
free(entry->overall_codes);
free(entry->pos_code);
free(entry->word);
free(entry);
}

/**
 * a new corpus_entry is pushes into the contextual matrix,
 * so we have to move back all elements to the left.
 * The leftmost object is deleted from matrix.
 */
void push_corpus_entry(struct corpus_entry* entry,struct corpus_entry** context){
free_corpus_entry(context[0]);
for(int i=0;i<MAX_CONTEXT-1;i++){
	context[i] = context[i+1];
}
context[MAX_CONTEXT-1] = entry;
}

/**
 * searches for a caracter in a string by starting from the end.
 * Return index of the position if found, -1 otherwise.
 */
int u_strrchr(unichar* s,unichar t){
for(int i=u_strlen(s)-1;i>0;i--){
	if(s[i]==t){
		return i;
	}
}
return -1;
}

/**
 * same as u_strrchr(unichar*,unichar) but here a char is searched.
 */
int u_strrchr(unichar* s,char t){
unichar dest[1];
u_sprintf(dest,"%c",t);
return u_strrchr(s,dest[0]);
}

/**
 * Allocates, initializes and returns a new corpus_entry structure.
 */
struct corpus_entry* new_corpus_entry(unichar* line){
struct corpus_entry* entry = (corpus_entry*)malloc(sizeof(corpus_entry));
if(entry == NULL){
	fatal_alloc_error("compute_corpus_entry");
}
/* we fill corpus entry with information extracted from the corpus line*/
int pos = u_strrchr(line,'/');
if(pos == -1){
	fatal_error("Bad return value of strrchr in compute_corpus_entry\n");
}
entry->word = (unichar*)malloc(sizeof(unichar)*(pos+1));
if(entry->word == NULL){
	fatal_alloc_error("compute_corpus_entry");
}
unichar* tmp = u_strcpy_sized(entry->word,pos+1,line);
u_strcat(tmp,"\0");
/* if we treat with compound tags, we don't catch this information
 * (maybe later) */
int cpos = u_strrchr(line,'+');
if(cpos != -1){
	pos = cpos;
}
int code_pos = u_strrchr(line,':');
/* there are no morphological codes associated to this entry */
if(code_pos == -1){
	entry->pos_code = (unichar*)malloc(sizeof(unichar)*(u_strlen(line)-pos));
	if(entry->pos_code == NULL){
		fatal_alloc_error("new_corpus_entry");
	}
	u_strcpy(entry->pos_code,&line[pos+1]);
	entry->overall_codes = u_strdup(entry->pos_code);
}
else{
	entry->pos_code = (unichar*)malloc(sizeof(unichar)*(code_pos-pos));
	if(entry->pos_code == NULL){
		fatal_alloc_error("new_corpus_entry");
	}
	entry->overall_codes = (unichar*)malloc(sizeof(unichar)*(u_strlen(line)-pos));
	if(entry->overall_codes == NULL){
		fatal_alloc_error("new_corpus_entry");
	}
	unichar* tmp = u_strcpy_sized(entry->pos_code,code_pos-pos,&line[pos+1]);
	u_strcat(tmp,"\0");
	u_strcpy(entry->overall_codes,&line[pos+1]);
}
/* if the token is not annotated in the corpus, we put "UNK" */
if(u_strlen(entry->pos_code) == 0){
	free(entry->pos_code);
	free(entry->overall_codes);
	entry->pos_code = u_strdup("UNK");
	entry->overall_codes = u_strdup("UNK");
}
return entry;
}

/**
 * Frees all the memory associated to the given matrix of corpus_entry structure.
 */
void free_context_matrix(struct corpus_entry** context){
for(int i=0;i<MAX_CONTEXT;i++){
	if(context[i]!=NULL){
		free_corpus_entry(context[i]);
		context[i] = NULL;
	}
}
free(context);
}

/**
 * Frees all the memory associated to the given matrix of corpus_entry structure.
 */
void initialize_context_matrix(struct corpus_entry** context){
for(int i=0;i<MAX_CONTEXT;i++){
	if(context[i] != NULL){
		free_corpus_entry(context[i]);
	}
	unichar* str = u_strdup("#/#");
	context[i] = new_corpus_entry(str);
	free(str);
}
}

/**
 * Allocates, initializes and returns a new matrix of corpus_entry structure.
 */
struct corpus_entry** new_context_matrix(){
struct corpus_entry** matrix = (struct corpus_entry**)malloc(sizeof(struct corpus_entry)*MAX_CONTEXT);
if(matrix == NULL){
	fatal_alloc_error("new_context_matrix");
}
for(int i=0;i<MAX_CONTEXT;i++){
	matrix[i] = NULL;
}
return matrix;
}

/**
 * Adds a new pair (key,value) in the hash table.
 */
void add_key_table(unichar* key,struct string_hash_ptr* table){
void* value = get_value(key,table);
if(value != NULL){
	table->value[get_value_index(key,table)] = (void*)(((char*)value)+1);
}
else{
	get_value_index(key,table,INSERT_IF_NEEDED,(void*)1);
}
}

/**
 * Adds a new pair (key,value) in the hash table.
 * Here the key is a string (char*).
 */
void add_key_table(char* key,struct string_hash_ptr* table){
unichar* str = u_strdup(key);
add_key_table(str,table);
free(str);
}

/**
 * Computes string for contextual entries. For example, if we have contextual matrix
 * at [(je,PRO:ms),(mange,V:P3s),(des,DET:ms)], so contextual entries will be :
 * "PRO V DET" for simple forms and "PRO:ms V:P3s DET:ms" for compound forms.
 */
unichar* compute_contextual_entries(struct corpus_entry** context,int start_index,int mode){
unichar* line = NULL;
if(mode == COMPOUND_FORMS){
	line = u_strdup(context[start_index]->overall_codes);
}
else if(mode == SIMPLE_FORMS){
	line = u_strdup(context[start_index]->pos_code);
}
unichar* tmp = NULL;
for(int i=start_index+1;i<MAX_CONTEXT;i++){
	if (mode == SIMPLE_FORMS){
		tmp = create_bigram_sequence(line,context[i]->pos_code,1);
	}
	else if(mode == COMPOUND_FORMS){
		tmp = create_bigram_sequence(line,context[i]->overall_codes,1);
	}
	free(line);
	line = tmp;
}
return line;
}

/**
 * Computes lexical and contextual entries to put into the file containing statistics.
 */
void add_statistics(struct corpus_entry** context,struct string_hash_ptr* sforms_table,struct string_hash_ptr* cforms_table){
char prefix[] = "word_";
/* we first raise the number of time current word occurs in the corpus (unigrams) */
struct corpus_entry* current = context[MAX_CONTEXT-1];
unichar* word = create_bigram_sequence(prefix,current->word,0);
if(sforms_table != NULL){
	add_key_table(word,sforms_table);
}
if(cforms_table != NULL){
	add_key_table(word,cforms_table);
}
/* then we compute line for bigrams (tag,word), bigrams (tag(i-1),tag(i))
 * and trigrams (tag(i-2),tag(i-1),tag(i)) */
unichar* line = NULL;
if(sforms_table != NULL){
	line = create_bigram_sequence(current->pos_code,word,1);
	add_key_table(line,sforms_table);
	free(line);
	line = compute_contextual_entries(context,1,SIMPLE_FORMS);
	add_key_table(line,sforms_table);
	free(line);
	line = compute_contextual_entries(context,0,SIMPLE_FORMS);
	add_key_table(line,sforms_table);
	free(line);
}
if(cforms_table != NULL){
	line = create_bigram_sequence(current->overall_codes,word,1);
	add_key_table(line,cforms_table);
	free(line);
	line = compute_contextual_entries(context,1,COMPOUND_FORMS);
	add_key_table(line,cforms_table);
	free(line);
	line = compute_contextual_entries(context,0,COMPOUND_FORMS);
	add_key_table(line,cforms_table);
	free(line);
}
free(word);
}

/**
 * Writes pairs (key,value) into the file containing statistics.
 */
void write_keys_values(struct string_hash_ptr* table,struct string_hash_tree_node* node,unichar* str,U_FILE* file){
if(node->value_index != NO_VALUE_INDEX){
	u_fprintf(file,"%S,.%d\n",str,table->value[node->value_index]);
}
for(struct string_hash_tree_transition* trans=node->trans;trans!=NULL;trans=trans->next){
	unichar* new_str = (unichar*)malloc(DIC_LINE_SIZE);
	u_sprintf(new_str,"%S%C",str,trans->letter);
	write_keys_values(table,trans->node,new_str,file);
	free(new_str);
}
}

/**
 * Computes training by extracting statistics from a tagged corpus file.
 */
void do_training(U_FILE* input_text,U_FILE* sforms_file,U_FILE* cforms_file){
/* these two hash tables are respectively for simple and compound entries */
struct string_hash_ptr* sforms_table = NULL, *cforms_table = NULL;
if(sforms_file != NULL){
	sforms_table = new_string_hash_ptr(200000);
}
if(cforms_file != NULL){
	cforms_table = new_string_hash_ptr(200000);
}
/* we initialize a contextual matrix */
struct corpus_entry** context = new_context_matrix();
initialize_context_matrix(context);
unichar line[4096];
while(u_fgets(line,input_text) !=EOF){
	if(u_strlen(line) == 0){
		initialize_context_matrix(context);
	}
	else{
		corpus_entry* entry = new_corpus_entry(line);
		push_corpus_entry(entry,context);
		add_statistics(context,sforms_table,cforms_table);
	}
}
free_context_matrix(context);
/* we fill dictionary files with pairs (tuple,value) and then
 * we add a special line "CODE\tFEATURES,.value" in order to
 * specify whether the dictionary contains compound or simple form tuples*/
unichar* str = u_strdup("");
if(sforms_table != NULL){
	write_keys_values(sforms_table,sforms_table->hash->root,str,sforms_file);
	u_fprintf(sforms_file,"%s,.%d\n","CODE\tFEATURES",0);
	free_string_hash_ptr(sforms_table,NULL);
}
if(cforms_table != NULL){
	write_keys_values(cforms_table,cforms_table->hash->root,str,cforms_file);
	u_fprintf(cforms_file,"%s,.%d\n","CODE\tFEATURES",1);
	free_string_hash_ptr(cforms_table,NULL);
}
free(str);
}
