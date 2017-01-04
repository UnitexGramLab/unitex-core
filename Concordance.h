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

#ifndef ConcordanceH
#define ConcordanceH

#include "Text_tokens.h"
#include "Offsets.h"
#include "PRLG.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

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
#define DIFF_ 11
#define LEMMATIZE_ 12
#define CSV_ 13


/**
 * This structure is used to store information about the current
 * concordance build. It is used to avoid giving too much parameters
 * to functions.
 */
struct conc_opt {
  int only_ambiguous;
  int sort_mode;
  int left_context;
  int right_context;
  unsigned char left_context_until_eos;
  unsigned char right_context_until_eos;
  int thai_mode;
  int convLFtoCRLF;
  char* fontname;
  int fontsize;
  int result_mode;
  char output[FILENAME_MAX];
  char* script;
  char* sort_alphabet;
  char working_directory[FILENAME_MAX];
  /* snt_offsets is used to compute correct positions in .snt file
   * for coherence with highlighting in GUI */
  vector_int* snt_offsets;
  /* uima_offsets is used with --uima option */
  vector_uima_offset* uima_offsets;
  /* Data extracted from the PRLG file produced by Unxmlize's --PRLG option */
  PRLG* PRLG_data;
  char only_matches;
  char original_file_offsets;
  char input_offsets[FILENAME_MAX];
  char output_offsets[FILENAME_MAX];
};

struct conc_opt* new_conc_opt();
void free_conc_opt(struct conc_opt*);

int create_concordance(const VersatileEncodingConfig*,U_FILE*,ABSTRACTMAPFILE*,struct text_tokens*,
                        int,int*,struct conc_opt*);

} // namespace unitex

#endif
