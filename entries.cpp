 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "unicode.h"
#include "String_hash.h"
#include "utils.h"

#define FLEC_SIZE 64
#define CANO_SIZE 64
#define GRAM_SIZE  8
#define FLEX_SIZE 24

typedef struct entry_t {

  unichar flechie[FLEC_SIZE];
  unichar canonic[CANO_SIZE];
  unichar gramm[GRAM_SIZE];
  unichar flex[FLEX_SIZE];
} entry_t;





int main(int argc, char ** argv) {


  if (argc < 2) { die("bad args\n"); }

  FILE * dic;
  FILE * out = stdout;


  if ((dic = u_fopen(argv[1], U_READ)) == NULL) { die("open %s\n", argv[1]); }

  if (argc > 2) {
    if ((out = u_fopen(argv[2], U_WRITE)) == NULL) { die("open %s\n", argv[2]); }
  }


  int len;
  int bufsize = 1024;
  unichar * buf = (unichar *) xmalloc(bufsize * sizeof(unichar));
  unichar * p;

  struct string_hash * hash = new_string_hash();

  unichar hashbuf[1024];

  while ((len = u_fgets(buf, bufsize, dic)) != 0) {

    while (len == (bufsize - 1) && (buf[len - 1] != '\n')) {

      bufsize = 2 * bufsize;
      buf = (unichar *) xrealloc(buf, bufsize * sizeof(unichar));
      p = buf + len;
      len = len + u_fgets(p, bufsize - len, dic);
    }


    if (buf[len - 1] == '\n') { buf[len - 1] = 0; }

    if ((p = u_strchr(buf, '.')) == NULL) { warning("weird entry : %S\n", buf); continue; }

    *p = 0;

    unichar * gramm = ++p;

    while ((*p != ':')) {
      if (*p == 0) { break; }
      if ((*p == '+') || (*p == '-')) { *p = 0; }
      p++;
    }


    if (*p == 0) { // pas de traits flexionnels
      u_sprintf(hashbuf, "%S\t%S", gramm, buf);
      get_hash_number(hashbuf, hash);
      continue;
    }

    int cont = 1;

    while (cont) {

      unichar * flex = ++p;

      while (*p && (*p != ':')) {
	if ((*p == '+') || (*p == '-')) { fatal_error("bad label with flex : %S\n", flex); }
	p++;
      }

      if (*p == 0) { cont = 0; }

      *p = 0;

      u_sprintf(hashbuf, "%S:%S\t%S", gramm, flex, buf);
      get_hash_number(hashbuf, hash);

    }

  }

  error("%d diff codes\n", hash->N);

  for (int i = 0; i < hash->N; i++) {
    u_fprintf(out, "%S\n", hash->tab[i]);
  }

  return 0;
}
