/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Tagset.h"
#include "Error.h"
#include <ctype.h>

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This structure defines a keyword and its associated value.
 */
typedef struct keyword_t {
   const char* str;
   int val;
} keyword_t;


/**
 * Here are the keywords that can be used in a tagset definition file.
 */
static const keyword_t keywords[] = {
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


/**
 * This function allocates and returns a token_t structure corresponding to the given
 * string.
 */
token_t* new_token_t(unichar* str) {
token_t* tok=(token_t*)malloc(sizeof(token_t));
if (tok==NULL) {
   fatal_alloc_error("new_token_t");
}
for (const keyword_t* key=keywords;key->str!=NULL;key++) {
   if (!u_strcmp(str, key->str)) {
      /* If the token is a keyword */
      tok->type=key->val;
      tok->str= NULL;
      tok->next=NULL;
      return tok;
   }
}
if (*str=='<') {
   /* If we have a '<', we look for the ending '>' */
   unichar* p=u_strchr(str,'>');
   if (p==NULL || *(p+1)!='\0') {
      fatal_error("Invalid token: '%S'\n",str);
   }
   *p='\0';
   tok->type=TOK_ANGLE;
   /* We copy the content between the angle brackets */
   tok->str=u_strdup(str+1);
   tok->next=NULL;
   return tok;
}
/* Otherwise, we create a default token with the string */
tok->type=TOK_STR;
tok->str=u_strdup(str);
tok->next=NULL;
return tok;
}


/**
 * Frees a single token_t.
 */
void free_token_t_element(token_t* tok) {
if (tok==NULL) return;
if (tok->str!=NULL) free(tok->str);
free(tok);
}


/**
 * Frees all the memory associated to a token_t list.
 */
void free_token_t(token_t* tok) {
while (tok!=NULL) {
   token_t* next=tok->next;
   free_token_t_element(tok);
   tok = next;
}
}


/**
 * Allocates, initializes and returns a new tokens list.
 */
tokens_list* new_tokens_list(token_t* tokens,tokens_list* next) {
tokens_list* res =(tokens_list*)malloc(sizeof(tokens_list));
if (res==NULL) {
   fatal_alloc_error("new_tokens_list");
}
res->tokens=tokens;
res->next=next;
return res;
}


/**
 * Frees all the memory associated to a tokens_list.
 */
void free_tokens_list(tokens_list* list) {
tokens_list* next;
while (list!=NULL) {
   next=list->next;
   free_token_t(list->tokens);
   free(list);
   list=next;
}
}


/**
 * This function appends the given token_t list at the end of the given tokens_list.
 */
tokens_list* tokens_list_append(tokens_list* list,token_t* toks) {
if (list==NULL) {
   return new_tokens_list(toks,NULL);
}
tokens_list* end;
for (end=list;end->next!=NULL;end=end->next) {}
end->next=new_tokens_list(toks,NULL);
return list;
}


/**
 * Allocates, initializes and returns a new POS definition structure.
 */
pos_section_t* new_pos_section_t(char* name) {
pos_section_t* res=(pos_section_t*)malloc(sizeof(pos_section_t));
if (res==NULL) {
   fatal_alloc_error("new_pos_section_t");
}
res->name=u_strdup(name);
res->ignore=false;
for (int i=0;i<PART_NUM;i++) {
   res->parts[i]=NULL;
}
return res;
}


/**
 * Allocates, initializes and returns a new POS definition structure.
 */
pos_section_t* new_pos_section_t(unichar* name) {
pos_section_t* res=(pos_section_t*)malloc(sizeof(pos_section_t));
if (res==NULL) {
   fatal_alloc_error("new_pos_section_t");
}
res->name=u_strdup(name);
res->ignore=false;
for (int i=0;i<PART_NUM;i++) {
   res->parts[i]=NULL;
}
return res;
}


/**
 * Frees all the memory associated to the single POS definition structure.
 */
void free_pos_section_t_element(pos_section_t* pos_section) {
if (pos_section==NULL) return;
if (pos_section->name!=NULL) free(pos_section->name);
for (int i=0;i<PART_NUM;i++) {
   free_tokens_list(pos_section->parts[i]);
}
free(pos_section);
}


/**
 * Frees all the memory associated to the given POS definition structure list.
 */
void free_pos_section_t(pos_section_t *list) {
pos_section_t* next;
while (list!=NULL) {
   next=list->next;
   free_pos_section_t_element(list);
   list=next;
}
}


/**
 * Allocates, initializes and returns a new tagset_t structure.
 */
tagset_t* new_tagset_t(unichar* name) {
tagset_t* tree=(tagset_t*)malloc(sizeof(tagset_t));
if (tree==NULL) {
   fatal_alloc_error("new_tagset_t");
}
tree->name=u_strdup(name);
tree->pos_sections=NULL;
return tree;
}


/**
 * Frees all the memory associated to the given tagset_t structure.
 */
void free_tagset_t(tagset_t* tree) {
if (tree==NULL) return;
if (tree->name!=NULL) free(tree->name);
free_pos_section_t(tree->pos_sections);
free(tree);
}


/**
 * Replaces the firt occurrence of \r \n or # by \0 and returns.
 */
static void line_cleanup(unichar* line) {
while (*line) {
   if (*line=='\r' || *line=='\n' || *line=='#') {
      (*line)='\0';
      return;
   }
   line++;
}
}


/**
 * This function tokenizes the given line, taking spaces as separators.
 * It returns a list of tokens.
 */
static token_t* tokenize(unichar* line) {
/* We skip spaces */
while (isspace(*line)) {
   line++;
}
if (*line=='\0') {
   /* Nothing to read if the line was made of spaces */
   return NULL;
}
token_t* tok;
unichar* tmp=line;
while (!isspace(*line)) {
   if (*line=='\0') {
      /* End of line */
      tok=new_token_t(tmp);
      return tok;
   }
   line++;
}
*line='\0';
tok=new_token_t(tmp);
tok->next=tokenize(line+1);
return tok;
}


/**
 * This function takes a token sequence that is supposed to belong to
 * the "cat:" part of a POS definition. If returns -1 in case the token sequence
 * is invalid; 0 otherwise.
 */
static inline int check_cat_line(token_t* toks) {
bool blank=false;
if (toks==NULL) {
   error("check_cat_line: no tokens\n");
   return -1;
}
if (toks->type!=TOK_STR) {
   error("check_cat_line: string expected\n");
   return -1;
}
toks=toks->next;
if (toks==NULL) {
   error("check_cat_line: line is too short\n");
   return -1;
}
if (toks->type!=TOK_EQUAL) {
   error("check_cat_line: '=' expected\n");
   return -1;
}
toks = toks->next;
if (toks==NULL) {
   error("check_cat_line: line is too short\n");
   return -1;
}
while (toks!=NULL) {
   if (toks->type!=TOK_STR) {
      if (toks->type==TOK_BLANK) {
         if (blank) {
            error("check_cat_line: too much '_'\n");
         } else {
            blank=true;
         }
      } else {
         error("check_cat_line: string expected\n");
         return -1;
      }
   }
   toks=toks->next;
}
return 0;
}


/**
 * This function takes a token sequence that is supposed to belong to
 * the "flex:" part of a POS definition. If returns -1 in case the token sequence
 * is invalid; 0 otherwise.
 */
static inline int check_flex_line(token_t* toks) {
if (toks==NULL) {
   error("check_flex_line: no tokens\n");
   return -1;
}
if (toks->type!=TOK_STR) {
   error("check_flex_line: string expected\n");
   return -1;
}
toks=toks->next;
if (toks==NULL) {
   error("check_flex_line: line is too short\n");
   return -1;
}
if (toks->type!=TOK_EQUAL) {
   error("check_flex_line: '=' expected\n");
   return -1;
}
toks=toks->next;
if (toks==NULL) {
   error("check_flex_line: line is too short\n");
   return -1;
}
while (toks!=NULL) {
   if (toks->type!=TOK_STR) {
      error("check_flex_line: string expected\n");
      return -1;
   }
   if (*toks->str=='\0' || *(toks->str+1)!='\0') {
      error("check_flex_line: inflectional code '%S' should be 1 char long\n",toks->str);
      return -1;
   }
   toks=toks->next;
}
return 0;
}


/**
 * This function takes a token sequence that is supposed to belong to
 * the "complete:" part of a POS definition. If returns -1 in case the
 * token sequence is invalid; 0 otherwise.
 */
static inline int check_complete_line(token_t * toks) {
if (toks==NULL) {
   error("check_complete_line: no tokens\n");
   return -1;
}
if (toks->type==TOK_BLANK) {
   if (toks->next!=NULL) {
      error("check_complete_line: '_' can only be specified alone\n");
      return -1;
   }
   return 0;
}
while (toks!=NULL) {
   if (toks->type!=TOK_STR && toks->type!=TOK_ANGLE) {
      error("check_complete_line: string expected\n");
      return -1;
   }
   toks=toks->next;
}
return 0;
}


#define MAXBUF  1024


/**
 * This function reads a POS section from the given tagset file and returns the
 * corresponding structure.
 */
pos_section_t* parse_pos_section(U_FILE* f) {
unichar buf[MAXBUF];
unichar line[MAXBUF];
/* We look for a non empty line containing "POS xxx" */
token_t* toks=NULL;
while (toks==NULL) {
   if (u_fgets(line,MAXBUF,f)==EOF) {
      return NULL;
   }
   line_cleanup(line);
   u_strcpy(buf,line);
   toks=tokenize(buf);
}
if (toks->type!=TOK_POS) {
   fatal_error("Parsing error: 'POS' section expected (%S).\n",line);
}
if (toks->next==NULL || toks->next->str==NULL) {
   fatal_error("POS section needs a name\n");
}
pos_section_t* pos_section=new_pos_section_t(toks->next->str);
free_token_t(toks);
/* Then, we look for all the elements of the POS definition */
int partid=PART_NUM;
while (partid!=-1 && u_fgets(line,MAXBUF,f)>0) {
   line_cleanup(line);
   u_strcpy(buf,line);
   toks=tokenize(buf);
   if (toks==NULL) {
      continue;
   }
   switch (toks->type) {
      case TOK_IGNORE:
         pos_section->ignore=true;
         break;

      case TOK_DISCR:
         partid=PART_DISCR;
         free_token_t(toks);
         break;

      case TOK_FLEX:
         partid=PART_FLEX;
         free_token_t(toks);
         break;

      case TOK_CAT:
         partid=PART_CAT;
         free_token_t(toks);
         break;

      case TOK_COMPLET:
         partid=PART_COMP;
         free_token_t(toks);
         break;

      case TOK_END:
         partid=-1;
         free_token_t(toks);
         break;

      /* We add a tokenized line to the current POS section part */
      case TOK_STR:
      case TOK_ANGLE:
      case TOK_BLANK:
         switch (partid) {
            case PART_DISCR:
               if (pos_section->parts[PART_DISCR]!=NULL) {
                  fatal_error("Only one discriminant category can be specified.\n");
               }

            case PART_CAT:
               if (check_cat_line(toks)==-1) {
                  fatal_error("Bad cat line format: '%S'\n", line);
               }
               break;

            case PART_FLEX:
               if (check_flex_line(toks)==-1) {
                  fatal_error("Bad flex line format: '%S'\n", line);
               }
               break;

            case PART_COMP:
               if (check_complete_line(toks)==-1) {
                  fatal_error("Bad complete line format: '%S'\n", line);
               }
               break;

            case PART_NUM:
               fatal_error("No section specified. (line '%S')\n", line);

            default:	fatal_error("While parsing POS section: what am i doing here?\n");
         }
         pos_section->parts[partid]=tokens_list_append(pos_section->parts[partid],toks);
         break;

      default: fatal_error("Error while parsing POS section with line '%S'\n", line);
   }
}
return pos_section;
}


/**
 * This function loads the given tagset file and returns the corresponding tagset_t
 * structure.
 */
tagset_t* load_tagset(U_FILE* f) {
unichar buf[MAXBUF];
token_t* toks=NULL;
/* First, we read the language name */
while (toks==NULL) {
   if ((u_fgets(buf,MAXBUF,f)) == EOF) {
      error("Tagset definition file is empty\n");
      return NULL;
   }
   line_cleanup(buf);
   toks=tokenize(buf);
}
if (toks->type!=TOK_NAME || toks->next==NULL || toks->next->str==NULL) {
   fatal_error("Tagset language needs a name\n");
}
tagset_t* tagset=new_tagset_t(toks->next->str);
int nb=0;
pos_section_t* pos;
while ((pos=parse_pos_section(f))!=NULL) {
   pos->next=tagset->pos_sections;
   tagset->pos_sections=pos;
   nb++;
}
free_token_t(toks);
u_printf("%d POS definitions loaded.\n",nb);
return tagset;
}

} // namespace unitex
