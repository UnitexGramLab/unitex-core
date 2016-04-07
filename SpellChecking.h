/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef SpellCheckingH
#define SpellCheckingH

#include "Unicode.h"
#include "CompressedDic.h"
#include "DELA.h"
#include "Vector.h"
#include "Keyboard.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Here are the four main kinds of tolerated errors.
 */
typedef enum {
	SP_INSERT, 	/* happy => hazppy */
	SP_SUPPR,	/* happy => hapy */
	SP_SWAP,	/* happy => hpapy */
	SP_CHANGE	/* happy => hzppy */
} SPBasicOp;


/**
 * Here are the subtypes of tolerated errors.
 */
typedef enum {
	/* Subtypes for SP_INSERT */
	SP_INSERT_DOUBLE,	/* devil => devvil */
	SP_INSERT_DEFAULT,	/* anything else */
	/* Subtypes for SP_SUPPR */
	SP_SUPPR_DOUBLE,	/* battle => batle */
	SP_SUPPR_DEFAULT,	/* anything else */
	/* There is not subtypes for SP_SWAP, but this declaration is
	 * made to be consistent with other SPBasicOp values */
	SP_SWAP_DEFAULT,
	/* Subtypes for SP_CHANGE */
	SP_CHANGE_DIACRITIC,	/* étaient => etaient */
	SP_CHANGE_CASE,			/* London => london */
	SP_CHANGE_KEYBOARD,		/* battle => bzttle (since z is close to a on the keyboard) */
	SP_CHANGE_DEFAULT,		/* anything else */

	/* This last line is used to know the number of SPSubOp that have been defined */
	N_SPSubOp
} SPSubOp;


typedef struct {
	/* The file we read from words to check */
	U_FILE* in;
	/* The file where we print the analysis */
	U_FILE* out;
	/* Do we want to modify the input file ? (see SpellCheck usage for possible values) */
	char input_op;
	/* If we want to modify the input file, here is the tmp file pointer to use */
	U_FILE* modified_input;
	/* Number of lines printed in modified_input */
	int n_input_lines;
	/* Number of lines printed in out */
	int n_output_lines;
	/* The dictionaries to lookup in for spellchecking */
	Dictionary** dics;
	int n_dics;
	/* The keyboard to use to test whether two letters are
	 * geographically close or not */
	Keyboard* keyboard;

	/* Current and maximum number of errors tolerated in one word */
	unsigned int current_errors;
	unsigned int max_errors;
	/* Current and maximum number of errors tolerated for each kind of error */
	unsigned int current_SP_INSERT;
	unsigned int current_SP_SUPPR;
	unsigned int current_SP_SWAP;
	unsigned int current_SP_CHANGE;
	unsigned int max_SP_INSERT;
	unsigned int max_SP_SUPPR;
	unsigned int max_SP_SWAP;
	unsigned int max_SP_CHANGE;

	/* Minimum word length required to tolerate 1, 2 or 3+ errors */
	unsigned int min_length1;
	unsigned int min_length2;
	unsigned int min_length3;

	/* This array associates a score to each kind of error.
	 * The bigger the score, the more unlikely the error */
	int score[N_SPSubOp];

	/* tmp strings, to avoid multiple allocations */
	Ustring* tmp;
	Ustring* inflected;
	Ustring* output;

	/* This vector must contain pairs of integer (pos,op), where
	 * pos is the position of the event in the input word and op is the
	 * value of the SPSubOp corresponding to the given event.
	 * The meaning of pos is as follows:
	 *
	 * SP_INSERT_XXX: position of the extra char => pos=4 for happ(t)y
	 * SP_SUPPR_XXX:  position in the input word of the missing char
	 *                => pos=1 for h(a)pp
	 *                => pos=3 for hpp(y) (if we already tolerated the suppression of 'a')
	 * SP_SWAP_DEFAULT: position of the first swapped char => pos=1 for h(pa)py
	 * SP_CHANGE_XXX: position of the modified char => pos=1 for h(z)ppy
	 */
	vector_int* pairs;

	/* Do we allow SP_INSERT or SP_CHANGE involving an uppercase as the first letter
	 * of the word to analyze? */
	int allow_uppercase_initial;
} SpellCheckConfig;

extern int default_scores[N_SPSubOp];

void spellcheck(SpellCheckConfig*);
void init_scores(int* score,int s1,int s2,int s3,int s4,int s5,int s6,int s7,int s8,int s9);

} // namespace unitex

#endif

