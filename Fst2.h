 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//---------------------------------------------------------------------------
#ifndef Fst2H
#define Fst2H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unicode.h"


/* Maximum number of tags in a .fst2 */
#define MAX_FST2_TAGS 100000

/* Maximum number of states in a .fst2 */
#define MAX_FST2_STATES 500000

#define FST2_FINAL_STATE_BIT_MASK 1
#define FST2_INITIAL_STATE_BIT_MASK 2



extern int etiquette_courante;


/**
 * This structure represents a tag of a .fst2 file.
 */
struct fst2Tag {
	/* number of the tag in the .fst2 */
	int number;
	
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
	 * 'output' represents the input part of a tag, without the '/' separator.
	 * If a tag contains no transduction, this field is supposed not to be NULL
	 * but to contain an empty string just made of '\0'.
	 */
	unichar* output;
	
	/*
	 * When the input of a tag is of the form "<built,build.V>" or "{built,build.V:K}",
	 * this field represents the inflected part of the input (here "built"). If this
	 * field is not relevant for a tag, it is setted to NULL.
	 */
	unichar* inflected;
	
	/*
	 * When the input of a tag is of the form "<build.V>", "<build>", "<built,build.V>"
	 * or "{built,build.V:K}", this field represents the lemma part of the input (here 
	 * "build"). If this field is not relevant for a tag, it is setted to NULL.
	 */
	unichar* lemma;
	
	/*
	 * When the input of a tag is of the form "<build.V+t:P1s:P2s>", "<V+t:P1s:P2s>",
	 * "<build,build.V+t:P1s:P2s>" or "{build,build.V+t:P1s:P2s}", this field
	 * represents the sequences of grammatical, semantic and inflectional codes of 
	 * the input (here "V+t:P1s:P2s"). If this field is not relevant for a tag,
	 * it is setted to NULL.
	 */
	unichar* codes;
	
	/*
	 * This field represents the list of the numbers of the tokens that this tag
	 * can match.
	 */
	struct liste_nombres* matching_tokens;
	
	/*
	 * 'number_of_matching_tokens' is the length of the list 'matching_tokens'. It
	 * is cached for efficiency reasons.
	 */
	int number_of_matching_tokens;
	
	/*
	 * If the tag can match one or several compound words, a compound pattern is
	 * created, and this field is used to store the number of this compound
	 * pattern. Note that if a tag ("<Einstein>") can match both simple ("Einstein")
	 * and compound ("Albert Einstein") words, simple words will be handled as
	 * tokens, and compound words will be handled via a compound pattern.
	 */
	int compound_pattern;
	
	/* $CD$ begin */
	/*
	 * If a tag contains a morphological filter, it is copied into this field 
	 * at the loading of the fst2. It is setted to NULL if there is
	 * no morphological filter.
	 */
	unichar* contentGF;
	/*
	 * When a fst2 is used by the Locate program, all morphological filters are
	 * compiled into automata in the GF_lib library. This field is used
	 * to store the number of the morphological filter of the tag, if any. It is setted 
	 * to -1 if there is no morphological filter. These numbers are global so two tags
	 * can share the same filter number if their filters are identical (for instance
	 * "<A><<^in>>" and "<N><<^in>>/NOUN").
	 */
	int entryMasterGF;
	/* $CD$ end   */
};
typedef struct fst2Tag* Fst2Tag;


/*
 * This structure represents a state of a .fst2 file.
 */
struct fst2State {
	/* This control byte is used to set information about the state with
	 * bit masks (FST2_FINAL_STATE_BIT_MASK and FST2_INITIAL_STATE_BIT_MASK).
	 * This field can also be used to mark states when exploring a fst2. For
	 * instance, it is used for cycle detection in the Grf2Fst2 program.
	 */
	unsigned char control;
	
	/*
	 * Transitions outgoing from this state.
	 */
	struct transition_fst *transitions;
};
typedef struct fst2State* Fst2State;


struct transition_fst {
  int etiquette;                // etiquette de la transition : un entier
  int arr;                      // etat d'arrivee de la transition
  struct transition_fst *suivant;   // transition suivante
};

typedef struct transition_fst *liste_transition;


struct variable_list {
  unichar* name;
  int start;
  int end;
  struct variable_list* suivant;
};


struct automate_fst2 {
    Fst2State* etat;
    Fst2Tag* etiquette;
    int nombre_graphes;
    int nombre_etats;
    int nombre_etiquettes;
    int* debut_graphe_fst2;
    unichar** nom_graphe;
    int* nombre_etats_par_grf;
    struct variable_list* variables;
};

typedef struct automate_fst2 Automate_fst2;


//----------PROTOTYPES-------------------------------------------
void charger_graphe_fst2(FILE*,Fst2State[],Fst2Tag[],int*,int*,int*,int**,
                         unichar***,int,int**);

liste_transition nouvelle_transition_mat();
Automate_fst2* load_fst2(char*,int);
void free_fst2(Automate_fst2*);
struct variable_list* get_variable(unichar*,struct variable_list*);
Automate_fst2* load_one_sentence_of_fst2(char*,int,FILE*);
int is_final_state(Fst2State);
void unprotect_characters_in_fst2_tags(Automate_fst2*);
void free_transition(struct transition_fst*);
#endif
