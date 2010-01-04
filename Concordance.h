 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef ConcordanceH
#define ConcordanceH

#include "Text_tokens.h"


#define TEXT_ORDER 0
#define LEFT_CENTER 1
#define LEFT_RIGHT 2
#define CENTER_LEFT 3
#define CENTER_RIGHT 4
#define RIGHT_LEFT 5
#define RIGHT_CENTER 6
#define MAX_CONTEXT_IN_UNITS 5000


#define HTML_ 0
#define TEXT_ 1
#define GLOSSANET_ 2
#define INDEX_ 3
#define AXIS_ 4
#define XALIGN_ 5
/* UIMA: begin & end positions in chars in the txt file, ignoring {S} */
#define UIMA_ 6
#define MERGE_ 7
#define SCRIPT_ 8
#define XML_ 9
#define XML_WITH_HEADER_ 10

/**
 * This structure is used to store information about the current
 * concordance build. It is used to avoid giving too much parameters
 * to functions.
 */
struct conc_opt {
  int sort_mode;
  int left_context;
  int right_context;
  unsigned char left_context_until_eos;
  unsigned char right_context_until_eos;
  int thai_mode;
  char* fontname;
  int fontsize;
  int result_mode;
  char output[FILENAME_MAX];
  char* script;
  char* sort_alphabet;
  char working_directory[FILENAME_MAX];
};

struct conc_opt* new_conc_opt();
void free_conc_opt(struct conc_opt*);

void create_concordance(Encoding encoding_output,int bom_output,U_FILE*,U_FILE*,struct text_tokens*,
                        int,int*,struct conc_opt*);


#endif
