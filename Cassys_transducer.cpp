/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * Cassys_transducer.cpp
 *
 *  Created on: 8 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */


#include <ctype.h>
#include "File.h"
#include "Cassys.h"
#include "Cassys_transducer.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Adds the transducer 'fileName' to the linked list of transducer 'current_list'. The mode of the transductor is assumed to be by
 * 'set_last_transducer_linked_list_mode' function
 */
struct transducer_name_and_mode_linked_list* add_transducer_linked_list_new_name(
			struct transducer_name_and_mode_linked_list *current_list,
			const char*filename,
			int repeat_mode, int generic_graph)
{
    struct transducer_name_and_mode_linked_list* new_item=(struct transducer_name_and_mode_linked_list*)malloc(sizeof(struct transducer_name_and_mode_linked_list));
    if (new_item==NULL) {
		fatal_alloc_error("add_transducer_linked_list_new_name");
		exit(1);
	}

    new_item->transducer_filename = strdup(filename);
    new_item->transducer_mode=IGNORE_OUTPUTS;
    new_item->repeat_mode=repeat_mode;
    new_item->generic_graph = generic_graph;
    new_item->next=NULL;
    if (new_item->transducer_filename==NULL) {
		fatal_alloc_error("add_transducer_linked_list_new_name");
		exit(1);
	}

    if (current_list==NULL)
        return new_item;

    struct transducer_name_and_mode_linked_list *browse_current_list = current_list;

    while (browse_current_list->next != NULL)
        browse_current_list = browse_current_list->next;
    browse_current_list->next=new_item;
    return current_list;
}

int extract_cassys_generic_mark(const char *line) {

	int i = 0;
	// filename
	while (line[i] != '"' && line[i] != '\0') {
		i++;
	}
	i++;
	while (line[i] != '"' && line[i] != '\0') {
		i++;
	}
	i++;
	while (isspace(line[i])) {
		i++;
	}

	// merge or replace policy
	while (isalpha(line[i])) {
		i++;
	}
	while (isspace(line[i])) {
		i++;
	}

	// disabled or enabled
	while (isalpha(line[i])) {
		i++;
	}
	while (isspace(line[i])) {
		i++;
	}
        //fix until
	i++;
	//generic graph
        if(line[i] != '\0') {
            while (line[i] != '\0' && isspace(line[i])) {
		i++;
            }
            if(line[i] != '\0' && line[i] == '@')
                return 1;
        }
        
        return 0;

}

struct transducer_name_and_mode_linked_list* add_transducer_linked_list_new_name(
			struct transducer_name_and_mode_linked_list *current_list,
			const char*filename)
{
	return add_transducer_linked_list_new_name(current_list, filename, 1,0);
}

void set_last_transducer_linked_list_mode(struct transducer_name_and_mode_linked_list *current_list,OutputPolicy mode)
{
    struct transducer_name_and_mode_linked_list *browse_current_list = current_list;
    if (current_list == NULL)
        return;
    while (browse_current_list->next != NULL)
        browse_current_list = browse_current_list->next;
    browse_current_list->transducer_mode = mode;
}


OutputPolicy GetOutputPolicyFromString(const char*option_name)
{
	if (strcmp(option_name, "M") == 0 || strcmp(option_name, "MERGE") == 0 || strcmp(
			option_name, "Merge") == 0) {
		return MERGE_OUTPUTS;
	}
	if (strcmp(option_name, "R") == 0 || strcmp(option_name, "REPLACE") == 0
			|| strcmp(option_name, "Replace") == 0) {
		return REPLACE_OUTPUTS;
	}
	return IGNORE_OUTPUTS;
}




void set_last_transducer_linked_list_mode_by_string(struct transducer_name_and_mode_linked_list *current_list,const char*option_name)
{
    set_last_transducer_linked_list_mode(current_list,GetOutputPolicyFromString(option_name));
}



void free_transducer_name_and_mode_linked_list(struct transducer_name_and_mode_linked_list *list)
{
    while (list!=NULL) {
        struct transducer_name_and_mode_linked_list *list_next = list->next;
        free(list->transducer_filename);
        free(list);
        list = list_next;
    }
}


void translate_path_separator_to_native_in_filename(char* filename) {
	char * walk = filename;
	while ((*walk) != '\0') {
		char c = *walk;
		if ((c == '\\') || (c == '/')) {
			*walk = PATH_SEPARATOR_CHAR;
		}
		walk++;
	}
}
 
struct transducer_name_and_mode_linked_list *load_transducer_list_file(const char *transducer_list_name, int translate_path_separator_to_native) {

	U_FILE *file_transducer_list;
    struct transducer_name_and_mode_linked_list * res=NULL;

	file_transducer_list = u_fopen(ASCII, transducer_list_name,U_READ);
	if( file_transducer_list == NULL){
		fatal_error("Cannot open file %s\n",transducer_list_name);
		exit(1);
	}

    char line[1024];
    int i=1;
	while (cassys_fgets(line,1024,file_transducer_list) != NULL){
		char *transducer_file_name;
		char *enabled_policy;
		int repeat_policy;
                int generic_graph;

		OutputPolicy transducer_policy;

		remove_cassys_comments(line);

		transducer_file_name = extract_cassys_transducer_name(line);
		if ((translate_path_separator_to_native != 0) && (transducer_file_name != NULL)) {
			translate_path_separator_to_native_in_filename(transducer_file_name);

		}

		//u_printf("transducer name read =%s\n",transducer_file_name);

		transducer_policy = extract_cassys_transducer_policy(line);
		enabled_policy = extract_cassys_disabled(line);
		repeat_policy = extract_cassys_tranducer_star(line);
                generic_graph = extract_cassys_generic_mark(line);
		if (transducer_file_name != NULL && transducer_policy != IGNORE_OUTPUTS && (strcmp("",enabled_policy)==0 || strcmp("Enabled",enabled_policy)==0)) {
			res=add_transducer_linked_list_new_name(res,transducer_file_name, repeat_policy, generic_graph);
            set_last_transducer_linked_list_mode(res,transducer_policy);
		}
		else {
			if (transducer_file_name == NULL) {
				u_printf("Line %d : Empty line\n",i);
			} else if (transducer_policy == IGNORE_OUTPUTS) {
				u_printf("Line %d : Transducer policy not recognized\n",i);
			}
			if(strcmp("Disabled",enabled_policy)!=0){
				u_printf("Line %d : Could not recognize whether transducer is enabled\n",i);
			} else {
				u_printf("transducer %s is Disabled\n",transducer_file_name);
			}
		}
		free(enabled_policy);
        free(transducer_file_name);
		i++;
	}
    u_fclose(file_transducer_list);

	return res;
}

/**  if a filename must be concatenated, we must remove the absolute prefix on filename to concat
  *  (by example, replace 'c:\folder\sub\file' by 'folder\sub\file')
  */
static const char*skip_absolute_prefix(const char* filename) {
	// if Windows filename begin with 'c:', we skip two char
	if ((*(filename)) != '\0')
		if ((*(filename + 1)) == ':')
			filename += 2;
	if (((*filename) == '\\') | ((*filename) == '/'))
		filename++;
	return filename;
}

struct fifo *load_transducer_from_linked_list(const struct transducer_name_and_mode_linked_list *list,const char* transducer_filename_prefix){
	struct fifo *transducer_fifo = new_fifo();

	int i=1;
	while (list != NULL){
		char *transducer_file_name;
		OutputPolicy transducer_policy;
		transducer *t;
		int repeat_policy;
                int generic_graph;

        transducer_file_name = list->transducer_filename;
		//u_printf("transducer name read =%s\n",transducer_file_name);

        transducer_policy = list->transducer_mode;
        repeat_policy = list->repeat_mode;
        generic_graph = list->generic_graph;

		if (transducer_file_name != NULL && transducer_policy != IGNORE_OUTPUTS) {
			//u_printf("transducer to be loaded\n");
			t = (transducer*) malloc(sizeof(transducer) * 1);
			if (t == NULL) {
				fatal_alloc_error("load_transducer_from_linked_list");
				exit(1);
			}
            size_t transducer_filename_prefix_len = 0;
            if (transducer_filename_prefix != NULL)
                transducer_filename_prefix_len = strlen(transducer_filename_prefix);
			t->transducer_file_name = (char*)malloc(sizeof(char)*(transducer_filename_prefix_len+strlen(transducer_file_name)+1));
			if(t->transducer_file_name == NULL){
				fatal_alloc_error("load_transducer_from_linked_list");
				exit(1);
			}

            t->transducer_file_name[0] = '\0';
            if (transducer_filename_prefix != NULL)
                strcpy(t->transducer_file_name, transducer_filename_prefix);

			const char* transducer_file_name_to_add = (transducer_filename_prefix_len > 0) ? skip_absolute_prefix(transducer_file_name) : transducer_file_name;
			strcat(t->transducer_file_name, transducer_file_name_to_add);

			t->output_policy = transducer_policy;
			t->repeat_mode = repeat_policy;
                        t->generic_graph = generic_graph;

			struct any value;
			value._ptr = t;
			put_any(transducer_fifo,value);
			if (!is_empty(transducer_fifo)) {
				u_printf("transducer %s successfully loaded\n",
						t->transducer_file_name);
			}
		}
		else {
			if (transducer_file_name == NULL) {
				u_printf("Transducer %d : Empty filename\n",i);
			} else if (transducer_policy == IGNORE_OUTPUTS) {
				u_printf("Transducer %d : Transducer mode not recognized\n",i);
			}
		}
		i++;
        list=list->next;
	}


	return transducer_fifo;
}


/**
 * \brief Suppress Cassys comment line.
 */
void remove_cassys_comments(char *line){
	int i=0;
	while(line[i] != '\0' && line[i] != '#'){
		i++;
	}
	line[i]='\0';
}

/**
 * \brief \b fgets working with \b U_FILE and storing \b char
 *
 * Needed to process configuration file
 *
 * @param[out] line the text read
 * @param[in] n max number of character read
 * @param[in] u file descriptor
 *
 * @return NULL if no character has been read before \c EOF has been encountered, \c line otherwise
 */
char *cassys_fgets(char *line, int n, U_FILE *u) {
	int i = 0;
	int c;

	c = u_fgetc(u);
	if (c == EOF) {
		return NULL;
	}
	while (c != EOF && c != '\n' && i < n) {
		line[i] = (char) c;
		c=u_fgetc(u);
		i++;
	}
	line[i] = '\0';
	//u_printf("fgets result =%s\n",line);
	return line;
}



/**
 *
 */
char* extract_cassys_transducer_name(const char *line){
	char *transducer_name;
	int i=0;
	while(line[i]!='"' && line[i] != '\0'){
		i++;
	}
	if(line[i] == '\0'){
		return NULL;
	}
	i++;
	int j=i;
	while(line[i]!='"' && line[i] != '\0'){
			i++;
		}
	if(line[i] == '\0'){
			return NULL;
		}

	transducer_name = (char*)malloc(sizeof(char)*((i-j)+1));
	if(transducer_name == NULL){
		fatal_alloc_error("extract_cassys_transducer_name");
		exit(1);
	}
	strncpy(transducer_name,line+j,(i-j));
	transducer_name[i-j]='\0';

	return transducer_name;
}


int extract_cassys_tranducer_star(const char *line) {

	int i = 0;
	// filename
	while (line[i] != '"' && line[i] != '\0') {
		i++;
	}
	i++;
	while (line[i] != '"' && line[i] != '\0') {
		i++;
	}
	i++;
	while (isspace(line[i])) {
		i++;
	}

	// merge or replace policy
	while (isalpha(line[i])) {
		i++;
	}
	while (isspace(line[i])) {
		i++;
	}

	// disabled or enabled
	while (isalpha(line[i])) {
		i++;
	}
	while (isspace(line[i])) {
		i++;
	}

	if(line[i]=='*'){
		return INFINITY;
	}

	return 1;

}

char *extract_cassys_disabled(const char *line){
	char *enabled_policy;

	int i = 0;
	// filename
	while (line[i] != '"' && line[i] != '\0') {
		i++;
	}
	i++;
	while (line[i] != '"' && line[i] != '\0') {
		i++;
	}
	i++;
	while (isspace(line[i])) {
		i++;
	}

	// merge or replace policy
	while (isalpha(line[i])) {
		i++;
	}
	while (isspace(line[i])) {
		i++;
	}

	int j=0;
	char option_name[FILENAME_MAX];
	while(isalpha(line[i])){
		option_name[j]=line[i];
		i++;j++;
	}
	option_name[j]='\0';

	enabled_policy = (char *)malloc(sizeof(char)*strlen(option_name)+1);
	if (enabled_policy == NULL) {
		fatal_alloc_error("extract_cassys_disabled memory allocation\n");
		exit(1);
	}
	strcpy(enabled_policy,option_name);

	return enabled_policy;

}


OutputPolicy extract_cassys_transducer_policy(const char *line) {
	int i = 0;
	while (line[i] != '"' && line[i] != '\0') {
		i++;
	}
	i++;
	while (line[i] != '"' && line[i] != '\0') {
		i++;
	}
	i++;
	while(isspace(line[i])){
		i++;
	}

	char option_name[FILENAME_MAX];
	int j=0;
	while(isalpha(line[i])){
		option_name[j]=line[i];
		i++;j++;
	}

	option_name[j]='\0';

	//u_printf("extract option =%s\n",option_name);

	if (strcmp(option_name, "M") == 0 || strcmp(option_name, "MERGE") == 0 || strcmp(
			option_name, "Merge") == 0) {
		return MERGE_OUTPUTS;
	}
	if (strcmp(option_name, "R") == 0 || strcmp(option_name, "REPLACE") == 0
			|| strcmp(option_name, "Replace") == 0) {
		return REPLACE_OUTPUTS;
	}
	return IGNORE_OUTPUTS;
}

bool is_debug_mode(transducer *t, const VersatileEncodingConfig* vec){

	U_FILE *graph_file;
	graph_file = u_fopen(vec, t->transducer_file_name, U_READ);
	if (graph_file == NULL) {
		fatal_error("Cannot open file %s\n", t->transducer_file_name);
		exit(1);
	}

	unichar c = (unichar)u_fgetc(graph_file);

	u_fclose(graph_file);

	if(c=='d'){
		return true;
	} else {
		return false;
	}

}

}



