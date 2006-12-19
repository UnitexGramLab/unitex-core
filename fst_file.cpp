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

#include <stdio.h>
#include <ctype.h>

#include "utils.h"
#include "ustring.h"
#include "hash_str_table.h"
#include "symbol.h"
#include "autalmot.h"
#include "fst_file.h"

#define MAXBUF  1024


static int fst_file_load_symbols(fst_file_in_t * fstf) {

  //  debug("load_symbols\n");

  hash_str_table_empty(fstf->symbols);

  long fpos = ftell(fstf->f);

  rewind(fstf->f);


  /* go to labels section */
  
  unichar buf[MAXBUF];

  int i = 0, len;
  while (i < fstf->nbelems) {

    if ((len = u_fgets(buf, MAXBUF, fstf->f)) == 0) {
      o_error("load_fst_symbols: %s: unexpected EOF\n", fstf->name); return -1;
    }

    if (buf[0] == 'f' && isspace(buf[1])) { i++; }

    while ((len == MAXBUF - 1) && (buf[len - 1] != '\n')) { len = u_fgets(buf, MAXBUF, fstf->f); } // read end of line
  }


  symbol_t * (*load_symbol)(language_t * lang, unichar * lbl);
  load_symbol = (fstf->type == FST_TEXT) ? load_text_symbol : load_grammar_symbol;


  ustring_t * ustr = ustring_new(64);

  while (ustring_readline(ustr, fstf->f) && ustr->str[0] != 'f') {

    if (ustr->str[0] != '%' && ustr->str[0] != '@') {
      o_error("load_fst_symbols: %s: bad symbol line: '%S'\n", fstf->name, ustr->str);
      return -1;
    }

    ustring_chomp_nl(ustr);

    // debug("symb '%S':\n", ustr->str + 1);

    symbol_t * symb = load_symbol(fstf->lang, ustr->str + 1);

    if (symb == NULL) { o_error("fst_load_symbols: unable to load '%S'\n", ustr->str + 1); }

    //    debug("'%S'\t->\t", ustr->str + 1); symbols_dump(symb); endl();

    hash_str_table_add(fstf->symbols, ustr->str + 1, symb);
  }

  if (*ustr->str == 0) { fatal_error("load_symbols: unexpected eof\n"); }

//  debug("%d symbols loaded.\n", fstf->symbols->nbelems);

  ustring_delete(ustr);

  fseek(fstf->f, fpos, SEEK_SET);

  return 0;
}



fst_file_in_t * fst_file_in_open(char * fname, int type, language_t * lang) {

  fst_file_in_t * fstf = (fst_file_in_t *) xmalloc(sizeof(fst_file_in_t));

  fstf->name = strdup(fname);

  if ((fstf->f = u_fopen(fname, U_READ)) == NULL) {
    o_error("fst_file_open: unable to open '%s' for reading\n", fname);
    goto error_fstf;
  }

  unichar buf[MAXBUF];

  if (u_fgets(buf, MAXBUF, fstf->f) == 0) {
    o_error("fst_file_open: '%s' is empty\n", fname);
    goto error_f;
  }

  if (! u_is_digit(*buf)) {
    o_error("fst_file_open: %s: bad file format\n", fname);
    goto error_f;
  } 

  fstf->nbelems = u_parse_int(buf);

  // debug("fst_in_open: %s: %d autos\n", fstf->name, fstf->nbelems);


  if (type != FST_TEXT && type != FST_GRAMMAR) {
    o_error("fst_file_in_open: bad fst_type=%d\n", type);
    goto error_f;
  }

  fstf->lang = lang;
  fstf->type = type;
  fstf->pos0 = ftell(fstf->f);

  fstf->symbols = hash_str_table_new(64);
  if (fst_file_load_symbols(fstf) == -1) {
    o_error("fst_file_open: %s: cannot load symbols\n", fstf->name);
    goto error_symbols;
  }

  fstf->pos = 0;

  return fstf;


error_symbols:
  hash_str_table_delete(fstf->symbols);

error_f:
  fclose(fstf->f);

error_fstf:
  free(fstf->name);
  free(fstf);

  return NULL;
}


void fst_file_close(fst_file_in_t * fstf) {

  free(fstf->name);
  fclose(fstf->f);
  hash_str_table_delete(fstf->symbols, (release_f) symbols_delete);

  free(fstf);
}




autalmot_t * fst_file_autalmot_load_next(fst_file_in_t * fstf) {

//  debug("autalmot_load_next\n");

  if (fstf->pos >= fstf->nbelems) { return NULL; }

  ustring_t * ustr = ustring_new();

  ustring_readline(ustr, fstf->f);
  ustring_chomp_nl(ustr);

  // debug("line=%S\n", ustr->str);

  unichar * p = ustr->str;

  if (*p != '-') { fatal_error("fst_load_next: %s: bad file format\n", fstf->name); }

  p++;
  int i = u_parse_int(p, & p);

  //  debug("i=%d\n", i);

  if (i != fstf->pos + 1) { 
    die("fst_load: %s: parsing error with line '%S' ('-%d ...' expected)\n", fstf->name, ustr->str, fstf->pos + 1);
  }

  p++;  // p == name

//  debug("name=%S\n", p);

  autalmot_t * A = autalmot_new(p);

  while (ustring_readline(ustr, fstf->f) && *ustr->str != 'f') { /* read states */

    ustring_chomp_nl(ustr);
    p = ustr->str;

    int i = autalmot_add_state(A, (*p == 't') ? AUT_FINAL : 0);

  //  debug("i=%d\n", i);

    while (*p && ! u_is_digit(*p)) { p++; }

    while (*p) { /* new trans */

      int lbl = u_parse_int(p, &p);

      while (*p && ! u_is_digit(*p)) { p++; }

      if (*p == 0) { fatal_error("fst_load: %S: bad file format (line='%S')\n", fstf->name, ustr->str); }
      int to = u_parse_int(p, &p);

      if (fstf->symbols->tab[lbl]) { /* if it is a good symbol (successfully loaded), make transition */
	autalmot_add_trans(A, i, (symbol_t *) fstf->symbols->tab[lbl], to);
      }

      while (*p && ! u_is_digit(*p)) { p++; }      
    }
  }

  if (*ustr->str == 0) { fatal_error("fst_file_read: unexpected end of file\n"); }

  if (A->nbstates == 0) {

    o_error("fst_file_read: automaton with no state\n");

  } else { autalmot_set_initial(A, 0); }

//  debug("%d states\n", A->nbstates);

  fstf->pos++;

  ustring_delete(ustr);

  return A;
}


void fst_file_seek(fst_file_in_t * fstin, int no) {

  if (no < 0 || no >= fstin->nbelems) {
    fatal_error("fst_seek(%d): only %d automat%s in file\n", no, fstin->nbelems, (no > 1) ? "a" : "on");
  }

  if (no < fstin->pos) {
    fseek(fstin->f, fstin->pos0, SEEK_SET);
    fstin->pos = 0;
  }

  unichar buf[MAXBUF];
  int len;

  while (fstin->pos < no) {
    
    if ((len = u_fgets(buf, MAXBUF, fstin->f)) == 0) { fatal_error("fst_seek: %s: unexpected EOF\n", fstin->name); }

    if (buf[0] == 'f' && isspace(buf[1])) { fstin->pos++; }

    while ((len == MAXBUF - 1) && (buf[len - 1] != '\n')) { len = u_fgets(buf, MAXBUF, fstin->f); } // read end of line
  }
}


autalmot_t * fst_file_autalmot_load(fst_file_in_t * fstin, int no) {

  fst_file_seek(fstin, no);

  return fst_file_autalmot_load_next(fstin);
}



fst_file_out_t * fst_file_out_open(char * fname, int type) {

  fst_file_out_t * res = (fst_file_out_t *) xmalloc(sizeof(fst_file_out_t));

  if (type < 0 || type >= FST_BAD_TYPE) { fatal_error("fst_out_open: bad FST_TYPE\n"); }

  if ((res->f = u_fopen(fname, U_WRITE)) == NULL) {
    o_error("fst_out_open: unable to open '%s'\n", fname);
    free(res);
    return NULL;
  }

  res->fstart = ftell(res->f);

  u_fprints_char("0000000000\n", res->f);

  res->name    = strdup(fname);
  res->type    = type;
  res->nbelems = 0;
  res->labels  = hash_str_table_new(16);

  /* add <E> */

  unichar epsilon[] = { '<', 'E', '>', 0 };
  hash_str_table_add(res->labels, epsilon, u_strdup(epsilon));

  return res;
}



void output_labels(fst_file_out_t * fstout) {
  for (int i = 0; i < fstout->labels->nbelems; i++) { u_fprintf(fstout->f, "%%%S\n", fstout->labels->tab[i]); }
  u_fprintf(fstout->f, "f\n");
}


void fst_file_close(fst_file_out_t * fstout) {

  /* output labels */
  output_labels(fstout);

  /* writing header */

  fseek(fstout->f, fstout->fstart, SEEK_SET);

  unichar buf[16];
  u_strcpy_char(buf, "0000000000");

  int i = 9;
  int n = fstout->nbelems;

  if (n == 0) { o_error("fstfile without automaton\n"); }

  while (n) {
    buf[i--] = '0' + (n % 10);
    n = n / 10;
  }
    
  u_fprintf(fstout->f, "%S\n", buf);

  /* close all */

  fclose(fstout->f);

  hash_str_table_delete(fstout->labels, (release_f) free);
  free(fstout->name);

  free(fstout);
}



void PNC_trans_write(fst_file_out_t * fstf, int to) {

  unichar label[4];
  int idx;

  label[1] = 0;

  for (unichar * pnc = PUNC_TAB; *pnc; pnc++) {

    if (*pnc != '{') {
      label[0] = *pnc;

      if ((idx = hash_str_table_idx_lookup(fstf->labels, label)) == -1) {
	idx = hash_str_table_add(fstf->labels, label, u_strdup(label));
      }

      u_fprintf(fstf->f, "%d %d ", idx, to);
    }
  }

/*
  u_strcpy_char(label, "{S}");

  if ((idx = hash_str_table_idx_lookup(fstf->labels, label)) == -1) {
    idx = hash_str_table_add(fstf->labels, label, u_strdup(label));
  }

  u_fprintf(fstf->f, "%d %d ", idx, to);
*/
  
}



void CHFA_trans_write(fst_file_out_t * fstf, int to) {

  unichar label[2];
  int idx;

  label[1] = 0;

  for (unichar C = '0'; C <= '9'; C++) {

    label[0] = C;

    if ((idx = hash_str_table_idx_lookup(fstf->labels, label)) == -1) {
      idx = hash_str_table_add(fstf->labels, label, u_strdup(label));
    }

    u_fprintf(fstf->f, "%d %d ", idx, to);
  }
}



void LEXIC_trans_write(fst_file_out_t * fstf, int to) {

  unichar label[8];
  int idx;

  u_strcpy_char(label, "<MOT>");

  if ((idx = hash_str_table_idx_lookup(fstf->labels, label)) == -1) {
    idx = hash_str_table_add(fstf->labels, label, u_strdup(label));
  }

  u_fprintf(fstf->f, "%d %d ", idx, to);

  u_strcpy_char(label, "<!MOT>");

  if ((idx = hash_str_table_idx_lookup(fstf->labels, label)) == -1) {
    idx = hash_str_table_add(fstf->labels, label, u_strdup(label));
  }

  u_fprintf(fstf->f, "%d %d ", idx, to);
}



void fst_file_write(fst_file_out_t * fstf, const autalmot_t * A) {

  ustring_t * label = ustring_new();


  void (*symbol_to_label)(const symbol_t *, ustring_t *) = NULL;
  
  switch (fstf->type) {

  case FST_TEXT:
    symbol_to_label = symbol_to_text_label;
    break;

  case FST_TEXT_IMPLOSED:
    symbol_to_label = symbol_to_implosed_text_label;
    break;

  case FST_GRAMMAR:
    symbol_to_label = symbol_to_grammar_label;
    break;

  case FST_LOCATE:
    symbol_to_label = symbol_to_locate_label;
    break;

  default:
    fatal_error("fst_write: invalid fstf->type: %d\n", fstf->type);
  }

  u_fprintf(fstf->f, "-%d %S\n", fstf->nbelems + 1, A->name);

  int idx;

  unichar deflabel[] = { '<', 'd', 'e', 'f', '>', 0 };

  for (int q = 0; q < A->nbstates; q++) {

    //    debug("q=%d\n", q);

    u_fputc((A->states[q].flags & AUT_TERMINAL) ? 't' : ':', fstf->f);
    u_fputc(' ', fstf->f);

    for (transition_t * t = A->states[q].trans; t; t = t->next) {

      if (t->label == SYMBOL_DEF) { fatal_error("fst_file_write: symbol <def> in trans list ???\n"); }

      symbol_to_label(t->label, label);

      // debug("label=%S ", label->str); symbol_dump(t->label); endl();

      if (fstf->type == FST_LOCATE) {

	if (u_strcmp_char(label->str, "<PNC>") == 0) {

	  PNC_trans_write(fstf, t->to);

	} else if (u_strcmp_char(label->str, "<CHFA>") == 0 
                   || u_strcmp_char(label->str, "<NB>") == 0) {

	  CHFA_trans_write(fstf, t->to);

	} else if (u_strcmp_char(label->str, "<.>") == 0) {

	  LEXIC_trans_write(fstf, t->to);

	} else { goto normal_output; }

      } else {


normal_output:

	if ((idx = hash_str_table_idx_lookup(fstf->labels, label->str)) == -1) {
	  idx = hash_str_table_add(fstf->labels, label->str, u_strdup(label->str));
	}

	u_fprintf(fstf->f, "%d %d ", idx, t->to);
      }
    }

    if (A->states[q].defto != -1) {

      if (fstf->type != FST_GRAMMAR) { o_error("<def> label in text|locate automaton???\n"); }

      if ((idx = hash_str_table_idx_lookup(fstf->labels, deflabel)) == -1) {
	idx = hash_str_table_add(fstf->labels, deflabel, u_strdup(deflabel));
      }

      u_fprintf(fstf->f, "%d %d ", idx, A->states[q].defto);
    }

    u_fputc('\n', fstf->f);
  }

  u_fprints_char("f \n", fstf->f);

  ustring_delete(label);

  fstf->nbelems++;
}



autalmot_t * load_grammar_automaton(char * name, language_t * lang) {

  if (lang == NULL) { fatal_error("load grammar: LANG is not set\n"); }

//  debug("load_grammar_automaton(%s)\n", name);

  fst_file_in_t * fstin = fst_file_in_open(name, FST_GRAMMAR, lang);

//  debug("fstin=%d\n", fstin);

  if (fstin == NULL) {
    o_error("unable to open '%s'\n", name);
    return NULL; 
  }

  autalmot_t * A = fst_file_autalmot_load(fstin, 0);

//  debug("auto loaded.\n");

  fst_file_close(fstin);

//  debug("out\n");

  return A;
}
