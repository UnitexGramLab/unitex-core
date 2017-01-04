/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Vector.h"
#include "Text_tokens.h"

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
     * The iteration that added the token to the text
     */
    int iteration;

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
* \struct cassys_tokens_allocation_tool
* \brief A set of data used by cassys allocator to help free data
*
*/

typedef struct cassys_tokens_allocation_tool_item{
    cassys_tokens_list* tokens_list_item;
    struct cassys_tokens_allocation_tool_item* next;
}cassys_tokens_allocation_tool_item;

typedef struct cassys_tokens_allocation_tool{
    struct cassys_tokens_allocation_tool_item* first_item;
    Abstract_allocator allocator_tokens_list;
    int must_free_tokens_list;
} cassys_tokens_allocation_tool;

/**
 * \brief build structure needed for create cassys_token
 *
 * \param[in] token_txt_name name of the file containing the tokens of the text
 */
cassys_tokens_allocation_tool *build_cassys_tokens_allocation_tool();

/**
 * \brief free structure needed for create cassys_token and all token created
 *
 * \param[in] allocation_tool the structure to free
 */
void free_cassys_tokens_allocation_tool(cassys_tokens_allocation_tool * allocation_tool);

/**
 * \brief Constructs the text representation in the cassys_tokens_list
 *
 * \param[in] token_txt_name name of the file containing the tokens of the text
 * \param[in] text_cod_name name of the containing the sequence of token representing the text
 * \param[out] tokens structure containing the token present in the text
 *
 * Each element of the cassys_tokens_list token field is pointing on a element of tokens.
 */
cassys_tokens_list *cassys_load_text(const VersatileEncodingConfig*,const char *token_text_name, const char * text_cod_name,
         struct text_tokens **tokens, const vector_int* uima_offset, cassys_tokens_allocation_tool * allocation_tool);

cassys_tokens_list *add_replaced_text(const char *text, cassys_tokens_list *list, int previous_transducer, int previous_iteration,
         int transducer_id, int iteration, const char *alphabet, const VersatileEncodingConfig*, cassys_tokens_allocation_tool * allocation_tool);

//void free_cassys_tokens_list(cassys_tokens_list *l);



/**
 * @brief returns the next elements of a list cassys_token_list
 *
 * @param list the list
 * @param transducer_id the transducer_id
 *
 * @return the next element or NULL if no next element exists
 */
cassys_tokens_list *next_element(cassys_tokens_list *list, int transducer_id, int iteration);

const unichar *next_token(cassys_tokens_list *list, int transducer_id, int iteration);

cassys_tokens_list *get_element_at(cassys_tokens_list *list, int transducer_id, int iteration, int position);
cassys_tokens_list *new_list(list_ustring *u, int transducer_id, int iteration, cassys_tokens_allocation_tool * allocation_tool);
cassys_tokens_list *new_element(const unichar *u, int transducer_id, int iteration, cassys_tokens_allocation_tool * allocation_tool);
cassys_tokens_list *add_output(cassys_tokens_list *list,
cassys_tokens_list *output, int previous_transducer, int previous_iteration, int transducer_id, int iteration, int number_of_tokens_replaced, int number_of_output_tokens);

void display_text(cassys_tokens_list *l, int transducer_id, int iteration);
cassys_tokens_list *get_output(cassys_tokens_list *list, int transducer_id, int iteration);

void cassys_tokens_2_graph(cassys_tokens_list *c, const char *fileName, int realignPtrToBase);
void cassys_tokens_2_graph_subgraph(cassys_tokens_list *c, U_FILE *u, int realignPtrToBase);
void cassys_tokens_2_graph_walk_for_subgraph(cassys_tokens_list *c, U_FILE *u, cassys_tokens_list *predecessor, int realignPtrToBase);
unichar *protect_from_record(const unichar *u);
unichar *protect_quote(const unichar *u);
unichar *unprotect_lexical_tags(unichar *u);
void cassys_tokens_2_graph_same_ranked(cassys_tokens_list *c, U_FILE *u);

} // namespace unitex

#endif /* CASSYS_TOKENS_H_ */
