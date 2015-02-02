/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "LoadInf.h"
#include "StringParsing.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define INF_LINE_SIZE 4096

/**
 * This function takes a line of a .INF file and tokenize it into
 * several single codes.
 * Example: .N,.V  =>  code 0=".N" ; code 1=".V"
 */
struct list_ustring* tokenize_compressed_info(const unichar* line,Abstract_allocator prv_alloc) {
struct list_ustring* result=NULL;
unichar tmp[INF_LINE_SIZE];
int pos=0;
/* Note: all protected characters must stay protected */
while (P_EOS!=parse_string(line,&pos,tmp,P_COMMA,P_EMPTY,NULL)) {
   result=new_list_ustring(tmp,result,prv_alloc);
   if (line[pos]==',') pos++;
}
return result;
}


/**
 * This function loads the content of an .inf file and returns
 * a structure containing the lines of the file tokenized into INF
 * codes.
 */
struct INF_codes* load_INF_file(const VersatileEncodingConfig* vec,const char* name,Abstract_allocator prv_alloc) {
struct INF_codes* res;
U_FILE* f=u_fopen(vec,name,U_READ);
if (f==NULL) {
   error("Cannot open %s\n",name);
   return NULL;
}
res=(struct INF_codes*)malloc_cb(sizeof(struct INF_codes),prv_alloc);
if (res==NULL) {
   fatal_alloc_error("in load_INF_file");
}
if (1!=u_fscanf(f,"%d\n",&(res->N))) {
   fatal_error("Invalid INF file: %s\n",name);
}
res->codes=(struct list_ustring**)malloc_cb(sizeof(struct list_ustring*)*(res->N),prv_alloc);
if (res->codes==NULL) {
   fatal_alloc_error("in load_INF_file");
}
Ustring *s=new_Ustring(1024);
int i=0;
/* For each line of the .inf file, we tokenize it to get the single INF codes
 * it contains. */
while (EOF!=readline(s,f)) {
   res->codes[i++]=tokenize_compressed_info(s->str,prv_alloc);
}
free_Ustring(s);
u_fclose(f);
return res;
}


/**
 * Frees all the memory allocated for the given structure.
 */
void free_INF_codes(struct INF_codes* INF,Abstract_allocator prv_alloc) {
if (INF==NULL) {return;}
for (int i=0;i<INF->N;i++) {
   free_list_ustring(INF->codes[i],prv_alloc);
}
free_cb(INF->codes,prv_alloc);
free_cb(INF,prv_alloc);
}

} // namespace unitex

