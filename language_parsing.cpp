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

#include "language_parsing.h"
#include "utils.h"
#include <ctype.h>

//enum { STR = 0, GENERAL_SECTION, POS_SECTION, ENDSECTION, VAR, FLEX, CAT, COMPLET, EQUAL };

typedef struct keyword_t {
  char * str;
  int val;
} keyword_t;



static keyword_t keywords[] = {
  { "NAME", TOK_NAME },
  { "POS",  TOK_POS  },
  { ".", TOK_END },
  { "=", TOK_EQUAL },
  { "_", TOK_BLANK },
  { "discr:", TOK_DISCR },
  { "cat:", TOK_CAT },
  { "flex:", TOK_FLEX },
  { "inflex:", TOK_FLEX },
  { "complet:", TOK_COMPLET },
  { "complete:", TOK_COMPLET },
  { "IGNORE", TOK_IGNORE },
  { NULL, 0 }
};



token_t * token_new(unichar * str) {

  token_t * tok = (token_t *) xmalloc(sizeof(token_t));

  for (keyword_t * key = keywords; key->str; key++) {
    if (u_strcmp_char(str, key->str) == 0) {
      tok->type = key->val;
      tok->str  = NULL;
      tok->next = NULL;
      return tok;
    }
  }


  if (*str == '<') {

    /* strip ending '>' */

    unichar * p = u_strchr(str, '>');
    if (p == NULL || *(p + 1) != 0) { fatal_error("bad token: '%S'\n", str); }
    *p = 0;

    tok->type = TOK_ANGLE;
    tok->str  = u_strdup(str + 1);
    tok->next = NULL;

    return tok;
  }

  tok->type = TOK_STR;
  tok->str = u_strdup(str);
  tok->next = NULL;

  return tok;
}


void token_delete(token_t * tok) {
  if (tok == NULL) { return; }
  free(tok->str);
  free(tok);
}



void tokens_delete(token_t * tok) {

  while (tok) {
    token_t * next = tok->next;
    token_delete(tok);
    tok = next;
  }
}




tokens_list * tokens_list_new(token_t * tokens, tokens_list * next) {
  tokens_list * res = (tokens_list *) xmalloc(sizeof(tokens_list));
  res->tokens = tokens;
  res->next   = next;
  return res;
}


void tokens_list_delete(tokens_list * tlist) {
  tokens_list * next;
  while (tlist) {
    next = tlist->next;
    tokens_delete(tlist->tokens);
    free(tlist);
    tlist = next;
  }
}

tokens_list * tokens_list_concat(tokens_list * tlist, token_t * toks) {

  if (tlist == NULL) { return tokens_list_new(toks, NULL); }

  tokens_list * end;
  for (end = tlist; end->next; end = end->next);

  end->next = tokens_list_new(toks, NULL);

  return tlist;
}


pos_section_t * pos_section_new(char * name) {

  pos_section_t * res = (pos_section_t *) xmalloc(sizeof(pos_section_t));

  res->name = u_strdup_char(name);
  res->ignore = false;

  for (int i = 0; i < PART_NUM; i++) { res->parts[i] = NULL; }

  return res;
}


pos_section_t * pos_section_new(unichar * name) {

  pos_section_t * res = (pos_section_t *) xmalloc(sizeof(pos_section_t));

  res->name = u_strdup(name);
  res->ignore = false;
  for (int i = 0; i < PART_NUM; i++) { res->parts[i] = NULL; }

  return res;
}

void pos_section_delete(pos_section_t * sec) {
  free(sec->name);
  for (int i = 0; i < PART_NUM; i++) { tokens_list_delete(sec->parts[i]); }
  free(sec);
}

void pos_sections_delete(pos_section_t * sec) {
  pos_section_t * next;
  while (sec) {
    next = sec->next;
    pos_section_delete(sec);
    sec = next;
  }
}


language_tree_t * language_tree_new(unichar * name) {
  language_tree_t * tree = (language_tree_t *) xmalloc(sizeof(language_tree_t));
  tree->name = u_strdup(name);
  tree->pos_secs = NULL;
  return tree;
}

void language_tree_delete(language_tree_t * tree) {
  free(tree->name);
  pos_sections_delete(tree->pos_secs);
  free(tree);
}









void token_dump(token_t * tok, FILE * f) {
  switch (tok->type) {
  case TOK_STR:
    i_fprintf(f, "'%S'", tok->str);
    break;
  case TOK_ANGLE:
    i_fprintf(f, "<%S>", tok->str);
    break;
  case TOK_NAME:
    fprintf(f, "NAME");
    break;
  case TOK_POS:
    fprintf(f, "POS");
    break;
  case TOK_END:
    fprintf(f, "END");
    break;
  case TOK_DISCR:
    fprintf(f, "DISCR");
    break;
  case TOK_FLEX:
    fprintf(f, "FLEX");
    break;
  case TOK_CAT:
    fprintf(f, "CAT");
    break;
  case TOK_COMPLET:
    fprintf(f, "COMPLET");
    break;
  case TOK_EQUAL:
    fprintf(f, "=");
    break;
  case TOK_BLANK:
    fprintf(f, "_");
    break;
  default:
    fprintf(f, "WIERD????");
    break;
  }
}

void tokens_dump(token_t * tok, FILE * f) {
  fprintf(f, "(");
  while (tok) {
    token_dump(tok, f);
    fprintf(f, ", ");
    tok = tok->next;
  }
  fprintf(f, ")");
}

void tokens_list_dump(tokens_list * tlist, FILE * f) {
  while (tlist) {
    fprintf(f, "> ");
    tokens_dump(tlist->tokens, f);
    fprintf(f, "\n");
    tlist = tlist->next;
  }
}

void pos_section_dump(pos_section_t * sec, FILE * f) {
  i_fprintf(f, "name=%S\n", sec->name);
  fprintf(f, "\nflex:\n");
  tokens_list_dump(sec->parts[PART_FLEX], f);
  fprintf(f, "\ndiscr:\n");
  tokens_list_dump(sec->parts[PART_DISCR], f);
  fprintf(f, "\ncat:\n");
  tokens_list_dump(sec->parts[PART_CAT], f);
  fprintf(f, "\ncomplet:\n");
  tokens_list_dump(sec->parts[PART_COMP], f);
}


void language_tree_dump(language_tree_t * tree, FILE * f) {
  i_fprintf(f, "name=%S\n", tree->name);
  for (pos_section_t * sec = tree->pos_secs; sec; sec = sec->next) {
    fprintf(f, "\nPOS:\n");
    pos_section_dump(sec, f);
  }
}








static void line_cleanup(unichar * line) {
  while (*line) {
    if (*line == '\r' || *line == '\n' || *line == '#') { *line = 0; return; }
    line++;
  }
}



static token_t * tokenize(unichar * line) {

  while (isspace(*line)) { line++; }
  if (*line == 0) { return NULL; }

  token_t * tok;

  unichar * beg = line;

  while (! isspace(*line)) {
    if (*line == 0) { // end of line
      tok = token_new(beg);
      return tok;
    }
    line++;
  }

  *line = 0;
  tok = token_new(beg);
  tok->next = tokenize(line + 1);

  return tok;
}


static inline int check_cat_line(token_t * toks) {

  bool blank = false;

  if (toks == NULL) { error("check_cat_line: no tokens\n"); return -1; }

  if (toks->type != TOK_STR) { error("check_cat_line: string expected\n"); return -1; }

  toks = toks->next;

  if (toks == NULL) { error("line is really too short\n"); return -1; }

  if (toks->type != TOK_EQUAL) { error("'=' expected\n"); return -1; }

  toks = toks->next;

  if (toks == NULL) { error("line is too short\n"); return -1; }

  while (toks) {

    if (toks->type != TOK_STR) {

      if (toks->type == TOK_BLANK) {
	if (blank) {
	  error("too much '_'!\n");
	} else { blank = true; }

      } else { error("string expected\n"); return -1; }
    }
    toks = toks->next;
  }

  return 0;
}


static inline int check_flex_line(token_t * toks) {

  if (toks == NULL) { error("check_cat_line: no tokens\n"); return -1; }

  if (toks->type != TOK_STR) { error("check_cat_line: string expected\n"); return -1; }

  toks = toks->next;

  if (toks == NULL) { error("line is really too short\n"); return -1; }

  if (toks->type != TOK_EQUAL) { error("'=' expected\n"); return -1; }

  toks = toks->next;

  if (toks == NULL) { error("line is too short\n"); return -1; }

  while (toks) {
    if (toks->type != TOK_STR) { error("string expected\n"); return -1; }
    if (*toks->str == 0 || *(toks->str + 1)) { error("flex code '%S' is too long\n"); return -1; }
    toks = toks->next;
  }

  return 0;
}


static inline int check_complet_line(token_t * toks) {

  if (toks == NULL) { error("no tokens??\n"); return -1; }

  if (toks->type == TOK_BLANK) {
    if (toks->next) { error("complet section: '_' can only be specified alone\n"); return -1; }
    return 0;
  }

  while (toks) {
    if (toks->type != TOK_STR && toks->type != TOK_ANGLE) { error("string expected\n"); return -1; }
    toks = toks->next;
  }

  return 0;
}


#define MAXBUF  1024


pos_section_t * parse_pos_section(FILE * f) {

  unichar buf[MAXBUF];
  unichar line[MAXBUF];

  token_t * toks = NULL;

  while (toks == NULL) {

    if (u_fgets(line, MAXBUF, f) == 0) { return NULL; }

    line_cleanup(line);

    u_strcpy(buf, line);
    toks = tokenize(buf);
  }

  if (toks->type != TOK_POS) { fatal_error("parsing error: 'POS' section expected (%S).\n", line); }
  if (toks->next == NULL || toks->next->str == NULL) { fatal_error("POS section need a name\n"); }

  pos_section_t * section = pos_section_new(toks->next->str);

  tokens_delete(toks);

  int partid = PART_NUM;
  while (partid != -1 && u_fgets(line, MAXBUF, f)) {

    line_cleanup(line);
    u_strcpy(buf, line);
    toks = tokenize(buf);

    if (toks == NULL) { continue; }

    switch (toks->type) {

    case TOK_IGNORE:
      section->ignore = true;
      break;

    case TOK_DISCR:
      partid = PART_DISCR;
      tokens_delete(toks);
      break;

    case TOK_FLEX:
      partid = PART_FLEX;
      tokens_delete(toks);
      break;

    case TOK_CAT:
      partid = PART_CAT;
      tokens_delete(toks);
      break;

    case TOK_COMPLET:
      partid = PART_COMP;
      tokens_delete(toks);
      break;

    case TOK_END:
      partid = -1;
      tokens_delete(toks);
      break;

    case TOK_STR: // add tokenized line to current section part
    case TOK_ANGLE:
    case TOK_BLANK:
      switch (partid) {
      case PART_DISCR:
	if (section->parts[PART_DISCR] != NULL) {
          fatal_error("only one discriminant category could be specified.\n");
        }
      case PART_CAT:
	if (check_cat_line(toks) == -1) { die("bad line format: '%S'\n", line);	}
	break;
      case PART_FLEX:
	if (check_flex_line(toks) == -1) { die("bad line format: '%S'\n", line);	}
	break;	
      case PART_COMP:
	if (check_complet_line(toks) == -1) { die("bad complet line format: '%S'\n", line); }
	break;
      case PART_NUM:
	fatal_error("no section specified. (line '%S')\n", line);
      default:
	fatal_error("while parsing POS section: what am i doing here?\n");
      }
      section->parts[partid] = tokens_list_concat(section->parts[partid], toks);
      // section->parts[partid] = tokens_list_new(toks, section->parts[partid]);
      break;

    default:
      tokens_dump(toks);
      fatal_error("error while parsing POS section with line '%S'\n", line);
      break;
    }
  }


  return section;
}



language_tree_t * language_parse(FILE * f) {

  unichar buf[MAXBUF];

  token_t * toks = NULL;

  while (toks == NULL) {

    if ((u_fgets(buf, MAXBUF, f)) == 0) {
      error("parse language: file is empty\n");
      return NULL;
    }

    line_cleanup(buf);

    toks = tokenize(buf);
  }


  if ((toks->type != TOK_NAME) || (toks->next == NULL) || (toks->next->str == NULL)) {
    fatal_error("language need a name\n");
  }

  language_tree_t * tree = language_tree_new(toks->next->str);

  int nb = 0;

  while (pos_section_t * sec = parse_pos_section(f)) {
    sec->next = tree->pos_secs;
    tree->pos_secs = sec;
    nb++;
  }

  tokens_delete(toks);

  printf("%d POS definitions loaded.\n", nb);

  return tree;
}




