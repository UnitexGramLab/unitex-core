 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include "Error.h"
#include "AbstractFilePlugCallback.h"
#include "Af_stdio.h"
#include "MappedFileHelper.h"
#include "ActivityLogger.h"



struct AbstractFileSpace {
	t_fileio_func_array_ex func_array;
	void* privateSpacePtr;
} ;


struct List_AbstractFileSpace {
	AbstractFileSpace afs;
	List_AbstractFileSpace* next;
} ;


struct List_AbstractFileSpace* p_abstract_file_space_list=NULL;



UNITEX_FUNC int UNITEX_CALL AddAbstractFileSpaceEx(const t_fileio_func_array_ex* func_array,void* privateSpacePtr)
{
	struct List_AbstractFileSpace* new_item;
	new_item = (struct List_AbstractFileSpace*)malloc(sizeof(struct List_AbstractFileSpace));
	if (new_item == NULL)
		return 0;

	new_item->afs.func_array = *func_array;
	new_item->afs.privateSpacePtr = privateSpacePtr;
	new_item->next = NULL;

	if (p_abstract_file_space_list == NULL)
		p_abstract_file_space_list = new_item;
	else {
		struct List_AbstractFileSpace* tmp = p_abstract_file_space_list;
		while ((tmp->next) != NULL)
			tmp = tmp->next;
		tmp->next = new_item;
	}

	if ((new_item->afs.func_array.fnc_Init_FileSpace) != NULL)
		(*(new_item->afs.func_array.fnc_Init_FileSpace))(new_item->afs.privateSpacePtr);

	return 1;
}

UNITEX_FUNC int UNITEX_CALL RemoveAbstractFileSpaceEx(const t_fileio_func_array_ex* func_array,void* privateSpacePtr)
{
	struct List_AbstractFileSpace* tmp = p_abstract_file_space_list;
	struct List_AbstractFileSpace* tmp_previous = NULL;

	while (tmp != NULL)
	{
		if ((memcmp(&tmp->afs.func_array,func_array,sizeof(t_fileio_func_array_ex))==0) &&
			(tmp->afs.privateSpacePtr == privateSpacePtr))
		{
			if (tmp_previous == NULL)
				p_abstract_file_space_list = tmp->next;
			else
				tmp_previous->next = tmp->next;

			if ((tmp->afs.func_array.fnc_Uninit_FileSpace) != NULL)
				(*(tmp->afs.func_array.fnc_Uninit_FileSpace))(tmp->afs.privateSpacePtr);

			free(tmp);
			return 1;
		}
		tmp_previous = tmp;
		tmp = tmp->next;
	}
	return 0;
}

UNITEX_FUNC int UNITEX_CALL AddAbstractFileSpace(const t_fileio_func_array* func_array,void* privateSpacePtr)
{
    t_fileio_func_array_ex func_array_ex;
    memset(&func_array_ex,0,sizeof(t_fileio_func_array_ex));
    memcpy(&func_array_ex,func_array,sizeof(t_fileio_func_array));
    return AddAbstractFileSpaceEx(&func_array_ex,privateSpacePtr);
}

UNITEX_FUNC int UNITEX_CALL RemoveAbstractFileSpace(const t_fileio_func_array* func_array,void* privateSpacePtr)
{
    t_fileio_func_array_ex func_array_ex;
    memset(&func_array_ex,0,sizeof(t_fileio_func_array_ex));
    memcpy(&func_array_ex,func_array,sizeof(t_fileio_func_array));
    return RemoveAbstractFileSpaceEx(&func_array_ex,privateSpacePtr);
}

UNITEX_FUNC int UNITEX_CALL GetNbAbstractFileSpaceInstalled()
{
    int count=0;
    struct List_AbstractFileSpace* tmp = p_abstract_file_space_list;
	while (tmp != NULL)
	{
        count++;
		tmp = tmp->next;
	}
	return count;
}


const AbstractFileSpace * GetFileSpaceForFileName(const char*name)
{
	const struct List_AbstractFileSpace* tmp = p_abstract_file_space_list;

	while (tmp != NULL)
	{
		const AbstractFileSpace * test_afs = &(tmp->afs);
		if (tmp->afs.func_array.fnc_is_filename_object(name,tmp->afs.privateSpacePtr) != 0)
			return test_afs;		

		tmp = tmp->next;
	}
	return NULL;
}


/*********************************************************************/

typedef struct _ABSTRACTFILE_REAL
{
	union
	{
		FILE* f;
		ABSTRACTFILE_PTR fabstr;
	} ;
	const AbstractFileSpace * afs;
} ABSTRACTFILE_REAL;


typedef struct _ABSTRACTMAPFILE_REAL
{
	union
	{
		MAPFILE* f;
		ABSTRACTFILE_PTR fabstr;
	} ;
	const AbstractFileSpace * afs;
} ABSTRACTMAPFILE_REAL;

/*********************************************************************/


const ABSTRACTFILE_REAL VF_StdIn = { {stdin},NULL };
const ABSTRACTFILE_REAL VF_StdOut = { {stdout},NULL };
const ABSTRACTFILE_REAL VF_StdErr = { {stderr},NULL };

const ABSTRACTFILE* pVF_StdIn  = (const ABSTRACTFILE*)&VF_StdIn;
const ABSTRACTFILE* pVF_StdOut = (const ABSTRACTFILE*)&VF_StdOut;
const ABSTRACTFILE* pVF_StdErr = (const ABSTRACTFILE*)&VF_StdErr;


int IsStdIn(ABSTRACTFILE* stream)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	return (((p_abfr->f)==stdin) && (p_abfr->afs==NULL));
}


int IsStdOut(ABSTRACTFILE* stream)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	return (((p_abfr->f)==stdout) && (p_abfr->afs==NULL));
}


int IsStdErr(ABSTRACTFILE* stream)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	return (((p_abfr->f)==stderr) && (p_abfr->afs==NULL));
}


struct stdwrite_param
{
	int trashOutput;
	t_fnc_stdOutWrite fnc_stdOutWrite;
	void* privatePtr;
};


struct stdwrite_param stdwrite_setparam[2] = { { 0 , NULL, NULL } , { 0 , NULL , NULL } };


struct stdwrite_param* get_std_write_param(ABSTRACTFILE*stream)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	if (p_abfr->afs == NULL)
	{
		if (((p_abfr->f)==stdout))
			return &stdwrite_setparam[stdwrite_kind_out];

		if (((p_abfr->f)==stderr))
			return &stdwrite_setparam[stdwrite_kind_err];
	}

	return NULL;
}

UNITEX_FUNC int UNITEX_CALL SetStdWriteCB(enum stdwrite_kind swk, int trashOutput, 
										t_fnc_stdOutWrite fnc_stdOutWrite,void* privatePtr)
{
	if ((swk != stdwrite_kind_out) && (swk != stdwrite_kind_err))
		return 0;

	if ((stdwrite_setparam[swk].trashOutput == 0) && (stdwrite_setparam[swk].fnc_stdOutWrite != NULL))
	{
		(*(stdwrite_setparam[swk].fnc_stdOutWrite))(NULL,0,stdwrite_setparam[swk].privatePtr);
	}
	
	stdwrite_setparam[swk].trashOutput = trashOutput;
	stdwrite_setparam[swk].fnc_stdOutWrite = fnc_stdOutWrite;
	stdwrite_setparam[swk].privatePtr = privatePtr;

	return 1;
}

UNITEX_FUNC int UNITEX_CALL GetStdWriteCB(enum stdwrite_kind swk, int* p_trashOutput, 
										t_fnc_stdOutWrite* p_fnc_stdOutWrite,void** p_privatePtr)
{
	if ((swk != stdwrite_kind_out) && (swk != stdwrite_kind_err))
		return 0;

	if (p_trashOutput != NULL)
		*p_trashOutput = stdwrite_setparam[swk].trashOutput ;

	if (p_fnc_stdOutWrite != NULL)
		*p_fnc_stdOutWrite = stdwrite_setparam[swk].fnc_stdOutWrite ;

	if (p_privatePtr != NULL)
		*p_privatePtr = stdwrite_setparam[swk].privatePtr ;

	return 1;
}


/*******************************************************/


struct t_stdin_param
{
	t_fnc_stdIn fnc_stdIn;
	void* privatePtr;
};


struct t_stdin_param stdin_param = { 0 , NULL } ;


struct t_stdin_param* get_std_in(ABSTRACTFILE*stream)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	if (p_abfr->afs == NULL)
	{
		if (((p_abfr->f)==stdin))
			return &stdin_param;
	}

	return NULL;
}

UNITEX_FUNC int UNITEX_CALL SetStdInCB(t_fnc_stdIn fnc_stdInRead,void* privatePtr)
{
	if (stdin_param.fnc_stdIn != NULL)
	{
		(*(stdin_param.fnc_stdIn))(NULL,0,stdin_param.privatePtr);
	}
	
	stdin_param.fnc_stdIn = fnc_stdInRead;
	stdin_param.privatePtr = privatePtr;

	return 1;
}

UNITEX_FUNC int UNITEX_CALL GetStdInCB(t_fnc_stdIn* p_fnc_stdInRead,void** p_privatePtr)
{
	if (p_fnc_stdInRead != NULL)
		*p_fnc_stdInRead = stdin_param.fnc_stdIn;

	if (p_privatePtr != NULL)
		*p_privatePtr = stdin_param.privatePtr ;

	return 1;
}


/*
 * is_filename_in_abstract_file_space return 
       0 if a filename is a file opened with fopen
       1 if a filename use an installed abstract filespace
 */
int is_filename_in_abstract_file_space(const char*name)
{
    const AbstractFileSpace * pafs ;
    pafs = GetFileSpaceForFileName(name);

    return (pafs == NULL) ? 0 : 1;
}

/*****************************************************************************/
/*
 * f_open like function
 * MODE value used in Unitex : "rb", "ab", "r+b", "wb"
 */
ABSTRACTFILE* af_fopen_unlogged(const char* name,const char* MODE)
{
	ABSTRACTFILE_REAL* vf= (ABSTRACTFILE_REAL*)malloc(sizeof(ABSTRACTFILE_REAL));
	const AbstractFileSpace * pafs ;
	if (vf==NULL) {
		fatal_alloc_error("af_fopen");
		return NULL;
	}

	pafs = GetFileSpaceForFileName(name);
	vf->afs = pafs;
	if (pafs == NULL) {
		vf->f = fopen(name,MODE);
		if (vf->f == NULL) {
			free(vf);
			vf = NULL;
		}
	}
	else
	{
        TYPEOPEN_MF TypeOpen;
        if ((*(MODE))=='w')
            TypeOpen = OPEN_CREATE_MF;
        else {
            TypeOpen = OPEN_READWRITE_MF;
			if ((*(MODE))=='r')
				if ((*(MODE+1))=='b')
					if ((*(MODE+2))=='\0')
						TypeOpen = OPEN_READ_MF;
		}
        //vfRet -> Std_Stream_Type = STD_STREAM_MEMFILE;
		vf->afs = pafs;
		vf->fabstr = (*(pafs->func_array.fnc_memOpenLowLevel))(name, TypeOpen,
			                      pafs->privateSpacePtr);

        if (vf->fabstr == NULL)
        {
            free(vf);
            vf= NULL;
        }
        else
        {
          if ((*(MODE))=='a')
              (*(pafs->func_array.fnc_memLowLevelSeek))(vf->fabstr,0,SEEK_END,pafs->privateSpacePtr);        
        }
	}
	return (ABSTRACTFILE*)vf;
}


int af_fclose_unlogged(ABSTRACTFILE* stream)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	ABSTRACTFILE_REAL abfr=*p_abfr;
	free(p_abfr);
	if (abfr.afs == NULL)
		return fclose(abfr.f);
	else
		return (*(abfr.afs->func_array.fnc_memLowLevelClose))(abfr.fabstr,abfr.afs->privateSpacePtr);
}


size_t af_fread(void *ptr,size_t sizeItem,size_t nmemb,ABSTRACTFILE *stream)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	if (p_abfr->afs == NULL)
	{
		struct t_stdin_param* p_stdin_param = get_std_in(stream);
		if (p_stdin_param != NULL)
			if (p_stdin_param -> fnc_stdIn != NULL) {
				size_t nbByteToRead = sizeItem * nmemb;
				size_t res = (*(p_stdin_param -> fnc_stdIn))(ptr,nbByteToRead,p_stdin_param->privatePtr);
				if ((res > 0) && (sizeItem>0))
					res /= sizeItem;
				return res;
			}

		return fread(ptr,sizeItem,nmemb,p_abfr->f);
	}
	else {
		size_t nbByteToRead = sizeItem * nmemb;
		size_t res = (*(p_abfr->afs->func_array.fnc_memLowLevelRead))(p_abfr->fabstr,ptr,nbByteToRead,p_abfr->afs->privateSpacePtr);
		if ((res > 0) && (sizeItem>0))
			res /= sizeItem;
		return res;
	}
}

size_t af_fwrite(const void *ptr,size_t sizeItem,size_t nmemb,ABSTRACTFILE *stream)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	if (p_abfr->afs == NULL)
	{
		struct stdwrite_param* p_std_write_param = get_std_write_param(stream);
		if (p_std_write_param != NULL)
		{
			/* trashOutput == 2 mean hard trash : no log of output, include logger */
			/* trashOutput == 1 mean softer trash : log of output go to logger */
			if (p_std_write_param->trashOutput != 2)
			{
				if (IsStdOut(stream))
					Call_logger_fnc_LogOutWrite(ptr,sizeItem*nmemb);
				if (IsStdErr(stream))
					Call_logger_fnc_LogErrWrite(ptr,sizeItem*nmemb);
			}

			if (p_std_write_param->trashOutput != 0)
				return nmemb;

			if (p_std_write_param->fnc_stdOutWrite != NULL)
			{
				size_t nbByteToWrite = sizeItem * nmemb;
				if (nbByteToWrite == 0)
					return 0;
				size_t res = (*(p_std_write_param->fnc_stdOutWrite))(ptr,nbByteToWrite,p_std_write_param->privatePtr);
				if ((res > 0) && (sizeItem>0))
					res /= sizeItem;
				return res;
			}

		}
		return fwrite(ptr,sizeItem,nmemb,p_abfr->f);
	}
	else {
		size_t nbByteToWrite = sizeItem * nmemb;
		size_t res = (*(p_abfr->afs->func_array.fnc_memLowLevelWrite))(p_abfr->fabstr,ptr,nbByteToWrite,p_abfr->afs->privateSpacePtr);
		if ((res > 0) && (sizeItem>0))
			res /= sizeItem;
		return res;
	}
}


char *af_fgets(char * _Buf, int count, ABSTRACTFILE * stream)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	if (p_abfr->afs == NULL)
		return fgets(_Buf,count,p_abfr->f);
	else { 
		char* retval = _Buf;
		char* pointer = _Buf;

		if(retval!=NULL)
        {
            while (--count)
            {
              char ch;
              if (((*(p_abfr->afs->func_array.fnc_memLowLevelRead))(p_abfr->fabstr,&ch,1,p_abfr->afs->privateSpacePtr))!=1)
                    {
						if (pointer == _Buf)
							return NULL;
						else
							return retval;
                    }

              (*(pointer)) = ch ;
              pointer++;
              if (ch == '\n')
                break;
            }
			*pointer = '\0';
        }
		return retval;	
	}
}


int af_fseek(ABSTRACTFILE* stream, long offset, int whence)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	if (p_abfr->afs == NULL)
		return fseek(p_abfr->f,offset,whence);
	else
		return (*(p_abfr->afs->func_array.fnc_memLowLevelSeek))(p_abfr->fabstr, offset,whence,p_abfr->afs->privateSpacePtr);
}

long af_ftell(ABSTRACTFILE* stream)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	if (p_abfr->afs == NULL)
		return ftell(p_abfr->f);
	else {
		afs_size_type pos=0;
        (*(p_abfr->afs->func_array.fnc_memLowLevelTell))(p_abfr->fabstr, &pos,p_abfr->afs->privateSpacePtr);
        return (long)pos;
	}
}


int af_feof(ABSTRACTFILE* stream)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	if (p_abfr->afs == NULL)
		return feof(p_abfr->f);
	else {
		afs_size_type pos=0;
		afs_size_type sizeFile = 0;
        (*(p_abfr->afs->func_array.fnc_memLowLevelTell))(p_abfr->fabstr, &pos,p_abfr->afs->privateSpacePtr);
		(*(p_abfr->afs->func_array.fnc_memLowLevelGetSize))(p_abfr->fabstr, &sizeFile,p_abfr->afs->privateSpacePtr);
		if (sizeFile == pos)
			return 1;
		else
			return 0;
	}
}

int af_ungetc(int ch, ABSTRACTFILE* stream)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	if (p_abfr->afs == NULL)
		return ungetc(ch,p_abfr->f);
	else
		return (af_fseek(stream,-1,SEEK_CUR) != 0 ) ? EOF:ch;
}



ABSTRACTMAPFILE* af_open_mapfile_unlogged(const char* name)
{
	ABSTRACTMAPFILE_REAL* vf= (ABSTRACTMAPFILE_REAL*)malloc(sizeof(ABSTRACTMAPFILE_REAL));
	const AbstractFileSpace * pafs ;
	if (vf==NULL) {
		fatal_alloc_error("af_open_mapfile_unlogged");
		return NULL;
	}

	pafs = GetFileSpaceForFileName(name);
	vf->afs = pafs;
	if (pafs == NULL) {
		vf->f = iomap_open_mapfile(name);
		if (vf->f == NULL) {
			free(vf);
			vf = NULL;
		}
	}
	else
	{
        TYPEOPEN_MF TypeOpen;
        TypeOpen = OPEN_READ_MF;

		vf->afs = pafs;
		vf->fabstr = (*(pafs->func_array.fnc_memOpenLowLevel))(name, TypeOpen,
			                      pafs->privateSpacePtr);

        if (vf->fabstr == NULL)
        {
            free(vf);
            vf= NULL;
        }
	}
	return (ABSTRACTMAPFILE*)vf;
}


void af_close_mapfile_unlogged(ABSTRACTMAPFILE* stream)
{
	ABSTRACTMAPFILE_REAL* p_abfr=(ABSTRACTMAPFILE_REAL*)stream;
	ABSTRACTMAPFILE_REAL abfr=*p_abfr;
	free(p_abfr);
	if (abfr.afs == NULL)
		return iomap_close_mapfile(abfr.f);
	else
		/*return*/ (*(abfr.afs->func_array.fnc_memLowLevelClose))(abfr.fabstr,abfr.afs->privateSpacePtr);
}


size_t af_get_mapfile_size(ABSTRACTMAPFILE* streammap)
{
	ABSTRACTMAPFILE_REAL* p_abfr=(ABSTRACTMAPFILE_REAL*)streammap;
	if (p_abfr->afs == NULL)
		return iomap_get_mapfile_size(p_abfr->f);
	else {
		afs_size_type pos=0;
        (*(p_abfr->afs->func_array.fnc_memLowLevelGetSize))(p_abfr->fabstr, &pos,p_abfr->afs->privateSpacePtr);
        return (size_t)pos;
	}
}

const void* af_get_mapfile_pointer(ABSTRACTMAPFILE* streammap, size_t pos, size_t sizemap)
{
    ABSTRACTMAPFILE_REAL* p_abfr=(ABSTRACTMAPFILE_REAL*)streammap;
    size_t filesize = af_get_mapfile_size(streammap);
    if ((pos==0) && (sizemap==0))
        sizemap = filesize;
    if (sizemap == 0)
        return NULL;
    if (pos>filesize)
        return NULL;

	if (p_abfr->afs == NULL)
		return iomap_get_mapfile_pointer(p_abfr->f,pos,sizemap);
	else {
		afs_size_type pos=0;
        if (p_abfr->afs->func_array.fnc_memFile_getMapPointer != NULL)
            return (*(p_abfr->afs->func_array.fnc_memFile_getMapPointer))(p_abfr->fabstr, pos, sizemap,p_abfr->afs->privateSpacePtr);
        else
        {
            void* buf=malloc(sizemap);
            if (buf==NULL) {
                fatal_alloc_error("af_get_mapfile_pointer");
            }
            afs_size_type prev_pos=0 ;
            int success=1;
            (*(p_abfr->afs->func_array.fnc_memLowLevelTell))(p_abfr->fabstr, &prev_pos, p_abfr->afs->privateSpacePtr);
            (*(p_abfr->afs->func_array.fnc_memLowLevelSeek))(p_abfr->fabstr, pos, SEEK_SET, p_abfr->afs->privateSpacePtr);
            if ((*(p_abfr->afs->func_array.fnc_memLowLevelRead))(p_abfr->fabstr, buf, sizemap, p_abfr->afs->privateSpacePtr) != sizemap)
                success=0;
            (*(p_abfr->afs->func_array.fnc_memLowLevelSeek))(p_abfr->fabstr, prev_pos, SEEK_SET , p_abfr->afs->privateSpacePtr);

            if (success==0) {
                free(buf);
                return NULL;
            }
            return buf;
        }
	}
}

void af_release_mapfile_pointer(ABSTRACTMAPFILE*streammap, const void* buf, size_t sizemap)
{
    ABSTRACTMAPFILE_REAL* p_abfr=(ABSTRACTMAPFILE_REAL*)streammap;
	if (p_abfr->afs == NULL)
		return iomap_release_mapfile_pointer(p_abfr->f,buf);
	else {
        if (p_abfr->afs->func_array.fnc_memFile_getMapPointer != NULL) {
            if (p_abfr->afs->func_array.fnc_memFile_getMapPointer != NULL) {
                (*(p_abfr->afs->func_array.fnc_memFile_releaseMapPointer))(p_abfr->fabstr, buf, sizemap,p_abfr->afs->privateSpacePtr);
            }
        }
        else
        {
            if (buf != NULL)
                free((void*)buf);
        }
	}
}

/* when we create a new file, before write into, we can set the filesize,
   (if we known the size of result file)
   like the chsize or _chsize . This can help to avoid fragmentation or 
   rewrite the File allocation table often, by example */
void af_setsizereservation(ABSTRACTFILE* stream, long size_planned)
{
	ABSTRACTFILE_REAL* p_abfr=(ABSTRACTFILE_REAL*)stream;
	if (p_abfr->afs != NULL)
		if (p_abfr->afs->func_array.fnc_memLowLevelSetSizeReservation != NULL)
			(*(p_abfr->afs->func_array.fnc_memLowLevelSetSizeReservation))(p_abfr->fabstr, size_planned,p_abfr->afs->privateSpacePtr);	
}

int af_remove_unlogged(const char * Filename)
{
	const AbstractFileSpace * pafs = GetFileSpaceForFileName(Filename);
	if (pafs==NULL)
		return remove(Filename);
	else
		return (*(pafs->func_array.fnc_memFileRemove))(Filename,pafs->privateSpacePtr);
}


#define BUFFER_IO_SIZE (0x8000)
int af_copy_unlogged(const char* srcFile, const char* dstFile)
{
    ABSTRACTFILE* vfRead;
    ABSTRACTFILE* vfWrite;
    long size_to_do = 0;    
    int iSuccessCopyingRet=0;		
    char *szBuffer = NULL;
    int buffer_size = 0;


    vfRead = af_fopen(srcFile,"rb");
    if (vfRead == NULL)
        return -1;

    if (af_fseek(vfRead, 0, SEEK_END) == 0)
	{
		size_to_do = af_ftell(vfRead);
		buffer_size = ((size_to_do+1) < BUFFER_IO_SIZE) ? ((int)size_to_do+1) : BUFFER_IO_SIZE;
		szBuffer = (char *)malloc(buffer_size);
		if (szBuffer == NULL)
		{
			af_fclose(vfRead);
			return -1;
		}
	}

    vfWrite = af_fopen(dstFile,"wb");
    if (vfWrite == NULL)
    {
        af_fclose(vfRead);
        free(szBuffer);
        return 1;
    }    

    af_setsizereservation(vfWrite, size_to_do);

    if (af_fseek(vfRead, 0, SEEK_SET) != 0)
        iSuccessCopyingRet = 1;

    while (size_to_do>0)
    {
        int iThis = (size_to_do < buffer_size) ? (((int)size_to_do)) : buffer_size;
        int iReadDone = (int)af_fread(szBuffer,1,iThis,vfRead);
        int iWriteDone ;
        if (iReadDone == 0)
            break;
        iWriteDone = (int)af_fwrite(szBuffer,1,iReadDone,vfWrite);
        if (iWriteDone != iReadDone)
            break;
        size_to_do -= iWriteDone;
    }
    af_fclose(vfRead);
    af_fclose(vfWrite);
    free(szBuffer);

    if (size_to_do==0)
        return 0; /* success */
	else
        return -1;
}


int af_rename_unlogged(const char * OldFilename, const char * NewFilename)
{
	const AbstractFileSpace * pafsOld = GetFileSpaceForFileName(OldFilename);
	const AbstractFileSpace * pafsNew = GetFileSpaceForFileName(NewFilename);

	if (pafsOld == pafsNew)
	{
		if (pafsOld==NULL)
			return rename(OldFilename,NewFilename);
		else
			return (*(pafsOld->func_array.fnc_memFileRename))(OldFilename,NewFilename,pafsOld->privateSpacePtr);
	}
	else
	{
		int ret = af_copy(OldFilename,NewFilename);
		if (ret != 0)
			return ret;
		return af_remove(OldFilename);
	}
}

ABSTRACTFILE* af_fopen(const char* name,const char* MODE)
{
    ABSTRACTFILE* stream;
    Call_logger_fnc_before_af_fopen(name,MODE);
    stream = af_fopen_unlogged(name,MODE);
    Call_logger_fnc_after_af_fopen(name,MODE,stream);
    return stream;
}

int af_fclose(ABSTRACTFILE* stream)
{
    int ret;
    Call_logger_fnc_before_af_fclose(stream);
    ret = af_fclose_unlogged(stream);
    Call_logger_fnc_after_af_fclose(stream, ret);
    return ret;
}


ABSTRACTMAPFILE* af_open_mapfile(const char*name)
{
    ABSTRACTMAPFILE* stream;
    //Call_logger_fnc_before_af_fopen(name,MODE);
    Call_logger_fnc_before_af_fopen(name,"rb");
    stream = af_open_mapfile_unlogged(name);
    //Call_logger_fnc_after_af_fopen(name,MODE,stream);
    Call_logger_fnc_after_af_fopen(name,"rb",NULL);
    return stream;
}

void af_close_mapfile(ABSTRACTMAPFILE* stream)
{
    //Call_logger_fnc_before_af_fclose(stream);
    af_close_mapfile_unlogged(stream);
    //Call_logger_fnc_after_af_fclose(stream, ret);
}

int af_remove(const char * Filename)
{
    int ret;
    Call_logger_fnc_before_af_remove(Filename);
    ret = af_remove_unlogged(Filename);
    Call_logger_fnc_after_af_remove(Filename,ret);
    return ret;
}

int af_copy(const char* srcFile, const char* dstFile)
{
    int ret;
    Call_logger_fnc_before_af_copy(srcFile, dstFile);
    ret = af_copy_unlogged(srcFile, dstFile);
    Call_logger_fnc_after_af_copy(srcFile, dstFile, ret);
    return ret;
}

int af_rename(const char * OldFilename, const char * NewFilename)
{
    int ret;
    Call_logger_fnc_before_af_rename(OldFilename, NewFilename);
    ret = af_rename_unlogged(OldFilename, NewFilename);
    Call_logger_fnc_after_af_rename(OldFilename, NewFilename, ret);
    return ret;
}
