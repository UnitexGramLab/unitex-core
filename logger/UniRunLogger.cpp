 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


/* */
/*
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "Copyright.h"

#include "ActivityLoggerPlugCallback.h"
#include "File.h"

#include "FilePack.h"
#include "FileUnPack.h"
#include "FilePackCrc32.h"


#include "UniLogger.h"
#include "UniRunLogger.h"
#include "DirLogger.h"
#include "SyncLogger.h"
#include "ReworkArg.h"

#include "Error.h"

#include "AbstractFilePlugCallback.h"

class InstallLoggerForRunner
{
public:
    InstallLoggerForRunner(int real_content_in_log);
    ~InstallLoggerForRunner();

    int SelectNextLogName(const char*LogName,const char*portion_ignore_pathname);
private:
    struct UniLoggerSpace ule;
    int init_done;
};


InstallLoggerForRunner::InstallLoggerForRunner(int real_content_in_log)
{
    init_done = 0;

    /* we want "mini log" with only list */

    ule.privateUnloggerData = NULL;
    ule.szPathLog = NULL;
    ule.store_file_out_content = real_content_in_log;
    ule.store_list_file_out_content = 1;

    ule.store_file_in_content = real_content_in_log;
    ule.store_list_file_in_content = real_content_in_log;

    /* we dont want "auto generate" log */
    ule.auto_increment_logfilename = 0;

    if (AddActivityLogger(&ule) != 0)
        init_done = 1;
}

int InstallLoggerForRunner::SelectNextLogName(const char*LogName,const char*portion_ignore_pathname)
{
    return ::SelectNextLogName(&ule,LogName,portion_ignore_pathname);
}

InstallLoggerForRunner::~InstallLoggerForRunner()
{
    if (init_done != 0)
    {
        RemoveActivityLogger(&ule);
    }
}

//InstallLoggerForRunner InstallLoggerForRunnerSingleton;

/****************************************************************************************/

struct dir_list_for_clean_item {
    char*dirname;
    struct dir_list_for_clean_item *next;
};

struct dir_list_for_clean {    
    struct dir_list_for_clean_item *first;
} ;

struct dir_list_for_clean* build_list_dir_for_clean()
{
    struct dir_list_for_clean* ret = (struct dir_list_for_clean*)malloc(sizeof(struct dir_list_for_clean));
    if (ret != NULL)
    {
        ret -> first = NULL;
    }
    return ret;
}

void clean_list_dir_for_clean(struct dir_list_for_clean* pdlfc)
{
    if (pdlfc != NULL)
    {
        struct dir_list_for_clean_item *browse = pdlfc ->first;
        while (browse != NULL)
        {
            struct dir_list_for_clean_item *tmp = browse;
            browse = browse -> next;
            free(tmp->dirname);
            free(tmp);
        }
        free(pdlfc);
    }
}

int add_dir_in_log_for_clean(struct dir_list_for_clean* pdlfc,const char*dirname)
{
    if (pdlfc == NULL)
        return 0;
    else
    {
        struct dir_list_for_clean_item * pdlfci;
        pdlfci = (struct dir_list_for_clean_item *)malloc(sizeof(struct dir_list_for_clean_item));
        if (pdlfci == NULL)
            return 0;
        pdlfci->dirname = strdup(dirname);
        pdlfci->next = NULL;
        if (pdlfci->dirname == NULL)
        {
            free(pdlfci);
            return 0;
        }

        pdlfci->next = pdlfc->first;
        pdlfc->first = pdlfci;
    
        return 1;
    }
}

int mkDirPortable_log_for_clean(const char*dirname,struct dir_list_for_clean* pdlfc)
{
    int ret=mkDirPortable(dirname);
    if ((ret == 0) && (pdlfc != NULL))
    {
        add_dir_in_log_for_clean(pdlfc,dirname);
    }
    return ret;
}

int rm_log_dir_portable(struct dir_list_for_clean* pdlfc)
{
    if (pdlfc == NULL)
        return 0;
    else
    {
        struct dir_list_for_clean_item *browse = pdlfc ->first;
        while (browse != NULL)
        {
            rmDirPortable(browse->dirname);
            browse = browse -> next;
        }
        return 1;
    }
}

int mkdir_recursive(const char*dirname,int iContainOnlyDirName,struct dir_list_for_clean* pdlfc)
{
  if ((*dirname)!='\0')
  {
      char* dupname=strdup(dirname);
      char* lpParc = dupname;

      if ((*lpParc)!=0)
          lpParc++;

      while ((*lpParc)!=0)
      {
          if ( ((*(lpParc))=='\\')|| ((*(lpParc))=='/') )
              {
                  char c=*(lpParc);
                  *(lpParc) = '\0';
                  mkDirPortable_log_for_clean (dupname,pdlfc);
                  *(lpParc) = c;
              }

          lpParc++;
      }
      free(dupname);
  }

  int ret = 0;
  if (iContainOnlyDirName != 0)
      ret = mkDirPortable_log_for_clean (dirname,pdlfc);
  return ret;
}

/****************************************************************************************/

#define WRITEBUFFERSIZE 0x8000

char* get_filename_withoutpath_portion(char* fn)
{
    char *p=fn;
    char *filename_withoutpath = fn;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }
    return filename_withoutpath;
}

int do_extracting_currentfile(
    unzFile uf,
    char* write_filename,
    struct dir_list_for_clean* pdlfc,
    const int* popt_extract_without_path,
    int* popt_overwrite,
    const char* password)
{
    char filename_inzip[256];
    char* filename_touse;
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    ABSTRACTFILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info file_info;
    //uLong ratio=0;
    err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip)-1,NULL,0,NULL,0);

    filename_touse = (write_filename != NULL) ? write_filename : filename_inzip;

    if (err!=UNZ_OK)
    {
        //printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
    {
        //printf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_touse;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    if ((*filename_withoutpath)!='\0')
    {
        const char* write_filename;
        int skip=0;

        if ((*popt_extract_without_path)==0)
            write_filename = filename_touse;
        else
            write_filename = filename_withoutpath;

        err = unzOpenCurrentFilePassword(uf,password);
        if (err!=UNZ_OK)
        {
            //printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
            free(buf);
            return UNZ_INTERNALERROR;
        }

        if (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            skip = 1;
        }

        if ((skip==0) && (err==UNZ_OK))
        {
            fout=af_fopen_unlogged(write_filename,"wb");

            /* some zipfile don't contain directory alone before file */
            if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(char*)filename_touse))
              if (is_filename_in_abstract_file_space(write_filename)==0)
                {
                    mkdir_recursive(write_filename,0,pdlfc);
                    fout=af_fopen_unlogged(write_filename,"wb");
                }
        }

        if (fout!=NULL)
        {
            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    break;
                }
                if (err>0)
                    if (af_fwrite(buf,err,1,fout)!=1)
                    {
                        err=UNZ_ERRNO;
                        break;
                    }
            }
            while (err>0);
            if (fout)
                    af_fclose_unlogged(fout);
/*
            if (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
                                 */
        }

        if (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK)
            {
                //printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    free(buf);
    return err;
}



int do_extracting_currentfile_memory(
    unzFile uf,
    void** ptr,
    size_t *size_file,
    const char*password)
{
    int err=UNZ_OK;
    unz_file_info file_info;
    //uLong ratio=0;
    char filename_inzip[256];
    err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip)-1,NULL,0,NULL,0);
    if (err!=UNZ_OK)
    {
        //printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    *ptr = malloc(file_info.uncompressed_size+2);
    if ((*ptr) == NULL)
        return UNZ_INTERNALERROR;
    *size_file = file_info.uncompressed_size;



    err = unzOpenCurrentFilePassword(uf,password);
    if (err==UNZ_OK)
            if (unzReadCurrentFile(uf,*ptr,(unsigned int)file_info.uncompressed_size) != ((int)file_info.uncompressed_size))
                err = UNZ_BADZIPFILE;

    if (err==UNZ_OK)
        err = unzCloseCurrentFile (uf);

    if (err!=UNZ_OK)
    {
        *ptr = NULL;
        free(*ptr);
        *size_file = 0;
    }
    else
    {   /* we append two '\0' at end of file */
        *(((char*)(*ptr)) + file_info.uncompressed_size) = 0;
        *(((char*)(*ptr)) + file_info.uncompressed_size+1) = 0;
    }
    return err;
}


char* GetNextLine(char** pLine)
{
    if ((*pLine) == NULL)
        return NULL;
    char *ret= *pLine;

    for (;;)
    {
        if (((**pLine) == 0x0a) || ((**pLine) == 0x0d))
        {
            while (((**pLine) == 0x0a) || ((**pLine) == 0x0d))
            {
                (**pLine) = 0;
                (*pLine)++;
            }
            return ret;
        }
        if ((**pLine) == 0)
        {
            *pLine = NULL;
            return ret;
        }
        (*pLine)++;
    }
}


unsigned int get_max_nb_line_in_file(char*buf,size_t size_file)
{
      size_t walk_arglist;
      unsigned int nb_max_arg_for_allocwide = 1;
      for (walk_arglist=0;walk_arglist<size_file;walk_arglist++)
      {
          char c=*(buf+walk_arglist);
          if ((c==0x0d) || (c==0x0a))
              nb_max_arg_for_allocwide ++;
          if (c==0)
              break;
      }
      return nb_max_arg_for_allocwide;
}

struct ListFile_entry {
    char* filename;
    uLong crc;
    uLong size;
} ;

struct ListFile {
    unsigned int iNbFile;
    unsigned int iNbAllocated;
    struct ListFile_entry * p_ListFile_entry;
};

struct ListFile* AllocListFile(unsigned int iNbAlloc)
{
    struct ListFile_entry * p_ListFile_entry = 
        (struct ListFile_entry *)malloc(sizeof(struct ListFile_entry)*(iNbAlloc+1));
    if (p_ListFile_entry == NULL)
        return NULL;
    struct ListFile* p_ListFile = (struct ListFile*)malloc(sizeof(struct ListFile));
    if (p_ListFile == NULL)
    {
        free(p_ListFile_entry);
        return NULL;
    }
    p_ListFile->p_ListFile_entry = p_ListFile_entry;
    p_ListFile->iNbAllocated = iNbAlloc;
    p_ListFile->iNbFile = 0;

    return p_ListFile ;
}

void FreeListFile(struct ListFile* p_ListFile)
{
  unsigned int walk;
  if (p_ListFile != NULL)
  {
      for (walk=0;walk < p_ListFile->iNbFile;walk++)
          free(((p_ListFile->p_ListFile_entry) + walk) -> filename);

      free(p_ListFile->p_ListFile_entry);
      free(p_ListFile);
  }
}

struct ListFile* ReadListFile(char* content,size_t list_out)
{
    struct ListFile* p_ListFile;
    unsigned int max_nb_line = get_max_nb_line_in_file(content,list_out);

    p_ListFile = AllocListFile(max_nb_line+1);
    if (p_ListFile == NULL)
        return NULL;

    char *walk_list = content;
    unsigned int walk;
    for (walk=0;(walk<max_nb_line);walk++)
    {
        char *curLine = GetNextLine(&walk_list);
        if (curLine == NULL)
            break;
        unsigned long lSize=0;
        unsigned long lCrc=0;
        char*name=NULL;
        name=(char*)malloc(strlen(curLine)+1);
        name[0] = 0;
        sscanf(curLine,"%lu\t%lx\t%s",&lSize,&lCrc,name);

        if (name[0] != 0)
        {
            ((p_ListFile->p_ListFile_entry) + (p_ListFile->iNbFile)) -> crc = lCrc;
            ((p_ListFile->p_ListFile_entry) + (p_ListFile->iNbFile)) -> size = lSize;
            ((p_ListFile->p_ListFile_entry) + (p_ListFile->iNbFile)) -> filename = (name);
            (p_ListFile->iNbFile)++;
        }
        else
        {
            free(name);
            break;
        }
    }
    return p_ListFile;
}


void CombineRunPathOnPathName(char* filename_to_write,const char*FileRunPath,int need_add_dir_sep,const char*filename_original)
{
      strcpy(filename_to_write,FileRunPath);
      if (need_add_dir_sep != 0)
          strcat(filename_to_write,"/");
      strcat(filename_to_write,filename_original);

      char * browse=filename_to_write;
      while ((*browse) != '\0')
      {
          if (((*browse) == '\\') || ((*browse) == '/'))
              *browse=PATH_SEPARATOR_CHAR;
          browse++;
      }
}

int CompareFileName(const char* n1,const char*n2)
{
    for (;;)
    {
        if ((*n1) == 0)
            return ((*n2) == 0) ? 0 : 1;
        if ((*n2) == 0)
            return 1;
        char c1 = *n1;
        char c2 = *n2;
        if (c1 == '\\') c1='/';
        if (c2 == '\\') c2='/';
        if (c1 != c2)
            return 1;
        n1++;
        n2++;
    }
}


int AddMsgToSummaryBuf(const char*msgThis,char**summaryInfo)
{
  size_t lenMsg = strlen(msgThis);
  if (summaryInfo != NULL)
  {
      if (*summaryInfo == NULL)
      {
          *summaryInfo = (char*)malloc(lenMsg+1);
          if ((*summaryInfo) != NULL)
            memcpy(*summaryInfo,msgThis,lenMsg+1);
          return ((*summaryInfo) == NULL) ? 0 : 1;
      }
      else
      {
          size_t pos = strlen(*summaryInfo);
          char*newbuf = (char*)realloc(*summaryInfo,pos+(lenMsg+1));
          if (newbuf != NULL)
          {
              *summaryInfo = newbuf;
              memcpy(newbuf+pos,msgThis,lenMsg+1);
          }
          return ((newbuf) == NULL) ? 0 : 1;
      }
  }
  else
      return 0;
}

UNITEX_FUNC int UNITEX_CALL RunLogParam(const char* LogNameRead,const char* FileRunPath,const char* LogNameWrite,int clean_file,
                                        int real_content_in_log,
                                        const char* /* LocationUnfoundVirtualRessource */,
                                        char** summaryInfo,
                                        int *pReturn,unsigned int*pTimeElapsed,
                                        Exec_status* p_exec_status)
{
    zlib_filefunc_def zlib_filefunc;
    fill_afopen_filefunc(&zlib_filefunc);

    char* command_line_buf = NULL;
    size_t size_command_line = 0;

    char* list_out_newlog_buf = NULL;
    size_t size_list_out_newlog = 0;

    char* list_out_buf = NULL;
    size_t size_list_out = 0;

    struct ListFile* list_file_out_newlog = NULL;
    struct ListFile* list_file_out = NULL;
    struct ListFile* list_file_in = NULL;

    struct dir_list_for_clean* pdlfc = NULL;
    Exec_status exec_status = EXEC_NOTRUN;

    InstallLoggerForRunner InstallLoggerForRunnerSingleton(real_content_in_log);

    if (FileRunPath==NULL)
        FileRunPath="";
    int need_add_dir_sep = 0;
    size_t len_RunPath = strlen(FileRunPath);
    if (len_RunPath>0)
        if (((*(FileRunPath+len_RunPath-1)) != '/') && ((*(FileRunPath+len_RunPath-1)) != '\\'))
            need_add_dir_sep=1;

    unzFile uf = unzOpen2(LogNameRead,&zlib_filefunc);
    if (uf == NULL)
    {
        if (p_exec_status!=NULL)
            *p_exec_status = exec_status;
        return -1;
    }

    uLong i;
    unz_global_info gi;
    int err;
    
    
    err = unzGetGlobalInfo (uf,&gi);
    if (err==UNZ_OK)
    {
      list_file_in = AllocListFile(gi.number_entry);
      pdlfc = build_list_dir_for_clean();
      int nb_listfile_in=0;

      for (i=0;i<gi.number_entry;i++)
      {
          char filename_inzip[256];
          unz_file_info file_info;
          err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip)-1,NULL,0,NULL,0);

          if (memcmp(filename_inzip,"src/",4)==0)
          {
              char* filename_to_extract = filename_inzip + 4;
              /* uncompress to FileRunPath */
              //if (DoUncompressToFile(

              int opt_extract_without_path = 0;
              int opt_overwrite = 1;
              char filename_to_write[256];

              CombineRunPathOnPathName(filename_to_write,FileRunPath,need_add_dir_sep,filename_to_extract);
              /*
              strcpy(filename_to_write,FileRunPath);
              if (need_add_dir_sep != 0)
                  strcat(filename_to_write,"/");
              strcat(filename_to_write,filename_to_extract);

              char * browse=filename_to_write;
              while ((*browse) != '\0')
              {
                  if (((*browse) == '\\') || ((*browse) == '/'))
                      *browse=PATH_SEPARATOR_CHAR;
                  browse++;
              }
*/
              int extres = do_extracting_currentfile(uf,filename_to_write,pdlfc,&opt_extract_without_path,&opt_overwrite,NULL);

              if (extres == UNZ_OK)
              {
                  ((list_file_in->p_ListFile_entry) + nb_listfile_in) -> filename = strdup(filename_to_extract);
                  ((list_file_in->p_ListFile_entry) + nb_listfile_in) -> crc = file_info.crc;
                  ((list_file_in->p_ListFile_entry) + nb_listfile_in) -> size = file_info.uncompressed_size;
                  nb_listfile_in++;
                  list_file_in->iNbFile = nb_listfile_in;
              }
          }
/*
          if (strcmp(filename_inzip,"test_info/list_file_in.txt")==0)
          {
              do_extracting_currentfile_memory(uf,(void**)&list_in_buf,&size_list_in,NULL);
          }
*/
          if (strcmp(filename_inzip,"test_info/list_file_out.txt")==0)
          {
              do_extracting_currentfile_memory(uf,(void**)&list_out_buf,&size_list_out,NULL);
              list_file_out = ReadListFile(list_out_buf,size_list_out);
          }

          if (strcmp(filename_inzip,"test_info/command_line.txt")==0)
          {
              do_extracting_currentfile_memory(uf,(void**)&command_line_buf,&size_command_line,NULL);
          }

          if ((i+1)<gi.number_entry)
            {
                err = unzGoToNextFile(uf);
                if (err!=UNZ_OK)
                {
                    error("error %d with zipfile in unzGoToNextFile\n",err);
                    break;
                }
            }
      }

      //size_t walk_arglist;
      int nb_max_arg_for_allocwide = get_max_nb_line_in_file(command_line_buf,size_command_line);

      char* buf_arg = (char*)malloc(size_command_line + (nb_max_arg_for_allocwide * len_RunPath) + 0x100);
      char** argv_log = (char**)malloc(sizeof(char*) * (nb_max_arg_for_allocwide+2+1));

      char* buf_arg_reworked = (char*)malloc(size_command_line + (nb_max_arg_for_allocwide * len_RunPath) + 0x100);
      const char** argv_log_reworked = (const char**)malloc(sizeof(char*) * (nb_max_arg_for_allocwide+2+1));

      char *walk_arg = command_line_buf;
      char *cur_command_line_entry;

      cur_command_line_entry = GetNextLine(&walk_arg);
      //int retvalue = atoi(cur_command_line_entry);

      cur_command_line_entry = GetNextLine(&walk_arg);
      int nb_arg = 0;
      if (cur_command_line_entry != NULL)
        nb_arg = atoi(cur_command_line_entry);

      int walk;
      int argc_log=0;
      for (walk=0;(walk<nb_max_arg_for_allocwide) && (walk<nb_arg);walk++)
      {
          *(argv_log+argc_log) = GetNextLine(&walk_arg);
          if (*(argv_log+argc_log) != NULL)
              argc_log++;
          else
              break;
      }

      char* next_buf_arg_reworked = buf_arg_reworked;
      *(argv_log_reworked+0) = "UnitexTool";
      for (walk=0;walk<argc_log;walk++)
      {
          reworkCommandLineAddPrefix(next_buf_arg_reworked,*(argv_log+walk),FileRunPath);
          *(argv_log_reworked+walk+1) = next_buf_arg_reworked;
          next_buf_arg_reworked += strlen(next_buf_arg_reworked) + 1;
          //u_printf("%d\n'%s'\n'%s'\n\n",walk,*(argv_log+walk),*(argv_log_reworked+walk+1));
      }

      argc_log+=0;

      unzClose(uf);
      /* we have done extracting */

      //char *buf_reworked=malloc(size_command_line + 

      if (LogNameWrite != NULL)
          InstallLoggerForRunnerSingleton.SelectNextLogName(LogNameWrite,FileRunPath);

      unsigned int walk_list_out;
      if (list_file_out != NULL)
          for (walk_list_out = 0;walk_list_out < list_file_out->iNbFile;walk_list_out++)
          {
              const char* filename_out = ((list_file_out->p_ListFile_entry) + walk_list_out) -> filename;
              const char* filename_out_usable = get_filename_to_copy(filename_out);
              char filename_to_be_written[256];
              CombineRunPathOnPathName(filename_to_be_written,FileRunPath,need_add_dir_sep,filename_out_usable);
              if (is_filename_in_abstract_file_space(filename_to_be_written) == 0)
                mkdir_recursive(filename_to_be_written,0,pdlfc);
          }


      hTimeElasped htm = NULL;
      if (pTimeElapsed != NULL)
        htm = SyncBuidTimeMarkerObject();
      int ret_tool = main_UnitexTool_C(argc_log+1,(char**)argv_log_reworked);
      if (htm != NULL)
          *pTimeElapsed = SyncGetMSecElapsed(htm);

      if (pReturn != NULL)
          *pReturn = ret_tool;
      

      if (LogNameWrite!=NULL)
      {
        unzFile ufNewLog = unzOpen2(LogNameWrite,&zlib_filefunc);

        if (ufNewLog != NULL)
        {
            uLong i;
            unz_global_info gi;
            int err;
            
            err = unzGetGlobalInfo (ufNewLog,&gi);
            if (err==UNZ_OK)
            {
              //list_file_in = AllocListFile(gi.number_entry);

              for (i=0;i<gi.number_entry;i++)
              {
                  char filename_inzip[256];
                  unz_file_info file_info;
                  err = unzGetCurrentFileInfo(ufNewLog,&file_info,filename_inzip,sizeof(filename_inzip)-1,NULL,0,NULL,0);

                  if (strcmp(filename_inzip,"test_info/list_file_out.txt")==0)
                  {
                      do_extracting_currentfile_memory(ufNewLog,(void**)&list_out_newlog_buf,&size_list_out_newlog,NULL);
                      list_file_out_newlog = ReadListFile(list_out_newlog_buf,size_list_out_newlog);
                  }


                  if ((i+1)<gi.number_entry)
                    {
                        err = unzGoToNextFile(ufNewLog);
                        if (err!=UNZ_OK)
                        {
                            error("error %d with zipfile in unzGoToNextFile\n",err);
                            break;
                        }
                    }
              }
            }
            unzClose(ufNewLog);
        }
      }

      
      if ((clean_file != 0) && (list_file_out_newlog != NULL))
      {
          unsigned int i,j;
          if (list_file_in != NULL)
              for (i=0;i<list_file_in->iNbFile;i++)
              {
                  const char* filename = (((list_file_in->p_ListFile_entry)+i)->filename);
                  char filename_written[256];
                  CombineRunPathOnPathName(filename_written,FileRunPath,need_add_dir_sep,filename);
                  af_remove_unlogged(filename_written);
              }


          if (list_file_out_newlog != NULL)
              for (j=0;j<list_file_out_newlog->iNbFile;j++)
              {
                  const char* filename = (((list_file_out_newlog->p_ListFile_entry)+j)->filename);
                  char filename_written[256];
                  CombineRunPathOnPathName(filename_written,FileRunPath,need_add_dir_sep,get_filename_to_copy(filename));
                  af_remove_unlogged(filename_written);
              }

          rm_log_dir_portable(pdlfc);
      }

      exec_status = EXEC_COMPARE_OK;

      if ((list_file_out_newlog != NULL) && (list_file_out != NULL))
      {
          unsigned int i,j;
          int error_compare=0;
          for (i=0;i<list_file_out->iNbFile;i++)
          {
              char msgThis[0x400]="";
              const char* fn_compare_original_log = get_filename_to_copy(((list_file_out->p_ListFile_entry)+i)->filename);
              for (j=0;j<list_file_out_newlog->iNbFile;j++)
              {
                  const char* fn_compare_new_log = get_filename_to_copy(((list_file_out_newlog->p_ListFile_entry)+j)->filename);
                  if (CompareFileName(fn_compare_original_log,fn_compare_new_log)==0)
                  {
                      if (((((list_file_out->p_ListFile_entry)+i)->size) != (((list_file_out_newlog->p_ListFile_entry)+j)->size)) ||
                          ((((list_file_out->p_ListFile_entry)+i)->crc) != (((list_file_out_newlog->p_ListFile_entry)+j)->crc)))
                      {
                          error_compare = 1;
                          sprintf(msgThis,"-> log error : bad compare %s\n",fn_compare_original_log);
                          exec_status = EXEC_COMPARE_ERROR;
                      }
                      break;
                  }
              }
              if (j == list_file_out_newlog->iNbFile)
              {
                  error_compare = 1;
                  if ((((list_file_out->p_ListFile_entry)+i)->size) == 0)
                  {
                      sprintf(msgThis,"-> log warning : empty file %s not found in new log\n",fn_compare_original_log);
                      if (exec_status != EXEC_COMPARE_ERROR)
                          exec_status = EXEC_COMPARE_WARNING;
                  }
                  else
                  {
                      sprintf(msgThis,"-> log error : file %s not found in new log\n",fn_compare_original_log);
                      exec_status = EXEC_COMPARE_ERROR;
                  }
              }

              if (msgThis[0] != 0)
              {
                  AddMsgToSummaryBuf(msgThis,summaryInfo);
              }
              
          }

          {
              char msgEnd[0x400];
              char msgTime[0x80];
              if (pTimeElapsed != NULL)
                  sprintf(msgTime,"%u msec, ",*pTimeElapsed);
              else
                  msgTime[0]=0;
              
              if (error_compare == 0)
                  sprintf(msgEnd,"--> LOG COMPARE: good news: %snew log result is compatible with previous log for %s and %s\n\n",msgTime,LogNameRead,LogNameWrite);
              else
                  sprintf(msgEnd,"--> LOG COMPARE: bad news: %snew log result is NOT compatible with previous log for %s and %s\n\n",msgTime,LogNameRead,LogNameWrite);
              AddMsgToSummaryBuf(msgEnd,summaryInfo);
          }
      }

      free(buf_arg);
      free(argv_log);

      free(buf_arg_reworked);
      free(argv_log_reworked);      


      if (command_line_buf != NULL)
          free(command_line_buf);

      if (list_out_buf != NULL)
          free(list_out_buf);

      if (list_out_newlog_buf != NULL)
          free(list_out_newlog_buf);

      FreeListFile(list_file_out_newlog);
      FreeListFile(list_file_out);
      FreeListFile(list_file_in);
    }

    clean_list_dir_for_clean(pdlfc);

    if (p_exec_status!=NULL)
        *p_exec_status = exec_status;

    return 0;
}


UNITEX_FUNC int UNITEX_CALL RunLog(const char* LogNameRead,const char* FileRunPath,const char* LogNameWrite)
{
    char*summary=NULL;
    int ret= RunLogParam(LogNameRead,FileRunPath,LogNameWrite,1,1,NULL,&summary,NULL,NULL,NULL);
    if (summary!=NULL)
    {
        u_printf("%s",summary);
        free(summary);
    }
    return ret;
}






static int IsNumber(char c)
{
  return ((c>='0') && (c<='9'));
}

int IncNumberInName(char* lpFn,int fNeedAbsolute,unsigned long dwStepIncr,unsigned long* prev_value)
{
int i,iBegName,iPoint,iEndName,iln,iBegChif,iEndChif;
int fFoundChif=0;
int fForce=0;
int fInExt;
int fExpand = 0;

    iBegName = 0;
    iBegChif = iEndChif = 0;
    fInExt = 0;

    lpFn += get_filename_withoutpath_position(lpFn);
    iEndName = iln = (int)strlen(lpFn);

    if (iln == 0) return 0;
    for (i=iln-1;i>=0;i--)
      {
      if ((*(lpFn+i)) == '.') iEndName = i;
      }
    iPoint = iEndName;

    //if ((!fFoundChif) && ((*(lpFn+iPoint)) == '.'))
    if ((*(lpFn+iPoint)) == '.') // search number in ext if there is ext
      {
      iBegName = iPoint + 1;
      iEndName = iln;
      for (i=iEndName-1;i>=iBegName;i--)
        if (IsNumber(*(lpFn+i)))
        {
          for (iBegChif=iEndChif=i;iBegChif>0;iBegChif--)
            if (!IsNumber(*(lpFn+iBegChif-1))) break;
          break;
        }
      fInExt = fFoundChif = (i>=iBegName) ;
      }

    if (!fFoundChif)
      {
      iBegName = 0 ;
      iEndName = iPoint;
      for (i=iEndName-1;i>=iBegName;i--)
        if (IsNumber(*(lpFn+i)))
          {
            for (iBegChif=iEndChif=i;iBegChif>0;iBegChif--)
              if (!IsNumber(*(lpFn+iBegChif-1))) break;
            break;
          }
      fFoundChif = (i>=iBegName) ;
      }

    if ((!fFoundChif) && fNeedAbsolute)
      {
      fForce = 1;/*
      if (iPoint > 7)
        {
          iBegChif = iEndChif = 7 ;
          fExpand = 0;
        }
       else*/
        {
          iBegChif = iPoint;
          iEndChif = iBegChif -1;
          fExpand = 1;
        }
      }

    if (fFoundChif || fForce)
      {
      unsigned long dwNb;
      char szBuf[32];
      char szFmt[16];
      int iLenChif;

      int j;
    if (!fForce)
      {
      fExpand = 1;
      for (j=iBegChif;j<=iEndChif;j++)
        if (*(lpFn+j) != '9') fExpand = 0;
      if ((iEndName-iBegName > (fInExt ? 2 : 7)) && (fExpand))
        {
        fExpand = 0;
        if (((iBegChif > 0) && (!fInExt)) || ((iBegChif > (iPoint+2)) && (fInExt)))
          {
        iBegChif --;
        *(lpFn+iBegChif) = '0';
          }
        }
      }
    if (fExpand)
      {
        for (j = iln;j>iEndChif;j--)
          * (lpFn+j+1) = * (lpFn+j);
        iEndChif++;
      }

    iLenChif=iEndChif+1-iBegChif;
    memcpy(szBuf,lpFn+iBegChif,iLenChif);
    szBuf[iLenChif] = '\0';
    if (fForce) dwNb =0 ;
    else
        dwNb = atol(szBuf) ;

    dwNb+=dwStepIncr;
    if (prev_value!=NULL)
        *prev_value=dwNb;

    {
        char szTest[0x20];
        sprintf(szTest,"%lu",dwNb);
        int iNewLenChif = (int)strlen(szTest);
        if (iNewLenChif > iLenChif)
        {
            memmove(lpFn+iBegChif+iNewLenChif,lpFn+iBegChif+iLenChif,strlen(lpFn+iBegChif+iLenChif)+1);
            iLenChif = iNewLenChif ;
        }
    }

    sprintf(szFmt,("%%0%ulu"),iLenChif);
    sprintf(szBuf,szFmt,dwNb);
    memcpy((lpFn)+iBegChif,szBuf,iLenChif);
    return 1;
      }
  return 0;
}


const char* usage_RunLog =
         "Usage : RunLog [OPTIONS] <ulp>\n"
         "\n"
         "  <ulp>: name of log to run\n"
         "\n"
         "OPTIONS:\n"
         "  -p/--quiet: do not emit message when running\n"
         "  -v/--verbose: emit message when running\n"
         "  -d DIR/--rundir=DIR: path where log is executed\n"
         "  -r newfile.ulp/--result=newfile.ulp : name of result ulp created\n"
         "  -c/--clean: remove work file after execution\n"
         "  -k/--keep: keep work file after execution\n"
         "  -s file.txt/--summary=file.txt: name of summary file with log compare result\n"
         "  -n/--cleanlog: remove result ulp after execution\n"
         "  -l/--keeplog: keep result ulp after execution\n"
         "\n"
         "future:\n"
         "  -i N/--increment=N: increment filename until N\n"
         "  -t N/--thread=N: create N thread\n"
         "  -a N/--random=N: select N time a random log in the list (in each thread)\n"
         "\n"
         "rerun a log.\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_RunLog);
}
const char* optstring_RunLog=":pcd:r:i:s:mvt:lna:";
const struct option_TS lopts_RunLog[]= {
      {"rundir",required_argument_TS,NULL,'d'},
      {"result",required_argument_TS,NULL,'r'},
      {"increment",required_argument_TS,NULL,'i'},
      {"clean",no_argument_TS,NULL,'c'},
      {"keep",no_argument_TS,NULL,'p'},
      {"summary",required_argument_TS,NULL,'s'},
      {"quiet",no_argument_TS,NULL,'m'},
      {"verbose",no_argument_TS,NULL,'v'},
      {"thread",required_argument_TS,NULL,'t'},
      {"cleanlog",no_argument_TS,NULL,'n'},
      {"keeplog",no_argument_TS,NULL,'l'},
      {"increment",required_argument_TS,NULL,'i'},
      {"thread",required_argument_TS,NULL,'t'},
      {"random",required_argument_TS,NULL,'a'},
      {NULL,no_argument_TS,NULL,0}
};


typedef struct {
    char resultulp[0x200];
    char rundir[0x200];
    char summaryfile[0x200];
    const char* runulp;

    int quiet;
    int clean;
    int cleanlog;
    int nb_thread;
    long increment;
    long random;
} RunLog_ctx;


typedef struct {
    const RunLog_ctx * p_RunLog_ctx;
    char*summary;
    unsigned int num_thread;
} RunLog_ThreadData;

void SYNC_CALLBACK_UNITEX DoWork(void* privateDataPtr,unsigned int /*iNbThread*/)
{
    RunLog_ThreadData* p_RunLog_ThreadData =(RunLog_ThreadData*)privateDataPtr;
    const RunLog_ctx* p_RunLog_ctx = p_RunLog_ThreadData->p_RunLog_ctx;

    long i;
    long nb_iteration = (p_RunLog_ctx->random != 0) ? p_RunLog_ctx->random : p_RunLog_ctx->increment;
    if (nb_iteration==0)
        nb_iteration=1;
    unsigned long add_thread = p_RunLog_ThreadData->num_thread * nb_iteration;

    if (p_RunLog_ctx->random != 0)
        srand ( (unsigned int)(time(NULL) + p_RunLog_ThreadData->num_thread) );
    for (i=0;i<nb_iteration;i++)
    {
        char runulp[0x200];
        char resultulp[0x200];
        char rundir[0x200];


        strcpy(runulp,p_RunLog_ctx->runulp);
        strcpy(rundir,p_RunLog_ctx->rundir);
        strcpy(resultulp,p_RunLog_ctx->resultulp);

        if ((p_RunLog_ctx->increment>0) || (p_RunLog_ctx->random>0))
        {
            unsigned long lprev=0;
            unsigned long inc_this = i;
            if (p_RunLog_ctx->random != 0)
            { 
                unsigned int j;
                inc_this = 0;
                for (j=0;j<17;j++)
                    inc_this = inc_this ^ (rand() << j);
                inc_this = inc_this % p_RunLog_ctx->increment;
            }
            IncNumberInName(runulp,1,(unsigned long)inc_this,(i==0) ? (&lprev) : NULL);
            if (p_RunLog_ctx->random != 0)
                lprev=0;
            IncNumberInName(rundir,1,(unsigned long)((i+lprev)+add_thread),NULL);
            IncNumberInName(resultulp,1,(unsigned long)((i+lprev)+add_thread),NULL);
        }

        unsigned int time_elasped=0;
        Exec_status exec_statuc;
        RunLogParam(runulp,rundir,resultulp,
                              p_RunLog_ctx->clean,(p_RunLog_ctx->cleanlog==1) ? 0 : 1,
                              NULL,&p_RunLog_ThreadData->summary,NULL,&time_elasped,&exec_statuc);

        {
        char resume[0x400];
        sprintf(resume,"resume : thread %u , run %s on %s to %s %u\n",p_RunLog_ThreadData->num_thread,runulp,rundir,resultulp,add_thread);
        puts(resume);
        }
        if (p_RunLog_ctx->cleanlog==1)
          af_remove_unlogged(p_RunLog_ctx->resultulp);
    }
}

/**
 * The same than main, but no call to setBufferMode.
 */
int main_RunLog(int argc,char* argv[]) {
if (argc==1) {
   usage();
   return 0;
}

RunLog_ctx runLog_ctx;
runLog_ctx.resultulp[0]='\0';
runLog_ctx.rundir[0]='\0';
runLog_ctx.summaryfile[0]='\0';
runLog_ctx.quiet=2;
runLog_ctx.clean=1;
runLog_ctx.cleanlog=0;
runLog_ctx.nb_thread=1;
runLog_ctx.increment=0;
runLog_ctx.random=0;

int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_RunLog,lopts_RunLog,&index,vars))) {
   switch(val) {
   case 'c': runLog_ctx.clean=1; break;
   case 'p': runLog_ctx.clean=0; break;
   case 'n': runLog_ctx.cleanlog=1; break;
   case 'l': runLog_ctx.cleanlog=0; break;
   case 'm': runLog_ctx.quiet=1; break;
   case 'v': runLog_ctx.quiet=0; break;
   case 'a': 
             if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a number of random execution\n");
             }
             
             runLog_ctx.random=atol(vars->optarg);
             break;
   case 'i': 
             if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a number of increment\n");
             }
             runLog_ctx.increment=atol(vars->optarg);
             break;
   case 't': 
             if (vars->optarg[0]=='\0') {
                fatal_error("You must specify a number of thread\n");
             }
             runLog_ctx.nb_thread=atoi(vars->optarg);
             break;
   case 'r': strcpy(runLog_ctx.resultulp,vars->optarg); break;
   case 'd': strcpy(runLog_ctx.rundir,vars->optarg); break;
   case 's': strcpy(runLog_ctx.summaryfile,vars->optarg); break;
   }
   index=-1;
}

if (vars->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   free_OptVars(vars);
   return 1;
}
if (runLog_ctx.nb_thread == 0)
  runLog_ctx.nb_thread = 1;

if (runLog_ctx.nb_thread > 1)
  if (IsSeveralThreadsPossible() == 0)
      runLog_ctx.nb_thread = 1;

if (runLog_ctx.quiet==2)
  runLog_ctx.quiet = (runLog_ctx.nb_thread == 1) ? 0 : 1;


runLog_ctx.runulp=argv[vars->optind];


int trash_out=0;
int trash_err=0;
t_fnc_stdOutWrite fnc_out=NULL;
t_fnc_stdOutWrite fnc_err=NULL;
void* private_out=NULL;
void* private_err=NULL;
if (runLog_ctx.quiet==1)
{
    GetStdWriteCB(stdwrite_kind_out, &trash_out, &fnc_out,&private_out);
    GetStdWriteCB(stdwrite_kind_err, &trash_err, &fnc_err,&private_err);
    SetStdWriteCB(stdwrite_kind_out,1,NULL,NULL);
    SetStdWriteCB(stdwrite_kind_err,1,NULL,NULL);
}

RunLog_ThreadData* prunLog_ThreadData;

prunLog_ThreadData = (RunLog_ThreadData*)malloc(sizeof(RunLog_ThreadData)*(runLog_ctx.nb_thread+1));
void** ptrptr = (void**)malloc(sizeof(void**)*(runLog_ctx.nb_thread+1));

int ut;
for (ut=0;ut<runLog_ctx.nb_thread;ut++) {
    (prunLog_ThreadData+ut)->p_RunLog_ctx = &runLog_ctx;
    (prunLog_ThreadData+ut)->summary = NULL;
    (prunLog_ThreadData+ut)->num_thread = ut;

    *(ptrptr+ut) = (void*)(prunLog_ThreadData+ut);
}

//int res = RunLogParam(runLog_ctx.runulp,runLog_ctx.rundir,runLog_ctx.resultulp,runLog_ctx.clean,NULL,&summary,NULL,NULL,NULL);
//DoWork(prunLog_ThreadData);


SyncDoRunThreads(runLog_ctx.nb_thread,DoWork,ptrptr);

if (runLog_ctx.quiet==1)
{
    SetStdWriteCB(stdwrite_kind_out, trash_out, fnc_out, private_out);
    SetStdWriteCB(stdwrite_kind_err, trash_err, fnc_err, private_err);
}

for (ut=0;ut<runLog_ctx.nb_thread;ut++) {
    if ((prunLog_ThreadData+ut)->summary!=NULL)
    {
        if (runLog_ctx.quiet == 0)
          u_printf("%s",(prunLog_ThreadData+ut)->summary);
        if (runLog_ctx.summaryfile != NULL)
        {
            ABSTRACTFILE* afw = af_fopen_unlogged(runLog_ctx.summaryfile,"ab");
            if (afw != NULL)
            {
                af_fwrite((prunLog_ThreadData+ut)->summary,strlen((prunLog_ThreadData+ut)->summary),1,afw);
                af_fclose_unlogged(afw);
            }
        }
        free((prunLog_ThreadData+ut)->summary);
        (prunLog_ThreadData+ut)->summary=NULL;
    }
}
free(prunLog_ThreadData);
free(ptrptr);
free_OptVars(vars);
return 0;
}
