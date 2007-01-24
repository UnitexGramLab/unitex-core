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

#include "utils.h"
#include "ustring.h"

#define MAXBUF 1024


ustring_t * ustring_new(const unichar * str) {

  unichar empty[] = { 0 };
  ustring_t * res = (ustring_t *) xmalloc(sizeof(ustring_t));

  if (str == NULL) { str = empty; }

  res->len = u_strlen(str);
  res->size = res->len + 1;

  res->str = (unichar *) xmalloc(res->size * sizeof(unichar));

  for (int i = 0; i < res->size; i++) { res->str[i] = str[i]; }

  return res;
}

ustring_t * ustring_new(int size) {

  ustring_t * res = (ustring_t *) xmalloc(sizeof(ustring_t));

  if (size <= 0) { size = 1; }

  res->len  = 0;
  res->size = size;

  res->str = (unichar *) xmalloc(res->size * sizeof(unichar));

  res->str[0] = 0;

  return res;
}


void ustring_delete(ustring_t * ustr) {
  free(ustr->str);
  free(ustr);
}


void ustring_resize(ustring_t * ustr, int size) {

  if (size < 1) { size = 1; }

  ustr->str = (unichar *) xrealloc(ustr->str, size * sizeof(unichar));

  if (size < ustr->len) {
    ustr->len = size - 1;
    ustr->str[ustr->len] = 0;
  }

  ustr->size = size;
}


void ustring_concat(ustring_t * ustr, const unichar * str, int strlen) {

  if (str == NULL || *str == 0 || strlen == 0) { return; }

  int newlen = ustr->len + strlen;

  if (ustr->size < newlen + 1) { ustring_resize(ustr, newlen + 1); }

  for (int i = 0; i < strlen; i++) { ustr->str[ustr->len + i] = str[i]; }
  ustr->str[newlen] = 0;
  ustr->len = newlen;
}


void ustring_concat(ustring_t * ustr, const char * str, int strlen) {

  if (str == NULL || *str == 0 || strlen == 0) { return; }

  int newlen = ustr->len + strlen;

  if (ustr->size < newlen + 1) { ustring_resize(ustr, newlen + 1); }

  for (int i = 0; i < strlen; i++) { ustr->str[ustr->len + i] = str[i]; }
  ustr->str[newlen] = 0;
  ustr->len = newlen;
}


int ustring_readline(ustring_t * ustr, FILE * f) {

  unichar buf[MAXBUF];
  int len = 0;

  ustring_empty(ustr);

  do {
    len = u_fgets(buf, MAXBUF, f);
    ustring_concat(ustr, buf, len);
  } while ((len == MAXBUF - 1) && (buf[len - 1] != '\n'));  /* long line ... */

  return ustr->len;
}



void ustring_printf(ustring_t * ustr, char * fmt, ...) {

  va_list plist;
  va_start(plist, fmt);

  int size = calc_printf_size(fmt, plist);

  ustring_resize(ustr, size + 1);
  
  va_start(plist, fmt);

  u_vsprintf(ustr->str, fmt, plist);
  
  ustr->len = u_strlen(ustr->str);

  if (size != ustr->len) { error("ustring_printf: strlen=%d (expected %d)\n", ustr->len, size); }
}


void ustring_concatf(ustring_t * ustr, char * fmt, ...) {

  va_list plist;
  va_start(plist, fmt);

  int size = calc_printf_size(fmt, plist) + ustr->len;

  ustring_resize(ustr, size + 1);

  va_start(plist, fmt);

  u_vsprintf(ustr->str + ustr->len, fmt, plist);

  ustr->len = u_strlen(ustr->str);

  if (size != ustr->len) { error("ustring_concatf: strlen=%d (expected %d)\n", ustr->len, size); }
}
