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

#ifndef _FST_FILE_H_
#define _FST_FILE_H_

#include "language.h"
#include "hash_str_table.h"
#include "autalmot.h"



/* for symbol -> label translation */

enum { FST_TEXT = 0, FST_TEXT_IMPLOSED, FST_GRAMMAR, FST_LOCATE, FST_BAD_TYPE };


typedef struct fst_file_in_t {

  char * name;
  FILE * f;

  language_t * lang;
  int type;  /* TEXT | GRAMMAR */
  int pos0;  /* offset for the first automaton (after headers) */

  int nbelems;
  int pos;

  hash_str_table_t * symbols; /* table on symbole_t * */

} fst_file_in_t;



typedef struct fst_file_out_t {

  char * name;
  FILE * f;

  long fstart;  // start of unicode file

  int type;   /* TEXT | GRAMMAR */

  int nbelems;

  hash_str_table_t * labels; // table on unichar *

} fst_file_out_t;


fst_file_in_t * fst_file_in_open(char * fname, int type, language_t * lang = LANG);
void fst_file_close(fst_file_in_t * fstf);

void fst_file_seek(fst_file_in_t * fstin, int pos);
inline void fst_file_rewind(fst_file_in_t * fstin) { fst_file_seek(fstin, 0); }

autalmot_t * fst_file_autalmot_load_next(fst_file_in_t * fstf);
autalmot_t * fst_file_autalmot_load(fst_file_in_t * fstf, int no);


fst_file_out_t * fst_file_out_open(char * fname, int type);
void fst_file_close(fst_file_out_t * fstout);

void fst_file_write(fst_file_out_t * fstf, const autalmot_t * A);

#endif
