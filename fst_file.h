 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef _FST_FILE_H_
#define _FST_FILE_H_

#include "LanguageDefinition.h"
#include "String_hash.h"
#include "autalmot.h"



/* for symbol -> label translation */

enum { FST_TEXT = 0, FST_GRAMMAR, FST_LOCATE, FST_BAD_TYPE };


/**
 * This structure is used to load a .fst2
 */
typedef struct fst_file_in_t {
   /* File name */
   char* name;
   
   /* The file itself */
   FILE* f;
   
   /* The language definition for the fst2's language */
   language_t* language;
   
   /* The kind of .fst2 (text, grammar,...) */
   int type;
   
   /* Offset for the first automaton (after headers) */
   int pos0;
   
   /* The number of automata contained in the .fst2 */
   int nb_automata;
   
   /* The number of automata that have already been read from the
    * .fst2 file */
   int pos;

   /* This structure will contain all the symbols of the grammar */
   struct string_hash_ptr* symbols;
   
   /* This array is used to renumber tags that can have been made
    * equivalent. For instance, when we load a text automaton, we can
    * have different tags like "<comme,.CONJS+5>" and "<comme,.CONJS+8>".
    * However, these tags will both be taken by TagsetNormFst2 as
    * "<comme,.CONJS>", since semantic value "5" and "8" are not defined
    * in the tagset file. So, at the time of loading the text .fst2,
    * we will really load "<comme,.CONJS>" into 'symbols' and we will
    * set renumber[i]=renumber[j]=x where i and j are the tag numbers for
    * "<comme,.CONJS+5>" and "<comme,.CONJS+8>" in the original .fst2,
    * and x is the index of "<comme,.CONJS>" in 'symbols->value'. */
   int* renumber;
} fst_file_in_t;


/**
 * This structure is used to save a .fst2
 */
typedef struct fst_file_out_t {
   /* File name */
   char* name;
   
   /* The file itself */
   FILE* f;

   /* The start position in the Unicode file, after the byte-order mark,
    * if any. */
   long fstart;
   
   /* The kind of .fst2: TEXT | GRAMMAR */
   int type;
   
   /* The number of automata in the .fst2 */
   int nb_automata;
   
   /* The tags of the .fst2 */
   struct string_hash* labels;
} fst_file_out_t;


fst_file_in_t* load_fst_file(char*,int,language_t*);
void fst_file_close_in(fst_file_in_t*);

void fst_file_seek(fst_file_in_t*,int);
inline void fst_file_rewind(fst_file_in_t* fstin) { fst_file_seek(fstin, 1); }

Fst2Automaton* load_automaton(fst_file_in_t*);
Fst2Automaton* fst_file_autalmot_load(fst_file_in_t*,int);


fst_file_out_t* fst_file_out_open(char*,int);
void fst_file_close_out(fst_file_out_t * fstout);

void fst_file_write(fst_file_out_t * fstf, const Fst2Automaton * A);

#endif
