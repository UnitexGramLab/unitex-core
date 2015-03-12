

/* derived from public work mztools.c from Xavier Roche (integrated in Gilles Vollant minizip distribution)
   The Zlib licence choosen by Xavier Roche allow including in Unitex distribution
*/

/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/* Code */

#ifndef NO_UNITEX_LOGGER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "Unicode.h"


#include "Af_stdio.h"
#include "File.h"
#include "DirHelper.h"

#include "FilePackType.h"
#include "FileUnPack.h"
#include "UnpackFile.h"

#include "UnpackFileTool.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {




static void DisplaySize(uLong n, int size_char)
{
  /* to avoid compatibility problem , we do here the conversion */
  char number[21];
  int offset=19;
  int pos_string = 19;
  number[20]=0;
  for (;;) {
      number[offset]=(char)((n%10)+'0');
      if (number[offset] != '0')
          pos_string=offset;
      n/=10;
      if (offset==0)
          break;
      offset--;
  }
  {
      int size_display_string = 19-pos_string;
      while (size_char > size_display_string)
      {
          size_char--;
          u_printf(" ");
      }
  }

  u_printf("%s",&number[pos_string]);
}


void freeFileInPackArchiveListFile(char** archiveListFile)
{
	unsigned int i=0;
	while ((*(archiveListFile + i)) != NULL)
	{
		free(*(archiveListFile + i));
		i++;
	}
	free(archiveListFile);
}

unsigned int countFileInPackArchiveListFile(char** archiveListFile)
{
	unsigned int i=0;
	while ((*(archiveListFile + i)) != NULL)
	{		
		i++;
	}
	return i;
}

char** buildOpenedPackArchiveListFile(unzFile uf)
{
    uLong i;
    unz_global_info gi;
    int err;

    err = unzGetGlobalInfo(uf,&gi);
    if (err!=UNZ_OK)
        return NULL;

	if (gi.number_entry != 0)
	{
		err = unzGoToFirstFile(uf);
		if (err!=UNZ_OK)
			return NULL;
	}

	char** globalList = (char**)malloc(sizeof(char*)*(gi.number_entry+1));
	if (globalList == NULL)
		return NULL;

	for (i=0;i<=gi.number_entry;i++)
		*(globalList + i) = NULL;

	
	for (i=0;i<gi.number_entry;i++)
	{
		
        char filename_inzip[256] ;
		char* filename_inzip_duplicated = NULL;
        unz_file_info file_info;

		filename_inzip[0] = '\0';
        err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
		if (err == UNZ_OK)
			if (filename_inzip[0] != '\0')
		{
			filename_inzip_duplicated = (char*)malloc(strlen(filename_inzip)+1);
			if (filename_inzip_duplicated != NULL)
				strcpy(filename_inzip_duplicated,filename_inzip);
		}

		if (filename_inzip_duplicated == NULL)
		{
			freeFileInPackArchiveListFile(globalList);
			return NULL;
		}

		*(globalList + i) = filename_inzip_duplicated;


        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                freeFileInPackArchiveListFile(globalList);
				return NULL;
            }
        }
	}
	
	return globalList;
}

int do_list_from_opened_pack_archive(unzFile uf)
{
    uLong i;
    unz_global_info gi;
    int err;

    err = unzGetGlobalInfo(uf,&gi);
    if (err!=UNZ_OK)
        u_printf("error %d with zipfile in unzGetGlobalInfo \n",err);

	
	if (gi.number_entry != 0)
	{
		err = unzGoToFirstFile(uf);
		if (err!=UNZ_OK)
			u_printf("error %d with zipfile in unzGoToFirstFile \n",err);
	}


    u_printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    u_printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    for (i=0;i<gi.number_entry;i++)
    {
        char filename_inzip[256];
        unz_file_info file_info;
        uLong ratio=0;
        const char *string_method;
        char charCrypt=' ';
        err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
        if (err!=UNZ_OK)
        {
            u_printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            break;        }
        if (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is crypted */
        if ((file_info.flag & 1) != 0)
            charCrypt='*';

        if (file_info.compression_method==0)
            string_method="Stored";
        else
			string_method="COMPRESSED!";

        DisplaySize(file_info.uncompressed_size,7);
        u_printf("  %6s%c",string_method,charCrypt);
        DisplaySize(file_info.compressed_size,7);
        u_printf(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                ratio,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.crc,filename_inzip);
        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                u_printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}


static void removeNoPathPortionInFileName(char* fn)
{
    char* p = fn;
	char* filename_withoutpath = NULL;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }
	if (filename_withoutpath != NULL)
		*filename_withoutpath = '\0';
}

#define UNPACK_WRITEBUFFERSIZE (65536)

int do_extract_from_opened_pack_archive_currentfile(
    unzFile uf,
    const int* popt_extract_without_path,
	const char* prefix_extracting_name,
	int transform_path_separator, int quiet)
{
    char filename_inzip[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    ABSTRACTFILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info file_info;
    err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

    if (err!=UNZ_OK)
    {
        error("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    size_buf = UNPACK_WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
    {
        error("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
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
            if (quiet == 0)
				u_printf("creating directory: %s\n",filename_inzip);
            mkDirPortable(filename_inzip);
        }
    }
    else
    {	
	    char* previousPathCreated = NULL;
        const char* write_filename;
        int skip=0;

        if ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        else
            write_filename = filename_withoutpath;

        err = unzOpenCurrentFile(uf);
        if (err!=UNZ_OK)
        {
            error("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        

        if ((skip==0) && (err==UNZ_OK))
        {
			char* provis_concat_name = NULL;
			if (prefix_extracting_name == NULL)
				prefix_extracting_name = "";

			if (prefix_extracting_name != NULL)
				{
					size_t total_len = strlen(prefix_extracting_name)+strlen(write_filename);
					provis_concat_name = (char*)malloc(total_len+2);
					if (provis_concat_name != NULL)
					{
						strcpy(provis_concat_name,prefix_extracting_name);
						strcat(provis_concat_name,write_filename);

						if (transform_path_separator != 0)
							for (size_t pos=0; pos<total_len; pos++)
							{
								char c = *(provis_concat_name+pos);
								if ((c=='\\') || (c=='/'))
									*(provis_concat_name+pos) = PATH_SEPARATOR_CHAR;
							}
					}
				}

			const char* useFileName = (provis_concat_name != NULL) ? provis_concat_name : write_filename;

			if ((*popt_extract_without_path)==0)
			{
				char* newPathOnlyPortion = (char*)malloc(strlen(useFileName)+1);
				if (newPathOnlyPortion != NULL)
				{
					strcpy(newPathOnlyPortion,useFileName);
					removeNoPathPortionInFileName(newPathOnlyPortion);

					int can_skip_creating = 0;
					if (previousPathCreated != NULL)
						if (strcmp(previousPathCreated,newPathOnlyPortion) == 0)
							can_skip_creating = 1;

					if (can_skip_creating == 0)
					{
						if (quiet == 0)
							u_printf("Creating directory: %s\n",newPathOnlyPortion);
						mkDirPortable(newPathOnlyPortion);
					}

					if (previousPathCreated != NULL)
						free(previousPathCreated);
					previousPathCreated = newPathOnlyPortion;
				}
			}
            fout=af_fopen(useFileName,"wb");



			if (quiet == 0)
				u_printf("extracting %s to %s...",filename_inzip,useFileName);

			if (fout==NULL)
            {
                error("error opening %s\n",useFileName);
            }

			if (provis_concat_name != NULL)
				free(provis_concat_name);

            /* some zipfile don't contain directory alone before file */
            if ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                mkDirPortable(write_filename);
                *(filename_withoutpath-1)=c;
                fout=af_fopen(write_filename,"wb");
            }

           

			if (quiet == 0)
				u_printf(" done\n");
        }

        if (fout!=NULL)
        {
            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    error("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                if (err>0)
                    if (af_fwrite(buf,err,1,fout)!=1)
                    {
                        error("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
            }
            while (err>0);
            if (fout)
                    af_fclose(fout);
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
                error("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */

		
	    if (previousPathCreated != NULL)
		   free(previousPathCreated);
    }

    free(buf);
    return err;
}

int is_last_char_path_separator(const char*fn)
{
	int ret = 0;
	if (fn == NULL)
		return ret;

	while ((*fn) != '\0')
	{
		char c = *fn;
		ret = ((c == '/') || (c == '\\')) ? 1 : 0;
		fn++;
	}
	return ret;
}

int do_extract_from_opened_pack_archive(
    unzFile uf,
    int opt_extract_without_path,
	const char* prefix_extracting_name,
	int transform_path_separator, int quiet)
{
    uLong i;
    unz_global_info gi;
    int err;
	int retValue = 0;

    err = unzGetGlobalInfo(uf,&gi);
    if (err!=UNZ_OK)
	{
        u_printf("error %d with zipfile in unzGetGlobalInfo \n",err);
		return 1;
	}

	if (gi.number_entry != 0)
	{
		err = unzGoToFirstFile(uf);
		if (err!=UNZ_OK)
		{
			u_printf("error %d with zipfile in unzGetGlobalInfo \n",err);
			return 1;
		}			 
	}

	if (is_last_char_path_separator(prefix_extracting_name))
	{
		if (quiet == 0)
			u_printf("Creating extracting directory: %s\n",prefix_extracting_name);
		mkDirPortable(prefix_extracting_name);
	}

    for (i=0;i<gi.number_entry;i++)
    {
        if (do_extract_from_opened_pack_archive_currentfile(uf,&opt_extract_without_path,prefix_extracting_name,transform_path_separator,quiet) != UNZ_OK)
            break;

        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                u_printf("error %d with zipfile in unzGoToNextFile\n",err);
                retValue = 1;
            }
        }
    }

    return retValue;
}

int do_extract_from_opened_pack_archive_onefile(
    unzFile uf,
    const char* filename,
    int opt_extract_without_path,
	const char* prefix_extracting_name,
	int transform_path_separator, int quiet)
{
    int iCaseSensitivity = 1;
    if (unzLocateFile(uf,filename,iCaseSensitivity)!=UNZ_OK)
    {
        u_printf("file %s not found in the zipfile\n",filename);
        return 2;
    }

	if (is_last_char_path_separator(prefix_extracting_name))
	{
		if (quiet == 0)
			u_printf("Creating extracting directory: %s\n",prefix_extracting_name);
		mkDirPortable(prefix_extracting_name);
	}


    if (do_extract_from_opened_pack_archive_currentfile(uf,&opt_extract_without_path,prefix_extracting_name,transform_path_separator,quiet) == UNZ_OK)
        return 0;
    else
        return 1;
}

int do_list_file_in_pack_archive(const char* packFileName)
{
        unzFile uf = unzOpen(packFileName);

		if (uf == NULL)
			return 1;

		int retValue = do_list_from_opened_pack_archive(uf);
		int retClose = unzClose(uf);
		if ((retValue == 0) && (retClose != UNZ_OK))
				retValue = 1;
		return retValue;
}


char** createFileInPackArchiveListFile(const char* packFileName)
{
        unzFile uf = unzOpen(packFileName);

		if (uf == NULL)
			return NULL;

		char** archiveListFile = buildOpenedPackArchiveListFile(uf);
		int retClose = unzClose(uf);
		if ((archiveListFile != NULL) && (retClose != UNZ_OK))
		{
				freeFileInPackArchiveListFile(archiveListFile);
				archiveListFile = NULL;
		}
		return archiveListFile;
}

int do_extract_from_pack_archive_onefile(
    const char* packFileName,
    int opt_extract_without_path,
	const char* prefix_extracting_name,
	const char* filename_to_extract,
	int transform_path_separator, int quiet)
{
	
        unzFile uf = unzOpen(packFileName);

		if (uf == NULL)
			return 1;
		int retValue = do_extract_from_opened_pack_archive_onefile(uf,filename_to_extract,opt_extract_without_path,prefix_extracting_name,transform_path_separator,quiet);
		int retClose = unzClose(uf);
		if ((retValue == 0) && (retClose != UNZ_OK))
				retValue = 1;
		return retValue;
}


int do_extract_from_pack_archive(
    const char* packFileName,
    int opt_extract_without_path,
	const char* prefix_extracting_name,
	int transform_path_separator, int quiet)
{
	
        unzFile uf = unzOpen(packFileName);

		if (uf == NULL)
			return UNZ_ERRNO;
		int retValue = do_extract_from_opened_pack_archive(uf,opt_extract_without_path,prefix_extracting_name,transform_path_separator,quiet);
		int retClose = unzClose(uf);
		if ((retValue == 0) && (retClose != UNZ_OK))
				retValue = 1;
		return retValue;
}


UNITEX_FUNC char** UNITEX_CALL GetListOfFileInUnitexArchive(const char* packFileName)
{
	return createFileInPackArchiveListFile(packFileName);
}

UNITEX_FUNC unsigned int UNITEX_CALL GetFilesNumberInListOfFileInUnitexArchive(char** archiveListFile)
{
	return countFileInPackArchiveListFile(archiveListFile);
}

UNITEX_FUNC void UNITEX_CALL ReleaseListOfFileInUnitexArchive(char** archiveListFile)
{
	freeFileInPackArchiveListFile(archiveListFile);
}

UNITEX_FUNC int UNITEX_CALL ExtractOneFileFromUnitexArchive(
    const char* packFileName,
    int opt_extract_without_path,
	const char* prefix_extracting_name,
	const char* filename_to_extract,
	int transform_path_separator)
{
	int quiet = 1;
	return do_extract_from_pack_archive_onefile(
    packFileName,
    opt_extract_without_path,
	prefix_extracting_name,
	filename_to_extract,
	transform_path_separator, quiet);
}



UNITEX_FUNC int UNITEX_CALL ExtractFilesFromUnitexArchive(
    const char* packFileName,
    int opt_extract_without_path,
	const char* prefix_extracting_name,
	int transform_path_separator)
{
	int quiet = 1;
	return do_extract_from_pack_archive(
    packFileName,
    opt_extract_without_path,
	prefix_extracting_name,
	transform_path_separator, quiet);
}
 


} // namespace logger
} // namespace unitex

#endif
