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

#include <stdio.h>
#include <ctype.h>

#include "utils.h"
#include "ustring.h"
#include "hash_str_table.h"
#include "symbol.h"
#include "autalmot.h"
#include "fst_file.h"

#define MAXBUF  1024


/**
 * Loads the tags of the given .fst2 file. Returns 0 in case of success; -1 otherwise.
 * Note that the position in the file is unchanged after a call to this function.
 */
int load_fst_tags(fst_file_in_t* fst) {
//hash_str_table_empty(fstf->symbols);
/* We backup the position in the file, and we come back at the
 * beginning of the file */
long fpos=ftell(fst->f);
rewind(fst->f);
/* Now, we go to the tags section, skipping all the automata */
unichar buf[MAXBUF];
int i=0;
int len;
while (i<fst->nb_automata) {
   if ((len=u_fgets(buf,MAXBUF,fst->f))==EOF) {
      error("load_fst_tags: %s: unexpected EOF\n",fst->name);
      return -1;
   }
   if (buf[0]=='f' && isspace(buf[1])) {
      i++;
   }
   /* If we have read the beginning of a long line, we skip the rest of the line */
   while ((len==MAXBUF-1) && (buf[len-1]!='\n')) {
      len=u_fgets(buf,MAXBUF,fst->f);
   }
}
/* We set the tag loading function, depending on the kind of .fst2 we have */
symbol_t* (*load_symbol)(language_t*,unichar*);
load_symbol=(fst->type==FST_TEXT)?load_text_symbol:load_grammar_symbol;
Ustring* ustr=new_Ustring(64);
while (readline(ustr,fst->f) && ustr->str[0]!='f') {
   if (ustr->str[0]!='%' && ustr->str[0]!='@') {
      error("load_fst_tags: %s: bad symbol line: '%S'\n",fst->name,ustr->str);
      return -1;
   }
   chomp_new_line(ustr);
   /* +1 because we ignore the % or @ at the beginning of the line */
   symbol_t* symbol=load_symbol(fst->lang,ustr->str+1);
   if (symbol==NULL) {
      error("load_fst_tags: unable to load '%S'\n",ustr->str+1);
   }
   /* We add this symbol to the symbols of the .fst2 */
   get_value_index(ustr->str+1,fst->symbols,INSERT_IF_NEEDED,symbol);
}
if (*ustr->str==0) {
   fatal_error("load_fst_tags: unexpected EOF\n");
}
free_Ustring(ustr);
/* We set back the position in the file */
fseek(fst->f,fpos,SEEK_SET);
return 0;
}


/**
 * Loads a .fst2 file with the given name and type, according to the
 * given language description.
 */
fst_file_in_t* load_fst_file(char* fname,int type,language_t* language) {
fst_file_in_t* fstf=(fst_file_in_t*)malloc(sizeof(fst_file_in_t));
if (fstf==NULL) {
   fatal_error("Not enough memory in load_fst_file\n");
}
fstf->name=strdup(fname);
if ((fstf->f=u_fopen(fname,U_READ))==NULL) {
   error("load_fst_file: unable to open '%s' for reading\n",fname);
   goto error_fstf;
}
unichar buf[MAXBUF];
if (u_fgets(buf,MAXBUF,fstf->f)==EOF) {
   error("load_fst_file: '%s' is empty\n",fname);
   goto error_f;
}
if (!u_is_digit(*buf)) {
   error("load_fst_file: %s: bad file format\n",fname);
   goto error_f;
} 
fstf->nb_automata=u_parse_int(buf);
if (type!=FST_TEXT && type!=FST_GRAMMAR) {
   error("load_fst_file: bad fst_type=%d\n",type);
   goto error_f;
}
fstf->lang=language;
fstf->type=type;
fstf->pos0=ftell(fstf->f);
fstf->symbols=new_string_hash_ptr(64);
if (load_fst_tags(fstf)==-1) {
   error("load_fst_file: %s: cannot load symbols\n",fstf->name);
   goto error_symbols;
}
fstf->pos=0;
return fstf;
/* If an error occurs */
error_symbols: free_string_hash_ptr(fstf->symbols,(void(*)(void*))free_symbols);

error_f: fclose(fstf->f);

error_fstf: free(fstf->name);

free(fstf);
return NULL;
}


void fst_file_close(fst_file_in_t * fstf) {

  free(fstf->name);
  fclose(fstf->f);
  free_string_hash_ptr(fstf->symbols,(void(*)(void*))free_symbols);

  free(fstf);
}




autalmot_t * fst_file_autalmot_load_next(fst_file_in_t * fstf) {

//  debug("autalmot_load_next\n");

  if (fstf->pos >= fstf->nb_automata) { return NULL; }

  Ustring * ustr = new_Ustring();

  readline(ustr, fstf->f);
  chomp_new_line(ustr);

  // debug("line=%S\n", ustr->str);

  unichar * p = ustr->str;

  if (*p != '-') { fatal_error("fst_load_next: %s: bad file format\n", fstf->name); }

  p++;
  int i = u_parse_int(p, & p);

  //  debug("i=%d\n", i);

  if (i != fstf->pos + 1) { 
    fatal_error("fst_load: %s: parsing error with line '%S' ('-%d ...' expected)\n", fstf->name, ustr->str, fstf->pos + 1);
  }

  p++;  // p == name

//  debug("name=%S\n", p);

  autalmot_t * A = autalmot_new(p);

  while (readline(ustr, fstf->f) && *ustr->str != 'f') { /* read states */

    chomp_new_line(ustr);
    p = ustr->str;

    int i = autalmot_add_state(A, (*p == 't') ? AUT_FINAL : 0);

  //  debug("i=%d\n", i);

    while (*p && ! u_is_digit(*p)) { p++; }

    while (*p) { /* new trans */

      int lbl = u_parse_int(p, &p);

      while (*p && ! u_is_digit(*p)) { p++; }

      if (*p == 0) { fatal_error("fst_load: %S: bad file format (line='%S')\n", fstf->name, ustr->str); }
      int to = u_parse_int(p, &p);

      if (fstf->symbols->value[lbl]!=NULL) { /* if it is a good symbol (successfully loaded), make transition */
	      autalmot_add_trans(A, i, (symbol_t *) fstf->symbols->value[lbl], to);
      }

      while (*p && ! u_is_digit(*p)) { p++; }      
    }
  }

  if (*ustr->str == 0) { fatal_error("fst_file_read: unexpected end of file\n"); }

  if (A->nbstates == 0) {

    error("fst_file_read: automaton with no state\n");

  } else { autalmot_set_initial(A, 0); }

//  debug("%d states\n", A->nbstates);

  fstf->pos++;

  free_Ustring(ustr);

  return A;
}


void fst_file_seek(fst_file_in_t * fstin, int no) {

  if (no < 0 || no >= fstin->nb_automata) {
    fatal_error("fst_seek(%d): only %d automat%s in file\n", no, fstin->nb_automata, (no > 1) ? "a" : "on");
  }

  if (no < fstin->pos) {
    fseek(fstin->f, fstin->pos0, SEEK_SET);
    fstin->pos = 0;
  }

  unichar buf[MAXBUF];
  int len;

  while (fstin->pos < no) {
    
    if ((len = u_fgets(buf, MAXBUF, fstin->f)) == EOF) { fatal_error("fst_seek: %s: unexpected EOF\n", fstin->name); }

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
    error("fst_out_open: unable to open '%s'\n", fname);
    free(res);
    return NULL;
  }

  res->fstart = ftell(res->f);

  u_fprintf(res->f,"0000000000\n");

  res->name    = strdup(fname);
  res->type    = type;
  res->nbelems = 0;
  res->labels=new_string_hash(16);

  /* add <E> */

  unichar epsilon[] = { '<', 'E', '>', 0 };
  get_value_index(epsilon,res->labels);

  return res;
}



void output_labels(fst_file_out_t * fstout) {
  for (int i = 0; i < fstout->labels->size; i++) { u_fprintf(fstout->f, "%%%S\n", fstout->labels->value[i]); }
  u_fprintf(fstout->f, "f\n");
}


void fst_file_close(fst_file_out_t * fstout) {

  /* output labels */
  output_labels(fstout);

  /* writing header */

  fseek(fstout->f, fstout->fstart, SEEK_SET);

  unichar buf[16];
  u_strcpy(buf, "0000000000");

  int i = 9;
  int n = fstout->nbelems;

  if (n == 0) { error("fstfile without automaton\n"); }

  while (n) {
    buf[i--] = '0' + (n % 10);
    n = n / 10;
  }
    
  u_fprintf(fstout->f, "%S\n", buf);

  /* close all */

  fclose(fstout->f);

  free_string_hash(fstout->labels);
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

      idx=get_value_index(label,fstf->labels);
      u_fprintf(fstf->f, "%d %d ", idx, to);
    }
  }

}



void CHFA_trans_write(fst_file_out_t * fstf, int to) {

  unichar label[2];
  int idx;

  label[1] = 0;

  for (unichar C = '0'; C <= '9'; C++) {

    label[0] = C;

    idx=get_value_index(label,fstf->labels);

    u_fprintf(fstf->f, "%d %d ", idx, to);
  }
}



void LEXIC_trans_write(fst_file_out_t * fstf, int to) {

  unichar label[8];
  int idx;

  u_strcpy(label, "<MOT>");

  idx=get_value_index(label,fstf->labels);

  u_fprintf(fstf->f, "%d %d ", idx, to);

  u_strcpy(label, "<!MOT>");

  idx=get_value_index(label,fstf->labels);

  u_fprintf(fstf->f, "%d %d ", idx, to);
}



void fst_file_write(fst_file_out_t * fstf, const autalmot_t * A) {

  Ustring * label = new_Ustring();


  void (*symbol_to_label)(const symbol_t *, Ustring *) = NULL;
  
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

    u_fputc((A->states[q].flags & AUT_TERMINAL) ? 't' : ':', fstf->f);
    u_fputc(' ', fstf->f);

    for (transition_t * t = A->states[q].trans; t; t = t->next) {
      if (t->label == SYMBOL_DEF) { fatal_error("fst_file_write: symbol <def> in trans list ???\n"); }
      symbol_to_label(t->label,label);

      if (fstf->type == FST_LOCATE) {
         if (u_strcmp(label->str, "<PNC>") == 0) {
            PNC_trans_write(fstf, t->to);
         } else if (u_strcmp(label->str, "<CHFA>") == 0 || u_strcmp(label->str, "<NB>") == 0) {
            CHFA_trans_write(fstf, t->to);
         } else if (u_strcmp(label->str, "<.>") == 0) {
            LEXIC_trans_write(fstf, t->to);
         } else {
            goto normal_output;
         }
      } else {
         normal_output:
         idx=get_value_index(label->str,fstf->labels);
         u_fprintf(fstf->f, "%d %d ", idx, t->to);
      }
   }
   if (A->states[q].defto != -1) {
      if (fstf->type != FST_GRAMMAR) { error("<def> label in text|locate automaton???\n"); }
      idx=get_value_index(deflabel,fstf->labels);
      u_fprintf(fstf->f, "%d %d ", idx, A->states[q].defto);
    }

    u_fputc('\n', fstf->f);
  }

  u_fprintf(fstf->f,"f \n");

  free_Ustring(label);

  fstf->nbelems++;
}



autalmot_t * load_grammar_automaton(char * name, language_t * lang) {

  if (lang == NULL) { fatal_error("load grammar: LANG is not set\n"); }

//  debug("load_grammar_automaton(%s)\n", name);

  fst_file_in_t * fstin = load_fst_file(name, FST_GRAMMAR, lang);

//  debug("fstin=%d\n", fstin);

  if (fstin == NULL) {
    error("unable to open '%s'\n", name);
    return NULL; 
  }

  autalmot_t * A = fst_file_autalmot_load(fstin, 0);

//  debug("auto loaded.\n");

  fst_file_close(fstin);

//  debug("out\n");

  return A;
}
