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

#ifndef CASSYS_TOKENS_H_
#define CASSYS_TOKENS_H_

#include "List_ustring.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

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

} // namespace unitex

#endif /* CASSYS_TOKENS_H_ */
