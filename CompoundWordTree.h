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

#ifndef CompoundWordTreeH
#define CompoundWordTreeH

#include "Unicode.h"
#include "Alphabet.h"
#include "String_hash.h"
#include "List_int.h"
#include "LocateConstants.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define NO_CASE_VARIANT_IS_ALLOWED 0
#define ALL_CASE_VARIANTS_ARE_ALLOWED 1
#define MAX_TOKEN_IN_A_COMPOUND_WORD 256

#define BEGIN_CASE_VARIANT_LIST -3
#define END_CASE_VARIANT_LIST -5
#define END_TOKEN_LIST -1

/* Generic pattern used to indicate that a compound word is matchable with <DIC> */
#define COMPOUND_WORD_PATTERN -555

/**
 * This structure represents a node in a compound word tree. It is used
 * by the locate functions to build a tree of all the compound words
 * in order to speed up the consultation. In fact, this tree is a token tree:
 * the branches are tagged by the numbers of the tokens that constitute
 * compound words. In a first step, the tree is built with transition lists.
 * In a second step, these lists are replaced by sorted arrays that 1) avoid
 * duplicates and 2) allow dichotomy searches to speed up the process. Each
 * node has two array: one for the token numbers ('destination_tokens') and
 * one for the nodes pointed out by the transitions ('destination_nodes'). The
 * set of transitions for a given token t is then represented by the couple:
 *
 * (destination_tokens[k] , destination_nodes[k])
 *
 * where destination_tokens[k] == t. These two arrays have been used to avoid
 * an array of structure. This is a trick to optimize dichotomy searches, because
 * testing destination_tokens[k] is quicker than testing t[k]->token.
 *
 * For instance, if we have a transition tagged by 'the' that goes to the node 47
 * and another tagged by "The" that goes to the node 129, and if the text
 * contains the tokens "the", "The" and "THE" the original transition list is:
 *
 * ("the" , 47) -> ("The" , 129) -> NULL
 *
 * Once we have replaced tokens by all their case equivalents, we obtain the following
 * arrays:
 *
 * destination_tokens   destination_nodes
 * THE                     47,129
 * The                     47,129
 * the                     47
 */
struct DLC_tree_node {
    /*
     * to allow duplicate pointer to a DLC_tree_node, we count reference
     * use increment_reference_DLC_tree_node when a pointer is duplicated
     * use decrement_reference_DLC_tree_node when a pointer is removed
     *   (this free_DLC_tree_node)
     */

    unsigned int count_reference;
    /*
     * 'patterns' is the list of the numbers of all the patterns that
     * can match the compound words corresponding to this node.
     */
    struct list_int* patterns;
    /*
     * 'number_of_patterns' is the length of the list 'patterns'
     */
    int number_of_patterns;
    /*
     * For optimization reasons, the list 'patterns' is replaced
     * by the sorted array 'array_of_patterns' before being used
     * in the locate.
     */
    int* array_of_patterns;
    /*
     * 'transitions' is the list of transitions that outgo from
     * this node. NOTE: transitions are supposed to be sorted
     * by token numbers.
     */
    struct DLC_tree_transition* transitions;
    /*
     * WARNING: 'number_of_transitions' is NOT the length of the list
    *          'transitions', but the size of the 'destination_tokens'
    *          and 'destination_nodes' arrays.
     */
    int number_of_transitions;
    /*
     * Sorted array of tokens, including case equivalents. See comment above.
     */
    int* destination_tokens;
    /*
     * Sorted array of nodes. See comment above.
     */
    struct DLC_tree_node** destination_nodes;
};


/**
 * We define an IntSequence as an array containing sorted >=0 integers
 * and ended by -1.
 */
typedef int* IntSequence;


/**
 * This structure represent a list of transitions in a compound word tree.
 */
struct DLC_tree_transition {
    /* The number of the token */
    IntSequence token_sequence;
    /* The destination node */
    struct DLC_tree_node* node;
    /* The following element of the list */
    struct DLC_tree_transition* next;
};


/**
 * This structure contains the root of a compound word tree, and
 * an array that is an index for accessing quickly to the compound
 * words that begin by a given token.
 */
struct DLC_tree_info {
    struct DLC_tree_node* root;
    struct DLC_tree_node** index;
};


struct DLC_tree_info* new_DLC_tree(int);
void free_DLC_tree(struct DLC_tree_info*);
void tokenize_compound_word(const unichar*,int*,const Alphabet*,struct string_hash*,TokenizationPolicy);
void add_compound_word_with_no_pattern(const unichar*,const Alphabet*,struct string_hash*,struct DLC_tree_info*,TokenizationPolicy);
void add_compound_word_with_pattern(const unichar*,int,const Alphabet*,struct string_hash*,struct DLC_tree_info*,TokenizationPolicy);
int conditional_insertion_in_DLC_tree(const unichar*,int,int,const Alphabet*,struct string_hash*,struct DLC_tree_info*,TokenizationPolicy);
void optimize_DLC(struct DLC_tree_info*);

} // namespace unitex

#endif
