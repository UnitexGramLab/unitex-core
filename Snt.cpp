/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Snt.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Allocates, initializes and returns a structure that contains the names
 * all the files related to a text in a given path. This path is supposed
 * to correspond to a "..._snt" directory.
 */
struct snt_files* new_snt_files_from_path(const char* path) {
struct snt_files* snt_files=(struct snt_files*)malloc(sizeof(struct snt_files));
if (snt_files==NULL) {
    fatal_alloc_error("new_snt_files_from_path");
}
strcpy(snt_files->path,path);
new_file(path,"dlf",snt_files->dlf);
new_file(path,"dlf.n",snt_files->dlf_n);
new_file(path,"dlc",snt_files->dlc);
new_file(path,"dlc.n",snt_files->dlc_n);
new_file(path,"err",snt_files->err);
new_file(path,"err.n",snt_files->err_n);
new_file(path,"tags_err",snt_files->tags_err);
new_file(path,"tags_err.n",snt_files->tags_err_n);
new_file(path,"tags.ind",snt_files->tags_ind);
new_file(path,"stats.n",snt_files->stats_n);
new_file(path,"stat_dic.n",snt_files->stat_dic_n);
new_file(path,"text.cod",snt_files->text_cod);
new_file(path,"text.fst2",snt_files->text_fst2);
new_file(path,"tokens.txt",snt_files->tokens_txt);
new_file(path,"tok_by_alph.txt",snt_files->tok_by_alph_txt);
new_file(path,"tok_by_freq.txt",snt_files->tok_by_freq_txt);
new_file(path,"enter.pos",snt_files->enter_pos);
new_file(path,"snt_offsets.pos",snt_files->snt_offsets_pos);
new_file(path,"concord.ind",snt_files->concord_ind);
new_file(path,"concord.txt",snt_files->concord_txt);
new_file(path,"concord.html",snt_files->concord_html);
new_file(path,"morpho.dic",snt_files->morpho_dic);
new_file(path,"morpho.bin",snt_files->morpho_bin);
new_file(path,"morpho.inf",snt_files->morpho_inf);
return snt_files;
}


/**
 * Allocates, initializes and returns a structure that contains the names
 * all the files related to a given .snt text.
 */
struct snt_files* new_snt_files(const char* text_snt) {
char path[FILENAME_MAX];
get_snt_path((const char*)text_snt,path);
return new_snt_files_from_path(path);
}


/**
 * Frees a snt_file structure.
 */
void free_snt_files(struct snt_files* snt_files) {
if (snt_files==NULL) return;
free(snt_files);
}

} // namespace unitex
