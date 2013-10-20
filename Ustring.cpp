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

#include "Ustring.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define MAXBUF 1024


/**
 * Allocates, initializes and returns a Ustring representing the given string.
 */
Ustring* new_Ustring(const unichar* str) {
Ustring* res=(Ustring*)malloc(sizeof(Ustring));
if (res==NULL) {
   fatal_alloc_error("new_Ustring");
}
if (str==NULL) {
   str=U_EMPTY;
}
res->len=u_strlen(str);
res->size=accurate_rounded_size(res->len+1);
res->str=(unichar*)malloc(res->size*sizeof(unichar));
if (res->str==NULL) {
   fatal_alloc_error("new_Ustring");
}
memcpy(res->str,str,(res->len+1)*sizeof(unichar));
return res;
}


/**
 * Allocates, initializes and returns a Ustring representing the empty string.
 */
Ustring* new_Ustring() {
return new_Ustring((unichar*)NULL);
}


/**
 * Allocates, initializes and returns a Ustring representing the empty string.
 * The internal buffer is set to the given size.
 */
Ustring* new_Ustring(unsigned int size) {
Ustring* res=(Ustring*)malloc(sizeof(Ustring));
if (res==NULL) {
   fatal_alloc_error("new_Ustring");
}
res->len=0;
res->size=accurate_rounded_size(size);
res->str=(unichar*)malloc(res->size*sizeof(unichar));
if (res->str==NULL) {
   fatal_alloc_error("new_Ustring");
}
res->str[0]='\0';
return res;
}


/**
 * Frees all the memory associated to the given Ustring.
 */
void free_Ustring(Ustring* ustr) {
if (ustr==NULL) return;
if (ustr->str!=NULL) free(ustr->str);
free(ustr);
}


/**
 * Resizes th internal buffer of the given Ustring to the given size.
 * The buffer size is never decreased. Note that you cannot set a size<1.
 */
void resize(Ustring* ustr,unsigned int size) {
if (size<1) {
   size=1;
}
if (size<=ustr->size) {
   return;
}
ustr->str=(unichar*)realloc(ustr->str,size*sizeof(unichar));
if (ustr->str==NULL) {
   fatal_alloc_error("resize");
}
if (size<ustr->len) {
   /* If we truncate the string */
   ustr->len=size-1;
   ustr->str[ustr->len]='\0';
}
ustr->size=size;
}


/**
 * Truncates the given string to the given length that must be lesser than the current
 * one.
 */
void truncate(Ustring* ustr,unsigned int length) {
if (length>ustr->len) {
	fatal_error("Cannot truncate Ustring of length %u to greater size %u\n",ustr->len,length);
}
ustr->len=length;
ustr->str[length]='\0';
}


/**
 * Concatenates the given unicode string to the given Ustring.
 * 'length' is the number of chars of 'str' to be copied.
 */
void u_strcat(Ustring* ustr,const unichar* str,unsigned int length) {
if (str==NULL || length==0 || str[0]=='\0') {
   /* If there is nothing to concatenate */
   return;
}
unsigned int newlen=ustr->len+length;
if (ustr->size<newlen+1) {
   /* If necessary, we enlarge the internal buffer */
	unsigned int n=ustr->size;
	if (n==0) n=1;
   while (n<=newlen+1) {
      n=n*2;
   }
   resize(ustr,n);
}
for (unsigned int i=0;i<length;i++) {
   ustr->str[ustr->len+i]=str[i];
}
ustr->str[newlen]='\0';
ustr->len=newlen;
}


/**
 * Concatenates the given unicode character to the given Ustring.
 */
void u_strcat(Ustring* ustr,unichar c) {
unsigned int newlen=ustr->len+1;
if (ustr->size<newlen+1) {
	/* If necessary, we enlarge the internal buffer */
	unsigned int n=ustr->size;
	if (n==0) n=1;
    while (n<=newlen+1) {
    	n=n*2;
    }
    resize(ustr,n);
}
ustr->str[ustr->len]=c;
ustr->str[ustr->len+1]='\0';
ustr->len++;
}


/**
 * Concatenates the given string to the given Ustring.
 * 'length' is the number of chars of 'str' to be copied.
 */
void u_strcat(Ustring* ustr,const char* str,unsigned int length) {
if (str==NULL || str[0]=='\0' || length==0) {
   /* If there is nothing to concatenate */
   return;
}
unsigned int newlen=ustr->len+length;
if (ustr->size<newlen+1) {
   /* If necessary, we enlarge the internal buffer */
	unsigned int n=ustr->size;
	if (n==0) n=1;
   while (n<=newlen+1) {
       n=n*2;
   }
   resize(ustr,n);
}
for (unsigned int i=0;i<length;i++) {
   ustr->str[ustr->len+i]=str[i];
}
ustr->str[newlen]='\0';
ustr->len=newlen;
}


/**
 * This function performs a safe read line from the given file.
 * The line content is copied into the given Ustring, whose previous
 * content is lost. The length the line read is returned.
 */
int readline_keep_CR(Ustring* ustr,U_FILE* f) {
unichar buf[MAXBUF];
int len=0;
empty(ustr);

/* - EOF : means that it was the end of file
* - 0 : means that we have read an empty line ended by '\n'
* - (len ==(size-1)) and (line[len-1] != '\n') : means that the line was too long for the buffer
* - (0 < len < size-1) : means that we have read a complete line (str[len-1]=='\n', unless EOF)
*/
do {
   len=u_fgets(buf,MAXBUF,f);
   if (len==EOF) return EOF;
   u_strcat(ustr,buf,len);
} while ((len==MAXBUF-1) && buf[len-1]!='\n'); /* If we are not at the end of the line... */
return ustr->len;
}

/**
 * The same as readline_keep_CR but \n is removed.
 */
int readline(Ustring* ustr,U_FILE* f) {
if (EOF==readline_keep_CR(ustr,f)) return EOF;
chomp_new_line(ustr);
return ustr->len;
}


/**
 * The same as u_sprintf, but into a Ustring, whose previous
 * content is lost.
 */
void u_sprintf(Ustring* ustr,const char* fmt,...) {
va_list plist;
va_start(plist,fmt);
unsigned int size=u_vsprintf(NULL,fmt,plist);
resize(ustr,size+1);
va_start(plist,fmt);
u_vsprintf(ustr->str,fmt,plist);
ustr->len=u_strlen(ustr->str);
if (size!=ustr->len) {
   error("u_sprintf: strlen=%d (expected %d)\n",ustr->len,size);
}
}


/**
 * The same as u_sprintf above, except that the printed string
 * is appended at the end of the given Ustring instead of replacing
 * its previous content.
 */
void u_strcatf(Ustring* ustr,const char* fmt,...) {
va_list plist;
va_start(plist,fmt);
unsigned int size=u_vsprintf(NULL,fmt,plist)+ustr->len;
resize(ustr,size+1);
va_start(plist,fmt);
u_vsprintf(ustr->str+ustr->len,fmt,plist);
ustr->len=u_strlen(ustr->str);
if (size!=ustr->len) {
   error("u_strcatf: strlen=%d (expected %d)\n",ustr->len,size);
}
}


/**
 * Uses a Ustring to perform a safe read line operation. Returns NULL if
 * the end of file was found. \n is removed.
 */
unichar* readline_safe(U_FILE* f) {
Ustring* s=new_Ustring();
int ret=readline(s,f);
unichar* result=NULL;
if (ret!=EOF) {
	result=s->str;
	s->str=NULL;
}
free_Ustring(s);
return result;
}

} // namespace unitex
