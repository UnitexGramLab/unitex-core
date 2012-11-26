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
 *
 *  Created on: 29 avr. 2010
 *  Authors: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#include "Cassys_tokens.h"
#include "Cassys.h"
#include "Cassys_concord.h"
#include "Cassys_lexical_tags.h"
#include "Snt.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {


void cassys_tokens_2_graph(cassys_tokens_list *c, const VersatileEncodingConfig* vec){

	U_FILE *dot_desc_file = u_fopen(ASCII, "/users/dnott/Bureau/graph/text.dot", U_WRITE);
	if (dot_desc_file == NULL) {
		fatal_error("Cannot open file \n");
		exit(1);
	}

	u_fprintf(dot_desc_file,"digraph text {\n");
	cassys_tokens_2_graph_subgraph(c,dot_desc_file);

	cassys_tokens_2_graph_walk_for_subgraph(c, dot_desc_file);

	u_fprintf(dot_desc_file, "\tNULL[label=\"EOF\"]\n");
	u_fprintf(dot_desc_file,"}\n");

	u_fclose(dot_desc_file);
}


void cassys_tokens_2_graph_walk_for_subgraph(cassys_tokens_list *c, U_FILE *u){

	cassys_tokens_list *i;
	for(i=c; i != NULL && i->transducer_id == c->transducer_id; i = i->next_token) {
		if(i->output != NULL) {
			cassys_tokens_2_graph_subgraph(i->output,u);
			cassys_tokens_2_graph_walk_for_subgraph(i->output, u);
			u_fprintf(u, "\t_%p -> _%p\n", i, i->output);
		}
	}
}

void cassys_tokens_2_graph_subgraph(cassys_tokens_list *c, U_FILE *u){

	static int cluster_number = 0;
	cassys_tokens_list *i;
	u_fprintf(u, "\tsubgraph cluster%d {\n", cluster_number);
	u_fprintf(u, "\tlabel = \"transducer = %d_%d\"\n", c->transducer_id, c->iteration);
	for (i = c; i != NULL && i->transducer_id == c->transducer_id; i = i->next_token) {
		u_fprintf(u, "\t\t_%p[label=\"%S(%d)\"]\n", i, i->token,i->transducer_id);
		if (i->next_token != NULL) {
			u_fprintf(u, "\t\t_%p -> _%p\n", i, i->next_token);
		} else {
			u_fprintf(u, "\t\t_%p -> NULL\n", i);
		}
	}
	u_fprintf(u,"\t}\n");
	cluster_number++;
}



cassys_tokens_list *cassys_load_text(const VersatileEncodingConfig* vec,const char *tokens_text_name, const char *text_cod_name, struct text_tokens **tokens){

	*tokens = load_text_tokens(vec,tokens_text_name);

	U_FILE *f = u_fopen(BINARY, text_cod_name,U_READ);
	if( f == NULL){
		fatal_error("Cannot open file %s\n",text_cod_name);
		exit(1);
	}

	cassys_tokens_list *list = NULL;
	cassys_tokens_list *temp = list;

	int token_id;
	int char_read = (int)fread(&token_id,sizeof(int),1,f);
	while(char_read ==1){
		if(list==NULL){
			list = new_element((*tokens)->token[token_id],0,0);
			temp = list;
		}
		else {
			temp ->next_token = new_element((*tokens)->token[token_id],0,0);
			temp = temp -> next_token;
		}

		char_read = (int)fread(&token_id,sizeof(int),1,f);
	}
	u_fclose(f);

	return list;
}



/**
 *
 */
cassys_tokens_list *add_replaced_text(
			const char *text, cassys_tokens_list *list,
			int previous_transducer, int previous_iteration,
			int transducer_id, int iteration,
			const char *alphabet_name,
			const VersatileEncodingConfig* vec) {

	locate_pos *prev_l=NULL;
	Alphabet *alphabet = load_alphabet(vec,alphabet_name);

	struct snt_files *snt_text_files = new_snt_files(text);

	struct fifo *stage_concord = read_concord_file(snt_text_files->concord_ind,vec);

	// performance enhancement
	cassys_tokens_list *current_list_position = list;
	long current_token_position = 0;

	while (!is_empty(stage_concord)) {

		locate_pos *l=(locate_pos*)take_ptr(stage_concord);
		if(prev_l!=NULL){ // manage the fact that when writing a text merging the concord.ind,
			// and when there is more than one pattern beginning at the same position in the text,
			// the behavior of concord.exe is to take the first of those patterns.
			// when we create the concordance of a cascade, it is needed to chose the same path (the first)
			// as in concord.exe
			if(prev_l->token_start_offset==l->token_start_offset){
				while(prev_l!=NULL && l!=NULL && prev_l->token_start_offset==l->token_start_offset){
					free(prev_l->label);
					free(prev_l);
					prev_l=l;
					if(!is_empty(stage_concord))
						l=(locate_pos*)take_ptr(stage_concord);
					else l=NULL;
				}
			}
			else {
				free(prev_l->label);
				free(prev_l);
				prev_l=NULL;
			}
		}
		if(l!=NULL){
			struct list_ustring *new_sentence_lu = cassys_tokenize_word_by_word(l->label, alphabet);
			cassys_tokens_list *new_sentence_ctl =
				new_list(new_sentence_lu, transducer_id, iteration);

			// performance enhancement :
			// Since matches are sorted, we begin the search from the last known position in the list.
			// We have to substract from the text position the current token position.
			cassys_tokens_list *list_position = get_element_at(current_list_position, previous_transducer, previous_iteration,
				l->token_start_offset - current_token_position);

			int replaced_sentence_length = l->token_end_offset - l->token_start_offset+1;
			int new_sentence_length = length(new_sentence_lu);

			add_output(list_position, new_sentence_ctl, previous_transducer, previous_iteration, transducer_id, iteration,
				replaced_sentence_length, new_sentence_length-1);

			// performance enhancement
			current_list_position = list_position;
			current_token_position = l-> token_start_offset;
			prev_l=l;
			free_list_ustring(new_sentence_lu);
		}
		if(l!=NULL && is_empty(stage_concord)){
			free(l->label);
			free(l);
		}
	}
	free_fifo(stage_concord);
	free_snt_files(snt_text_files);
    free_alphabet(alphabet);

	return list;
}








cassys_tokens_list *next_element(cassys_tokens_list *list, int transducer_id, int iteration){
	if(list->next_token == NULL){
		return NULL;
	}

	cassys_tokens_list *temp = list->next_token;
	temp = get_output(temp,transducer_id, iteration);

	return temp;
}


unichar *next_token(cassys_tokens_list *list, int transducer_id, int iteration){
	cassys_tokens_list *temp = next_element(list,transducer_id, iteration);

	if(temp == NULL){
		return NULL;
	}
	return temp -> token;
}


cassys_tokens_list *get_output(cassys_tokens_list *list, int transducer_id, int iteration){
	cassys_tokens_list *temp = list;

	if(list == NULL){
		return NULL;
	}

	while (temp -> output != NULL && temp -> output -> transducer_id
			<= transducer_id && temp->output->iteration <= iteration) {
		temp = temp -> output;
	}

	return temp;
}


cassys_tokens_list *get_element_at(cassys_tokens_list *list, int transducer_id, int iteration, int position){
	int current_position = 0;
	cassys_tokens_list *temp = list;

    if (temp==NULL)
        return NULL;

    while (temp -> output != NULL && temp -> output -> transducer_id
			<= transducer_id && temp->output->iteration <= iteration) {
		temp = temp -> output;
	}

 	while(current_position < position){
        if(temp->next_token != NULL)
        {
            temp=temp->next_token;

	        while (temp -> output != NULL && temp -> output -> transducer_id
			        <= transducer_id && temp->output->iteration <= iteration) {
	            temp = temp -> output;
	        }
        }
        else
        {
            return NULL;
        }

		current_position++;
	}

	return temp;
}


cassys_tokens_list *add_output(cassys_tokens_list *list,
		cassys_tokens_list *output, int previous_transducer, int previous_iteration,
		int transducer_id, int iteration,
		int number_of_tokens_replaced, int number_of_output_tokens) {
	if (list == NULL) {
		return NULL;
	}

    list ->output = output;
	cassys_tokens_list *replacement_end = get_element_at(list, previous_transducer, previous_iteration, number_of_tokens_replaced);
	cassys_tokens_list *output_end =NULL;
	if(list->output!=NULL)
		output_end = get_element_at(list->output, list->output->transducer_id, list->output->iteration, number_of_output_tokens);
	if (output_end == NULL) {
		return NULL;
	}

	output_end -> next_token = replacement_end;

	return list;
}


cassys_tokens_list *new_list(list_ustring *l_u, int transducer_id, int iteration){
	cassys_tokens_list *head = NULL;


	if(l_u!=NULL){
		head = new_element(l_u -> string, transducer_id, iteration);
		l_u=l_u->next;
	}


	cassys_tokens_list *current = head;

	while(l_u!=NULL){
		// free ajout� pour lib�rer next_token : verifier son utilit� !
		free(current->next_token);
		current -> next_token = new_element(l_u -> string, transducer_id, iteration);


		current = current ->next_token;

		list_ustring *l_u_next = l_u->next;
		l_u= l_u_next;
	}

	return head;
}

cassys_tokens_list *new_element( unichar *u, int transducer_id, int iteration){

	cassys_tokens_list *l = (cassys_tokens_list*)malloc(sizeof(cassys_tokens_list)*1);
	if(l == NULL){
		fatal_alloc_error("new_element");
		exit(1);
	}

	l->transducer_id = transducer_id;
	l->iteration = iteration;
	l->output = NULL;
	l->next_token = NULL;
	l->token=u_strdup(u);
	return l;
}

void free_cassys_tokens_list(cassys_tokens_list *l){
	while(l!=NULL){
        cassys_tokens_list *l_next_token=NULL;
		free(l->token);
		free_cassys_tokens_list(l->output);
		if(l->next_token!=NULL  && l->transducer_id == l->next_token -> transducer_id){
			l_next_token = l->next_token;
		}
		free(l);
        l=l_next_token;
	}
}

void display_text(cassys_tokens_list *l, int transducer_id, int iteration){
	u_printf("cassys_token_list = ");
	while(l!=NULL){
		u_printf("%S",l->token);
		l=next_element(l,transducer_id, iteration);
	}
	u_printf("\n");
}

} // namespace unitex




