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

#include <stdlib.h>
#include <string.h>
#include "Persistence.h"
#include "Error.h"

typedef struct PS_ {
	char* name;
	void* ptr;
	struct PS_* next;
} PersistentStructure;


static PersistentStructure* list=NULL;


/**
 * Returns the persistent pointer associated to the given file name,
 * if any; NULL otherwise.
 */
void* get_persistent_structure(const char* filename) {
PersistentStructure* tmp=list;
while (tmp!=NULL) {
	if (!strcmp(tmp->name,filename)) return tmp->ptr;
	tmp=tmp->next;
}
return NULL;
}


static PersistentStructure* remove_filename(PersistentStructure* l,const char* filename) {
if (l==NULL) return NULL;
if (!strcmp(l->name,filename)) {
	PersistentStructure* next=l->next;
	free(l->name);
	free(l);
	return next;
}
l->next=remove_filename(l->next,filename);
return l;
}


/**
 * Associates the given pointer to the given file name if ptr is not NULL.
 * If ptr is NULL, then the pointer is removed from the list.
 */
void set_persistent_structure(const char* filename,void* ptr) {
if (ptr==NULL) {
	list=remove_filename(list,filename);
	return;
}
PersistentStructure* tmp=(PersistentStructure*)malloc(sizeof(PersistentStructure));
if (tmp==NULL) {
	fatal_alloc_error("set_persistent_structure");
}
tmp->name=strdup(filename);
if (tmp->name==NULL) {
	fatal_alloc_error("set_persistent_structure");
}
tmp->ptr=ptr;
tmp->next=list;
list=tmp;
}

/**
 * Returns 1 if the given pointer is a persistent one; 0 otherwise.
 */
int is_persistent_structure(void* ptr) {
PersistentStructure* tmp=list;
while (tmp!=NULL) {
	if (tmp->ptr==ptr) return 1;
	tmp=tmp->next;
}
return 0;
}

