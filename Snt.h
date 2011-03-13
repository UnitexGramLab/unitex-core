/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdio.h>
#include "File.h"

/**
 * This structure contains all the files related to a given .snt text
 * file. In order to avoid multiple definitions, none of these files
 * should be named without a structure of this kind.
 */
struct snt_files {
   char path[FILENAME_MAX];
   char dlf[FILENAME_MAX];
   char dlf_n[FILENAME_MAX];
   char dlc[FILENAME_MAX];
   char dlc_n[FILENAME_MAX];
   char err[FILENAME_MAX];
   char err_n[FILENAME_MAX];
   char tags_err[FILENAME_MAX];
   char tags_err_n[FILENAME_MAX];
   char tags_ind[FILENAME_MAX];
   char stats_n[FILENAME_MAX];
   char stat_dic_n[FILENAME_MAX];
   char text_cod[FILENAME_MAX];
   char text_fst2[FILENAME_MAX];
   char tokens_txt[FILENAME_MAX];
   char tok_by_alph_txt[FILENAME_MAX];
   char tok_by_freq_txt[FILENAME_MAX];
   char enter_pos[FILENAME_MAX];
   char snt_offsets_pos[FILENAME_MAX];
   char concord_ind[FILENAME_MAX];
   char concord_txt[FILENAME_MAX];
   char concord_html[FILENAME_MAX];
   char morpho_dic[FILENAME_MAX];
   char morpho_bin[FILENAME_MAX];
   char morpho_inf[FILENAME_MAX];
};


struct snt_files* new_snt_files(const char*);
struct snt_files* new_snt_files_from_path(const char*);
void free_snt_files(struct snt_files*);

#endif

