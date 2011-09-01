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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "VirtualFiles.h"
#include "Error.h"
#include "AbstractFilePlugCallback.h"

#define PFX "$:"

/**
 * Description of a VFS inode.
 */
typedef struct VFS_INODE_ {
	struct VFS* vfs;
	char* name;
	void* ptr;
	long size;
	long capacity;
	int open_in_write_mode;
	/* How many open pointers have we on this inode ?
	 * If we don't know, we can't make remove operation safely */
	int n_open;
	int to_remove;

	struct VFS_INODE_* next;
} VFS_INODE;


/**
 * Description of a file pointer returned by our open.
 */
typedef struct {
	VFS_INODE* inode;
	TYPEOPEN_MF open_type;
	long pos;
} VFS_FILE;


static t_fileio_func_array my_VFS;
static t_fileio_func_array_ex my_VFS_ex;


/**
 * Description of the VFS space
 */
struct VFS {
	const char* pfx;
	VFS_INODE* list;
};

static struct VFS VFS_id={PFX,NULL};


/**
 * Is the given file name concerned by our VSF ?
 */
int my_fnc_is_filename_object(const char* name,void* privateSpacePtr) {
VFS* vfs=(VFS*)privateSpacePtr;
return strstr(name,vfs->pfx)==name;
}


/**
 * Tests if there is an inode for the given name.
 */
static VFS_INODE* get_inode(VFS* vfs,const char* name) {
VFS_INODE* inode=vfs->list;
while (inode!=NULL) {
	if (!strcmp(inode->name,name)) return inode;
	inode=inode->next;
}
return NULL;
}


static VFS_INODE* create_inode(VFS* vfs,const char* name,TYPEOPEN_MF TypeOpen) {
VFS_INODE* inode=(VFS_INODE*)malloc(sizeof(VFS_INODE));
if (inode==NULL) {
	fatal_alloc_error("create_inode");
}
inode->vfs=vfs;
inode->name=strdup(name);
if (inode->name==NULL) {
	fatal_alloc_error("create_inode");
}
inode->ptr=NULL;
inode->size=0;
inode->capacity=0;
inode->open_in_write_mode=0;
inode->n_open=0;
inode->to_remove=0;
inode->next=vfs->list;
vfs->list=inode;
return inode;
}


static VFS_INODE* remove_inode(VFS_INODE* list,VFS_INODE* inode) {
if (list==NULL) return NULL;
if (list!=inode) {
	list->next=remove_inode(list->next,inode);
	return list;
}
VFS_INODE* next=list->next;
free(inode->name);
free(inode);
return next;
}

static void delete_inode(VFS_INODE* inode) {
VFS* vfs=inode->vfs;
vfs->list=remove_inode(vfs->list,inode);
}


/**
 * Returns 1 if the file content has been successfully loaded in memory,
 * or 0 if the file did not exist. A fatal error is raised if there
 * is no memory enough to load the file.
 */
static int load_file_content(VFS_INODE* inode) {
FILE* f=fopen(inode->name+strlen(inode->vfs->pfx),"rb");
if (f==NULL) {
	return 0;
}
fseek(f,0,SEEK_END);
inode->size=ftell(f);
inode->capacity=inode->size;
fseek(f,0,SEEK_SET);
inode->ptr=malloc(inode->size);
if (inode->ptr==NULL) {
	fatal_alloc_error("load_file_content");
}
if (inode->size!=(long)fread(inode->ptr,1,inode->size,f)) {
	fatal_error("Error loading content of %s\n",inode->name);
}
fclose(f);
return 1;
}


/**
 * open
 */
ABSTRACTFILE_PTR my_fnc_memOpenLowLevel(const char* name,TYPEOPEN_MF TypeOpen,void* privateSpacePtr) {
VFS* vfs=(VFS*)privateSpacePtr;
VFS_INODE* inode=get_inode(vfs,name);
if (inode==NULL) {
	inode=create_inode(vfs,name,TypeOpen);
}
/* If an inode exists, we must test if there is a concurrent
 * write access on the file */
if (TypeOpen!=OPEN_READ_MF) {
	if (inode->open_in_write_mode) {
		fatal_error("Cannot have a concurrent write access on virtual file %s\n",name);
	} else {
		inode->open_in_write_mode=1;
	}
}
if (inode->ptr==NULL && TypeOpen!=OPEN_CREATE_MF) {
	/* If we have to load the file content from disk, we do it */
	if (!load_file_content(inode)) return NULL;
}
if (TypeOpen==OPEN_CREATE_MF) {
	/* A created file must be truncated */
	inode->size=0;
}
VFS_FILE* f=(VFS_FILE*)malloc(sizeof(VFS_FILE));
if (f==NULL) {
	fatal_alloc_error("my_fnc_memOpenLowLevel");
}
f->inode=inode;
f->open_type=TypeOpen;
f->pos=0;
(inode->n_open)++;
return f;
}


/**
 * read
 */
size_t my_fnc_memLowLevelRead(ABSTRACTFILE_PTR llFile, void *Buf, size_t size,void* privateSpacePtr) {
VFS_FILE* f=(VFS_FILE*)llFile;
if (f->open_type==OPEN_CREATE_MF) {
	/* Cannot read in write-only mode */
	return 0;
}
int to_read=f->inode->size-f->pos;
if (size<to_read) {
	to_read=size;
}
memcpy(Buf,f->inode->ptr+f->pos,to_read);
f->pos+=to_read;
return to_read;
}


/**
 * write
 */
size_t my_fnc_memLowLevelWrite(ABSTRACTFILE_PTR llFile, void const *Buf, size_t size,void* privateSpacePtr) {
VFS_FILE* f=(VFS_FILE*)llFile;
if (f->open_type==OPEN_READ_MF) {
	/* Cannot write in read-only mode */
	return 0;
}
if (f->pos+size>f->inode->capacity) {
	/* We need to enlarge our buffer */
	int n=f->inode->capacity;
	if (n==0) n=1;
	while (f->pos+size>n) n=n*2;
	f->inode->ptr=realloc(f->inode->ptr,n);
	if (f->inode->ptr==NULL) {
		fatal_error("Cannot allocate buffer to write virtual file %s\n",f->inode->name);
	}
	f->inode->capacity=n;
}
memcpy(f->inode->ptr+f->pos,Buf,size);
f->pos+=size;
if (f->pos>f->inode->size) {
	f->inode->size=f->pos;
}
return size;
}


/**
 * seek
 */
int my_fnc_memLowLevelSeek(ABSTRACTFILE_PTR llFile, afs_size_type Pos, int TypeSeek,void* privateSpacePtr) {
VFS_FILE* f=(VFS_FILE*)llFile;
int new_pos;
switch (TypeSeek) {
case SEEK_SET: new_pos=Pos; break;
case SEEK_CUR: new_pos=f->pos+Pos; break;
case SEEK_END: new_pos=f->inode->size+Pos; break;
}
if (new_pos<0) return -1;
if (new_pos>f->inode->size) {
	fatal_error("Nasty fseek beyond the end of virtual file %s is not permitted\n",f->inode->name);
	return -1;
}
f->pos=new_pos;
return 0;
}


/**
 * close
 */
int my_fnc_memLowLevelClose(ABSTRACTFILE_PTR llFile,void* privateSpacePtr) {
VFS_FILE* f=(VFS_FILE*)llFile;
VFS_INODE* inode=f->inode;
(f->inode->n_open)--;
/* We have to update the open_in_write_mode flag */
if (f->open_type!=OPEN_READ_MF) {
	/* As there should only be one file in write mode at the same time,
	 * if the current one was, we reset the flag
	 */
	f->inode->open_in_write_mode=0;
}
free(f);
if (inode->n_open==0 && inode->to_remove) {
	delete_inode(inode);
}
return 0;
}


/**
 * getSize
 */
void my_fnc_memLowLevelGetSize(ABSTRACTFILE_PTR llFile,afs_size_type *pPos,void* privateSpacePtr) {
VFS_FILE* f=(VFS_FILE*)llFile;
*pPos=f->inode->size;
}


/**
 * ftell
 */
void my_fnc_memLowLevelTell(ABSTRACTFILE_PTR llFile,afs_size_type *pPos,void* privateSpacePtr) {
VFS_FILE* f=(VFS_FILE*)llFile;
*pPos=f->pos;
}


/**
 * rename
 */
int my_fnc_memFileRename(const char* _OldFilename,const char* _NewFilename,void* privateSpacePtr) {
VFS_INODE* inode=get_inode((VFS*)privateSpacePtr,_OldFilename);
if (inode==NULL) return 1;
if (!strcmp(_OldFilename,_NewFilename)) return 0;
free(inode->name);
inode->name=strdup(_NewFilename);
if (inode->name==NULL) {
	fatal_alloc_error("my_fnc_memFileRename");
}
return 0;
}


/**
 * remove
 */
int my_fnc_memFileRemove(const char* lpFileName,void* privateSpacePtr) {
VFS_INODE* inode=get_inode((VFS*)privateSpacePtr,lpFileName);
if (inode==NULL) return 1;
inode->to_remove=1;
if (inode->n_open==0) {
	delete_inode(inode);
}
return 0;
}


/**
 * Initialization of the virtual file system
 */
static int init() {
my_VFS.fnc_is_filename_object=my_fnc_is_filename_object;
my_VFS.fnc_Init_FileSpace=NULL;
my_VFS.fnc_Uninit_FileSpace=NULL;
my_VFS.fnc_memOpenLowLevel=my_fnc_memOpenLowLevel;
my_VFS.fnc_memLowLevelWrite=my_fnc_memLowLevelWrite;
my_VFS.fnc_memLowLevelRead=my_fnc_memLowLevelRead;
my_VFS.fnc_memLowLevelSeek=my_fnc_memLowLevelSeek;
my_VFS.fnc_memLowLevelGetSize=my_fnc_memLowLevelGetSize;
my_VFS.fnc_memLowLevelTell=my_fnc_memLowLevelTell;
my_VFS.fnc_memLowLevelClose=my_fnc_memLowLevelClose;
my_VFS.fnc_memLowLevelSetSizeReservation=NULL;
my_VFS.fnc_memFileRemove=my_fnc_memFileRemove;
my_VFS.fnc_memFileRename=my_fnc_memFileRename;

if (!AddAbstractFileSpace(&my_VFS,&VFS_id)) {
	fatal_error("Cannot create virtual file system\n");
}
return 0;
}


static int init_=init();
