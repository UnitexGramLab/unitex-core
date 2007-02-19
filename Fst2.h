 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef Fst2H
#define Fst2H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unicode.h"
#include "TransductionVariables.h"
#include "MetaSymbols.h"
#include "Pattern.h"
#include "List_int.h"


/* Maximum number of tags in a .fst2 */
#define MAX_FST2_TAGS 100000

/* Maximum number of states in a .fst2 */
#define MAX_FST2_STATES 500000

/* Used when no compound pattern is defined for a given Fst2Tag */
#define NO_COMPOUND_PATTERN -1

/* These bit masks are used when a fst2 is loaded */
#define TRANSDUCTION_TAG_BIT_MASK 1
#define NEGATION_TAG_BIT_MASK 2
#define RESPECT_CASE_TAG_BIT_MASK 4 /* to 1 if case variants are not allowed */

/**
 * Here we define the different kinds of tag.
 */
enum tag_type {
   UNDEFINED_TAG, // used at initialization of a tag
   META_TAG,      // <MOT>, <MIN>, etc.
   PATTERN_TAG,   // <be.V>
   PATTERN_NUMBER_TAG, // used when patterns have been numbered
   TOKEN_LIST_TAG,  // used when the tag matches a list of tokens. This will
                    // happen when the tag contains a single token or a pattern
                    // with a lemma (<be.V>). In this last case, the pattern will
                    // be replaced for optimization reasons by the exhaustive
                    // list of tokens that can be matched
   BEGIN_VAR_TAG, // $a(
   END_VAR_TAG,   // $a)
   BEGIN_POSITIVE_CONTEXT_TAG, // $[
   BEGIN_NEGATIVE_CONTEXT_TAG, // $![
   END_CONTEXT_TAG             // $]
};



/**
 * This structure represents a tag of a .fst2 file.
 */
struct fst2Tag {
	/* Field used to indicate the nature of the tag */
   enum tag_type type;
	
	/* This control byte is used to set information about the tag with
	 * bit masks. The meaning of these tags can be found in LocateConstants.h
	 * 
	 * This field is also used in the Grf2Fst2 program in order to mark the
	 * tags that can match the empty word <E>.
	 */
	unsigned char control;
	
	/*
	 * 'input' represents the input part of a tag, that is to say without
	 * its morphological filter and output if any.
	 * Example: "<V:P><<^in>>/[V]" => input="<V:P>"
	 * 
	 * NOTE: if the input only contains a morphological filter like "<<^in>>",
	 *       the default sequence "<TOKEN>" will be copied in the 'input' field.
	 */
	unichar* input;
	
   /*
    * If a tag contains a morphological filter, it is copied into this field 
    * at the loading of the fst2. It is setted to NULL if there is
    * no morphological filter.
    */
   unichar* morphological_filter;

   /*
    * When a fst2 is used by the Locate program, all morphological filters are
    * compiled into automata in the MorphologicalFilters library. This field is used
    * to store the number of the morphological filter of the tag, if any. It is setted 
    * to -1 if there is no morphological filter. These numbers are global so two tags
    * can share the same filter number if their filters are identical (for instance
    * "<A><<^in>>" and "<N><<^in>>/NOUN").
    */
   int filter_number;
	
   /*
    * 'output' represents the output part of a tag, without the '/' separator, or
    * NULL if the tag contains no output.
    */
   unichar* output;

   /**
    * A tag can be a meta symbol like <MOT>, a pattern
    * or a variable. Note that a pattern is, in a first step, represented as
    * a set of constraints (struct pattern*) and, in a second step,
    * it is coded by a pattern number.
    */
    enum meta_symbol meta;
    struct pattern* pattern;
    int pattern_number;
    unichar* variable;
   
   /*
	 * This field represents the list of the numbers of the tokens that this tag
	 * can match.
	 */
	struct list_int* matching_tokens;
	
	/*
	 * If the tag can match one or several compound words, a compound pattern is
	 * created, and this field is used to store the number of this compound
	 * pattern. Note that if a tag ("<Einstein>") can match both simple ("Einstein")
	 * and compound ("Albert Einstein") words, simple words will be handled as
	 * tokens, and compound words will be handled via a compound pattern.
	 */
	int compound_pattern;
};
typedef struct fst2Tag* Fst2Tag;


/*
 * This structure represents a state of a .fst2 file.
 */
struct fst2State {
	/* This control byte is used to set information about the state with
	 * bit masks. The two lowest bits are reserved to mark initial and final
	 * states. This field can also be used to mark states when exploring a fst2. For
	 * instance, it is used for cycle detection in the Grf2Fst2 program.
	 */
	unsigned char control;
	
	/* Transitions outgoing from this state */
	struct fst2Transition* transitions;
};
typedef struct fst2State* Fst2State;


/*
 * This structure represents a transition list in a fst2
 */
struct fst2Transition {
	/* Number of the transition tag */
	int tag_number;
	
	/*
	 * Number of the state pointed by the transition. Note that this
	 * number is ABSOLUTE. For instance, if the subgraph number 3 starts
	 * at the state number 45, the 6th state of this subgraph will have the
	 * number 45+6=51.
	 * 
	 */
	int state_number;
	
	/* Next transition of the list */
	struct fst2Transition* next;
};
typedef struct fst2Transition* Fst2Transition;


/*
 * This structure represents a list of graph variables corresponding to
 * declarations like $a( or $a)
 * 
 * This structure is declared here instead of in TransductionVariables, 
 * in order to minimize library dependancies, because many programs
 * that use the Fst2 library do not use variables.
 */
struct variable_list {
	/* Name of the variable */
	unichar* name;
	
	/* Starting position of the variable in the text, -1 if the starting position
	 * of the variable has not been defined */
	int start;
	
	/* Ending position of the variable in the text, -1 if the ending position
	 * of the variable has not been defined */
	int end;
	
	/* Next variable in the list */
	struct variable_list* next;
};


/*
 * This structure represent a fst2. 
 */
struct fst2 {
	/* Array that contains all the states of the fst2 */
    Fst2State* states;
    
    /* Array that contains all the tags of the fst2 */
    Fst2Tag* tags;
    
    /* Number of graphs contained in the fst2 */
    int number_of_graphs;
    
    /* Number of states contained in the fst2 */
    int number_of_states;
    
    /* Number of tags of the fst2 */
    int number_of_tags;
    
    /* Array that indicates for each graph the number of its initial state */
    int* initial_states;
    
    /*
     * This array is used to store the graph names. We use the type unichar and
     * not char, because this array is also used to store sentences, when the fst2
     * represents a text automaton.
     */
    unichar** graph_names;
    
    /* This array indicates for each graph its number of states */
    int* number_of_states_per_graphs;
    
    /* List of variables used in the graph. This list is initialized from
     * the $a( and $a) deaclarations found in the tags. */
    struct variable_list* variables;
};
typedef struct fst2 Fst2;


Fst2Transition new_Fst2Transition(int,int);
void free_Fst2Transition(Fst2Transition);


/* Functions for loading grammars */
Fst2* load_fst2(char*,int);
Fst2* load_one_sentence_from_fst2(char*,int);
void free_Fst2(Fst2*);

/* Functions for writing grammars */
void write_fst2_graph(FILE*,Fst2*,int);
void write_fst2_tags(FILE*,Fst2*);


struct variable_list* get_variable(unichar*,struct variable_list*);
int is_initial_state(Fst2State);
int is_final_state(Fst2State);

#endif
