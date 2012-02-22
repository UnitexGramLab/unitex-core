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

/*
 * author : Anthony Sigogne
 */

#include "TrainingProcess.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Creates a Disclaimer text file for each one of the training dictionaries.
 * Indicates that those cannot be used like other dictionaries.
 */
void create_disclaimer(const VersatileEncodingConfig* vec,const char* file){
	U_FILE* disclaimer = u_fopen(vec,file,U_WRITE);
	u_fprintf(disclaimer,"This file contains statistics gathered from a tagged corpus.\nIt cannot be used like other .bin dictionaries but only in input of Tagger program.\n\nContact : unitex@univ-mlv.fr");
	u_fclose(disclaimer);
}

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
 * Allocates, initializes and returns a new corpus_entry structure.
 */
struct corpus_entry* new_corpus_entry(const unichar* line){
struct corpus_entry* entry = (corpus_entry*)malloc(sizeof(corpus_entry));
if(entry == NULL){
	fatal_alloc_error("compute_corpus_entry");
}
/* we fill corpus entry with information extracted from the corpus line*/
int pos = u_strrchr(line,'/');
if(pos == -1){
	fatal_error("Wrong format for line %S\n",line);
}
entry->word = (unichar*)malloc(sizeof(unichar)*(pos+1));
if(entry->word == NULL){
	fatal_alloc_error("compute_corpus_entry");
}
unichar* tmp = u_strcpy_sized(entry->word,pos+1,line);
u_strcat(tmp,"\0");

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
	unichar* tmp2 = u_strcpy_sized(entry->pos_code,code_pos-pos,&line[pos+1]);
	u_strcat(tmp2,"\0");
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
 * Check the format of the first line of the corpus in order
 * to determine the format of the whole corpus.
 */
int check_corpus_entry(const unichar* line){
if(u_strrchr(line,'/') == -1 || line[0] == '{')
	return 1;
return 0;
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
void add_key_table(const unichar* key,struct string_hash_ptr* table){
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
void add_key_table(const char* key,struct string_hash_ptr* table){
unichar* str = u_strdup(key);
add_key_table(str,table);
free(str);
}

/**
 * Computes string for contextual entries. For example, if we have contextual matrix
 * at [(je,PRO:ms),(mange,V:P3s),(des,DET:ms)], so contextual entries will be :
 * "PRO V DET" for raw forms and "PRO:ms V:P3s DET:ms" for inflected forms.
 */
unichar* compute_contextual_entries(struct corpus_entry** context,int start_index,int mode){
unichar* line = NULL;
if(mode == INFLECTED_FORMS){
	line = u_strdup(context[start_index]->overall_codes);
}
else if(mode == RAW_FORMS){
	line = u_strdup(context[start_index]->pos_code);
}
unichar* tmp = NULL;
for(int i=start_index+1;i<MAX_CONTEXT;i++){
	if (mode == RAW_FORMS){
		tmp = create_bigram_sequence(line,context[i]->pos_code,1);
	}
	else if(mode == INFLECTED_FORMS){
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
void add_statistics(struct corpus_entry** context,struct string_hash_ptr* rforms_table,
		            struct string_hash_ptr* iforms_table){
char prefix[] = "word_";
/* we first raise the number of time current word occurs in the corpus (unigrams) */
struct corpus_entry* current = context[MAX_CONTEXT-1];
unichar* word = create_bigram_sequence(prefix,current->word,0);
if(rforms_table != NULL){
	add_key_table(word,rforms_table);
}
if(iforms_table != NULL){
	add_key_table(word,iforms_table);
}
/* then we compute line for bigrams (tag,word), bigrams (tag(i-1),tag(i))
 * and trigrams (tag(i-2),tag(i-1),tag(i)) */
unichar* line = NULL;
if(rforms_table != NULL){
	line = create_bigram_sequence(current->pos_code,word,1);
	add_key_table(line,rforms_table);
	free(line);
	line = compute_contextual_entries(context,1,RAW_FORMS);
	add_key_table(line,rforms_table);
	free(line);
	line = compute_contextual_entries(context,0,RAW_FORMS);
	add_key_table(line,rforms_table);
	free(line);
}
if(iforms_table != NULL){
	line = create_bigram_sequence(current->overall_codes,word,1);
	add_key_table(line,iforms_table);
	free(line);
	line = compute_contextual_entries(context,1,INFLECTED_FORMS);
	add_key_table(line,iforms_table);
	free(line);
	line = compute_contextual_entries(context,0,INFLECTED_FORMS);
	add_key_table(line,iforms_table);
	free(line);
}
free(word);
}

/**
 * Writes pairs (key,value) into the file containing statistics.
 */
void write_keys_values(struct string_hash_ptr* table,struct string_hash_tree_node* node,const unichar* str,U_FILE* file){
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

corpus_entry* new_simple_word_entry(const unichar* word,corpus_entry* entry,int start){
	corpus_entry* wentry = (corpus_entry*)malloc(sizeof(corpus_entry));
	wentry->word = u_strdup(word);
	wentry->pos_code = (unichar*)malloc(sizeof(unichar)*(u_strlen(entry->pos_code)+3));
	wentry->overall_codes = (unichar*)malloc(sizeof(unichar)*(u_strlen(entry->overall_codes)+3));
	unichar* tmp = u_strcpy_sized(wentry->pos_code,u_strlen(entry->pos_code)+1,entry->pos_code);
	unichar* tmp2 = u_strcpy_sized(wentry->overall_codes,u_strlen(entry->overall_codes)+1,entry->overall_codes);
	if(start == 0){
		u_strcat(tmp,"+I\0");
		u_strcat(tmp2,"+I\0");
	}
	else {
		u_strcat(tmp,"+B\0");
		u_strcat(tmp2,"+B\0");
	}
	return wentry;
}

/**
 * Extract simple words contained into compound words.
 */
corpus_entry** extract_simple_words(corpus_entry* entry){
	corpus_entry** words = (corpus_entry**)malloc(sizeof(corpus_entry)*100);
	words[0] = NULL;
	unichar* word = u_strchr(entry->word,'_');
	int old_value=0,nb_words=0;
	while(word != NULL) {
		unichar* simple_word = u_strdup(entry->word+old_value,u_strlen(entry->word)-old_value-u_strlen(word));
		if(simple_word[0] != '-'){
			if(nb_words > 0){
				words[nb_words] = new_simple_word_entry(simple_word,entry,0);
			}
			else{
				words[nb_words] = new_simple_word_entry(simple_word,entry,1);
			}
		}
		else{
			nb_words -= 1;
		}
		nb_words+=1;
		old_value += u_strlen(simple_word)+1;
		word = u_strchr(entry->word+old_value,'_');
		free(simple_word);
		if(word == NULL){
			if(u_strlen(entry->word)-old_value != 0){
				word = entry->word+old_value;
				words[nb_words] = new_simple_word_entry(word,entry,0);
				words[nb_words+1] = NULL;
			}
			break;
		}
	}
	return words;
}

/**
 * Computes training by extracting statistics from a tagged corpus file.
 */
void do_training(U_FILE* input_text,U_FILE* rforms_file,U_FILE* iforms_file){
/* these two hash tables are respectively for simple and compound entries */
struct string_hash_ptr* rforms_table = NULL, *iforms_table = NULL;
if(rforms_file != NULL){
	rforms_table = new_string_hash_ptr(200000);
}
if(iforms_file != NULL){
	iforms_table = new_string_hash_ptr(200000);
}


/* we initialize a contextual matrix */
struct corpus_entry** context = new_context_matrix();
initialize_context_matrix(context);


unichar line[MAX_TAGGED_CORPUS_LINE];

/* check the format of the corpus */
long previous_file_position = ftell(input_text);
if(u_fgets(line,input_text) == EOF){
	fatal_error("File is empty");
}
fseek(input_text,previous_file_position,SEEK_SET);

int format_corpus = check_corpus_entry(line);

if(format_corpus == 0){
	// the corpus is in the Tagger format, one word per line where line=word/tag
	while(u_fgets(line,input_text) !=EOF){
		if(u_strlen(line) == 0){
			initialize_context_matrix(context);
		}
		else{
			corpus_entry* entry = new_corpus_entry(line);
			if(u_strchr(line,'_')!=NULL && line[0]!='_'){
				corpus_entry** entries = extract_simple_words(entry);
				free_corpus_entry(entry);
				for(int i=0;entries[i]!=NULL;i++){
					push_corpus_entry(entries[i],context);
					add_statistics(context,rforms_table,iforms_table);
				}
				free(entries);
			}
			else {
				push_corpus_entry(entry,context);
				add_statistics(context,rforms_table,iforms_table);
			}
		}
	}
}
else {
	// the corpus is in the Unitex tagged format, one sentence per line where token={word,lemma.tag}
	unichar *tmp,*s = (unichar*)malloc(sizeof(unichar)*(MAX_TAGGED_CORPUS_LINE));
	int current_len,len;
	unsigned int i;
	while(u_fgets(line,input_text) != EOF){
		current_len = 0, len = 0;
		/* extract each token of the sentence */
		for (;;) {
			len = 1+u_strlen(line+current_len)-u_strlen(u_strchr(line+current_len,'}'));
			tmp = u_strcpy_sized(s,len-1,line+current_len+1);
			u_strcat(tmp,"\0");
			if(u_strcmp(s,"S") == 0)
				break;

			//particular case: '\},\}.PONCT'
			if(line[current_len+2] == '}'){
				int start = current_len+3;
				do{
					tmp = u_strchr(line+start,'}');
					start += 1+u_strlen(line+start)-u_strlen(tmp);
				}
				while(*(tmp+1) != ' ');
				tmp = u_strcpy_sized(s,start-current_len-1,line+current_len+1);
				u_strcat(tmp,"\0");
				len += start-current_len-3;
			}

			/* format the {XX.YY} into standard tagger format, XX/YY */
			unichar* newline = (unichar*)malloc(sizeof(unichar)*(8096));
			if(u_strchr(s,',')[1] == ','){
				u_strcpy(newline,",");
			}
			else
				u_strcpy_sized(newline,1+u_strlen(s)-u_strlen(u_strchr(s,',')),s);
			u_sprintf(newline,"%S/%S\0",newline,s+u_strrchr(s,'.')+1);
			for(i=0;i<u_strlen(newline);i++){
				if(newline[i] == ' ')
					newline[i] = '_';
			}

			//create corpus entry
			corpus_entry* entry = new_corpus_entry(newline);
			if(u_strchr(newline,'_') != NULL && newline[0] != '_'){
				corpus_entry** entries = extract_simple_words(entry);
				free_corpus_entry(entry);
				for(int i=0;entries[i]!=NULL;i++){
					push_corpus_entry(entries[i],context);
					add_statistics(context,rforms_table,iforms_table);
				}
				free(entries);
			}
			else {
				push_corpus_entry(entry,context);
				add_statistics(context,rforms_table,iforms_table);
			}

			free(newline);
			current_len += len+1;
		}
		initialize_context_matrix(context);
	}
	free(s);
}
free_context_matrix(context);
/* we fill dictionary files with pairs (tuple,value) and then
 * we add a special line "CODE\tFEATURES,.value" in order to
 * specify whether the dictionary contains inflected or raw form tuples*/
unichar* str = u_strdup("");
if(rforms_table != NULL){
	write_keys_values(rforms_table,rforms_table->hash->root,str,rforms_file);
	u_fprintf(rforms_file,"%s,.%d\n","CODE\tFEATURES",0);
	free_string_hash_ptr(rforms_table,NULL);
}
if(iforms_table != NULL){
	write_keys_values(iforms_table,iforms_table->hash->root,str,iforms_file);
	u_fprintf(iforms_file,"%s,.%d\n","CODE\tFEATURES",1);
	free_string_hash_ptr(iforms_table,NULL);
}
free(str);
}

} // namespace unitex
