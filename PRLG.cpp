/*
 * Unitex
 *
 * Copyright (C) 2001-2013 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdlib.h>
#include "Error.h"
#include "PRLG.h"
#include "Ustring.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

PRLG_DATA* new_PRLG_DATA(int offset,unichar* s) {
PRLG_DATA* d=(PRLG_DATA*)malloc(sizeof(PRLG_DATA));
if (d==NULL) {
	fatal_alloc_error("new_PRLG_DATA");
}
d->offset=offset;
d->data=u_strdup(s);
return d;
}


void free_PRLG_DATA(PRLG_DATA* d) {
if (d==NULL) return;
free(d->data);
free(d);
}


PRLG* load_PRLG_data(VersatileEncodingConfig* vec,char* filename) {
U_FILE* f=u_fopen(vec,filename,U_READ);
if (f==NULL) return NULL;
PRLG* p=(PRLG*)malloc(sizeof(PRLG));
if (p==NULL) {
	fatal_alloc_error("load_PRLG_data");
}
p->data=new_vector_ptr();
p->max_width=0;
Ustring* line=new_Ustring(1024);
int offset,n;
while (EOF!=readline(line,f)) {
	if (1!=u_sscanf(line->str,"%d%n",&offset,&n)) {
		fatal_error("Invalid line in PRLG file %s:\n%S\n",filename,line->str);
	}
	if (u_strlen(line)-n>p->max_width) {
		p->max_width=u_strlen(line)-n;
	}
	vector_ptr_add(p->data,new_PRLG_DATA(offset,line->str+n));
}
free_Ustring(line);
u_fclose(f);
return p;
}


void free_PRLG(PRLG* p) {
if (p==NULL) return;
free_vector_ptr(p->data,(void(*)(void*))free_PRLG_DATA);
free(p);
}


#define get_offset(x) ((PRLG_DATA*)p->data->tab[x])->offset
#define get_tag(x) ((PRLG_DATA*)p->data->tab[x])->data


/**
 * Returns the closest matching offset. It can be start-1 if no offset
 * in the given range is the matching one.
 */
int get_closest_offset(PRLG* p,int offset,int start,int end) {
if (start>end) return end;
if (start==end) {
	if (get_offset(start)<=offset) return start;
	return start-1;
}
int current=(start+end)/2;
int current_offset=get_offset(current);
if (current_offset==offset) return current;
if (current_offset<offset) return get_closest_offset(p,offset,current+1,end);
return get_closest_offset(p,offset,start,current-1);
}


/**
 * Returns a pointer to the closest previous PRLG tag found before the given offset
 * or a pointer to the empty string if none is found. The returned pointer MUST NOT
 * BE FREED!
 */
const unichar* get_closest_PRLG_tag(PRLG* p,int offset) {
int current=get_closest_offset(p,offset,0,p->data->nbelems-1);
if (current==-1) return U_EMPTY;
return get_tag(current);
}

} // namespace unitex
