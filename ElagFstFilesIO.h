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

#ifndef ElagFstFilesIOH
#define ElagFstFilesIOH

#include "LanguageDefinition.h"
#include "String_hash.h"
#include "Fst2Automaton.h"
#include "Tfst.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/* for symbol -> label translation */

enum { FST_TEXT = 0, FST_GRAMMAR, FST_LOCATE, FST_BAD_TYPE };


/**
 * This structure is used to load a .fst2
 */
typedef struct fst_file_in_t {
   /* File name */
   char* name;

   /* The file itself */
   U_FILE* f;

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
} Elag_fst_file_in;


/**
 * This structure is used to load a .tfst
 */
typedef struct tfst_file_in_t {

   /* The .tfst */
   Tfst* tfst;

   /* The language definition for the tfst's language */
   language_t* language;

} Elag_Tfst_file_in;



/**
 * This structure is used to save a .fst2
 */
typedef struct fst_file_out_t {
   /* File name */
   char* name;

   /* The file itself */
   U_FILE* f;

   /* The start position in the Unicode file, after the byte-order mark,
    * if any. */
   long fstart;

   /* The kind of .fst2: TEXT | GRAMMAR */
   int type;

   /* The number of automata in the .fst2 */
   int nb_automata;

   /* The tags of the .fst2 */
   struct string_hash* labels;
} Elag_fst_file_out;


Elag_Tfst_file_in* load_tfst_file(const VersatileEncodingConfig*,const char*,language_t*);
void load_tfst_sentence_automaton(Elag_Tfst_file_in*,int);
void tfst_file_close_in(Elag_Tfst_file_in*);

Elag_fst_file_in* load_fst_file(const VersatileEncodingConfig*,const char*,int,language_t*);
void fst_file_close_in(Elag_fst_file_in*);

void fst_file_seek(Elag_fst_file_in*,int);
inline void fst_file_rewind(Elag_fst_file_in* fstin) { fst_file_seek(fstin, 1); }

Fst2Automaton* load_automaton(Elag_fst_file_in*);
Fst2Automaton* fst_file_autalmot_load(Elag_fst_file_in*,int);
Fst2Automaton* load_elag_grammar_automaton(const VersatileEncodingConfig*,const char* fst2,language_t*);

Elag_fst_file_out* fst_file_out_open(const VersatileEncodingConfig*,const char*,int);
void fst_file_close_out(Elag_fst_file_out * fstout);

void fst_file_write(Elag_fst_file_out * fstf, const Fst2Automaton * A);

} // namespace unitex

#endif
