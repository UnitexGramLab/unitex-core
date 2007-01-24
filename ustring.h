 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef _USTRING_H_
#define _USTRING_H_

#include "unicode.h"

typedef struct ustring_t {

  unichar * str;
  int size;
  int len;

} ustring_t;

ustring_t * ustring_new(const unichar * str = NULL);
ustring_t * ustring_new(int size);
void ustring_delete(ustring_t * ustr);

static inline void ustring_empty(ustring_t * ustr);

void ustring_resize(ustring_t * ustr, int size);

static inline void ustring_copy(ustring_t * ustr, const unichar * str);
static inline void ustring_copy(ustring_t * dest, const ustring_t * src);
static inline void ustring_copy(ustring_t * ustr, const char * str);

void ustring_concat(ustring_t * ustr, const unichar * str, int strlen);
static inline void ustring_concat(ustring_t * ustr, const unichar * str);
static inline void ustring_concat(ustring_t * a, const ustring_t * b);

void ustring_concat(ustring_t * ustr, const char * str, int strlen);
static inline void ustring_concat(ustring_t * ustr, const char * str);

static inline void ustring_chomp_nl(ustring_t * ustr);

void ustring_concatf(ustring_t * ustr, char * fmt, ...);
void ustring_printf(ustring_t * ustr, char * fmt, ...);


/* read a complete line from f */

int ustring_readline(ustring_t * ustr, FILE * f);




/* inline implementations */

static inline void ustring_empty(ustring_t * ustr) { ustr->str[0] = 0; ustr->len = 0; }

static inline void ustring_concat(ustring_t * ustr, const unichar * str) { ustring_concat(ustr, str, u_strlen(str)); }
static inline void ustring_concat(ustring_t * a, const ustring_t * b)    { ustring_concat(a, b->str, b->len); }

static inline void ustring_copy(ustring_t * ustr, const unichar * str) {
  ustring_empty(ustr);
  ustring_concat(ustr, str);
}

static inline void ustring_concat(ustring_t * ustr, const char * str) { ustring_concat(ustr, str, strlen(str)); }

static inline void ustring_copy(ustring_t * dest, const ustring_t * src) {
  ustring_empty(dest);
  ustring_concat(dest, src);
}


static inline void ustring_copy(ustring_t * ustr, const char * str) {
  ustring_empty(ustr);
  ustring_concat(ustr, str);
}


static inline void ustring_chomp_nl(ustring_t * ustr) {
  if (ustr->str[ustr->len - 1] == '\n') { ustr->len--; ustr->str[ustr->len] = 0; }
}

#endif
