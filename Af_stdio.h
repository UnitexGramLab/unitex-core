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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS)
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
 *
 */




#ifndef _ABSTRACT_FILE_STDIO_INCLUDED
#define _ABSTRACT_FILE_STDIO_INCLUDED

#define HAS_LIST_FILE 1

#ifdef __cplusplus
extern "C" {
#endif

struct _ABSTRACTFILE {
        void* dummy;
        };
typedef struct _ABSTRACTFILE ABSTRACTFILE;

struct _ABSTRACTMAPFILE {
        void* dummy;
        };
typedef struct _ABSTRACTMAPFILE ABSTRACTMAPFILE;

#define MAPFILE_OPTION_READ ((int)0x00000001)

ABSTRACTMAPFILE* af_open_mapfile(const char*name,int options, size_t value_for_option);
ABSTRACTMAPFILE* af_open_mapfile_unlogged(const char*name,int options, size_t value_for_option);

size_t af_get_mapfile_size(ABSTRACTMAPFILE*);

#ifdef __cplusplus
const void* af_get_mapfile_pointer(ABSTRACTMAPFILE*, size_t pos=0, size_t sizemap=0);
void af_release_mapfile_pointer(ABSTRACTMAPFILE*, const void*,size_t sizemap=0);
#else
const void* af_get_mapfile_pointer(ABSTRACTMAPFILE*, size_t pos, size_t sizemap);
void af_release_mapfile_pointer(ABSTRACTMAPFILE*, const void*,size_t sizemap);
#endif

void af_close_mapfile(ABSTRACTMAPFILE*);
void af_close_mapfile_unlogged(ABSTRACTMAPFILE*);

int is_filename_in_abstract_file_space(const char*name);

size_t af_fread(void *ptr,size_t size,size_t nmemb,ABSTRACTFILE *stream);

size_t af_fwrite(const void *ptr,size_t size,size_t nmemb,ABSTRACTFILE *stream);

char *af_fgets(char * _Buf, int _MaxCount, ABSTRACTFILE * _File);

ABSTRACTFILE* af_fopen(const char* name,const char* MODE);

ABSTRACTFILE* af_fopen_unlogged(const char* name,const char* MODE);

int af_fseek(ABSTRACTFILE* stream, long offset, int whence) ;

long af_ftell(ABSTRACTFILE* stream) ;

int af_feof(ABSTRACTFILE* stream) ;

int af_ungetc(int, ABSTRACTFILE* stream) ;

int af_fclose(ABSTRACTFILE* stream) ;

int af_fclose_unlogged(ABSTRACTFILE* stream) ;

void af_setsizereservation(ABSTRACTFILE* stream, long size_planned);


char** af_get_list_file(const char*name);

void af_release_list_file(const char*name,char**list);



int af_rename(const char * OldFilename, const char * NewFilename);
int af_rename_unlogged(const char * OldFilename, const char * NewFilename);

int af_remove(const char * Filename);
int af_remove_unlogged(const char * Filename);

#define AF_REMOVE_FOLDER_PRESENT_AF_STDIO 1
int af_remove_folder(const char*folderName);
int af_remove_folder_unlogged(const char*folderName);

int af_copy(const char* srcFile, const char* dstFile);
int af_copy_unlogged(const char* srcFile, const char* dstFile);


extern const ABSTRACTFILE* pVF_StdIn ;
extern const ABSTRACTFILE* pVF_StdOut ;
extern const ABSTRACTFILE* pVF_StdErr ;

int IsStdIn(ABSTRACTFILE*);


#ifdef __cplusplus
}
#endif


#endif
