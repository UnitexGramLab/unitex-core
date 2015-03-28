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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/* We need fopen */
FILE* (*real_fopen)(const char *,const char *)=fopen;

#include "Unicode.h"
#include "VirtualFiles.h"
#include "Error.h"
#include "AbstractFilePlugCallback.h"
#include "UnusedParameter.h"
#include "File.h"
#include "SyncTool.h"

using namespace unitex;

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
namespace virtualfile {

/**
 * Description of a VFS inode.
 */
typedef struct VFS_INODE_ {
	struct VFS* vfs;
	char* name;
	void* ptr;
	unsigned long size;
	unsigned long capacity;
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
	unsigned long pos;
} VFS_FILE;


static SYNC_Mutex_OBJECT VFS_mutex=NULL;


/**
 * Description of the VFS space
 */
struct VFS {
	const char* pfx;
	int default_block_size;
	VFS_INODE* list;
};

static struct VFS VFS_id={VIRTUAL_FILE_PFX,4096,NULL};


/**
 * Is the given file name concerned by our VSF ?
 */
int ABSTRACT_CALLBACK_UNITEX my_fnc_is_filename_object(const char* name,void* privateSpacePtr) {
VFS* vfs=(VFS*)privateSpacePtr;
return strstr(name,vfs->pfx)==name;
}


/**
 * Tests if there is an inode for the given name.
 */
static VFS_INODE* get_inode(VFS* vfs,const char* name) {
SyncGetMutex(VFS_mutex);
VFS_INODE* inode=vfs->list;
while (inode!=NULL) {
	if (!strcmp(inode->name,name)) {
		SyncReleaseMutex(VFS_mutex);
		return inode;
	}
	inode=inode->next;
}
SyncReleaseMutex(VFS_mutex);
return NULL;
}


static VFS_INODE* create_inode(VFS* vfs,const char* name,TYPEOPEN_MF TypeOpen) {
DISCARD_UNUSED_PARAMETER(TypeOpen)
VFS_INODE* inode=(VFS_INODE*)malloc(sizeof(VFS_INODE));
if (inode==NULL) {
	fatal_alloc_error("create_inode");
}
inode->vfs=vfs;
inode->name=strdup(name);
if (inode->name==NULL) {
	fatal_alloc_error("create_inode");
}
inode->size=0;
inode->capacity=inode->vfs->default_block_size;
inode->ptr=malloc(inode->capacity);
if (inode->ptr==NULL) {
	fatal_alloc_error("create_inode");
}
inode->open_in_write_mode=0;
inode->n_open=0;
inode->to_remove=0;
SyncGetMutex(VFS_mutex);
inode->next=vfs->list;
vfs->list=inode;
SyncReleaseMutex(VFS_mutex);
return inode;
}


static VFS_INODE* remove_inode(VFS_INODE* list,VFS_INODE* inode) {
if (list==NULL) {
	return NULL;
}
if (list!=inode) {
	list->next=remove_inode(list->next,inode);
	return list;
}
VFS_INODE* next=list->next;
if (inode->ptr!=NULL) {
	free(inode->ptr);
}
free(inode->name);
free(inode);
return next;
}

static void delete_inode(VFS_INODE* inode) {
SyncGetMutex(VFS_mutex);
VFS* vfs=inode->vfs;
vfs->list=remove_inode(vfs->list,inode);
SyncReleaseMutex(VFS_mutex);
}


/**
 * Returns 1 if the file content has been successfully loaded in memory,
 * or 0 if the file did not exist. A fatal error is raised if there
 * is no memory enough to load the file.
 */
static int load_file_content(VFS_INODE* inode) {
SyncGetMutex(VFS_mutex);
FILE* f=real_fopen(inode->name+strlen(inode->vfs->pfx),"rb");
if (f==NULL) {
	SyncReleaseMutex(VFS_mutex);
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
if (inode->size!=(unsigned long)fread(inode->ptr,1,inode->size,f)) {
	fatal_error("Error loading content of %s\n",inode->name);
}
fclose(f);
SyncReleaseMutex(VFS_mutex);
return 1;
}


/**
 * open
 */
ABSTRACTFILE_PTR ABSTRACT_CALLBACK_UNITEX my_fnc_memOpenLowLevel(const char* name,TYPEOPEN_MF TypeOpen,void* privateSpacePtr) {
VFS* vfs=(VFS*)privateSpacePtr;
/*switch(TypeOpen) {
case OPEN_READ_MF: error("open READ: %s\n",name); break;
case OPEN_READWRITE_MF: error("open READWRITE: %s\n",name); break;
case OPEN_CREATE_MF: error("open CREATE: %s\n",name); break;
}*/
VFS_INODE* inode=get_inode(vfs,name);
int inode_created=0;
if (inode==NULL) {
	if (TypeOpen == OPEN_READ_MF) {
		return NULL;
	}
	inode=create_inode(vfs,name,TypeOpen);
	inode_created=1;
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
if (inode_created && TypeOpen!=OPEN_CREATE_MF) {
	/* If we have to load the file content from disk, we do it */
	if (!load_file_content(inode)) {
		return NULL;
	}
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
size_t ABSTRACT_CALLBACK_UNITEX my_fnc_memLowLevelRead(ABSTRACTFILE_PTR llFile, void *Buf, size_t size,void* privateSpacePtr) {
DISCARD_UNUSED_PARAMETER(privateSpacePtr)
VFS_FILE* f=(VFS_FILE*)llFile;
if (f->open_type==OPEN_CREATE_MF) {
	/* Cannot read in write-only mode */
	return 0;
}
unsigned int to_read=f->inode->size-f->pos;
if (size<to_read) {
	to_read=(unsigned int)size;
}
memcpy(Buf,((char*)f->inode->ptr)+f->pos,to_read);
f->pos+=to_read;
return to_read;
}


/**
 * write
 */
size_t ABSTRACT_CALLBACK_UNITEX my_fnc_memLowLevelWrite(ABSTRACTFILE_PTR llFile, void const *Buf, size_t size,void* privateSpacePtr) {
DISCARD_UNUSED_PARAMETER(privateSpacePtr)
VFS_FILE* f=(VFS_FILE*)llFile;
if (f->open_type==OPEN_READ_MF) {
	/* Cannot write in read-only mode */
	return 0;
}
if (f->pos+size>f->inode->capacity) {
	/* We need to enlarge our buffer */
	unsigned int n=f->inode->capacity;
	if (n==0) n=1;
	while (f->pos+size>n) n=n*2;
	f->inode->ptr=realloc(f->inode->ptr,n);
	if (f->inode->ptr==NULL) {
		fatal_error("Cannot allocate buffer to write virtual file %s\n",f->inode->name);
	}
	f->inode->capacity=n;
}
memcpy((char*)(f->inode->ptr)+f->pos,Buf,size);
f->pos+=(unsigned long)size;
if (f->pos>f->inode->size) {
	f->inode->size=f->pos;
}
return size;
}


/**
 * seek
 */
int ABSTRACT_CALLBACK_UNITEX my_fnc_memLowLevelSeek(ABSTRACTFILE_PTR llFile, afs_size_type Pos, int TypeSeek,void* privateSpacePtr) {
DISCARD_UNUSED_PARAMETER(privateSpacePtr)
VFS_FILE* f=(VFS_FILE*)llFile;
int new_pos=0;
switch (TypeSeek) {
case SEEK_SET: new_pos=(int)Pos; break;
case SEEK_CUR: new_pos=(int)(f->pos+Pos); break;
case SEEK_END: new_pos=(int)(f->inode->size+Pos); break;
}
if (new_pos<0) return -1;
if ((unsigned int)new_pos>f->inode->size) {
	fatal_error("Nasty fseek beyond the end of virtual file %s is not permitted\n",f->inode->name);
	return -1;
}
f->pos=new_pos;
return 0;
}


/**
 * close
 */
int ABSTRACT_CALLBACK_UNITEX my_fnc_memLowLevelClose(ABSTRACTFILE_PTR llFile,void* privateSpacePtr) {
DISCARD_UNUSED_PARAMETER(privateSpacePtr)
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
void ABSTRACT_CALLBACK_UNITEX my_fnc_memLowLevelGetSize(ABSTRACTFILE_PTR llFile,afs_size_type *pPos,void* privateSpacePtr) {
DISCARD_UNUSED_PARAMETER(privateSpacePtr)
VFS_FILE* f=(VFS_FILE*)llFile;
*pPos=f->inode->size;
}


/**
 * ftell
 */
void ABSTRACT_CALLBACK_UNITEX my_fnc_memLowLevelTell(ABSTRACTFILE_PTR llFile,afs_size_type *pPos,void* privateSpacePtr) {
DISCARD_UNUSED_PARAMETER(privateSpacePtr)
VFS_FILE* f=(VFS_FILE*)llFile;
*pPos=f->pos;
}


/**
 * rename
 */
int ABSTRACT_CALLBACK_UNITEX my_fnc_memFileRename(const char* _OldFilename,const char* _NewFilename,void* privateSpacePtr) {
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
int ABSTRACT_CALLBACK_UNITEX my_fnc_memFileRemove(const char* lpFileName,void* privateSpacePtr) {
VFS_INODE* inode=get_inode((VFS*)privateSpacePtr,lpFileName);
if (inode==NULL) return 1;
inode->to_remove=1;
if (inode->n_open==0) {
	delete_inode(inode);
}
return 0;
}


const void* my_fnc_memFile_getMapPointer(ABSTRACTFILE_PTR llFile, afs_size_type pos, afs_size_type /*len*/,int /*options*/,
		afs_size_type /* value_for_options*/,void* /* privateSpacePtr */) {
VFS_FILE* f=(VFS_FILE*)llFile;
return ((char*)f->inode->ptr)+pos;
}

/**
 * Returns the current list of virtual files.
 */
char** VFS_ls() {
VFS_INODE* inode=VFS_id.list;
int n=0;
while (inode!=NULL) {
	n++;
	inode=inode->next;
}
n++;
char** names=(char**)malloc(n*sizeof(char*));
if (names==NULL) {
	fatal_alloc_error("VFS_ls");
}
inode=VFS_id.list;
n=0;
while (inode!=NULL) {
	names[n]=strdup(inode->name);
	if (names[n]==NULL) {
		fatal_alloc_error("VFS_ls");
	}
	n++;
	inode=inode->next;
}
names[n]=NULL;
return names;
}

char** ABSTRACT_CALLBACK_UNITEX my_memFile_getList(void* /*privateSpacePtr*/) {
	return VFS_ls();
}


void ABSTRACT_CALLBACK_UNITEX my_memFile_releaseList(char**list,void* /*privateSpacePtr*/)
{
	if (list==NULL)
		return;

	char** list_walk=list;
	while ((*list_walk)!=NULL)
	{
		free(*list_walk);
		list_walk++;
	}
	free(list);
}

/**
 * Initialization of the virtual file system
 */

static void fill_fileio_func_array_extensible(t_fileio_func_array_extensible * my_VFS) {
	memset(my_VFS, 0, sizeof(t_fileio_func_array_extensible));
	my_VFS->size_func_array = sizeof(t_fileio_func_array_extensible);
	my_VFS->fnc_is_filename_object = my_fnc_is_filename_object;
	my_VFS->fnc_Init_FileSpace = NULL;
	my_VFS->fnc_Uninit_FileSpace = NULL;
	my_VFS->fnc_memOpenLowLevel = my_fnc_memOpenLowLevel;
	my_VFS->fnc_memLowLevelWrite = my_fnc_memLowLevelWrite;
	my_VFS->fnc_memLowLevelRead = my_fnc_memLowLevelRead;
	my_VFS->fnc_memLowLevelSeek = my_fnc_memLowLevelSeek;
	my_VFS->fnc_memLowLevelGetSize = my_fnc_memLowLevelGetSize;
	my_VFS->fnc_memLowLevelTell = my_fnc_memLowLevelTell;
	my_VFS->fnc_memLowLevelClose = my_fnc_memLowLevelClose;
	my_VFS->fnc_memLowLevelSetSizeReservation = NULL;
	my_VFS->fnc_memFileRemove = my_fnc_memFileRemove;
	my_VFS->fnc_memFileRename = my_fnc_memFileRename;
	my_VFS->fnc_memFile_getList = my_memFile_getList;
	my_VFS->fnc_memFile_releaseList = my_memFile_releaseList;

	VFS_mutex = SyncBuildMutex();
}

void init_virtual_files() {
t_fileio_func_array_extensible my_VFS;
fill_fileio_func_array_extensible(&my_VFS);

if (VFS_mutex != NULL) {
	SyncReleaseMutex(VFS_mutex);
	VFS_mutex = NULL;
}

if (!AddAbstractFileSpaceExtensible(&my_VFS,&VFS_id)) {
	fatal_error("Cannot create virtual file system\n");
}
}

void uninit_virtual_files() {
	t_fileio_func_array_extensible my_VFS;
	fill_fileio_func_array_extensible(&my_VFS);

	if (!RemoveAbstractFileSpaceExtensible(&my_VFS, &VFS_id)) {
		fatal_error("Cannot uninstall virtual file system\n");
	}
}

/**
 * Returns the length in bytes of the given virtual file,
 * or -1 if not found
 */
long VFS_size(const char* name) {
VFS_INODE* inode=get_inode(&VFS_id,name);
if (inode==NULL) {
	return -1;
}
return inode->size;
}


/**
 * Returns the pointer to the memory content of the given virtual file,
 * or NULL if not found.
 *
 * DON'T FREE THIS POINTER!
 */
void* VFS_content(const char* name) {
VFS_INODE* inode=get_inode(&VFS_id,name);
if (inode==NULL) {
	return NULL;
}
return inode->ptr;
}


/**
 * Reloads the given file from the disk if it is already loaded in memory.
 * Returns 1 in case of success; 0 otherwise.
 */
int VFS_reload(const char* name) {
VFS_INODE* inode=get_inode(&VFS_id,name);
if (inode==NULL) {
	return 0;
}
return load_file_content(inode);
}


/**
 * Dumps the given file on the disk.
 * Returns 1 in case of success; 0 otherwise.
 */
int VFS_dump(const char* name) {
VFS_INODE* inode=get_inode(&VFS_id,name);
if (inode==NULL) return 0;
create_path_to_file(name+strlen(inode->vfs->pfx));
FILE* f=real_fopen(name+strlen(inode->vfs->pfx),"wb");
if (f==NULL) {
	return 0;
}
if (inode->size!=fwrite(inode->ptr,1,inode->size,f)) {
	fclose(f);
	return 0;
}
fclose(f);
return 1;
}


void VFS_remove(const char* name) {
VFS_INODE* inode=get_inode(&VFS_id,name);
if (inode==NULL) return;
my_fnc_memFileRemove(inode->name,&VFS_id);
}


void VFS_reset() {
while (VFS_id.list!=NULL) {
	my_fnc_memFileRemove(VFS_id.list->name,&VFS_id);
}
}



class InitVirtualFiles
{
public:
	InitVirtualFiles();
	~InitVirtualFiles();
private:
};

InitVirtualFiles::InitVirtualFiles()
{
	init_virtual_files();
}

InitVirtualFiles::~InitVirtualFiles()
{
	uninit_virtual_files();
}

InitVirtualFiles InitVirtualFilesInstance;

} // namespace virtualfile
} // namespace unitex
