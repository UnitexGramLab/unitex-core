/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


cassys_tokens_list *next_element(cassys_tokens_list *list, int transducer_id){
	if(list->next_token == NULL){
		return NULL;
	}

	cassys_tokens_list *temp = list->next_token;
	temp = get_output(temp,transducer_id);

	return temp;
}


unichar *next_token(cassys_tokens_list *list, int transducer_id){
	cassys_tokens_list *temp = next_element(list,transducer_id);

	if(temp == NULL){
		return NULL;
	}
	return temp -> token;
}


cassys_tokens_list *get_output(cassys_tokens_list *list, int transducer_id){
	cassys_tokens_list *temp = list;

	if(list == NULL){
		return NULL;
	}

	while (temp -> output != NULL && temp -> output -> transducer_id
			<= transducer_id) {
		temp = temp -> output;
	}

	return temp;
}


cassys_tokens_list *get_element_at(cassys_tokens_list *list, int transducer_id, int position){
	int current_position = 0;
	cassys_tokens_list *temp = list;
    if (temp==NULL)
        return NULL;

	while(current_position < position){
        if(temp->next_token != NULL)
        {
            temp=temp->next_token;

	        while (temp -> output != NULL && temp -> output -> transducer_id
			        <= transducer_id) {
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
		cassys_tokens_list *output, int transducer_id,
		int number_of_tokens_replaced, int number_of_output_tokens) {
	if (list == NULL) {
		return NULL;
	}
	list ->output = output;

	cassys_tokens_list *replacement_end = get_element_at(list, transducer_id,
			number_of_tokens_replaced);

	cassys_tokens_list *output_end =NULL;
	if(list->output!=NULL)
		output_end = get_element_at(list->output,
			list->output->transducer_id, number_of_output_tokens);

	if (output_end == NULL) {
		return NULL;
	}

	output_end -> next_token = replacement_end;

	return list;
}


cassys_tokens_list *new_list(list_ustring *l_u, int transducer_id){
	cassys_tokens_list *head = NULL;


	if(l_u!=NULL){
		head = new_element(l_u -> string, transducer_id);
		l_u=l_u->next;
	}

	cassys_tokens_list *current = head;

	while(l_u!=NULL){
		current -> next_token = new_element(l_u -> string, transducer_id);


		current = current ->next_token;

		list_ustring *l_u_next = l_u->next;
		l_u= l_u_next;
	}

	return head;
}

cassys_tokens_list *new_element( unichar *u, int transducer_id){

	cassys_tokens_list *l = (cassys_tokens_list*)malloc(sizeof(cassys_tokens_list)*1);
	if(l == NULL){
		perror("malloc\n");
		fprintf(stderr,"Impossible d'allouer de la mémoire\n");
		exit(1);
	}

	l->transducer_id = transducer_id;
	l->output = NULL;
	l->next_token = NULL;

	int token_size = u_strlen(u);
	l->token = (unichar*)malloc(sizeof(unichar)*(token_size+1));
	if(l->token == NULL){
		perror("malloc\n");
		fprintf(stderr,"Impossible to allocate memory\n");
		exit(1);
	}

	u_strcpy(l->token,u);

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

void display_text(cassys_tokens_list *l, int transducer_id){
	u_printf("cassys_token_list = ");
	while(l!=NULL){
		u_printf("%S",l->token);
		l=next_element(l,transducer_id);
	}
	u_printf("\n");
}





