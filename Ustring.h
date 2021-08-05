/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef Ustring_H
#define Ustring_H

#include "Unicode.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This library provides functions for safely manipulating
 * unicode strings.
 *
 * Author: Olivier Blanc
 * Modified by Sébastien Paumier
 * Modified by Cristian Martinez
 */

/**
 * Here is the type we use for manipulating safe strings.
 */
typedef struct {
   /* The unichar buffer */
   unichar* str;

   /* Its maximum capacity */
   unsigned int size;

   /* The actual length of the string being represented */
   unsigned int len;
} Ustring;

#define MAXBUF    1024
#define MINBUF      16

void resize(Ustring* ustr,unsigned int size);

Ustring* new_Ustring(const unichar*);
Ustring* new_Ustring();
Ustring* new_Ustring(unsigned int);
void free_Ustring(Ustring * ustr);
static inline unichar* free_Ustring_get_str(Ustring*s);

static inline void empty(Ustring * ustr);
void truncate(Ustring* ustr,unsigned int length);

static inline int u_strlen(Ustring*);
void u_strcat(Ustring*,const unichar*,unsigned int);
void u_strcat(Ustring*,const char*,unsigned int);
void u_strcat(Ustring*,unichar);
static inline void u_strcat(Ustring*,const Ustring*);
static inline void u_strcat(Ustring*,const unichar*);
static inline void u_strcat(Ustring*,const char*);

static inline void u_strcpy(Ustring*,const Ustring*);
static inline void u_strcpy(Ustring*,const unichar*);
static inline void u_strcpy(Ustring*,const char*);
void u_strcpy(Ustring*,const char*,unsigned int);
void u_strcpy(Ustring*,const unichar*,unsigned int);

static inline void chomp_new_line(Ustring*);

void u_sprintf(Ustring*,const char*,...);
void u_sprintf(Ustring*,const char*,...);
void u_strcatf(Ustring*,const char*,...);

int readline_keep_CR(Ustring*,U_FILE*);
int readline(Ustring*,U_FILE*);
unichar* readline_safe(U_FILE* f);


void u_strcpy(Ustring* ustr,const unichar* str,unsigned int length);

/* Inline implementations */

/**
 * Empties the given Ustring.
 */
static inline void empty(Ustring* ustr) {
if (ustr==NULL) {
   fatal_error("Cannot empty a NULL Ustring\n");
}
ustr->str[0]=0;
ustr->len=0;
}


/**
 * Concatenates the given unicode string to the given Ustring.
 */
static inline void u_strcat(Ustring* ustr,const unichar* str) {
if (ustr==NULL) {
   fatal_error("NULL Ustring error in u_strcat\n");
}
if (str==NULL || str[0]=='\0') return;
u_strcat(ustr,str,u_strlen(str));
}

/*
 * Returns 1 if a is equal to b, 0 otherwise
 */
static inline int u_equal(const Ustring* a,const Ustring* b) {
  return (a->len == b->len && u_strncmp(a->str, b->str, a->len) == 0);
}


static inline int u_strlen(Ustring* s) {
return s->len;
}


static inline unichar* free_Ustring_get_str(Ustring*s)
{
    unichar*str = s->str;
    free(s);
    return str;
}


/**
 * Concatenates the given Utring to the given Ustring.
 */
static inline void u_strcat(Ustring* a,const Ustring* b) {
if (a==NULL || b==NULL) {
   fatal_error("NULL Ustring error in u_strcat\n");
}
if (b->str==NULL || b->str[0]=='\0') return;
u_strcat(a,b->str,b->len);
}

void u_strcat(Ustring* ustr,const char* str,unsigned int length);

/**
 * Concatenates the given string to the given Ustring.
 */
static inline void u_strcat(Ustring* ustr,const char* str) {
if (ustr==NULL) {
   fatal_error("NULL Ustring error in u_strcat\n");
}
if (str==NULL || str[0]=='\0') return;
u_strcat(ustr,(char*)str,(int)strlen(str));
}


/**
 * Copies 'src' content in to the given Ustring, whose previous content
 * is lost.
 */

static inline void u_strcpy(Ustring* dest,const unichar* str) {
if (dest==NULL) {
   fatal_error("NULL Ustring error in u_strcpy\n");
}

if ((str!=NULL) && (str[0]!='\0')) {
  u_strcpy(dest,str,(int)u_strlen(str));
}
else
{
  dest->str[0]=0;
  dest->len=0;
}
}


/**
 * Copies 'src' content in to the given Ustring, whose previous content
 * is lost.
 */
static inline void u_strcpy(Ustring* dest,const Ustring* src) {
if (dest==NULL || src==NULL) {
   fatal_error("NULL Ustring error in u_strcpy\n");
}
empty(dest);
u_strcat(dest,src);
}


/**
 * Copies 'src' content in to the given Ustring, whose previous content
 * is lost.
 */
static inline void u_strcpy(Ustring* dest, const char* src) {
if (dest==NULL) {
   fatal_error("NULL Ustring error in u_strcpy\n");
}
empty(dest);
u_strcat(dest,src);
}

inline void u_switch(Ustring* str1,Ustring* str2) {
Ustring tmp=*str1;
*str1=*str2;
*str2=tmp;
}

/**
 * Removes the '\n' at the end of the given Ustring, if any.
 */
static inline void chomp_new_line(Ustring* ustr) {
if (ustr==NULL) {
   fatal_error("NULL Ustring error in chomp_new_line\n");
}
while (ustr->len>0 && ustr->str[ustr->len-1]=='\n') {
   ustr->len--;
   ustr->str[ustr->len]='\0';
}
}


/**
 * Removes n chars from the string. If n>len, the string is emptied.
 */
static inline void remove_n_chars(Ustring* ustr,unsigned int n) {
if (ustr==NULL) {
   fatal_error("NULL Ustring error in remove_n_chars\n");
}
if (n>ustr->len) n=ustr->len;
ustr->len=ustr->len-n;
ustr->str[ustr->len]='\0';
}

} // namespace unitex

#endif
