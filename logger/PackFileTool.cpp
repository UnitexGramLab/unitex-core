

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

#include "Af_stdio.h"

#include "FilePackType.h"




#include "Af_stdio.h"
#include "DirHelper.h"

#include "FilePack.h"
#include "FilePackType.h"



#include "FilePackIo.h"
#include "FilePackCrc32.h"

#include "PackFileTool.h"

#include "Unicode.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {


#define PACK_WRITEBUFFERSIZE (65536)


// return 1 if ok, 0 if error
static int addFileInPackFile(zipFile zf,const char* fileNameInArchive, const char* fileNameToRead,int quiet)
{
    size_t size_buf=PACK_WRITEBUFFERSIZE;
	uLong crc = 0;
	uLong size_done = 0;
    char* buf = (char*)malloc(size_buf);

	if ((*fileNameInArchive) == '*')
		fileNameInArchive++;
	else
	if (((*fileNameInArchive) == '$') ||
		(((*fileNameInArchive) >= 'A') && ((*fileNameInArchive) <= 'Z')) ||
		(((*fileNameInArchive) >= 'z') && ((*fileNameInArchive) <= 'z')))
		if ((*(fileNameInArchive+1)) == ':')
			fileNameInArchive+=2;

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

	if (quiet != 0)
		u_printf("packing %s to %s...",fileNameToRead,fileNameInArchive);

    fin = af_fopen(fileNameToRead,"rb");
    if (fin==NULL)
    {
        err=ZIP_ERRNO;

        free(buf);
        return 0;
    }

    err = zipOpenNewFileInZip(zf,fileNameInArchive,&zi,NULL,0,NULL,0,NULL /* param_list */,
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

                    error("error in reading %s\n",fileNameToRead);

                    err = ZIP_ERRNO;
                }

                if (size_read>0)
                {
                    err = zipWriteInFileInZip (zf,buf,(unsigned int)size_read);
                    if (err<0)
                    {

                        error("error in writing %s in the zipfile\n",
                                         fileNameInArchive);
                    }
                    else
                    {
                        size_done += (unsigned int)size_read;
                        crc = crc32(crc,buf,size_read);
                    }

                }
            } while ((err == ZIP_OK) && (size_read>0));



            if (err<0)
            {
                err=ZIP_ERRNO;
                zipCloseFileInZip(zf);
            }
            else
            {
                err = zipCloseFileInZip(zf);

                if (err!=ZIP_OK)
                    error("error in closing %s in the zipfile\n",
                                fileNameInArchive);
            }
    }
    af_fclose(fin);
    free(buf);

	if (quiet != 0)
		u_printf(" done\n");

    return (err == ZIP_OK) ? 1 : 0;
}

static const char* filterFileNameByJunkPrefix(const char* filename, const char* junk_prefix, int nb_char_remove_end_prefix)
{
	if (junk_prefix == NULL)
		return filename;

	if ((*junk_prefix) == '\0')
		return filename;

	size_t len_prefix = strlen(junk_prefix);
	size_t len_filename = strlen(filename);

	if (((int)len_prefix) < nb_char_remove_end_prefix)
		return filename;

	if (len_filename < (len_prefix - nb_char_remove_end_prefix))
		return filename;

	if (memcmp(filename,junk_prefix,len_prefix - nb_char_remove_end_prefix) == 0)
		return filename + len_prefix - nb_char_remove_end_prefix;
	else
		return filename;
}


int buildPackFile(const char* packFile,int append,const char* global_comment,
                  const char* file_or_prefix_to_add,int add_one_file_only,const char* junk_prefix,
				  int quiet)
{
	// append_status can be APPEND_STATUS_CREATE
	zipFile zf=zipOpen(packFile,(append != 0) ?  APPEND_STATUS_ADDINZIP : APPEND_STATUS_CREATE);
    if (zf==NULL)
    {
        return 1;
    }


	int retValue = 1;

	if (add_one_file_only != 0)
		retValue = addFileInPackFile(zf,
		                   filterFileNameByJunkPrefix(file_or_prefix_to_add,junk_prefix,0),
						   file_or_prefix_to_add,quiet);
	else
	{
		char ** listfile = af_get_list_file(file_or_prefix_to_add);
		if (listfile != NULL)
		{
			int i=0;
			while (*(listfile + i) != NULL)
			{
				retValue = addFileInPackFile(zf,
								   filterFileNameByJunkPrefix(*(listfile + i),junk_prefix,0),
								   *(listfile + i),quiet);

				if (retValue == 0)
					break;

				i++;
			}

			af_release_list_file(file_or_prefix_to_add,listfile);
		}
	}

	zipClose(zf,global_comment);
	return retValue;
}


UNITEX_FUNC int UNITEX_CALL CreateUnitexPackOneFile(const char* packFile,int append_status,
														const char* file_to_include, const char* junk_prefix,
														const char* global_comment)
{
	int quiet = 1;
	return buildPackFile( packFile,append_status,  global_comment,
                  file_to_include,1, junk_prefix,
				  quiet);
}

UNITEX_FUNC int UNITEX_CALL CreateUnitexPackMultiFile(const char* packFile,int append_status,
														const char* file_to_include, const char* junk_prefix,
														const char* global_comment)
{
	int quiet = 1;
	return buildPackFile( packFile,append_status,  global_comment,
                  file_to_include,0, junk_prefix,
				  quiet);
}

} // namespace logger
} // namespace unitex

#endif




