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

#ifndef SntH
#define SntH

#include "FileName.h"

/**
 * This structure contains all the files related to a given .snt text
 * file. In order to avoid multiple definitions, none of these files
 * should be named without a structure of this kind.
 */
struct snt_files {
   char path[FILENAME_SIZE];
   char dlf[FILENAME_SIZE];
   char dlf_n[FILENAME_SIZE];
   char dlc[FILENAME_SIZE];
   char dlc_n[FILENAME_SIZE];
   char err[FILENAME_SIZE];
   char err_n[FILENAME_SIZE];
   char stats_n[FILENAME_SIZE];
   char stat_dic_n[FILENAME_SIZE];
   char text_cod[FILENAME_SIZE];
   char tokens_txt[FILENAME_SIZE];
   char tok_by_alph_txt[FILENAME_SIZE];
   char tok_by_freq_txt[FILENAME_SIZE];
   char enter_pos[FILENAME_SIZE];
   char concord_ind[FILENAME_SIZE];
   char concord_txt[FILENAME_SIZE];
   char concord_html[FILENAME_SIZE];
};


struct snt_files* new_snt_files(char*);
struct snt_files* new_snt_files_from_path(char*);
void free_snt_files(struct snt_files*);

#endif

