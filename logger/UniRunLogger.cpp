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



class InstallLoggerForRunner
{
public:
    InstallLoggerForRunner();
    ~InstallLoggerForRunner();

    int SelectNextLogName(const char*LogName,const char*portion_ignore_pathname);
private:
    struct UniLoggerSpace ule;
    int init_done;
};


InstallLoggerForRunner::InstallLoggerForRunner()
{
    init_done = 0;

    /* we want "mini log" with only list */
    
    ule.privateUnloggerData = NULL;
    ule.szPathLog = NULL;
    ule.store_file_out_content = 0+1;
    ule.store_list_file_out_content = 1;

    ule.store_file_in_content = 0+1;
    ule.store_list_file_in_content = 1;

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

InstallLoggerForRunner InstallLoggerForRunnerInstance;

/****************************************************************************************/



int mkdir_recursive(const char*dirname,int iContainOnlyDirName)
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
                  mkDirPortable (dupname);
                  *(lpParc) = c;
              }

          lpParc++;
      }
      free(dupname);
  }

  int ret = 0;
  if (iContainOnlyDirName != 0)
      ret = mkDirPortable(dirname);
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

    if ((*filename_withoutpath)=='\0')
    {
        if ((*popt_extract_without_path)==0)
        {
            //printf("creating directory: %s\n",filename_touse);
            //mkdir_recursive(filename_touse);
        }
    }
    else
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
                /*
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                mkdir_recursive(write_filename);
                *(filename_withoutpath-1)=c;
                */
                mkdir_recursive(write_filename,0);
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

UNITEX_FUNC int UNITEX_CALL RunUnitexLog(const char* LogNameRead,const char* FileRunPath,const char* LogNameWrite)
{
    zlib_filefunc_def zlib_filefunc;
    fill_afopen_filefunc(&zlib_filefunc);

    char* command_line_buf = NULL;
    size_t size_command_line = 0;

    char* list_out_buf = NULL;
    size_t size_list_out = 0;

    struct ListFile* list_file_out = NULL;
    struct ListFile* list_file_in = NULL;

    if (FileRunPath==NULL)
        FileRunPath="";
    int need_add_dir_sep = 0;
    size_t len_RunPath = strlen(FileRunPath);
    if (len_RunPath>0)
        if (((*(FileRunPath+len_RunPath-1)) != '/') && ((*(FileRunPath+len_RunPath-1)) != '\\'))
            need_add_dir_sep=1;

    unzFile uf = unzOpen2(LogNameRead,&zlib_filefunc);
    if (uf == NULL)
        return -1;

    uLong i;
    unz_global_info gi;
    int err;
    
    err = unzGetGlobalInfo (uf,&gi);
    if (err==UNZ_OK)
    {
      list_file_in = AllocListFile(gi.number_entry);
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
              int extres = do_extracting_currentfile(uf,filename_to_write,&opt_extract_without_path,&opt_overwrite,NULL);

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
      int nb_arg = atoi(cur_command_line_entry);

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
          u_printf("%d\n'%s'\n'%s'\n\n",walk,*(argv_log+walk),*(argv_log_reworked+walk+1));
      }

      argc_log+=0;

      unzClose(uf);
      /* we have done extracting */

      //char *buf_reworked=malloc(size_command_line + 

      if (LogNameWrite != NULL)
          InstallLoggerForRunnerInstance.SelectNextLogName(LogNameWrite,FileRunPath);

      unsigned int walk_list_out;
      for (walk_list_out = 0;walk_list_out < list_file_out->iNbFile;walk_list_out++)
      {
          const char* filename_out = ((list_file_out->p_ListFile_entry) + walk_list_out) -> filename;
          const char* filename_out_usable = get_filename_to_copy(filename_out);
          char filename_to_be_written[256];
          CombineRunPathOnPathName(filename_to_be_written,FileRunPath,need_add_dir_sep,filename_out_usable);
          mkdir_recursive(filename_to_be_written,0);
      }

      main_UnitexTool_C(argc_log+1,(char**)argv_log_reworked);



      free(buf_arg);
      free(argv_log);

      free(buf_arg_reworked);
      free(argv_log_reworked);


      if (command_line_buf != NULL)
          free(command_line_buf);

      if (list_out_buf != NULL)
          free(list_out_buf);

      FreeListFile(list_file_out);
      FreeListFile(list_file_in);
    }

    return 0;
}
