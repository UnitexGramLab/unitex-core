/*
 * Cassys_tokens.h
 *
 *  Created on: 29 avr. 2010
 *      Author: dnott
 */

#ifndef CASSYS_TOKENS_H_
#define CASSYS_TOKENS_H_

#include "List_ustring.h"

/**
 * \struct cassys_tokens_list
 * \brief A sparse_matrix like structure to represent a text at any state of the cascade.
 *
 *
 */
typedef struct cassys_tokens_list{
	/**
	 * Current token of the text
	 */
	unichar *token;

	/**
	 * The label of the transducer that added the token to the text. The id 0 means token from the initial text.
	 */
	int transducer_id;

	/**
	 * The next token of the text.
	 */
	struct cassys_tokens_list *next_token;

	/**
	 * If not NULL, beginning of an output found by a transducer.
	 */
	struct cassys_tokens_list *output;
}cassys_tokens_list;

/**
 * @brief returns the next elements of a list cassys_token_list
 *
 * @param list the list
 * @param transducer_id the transducer_id
 *
 * @return the next element or NULL if no next element exists
 */
cassys_tokens_list *next_element(cassys_tokens_list *list, int transducer_id);

unichar *next_token(cassys_tokens_list *list, int transducer_id);

cassys_tokens_list *get_element_at(cassys_tokens_list *list, int transducer_id, int position);
cassys_tokens_list *new_list(list_ustring *u, int transducer_id);
cassys_tokens_list *new_element(unichar *u, int transducer_id);
cassys_tokens_list *add_output(cassys_tokens_list *list,
cassys_tokens_list *output, int transducer_id,int number_of_tokens_replaced, int number_of_output_tokens);
void free_cassys_tokens_list(cassys_tokens_list *l);
void display_text(cassys_tokens_list *l, int transducer_id);
cassys_tokens_list *get_output(cassys_tokens_list *list, int transducer_id);

#endif /* CASSYS_TOKENS_H_ */
