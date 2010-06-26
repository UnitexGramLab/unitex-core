/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Af_stdio.h"
static ABSTRACTFILE* (*real_fopen)(const char*,const char*)=af_fopen;

#include "Unicode.h"
#include "Error.h"
#include "AbstractAllocator.h"


/* This array is a bit array used to define characters that are letters */
static char tab_is_letter[8192];


/**
 * We define here the unicode NULL character and the unicode
 * empty string.
 */
const unichar U_NULL='\0';
const unichar* U_EMPTY=&U_NULL;

const unichar EPSILON[]={'<','E','>','\0'};


/**
 * Allocates, initializes and returns a new U_FILE*
 * f is supposed to have been opened.
 */
U_FILE* new_U_FILE(ABSTRACTFILE* f,Encoding e) {
U_FILE* u=(U_FILE*)malloc(sizeof(U_FILE));
u->f=f;
u->enc=e;
return u;
}


/**
 * Frees the U_FILE*. The caller has to fclose the file.
 */
void free_U_FILE(U_FILE* u) {
if (u==NULL) return;
free(u);
}


const U_FILE CTE_U_STDIN  = { (ABSTRACTFILE*)pVF_StdIn,  UTF8 };
const U_FILE CTE_U_STDOUT = { (ABSTRACTFILE*)pVF_StdOut, UTF8 };
const U_FILE CTE_U_STDERR = { (ABSTRACTFILE*)pVF_StdErr, UTF8 };

U_FILE* U_STDIN  = (U_FILE*)&CTE_U_STDIN;
U_FILE* U_STDOUT = (U_FILE*)&CTE_U_STDOUT;
U_FILE* U_STDERR = (U_FILE*)&CTE_U_STDERR;

int fseek(U_FILE* stream, long offset, int whence) {
return af_fseek(stream->f,offset,whence);
}

long ftell(U_FILE* stream) {
return af_ftell(stream->f);
}

void rewind(U_FILE* stream) {
fseek( stream, 0L, SEEK_SET );
}

int u_feof(U_FILE* stream) {
return af_feof(stream->f);
}

size_t fread(void *ptr,size_t size,size_t nmemb,U_FILE *stream) {
return af_fread(ptr,size,nmemb,stream->f);
}

size_t fwrite(const void *ptr,size_t size,size_t nmemb,U_FILE *stream) {
return af_fwrite(ptr,size,nmemb,stream->f);
}



int u_fgetc_raw(Encoding,ABSTRACTFILE*);
int u_fgetc_raw(U_FILE* f) {
return u_fgetc_raw(f->enc,f->f);
}

int u_fgetc(Encoding,ABSTRACTFILE*);
int u_fgetc_UTF16LE(ABSTRACTFILE* f) {
return u_fgetc(UTF16_LE,f);
}

int u_fgetc_UTF16BE(ABSTRACTFILE* f) {
return u_fgetc(BIG_ENDIAN_UTF16,f);
}

int u_fgetc_UTF8(ABSTRACTFILE* f) {
return u_fgetc(UTF8,f);
}

int u_fgetc(U_FILE* f) {
return u_fgetc(f->enc,f->f);
}

int u_fgetc_CR(Encoding,ABSTRACTFILE*);
int u_fgetc_CR(U_FILE* f) {
return u_fgetc_CR(f->enc,f->f);
}

int u_fread_raw(Encoding,unichar*,int,ABSTRACTFILE*);
int u_fread_raw(unichar* t,int N,U_FILE* f) {
return u_fread_raw(f->enc,t,N,f->f);
}

int u_fread(Encoding,unichar*,int,ABSTRACTFILE*,int*);
int u_fread(unichar* t,int N,U_FILE* f,int *OK) {
return u_fread(f->enc,t,N,f->f,OK);
}

int u_fputc_raw(Encoding,unichar,ABSTRACTFILE*);
int u_fputc_raw(unichar c,U_FILE* f) {
return u_fputc_raw(f->enc,c,f->f);
}

int u_fputc(Encoding,unichar,ABSTRACTFILE*);
int u_fputc_UTF16LE(unichar c,ABSTRACTFILE* f) {
return u_fputc(UTF16_LE,c,f);
}

int u_fputc_UTF16BE(unichar c,ABSTRACTFILE* f) {
return u_fputc(BIG_ENDIAN_UTF16,c,f);
}

int u_fputc_UTF8(unichar c,ABSTRACTFILE* f) {
return u_fputc(UTF8,c,f);
}

int u_fputc(unichar c,U_FILE* f) {
return u_fputc(f->enc,c,f->f);
}

int u_ungetc(Encoding,unichar,ABSTRACTFILE*);
int u_ungetc(unichar c,U_FILE* f) {
return u_ungetc(f->enc,c,f->f);
}

int u_fwrite_raw(Encoding,unichar*,int,ABSTRACTFILE*);
int u_fwrite_raw(unichar* t,int N,U_FILE* f) {
return u_fwrite_raw(f->enc,t,N,f->f);
}

int u_fwrite(Encoding,unichar*,int,ABSTRACTFILE*);
int u_fwrite(unichar* t,int N,U_FILE* f) {
return u_fwrite(f->enc,t,N,f->f);
}

int u_fgets(Encoding,unichar*,ABSTRACTFILE*);
int u_fgets(unichar* s,U_FILE* f) {
return u_fgets(f->enc,s,f->f);
}

int u_fgets(Encoding,unichar*,int,ABSTRACTFILE*);
int u_fgets(unichar* s,int size,U_FILE* f) {
return u_fgets(f->enc,s,size,f->f);
}

int u_fgets_treat_cr_as_lf(Encoding,unichar* s,int size,ABSTRACTFILE* f,int supress_null,int* found_null);
int u_fgets_treat_cr_as_lf(unichar* s,int size,U_FILE* f,int supress_null,int* found_null) {
return u_fgets_treat_cr_as_lf(f->enc,s,size,f->f,supress_null,found_null);
}

int u_fgets2(Encoding,unichar*,ABSTRACTFILE*);
int u_fgets2(unichar* s,U_FILE* f) {
return u_fgets2(f->enc,s,f->f);
}

int u_fgets_limit2(Encoding,unichar*,int,ABSTRACTFILE*);
int u_fgets_limit2(unichar* s,int size,U_FILE* f) {
return u_fgets_limit2(f->enc,s,size,f->f);
}

/**
 * By default, messages printed to the standard output are UTF8-encoded.
 */
int u_printf(const char* format,...) {
va_list list;
va_start(list,format);
int n=u_vfprintf(U_STDOUT,format,list);
va_end(list);
return n;
}

void u_fprints(Encoding,const unichar*,ABSTRACTFILE*);
void u_fprints(const unichar* s,U_FILE* f) {
u_fprints(f->enc,s,f->f);
}

void u_fprints(Encoding,const char*,ABSTRACTFILE*);
void u_fprints(const char* s,U_FILE* f) {
u_fprints(f->enc,s,f->f);
}

int u_scanf(const char* format,...) {
va_list list;
va_start(list,format);
int n=u_vfscanf(U_STDIN,format,list);
va_end(list);
return n;
}


/* ------------------ Encoding function ----------------- */

struct reading_encoding_item
{
    const char* encoding_name;
    size_t len;
    int encoding_flag;
};


const struct reading_encoding_item reading_encoding_item_list[] =
{
    { "ascii", 5, ASCII_NO_BOM_POSSIBLE },

    { "bom", 3, UTF8_BOM_POSSIBLE | UTF16_LE_BOM_POSSIBLE | BIG_ENDIAN_UTF16_BOM_POSSIBLE },

    { "utf8", 4, UTF8_BOM_POSSIBLE | UTF8_NO_BOM_POSSIBLE },
    { "utf8-no-bom", 11, UTF8_NO_BOM_POSSIBLE },
    { "utf8-bom", 8, UTF8_BOM_POSSIBLE },

    { "utf-16-le", 9, UTF16_LE_BOM_POSSIBLE | UTF16_LE_NO_BOM_POSSIBLE },
    { "utf-16-le-no-bom", 16, UTF16_LE_NO_BOM_POSSIBLE },
    { "utf-16-le-bom", 13, UTF16_LE_BOM_POSSIBLE },

    { "utf16-le", 8, UTF16_LE_BOM_POSSIBLE | UTF16_LE_NO_BOM_POSSIBLE },
    { "utf16-le-no-bom", 15, UTF16_LE_NO_BOM_POSSIBLE },
    { "utf16-le-bom", 12, UTF16_LE_BOM_POSSIBLE },

    { "utf16le", 7, UTF16_LE_BOM_POSSIBLE | UTF16_LE_NO_BOM_POSSIBLE },
    { "utf16le-no-bom", 14, UTF16_LE_NO_BOM_POSSIBLE },
    { "utf16le-bom", 11, UTF16_LE_BOM_POSSIBLE },

    { "little-endian", 13, UTF16_LE_BOM_POSSIBLE | UTF16_LE_NO_BOM_POSSIBLE },
    { "little-endian-no-bom", 20, UTF16_LE_NO_BOM_POSSIBLE },
    { "little-endian-bom", 17, UTF16_LE_BOM_POSSIBLE },

    { "utf-16-be", 9, BIG_ENDIAN_UTF16_BOM_POSSIBLE | BIG_ENDIAN_UTF16_NO_BOM_POSSIBLE },
    { "utf-16-be-no-bom", 16, BIG_ENDIAN_UTF16_NO_BOM_POSSIBLE },
    { "utf-16-be-bom", 13, BIG_ENDIAN_UTF16_BOM_POSSIBLE },

    { "utf16-be", 8, BIG_ENDIAN_UTF16_BOM_POSSIBLE | BIG_ENDIAN_UTF16_NO_BOM_POSSIBLE },
    { "utf16-be-no-bom", 15, BIG_ENDIAN_UTF16_NO_BOM_POSSIBLE },
    { "utf16-be-bom", 12, BIG_ENDIAN_UTF16_BOM_POSSIBLE },

    { "utf16be", 7, BIG_ENDIAN_UTF16_BOM_POSSIBLE | BIG_ENDIAN_UTF16_NO_BOM_POSSIBLE },
    { "utf16be-no-bom", 14, BIG_ENDIAN_UTF16_NO_BOM_POSSIBLE },
    { "utf16be-bom", 11, BIG_ENDIAN_UTF16_BOM_POSSIBLE },

    { "big-endian", 10, BIG_ENDIAN_UTF16_BOM_POSSIBLE | BIG_ENDIAN_UTF16_NO_BOM_POSSIBLE },
    { "big-endian-no-bom", 17, BIG_ENDIAN_UTF16_NO_BOM_POSSIBLE },
    { "big-endian-bom", 14, BIG_ENDIAN_UTF16_BOM_POSSIBLE },

    { NULL, 0, 0 }
};


struct write_encoding_item
{
    const char* encoding_name;
    size_t len;
    Encoding encoding;
    int bom;
};

const struct write_encoding_item write_encoding_item_list[] =
{
    { "ascii", 5, ASCII, 0 },

    { "utf8", 4, UTF8, 2 },
    { "utf8-no-bom", 11, UTF8, 0 },
    { "utf8-bom", 8, UTF8,1 },

    { "utf-16-le", 9, UTF16_LE, 2 },
    { "utf-16-le-no-bom", 16, UTF16_LE, 0 },
    { "utf-16-le-bom", 13, UTF16_LE, 1 },

    { "utf16-le", 8, UTF16_LE, 2 },
    { "utf16-le-no-bom", 15, UTF16_LE, 0 },
    { "utf16-le-bom", 12, UTF16_LE, 1 },

    { "utf16le", 7, UTF16_LE, 2 },
    { "utf16le-no-bom", 14, UTF16_LE, 0 },
    { "utf16le-bom", 11, UTF16_LE, 1 },

    { "little-endian", 13, UTF16_LE, 2 },
    { "little-endian-no-bom", 20, UTF16_LE, 0 },
    { "little-endian-bom", 17, UTF16_LE, 1 },

    { "utf-16-be", 9, BIG_ENDIAN_UTF16, 2 },
    { "utf-16-be-no-bom", 16, BIG_ENDIAN_UTF16, 0 },
    { "utf-16-be-bom", 13, BIG_ENDIAN_UTF16, 1 },

    { "utf16-be", 8, BIG_ENDIAN_UTF16, 2 },
    { "utf16-be-no-bom", 15, BIG_ENDIAN_UTF16, 0 },
    { "utf16-be-bom", 12, BIG_ENDIAN_UTF16, 1 },

    { "utf16be", 7, BIG_ENDIAN_UTF16, 2 },
    { "utf16be-no-bom", 14, BIG_ENDIAN_UTF16, 0 },
    { "utf16be-bom", 11, BIG_ENDIAN_UTF16, 1 },

    { "big-endian", 10, BIG_ENDIAN_UTF16, 2 },
    { "big-endian-no-bom*", 17, BIG_ENDIAN_UTF16, 0 },
    { "big-endian-bom", 14, BIG_ENDIAN_UTF16, 1 },

    { NULL, 0, ASCII, 0 }
};

/*
 * duplicate a string and convert to lower case
 */
char *strdup_lower_case(const char* text)
{
    size_t len_text=strlen(text);
    char* new_str = (char*)malloc(len_text+1);
    if (new_str==NULL)
        return NULL;
    strcpy(new_str,text);
    for (size_t i=0;i < len_text;i++)
    {
        if (((*(new_str+i))>='A') && ((*(new_str+i))<='Z')) (*(new_str+i)) += 32; /* 32 = 'a'-'A' */
        if (((*(new_str+i))=='_')) (*(new_str+i)) = '-'; /* 32 = 'a'-'A' */
    }

    return new_str;
}

/*
 * decode encoding parameter for reading file and fill encoding_compatibility mask
 */
int decode_reading_encoding_parameter(int* p_mask_encoding_compatibility,const char* encoding_text)
{
    char* lower_encoding_text=strdup_lower_case(encoding_text);
    int ret=0;   
    char * cur_encoding_text = lower_encoding_text;
    char * next_encoding_text;
    int mask_encoding_compatibility = 0;


    while (cur_encoding_text != NULL)
    {
        int i;

        i=0;
        while ((*(cur_encoding_text+i)!='\0') && (*(cur_encoding_text+i)!=',')) {
            i++;
        }

        if (*(cur_encoding_text+i)==',')
        {
            *(cur_encoding_text+i)='\0';
            next_encoding_text = cur_encoding_text+i+1;
        }
        else
            next_encoding_text = NULL;

        i=0;
        size_t len_search = strlen(cur_encoding_text);
        while (reading_encoding_item_list[i].encoding_name != NULL)
        {
            if (len_search == reading_encoding_item_list[i].len)
                if (strcmp(cur_encoding_text,reading_encoding_item_list[i].encoding_name)==0)
                {
                    ret = 1;
                    mask_encoding_compatibility = mask_encoding_compatibility | reading_encoding_item_list[i].encoding_flag;
                    break;
                }           

            i++;
        }
        cur_encoding_text = next_encoding_text;
    }
    free(lower_encoding_text);
    *p_mask_encoding_compatibility = mask_encoding_compatibility;
    return ret;
}


int get_reading_encoding_text(char* text_encoding,size_t size_text_buffer,int mask_encoding_compatibility)
{
    char result[0x100];
    result[0]=result[1]=0;

    if ((mask_encoding_compatibility & UTF16_LE_BOM_POSSIBLE) != 0)
        strcat(result,",utf16-le-bom");

    if ((mask_encoding_compatibility & BIG_ENDIAN_UTF16_BOM_POSSIBLE) != 0)
        strcat(result,",utf16-be-bom");

    if ((mask_encoding_compatibility & UTF8_BOM_POSSIBLE) != 0)
        strcat(result,",utf8-bom");

    if ((mask_encoding_compatibility & UTF16_LE_NO_BOM_POSSIBLE) != 0)
        strcat(result,",utf16-le-no-bom");

    if ((mask_encoding_compatibility & BIG_ENDIAN_UTF16_NO_BOM_POSSIBLE) != 0)
        strcat(result,",utf16-be-no-bom");

    if ((mask_encoding_compatibility & UTF8_NO_BOM_POSSIBLE) != 0)
        strcat(result,",utf8-no-bom");

    if ((mask_encoding_compatibility & ASCII_NO_BOM_POSSIBLE) != 0)
        strcat(result,",ascii");

    size_t len_result=strlen(result+1);
    if (len_result+1 > size_text_buffer)
        return 0;
    strcpy(text_encoding,result+1);
    return 1;
}

int decode_writing_encoding_parameter(Encoding* p_encoding,int* p_bom,const char* encoding_text)
{
    char* lower_encoding_text=strdup_lower_case(encoding_text);
    int i=0;
    size_t len_search=strlen(lower_encoding_text);
    while (write_encoding_item_list[i].encoding_name != NULL)
    {
        if (len_search == write_encoding_item_list[i].len)
            if (strcmp(write_encoding_item_list[i].encoding_name,lower_encoding_text)==0)
            {
                *p_bom = write_encoding_item_list[i].bom;
                *p_encoding = write_encoding_item_list[i].encoding;
                free(lower_encoding_text);
                return 1;
            }
        i++;
    }

    free(lower_encoding_text);
    return 0;
}



int get_writing_encoding_text(char* text_encoding,size_t size_text_buffer,Encoding encoding,int bom)
{
    const char* result="";

    switch (encoding)
    {
        case UTF16_LE : result = (bom != 0) ? "utf16-le-bom" : "utf16-le-no-bom"; break;
        case BIG_ENDIAN_UTF16 : result = (bom != 0) ? "utf16-be-bom" : "utf16-be-no-bom"; break;
        case UTF8 : result = (bom == 1) ? "utf8-bom" : "utf8-no-bom"; break;
        case ASCII : result = "ascii"; break;
    }

    size_t len_result=strlen(result);
    if (len_result+1 > size_text_buffer)
        return 0;
    strcpy(text_encoding,result);
    return 1;
}

/* ------------------- File functions ------------------- */

/*

 */
int GetFileEncoding(ABSTRACTFILE* f,Encoding* encoding,int *is_BOM,int MASK_ENCODING_COMPATIBILITY)
{
    af_fseek(f,0,0);

    if ((MASK_ENCODING_COMPATIBILITY & (UTF16_LE_BOM_POSSIBLE | BIG_ENDIAN_UTF16_BOM_POSSIBLE | UTF8_BOM_POSSIBLE)) != 0)
    {
        /* check the BOM */
        unsigned char tab[4];
        if (af_fread(&tab[0],1,2,f)!=2)
            tab[0]=tab[1]=0;

        /* now we test BOM */
        if ((MASK_ENCODING_COMPATIBILITY & UTF16_LE_BOM_POSSIBLE) != 0)
            if ((tab[0] == 0xff) && (tab[1]==0xfe))
            {
                af_fseek(f,0,0);            
                *encoding=UTF16_LE;
                *is_BOM=1;
                return 1;
            }

        if ((MASK_ENCODING_COMPATIBILITY & BIG_ENDIAN_UTF16_BOM_POSSIBLE) != 0)
            if ((tab[0] == 0xfe) && (tab[1]==0xff))
            {
                af_fseek(f,0,0);
                *encoding=BIG_ENDIAN_UTF16;
                *is_BOM=1;
                return 1;
            }


        if ((MASK_ENCODING_COMPATIBILITY & UTF8_BOM_POSSIBLE) != 0)
        {
            if (af_fread(&tab[2],1,1,f)!=1)
                tab[2]=0;

            if ((tab[0] == 0xef) && (tab[1]==0xbb) && (tab[2]==0xbf))
            {
                af_fseek(f,0,0);
                *encoding=UTF8;
                *is_BOM=1;
                return 1;
            }
        }

        af_fseek(f,0,0);
    }
    /* we known there is no BOM */


    if ((MASK_ENCODING_COMPATIBILITY & UTF8_NO_BOM_POSSIBLE) != 0)
    {
        *encoding=UTF8;
        *is_BOM=0;
        return 1;
    }

    if ((MASK_ENCODING_COMPATIBILITY & ASCII_NO_BOM_POSSIBLE) != 0)
    {
        *encoding=ASCII;
        *is_BOM=0;
        return 1;
    }

    if ((MASK_ENCODING_COMPATIBILITY & UTF16_LE_NO_BOM_POSSIBLE) != 0)
    {
        *encoding=UTF16_LE;
        *is_BOM=0;
        return 1;
    }

    if ((MASK_ENCODING_COMPATIBILITY & BIG_ENDIAN_UTF16_NO_BOM_POSSIBLE) != 0)
    {
        *encoding=BIG_ENDIAN_UTF16;
        *is_BOM=0;
        return 1;
    }

    return 0;
}

/**
 * Opens a file in binary mode for unicode I/O and returns the
 * file in case of success; NULL otherwise. If you try to open a file
 * in READ mode, NULL will be returned if the file does not exist or
 * if it does not start with the byte order mark, in the case of a UTF16
 * file. In this last case, an error message will be printed to indicate
 * that the file is not a UTF16 one.
 *
 * 'MODE' should be U_READ, U_WRITE, U_APPEND or U_MODIFY
 * is_BOM = 0 : no BOM
 * is_BOM = 1 : we have BOM
 * is_BOM = 2 : we have BOM only for UTF16_LE or BIG_ENDIAN_UTF16
 */
U_FILE* u_fopen_internal(Encoding encoding,int is_BOM,const char* name,OpenMode MODE,int MASK_ENCODING_COMPATIBILITY) {
if (name==NULL) {
	fatal_error("NULL file name in u_fopen\n");
}
ABSTRACTFILE* f;
if (MODE==U_APPEND || MODE==U_MODIFY) {
   /* If we are in APPEND or MODIFY mode, we check first if the file already exists */
   f=real_fopen(name,"rb");
   if (f!=NULL) {
      if (MASK_ENCODING_COMPATIBILITY != USE_ENCODING_VALUE)
          if (GetFileEncoding(f,&encoding,&is_BOM,MASK_ENCODING_COMPATIBILITY) == 0)
          {
              af_fclose(f);
              error("u_fopen error: %s is not a compatible text file\n",name);
              return NULL;
          }
      /* If the file exists, we close it and reopen it in APPEND mode */
      af_fclose(f);
      f=real_fopen(name,(MODE==U_APPEND)?"ab":"r+b");
      if (MODE==U_MODIFY) {
         /* If we are in MODIFY mode, we must set the cursor at the beginning of the
          * file, i.e. after the byte order mark, if any. */
         if ((encoding==UTF16_LE || encoding==BIG_ENDIAN_UTF16) && (is_BOM!=0)) {
            af_fseek(f,2,0);
         }
         if ((encoding==UTF8) && (is_BOM==1)) {
            af_fseek(f,3,0);
         }
      }
      return new_U_FILE(f,encoding);
   }
   /* If the file does not exists, we are in WRITE mode */
   f=real_fopen(name,"wb");
   if (f==NULL) return NULL;
   /* As the file is new, we must insert the byte order char */
   if ((encoding==UTF16_LE) && (is_BOM!=0)) u_fputc_UTF16LE(U_BYTE_ORDER_MARK,f);
   else if ((encoding==BIG_ENDIAN_UTF16) && (is_BOM!=0)) u_fputc_UTF16BE(U_BYTE_ORDER_MARK,f);
   else if ((encoding==UTF8) && (is_BOM==1)) u_fputc_UTF8(U_BYTE_ORDER_MARK,f);

   return new_U_FILE(f,encoding);
}
/* Here we have U_READ or U_WRITE */
f=real_fopen(name,(MODE==U_READ)?"rb":"wb");
int c;
if (f==NULL) return NULL;
/* If the file is opened in READ mode and if we are in UTF16,
 * we check the presence of the byte order mark. */
if (MODE==U_READ) {
   if (MASK_ENCODING_COMPATIBILITY != USE_ENCODING_VALUE)
      if (GetFileEncoding(f,&encoding,&is_BOM,MASK_ENCODING_COMPATIBILITY) == 0)
      {
          af_fclose(f);
          error("u_fopen error: %s is not a compatible text file\n",name);
          return NULL;
      }

   if ((encoding==UTF16_LE) && (is_BOM!=0)) {
      c=u_fgetc_UTF16LE(f);
      if (c!=U_BYTE_ORDER_MARK) {
         error("u_fopen error: %s is not a UTF16-LE text file\n",name);
         af_fclose(f);
         return NULL;
      }
      return new_U_FILE(f,encoding);
   }

   if ((encoding==BIG_ENDIAN_UTF16) && (is_BOM!=0)) {
      c=u_fgetc_UTF16BE(f);
      if (c!=U_BYTE_ORDER_MARK) {
         error("u_fopen error: %s is not a UTF16-BE text file\n",name);
         af_fclose(f);
         return NULL;
      }
      return new_U_FILE(f,encoding);
   }

   if ((encoding==UTF8) && (is_BOM==1)) {
      c=u_fgetc_UTF8(f);
      if (c!=U_BYTE_ORDER_MARK) {
         error("u_fopen error: %s is not a UTF8 text file\n",name);
         af_fclose(f);
         return NULL;
      }
      return new_U_FILE(f,encoding);
   }

   return new_U_FILE(f,encoding);
}
/* If the file is opened in WRITE mode, we may insert the 0xFEFF unicode char */
if (MODE==U_WRITE) {
   if ((encoding==UTF16_LE) && (is_BOM!=0)) u_fputc_UTF16LE(U_BYTE_ORDER_MARK,f);
   else if ((encoding==BIG_ENDIAN_UTF16) && (is_BOM!=0)) u_fputc_UTF16BE(U_BYTE_ORDER_MARK,f);
   else if ((encoding==UTF8) && (is_BOM==1)) u_fputc_UTF8(U_BYTE_ORDER_MARK,f);
}
return new_U_FILE(f,encoding);
}

/*
 * Create OR Open existing file with specific encoding
 * 2 as BOM mean : BOM only for UTF16LE and UTF16BE (like previuous Unitex 2.1 revision 1045)
 */
U_FILE* u_fopen(Encoding encoding,const char* name,OpenMode MODE) {
    return u_fopen_internal(encoding,2,name,MODE,USE_ENCODING_VALUE);
}

/*
 * open existing text file, whith versatile encoding compatibility
 * the encoding parameter we give will be used only if we create file
 */
U_FILE* u_fopen_versatile_encoding(Encoding encoding,int write_bom,int MASK_ENCODING_COMPATIBILITY,const char* name,OpenMode MODE) {
    return u_fopen_internal(encoding,write_bom,name,MODE,MASK_ENCODING_COMPATIBILITY);
}

/*
 * Same function, dedicated for file that already exist, without parameter used only for file creation
 */
U_FILE* u_fopen_existing_versatile_encoding(int MASK_ENCODING_COMPATIBILITY,const char* name,OpenMode MODE) {
    return u_fopen_internal(UTF16_LE,1,name,MODE,MASK_ENCODING_COMPATIBILITY);
}

/*
 * Same function, dedicated for file that NOT already exist, without parameter used only for file creation
 * MODE must be U_WRITE
 */
U_FILE* u_fopen_creating_versatile_encoding(Encoding encoding,int write_bom,const char* name,OpenMode MODE) {
    return u_fopen_internal(encoding,write_bom,name,MODE,USE_ENCODING_VALUE);
}

/*
 * an opening creating function for Unitex specific file
 * (like .FST2, .INF, .TFST
 * MODE must be U_WRITE, be this parameter is not removed for easy modification of code
 * we try use encoding parameter, but we create always UTF (UTF16LE, UTF16BE or UTF8) with BOM
 */
U_FILE* u_fopen_creating_unitex_text_format(Encoding encoding,int write_bom,const char* name,OpenMode MODE) {
    if ((encoding != UTF16_LE) && (encoding != BIG_ENDIAN_UTF16))
        encoding = UTF8;
    write_bom = 1;
    return u_fopen_internal(encoding,write_bom,name,MODE,ALL_ENCODING_BOM_POSSIBLE);
}


/*
 * function to open same file than u_fopen_creating_unitex_text_format
 * this function accept all UTF with BOM file, regardless user parameter
 * so FST2, INF, TFST... file are universal
 * MODE must be U_READ, U_MODIFY or U_APPEND
 */
U_FILE* u_fopen_existing_unitex_text_format(const char* name,OpenMode MODE)
{
    return u_fopen_internal(UTF16_LE,1,name,MODE,ALL_ENCODING_BOM_POSSIBLE);
}

/**
 * Closes a file.
 */
int u_fclose(U_FILE* f) {
if (f==NULL) return 0;
int ret=af_fclose(f->f);
free_U_FILE(f);
return ret;
}


/**
 * This function creates an empty Unicode file that just contains the
 * byte order mark. It returns 0 if it fails; 1 otherwise.
 */
int u_fempty(Encoding encoding,int bom,const char* name) {
U_FILE* f=u_fopen_versatile_encoding(encoding,bom,USE_ENCODING_VALUE,name,U_WRITE);
if (f==NULL) {
   return 0;
}
u_fclose(f);
return 1;
}


/**
 * This function tests if the given file name correspond to a UTF16 file.
 * obsolete
 */
int u_is_UTF16(const char* name) {
ABSTRACTFILE* f=real_fopen(name,"rb");
unsigned char tab[4];

if (f==NULL) {
   /* If the file does not exist */
   return FILE_DOES_NOT_EXIST;
}
size_t r1=af_fread(&tab[0],1,2,f);
af_fclose(f);
if (r1 == 2)
  if ((tab[0] == 0xff) && (tab[1]==0xfe))
      return UTF16_LITTLE_ENDIAN_FILE;

if (r1 == 2)
  if ((tab[0] == 0xfe) && (tab[1]==0xff))
      return UTF16_BIG_ENDIAN_FILE;

return NOT_A_UTF16_FILE;
}

/**
 * This function tests if opened file correspond to a UTF16 file.
 */
int u_is_UTF16(U_FILE *f) {
 if (f->enc == UTF16_LE)
     return UTF16_LITTLE_ENDIAN_FILE;
 if (f->enc == BIG_ENDIAN_UTF16)
     return UTF16_BIG_ENDIAN_FILE;
 return NOT_A_UTF16_FILE;
}

/**
 * UTF16-LE version of fgetc. It does not read a char after reading 0x0D.
 * It returns EOF if it cannot read a UTF16-LE character. Moreover, it
 * prints an error message if it can read just one byte.
 */
int u_fgetc_UTF16LE_raw(ABSTRACTFILE* f) {
unsigned char tab[2];
size_t ret_read = af_fread(&tab[0],1,2,f);
if (ret_read == 2)
  return (((int)tab[1]) << 8) | tab[0];
if (ret_read == 1)
  error("Alignment error: odd number of characters in a UTF16 file\n");
return EOF;
}


/**
 * UTF16-BE version of fgetc. It does not read a char after reading 0x0D.
 * It returns EOF if it cannot read a UTF16-BE character. Moreover, it
 * prints an error message if it can read just one byte.
 */
int u_fgetc_UTF16BE_raw(ABSTRACTFILE* f) {
unsigned char tab[2];
size_t ret_read = af_fread(&tab[0],1,2,f);
if (ret_read == 2)
  return (((int)tab[0]) << 8) | tab[1];
if (ret_read == 1)
  error("Alignment error: odd number of characters in a UTF16 file\n");
return EOF;
}


/**
 * Reads an UTF8 encoded character from the given file and returns its
 * unicode number. Returns EOF if the end of file has been reached.
 * Prints an error and returns '?' if the end of file is found while reading a
 * compound character, or if there is an encoding error.
 *
 * IMPORTANT: This function allows reading characters > 65536, so if
 *            it is used only for 16 bits unicode, the caller
 *            must check that the value is not greater than expected.
 */
int u_fgetc_UTF8_raw(ABSTRACTFILE* f) {
unsigned char c;
unsigned char tab[8];
if (af_fread(&c,1,1,f)!=1) return EOF;
if (c<=0x7F) {
   /* Case of a 1 byte character 0XXX XXXX */
   return c;
}
/* Case of a character encoded on several bytes */
int number_of_bytes;
unsigned int value;
if ((c&0xE0)==0xC0) {
   /* 2 bytes 110X XXXX*/
   value=c&31;
   number_of_bytes=2;
}
else if ((c&0xF0)==0xE0) {
   /* 3 bytes 1110X XXXX */
   value=c&15;
   number_of_bytes=3;
}
else if ((c&0xF8)==0xF0) {
   /* 4 bytes 1111 0XXX */
   value=c&7;
   number_of_bytes=4;
}
else if ((c&0xFC)==0xF8) {
   /* 5 bytes 1111 10XX */
   value=c&3;
   number_of_bytes=5;
}
else if ((c&0xFE)==0xFC) {
   /* 6 bytes 1111 110X */
   value=c&1;
   number_of_bytes=6;
}
else {
   error("Encoding error in first byte of a unicode sequence\n");
   return '?';
}
/* If there are several bytes, we read them and compute the unicode
 * number of the character */
if (((int)af_fread(&tab[0],1,number_of_bytes-1,f)) != (number_of_bytes-1))
   return EOF;
for (int i=0;i<number_of_bytes-1;i++) {
   c = tab[i];
   /* Following bytes should be of the form 10XX XXXX */
   if ((c&0xC0)!=0x80) {
      error("Encoding error in byte %d of a %d byte unicode sequence\n",i+2,number_of_bytes);
      return '?';
   }
   value=(value<<6)|(c&0x3F);
}
return value;
}


/**
 * Unicode version of fgetc. It does not read a char after reading 0x0D.
 * In UTF16:
 * - It returns EOF if it cannot read a well-formed character. Moreover, it
 *   prints an error message if it can read just one byte.
 *
 * In UTF8:
 * - It returns EOF at the end of file or '?' if it cannot read a well-formed
 *   character. In that case, it prints an error message.
 */
int u_fgetc_raw(Encoding encoding,ABSTRACTFILE* f) {
switch(encoding) {
   case UTF16_LE: return u_fgetc_UTF16LE_raw(f);
   case BIG_ENDIAN_UTF16: return u_fgetc_UTF16BE_raw(f);
   case UTF8: return u_fgetc_UTF8_raw(f);
   case ASCII: {
	   unsigned char c;
	   if (af_fread(&c,1,1,f)==1)
		   return (int)c;
	   else
		   return EOF;
			   }
}
return EOF;
}


/**
 * Unicode version of fgetc. This function reads Windows-style end-of-lines,
 * i.e. if it reads a '\r', it skips the '\n' that is supposed to come after.
 *
 * See u_fgetc_raw for returned values.
 */
int u_fgetc(Encoding encoding,ABSTRACTFILE *f) {
int c=u_fgetc_raw(encoding,f);
if (c==0x0D) {
   /* If we read a '\r', we try to skip the '\n' */
   if (EOF==u_fgetc_raw(encoding,f)) return EOF;
   return '\n';
}
return c;
}


int u_ungetc_raw(Encoding,unichar,ABSTRACTFILE*);
/**
 * A version of u_fgetc that returns \n whatever it reads \n, \r or \r\n.
 */
int u_fgetc_CR(Encoding encoding,ABSTRACTFILE* f) {
int c=u_fgetc_raw(encoding,f);
if (c==EOF) {
   return EOF;
}
if (c==0x0A) {
   return '\n';
}
if (c==0x0D) {
   if (encoding==UTF8) {
      /* In UTF8, we know that if we look for a \n, it will take 1 byte. This
       * is a trick in order to avoid reading a character made of several bytes
       * that we should put back to the file. */
      if (af_fread(&c,1,1,f)!=1)
		  c=EOF;
   } else {
      c=u_fgetc_raw(encoding,f);
   }
   if (c==EOF) {
      return '\n';
   }
   if (c!=0x0A) {
      /* If there is no 0x0A after 0x0D, we put back the character */
      switch(encoding) {
         case UTF16_LE:
         case BIG_ENDIAN_UTF16: {
            u_ungetc_raw(encoding,(unichar)c,f);
            break;
         }
         case UTF8:
         case ASCII: {
            af_ungetc((char)c,f);
            break;
         }
      }
   }
   return '\n';
}
return c;
}


/**
 * Reads N characters and stores them in 't', that is supposed to be large enough.
 * Returns the number of characters read.
 * This function reads raw chars, since it does not convert \r\n into \n.
 *
 * WARNING: this function will be deprecated
 */
int u_fread_raw(Encoding encoding,unichar* t,int N,ABSTRACTFILE* f) {
int i,c;
for (i=0;i<N;i++) {
   c=u_fgetc_raw(encoding,f);
   if (c==EOF) return i;
   t[i]=(unichar)c;
}
return i;
}


/**
 * Reads N characters THAT ARE NOT '\0' and stores them in 't', that is supposed to be large enough.
 * Returns the number of characters read. This function converts \r\n into \n.
 *
 * The '*OK' parameter is set to 0 if at least one '\0' was found and ignored; 1 otherwise.
 */
int u_fread(Encoding encoding,unichar* t,int N,ABSTRACTFILE* f,int *OK) {
int i,c;
*OK=1;
i=0;
while (i<N) {
   c=u_fgetc_CR(encoding,f);
   if (c==EOF) return i;
   if (c=='\0') {
      *OK=0;
   } else {
      t[i++]=(unichar)c;
   }
}
return i;
}


/**
 * UTF16-LE version of fputc. It does not put a 0xOA after a 0x0D.
 * Returns 1 in case of success; 0 otherwise.
 */
int u_fputc_UTF16LE_raw(unichar c,ABSTRACTFILE *f) {
unsigned char tab[2];
int ret;
tab[1]=(unsigned char)(c >> 8);
tab[0]=(unsigned char)(c & 0xffff);
ret=(af_fwrite(&tab[0],1,2,f) == 2);
return ret;
}


/**
 * UTF16-BE version of fputc. It does not put a 0xOA after a 0x0D.
 * Returns 1 in case of success; 0 otherwise.
 */
int u_fputc_UTF16BE_raw(unichar c,ABSTRACTFILE *f) {
unsigned char tab[2];
int ret;
tab[0]=(unsigned char)(c >> 8);
tab[1]=(unsigned char)(c & 0xffff);
ret=(af_fwrite(&tab[0],1,2,f) == 2);
return ret;
}


/**
 * A version of putc that does not prints \r\n when you want it to print a \n.
 * Returns 1 in case of success; 0 otherwise.
 */
int fputc_raw(unsigned char c,ABSTRACTFILE *f) {
return (int)af_fwrite(&c,1,1,f);
}


/**
 * This function writes a 2-bytes unicode character in the given file
 * encoding it in UTF8. It does not put a 0xOA after a 0x0D.
 * Returns 0 if an error occurs; 1 otherwise.
 *
 * NOTE: as it takes a unichar, this function cannot be used for writing
 *       a unicode character > 0xFFFF
 */
int u_fputc_UTF8_raw(unichar c,ABSTRACTFILE *f) {
unsigned char tab[8];
unsigned int iCountByte;
if (c<=0x7F) {
   tab[0] = (unsigned char)c;
   iCountByte = 1;
}
else
if (c<=0x7FF) {
   tab[0]=(unsigned char) (0xC0 | (c>>6));
   tab[1]=(unsigned char) (0x80 | (c & 0x3F));
   iCountByte = 2;
}
else
{
    tab[0]=(unsigned char) (0xE0 | (c>>12));
    //tab[1]=(unsigned char) (0x80 | ((c-(c&12))>>6));   //$CD:20021119 old
    tab[1]=(unsigned char) (0x80 | ((c>>6)&0x3F));       //$CD:20021119
    tab[2]=(unsigned char) (0x80 | (c&0x3F));
    iCountByte = 3;
}
return (af_fwrite(&tab[0],1,iCountByte,f) == iCountByte);
}


/**
 * Unicode version of fputc. Returns 0 if an error occurs; 1 otherwise.
 */
int u_fputc_raw(Encoding encoding,unichar c,ABSTRACTFILE* f) {
switch(encoding) {
   case UTF16_LE: return u_fputc_UTF16LE_raw(c,f);
   case BIG_ENDIAN_UTF16: return u_fputc_UTF16BE_raw(c,f);
   case UTF8: return u_fputc_UTF8_raw(c,f);
   case ASCII: return (af_fwrite(&c,1,1,f) == 1);
}
return 0;
}


/**
 * Unicode version of fputc that saves '\n' as '\r\n'.
 * Returns 0 if an error occurs; 1 otherwise.
 */
int u_fputc(Encoding encoding,unichar c,ABSTRACTFILE* f) {
if (c=='\n') {
   if (!u_fputc_raw(encoding,0x0D,f)) return 0;
}
return u_fputc_raw(encoding,c,f);
}


/**
 * UTF16 version of ungetc. In fact, we just rewind 2 bytes before in 'f'.
 * At the opposite of the real ungetc, it does not push back the given
 * character to the stream.
 *
 * Returns 1 in case of success; 0 otherwise.
 */
int u_ungetc_UTF16_raw(ABSTRACTFILE *f) {
return (af_fseek(f,-2,SEEK_CUR)==0)?1:0;
}


/**
 * UTF16-LE version of ungetc. In fact, we just rewind 2 bytes before in 'f'.
 * At the opposite of the real ungetc, it does not push back the given
 * character to the stream.
 *
 * Returns 1 in case of success; 0 otherwise.
 */
int u_ungetc_UTF16LE_raw(ABSTRACTFILE *f) {
return u_ungetc_UTF16_raw(f);
}


/**
 * UTF16-BE version of ungetc. In fact, we just rewind 2 bytes before in 'f'.
 * At the opposite of the real ungetc, it does not push back the given
 * character to the stream.
 *
 * Returns 1 in case of success; 0 otherwise.
 */
int u_ungetc_UTF16BE_raw(ABSTRACTFILE *f) {
return u_ungetc_UTF16_raw(f);
}


/**
 * UTF8 version of ungetc. In fact, we just rewind 'n' bytes before in 'f',
 * where 'n' is the length of the UTF8 represention of 'c'.
 * At the opposite of the real ungetc, it does not push back the given
 * character to the stream.
 *
 * Returns 1 in case of success; 0 otherwise.
 */
int u_ungetc_UTF8_raw(unichar c,ABSTRACTFILE *f) {
int number_of_bytes;
if (c<=0x7F) {
   /* Case of a 1 byte character 0XXX XXXX */
   number_of_bytes=-1;
}
else if (c<=0x7FF) {
   /* 2 bytes 110X XXXX*/
   number_of_bytes=-2;
}
else /* The following test is always true: if (c<=0xFFFF) */ {
   /* 3 bytes 1110X XXXX */
   number_of_bytes=-3;
}
return (af_fseek(f,number_of_bytes,SEEK_CUR)==0)?1:0;
}


/**
 * Unicode version of ungetc. In fact, we just rewind 'n' bytes before in 'f',
 * where 'n' is the length of the represention of 'c' in the given encoding.
 * At the opposite of the real ungetc, it does not push back the given
 * character to the stream.
 *
 * Returns 1 in case of success; 0 otherwise.
 */
int u_ungetc_raw(Encoding encoding,unichar c,ABSTRACTFILE *f) {
switch(encoding) {
   case UTF16_LE: return u_ungetc_UTF16LE_raw(f);
   case BIG_ENDIAN_UTF16: return u_ungetc_UTF16BE_raw(f);
   case UTF8: return u_ungetc_UTF8_raw(c,f);
   case ASCII: return af_ungetc(c,f);
}
return 0;
}


/**
 * Unicode version of ungetc. In fact, we just rewind 'n' bytes before in 'f',
 * where 'n' is the length of the represention of 'c' in the given encoding.
 * If 'c' is '\n', then we also ungetc one char representing a '\r'.
 * At the opposite of the real ungetc, it does not push back the given
 * character to the stream.
 *
 * Returns 1 in case of success; 0 otherwise.
 */
int u_ungetc(Encoding encoding,unichar c,ABSTRACTFILE *f) {
if (c=='\n') {
   if (!u_ungetc_raw(encoding,c,f)) return 0;
   if (!u_ungetc_raw(encoding,c,f)) return 0;
   return 1;
}
return u_ungetc_raw(encoding,c,f);
}


/**
 * Writes N characters from t. Returns the number of characters written.
 * It does not write '\r\n' for '\n'.
 */
int u_fwrite_raw(Encoding encoding,unichar* t,int N,ABSTRACTFILE* f) {
for (int i=0;i<N;i++) {
   if (!u_fputc_raw(encoding,t[i],f)) return i;
}
return N;
}


/**
 * Writes N characters from t. Returns the number of characters written.
 * It writes '\r\n' for '\n'.
 */
int u_fwrite(Encoding encoding,unichar* t,int N,ABSTRACTFILE* f) {
for (int i=0;i<N;i++) {
   if (!u_fputc(encoding,t[i],f)) return i;
}
return N;
}


/**
 * Prints a char string into a file. Characters are promoted to unicode
 * and encoded according to the given encoding.
 */
void u_fprints_char(Encoding encoding,char* s,ABSTRACTFILE* f) {
int i=0;
while (s[i]!='\0')
   u_fputc(encoding,(unichar)((unsigned char)s[i++]),f);
}



int u_fgets_unbuffered(Encoding encoding,unichar* line,ABSTRACTFILE* f) {
int c;
int i=0;
while ((c=u_fgetc(encoding,f))!=EOF && c!='\n') {
   line[i++]=(unichar)c;
}
if (i==0 && c==EOF) {
   /* If we are at the end of FILE */
   return EOF;
}
line[i]='\0';
return i;
}


int u_fgets_unbuffered(Encoding encoding,unichar* line,int size,ABSTRACTFILE* f) {
int i=0;
int c=0;
while ((i < (size-1)) && ((c=u_fgetc(encoding,f))!=EOF)) {
   line[i++]=(unichar)c;
   if (c=='\n') break;
}
if (i==0 && c!='\n') return EOF;
line[i]='\0';
return i;
}


/**
  * this function act as u_fgets(encoding,line,f) if i_size==0 and
  *          like u_fgets(encoding,line,size,f) if i_size==1 and
  *	optimized by reading several char at same time in a buffer
  */


#define GetUtf8Size(ch)  \
        (((((unsigned char)(ch)) & ((unsigned char)0x80))==((unsigned char)0x00)) ? 1 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xe0))==((unsigned char)0xc0)) ? 2 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xf0))==((unsigned char)0xe0)) ? 3 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xf8))==((unsigned char)0xf0)) ? 4 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xfc))==((unsigned char)0xf8)) ? 5 : \
        (((((unsigned char)(ch)) & ((unsigned char)0xfe))==((unsigned char)0xfc)) ? 6 : 001))))))

#define GetUtf8Mask(ch)  \
        (((((unsigned char)(ch)) & ((unsigned char)0x80))==((unsigned char)0x00)) ? ((unsigned char)0x7f) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xe0))==((unsigned char)0xc0)) ? ((unsigned char)0x1f) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xf0))==((unsigned char)0xe0)) ? ((unsigned char)0x0f) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xf8))==((unsigned char)0xf0)) ? ((unsigned char)0x07) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xfc))==((unsigned char)0xf8)) ? ((unsigned char)0x03) : \
        (((((unsigned char)(ch)) & ((unsigned char)0xfe))==((unsigned char)0xfc)) ? ((unsigned char)0x01) : 0))))))


#define BUFFER_IN_CACHE_SIZE (0x100)


int u_fgets_buffered(Encoding encoding,unichar* line,int i_is_size,int size,ABSTRACTFILE* f,int treat_CR_as_LF,int suppress_null,int* found_null)
{
   unsigned char tab_in[BUFFER_IN_CACHE_SIZE];

   if ((i_is_size != 0) && (size == 0))
      return EOF;

   switch(encoding) {
      case UTF16_LE:
      case BIG_ENDIAN_UTF16:
        {
           int pos_in_unichar_line = 0;
           int hibytepos = (encoding == UTF16_LE) ? 1 : 0;

           for (;;)
           {
              int size_to_read_unichar = BUFFER_IN_CACHE_SIZE/2;
              if (i_is_size != 0)
                 if ((size-1) < BUFFER_IN_CACHE_SIZE)
                    size_to_read_unichar = size-1;

              size_t read_utf16_in_file = 0;
              if (size_to_read_unichar>0)
                 read_utf16_in_file = af_fread(&tab_in[0],2,(size_t)size_to_read_unichar/2,f);
              if (read_utf16_in_file<=0)
              {
                 if (pos_in_unichar_line == 0)
                    return EOF;
                 else
                 {
                    line[pos_in_unichar_line]='\0';
                    return pos_in_unichar_line;
                 }
              }

              if (read_utf16_in_file > 0)
              {
                 int i;
                 for (i=0;i<(int)read_utf16_in_file;i++)
                 {
                    unichar c;
                    c = (((unichar)tab_in[(i*2)+hibytepos]) << 8) | (tab_in[(i*2)+(1-hibytepos)]) ;
                    if (c==0)
                        if (found_null!=NULL)
                            *found_null=1;

                    if (!(((c==0x0d) && (treat_CR_as_LF == 0)) || ((c==0) && (suppress_null!=0))))
                    {
                       if ((c=='\n') || (c==0x0d))
                       {
                          int i_retract_for_newline=0;
                          if ((i_is_size==1) && (pos_in_unichar_line == (size-1)))
                              i_retract_for_newline=1;
                          if ((i_is_size==1) && (pos_in_unichar_line < (size-1))) {
                             line[pos_in_unichar_line++]=c;
                          }
                          line[pos_in_unichar_line]='\0';
                          af_fseek(f,-2 * (long)((read_utf16_in_file - (i+1))+i_retract_for_newline),SEEK_CUR);
                          return pos_in_unichar_line;
                       }

                       if ((i_is_size!=0) && (pos_in_unichar_line == (size-1)))
                       {
                          af_fseek(f,-2 * (long)(read_utf16_in_file - i),SEEK_CUR);
                          line[pos_in_unichar_line]='\0';
                          return pos_in_unichar_line;
                       }

                       line[pos_in_unichar_line++] = c;
                    }
                 }
              }
           }
        }


       case ASCII:
        {
           int pos_in_unichar_line = 0;

           for (;;)
           {
              int size_to_read_binary = BUFFER_IN_CACHE_SIZE;
              if (i_is_size != 0)
                 if ((size-1) < BUFFER_IN_CACHE_SIZE)
                    size_to_read_binary = size-1;

              size_t read_ascii_in_file = 0;
              if (size_to_read_binary>0)
                 read_ascii_in_file = af_fread(&tab_in[0],1,(size_t)size_to_read_binary,f);
              if (read_ascii_in_file<=0)
              {
                 if (pos_in_unichar_line == 0)
                    return EOF;
                 else
                 {
                    line[pos_in_unichar_line]='\0';
                    return pos_in_unichar_line;
                 }
              }

              if (read_ascii_in_file > 0)
              {
                 int i;
                 for (i=0;i<(int)read_ascii_in_file;i++)
                 {
                    unichar c;
                    c = (((unichar)tab_in[(i)])) ;
                    
                    if (c==0)
                        if (found_null!=NULL)
                            *found_null=1;

                    if (!(((c==0x0d) && (treat_CR_as_LF == 0)) || ((c==0) && (suppress_null!=0))))
                    {
                       if ((c=='\n') || (c==0x0d))
                       {
                          int i_retract_for_newline=0;
                          if ((i_is_size==1) && (pos_in_unichar_line == (size-1)))
                              i_retract_for_newline=1;
                          if ((i_is_size==1) && (pos_in_unichar_line < (size-1))) {
                             line[pos_in_unichar_line++]=c;
                          }
                          line[pos_in_unichar_line]='\0';
                          af_fseek(f,-1 * (long)((read_ascii_in_file - (i+1))+i_retract_for_newline),SEEK_CUR);
                          return pos_in_unichar_line;
                       }

                       if ((i_is_size!=0) && (pos_in_unichar_line == (size-1)))
                       {
                          af_fseek(f,-1 * (long)(read_ascii_in_file - i),SEEK_CUR);
                          line[pos_in_unichar_line]='\0';
                          return pos_in_unichar_line;
                       }

                       line[pos_in_unichar_line++] = c;
                    }
                 }
              }
           }
        }

       case UTF8:
        {
              /*  pos_already_read_in_disk is non zero when we have already read only
               *   a portion of an UTF8 char
               *   so tab_in contain the first pos_already_read_in_disk byte of the UTF8 char
               */
              size_t pos_already_read_in_disk = 0;
           int pos_in_unichar_line = 0;

           for (;;)
           {
              size_t size_to_read_binary = BUFFER_IN_CACHE_SIZE - pos_already_read_in_disk;

              size_t read_binary_in_file = 0;
              if (size_to_read_binary>0)
                 read_binary_in_file = af_fread(&tab_in[pos_already_read_in_disk],1,(size_t)size_to_read_binary,f);
              if (read_binary_in_file<=0)
              {
                 if (pos_in_unichar_line == 0)
                    return EOF;
                 else
                 {
                    line[pos_in_unichar_line]='\0';
                    return pos_in_unichar_line;
                 }
              }
                  read_binary_in_file += pos_already_read_in_disk;

              if (read_binary_in_file > 0)
              {
                 int i;
                 pos_already_read_in_disk=0;
                 for (i=0;i<(int)read_binary_in_file;)
                 {
                    unichar c;
                          unsigned char ch=tab_in[i];
                          int nbbyte=GetUtf8Size(ch);

                          if (i+nbbyte > (int)read_binary_in_file)
                          {
                              pos_already_read_in_disk = read_binary_in_file-i;
                              int j;
                              for (j=0;j<(int)pos_already_read_in_disk;j++)
                                  tab_in[j] = tab_in[i+j];
                              break;
                          }

                          c=((unichar)ch) & GetUtf8Mask(ch);
                          int nbbyte_loop=nbbyte;

                          if (nbbyte_loop>0) {
                              int i_in_char = 0;
                              for(;;)
                              {
                                   nbbyte_loop--;
                                   if (nbbyte_loop==0)
                                       break;
                                   i_in_char++;
                                   c = (c<<6) | ( (tab_in[i+i_in_char]) & 0x3F);
                              }
                          }


                    if (c==0)
                        if (found_null!=NULL)
                            *found_null=1;

                    if (!(((c==0x0d) && (treat_CR_as_LF == 0)) || ((c==0) && (suppress_null!=0))))
                    {
                       if ((c=='\n') || (c==0x0d))
                       {
                          int i_retract_for_newline=0;
                          if ((i_is_size==1) && (pos_in_unichar_line == (size-1)))
                              i_retract_for_newline=1;
                          if ((i_is_size==1) && (pos_in_unichar_line < (size-1))) {
                             line[pos_in_unichar_line++]=c;
                          }
                          line[pos_in_unichar_line]='\0';
                          af_fseek(f,-1 * (long)((read_binary_in_file - (i+nbbyte))+i_retract_for_newline),SEEK_CUR);
                          return pos_in_unichar_line;
                       }

                       if ((i_is_size!=0) && (pos_in_unichar_line == (size-1)))
                       {
                          af_fseek(f,-1 * (long)(read_binary_in_file - i),SEEK_CUR);
                          line[pos_in_unichar_line]='\0';
                          return pos_in_unichar_line;
                       }

                       line[pos_in_unichar_line++] = c;
                    }
                          i += nbbyte;
                 }
              }
           }
        }

      default:
         if (i_is_size==0)
            return u_fgets_unbuffered(encoding,line,f);
         else
            return u_fgets_unbuffered(encoding,line,size,f);
   }
}


/**
 * Reads from the file 'f' until it finds the end of line '\n' or
 * the end of file. The characters read are written in 'line'. The
 * function returns EOF if the current position in the file is at the
 * end of file; otherwise, it returns the number of characters read, possibly
 * 0 if there is an empty line.
 *
 * NOTE: there is no overflow control!
 */
int u_fgets(Encoding encoding,unichar* line,ABSTRACTFILE* f) {
	return u_fgets_buffered(encoding,line,0,0,f,0,0,NULL);
}


/**
 * Reads a complete line or at most (size-1) unichars if the line is too long.
 * If the line is not too long, the '\n' is put before the final '\0'.
 * The function skips all '\r' and the resulting buffer is always '\0' ended.
 * It returns the length of the string:
 * - EOF : means that it was the end of file
 * - 0 : means that we have read an empty line ended by '\n'
 * - (len ==(size-1)) and (line[len-1] != '\n') : means that the line was too long for the buffer
 * - (0 < len < size-1) : means that we have read a complete line (str[len-1]=='\n', unless EOF)
 *
 * Author: Olivier Blanc
 * Modified by Sébastien Paumier
 */
int u_fgets(Encoding encoding,unichar* line,int size,ABSTRACTFILE* f) {
return u_fgets_buffered(encoding,line,1,size,f,0,0,NULL);
}

/*
 * same thing, but all CR are converted as LF
 */
int u_fgets_treat_cr_as_lf(Encoding encoding,unichar* line,int size,ABSTRACTFILE* f,int supress_null,int* found_null) {
return u_fgets_buffered(encoding,line,1,size,f,1,supress_null,found_null);
}


/**
 * Reads a complete line or at most (size-1) unichars if the line is too long.
 * If the line is not too long, the '\n' is put before the final '\0'.
 * The function skips all '\r' and the resulting buffer is always '\0' ended.
 * It returns the length of the string:
 * - EOF : means that it was the end of file
 * - 0 : means that we have read an empty line ended by '\n'
 * - (len ==(size-1)) : means that the line was too long for the buffer (or fill exactly!)
 * - (0 < len < size-1) : means that we have read a complete line (str[len]=='\0', str[len-1]!='\n')
 *
 *      this function is useful to don't have to remove the \n at end of line
 *
 * Original author: Olivier Blanc
 * Modified by Sébastien Paumier
 * option limit2 by Gilles Vollant
 */
int u_fgets_limit2(Encoding encoding,unichar* line,int size,ABSTRACTFILE* f) {
	return u_fgets_buffered(encoding,line,2,size,f,0,0,NULL);
}


/**
 * This function acts exactly as 'u_fgets' does, except that
 * it stops at an end of line if and only if it is not protected by
 * a backslash. Backslashe that are not immediately before a '\n' are taken
 * as normal characters. The function returns the length of 'line'.
 * NOTE: this is an approximation, since we cannot represent a single line
 * ended by a backslash.
 *
 * Example:
 *
 * abc\de\
 * ef
 *
 * will lead to a string like: a b c \ d e \n e f
 */
int u_fgets2(Encoding encoding,unichar* line,ABSTRACTFILE* f) {
int pos,length;
if (EOF==(pos=u_fgets(encoding,line,f))) {
   /* If we are at the end of file, then we return EOF */
   return EOF;
}
if (pos==0) {
   /* If we have read an empty line, there is nothing more to do */
   return 0;
}
length=pos;
/* Otherwise, we check if the line we have just read is ended by a backslash.
 * In that case, we add a \n to it and we read another line. */
while (line[length-1]=='\\') {
   /* We try to read another line. We try to store it at &(line[length]),
    * because, if we can read such a line, we will have to replace the
    * backslash by a \n */
   pos=u_fgets(encoding,&(line[length]),f);
   if (pos==EOF) {
      /* If we cannot read another line, we return the current length */
      return length;
   }
   /* Otherwise, we put a \n before the line we have just read, and we update the length */
   line[length-1]='\n';
   length=length+pos;
}
return length;
}


/* Now : Gilles Vollant code for write unicode string "one call"
*/
#define BUFFER_OUT_CACHE_SIZE (0x80)
typedef struct
{
    unsigned char tabOut[BUFFER_OUT_CACHE_SIZE+4];
    int iPosInTabOut;
} Buffer_Out;

void ClearBufferOut(Buffer_Out* pBufOut)
{
    pBufOut->iPosInTabOut=0;
}

int FlushBufferOut(Buffer_Out* pBufOut,ABSTRACTFILE* f)
{
    size_t write_done_res ;
    size_t to_be_written;
    if (pBufOut->iPosInTabOut == 0)
        return 1;
    to_be_written = pBufOut->iPosInTabOut;
    write_done_res = af_fwrite(pBufOut->tabOut,1,to_be_written,f) ;
    pBufOut->iPosInTabOut=0;
    return (write_done_res == to_be_written);
}


int BuildEncodedOutForUnicharString(Encoding encoding,unichar *pc,Buffer_Out* pBufOut,int convLFtoCRLF,ABSTRACTFILE* f)
{
    while ((*pc)!=0)
    {
        unichar c=*pc;
        // with 4 char, we are sure all unichar will be written, including CRLF
        if ((pBufOut->iPosInTabOut + 4) >= BUFFER_OUT_CACHE_SIZE)
        {
            if (FlushBufferOut(pBufOut,f)==0)
                return 0;
        }
        if ((c=='\n') && (convLFtoCRLF!=0))
        {
            unichar CR[2];
            CR[0]=0x0D;
            CR[1]=0;
            BuildEncodedOutForUnicharString(encoding,&CR[0],pBufOut,0,f);
        }

        switch(encoding)
        {
           case UTF16_LE:
               {
                   pBufOut->tabOut[(pBufOut->iPosInTabOut)++]=(unsigned char)(c & 0xffff);
                   pBufOut->tabOut[(pBufOut->iPosInTabOut)++]=(unsigned char)(c >> 8);
                   break;
               }

           case BIG_ENDIAN_UTF16:
               {
                   pBufOut->tabOut[(pBufOut->iPosInTabOut)++]=(unsigned char)(c >> 8);
                   pBufOut->tabOut[(pBufOut->iPosInTabOut)++]=(unsigned char)(c & 0xffff);
                   break;
               }
           case BINARY:
           {
        	   pBufOut->tabOut[(pBufOut->iPosInTabOut)++]=(unsigned char)(c);
        	   break;
           }

           case UTF8:
               {
                    if (c<=0x7F) {
                       pBufOut->tabOut[(pBufOut->iPosInTabOut)++] = (unsigned char)c;
                    }
                    else
                    if (c<=0x7FF) {
                       pBufOut->tabOut[(pBufOut->iPosInTabOut)++] = (unsigned char) (0xC0 | (c>>6));
                       pBufOut->tabOut[(pBufOut->iPosInTabOut)++] = (unsigned char) (0x80 | (c & 0x3F));
                    }
                    else
                    {
                        pBufOut->tabOut[(pBufOut->iPosInTabOut)++] = (unsigned char) (0xE0 | (c>>12));

                        //pBufOut->tabOut[(pBufOut->iPosInTabOut)++]=(unsigned char) (0x80 | ((c-(c&12))>>6));   //$CD:20021119 old
                        pBufOut->tabOut[(pBufOut->iPosInTabOut)++] = (unsigned char) (0x80 | ((c>>6)&0x3F));       //$CD:20021119

                        pBufOut->tabOut[(pBufOut->iPosInTabOut)++] = (unsigned char) (0x80 | (c&0x3F));
                    }
               }
        }


        pc++;
    }

    return 1;
}

int BuildEncodedOutForUnicharItem(Encoding encoding,unichar w,Buffer_Out* pBufOut,int convLFtoCRLF,ABSTRACTFILE* f)
{
    unichar tab[2];
    tab[0]=w;
    tab[1]=0;
    return BuildEncodedOutForUnicharString(encoding,&tab[0],pBufOut,convLFtoCRLF,f);
}

int BuildEncodedOutForCharString(Encoding encoding,const char *pc,Buffer_Out* pBufOut,int convLFtoCRLF,ABSTRACTFILE* f)
{
    while ((*pc)!=0)
    {
        unichar c=(unichar)(*pc);
        // with 4 char, we are sure all unichar will be written, including CRLF
        if ((pBufOut->iPosInTabOut + 4) >= BUFFER_OUT_CACHE_SIZE)
        {
            if (FlushBufferOut(pBufOut,f)==0)
                return 0;
        }
        if ((c=='\n') && (convLFtoCRLF!=0))
        {
            unichar CR[2];
            CR[0]=0x0D;
            CR[1]=0;
            BuildEncodedOutForUnicharString(encoding,&CR[0],pBufOut,0,f);
        }

        switch(encoding)
        {
           case UTF16_LE:
               {
                   pBufOut->tabOut[(pBufOut->iPosInTabOut)++]=(unsigned char)(c & 0xffff);
                   pBufOut->tabOut[(pBufOut->iPosInTabOut)++]=(unsigned char)(c >> 8);
                   break;
               }

           case BIG_ENDIAN_UTF16:
               {
                   pBufOut->tabOut[(pBufOut->iPosInTabOut)++]=(unsigned char)(c >> 8);
                   pBufOut->tabOut[(pBufOut->iPosInTabOut)++]=(unsigned char)(c & 0xffff);
                   break;
               }

           case BINARY: {
        	   pBufOut->tabOut[(pBufOut->iPosInTabOut)++] = (unsigned char)c;
        	   break;
           }

           case UTF8:
               {

                    if (c<=0x7F) {
                       pBufOut->tabOut[(pBufOut->iPosInTabOut)++] = (unsigned char)c;
                    }
                    else
                    if (c<=0x7FF) {
                       pBufOut->tabOut[(pBufOut->iPosInTabOut)++] = (unsigned char) (0xC0 | (c>>6));
                       pBufOut->tabOut[(pBufOut->iPosInTabOut)++] = (unsigned char) (0x80 | (c & 0x3F));
                    }
                    else
                    {
                        pBufOut->tabOut[(pBufOut->iPosInTabOut)++] = (unsigned char) (0xE0 | (c>>12));

                        //pBufOut->tabOut[(pBufOut->iPosInTabOut)++]=(unsigned char) (0x80 | ((c-(c&12))>>6));   //$CD:20021119 old
                        pBufOut->tabOut[(pBufOut->iPosInTabOut)++] = (unsigned char) (0x80 | ((c>>6)&0x3F));       //$CD:20021119

                        pBufOut->tabOut[(pBufOut->iPosInTabOut)++] = (unsigned char) (0x80 | (c&0x3F));
                    }
               }
        }


        pc++;
    }
/*
    if (flush != 0)
    {
        if (FlushBufferOut(pBufOut,f)==0)
            return 0;
    }
*/
    return 1;
}

/**
 * Unicode version of fprintf. It supports all the printf format options.
 * It also supports:
 * - %C for printing a unicode character
 * - %S for printing a unicode string
 * - %R for printing the reversed of a unicode string
 * - %H combined with one of the 3 previous can be used to display HTML
 *   things. For instance, if we do u_printf("%HC",c); it will print:
 *   &lt;   if c='<'
 *   &gt;   if c='>'
 *   &amp;  if c='&'
 *   c      otherwise
 *   See 'htmlize' for details.
 *
 * - %U works in the same way than %H, but it invokes 'URLize'
 *
 * Author: Sébastien Paumier
 * Original version with format option restrictions: Olivier Blanc
 */
int u_vfprintf(U_FILE* ufile,const char* format,va_list list) {
Encoding encoding=ufile->enc;
ABSTRACTFILE* f=ufile->f;
int n_printed=0;
int i;
double d;
char c;
void* p;
unichar uc;
char* s;
unichar* us;
Buffer_Out BufOut;
ClearBufferOut(&BufOut);

while (*format) {
   if (*format=='%') {
      /* If we have a special sequence introduced by '%' */
      format++;
      switch (*format) {
         /* If we have %% we must print a '%' */
         case '%': BuildEncodedOutForUnicharItem(encoding,'%',&BufOut,0,f); n_printed++; break;

         /* If we have %c or %C we must print an unicode character */
         case 'c': /* We intercept %c here, because of the \n that always
                      must be encoded as \r\n */
         case 'C': {
            uc=(unichar)va_arg(list,int);
            BuildEncodedOutForUnicharItem(encoding,uc,&BufOut,1,f);
            n_printed++;
            break;
         }

         case 'H':
         case 'U': {
            int (*XXXize)(unichar*,unichar*);
            if (*format=='H') {
            	XXXize=htmlize;
            } else {
            	XXXize=URLize;
            }
        	/* If we have a '%H' (or '%U'), it means that we have to print HTML things (or URLs) */
            format++;
            if (*format=='C' || *format=='c') {
               /* If we have to print a HTML character */
               unichar tmp[2];
               tmp[0]=(unichar)va_arg(list,int);
               tmp[1]='\0';
               unichar html[32];
               int l=XXXize(tmp,html);
               BuildEncodedOutForUnicharString(encoding,html,&BufOut,1,f);
               n_printed=n_printed+l;
            } else if (*format=='S') {
               /* If we have to print a HTML string */
               us=va_arg(list,unichar*);
               if (us==NULL) {
                  BuildEncodedOutForCharString(encoding,"(null)",&BufOut,1,f);
                  n_printed=n_printed+6;
               } else {
                  unichar html[4096];
                  int l=XXXize(us,html);
                  BuildEncodedOutForUnicharString(encoding,html,&BufOut,1,f);
                  n_printed=n_printed+l;
               }
            } else if (*format=='R') {
               /* If we have to print a HTML reversed string */
               us=va_arg(list,unichar*);
               if (us==NULL) {
                  BuildEncodedOutForCharString(encoding,"(null)",&BufOut,1,f);
                  n_printed=n_printed+6;
               } else {
                  unichar reversed[4096];
                  mirror(us,reversed);
                  unichar html[4096];
                  int l=XXXize(reversed,html);
                  BuildEncodedOutForUnicharString(encoding,html,&BufOut,1,f);
                  n_printed=n_printed+l;
               }
            } else fatal_error("Invalid format option %c%c\n",*(format-1),*format);
            break;
         }

         /* If we have %S we must print an unicode string */
         case 'S': {
            us=va_arg(list,unichar*);
            if (us==NULL) {
               BuildEncodedOutForCharString(encoding,"(null)",&BufOut,1,f);
               n_printed=n_printed+6;
            } else {
               BuildEncodedOutForUnicharString(encoding,us,&BufOut,1,f);
               n_printed=n_printed+u_strlen(us);
            }
            break;
         }

         /* If we have %R we must print a reversed unicode string */
         case 'R': {
            us=va_arg(list,unichar*);
            if (us==NULL) {
               /* We don't want to print ")llun(" when the string to reverse is NULL */
               BuildEncodedOutForCharString(encoding,"(null)",&BufOut,1,f);
               n_printed=n_printed+6;
               break;
            }
            unichar reversed[4096];
            n_printed=n_printed+mirror(us,reversed);
            BuildEncodedOutForUnicharString(encoding,reversed,&BufOut,1,f);
            break;
         }

         /* If we have %n, we must store the number of characters that have
          * already been printed into the given int*. */
         case 'n': {
            int *res=va_arg(list,int*);
            *res=n_printed;
            break;
         }

         /* If we have '%???', we let sprintf do the job */
         default: {
            /* We get back on the '%' */
            format--;
            int z=0;
            char format2[64];
            char result[4096];
            do {
               format2[z++]=*format;
               format++;
            } while (format2[z-1]!='\0' && !strchr("diouxXeEfgcsp",format2[z-1]));
            /* We get back one character */
            format--;
            if (format2[z-1]=='\0') {
               fatal_error("Invalid format option in u_printf:\n%s\n",format2);
            }
            format2[z]='\0';
            switch (format2[z-1]) {
               case 'd': case 'i': case 'o': case 'u': case 'x': case 'X': {
                  i=va_arg(list,int);
                  n_printed=n_printed+sprintf(result,format2,i);
                  break;
               }
               case 'e':  case 'E': case 'f':  case 'g': {
                  d=va_arg(list,double);
                  n_printed=n_printed+sprintf(result,format2,d);
                  break;
               }
               case 'c': {
                  c=(char)(va_arg(list,int));
                  n_printed=n_printed+sprintf(result,format2,c);
                  break;
               }
               case 's': {
                  s=va_arg(list,char*);
                  n_printed=n_printed+sprintf(result,format2,s);
                  break;
               }
               case 'p': {
                  p=va_arg(list,void*);
                  n_printed=n_printed+sprintf(result,format2,p);
                  break;
               }
            }
            BuildEncodedOutForCharString(encoding,result,&BufOut,1,f);
            break;
         }
      }
   } else {
      /* If we have a normal character, we print it */
      BuildEncodedOutForUnicharItem(encoding,(unsigned char)*format,&BufOut,1,f);
      n_printed++;
   }
   format++;
}
FlushBufferOut(&BufOut,f);
return n_printed;
}


/**
 * Unicode version of fprintf. See u_vfprintf for supported format options.
 *
 * Author: Olivier Blanc
 * Modified by Sébastien Paumier
 */
int u_fprintf(U_FILE* f,const char* format,...) {
va_list list;
va_start(list,format);
int n=u_vfprintf(f,format,list);
va_end(list);
return n;
}


/**
 * This function prints a message into a unicode string and returns the
 * size of the result. Note that if 'dest' is NULL, the function will
 * only count the size of the result without actually building it.
 * See u_fprintf for supported format options.
 *
 * Author: Olivier Blanc
 */
int u_sprintf(unichar* dest,const char* format,...) {
va_list list;
va_start(list,format);
int n=u_vsprintf(dest,format,list);
va_end(list);
return n;
}


/**
 * Unicode version of sprintf. It supports all the printf format options.
 * It also supports:
 * - %C for printing a unicode character
 * - %S for printing a unicode string
 * - %HS for printing a unicode string in HTML (see htmlize)
 * - %R for printing the reversed of a unicode string
 * - %HR for printing the reversed of a unicode string in HTML (see htmlize)
 * - %US for printing a unicode string as a URL (see URLize)
 * - %UR for printing the reversed of a unicode string as a URL (see URLize)
 *
 * Author: Sébastien Paumier
 * Original version with format option restrictions: Olivier Blanc
 */
int u_vsprintf(unichar* dest,const char* format,va_list list) {
int n_printed=0;
int i;
double d;
char c;
void* p;
unichar uc;
char* s;
unichar* us;
while (*format) {
   if (*format=='%') {
      /* If we have a special sequence introduced by '%' */
      format++;
      switch (*format) {
         /* If we have %% we must print a '%' */
         case '%': if (dest) dest[n_printed]='%'; n_printed++; break;

         /* If we have %C we must print an unicode character */
         case 'C': {
            uc=(unichar)va_arg(list,int);
            if (dest) dest[n_printed]=uc;
            n_printed++;
            break;
         }

         case 'H':
         case 'U': {
            int (*XXXize)(unichar*,unichar*);
            if (*format=='H') {
            	XXXize=htmlize;
            } else {
            	XXXize=URLize;
            }
        	/* If we have a '%H', it means that we have to print HTML things */
            format++;
            if (*format=='S') {
               /* If we have to print a HTML string */
               us=va_arg(list,unichar*);
               if (us==NULL) {
                  if (dest) u_strcpy(&(dest[n_printed]),"(null)");
                  n_printed=n_printed+6;
               } else {
                  unichar html[4096];
                  int l=XXXize(us,html);
                  if (dest) u_strcpy(&(dest[n_printed]),html);
                  n_printed=n_printed+l;
               }
            } else if (*format=='R') {
               /* If we have to print a HTML reversed string */
               us=va_arg(list,unichar*);
               if (us==NULL) {
                  if (dest) u_strcpy(&(dest[n_printed]),"(null)");
                  n_printed=n_printed+6;
               } else {
                  unichar reversed[4096];
                  mirror(us,reversed);
                  unichar html[4096];
                  int l=XXXize(reversed,html);
                  if (dest) u_strcpy(&(dest[n_printed]),html);
                  n_printed=n_printed+l;
               }
            } else fatal_error("Invalid format option %c%c\n",*(format-1),*format);
            break;
         }

         /* If we have %S we must print an unicode string */
         case 'S': {
            us=va_arg(list,unichar*);
            if (us==NULL) {
               if (dest) u_strcpy(&(dest[n_printed]),"(null)");
               n_printed=n_printed+6;
            } else {
               if (dest) u_strcpy(&(dest[n_printed]),us);
               n_printed=n_printed+u_strlen(us);
            }
            break;
         }

         /* If we have %R we must print a reversed unicode string */
         case 'R': {
            us=va_arg(list,unichar*);
            if (us==NULL) {
               /* We don't want to print ")llun(" when the string to reverse is NULL */
               if (dest) u_strcpy(&(dest[n_printed]),"(null)");
               n_printed=n_printed+6;
               break;
            }
            unichar reversed[4096];
            int old=n_printed;
            n_printed=n_printed+mirror(us,reversed);
            if (dest) u_strcpy(&(dest[old]),reversed);
            break;
         }

         /* If we have %n, we must store the number of characters that have
          * already been printed into the given int*. */
         case 'n': {
            int *res=va_arg(list,int*);
            *res=n_printed;
            break;
         }

         /* If we have '%???', we let sprintf do the job */
         default: {
            /* We get back on the '%' */
            format--;
            int z=0;
            char format2[64];
            char result[4096];
            do {
               format2[z++]=*format;
               format++;
            } while (format2[z-1]!='\0' && !strchr("diouxXeEfgcsp",format2[z-1]));
            /* We get back one character */
            format--;
            if (format2[z-1]=='\0') {
               fatal_error("Invalid format option in u_printf:\n%s\n",format2);
            }
            format2[z]='\0';
            int n_printed_old=n_printed;
            switch (format2[z-1]) {
               case 'd': case 'i': case 'o': case 'u': case 'x': case 'X': {
                  i=va_arg(list,int);
                  n_printed=n_printed+sprintf(result,format2,i);
                  break;
               }
               case 'e':  case 'E': case 'f':  case 'g': {
                  d=va_arg(list,double);
                  n_printed=n_printed+sprintf(result,format2,d);
                  break;
               }
               case 'c': {
                  c=(char)va_arg(list,int);
                  n_printed=n_printed+sprintf(result,format2,c);
                  break;
               }
               case 's': {
                  s=va_arg(list,char*);
                  n_printed=n_printed+sprintf(result,format2,s);
                  break;
               }
               case 'p': {
                  p=va_arg(list,void*);
                  n_printed=n_printed+sprintf(result,format2,p);
                  break;
               }
            }
            if (dest) u_strcpy(&(dest[n_printed_old]),result);
            break;
         }
      }
   } else {
      /* If we have a normal character, we print it */
      if (dest) dest[n_printed]=(unsigned char)*format;
      n_printed++;
   }
   format++;
}
if (dest) dest[n_printed]='\0';
return n_printed;
}


/**
 * Returns a non-zero value if 'c' is an hexadecimal digit; 0 otherwise.
 */
int u_is_hexa_digit(unichar c) {
return (c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F');
}


/**
 * Returns a non-zero value if 'c' is a separator; 0 otherwise.
 */
int is_separator(unichar c) {
return (c==' ') || (c=='\t') || (c=='\r') || (c=='\n');
}


/**
 * Unicode version of fscanf. The supported format options are:
 * %c : normal character
 * %C : unicode character
 * %s : normal string
 * %S : unicode string
 * %d : decimal integer of the form 45 -45 or +45
 * %x : hexadecimal integer of the form a9 -B75 or +f8CE
 *
 * The separators are space, tabulation '\r' and '\n'. They are skipped in
 * input stream BUT NOT IN FORMAT STRING!!! So, if we do:
 *
 *    u_fscanf(f,"%d %s",&i,s);
 *
 * it will match only if there is a space after the integer. For instance, you can use this
 * feature to eat EOL. If we want to an integer and the EOL sequences that follows,
 * just use:
 *
 *    u_fscanf(f,"%d\r\n",&i);
 *
 * Note 1: if this function is applied on stdin, it may have read one character in advance.
 * Note 2: if yout type u_fscanf(f,"%d\n",&i); the function will skip any separator that is
 *         not '\n' after the integer, so that the line " 45   \t   \n" will be entirely read.
 *
 * Author: Sébastien Paumier
 */
int u_vfscanf(U_FILE* ufile,const char* format,va_list list) {
Encoding encoding=ufile->enc;
ABSTRACTFILE* f=ufile->f;
int c;
int *i;
unichar *uc;
char *ch;
int n_variables=0;
static int stdin_ch=-1;
while (*format) {
   /* First, we get the current character */
   if (IsStdIn(f)) {
      /* If we read from the input stream, we may have to use the 1-char buffer */
      if (stdin_ch!=-1) {
         c=stdin_ch;
         stdin_ch=-1;
      } else {
         /* If we have no character in the 1-char buffer, we take one from the ABSTRACTFILE */
         c=u_fgetc_raw(encoding,f);
      }
   } else {
      /* If we have to take one from the ABSTRACTFILE */
      c=u_fgetc_raw(encoding,f);
   }
   if (c==EOF) {
      if (n_variables==0) {
         /* If the EOF occurs before the first conversion, we return EOF */
         return EOF;
      }
      /* Otherwise, we return the number of variables that have already been read */
      return n_variables;
   }
   /* Then we deal with the separators. If the current format char is a separator,
    * we have two cases: */
   if (is_separator(*format)) {
      if (c==*format) {
         /* 1) the format char is the same than the input char => we go on */
         format++;
         continue;
      } else {
         /* 2) the format is for instance a '\t' and we have a current input
          *    separator that is not a '\t' => we skip all separators that are not '\t' */
         while ((c=u_fgetc_raw(encoding,f))!=EOF && is_separator((unichar)c) && c!=*format) {}
         /* Subcase 1: EOF */
         if (c==EOF) return (n_variables==0)?EOF:n_variables;
         /* Subcase 2: we found the correct separator */
         if (c==*format) {
            format++;
            continue;
         }
         /* Subcase 3: we found a character that is not the expected separator => we fail */
         return n_variables;
      }
   }
   /* Now we must deal with an input separator when the current format character
    * is not a separator */
   while (c!=EOF && is_separator((unichar)c)) {
      c=u_fgetc_raw(encoding,f);
   }
   /* Again, we may have reached the EOF */
   if (c==EOF) {
      if (n_variables==0) {
         /* If the EOF occurs before the first conversion, we return EOF */
         return EOF;
      }
      /* Otherwise, we return the number of variables that have already been read */
      return n_variables;
   }
   if (*format=='%') {
      /* If we have a special sequence introduced by '%' */
      format++;
      switch (*format) {
         /* If we have %% we must read a '%' */
         case '%': {
            if (c!='%') return n_variables;
            break;
         }

         /* If we have %c we must read a normal character */
         case 'c': {
            ch=va_arg(list,char*);
            *ch=(char)c;
            n_variables++;
            break;
         }

         /* If we have %C we must read a unicode character */
         case 'C': {
            uc=va_arg(list,unichar*);
            *uc=(unichar)c;
            n_variables++;
            break;
         }

         /* If we have %s we must read a normal string */
         case 's': {
            ch=va_arg(list,char*);
            int pos=0;
            do {
               ch[pos++]=(char)c;
            } while ((c=u_fgetc_raw(encoding,f))!=EOF && !is_separator((unichar)c));
            ch[pos]='\0';
            if (c!=EOF) {
               /* If we have read a separator, we put it back in the file, for
                * the case where the user would like to read it with another read function */
               if (IsStdIn(f)) {
                  stdin_ch=c;
               }
               else {
                  u_ungetc_raw(encoding,(unichar)c,f);
               }
            }
            n_variables++;
            break;
         }

         /* If we have %S we must read a unicode string */
         case 'S': {
            uc=va_arg(list,unichar*);
            int pos=0;
            do {
               uc[pos++]=(unichar)c;
            } while ((c=u_fgetc_raw(encoding,f))!=EOF && !is_separator((unichar)c));
            uc[pos]='\0';
            if (c!=EOF) {
               /* If we have read a separator, we put it back in the file, for
                * the case where the user would like to read it with another read function */
               if (IsStdIn(f)) {
                  stdin_ch=c;
               }
               else {
                  u_ungetc_raw(encoding,(unichar)c,f);
               }
            }
            n_variables++;
            break;
         }

         /* If we have %d we must read a decimal integer, eventually preceeded by '+' or '-' */
         case 'd': {
            i=va_arg(list,int*);
            int multiplier=1;
            if (c=='+' || c=='-') {
               /* If we have a sign, we must read the next character */
               if (c=='-') multiplier=-1;
               c=u_fgetc_raw(encoding,f);
               if (c==EOF || c<'0' || c>'9') {
                  /* If we have reached the EOF or if we have a non digit character */
                  return n_variables;
               }
            } else if (c<'0' || c>'9') {
               /* If we have a character that neither a sign nor a digit, we fail */
               return n_variables;
            }
            *i=0;
            do {
               *i=(*i)*10+(unichar)c-'0';
            } while ((c=u_fgetc_raw(encoding,f))!=EOF && c>='0' && c<='9');
            *i=(*i)*multiplier;
            if (c!=EOF) {
               /* If we have read a non digit, we put it back in the file, for
                * the case where the user would like to read it with another read function */
               if (IsStdIn(f)) {
                  stdin_ch=c;
               }
               else {
                  u_ungetc_raw(encoding,(unichar)c,f);
               }
            }
            n_variables++;
            break;
         }

         /* If we have %x we must read an hexadecimal integer, eventually preceeded by '+' or '-' */
         case 'x': {
            i=va_arg(list,int*);
            int multiplier=1;
            if (c=='+' || c=='-') {
               /* If we have a sign, we must read the next character */
               if (c=='-') multiplier=-1;
               c=u_fgetc_raw(encoding,f);
               if (c==EOF || !u_is_hexa_digit((unichar)c)) {
                  /* If we have reached the EOF or if we have a non hexa digit character */
                  return n_variables;
               }
            } else if (!u_is_hexa_digit((unichar)c)) {
               /* If we have a character that neither a sign nor a digit, we fail */
               return n_variables;
            }
            *i=0;
            do {
               if (c>='0' && c<='9') c=c-'0';
               else if (c>='a' && c<='f') c=c-'a'+10;
               else c=c-'A'+10;
               *i=(*i)*16+c;
            } while ((c=u_fgetc_raw(encoding,f))!=EOF && u_is_hexa_digit((unichar)c));
            *i=(*i)*multiplier;
            if (c!=EOF) {
               /* If we have read a non digit, we put it back in the file, for
                * the case where the user would like to read it with another read function */
               if (IsStdIn(f)) {
                  stdin_ch=c;
               }
               else {
                  u_ungetc_raw(encoding,(unichar)c,f);
               }
            }
            n_variables++;
            break;
         }

         default: error("Unsupported format in u_vfscanf: %%%c\n",*format);

      }
   } else {
      /* If we have a normal character, we must read it */
      if (c!=*format) return n_variables;
   }
   format++;
}
return n_variables;
}


/**
 * Unicode version of fscanf. See u_vfscanf for supported format options.
 *
 * Author: Sébastien Paumier
 */
int u_fscanf(U_FILE* f,const char* format,...) {
va_list list;
va_start(list,format);
int n=u_vfscanf(f,format,list);
va_end(list);
return n;
}


/**
 * Unicode version of sscanf. See u_vfscanf for supported format options.
 *
 * Author: Sébastien Paumier
 */
int u_vsscanf(unichar* s,const char* format,va_list list) {
int c;
int *i;
unichar *uc;
char *ch;
int n_variables=0;
int pos=0;
while (*format) {
   /* First, we skip the separators, but only if the current format character is
    * not a separator */
   if (!is_separator(*format)) {
      while (is_separator(s[pos])) {
         pos++;
      }
      if (s[pos]=='\0' && !(*format=='%' && *(format+1)=='n')) {
         /* We will stop only if the current format is not %n, because %n can
          * work even if the end of the input has been reached */
         if (n_variables==0) {
            /* If the EOF occurs before the first conversion, we return EOF */
            return EOF;
         }
         /* Otherwise, we return the number of variables that have already been read */
         return n_variables;
      }
   } else {
      /* If we have a separator in the format string, we must look exactly for it */
      while (s[pos]!='\0' && is_separator(s[pos]) && s[pos]!=*format) pos++;
      if (s[pos]=='\0' || s[pos]!=*format) {
         /* If we have not found the wanted separator, we fail */
         return (n_variables==0)?EOF:n_variables;
      }
      format++;
      continue;
   }
   if (*format=='%') {
      /* If we have a special sequence introduced by '%' */
      format++;
      switch (*format) {
         /* If we have %% we must read a '%' */
         case '%': {
            if (s[pos]!='%') return n_variables;
            break;
         }

         /* If we have %c we must read a normal character */
         case 'c': {
            ch=va_arg(list,char*);
            *ch=(char)s[pos++];
            n_variables++;
            break;
         }

         /* If we have %C we must read a unicode character */
         case 'C': {
            uc=va_arg(list,unichar*);
            *uc=s[pos++];
            n_variables++;
            break;
         }

         /* If we have %s we must read a normal string */
         case 's': {
            ch=va_arg(list,char*);
            int pos2=0;
            do {
               ch[pos2++]=(char)s[pos++];
            } while (s[pos]!='\0' && !is_separator(s[pos]));
            ch[pos2]='\0';
            if (s[pos]!='\0') {
               /* If we have read a separator, we have nothing to do */
            }
            n_variables++;
            break;
         }

         /* If we have %S we must read a unicode string */
         case 'S': {
            uc=va_arg(list,unichar*);
            int pos2=0;
            do {
               uc[pos2++]=s[pos++];
            } while (s[pos]!='\0' && !is_separator(s[pos]));
            uc[pos2]='\0';
            if (s[pos]!='\0') {
               /* If we have read a separator, we have nothing to do */
            }
            n_variables++;
            break;
         }

         /* If we have %d we must read a decimal integer, eventually preceeded by '+' or '-' */
         case 'd': {
            i=va_arg(list,int*);
            int multiplier=1;
            if (s[pos]=='+' || s[pos]=='-') {
               /* If we have a sign, we must read the next character */
               if (s[pos]=='-') multiplier=-1;
               pos++;
               if (s[pos]=='\0' || s[pos]<'0' || s[pos]>'9') {
                  /* If we have reached the EOF or if we have a non digit character */
                  return n_variables;
               }
            } else if (s[pos]<'0' || s[pos]>'9') {
               /* If we have a character that neither a sign nor a digit, we fail */
               return n_variables;
            }
            *i=0;
            do {
               *i=(*i)*10+(unichar)s[pos++]-'0';
            } while (s[pos]!='\0' && s[pos]>='0' && s[pos]<='9');
            *i=(*i)*multiplier;
            if (s[pos]!='\0') {
               /* If we have read a non digit, we have nothing to do */
            }
            n_variables++;
            break;
         }

         /* If we have %x we must read an hexadecimal integer, eventually preceeded by '+' or '-' */
         case 'x': {
            i=va_arg(list,int*);
            int multiplier=1;
            if (s[pos]=='+' || s[pos]=='-') {
               /* If we have a sign, we must read the next character */
               if (s[pos]=='-') multiplier=-1;
               pos++;
               if (s[pos]=='\0' || !u_is_hexa_digit(s[pos])) {
                  /* If we have reached the EOF or if we have a non hexa digit character */
                  return n_variables;
               }
            } else if (!u_is_hexa_digit(s[pos])) {
               /* If we have a character that neither a sign nor a digit, we fail */
               return n_variables;
            }
            *i=0;
            do {
               c=s[pos++];
               if (c>='0' && c<='9') c=c-'0';
               else if (c>='a' && c<='f') c=c-'a'+10;
               else c=c-'A'+10;
               *i=(*i)*16+c;
            } while (s[pos]!='\0' && u_is_hexa_digit(s[pos]));
            *i=(*i)*multiplier;
            if (s[pos]!='\0') {
               /* If we have read a non digit, we have nothing to do */
            }
            n_variables++;
            break;
         }

         /* If we have %n we must store the number of characters that have already
          * been read from the input string */
         case 'n': {
            i=va_arg(list,int*);
            *i=pos;
            break;
         }

         default: error("Unsupported format in u_vsscanf: %%%c\n",*format);
      }
   } else {
      /* If we have a normal character, we must read it */
      if (s[pos++]!=*format) return n_variables;
   }
   format++;
}
return n_variables;
}


/**
 * Unicode version of sscanf. See u_vfscanf for supported format options.
 *
 * Author: Sébastien Paumier
 */
int u_sscanf(unichar* input,const char* format,...) {
va_list list;
va_start(list,format);
int n=u_vsscanf(input,format,list);
va_end(list);
return n;
}


/**
 * Prints an unicode string into a file.
 */
void u_fprints(Encoding encoding,const unichar* s,ABSTRACTFILE* f) {
int i=0;
if (s==NULL) {
   return;
}
while (s[i]!='\0') {
   u_fputc(encoding,s[i++],f);
}
}


/**
 * Prints a char string into a file.
 */
void u_fprints(Encoding encoding,const char* s,ABSTRACTFILE* f) {
int i=0;
if (s==NULL) {
   return;
}
while (s[i]!='\0') {
   u_fputc(encoding,(unichar)s[i++],f);
}
}


/* ------------------- String functions ------------------- */

/**
 * Unicode version of strlen.
 */
unsigned int u_strlen(const unichar* s) {
register int i=0;
while (s[i++]) {}
return (i-1);
}


/**
 * Unicode version of strcpy.
 */
unichar* u_strcpy(unichar* dest,const unichar* src) {
unichar *s = dest; // backup pointer to start of destination string
register unichar c;
do {
   c=*src++;
   *dest++=c;
} while (c!='\0');
return s;
}


/**
 * unicode version of strncpy
 */
unichar* u_strncpy(unichar *dest,const unichar *src,unsigned int n) {
register unichar c;
unichar *s = dest; // backup pointer to start of destination string
do {
   c = *src++;
   *dest++ = c;
   if (--n == 0)
     return s;
} while (c != 0);
// null-padding
do
  *dest++ = 0;
while (--n > 0);
return s;
}


/**
 * unicode version of a secure strcpy : like u_strncpy, but add 0 at end of string 
 * if truncate
 * do not full pad buffer with 0, just add one 0 to terminate string
 * typical usage :
 unichar dest[SIZE_BUFFER];
 or
 unichar*dest=malloc(sizeof(unichar)*SIZE_BUFFER);
 u_strcpy_sized(dest,SIZE_BUFFER,src);
 */
unichar* u_strcpy_sized(unichar *dest,size_t n,const unichar *src) {
if (n==0)
  return dest;
register unichar c;
unichar *s = dest; // backup pointer to start of destination string
do {
   n--;
   if (n == 0) {
     *dest = 0;
     return s;
   }
   c = *src++;
   *dest++ = c;
} while (c != 0);

return s;
}


/**
 * unicode version of a secure strcpy that takes a non unicode source string
 * like u_strncpy, but add 0 at end of string if truncate
 * do not full pad buffer with 0, just add one 0 to terminate string
 */
unichar* u_strcpy_sized(unichar *dest,size_t n,const char *src) {
if (n==0)
  return dest;
register unichar c;
unichar *s = dest; // backup pointer to start of destination string
do {
   n--;
   if (n == 0) {
     *dest = 0;
     return s;
   }
   c = *src++;
   *dest++ = c;
} while (c != 0);

return s;
}


/**
 * Unicode version of strcpy that takes a non unicode source string.
 */
unichar* u_strcpy(unichar* dest,const char* src) {
unichar *s = dest; // backup pointer to start of destination string
register unichar c;
do {
   c=*src++;
   *dest++=c;
} while (c!='\0');
return s;
}


/**
 * Unicode version of strcat.
 */
unichar* u_strcat(unichar* dest,const unichar* src) {
unichar *s1=dest;
const unichar *s2=src;
register unichar c;
/* First we go at the end of the destination string */
do {
	c=*s1++;
} while (c!=(unichar)'\0');
s1-=2;
/* And we concatenate the 'src' string */
do {
	c=*s2++;
	*++s1=c;
} while (c!=(unichar)'\0');
return dest;
}


/**
 * Unicode version of strcat.
 */
unichar* u_strcat(unichar* dest,const char* src) {
int i,j=0;
i=u_strlen(dest);
while ((dest[i++]=(unichar)((unsigned char)src[j++]))!=0) {}
return dest;
}


/**
 * Unicode version of strcmp that tolerates NULL strings.
 */
int u_strcmp(const unichar* a,const unichar* b) {
if ((a!=NULL) && (b!=NULL)) {
    const unichar *a_p=a;
    const unichar *b_p=b;
    unichar a_c;
    unichar b_c;


    for(;;) {
       a_c=(unichar)*(a_p);
       b_c=(unichar)*(b_p);
	   if (a_c=='\0')
		   return -b_c;
	   if (a_c-b_c!=0)
		   return (a_c-b_c);

       a_c=(unichar)*(a_p+1);
       b_c=(unichar)*(b_p+1);
	   if (a_c=='\0')
		   return -b_c;
	   if (a_c-b_c!=0)
		   return (a_c-b_c);

       a_c=(unichar)*(a_p+2);
       b_c=(unichar)*(b_p+2);
	   if (a_c=='\0')
		   return -b_c;
	   if (a_c-b_c!=0)
		   return (a_c-b_c);

       a_c=(unichar)*(a_p+3);
       b_c=(unichar)*(b_p+3);
       a_p+=4;
	   b_p+=4;
	   if (a_c=='\0')
		   return -b_c;
	   if (a_c-b_c!=0)
		   return (a_c-b_c);
    } ;
} else {
  if (a==NULL) {
       if (b==NULL) return 0;
       return 1;
    }
  return -1;
  }
}


/**
 * Unicode version of strcmp that tolerates NULL strings.
 */
int u_strcmp(const unichar* a,const char* b) {
if ((a!=NULL) && (b!=NULL)) {
    const unichar *a_p=a;
    const unsigned char *b_p=(const unsigned char*)b;
    unichar a_c;
    unichar b_c;

    for(;;) {
       a_c=(unichar)*(a_p);
       b_c=(unichar)((unsigned char)*(b_p));
	   if (a_c=='\0')
		   return -b_c;
	   if (a_c-b_c!=0)
		   return (a_c-b_c);

       a_c=(unichar)*(a_p+1);
       b_c=(unichar)((unsigned char)*(b_p+1));
	   if (a_c=='\0')
		   return -b_c;
	   if (a_c-b_c!=0)
		   return (a_c-b_c);

       a_c=(unichar)*(a_p+2);
       b_c=(unichar)((unsigned char)*(b_p+2));
	   if (a_c=='\0')
		   return -b_c;
	   if (a_c-b_c!=0)
		   return (a_c-b_c);

       a_c=(unichar)*(a_p+3);
       b_c=(unichar)((unsigned char)*(b_p+3));
       a_p+=4;
	   b_p+=4;
	   if (a_c=='\0')
		   return -b_c;
	   if (a_c-b_c!=0)
		   return (a_c-b_c);
    } ;
} else {
  if (a==NULL) {
       if (b==NULL) return 0;
       return 1;
    }
  return -1;
  }
}


/**
 * Returns 1 if a is the same as b; 0 otherwise.
 */
int u_equal(const unichar* a, const unichar* b) {
return !u_strcmp(a,b);
}

/**
 * Unicode version of strdup.
 * This function returns an allocated string that is a copy of the given one.
 *
 * Author: Olivier Blanc
 */
unichar* u_strdup(const unichar* str) {
if (str==NULL) return NULL;
size_t buflen=(u_strlen(str)+1)*sizeof(unichar);
unichar* res=(unichar*)malloc(buflen);
if (res==NULL) {
   fatal_alloc_error("u_strdup");
}
return (unichar*)memcpy(res,str,buflen);
}


unichar* u_strdup(const unichar* str,Abstract_allocator prv_alloc) {
if (str==NULL) return NULL;
size_t buflen=(u_strlen(str)+1)*sizeof(unichar);
unichar* res=(unichar*)malloc_cb(buflen,prv_alloc);
if (res==NULL) {
   fatal_alloc_error("u_strdup");
}
return (unichar*)memcpy(res,str,buflen);
}


/**
 * This version has the correct prototype to be used as a keycopy function for
 * hash tables.
 */
unichar* keycopy(unichar* key) {
return u_strdup(key);
}


/**
 * Unicode version of strndup.
 *   -- why this is then called u_strdup, and not u_strndup? (Sebastian, Munich)
 * This version returns an allocated string that is a copy of the
 * n first bytes of the given one.
 *
 * Author: Olivier Blanc
 */
unichar* u_strdup(const unichar* str,unsigned int n) {
if (str==NULL) return NULL;
unsigned int length=u_strlen(str);
if (length<n) {
   n=length;
}
unichar* res=(unichar*)malloc((n+1)*sizeof(unichar));
if (res==NULL) {
   fatal_alloc_error("u_strdup");
}
u_strncpy(res,str,n);
res[n]='\0';
return res;
}


unichar* u_strdup(const unichar* str,int n,Abstract_allocator prv_alloc) {
if (str==NULL) return NULL;
if (n<0) {
   fatal_error("Invalid length in u_strdup\n");
}
int length=u_strlen(str);
if (length<n) {
   n=length;
}
unichar* res=(unichar*)malloc_cb((n+1)*sizeof(unichar),prv_alloc);
if (res==NULL) {
   fatal_alloc_error("u_strdup");
}
u_strncpy(res,str,n);
res[n]='\0';
return res;
}


/**
 * Unicode version of strdup.
 * This function returns an allocated string that is a copy of the given one.
 *
 * Author: Olivier Blanc
 */
unichar* u_strdup(const char* str) {
if (str==NULL) return NULL;
unichar* res=(unichar*)malloc((strlen(str)+1)*sizeof(unichar));
if (res==NULL) {
   fatal_alloc_error("u_strdup");
}
return u_strcpy(res,str);
}


unichar* u_strdup(const char* str,Abstract_allocator prv_alloc) {
if (str==NULL) return NULL;
unichar* res=(unichar*)malloc_cb((strlen(str)+1)*sizeof(unichar),prv_alloc);
if (res==NULL) {
   fatal_alloc_error("u_strdup");
}
return u_strcpy(res,str);
}


/**
 * Unicode version of strchr.
 * This function returns a pointer on the first occurrence of 'c' in 's', or
 * NULL if not found. If 'unprotected' is not null, the function looks for the
 * first unprotected occurrence of 'c'; otherwise, it looks for the first
 * occurrence, protected by a backslash or not.
 *
 * Author: Olivier Blanc
 * Modified by Sébastien Paumier
 */
unichar* u_strchr(const unichar* s,unichar c,int unprotected) {
if (s==NULL) return NULL;
while (*s) {
   if (*s=='\\' && unprotected) {
      /* If we are looking for an unprotected character, we skip any
       * protected character */
      s++;
      if (*s=='\0') return NULL;
   }
   if (*s==c) {
      return (unichar*)s;
   }
   s++;
}
return NULL;
}


/**
 * Unicode version of strchr.
 * This function returns a pointer on the first occurrence of 'c' in 's', or
 * NULL if not found.
 *
 * Author: Olivier Blanc
 */
unichar* u_strchr(const unichar* s,unichar c) {
return u_strchr(s,c,0);
}


/**
 * A version of strchr that looks for a unicode character in a normal string.
 * We do this instead of calling the original strchr with a cast of 'c' to 'char',
 * because such a cast would cause invalid matches.
 *
 * Author: Olivier Blanc
 * Modified by Sébastien Paumier
 */
const char* u_strchr(const char* s,unichar c) {
if (s==NULL) return NULL;
while (*s) {
   if (c==(unichar)*s) return s;
   s++;
}
return NULL;
}


/**
 * Unicode version of strpbrk.
 * This function returns a pointer on the first occurrence of any delimiter 's', or
 * NULL if not found.
 *
 * Author: Olivier Blanc
 */
unichar* u_strpbrk(const unichar* s,unichar* delimiters) {
if (s==NULL) return NULL;
while (*s) {
   if (u_strchr(delimiters,*s)) return (unichar*)s;
   s++;
}
return NULL;
}


/**
 * Unicode version of strpbrk.
 * This function returns a pointer on the first occurrence of any delimiter 's', or
 * NULL if not found.
 *
 * Author: Olivier Blanc
 */
unichar* u_strpbrk(const unichar* s,char* delimiters) {
if (s==NULL) return NULL;
while (*s) {
   if (u_strchr(delimiters,*s)) return (unichar*)s;
   s++;
}
return NULL;
}


/**
 * Returns 1 if s starts with the given prefix; 0 otherwise.
 */
int u_starts_with(const unichar* s,const unichar* prefix) {
if (s==NULL) return 0;
if (prefix==NULL) return 1;
for (int i=0;prefix[i]!='\0';i++) {
   if (s[i]!=prefix[i]) {
      return 0;
   }
}
return 1;
}


/**
 * Returns 1 if s starts with the given prefix; 0 otherwise.
 */
int u_starts_with(const unichar* s,const char* prefix) {
if (s==NULL) return 0;
if (prefix==NULL) return 1;
for (int i=0;prefix[i]!='\0';i++) {
   if (s[i]!=prefix[i]) {
      return 0;
   }
}
return 1;
}

/**
 * Returns 1 if s ends with the given suffix; 0 otherwise.
 */
int u_ends_with(const unichar* s,const unichar* suffix) {
if (s==NULL) return 0;
if (suffix==NULL) return 1;
int l1=u_strlen(s);
int l2=u_strlen(suffix);
if (l2>l1) return 0;
return !u_strcmp(s+(l1-l2),suffix);
}


/**
 * Returns 1 if s ends with the given suffix; 0 otherwise.
 */
int u_ends_with(const unichar* s,const char* suffix) {
if (s==NULL) return 0;
if (suffix==NULL) return 1;
int l1=u_strlen(s);
int l2=(int)strlen(suffix);
if (l2>l1) return 0;
return !u_strcmp(s+(l1-l2),suffix);
}


/**
 * Converts the unichar* src into a char* dest.
 * dest is encoded in latin-1 (iso-8859-1) and non-convertible characters are skipped.
 *
 * Author: Sébastien Paumier
 * Modified by S?bastian Nagel
 */
void u_to_char(char *dest,unichar *src) {
register unichar c;
do {
   c=*src++;
   if (c<=0xFF) *dest++ = (char)c;
} while (c!='\0');
}


/**
 * Removes the \n at the end of the string, if any.
 */
void u_chomp_new_line(unichar* s) {
int l=u_strlen(s);
if (l>0 && s[l-1]=='\n') {
	s[l-1]='\0';
}
}


/**
 * Puts a copy of 'src' into 'dst', replacing:
 * - ASCII alphanumerics by themselves
 * - other by %YY where YY is the hexadecimal representation of the unicode character value
 *
 * Note that 'dst' is supposed to be large enough.
 * The function returns the length of 'dst'.
 */
int URLize(unichar* src,unichar* dst) {
if (src==NULL) {
   fatal_error("NULL error in URLize\n");
}
int pos=0;
for (int i=0;src[i]!='\0';i++) {
	if (u_is_ASCII_alphanumeric(src[i])) {
		dst[pos++]=src[i];
	} else {
		int n=u_sprintf(&(dst[pos]),"%%%X",src[i]);
		pos=pos+n;
	}
}
dst[pos]='\0';
return pos;
}


/**
 * Puts a copy of 'src' into 'dst', replacing:
 * - multi-spaces by non-breakable spaces. Spaces at the beginning of the string
 *   are also converted, even if there is only one
 * - '< ' by "&lt;"
 * - '> ' by "&gt;"
 * - '& ' by "&amp;"
 *
 * Note that 'dst' is supposed to be large enough.
 * The function returns the length of 'dst'.
 */
int htmlize(unichar* src,unichar* dst) {
if (src==NULL) {
   fatal_error("NULL error in htmlize\n");
}
int i=0;
int pos=0;
while (src[i]!='\0') {
   if (src[i]==' ') {
      /* If we find a space, we look if we are at the beginning of the line,
       * or at the end, or on the first character of a space sequence. */
      if (i==0 || src[i+1]=='\0' || src[i+1]==' ') {
         /* Here, we have to print "&nbsp;" as many times as needed */
         while (src[i]==' ') {
            u_strcpy(&dst[pos],"&nbsp;");
            pos=pos+6;
            i++;
         }
      } else {
         /* Otherwise, we put a single normal space */
         dst[pos++]=' ';
         i++;
      }
   } else if (src[i]=='<') {
      u_strcpy(&dst[pos],"&lt;");
      pos=pos+4;
      i++;
   } else if (src[i]=='>') {
      u_strcpy(&dst[pos],"&gt;");
      pos=pos+4;
      i++;
   } else if (src[i]=='&') {
      u_strcpy(&dst[pos],"&amp;");
      pos=pos+5;
      i++;
   } else {
      dst[pos++]=src[i++];
   }
}
dst[pos]='\0';
return pos;
}


/**
 * Copies the mirror of 'src' into 'dst'. Returns the length of the strings.
 */
int mirror(unichar* src,unichar* dst) {
if (src==NULL) {
   fatal_error("NULL error in mirror\n");
}
int l=u_strlen(src)-1;
int pos=0;
while (l>=0) {
   dst[pos++]=src[l--];
}
dst[pos]='\0';
return pos;
}


/**
 * Replaces 's' by its mirror and returns the length of the string,
 * or -1 if the string is NULL.
 */
int mirror(unichar* s) {
if (s==NULL) return -1;
int length=u_strlen(s);
int a=0;
int b=length-1;
unichar tmp;
while (a<b) {
   tmp=s[a];
   s[a]=s[b];
   s[b]=tmp;
   a++;
   b--;
}
return length;
}


/**
 * Returns the length of the longuest prefix common to the strings 'a' and 'b'.
 */
int get_longuest_prefix(unichar* a,unichar* b) {
if (a==NULL || b==NULL) {
   return 0;
}
int i=0;
while (a[i]==b[i] && a[i]!='\0') i++;
return i;
}


/**
 * This function returns a hash code for a unicode string.
 */
unsigned int hash_unichar(unichar* s) {
if (s==NULL) {
   return 0;
}
unsigned int code=0;
int i=0;
while (s[i]!='\0') {
   code=code*31+s[i];
   i++;
}
return code;
}



//
// unicode version of isdigit
//
int u_is_digit(unichar c) {
return (c>='0' && c<='9');
}


//
// returns true if c is a basic latin letter
//
int u_is_basic_latin_letter(unichar c) {
return ((c>='a' && c<='z') || (c>='A' && c<='Z'));
}


int u_is_ASCII_alphanumeric(unichar c) {
return u_is_digit(c) || u_is_basic_latin_letter(c);
}

//
// returns true if c is a latin-1 supplement letter
//
int u_is_latin1_supplement_letter(unichar c) {
return (c>=0xC0 && c<=0xFF && c!=0xD7 && c!=0xF7);
}


//
// returns true if c is a latin extended-A letter
//
int u_is_latin_extendedA_letter(unichar c) {
return (c>=0x0100 && c<=0x017F);
}


//
// returns true if c is a latin extended-B letter
//
int u_is_latin_extendedB_letter(unichar c) {
return (c>=0x0180 && c<=0x0233 && c!=0x0220 && c!=0x221);
}


//
// returns true if c is in the IPA extensions letters
//
int u_is_IPA_extensions_letter(unichar c) {
return (c>=0x0250 && c<=0x02AD);
}


//
// returns true if c is a greek letter
//
int u_is_greek_letter(unichar c) {
return (c>=0x0386 && c<=0x03F5 && c!=0x0387 && c!=0x038B
        && c!=0x038D && c!=0x03A2 && c!=0x03CF && c!=0x03D8 && c!=0x03D9);
}

//------Beginning of Hyungue's inserts--------------

//
//	return true if c is a korean syllalbe
//
int u_is_Hangul(unichar c)
{
	return( (c >= 0xac00) && (c<= 0xd7a3));
}
//
//	return true if c is a korean ideograme
//

int u_is_CJK_Unified_Ideograph(unichar c)
{
	return( (c>= 0x4e00) && (c <= 0x9fff));
}
int u_is_CJK_compatibility_ideograph(unichar c)
{
	return( (c>= 0xf900) && (c <= 0xfaff));
}
//
//	return true if c is a character of the alphabet coreen
//	when characters of this zone exit in the korean text
//	these is symbols
//
int u_is_Hangul_Compatility_Jamo(unichar c)
{
	return( (c>= 0x3130) && (c <= 0x3163));
}
//
//	return true
//	these charcters of this zone can not existe in the korean text
//
int u_is_Hangul_Jamo(unichar c)
{
	return( (c>= 0x1100) && (c <= 0x11FF));
}

int u_is_Hangul_Jamo_initial_consonant(unichar c)
{
   return( (c>= 0x1100) && (c <= 0x1159));
}

int u_is_Hangul_Jamo_final_consonant(unichar c)
{
   return( (c>= 0x11A8) && (c <= 0x11F9));
}

int u_is_Hangul_Jamo_consonant(unichar c)
{
   return u_is_Hangul_Jamo_initial_consonant(c) || u_is_Hangul_Jamo_final_consonant(c);
}

int u_is_Hangul_Jamo_medial_vowel(unichar c)
{
   return( (c>= 0x1160) && (c <= 0x11A2));
}


//------End of Hyungue's inserts--------------

//
// returns true if c is a cyrillic letter
//
int u_is_cyrillic_letter(unichar c) {
return (c>=0x0400 && c<=0x4F9 && (c<0x0482 || c>0x048B) && c!=0x04C5 && c!=0x04C6
        && c!=0x04C9 && c!=0x04CA && c!=0x04CD && c!=0x04CE && c!=0x04CF && c!=0x04F6
        && c!=0x04F7);
}


//
// returns true if c is an armenian letter
//
int u_is_armenian_letter(unichar c) {
return (c>=0x0531 && c<=0x587 && (c<0x0557 || c>0x0560));
}


//
// returns true if c is an hebrew letter
//
int u_is_hebrew_letter(unichar c) {
return (c>=0x05D0 && c<=0x05EA) || (c==0x05F0 || c==0x05F1 || c==0x05F2);
}


//
// returns true if c is an arabic letter
//
int u_is_arabic_letter(unichar c) {
return ((c>=0x0621 && c<=0x063A) || (c>=0x0641 && c<=0x64A)
        || (c>=0x0671 && c<=0x06D3) || c==0x06D5
        || (c>=0x06FA && c<=0x06FC));
}


//
// returns true if c is a syriac letter
//
int u_is_syriac_letter(unichar c) {
return (c>=0x0710 && c<=0x072C);
}


//
// returns true if c is a thaana letter
//
int u_is_thaana_letter(unichar c) {
return (c>=0x0780 && c<=0x07A5);
}


//
// returns true if c is a devanagari letter
//
int u_is_devanagari_letter(unichar c) {
return ((c>=0x0905 && c<=0x0939) || (c>=0x093C && c<=0x094D)
        || (c>=0x0950 && c<=0x0954) || (c>=0x0958 && c<=0x0970));
}


//
// returns true if c is a bengali letter
//
int u_is_bengali_letter(unichar c) {
return (c>=0x0985 && c<=0x09B9 && c!=0x098D && c!=0x098E
        && c!=0x0991 && c!=0x0992 && c!=0x09B1 && c!=0x09B3
        && c!=0x09B4 && c!=0x09B5) ||
       (c>=0x09BE && c<=0x09CC && c!=0x09C5 && c!=0x09C6
        && c!=0x09C9 && c!=0x09CA) ||
       (c>=0x09DC && c<=0x09E3 && c!=0x09DE) ||
       (c==0x09F0 || c==0x09F1);
}


//
// returns true if c is a gurmukhi letter
//
int u_is_gurmukhi_letter(unichar c) {
return (c>=0x0A05 && c<=0x0A0A) ||
       (c==0x0A0F || c==0x0A10) ||
       (c>=0x0A13 && c<=0x0A39 && c!=0x0A29 && c!=0x0A31
        && c!=0x0A34 && c!=0x0A37) ||
       (c>=0x0A3E && c<=0x0A42) ||
       (c==0x0A47 || c==0x0A48) ||
       (c>=0x0A4B && c<=0x0A4D) ||
       (c>=0x0A59 && c<=0x0A5E && c!=0x0A5D) ||
       (c>=0x0A70 && c<=0x0A74);
}


//
// returns true if c is a gujarati letter
//
int u_is_gujarati_letter(unichar c) {
return (c>=0x0A85 && c<=0x0ACC && c!=0x0A8C && c!=0x0A8E
        && c!=0x0A92 && c!=0x0AA9 && c!=0x0AB1 && c!=0x0AB4
        && c!=0x0ABA && c!=0x0ABB && c!=0x0AC6 && c!=0x0ACA);
}


//
// returns true if c is an oriya letter
//
int u_is_oriya_letter(unichar c) {
return (c>=0x0B05 && c<=0x0B39 && c!=0x0B0D && c!=0x0B0E
        && c!=0x0B11 && c!=0x0B12 && c!=0x0B29 && c!=0x0B31
        && c!=0x0B34 && c!=0x0B35) ||
       (c>=0x0B3E && c<=0x0B43) ||
       (c==0x0B47 || c==0x0B48 || c==0x0B4B || c==0x0B4C) ||
       (c>=0x0B5C && c<=0x0B61 && c!=0x0B5E);
}


//
// returns true if c is a tamil letter
//
int u_is_tamil_letter(unichar c) {
return (c>=0x0B85 && c<=0x0BCC && c!=0x0B8B && c!=0x0B8C
        && c!=0x0B8D && c!=0x0B91 && c!=0x0B96 && c!=0x0B97
        && c!=0x0B98 && c!=0x0B9B && c!=0x0B9D && c!=0x0BA0
        && c!=0x0BA1 && c!=0x0BA2 && c!=0x0BA5 && c!=0x0BA6
        && c!=0x0BA7 && c!=0x0BAB && c!=0x0BAC && c!=0x0BAD
        && c!=0x0BB6 && c!=0x0BBA && c!=0x0BBB && c!=0x0BBC
        && c!=0x0BBD && c!=0x0BC3 && c!=0x0BC4 && c!=0x0BC5
        && c!=0x0BC9);
}


//
// returns true if c is a telugu letter
//
int u_is_telugu_letter(unichar c) {
return (c>=0x0C05 && c<=0x0C4C && c!=0x0C0D && c!=0x0C11
        && c!=0x0C29 && c!=0x0C34 && c!=0x0C3A && c!=0x0C3B
        && c!=0x0C3C && c!=0x0C3D && c!=0x0C45 && c!=0x0C49);
}


//
// returns true if c is a kannada letter
//
int u_is_kannada_letter(unichar c) {
return (c>=0x0C85 && c<=0x0CCC && c!=0x0C8D && c!=0x0C91
        && c!=0x0CA9 && c!=0x0CB4 && c!=0x0CBA && c!=0x0CBB
        && c!=0x0CBC && c!=0x0CBD && c!=0x0CC5 && c!=0x0CC9) ||
       (c==0x0CDE || c==0x0CE0 || c==0x0CE1);
}


//
// returns true if c is a malayalam letter
//
int u_is_malayalam_letter(unichar c) {
return (c>=0x0D05 && c<=0x0D4C && c!=0x0D0D && c!=0x0D11
        && c!=0x0D29 && c!=0x0D3A && c!=0x0D3B && c!=0x0D44
        && c!=0x0D3C && c!=0x0D3D && c!=0x0D45 && c!=0x0D49)||
       (c==0x0D60 || c==0x0D61);
}


//
// returns true if c is a sinhala letter
//
int u_is_sinhala_letter(unichar c) {
return (c>=0x0D85 && c<=0x0DC6 && c!=0x0D97 && c!=0x0D98
        && c!=0x0D99 && c!=0x0DB2 && c!=0x0DBC && c!=0x0DBE
        && c!=0x0DBF && c!=0x0D) ||
       (c>=0x0DCF && c<=0x0DDF && c!=0x0DD5 && c!=0x0DD7) ||
       (c==0x0DF2 || c==0x0DF3);
}


// returns true if c is a thai letter
//
int u_is_thai_letter(unichar c) {
return (c>=0x0E01 && c<=0x0E39 && c!=0x0E3F) ||
       (c>=0x0E40 && c<=0x0E4B);
}

// returns true if c is a greek extended letter
//
int u_is_greek_extended_letter(unichar c) {							//$CD:20021115
return (c>=0x1F00 && c<=0x1F15) || (c>=0x1F18 && c<=0x1F1D) ||		//$CD:20021115
       (c>=0x1F20 && c<=0x1F45) || (c>=0x1F48 && c<=0x1F4D) ||		//$CD:20021115
       (c>=0x1F50 && c<=0x1F57) || 									//$CD:20021115
        c==0x1F59 || c==0x1F5B || c==0x1F5D || 						//$CD:20021115
       (c>=0x1F5F && c<=0x1F7D) || (c>=0x1F80 && c<=0x1FB4) ||		//$CD:20021115
       (c>=0x1FB6 && c<=0x1FBC) || (c>=0x1FC2 && c<=0x1FC4) ||		//$CD:20021115
       (c>=0x1FC6 && c<=0x1FCC) || (c>=0x1FD0 && c<=0x1FD3) ||		//$CD:20021115
       (c>=0x1FD6 && c<=0x1FDB) || (c>=0x1FE0 && c<=0x1FEC) ||		//$CD:20021115
       (c>=0x1FF2 && c<=0x1FF4) || (c>=0x1FF6 && c<=0x1FFC);		//$CD:20021115
}																	//$CD:20021115


//
// returns true if c is a letter in a naive way
//
int u_is_letter_internal(unichar c) {
return u_is_basic_latin_letter(c)
       || u_is_latin1_supplement_letter(c)
       || u_is_latin_extendedA_letter(c)
       || u_is_latin_extendedB_letter(c)
       || u_is_IPA_extensions_letter(c)
       || u_is_greek_letter(c)
       || u_is_cyrillic_letter(c)
       || u_is_armenian_letter(c)
       || u_is_hebrew_letter(c)
       || u_is_arabic_letter(c)
       || u_is_thaana_letter(c)
       || u_is_devanagari_letter(c)
       || u_is_bengali_letter(c)
       || u_is_gurmukhi_letter(c)
       || u_is_gujarati_letter(c)
       || u_is_oriya_letter(c)
       || u_is_tamil_letter(c)
       || u_is_telugu_letter(c)
       || u_is_kannada_letter(c)
       || u_is_malayalam_letter(c)
       || u_is_sinhala_letter(c)
       || u_is_thai_letter(c)
       || u_is_greek_extended_letter(c)	//$CD:20021115
//---------Beginning of Hyungue's inserts--------
       || u_is_Hangul(c)
	   || u_is_CJK_Unified_Ideograph(c)
	   || u_is_CJK_compatibility_ideograph(c)
//---------End of Hyungue's inserts--------
       ;
}


/**
 * Initializes the array : bit i = 1 if i is a letter, 0 otherwise.
 */
char init_unicode_table() {
int i;
for (i=0;i<8192;i++) tab_is_letter[i]=0;
for (i=0;i<=0xFFFF;i++) {
   if (u_is_letter_internal((unichar)i)) {
      tab_is_letter[i/8]=(char)(tab_is_letter[i/8]|(1<<(i%8)));
   }
}
return 1;
}


// this line is used to initialize automatically the unicode table
char foo=(char)(init_unicode_table()/*+make_CR()*/);


/**
 * Returns a non zero value if 'c' is a letter looking up at the unicode table;
 * 0 otherwise.
 */
int u_is_letter(unichar c) {
return (tab_is_letter[c/8]&(1<<(c%8)));
}


/**
 * This function returns 1 if the given string is only made of letters.
 */
int u_is_word(const unichar* s) {
if (s==NULL) {
   fatal_error("NULL error in is_word\n");
}
for (int i=0;s[i]!='\0';i++) {
   if (!u_is_letter(s[i])) return 0;
}
return 1;
}


/**
 * This function returns 1 if the given string is only made of digits.
 */
int u_are_digits(const unichar* s) {
if (s==NULL) {
   fatal_error("NULL error in u_are_digits\n");
}
for (int i=0;s[i]!='\0';i++) {
   if (!u_is_digit(s[i])) return 0;
}
return 1;
}


/**
 * Reads an integer from the string 'str'. If 'next' is not NULL,
 * it will contains a pointer to the first character that follows the
 * integer.
 *
 * Author: Olivier Blanc
 */
int u_parse_int(unichar* str,unichar* *next) {
int res=0;
while (u_is_digit(*str)) {
   res=res*10+(*str-'0');
   str++;
}
if (next) {
   *next=str;
}
return res;
}





/* S.N.
   functions for unicode case conversion

u_tolower
u_toupper

Convert a unichar to uppercase/lowercase unichar using
the unicode case folding table:
  http://www.unicode.org/Public/UNIDATA/CaseFolding.txt

Case folding is done only for "common" and "simple" case
folding, i.e. only single characters to single characters.
Foldings like ? -> A' (A+accute) or (I -> i, I -> ?)
are excluded, see
  http://www.unicode.org/Public/UNIDATA/SpecialCasing.txt
for many examples from various languages.

To use the alphabet file is not possible, because of
multiple mappings (A -> a or ? in French), see commentary
on "turn_portuguese_sequence_to_lowercase" in Alphabet.cpp

The case folding tables are implemented by a switch statement which
will be (hopefully) optimized by gcc to a table lookup or (even not
bad) a b-tree.
Of course, it could also be hardcoded as a sparse array with first and
second 8 bits of unichar as dimensions. Because scripts are allocated
in unicode blocks this will be memory efficient.

Switch statements are autogenerated from
  http://www.unicode.org/Public/UNIDATA/CaseFolding.txt
using this small perl script:

% cat CaseFolding.txt \
  | perl -ne 'do { next LINE if ($1 > 0xffff || $2 > 0xffff);
                   print "    case 0x", lc($1), ": r=0x", lc($2),
                     "; break; // ", $3, "\n" }
                if /([0-9A-F]+);\s*[CS];\s*([0-9A-F]+);\s*#\s*(.+)/' \
  > tolower_tab

resp.

% cat CaseFolding.txt \
  | perl -ne 'do { next LINE if (hex($1) > 0xffff || hex($2) > 0xffff); next if $h{$2}++;
                   print "    case 0x", lc($2), ": r=0x", lc($1),
                      "; break; // ", $3, "\n" }
                if /([0-9A-F]+);\s*[CS];\s*([0-9A-F]+);\s*#\s*(.+)/' \
  > toupper_tab

For the uppercase table duplicate values in the switch statement are skipped.

Since Unicode code points > 0xffff can't be used with Unitex,
they are skipped from the table.
*************************************************************/

unichar u_toupper (unichar c) {
  unichar r;
  switch (c) {
      /* begin of autogenerated code */
    case 0x0061: r=0x0041; break; // LATIN CAPITAL LETTER A
    case 0x0062: r=0x0042; break; // LATIN CAPITAL LETTER B
    case 0x0063: r=0x0043; break; // LATIN CAPITAL LETTER C
    case 0x0064: r=0x0044; break; // LATIN CAPITAL LETTER D
    case 0x0065: r=0x0045; break; // LATIN CAPITAL LETTER E
    case 0x0066: r=0x0046; break; // LATIN CAPITAL LETTER F
    case 0x0067: r=0x0047; break; // LATIN CAPITAL LETTER G
    case 0x0068: r=0x0048; break; // LATIN CAPITAL LETTER H
    case 0x0069: r=0x0049; break; // LATIN CAPITAL LETTER I
    case 0x006a: r=0x004a; break; // LATIN CAPITAL LETTER J
    case 0x006b: r=0x004b; break; // LATIN CAPITAL LETTER K
    case 0x006c: r=0x004c; break; // LATIN CAPITAL LETTER L
    case 0x006d: r=0x004d; break; // LATIN CAPITAL LETTER M
    case 0x006e: r=0x004e; break; // LATIN CAPITAL LETTER N
    case 0x006f: r=0x004f; break; // LATIN CAPITAL LETTER O
    case 0x0070: r=0x0050; break; // LATIN CAPITAL LETTER P
    case 0x0071: r=0x0051; break; // LATIN CAPITAL LETTER Q
    case 0x0072: r=0x0052; break; // LATIN CAPITAL LETTER R
    case 0x0073: r=0x0053; break; // LATIN CAPITAL LETTER S
    case 0x0074: r=0x0054; break; // LATIN CAPITAL LETTER T
    case 0x0075: r=0x0055; break; // LATIN CAPITAL LETTER U
    case 0x0076: r=0x0056; break; // LATIN CAPITAL LETTER V
    case 0x0077: r=0x0057; break; // LATIN CAPITAL LETTER W
    case 0x0078: r=0x0058; break; // LATIN CAPITAL LETTER X
    case 0x0079: r=0x0059; break; // LATIN CAPITAL LETTER Y
    case 0x007a: r=0x005a; break; // LATIN CAPITAL LETTER Z
    case 0x03bc: r=0x00b5; break; // MICRO SIGN
    case 0x00e0: r=0x00c0; break; // LATIN CAPITAL LETTER A WITH GRAVE
    case 0x00e1: r=0x00c1; break; // LATIN CAPITAL LETTER A WITH ACUTE
    case 0x00e2: r=0x00c2; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
    case 0x00e3: r=0x00c3; break; // LATIN CAPITAL LETTER A WITH TILDE
    case 0x00e4: r=0x00c4; break; // LATIN CAPITAL LETTER A WITH DIAERESIS
    case 0x00e5: r=0x00c5; break; // LATIN CAPITAL LETTER A WITH RING ABOVE
    case 0x00e6: r=0x00c6; break; // LATIN CAPITAL LETTER AE
    case 0x00e7: r=0x00c7; break; // LATIN CAPITAL LETTER C WITH CEDILLA
    case 0x00e8: r=0x00c8; break; // LATIN CAPITAL LETTER E WITH GRAVE
    case 0x00e9: r=0x00c9; break; // LATIN CAPITAL LETTER E WITH ACUTE
    case 0x00ea: r=0x00ca; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX
    case 0x00eb: r=0x00cb; break; // LATIN CAPITAL LETTER E WITH DIAERESIS
    case 0x00ec: r=0x00cc; break; // LATIN CAPITAL LETTER I WITH GRAVE
    case 0x00ed: r=0x00cd; break; // LATIN CAPITAL LETTER I WITH ACUTE
    case 0x00ee: r=0x00ce; break; // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
    case 0x00ef: r=0x00cf; break; // LATIN CAPITAL LETTER I WITH DIAERESIS
    case 0x00f0: r=0x00d0; break; // LATIN CAPITAL LETTER ETH
    case 0x00f1: r=0x00d1; break; // LATIN CAPITAL LETTER N WITH TILDE
    case 0x00f2: r=0x00d2; break; // LATIN CAPITAL LETTER O WITH GRAVE
    case 0x00f3: r=0x00d3; break; // LATIN CAPITAL LETTER O WITH ACUTE
    case 0x00f4: r=0x00d4; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
    case 0x00f5: r=0x00d5; break; // LATIN CAPITAL LETTER O WITH TILDE
    case 0x00f6: r=0x00d6; break; // LATIN CAPITAL LETTER O WITH DIAERESIS
    case 0x00f8: r=0x00d8; break; // LATIN CAPITAL LETTER O WITH STROKE
    case 0x00f9: r=0x00d9; break; // LATIN CAPITAL LETTER U WITH GRAVE
    case 0x00fa: r=0x00da; break; // LATIN CAPITAL LETTER U WITH ACUTE
    case 0x00fb: r=0x00db; break; // LATIN CAPITAL LETTER U WITH CIRCUMFLEX
    case 0x00fc: r=0x00dc; break; // LATIN CAPITAL LETTER U WITH DIAERESIS
    case 0x00fd: r=0x00dd; break; // LATIN CAPITAL LETTER Y WITH ACUTE
    case 0x00fe: r=0x00de; break; // LATIN CAPITAL LETTER THORN
    case 0x0101: r=0x0100; break; // LATIN CAPITAL LETTER A WITH MACRON
    case 0x0103: r=0x0102; break; // LATIN CAPITAL LETTER A WITH BREVE
    case 0x0105: r=0x0104; break; // LATIN CAPITAL LETTER A WITH OGONEK
    case 0x0107: r=0x0106; break; // LATIN CAPITAL LETTER C WITH ACUTE
    case 0x0109: r=0x0108; break; // LATIN CAPITAL LETTER C WITH CIRCUMFLEX
    case 0x010b: r=0x010a; break; // LATIN CAPITAL LETTER C WITH DOT ABOVE
    case 0x010d: r=0x010c; break; // LATIN CAPITAL LETTER C WITH CARON
    case 0x010f: r=0x010e; break; // LATIN CAPITAL LETTER D WITH CARON
    case 0x0111: r=0x0110; break; // LATIN CAPITAL LETTER D WITH STROKE
    case 0x0113: r=0x0112; break; // LATIN CAPITAL LETTER E WITH MACRON
    case 0x0115: r=0x0114; break; // LATIN CAPITAL LETTER E WITH BREVE
    case 0x0117: r=0x0116; break; // LATIN CAPITAL LETTER E WITH DOT ABOVE
    case 0x0119: r=0x0118; break; // LATIN CAPITAL LETTER E WITH OGONEK
    case 0x011b: r=0x011a; break; // LATIN CAPITAL LETTER E WITH CARON
    case 0x011d: r=0x011c; break; // LATIN CAPITAL LETTER G WITH CIRCUMFLEX
    case 0x011f: r=0x011e; break; // LATIN CAPITAL LETTER G WITH BREVE
    case 0x0121: r=0x0120; break; // LATIN CAPITAL LETTER G WITH DOT ABOVE
    case 0x0123: r=0x0122; break; // LATIN CAPITAL LETTER G WITH CEDILLA
    case 0x0125: r=0x0124; break; // LATIN CAPITAL LETTER H WITH CIRCUMFLEX
    case 0x0127: r=0x0126; break; // LATIN CAPITAL LETTER H WITH STROKE
    case 0x0129: r=0x0128; break; // LATIN CAPITAL LETTER I WITH TILDE
    case 0x012b: r=0x012a; break; // LATIN CAPITAL LETTER I WITH MACRON
    case 0x012d: r=0x012c; break; // LATIN CAPITAL LETTER I WITH BREVE
    case 0x012f: r=0x012e; break; // LATIN CAPITAL LETTER I WITH OGONEK
    case 0x0133: r=0x0132; break; // LATIN CAPITAL LIGATURE IJ
    case 0x0135: r=0x0134; break; // LATIN CAPITAL LETTER J WITH CIRCUMFLEX
    case 0x0137: r=0x0136; break; // LATIN CAPITAL LETTER K WITH CEDILLA
    case 0x013a: r=0x0139; break; // LATIN CAPITAL LETTER L WITH ACUTE
    case 0x013c: r=0x013b; break; // LATIN CAPITAL LETTER L WITH CEDILLA
    case 0x013e: r=0x013d; break; // LATIN CAPITAL LETTER L WITH CARON
    case 0x0140: r=0x013f; break; // LATIN CAPITAL LETTER L WITH MIDDLE DOT
    case 0x0142: r=0x0141; break; // LATIN CAPITAL LETTER L WITH STROKE
    case 0x0144: r=0x0143; break; // LATIN CAPITAL LETTER N WITH ACUTE
    case 0x0146: r=0x0145; break; // LATIN CAPITAL LETTER N WITH CEDILLA
    case 0x0148: r=0x0147; break; // LATIN CAPITAL LETTER N WITH CARON
    case 0x014b: r=0x014a; break; // LATIN CAPITAL LETTER ENG
    case 0x014d: r=0x014c; break; // LATIN CAPITAL LETTER O WITH MACRON
    case 0x014f: r=0x014e; break; // LATIN CAPITAL LETTER O WITH BREVE
    case 0x0151: r=0x0150; break; // LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
    case 0x0153: r=0x0152; break; // LATIN CAPITAL LIGATURE OE
    case 0x0155: r=0x0154; break; // LATIN CAPITAL LETTER R WITH ACUTE
    case 0x0157: r=0x0156; break; // LATIN CAPITAL LETTER R WITH CEDILLA
    case 0x0159: r=0x0158; break; // LATIN CAPITAL LETTER R WITH CARON
    case 0x015b: r=0x015a; break; // LATIN CAPITAL LETTER S WITH ACUTE
    case 0x015d: r=0x015c; break; // LATIN CAPITAL LETTER S WITH CIRCUMFLEX
    case 0x015f: r=0x015e; break; // LATIN CAPITAL LETTER S WITH CEDILLA
    case 0x0161: r=0x0160; break; // LATIN CAPITAL LETTER S WITH CARON
    case 0x0163: r=0x0162; break; // LATIN CAPITAL LETTER T WITH CEDILLA
    case 0x0165: r=0x0164; break; // LATIN CAPITAL LETTER T WITH CARON
    case 0x0167: r=0x0166; break; // LATIN CAPITAL LETTER T WITH STROKE
    case 0x0169: r=0x0168; break; // LATIN CAPITAL LETTER U WITH TILDE
    case 0x016b: r=0x016a; break; // LATIN CAPITAL LETTER U WITH MACRON
    case 0x016d: r=0x016c; break; // LATIN CAPITAL LETTER U WITH BREVE
    case 0x016f: r=0x016e; break; // LATIN CAPITAL LETTER U WITH RING ABOVE
    case 0x0171: r=0x0170; break; // LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
    case 0x0173: r=0x0172; break; // LATIN CAPITAL LETTER U WITH OGONEK
    case 0x0175: r=0x0174; break; // LATIN CAPITAL LETTER W WITH CIRCUMFLEX
    case 0x0177: r=0x0176; break; // LATIN CAPITAL LETTER Y WITH CIRCUMFLEX
    case 0x00ff: r=0x0178; break; // LATIN CAPITAL LETTER Y WITH DIAERESIS
    case 0x017a: r=0x0179; break; // LATIN CAPITAL LETTER Z WITH ACUTE
    case 0x017c: r=0x017b; break; // LATIN CAPITAL LETTER Z WITH DOT ABOVE
    case 0x017e: r=0x017d; break; // LATIN CAPITAL LETTER Z WITH CARON
    case 0x0253: r=0x0181; break; // LATIN CAPITAL LETTER B WITH HOOK
    case 0x0183: r=0x0182; break; // LATIN CAPITAL LETTER B WITH TOPBAR
    case 0x0185: r=0x0184; break; // LATIN CAPITAL LETTER TONE SIX
    case 0x0254: r=0x0186; break; // LATIN CAPITAL LETTER OPEN O
    case 0x0188: r=0x0187; break; // LATIN CAPITAL LETTER C WITH HOOK
    case 0x0256: r=0x0189; break; // LATIN CAPITAL LETTER AFRICAN D
    case 0x0257: r=0x018a; break; // LATIN CAPITAL LETTER D WITH HOOK
    case 0x018c: r=0x018b; break; // LATIN CAPITAL LETTER D WITH TOPBAR
    case 0x01dd: r=0x018e; break; // LATIN CAPITAL LETTER REVERSED E
    case 0x0259: r=0x018f; break; // LATIN CAPITAL LETTER SCHWA
    case 0x025b: r=0x0190; break; // LATIN CAPITAL LETTER OPEN E
    case 0x0192: r=0x0191; break; // LATIN CAPITAL LETTER F WITH HOOK
    case 0x0260: r=0x0193; break; // LATIN CAPITAL LETTER G WITH HOOK
    case 0x0263: r=0x0194; break; // LATIN CAPITAL LETTER GAMMA
    case 0x0269: r=0x0196; break; // LATIN CAPITAL LETTER IOTA
    case 0x0268: r=0x0197; break; // LATIN CAPITAL LETTER I WITH STROKE
    case 0x0199: r=0x0198; break; // LATIN CAPITAL LETTER K WITH HOOK
    case 0x026f: r=0x019c; break; // LATIN CAPITAL LETTER TURNED M
    case 0x0272: r=0x019d; break; // LATIN CAPITAL LETTER N WITH LEFT HOOK
    case 0x0275: r=0x019f; break; // LATIN CAPITAL LETTER O WITH MIDDLE TILDE
    case 0x01a1: r=0x01a0; break; // LATIN CAPITAL LETTER O WITH HORN
    case 0x01a3: r=0x01a2; break; // LATIN CAPITAL LETTER OI
    case 0x01a5: r=0x01a4; break; // LATIN CAPITAL LETTER P WITH HOOK
    case 0x0280: r=0x01a6; break; // LATIN LETTER YR
    case 0x01a8: r=0x01a7; break; // LATIN CAPITAL LETTER TONE TWO
    case 0x0283: r=0x01a9; break; // LATIN CAPITAL LETTER ESH
    case 0x01ad: r=0x01ac; break; // LATIN CAPITAL LETTER T WITH HOOK
    case 0x0288: r=0x01ae; break; // LATIN CAPITAL LETTER T WITH RETROFLEX HOOK
    case 0x01b0: r=0x01af; break; // LATIN CAPITAL LETTER U WITH HORN
    case 0x028a: r=0x01b1; break; // LATIN CAPITAL LETTER UPSILON
    case 0x028b: r=0x01b2; break; // LATIN CAPITAL LETTER V WITH HOOK
    case 0x01b4: r=0x01b3; break; // LATIN CAPITAL LETTER Y WITH HOOK
    case 0x01b6: r=0x01b5; break; // LATIN CAPITAL LETTER Z WITH STROKE
    case 0x0292: r=0x01b7; break; // LATIN CAPITAL LETTER EZH
    case 0x01b9: r=0x01b8; break; // LATIN CAPITAL LETTER EZH REVERSED
    case 0x01bd: r=0x01bc; break; // LATIN CAPITAL LETTER TONE FIVE
    case 0x01c6: r=0x01c4; break; // LATIN CAPITAL LETTER DZ WITH CARON
    case 0x01c9: r=0x01c7; break; // LATIN CAPITAL LETTER LJ
    case 0x01cc: r=0x01ca; break; // LATIN CAPITAL LETTER NJ
    case 0x01ce: r=0x01cd; break; // LATIN CAPITAL LETTER A WITH CARON
    case 0x01d0: r=0x01cf; break; // LATIN CAPITAL LETTER I WITH CARON
    case 0x01d2: r=0x01d1; break; // LATIN CAPITAL LETTER O WITH CARON
    case 0x01d4: r=0x01d3; break; // LATIN CAPITAL LETTER U WITH CARON
    case 0x01d6: r=0x01d5; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND MACRON
    case 0x01d8: r=0x01d7; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND ACUTE
    case 0x01da: r=0x01d9; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND CARON
    case 0x01dc: r=0x01db; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND GRAVE
    case 0x01df: r=0x01de; break; // LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON
    case 0x01e1: r=0x01e0; break; // LATIN CAPITAL LETTER A WITH DOT ABOVE AND MACRON
    case 0x01e3: r=0x01e2; break; // LATIN CAPITAL LETTER AE WITH MACRON
    case 0x01eb: r=0x01ea; break; // LATIN CAPITAL LETTER O WITH OGONEK
    case 0x01ed: r=0x01ec; break; // LATIN CAPITAL LETTER O WITH OGONEK AND MACRON
    case 0x01ef: r=0x01ee; break; // LATIN CAPITAL LETTER EZH WITH CARON
    case 0x01f3: r=0x01f1; break; // LATIN CAPITAL LETTER DZ
    case 0x01f5: r=0x01f4; break; // LATIN CAPITAL LETTER G WITH ACUTE
    case 0x0195: r=0x01f6; break; // LATIN CAPITAL LETTER HWAIR
    case 0x01bf: r=0x01f7; break; // LATIN CAPITAL LETTER WYNN
    case 0x01f9: r=0x01f8; break; // LATIN CAPITAL LETTER N WITH GRAVE
    case 0x01fb: r=0x01fa; break; // LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE
    case 0x01fd: r=0x01fc; break; // LATIN CAPITAL LETTER AE WITH ACUTE
    case 0x01ff: r=0x01fe; break; // LATIN CAPITAL LETTER O WITH STROKE AND ACUTE
    case 0x0201: r=0x0200; break; // LATIN CAPITAL LETTER A WITH DOUBLE GRAVE
    case 0x0203: r=0x0202; break; // LATIN CAPITAL LETTER A WITH INVERTED BREVE
    case 0x0205: r=0x0204; break; // LATIN CAPITAL LETTER E WITH DOUBLE GRAVE
    case 0x0207: r=0x0206; break; // LATIN CAPITAL LETTER E WITH INVERTED BREVE
    case 0x0209: r=0x0208; break; // LATIN CAPITAL LETTER I WITH DOUBLE GRAVE
    case 0x020b: r=0x020a; break; // LATIN CAPITAL LETTER I WITH INVERTED BREVE
    case 0x020d: r=0x020c; break; // LATIN CAPITAL LETTER O WITH DOUBLE GRAVE
    case 0x020f: r=0x020e; break; // LATIN CAPITAL LETTER O WITH INVERTED BREVE
    case 0x0211: r=0x0210; break; // LATIN CAPITAL LETTER R WITH DOUBLE GRAVE
    case 0x0213: r=0x0212; break; // LATIN CAPITAL LETTER R WITH INVERTED BREVE
    case 0x0215: r=0x0214; break; // LATIN CAPITAL LETTER U WITH DOUBLE GRAVE
    case 0x0217: r=0x0216; break; // LATIN CAPITAL LETTER U WITH INVERTED BREVE
    case 0x0219: r=0x0218; break; // LATIN CAPITAL LETTER S WITH COMMA BELOW
    case 0x021b: r=0x021a; break; // LATIN CAPITAL LETTER T WITH COMMA BELOW
    case 0x021d: r=0x021c; break; // LATIN CAPITAL LETTER YOGH
    case 0x021f: r=0x021e; break; // LATIN CAPITAL LETTER H WITH CARON
    case 0x019e: r=0x0220; break; // LATIN CAPITAL LETTER N WITH LONG RIGHT LEG
    case 0x0223: r=0x0222; break; // LATIN CAPITAL LETTER OU
    case 0x0225: r=0x0224; break; // LATIN CAPITAL LETTER Z WITH HOOK
    case 0x0227: r=0x0226; break; // LATIN CAPITAL LETTER A WITH DOT ABOVE
    case 0x0229: r=0x0228; break; // LATIN CAPITAL LETTER E WITH CEDILLA
    case 0x022b: r=0x022a; break; // LATIN CAPITAL LETTER O WITH DIAERESIS AND MACRON
    case 0x022d: r=0x022c; break; // LATIN CAPITAL LETTER O WITH TILDE AND MACRON
    case 0x022f: r=0x022e; break; // LATIN CAPITAL LETTER O WITH DOT ABOVE
    case 0x0231: r=0x0230; break; // LATIN CAPITAL LETTER O WITH DOT ABOVE AND MACRON
    case 0x0233: r=0x0232; break; // LATIN CAPITAL LETTER Y WITH MACRON
    case 0x023c: r=0x023b; break; // LATIN CAPITAL LETTER C WITH STROKE
    case 0x019a: r=0x023d; break; // LATIN CAPITAL LETTER L WITH BAR
    case 0x0294: r=0x0241; break; // LATIN CAPITAL LETTER GLOTTAL STOP
    case 0x03b9: r=0x0345; break; // COMBINING GREEK YPOGEGRAMMENI
    case 0x03ac: r=0x0386; break; // GREEK CAPITAL LETTER ALPHA WITH TONOS
    case 0x03ad: r=0x0388; break; // GREEK CAPITAL LETTER EPSILON WITH TONOS
    case 0x03ae: r=0x0389; break; // GREEK CAPITAL LETTER ETA WITH TONOS
    case 0x03af: r=0x038a; break; // GREEK CAPITAL LETTER IOTA WITH TONOS
    case 0x03cc: r=0x038c; break; // GREEK CAPITAL LETTER OMICRON WITH TONOS
    case 0x03cd: r=0x038e; break; // GREEK CAPITAL LETTER UPSILON WITH TONOS
    case 0x03ce: r=0x038f; break; // GREEK CAPITAL LETTER OMEGA WITH TONOS
    case 0x03b1: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA
    case 0x03b2: r=0x0392; break; // GREEK CAPITAL LETTER BETA
    case 0x03b3: r=0x0393; break; // GREEK CAPITAL LETTER GAMMA
    case 0x03b4: r=0x0394; break; // GREEK CAPITAL LETTER DELTA
    case 0x03b5: r=0x0395; break; // GREEK CAPITAL LETTER EPSILON
    case 0x03b6: r=0x0396; break; // GREEK CAPITAL LETTER ZETA
    case 0x03b7: r=0x0397; break; // GREEK CAPITAL LETTER ETA
    case 0x03b8: r=0x0398; break; // GREEK CAPITAL LETTER THETA
    case 0x03ba: r=0x039a; break; // GREEK CAPITAL LETTER KAPPA
    case 0x03bb: r=0x039b; break; // GREEK CAPITAL LETTER LAMDA
    case 0x03bd: r=0x039d; break; // GREEK CAPITAL LETTER NU
    case 0x03be: r=0x039e; break; // GREEK CAPITAL LETTER XI
    case 0x03bf: r=0x039f; break; // GREEK CAPITAL LETTER OMICRON
    case 0x03c0: r=0x03a0; break; // GREEK CAPITAL LETTER PI
    case 0x03c1: r=0x03a1; break; // GREEK CAPITAL LETTER RHO
    case 0x03c3: r=0x03a3; break; // GREEK CAPITAL LETTER SIGMA
    case 0x03c4: r=0x03a4; break; // GREEK CAPITAL LETTER TAU
    case 0x03c5: r=0x03a5; break; // GREEK CAPITAL LETTER UPSILON
    case 0x03c6: r=0x03a6; break; // GREEK CAPITAL LETTER PHI
    case 0x03c7: r=0x03a7; break; // GREEK CAPITAL LETTER CHI
    case 0x03c8: r=0x03a8; break; // GREEK CAPITAL LETTER PSI
    case 0x03c9: r=0x03a9; break; // GREEK CAPITAL LETTER OMEGA
    case 0x03ca: r=0x03aa; break; // GREEK CAPITAL LETTER IOTA WITH DIALYTIKA
    case 0x03cb: r=0x03ab; break; // GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA
    case 0x03d9: r=0x03d8; break; // GREEK LETTER ARCHAIC KOPPA
    case 0x03db: r=0x03da; break; // GREEK LETTER STIGMA
    case 0x03dd: r=0x03dc; break; // GREEK LETTER DIGAMMA
    case 0x03df: r=0x03de; break; // GREEK LETTER KOPPA
    case 0x03e1: r=0x03e0; break; // GREEK LETTER SAMPI
    case 0x03e3: r=0x03e2; break; // COPTIC CAPITAL LETTER SHEI
    case 0x03eb: r=0x03ea; break; // COPTIC CAPITAL LETTER GANGIA
    case 0x03ed: r=0x03ec; break; // COPTIC CAPITAL LETTER SHIMA
    case 0x03ef: r=0x03ee; break; // COPTIC CAPITAL LETTER DEI
    case 0x03f8: r=0x03f7; break; // GREEK CAPITAL LETTER SHO
    case 0x03f2: r=0x03f9; break; // GREEK CAPITAL LUNATE SIGMA SYMBOL
    case 0x03fb: r=0x03fa; break; // GREEK CAPITAL LETTER SAN
    case 0x0450: r=0x0400; break; // CYRILLIC CAPITAL LETTER IE WITH GRAVE
    case 0x0451: r=0x0401; break; // CYRILLIC CAPITAL LETTER IO
    case 0x0452: r=0x0402; break; // CYRILLIC CAPITAL LETTER DJE
    case 0x0453: r=0x0403; break; // CYRILLIC CAPITAL LETTER GJE
    case 0x0454: r=0x0404; break; // CYRILLIC CAPITAL LETTER UKRAINIAN IE
    case 0x0455: r=0x0405; break; // CYRILLIC CAPITAL LETTER DZE
    case 0x0456: r=0x0406; break; // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
    case 0x0457: r=0x0407; break; // CYRILLIC CAPITAL LETTER YI
    case 0x0458: r=0x0408; break; // CYRILLIC CAPITAL LETTER JE
    case 0x0459: r=0x0409; break; // CYRILLIC CAPITAL LETTER LJE
    case 0x045a: r=0x040a; break; // CYRILLIC CAPITAL LETTER NJE
    case 0x045b: r=0x040b; break; // CYRILLIC CAPITAL LETTER TSHE
    case 0x045c: r=0x040c; break; // CYRILLIC CAPITAL LETTER KJE
    case 0x045d: r=0x040d; break; // CYRILLIC CAPITAL LETTER I WITH GRAVE
    case 0x045e: r=0x040e; break; // CYRILLIC CAPITAL LETTER SHORT U
    case 0x045f: r=0x040f; break; // CYRILLIC CAPITAL LETTER DZHE
    case 0x0430: r=0x0410; break; // CYRILLIC CAPITAL LETTER A
    case 0x0431: r=0x0411; break; // CYRILLIC CAPITAL LETTER BE
    case 0x0432: r=0x0412; break; // CYRILLIC CAPITAL LETTER VE
    case 0x0433: r=0x0413; break; // CYRILLIC CAPITAL LETTER GHE
    case 0x0434: r=0x0414; break; // CYRILLIC CAPITAL LETTER DE
    case 0x0435: r=0x0415; break; // CYRILLIC CAPITAL LETTER IE
    case 0x0436: r=0x0416; break; // CYRILLIC CAPITAL LETTER ZHE
    case 0x0437: r=0x0417; break; // CYRILLIC CAPITAL LETTER ZE
    case 0x0438: r=0x0418; break; // CYRILLIC CAPITAL LETTER I
    case 0x0439: r=0x0419; break; // CYRILLIC CAPITAL LETTER SHORT I
    case 0x043a: r=0x041a; break; // CYRILLIC CAPITAL LETTER KA
    case 0x043b: r=0x041b; break; // CYRILLIC CAPITAL LETTER EL
    case 0x043c: r=0x041c; break; // CYRILLIC CAPITAL LETTER EM
    case 0x043d: r=0x041d; break; // CYRILLIC CAPITAL LETTER EN
    case 0x043e: r=0x041e; break; // CYRILLIC CAPITAL LETTER O
    case 0x043f: r=0x041f; break; // CYRILLIC CAPITAL LETTER PE
    case 0x0440: r=0x0420; break; // CYRILLIC CAPITAL LETTER ER
    case 0x0441: r=0x0421; break; // CYRILLIC CAPITAL LETTER ES
    case 0x0442: r=0x0422; break; // CYRILLIC CAPITAL LETTER TE
    case 0x0443: r=0x0423; break; // CYRILLIC CAPITAL LETTER U
    case 0x0444: r=0x0424; break; // CYRILLIC CAPITAL LETTER EF
    case 0x0445: r=0x0425; break; // CYRILLIC CAPITAL LETTER HA
    case 0x0446: r=0x0426; break; // CYRILLIC CAPITAL LETTER TSE
    case 0x0447: r=0x0427; break; // CYRILLIC CAPITAL LETTER CHE
    case 0x0448: r=0x0428; break; // CYRILLIC CAPITAL LETTER SHA
    case 0x0449: r=0x0429; break; // CYRILLIC CAPITAL LETTER SHCHA
    case 0x044a: r=0x042a; break; // CYRILLIC CAPITAL LETTER HARD SIGN
    case 0x044b: r=0x042b; break; // CYRILLIC CAPITAL LETTER YERU
    case 0x044c: r=0x042c; break; // CYRILLIC CAPITAL LETTER SOFT SIGN
    case 0x044d: r=0x042d; break; // CYRILLIC CAPITAL LETTER E
    case 0x044e: r=0x042e; break; // CYRILLIC CAPITAL LETTER YU
    case 0x044f: r=0x042f; break; // CYRILLIC CAPITAL LETTER YA
    case 0x0461: r=0x0460; break; // CYRILLIC CAPITAL LETTER OMEGA
    case 0x0463: r=0x0462; break; // CYRILLIC CAPITAL LETTER YAT
    case 0x0465: r=0x0464; break; // CYRILLIC CAPITAL LETTER IOTIFIED E
    case 0x0467: r=0x0466; break; // CYRILLIC CAPITAL LETTER LITTLE YUS
    case 0x0469: r=0x0468; break; // CYRILLIC CAPITAL LETTER IOTIFIED LITTLE YUS
    case 0x046b: r=0x046a; break; // CYRILLIC CAPITAL LETTER BIG YUS
    case 0x046d: r=0x046c; break; // CYRILLIC CAPITAL LETTER IOTIFIED BIG YUS
    case 0x046f: r=0x046e; break; // CYRILLIC CAPITAL LETTER KSI
    case 0x0471: r=0x0470; break; // CYRILLIC CAPITAL LETTER PSI
    case 0x0473: r=0x0472; break; // CYRILLIC CAPITAL LETTER FITA
    case 0x0475: r=0x0474; break; // CYRILLIC CAPITAL LETTER IZHITSA
    case 0x0477: r=0x0476; break; // CYRILLIC CAPITAL LETTER IZHITSA WITH DOUBLE GRAVE ACCENT
    case 0x0479: r=0x0478; break; // CYRILLIC CAPITAL LETTER UK
    case 0x047b: r=0x047a; break; // CYRILLIC CAPITAL LETTER ROUND OMEGA
    case 0x047d: r=0x047c; break; // CYRILLIC CAPITAL LETTER OMEGA WITH TITLO
    case 0x047f: r=0x047e; break; // CYRILLIC CAPITAL LETTER OT
    case 0x0481: r=0x0480; break; // CYRILLIC CAPITAL LETTER KOPPA
    case 0x048b: r=0x048a; break; // CYRILLIC CAPITAL LETTER SHORT I WITH TAIL
    case 0x048d: r=0x048c; break; // CYRILLIC CAPITAL LETTER SEMISOFT SIGN
    case 0x048f: r=0x048e; break; // CYRILLIC CAPITAL LETTER ER WITH TICK
    case 0x0491: r=0x0490; break; // CYRILLIC CAPITAL LETTER GHE WITH UPTURN
    case 0x0493: r=0x0492; break; // CYRILLIC CAPITAL LETTER GHE WITH STROKE
    case 0x0495: r=0x0494; break; // CYRILLIC CAPITAL LETTER GHE WITH MIDDLE HOOK
    case 0x0497: r=0x0496; break; // CYRILLIC CAPITAL LETTER ZHE WITH DESCENDER
    case 0x0499: r=0x0498; break; // CYRILLIC CAPITAL LETTER ZE WITH DESCENDER
    case 0x049b: r=0x049a; break; // CYRILLIC CAPITAL LETTER KA WITH DESCENDER
    case 0x049d: r=0x049c; break; // CYRILLIC CAPITAL LETTER KA WITH VERTICAL STROKE
    case 0x049f: r=0x049e; break; // CYRILLIC CAPITAL LETTER KA WITH STROKE
    case 0x04a1: r=0x04a0; break; // CYRILLIC CAPITAL LETTER BASHKIR KA
    case 0x04a3: r=0x04a2; break; // CYRILLIC CAPITAL LETTER EN WITH DESCENDER
    case 0x04a5: r=0x04a4; break; // CYRILLIC CAPITAL LIGATURE EN GHE
    case 0x04a7: r=0x04a6; break; // CYRILLIC CAPITAL LETTER PE WITH MIDDLE HOOK
    case 0x04a9: r=0x04a8; break; // CYRILLIC CAPITAL LETTER ABKHASIAN HA
    case 0x04ab: r=0x04aa; break; // CYRILLIC CAPITAL LETTER ES WITH DESCENDER
    case 0x04ad: r=0x04ac; break; // CYRILLIC CAPITAL LETTER TE WITH DESCENDER
    case 0x04af: r=0x04ae; break; // CYRILLIC CAPITAL LETTER STRAIGHT U
    case 0x04b1: r=0x04b0; break; // CYRILLIC CAPITAL LETTER STRAIGHT U WITH STROKE
    case 0x04b3: r=0x04b2; break; // CYRILLIC CAPITAL LETTER HA WITH DESCENDER
    case 0x04b5: r=0x04b4; break; // CYRILLIC CAPITAL LIGATURE TE TSE
    case 0x04b7: r=0x04b6; break; // CYRILLIC CAPITAL LETTER CHE WITH DESCENDER
    case 0x04b9: r=0x04b8; break; // CYRILLIC CAPITAL LETTER CHE WITH VERTICAL STROKE
    case 0x04bb: r=0x04ba; break; // CYRILLIC CAPITAL LETTER SHHA
    case 0x04bd: r=0x04bc; break; // CYRILLIC CAPITAL LETTER ABKHASIAN CHE
    case 0x04bf: r=0x04be; break; // CYRILLIC CAPITAL LETTER ABKHASIAN CHE WITH DESCENDER
    case 0x04c2: r=0x04c1; break; // CYRILLIC CAPITAL LETTER ZHE WITH BREVE
    case 0x04c4: r=0x04c3; break; // CYRILLIC CAPITAL LETTER KA WITH HOOK
    case 0x04c6: r=0x04c5; break; // CYRILLIC CAPITAL LETTER EL WITH TAIL
    case 0x04c8: r=0x04c7; break; // CYRILLIC CAPITAL LETTER EN WITH HOOK
    case 0x04ca: r=0x04c9; break; // CYRILLIC CAPITAL LETTER EN WITH TAIL
    case 0x04cc: r=0x04cb; break; // CYRILLIC CAPITAL LETTER KHAKASSIAN CHE
    case 0x04ce: r=0x04cd; break; // CYRILLIC CAPITAL LETTER EM WITH TAIL
    case 0x04d1: r=0x04d0; break; // CYRILLIC CAPITAL LETTER A WITH BREVE
    case 0x04d3: r=0x04d2; break; // CYRILLIC CAPITAL LETTER A WITH DIAERESIS
    case 0x04d5: r=0x04d4; break; // CYRILLIC CAPITAL LIGATURE A IE
    case 0x04d7: r=0x04d6; break; // CYRILLIC CAPITAL LETTER IE WITH BREVE
    case 0x04d9: r=0x04d8; break; // CYRILLIC CAPITAL LETTER SCHWA
    case 0x04db: r=0x04da; break; // CYRILLIC CAPITAL LETTER SCHWA WITH DIAERESIS
    case 0x04dd: r=0x04dc; break; // CYRILLIC CAPITAL LETTER ZHE WITH DIAERESIS
    case 0x04df: r=0x04de; break; // CYRILLIC CAPITAL LETTER ZE WITH DIAERESIS
    case 0x04e1: r=0x04e0; break; // CYRILLIC CAPITAL LETTER ABKHASIAN DZE
    case 0x04e3: r=0x04e2; break; // CYRILLIC CAPITAL LETTER I WITH MACRON
    case 0x04eb: r=0x04ea; break; // CYRILLIC CAPITAL LETTER BARRED O WITH DIAERESIS
    case 0x04ed: r=0x04ec; break; // CYRILLIC CAPITAL LETTER E WITH DIAERESIS
    case 0x04ef: r=0x04ee; break; // CYRILLIC CAPITAL LETTER U WITH MACRON
    case 0x04f1: r=0x04f0; break; // CYRILLIC CAPITAL LETTER U WITH DIAERESIS
    case 0x04f3: r=0x04f2; break; // CYRILLIC CAPITAL LETTER U WITH DOUBLE ACUTE
    case 0x04f5: r=0x04f4; break; // CYRILLIC CAPITAL LETTER CHE WITH DIAERESIS
    case 0x04f7: r=0x04f6; break; // CYRILLIC CAPITAL LETTER GHE WITH DESCENDER
    case 0x04f9: r=0x04f8; break; // CYRILLIC CAPITAL LETTER YERU WITH DIAERESIS
    case 0x0501: r=0x0500; break; // CYRILLIC CAPITAL LETTER KOMI DE
    case 0x0503: r=0x0502; break; // CYRILLIC CAPITAL LETTER KOMI DJE
    case 0x0505: r=0x0504; break; // CYRILLIC CAPITAL LETTER KOMI ZJE
    case 0x0507: r=0x0506; break; // CYRILLIC CAPITAL LETTER KOMI DZJE
    case 0x0509: r=0x0508; break; // CYRILLIC CAPITAL LETTER KOMI LJE
    case 0x050b: r=0x050a; break; // CYRILLIC CAPITAL LETTER KOMI NJE
    case 0x050d: r=0x050c; break; // CYRILLIC CAPITAL LETTER KOMI SJE
    case 0x050f: r=0x050e; break; // CYRILLIC CAPITAL LETTER KOMI TJE
    case 0x0561: r=0x0531; break; // ARMENIAN CAPITAL LETTER AYB
    case 0x0562: r=0x0532; break; // ARMENIAN CAPITAL LETTER BEN
    case 0x0563: r=0x0533; break; // ARMENIAN CAPITAL LETTER GIM
    case 0x0564: r=0x0534; break; // ARMENIAN CAPITAL LETTER DA
    case 0x0565: r=0x0535; break; // ARMENIAN CAPITAL LETTER ECH
    case 0x0566: r=0x0536; break; // ARMENIAN CAPITAL LETTER ZA
    case 0x0567: r=0x0537; break; // ARMENIAN CAPITAL LETTER EH
    case 0x0568: r=0x0538; break; // ARMENIAN CAPITAL LETTER ET
    case 0x0569: r=0x0539; break; // ARMENIAN CAPITAL LETTER TO
    case 0x056a: r=0x053a; break; // ARMENIAN CAPITAL LETTER ZHE
    case 0x056b: r=0x053b; break; // ARMENIAN CAPITAL LETTER INI
    case 0x056c: r=0x053c; break; // ARMENIAN CAPITAL LETTER LIWN
    case 0x056d: r=0x053d; break; // ARMENIAN CAPITAL LETTER XEH
    case 0x056e: r=0x053e; break; // ARMENIAN CAPITAL LETTER CA
    case 0x056f: r=0x053f; break; // ARMENIAN CAPITAL LETTER KEN
    case 0x0570: r=0x0540; break; // ARMENIAN CAPITAL LETTER HO
    case 0x0571: r=0x0541; break; // ARMENIAN CAPITAL LETTER JA
    case 0x0572: r=0x0542; break; // ARMENIAN CAPITAL LETTER GHAD
    case 0x0573: r=0x0543; break; // ARMENIAN CAPITAL LETTER CHEH
    case 0x0574: r=0x0544; break; // ARMENIAN CAPITAL LETTER MEN
    case 0x0575: r=0x0545; break; // ARMENIAN CAPITAL LETTER YI
    case 0x0576: r=0x0546; break; // ARMENIAN CAPITAL LETTER NOW
    case 0x0577: r=0x0547; break; // ARMENIAN CAPITAL LETTER SHA
    case 0x0578: r=0x0548; break; // ARMENIAN CAPITAL LETTER VO
    case 0x0579: r=0x0549; break; // ARMENIAN CAPITAL LETTER CHA
    case 0x057a: r=0x054a; break; // ARMENIAN CAPITAL LETTER PEH
    case 0x057b: r=0x054b; break; // ARMENIAN CAPITAL LETTER JHEH
    case 0x057c: r=0x054c; break; // ARMENIAN CAPITAL LETTER RA
    case 0x057d: r=0x054d; break; // ARMENIAN CAPITAL LETTER SEH
    case 0x057e: r=0x054e; break; // ARMENIAN CAPITAL LETTER VEW
    case 0x057f: r=0x054f; break; // ARMENIAN CAPITAL LETTER TIWN
    case 0x0580: r=0x0550; break; // ARMENIAN CAPITAL LETTER REH
    case 0x0581: r=0x0551; break; // ARMENIAN CAPITAL LETTER CO
    case 0x0582: r=0x0552; break; // ARMENIAN CAPITAL LETTER YIWN
    case 0x0583: r=0x0553; break; // ARMENIAN CAPITAL LETTER PIWR
    case 0x0584: r=0x0554; break; // ARMENIAN CAPITAL LETTER KEH
    case 0x0585: r=0x0555; break; // ARMENIAN CAPITAL LETTER OH
    case 0x0586: r=0x0556; break; // ARMENIAN CAPITAL LETTER FEH
    case 0x2d00: r=0x10a0; break; // GEORGIAN CAPITAL LETTER AN
    case 0x2d01: r=0x10a1; break; // GEORGIAN CAPITAL LETTER BAN
    case 0x2d02: r=0x10a2; break; // GEORGIAN CAPITAL LETTER GAN
    case 0x2d03: r=0x10a3; break; // GEORGIAN CAPITAL LETTER DON
    case 0x2d04: r=0x10a4; break; // GEORGIAN CAPITAL LETTER EN
    case 0x2d05: r=0x10a5; break; // GEORGIAN CAPITAL LETTER VIN
    case 0x2d06: r=0x10a6; break; // GEORGIAN CAPITAL LETTER ZEN
    case 0x2d07: r=0x10a7; break; // GEORGIAN CAPITAL LETTER TAN
    case 0x2d08: r=0x10a8; break; // GEORGIAN CAPITAL LETTER IN
    case 0x2d09: r=0x10a9; break; // GEORGIAN CAPITAL LETTER KAN
    case 0x2d0a: r=0x10aa; break; // GEORGIAN CAPITAL LETTER LAS
    case 0x2d0b: r=0x10ab; break; // GEORGIAN CAPITAL LETTER MAN
    case 0x2d0c: r=0x10ac; break; // GEORGIAN CAPITAL LETTER NAR
    case 0x2d0d: r=0x10ad; break; // GEORGIAN CAPITAL LETTER ON
    case 0x2d0e: r=0x10ae; break; // GEORGIAN CAPITAL LETTER PAR
    case 0x2d0f: r=0x10af; break; // GEORGIAN CAPITAL LETTER ZHAR
    case 0x2d10: r=0x10b0; break; // GEORGIAN CAPITAL LETTER RAE
    case 0x2d11: r=0x10b1; break; // GEORGIAN CAPITAL LETTER SAN
    case 0x2d12: r=0x10b2; break; // GEORGIAN CAPITAL LETTER TAR
    case 0x2d13: r=0x10b3; break; // GEORGIAN CAPITAL LETTER UN
    case 0x2d14: r=0x10b4; break; // GEORGIAN CAPITAL LETTER PHAR
    case 0x2d15: r=0x10b5; break; // GEORGIAN CAPITAL LETTER KHAR
    case 0x2d16: r=0x10b6; break; // GEORGIAN CAPITAL LETTER GHAN
    case 0x2d17: r=0x10b7; break; // GEORGIAN CAPITAL LETTER QAR
    case 0x2d18: r=0x10b8; break; // GEORGIAN CAPITAL LETTER SHIN
    case 0x2d19: r=0x10b9; break; // GEORGIAN CAPITAL LETTER CHIN
    case 0x2d1a: r=0x10ba; break; // GEORGIAN CAPITAL LETTER CAN
    case 0x2d1b: r=0x10bb; break; // GEORGIAN CAPITAL LETTER JIL
    case 0x2d1c: r=0x10bc; break; // GEORGIAN CAPITAL LETTER CIL
    case 0x2d1d: r=0x10bd; break; // GEORGIAN CAPITAL LETTER CHAR
    case 0x2d1e: r=0x10be; break; // GEORGIAN CAPITAL LETTER XAN
    case 0x2d1f: r=0x10bf; break; // GEORGIAN CAPITAL LETTER JHAN
    case 0x2d20: r=0x10c0; break; // GEORGIAN CAPITAL LETTER HAE
    case 0x2d21: r=0x10c1; break; // GEORGIAN CAPITAL LETTER HE
    case 0x2d22: r=0x10c2; break; // GEORGIAN CAPITAL LETTER HIE
    case 0x2d23: r=0x10c3; break; // GEORGIAN CAPITAL LETTER WE
    case 0x2d24: r=0x10c4; break; // GEORGIAN CAPITAL LETTER HAR
    case 0x2d25: r=0x10c5; break; // GEORGIAN CAPITAL LETTER HOE
    case 0x1e01: r=0x1e00; break; // LATIN CAPITAL LETTER A WITH RING BELOW
    case 0x1e03: r=0x1e02; break; // LATIN CAPITAL LETTER B WITH DOT ABOVE
    case 0x1e0b: r=0x1e0a; break; // LATIN CAPITAL LETTER D WITH DOT ABOVE
    case 0x1e0d: r=0x1e0c; break; // LATIN CAPITAL LETTER D WITH DOT BELOW
    case 0x1e0f: r=0x1e0e; break; // LATIN CAPITAL LETTER D WITH LINE BELOW
    case 0x1e1b: r=0x1e1a; break; // LATIN CAPITAL LETTER E WITH TILDE BELOW
    case 0x1e1d: r=0x1e1c; break; // LATIN CAPITAL LETTER E WITH CEDILLA AND BREVE
    case 0x1e1f: r=0x1e1e; break; // LATIN CAPITAL LETTER F WITH DOT ABOVE
    case 0x1e2b: r=0x1e2a; break; // LATIN CAPITAL LETTER H WITH BREVE BELOW
    case 0x1e2d: r=0x1e2c; break; // LATIN CAPITAL LETTER I WITH TILDE BELOW
    case 0x1e2f: r=0x1e2e; break; // LATIN CAPITAL LETTER I WITH DIAERESIS AND ACUTE
    case 0x1e3b: r=0x1e3a; break; // LATIN CAPITAL LETTER L WITH LINE BELOW
    case 0x1e3d: r=0x1e3c; break; // LATIN CAPITAL LETTER L WITH CIRCUMFLEX BELOW
    case 0x1e3f: r=0x1e3e; break; // LATIN CAPITAL LETTER M WITH ACUTE
    case 0x1e4b: r=0x1e4a; break; // LATIN CAPITAL LETTER N WITH CIRCUMFLEX BELOW
    case 0x1e4d: r=0x1e4c; break; // LATIN CAPITAL LETTER O WITH TILDE AND ACUTE
    case 0x1e4f: r=0x1e4e; break; // LATIN CAPITAL LETTER O WITH TILDE AND DIAERESIS
    case 0x1ea1: r=0x1ea0; break; // LATIN CAPITAL LETTER A WITH DOT BELOW
    case 0x1ea3: r=0x1ea2; break; // LATIN CAPITAL LETTER A WITH HOOK ABOVE
    case 0x1ea5: r=0x1ea4; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND ACUTE
    case 0x1ea7: r=0x1ea6; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND GRAVE
    case 0x1ea9: r=0x1ea8; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND HOOK ABOVE
    case 0x1eab: r=0x1eaa; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND TILDE
    case 0x1ead: r=0x1eac; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND DOT BELOW
    case 0x1eaf: r=0x1eae; break; // LATIN CAPITAL LETTER A WITH BREVE AND ACUTE
    case 0x1eb1: r=0x1eb0; break; // LATIN CAPITAL LETTER A WITH BREVE AND GRAVE
    case 0x1eb3: r=0x1eb2; break; // LATIN CAPITAL LETTER A WITH BREVE AND HOOK ABOVE
    case 0x1eb5: r=0x1eb4; break; // LATIN CAPITAL LETTER A WITH BREVE AND TILDE
    case 0x1eb7: r=0x1eb6; break; // LATIN CAPITAL LETTER A WITH BREVE AND DOT BELOW
    case 0x1eb9: r=0x1eb8; break; // LATIN CAPITAL LETTER E WITH DOT BELOW
    case 0x1ebb: r=0x1eba; break; // LATIN CAPITAL LETTER E WITH HOOK ABOVE
    case 0x1ebd: r=0x1ebc; break; // LATIN CAPITAL LETTER E WITH TILDE
    case 0x1ebf: r=0x1ebe; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND ACUTE
    case 0x1ec1: r=0x1ec0; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND GRAVE
    case 0x1ec3: r=0x1ec2; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND HOOK ABOVE
    case 0x1ec5: r=0x1ec4; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND TILDE
    case 0x1ec7: r=0x1ec6; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND DOT BELOW
    case 0x1ec9: r=0x1ec8; break; // LATIN CAPITAL LETTER I WITH HOOK ABOVE
    case 0x1ecb: r=0x1eca; break; // LATIN CAPITAL LETTER I WITH DOT BELOW
    case 0x1ecd: r=0x1ecc; break; // LATIN CAPITAL LETTER O WITH DOT BELOW
    case 0x1ecf: r=0x1ece; break; // LATIN CAPITAL LETTER O WITH HOOK ABOVE
    case 0x1ed1: r=0x1ed0; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND ACUTE
    case 0x1ed3: r=0x1ed2; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND GRAVE
    case 0x1ed5: r=0x1ed4; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND HOOK ABOVE
    case 0x1ed7: r=0x1ed6; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND TILDE
    case 0x1ed9: r=0x1ed8; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND DOT BELOW
    case 0x1edb: r=0x1eda; break; // LATIN CAPITAL LETTER O WITH HORN AND ACUTE
    case 0x1edd: r=0x1edc; break; // LATIN CAPITAL LETTER O WITH HORN AND GRAVE
    case 0x1edf: r=0x1ede; break; // LATIN CAPITAL LETTER O WITH HORN AND HOOK ABOVE
    case 0x1ee1: r=0x1ee0; break; // LATIN CAPITAL LETTER O WITH HORN AND TILDE
    case 0x1ee3: r=0x1ee2; break; // LATIN CAPITAL LETTER O WITH HORN AND DOT BELOW
    case 0x1ee5: r=0x1ee4; break; // LATIN CAPITAL LETTER U WITH DOT BELOW
    case 0x1ee7: r=0x1ee6; break; // LATIN CAPITAL LETTER U WITH HOOK ABOVE
    case 0x1ee9: r=0x1ee8; break; // LATIN CAPITAL LETTER U WITH HORN AND ACUTE
    case 0x1eeb: r=0x1eea; break; // LATIN CAPITAL LETTER U WITH HORN AND GRAVE
    case 0x1eed: r=0x1eec; break; // LATIN CAPITAL LETTER U WITH HORN AND HOOK ABOVE
    case 0x1eef: r=0x1eee; break; // LATIN CAPITAL LETTER U WITH HORN AND TILDE
    case 0x1ef1: r=0x1ef0; break; // LATIN CAPITAL LETTER U WITH HORN AND DOT BELOW
    case 0x1ef3: r=0x1ef2; break; // LATIN CAPITAL LETTER Y WITH GRAVE
    case 0x1ef5: r=0x1ef4; break; // LATIN CAPITAL LETTER Y WITH DOT BELOW
    case 0x1ef7: r=0x1ef6; break; // LATIN CAPITAL LETTER Y WITH HOOK ABOVE
    case 0x1ef9: r=0x1ef8; break; // LATIN CAPITAL LETTER Y WITH TILDE
    case 0x1f00: r=0x1f08; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI
    case 0x1f01: r=0x1f09; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA
    case 0x1f02: r=0x1f0a; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND VARIA
    case 0x1f03: r=0x1f0b; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND VARIA
    case 0x1f04: r=0x1f0c; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND OXIA
    case 0x1f05: r=0x1f0d; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND OXIA
    case 0x1f06: r=0x1f0e; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PERISPOMENI
    case 0x1f07: r=0x1f0f; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PERISPOMENI
    case 0x1f10: r=0x1f18; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI
    case 0x1f11: r=0x1f19; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA
    case 0x1f12: r=0x1f1a; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI AND VARIA
    case 0x1f13: r=0x1f1b; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA AND VARIA
    case 0x1f14: r=0x1f1c; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI AND OXIA
    case 0x1f15: r=0x1f1d; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA AND OXIA
    case 0x1f20: r=0x1f28; break; // GREEK CAPITAL LETTER ETA WITH PSILI
    case 0x1f21: r=0x1f29; break; // GREEK CAPITAL LETTER ETA WITH DASIA
    case 0x1f22: r=0x1f2a; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND VARIA
    case 0x1f23: r=0x1f2b; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND VARIA
    case 0x1f24: r=0x1f2c; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND OXIA
    case 0x1f25: r=0x1f2d; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND OXIA
    case 0x1f26: r=0x1f2e; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PERISPOMENI
    case 0x1f27: r=0x1f2f; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PERISPOMENI
    case 0x1f30: r=0x1f38; break; // GREEK CAPITAL LETTER IOTA WITH PSILI
    case 0x1f31: r=0x1f39; break; // GREEK CAPITAL LETTER IOTA WITH DASIA
    case 0x1f32: r=0x1f3a; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND VARIA
    case 0x1f33: r=0x1f3b; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND VARIA
    case 0x1f34: r=0x1f3c; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND OXIA
    case 0x1f35: r=0x1f3d; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND OXIA
    case 0x1f36: r=0x1f3e; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND PERISPOMENI
    case 0x1f37: r=0x1f3f; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND PERISPOMENI
    case 0x1f40: r=0x1f48; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI
    case 0x1f41: r=0x1f49; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA
    case 0x1f42: r=0x1f4a; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI AND VARIA
    case 0x1f43: r=0x1f4b; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA AND VARIA
    case 0x1f44: r=0x1f4c; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI AND OXIA
    case 0x1f45: r=0x1f4d; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA AND OXIA
    case 0x1f51: r=0x1f59; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA
    case 0x1f53: r=0x1f5b; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND VARIA
    case 0x1f55: r=0x1f5d; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND OXIA
    case 0x1f57: r=0x1f5f; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND PERISPOMENI
    case 0x1f60: r=0x1f68; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI
    case 0x1f61: r=0x1f69; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA
    case 0x1f62: r=0x1f6a; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND VARIA
    case 0x1f63: r=0x1f6b; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND VARIA
    case 0x1f64: r=0x1f6c; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND OXIA
    case 0x1f65: r=0x1f6d; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND OXIA
    case 0x1f66: r=0x1f6e; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PERISPOMENI
    case 0x1f67: r=0x1f6f; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PERISPOMENI
    case 0x1f80: r=0x1f88; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PROSGEGRAMMENI
    case 0x1f81: r=0x1f89; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PROSGEGRAMMENI
    case 0x1f82: r=0x1f8a; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND VARIA AND PROSGEGRAMMENI
    case 0x1f83: r=0x1f8b; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND VARIA AND PROSGEGRAMMENI
    case 0x1f84: r=0x1f8c; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND OXIA AND PROSGEGRAMMENI
    case 0x1f85: r=0x1f8d; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND OXIA AND PROSGEGRAMMENI
    case 0x1f86: r=0x1f8e; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1f87: r=0x1f8f; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1f90: r=0x1f98; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PROSGEGRAMMENI
    case 0x1f91: r=0x1f99; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PROSGEGRAMMENI
    case 0x1f92: r=0x1f9a; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND VARIA AND PROSGEGRAMMENI
    case 0x1f93: r=0x1f9b; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND VARIA AND PROSGEGRAMMENI
    case 0x1f94: r=0x1f9c; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND OXIA AND PROSGEGRAMMENI
    case 0x1f95: r=0x1f9d; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND OXIA AND PROSGEGRAMMENI
    case 0x1f96: r=0x1f9e; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1f97: r=0x1f9f; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1fa0: r=0x1fa8; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PROSGEGRAMMENI
    case 0x1fa1: r=0x1fa9; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PROSGEGRAMMENI
    case 0x1fa2: r=0x1faa; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND VARIA AND PROSGEGRAMMENI
    case 0x1fa3: r=0x1fab; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND VARIA AND PROSGEGRAMMENI
    case 0x1fa4: r=0x1fac; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND OXIA AND PROSGEGRAMMENI
    case 0x1fa5: r=0x1fad; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND OXIA AND PROSGEGRAMMENI
    case 0x1fa6: r=0x1fae; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1fa7: r=0x1faf; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1fb0: r=0x1fb8; break; // GREEK CAPITAL LETTER ALPHA WITH VRACHY
    case 0x1fb1: r=0x1fb9; break; // GREEK CAPITAL LETTER ALPHA WITH MACRON
    case 0x1f70: r=0x1fba; break; // GREEK CAPITAL LETTER ALPHA WITH VARIA
    case 0x1f71: r=0x1fbb; break; // GREEK CAPITAL LETTER ALPHA WITH OXIA
    case 0x1fb3: r=0x1fbc; break; // GREEK CAPITAL LETTER ALPHA WITH PROSGEGRAMMENI
    case 0x1f72: r=0x1fc8; break; // GREEK CAPITAL LETTER EPSILON WITH VARIA
    case 0x1f73: r=0x1fc9; break; // GREEK CAPITAL LETTER EPSILON WITH OXIA
    case 0x1f74: r=0x1fca; break; // GREEK CAPITAL LETTER ETA WITH VARIA
    case 0x1f75: r=0x1fcb; break; // GREEK CAPITAL LETTER ETA WITH OXIA
    case 0x1fc3: r=0x1fcc; break; // GREEK CAPITAL LETTER ETA WITH PROSGEGRAMMENI
    case 0x1fd0: r=0x1fd8; break; // GREEK CAPITAL LETTER IOTA WITH VRACHY
    case 0x1fd1: r=0x1fd9; break; // GREEK CAPITAL LETTER IOTA WITH MACRON
    case 0x1f76: r=0x1fda; break; // GREEK CAPITAL LETTER IOTA WITH VARIA
    case 0x1f77: r=0x1fdb; break; // GREEK CAPITAL LETTER IOTA WITH OXIA
    case 0x1fe0: r=0x1fe8; break; // GREEK CAPITAL LETTER UPSILON WITH VRACHY
    case 0x1fe1: r=0x1fe9; break; // GREEK CAPITAL LETTER UPSILON WITH MACRON
    case 0x1f7a: r=0x1fea; break; // GREEK CAPITAL LETTER UPSILON WITH VARIA
    case 0x1f7b: r=0x1feb; break; // GREEK CAPITAL LETTER UPSILON WITH OXIA
    case 0x1fe5: r=0x1fec; break; // GREEK CAPITAL LETTER RHO WITH DASIA
    case 0x1f78: r=0x1ff8; break; // GREEK CAPITAL LETTER OMICRON WITH VARIA
    case 0x1f79: r=0x1ff9; break; // GREEK CAPITAL LETTER OMICRON WITH OXIA
    case 0x1f7c: r=0x1ffa; break; // GREEK CAPITAL LETTER OMEGA WITH VARIA
    case 0x1f7d: r=0x1ffb; break; // GREEK CAPITAL LETTER OMEGA WITH OXIA
    case 0x1ff3: r=0x1ffc; break; // GREEK CAPITAL LETTER OMEGA WITH PROSGEGRAMMENI
    case 0x2170: r=0x2160; break; // ROMAN NUMERAL ONE
    case 0x2171: r=0x2161; break; // ROMAN NUMERAL TWO
    case 0x2172: r=0x2162; break; // ROMAN NUMERAL THREE
    case 0x2173: r=0x2163; break; // ROMAN NUMERAL FOUR
    case 0x2174: r=0x2164; break; // ROMAN NUMERAL FIVE
    case 0x2175: r=0x2165; break; // ROMAN NUMERAL SIX
    case 0x2176: r=0x2166; break; // ROMAN NUMERAL SEVEN
    case 0x2177: r=0x2167; break; // ROMAN NUMERAL EIGHT
    case 0x2178: r=0x2168; break; // ROMAN NUMERAL NINE
    case 0x2179: r=0x2169; break; // ROMAN NUMERAL TEN
    case 0x217a: r=0x216a; break; // ROMAN NUMERAL ELEVEN
    case 0x217b: r=0x216b; break; // ROMAN NUMERAL TWELVE
    case 0x217c: r=0x216c; break; // ROMAN NUMERAL FIFTY
    case 0x217d: r=0x216d; break; // ROMAN NUMERAL ONE HUNDRED
    case 0x217e: r=0x216e; break; // ROMAN NUMERAL FIVE HUNDRED
    case 0x217f: r=0x216f; break; // ROMAN NUMERAL ONE THOUSAND
    case 0x24d0: r=0x24b6; break; // CIRCLED LATIN CAPITAL LETTER A
    case 0x24d1: r=0x24b7; break; // CIRCLED LATIN CAPITAL LETTER B
    case 0x24d2: r=0x24b8; break; // CIRCLED LATIN CAPITAL LETTER C
    case 0x24d3: r=0x24b9; break; // CIRCLED LATIN CAPITAL LETTER D
    case 0x24d4: r=0x24ba; break; // CIRCLED LATIN CAPITAL LETTER E
    case 0x24d5: r=0x24bb; break; // CIRCLED LATIN CAPITAL LETTER F
    case 0x24d6: r=0x24bc; break; // CIRCLED LATIN CAPITAL LETTER G
    case 0x24d7: r=0x24bd; break; // CIRCLED LATIN CAPITAL LETTER H
    case 0x24d8: r=0x24be; break; // CIRCLED LATIN CAPITAL LETTER I
    case 0x24d9: r=0x24bf; break; // CIRCLED LATIN CAPITAL LETTER J
    case 0x24da: r=0x24c0; break; // CIRCLED LATIN CAPITAL LETTER K
    case 0x24db: r=0x24c1; break; // CIRCLED LATIN CAPITAL LETTER L
    case 0x24dc: r=0x24c2; break; // CIRCLED LATIN CAPITAL LETTER M
    case 0x24dd: r=0x24c3; break; // CIRCLED LATIN CAPITAL LETTER N
    case 0x24de: r=0x24c4; break; // CIRCLED LATIN CAPITAL LETTER O
    case 0x24df: r=0x24c5; break; // CIRCLED LATIN CAPITAL LETTER P
    case 0x24e0: r=0x24c6; break; // CIRCLED LATIN CAPITAL LETTER Q
    case 0x24e1: r=0x24c7; break; // CIRCLED LATIN CAPITAL LETTER R
    case 0x24e2: r=0x24c8; break; // CIRCLED LATIN CAPITAL LETTER S
    case 0x24e3: r=0x24c9; break; // CIRCLED LATIN CAPITAL LETTER T
    case 0x2c30: r=0x2c00; break; // GLAGOLITIC CAPITAL LETTER AZU
    case 0x2c31: r=0x2c01; break; // GLAGOLITIC CAPITAL LETTER BUKY
    case 0x2c32: r=0x2c02; break; // GLAGOLITIC CAPITAL LETTER VEDE
    case 0x2c33: r=0x2c03; break; // GLAGOLITIC CAPITAL LETTER GLAGOLI
    case 0x2c34: r=0x2c04; break; // GLAGOLITIC CAPITAL LETTER DOBRO
    case 0x2c35: r=0x2c05; break; // GLAGOLITIC CAPITAL LETTER YESTU
    case 0x2c36: r=0x2c06; break; // GLAGOLITIC CAPITAL LETTER ZHIVETE
    case 0x2c37: r=0x2c07; break; // GLAGOLITIC CAPITAL LETTER DZELO
    case 0x2c38: r=0x2c08; break; // GLAGOLITIC CAPITAL LETTER ZEMLJA
    case 0x2c39: r=0x2c09; break; // GLAGOLITIC CAPITAL LETTER IZHE
    case 0x2c3a: r=0x2c0a; break; // GLAGOLITIC CAPITAL LETTER INITIAL IZHE
    case 0x2c3b: r=0x2c0b; break; // GLAGOLITIC CAPITAL LETTER I
    case 0x2c3c: r=0x2c0c; break; // GLAGOLITIC CAPITAL LETTER DJERVI
    case 0x2c3d: r=0x2c0d; break; // GLAGOLITIC CAPITAL LETTER KAKO
    case 0x2c3e: r=0x2c0e; break; // GLAGOLITIC CAPITAL LETTER LJUDIJE
    case 0x2c3f: r=0x2c0f; break; // GLAGOLITIC CAPITAL LETTER MYSLITE
    case 0x2c40: r=0x2c10; break; // GLAGOLITIC CAPITAL LETTER NASHI
    case 0x2c41: r=0x2c11; break; // GLAGOLITIC CAPITAL LETTER ONU
    case 0x2c42: r=0x2c12; break; // GLAGOLITIC CAPITAL LETTER POKOJI
    case 0x2c43: r=0x2c13; break; // GLAGOLITIC CAPITAL LETTER RITSI
    case 0x2c44: r=0x2c14; break; // GLAGOLITIC CAPITAL LETTER SLOVO
    case 0x2c45: r=0x2c15; break; // GLAGOLITIC CAPITAL LETTER TVRIDO
    case 0x2c46: r=0x2c16; break; // GLAGOLITIC CAPITAL LETTER UKU
    case 0x2c47: r=0x2c17; break; // GLAGOLITIC CAPITAL LETTER FRITU
    case 0x2c48: r=0x2c18; break; // GLAGOLITIC CAPITAL LETTER HERU
    case 0x2c49: r=0x2c19; break; // GLAGOLITIC CAPITAL LETTER OTU
    case 0x2c4a: r=0x2c1a; break; // GLAGOLITIC CAPITAL LETTER PE
    case 0x2c4b: r=0x2c1b; break; // GLAGOLITIC CAPITAL LETTER SHTA
    case 0x2c4c: r=0x2c1c; break; // GLAGOLITIC CAPITAL LETTER TSI
    case 0x2c4d: r=0x2c1d; break; // GLAGOLITIC CAPITAL LETTER CHRIVI
    case 0x2c4e: r=0x2c1e; break; // GLAGOLITIC CAPITAL LETTER SHA
    case 0x2c4f: r=0x2c1f; break; // GLAGOLITIC CAPITAL LETTER YERU
    case 0x2c50: r=0x2c20; break; // GLAGOLITIC CAPITAL LETTER YERI
    case 0x2c51: r=0x2c21; break; // GLAGOLITIC CAPITAL LETTER YATI
    case 0x2c52: r=0x2c22; break; // GLAGOLITIC CAPITAL LETTER SPIDERY HA
    case 0x2c53: r=0x2c23; break; // GLAGOLITIC CAPITAL LETTER YU
    case 0x2c54: r=0x2c24; break; // GLAGOLITIC CAPITAL LETTER SMALL YUS
    case 0x2c55: r=0x2c25; break; // GLAGOLITIC CAPITAL LETTER SMALL YUS WITH TAIL
    case 0x2c56: r=0x2c26; break; // GLAGOLITIC CAPITAL LETTER YO
    case 0x2c57: r=0x2c27; break; // GLAGOLITIC CAPITAL LETTER IOTATED SMALL YUS
    case 0x2c58: r=0x2c28; break; // GLAGOLITIC CAPITAL LETTER BIG YUS
    case 0x2c59: r=0x2c29; break; // GLAGOLITIC CAPITAL LETTER IOTATED BIG YUS
    case 0x2c5a: r=0x2c2a; break; // GLAGOLITIC CAPITAL LETTER FITA
    case 0x2c5b: r=0x2c2b; break; // GLAGOLITIC CAPITAL LETTER IZHITSA
    case 0x2c5c: r=0x2c2c; break; // GLAGOLITIC CAPITAL LETTER SHTAPIC
    case 0x2c5d: r=0x2c2d; break; // GLAGOLITIC CAPITAL LETTER TROKUTASTI A
    case 0x2c5e: r=0x2c2e; break; // GLAGOLITIC CAPITAL LETTER LATINATE MYSLITE
    case 0x2c81: r=0x2c80; break; // COPTIC CAPITAL LETTER ALFA
    case 0x2c83: r=0x2c82; break; // COPTIC CAPITAL LETTER VIDA
    case 0x2c85: r=0x2c84; break; // COPTIC CAPITAL LETTER GAMMA
    case 0x2c87: r=0x2c86; break; // COPTIC CAPITAL LETTER DALDA
    case 0x2c89: r=0x2c88; break; // COPTIC CAPITAL LETTER EIE
    case 0x2c8b: r=0x2c8a; break; // COPTIC CAPITAL LETTER SOU
    case 0x2c8d: r=0x2c8c; break; // COPTIC CAPITAL LETTER ZATA
    case 0x2c8f: r=0x2c8e; break; // COPTIC CAPITAL LETTER HATE
    case 0x2c91: r=0x2c90; break; // COPTIC CAPITAL LETTER THETHE
    case 0x2c93: r=0x2c92; break; // COPTIC CAPITAL LETTER IAUDA
    case 0x2c95: r=0x2c94; break; // COPTIC CAPITAL LETTER KAPA
    case 0x2c97: r=0x2c96; break; // COPTIC CAPITAL LETTER LAULA
    case 0x2c99: r=0x2c98; break; // COPTIC CAPITAL LETTER MI
    case 0x2c9b: r=0x2c9a; break; // COPTIC CAPITAL LETTER NI
    case 0x2c9d: r=0x2c9c; break; // COPTIC CAPITAL LETTER KSI
    case 0x2c9f: r=0x2c9e; break; // COPTIC CAPITAL LETTER O
    case 0x2ca1: r=0x2ca0; break; // COPTIC CAPITAL LETTER PI
    case 0x2ca3: r=0x2ca2; break; // COPTIC CAPITAL LETTER RO
    case 0x2ca5: r=0x2ca4; break; // COPTIC CAPITAL LETTER SIMA
    case 0x2ca7: r=0x2ca6; break; // COPTIC CAPITAL LETTER TAU
    case 0x2ca9: r=0x2ca8; break; // COPTIC CAPITAL LETTER UA
    case 0x2cab: r=0x2caa; break; // COPTIC CAPITAL LETTER FI
    case 0x2cad: r=0x2cac; break; // COPTIC CAPITAL LETTER KHI
    case 0x2caf: r=0x2cae; break; // COPTIC CAPITAL LETTER PSI
    case 0x2cb1: r=0x2cb0; break; // COPTIC CAPITAL LETTER OOU
    case 0x2cb3: r=0x2cb2; break; // COPTIC CAPITAL LETTER DIALECT-P ALEF
    case 0x2cb5: r=0x2cb4; break; // COPTIC CAPITAL LETTER OLD COPTIC AIN
    case 0x2cb7: r=0x2cb6; break; // COPTIC CAPITAL LETTER CRYPTOGRAMMIC EIE
    case 0x2cb9: r=0x2cb8; break; // COPTIC CAPITAL LETTER DIALECT-P KAPA
    case 0x2cbb: r=0x2cba; break; // COPTIC CAPITAL LETTER DIALECT-P NI
    case 0x2cbd: r=0x2cbc; break; // COPTIC CAPITAL LETTER CRYPTOGRAMMIC NI
    case 0x2cbf: r=0x2cbe; break; // COPTIC CAPITAL LETTER OLD COPTIC OOU
    case 0x2cc1: r=0x2cc0; break; // COPTIC CAPITAL LETTER SAMPI
    case 0x2cc3: r=0x2cc2; break; // COPTIC CAPITAL LETTER CROSSED SHEI
    case 0x2cc5: r=0x2cc4; break; // COPTIC CAPITAL LETTER OLD COPTIC SHEI
    case 0x2cc7: r=0x2cc6; break; // COPTIC CAPITAL LETTER OLD COPTIC ESH
    case 0x2cc9: r=0x2cc8; break; // COPTIC CAPITAL LETTER AKHMIMIC KHEI
    case 0x2ccb: r=0x2cca; break; // COPTIC CAPITAL LETTER DIALECT-P HORI
    case 0x2ccd: r=0x2ccc; break; // COPTIC CAPITAL LETTER OLD COPTIC HORI
    case 0x2ccf: r=0x2cce; break; // COPTIC CAPITAL LETTER OLD COPTIC HA
    case 0x2cd1: r=0x2cd0; break; // COPTIC CAPITAL LETTER L-SHAPED HA
    case 0x2cd3: r=0x2cd2; break; // COPTIC CAPITAL LETTER OLD COPTIC HEI
    case 0x2cd5: r=0x2cd4; break; // COPTIC CAPITAL LETTER OLD COPTIC HAT
    case 0x2cd7: r=0x2cd6; break; // COPTIC CAPITAL LETTER OLD COPTIC GANGIA
    case 0x2cd9: r=0x2cd8; break; // COPTIC CAPITAL LETTER OLD COPTIC DJA
    case 0x2cdb: r=0x2cda; break; // COPTIC CAPITAL LETTER OLD COPTIC SHIMA
    case 0x2cdd: r=0x2cdc; break; // COPTIC CAPITAL LETTER OLD NUBIAN SHIMA
    case 0x2cdf: r=0x2cde; break; // COPTIC CAPITAL LETTER OLD NUBIAN NGI
    case 0x2ce1: r=0x2ce0; break; // COPTIC CAPITAL LETTER OLD NUBIAN NYI
    case 0x2ce3: r=0x2ce2; break; // COPTIC CAPITAL LETTER OLD NUBIAN WAU
    case 0xff41: r=0xff21; break; // FULLWIDTH LATIN CAPITAL LETTER A
    case 0xff42: r=0xff22; break; // FULLWIDTH LATIN CAPITAL LETTER B
    case 0xff43: r=0xff23; break; // FULLWIDTH LATIN CAPITAL LETTER C
    case 0xff44: r=0xff24; break; // FULLWIDTH LATIN CAPITAL LETTER D
    case 0xff45: r=0xff25; break; // FULLWIDTH LATIN CAPITAL LETTER E
    case 0xff46: r=0xff26; break; // FULLWIDTH LATIN CAPITAL LETTER F
    case 0xff47: r=0xff27; break; // FULLWIDTH LATIN CAPITAL LETTER G
    case 0xff48: r=0xff28; break; // FULLWIDTH LATIN CAPITAL LETTER H
    case 0xff49: r=0xff29; break; // FULLWIDTH LATIN CAPITAL LETTER I
    case 0xff4a: r=0xff2a; break; // FULLWIDTH LATIN CAPITAL LETTER J
    case 0xff4b: r=0xff2b; break; // FULLWIDTH LATIN CAPITAL LETTER K
    case 0xff4c: r=0xff2c; break; // FULLWIDTH LATIN CAPITAL LETTER L
    case 0xff4d: r=0xff2d; break; // FULLWIDTH LATIN CAPITAL LETTER M
    case 0xff4e: r=0xff2e; break; // FULLWIDTH LATIN CAPITAL LETTER N
    case 0xff4f: r=0xff2f; break; // FULLWIDTH LATIN CAPITAL LETTER O
    case 0xff50: r=0xff30; break; // FULLWIDTH LATIN CAPITAL LETTER P
    case 0xff51: r=0xff31; break; // FULLWIDTH LATIN CAPITAL LETTER Q
    case 0xff52: r=0xff32; break; // FULLWIDTH LATIN CAPITAL LETTER R
    case 0xff53: r=0xff33; break; // FULLWIDTH LATIN CAPITAL LETTER S
    case 0xff54: r=0xff34; break; // FULLWIDTH LATIN CAPITAL LETTER T
    case 0xff55: r=0xff35; break; // FULLWIDTH LATIN CAPITAL LETTER U
    case 0xff56: r=0xff36; break; // FULLWIDTH LATIN CAPITAL LETTER V
    case 0xff57: r=0xff37; break; // FULLWIDTH LATIN CAPITAL LETTER W
    case 0xff58: r=0xff38; break; // FULLWIDTH LATIN CAPITAL LETTER X
    case 0xff59: r=0xff39; break; // FULLWIDTH LATIN CAPITAL LETTER Y
    case 0xff5a: r=0xff3a; break; // FULLWIDTH LATIN CAPITAL LETTER Z
      /* end of autogenerated code */
    default: r=c; break;
  }
  return r;
}


unichar u_tolower (unichar c) {
  unichar r;
  switch (c) {
      /* begin of autogenerated code */
    case 0x0041: r=0x0061; break; // LATIN CAPITAL LETTER A
    case 0x0042: r=0x0062; break; // LATIN CAPITAL LETTER B
    case 0x0043: r=0x0063; break; // LATIN CAPITAL LETTER C
    case 0x0044: r=0x0064; break; // LATIN CAPITAL LETTER D
    case 0x0045: r=0x0065; break; // LATIN CAPITAL LETTER E
    case 0x0046: r=0x0066; break; // LATIN CAPITAL LETTER F
    case 0x0047: r=0x0067; break; // LATIN CAPITAL LETTER G
    case 0x0048: r=0x0068; break; // LATIN CAPITAL LETTER H
    case 0x0049: r=0x0069; break; // LATIN CAPITAL LETTER I
    case 0x004a: r=0x006a; break; // LATIN CAPITAL LETTER J
    case 0x004b: r=0x006b; break; // LATIN CAPITAL LETTER K
    case 0x004c: r=0x006c; break; // LATIN CAPITAL LETTER L
    case 0x004d: r=0x006d; break; // LATIN CAPITAL LETTER M
    case 0x004e: r=0x006e; break; // LATIN CAPITAL LETTER N
    case 0x004f: r=0x006f; break; // LATIN CAPITAL LETTER O
    case 0x0050: r=0x0070; break; // LATIN CAPITAL LETTER P
    case 0x0051: r=0x0071; break; // LATIN CAPITAL LETTER Q
    case 0x0052: r=0x0072; break; // LATIN CAPITAL LETTER R
    case 0x0053: r=0x0073; break; // LATIN CAPITAL LETTER S
    case 0x0054: r=0x0074; break; // LATIN CAPITAL LETTER T
    case 0x0055: r=0x0075; break; // LATIN CAPITAL LETTER U
    case 0x0056: r=0x0076; break; // LATIN CAPITAL LETTER V
    case 0x0057: r=0x0077; break; // LATIN CAPITAL LETTER W
    case 0x0058: r=0x0078; break; // LATIN CAPITAL LETTER X
    case 0x0059: r=0x0079; break; // LATIN CAPITAL LETTER Y
    case 0x005a: r=0x007a; break; // LATIN CAPITAL LETTER Z
    case 0x00b5: r=0x03bc; break; // MICRO SIGN
    case 0x00c0: r=0x00e0; break; // LATIN CAPITAL LETTER A WITH GRAVE
    case 0x00c1: r=0x00e1; break; // LATIN CAPITAL LETTER A WITH ACUTE
    case 0x00c2: r=0x00e2; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
    case 0x00c3: r=0x00e3; break; // LATIN CAPITAL LETTER A WITH TILDE
    case 0x00c4: r=0x00e4; break; // LATIN CAPITAL LETTER A WITH DIAERESIS
    case 0x00c5: r=0x00e5; break; // LATIN CAPITAL LETTER A WITH RING ABOVE
    case 0x00c6: r=0x00e6; break; // LATIN CAPITAL LETTER AE
    case 0x00c7: r=0x00e7; break; // LATIN CAPITAL LETTER C WITH CEDILLA
    case 0x00c8: r=0x00e8; break; // LATIN CAPITAL LETTER E WITH GRAVE
    case 0x00c9: r=0x00e9; break; // LATIN CAPITAL LETTER E WITH ACUTE
    case 0x00ca: r=0x00ea; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX
    case 0x00cb: r=0x00eb; break; // LATIN CAPITAL LETTER E WITH DIAERESIS
    case 0x00cc: r=0x00ec; break; // LATIN CAPITAL LETTER I WITH GRAVE
    case 0x00cd: r=0x00ed; break; // LATIN CAPITAL LETTER I WITH ACUTE
    case 0x00ce: r=0x00ee; break; // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
    case 0x00cf: r=0x00ef; break; // LATIN CAPITAL LETTER I WITH DIAERESIS
    case 0x00d0: r=0x00f0; break; // LATIN CAPITAL LETTER ETH
    case 0x00d1: r=0x00f1; break; // LATIN CAPITAL LETTER N WITH TILDE
    case 0x00d2: r=0x00f2; break; // LATIN CAPITAL LETTER O WITH GRAVE
    case 0x00d3: r=0x00f3; break; // LATIN CAPITAL LETTER O WITH ACUTE
    case 0x00d4: r=0x00f4; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
    case 0x00d5: r=0x00f5; break; // LATIN CAPITAL LETTER O WITH TILDE
    case 0x00d6: r=0x00f6; break; // LATIN CAPITAL LETTER O WITH DIAERESIS
    case 0x00d8: r=0x00f8; break; // LATIN CAPITAL LETTER O WITH STROKE
    case 0x00d9: r=0x00f9; break; // LATIN CAPITAL LETTER U WITH GRAVE
    case 0x00da: r=0x00fa; break; // LATIN CAPITAL LETTER U WITH ACUTE
    case 0x00db: r=0x00fb; break; // LATIN CAPITAL LETTER U WITH CIRCUMFLEX
    case 0x00dc: r=0x00fc; break; // LATIN CAPITAL LETTER U WITH DIAERESIS
    case 0x00dd: r=0x00fd; break; // LATIN CAPITAL LETTER Y WITH ACUTE
    case 0x00de: r=0x00fe; break; // LATIN CAPITAL LETTER THORN
    case 0x0100: r=0x0101; break; // LATIN CAPITAL LETTER A WITH MACRON
    case 0x0102: r=0x0103; break; // LATIN CAPITAL LETTER A WITH BREVE
    case 0x0104: r=0x0105; break; // LATIN CAPITAL LETTER A WITH OGONEK
    case 0x0106: r=0x0107; break; // LATIN CAPITAL LETTER C WITH ACUTE
    case 0x0108: r=0x0109; break; // LATIN CAPITAL LETTER C WITH CIRCUMFLEX
    case 0x010a: r=0x010b; break; // LATIN CAPITAL LETTER C WITH DOT ABOVE
    case 0x010c: r=0x010d; break; // LATIN CAPITAL LETTER C WITH CARON
    case 0x010e: r=0x010f; break; // LATIN CAPITAL LETTER D WITH CARON
    case 0x0110: r=0x0111; break; // LATIN CAPITAL LETTER D WITH STROKE
    case 0x0112: r=0x0113; break; // LATIN CAPITAL LETTER E WITH MACRON
    case 0x0114: r=0x0115; break; // LATIN CAPITAL LETTER E WITH BREVE
    case 0x0116: r=0x0117; break; // LATIN CAPITAL LETTER E WITH DOT ABOVE
    case 0x0118: r=0x0119; break; // LATIN CAPITAL LETTER E WITH OGONEK
    case 0x011a: r=0x011b; break; // LATIN CAPITAL LETTER E WITH CARON
    case 0x011c: r=0x011d; break; // LATIN CAPITAL LETTER G WITH CIRCUMFLEX
    case 0x011e: r=0x011f; break; // LATIN CAPITAL LETTER G WITH BREVE
    case 0x0120: r=0x0121; break; // LATIN CAPITAL LETTER G WITH DOT ABOVE
    case 0x0122: r=0x0123; break; // LATIN CAPITAL LETTER G WITH CEDILLA
    case 0x0124: r=0x0125; break; // LATIN CAPITAL LETTER H WITH CIRCUMFLEX
    case 0x0126: r=0x0127; break; // LATIN CAPITAL LETTER H WITH STROKE
    case 0x0128: r=0x0129; break; // LATIN CAPITAL LETTER I WITH TILDE
    case 0x012a: r=0x012b; break; // LATIN CAPITAL LETTER I WITH MACRON
    case 0x012c: r=0x012d; break; // LATIN CAPITAL LETTER I WITH BREVE
    case 0x012e: r=0x012f; break; // LATIN CAPITAL LETTER I WITH OGONEK
    case 0x0132: r=0x0133; break; // LATIN CAPITAL LIGATURE IJ
    case 0x0134: r=0x0135; break; // LATIN CAPITAL LETTER J WITH CIRCUMFLEX
    case 0x0136: r=0x0137; break; // LATIN CAPITAL LETTER K WITH CEDILLA
    case 0x0139: r=0x013a; break; // LATIN CAPITAL LETTER L WITH ACUTE
    case 0x013b: r=0x013c; break; // LATIN CAPITAL LETTER L WITH CEDILLA
    case 0x013d: r=0x013e; break; // LATIN CAPITAL LETTER L WITH CARON
    case 0x013f: r=0x0140; break; // LATIN CAPITAL LETTER L WITH MIDDLE DOT
    case 0x0141: r=0x0142; break; // LATIN CAPITAL LETTER L WITH STROKE
    case 0x0143: r=0x0144; break; // LATIN CAPITAL LETTER N WITH ACUTE
    case 0x0145: r=0x0146; break; // LATIN CAPITAL LETTER N WITH CEDILLA
    case 0x0147: r=0x0148; break; // LATIN CAPITAL LETTER N WITH CARON
    case 0x014a: r=0x014b; break; // LATIN CAPITAL LETTER ENG
    case 0x014c: r=0x014d; break; // LATIN CAPITAL LETTER O WITH MACRON
    case 0x014e: r=0x014f; break; // LATIN CAPITAL LETTER O WITH BREVE
    case 0x0150: r=0x0151; break; // LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
    case 0x0152: r=0x0153; break; // LATIN CAPITAL LIGATURE OE
    case 0x0154: r=0x0155; break; // LATIN CAPITAL LETTER R WITH ACUTE
    case 0x0156: r=0x0157; break; // LATIN CAPITAL LETTER R WITH CEDILLA
    case 0x0158: r=0x0159; break; // LATIN CAPITAL LETTER R WITH CARON
    case 0x015a: r=0x015b; break; // LATIN CAPITAL LETTER S WITH ACUTE
    case 0x015c: r=0x015d; break; // LATIN CAPITAL LETTER S WITH CIRCUMFLEX
    case 0x015e: r=0x015f; break; // LATIN CAPITAL LETTER S WITH CEDILLA
    case 0x0160: r=0x0161; break; // LATIN CAPITAL LETTER S WITH CARON
    case 0x0162: r=0x0163; break; // LATIN CAPITAL LETTER T WITH CEDILLA
    case 0x0164: r=0x0165; break; // LATIN CAPITAL LETTER T WITH CARON
    case 0x0166: r=0x0167; break; // LATIN CAPITAL LETTER T WITH STROKE
    case 0x0168: r=0x0169; break; // LATIN CAPITAL LETTER U WITH TILDE
    case 0x016a: r=0x016b; break; // LATIN CAPITAL LETTER U WITH MACRON
    case 0x016c: r=0x016d; break; // LATIN CAPITAL LETTER U WITH BREVE
    case 0x016e: r=0x016f; break; // LATIN CAPITAL LETTER U WITH RING ABOVE
    case 0x0170: r=0x0171; break; // LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
    case 0x0172: r=0x0173; break; // LATIN CAPITAL LETTER U WITH OGONEK
    case 0x0174: r=0x0175; break; // LATIN CAPITAL LETTER W WITH CIRCUMFLEX
    case 0x0176: r=0x0177; break; // LATIN CAPITAL LETTER Y WITH CIRCUMFLEX
    case 0x0178: r=0x00ff; break; // LATIN CAPITAL LETTER Y WITH DIAERESIS
    case 0x0179: r=0x017a; break; // LATIN CAPITAL LETTER Z WITH ACUTE
    case 0x017b: r=0x017c; break; // LATIN CAPITAL LETTER Z WITH DOT ABOVE
    case 0x017d: r=0x017e; break; // LATIN CAPITAL LETTER Z WITH CARON
    case 0x017f: r=0x0073; break; // LATIN SMALL LETTER LONG S
    case 0x0181: r=0x0253; break; // LATIN CAPITAL LETTER B WITH HOOK
    case 0x0182: r=0x0183; break; // LATIN CAPITAL LETTER B WITH TOPBAR
    case 0x0184: r=0x0185; break; // LATIN CAPITAL LETTER TONE SIX
    case 0x0186: r=0x0254; break; // LATIN CAPITAL LETTER OPEN O
    case 0x0187: r=0x0188; break; // LATIN CAPITAL LETTER C WITH HOOK
    case 0x0189: r=0x0256; break; // LATIN CAPITAL LETTER AFRICAN D
    case 0x018a: r=0x0257; break; // LATIN CAPITAL LETTER D WITH HOOK
    case 0x018b: r=0x018c; break; // LATIN CAPITAL LETTER D WITH TOPBAR
    case 0x018e: r=0x01dd; break; // LATIN CAPITAL LETTER REVERSED E
    case 0x018f: r=0x0259; break; // LATIN CAPITAL LETTER SCHWA
    case 0x0190: r=0x025b; break; // LATIN CAPITAL LETTER OPEN E
    case 0x0191: r=0x0192; break; // LATIN CAPITAL LETTER F WITH HOOK
    case 0x0193: r=0x0260; break; // LATIN CAPITAL LETTER G WITH HOOK
    case 0x0194: r=0x0263; break; // LATIN CAPITAL LETTER GAMMA
    case 0x0196: r=0x0269; break; // LATIN CAPITAL LETTER IOTA
    case 0x0197: r=0x0268; break; // LATIN CAPITAL LETTER I WITH STROKE
    case 0x0198: r=0x0199; break; // LATIN CAPITAL LETTER K WITH HOOK
    case 0x019c: r=0x026f; break; // LATIN CAPITAL LETTER TURNED M
    case 0x019d: r=0x0272; break; // LATIN CAPITAL LETTER N WITH LEFT HOOK
    case 0x019f: r=0x0275; break; // LATIN CAPITAL LETTER O WITH MIDDLE TILDE
    case 0x01a0: r=0x01a1; break; // LATIN CAPITAL LETTER O WITH HORN
    case 0x01a2: r=0x01a3; break; // LATIN CAPITAL LETTER OI
    case 0x01a4: r=0x01a5; break; // LATIN CAPITAL LETTER P WITH HOOK
    case 0x01a6: r=0x0280; break; // LATIN LETTER YR
    case 0x01a7: r=0x01a8; break; // LATIN CAPITAL LETTER TONE TWO
    case 0x01a9: r=0x0283; break; // LATIN CAPITAL LETTER ESH
    case 0x01ac: r=0x01ad; break; // LATIN CAPITAL LETTER T WITH HOOK
    case 0x01ae: r=0x0288; break; // LATIN CAPITAL LETTER T WITH RETROFLEX HOOK
    case 0x01af: r=0x01b0; break; // LATIN CAPITAL LETTER U WITH HORN
    case 0x01b1: r=0x028a; break; // LATIN CAPITAL LETTER UPSILON
    case 0x01b2: r=0x028b; break; // LATIN CAPITAL LETTER V WITH HOOK
    case 0x01b3: r=0x01b4; break; // LATIN CAPITAL LETTER Y WITH HOOK
    case 0x01b5: r=0x01b6; break; // LATIN CAPITAL LETTER Z WITH STROKE
    case 0x01b7: r=0x0292; break; // LATIN CAPITAL LETTER EZH
    case 0x01b8: r=0x01b9; break; // LATIN CAPITAL LETTER EZH REVERSED
    case 0x01bc: r=0x01bd; break; // LATIN CAPITAL LETTER TONE FIVE
    case 0x01c4: r=0x01c6; break; // LATIN CAPITAL LETTER DZ WITH CARON
    case 0x01c5: r=0x01c6; break; // LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH CARON
    case 0x01c7: r=0x01c9; break; // LATIN CAPITAL LETTER LJ
    case 0x01c8: r=0x01c9; break; // LATIN CAPITAL LETTER L WITH SMALL LETTER J
    case 0x01ca: r=0x01cc; break; // LATIN CAPITAL LETTER NJ
    case 0x01cb: r=0x01cc; break; // LATIN CAPITAL LETTER N WITH SMALL LETTER J
    case 0x01cd: r=0x01ce; break; // LATIN CAPITAL LETTER A WITH CARON
    case 0x01cf: r=0x01d0; break; // LATIN CAPITAL LETTER I WITH CARON
    case 0x01d1: r=0x01d2; break; // LATIN CAPITAL LETTER O WITH CARON
    case 0x01d3: r=0x01d4; break; // LATIN CAPITAL LETTER U WITH CARON
    case 0x01d5: r=0x01d6; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND MACRON
    case 0x01d7: r=0x01d8; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND ACUTE
    case 0x01d9: r=0x01da; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND CARON
    case 0x01db: r=0x01dc; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND GRAVE
    case 0x01de: r=0x01df; break; // LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON
    case 0x01e0: r=0x01e1; break; // LATIN CAPITAL LETTER A WITH DOT ABOVE AND MACRON
    case 0x01e2: r=0x01e3; break; // LATIN CAPITAL LETTER AE WITH MACRON
    case 0x01e4: r=0x01e5; break; // LATIN CAPITAL LETTER G WITH STROKE
    case 0x01e6: r=0x01e7; break; // LATIN CAPITAL LETTER G WITH CARON
    case 0x01e8: r=0x01e9; break; // LATIN CAPITAL LETTER K WITH CARON
    case 0x01ea: r=0x01eb; break; // LATIN CAPITAL LETTER O WITH OGONEK
    case 0x01ec: r=0x01ed; break; // LATIN CAPITAL LETTER O WITH OGONEK AND MACRON
    case 0x01ee: r=0x01ef; break; // LATIN CAPITAL LETTER EZH WITH CARON
    case 0x01f1: r=0x01f3; break; // LATIN CAPITAL LETTER DZ
    case 0x01f2: r=0x01f3; break; // LATIN CAPITAL LETTER D WITH SMALL LETTER Z
    case 0x01f4: r=0x01f5; break; // LATIN CAPITAL LETTER G WITH ACUTE
    case 0x01f6: r=0x0195; break; // LATIN CAPITAL LETTER HWAIR
    case 0x01f7: r=0x01bf; break; // LATIN CAPITAL LETTER WYNN
    case 0x01f8: r=0x01f9; break; // LATIN CAPITAL LETTER N WITH GRAVE
    case 0x01fa: r=0x01fb; break; // LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE
    case 0x01fc: r=0x01fd; break; // LATIN CAPITAL LETTER AE WITH ACUTE
    case 0x01fe: r=0x01ff; break; // LATIN CAPITAL LETTER O WITH STROKE AND ACUTE
    case 0x0200: r=0x0201; break; // LATIN CAPITAL LETTER A WITH DOUBLE GRAVE
    case 0x0202: r=0x0203; break; // LATIN CAPITAL LETTER A WITH INVERTED BREVE
    case 0x0204: r=0x0205; break; // LATIN CAPITAL LETTER E WITH DOUBLE GRAVE
    case 0x0206: r=0x0207; break; // LATIN CAPITAL LETTER E WITH INVERTED BREVE
    case 0x0208: r=0x0209; break; // LATIN CAPITAL LETTER I WITH DOUBLE GRAVE
    case 0x020a: r=0x020b; break; // LATIN CAPITAL LETTER I WITH INVERTED BREVE
    case 0x020c: r=0x020d; break; // LATIN CAPITAL LETTER O WITH DOUBLE GRAVE
    case 0x020e: r=0x020f; break; // LATIN CAPITAL LETTER O WITH INVERTED BREVE
    case 0x0210: r=0x0211; break; // LATIN CAPITAL LETTER R WITH DOUBLE GRAVE
    case 0x0212: r=0x0213; break; // LATIN CAPITAL LETTER R WITH INVERTED BREVE
    case 0x0214: r=0x0215; break; // LATIN CAPITAL LETTER U WITH DOUBLE GRAVE
    case 0x0216: r=0x0217; break; // LATIN CAPITAL LETTER U WITH INVERTED BREVE
    case 0x0218: r=0x0219; break; // LATIN CAPITAL LETTER S WITH COMMA BELOW
    case 0x021a: r=0x021b; break; // LATIN CAPITAL LETTER T WITH COMMA BELOW
    case 0x021c: r=0x021d; break; // LATIN CAPITAL LETTER YOGH
    case 0x021e: r=0x021f; break; // LATIN CAPITAL LETTER H WITH CARON
    case 0x0220: r=0x019e; break; // LATIN CAPITAL LETTER N WITH LONG RIGHT LEG
    case 0x0222: r=0x0223; break; // LATIN CAPITAL LETTER OU
    case 0x0224: r=0x0225; break; // LATIN CAPITAL LETTER Z WITH HOOK
    case 0x0226: r=0x0227; break; // LATIN CAPITAL LETTER A WITH DOT ABOVE
    case 0x0228: r=0x0229; break; // LATIN CAPITAL LETTER E WITH CEDILLA
    case 0x022a: r=0x022b; break; // LATIN CAPITAL LETTER O WITH DIAERESIS AND MACRON
    case 0x022c: r=0x022d; break; // LATIN CAPITAL LETTER O WITH TILDE AND MACRON
    case 0x022e: r=0x022f; break; // LATIN CAPITAL LETTER O WITH DOT ABOVE
    case 0x0230: r=0x0231; break; // LATIN CAPITAL LETTER O WITH DOT ABOVE AND MACRON
    case 0x0232: r=0x0233; break; // LATIN CAPITAL LETTER Y WITH MACRON
    case 0x023b: r=0x023c; break; // LATIN CAPITAL LETTER C WITH STROKE
    case 0x023d: r=0x019a; break; // LATIN CAPITAL LETTER L WITH BAR
    case 0x0241: r=0x0294; break; // LATIN CAPITAL LETTER GLOTTAL STOP
    case 0x0345: r=0x03b9; break; // COMBINING GREEK YPOGEGRAMMENI
    case 0x0386: r=0x03ac; break; // GREEK CAPITAL LETTER ALPHA WITH TONOS
    case 0x0388: r=0x03ad; break; // GREEK CAPITAL LETTER EPSILON WITH TONOS
    case 0x0389: r=0x03ae; break; // GREEK CAPITAL LETTER ETA WITH TONOS
    case 0x038a: r=0x03af; break; // GREEK CAPITAL LETTER IOTA WITH TONOS
    case 0x038c: r=0x03cc; break; // GREEK CAPITAL LETTER OMICRON WITH TONOS
    case 0x038e: r=0x03cd; break; // GREEK CAPITAL LETTER UPSILON WITH TONOS
    case 0x038f: r=0x03ce; break; // GREEK CAPITAL LETTER OMEGA WITH TONOS
    case 0x0391: r=0x03b1; break; // GREEK CAPITAL LETTER ALPHA
    case 0x0392: r=0x03b2; break; // GREEK CAPITAL LETTER BETA
    case 0x0393: r=0x03b3; break; // GREEK CAPITAL LETTER GAMMA
    case 0x0394: r=0x03b4; break; // GREEK CAPITAL LETTER DELTA
    case 0x0395: r=0x03b5; break; // GREEK CAPITAL LETTER EPSILON
    case 0x0396: r=0x03b6; break; // GREEK CAPITAL LETTER ZETA
    case 0x0397: r=0x03b7; break; // GREEK CAPITAL LETTER ETA
    case 0x0398: r=0x03b8; break; // GREEK CAPITAL LETTER THETA
    case 0x0399: r=0x03b9; break; // GREEK CAPITAL LETTER IOTA
    case 0x039a: r=0x03ba; break; // GREEK CAPITAL LETTER KAPPA
    case 0x039b: r=0x03bb; break; // GREEK CAPITAL LETTER LAMDA
    case 0x039c: r=0x03bc; break; // GREEK CAPITAL LETTER MU
    case 0x039d: r=0x03bd; break; // GREEK CAPITAL LETTER NU
    case 0x039e: r=0x03be; break; // GREEK CAPITAL LETTER XI
    case 0x039f: r=0x03bf; break; // GREEK CAPITAL LETTER OMICRON
    case 0x03a0: r=0x03c0; break; // GREEK CAPITAL LETTER PI
    case 0x03a1: r=0x03c1; break; // GREEK CAPITAL LETTER RHO
    case 0x03a3: r=0x03c3; break; // GREEK CAPITAL LETTER SIGMA
    case 0x03a4: r=0x03c4; break; // GREEK CAPITAL LETTER TAU
    case 0x03a5: r=0x03c5; break; // GREEK CAPITAL LETTER UPSILON
    case 0x03a6: r=0x03c6; break; // GREEK CAPITAL LETTER PHI
    case 0x03a7: r=0x03c7; break; // GREEK CAPITAL LETTER CHI
    case 0x03a8: r=0x03c8; break; // GREEK CAPITAL LETTER PSI
    case 0x03a9: r=0x03c9; break; // GREEK CAPITAL LETTER OMEGA
    case 0x03aa: r=0x03ca; break; // GREEK CAPITAL LETTER IOTA WITH DIALYTIKA
    case 0x03ab: r=0x03cb; break; // GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA
    case 0x03c2: r=0x03c3; break; // GREEK SMALL LETTER FINAL SIGMA
    case 0x03d0: r=0x03b2; break; // GREEK BETA SYMBOL
    case 0x03d1: r=0x03b8; break; // GREEK THETA SYMBOL
    case 0x03d5: r=0x03c6; break; // GREEK PHI SYMBOL
    case 0x03d6: r=0x03c0; break; // GREEK PI SYMBOL
    case 0x03d8: r=0x03d9; break; // GREEK LETTER ARCHAIC KOPPA
    case 0x03da: r=0x03db; break; // GREEK LETTER STIGMA
    case 0x03dc: r=0x03dd; break; // GREEK LETTER DIGAMMA
    case 0x03de: r=0x03df; break; // GREEK LETTER KOPPA
    case 0x03e0: r=0x03e1; break; // GREEK LETTER SAMPI
    case 0x03e2: r=0x03e3; break; // COPTIC CAPITAL LETTER SHEI
    case 0x03e4: r=0x03e5; break; // COPTIC CAPITAL LETTER FEI
    case 0x03e6: r=0x03e7; break; // COPTIC CAPITAL LETTER KHEI
    case 0x03e8: r=0x03e9; break; // COPTIC CAPITAL LETTER HORI
    case 0x03ea: r=0x03eb; break; // COPTIC CAPITAL LETTER GANGIA
    case 0x03ec: r=0x03ed; break; // COPTIC CAPITAL LETTER SHIMA
    case 0x03ee: r=0x03ef; break; // COPTIC CAPITAL LETTER DEI
    case 0x03f0: r=0x03ba; break; // GREEK KAPPA SYMBOL
    case 0x03f1: r=0x03c1; break; // GREEK RHO SYMBOL
    case 0x03f4: r=0x03b8; break; // GREEK CAPITAL THETA SYMBOL
    case 0x03f5: r=0x03b5; break; // GREEK LUNATE EPSILON SYMBOL
    case 0x03f7: r=0x03f8; break; // GREEK CAPITAL LETTER SHO
    case 0x03f9: r=0x03f2; break; // GREEK CAPITAL LUNATE SIGMA SYMBOL
    case 0x03fa: r=0x03fb; break; // GREEK CAPITAL LETTER SAN
    case 0x0400: r=0x0450; break; // CYRILLIC CAPITAL LETTER IE WITH GRAVE
    case 0x0401: r=0x0451; break; // CYRILLIC CAPITAL LETTER IO
    case 0x0402: r=0x0452; break; // CYRILLIC CAPITAL LETTER DJE
    case 0x0403: r=0x0453; break; // CYRILLIC CAPITAL LETTER GJE
    case 0x0404: r=0x0454; break; // CYRILLIC CAPITAL LETTER UKRAINIAN IE
    case 0x0405: r=0x0455; break; // CYRILLIC CAPITAL LETTER DZE
    case 0x0406: r=0x0456; break; // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
    case 0x0407: r=0x0457; break; // CYRILLIC CAPITAL LETTER YI
    case 0x0408: r=0x0458; break; // CYRILLIC CAPITAL LETTER JE
    case 0x0409: r=0x0459; break; // CYRILLIC CAPITAL LETTER LJE
    case 0x040a: r=0x045a; break; // CYRILLIC CAPITAL LETTER NJE
    case 0x040b: r=0x045b; break; // CYRILLIC CAPITAL LETTER TSHE
    case 0x040c: r=0x045c; break; // CYRILLIC CAPITAL LETTER KJE
    case 0x040d: r=0x045d; break; // CYRILLIC CAPITAL LETTER I WITH GRAVE
    case 0x040e: r=0x045e; break; // CYRILLIC CAPITAL LETTER SHORT U
    case 0x040f: r=0x045f; break; // CYRILLIC CAPITAL LETTER DZHE
    case 0x0410: r=0x0430; break; // CYRILLIC CAPITAL LETTER A
    case 0x0411: r=0x0431; break; // CYRILLIC CAPITAL LETTER BE
    case 0x0412: r=0x0432; break; // CYRILLIC CAPITAL LETTER VE
    case 0x0413: r=0x0433; break; // CYRILLIC CAPITAL LETTER GHE
    case 0x0414: r=0x0434; break; // CYRILLIC CAPITAL LETTER DE
    case 0x0415: r=0x0435; break; // CYRILLIC CAPITAL LETTER IE
    case 0x0416: r=0x0436; break; // CYRILLIC CAPITAL LETTER ZHE
    case 0x0417: r=0x0437; break; // CYRILLIC CAPITAL LETTER ZE
    case 0x0418: r=0x0438; break; // CYRILLIC CAPITAL LETTER I
    case 0x0419: r=0x0439; break; // CYRILLIC CAPITAL LETTER SHORT I
    case 0x041a: r=0x043a; break; // CYRILLIC CAPITAL LETTER KA
    case 0x041b: r=0x043b; break; // CYRILLIC CAPITAL LETTER EL
    case 0x041c: r=0x043c; break; // CYRILLIC CAPITAL LETTER EM
    case 0x041d: r=0x043d; break; // CYRILLIC CAPITAL LETTER EN
    case 0x041e: r=0x043e; break; // CYRILLIC CAPITAL LETTER O
    case 0x041f: r=0x043f; break; // CYRILLIC CAPITAL LETTER PE
    case 0x0420: r=0x0440; break; // CYRILLIC CAPITAL LETTER ER
    case 0x0421: r=0x0441; break; // CYRILLIC CAPITAL LETTER ES
    case 0x0422: r=0x0442; break; // CYRILLIC CAPITAL LETTER TE
    case 0x0423: r=0x0443; break; // CYRILLIC CAPITAL LETTER U
    case 0x0424: r=0x0444; break; // CYRILLIC CAPITAL LETTER EF
    case 0x0425: r=0x0445; break; // CYRILLIC CAPITAL LETTER HA
    case 0x0426: r=0x0446; break; // CYRILLIC CAPITAL LETTER TSE
    case 0x0427: r=0x0447; break; // CYRILLIC CAPITAL LETTER CHE
    case 0x0428: r=0x0448; break; // CYRILLIC CAPITAL LETTER SHA
    case 0x0429: r=0x0449; break; // CYRILLIC CAPITAL LETTER SHCHA
    case 0x042a: r=0x044a; break; // CYRILLIC CAPITAL LETTER HARD SIGN
    case 0x042b: r=0x044b; break; // CYRILLIC CAPITAL LETTER YERU
    case 0x042c: r=0x044c; break; // CYRILLIC CAPITAL LETTER SOFT SIGN
    case 0x042d: r=0x044d; break; // CYRILLIC CAPITAL LETTER E
    case 0x042e: r=0x044e; break; // CYRILLIC CAPITAL LETTER YU
    case 0x042f: r=0x044f; break; // CYRILLIC CAPITAL LETTER YA
    case 0x0460: r=0x0461; break; // CYRILLIC CAPITAL LETTER OMEGA
    case 0x0462: r=0x0463; break; // CYRILLIC CAPITAL LETTER YAT
    case 0x0464: r=0x0465; break; // CYRILLIC CAPITAL LETTER IOTIFIED E
    case 0x0466: r=0x0467; break; // CYRILLIC CAPITAL LETTER LITTLE YUS
    case 0x0468: r=0x0469; break; // CYRILLIC CAPITAL LETTER IOTIFIED LITTLE YUS
    case 0x046a: r=0x046b; break; // CYRILLIC CAPITAL LETTER BIG YUS
    case 0x046c: r=0x046d; break; // CYRILLIC CAPITAL LETTER IOTIFIED BIG YUS
    case 0x046e: r=0x046f; break; // CYRILLIC CAPITAL LETTER KSI
    case 0x0470: r=0x0471; break; // CYRILLIC CAPITAL LETTER PSI
    case 0x0472: r=0x0473; break; // CYRILLIC CAPITAL LETTER FITA
    case 0x0474: r=0x0475; break; // CYRILLIC CAPITAL LETTER IZHITSA
    case 0x0476: r=0x0477; break; // CYRILLIC CAPITAL LETTER IZHITSA WITH DOUBLE GRAVE ACCENT
    case 0x0478: r=0x0479; break; // CYRILLIC CAPITAL LETTER UK
    case 0x047a: r=0x047b; break; // CYRILLIC CAPITAL LETTER ROUND OMEGA
    case 0x047c: r=0x047d; break; // CYRILLIC CAPITAL LETTER OMEGA WITH TITLO
    case 0x047e: r=0x047f; break; // CYRILLIC CAPITAL LETTER OT
    case 0x0480: r=0x0481; break; // CYRILLIC CAPITAL LETTER KOPPA
    case 0x048a: r=0x048b; break; // CYRILLIC CAPITAL LETTER SHORT I WITH TAIL
    case 0x048c: r=0x048d; break; // CYRILLIC CAPITAL LETTER SEMISOFT SIGN
    case 0x048e: r=0x048f; break; // CYRILLIC CAPITAL LETTER ER WITH TICK
    case 0x0490: r=0x0491; break; // CYRILLIC CAPITAL LETTER GHE WITH UPTURN
    case 0x0492: r=0x0493; break; // CYRILLIC CAPITAL LETTER GHE WITH STROKE
    case 0x0494: r=0x0495; break; // CYRILLIC CAPITAL LETTER GHE WITH MIDDLE HOOK
    case 0x0496: r=0x0497; break; // CYRILLIC CAPITAL LETTER ZHE WITH DESCENDER
    case 0x0498: r=0x0499; break; // CYRILLIC CAPITAL LETTER ZE WITH DESCENDER
    case 0x049a: r=0x049b; break; // CYRILLIC CAPITAL LETTER KA WITH DESCENDER
    case 0x049c: r=0x049d; break; // CYRILLIC CAPITAL LETTER KA WITH VERTICAL STROKE
    case 0x049e: r=0x049f; break; // CYRILLIC CAPITAL LETTER KA WITH STROKE
    case 0x04a0: r=0x04a1; break; // CYRILLIC CAPITAL LETTER BASHKIR KA
    case 0x04a2: r=0x04a3; break; // CYRILLIC CAPITAL LETTER EN WITH DESCENDER
    case 0x04a4: r=0x04a5; break; // CYRILLIC CAPITAL LIGATURE EN GHE
    case 0x04a6: r=0x04a7; break; // CYRILLIC CAPITAL LETTER PE WITH MIDDLE HOOK
    case 0x04a8: r=0x04a9; break; // CYRILLIC CAPITAL LETTER ABKHASIAN HA
    case 0x04aa: r=0x04ab; break; // CYRILLIC CAPITAL LETTER ES WITH DESCENDER
    case 0x04ac: r=0x04ad; break; // CYRILLIC CAPITAL LETTER TE WITH DESCENDER
    case 0x04ae: r=0x04af; break; // CYRILLIC CAPITAL LETTER STRAIGHT U
    case 0x04b0: r=0x04b1; break; // CYRILLIC CAPITAL LETTER STRAIGHT U WITH STROKE
    case 0x04b2: r=0x04b3; break; // CYRILLIC CAPITAL LETTER HA WITH DESCENDER
    case 0x04b4: r=0x04b5; break; // CYRILLIC CAPITAL LIGATURE TE TSE
    case 0x04b6: r=0x04b7; break; // CYRILLIC CAPITAL LETTER CHE WITH DESCENDER
    case 0x04b8: r=0x04b9; break; // CYRILLIC CAPITAL LETTER CHE WITH VERTICAL STROKE
    case 0x04ba: r=0x04bb; break; // CYRILLIC CAPITAL LETTER SHHA
    case 0x04bc: r=0x04bd; break; // CYRILLIC CAPITAL LETTER ABKHASIAN CHE
    case 0x04be: r=0x04bf; break; // CYRILLIC CAPITAL LETTER ABKHASIAN CHE WITH DESCENDER
    case 0x04c1: r=0x04c2; break; // CYRILLIC CAPITAL LETTER ZHE WITH BREVE
    case 0x04c3: r=0x04c4; break; // CYRILLIC CAPITAL LETTER KA WITH HOOK
    case 0x04c5: r=0x04c6; break; // CYRILLIC CAPITAL LETTER EL WITH TAIL
    case 0x04c7: r=0x04c8; break; // CYRILLIC CAPITAL LETTER EN WITH HOOK
    case 0x04c9: r=0x04ca; break; // CYRILLIC CAPITAL LETTER EN WITH TAIL
    case 0x04cb: r=0x04cc; break; // CYRILLIC CAPITAL LETTER KHAKASSIAN CHE
    case 0x04cd: r=0x04ce; break; // CYRILLIC CAPITAL LETTER EM WITH TAIL
    case 0x04d0: r=0x04d1; break; // CYRILLIC CAPITAL LETTER A WITH BREVE
    case 0x04d2: r=0x04d3; break; // CYRILLIC CAPITAL LETTER A WITH DIAERESIS
    case 0x04d4: r=0x04d5; break; // CYRILLIC CAPITAL LIGATURE A IE
    case 0x04d6: r=0x04d7; break; // CYRILLIC CAPITAL LETTER IE WITH BREVE
    case 0x04d8: r=0x04d9; break; // CYRILLIC CAPITAL LETTER SCHWA
    case 0x04da: r=0x04db; break; // CYRILLIC CAPITAL LETTER SCHWA WITH DIAERESIS
    case 0x04dc: r=0x04dd; break; // CYRILLIC CAPITAL LETTER ZHE WITH DIAERESIS
    case 0x04de: r=0x04df; break; // CYRILLIC CAPITAL LETTER ZE WITH DIAERESIS
    case 0x04e0: r=0x04e1; break; // CYRILLIC CAPITAL LETTER ABKHASIAN DZE
    case 0x04e2: r=0x04e3; break; // CYRILLIC CAPITAL LETTER I WITH MACRON
    case 0x04e4: r=0x04e5; break; // CYRILLIC CAPITAL LETTER I WITH DIAERESIS
    case 0x04e6: r=0x04e7; break; // CYRILLIC CAPITAL LETTER O WITH DIAERESIS
    case 0x04e8: r=0x04e9; break; // CYRILLIC CAPITAL LETTER BARRED O
    case 0x04ea: r=0x04eb; break; // CYRILLIC CAPITAL LETTER BARRED O WITH DIAERESIS
    case 0x04ec: r=0x04ed; break; // CYRILLIC CAPITAL LETTER E WITH DIAERESIS
    case 0x04ee: r=0x04ef; break; // CYRILLIC CAPITAL LETTER U WITH MACRON
    case 0x04f0: r=0x04f1; break; // CYRILLIC CAPITAL LETTER U WITH DIAERESIS
    case 0x04f2: r=0x04f3; break; // CYRILLIC CAPITAL LETTER U WITH DOUBLE ACUTE
    case 0x04f4: r=0x04f5; break; // CYRILLIC CAPITAL LETTER CHE WITH DIAERESIS
    case 0x04f6: r=0x04f7; break; // CYRILLIC CAPITAL LETTER GHE WITH DESCENDER
    case 0x04f8: r=0x04f9; break; // CYRILLIC CAPITAL LETTER YERU WITH DIAERESIS
    case 0x0500: r=0x0501; break; // CYRILLIC CAPITAL LETTER KOMI DE
    case 0x0502: r=0x0503; break; // CYRILLIC CAPITAL LETTER KOMI DJE
    case 0x0504: r=0x0505; break; // CYRILLIC CAPITAL LETTER KOMI ZJE
    case 0x0506: r=0x0507; break; // CYRILLIC CAPITAL LETTER KOMI DZJE
    case 0x0508: r=0x0509; break; // CYRILLIC CAPITAL LETTER KOMI LJE
    case 0x050a: r=0x050b; break; // CYRILLIC CAPITAL LETTER KOMI NJE
    case 0x050c: r=0x050d; break; // CYRILLIC CAPITAL LETTER KOMI SJE
    case 0x050e: r=0x050f; break; // CYRILLIC CAPITAL LETTER KOMI TJE
    case 0x0531: r=0x0561; break; // ARMENIAN CAPITAL LETTER AYB
    case 0x0532: r=0x0562; break; // ARMENIAN CAPITAL LETTER BEN
    case 0x0533: r=0x0563; break; // ARMENIAN CAPITAL LETTER GIM
    case 0x0534: r=0x0564; break; // ARMENIAN CAPITAL LETTER DA
    case 0x0535: r=0x0565; break; // ARMENIAN CAPITAL LETTER ECH
    case 0x0536: r=0x0566; break; // ARMENIAN CAPITAL LETTER ZA
    case 0x0537: r=0x0567; break; // ARMENIAN CAPITAL LETTER EH
    case 0x0538: r=0x0568; break; // ARMENIAN CAPITAL LETTER ET
    case 0x0539: r=0x0569; break; // ARMENIAN CAPITAL LETTER TO
    case 0x053a: r=0x056a; break; // ARMENIAN CAPITAL LETTER ZHE
    case 0x053b: r=0x056b; break; // ARMENIAN CAPITAL LETTER INI
    case 0x053c: r=0x056c; break; // ARMENIAN CAPITAL LETTER LIWN
    case 0x053d: r=0x056d; break; // ARMENIAN CAPITAL LETTER XEH
    case 0x053e: r=0x056e; break; // ARMENIAN CAPITAL LETTER CA
    case 0x053f: r=0x056f; break; // ARMENIAN CAPITAL LETTER KEN
    case 0x0540: r=0x0570; break; // ARMENIAN CAPITAL LETTER HO
    case 0x0541: r=0x0571; break; // ARMENIAN CAPITAL LETTER JA
    case 0x0542: r=0x0572; break; // ARMENIAN CAPITAL LETTER GHAD
    case 0x0543: r=0x0573; break; // ARMENIAN CAPITAL LETTER CHEH
    case 0x0544: r=0x0574; break; // ARMENIAN CAPITAL LETTER MEN
    case 0x0545: r=0x0575; break; // ARMENIAN CAPITAL LETTER YI
    case 0x0546: r=0x0576; break; // ARMENIAN CAPITAL LETTER NOW
    case 0x0547: r=0x0577; break; // ARMENIAN CAPITAL LETTER SHA
    case 0x0548: r=0x0578; break; // ARMENIAN CAPITAL LETTER VO
    case 0x0549: r=0x0579; break; // ARMENIAN CAPITAL LETTER CHA
    case 0x054a: r=0x057a; break; // ARMENIAN CAPITAL LETTER PEH
    case 0x054b: r=0x057b; break; // ARMENIAN CAPITAL LETTER JHEH
    case 0x054c: r=0x057c; break; // ARMENIAN CAPITAL LETTER RA
    case 0x054d: r=0x057d; break; // ARMENIAN CAPITAL LETTER SEH
    case 0x054e: r=0x057e; break; // ARMENIAN CAPITAL LETTER VEW
    case 0x054f: r=0x057f; break; // ARMENIAN CAPITAL LETTER TIWN
    case 0x0550: r=0x0580; break; // ARMENIAN CAPITAL LETTER REH
    case 0x0551: r=0x0581; break; // ARMENIAN CAPITAL LETTER CO
    case 0x0552: r=0x0582; break; // ARMENIAN CAPITAL LETTER YIWN
    case 0x0553: r=0x0583; break; // ARMENIAN CAPITAL LETTER PIWR
    case 0x0554: r=0x0584; break; // ARMENIAN CAPITAL LETTER KEH
    case 0x0555: r=0x0585; break; // ARMENIAN CAPITAL LETTER OH
    case 0x0556: r=0x0586; break; // ARMENIAN CAPITAL LETTER FEH
    case 0x10a0: r=0x2d00; break; // GEORGIAN CAPITAL LETTER AN
    case 0x10a1: r=0x2d01; break; // GEORGIAN CAPITAL LETTER BAN
    case 0x10a2: r=0x2d02; break; // GEORGIAN CAPITAL LETTER GAN
    case 0x10a3: r=0x2d03; break; // GEORGIAN CAPITAL LETTER DON
    case 0x10a4: r=0x2d04; break; // GEORGIAN CAPITAL LETTER EN
    case 0x10a5: r=0x2d05; break; // GEORGIAN CAPITAL LETTER VIN
    case 0x10a6: r=0x2d06; break; // GEORGIAN CAPITAL LETTER ZEN
    case 0x10a7: r=0x2d07; break; // GEORGIAN CAPITAL LETTER TAN
    case 0x10a8: r=0x2d08; break; // GEORGIAN CAPITAL LETTER IN
    case 0x10a9: r=0x2d09; break; // GEORGIAN CAPITAL LETTER KAN
    case 0x10aa: r=0x2d0a; break; // GEORGIAN CAPITAL LETTER LAS
    case 0x10ab: r=0x2d0b; break; // GEORGIAN CAPITAL LETTER MAN
    case 0x10ac: r=0x2d0c; break; // GEORGIAN CAPITAL LETTER NAR
    case 0x10ad: r=0x2d0d; break; // GEORGIAN CAPITAL LETTER ON
    case 0x10ae: r=0x2d0e; break; // GEORGIAN CAPITAL LETTER PAR
    case 0x10af: r=0x2d0f; break; // GEORGIAN CAPITAL LETTER ZHAR
    case 0x10b0: r=0x2d10; break; // GEORGIAN CAPITAL LETTER RAE
    case 0x10b1: r=0x2d11; break; // GEORGIAN CAPITAL LETTER SAN
    case 0x10b2: r=0x2d12; break; // GEORGIAN CAPITAL LETTER TAR
    case 0x10b3: r=0x2d13; break; // GEORGIAN CAPITAL LETTER UN
    case 0x10b4: r=0x2d14; break; // GEORGIAN CAPITAL LETTER PHAR
    case 0x10b5: r=0x2d15; break; // GEORGIAN CAPITAL LETTER KHAR
    case 0x10b6: r=0x2d16; break; // GEORGIAN CAPITAL LETTER GHAN
    case 0x10b7: r=0x2d17; break; // GEORGIAN CAPITAL LETTER QAR
    case 0x10b8: r=0x2d18; break; // GEORGIAN CAPITAL LETTER SHIN
    case 0x10b9: r=0x2d19; break; // GEORGIAN CAPITAL LETTER CHIN
    case 0x10ba: r=0x2d1a; break; // GEORGIAN CAPITAL LETTER CAN
    case 0x10bb: r=0x2d1b; break; // GEORGIAN CAPITAL LETTER JIL
    case 0x10bc: r=0x2d1c; break; // GEORGIAN CAPITAL LETTER CIL
    case 0x10bd: r=0x2d1d; break; // GEORGIAN CAPITAL LETTER CHAR
    case 0x10be: r=0x2d1e; break; // GEORGIAN CAPITAL LETTER XAN
    case 0x10bf: r=0x2d1f; break; // GEORGIAN CAPITAL LETTER JHAN
    case 0x10c0: r=0x2d20; break; // GEORGIAN CAPITAL LETTER HAE
    case 0x10c1: r=0x2d21; break; // GEORGIAN CAPITAL LETTER HE
    case 0x10c2: r=0x2d22; break; // GEORGIAN CAPITAL LETTER HIE
    case 0x10c3: r=0x2d23; break; // GEORGIAN CAPITAL LETTER WE
    case 0x10c4: r=0x2d24; break; // GEORGIAN CAPITAL LETTER HAR
    case 0x10c5: r=0x2d25; break; // GEORGIAN CAPITAL LETTER HOE
    case 0x1e00: r=0x1e01; break; // LATIN CAPITAL LETTER A WITH RING BELOW
    case 0x1e02: r=0x1e03; break; // LATIN CAPITAL LETTER B WITH DOT ABOVE
    case 0x1e04: r=0x1e05; break; // LATIN CAPITAL LETTER B WITH DOT BELOW
    case 0x1e06: r=0x1e07; break; // LATIN CAPITAL LETTER B WITH LINE BELOW
    case 0x1e08: r=0x1e09; break; // LATIN CAPITAL LETTER C WITH CEDILLA AND ACUTE
    case 0x1e0a: r=0x1e0b; break; // LATIN CAPITAL LETTER D WITH DOT ABOVE
    case 0x1e0c: r=0x1e0d; break; // LATIN CAPITAL LETTER D WITH DOT BELOW
    case 0x1e0e: r=0x1e0f; break; // LATIN CAPITAL LETTER D WITH LINE BELOW
    case 0x1e10: r=0x1e11; break; // LATIN CAPITAL LETTER D WITH CEDILLA
    case 0x1e12: r=0x1e13; break; // LATIN CAPITAL LETTER D WITH CIRCUMFLEX BELOW
    case 0x1e14: r=0x1e15; break; // LATIN CAPITAL LETTER E WITH MACRON AND GRAVE
    case 0x1e16: r=0x1e17; break; // LATIN CAPITAL LETTER E WITH MACRON AND ACUTE
    case 0x1e18: r=0x1e19; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX BELOW
    case 0x1e1a: r=0x1e1b; break; // LATIN CAPITAL LETTER E WITH TILDE BELOW
    case 0x1e1c: r=0x1e1d; break; // LATIN CAPITAL LETTER E WITH CEDILLA AND BREVE
    case 0x1e1e: r=0x1e1f; break; // LATIN CAPITAL LETTER F WITH DOT ABOVE
    case 0x1e20: r=0x1e21; break; // LATIN CAPITAL LETTER G WITH MACRON
    case 0x1e22: r=0x1e23; break; // LATIN CAPITAL LETTER H WITH DOT ABOVE
    case 0x1e24: r=0x1e25; break; // LATIN CAPITAL LETTER H WITH DOT BELOW
    case 0x1e26: r=0x1e27; break; // LATIN CAPITAL LETTER H WITH DIAERESIS
    case 0x1e28: r=0x1e29; break; // LATIN CAPITAL LETTER H WITH CEDILLA
    case 0x1e2a: r=0x1e2b; break; // LATIN CAPITAL LETTER H WITH BREVE BELOW
    case 0x1e2c: r=0x1e2d; break; // LATIN CAPITAL LETTER I WITH TILDE BELOW
    case 0x1e2e: r=0x1e2f; break; // LATIN CAPITAL LETTER I WITH DIAERESIS AND ACUTE
    case 0x1e30: r=0x1e31; break; // LATIN CAPITAL LETTER K WITH ACUTE
    case 0x1e32: r=0x1e33; break; // LATIN CAPITAL LETTER K WITH DOT BELOW
    case 0x1e34: r=0x1e35; break; // LATIN CAPITAL LETTER K WITH LINE BELOW
    case 0x1e36: r=0x1e37; break; // LATIN CAPITAL LETTER L WITH DOT BELOW
    case 0x1e38: r=0x1e39; break; // LATIN CAPITAL LETTER L WITH DOT BELOW AND MACRON
    case 0x1e3a: r=0x1e3b; break; // LATIN CAPITAL LETTER L WITH LINE BELOW
    case 0x1e3c: r=0x1e3d; break; // LATIN CAPITAL LETTER L WITH CIRCUMFLEX BELOW
    case 0x1e3e: r=0x1e3f; break; // LATIN CAPITAL LETTER M WITH ACUTE
    case 0x1e40: r=0x1e41; break; // LATIN CAPITAL LETTER M WITH DOT ABOVE
    case 0x1e42: r=0x1e43; break; // LATIN CAPITAL LETTER M WITH DOT BELOW
    case 0x1e44: r=0x1e45; break; // LATIN CAPITAL LETTER N WITH DOT ABOVE
    case 0x1e46: r=0x1e47; break; // LATIN CAPITAL LETTER N WITH DOT BELOW
    case 0x1e48: r=0x1e49; break; // LATIN CAPITAL LETTER N WITH LINE BELOW
    case 0x1e4a: r=0x1e4b; break; // LATIN CAPITAL LETTER N WITH CIRCUMFLEX BELOW
    case 0x1e4c: r=0x1e4d; break; // LATIN CAPITAL LETTER O WITH TILDE AND ACUTE
    case 0x1e4e: r=0x1e4f; break; // LATIN CAPITAL LETTER O WITH TILDE AND DIAERESIS
    case 0x1e50: r=0x1e51; break; // LATIN CAPITAL LETTER O WITH MACRON AND GRAVE
    case 0x1e52: r=0x1e53; break; // LATIN CAPITAL LETTER O WITH MACRON AND ACUTE
    case 0x1e54: r=0x1e55; break; // LATIN CAPITAL LETTER P WITH ACUTE
    case 0x1e56: r=0x1e57; break; // LATIN CAPITAL LETTER P WITH DOT ABOVE
    case 0x1e58: r=0x1e59; break; // LATIN CAPITAL LETTER R WITH DOT ABOVE
    case 0x1e5a: r=0x1e5b; break; // LATIN CAPITAL LETTER R WITH DOT BELOW
    case 0x1e5c: r=0x1e5d; break; // LATIN CAPITAL LETTER R WITH DOT BELOW AND MACRON
    case 0x1e5e: r=0x1e5f; break; // LATIN CAPITAL LETTER R WITH LINE BELOW
    case 0x1e60: r=0x1e61; break; // LATIN CAPITAL LETTER S WITH DOT ABOVE
    case 0x1e62: r=0x1e63; break; // LATIN CAPITAL LETTER S WITH DOT BELOW
    case 0x1e64: r=0x1e65; break; // LATIN CAPITAL LETTER S WITH ACUTE AND DOT ABOVE
    case 0x1e66: r=0x1e67; break; // LATIN CAPITAL LETTER S WITH CARON AND DOT ABOVE
    case 0x1e68: r=0x1e69; break; // LATIN CAPITAL LETTER S WITH DOT BELOW AND DOT ABOVE
    case 0x1e6a: r=0x1e6b; break; // LATIN CAPITAL LETTER T WITH DOT ABOVE
    case 0x1e6c: r=0x1e6d; break; // LATIN CAPITAL LETTER T WITH DOT BELOW
    case 0x1e6e: r=0x1e6f; break; // LATIN CAPITAL LETTER T WITH LINE BELOW
    case 0x1e70: r=0x1e71; break; // LATIN CAPITAL LETTER T WITH CIRCUMFLEX BELOW
    case 0x1e72: r=0x1e73; break; // LATIN CAPITAL LETTER U WITH DIAERESIS BELOW
    case 0x1e74: r=0x1e75; break; // LATIN CAPITAL LETTER U WITH TILDE BELOW
    case 0x1e76: r=0x1e77; break; // LATIN CAPITAL LETTER U WITH CIRCUMFLEX BELOW
    case 0x1e78: r=0x1e79; break; // LATIN CAPITAL LETTER U WITH TILDE AND ACUTE
    case 0x1e7a: r=0x1e7b; break; // LATIN CAPITAL LETTER U WITH MACRON AND DIAERESIS
    case 0x1e7c: r=0x1e7d; break; // LATIN CAPITAL LETTER V WITH TILDE
    case 0x1e7e: r=0x1e7f; break; // LATIN CAPITAL LETTER V WITH DOT BELOW
    case 0x1e80: r=0x1e81; break; // LATIN CAPITAL LETTER W WITH GRAVE
    case 0x1e82: r=0x1e83; break; // LATIN CAPITAL LETTER W WITH ACUTE
    case 0x1e84: r=0x1e85; break; // LATIN CAPITAL LETTER W WITH DIAERESIS
    case 0x1e86: r=0x1e87; break; // LATIN CAPITAL LETTER W WITH DOT ABOVE
    case 0x1e88: r=0x1e89; break; // LATIN CAPITAL LETTER W WITH DOT BELOW
    case 0x1e8a: r=0x1e8b; break; // LATIN CAPITAL LETTER X WITH DOT ABOVE
    case 0x1e8c: r=0x1e8d; break; // LATIN CAPITAL LETTER X WITH DIAERESIS
    case 0x1e8e: r=0x1e8f; break; // LATIN CAPITAL LETTER Y WITH DOT ABOVE
    case 0x1e90: r=0x1e91; break; // LATIN CAPITAL LETTER Z WITH CIRCUMFLEX
    case 0x1e92: r=0x1e93; break; // LATIN CAPITAL LETTER Z WITH DOT BELOW
    case 0x1e94: r=0x1e95; break; // LATIN CAPITAL LETTER Z WITH LINE BELOW
    case 0x1e9b: r=0x1e61; break; // LATIN SMALL LETTER LONG S WITH DOT ABOVE
    case 0x1ea0: r=0x1ea1; break; // LATIN CAPITAL LETTER A WITH DOT BELOW
    case 0x1ea2: r=0x1ea3; break; // LATIN CAPITAL LETTER A WITH HOOK ABOVE
    case 0x1ea4: r=0x1ea5; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND ACUTE
    case 0x1ea6: r=0x1ea7; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND GRAVE
    case 0x1ea8: r=0x1ea9; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND HOOK ABOVE
    case 0x1eaa: r=0x1eab; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND TILDE
    case 0x1eac: r=0x1ead; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND DOT BELOW
    case 0x1eae: r=0x1eaf; break; // LATIN CAPITAL LETTER A WITH BREVE AND ACUTE
    case 0x1eb0: r=0x1eb1; break; // LATIN CAPITAL LETTER A WITH BREVE AND GRAVE
    case 0x1eb2: r=0x1eb3; break; // LATIN CAPITAL LETTER A WITH BREVE AND HOOK ABOVE
    case 0x1eb4: r=0x1eb5; break; // LATIN CAPITAL LETTER A WITH BREVE AND TILDE
    case 0x1eb6: r=0x1eb7; break; // LATIN CAPITAL LETTER A WITH BREVE AND DOT BELOW
    case 0x1eb8: r=0x1eb9; break; // LATIN CAPITAL LETTER E WITH DOT BELOW
    case 0x1eba: r=0x1ebb; break; // LATIN CAPITAL LETTER E WITH HOOK ABOVE
    case 0x1ebc: r=0x1ebd; break; // LATIN CAPITAL LETTER E WITH TILDE
    case 0x1ebe: r=0x1ebf; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND ACUTE
    case 0x1ec0: r=0x1ec1; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND GRAVE
    case 0x1ec2: r=0x1ec3; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND HOOK ABOVE
    case 0x1ec4: r=0x1ec5; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND TILDE
    case 0x1ec6: r=0x1ec7; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND DOT BELOW
    case 0x1ec8: r=0x1ec9; break; // LATIN CAPITAL LETTER I WITH HOOK ABOVE
    case 0x1eca: r=0x1ecb; break; // LATIN CAPITAL LETTER I WITH DOT BELOW
    case 0x1ecc: r=0x1ecd; break; // LATIN CAPITAL LETTER O WITH DOT BELOW
    case 0x1ece: r=0x1ecf; break; // LATIN CAPITAL LETTER O WITH HOOK ABOVE
    case 0x1ed0: r=0x1ed1; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND ACUTE
    case 0x1ed2: r=0x1ed3; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND GRAVE
    case 0x1ed4: r=0x1ed5; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND HOOK ABOVE
    case 0x1ed6: r=0x1ed7; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND TILDE
    case 0x1ed8: r=0x1ed9; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND DOT BELOW
    case 0x1eda: r=0x1edb; break; // LATIN CAPITAL LETTER O WITH HORN AND ACUTE
    case 0x1edc: r=0x1edd; break; // LATIN CAPITAL LETTER O WITH HORN AND GRAVE
    case 0x1ede: r=0x1edf; break; // LATIN CAPITAL LETTER O WITH HORN AND HOOK ABOVE
    case 0x1ee0: r=0x1ee1; break; // LATIN CAPITAL LETTER O WITH HORN AND TILDE
    case 0x1ee2: r=0x1ee3; break; // LATIN CAPITAL LETTER O WITH HORN AND DOT BELOW
    case 0x1ee4: r=0x1ee5; break; // LATIN CAPITAL LETTER U WITH DOT BELOW
    case 0x1ee6: r=0x1ee7; break; // LATIN CAPITAL LETTER U WITH HOOK ABOVE
    case 0x1ee8: r=0x1ee9; break; // LATIN CAPITAL LETTER U WITH HORN AND ACUTE
    case 0x1eea: r=0x1eeb; break; // LATIN CAPITAL LETTER U WITH HORN AND GRAVE
    case 0x1eec: r=0x1eed; break; // LATIN CAPITAL LETTER U WITH HORN AND HOOK ABOVE
    case 0x1eee: r=0x1eef; break; // LATIN CAPITAL LETTER U WITH HORN AND TILDE
    case 0x1ef0: r=0x1ef1; break; // LATIN CAPITAL LETTER U WITH HORN AND DOT BELOW
    case 0x1ef2: r=0x1ef3; break; // LATIN CAPITAL LETTER Y WITH GRAVE
    case 0x1ef4: r=0x1ef5; break; // LATIN CAPITAL LETTER Y WITH DOT BELOW
    case 0x1ef6: r=0x1ef7; break; // LATIN CAPITAL LETTER Y WITH HOOK ABOVE
    case 0x1ef8: r=0x1ef9; break; // LATIN CAPITAL LETTER Y WITH TILDE
    case 0x1f08: r=0x1f00; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI
    case 0x1f09: r=0x1f01; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA
    case 0x1f0a: r=0x1f02; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND VARIA
    case 0x1f0b: r=0x1f03; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND VARIA
    case 0x1f0c: r=0x1f04; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND OXIA
    case 0x1f0d: r=0x1f05; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND OXIA
    case 0x1f0e: r=0x1f06; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PERISPOMENI
    case 0x1f0f: r=0x1f07; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PERISPOMENI
    case 0x1f18: r=0x1f10; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI
    case 0x1f19: r=0x1f11; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA
    case 0x1f1a: r=0x1f12; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI AND VARIA
    case 0x1f1b: r=0x1f13; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA AND VARIA
    case 0x1f1c: r=0x1f14; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI AND OXIA
    case 0x1f1d: r=0x1f15; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA AND OXIA
    case 0x1f28: r=0x1f20; break; // GREEK CAPITAL LETTER ETA WITH PSILI
    case 0x1f29: r=0x1f21; break; // GREEK CAPITAL LETTER ETA WITH DASIA
    case 0x1f2a: r=0x1f22; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND VARIA
    case 0x1f2b: r=0x1f23; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND VARIA
    case 0x1f2c: r=0x1f24; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND OXIA
    case 0x1f2d: r=0x1f25; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND OXIA
    case 0x1f2e: r=0x1f26; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PERISPOMENI
    case 0x1f2f: r=0x1f27; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PERISPOMENI
    case 0x1f38: r=0x1f30; break; // GREEK CAPITAL LETTER IOTA WITH PSILI
    case 0x1f39: r=0x1f31; break; // GREEK CAPITAL LETTER IOTA WITH DASIA
    case 0x1f3a: r=0x1f32; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND VARIA
    case 0x1f3b: r=0x1f33; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND VARIA
    case 0x1f3c: r=0x1f34; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND OXIA
    case 0x1f3d: r=0x1f35; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND OXIA
    case 0x1f3e: r=0x1f36; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND PERISPOMENI
    case 0x1f3f: r=0x1f37; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND PERISPOMENI
    case 0x1f48: r=0x1f40; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI
    case 0x1f49: r=0x1f41; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA
    case 0x1f4a: r=0x1f42; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI AND VARIA
    case 0x1f4b: r=0x1f43; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA AND VARIA
    case 0x1f4c: r=0x1f44; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI AND OXIA
    case 0x1f4d: r=0x1f45; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA AND OXIA
    case 0x1f59: r=0x1f51; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA
    case 0x1f5b: r=0x1f53; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND VARIA
    case 0x1f5d: r=0x1f55; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND OXIA
    case 0x1f5f: r=0x1f57; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND PERISPOMENI
    case 0x1f68: r=0x1f60; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI
    case 0x1f69: r=0x1f61; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA
    case 0x1f6a: r=0x1f62; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND VARIA
    case 0x1f6b: r=0x1f63; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND VARIA
    case 0x1f6c: r=0x1f64; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND OXIA
    case 0x1f6d: r=0x1f65; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND OXIA
    case 0x1f6e: r=0x1f66; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PERISPOMENI
    case 0x1f6f: r=0x1f67; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PERISPOMENI
    case 0x1f88: r=0x1f80; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PROSGEGRAMMENI
    case 0x1f89: r=0x1f81; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PROSGEGRAMMENI
    case 0x1f8a: r=0x1f82; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND VARIA AND PROSGEGRAMMENI
    case 0x1f8b: r=0x1f83; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND VARIA AND PROSGEGRAMMENI
    case 0x1f8c: r=0x1f84; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND OXIA AND PROSGEGRAMMENI
    case 0x1f8d: r=0x1f85; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND OXIA AND PROSGEGRAMMENI
    case 0x1f8e: r=0x1f86; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1f8f: r=0x1f87; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1f98: r=0x1f90; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PROSGEGRAMMENI
    case 0x1f99: r=0x1f91; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PROSGEGRAMMENI
    case 0x1f9a: r=0x1f92; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND VARIA AND PROSGEGRAMMENI
    case 0x1f9b: r=0x1f93; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND VARIA AND PROSGEGRAMMENI
    case 0x1f9c: r=0x1f94; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND OXIA AND PROSGEGRAMMENI
    case 0x1f9d: r=0x1f95; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND OXIA AND PROSGEGRAMMENI
    case 0x1f9e: r=0x1f96; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1f9f: r=0x1f97; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1fa8: r=0x1fa0; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PROSGEGRAMMENI
    case 0x1fa9: r=0x1fa1; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PROSGEGRAMMENI
    case 0x1faa: r=0x1fa2; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND VARIA AND PROSGEGRAMMENI
    case 0x1fab: r=0x1fa3; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND VARIA AND PROSGEGRAMMENI
    case 0x1fac: r=0x1fa4; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND OXIA AND PROSGEGRAMMENI
    case 0x1fad: r=0x1fa5; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND OXIA AND PROSGEGRAMMENI
    case 0x1fae: r=0x1fa6; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1faf: r=0x1fa7; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI
    case 0x1fb8: r=0x1fb0; break; // GREEK CAPITAL LETTER ALPHA WITH VRACHY
    case 0x1fb9: r=0x1fb1; break; // GREEK CAPITAL LETTER ALPHA WITH MACRON
    case 0x1fba: r=0x1f70; break; // GREEK CAPITAL LETTER ALPHA WITH VARIA
    case 0x1fbb: r=0x1f71; break; // GREEK CAPITAL LETTER ALPHA WITH OXIA
    case 0x1fbc: r=0x1fb3; break; // GREEK CAPITAL LETTER ALPHA WITH PROSGEGRAMMENI
    case 0x1fbe: r=0x03b9; break; // GREEK PROSGEGRAMMENI
    case 0x1fc8: r=0x1f72; break; // GREEK CAPITAL LETTER EPSILON WITH VARIA
    case 0x1fc9: r=0x1f73; break; // GREEK CAPITAL LETTER EPSILON WITH OXIA
    case 0x1fca: r=0x1f74; break; // GREEK CAPITAL LETTER ETA WITH VARIA
    case 0x1fcb: r=0x1f75; break; // GREEK CAPITAL LETTER ETA WITH OXIA
    case 0x1fcc: r=0x1fc3; break; // GREEK CAPITAL LETTER ETA WITH PROSGEGRAMMENI
    case 0x1fd8: r=0x1fd0; break; // GREEK CAPITAL LETTER IOTA WITH VRACHY
    case 0x1fd9: r=0x1fd1; break; // GREEK CAPITAL LETTER IOTA WITH MACRON
    case 0x1fda: r=0x1f76; break; // GREEK CAPITAL LETTER IOTA WITH VARIA
    case 0x1fdb: r=0x1f77; break; // GREEK CAPITAL LETTER IOTA WITH OXIA
    case 0x1fe8: r=0x1fe0; break; // GREEK CAPITAL LETTER UPSILON WITH VRACHY
    case 0x1fe9: r=0x1fe1; break; // GREEK CAPITAL LETTER UPSILON WITH MACRON
    case 0x1fea: r=0x1f7a; break; // GREEK CAPITAL LETTER UPSILON WITH VARIA
    case 0x1feb: r=0x1f7b; break; // GREEK CAPITAL LETTER UPSILON WITH OXIA
    case 0x1fec: r=0x1fe5; break; // GREEK CAPITAL LETTER RHO WITH DASIA
    case 0x1ff8: r=0x1f78; break; // GREEK CAPITAL LETTER OMICRON WITH VARIA
    case 0x1ff9: r=0x1f79; break; // GREEK CAPITAL LETTER OMICRON WITH OXIA
    case 0x1ffa: r=0x1f7c; break; // GREEK CAPITAL LETTER OMEGA WITH VARIA
    case 0x1ffb: r=0x1f7d; break; // GREEK CAPITAL LETTER OMEGA WITH OXIA
    case 0x1ffc: r=0x1ff3; break; // GREEK CAPITAL LETTER OMEGA WITH PROSGEGRAMMENI
    case 0x2126: r=0x03c9; break; // OHM SIGN
    case 0x212a: r=0x006b; break; // KELVIN SIGN
    case 0x212b: r=0x00e5; break; // ANGSTROM SIGN
    case 0x2160: r=0x2170; break; // ROMAN NUMERAL ONE
    case 0x2161: r=0x2171; break; // ROMAN NUMERAL TWO
    case 0x2162: r=0x2172; break; // ROMAN NUMERAL THREE
    case 0x2163: r=0x2173; break; // ROMAN NUMERAL FOUR
    case 0x2164: r=0x2174; break; // ROMAN NUMERAL FIVE
    case 0x2165: r=0x2175; break; // ROMAN NUMERAL SIX
    case 0x2166: r=0x2176; break; // ROMAN NUMERAL SEVEN
    case 0x2167: r=0x2177; break; // ROMAN NUMERAL EIGHT
    case 0x2168: r=0x2178; break; // ROMAN NUMERAL NINE
    case 0x2169: r=0x2179; break; // ROMAN NUMERAL TEN
    case 0x216a: r=0x217a; break; // ROMAN NUMERAL ELEVEN
    case 0x216b: r=0x217b; break; // ROMAN NUMERAL TWELVE
    case 0x216c: r=0x217c; break; // ROMAN NUMERAL FIFTY
    case 0x216d: r=0x217d; break; // ROMAN NUMERAL ONE HUNDRED
    case 0x216e: r=0x217e; break; // ROMAN NUMERAL FIVE HUNDRED
    case 0x216f: r=0x217f; break; // ROMAN NUMERAL ONE THOUSAND
    case 0x24b6: r=0x24d0; break; // CIRCLED LATIN CAPITAL LETTER A
    case 0x24b7: r=0x24d1; break; // CIRCLED LATIN CAPITAL LETTER B
    case 0x24b8: r=0x24d2; break; // CIRCLED LATIN CAPITAL LETTER C
    case 0x24b9: r=0x24d3; break; // CIRCLED LATIN CAPITAL LETTER D
    case 0x24ba: r=0x24d4; break; // CIRCLED LATIN CAPITAL LETTER E
    case 0x24bb: r=0x24d5; break; // CIRCLED LATIN CAPITAL LETTER F
    case 0x24bc: r=0x24d6; break; // CIRCLED LATIN CAPITAL LETTER G
    case 0x24bd: r=0x24d7; break; // CIRCLED LATIN CAPITAL LETTER H
    case 0x24be: r=0x24d8; break; // CIRCLED LATIN CAPITAL LETTER I
    case 0x24bf: r=0x24d9; break; // CIRCLED LATIN CAPITAL LETTER J
    case 0x24c0: r=0x24da; break; // CIRCLED LATIN CAPITAL LETTER K
    case 0x24c1: r=0x24db; break; // CIRCLED LATIN CAPITAL LETTER L
    case 0x24c2: r=0x24dc; break; // CIRCLED LATIN CAPITAL LETTER M
    case 0x24c3: r=0x24dd; break; // CIRCLED LATIN CAPITAL LETTER N
    case 0x24c4: r=0x24de; break; // CIRCLED LATIN CAPITAL LETTER O
    case 0x24c5: r=0x24df; break; // CIRCLED LATIN CAPITAL LETTER P
    case 0x24c6: r=0x24e0; break; // CIRCLED LATIN CAPITAL LETTER Q
    case 0x24c7: r=0x24e1; break; // CIRCLED LATIN CAPITAL LETTER R
    case 0x24c8: r=0x24e2; break; // CIRCLED LATIN CAPITAL LETTER S
    case 0x24c9: r=0x24e3; break; // CIRCLED LATIN CAPITAL LETTER T
    case 0x24ca: r=0x24e4; break; // CIRCLED LATIN CAPITAL LETTER U
    case 0x24cb: r=0x24e5; break; // CIRCLED LATIN CAPITAL LETTER V
    case 0x24cc: r=0x24e6; break; // CIRCLED LATIN CAPITAL LETTER W
    case 0x24cd: r=0x24e7; break; // CIRCLED LATIN CAPITAL LETTER X
    case 0x24ce: r=0x24e8; break; // CIRCLED LATIN CAPITAL LETTER Y
    case 0x24cf: r=0x24e9; break; // CIRCLED LATIN CAPITAL LETTER Z
    case 0x2c00: r=0x2c30; break; // GLAGOLITIC CAPITAL LETTER AZU
    case 0x2c01: r=0x2c31; break; // GLAGOLITIC CAPITAL LETTER BUKY
    case 0x2c02: r=0x2c32; break; // GLAGOLITIC CAPITAL LETTER VEDE
    case 0x2c03: r=0x2c33; break; // GLAGOLITIC CAPITAL LETTER GLAGOLI
    case 0x2c04: r=0x2c34; break; // GLAGOLITIC CAPITAL LETTER DOBRO
    case 0x2c05: r=0x2c35; break; // GLAGOLITIC CAPITAL LETTER YESTU
    case 0x2c06: r=0x2c36; break; // GLAGOLITIC CAPITAL LETTER ZHIVETE
    case 0x2c07: r=0x2c37; break; // GLAGOLITIC CAPITAL LETTER DZELO
    case 0x2c08: r=0x2c38; break; // GLAGOLITIC CAPITAL LETTER ZEMLJA
    case 0x2c09: r=0x2c39; break; // GLAGOLITIC CAPITAL LETTER IZHE
    case 0x2c0a: r=0x2c3a; break; // GLAGOLITIC CAPITAL LETTER INITIAL IZHE
    case 0x2c0b: r=0x2c3b; break; // GLAGOLITIC CAPITAL LETTER I
    case 0x2c0c: r=0x2c3c; break; // GLAGOLITIC CAPITAL LETTER DJERVI
    case 0x2c0d: r=0x2c3d; break; // GLAGOLITIC CAPITAL LETTER KAKO
    case 0x2c0e: r=0x2c3e; break; // GLAGOLITIC CAPITAL LETTER LJUDIJE
    case 0x2c0f: r=0x2c3f; break; // GLAGOLITIC CAPITAL LETTER MYSLITE
    case 0x2c10: r=0x2c40; break; // GLAGOLITIC CAPITAL LETTER NASHI
    case 0x2c11: r=0x2c41; break; // GLAGOLITIC CAPITAL LETTER ONU
    case 0x2c12: r=0x2c42; break; // GLAGOLITIC CAPITAL LETTER POKOJI
    case 0x2c13: r=0x2c43; break; // GLAGOLITIC CAPITAL LETTER RITSI
    case 0x2c14: r=0x2c44; break; // GLAGOLITIC CAPITAL LETTER SLOVO
    case 0x2c15: r=0x2c45; break; // GLAGOLITIC CAPITAL LETTER TVRIDO
    case 0x2c16: r=0x2c46; break; // GLAGOLITIC CAPITAL LETTER UKU
    case 0x2c17: r=0x2c47; break; // GLAGOLITIC CAPITAL LETTER FRITU
    case 0x2c18: r=0x2c48; break; // GLAGOLITIC CAPITAL LETTER HERU
    case 0x2c19: r=0x2c49; break; // GLAGOLITIC CAPITAL LETTER OTU
    case 0x2c1a: r=0x2c4a; break; // GLAGOLITIC CAPITAL LETTER PE
    case 0x2c1b: r=0x2c4b; break; // GLAGOLITIC CAPITAL LETTER SHTA
    case 0x2c1c: r=0x2c4c; break; // GLAGOLITIC CAPITAL LETTER TSI
    case 0x2c1d: r=0x2c4d; break; // GLAGOLITIC CAPITAL LETTER CHRIVI
    case 0x2c1e: r=0x2c4e; break; // GLAGOLITIC CAPITAL LETTER SHA
    case 0x2c1f: r=0x2c4f; break; // GLAGOLITIC CAPITAL LETTER YERU
    case 0x2c20: r=0x2c50; break; // GLAGOLITIC CAPITAL LETTER YERI
    case 0x2c21: r=0x2c51; break; // GLAGOLITIC CAPITAL LETTER YATI
    case 0x2c22: r=0x2c52; break; // GLAGOLITIC CAPITAL LETTER SPIDERY HA
    case 0x2c23: r=0x2c53; break; // GLAGOLITIC CAPITAL LETTER YU
    case 0x2c24: r=0x2c54; break; // GLAGOLITIC CAPITAL LETTER SMALL YUS
    case 0x2c25: r=0x2c55; break; // GLAGOLITIC CAPITAL LETTER SMALL YUS WITH TAIL
    case 0x2c26: r=0x2c56; break; // GLAGOLITIC CAPITAL LETTER YO
    case 0x2c27: r=0x2c57; break; // GLAGOLITIC CAPITAL LETTER IOTATED SMALL YUS
    case 0x2c28: r=0x2c58; break; // GLAGOLITIC CAPITAL LETTER BIG YUS
    case 0x2c29: r=0x2c59; break; // GLAGOLITIC CAPITAL LETTER IOTATED BIG YUS
    case 0x2c2a: r=0x2c5a; break; // GLAGOLITIC CAPITAL LETTER FITA
    case 0x2c2b: r=0x2c5b; break; // GLAGOLITIC CAPITAL LETTER IZHITSA
    case 0x2c2c: r=0x2c5c; break; // GLAGOLITIC CAPITAL LETTER SHTAPIC
    case 0x2c2d: r=0x2c5d; break; // GLAGOLITIC CAPITAL LETTER TROKUTASTI A
    case 0x2c2e: r=0x2c5e; break; // GLAGOLITIC CAPITAL LETTER LATINATE MYSLITE
    case 0x2c80: r=0x2c81; break; // COPTIC CAPITAL LETTER ALFA
    case 0x2c82: r=0x2c83; break; // COPTIC CAPITAL LETTER VIDA
    case 0x2c84: r=0x2c85; break; // COPTIC CAPITAL LETTER GAMMA
    case 0x2c86: r=0x2c87; break; // COPTIC CAPITAL LETTER DALDA
    case 0x2c88: r=0x2c89; break; // COPTIC CAPITAL LETTER EIE
    case 0x2c8a: r=0x2c8b; break; // COPTIC CAPITAL LETTER SOU
    case 0x2c8c: r=0x2c8d; break; // COPTIC CAPITAL LETTER ZATA
    case 0x2c8e: r=0x2c8f; break; // COPTIC CAPITAL LETTER HATE
    case 0x2c90: r=0x2c91; break; // COPTIC CAPITAL LETTER THETHE
    case 0x2c92: r=0x2c93; break; // COPTIC CAPITAL LETTER IAUDA
    case 0x2c94: r=0x2c95; break; // COPTIC CAPITAL LETTER KAPA
    case 0x2c96: r=0x2c97; break; // COPTIC CAPITAL LETTER LAULA
    case 0x2c98: r=0x2c99; break; // COPTIC CAPITAL LETTER MI
    case 0x2c9a: r=0x2c9b; break; // COPTIC CAPITAL LETTER NI
    case 0x2c9c: r=0x2c9d; break; // COPTIC CAPITAL LETTER KSI
    case 0x2c9e: r=0x2c9f; break; // COPTIC CAPITAL LETTER O
    case 0x2ca0: r=0x2ca1; break; // COPTIC CAPITAL LETTER PI
    case 0x2ca2: r=0x2ca3; break; // COPTIC CAPITAL LETTER RO
    case 0x2ca4: r=0x2ca5; break; // COPTIC CAPITAL LETTER SIMA
    case 0x2ca6: r=0x2ca7; break; // COPTIC CAPITAL LETTER TAU
    case 0x2ca8: r=0x2ca9; break; // COPTIC CAPITAL LETTER UA
    case 0x2caa: r=0x2cab; break; // COPTIC CAPITAL LETTER FI
    case 0x2cac: r=0x2cad; break; // COPTIC CAPITAL LETTER KHI
    case 0x2cae: r=0x2caf; break; // COPTIC CAPITAL LETTER PSI
    case 0x2cb0: r=0x2cb1; break; // COPTIC CAPITAL LETTER OOU
    case 0x2cb2: r=0x2cb3; break; // COPTIC CAPITAL LETTER DIALECT-P ALEF
    case 0x2cb4: r=0x2cb5; break; // COPTIC CAPITAL LETTER OLD COPTIC AIN
    case 0x2cb6: r=0x2cb7; break; // COPTIC CAPITAL LETTER CRYPTOGRAMMIC EIE
    case 0x2cb8: r=0x2cb9; break; // COPTIC CAPITAL LETTER DIALECT-P KAPA
    case 0x2cba: r=0x2cbb; break; // COPTIC CAPITAL LETTER DIALECT-P NI
    case 0x2cbc: r=0x2cbd; break; // COPTIC CAPITAL LETTER CRYPTOGRAMMIC NI
    case 0x2cbe: r=0x2cbf; break; // COPTIC CAPITAL LETTER OLD COPTIC OOU
    case 0x2cc0: r=0x2cc1; break; // COPTIC CAPITAL LETTER SAMPI
    case 0x2cc2: r=0x2cc3; break; // COPTIC CAPITAL LETTER CROSSED SHEI
    case 0x2cc4: r=0x2cc5; break; // COPTIC CAPITAL LETTER OLD COPTIC SHEI
    case 0x2cc6: r=0x2cc7; break; // COPTIC CAPITAL LETTER OLD COPTIC ESH
    case 0x2cc8: r=0x2cc9; break; // COPTIC CAPITAL LETTER AKHMIMIC KHEI
    case 0x2cca: r=0x2ccb; break; // COPTIC CAPITAL LETTER DIALECT-P HORI
    case 0x2ccc: r=0x2ccd; break; // COPTIC CAPITAL LETTER OLD COPTIC HORI
    case 0x2cce: r=0x2ccf; break; // COPTIC CAPITAL LETTER OLD COPTIC HA
    case 0x2cd0: r=0x2cd1; break; // COPTIC CAPITAL LETTER L-SHAPED HA
    case 0x2cd2: r=0x2cd3; break; // COPTIC CAPITAL LETTER OLD COPTIC HEI
    case 0x2cd4: r=0x2cd5; break; // COPTIC CAPITAL LETTER OLD COPTIC HAT
    case 0x2cd6: r=0x2cd7; break; // COPTIC CAPITAL LETTER OLD COPTIC GANGIA
    case 0x2cd8: r=0x2cd9; break; // COPTIC CAPITAL LETTER OLD COPTIC DJA
    case 0x2cda: r=0x2cdb; break; // COPTIC CAPITAL LETTER OLD COPTIC SHIMA
    case 0x2cdc: r=0x2cdd; break; // COPTIC CAPITAL LETTER OLD NUBIAN SHIMA
    case 0x2cde: r=0x2cdf; break; // COPTIC CAPITAL LETTER OLD NUBIAN NGI
    case 0x2ce0: r=0x2ce1; break; // COPTIC CAPITAL LETTER OLD NUBIAN NYI
    case 0x2ce2: r=0x2ce3; break; // COPTIC CAPITAL LETTER OLD NUBIAN WAU
      /* end of autogenerated code */
    default: r=c; break;
  }
  return r;
}

/*

Remove accents from a unichar if accented (deaccentuate).
Table is created by the following Perl script (compare above
tolower/toupper):

cat UnicodeDataFull.txt \
    | perl -CDS -aF';' -ne \
    'next unless $F[1]=~/\bLETTER\b/;
     $info{$F[0]}=$F[1];
     undef $l;
     ($l) = ($F[5] =~ /^([0-9a-f]+)/i) if defined $F[5];
     if ((not defined $l) && $F[1]=~/(SMALL|CAPITAL) LETTER ([A-Z]) WITH/) {
       $l=sprintf("%x",ord((($1eq"SMALL")?lc($2):$2)));
     }
     while (exists $tab{$l}) {
       $l=$tab{$l}
     }
     next unless defined $l;
     $tab{$F[0]}=$l;
     print "    case 0x", lc($F[0]), ": r=0x", $l, "; break; // ",
       $F[1], " => ", $info{$l}, "\n";
     print STDERR pack("UU",hex($F[0]), hex($l)), "\n";' \
     > deacc_tab


*/
unichar u_deaccentuate (unichar c) {
  unichar r;
  switch (c) {
      /* begin of autogenerated code */
    case 0x00c0: r=0x0041; break; // LATIN CAPITAL LETTER A WITH GRAVE => LATIN CAPITAL LETTER A
    case 0x00c1: r=0x0041; break; // LATIN CAPITAL LETTER A WITH ACUTE => LATIN CAPITAL LETTER A
    case 0x00c2: r=0x0041; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX => LATIN CAPITAL LETTER A
    case 0x00c3: r=0x0041; break; // LATIN CAPITAL LETTER A WITH TILDE => LATIN CAPITAL LETTER A
    case 0x00c4: r=0x0041; break; // LATIN CAPITAL LETTER A WITH DIAERESIS => LATIN CAPITAL LETTER A
    case 0x00c5: r=0x0041; break; // LATIN CAPITAL LETTER A WITH RING ABOVE => LATIN CAPITAL LETTER A
    case 0x00c7: r=0x0043; break; // LATIN CAPITAL LETTER C WITH CEDILLA => LATIN CAPITAL LETTER C
    case 0x00c8: r=0x0045; break; // LATIN CAPITAL LETTER E WITH GRAVE => LATIN CAPITAL LETTER E
    case 0x00c9: r=0x0045; break; // LATIN CAPITAL LETTER E WITH ACUTE => LATIN CAPITAL LETTER E
    case 0x00ca: r=0x0045; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX => LATIN CAPITAL LETTER E
    case 0x00cb: r=0x0045; break; // LATIN CAPITAL LETTER E WITH DIAERESIS => LATIN CAPITAL LETTER E
    case 0x00cc: r=0x0049; break; // LATIN CAPITAL LETTER I WITH GRAVE => LATIN CAPITAL LETTER I
    case 0x00cd: r=0x0049; break; // LATIN CAPITAL LETTER I WITH ACUTE => LATIN CAPITAL LETTER I
    case 0x00ce: r=0x0049; break; // LATIN CAPITAL LETTER I WITH CIRCUMFLEX => LATIN CAPITAL LETTER I
    case 0x00cf: r=0x0049; break; // LATIN CAPITAL LETTER I WITH DIAERESIS => LATIN CAPITAL LETTER I
    case 0x00d1: r=0x004E; break; // LATIN CAPITAL LETTER N WITH TILDE => LATIN CAPITAL LETTER N
    case 0x00d2: r=0x004F; break; // LATIN CAPITAL LETTER O WITH GRAVE => LATIN CAPITAL LETTER O
    case 0x00d3: r=0x004F; break; // LATIN CAPITAL LETTER O WITH ACUTE => LATIN CAPITAL LETTER O
    case 0x00d4: r=0x004F; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX => LATIN CAPITAL LETTER O
    case 0x00d5: r=0x004F; break; // LATIN CAPITAL LETTER O WITH TILDE => LATIN CAPITAL LETTER O
    case 0x00d6: r=0x004F; break; // LATIN CAPITAL LETTER O WITH DIAERESIS => LATIN CAPITAL LETTER O
    case 0x00d8: r=0x4f; break; // LATIN CAPITAL LETTER O WITH STROKE =>
    case 0x00d9: r=0x0055; break; // LATIN CAPITAL LETTER U WITH GRAVE => LATIN CAPITAL LETTER U
    case 0x00da: r=0x0055; break; // LATIN CAPITAL LETTER U WITH ACUTE => LATIN CAPITAL LETTER U
    case 0x00db: r=0x0055; break; // LATIN CAPITAL LETTER U WITH CIRCUMFLEX => LATIN CAPITAL LETTER U
    case 0x00dc: r=0x0055; break; // LATIN CAPITAL LETTER U WITH DIAERESIS => LATIN CAPITAL LETTER U
    case 0x00dd: r=0x0059; break; // LATIN CAPITAL LETTER Y WITH ACUTE => LATIN CAPITAL LETTER Y
    case 0x00e0: r=0x0061; break; // LATIN SMALL LETTER A WITH GRAVE => LATIN SMALL LETTER A
    case 0x00e1: r=0x0061; break; // LATIN SMALL LETTER A WITH ACUTE => LATIN SMALL LETTER A
    case 0x00e2: r=0x0061; break; // LATIN SMALL LETTER A WITH CIRCUMFLEX => LATIN SMALL LETTER A
    case 0x00e3: r=0x0061; break; // LATIN SMALL LETTER A WITH TILDE => LATIN SMALL LETTER A
    case 0x00e4: r=0x0061; break; // LATIN SMALL LETTER A WITH DIAERESIS => LATIN SMALL LETTER A
    case 0x00e5: r=0x0061; break; // LATIN SMALL LETTER A WITH RING ABOVE => LATIN SMALL LETTER A
    case 0x00e7: r=0x0063; break; // LATIN SMALL LETTER C WITH CEDILLA => LATIN SMALL LETTER C
    case 0x00e8: r=0x0065; break; // LATIN SMALL LETTER E WITH GRAVE => LATIN SMALL LETTER E
    case 0x00e9: r=0x0065; break; // LATIN SMALL LETTER E WITH ACUTE => LATIN SMALL LETTER E
    case 0x00ea: r=0x0065; break; // LATIN SMALL LETTER E WITH CIRCUMFLEX => LATIN SMALL LETTER E
    case 0x00eb: r=0x0065; break; // LATIN SMALL LETTER E WITH DIAERESIS => LATIN SMALL LETTER E
    case 0x00ec: r=0x0069; break; // LATIN SMALL LETTER I WITH GRAVE => LATIN SMALL LETTER I
    case 0x00ed: r=0x0069; break; // LATIN SMALL LETTER I WITH ACUTE => LATIN SMALL LETTER I
    case 0x00ee: r=0x0069; break; // LATIN SMALL LETTER I WITH CIRCUMFLEX => LATIN SMALL LETTER I
    case 0x00ef: r=0x0069; break; // LATIN SMALL LETTER I WITH DIAERESIS => LATIN SMALL LETTER I
    case 0x00f1: r=0x006E; break; // LATIN SMALL LETTER N WITH TILDE => LATIN SMALL LETTER N
    case 0x00f2: r=0x006F; break; // LATIN SMALL LETTER O WITH GRAVE => LATIN SMALL LETTER O
    case 0x00f3: r=0x006F; break; // LATIN SMALL LETTER O WITH ACUTE => LATIN SMALL LETTER O
    case 0x00f4: r=0x006F; break; // LATIN SMALL LETTER O WITH CIRCUMFLEX => LATIN SMALL LETTER O
    case 0x00f5: r=0x006F; break; // LATIN SMALL LETTER O WITH TILDE => LATIN SMALL LETTER O
    case 0x00f6: r=0x006F; break; // LATIN SMALL LETTER O WITH DIAERESIS => LATIN SMALL LETTER O
    case 0x00f8: r=0x6f; break; // LATIN SMALL LETTER O WITH STROKE =>
    case 0x00f9: r=0x0075; break; // LATIN SMALL LETTER U WITH GRAVE => LATIN SMALL LETTER U
    case 0x00fa: r=0x0075; break; // LATIN SMALL LETTER U WITH ACUTE => LATIN SMALL LETTER U
    case 0x00fb: r=0x0075; break; // LATIN SMALL LETTER U WITH CIRCUMFLEX => LATIN SMALL LETTER U
    case 0x00fc: r=0x0075; break; // LATIN SMALL LETTER U WITH DIAERESIS => LATIN SMALL LETTER U
    case 0x00fd: r=0x0079; break; // LATIN SMALL LETTER Y WITH ACUTE => LATIN SMALL LETTER Y
    case 0x00ff: r=0x0079; break; // LATIN SMALL LETTER Y WITH DIAERESIS => LATIN SMALL LETTER Y
    case 0x0100: r=0x0041; break; // LATIN CAPITAL LETTER A WITH MACRON => LATIN CAPITAL LETTER A
    case 0x0101: r=0x0061; break; // LATIN SMALL LETTER A WITH MACRON => LATIN SMALL LETTER A
    case 0x0102: r=0x0041; break; // LATIN CAPITAL LETTER A WITH BREVE => LATIN CAPITAL LETTER A
    case 0x0103: r=0x0061; break; // LATIN SMALL LETTER A WITH BREVE => LATIN SMALL LETTER A
    case 0x0104: r=0x0041; break; // LATIN CAPITAL LETTER A WITH OGONEK => LATIN CAPITAL LETTER A
    case 0x0105: r=0x0061; break; // LATIN SMALL LETTER A WITH OGONEK => LATIN SMALL LETTER A
    case 0x0106: r=0x0043; break; // LATIN CAPITAL LETTER C WITH ACUTE => LATIN CAPITAL LETTER C
    case 0x0107: r=0x0063; break; // LATIN SMALL LETTER C WITH ACUTE => LATIN SMALL LETTER C
    case 0x0108: r=0x0043; break; // LATIN CAPITAL LETTER C WITH CIRCUMFLEX => LATIN CAPITAL LETTER C
    case 0x0109: r=0x0063; break; // LATIN SMALL LETTER C WITH CIRCUMFLEX => LATIN SMALL LETTER C
    case 0x010a: r=0x0043; break; // LATIN CAPITAL LETTER C WITH DOT ABOVE => LATIN CAPITAL LETTER C
    case 0x010b: r=0x0063; break; // LATIN SMALL LETTER C WITH DOT ABOVE => LATIN SMALL LETTER C
    case 0x010c: r=0x0043; break; // LATIN CAPITAL LETTER C WITH CARON => LATIN CAPITAL LETTER C
    case 0x010d: r=0x0063; break; // LATIN SMALL LETTER C WITH CARON => LATIN SMALL LETTER C
    case 0x010e: r=0x0044; break; // LATIN CAPITAL LETTER D WITH CARON => LATIN CAPITAL LETTER D
    case 0x010f: r=0x0064; break; // LATIN SMALL LETTER D WITH CARON => LATIN SMALL LETTER D
    case 0x0110: r=0x44; break; // LATIN CAPITAL LETTER D WITH STROKE =>
    case 0x0111: r=0x64; break; // LATIN SMALL LETTER D WITH STROKE =>
    case 0x0112: r=0x0045; break; // LATIN CAPITAL LETTER E WITH MACRON => LATIN CAPITAL LETTER E
    case 0x0113: r=0x0065; break; // LATIN SMALL LETTER E WITH MACRON => LATIN SMALL LETTER E
    case 0x0114: r=0x0045; break; // LATIN CAPITAL LETTER E WITH BREVE => LATIN CAPITAL LETTER E
    case 0x0115: r=0x0065; break; // LATIN SMALL LETTER E WITH BREVE => LATIN SMALL LETTER E
    case 0x0116: r=0x0045; break; // LATIN CAPITAL LETTER E WITH DOT ABOVE => LATIN CAPITAL LETTER E
    case 0x0117: r=0x0065; break; // LATIN SMALL LETTER E WITH DOT ABOVE => LATIN SMALL LETTER E
    case 0x0118: r=0x0045; break; // LATIN CAPITAL LETTER E WITH OGONEK => LATIN CAPITAL LETTER E
    case 0x0119: r=0x0065; break; // LATIN SMALL LETTER E WITH OGONEK => LATIN SMALL LETTER E
    case 0x011a: r=0x0045; break; // LATIN CAPITAL LETTER E WITH CARON => LATIN CAPITAL LETTER E
    case 0x011b: r=0x0065; break; // LATIN SMALL LETTER E WITH CARON => LATIN SMALL LETTER E
    case 0x011c: r=0x0047; break; // LATIN CAPITAL LETTER G WITH CIRCUMFLEX => LATIN CAPITAL LETTER G
    case 0x011d: r=0x0067; break; // LATIN SMALL LETTER G WITH CIRCUMFLEX => LATIN SMALL LETTER G
    case 0x011e: r=0x0047; break; // LATIN CAPITAL LETTER G WITH BREVE => LATIN CAPITAL LETTER G
    case 0x011f: r=0x0067; break; // LATIN SMALL LETTER G WITH BREVE => LATIN SMALL LETTER G
    case 0x0120: r=0x0047; break; // LATIN CAPITAL LETTER G WITH DOT ABOVE => LATIN CAPITAL LETTER G
    case 0x0121: r=0x0067; break; // LATIN SMALL LETTER G WITH DOT ABOVE => LATIN SMALL LETTER G
    case 0x0122: r=0x0047; break; // LATIN CAPITAL LETTER G WITH CEDILLA => LATIN CAPITAL LETTER G
    case 0x0123: r=0x0067; break; // LATIN SMALL LETTER G WITH CEDILLA => LATIN SMALL LETTER G
    case 0x0124: r=0x0048; break; // LATIN CAPITAL LETTER H WITH CIRCUMFLEX => LATIN CAPITAL LETTER H
    case 0x0125: r=0x0068; break; // LATIN SMALL LETTER H WITH CIRCUMFLEX => LATIN SMALL LETTER H
    case 0x0126: r=0x48; break; // LATIN CAPITAL LETTER H WITH STROKE =>
    case 0x0127: r=0x68; break; // LATIN SMALL LETTER H WITH STROKE =>
    case 0x0128: r=0x0049; break; // LATIN CAPITAL LETTER I WITH TILDE => LATIN CAPITAL LETTER I
    case 0x0129: r=0x0069; break; // LATIN SMALL LETTER I WITH TILDE => LATIN SMALL LETTER I
    case 0x012a: r=0x0049; break; // LATIN CAPITAL LETTER I WITH MACRON => LATIN CAPITAL LETTER I
    case 0x012b: r=0x0069; break; // LATIN SMALL LETTER I WITH MACRON => LATIN SMALL LETTER I
    case 0x012c: r=0x0049; break; // LATIN CAPITAL LETTER I WITH BREVE => LATIN CAPITAL LETTER I
    case 0x012d: r=0x0069; break; // LATIN SMALL LETTER I WITH BREVE => LATIN SMALL LETTER I
    case 0x012e: r=0x0049; break; // LATIN CAPITAL LETTER I WITH OGONEK => LATIN CAPITAL LETTER I
    case 0x012f: r=0x0069; break; // LATIN SMALL LETTER I WITH OGONEK => LATIN SMALL LETTER I
    case 0x0130: r=0x0049; break; // LATIN CAPITAL LETTER I WITH DOT ABOVE => LATIN CAPITAL LETTER I
    case 0x0134: r=0x004A; break; // LATIN CAPITAL LETTER J WITH CIRCUMFLEX => LATIN CAPITAL LETTER J
    case 0x0135: r=0x006A; break; // LATIN SMALL LETTER J WITH CIRCUMFLEX => LATIN SMALL LETTER J
    case 0x0136: r=0x004B; break; // LATIN CAPITAL LETTER K WITH CEDILLA => LATIN CAPITAL LETTER K
    case 0x0137: r=0x006B; break; // LATIN SMALL LETTER K WITH CEDILLA => LATIN SMALL LETTER K
    case 0x0139: r=0x004C; break; // LATIN CAPITAL LETTER L WITH ACUTE => LATIN CAPITAL LETTER L
    case 0x013a: r=0x006C; break; // LATIN SMALL LETTER L WITH ACUTE => LATIN SMALL LETTER L
    case 0x013b: r=0x004C; break; // LATIN CAPITAL LETTER L WITH CEDILLA => LATIN CAPITAL LETTER L
    case 0x013c: r=0x006C; break; // LATIN SMALL LETTER L WITH CEDILLA => LATIN SMALL LETTER L
    case 0x013d: r=0x004C; break; // LATIN CAPITAL LETTER L WITH CARON => LATIN CAPITAL LETTER L
    case 0x013e: r=0x006C; break; // LATIN SMALL LETTER L WITH CARON => LATIN SMALL LETTER L
    case 0x013f: r=0x4c; break; // LATIN CAPITAL LETTER L WITH MIDDLE DOT =>
    case 0x0140: r=0x6c; break; // LATIN SMALL LETTER L WITH MIDDLE DOT =>
    case 0x0141: r=0x4c; break; // LATIN CAPITAL LETTER L WITH STROKE =>
    case 0x0142: r=0x6c; break; // LATIN SMALL LETTER L WITH STROKE =>
    case 0x0143: r=0x004E; break; // LATIN CAPITAL LETTER N WITH ACUTE => LATIN CAPITAL LETTER N
    case 0x0144: r=0x006E; break; // LATIN SMALL LETTER N WITH ACUTE => LATIN SMALL LETTER N
    case 0x0145: r=0x004E; break; // LATIN CAPITAL LETTER N WITH CEDILLA => LATIN CAPITAL LETTER N
    case 0x0146: r=0x006E; break; // LATIN SMALL LETTER N WITH CEDILLA => LATIN SMALL LETTER N
    case 0x0147: r=0x004E; break; // LATIN CAPITAL LETTER N WITH CARON => LATIN CAPITAL LETTER N
    case 0x0148: r=0x006E; break; // LATIN SMALL LETTER N WITH CARON => LATIN SMALL LETTER N
    case 0x014c: r=0x004F; break; // LATIN CAPITAL LETTER O WITH MACRON => LATIN CAPITAL LETTER O
    case 0x014d: r=0x006F; break; // LATIN SMALL LETTER O WITH MACRON => LATIN SMALL LETTER O
    case 0x014e: r=0x004F; break; // LATIN CAPITAL LETTER O WITH BREVE => LATIN CAPITAL LETTER O
    case 0x014f: r=0x006F; break; // LATIN SMALL LETTER O WITH BREVE => LATIN SMALL LETTER O
    case 0x0150: r=0x004F; break; // LATIN CAPITAL LETTER O WITH DOUBLE ACUTE => LATIN CAPITAL LETTER O
    case 0x0151: r=0x006F; break; // LATIN SMALL LETTER O WITH DOUBLE ACUTE => LATIN SMALL LETTER O
    case 0x0154: r=0x0052; break; // LATIN CAPITAL LETTER R WITH ACUTE => LATIN CAPITAL LETTER R
    case 0x0155: r=0x0072; break; // LATIN SMALL LETTER R WITH ACUTE => LATIN SMALL LETTER R
    case 0x0156: r=0x0052; break; // LATIN CAPITAL LETTER R WITH CEDILLA => LATIN CAPITAL LETTER R
    case 0x0157: r=0x0072; break; // LATIN SMALL LETTER R WITH CEDILLA => LATIN SMALL LETTER R
    case 0x0158: r=0x0052; break; // LATIN CAPITAL LETTER R WITH CARON => LATIN CAPITAL LETTER R
    case 0x0159: r=0x0072; break; // LATIN SMALL LETTER R WITH CARON => LATIN SMALL LETTER R
    case 0x015a: r=0x0053; break; // LATIN CAPITAL LETTER S WITH ACUTE => LATIN CAPITAL LETTER S
    case 0x015b: r=0x0073; break; // LATIN SMALL LETTER S WITH ACUTE => LATIN SMALL LETTER S
    case 0x015c: r=0x0053; break; // LATIN CAPITAL LETTER S WITH CIRCUMFLEX => LATIN CAPITAL LETTER S
    case 0x015d: r=0x0073; break; // LATIN SMALL LETTER S WITH CIRCUMFLEX => LATIN SMALL LETTER S
    case 0x015e: r=0x0053; break; // LATIN CAPITAL LETTER S WITH CEDILLA => LATIN CAPITAL LETTER S
    case 0x015f: r=0x0073; break; // LATIN SMALL LETTER S WITH CEDILLA => LATIN SMALL LETTER S
    case 0x0160: r=0x0053; break; // LATIN CAPITAL LETTER S WITH CARON => LATIN CAPITAL LETTER S
    case 0x0161: r=0x0073; break; // LATIN SMALL LETTER S WITH CARON => LATIN SMALL LETTER S
    case 0x0162: r=0x0054; break; // LATIN CAPITAL LETTER T WITH CEDILLA => LATIN CAPITAL LETTER T
    case 0x0163: r=0x0074; break; // LATIN SMALL LETTER T WITH CEDILLA => LATIN SMALL LETTER T
    case 0x0164: r=0x0054; break; // LATIN CAPITAL LETTER T WITH CARON => LATIN CAPITAL LETTER T
    case 0x0165: r=0x0074; break; // LATIN SMALL LETTER T WITH CARON => LATIN SMALL LETTER T
    case 0x0166: r=0x54; break; // LATIN CAPITAL LETTER T WITH STROKE =>
    case 0x0167: r=0x74; break; // LATIN SMALL LETTER T WITH STROKE =>
    case 0x0168: r=0x0055; break; // LATIN CAPITAL LETTER U WITH TILDE => LATIN CAPITAL LETTER U
    case 0x0169: r=0x0075; break; // LATIN SMALL LETTER U WITH TILDE => LATIN SMALL LETTER U
    case 0x016a: r=0x0055; break; // LATIN CAPITAL LETTER U WITH MACRON => LATIN CAPITAL LETTER U
    case 0x016b: r=0x0075; break; // LATIN SMALL LETTER U WITH MACRON => LATIN SMALL LETTER U
    case 0x016c: r=0x0055; break; // LATIN CAPITAL LETTER U WITH BREVE => LATIN CAPITAL LETTER U
    case 0x016d: r=0x0075; break; // LATIN SMALL LETTER U WITH BREVE => LATIN SMALL LETTER U
    case 0x016e: r=0x0055; break; // LATIN CAPITAL LETTER U WITH RING ABOVE => LATIN CAPITAL LETTER U
    case 0x016f: r=0x0075; break; // LATIN SMALL LETTER U WITH RING ABOVE => LATIN SMALL LETTER U
    case 0x0170: r=0x0055; break; // LATIN CAPITAL LETTER U WITH DOUBLE ACUTE => LATIN CAPITAL LETTER U
    case 0x0171: r=0x0075; break; // LATIN SMALL LETTER U WITH DOUBLE ACUTE => LATIN SMALL LETTER U
    case 0x0172: r=0x0055; break; // LATIN CAPITAL LETTER U WITH OGONEK => LATIN CAPITAL LETTER U
    case 0x0173: r=0x0075; break; // LATIN SMALL LETTER U WITH OGONEK => LATIN SMALL LETTER U
    case 0x0174: r=0x0057; break; // LATIN CAPITAL LETTER W WITH CIRCUMFLEX => LATIN CAPITAL LETTER W
    case 0x0175: r=0x0077; break; // LATIN SMALL LETTER W WITH CIRCUMFLEX => LATIN SMALL LETTER W
    case 0x0176: r=0x0059; break; // LATIN CAPITAL LETTER Y WITH CIRCUMFLEX => LATIN CAPITAL LETTER Y
    case 0x0177: r=0x0079; break; // LATIN SMALL LETTER Y WITH CIRCUMFLEX => LATIN SMALL LETTER Y
    case 0x0178: r=0x0059; break; // LATIN CAPITAL LETTER Y WITH DIAERESIS => LATIN CAPITAL LETTER Y
    case 0x0179: r=0x005A; break; // LATIN CAPITAL LETTER Z WITH ACUTE => LATIN CAPITAL LETTER Z
    case 0x017a: r=0x007A; break; // LATIN SMALL LETTER Z WITH ACUTE => LATIN SMALL LETTER Z
    case 0x017b: r=0x005A; break; // LATIN CAPITAL LETTER Z WITH DOT ABOVE => LATIN CAPITAL LETTER Z
    case 0x017c: r=0x007A; break; // LATIN SMALL LETTER Z WITH DOT ABOVE => LATIN SMALL LETTER Z
    case 0x017d: r=0x005A; break; // LATIN CAPITAL LETTER Z WITH CARON => LATIN CAPITAL LETTER Z
    case 0x017e: r=0x007A; break; // LATIN SMALL LETTER Z WITH CARON => LATIN SMALL LETTER Z
    case 0x0180: r=0x62; break; // LATIN SMALL LETTER B WITH STROKE =>
    case 0x0181: r=0x42; break; // LATIN CAPITAL LETTER B WITH HOOK =>
    case 0x0182: r=0x42; break; // LATIN CAPITAL LETTER B WITH TOPBAR =>
    case 0x0183: r=0x62; break; // LATIN SMALL LETTER B WITH TOPBAR =>
    case 0x0187: r=0x43; break; // LATIN CAPITAL LETTER C WITH HOOK =>
    case 0x0188: r=0x63; break; // LATIN SMALL LETTER C WITH HOOK =>
    case 0x018a: r=0x44; break; // LATIN CAPITAL LETTER D WITH HOOK =>
    case 0x018b: r=0x44; break; // LATIN CAPITAL LETTER D WITH TOPBAR =>
    case 0x018c: r=0x64; break; // LATIN SMALL LETTER D WITH TOPBAR =>
    case 0x0191: r=0x46; break; // LATIN CAPITAL LETTER F WITH HOOK =>
    case 0x0192: r=0x66; break; // LATIN SMALL LETTER F WITH HOOK =>
    case 0x0193: r=0x47; break; // LATIN CAPITAL LETTER G WITH HOOK =>
    case 0x0197: r=0x49; break; // LATIN CAPITAL LETTER I WITH STROKE =>
    case 0x0198: r=0x4b; break; // LATIN CAPITAL LETTER K WITH HOOK =>
    case 0x0199: r=0x6b; break; // LATIN SMALL LETTER K WITH HOOK =>
    case 0x019a: r=0x6c; break; // LATIN SMALL LETTER L WITH BAR =>
    case 0x019d: r=0x4e; break; // LATIN CAPITAL LETTER N WITH LEFT HOOK =>
    case 0x019e: r=0x6e; break; // LATIN SMALL LETTER N WITH LONG RIGHT LEG =>
    case 0x019f: r=0x4f; break; // LATIN CAPITAL LETTER O WITH MIDDLE TILDE =>
    case 0x01a0: r=0x004F; break; // LATIN CAPITAL LETTER O WITH HORN => LATIN CAPITAL LETTER O
    case 0x01a1: r=0x006F; break; // LATIN SMALL LETTER O WITH HORN => LATIN SMALL LETTER O
    case 0x01a4: r=0x50; break; // LATIN CAPITAL LETTER P WITH HOOK =>
    case 0x01a5: r=0x70; break; // LATIN SMALL LETTER P WITH HOOK =>
    case 0x01ab: r=0x74; break; // LATIN SMALL LETTER T WITH PALATAL HOOK =>
    case 0x01ac: r=0x54; break; // LATIN CAPITAL LETTER T WITH HOOK =>
    case 0x01ad: r=0x74; break; // LATIN SMALL LETTER T WITH HOOK =>
    case 0x01ae: r=0x54; break; // LATIN CAPITAL LETTER T WITH RETROFLEX HOOK =>
    case 0x01af: r=0x0055; break; // LATIN CAPITAL LETTER U WITH HORN => LATIN CAPITAL LETTER U
    case 0x01b0: r=0x0075; break; // LATIN SMALL LETTER U WITH HORN => LATIN SMALL LETTER U
    case 0x01b2: r=0x56; break; // LATIN CAPITAL LETTER V WITH HOOK =>
    case 0x01b3: r=0x59; break; // LATIN CAPITAL LETTER Y WITH HOOK =>
    case 0x01b4: r=0x79; break; // LATIN SMALL LETTER Y WITH HOOK =>
    case 0x01b5: r=0x5a; break; // LATIN CAPITAL LETTER Z WITH STROKE =>
    case 0x01b6: r=0x7a; break; // LATIN SMALL LETTER Z WITH STROKE =>
    case 0x01c5: r=0x44; break; // LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH CARON =>
    case 0x01c8: r=0x4c; break; // LATIN CAPITAL LETTER L WITH SMALL LETTER J =>
    case 0x01cb: r=0x4e; break; // LATIN CAPITAL LETTER N WITH SMALL LETTER J =>
    case 0x01cd: r=0x0041; break; // LATIN CAPITAL LETTER A WITH CARON => LATIN CAPITAL LETTER A
    case 0x01ce: r=0x0061; break; // LATIN SMALL LETTER A WITH CARON => LATIN SMALL LETTER A
    case 0x01cf: r=0x0049; break; // LATIN CAPITAL LETTER I WITH CARON => LATIN CAPITAL LETTER I
    case 0x01d0: r=0x0069; break; // LATIN SMALL LETTER I WITH CARON => LATIN SMALL LETTER I
    case 0x01d1: r=0x004F; break; // LATIN CAPITAL LETTER O WITH CARON => LATIN CAPITAL LETTER O
    case 0x01d2: r=0x006F; break; // LATIN SMALL LETTER O WITH CARON => LATIN SMALL LETTER O
    case 0x01d3: r=0x0055; break; // LATIN CAPITAL LETTER U WITH CARON => LATIN CAPITAL LETTER U
    case 0x01d4: r=0x0075; break; // LATIN SMALL LETTER U WITH CARON => LATIN SMALL LETTER U
    case 0x01d5: r=0x0055; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND MACRON => LATIN CAPITAL LETTER U
    case 0x01d6: r=0x0075; break; // LATIN SMALL LETTER U WITH DIAERESIS AND MACRON => LATIN SMALL LETTER U
    case 0x01d7: r=0x0055; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND ACUTE => LATIN CAPITAL LETTER U
    case 0x01d8: r=0x0075; break; // LATIN SMALL LETTER U WITH DIAERESIS AND ACUTE => LATIN SMALL LETTER U
    case 0x01d9: r=0x0055; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND CARON => LATIN CAPITAL LETTER U
    case 0x01da: r=0x0075; break; // LATIN SMALL LETTER U WITH DIAERESIS AND CARON => LATIN SMALL LETTER U
    case 0x01db: r=0x0055; break; // LATIN CAPITAL LETTER U WITH DIAERESIS AND GRAVE => LATIN CAPITAL LETTER U
    case 0x01dc: r=0x0075; break; // LATIN SMALL LETTER U WITH DIAERESIS AND GRAVE => LATIN SMALL LETTER U
    case 0x01de: r=0x0041; break; // LATIN CAPITAL LETTER A WITH DIAERESIS AND MACRON => LATIN CAPITAL LETTER A
    case 0x01df: r=0x0061; break; // LATIN SMALL LETTER A WITH DIAERESIS AND MACRON => LATIN SMALL LETTER A
    case 0x01e0: r=0x0226; break; // LATIN CAPITAL LETTER A WITH DOT ABOVE AND MACRON =>
    case 0x01e1: r=0x0227; break; // LATIN SMALL LETTER A WITH DOT ABOVE AND MACRON =>
    case 0x01e2: r=0x00C6; break; // LATIN CAPITAL LETTER AE WITH MACRON => LATIN CAPITAL LETTER AE
    case 0x01e3: r=0x00E6; break; // LATIN SMALL LETTER AE WITH MACRON => LATIN SMALL LETTER AE
    case 0x01e4: r=0x47; break; // LATIN CAPITAL LETTER G WITH STROKE =>
    case 0x01e5: r=0x67; break; // LATIN SMALL LETTER G WITH STROKE =>
    case 0x01e6: r=0x0047; break; // LATIN CAPITAL LETTER G WITH CARON => LATIN CAPITAL LETTER G
    case 0x01e7: r=0x0067; break; // LATIN SMALL LETTER G WITH CARON => LATIN SMALL LETTER G
    case 0x01e8: r=0x004B; break; // LATIN CAPITAL LETTER K WITH CARON => LATIN CAPITAL LETTER K
    case 0x01e9: r=0x006B; break; // LATIN SMALL LETTER K WITH CARON => LATIN SMALL LETTER K
    case 0x01ea: r=0x004F; break; // LATIN CAPITAL LETTER O WITH OGONEK => LATIN CAPITAL LETTER O
    case 0x01eb: r=0x006F; break; // LATIN SMALL LETTER O WITH OGONEK => LATIN SMALL LETTER O
    case 0x01ec: r=0x004F; break; // LATIN CAPITAL LETTER O WITH OGONEK AND MACRON => LATIN CAPITAL LETTER O
    case 0x01ed: r=0x006F; break; // LATIN SMALL LETTER O WITH OGONEK AND MACRON => LATIN SMALL LETTER O
    case 0x01ee: r=0x01B7; break; // LATIN CAPITAL LETTER EZH WITH CARON => LATIN CAPITAL LETTER EZH
    case 0x01ef: r=0x0292; break; // LATIN SMALL LETTER EZH WITH CARON =>
    case 0x01f0: r=0x006A; break; // LATIN SMALL LETTER J WITH CARON => LATIN SMALL LETTER J
    case 0x01f2: r=0x44; break; // LATIN CAPITAL LETTER D WITH SMALL LETTER Z =>
    case 0x01f4: r=0x0047; break; // LATIN CAPITAL LETTER G WITH ACUTE => LATIN CAPITAL LETTER G
    case 0x01f5: r=0x0067; break; // LATIN SMALL LETTER G WITH ACUTE => LATIN SMALL LETTER G
    case 0x01f8: r=0x004E; break; // LATIN CAPITAL LETTER N WITH GRAVE => LATIN CAPITAL LETTER N
    case 0x01f9: r=0x006E; break; // LATIN SMALL LETTER N WITH GRAVE => LATIN SMALL LETTER N
    case 0x01fa: r=0x0041; break; // LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE => LATIN CAPITAL LETTER A
    case 0x01fb: r=0x0061; break; // LATIN SMALL LETTER A WITH RING ABOVE AND ACUTE => LATIN SMALL LETTER A
    case 0x01fc: r=0x00C6; break; // LATIN CAPITAL LETTER AE WITH ACUTE => LATIN CAPITAL LETTER AE
    case 0x01fd: r=0x00E6; break; // LATIN SMALL LETTER AE WITH ACUTE => LATIN SMALL LETTER AE
    case 0x01fe: r=0x4f; break; // LATIN CAPITAL LETTER O WITH STROKE AND ACUTE =>
    case 0x01ff: r=0x6f; break; // LATIN SMALL LETTER O WITH STROKE AND ACUTE =>
    case 0x0200: r=0x0041; break; // LATIN CAPITAL LETTER A WITH DOUBLE GRAVE => LATIN CAPITAL LETTER A
    case 0x0201: r=0x0061; break; // LATIN SMALL LETTER A WITH DOUBLE GRAVE => LATIN SMALL LETTER A
    case 0x0202: r=0x0041; break; // LATIN CAPITAL LETTER A WITH INVERTED BREVE => LATIN CAPITAL LETTER A
    case 0x0203: r=0x0061; break; // LATIN SMALL LETTER A WITH INVERTED BREVE => LATIN SMALL LETTER A
    case 0x0204: r=0x0045; break; // LATIN CAPITAL LETTER E WITH DOUBLE GRAVE => LATIN CAPITAL LETTER E
    case 0x0205: r=0x0065; break; // LATIN SMALL LETTER E WITH DOUBLE GRAVE => LATIN SMALL LETTER E
    case 0x0206: r=0x0045; break; // LATIN CAPITAL LETTER E WITH INVERTED BREVE => LATIN CAPITAL LETTER E
    case 0x0207: r=0x0065; break; // LATIN SMALL LETTER E WITH INVERTED BREVE => LATIN SMALL LETTER E
    case 0x0208: r=0x0049; break; // LATIN CAPITAL LETTER I WITH DOUBLE GRAVE => LATIN CAPITAL LETTER I
    case 0x0209: r=0x0069; break; // LATIN SMALL LETTER I WITH DOUBLE GRAVE => LATIN SMALL LETTER I
    case 0x020a: r=0x0049; break; // LATIN CAPITAL LETTER I WITH INVERTED BREVE => LATIN CAPITAL LETTER I
    case 0x020b: r=0x0069; break; // LATIN SMALL LETTER I WITH INVERTED BREVE => LATIN SMALL LETTER I
    case 0x020c: r=0x004F; break; // LATIN CAPITAL LETTER O WITH DOUBLE GRAVE => LATIN CAPITAL LETTER O
    case 0x020d: r=0x006F; break; // LATIN SMALL LETTER O WITH DOUBLE GRAVE => LATIN SMALL LETTER O
    case 0x020e: r=0x004F; break; // LATIN CAPITAL LETTER O WITH INVERTED BREVE => LATIN CAPITAL LETTER O
    case 0x020f: r=0x006F; break; // LATIN SMALL LETTER O WITH INVERTED BREVE => LATIN SMALL LETTER O
    case 0x0210: r=0x0052; break; // LATIN CAPITAL LETTER R WITH DOUBLE GRAVE => LATIN CAPITAL LETTER R
    case 0x0211: r=0x0072; break; // LATIN SMALL LETTER R WITH DOUBLE GRAVE => LATIN SMALL LETTER R
    case 0x0212: r=0x0052; break; // LATIN CAPITAL LETTER R WITH INVERTED BREVE => LATIN CAPITAL LETTER R
    case 0x0213: r=0x0072; break; // LATIN SMALL LETTER R WITH INVERTED BREVE => LATIN SMALL LETTER R
    case 0x0214: r=0x0055; break; // LATIN CAPITAL LETTER U WITH DOUBLE GRAVE => LATIN CAPITAL LETTER U
    case 0x0215: r=0x0075; break; // LATIN SMALL LETTER U WITH DOUBLE GRAVE => LATIN SMALL LETTER U
    case 0x0216: r=0x0055; break; // LATIN CAPITAL LETTER U WITH INVERTED BREVE => LATIN CAPITAL LETTER U
    case 0x0217: r=0x0075; break; // LATIN SMALL LETTER U WITH INVERTED BREVE => LATIN SMALL LETTER U
    case 0x0218: r=0x0053; break; // LATIN CAPITAL LETTER S WITH COMMA BELOW => LATIN CAPITAL LETTER S
    case 0x0219: r=0x0073; break; // LATIN SMALL LETTER S WITH COMMA BELOW => LATIN SMALL LETTER S
    case 0x021a: r=0x0054; break; // LATIN CAPITAL LETTER T WITH COMMA BELOW => LATIN CAPITAL LETTER T
    case 0x021b: r=0x0074; break; // LATIN SMALL LETTER T WITH COMMA BELOW => LATIN SMALL LETTER T
    case 0x021e: r=0x0048; break; // LATIN CAPITAL LETTER H WITH CARON => LATIN CAPITAL LETTER H
    case 0x021f: r=0x0068; break; // LATIN SMALL LETTER H WITH CARON => LATIN SMALL LETTER H
    case 0x0220: r=0x4e; break; // LATIN CAPITAL LETTER N WITH LONG RIGHT LEG =>
    case 0x0224: r=0x5a; break; // LATIN CAPITAL LETTER Z WITH HOOK =>
    case 0x0225: r=0x7a; break; // LATIN SMALL LETTER Z WITH HOOK =>
    case 0x0226: r=0x0041; break; // LATIN CAPITAL LETTER A WITH DOT ABOVE => LATIN CAPITAL LETTER A
    case 0x0227: r=0x0061; break; // LATIN SMALL LETTER A WITH DOT ABOVE => LATIN SMALL LETTER A
    case 0x0228: r=0x0045; break; // LATIN CAPITAL LETTER E WITH CEDILLA => LATIN CAPITAL LETTER E
    case 0x0229: r=0x0065; break; // LATIN SMALL LETTER E WITH CEDILLA => LATIN SMALL LETTER E
    case 0x022a: r=0x004F; break; // LATIN CAPITAL LETTER O WITH DIAERESIS AND MACRON => LATIN CAPITAL LETTER O
    case 0x022b: r=0x006F; break; // LATIN SMALL LETTER O WITH DIAERESIS AND MACRON => LATIN SMALL LETTER O
    case 0x022c: r=0x004F; break; // LATIN CAPITAL LETTER O WITH TILDE AND MACRON => LATIN CAPITAL LETTER O
    case 0x022d: r=0x006F; break; // LATIN SMALL LETTER O WITH TILDE AND MACRON => LATIN SMALL LETTER O
    case 0x022e: r=0x004F; break; // LATIN CAPITAL LETTER O WITH DOT ABOVE => LATIN CAPITAL LETTER O
    case 0x022f: r=0x006F; break; // LATIN SMALL LETTER O WITH DOT ABOVE => LATIN SMALL LETTER O
    case 0x0230: r=0x004F; break; // LATIN CAPITAL LETTER O WITH DOT ABOVE AND MACRON => LATIN CAPITAL LETTER O
    case 0x0231: r=0x006F; break; // LATIN SMALL LETTER O WITH DOT ABOVE AND MACRON => LATIN SMALL LETTER O
    case 0x0232: r=0x0059; break; // LATIN CAPITAL LETTER Y WITH MACRON => LATIN CAPITAL LETTER Y
    case 0x0233: r=0x0079; break; // LATIN SMALL LETTER Y WITH MACRON => LATIN SMALL LETTER Y
    case 0x0253: r=0x62; break; // LATIN SMALL LETTER B WITH HOOK =>
    case 0x0255: r=0x63; break; // LATIN SMALL LETTER C WITH CURL =>
    case 0x0256: r=0x64; break; // LATIN SMALL LETTER D WITH TAIL =>
    case 0x0257: r=0x64; break; // LATIN SMALL LETTER D WITH HOOK =>
    case 0x0260: r=0x67; break; // LATIN SMALL LETTER G WITH HOOK =>
    case 0x0266: r=0x68; break; // LATIN SMALL LETTER H WITH HOOK =>
    case 0x0268: r=0x69; break; // LATIN SMALL LETTER I WITH STROKE =>
    case 0x026b: r=0x6c; break; // LATIN SMALL LETTER L WITH MIDDLE TILDE =>
    case 0x026c: r=0x6c; break; // LATIN SMALL LETTER L WITH BELT =>
    case 0x026d: r=0x6c; break; // LATIN SMALL LETTER L WITH RETROFLEX HOOK =>
    case 0x0271: r=0x6d; break; // LATIN SMALL LETTER M WITH HOOK =>
    case 0x0272: r=0x6e; break; // LATIN SMALL LETTER N WITH LEFT HOOK =>
    case 0x0273: r=0x6e; break; // LATIN SMALL LETTER N WITH RETROFLEX HOOK =>
    case 0x027c: r=0x72; break; // LATIN SMALL LETTER R WITH LONG LEG =>
    case 0x027d: r=0x72; break; // LATIN SMALL LETTER R WITH TAIL =>
    case 0x027e: r=0x72; break; // LATIN SMALL LETTER R WITH FISHHOOK =>
    case 0x0282: r=0x73; break; // LATIN SMALL LETTER S WITH HOOK =>
    case 0x0288: r=0x74; break; // LATIN SMALL LETTER T WITH RETROFLEX HOOK =>
    case 0x028b: r=0x76; break; // LATIN SMALL LETTER V WITH HOOK =>
    case 0x0290: r=0x7a; break; // LATIN SMALL LETTER Z WITH RETROFLEX HOOK =>
    case 0x0291: r=0x7a; break; // LATIN SMALL LETTER Z WITH CURL =>
    case 0x029d: r=0x6a; break; // LATIN SMALL LETTER J WITH CROSSED-TAIL =>
    case 0x02a0: r=0x71; break; // LATIN SMALL LETTER Q WITH HOOK =>
    case 0x0386: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH TONOS =>
    case 0x0388: r=0x0395; break; // GREEK CAPITAL LETTER EPSILON WITH TONOS =>
    case 0x0389: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH TONOS =>
    case 0x038a: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH TONOS =>
    case 0x038c: r=0x039F; break; // GREEK CAPITAL LETTER OMICRON WITH TONOS =>
    case 0x038e: r=0x03A5; break; // GREEK CAPITAL LETTER UPSILON WITH TONOS =>
    case 0x038f: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH TONOS =>
    case 0x0390: r=0x03CA; break; // GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS =>
    case 0x03aa: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH DIALYTIKA => GREEK CAPITAL LETTER IOTA
    case 0x03ab: r=0x03A5; break; // GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA => GREEK CAPITAL LETTER UPSILON
    case 0x03ac: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH TONOS =>
    case 0x03ad: r=0x03B5; break; // GREEK SMALL LETTER EPSILON WITH TONOS =>
    case 0x03ae: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH TONOS =>
    case 0x03af: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH TONOS =>
    case 0x03b0: r=0x03CB; break; // GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS =>
    case 0x03ca: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH DIALYTIKA => GREEK SMALL LETTER IOTA
    case 0x03cb: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH DIALYTIKA => GREEK SMALL LETTER UPSILON
    case 0x03cc: r=0x03BF; break; // GREEK SMALL LETTER OMICRON WITH TONOS => GREEK SMALL LETTER OMICRON
    case 0x03cd: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH TONOS => GREEK SMALL LETTER UPSILON
    case 0x03ce: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH TONOS => GREEK SMALL LETTER OMEGA
    case 0x0400: r=0x0415; break; // CYRILLIC CAPITAL LETTER IE WITH GRAVE =>
    case 0x0401: r=0x0415; break; // CYRILLIC CAPITAL LETTER IO =>
    case 0x0403: r=0x0413; break; // CYRILLIC CAPITAL LETTER GJE =>
    case 0x0407: r=0x0406; break; // CYRILLIC CAPITAL LETTER YI => CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
    case 0x040c: r=0x041A; break; // CYRILLIC CAPITAL LETTER KJE =>
    case 0x040d: r=0x0418; break; // CYRILLIC CAPITAL LETTER I WITH GRAVE =>
    case 0x040e: r=0x0423; break; // CYRILLIC CAPITAL LETTER SHORT U =>
    case 0x0419: r=0x0418; break; // CYRILLIC CAPITAL LETTER SHORT I => CYRILLIC CAPITAL LETTER I
    case 0x0439: r=0x0438; break; // CYRILLIC SMALL LETTER SHORT I => CYRILLIC SMALL LETTER I
    case 0x0450: r=0x0435; break; // CYRILLIC SMALL LETTER IE WITH GRAVE => CYRILLIC SMALL LETTER IE
    case 0x0451: r=0x0435; break; // CYRILLIC SMALL LETTER IO => CYRILLIC SMALL LETTER IE
    case 0x0453: r=0x0433; break; // CYRILLIC SMALL LETTER GJE => CYRILLIC SMALL LETTER GHE
    case 0x0457: r=0x0456; break; // CYRILLIC SMALL LETTER YI => CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
    case 0x045c: r=0x043A; break; // CYRILLIC SMALL LETTER KJE => CYRILLIC SMALL LETTER KA
    case 0x045d: r=0x0438; break; // CYRILLIC SMALL LETTER I WITH GRAVE => CYRILLIC SMALL LETTER I
    case 0x045e: r=0x0443; break; // CYRILLIC SMALL LETTER SHORT U => CYRILLIC SMALL LETTER U
    case 0x0476: r=0x0474; break; // CYRILLIC CAPITAL LETTER IZHITSA WITH DOUBLE GRAVE ACCENT => CYRILLIC CAPITAL LETTER IZHITSA
    case 0x0477: r=0x0475; break; // CYRILLIC SMALL LETTER IZHITSA WITH DOUBLE GRAVE ACCENT => CYRILLIC SMALL LETTER IZHITSA
    case 0x04c1: r=0x0416; break; // CYRILLIC CAPITAL LETTER ZHE WITH BREVE => CYRILLIC CAPITAL LETTER ZHE
    case 0x04c2: r=0x0436; break; // CYRILLIC SMALL LETTER ZHE WITH BREVE => CYRILLIC SMALL LETTER ZHE
    case 0x04d0: r=0x0410; break; // CYRILLIC CAPITAL LETTER A WITH BREVE => CYRILLIC CAPITAL LETTER A
    case 0x04d1: r=0x0430; break; // CYRILLIC SMALL LETTER A WITH BREVE => CYRILLIC SMALL LETTER A
    case 0x04d2: r=0x0410; break; // CYRILLIC CAPITAL LETTER A WITH DIAERESIS => CYRILLIC CAPITAL LETTER A
    case 0x04d3: r=0x0430; break; // CYRILLIC SMALL LETTER A WITH DIAERESIS => CYRILLIC SMALL LETTER A
    case 0x04d6: r=0x0415; break; // CYRILLIC CAPITAL LETTER IE WITH BREVE => CYRILLIC CAPITAL LETTER IE
    case 0x04d7: r=0x0435; break; // CYRILLIC SMALL LETTER IE WITH BREVE => CYRILLIC SMALL LETTER IE
    case 0x04da: r=0x04D8; break; // CYRILLIC CAPITAL LETTER SCHWA WITH DIAERESIS => CYRILLIC CAPITAL LETTER SCHWA
    case 0x04db: r=0x04D9; break; // CYRILLIC SMALL LETTER SCHWA WITH DIAERESIS => CYRILLIC SMALL LETTER SCHWA
    case 0x04dc: r=0x0416; break; // CYRILLIC CAPITAL LETTER ZHE WITH DIAERESIS => CYRILLIC CAPITAL LETTER ZHE
    case 0x04dd: r=0x0436; break; // CYRILLIC SMALL LETTER ZHE WITH DIAERESIS => CYRILLIC SMALL LETTER ZHE
    case 0x04de: r=0x0417; break; // CYRILLIC CAPITAL LETTER ZE WITH DIAERESIS => CYRILLIC CAPITAL LETTER ZE
    case 0x04df: r=0x0437; break; // CYRILLIC SMALL LETTER ZE WITH DIAERESIS => CYRILLIC SMALL LETTER ZE
    case 0x04e2: r=0x0418; break; // CYRILLIC CAPITAL LETTER I WITH MACRON => CYRILLIC CAPITAL LETTER I
    case 0x04e3: r=0x0438; break; // CYRILLIC SMALL LETTER I WITH MACRON => CYRILLIC SMALL LETTER I
    case 0x04e4: r=0x0418; break; // CYRILLIC CAPITAL LETTER I WITH DIAERESIS => CYRILLIC CAPITAL LETTER I
    case 0x04e5: r=0x0438; break; // CYRILLIC SMALL LETTER I WITH DIAERESIS => CYRILLIC SMALL LETTER I
    case 0x04e6: r=0x041E; break; // CYRILLIC CAPITAL LETTER O WITH DIAERESIS => CYRILLIC CAPITAL LETTER O
    case 0x04e7: r=0x043E; break; // CYRILLIC SMALL LETTER O WITH DIAERESIS => CYRILLIC SMALL LETTER O
    case 0x04ea: r=0x04E8; break; // CYRILLIC CAPITAL LETTER BARRED O WITH DIAERESIS => CYRILLIC CAPITAL LETTER BARRED O
    case 0x04eb: r=0x04E9; break; // CYRILLIC SMALL LETTER BARRED O WITH DIAERESIS => CYRILLIC SMALL LETTER BARRED O
    case 0x04ec: r=0x042D; break; // CYRILLIC CAPITAL LETTER E WITH DIAERESIS => CYRILLIC CAPITAL LETTER E
    case 0x04ed: r=0x044D; break; // CYRILLIC SMALL LETTER E WITH DIAERESIS => CYRILLIC SMALL LETTER E
    case 0x04ee: r=0x0423; break; // CYRILLIC CAPITAL LETTER U WITH MACRON => CYRILLIC CAPITAL LETTER U
    case 0x04ef: r=0x0443; break; // CYRILLIC SMALL LETTER U WITH MACRON => CYRILLIC SMALL LETTER U
    case 0x04f0: r=0x0423; break; // CYRILLIC CAPITAL LETTER U WITH DIAERESIS => CYRILLIC CAPITAL LETTER U
    case 0x04f1: r=0x0443; break; // CYRILLIC SMALL LETTER U WITH DIAERESIS => CYRILLIC SMALL LETTER U
    case 0x04f2: r=0x0423; break; // CYRILLIC CAPITAL LETTER U WITH DOUBLE ACUTE => CYRILLIC CAPITAL LETTER U
    case 0x04f3: r=0x0443; break; // CYRILLIC SMALL LETTER U WITH DOUBLE ACUTE => CYRILLIC SMALL LETTER U
    case 0x04f4: r=0x0427; break; // CYRILLIC CAPITAL LETTER CHE WITH DIAERESIS => CYRILLIC CAPITAL LETTER CHE
    case 0x04f5: r=0x0447; break; // CYRILLIC SMALL LETTER CHE WITH DIAERESIS => CYRILLIC SMALL LETTER CHE
    case 0x04f8: r=0x042B; break; // CYRILLIC CAPITAL LETTER YERU WITH DIAERESIS => CYRILLIC CAPITAL LETTER YERU
    case 0x04f9: r=0x044B; break; // CYRILLIC SMALL LETTER YERU WITH DIAERESIS => CYRILLIC SMALL LETTER YERU
    case 0x0622: r=0x0627; break; // ARABIC LETTER ALEF WITH MADDA ABOVE =>
    case 0x0623: r=0x0627; break; // ARABIC LETTER ALEF WITH HAMZA ABOVE =>
    case 0x0624: r=0x0648; break; // ARABIC LETTER WAW WITH HAMZA ABOVE =>
    case 0x0625: r=0x0627; break; // ARABIC LETTER ALEF WITH HAMZA BELOW =>
    case 0x0626: r=0x064A; break; // ARABIC LETTER YEH WITH HAMZA ABOVE =>
    case 0x06c0: r=0x06D5; break; // ARABIC LETTER HEH WITH YEH ABOVE =>
    case 0x06c2: r=0x06C1; break; // ARABIC LETTER HEH GOAL WITH HAMZA ABOVE => ARABIC LETTER HEH GOAL
    case 0x06d3: r=0x06D2; break; // ARABIC LETTER YEH BARREE WITH HAMZA ABOVE => ARABIC LETTER YEH BARREE
    case 0x0929: r=0x0928; break; // DEVANAGARI LETTER NNNA => DEVANAGARI LETTER NA
    case 0x0931: r=0x0930; break; // DEVANAGARI LETTER RRA => DEVANAGARI LETTER RA
    case 0x0934: r=0x0933; break; // DEVANAGARI LETTER LLLA => DEVANAGARI LETTER LLA
    case 0x0958: r=0x0915; break; // DEVANAGARI LETTER QA => DEVANAGARI LETTER KA
    case 0x0959: r=0x0916; break; // DEVANAGARI LETTER KHHA => DEVANAGARI LETTER KHA
    case 0x095a: r=0x0917; break; // DEVANAGARI LETTER GHHA => DEVANAGARI LETTER GA
    case 0x095b: r=0x091C; break; // DEVANAGARI LETTER ZA => DEVANAGARI LETTER JA
    case 0x095c: r=0x0921; break; // DEVANAGARI LETTER DDDHA => DEVANAGARI LETTER DDA
    case 0x095d: r=0x0922; break; // DEVANAGARI LETTER RHA => DEVANAGARI LETTER DDHA
    case 0x095e: r=0x092B; break; // DEVANAGARI LETTER FA => DEVANAGARI LETTER PHA
    case 0x095f: r=0x092F; break; // DEVANAGARI LETTER YYA => DEVANAGARI LETTER YA
    case 0x09dc: r=0x09A1; break; // BENGALI LETTER RRA => BENGALI LETTER DDA
    case 0x09dd: r=0x09A2; break; // BENGALI LETTER RHA => BENGALI LETTER DDHA
    case 0x09df: r=0x09AF; break; // BENGALI LETTER YYA => BENGALI LETTER YA
    case 0x0a33: r=0x0A32; break; // GURMUKHI LETTER LLA => GURMUKHI LETTER LA
    case 0x0a36: r=0x0A38; break; // GURMUKHI LETTER SHA =>
    case 0x0a59: r=0x0A16; break; // GURMUKHI LETTER KHHA => GURMUKHI LETTER KHA
    case 0x0a5a: r=0x0A17; break; // GURMUKHI LETTER GHHA => GURMUKHI LETTER GA
    case 0x0a5b: r=0x0A1C; break; // GURMUKHI LETTER ZA => GURMUKHI LETTER JA
    case 0x0a5e: r=0x0A2B; break; // GURMUKHI LETTER FA => GURMUKHI LETTER PHA
    case 0x0b5c: r=0x0B21; break; // ORIYA LETTER RRA => ORIYA LETTER DDA
    case 0x0b5d: r=0x0B22; break; // ORIYA LETTER RHA => ORIYA LETTER DDHA
    case 0x0b94: r=0x0B92; break; // TAMIL LETTER AU => TAMIL LETTER O
    case 0x0f43: r=0x0F42; break; // TIBETAN LETTER GHA => TIBETAN LETTER GA
    case 0x0f4d: r=0x0F4C; break; // TIBETAN LETTER DDHA => TIBETAN LETTER DDA
    case 0x0f52: r=0x0F51; break; // TIBETAN LETTER DHA => TIBETAN LETTER DA
    case 0x0f57: r=0x0F56; break; // TIBETAN LETTER BHA => TIBETAN LETTER BA
    case 0x0f5c: r=0x0F5B; break; // TIBETAN LETTER DZHA => TIBETAN LETTER DZA
    case 0x0f69: r=0x0F40; break; // TIBETAN LETTER KSSA => TIBETAN LETTER KA
    case 0x0f93: r=0x0F92; break; // TIBETAN SUBJOINED LETTER GHA => TIBETAN SUBJOINED LETTER GA
    case 0x0f9d: r=0x0F9C; break; // TIBETAN SUBJOINED LETTER DDHA => TIBETAN SUBJOINED LETTER DDA
    case 0x0fa2: r=0x0FA1; break; // TIBETAN SUBJOINED LETTER DHA => TIBETAN SUBJOINED LETTER DA
    case 0x0fa7: r=0x0FA6; break; // TIBETAN SUBJOINED LETTER BHA => TIBETAN SUBJOINED LETTER BA
    case 0x0fac: r=0x0FAB; break; // TIBETAN SUBJOINED LETTER DZHA => TIBETAN SUBJOINED LETTER DZA
    case 0x0fb9: r=0x0F90; break; // TIBETAN SUBJOINED LETTER KSSA => TIBETAN SUBJOINED LETTER KA
    case 0x1026: r=0x1025; break; // MYANMAR LETTER UU => MYANMAR LETTER U
    case 0x1e00: r=0x0041; break; // LATIN CAPITAL LETTER A WITH RING BELOW => LATIN CAPITAL LETTER A
    case 0x1e01: r=0x0061; break; // LATIN SMALL LETTER A WITH RING BELOW => LATIN SMALL LETTER A
    case 0x1e02: r=0x0042; break; // LATIN CAPITAL LETTER B WITH DOT ABOVE => LATIN CAPITAL LETTER B
    case 0x1e03: r=0x0062; break; // LATIN SMALL LETTER B WITH DOT ABOVE => LATIN SMALL LETTER B
    case 0x1e04: r=0x0042; break; // LATIN CAPITAL LETTER B WITH DOT BELOW => LATIN CAPITAL LETTER B
    case 0x1e05: r=0x0062; break; // LATIN SMALL LETTER B WITH DOT BELOW => LATIN SMALL LETTER B
    case 0x1e06: r=0x0042; break; // LATIN CAPITAL LETTER B WITH LINE BELOW => LATIN CAPITAL LETTER B
    case 0x1e07: r=0x0062; break; // LATIN SMALL LETTER B WITH LINE BELOW => LATIN SMALL LETTER B
    case 0x1e08: r=0x0043; break; // LATIN CAPITAL LETTER C WITH CEDILLA AND ACUTE => LATIN CAPITAL LETTER C
    case 0x1e09: r=0x0063; break; // LATIN SMALL LETTER C WITH CEDILLA AND ACUTE => LATIN SMALL LETTER C
    case 0x1e0a: r=0x0044; break; // LATIN CAPITAL LETTER D WITH DOT ABOVE => LATIN CAPITAL LETTER D
    case 0x1e0b: r=0x0064; break; // LATIN SMALL LETTER D WITH DOT ABOVE => LATIN SMALL LETTER D
    case 0x1e0c: r=0x0044; break; // LATIN CAPITAL LETTER D WITH DOT BELOW => LATIN CAPITAL LETTER D
    case 0x1e0d: r=0x0064; break; // LATIN SMALL LETTER D WITH DOT BELOW => LATIN SMALL LETTER D
    case 0x1e0e: r=0x0044; break; // LATIN CAPITAL LETTER D WITH LINE BELOW => LATIN CAPITAL LETTER D
    case 0x1e0f: r=0x0064; break; // LATIN SMALL LETTER D WITH LINE BELOW => LATIN SMALL LETTER D
    case 0x1e10: r=0x0044; break; // LATIN CAPITAL LETTER D WITH CEDILLA => LATIN CAPITAL LETTER D
    case 0x1e11: r=0x0064; break; // LATIN SMALL LETTER D WITH CEDILLA => LATIN SMALL LETTER D
    case 0x1e12: r=0x0044; break; // LATIN CAPITAL LETTER D WITH CIRCUMFLEX BELOW => LATIN CAPITAL LETTER D
    case 0x1e13: r=0x0064; break; // LATIN SMALL LETTER D WITH CIRCUMFLEX BELOW => LATIN SMALL LETTER D
    case 0x1e14: r=0x0045; break; // LATIN CAPITAL LETTER E WITH MACRON AND GRAVE => LATIN CAPITAL LETTER E
    case 0x1e15: r=0x0065; break; // LATIN SMALL LETTER E WITH MACRON AND GRAVE => LATIN SMALL LETTER E
    case 0x1e16: r=0x0045; break; // LATIN CAPITAL LETTER E WITH MACRON AND ACUTE => LATIN CAPITAL LETTER E
    case 0x1e17: r=0x0065; break; // LATIN SMALL LETTER E WITH MACRON AND ACUTE => LATIN SMALL LETTER E
    case 0x1e18: r=0x0045; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX BELOW => LATIN CAPITAL LETTER E
    case 0x1e19: r=0x0065; break; // LATIN SMALL LETTER E WITH CIRCUMFLEX BELOW => LATIN SMALL LETTER E
    case 0x1e1a: r=0x0045; break; // LATIN CAPITAL LETTER E WITH TILDE BELOW => LATIN CAPITAL LETTER E
    case 0x1e1b: r=0x0065; break; // LATIN SMALL LETTER E WITH TILDE BELOW => LATIN SMALL LETTER E
    case 0x1e1c: r=0x0045; break; // LATIN CAPITAL LETTER E WITH CEDILLA AND BREVE => LATIN CAPITAL LETTER E
    case 0x1e1d: r=0x0065; break; // LATIN SMALL LETTER E WITH CEDILLA AND BREVE => LATIN SMALL LETTER E
    case 0x1e1e: r=0x0046; break; // LATIN CAPITAL LETTER F WITH DOT ABOVE => LATIN CAPITAL LETTER F
    case 0x1e1f: r=0x0066; break; // LATIN SMALL LETTER F WITH DOT ABOVE => LATIN SMALL LETTER F
    case 0x1e20: r=0x0047; break; // LATIN CAPITAL LETTER G WITH MACRON => LATIN CAPITAL LETTER G
    case 0x1e21: r=0x0067; break; // LATIN SMALL LETTER G WITH MACRON => LATIN SMALL LETTER G
    case 0x1e22: r=0x0048; break; // LATIN CAPITAL LETTER H WITH DOT ABOVE => LATIN CAPITAL LETTER H
    case 0x1e23: r=0x0068; break; // LATIN SMALL LETTER H WITH DOT ABOVE => LATIN SMALL LETTER H
    case 0x1e24: r=0x0048; break; // LATIN CAPITAL LETTER H WITH DOT BELOW => LATIN CAPITAL LETTER H
    case 0x1e25: r=0x0068; break; // LATIN SMALL LETTER H WITH DOT BELOW => LATIN SMALL LETTER H
    case 0x1e26: r=0x0048; break; // LATIN CAPITAL LETTER H WITH DIAERESIS => LATIN CAPITAL LETTER H
    case 0x1e27: r=0x0068; break; // LATIN SMALL LETTER H WITH DIAERESIS => LATIN SMALL LETTER H
    case 0x1e28: r=0x0048; break; // LATIN CAPITAL LETTER H WITH CEDILLA => LATIN CAPITAL LETTER H
    case 0x1e29: r=0x0068; break; // LATIN SMALL LETTER H WITH CEDILLA => LATIN SMALL LETTER H
    case 0x1e2a: r=0x0048; break; // LATIN CAPITAL LETTER H WITH BREVE BELOW => LATIN CAPITAL LETTER H
    case 0x1e2b: r=0x0068; break; // LATIN SMALL LETTER H WITH BREVE BELOW => LATIN SMALL LETTER H
    case 0x1e2c: r=0x0049; break; // LATIN CAPITAL LETTER I WITH TILDE BELOW => LATIN CAPITAL LETTER I
    case 0x1e2d: r=0x0069; break; // LATIN SMALL LETTER I WITH TILDE BELOW => LATIN SMALL LETTER I
    case 0x1e2e: r=0x0049; break; // LATIN CAPITAL LETTER I WITH DIAERESIS AND ACUTE => LATIN CAPITAL LETTER I
    case 0x1e2f: r=0x0069; break; // LATIN SMALL LETTER I WITH DIAERESIS AND ACUTE => LATIN SMALL LETTER I
    case 0x1e30: r=0x004B; break; // LATIN CAPITAL LETTER K WITH ACUTE => LATIN CAPITAL LETTER K
    case 0x1e31: r=0x006B; break; // LATIN SMALL LETTER K WITH ACUTE => LATIN SMALL LETTER K
    case 0x1e32: r=0x004B; break; // LATIN CAPITAL LETTER K WITH DOT BELOW => LATIN CAPITAL LETTER K
    case 0x1e33: r=0x006B; break; // LATIN SMALL LETTER K WITH DOT BELOW => LATIN SMALL LETTER K
    case 0x1e34: r=0x004B; break; // LATIN CAPITAL LETTER K WITH LINE BELOW => LATIN CAPITAL LETTER K
    case 0x1e35: r=0x006B; break; // LATIN SMALL LETTER K WITH LINE BELOW => LATIN SMALL LETTER K
    case 0x1e36: r=0x004C; break; // LATIN CAPITAL LETTER L WITH DOT BELOW => LATIN CAPITAL LETTER L
    case 0x1e37: r=0x006C; break; // LATIN SMALL LETTER L WITH DOT BELOW => LATIN SMALL LETTER L
    case 0x1e38: r=0x004C; break; // LATIN CAPITAL LETTER L WITH DOT BELOW AND MACRON => LATIN CAPITAL LETTER L
    case 0x1e39: r=0x006C; break; // LATIN SMALL LETTER L WITH DOT BELOW AND MACRON => LATIN SMALL LETTER L
    case 0x1e3a: r=0x004C; break; // LATIN CAPITAL LETTER L WITH LINE BELOW => LATIN CAPITAL LETTER L
    case 0x1e3b: r=0x006C; break; // LATIN SMALL LETTER L WITH LINE BELOW => LATIN SMALL LETTER L
    case 0x1e3c: r=0x004C; break; // LATIN CAPITAL LETTER L WITH CIRCUMFLEX BELOW => LATIN CAPITAL LETTER L
    case 0x1e3d: r=0x006C; break; // LATIN SMALL LETTER L WITH CIRCUMFLEX BELOW => LATIN SMALL LETTER L
    case 0x1e3e: r=0x004D; break; // LATIN CAPITAL LETTER M WITH ACUTE => LATIN CAPITAL LETTER M
    case 0x1e3f: r=0x006D; break; // LATIN SMALL LETTER M WITH ACUTE => LATIN SMALL LETTER M
    case 0x1e40: r=0x004D; break; // LATIN CAPITAL LETTER M WITH DOT ABOVE => LATIN CAPITAL LETTER M
    case 0x1e41: r=0x006D; break; // LATIN SMALL LETTER M WITH DOT ABOVE => LATIN SMALL LETTER M
    case 0x1e42: r=0x004D; break; // LATIN CAPITAL LETTER M WITH DOT BELOW => LATIN CAPITAL LETTER M
    case 0x1e43: r=0x006D; break; // LATIN SMALL LETTER M WITH DOT BELOW => LATIN SMALL LETTER M
    case 0x1e44: r=0x004E; break; // LATIN CAPITAL LETTER N WITH DOT ABOVE => LATIN CAPITAL LETTER N
    case 0x1e45: r=0x006E; break; // LATIN SMALL LETTER N WITH DOT ABOVE => LATIN SMALL LETTER N
    case 0x1e46: r=0x004E; break; // LATIN CAPITAL LETTER N WITH DOT BELOW => LATIN CAPITAL LETTER N
    case 0x1e47: r=0x006E; break; // LATIN SMALL LETTER N WITH DOT BELOW => LATIN SMALL LETTER N
    case 0x1e48: r=0x004E; break; // LATIN CAPITAL LETTER N WITH LINE BELOW => LATIN CAPITAL LETTER N
    case 0x1e49: r=0x006E; break; // LATIN SMALL LETTER N WITH LINE BELOW => LATIN SMALL LETTER N
    case 0x1e4a: r=0x004E; break; // LATIN CAPITAL LETTER N WITH CIRCUMFLEX BELOW => LATIN CAPITAL LETTER N
    case 0x1e4b: r=0x006E; break; // LATIN SMALL LETTER N WITH CIRCUMFLEX BELOW => LATIN SMALL LETTER N
    case 0x1e4c: r=0x004F; break; // LATIN CAPITAL LETTER O WITH TILDE AND ACUTE => LATIN CAPITAL LETTER O
    case 0x1e4d: r=0x006F; break; // LATIN SMALL LETTER O WITH TILDE AND ACUTE => LATIN SMALL LETTER O
    case 0x1e4e: r=0x004F; break; // LATIN CAPITAL LETTER O WITH TILDE AND DIAERESIS => LATIN CAPITAL LETTER O
    case 0x1e4f: r=0x006F; break; // LATIN SMALL LETTER O WITH TILDE AND DIAERESIS => LATIN SMALL LETTER O
    case 0x1e50: r=0x004F; break; // LATIN CAPITAL LETTER O WITH MACRON AND GRAVE => LATIN CAPITAL LETTER O
    case 0x1e51: r=0x006F; break; // LATIN SMALL LETTER O WITH MACRON AND GRAVE => LATIN SMALL LETTER O
    case 0x1e52: r=0x004F; break; // LATIN CAPITAL LETTER O WITH MACRON AND ACUTE => LATIN CAPITAL LETTER O
    case 0x1e53: r=0x006F; break; // LATIN SMALL LETTER O WITH MACRON AND ACUTE => LATIN SMALL LETTER O
    case 0x1e54: r=0x0050; break; // LATIN CAPITAL LETTER P WITH ACUTE => LATIN CAPITAL LETTER P
    case 0x1e55: r=0x0070; break; // LATIN SMALL LETTER P WITH ACUTE => LATIN SMALL LETTER P
    case 0x1e56: r=0x0050; break; // LATIN CAPITAL LETTER P WITH DOT ABOVE => LATIN CAPITAL LETTER P
    case 0x1e57: r=0x0070; break; // LATIN SMALL LETTER P WITH DOT ABOVE => LATIN SMALL LETTER P
    case 0x1e58: r=0x0052; break; // LATIN CAPITAL LETTER R WITH DOT ABOVE => LATIN CAPITAL LETTER R
    case 0x1e59: r=0x0072; break; // LATIN SMALL LETTER R WITH DOT ABOVE => LATIN SMALL LETTER R
    case 0x1e5a: r=0x0052; break; // LATIN CAPITAL LETTER R WITH DOT BELOW => LATIN CAPITAL LETTER R
    case 0x1e5b: r=0x0072; break; // LATIN SMALL LETTER R WITH DOT BELOW => LATIN SMALL LETTER R
    case 0x1e5c: r=0x0052; break; // LATIN CAPITAL LETTER R WITH DOT BELOW AND MACRON => LATIN CAPITAL LETTER R
    case 0x1e5d: r=0x0072; break; // LATIN SMALL LETTER R WITH DOT BELOW AND MACRON => LATIN SMALL LETTER R
    case 0x1e5e: r=0x0052; break; // LATIN CAPITAL LETTER R WITH LINE BELOW => LATIN CAPITAL LETTER R
    case 0x1e5f: r=0x0072; break; // LATIN SMALL LETTER R WITH LINE BELOW => LATIN SMALL LETTER R
    case 0x1e60: r=0x0053; break; // LATIN CAPITAL LETTER S WITH DOT ABOVE => LATIN CAPITAL LETTER S
    case 0x1e61: r=0x0073; break; // LATIN SMALL LETTER S WITH DOT ABOVE => LATIN SMALL LETTER S
    case 0x1e62: r=0x0053; break; // LATIN CAPITAL LETTER S WITH DOT BELOW => LATIN CAPITAL LETTER S
    case 0x1e63: r=0x0073; break; // LATIN SMALL LETTER S WITH DOT BELOW => LATIN SMALL LETTER S
    case 0x1e64: r=0x0053; break; // LATIN CAPITAL LETTER S WITH ACUTE AND DOT ABOVE => LATIN CAPITAL LETTER S
    case 0x1e65: r=0x0073; break; // LATIN SMALL LETTER S WITH ACUTE AND DOT ABOVE => LATIN SMALL LETTER S
    case 0x1e66: r=0x0053; break; // LATIN CAPITAL LETTER S WITH CARON AND DOT ABOVE => LATIN CAPITAL LETTER S
    case 0x1e67: r=0x0073; break; // LATIN SMALL LETTER S WITH CARON AND DOT ABOVE => LATIN SMALL LETTER S
    case 0x1e68: r=0x0053; break; // LATIN CAPITAL LETTER S WITH DOT BELOW AND DOT ABOVE => LATIN CAPITAL LETTER S
    case 0x1e69: r=0x0073; break; // LATIN SMALL LETTER S WITH DOT BELOW AND DOT ABOVE => LATIN SMALL LETTER S
    case 0x1e6a: r=0x0054; break; // LATIN CAPITAL LETTER T WITH DOT ABOVE => LATIN CAPITAL LETTER T
    case 0x1e6b: r=0x0074; break; // LATIN SMALL LETTER T WITH DOT ABOVE => LATIN SMALL LETTER T
    case 0x1e6c: r=0x0054; break; // LATIN CAPITAL LETTER T WITH DOT BELOW => LATIN CAPITAL LETTER T
    case 0x1e6d: r=0x0074; break; // LATIN SMALL LETTER T WITH DOT BELOW => LATIN SMALL LETTER T
    case 0x1e6e: r=0x0054; break; // LATIN CAPITAL LETTER T WITH LINE BELOW => LATIN CAPITAL LETTER T
    case 0x1e6f: r=0x0074; break; // LATIN SMALL LETTER T WITH LINE BELOW => LATIN SMALL LETTER T
    case 0x1e70: r=0x0054; break; // LATIN CAPITAL LETTER T WITH CIRCUMFLEX BELOW => LATIN CAPITAL LETTER T
    case 0x1e71: r=0x0074; break; // LATIN SMALL LETTER T WITH CIRCUMFLEX BELOW => LATIN SMALL LETTER T
    case 0x1e72: r=0x0055; break; // LATIN CAPITAL LETTER U WITH DIAERESIS BELOW => LATIN CAPITAL LETTER U
    case 0x1e73: r=0x0075; break; // LATIN SMALL LETTER U WITH DIAERESIS BELOW => LATIN SMALL LETTER U
    case 0x1e74: r=0x0055; break; // LATIN CAPITAL LETTER U WITH TILDE BELOW => LATIN CAPITAL LETTER U
    case 0x1e75: r=0x0075; break; // LATIN SMALL LETTER U WITH TILDE BELOW => LATIN SMALL LETTER U
    case 0x1e76: r=0x0055; break; // LATIN CAPITAL LETTER U WITH CIRCUMFLEX BELOW => LATIN CAPITAL LETTER U
    case 0x1e77: r=0x0075; break; // LATIN SMALL LETTER U WITH CIRCUMFLEX BELOW => LATIN SMALL LETTER U
    case 0x1e78: r=0x0055; break; // LATIN CAPITAL LETTER U WITH TILDE AND ACUTE => LATIN CAPITAL LETTER U
    case 0x1e79: r=0x0075; break; // LATIN SMALL LETTER U WITH TILDE AND ACUTE => LATIN SMALL LETTER U
    case 0x1e7a: r=0x0055; break; // LATIN CAPITAL LETTER U WITH MACRON AND DIAERESIS => LATIN CAPITAL LETTER U
    case 0x1e7b: r=0x0075; break; // LATIN SMALL LETTER U WITH MACRON AND DIAERESIS => LATIN SMALL LETTER U
    case 0x1e7c: r=0x0056; break; // LATIN CAPITAL LETTER V WITH TILDE => LATIN CAPITAL LETTER V
    case 0x1e7d: r=0x0076; break; // LATIN SMALL LETTER V WITH TILDE => LATIN SMALL LETTER V
    case 0x1e7e: r=0x0056; break; // LATIN CAPITAL LETTER V WITH DOT BELOW => LATIN CAPITAL LETTER V
    case 0x1e7f: r=0x0076; break; // LATIN SMALL LETTER V WITH DOT BELOW => LATIN SMALL LETTER V
    case 0x1e80: r=0x0057; break; // LATIN CAPITAL LETTER W WITH GRAVE => LATIN CAPITAL LETTER W
    case 0x1e81: r=0x0077; break; // LATIN SMALL LETTER W WITH GRAVE => LATIN SMALL LETTER W
    case 0x1e82: r=0x0057; break; // LATIN CAPITAL LETTER W WITH ACUTE => LATIN CAPITAL LETTER W
    case 0x1e83: r=0x0077; break; // LATIN SMALL LETTER W WITH ACUTE => LATIN SMALL LETTER W
    case 0x1e84: r=0x0057; break; // LATIN CAPITAL LETTER W WITH DIAERESIS => LATIN CAPITAL LETTER W
    case 0x1e85: r=0x0077; break; // LATIN SMALL LETTER W WITH DIAERESIS => LATIN SMALL LETTER W
    case 0x1e86: r=0x0057; break; // LATIN CAPITAL LETTER W WITH DOT ABOVE => LATIN CAPITAL LETTER W
    case 0x1e87: r=0x0077; break; // LATIN SMALL LETTER W WITH DOT ABOVE => LATIN SMALL LETTER W
    case 0x1e88: r=0x0057; break; // LATIN CAPITAL LETTER W WITH DOT BELOW => LATIN CAPITAL LETTER W
    case 0x1e89: r=0x0077; break; // LATIN SMALL LETTER W WITH DOT BELOW => LATIN SMALL LETTER W
    case 0x1e8a: r=0x0058; break; // LATIN CAPITAL LETTER X WITH DOT ABOVE => LATIN CAPITAL LETTER X
    case 0x1e8b: r=0x0078; break; // LATIN SMALL LETTER X WITH DOT ABOVE => LATIN SMALL LETTER X
    case 0x1e8c: r=0x0058; break; // LATIN CAPITAL LETTER X WITH DIAERESIS => LATIN CAPITAL LETTER X
    case 0x1e8d: r=0x0078; break; // LATIN SMALL LETTER X WITH DIAERESIS => LATIN SMALL LETTER X
    case 0x1e8e: r=0x0059; break; // LATIN CAPITAL LETTER Y WITH DOT ABOVE => LATIN CAPITAL LETTER Y
    case 0x1e8f: r=0x0079; break; // LATIN SMALL LETTER Y WITH DOT ABOVE => LATIN SMALL LETTER Y
    case 0x1e90: r=0x005A; break; // LATIN CAPITAL LETTER Z WITH CIRCUMFLEX => LATIN CAPITAL LETTER Z
    case 0x1e91: r=0x007A; break; // LATIN SMALL LETTER Z WITH CIRCUMFLEX => LATIN SMALL LETTER Z
    case 0x1e92: r=0x005A; break; // LATIN CAPITAL LETTER Z WITH DOT BELOW => LATIN CAPITAL LETTER Z
    case 0x1e93: r=0x007A; break; // LATIN SMALL LETTER Z WITH DOT BELOW => LATIN SMALL LETTER Z
    case 0x1e94: r=0x005A; break; // LATIN CAPITAL LETTER Z WITH LINE BELOW => LATIN CAPITAL LETTER Z
    case 0x1e95: r=0x007A; break; // LATIN SMALL LETTER Z WITH LINE BELOW => LATIN SMALL LETTER Z
    case 0x1e96: r=0x0068; break; // LATIN SMALL LETTER H WITH LINE BELOW => LATIN SMALL LETTER H
    case 0x1e97: r=0x0074; break; // LATIN SMALL LETTER T WITH DIAERESIS => LATIN SMALL LETTER T
    case 0x1e98: r=0x0077; break; // LATIN SMALL LETTER W WITH RING ABOVE => LATIN SMALL LETTER W
    case 0x1e99: r=0x0079; break; // LATIN SMALL LETTER Y WITH RING ABOVE => LATIN SMALL LETTER Y
    case 0x1e9a: r=0x61; break; // LATIN SMALL LETTER A WITH RIGHT HALF RING =>
    case 0x1e9b: r=0x017F; break; // LATIN SMALL LETTER LONG S WITH DOT ABOVE => LATIN SMALL LETTER LONG S
    case 0x1ea0: r=0x0041; break; // LATIN CAPITAL LETTER A WITH DOT BELOW => LATIN CAPITAL LETTER A
    case 0x1ea1: r=0x0061; break; // LATIN SMALL LETTER A WITH DOT BELOW => LATIN SMALL LETTER A
    case 0x1ea2: r=0x0041; break; // LATIN CAPITAL LETTER A WITH HOOK ABOVE => LATIN CAPITAL LETTER A
    case 0x1ea3: r=0x0061; break; // LATIN SMALL LETTER A WITH HOOK ABOVE => LATIN SMALL LETTER A
    case 0x1ea4: r=0x0041; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND ACUTE => LATIN CAPITAL LETTER A
    case 0x1ea5: r=0x0061; break; // LATIN SMALL LETTER A WITH CIRCUMFLEX AND ACUTE => LATIN SMALL LETTER A
    case 0x1ea6: r=0x0041; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND GRAVE => LATIN CAPITAL LETTER A
    case 0x1ea7: r=0x0061; break; // LATIN SMALL LETTER A WITH CIRCUMFLEX AND GRAVE => LATIN SMALL LETTER A
    case 0x1ea8: r=0x0041; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND HOOK ABOVE => LATIN CAPITAL LETTER A
    case 0x1ea9: r=0x0061; break; // LATIN SMALL LETTER A WITH CIRCUMFLEX AND HOOK ABOVE => LATIN SMALL LETTER A
    case 0x1eaa: r=0x0041; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND TILDE => LATIN CAPITAL LETTER A
    case 0x1eab: r=0x0061; break; // LATIN SMALL LETTER A WITH CIRCUMFLEX AND TILDE => LATIN SMALL LETTER A
    case 0x1eac: r=0x0041; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND DOT BELOW => LATIN CAPITAL LETTER A
    case 0x1ead: r=0x0061; break; // LATIN SMALL LETTER A WITH CIRCUMFLEX AND DOT BELOW => LATIN SMALL LETTER A
    case 0x1eae: r=0x0041; break; // LATIN CAPITAL LETTER A WITH BREVE AND ACUTE => LATIN CAPITAL LETTER A
    case 0x1eaf: r=0x0061; break; // LATIN SMALL LETTER A WITH BREVE AND ACUTE => LATIN SMALL LETTER A
    case 0x1eb0: r=0x0041; break; // LATIN CAPITAL LETTER A WITH BREVE AND GRAVE => LATIN CAPITAL LETTER A
    case 0x1eb1: r=0x0061; break; // LATIN SMALL LETTER A WITH BREVE AND GRAVE => LATIN SMALL LETTER A
    case 0x1eb2: r=0x0041; break; // LATIN CAPITAL LETTER A WITH BREVE AND HOOK ABOVE => LATIN CAPITAL LETTER A
    case 0x1eb3: r=0x0061; break; // LATIN SMALL LETTER A WITH BREVE AND HOOK ABOVE => LATIN SMALL LETTER A
    case 0x1eb4: r=0x0041; break; // LATIN CAPITAL LETTER A WITH BREVE AND TILDE => LATIN CAPITAL LETTER A
    case 0x1eb5: r=0x0061; break; // LATIN SMALL LETTER A WITH BREVE AND TILDE => LATIN SMALL LETTER A
    case 0x1eb6: r=0x0041; break; // LATIN CAPITAL LETTER A WITH BREVE AND DOT BELOW => LATIN CAPITAL LETTER A
    case 0x1eb7: r=0x0061; break; // LATIN SMALL LETTER A WITH BREVE AND DOT BELOW => LATIN SMALL LETTER A
    case 0x1eb8: r=0x0045; break; // LATIN CAPITAL LETTER E WITH DOT BELOW => LATIN CAPITAL LETTER E
    case 0x1eb9: r=0x0065; break; // LATIN SMALL LETTER E WITH DOT BELOW => LATIN SMALL LETTER E
    case 0x1eba: r=0x0045; break; // LATIN CAPITAL LETTER E WITH HOOK ABOVE => LATIN CAPITAL LETTER E
    case 0x1ebb: r=0x0065; break; // LATIN SMALL LETTER E WITH HOOK ABOVE => LATIN SMALL LETTER E
    case 0x1ebc: r=0x0045; break; // LATIN CAPITAL LETTER E WITH TILDE => LATIN CAPITAL LETTER E
    case 0x1ebd: r=0x0065; break; // LATIN SMALL LETTER E WITH TILDE => LATIN SMALL LETTER E
    case 0x1ebe: r=0x0045; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND ACUTE => LATIN CAPITAL LETTER E
    case 0x1ebf: r=0x0065; break; // LATIN SMALL LETTER E WITH CIRCUMFLEX AND ACUTE => LATIN SMALL LETTER E
    case 0x1ec0: r=0x0045; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND GRAVE => LATIN CAPITAL LETTER E
    case 0x1ec1: r=0x0065; break; // LATIN SMALL LETTER E WITH CIRCUMFLEX AND GRAVE => LATIN SMALL LETTER E
    case 0x1ec2: r=0x0045; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND HOOK ABOVE => LATIN CAPITAL LETTER E
    case 0x1ec3: r=0x0065; break; // LATIN SMALL LETTER E WITH CIRCUMFLEX AND HOOK ABOVE => LATIN SMALL LETTER E
    case 0x1ec4: r=0x0045; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND TILDE => LATIN CAPITAL LETTER E
    case 0x1ec5: r=0x0065; break; // LATIN SMALL LETTER E WITH CIRCUMFLEX AND TILDE => LATIN SMALL LETTER E
    case 0x1ec6: r=0x0045; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND DOT BELOW => LATIN CAPITAL LETTER E
    case 0x1ec7: r=0x0065; break; // LATIN SMALL LETTER E WITH CIRCUMFLEX AND DOT BELOW => LATIN SMALL LETTER E
    case 0x1ec8: r=0x0049; break; // LATIN CAPITAL LETTER I WITH HOOK ABOVE => LATIN CAPITAL LETTER I
    case 0x1ec9: r=0x0069; break; // LATIN SMALL LETTER I WITH HOOK ABOVE => LATIN SMALL LETTER I
    case 0x1eca: r=0x0049; break; // LATIN CAPITAL LETTER I WITH DOT BELOW => LATIN CAPITAL LETTER I
    case 0x1ecb: r=0x0069; break; // LATIN SMALL LETTER I WITH DOT BELOW => LATIN SMALL LETTER I
    case 0x1ecc: r=0x004F; break; // LATIN CAPITAL LETTER O WITH DOT BELOW => LATIN CAPITAL LETTER O
    case 0x1ecd: r=0x006F; break; // LATIN SMALL LETTER O WITH DOT BELOW => LATIN SMALL LETTER O
    case 0x1ece: r=0x004F; break; // LATIN CAPITAL LETTER O WITH HOOK ABOVE => LATIN CAPITAL LETTER O
    case 0x1ecf: r=0x006F; break; // LATIN SMALL LETTER O WITH HOOK ABOVE => LATIN SMALL LETTER O
    case 0x1ed0: r=0x004F; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND ACUTE => LATIN CAPITAL LETTER O
    case 0x1ed1: r=0x006F; break; // LATIN SMALL LETTER O WITH CIRCUMFLEX AND ACUTE => LATIN SMALL LETTER O
    case 0x1ed2: r=0x004F; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND GRAVE => LATIN CAPITAL LETTER O
    case 0x1ed3: r=0x006F; break; // LATIN SMALL LETTER O WITH CIRCUMFLEX AND GRAVE => LATIN SMALL LETTER O
    case 0x1ed4: r=0x004F; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND HOOK ABOVE => LATIN CAPITAL LETTER O
    case 0x1ed5: r=0x006F; break; // LATIN SMALL LETTER O WITH CIRCUMFLEX AND HOOK ABOVE => LATIN SMALL LETTER O
    case 0x1ed6: r=0x004F; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND TILDE => LATIN CAPITAL LETTER O
    case 0x1ed7: r=0x006F; break; // LATIN SMALL LETTER O WITH CIRCUMFLEX AND TILDE => LATIN SMALL LETTER O
    case 0x1ed8: r=0x004F; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND DOT BELOW => LATIN CAPITAL LETTER O
    case 0x1ed9: r=0x006F; break; // LATIN SMALL LETTER O WITH CIRCUMFLEX AND DOT BELOW => LATIN SMALL LETTER O
    case 0x1eda: r=0x004F; break; // LATIN CAPITAL LETTER O WITH HORN AND ACUTE => LATIN CAPITAL LETTER O
    case 0x1edb: r=0x006F; break; // LATIN SMALL LETTER O WITH HORN AND ACUTE => LATIN SMALL LETTER O
    case 0x1edc: r=0x004F; break; // LATIN CAPITAL LETTER O WITH HORN AND GRAVE => LATIN CAPITAL LETTER O
    case 0x1edd: r=0x006F; break; // LATIN SMALL LETTER O WITH HORN AND GRAVE => LATIN SMALL LETTER O
    case 0x1ede: r=0x004F; break; // LATIN CAPITAL LETTER O WITH HORN AND HOOK ABOVE => LATIN CAPITAL LETTER O
    case 0x1edf: r=0x006F; break; // LATIN SMALL LETTER O WITH HORN AND HOOK ABOVE => LATIN SMALL LETTER O
    case 0x1ee0: r=0x004F; break; // LATIN CAPITAL LETTER O WITH HORN AND TILDE => LATIN CAPITAL LETTER O
    case 0x1ee1: r=0x006F; break; // LATIN SMALL LETTER O WITH HORN AND TILDE => LATIN SMALL LETTER O
    case 0x1ee2: r=0x004F; break; // LATIN CAPITAL LETTER O WITH HORN AND DOT BELOW => LATIN CAPITAL LETTER O
    case 0x1ee3: r=0x006F; break; // LATIN SMALL LETTER O WITH HORN AND DOT BELOW => LATIN SMALL LETTER O
    case 0x1ee4: r=0x0055; break; // LATIN CAPITAL LETTER U WITH DOT BELOW => LATIN CAPITAL LETTER U
    case 0x1ee5: r=0x0075; break; // LATIN SMALL LETTER U WITH DOT BELOW => LATIN SMALL LETTER U
    case 0x1ee6: r=0x0055; break; // LATIN CAPITAL LETTER U WITH HOOK ABOVE => LATIN CAPITAL LETTER U
    case 0x1ee7: r=0x0075; break; // LATIN SMALL LETTER U WITH HOOK ABOVE => LATIN SMALL LETTER U
    case 0x1ee8: r=0x0055; break; // LATIN CAPITAL LETTER U WITH HORN AND ACUTE => LATIN CAPITAL LETTER U
    case 0x1ee9: r=0x0075; break; // LATIN SMALL LETTER U WITH HORN AND ACUTE => LATIN SMALL LETTER U
    case 0x1eea: r=0x0055; break; // LATIN CAPITAL LETTER U WITH HORN AND GRAVE => LATIN CAPITAL LETTER U
    case 0x1eeb: r=0x0075; break; // LATIN SMALL LETTER U WITH HORN AND GRAVE => LATIN SMALL LETTER U
    case 0x1eec: r=0x0055; break; // LATIN CAPITAL LETTER U WITH HORN AND HOOK ABOVE => LATIN CAPITAL LETTER U
    case 0x1eed: r=0x0075; break; // LATIN SMALL LETTER U WITH HORN AND HOOK ABOVE => LATIN SMALL LETTER U
    case 0x1eee: r=0x0055; break; // LATIN CAPITAL LETTER U WITH HORN AND TILDE => LATIN CAPITAL LETTER U
    case 0x1eef: r=0x0075; break; // LATIN SMALL LETTER U WITH HORN AND TILDE => LATIN SMALL LETTER U
    case 0x1ef0: r=0x0055; break; // LATIN CAPITAL LETTER U WITH HORN AND DOT BELOW => LATIN CAPITAL LETTER U
    case 0x1ef1: r=0x0075; break; // LATIN SMALL LETTER U WITH HORN AND DOT BELOW => LATIN SMALL LETTER U
    case 0x1ef2: r=0x0059; break; // LATIN CAPITAL LETTER Y WITH GRAVE => LATIN CAPITAL LETTER Y
    case 0x1ef3: r=0x0079; break; // LATIN SMALL LETTER Y WITH GRAVE => LATIN SMALL LETTER Y
    case 0x1ef4: r=0x0059; break; // LATIN CAPITAL LETTER Y WITH DOT BELOW => LATIN CAPITAL LETTER Y
    case 0x1ef5: r=0x0079; break; // LATIN SMALL LETTER Y WITH DOT BELOW => LATIN SMALL LETTER Y
    case 0x1ef6: r=0x0059; break; // LATIN CAPITAL LETTER Y WITH HOOK ABOVE => LATIN CAPITAL LETTER Y
    case 0x1ef7: r=0x0079; break; // LATIN SMALL LETTER Y WITH HOOK ABOVE => LATIN SMALL LETTER Y
    case 0x1ef8: r=0x0059; break; // LATIN CAPITAL LETTER Y WITH TILDE => LATIN CAPITAL LETTER Y
    case 0x1ef9: r=0x0079; break; // LATIN SMALL LETTER Y WITH TILDE => LATIN SMALL LETTER Y
    case 0x1f00: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH PSILI => GREEK SMALL LETTER ALPHA
    case 0x1f01: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH DASIA => GREEK SMALL LETTER ALPHA
    case 0x1f02: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH PSILI AND VARIA => GREEK SMALL LETTER ALPHA
    case 0x1f03: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH DASIA AND VARIA => GREEK SMALL LETTER ALPHA
    case 0x1f04: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH PSILI AND OXIA => GREEK SMALL LETTER ALPHA
    case 0x1f05: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH DASIA AND OXIA => GREEK SMALL LETTER ALPHA
    case 0x1f06: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH PSILI AND PERISPOMENI => GREEK SMALL LETTER ALPHA
    case 0x1f07: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH DASIA AND PERISPOMENI => GREEK SMALL LETTER ALPHA
    case 0x1f08: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI => GREEK CAPITAL LETTER ALPHA
    case 0x1f09: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA => GREEK CAPITAL LETTER ALPHA
    case 0x1f0a: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND VARIA => GREEK CAPITAL LETTER ALPHA
    case 0x1f0b: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND VARIA => GREEK CAPITAL LETTER ALPHA
    case 0x1f0c: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND OXIA => GREEK CAPITAL LETTER ALPHA
    case 0x1f0d: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND OXIA => GREEK CAPITAL LETTER ALPHA
    case 0x1f0e: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PERISPOMENI => GREEK CAPITAL LETTER ALPHA
    case 0x1f0f: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PERISPOMENI => GREEK CAPITAL LETTER ALPHA
    case 0x1f10: r=0x03B5; break; // GREEK SMALL LETTER EPSILON WITH PSILI => GREEK SMALL LETTER EPSILON
    case 0x1f11: r=0x03B5; break; // GREEK SMALL LETTER EPSILON WITH DASIA => GREEK SMALL LETTER EPSILON
    case 0x1f12: r=0x03B5; break; // GREEK SMALL LETTER EPSILON WITH PSILI AND VARIA => GREEK SMALL LETTER EPSILON
    case 0x1f13: r=0x03B5; break; // GREEK SMALL LETTER EPSILON WITH DASIA AND VARIA => GREEK SMALL LETTER EPSILON
    case 0x1f14: r=0x03B5; break; // GREEK SMALL LETTER EPSILON WITH PSILI AND OXIA => GREEK SMALL LETTER EPSILON
    case 0x1f15: r=0x03B5; break; // GREEK SMALL LETTER EPSILON WITH DASIA AND OXIA => GREEK SMALL LETTER EPSILON
    case 0x1f18: r=0x0395; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI => GREEK CAPITAL LETTER EPSILON
    case 0x1f19: r=0x0395; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA => GREEK CAPITAL LETTER EPSILON
    case 0x1f1a: r=0x0395; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI AND VARIA => GREEK CAPITAL LETTER EPSILON
    case 0x1f1b: r=0x0395; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA AND VARIA => GREEK CAPITAL LETTER EPSILON
    case 0x1f1c: r=0x0395; break; // GREEK CAPITAL LETTER EPSILON WITH PSILI AND OXIA => GREEK CAPITAL LETTER EPSILON
    case 0x1f1d: r=0x0395; break; // GREEK CAPITAL LETTER EPSILON WITH DASIA AND OXIA => GREEK CAPITAL LETTER EPSILON
    case 0x1f20: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH PSILI => GREEK SMALL LETTER ETA
    case 0x1f21: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH DASIA => GREEK SMALL LETTER ETA
    case 0x1f22: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH PSILI AND VARIA => GREEK SMALL LETTER ETA
    case 0x1f23: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH DASIA AND VARIA => GREEK SMALL LETTER ETA
    case 0x1f24: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH PSILI AND OXIA => GREEK SMALL LETTER ETA
    case 0x1f25: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH DASIA AND OXIA => GREEK SMALL LETTER ETA
    case 0x1f26: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH PSILI AND PERISPOMENI => GREEK SMALL LETTER ETA
    case 0x1f27: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH DASIA AND PERISPOMENI => GREEK SMALL LETTER ETA
    case 0x1f28: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH PSILI => GREEK CAPITAL LETTER ETA
    case 0x1f29: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH DASIA => GREEK CAPITAL LETTER ETA
    case 0x1f2a: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND VARIA => GREEK CAPITAL LETTER ETA
    case 0x1f2b: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND VARIA => GREEK CAPITAL LETTER ETA
    case 0x1f2c: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND OXIA => GREEK CAPITAL LETTER ETA
    case 0x1f2d: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND OXIA => GREEK CAPITAL LETTER ETA
    case 0x1f2e: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PERISPOMENI => GREEK CAPITAL LETTER ETA
    case 0x1f2f: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PERISPOMENI => GREEK CAPITAL LETTER ETA
    case 0x1f30: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH PSILI => GREEK SMALL LETTER IOTA
    case 0x1f31: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH DASIA => GREEK SMALL LETTER IOTA
    case 0x1f32: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH PSILI AND VARIA => GREEK SMALL LETTER IOTA
    case 0x1f33: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH DASIA AND VARIA => GREEK SMALL LETTER IOTA
    case 0x1f34: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH PSILI AND OXIA => GREEK SMALL LETTER IOTA
    case 0x1f35: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH DASIA AND OXIA => GREEK SMALL LETTER IOTA
    case 0x1f36: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH PSILI AND PERISPOMENI => GREEK SMALL LETTER IOTA
    case 0x1f37: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH DASIA AND PERISPOMENI => GREEK SMALL LETTER IOTA
    case 0x1f38: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH PSILI => GREEK CAPITAL LETTER IOTA
    case 0x1f39: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH DASIA => GREEK CAPITAL LETTER IOTA
    case 0x1f3a: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND VARIA => GREEK CAPITAL LETTER IOTA
    case 0x1f3b: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND VARIA => GREEK CAPITAL LETTER IOTA
    case 0x1f3c: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND OXIA => GREEK CAPITAL LETTER IOTA
    case 0x1f3d: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND OXIA => GREEK CAPITAL LETTER IOTA
    case 0x1f3e: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH PSILI AND PERISPOMENI => GREEK CAPITAL LETTER IOTA
    case 0x1f3f: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH DASIA AND PERISPOMENI => GREEK CAPITAL LETTER IOTA
    case 0x1f40: r=0x03BF; break; // GREEK SMALL LETTER OMICRON WITH PSILI => GREEK SMALL LETTER OMICRON
    case 0x1f41: r=0x03BF; break; // GREEK SMALL LETTER OMICRON WITH DASIA => GREEK SMALL LETTER OMICRON
    case 0x1f42: r=0x03BF; break; // GREEK SMALL LETTER OMICRON WITH PSILI AND VARIA => GREEK SMALL LETTER OMICRON
    case 0x1f43: r=0x03BF; break; // GREEK SMALL LETTER OMICRON WITH DASIA AND VARIA => GREEK SMALL LETTER OMICRON
    case 0x1f44: r=0x03BF; break; // GREEK SMALL LETTER OMICRON WITH PSILI AND OXIA => GREEK SMALL LETTER OMICRON
    case 0x1f45: r=0x03BF; break; // GREEK SMALL LETTER OMICRON WITH DASIA AND OXIA => GREEK SMALL LETTER OMICRON
    case 0x1f48: r=0x039F; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI => GREEK CAPITAL LETTER OMICRON
    case 0x1f49: r=0x039F; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA => GREEK CAPITAL LETTER OMICRON
    case 0x1f4a: r=0x039F; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI AND VARIA => GREEK CAPITAL LETTER OMICRON
    case 0x1f4b: r=0x039F; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA AND VARIA => GREEK CAPITAL LETTER OMICRON
    case 0x1f4c: r=0x039F; break; // GREEK CAPITAL LETTER OMICRON WITH PSILI AND OXIA => GREEK CAPITAL LETTER OMICRON
    case 0x1f4d: r=0x039F; break; // GREEK CAPITAL LETTER OMICRON WITH DASIA AND OXIA => GREEK CAPITAL LETTER OMICRON
    case 0x1f50: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH PSILI => GREEK SMALL LETTER UPSILON
    case 0x1f51: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH DASIA => GREEK SMALL LETTER UPSILON
    case 0x1f52: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH PSILI AND VARIA => GREEK SMALL LETTER UPSILON
    case 0x1f53: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH DASIA AND VARIA => GREEK SMALL LETTER UPSILON
    case 0x1f54: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH PSILI AND OXIA => GREEK SMALL LETTER UPSILON
    case 0x1f55: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH DASIA AND OXIA => GREEK SMALL LETTER UPSILON
    case 0x1f56: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH PSILI AND PERISPOMENI => GREEK SMALL LETTER UPSILON
    case 0x1f57: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH DASIA AND PERISPOMENI => GREEK SMALL LETTER UPSILON
    case 0x1f59: r=0x03A5; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA => GREEK CAPITAL LETTER UPSILON
    case 0x1f5b: r=0x03A5; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND VARIA => GREEK CAPITAL LETTER UPSILON
    case 0x1f5d: r=0x03A5; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND OXIA => GREEK CAPITAL LETTER UPSILON
    case 0x1f5f: r=0x03A5; break; // GREEK CAPITAL LETTER UPSILON WITH DASIA AND PERISPOMENI => GREEK CAPITAL LETTER UPSILON
    case 0x1f60: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH PSILI => GREEK SMALL LETTER OMEGA
    case 0x1f61: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH DASIA => GREEK SMALL LETTER OMEGA
    case 0x1f62: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH PSILI AND VARIA => GREEK SMALL LETTER OMEGA
    case 0x1f63: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH DASIA AND VARIA => GREEK SMALL LETTER OMEGA
    case 0x1f64: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH PSILI AND OXIA => GREEK SMALL LETTER OMEGA
    case 0x1f65: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH DASIA AND OXIA => GREEK SMALL LETTER OMEGA
    case 0x1f66: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH PSILI AND PERISPOMENI => GREEK SMALL LETTER OMEGA
    case 0x1f67: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH DASIA AND PERISPOMENI => GREEK SMALL LETTER OMEGA
    case 0x1f68: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI => GREEK CAPITAL LETTER OMEGA
    case 0x1f69: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA => GREEK CAPITAL LETTER OMEGA
    case 0x1f6a: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND VARIA => GREEK CAPITAL LETTER OMEGA
    case 0x1f6b: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND VARIA => GREEK CAPITAL LETTER OMEGA
    case 0x1f6c: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND OXIA => GREEK CAPITAL LETTER OMEGA
    case 0x1f6d: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND OXIA => GREEK CAPITAL LETTER OMEGA
    case 0x1f6e: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PERISPOMENI => GREEK CAPITAL LETTER OMEGA
    case 0x1f6f: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PERISPOMENI => GREEK CAPITAL LETTER OMEGA
    case 0x1f70: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH VARIA => GREEK SMALL LETTER ALPHA
    case 0x1f71: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH OXIA => GREEK SMALL LETTER ALPHA
    case 0x1f72: r=0x03B5; break; // GREEK SMALL LETTER EPSILON WITH VARIA => GREEK SMALL LETTER EPSILON
    case 0x1f73: r=0x03B5; break; // GREEK SMALL LETTER EPSILON WITH OXIA => GREEK SMALL LETTER EPSILON
    case 0x1f74: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH VARIA => GREEK SMALL LETTER ETA
    case 0x1f75: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH OXIA => GREEK SMALL LETTER ETA
    case 0x1f76: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH VARIA => GREEK SMALL LETTER IOTA
    case 0x1f77: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH OXIA => GREEK SMALL LETTER IOTA
    case 0x1f78: r=0x03BF; break; // GREEK SMALL LETTER OMICRON WITH VARIA => GREEK SMALL LETTER OMICRON
    case 0x1f79: r=0x03BF; break; // GREEK SMALL LETTER OMICRON WITH OXIA => GREEK SMALL LETTER OMICRON
    case 0x1f7a: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH VARIA => GREEK SMALL LETTER UPSILON
    case 0x1f7b: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH OXIA => GREEK SMALL LETTER UPSILON
    case 0x1f7c: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH VARIA => GREEK SMALL LETTER OMEGA
    case 0x1f7d: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH OXIA => GREEK SMALL LETTER OMEGA
    case 0x1f80: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH PSILI AND YPOGEGRAMMENI => GREEK SMALL LETTER ALPHA
    case 0x1f81: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH DASIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ALPHA
    case 0x1f82: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH PSILI AND VARIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ALPHA
    case 0x1f83: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH DASIA AND VARIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ALPHA
    case 0x1f84: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH PSILI AND OXIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ALPHA
    case 0x1f85: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH DASIA AND OXIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ALPHA
    case 0x1f86: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH PSILI AND PERISPOMENI AND YPOGEGRAMMENI => GREEK SMALL LETTER ALPHA
    case 0x1f87: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH DASIA AND PERISPOMENI AND YPOGEGRAMMENI => GREEK SMALL LETTER ALPHA
    case 0x1f88: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ALPHA
    case 0x1f89: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ALPHA
    case 0x1f8a: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND VARIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ALPHA
    case 0x1f8b: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND VARIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ALPHA
    case 0x1f8c: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND OXIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ALPHA
    case 0x1f8d: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND OXIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ALPHA
    case 0x1f8e: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ALPHA
    case 0x1f8f: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ALPHA
    case 0x1f90: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH PSILI AND YPOGEGRAMMENI => GREEK SMALL LETTER ETA
    case 0x1f91: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH DASIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ETA
    case 0x1f92: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH PSILI AND VARIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ETA
    case 0x1f93: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH DASIA AND VARIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ETA
    case 0x1f94: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH PSILI AND OXIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ETA
    case 0x1f95: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH DASIA AND OXIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ETA
    case 0x1f96: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH PSILI AND PERISPOMENI AND YPOGEGRAMMENI => GREEK SMALL LETTER ETA
    case 0x1f97: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH DASIA AND PERISPOMENI AND YPOGEGRAMMENI => GREEK SMALL LETTER ETA
    case 0x1f98: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ETA
    case 0x1f99: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ETA
    case 0x1f9a: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND VARIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ETA
    case 0x1f9b: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND VARIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ETA
    case 0x1f9c: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND OXIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ETA
    case 0x1f9d: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND OXIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ETA
    case 0x1f9e: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ETA
    case 0x1f9f: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI => GREEK CAPITAL LETTER ETA
    case 0x1fa0: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH PSILI AND YPOGEGRAMMENI => GREEK SMALL LETTER OMEGA
    case 0x1fa1: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH DASIA AND YPOGEGRAMMENI => GREEK SMALL LETTER OMEGA
    case 0x1fa2: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH PSILI AND VARIA AND YPOGEGRAMMENI => GREEK SMALL LETTER OMEGA
    case 0x1fa3: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH DASIA AND VARIA AND YPOGEGRAMMENI => GREEK SMALL LETTER OMEGA
    case 0x1fa4: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH PSILI AND OXIA AND YPOGEGRAMMENI => GREEK SMALL LETTER OMEGA
    case 0x1fa5: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH DASIA AND OXIA AND YPOGEGRAMMENI => GREEK SMALL LETTER OMEGA
    case 0x1fa6: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH PSILI AND PERISPOMENI AND YPOGEGRAMMENI => GREEK SMALL LETTER OMEGA
    case 0x1fa7: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH DASIA AND PERISPOMENI AND YPOGEGRAMMENI => GREEK SMALL LETTER OMEGA
    case 0x1fa8: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PROSGEGRAMMENI => GREEK CAPITAL LETTER OMEGA
    case 0x1fa9: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER OMEGA
    case 0x1faa: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND VARIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER OMEGA
    case 0x1fab: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND VARIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER OMEGA
    case 0x1fac: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND OXIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER OMEGA
    case 0x1fad: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND OXIA AND PROSGEGRAMMENI => GREEK CAPITAL LETTER OMEGA
    case 0x1fae: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH PSILI AND PERISPOMENI AND PROSGEGRAMMENI => GREEK CAPITAL LETTER OMEGA
    case 0x1faf: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH DASIA AND PERISPOMENI AND PROSGEGRAMMENI => GREEK CAPITAL LETTER OMEGA
    case 0x1fb0: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH VRACHY => GREEK SMALL LETTER ALPHA
    case 0x1fb1: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH MACRON => GREEK SMALL LETTER ALPHA
    case 0x1fb2: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH VARIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ALPHA
    case 0x1fb3: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH YPOGEGRAMMENI => GREEK SMALL LETTER ALPHA
    case 0x1fb4: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH OXIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ALPHA
    case 0x1fb6: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH PERISPOMENI => GREEK SMALL LETTER ALPHA
    case 0x1fb7: r=0x03B1; break; // GREEK SMALL LETTER ALPHA WITH PERISPOMENI AND YPOGEGRAMMENI => GREEK SMALL LETTER ALPHA
    case 0x1fb8: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH VRACHY => GREEK CAPITAL LETTER ALPHA
    case 0x1fb9: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH MACRON => GREEK CAPITAL LETTER ALPHA
    case 0x1fba: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH VARIA => GREEK CAPITAL LETTER ALPHA
    case 0x1fbb: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH OXIA => GREEK CAPITAL LETTER ALPHA
    case 0x1fbc: r=0x0391; break; // GREEK CAPITAL LETTER ALPHA WITH PROSGEGRAMMENI => GREEK CAPITAL LETTER ALPHA
    case 0x1fc2: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH VARIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ETA
    case 0x1fc3: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH YPOGEGRAMMENI => GREEK SMALL LETTER ETA
    case 0x1fc4: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH OXIA AND YPOGEGRAMMENI => GREEK SMALL LETTER ETA
    case 0x1fc6: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH PERISPOMENI => GREEK SMALL LETTER ETA
    case 0x1fc7: r=0x03B7; break; // GREEK SMALL LETTER ETA WITH PERISPOMENI AND YPOGEGRAMMENI => GREEK SMALL LETTER ETA
    case 0x1fc8: r=0x0395; break; // GREEK CAPITAL LETTER EPSILON WITH VARIA => GREEK CAPITAL LETTER EPSILON
    case 0x1fc9: r=0x0395; break; // GREEK CAPITAL LETTER EPSILON WITH OXIA => GREEK CAPITAL LETTER EPSILON
    case 0x1fca: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH VARIA => GREEK CAPITAL LETTER ETA
    case 0x1fcb: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH OXIA => GREEK CAPITAL LETTER ETA
    case 0x1fcc: r=0x0397; break; // GREEK CAPITAL LETTER ETA WITH PROSGEGRAMMENI => GREEK CAPITAL LETTER ETA
    case 0x1fd0: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH VRACHY => GREEK SMALL LETTER IOTA
    case 0x1fd1: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH MACRON => GREEK SMALL LETTER IOTA
    case 0x1fd2: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH DIALYTIKA AND VARIA => GREEK SMALL LETTER IOTA
    case 0x1fd3: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH DIALYTIKA AND OXIA => GREEK SMALL LETTER IOTA
    case 0x1fd6: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH PERISPOMENI => GREEK SMALL LETTER IOTA
    case 0x1fd7: r=0x03B9; break; // GREEK SMALL LETTER IOTA WITH DIALYTIKA AND PERISPOMENI => GREEK SMALL LETTER IOTA
    case 0x1fd8: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH VRACHY => GREEK CAPITAL LETTER IOTA
    case 0x1fd9: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH MACRON => GREEK CAPITAL LETTER IOTA
    case 0x1fda: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH VARIA => GREEK CAPITAL LETTER IOTA
    case 0x1fdb: r=0x0399; break; // GREEK CAPITAL LETTER IOTA WITH OXIA => GREEK CAPITAL LETTER IOTA
    case 0x1fe0: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH VRACHY => GREEK SMALL LETTER UPSILON
    case 0x1fe1: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH MACRON => GREEK SMALL LETTER UPSILON
    case 0x1fe2: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND VARIA => GREEK SMALL LETTER UPSILON
    case 0x1fe3: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND OXIA => GREEK SMALL LETTER UPSILON
    case 0x1fe4: r=0x03C1; break; // GREEK SMALL LETTER RHO WITH PSILI => GREEK SMALL LETTER RHO
    case 0x1fe5: r=0x03C1; break; // GREEK SMALL LETTER RHO WITH DASIA => GREEK SMALL LETTER RHO
    case 0x1fe6: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH PERISPOMENI => GREEK SMALL LETTER UPSILON
    case 0x1fe7: r=0x03C5; break; // GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND PERISPOMENI => GREEK SMALL LETTER UPSILON
    case 0x1fe8: r=0x03A5; break; // GREEK CAPITAL LETTER UPSILON WITH VRACHY => GREEK CAPITAL LETTER UPSILON
    case 0x1fe9: r=0x03A5; break; // GREEK CAPITAL LETTER UPSILON WITH MACRON => GREEK CAPITAL LETTER UPSILON
    case 0x1fea: r=0x03A5; break; // GREEK CAPITAL LETTER UPSILON WITH VARIA => GREEK CAPITAL LETTER UPSILON
    case 0x1feb: r=0x03A5; break; // GREEK CAPITAL LETTER UPSILON WITH OXIA => GREEK CAPITAL LETTER UPSILON
    case 0x1fec: r=0x03A1; break; // GREEK CAPITAL LETTER RHO WITH DASIA => GREEK CAPITAL LETTER RHO
    case 0x1ff2: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH VARIA AND YPOGEGRAMMENI => GREEK SMALL LETTER OMEGA
    case 0x1ff3: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH YPOGEGRAMMENI => GREEK SMALL LETTER OMEGA
    case 0x1ff4: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH OXIA AND YPOGEGRAMMENI => GREEK SMALL LETTER OMEGA
    case 0x1ff6: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH PERISPOMENI => GREEK SMALL LETTER OMEGA
    case 0x1ff7: r=0x03C9; break; // GREEK SMALL LETTER OMEGA WITH PERISPOMENI AND YPOGEGRAMMENI => GREEK SMALL LETTER OMEGA
    case 0x1ff8: r=0x039F; break; // GREEK CAPITAL LETTER OMICRON WITH VARIA => GREEK CAPITAL LETTER OMICRON
    case 0x1ff9: r=0x039F; break; // GREEK CAPITAL LETTER OMICRON WITH OXIA => GREEK CAPITAL LETTER OMICRON
    case 0x1ffa: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH VARIA => GREEK CAPITAL LETTER OMEGA
    case 0x1ffb: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH OXIA => GREEK CAPITAL LETTER OMEGA
    case 0x1ffc: r=0x03A9; break; // GREEK CAPITAL LETTER OMEGA WITH PROSGEGRAMMENI => GREEK CAPITAL LETTER OMEGA
    case 0x304c: r=0x304B; break; // HIRAGANA LETTER GA => HIRAGANA LETTER KA
    case 0x304e: r=0x304D; break; // HIRAGANA LETTER GI => HIRAGANA LETTER KI
    case 0x3050: r=0x304F; break; // HIRAGANA LETTER GU => HIRAGANA LETTER KU
    case 0x3052: r=0x3051; break; // HIRAGANA LETTER GE => HIRAGANA LETTER KE
    case 0x3054: r=0x3053; break; // HIRAGANA LETTER GO => HIRAGANA LETTER KO
    case 0x3056: r=0x3055; break; // HIRAGANA LETTER ZA => HIRAGANA LETTER SA
    case 0x3058: r=0x3057; break; // HIRAGANA LETTER ZI => HIRAGANA LETTER SI
    case 0x305a: r=0x3059; break; // HIRAGANA LETTER ZU => HIRAGANA LETTER SU
    case 0x305c: r=0x305B; break; // HIRAGANA LETTER ZE => HIRAGANA LETTER SE
    case 0x305e: r=0x305D; break; // HIRAGANA LETTER ZO => HIRAGANA LETTER SO
    case 0x3060: r=0x305F; break; // HIRAGANA LETTER DA => HIRAGANA LETTER TA
    case 0x3062: r=0x3061; break; // HIRAGANA LETTER DI => HIRAGANA LETTER TI
    case 0x3065: r=0x3064; break; // HIRAGANA LETTER DU => HIRAGANA LETTER TU
    case 0x3067: r=0x3066; break; // HIRAGANA LETTER DE => HIRAGANA LETTER TE
    case 0x3069: r=0x3068; break; // HIRAGANA LETTER DO => HIRAGANA LETTER TO
    case 0x3070: r=0x306F; break; // HIRAGANA LETTER BA => HIRAGANA LETTER HA
    case 0x3071: r=0x306F; break; // HIRAGANA LETTER PA => HIRAGANA LETTER HA
    case 0x3073: r=0x3072; break; // HIRAGANA LETTER BI => HIRAGANA LETTER HI
    case 0x3074: r=0x3072; break; // HIRAGANA LETTER PI => HIRAGANA LETTER HI
    case 0x3076: r=0x3075; break; // HIRAGANA LETTER BU => HIRAGANA LETTER HU
    case 0x3077: r=0x3075; break; // HIRAGANA LETTER PU => HIRAGANA LETTER HU
    case 0x3079: r=0x3078; break; // HIRAGANA LETTER BE => HIRAGANA LETTER HE
    case 0x307a: r=0x3078; break; // HIRAGANA LETTER PE => HIRAGANA LETTER HE
    case 0x307c: r=0x307B; break; // HIRAGANA LETTER BO => HIRAGANA LETTER HO
    case 0x307d: r=0x307B; break; // HIRAGANA LETTER PO => HIRAGANA LETTER HO
    case 0x3094: r=0x3046; break; // HIRAGANA LETTER VU => HIRAGANA LETTER U
    case 0x30ac: r=0x30AB; break; // KATAKANA LETTER GA => KATAKANA LETTER KA
    case 0x30ae: r=0x30AD; break; // KATAKANA LETTER GI => KATAKANA LETTER KI
    case 0x30b0: r=0x30AF; break; // KATAKANA LETTER GU => KATAKANA LETTER KU
    case 0x30b2: r=0x30B1; break; // KATAKANA LETTER GE => KATAKANA LETTER KE
    case 0x30b4: r=0x30B3; break; // KATAKANA LETTER GO => KATAKANA LETTER KO
    case 0x30b6: r=0x30B5; break; // KATAKANA LETTER ZA => KATAKANA LETTER SA
    case 0x30b8: r=0x30B7; break; // KATAKANA LETTER ZI => KATAKANA LETTER SI
    case 0x30ba: r=0x30B9; break; // KATAKANA LETTER ZU => KATAKANA LETTER SU
    case 0x30bc: r=0x30BB; break; // KATAKANA LETTER ZE => KATAKANA LETTER SE
    case 0x30be: r=0x30BD; break; // KATAKANA LETTER ZO => KATAKANA LETTER SO
    case 0x30c0: r=0x30BF; break; // KATAKANA LETTER DA => KATAKANA LETTER TA
    case 0x30c2: r=0x30C1; break; // KATAKANA LETTER DI => KATAKANA LETTER TI
    case 0x30c5: r=0x30C4; break; // KATAKANA LETTER DU => KATAKANA LETTER TU
    case 0x30c7: r=0x30C6; break; // KATAKANA LETTER DE => KATAKANA LETTER TE
    case 0x30c9: r=0x30C8; break; // KATAKANA LETTER DO => KATAKANA LETTER TO
    case 0x30d0: r=0x30CF; break; // KATAKANA LETTER BA => KATAKANA LETTER HA
    case 0x30d1: r=0x30CF; break; // KATAKANA LETTER PA => KATAKANA LETTER HA
    case 0x30d3: r=0x30D2; break; // KATAKANA LETTER BI => KATAKANA LETTER HI
    case 0x30d4: r=0x30D2; break; // KATAKANA LETTER PI => KATAKANA LETTER HI
    case 0x30d6: r=0x30D5; break; // KATAKANA LETTER BU => KATAKANA LETTER HU
    case 0x30d7: r=0x30D5; break; // KATAKANA LETTER PU => KATAKANA LETTER HU
    case 0x30d9: r=0x30D8; break; // KATAKANA LETTER BE => KATAKANA LETTER HE
    case 0x30da: r=0x30D8; break; // KATAKANA LETTER PE => KATAKANA LETTER HE
    case 0x30dc: r=0x30DB; break; // KATAKANA LETTER BO => KATAKANA LETTER HO
    case 0x30dd: r=0x30DB; break; // KATAKANA LETTER PO => KATAKANA LETTER HO
    case 0x30f4: r=0x30A6; break; // KATAKANA LETTER VU => KATAKANA LETTER U
    case 0x30f7: r=0x30EF; break; // KATAKANA LETTER VA => KATAKANA LETTER WA
    case 0x30f8: r=0x30F0; break; // KATAKANA LETTER VI => KATAKANA LETTER WI
    case 0x30f9: r=0x30F1; break; // KATAKANA LETTER VE => KATAKANA LETTER WE
    case 0x30fa: r=0x30F2; break; // KATAKANA LETTER VO => KATAKANA LETTER WO
    case 0xfb1d: r=0x05D9; break; // HEBREW LETTER YOD WITH HIRIQ => HEBREW LETTER YOD
    case 0xfb2a: r=0x05E9; break; // HEBREW LETTER SHIN WITH SHIN DOT => HEBREW LETTER SHIN
    case 0xfb2b: r=0x05E9; break; // HEBREW LETTER SHIN WITH SIN DOT => HEBREW LETTER SHIN
    case 0xfb2c: r=0xFB49; break; // HEBREW LETTER SHIN WITH DAGESH AND SHIN DOT =>
    case 0xfb2d: r=0xFB49; break; // HEBREW LETTER SHIN WITH DAGESH AND SIN DOT =>
    case 0xfb2e: r=0x05D0; break; // HEBREW LETTER ALEF WITH PATAH => HEBREW LETTER ALEF
    case 0xfb2f: r=0x05D0; break; // HEBREW LETTER ALEF WITH QAMATS => HEBREW LETTER ALEF
    case 0xfb30: r=0x05D0; break; // HEBREW LETTER ALEF WITH MAPIQ => HEBREW LETTER ALEF
    case 0xfb31: r=0x05D1; break; // HEBREW LETTER BET WITH DAGESH => HEBREW LETTER BET
    case 0xfb32: r=0x05D2; break; // HEBREW LETTER GIMEL WITH DAGESH => HEBREW LETTER GIMEL
    case 0xfb33: r=0x05D3; break; // HEBREW LETTER DALET WITH DAGESH => HEBREW LETTER DALET
    case 0xfb34: r=0x05D4; break; // HEBREW LETTER HE WITH MAPIQ => HEBREW LETTER HE
    case 0xfb35: r=0x05D5; break; // HEBREW LETTER VAV WITH DAGESH => HEBREW LETTER VAV
    case 0xfb36: r=0x05D6; break; // HEBREW LETTER ZAYIN WITH DAGESH => HEBREW LETTER ZAYIN
    case 0xfb38: r=0x05D8; break; // HEBREW LETTER TET WITH DAGESH => HEBREW LETTER TET
    case 0xfb39: r=0x05D9; break; // HEBREW LETTER YOD WITH DAGESH => HEBREW LETTER YOD
    case 0xfb3a: r=0x05DA; break; // HEBREW LETTER FINAL KAF WITH DAGESH => HEBREW LETTER FINAL KAF
    case 0xfb3b: r=0x05DB; break; // HEBREW LETTER KAF WITH DAGESH => HEBREW LETTER KAF
    case 0xfb3c: r=0x05DC; break; // HEBREW LETTER LAMED WITH DAGESH => HEBREW LETTER LAMED
    case 0xfb3e: r=0x05DE; break; // HEBREW LETTER MEM WITH DAGESH => HEBREW LETTER MEM
    case 0xfb40: r=0x05E0; break; // HEBREW LETTER NUN WITH DAGESH => HEBREW LETTER NUN
    case 0xfb41: r=0x05E1; break; // HEBREW LETTER SAMEKH WITH DAGESH => HEBREW LETTER SAMEKH
    case 0xfb43: r=0x05E3; break; // HEBREW LETTER FINAL PE WITH DAGESH => HEBREW LETTER FINAL PE
    case 0xfb44: r=0x05E4; break; // HEBREW LETTER PE WITH DAGESH => HEBREW LETTER PE
    case 0xfb46: r=0x05E6; break; // HEBREW LETTER TSADI WITH DAGESH => HEBREW LETTER TSADI
    case 0xfb47: r=0x05E7; break; // HEBREW LETTER QOF WITH DAGESH => HEBREW LETTER QOF
    case 0xfb48: r=0x05E8; break; // HEBREW LETTER RESH WITH DAGESH => HEBREW LETTER RESH
    case 0xfb49: r=0x05E9; break; // HEBREW LETTER SHIN WITH DAGESH => HEBREW LETTER SHIN
    case 0xfb4a: r=0x05EA; break; // HEBREW LETTER TAV WITH DAGESH => HEBREW LETTER TAV
    case 0xfb4b: r=0x05D5; break; // HEBREW LETTER VAV WITH HOLAM => HEBREW LETTER VAV
    case 0xfb4c: r=0x05D1; break; // HEBREW LETTER BET WITH RAFE => HEBREW LETTER BET
    case 0xfb4d: r=0x05DB; break; // HEBREW LETTER KAF WITH RAFE => HEBREW LETTER KAF
    case 0xfb4e: r=0x05E4; break; // HEBREW LETTER PE WITH RAFE => HEBREW LETTER PE
      /* end of autogenerated code */
    default: r=c; break;
  }
  return r;
}


/* end of Sebastian Nagel */

