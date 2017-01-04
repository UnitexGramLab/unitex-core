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

#include <stdlib.h>
#include <string.h>
#include "Persistence.h"
#include "Error.h"
#include "AbstractCallbackFuncModifier.h"
#include "SyncTool.h"
#include "VirtualFiles.h"



#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

typedef struct PS_ {
    char* name;
    void* ptr;
    struct PS_* next;
} PersistentStructure;


class PersistenceMutexContainer
{
public:
    PersistenceMutexContainer();
    ~PersistenceMutexContainer();
    inline SYNC_Mutex_OBJECT getMutex() { return mutex; }
private:
    SYNC_Mutex_OBJECT mutex ;
};

PersistenceMutexContainer::PersistenceMutexContainer() :
  mutex(SyncBuildMutex())
{
}

PersistenceMutexContainer::~PersistenceMutexContainer()
{
    SyncDeleteMutex(mutex);
    mutex = NULL;
}

static PersistenceMutexContainer PersistenceMutexContainerInstance;

static PersistentStructure* list=NULL;

#define Persistence_Mutex (PersistenceMutexContainerInstance.getMutex())

/**
 * Returns the persistent pointer associated to the given file name,
 * if any; NULL otherwise.
 */
void* get_persistent_structure(const char* filename) {
SyncGetMutex(Persistence_Mutex);
if (strstr(filename,VIRTUAL_FILE_PFX)==filename) {
    filename=filename+strlen(VIRTUAL_FILE_PFX);
}
PersistentStructure* tmp=list;
void* res=NULL;
while (tmp!=NULL) {
    if (!strcmp(tmp->name,filename)) {
        res=tmp->ptr;
        break;
    }
    tmp=tmp->next;
}
SyncReleaseMutex(Persistence_Mutex);
return res;
}


static PersistentStructure* remove_filename(PersistentStructure* l,const char* filename) {
if (l==NULL) return NULL;
if (strstr(filename,VIRTUAL_FILE_PFX)==filename) {
    filename=filename+strlen(VIRTUAL_FILE_PFX);
}
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
SyncGetMutex(Persistence_Mutex);
if (strstr(filename,VIRTUAL_FILE_PFX)==filename) {
    filename=filename+strlen(VIRTUAL_FILE_PFX);
}
if (ptr==NULL) {
    list=remove_filename(list,filename);
    SyncReleaseMutex(Persistence_Mutex);
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
SyncReleaseMutex(Persistence_Mutex);
}

/**
 * Returns 1 if the given pointer is a persistent one; 0 otherwise.
 */
int is_persistent_structure(void* ptr) {
SyncGetMutex(Persistence_Mutex);
int res=0;
PersistentStructure* tmp=list;
while (tmp!=NULL) {
    if (tmp->ptr==ptr) {
        res=1;
        break;
    }
    tmp=tmp->next;
}
SyncReleaseMutex(Persistence_Mutex);
return res;
}




} // namespace unitex
