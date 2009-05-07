 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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


/**
 * This library provides functions for safely manipulating
 * unicode strings.
 *
 * Author: Olivier Blanc
 * Modified by S�bastien Paumier
 */

/**
 * Here is the type we use for manipulating safe strings.
 */
typedef struct {
   /* The unichar buffer */
   unichar* str;

   /* Its maximum capacity */
   int size;

   /* The actual length of the string being represented */
   int len;
} Ustring;



Ustring* new_Ustring(const unichar*);
Ustring* new_Ustring();
Ustring* new_Ustring(int);
void free_Ustring(Ustring * ustr);

static inline void empty(Ustring * ustr);

void u_strcat(Ustring*,const unichar*,int);
void u_strcat(Ustring*,const char*,int);
static inline void u_strcat(Ustring*,const Ustring*);
static inline void u_strcat(Ustring*,const unichar*);
static inline void u_strcat(Ustring*,const char*);

static inline void u_strcpy(Ustring*,const Ustring*);
static inline void u_strcpy(Ustring*,const unichar*);
static inline void u_strcpy(Ustring*,const char*);

static inline void chomp_new_line(Ustring*);

void u_sprintf(Ustring*,const char*,...);
void u_strcatf(Ustring*,const char*,...);

int readline(Ustring*,U_FILE*);




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
static inline void u_strcpy(Ustring* dest,const unichar* src) {
if (dest==NULL) {
   fatal_error("NULL Ustring error in u_strcpy\n");
}
empty(dest);
u_strcat(dest,src);
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


/**
 * Removes the '\n' at the end of the given Ustring, if any.
 */
static inline void chomp_new_line(Ustring* ustr) {
if (ustr==NULL) {
   fatal_error("NULL Ustring error in chomp_new_line\n");
}
if (ustr->len>0 && ustr->str[ustr->len-1]=='\n') {
   ustr->len--;
   ustr->str[ustr->len]='\0';
}
}

#endif
