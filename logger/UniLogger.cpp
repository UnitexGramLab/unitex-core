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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS) 
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
 *
 */





#ifndef NO_UNITEX_LOGGER

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "ActivityLoggerPlugCallback.h"
#include "File.h"

#include "FilePack.h"
#include "FilePackIo.h"
#include "FilePackCrc32.h"

#include "UniLogger.h"
#include "SyncTool.h"
#include "SyncLogger.h"
#include "ReworkArg.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {

struct ArrayExpanding {    
    unsigned int nb_item_filled;
    unsigned int nb_item_allocated;
    size_t size_item;

    void* ptrBuffer;
} ;

struct ArrayExpanding* InitArrayExpanding(size_t size_item, unsigned int init_alloc)
{
    struct ArrayExpanding* pAE;
    if (init_alloc<0x10)
        init_alloc=0x10;

    pAE = (struct ArrayExpanding*)malloc(sizeof(struct ArrayExpanding));
    if (pAE==NULL)
        return NULL;
    
    pAE->ptrBuffer = (void*)malloc(size_item*init_alloc);
    if (pAE->ptrBuffer == NULL)
    {
        free(pAE);
        return NULL;
    }
    pAE->size_item=size_item;
    pAE->nb_item_filled=0;
    pAE->nb_item_allocated=init_alloc;



    return pAE;
}

void FreeArrayExpanding(struct ArrayExpanding* pAE)
{
    if (pAE!=NULL)
    {
        free(pAE->ptrBuffer);
        free(pAE);
    }
}

int ExpandArrayExpanding(struct ArrayExpanding* pAE,unsigned int nb_item_needed)
{
    if (nb_item_needed >= (pAE->nb_item_allocated))
    {
        unsigned int nb_item_allocated_new=pAE->nb_item_allocated;
        while (nb_item_needed >= nb_item_allocated_new)
            nb_item_allocated_new *=2;
        void* new_ptrBuffer = realloc(pAE->ptrBuffer,(pAE->size_item)*nb_item_allocated_new);
        if (new_ptrBuffer == NULL)
            return 0;
        pAE->ptrBuffer = new_ptrBuffer;
        pAE->nb_item_allocated=nb_item_allocated_new;
    }
    if (nb_item_needed>pAE->nb_item_filled)
      pAE->nb_item_filled=nb_item_needed;
    return 1;
}

void* GetItemPtrArrayExpanding(struct ArrayExpanding* pAE,unsigned int num_item_needed)
{
    if (ExpandArrayExpanding(pAE,num_item_needed) == 0)
        return NULL;
    return (void*)(((unsigned char*)pAE->ptrBuffer) + ((pAE->size_item)*num_item_needed));
}


void DeleteItemPtrArrayExpanding(struct ArrayExpanding* pAE,unsigned int pos)
{
    char* ptrToRemove = (char*)(GetItemPtrArrayExpanding(pAE,pos));
    unsigned int nbItem = (pAE->nb_item_filled);
    unsigned int nbItemToCopy = (nbItem-pos)-1;

    memmove(ptrToRemove,ptrToRemove + (pAE->size_item), (pAE->size_item)*nbItemToCopy);
    pAE->nb_item_filled--;
}

unsigned int GetNbItemPtrArrayExpanding(struct ArrayExpanding* pAE)
{
    return (pAE->nb_item_filled);
}


void FlushOutData(const void*Buf, size_t size,
                  struct ArrayExpanding* pAEWrite)
{
    unsigned int previous_size = GetNbItemPtrArrayExpanding(pAEWrite);
    if (ExpandArrayExpanding(pAEWrite,(unsigned int)(previous_size + size + 0x00)) == 1)
    {
        memcpy(GetItemPtrArrayExpanding(pAEWrite,previous_size),Buf,size);
    }
}

/***********************************************************************************/



struct FileReadingInfoItem {
    const char*FileName;
};

struct FileToWriteInfoItem {
    const char*FileName;
    unsigned long crc;
    unsigned int size;
};



/***********************************************************************************/

struct ExecutionLogging {
    struct ArrayExpanding* pAE_FileReading;
    struct ArrayExpanding* pAE_FileToWrite;
    struct ArrayExpanding* pAE_StdOut;
    struct ArrayExpanding* pAE_StdErr;

    struct ArrayExpanding* pAE_FileReadList;
    int argc;
    char** argv;
    zipFile zf;
    zlib_filefunc_def zlib_filefunc;

    int store_file_out_content;
    int store_list_file_out_content;

    int store_file_in_content;
    int store_list_file_in_content;

    int store_std_out_content;
    int store_std_err_content;

    char* portion_ignore_pathname;
} ;

/***********************************************************************************/


int SearchFileInFileToReadArray(struct ExecutionLogging* pEL,const char*fn,unsigned int*pos)
{
    unsigned int nbItem=GetNbItemPtrArrayExpanding(pEL->pAE_FileReading);
    unsigned int i;
    for (i=0;i<nbItem;i++)
    {
        const struct FileReadingInfoItem* pFrif = (const struct FileReadingInfoItem*)GetItemPtrArrayExpanding(pEL->pAE_FileReading,i);
        if (strcmp(pFrif->FileName,fn)==0)
        {
            if (pos!=NULL)
                *pos=i;
            return 1;
        }
    }
    return 0;
}

int AddFileInFileToReadArray(struct ExecutionLogging* pEL,const char*fn)
{
    if (SearchFileInFileToReadArray(pEL,fn,NULL)!=0)
        return 1;
    struct FileReadingInfoItem Frif ;
    char *fnCopy=(char*)malloc(strlen(fn)+1);
    strcpy(fnCopy,fn);
    Frif.FileName=fnCopy;
    unsigned int nbItem=GetNbItemPtrArrayExpanding(pEL->pAE_FileReading);
    if (ExpandArrayExpanding(pEL->pAE_FileReading,nbItem+1)==0)
    {
        free(fnCopy);
        return 0;
    }
    *(struct FileReadingInfoItem*)(GetItemPtrArrayExpanding(pEL->pAE_FileReading,nbItem))=Frif;
    return 1;
}

void CleanFileReadArray(struct ExecutionLogging* pEL)
{
    if (pEL->pAE_FileReading != NULL)
    {
        unsigned int nbItem=GetNbItemPtrArrayExpanding(pEL->pAE_FileReading);
        unsigned int i;
        for (i=0;i<nbItem;i++)
        {
            struct FileReadingInfoItem* pFrif = (struct FileReadingInfoItem*)GetItemPtrArrayExpanding(pEL->pAE_FileReading,i);
            free((void*)pFrif->FileName);
        }
        FreeArrayExpanding(pEL->pAE_FileReading);
        pEL->pAE_FileReading=NULL;
    }
}


int SearchFileInFileToWriteArray(struct ExecutionLogging* pEL,const char*fn,unsigned int*pos)
{
    unsigned int nbItem=GetNbItemPtrArrayExpanding(pEL->pAE_FileToWrite);
    unsigned int i;
    for (i=0;i<nbItem;i++)
    {
        const struct FileToWriteInfoItem* pFrif = (const struct FileToWriteInfoItem*)GetItemPtrArrayExpanding(pEL->pAE_FileToWrite,i);
        if (strcmp(pFrif->FileName,fn)==0)
        {
            if (pos!=NULL)
                *pos=i;
            return 1;
        }
    }
    return 0;
}

int AddFileInFileToWriteArray(struct ExecutionLogging* pEL,const char*fn)
{
    if (SearchFileInFileToWriteArray(pEL,fn,NULL)!=0)
        return 1;
    struct FileToWriteInfoItem Frif ;
    char *fnCopy=(char*)malloc(strlen(fn)+1);
    strcpy(fnCopy,fn);
    Frif.FileName=fnCopy;
    Frif.crc = 0;
    Frif.size = 0;
    unsigned int nbItem=GetNbItemPtrArrayExpanding(pEL->pAE_FileToWrite);
    if (ExpandArrayExpanding(pEL->pAE_FileToWrite,nbItem+1)==0)
    {
        free(fnCopy);
        return 0;
    }
    *(struct FileToWriteInfoItem*)(GetItemPtrArrayExpanding(pEL->pAE_FileToWrite,nbItem))=Frif;
    return 1;
}

const char* GetFileToWriteItemFN(struct ExecutionLogging* pEL,unsigned int pos)
{
    return ((struct FileToWriteInfoItem*)(GetItemPtrArrayExpanding(pEL->pAE_FileToWrite,pos)))->FileName;
}

unsigned int GetNbFileToWrite(struct ExecutionLogging* pEL)
{
    return GetNbItemPtrArrayExpanding(pEL->pAE_FileToWrite);
}

void RemoveFileFileOnWriteArray(struct ExecutionLogging* pEL,const char*fn)
{
    unsigned int pos;
    if (SearchFileInFileToWriteArray(pEL,fn,&pos) == 0)
        return ;

    struct FileToWriteInfoItem* pFrif = (struct FileToWriteInfoItem*)GetItemPtrArrayExpanding(pEL->pAE_FileToWrite,pos);
    free((void*)pFrif->FileName);

    DeleteItemPtrArrayExpanding(pEL->pAE_FileToWrite,pos);
}

void CleanFileWriteArray(struct ExecutionLogging* pEL)
{
    if (pEL->pAE_FileToWrite != NULL)
    {
        unsigned int nbItem=GetNbItemPtrArrayExpanding(pEL->pAE_FileToWrite);
        unsigned int i;
        for (i=0;i<nbItem;i++)
        {
            struct FileToWriteInfoItem* pFrif = (struct FileToWriteInfoItem*)GetItemPtrArrayExpanding(pEL->pAE_FileToWrite,i);
            free((void*)pFrif->FileName);
        }

        FreeArrayExpanding(pEL->pAE_FileToWrite);
        pEL->pAE_FileToWrite=NULL;
    }
}

/***********************************************************************************/

struct ExecutionLogging* InitExecutionLogging(const char*pathZip)
{
    struct ExecutionLogging* pEL;

    pEL = (struct ExecutionLogging*)malloc(sizeof(struct ExecutionLogging));
    if (pEL==NULL)
        return NULL;

    fill_afopen_filefunc(&(pEL->zlib_filefunc));
    pEL->zf=zipOpen2(pathZip,APPEND_STATUS_CREATE,NULL,&(pEL->zlib_filefunc));
    if (pEL->zf==NULL)
    {
        free(pEL);
        return NULL;
    }
    
    pEL->pAE_FileReading = InitArrayExpanding(sizeof(struct FileReadingInfoItem),0);
    pEL->pAE_FileToWrite = InitArrayExpanding(sizeof(struct FileToWriteInfoItem),0);

    pEL->pAE_StdOut = InitArrayExpanding(sizeof(char),0);
    pEL->pAE_StdErr = InitArrayExpanding(sizeof(char),0);
    pEL->pAE_FileReadList = InitArrayExpanding(sizeof(char),0);

    pEL->argc = 0;
    pEL->argv = NULL;

    pEL -> store_file_out_content = pEL -> store_list_file_out_content = 
    pEL -> store_file_in_content = pEL -> store_list_file_in_content = 0;
    pEL -> store_std_out_content = pEL -> store_std_err_content = 1;

    pEL->portion_ignore_pathname = NULL;

    if ((pEL->pAE_StdOut == NULL) || (pEL->pAE_StdErr == NULL) ||
        (pEL->pAE_FileReading == NULL) || (pEL->pAE_FileToWrite == NULL) ||
        (pEL->pAE_FileReadList == NULL))
    {
        FreeArrayExpanding(pEL->pAE_StdOut);
        FreeArrayExpanding(pEL->pAE_StdErr);
        FreeArrayExpanding(pEL->pAE_FileReading);
        FreeArrayExpanding(pEL->pAE_FileToWrite);
        FreeArrayExpanding(pEL->pAE_FileReadList);
        zipClose(pEL->zf,"");
        free(pEL);
        return NULL;
    }

    return pEL;
}





/*************************************************************************/


char* buildDupFileNameWithPrefixDir(const char*szPathPrefix,const char*fn)
{
    char* szRet;
    
    if (szPathPrefix==NULL)
        szPathPrefix="";

    size_t len_prefix=strlen(szPathPrefix);
    size_t len_fn=strlen(fn);

    szRet=(char*)malloc(len_prefix+len_fn+8);

    int need_add_sep;
    if (len_prefix>0)
    {
        char c=*(szPathPrefix+len_prefix-1);
        if ((c=='\\') || (c=='/') || (c==':'))
            need_add_sep=0;
        else
            need_add_sep=1;
    }
    else
        need_add_sep=0;
    strcpy(szRet,szPathPrefix);
    if (need_add_sep != 0)
        strcat(szRet,PATH_SEPARATOR_STRING);
    strcat(szRet,fn);
    return szRet;
}

/****************************************************************************/


struct ActivityLoggerPrivateData
{
    struct UniLoggerSpace ule;
    SYNC_TLS_OBJECT pTlsSlot;
    SYNC_Mutex_OBJECT pMutexLog;
} ;

struct ExecutionLogging* BuildAllocInitExecutionLogging(void* privateLoggerPtr,const char* szLogFileName)
{
    struct UniLoggerSpace * pULS=(struct UniLoggerSpace *)privateLoggerPtr;
    if (pULS == NULL)
        return NULL;
    struct ActivityLoggerPrivateData* pALPD = (struct ActivityLoggerPrivateData*) pULS->privateUnloggerData;

    struct ExecutionLogging* pEL = (struct ExecutionLogging*)SyncTlsGetValue(pALPD->pTlsSlot);
    if (pEL != NULL)
        return pEL;

    pEL=InitExecutionLogging(szLogFileName);
        
    if (pEL == NULL)
        return NULL;

    pEL->store_file_out_content = pULS->store_file_out_content;
    pEL->store_list_file_out_content = pULS->store_list_file_out_content;

    pEL->store_std_out_content = pULS->store_std_out_content;
    pEL->store_std_err_content = pULS->store_std_err_content;

    pEL->store_file_in_content = pULS->store_file_in_content;
    pEL->store_list_file_in_content = pULS->store_list_file_in_content;

    SyncTlsSetValue(pALPD->pTlsSlot,(void*)pEL);
    return pEL;
}


struct ExecutionLogging* GetExecutionLogging(void* privateLoggerPtr)
{
    struct UniLoggerSpace * pULS=(struct UniLoggerSpace *)privateLoggerPtr;
    if (pULS == NULL)
        return NULL;
    struct ActivityLoggerPrivateData* pALPD = (struct ActivityLoggerPrivateData*) pULS->privateUnloggerData;

    struct ExecutionLogging* pEL = (struct ExecutionLogging*)SyncTlsGetValue(pALPD->pTlsSlot);
    return pEL;
}

void FreeExecutionLogging(void* privateLoggerPtr)
{
    struct UniLoggerSpace * pULS=(struct UniLoggerSpace *)privateLoggerPtr;
    if (pULS == NULL)
        return ;
    struct ActivityLoggerPrivateData* pALPD = (struct ActivityLoggerPrivateData*) pULS->privateUnloggerData;

    struct ExecutionLogging* pEL = (struct ExecutionLogging*)SyncTlsGetValue(pALPD->pTlsSlot);
    if (pEL != NULL)
    {
        int i;
        if (pEL->zf != NULL)
            zipClose(pEL->zf,NULL);
        pEL->zf=NULL;

        for (i=0;i<(pEL->argc);i++)
        {
            free(*(pEL->argv+i));
        }

        free(pEL->argv);
        pEL->argc = 0;
        pEL->argv = NULL;

        CleanFileReadArray(pEL);
        CleanFileWriteArray(pEL);
        FreeArrayExpanding(pEL->pAE_StdOut);
        pEL->pAE_StdOut = NULL;
        FreeArrayExpanding(pEL->pAE_StdErr);
        pEL->pAE_StdErr = NULL;

        FreeArrayExpanding(pEL->pAE_FileReadList);
        pEL->pAE_FileReadList = NULL;

        if (pEL->portion_ignore_pathname != NULL)
            free(pEL->portion_ignore_pathname);
        pEL->portion_ignore_pathname = NULL;

        free(pEL);
        SyncTlsSetValue(pALPD->pTlsSlot,NULL);
    }
    return ;
}

/*
void SetExecutionLogging(void* privateLoggerPtr,struct ExecutionLogging* pEL)
{
    struct UniLoggerSpace * pULS=(struct UniLoggerSpace *)privateLoggerPtr;
    pULS->privateUnloggerPtr = pEL;
}*/

UNITEX_FUNC int UNITEX_CALL SelectNextLogName(struct UniLoggerSpace *p_ule,const char* szLogFileName,const char* portion_ignore_pathname)
{
    void* privateLoggerPtr = (void*)p_ule;
    struct ExecutionLogging* pEL = GetExecutionLogging(privateLoggerPtr);

    if (pEL != NULL)
        return 0;

    pEL=BuildAllocInitExecutionLogging(privateLoggerPtr,szLogFileName);

    if (pEL == NULL)
        return 0;

    if (portion_ignore_pathname != NULL)
    {
        pEL ->portion_ignore_pathname = strdup(portion_ignore_pathname);
    }

    return 1;
}


static struct ExecutionLogging* BuildAllocInitExecutionLoggingForIncrementedNumber(void* privateLoggerPtr)
{
    struct ExecutionLogging* pEL = NULL;
 
    struct UniLoggerSpace * pULS=(struct UniLoggerSpace *)privateLoggerPtr;
    struct ActivityLoggerPrivateData* pALPD = (struct ActivityLoggerPrivateData*) pULS->privateUnloggerData;

    char szNumFileSuffix[256];


    if (pULS -> auto_increment_logfilename == 0)
    {
        return NULL;
    }
    /* we take a mutex, to be sure two thread don't increment and use same number at same time */
    SyncGetMutex(pALPD->pMutexLog);

    if (pULS->szNameLog == NULL) {


        unsigned int current_number = 0;
        const char* szNumFile = buildDupFileNameWithPrefixDir(pULS->szPathLog,"unitex_logging_parameters_count.txt");

        /* here : we will need protect with a mutex */
        ABSTRACTFILE *af_fin = af_fopen_unlogged(szNumFile,"rb");
        if (af_fin!=NULL)
        {
            size_t size_num_file=0;

            if (af_fseek(af_fin, 0, SEEK_END) == 0)
            {
	            size_num_file = af_ftell(af_fin);
                af_fseek(af_fin, 0, SEEK_SET);
            }

            char* buf_num_file=(char*)malloc(size_num_file+1);
            *(buf_num_file+size_num_file)=0;
            if (af_fread(buf_num_file,1,size_num_file,af_fin) == size_num_file)
            {
                sscanf(buf_num_file,"%u",&current_number);
            }
            af_fclose(af_fin);
            free(buf_num_file);        
        }

        current_number++;


        ABSTRACTFILE *af_fout = af_fopen_unlogged(szNumFile,"wb");
        if (af_fout!=NULL)
        {
            char szNumOut[32];
            sprintf(szNumOut,"%010u",current_number);
            af_fwrite(szNumOut,1,strlen(szNumOut),af_fout);
            af_fclose(af_fout);
        }
        free((void*)szNumFile);
        sprintf(szNumFileSuffix,"unitex_log_%08u.ulp",current_number);
    }
    else {
        sprintf(szNumFileSuffix,"%s",pULS->szNameLog);
    }
    SyncReleaseMutex(pALPD->pMutexLog);
    
    const char* szLogFileName = buildDupFileNameWithPrefixDir(pULS->szPathLog,szNumFileSuffix);

    pEL=BuildAllocInitExecutionLogging(privateLoggerPtr,szLogFileName);
    free((void*)szLogFileName);

    return pEL;
}


int DumpFileToPack(struct ExecutionLogging* pEL,const char* filename,const char* prefix,unsigned int*size_done,unsigned long*crc)
{
    char* name_to_store;
    size_t size_buf=0x10000;
    char*buf;
    *size_done =0;
    *crc=0;

    if (filename==NULL)
        return 0;
    if ((*filename)==0)
        return 0;

    buf = (char*)malloc(size_buf+ strlen(filename)+ ((prefix == NULL) ? 0 : strlen(prefix))+0x10);
    name_to_store = buf +size_buf;

    if (prefix != NULL)
        strcpy(name_to_store,prefix);
    else
        *name_to_store=0;

    const char* filenamecpy=GetFileNameRemovePrefixIfFound(filename,pEL->portion_ignore_pathname);

    filenamecpy = ExtractUsablePortionOfFileNameForPack(filenamecpy);

    strcat(name_to_store,filenamecpy);

    char* name_to_store_browse=name_to_store;
    while ((*name_to_store_browse)!=0)
    {
        if ((*name_to_store_browse)=='\\')
            (*name_to_store_browse)='/';
        name_to_store_browse++;
    }

    zip_fileinfo zi;

    zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
    zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
    zi.dosDate = 0;
    zi.internal_fa = 0;
    zi.external_fa = 0;

    zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
    zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = 9;
    zi.tmz_date.tm_year = 2009;
    zi.tmz_date.tm_mon--;

    ABSTRACTFILE*fin;
    int err;

    fin = af_fopen_unlogged(filename,"rb");
    if (fin==NULL)
    {
        err=ZIP_ERRNO;
        
        free(buf);
        return 0;
    }

    err = zipOpenNewFileInZip(pEL->zf,name_to_store,&zi,NULL,0,NULL,0,NULL /* param_list */,
                                 0,0);

    if (err==0)
    {
        size_t size_read;
        if (err == ZIP_OK)
            do
            {
                err = ZIP_OK;
                size_read = af_fread(buf,1,size_buf,fin);
                if (size_read < size_buf)
                    if (af_feof(fin)==0)
                {
                    /*
                    printf("error in reading %s\n",filenameinzip);
                    */
                    err = ZIP_ERRNO;
                }

                if (size_read>0)
                {
                    err = zipWriteInFileInZip (pEL->zf,buf,(unsigned int)size_read);
                    if (err<0)
                    {
                        /*
                        printf("error in writing %s in the zipfile\n",
                                         filenameinzip);*/
                    }
                    else 
                    {
                        *size_done += (unsigned int)size_read;
                        *crc = crc32(*crc,buf,size_read);
                    }

                }
            } while ((err == ZIP_OK) && (size_read>0));



            if (err<0)
            {
                err=ZIP_ERRNO;
                zipCloseFileInZip(pEL->zf);
            }
            else
            {
                err = zipCloseFileInZip(pEL->zf);
                /*
                if (err!=ZIP_OK)
                    printf("error in closing %s in the zipfile\n",
                                filenameinzip);
                                */
            }
    }
    af_fclose_unlogged(fin);
    free(buf);
    return 1;
}


int ComputeFileCrc(const char* filename,unsigned int*size_done,unsigned long*crc)
{
    size_t size_buf=0x10000;
    char*buf;
    int ret=1;

    if (size_done != NULL)
        *size_done =0;

    if (crc != NULL)
        *crc=0;

    if (filename==NULL)
        return 0;
    if ((*filename)==0)
        return 0;

    buf = (char*)malloc(size_buf);
    ABSTRACTFILE*fin;
    size_t size_read;

    fin = af_fopen_unlogged(filename,"rb");
    if (fin==NULL)
    {
        free(buf);
        return 0;
    }

    if ((crc != NULL) || (size_done != NULL))
    {
        do
        {
            size_read = af_fread(buf,1,size_buf,fin);
            if (size_read < size_buf)
                if (af_feof(fin)==0)
            {
                /*
                printf("error in reading %s\n",filenameinzip);
                */
                ret = 0;
            }

            if (size_read>0)
            {
                if (size_done != NULL)
                    *size_done += (unsigned int)size_read;
                if (crc != NULL)
                    *crc = crc32(*crc,buf,size_read);
            }
        } while ((size_read>0));
    }

    af_fclose_unlogged(fin);
    free(buf);
    return ret;
}



int DumpMemToPack(struct ExecutionLogging* pEL,const char* filename_to_store,const void* buf,unsigned int size)
{
    zip_fileinfo zi;

    zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
    zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
    zi.dosDate = 0;
    zi.internal_fa = 0;
    zi.external_fa = 0;

    zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
    zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = 9;
    zi.tmz_date.tm_year = 2009;
    zi.tmz_date.tm_mon--;

    int err;

    err = zipOpenNewFileInZip(pEL->zf,filename_to_store,&zi,NULL,0,NULL,0,NULL /* param_list */,
                                 0,0);

    if (err==0)
    {
        err = zipWriteInFileInZip (pEL->zf,buf,size);
        err = zipCloseFileInZip(pEL->zf);
    }

    return (err==0) ? 1 : 0;
}


void ABSTRACT_CALLBACK_UNITEX UniLogger_before_calling_tool(mainFunc*,int argc,char* const argv[],void* privateLoggerPtr)
{
    struct ExecutionLogging* pEL = GetExecutionLogging(privateLoggerPtr);

    if (pEL == NULL)
        pEL = BuildAllocInitExecutionLoggingForIncrementedNumber(privateLoggerPtr);
    
    if (pEL == NULL)
        return;

    pEL->argc = argc;
    pEL->argv=(char**)malloc((sizeof(char*)*(argc+1)));

    int i;
    size_t len_all_param_list = 0x100;
    for (i=0;i<argc;i++)
    {
        size_t len_this = strlen(argv[i]);
        len_all_param_list += len_this + 4;
        *(pEL->argv+i)=(char*)malloc(len_this+1);
        strcpy(*(pEL->argv+i),argv[i]);
    }
    *(pEL->argv+argc)=NULL;



    char* param_list= (char*)malloc(len_all_param_list);
    *param_list=0;
    for (i=0;i<(pEL->argc);i++)
    {
        size_t j=0;
        int add_quote=0;
        const char* curarg=*(pEL->argv+i);
        while ( * (curarg+j) != 0)
        {            
            char c=(*(curarg+j));
            if (c==' ') add_quote=1;
            j++;
        }
        if (add_quote != 0)
            strcat(param_list,"\"");
        //strcat(param_list,curarg);
        CopyReworkedArgRemoving(param_list+strlen(param_list),curarg,pEL->portion_ignore_pathname);
        if (add_quote != 0)
            strcat(param_list,"\"");
        
        if ((i+1)!=(pEL->argc))
            strcat(param_list," ");
    }

    DumpMemToPack(pEL,"test_info/command_line_synth.txt",param_list,(unsigned int)strlen(param_list));
    free(param_list);

}


void ABSTRACT_CALLBACK_UNITEX UniLogger_after_calling_tool(mainFunc*,int /*argc*/,char* const [] /* argv[]*/,int ret,void* privateLoggerPtr)
{
    struct ExecutionLogging* pEL = GetExecutionLogging(privateLoggerPtr);
    if (pEL == NULL)
        return;

    const unsigned int iNbFileWrite=GetNbFileToWrite(pEL);
    unsigned int iFile;
    for (iFile=0;iFile<iNbFileWrite;iFile++)
    {
        unsigned int size=0;
        unsigned long crc=0;
        
        if (pEL->store_file_out_content != 0)
        {
          DumpFileToPack(pEL,GetFileToWriteItemFN(pEL,iFile),"dest/",&size,&crc);
        }
        else
        if (pEL->store_list_file_out_content!=0)
        {
            ComputeFileCrc(GetFileToWriteItemFN(pEL,iFile),&size,&crc);
        }

        struct FileToWriteInfoItem* pFileToWriteInfoItem = (struct FileToWriteInfoItem*)
                      GetItemPtrArrayExpanding(pEL->pAE_FileToWrite,iFile);
        pFileToWriteInfoItem ->crc = crc;
        pFileToWriteInfoItem ->size = size;
    }

    char* buf_list_file_out = NULL;
    unsigned int pos_in_list_file_out = 0;

    if (pEL->store_list_file_out_content!=0)
    {
        size_t size_buf_list_file_out = 0x100;
        for (iFile=0;iFile<iNbFileWrite;iFile++)
            size_buf_list_file_out += strlen(GetFileToWriteItemFN(pEL,iFile)) + 0x40;

        
        buf_list_file_out = (char*)malloc(size_buf_list_file_out);
        *buf_list_file_out=0;
        pos_in_list_file_out = 0;

        for (iFile=0;iFile<iNbFileWrite;iFile++)
        {
            size_buf_list_file_out += strlen(GetFileToWriteItemFN(pEL,iFile)) + 0x40;
        }

        for (iFile=0;iFile<iNbFileWrite;iFile++)
        {
            struct FileToWriteInfoItem* pFileToWriteInfoItem = (struct FileToWriteInfoItem*)
                                  GetItemPtrArrayExpanding(pEL->pAE_FileToWrite,iFile);

            sprintf(buf_list_file_out + pos_in_list_file_out,"%010u\t%08lx\t%s\n",
                 pFileToWriteInfoItem->size,pFileToWriteInfoItem->crc,
                 ExtractUsablePortionOfFileNameForPack(GetFileNameRemovePrefixIfFound(pFileToWriteInfoItem->FileName,pEL->portion_ignore_pathname)));

            pos_in_list_file_out += (unsigned int)strlen(buf_list_file_out + pos_in_list_file_out);
        }
    }


    unsigned int len_std_out = GetNbItemPtrArrayExpanding(pEL->pAE_StdOut);
    if ((len_std_out != 0) && (pEL->store_std_out_content != 0))
        DumpMemToPack(pEL,"test_info/std_out.txt",GetItemPtrArrayExpanding(pEL->pAE_StdOut,0),len_std_out);

    unsigned int len_std_err = GetNbItemPtrArrayExpanding(pEL->pAE_StdErr);
    if ((len_std_err != 0) && (pEL->store_std_err_content != 0))
        DumpMemToPack(pEL,"test_info/std_err.txt",GetItemPtrArrayExpanding(pEL->pAE_StdErr,0),len_std_err);

    unsigned int len_list_in = GetNbItemPtrArrayExpanding(pEL->pAE_FileReadList);
    if (len_list_in != 0)
        DumpMemToPack(pEL,"test_info/list_file_in.txt",GetItemPtrArrayExpanding(pEL->pAE_FileReadList,0),len_list_in);


    if (pEL->store_list_file_out_content!=0)
    {
        DumpMemToPack(pEL,"test_info/list_file_out.txt",buf_list_file_out,pos_in_list_file_out);
        free(buf_list_file_out);
        buf_list_file_out = NULL;
    }

    size_t len_all_param_list = 0x100;
    int i;
    for (i=0;i<(pEL->argc);i++)
        len_all_param_list += strlen(*(pEL->argv+i)) + 4;

    char* param_list= (char*)malloc(len_all_param_list);

    *param_list=0;
    sprintf(param_list,"%010d\n%010d\n",ret,(pEL->argc));

    for (i=0;i<(pEL->argc);i++)
    {
        const char* curarg=*(pEL->argv+i);
        //strcat(param_list,curarg);
        CopyReworkedArgRemoving(param_list+strlen(param_list),curarg,pEL->portion_ignore_pathname);
        strcat(param_list,"\n");
    }

    DumpMemToPack(pEL,"test_info/command_line.txt",param_list,(unsigned int)strlen(param_list));
    
    *param_list=0;
    for (i=0;i<(pEL->argc);i++)
    {
        size_t j=0;
        int add_quote=0;
        const char* curarg=*(pEL->argv+i);
        while ( * (curarg+j) != 0)
        {            
            char c=(*(curarg+j));
            if (c==' ') add_quote=1;
            j++;
        }
        if (add_quote != 0)
            strcat(param_list,"\"");
        //strcat(param_list,curarg);
        CopyReworkedArgRemoving(param_list+strlen(param_list),curarg,pEL->portion_ignore_pathname);
        if (add_quote != 0)
            strcat(param_list,"\"");
        
        if ((i+1)!=(pEL->argc))
            strcat(param_list," ");
    }

    /* command_line_synth has moved */
    /*
    DumpMemToPack(pEL,"test_info/command_line_synth_after.txt",param_list,(unsigned int)strlen(param_list));
    */

    if (pEL->zf != NULL)
        zipClose(pEL->zf,param_list);
    pEL->zf=NULL;

    free(param_list);

    FreeExecutionLogging(privateLoggerPtr);
}

void DoFileReadWork(struct ExecutionLogging* pEL,const char* name)
{
    if (SearchFileInFileToWriteArray(pEL,name,NULL)==0)
        if (SearchFileInFileToReadArray(pEL,name,NULL)==0)
        {
            unsigned int size=0;
            unsigned long crc=0;
            int file_found=0;                

            if (pEL->store_file_in_content != 0)
              file_found = DumpFileToPack(pEL,name,"src/",&size,&crc);
            
            if ((pEL->store_file_in_content == 0) && (pEL->store_list_file_in_content != 0))
              file_found = ComputeFileCrc(name,&size,&crc);

            if ((pEL->store_file_in_content == 0) && (pEL->store_list_file_in_content == 0))
              file_found = ComputeFileCrc(name,NULL,NULL);

            if (file_found != 0)
              AddFileInFileToReadArray(pEL,name);

            if ((pEL->store_list_file_in_content != 0))
            {
                char* szBuf = (char*)malloc(strlen(name) + 0x40);
                if (szBuf != NULL)
                {
                    sprintf(szBuf,"%010u\t%08lx\t%s\n",size,crc,
                            ExtractUsablePortionOfFileNameForPack(GetFileNameRemovePrefixIfFound(name,pEL->portion_ignore_pathname)));
                    FlushOutData(szBuf,strlen(szBuf),pEL->pAE_FileReadList);
                    free(szBuf);
                }
            }
        }
}

void DoFileWriteWork(struct ExecutionLogging* pEL,const char* name)
{
        if (SearchFileInFileToWriteArray(pEL,name,NULL)==0)
        {
            AddFileInFileToWriteArray(pEL,name);
        }
}

/*
 *  In the Unitex context, MODE is one of these value :
 *   - "rb" : open the file in read only mode
 *   - "wb" : open the file in write only mode (the previous file is erased, if exist)
 *   - "r+b" or "ab" : open the file in read and write mode ("ab" mean append)
 */

void AnalyseMode(const char*MODE,int* p_file_read,int* p_file_write)
{
    int file_read=0;
    int file_write=0;

    if (((*MODE)=='r') && ((*(MODE+1))=='b'))
        file_read=1;

    if (((*MODE)=='w') && ((*(MODE+1))=='b'))
        file_write=1;

    if (((*MODE)=='r') && (((*(MODE+1))=='+') || ((*(MODE+1))=='w')))
        file_read=file_write=1;

    if (((*MODE)=='a'))
        file_read=file_write=1;

    if (p_file_read!=NULL)
        *p_file_read=file_read;

    if (p_file_write!=NULL)
        *p_file_write=file_write;
}

void ABSTRACT_CALLBACK_UNITEX UniLogger_before_af_fopen(const char* name,const char* MODE,void* privateLoggerPtr)
{
    struct ExecutionLogging* pEL = GetExecutionLogging(privateLoggerPtr);
    /* pEL can be NULL if we are not executing a tool */
    if (pEL == NULL)
        return;


    int file_read=0;
    int file_write=0;
    AnalyseMode(MODE,&file_read,&file_write);

    if (file_read==1)
    {
        DoFileReadWork(pEL,name);
    }
}


void ABSTRACT_CALLBACK_UNITEX UniLogger_after_af_fopen(const char* name,const char* MODE,ABSTRACTFILE* af,void* privateLoggerPtr)
{
    struct ExecutionLogging* pEL = GetExecutionLogging(privateLoggerPtr);
    if (pEL == NULL)
        return;

    int file_read=0;
    int file_write=0;
    AnalyseMode(MODE,&file_read,&file_write);
    /*
    if ((file_read==1) && (af != NULL))
    {
        DoFileReadWork(pEL,name);
    }
    */
    if ((file_write==1) && (af != NULL))
    {
        DoFileWriteWork(pEL,name);
    }
}

void ABSTRACT_CALLBACK_UNITEX UniLogger_before_af_rename(const char* name1,const char*,void* privateLoggerPtr)
{
    struct ExecutionLogging* pEL = GetExecutionLogging(privateLoggerPtr);
    if (pEL == NULL)
        return;

    DoFileReadWork(pEL,name1);
}

void ABSTRACT_CALLBACK_UNITEX UniLogger_after_af_rename(const char* name1,const char* name2,int result,void* privateLoggerPtr)
{
    struct ExecutionLogging* pEL = GetExecutionLogging(privateLoggerPtr);
    if (pEL == NULL)
        return;

    if (result==0)
    {
        DoFileWriteWork(pEL,name2);
        RemoveFileFileOnWriteArray(pEL,name1);
    }
}

void ABSTRACT_CALLBACK_UNITEX UniLogger_before_af_copy(const char* name1,const char*,void* privateLoggerPtr)
{
    struct ExecutionLogging* pEL = GetExecutionLogging(privateLoggerPtr);
    if (pEL == NULL)
        return;

    DoFileReadWork(pEL,name1);
}

void ABSTRACT_CALLBACK_UNITEX UniLogger_after_af_copy(const char*,const char* name2,int result,void* privateLoggerPtr)
{
    struct ExecutionLogging* pEL = GetExecutionLogging(privateLoggerPtr);

    if (pEL == NULL)
        return;

    if (result==0)
    {
        DoFileWriteWork(pEL,name2);
    }
}


void ABSTRACT_CALLBACK_UNITEX UniLogger_after_af_remove(const char* name,int result,void* privateLoggerPtr)
{
    struct ExecutionLogging* pEL = GetExecutionLogging(privateLoggerPtr);
    if (pEL == NULL)
        return;

    if (result==0)
        RemoveFileFileOnWriteArray(pEL,name);
}



void ABSTRACT_CALLBACK_UNITEX UniLogger_LogOutWrite(const void*Buf, size_t size,void* privateLoggerPtr)
{
    struct ExecutionLogging* pEL = GetExecutionLogging(privateLoggerPtr);
    if (pEL == NULL)
        return;

    if (pEL!=NULL)
        FlushOutData(Buf,size,pEL->pAE_StdOut);
}

void ABSTRACT_CALLBACK_UNITEX UniLogger_LogErrWrite(const void*Buf, size_t size,void* privateLoggerPtr)
{
    struct ExecutionLogging* pEL = GetExecutionLogging(privateLoggerPtr);
    if (pEL == NULL)
        return;

    FlushOutData(Buf,size,pEL->pAE_StdErr);
}

const t_logger_func_array logger_func_array =
{
    sizeof(t_logger_func_array), //unsigned int size_struct;

    NULL, // t_fnc_Init_Logger fnc_Init_Logger;
    NULL, // t_fnc_Uninit_Logger fnc_Uninit_Logger;

    UniLogger_before_af_fopen,//t_fnc_before_af_fopen fnc_before_af_fopen;
    UniLogger_after_af_fopen,//t_fnc_after_af_fopen  fnc_after_af_fopen;

    NULL, //t_fnc_before_af_fclose fnc_before_af_fclose;
    NULL, //t_fnc_after_af_fclose fnc_after_af_fclose;

    UniLogger_before_af_rename, //t_fnc_before_af_rename fnc_before_af_rename;
    UniLogger_after_af_rename, //t_fnc_after_af_rename fnc_after_af_rename;

    UniLogger_before_af_copy, //t_fnc_before_af_copy fnc_before_af_copy;
    UniLogger_after_af_copy, //t_fnc_after_af_copy fnc_after_af_copy;

    NULL, //t_fnc_before_af_remove fnc_before_af_remove;
    UniLogger_after_af_remove, //t_fnc_after_af_remove fnc_after_af_remove;

    UniLogger_before_calling_tool, //t_fnc_before_calling_tool fnc_before_calling_tool;
    UniLogger_after_calling_tool, //t_fnc_after_calling_tool fnc_after_calling_tool;

    UniLogger_LogOutWrite,//t_fnc_LogOutWrite fnc_LogOutWrite;
    UniLogger_LogErrWrite//t_fnc_LogErrWrite fnc_LogErrWrite;
} ;


/****************************************/


UNITEX_FUNC int UNITEX_CALL AddActivityLogger(struct UniLoggerSpace *p_ule)
{
    struct ActivityLoggerPrivateData* pALPD;
    pALPD = (struct ActivityLoggerPrivateData*)malloc(sizeof(struct ActivityLoggerPrivateData));
    if (pALPD == NULL)
        return 0;
    pALPD -> pTlsSlot = SyncBuildTls();
    
    if (pALPD -> pTlsSlot == NULL)
    {
        free(pALPD);
        return 0;
    }

    SyncTlsSetValue(pALPD -> pTlsSlot,NULL);

    pALPD -> pMutexLog = SyncBuildMutex();

    p_ule -> privateUnloggerData = (void*)pALPD;
    return AddLoggerInfo(&logger_func_array,p_ule);
}

UNITEX_FUNC int UNITEX_CALL RemoveActivityLogger(struct UniLoggerSpace *p_ule)
{
    struct ActivityLoggerPrivateData* pALPD = (struct ActivityLoggerPrivateData*)p_ule -> privateUnloggerData;
    SyncDeleteTls(pALPD->pTlsSlot);
    SyncDeleteMutex(pALPD->pMutexLog);
    free(pALPD);
    return RemoveLoggerInfo(&logger_func_array,p_ule);
}

} // namespace logger
} // namespace unitex

#endif
