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

#include "Unicode.h"
#include "DELA.h"
#include "Error.h"
#include "StringParsing.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Allocates, initializes and returns a dela_entry.
 */
struct dela_entry* new_dela_entry(const unichar* inflected,const unichar* lemma,const unichar* code,Abstract_allocator prv_alloc) {
struct dela_entry* res=(struct dela_entry*)malloc_cb(sizeof(struct dela_entry),prv_alloc);
if (res==NULL) {
   fatal_alloc_error("new_dela_entry");
}
res->inflected=u_strdup(inflected,prv_alloc);
res->lemma=u_strdup(lemma,prv_alloc);
res->n_semantic_codes=1;
res->semantic_codes[0]=u_strdup(code,prv_alloc);
res->n_inflectional_codes=0;
res->n_filter_codes=0;
res->filter_codes[0]= NULL;
return res;
}


/**
 * Returns a copy of the given entry.
 * WARNING: filters are not taken into account since they aren't
 *          used, except in the inflection module
 */
struct dela_entry* clone_dela_entry(const struct dela_entry* entry,Abstract_allocator prv_alloc) {
if (entry==NULL) return NULL;
int i;
struct dela_entry* res=(struct dela_entry*)malloc_cb(sizeof(struct dela_entry),prv_alloc);
if (res==NULL) {
   fatal_alloc_error("clone_dela_entry");
}
res->inflected=u_strdup(entry->inflected,prv_alloc);
res->lemma=u_strdup(entry->lemma,prv_alloc);
res->n_semantic_codes=entry->n_semantic_codes;
for (i=0;i<res->n_semantic_codes;i++) {
   res->semantic_codes[i]=u_strdup(entry->semantic_codes[i],prv_alloc);
}
res->n_inflectional_codes=entry->n_inflectional_codes;
for (i=0;i<res->n_inflectional_codes;i++) {
   res->inflectional_codes[i]=u_strdup(entry->inflectional_codes[i],prv_alloc);
}
res->n_filter_codes= entry->n_filter_codes;
for (i=0;i<res->n_filter_codes;i++) {
   res->filter_codes[i]=u_strdup(entry->filter_codes[i],prv_alloc);
}
return res;
}



/**
 * Returns 1 if a and b are identical; 0 otherwise.
 * a and b are supposed to be valid entries, that is to say, entries
 * will non NULL inflected forms and lemmas.
 *
 * WARNING: this comparison does not take inflection filters into account
 */
int equal(const struct dela_entry* a,const struct dela_entry* b) {
int i;
if (a==b) return 1;
if (a==NULL || b==NULL) return 0;
if (u_strcmp(a->inflected,b->inflected)) return 0;
if (u_strcmp(a->lemma,b->lemma)) return 0;
if (a->n_semantic_codes!=b->n_semantic_codes) return 0;
for (i=0;i<a->n_semantic_codes;i++) {
   if (u_strcmp(a->semantic_codes[i],b->semantic_codes[i])) return 0;
}
if (a->n_inflectional_codes!=b->n_inflectional_codes) return 0;
for (i=0;i<a->n_inflectional_codes;i++) {
   if (u_strcmp(a->inflectional_codes[i],b->inflectional_codes[i])) return 0;
}
return 1;
}


/**
 * Returns 1 if s contains several times the same characters; 0 otherwise.
 */
int is_duplicate_char_in_inflectional_code(const unichar* s) {
for (int i=0;s[i]!='\0';i++) {
   if (NULL!=u_strchr(s+i+1,s[i])) {
      return 1;
   }
}
return 0;
}


/**
 * Tokenizes a DELAF line and returns the information in a dela_entry structure, or
 * NULL if there is an error in the line. The second parameter indicates if
 * comments are allowed at the end of the line or not. If 'verbose' is NULL, the
 * function must print messages if there is an error; otherwise, the function prints
 * no error message and stores an error code in '*verbose'.
 * if strict_unprotected is not 0, we don't accept unprotected comma and dot (for CheckDic)
 */
static struct dela_entry* tokenize_DELAF_line(const unichar* line,int comments_allowed,
                                              int *verbose, int strict_unprotected,Abstract_allocator prv_alloc) {
struct dela_entry* res;
int i,val;
if (line==NULL) {
  if (!verbose) error("Internal NULL error in tokenize_DELAF_line\n");
   else (*verbose)=P_NULL_STRING;
  return NULL;
}
unichar* temp=(unichar*)malloc_cb(sizeof(unichar)*(1+u_strlen(line)),prv_alloc);
if (temp==NULL) {
  fatal_alloc_error("tokenize_DELAF_line");
}
/* Initialization of the result structure */
res=(struct dela_entry*)malloc_cb(sizeof(struct dela_entry),prv_alloc);
if (res==NULL) {
  fatal_alloc_error("tokenize_DELAF_line");
}
res->inflected=NULL;
res->lemma=NULL;
res->n_semantic_codes=1;   /* 0 would be an error (no grammatical code) */
res->semantic_codes[0]=NULL;
res->n_inflectional_codes=0;
res->inflectional_codes[0]=NULL;
res->n_filter_codes=0;
res->filter_codes[0]=NULL;
i=0;
/*
 * We read the inflected part
 */
val=parse_string(line,&i,temp,P_COMMA,strict_unprotected ? P_DOT : P_EMPTY,P_EMPTY);
if (val==P_BACKSLASH_AT_END) {
   if (!verbose) error("***Dictionary error: backslash at end of line\n_%S_\n",line);
   else (*verbose)=P_BACKSLASH_AT_END;
   goto error;
}
/* If we are at the end of line, it's an error */
if (line[i]=='\0') {
   if (!verbose) {
      error("***Dictionary error: unexpected end of line\n_%S_\n",line);
   } else (*verbose)=P_UNEXPECTED_END_OF_LINE;
   goto error;
}
/* The inflected form cannot be empty */
if (temp[0]=='\0') {
   if (!verbose) {
      error("***Dictionary error: empty inflected form in line\n_%S_\n",line);
   } else (*verbose)=P_EMPTY_INFLECTED_FORM;
   goto error;
}
if (val==P_FORBIDDEN_CHAR) {
   /* If the inflected form contains an unprotected dot, it's an error */
   if (!verbose) {
      error("***Dictionary error: unprotected dot in inflected form in line\n_%S_\n",line);
   } else (*verbose)=P_UNPROTECTED_DOT;
   goto error;
}
res->inflected=u_strdup(temp,prv_alloc);
/*
 * We read the lemma part
 */
i++;
val=parse_string(line,&i,temp,P_DOT,strict_unprotected ? P_COMMA : P_EMPTY,P_EMPTY);
if (val==P_BACKSLASH_AT_END) {
   if (!verbose) error("***Dictionary error: backslash at end of line\n_%S_\n",line);
   else (*verbose)=P_BACKSLASH_AT_END;
   goto error;
}
if (val==P_FORBIDDEN_CHAR) {
   /* If the lemma contains an unprotected comma, it's an error */
   if (!verbose) {
      error("***Dictionary error: unprotected comma in lemma in line\n_%S_\n",line);
   } else (*verbose)=P_UNPROTECTED_COMMA;
   goto error;
}
/* If we are at the end of line, it's an error */
if (line[i]=='\0') {
   if (!verbose) {
      error("***Dictionary error: unexpected end of line\n_%S_\n",line);
   } else (*verbose)=P_UNEXPECTED_END_OF_LINE;
   goto error;
}
if (temp[0]=='\0') {
  /* If the lemma is empty like in "eat,.V:W", it is supposed to be
   * the same as the inflected form. */
  res->lemma=u_strdup(res->inflected,prv_alloc);
}
else {
  /* Otherwise, we copy it */
  res->lemma=u_strdup(temp,prv_alloc);
}
/*
 * We read the grammatical code
 */
i++;
val=parse_string(line,&i,temp,P_PLUS_COLON_SLASH,P_EMPTY,P_EMPTY);
if (val==P_BACKSLASH_AT_END) {
   if (!verbose) error("***Dictionary error: backslash at end of line\n_%S_\n",line);
   else (*verbose)=P_BACKSLASH_AT_END;
   goto error;
}
/* The grammatical code cannot be empty */
if (temp[0]=='\0') {
   if (!verbose) {
      error("***Dictionary error: empty grammatical code in line\n_%S_\n",line);
   } else (*verbose)=P_EMPTY_SEMANTIC_CODE;
   goto error;
}
res->semantic_codes[0]=u_strdup(temp,prv_alloc);
/*
 * Now we read the other gramatical and semantic codes if any
 */
while (res->n_semantic_codes<MAX_SEMANTIC_CODES && line[i]=='+') {
  i++;
   val=parse_string(line,&i,temp,P_PLUS_COLON_SLASH,P_EMPTY,P_EMPTY);
   if (val==P_BACKSLASH_AT_END) {
      if (!verbose) error("***Dictionary error: backslash at end of line\n_%S_\n",line);
      else (*verbose)=P_BACKSLASH_AT_END;
      goto error;
   }
   /* A grammatical or semantic code cannot be empty */
   if (temp[0]=='\0') {
      if (!verbose) {
         error("***Dictionary error: empty semantic code in line\n_%S_\n",line);
      } else (*verbose)=P_EMPTY_SEMANTIC_CODE;
      goto error;
   }
   res->semantic_codes[res->n_semantic_codes]=u_strdup(temp,prv_alloc);
  (res->n_semantic_codes)++;
}
/*
 * Then we read the inflectional codes if any
 */
while (res->n_inflectional_codes<MAX_INFLECTIONAL_CODES && line[i]==':') {
  i++;
   val=parse_string(line,&i,temp,P_COLON_SLASH,P_EMPTY,P_EMPTY);
   if (val==P_BACKSLASH_AT_END) {
      if (!verbose) error("***Dictionary error: backslash at end of line\n_%S_\n",line);
      else (*verbose)=P_BACKSLASH_AT_END;
      goto error;
   }
   /* An inflectional code cannot be empty */
   if (temp[0]=='\0') {
      if (!verbose) {
         error("***Dictionary error: empty inflectional code in line\n_%S_\n",line);
      } else (*verbose)=P_EMPTY_INFLECTIONAL_CODE;
      goto error;
   }
   res->inflectional_codes[res->n_inflectional_codes]=u_strdup(temp,prv_alloc);
  (res->n_inflectional_codes)++;
}
/* Finally we check if there is a comment */
if (line[i]=='/' && !comments_allowed) {
   if (!verbose) error("***Dictionary error: unexpected comment at end of entry\n_%S_\n",line);
   else (*verbose)=P_UNEXPECTED_COMMENT;
   goto error;
}
/* We check if a character appears several times in an inflectional code like :KKms */
for (int il=0;il<res->n_inflectional_codes;il++) {
   if (is_duplicate_char_in_inflectional_code(res->inflectional_codes[il])) {
      if (!verbose) error("***Dictionary error: duplicate character in an inflectional code\n_%S_\n",line);
         else (*verbose)=P_DUPLICATE_CHAR_IN_INFLECTIONAL_CODE;
      goto error;
   }
}
/* We check if an inflectional code is a subset of another like :Kms:ms */
for (int il=0;il<res->n_inflectional_codes;il++) {
   for (int j=0;j<res->n_inflectional_codes;j++) {
      if (il==j) continue;
      if (one_inflectional_codes_contains_the_other(res->inflectional_codes[il],
                                                    res->inflectional_codes[j])) {
         if (!verbose) error("***Dictionary error: an inflectional code is a subset of another\n_%S_\n",line);
         else (*verbose)=P_DUPLICATE_INFLECTIONAL_CODE;
         goto error;
      }
   }
}
/* We check if a semantic code appears twice as in V+z1+z1 */
for (int il=0;il<res->n_semantic_codes;il++) {
   for (int j=0;j<res->n_semantic_codes;j++) {
      if (il==j) continue;
      if (!u_strcmp(res->semantic_codes[il],res->semantic_codes[j])) {
         if (!verbose) error("***Dictionary error: duplicate semantic code\n_%S_\n",line);
         else (*verbose)=P_DUPLICATE_SEMANTIC_CODE;
         goto error;
      }
   }
}
free_cb(temp,prv_alloc);
return res;
error:
free_dela_entry(res,prv_alloc);
free_cb(temp,prv_alloc);
return NULL;
}


/**
 * Tokenizes a DELAF line and returns the information in a dela_entry structure, or
 * NULL if there is an error in the line. The second parameter indicates if
 * comments are allowed at the end of the line or not. 'keep_equal_signs' indicates
 * if protected equal signs must be unprotected. This option is used by the Compress
 * program. If 'verbose' is NULL, the
 * function must print messages if there is an error; otherwise, the function prints
 * no error message and stores an error code in '*verbose'.
 */
struct dela_entry* tokenize_DELAF_line(const unichar* line,int comments_allowed,
                                       int *verbose,Abstract_allocator prv_alloc) {
return tokenize_DELAF_line(line,comments_allowed,verbose,0,prv_alloc);
}

/**
 * Tokenizes a DELAF line and returns the information in a dela_entry structure, or
 * NULL if there is an error in the line. The second parameter indicates if
 * comments are allowed at the end of the line or not. The function prints
 * error message to the standard output in case of error.
 */
struct dela_entry* tokenize_DELAF_line(const unichar* line,int comments_allowed,Abstract_allocator prv_alloc) {
return tokenize_DELAF_line(line,comments_allowed,NULL,0,prv_alloc);
}


/**
 * Tokenizes a DELAF line and returns the information in a dela_entry structure, or
 * NULL if there is an error in the line. Comments are allowed at the end of the
 * line.
 */
struct dela_entry* tokenize_DELAF_line(const unichar* line,Abstract_allocator prv_alloc) {
return tokenize_DELAF_line(line,1,NULL,0,prv_alloc);
}


/**
 * Tokenizes a DELAF line and returns the information in a dela_entry structure, or
 * NULL if there is an error in the line. Comments are not allowed at the end of the
 * line.
 * WARNING: this function does not perform all error checks. It should only be used
 *          when the input is known to be safe, like a string generated by the
 *          uncompress_entry function.
 */
struct dela_entry* tokenize_DELAF_line_opt(const unichar* line,Abstract_allocator prv_alloc) {
struct dela_entry* res;
unichar* temp=(unichar*)malloc_cb(sizeof(unichar)*(1+u_strlen(line)),prv_alloc);
if (temp==NULL) {
  fatal_alloc_error("tokenize_DELAF_line");
}
int i,j;
if (line==NULL) {
  fatal_error("Internal NULL error in tokenize_DELAF_line\n");
}
/* Initialization of the result structure */
res=(struct dela_entry*)malloc_cb(sizeof(struct dela_entry),prv_alloc);
if (res==NULL) {
  fatal_error("Not enough memory in tokenize_DELA_line\n");
}
res->inflected=NULL;
res->lemma=NULL;
res->n_semantic_codes=1;   /* 0 would be an error (no grammatical code) */
res->semantic_codes[0]=NULL;
res->n_inflectional_codes=0;
res->inflectional_codes[0]=NULL;
res->n_filter_codes=0;
res->filter_codes[0]=NULL;
/*
 * We read the inflected part
 */
i=0;
j=0;
while (line[i]!='\0' && line[i]!=',') {
  /* If there is a backslash, we must unprotect a character */
  if (line[i]=='\\') {
    i++;
    /* If the backslash is at the end of line, it's an error */
    if (line[i]=='\0') {
      fatal_error("***Dictionary error: incorrect line\n_%S_\n",line);
    }
    else if (line[i]=='=') {
      temp[j++]='\\';
    }
  }
  temp[j++]=line[i++];
}
temp[j]='\0';
/* If we are at the end of line, it's an error */
if (line[i]=='\0') {
  fatal_error("***Dictionary error: incorrect line\n_%S_\n",line);
}
res->inflected=u_strdup(temp,prv_alloc);
/*
 * We read the lemma part
 */
i++;
j=0;
while (line[i]!='\0' && line[i]!='.') {
  /* If there is a backslash, we must unprotect a character */
  if (line[i]=='\\') {
    i++;
    /* If the backslash is at the end of line, it's an error */
    if (line[i]=='\0') {
      fatal_error("***Dictionary error: incorrect line\n_%S_\n",line);
    } else if (line[i]=='=') {
      temp[j++]='\\';
    }
  }
  temp[j++]=line[i++];
}
temp[j]='\0';
/* If we are at the end of line, it's an error */
if (line[i]=='\0') {
  fatal_error("***Dictionary error: incorrect line\n_%S_\n",line);
}
if (j==0) {
  /* If the lemma is empty like in "eat,.V:W", it is supposed to be
   * the same as the inflected form. */
  res->lemma=u_strdup(res->inflected,prv_alloc);
}
else {
  /* Otherwise, we copy it */
  res->lemma=u_strdup(temp,prv_alloc);
}
/*
 * We read the grammatical code
 */
i++;
j=0;
while (line[i]!='\0' && line[i]!='+' && line[i]!='/' && line[i]!=':') {
  /* If there is a backslash, we must unprotect a character */
  if (line[i]=='\\') {
    i++;
    /* If the backslash is at the end of line, it's an error */
    if (line[i]=='\0') {
      fatal_error("***Dictionary error: incorrect line\n_%S_\n",line);
    }
  }
  temp[j++]=line[i++];
}
temp[j]='\0';
res->semantic_codes[0]=u_strdup(temp,prv_alloc);
/*
 * Now we read the other grammatical and semantic codes if any
 */
while (res->n_semantic_codes<MAX_SEMANTIC_CODES && line[i]=='+') {
  i++;
  j=0;
  while (line[i]!='\0' && line[i]!='+' && line[i]!=':' && line[i]!='/') {
    /* If there is a backslash, we must unprotect a character */
    if (line[i]=='\\') {
      i++;
      /* If the backslash is at the end of line, it's an error */
      if (line[i]=='\0') {
        fatal_error("***Dictionary error: incorrect line\n_%S_\n",line);
      }
    }
    temp[j++]=line[i++];
  }
  temp[j]='\0';
  res->semantic_codes[res->n_semantic_codes]=u_strdup(temp,prv_alloc);
  (res->n_semantic_codes)++;
}
/*
 * Then we read the inflectional codes if any
 */
while (res->n_inflectional_codes<MAX_INFLECTIONAL_CODES && line[i]==':') {
  i++;
  j=0;
  while (line[i]!='\0' && line[i]!=':' && line[i]!='/') {
    /* If there is a backslash, we must unprotect a character */
    if (line[i]=='\\') {
      i++;
      /* If the backslash is at the end of line, it's an error */
      if (line[i]=='\0') {
        fatal_error("***Dictionary error: incorrect line\n_%S_\n",line);
      }
    }
    temp[j++]=line[i++];
  }
  temp[j]='\0';
  res->inflectional_codes[res->n_inflectional_codes]=u_strdup(temp,prv_alloc);
  (res->n_inflectional_codes)++;
}
free_cb(temp,prv_alloc);
return res;
}



/**
 * This function tokenizes a tag token like {today,.ADV} and returns the
 * information in a dela_entry structure, or NULL if there is an error in
 * the tag.
 */
struct dela_entry* tokenize_tag_token(const unichar* tag,
          int emit_error,Abstract_allocator prv_alloc) {
if (tag==NULL || tag[0]!='{') {
  error("Internal error in tokenize_tag_token\n");
  return NULL;
}
/* We copy the tag content without the round brackets in a string. We
 * must take care not to unprotect chars during this operation, since
 * with a tag like:
 *    {3\,14,PI.CONST}
 * we would parse the following invalid dictionary entry:
 *    3,14,PI.CONST
 */
int i=1;
unichar* temp=(unichar*)malloc_cb(sizeof(unichar)*(1+u_strlen(tag)),prv_alloc);
if (temp==NULL) {
  fatal_alloc_error("tokenize_DELAF_line");
}
int val=parse_string(tag,&i,temp,P_CLOSING_ROUND_BRACKET,P_EMPTY,NULL);
if (tag[i]!='}' || val!=P_OK) {
   if (emit_error) error("Invalid tag in tokenize_tag_token\n");
   free_cb(temp,prv_alloc);
   return NULL;
}
/* And we tokenize it as a normal DELAF line */
int foo_error;
struct dela_entry* result=tokenize_DELAF_line(temp,0,&foo_error,0,prv_alloc);
free_cb(temp,prv_alloc);
return result;
}


/**
 * Tokenizes a DELAS line and returns the information in a dela_entry structure, or
 * NULL if there is an error in the line. If 'verbose' is NULL, the
 * function must print messages if there is an error; otherwise, the function prints
 * no error message and stores an error code in '*verbose'.
 */
struct dela_entry* tokenize_DELAS_line(const unichar* line,int *verbose,Abstract_allocator prv_alloc) {
struct dela_entry* res;
int i,val;
if (line==NULL) {
   if (!verbose) error("Internal NULL error in tokenize_DELAS_line\n");
   else (*verbose)=P_NULL_STRING;
   return NULL;
}
unichar* temp=(unichar*)malloc_cb(sizeof(unichar)*(1+u_strlen(line)),prv_alloc);
if (temp==NULL) {
  fatal_alloc_error("tokenize_DELAF_line");
}
/* Initialization of the result structure */
res=(struct dela_entry*)malloc_cb(sizeof(struct dela_entry),prv_alloc);
if (res==NULL) {
   fatal_alloc_error("tokenize_DELAS_line");
}
res->inflected=NULL;
res->lemma=NULL;
res->n_semantic_codes=1;   /* 0 would be an error (no grammatical code) */
res->semantic_codes[0]=NULL;
res->n_inflectional_codes=0;
res->inflectional_codes[0]=NULL;
res->n_filter_codes = 0;
res->filter_codes[0] = NULL;
i=0;
/*
 * We read the inflected part
 */
val=parse_string(line,&i,temp,P_COMMA,P_EMPTY,P_EMPTY);
if (val==P_BACKSLASH_AT_END) {
   if (!verbose) error("***Dictionary error: incorrect line\n_%S_\n",line);
   else (*verbose)=P_BACKSLASH_AT_END;
   goto error;
}
/* If we are at the end of line, it's an error */
if (line[i]=='\0') {
   if (!verbose) {
      error("***Dictionary error: incorrect line\n_%S_\n",line);
   } else (*verbose)=P_UNEXPECTED_END_OF_LINE;
   goto error;
}
/* The lemma form cannot be empty */
if (temp[0]=='\0') {
   if (!verbose) {
      error("***Dictionary error: incorrect line\n_%S_\n",line);
   } else (*verbose)=P_EMPTY_LEMMA;
   goto error;
}
res->lemma=u_strdup(temp,prv_alloc);
/*
 * We read the grammatical code
 */
i++;
val=parse_string(line,&i,temp,P_PLUS_COLON_SLASH_EXCLAMATION_OPENING_BRACKET,P_EMPTY,P_EMPTY);
if (val==P_BACKSLASH_AT_END) {
   if (!verbose) error("***Dictionary error: incorrect line\n_%S_\n",line);
   else (*verbose)=P_BACKSLASH_AT_END;
   goto error;
}
/* The grammatical code cannot be empty */
if (temp[0]=='\0') {
   if (!verbose) {
      error("***Dictionary error: incorrect line\n_%S_\n",line);
   } else (*verbose)=P_EMPTY_SEMANTIC_CODE;
   goto error;
}
res->semantic_codes[0]=u_strdup(temp,prv_alloc);
/*
 * Now we read the filters if any
 */
if (line[i] == '!' && line[i+1] == '[') {//Negative filter
  /* Initialization of the result structure */
  res->filter_polarity = 0;
  i=i+2; //we skip !
}
else if (line[i] == '[' ) {//Positive filter
  res->filter_polarity = 1;
  i++;
}
/*
 * Now we read the list of filters
 */
while ( res->n_filter_codes <MAX_FILTERS && line[i]==':' ) {
   i++;
   val=parse_string(line,&i,temp,P_COLON_CLOSING_BRACKET,P_EMPTY,P_EMPTY);
   if (val==P_BACKSLASH_AT_END) {
      if (!verbose) error("***Dictionary error: incorrect line\n_%S_\n",line);
      else (*verbose)=P_BACKSLASH_AT_END;
      goto error;
   }
   /*  */
   if (temp[0]=='\0') {
      if (!verbose) {
         error("***Dictionary error: incorrect line\n_%S_\n",line);
      } else (*verbose)=P_EMPTY_FILTER;
      goto error;
   }
   res->filter_codes[res->n_filter_codes]=u_strdup(temp,prv_alloc);
   (res->n_filter_codes)++;
}
if (line[i]==']') i++;
/*
 * Now we read the other gramatical and semantic codes if any
 */
while (res->n_semantic_codes<MAX_SEMANTIC_CODES && line[i]=='+') {
   i++;
   val=parse_string(line,&i,temp,P_PLUS_COLON_SLASH,P_EMPTY,P_EMPTY);
   if (val==P_BACKSLASH_AT_END) {
      if (!verbose) error("***Dictionary error: incorrect line\n_%S_\n",line);
      else (*verbose)=P_BACKSLASH_AT_END;
      goto error;
   }
   /* A grammatical or semantic code cannot be empty */
   if (val==P_EOS || temp[0]=='\0') {
      if (!verbose) {
         error("***Dictionary error: incorrect line\n_%S_\n",line);
      } else (*verbose)=P_EMPTY_SEMANTIC_CODE;
      goto error;
   }
   res->semantic_codes[res->n_semantic_codes]=u_strdup(temp,prv_alloc);
   (res->n_semantic_codes)++;
}
/*
 * Then we read the inflectional codes if any
 */
while (res->n_inflectional_codes<MAX_INFLECTIONAL_CODES && line[i]==':') {
   i++;
   val=parse_string(line,&i,temp,P_COLON_SLASH,P_EMPTY,P_EMPTY);
   if (val==P_BACKSLASH_AT_END) {
      if (!verbose) error("***Dictionary error: incorrect line\n_%S_\n",line);
      else (*verbose)=P_BACKSLASH_AT_END;
      goto error;
   }
   /* An inflectional code cannot be empty */
   if (val==P_EOS || temp[0]=='\0') {
      if (!verbose) {
         error("***Dictionary error: incorrect line\n_%S_\n",line);
      } else (*verbose)=P_EMPTY_INFLECTIONAL_CODE;
      goto error;
   }
   res->inflectional_codes[res->n_inflectional_codes]=u_strdup(temp,prv_alloc);
   (res->n_inflectional_codes)++;
}
/* We check if a character appears several times in an inflectional code like :KKms */
for (int il=0;il<res->n_inflectional_codes;il++) {
   if (is_duplicate_char_in_inflectional_code(res->inflectional_codes[il])) {
      if (!verbose) error("***Dictionary error: duplicate character in an inflectional code\n_%S_\n",line);
         else (*verbose)=P_DUPLICATE_CHAR_IN_INFLECTIONAL_CODE;
      goto error;
   }
}
/* We check if an inflectional code is a subset of another like :Kms:ms */
for (int il=0;il<res->n_inflectional_codes;il++) {
   for (int j=0;j<res->n_inflectional_codes;j++) {
      if (il==j) continue;
      if (one_inflectional_codes_contains_the_other(res->inflectional_codes[il],
                                                    res->inflectional_codes[j])) {
         if (!verbose) error("***Dictionary error: an inflectional code is a subset of another\n_%S_\n",line);
         else (*verbose)=P_DUPLICATE_INFLECTIONAL_CODE;
         goto error;
      }
   }
}
/* We check if a semantic code appears twice as in V+z1+z1 */
for (int il=0;il<res->n_semantic_codes;il++) {
   for (int j=0;j<res->n_semantic_codes;j++) {
      if (il==j) continue;
      if (!u_strcmp(res->semantic_codes[il],res->semantic_codes[j])) {
         if (!verbose) error("***Dictionary error: duplicate semantic code\n_%S_\n",line);
         else (*verbose)=P_DUPLICATE_SEMANTIC_CODE;
         goto error;
      }
   }
}
free_cb(temp,prv_alloc);
return res;
error:
free_dela_entry(res,prv_alloc);
free_cb(temp,prv_alloc);
return NULL;
}


/**
 * This function tests if the given line is a strict DELAS line, that is to say:
 * 1) it can be tokenized as a DELAS line
 * 2) the lemma is only made of letters
 * In case of success, the function returns a dela_entry structure describing the
 * line; NULL otherwise.
 */
struct dela_entry* is_strict_DELAS_line(const unichar* line,Alphabet* alphabet,Abstract_allocator prv_alloc) {
int verbose;
struct dela_entry* res=tokenize_DELAS_line(line,&verbose);
if (res==NULL) return NULL;
if (!is_sequence_of_letters(res->lemma,alphabet)) {
   free_dela_entry(res,prv_alloc);
   return NULL;
}
return res;
}


/**
 * This function fills the string 'codes' with all the codes of the given entry.
 * The result is ready to be concatenated with an inflected form and a lemma.
 * Special characters '+' ':' '/' and '\' are escaped with a backslash.
 *
 * Result example:
 *
 * .N+blood=A\+:ms
 */
void get_codes(const struct dela_entry* e,unichar* codes) {
int i,l;
/* First, we add the grammatical and semantic code */
codes[0]='.';
escape(e->semantic_codes[0],&(codes[1]),P_PLUS_COMMA_COLON_SLASH_BACKSLASH);
for (i=1;i<e->n_semantic_codes;i++) {
   l=u_strlen(codes);
   codes[l]='+';
   escape(e->semantic_codes[i],&(codes[l+1]),P_PLUS_COMMA_COLON_SLASH_BACKSLASH);
}
/* Then we add the inflectional codes */
for (i=0;i<e->n_inflectional_codes;i++) {
   l=u_strlen(codes);
   codes[l]=':';
   /* Here, the '+' char does need to be protected */
   escape(e->inflectional_codes[i],&(codes[l+1]),P_COLON_SLASH_BACKSLASH);
}
}


/**
 * This function counts the tokens of the string 's'. We delimit tokens by space
 * or '-' because this is a formal and language independent criterion.
 */
int get_number_of_tokens(unichar* s) {
int n=0;
int previous_was_a_letter=0;
if (s==NULL) return 0;
int i=0;
while (s[i]!='\0') {
   if (s[i]==' ' || s[i]=='-') {
      n++;
      previous_was_a_letter=0;
   }
   else if (!previous_was_a_letter) {
      n++;
      previous_was_a_letter=1;
   }
   i++;
}
return n;
}


/**
 * This function compares a lemma and an inflected form. If the lemma is a space
 * or an hyphen, it copies the lemma in 'result'. Otherwise, 'result' will
 * contain the length of the suffix to be removed from the inflected form in
 * order to get the longest common prefix, followed by the suffix
 * of the lemma. For instance:
 *
 * inflected="written"  lemma ="write"
 * - longest common prefix = "writ"
 * - 3 characters ("ten") to remove from "written" to get "writ"
 * - we must add "e" to get the lemma
 * => result="3e"
 */
static void get_compressed_token(unichar* inflected,unichar* lemma,unichar* result) {
int prefix=get_longuest_prefix(inflected,lemma);
int length_of_sfx_to_remove=u_strlen(inflected)-prefix;
/*int lemma_length=u_strlen(lemma);*/
if (/*lemma_length==1 && (lemma[0]==' ' || lemma[0]=='-') &&
   u_strlen(inflected)==1 && (inflected[0]==' ' || inflected[0]=='-')*/
    is_str_mono_unichar_string(lemma,' ') || is_str_mono_unichar_string(lemma,'-')) {
   /* If we have 2 separators, we write the lemma one rawly in order
    * to make the INF file visible.
    * Ex: "jean-pierre,jean-pierre.N" => "0-0.N" instead of "000.N" */
   result[0]=lemma[0];
   result[1]='\0';
   return;
}
/* We put the length to remove at the beginning of the result */
int j=u_sprintf(result,"%d",length_of_sfx_to_remove);
/* We need to protect the digits (used in the compression code),
 * the lemma and the point (used as delimitors in a DELAF line and, of
 * course, the backslash (protection character). */
escape(&(lemma[prefix]),&(result[j]),P_COMMA_DOT_BACKSLASH_DIGITS);
}


/**
 * Stores in 'token' the first token found from position
 * '*pos' in the string 's'. The position is updated.
 */
void get_token(unichar* s,unichar* token,int *pos) {
if (s[*pos]==' ' || s[*pos]=='-') {
   /* Spaces and hyphens are considered as tokens of length 1 */
   token[0]=s[*pos];
   token[1]='\0';
   (*pos)++;
   return;
}
/* Otherwise, we copy characters until we find a delimitor or
 * the end of string. */
int j=0;
while (s[*pos]!=' ' && s[*pos]!='-' && s[*pos]!='\0') {
   token[j++]=s[*pos];
   (*pos)++;
}
token[j]='\0';
}


/**
 * Compares 'inflected' and 'lemma' and produces a semitic compression of 'inflected',
 * where lemma's letters are referenced when possible as indices in 'inflected'.
 * '*min_full_letters' is used to store the minimum number of letters that appeared
 * in previous results. For example, if the first result is "1j6", then the value of
 * '*min_full_letters' is 1. It is used to keep the best result, i.e. the result with
 * as few full letters as possible, because it will produce most redundant compression
 * codes. 'n_full_letters' is the current number of letters already produced in the current
 * calculus. It is used to fail fast when we have no chance to produce a better result
 * than the existing one.
 *
 * '*min_full_letters' == -1 means that there is no current result, so the first one will
 * be temporarily the best one.
 *
 * 'result' is the best current result.
 * 'tmp_result' is the current result being computed.
 *
 * 'pos_in_inflected' is the position in 'inflected' where to start. For instance,
 * if inflected="abcdefgh" and if we already have matched the 'b', we don't want to
 * consider letters before 'b'.
 */
static void explore_semitic_tokens(const unichar* inflected,const unichar* lemma,unichar* result,
    unichar* tmp_result,int *min_full_letters,int n_full_letters,
    int pos_in_inflected,int pos_in_tmp_result) {
if (*lemma=='\0') {
  /* If we have finished to explore the lemma */
  if ((*min_full_letters)==-1 || n_full_letters<*min_full_letters) {
    /* If we have a best result than the current one, or if it's the first one */
    tmp_result[pos_in_tmp_result]='\0';
    u_strcpy(result,tmp_result);
    *min_full_letters=n_full_letters;
  }
  return;
}
/* We will look for the first letter of lemma. We don't go over position 9,
 * because we only want to use 1 digit to represent position in consonant
 * skeleton, which is reasonable. */
for (int i=pos_in_inflected;i<10 && inflected[i]!='\0';i++) {
  if (*lemma==inflected[i]) {
    tmp_result[pos_in_tmp_result]=(unichar)(i+'0');
    explore_semitic_tokens(inflected,lemma+1,result,tmp_result,
        min_full_letters,n_full_letters,i+1,pos_in_tmp_result+1);
    if (*min_full_letters==0) {
      /* If we have a perfect consonant skeleton, we can stop */
      return;
    }
  }
}
/* And finally, we will act as the letter is absent, because the letter may be
 * there but at an irrelevant place like 'h' for lemma="hbc" and inflected="abcdefgh".
 * In such a case, "7bc" would be a worse solution than "h12".
 */
if (n_full_letters==*min_full_letters) {
  /* No need to produce a worse solution than the current one */
  return;
}
tmp_result[pos_in_tmp_result]=*lemma;
explore_semitic_tokens(inflected,lemma+1,result,tmp_result,
    min_full_letters,n_full_letters+1,pos_in_inflected,pos_in_tmp_result+1);
}

/**
 * Compares 'inflected' and 'lemma' and produces a semitic compression of 'inflected',
 * where lemma's letters are referenced when possible as indices in 'inflected'.
 *
 * Example: inflected="abcdefgh" + lemma="bjg" => result="1j6"
 *          'j' appears, because there was no 'j' in 'inflected' between the
 *          'b' and the 'g'
 */
static void semitic_token_compression(const unichar* inflected,const unichar* lemma,unichar* result) {
int n=-1;
unichar tmp[1024];
explore_semitic_tokens(inflected,lemma,result,tmp,&n,0,0,0);
}


/**
 * Builds the compressed line from the given DELAF entry, and
 * stores it into the string 'result'.
 */
void get_compressed_line(struct dela_entry* e,unichar* result,int semitic) {
unichar code_gramm[DIC_LINE_SIZE];
/* Anyway, we will need the grammatical/inflectional codes of the
 * entry. */
get_codes(e,code_gramm);
/* If the 2 strings are identical, we just return the grammatical
 * code => .N+z1:ms */
if (!u_strcmp(e->inflected,e->lemma)) {
   u_strcpy(result,code_gramm);
   return;
}
/* Special case for semitic languages */
/* We test if the 2 strings have the same number of tokens */
int n_inflected=get_number_of_tokens(e->inflected);
int n_lemma=get_number_of_tokens(e->lemma);
if (n_inflected!=n_lemma
    || is_str_mono_unichar_string(e->inflected,' ') ||  is_str_mono_unichar_string(e->inflected,'-')) {
   /* If the 2 strings have not the same number of tokens,
    * we rawly consider them as two big tokens. However,
    * we put the prefix "_" in order to indicate that we have
    * a multi-token string that must be considered as a single
    * token. */
    result[0]='_';
    /* Here we use a trick to avoid a strcpy */
    get_compressed_token(e->inflected,e->lemma,&(result[1]));
    u_strcat(result,code_gramm);
    return;
}
/* We process now the case of 2 strings that have the same number of tokens */
int pos_inflected=0;
int pos_lemma=0;
unichar tmp_inflected[DIC_WORD_SIZE];
unichar tmp_lemma[DIC_WORD_SIZE];
unichar tmp_compressed[DIC_WORD_SIZE];
result[0]='\0';
if (semitic) {
  /* We use "__" as a marker to indicate that semitic compression has been used */
  u_strcpy(result,"__");
}
for (int i=0;i<n_inflected;i++) {
   /* Tokens are compressed one by one */
   get_token(e->inflected,tmp_inflected,&pos_inflected);
   get_token(e->lemma,tmp_lemma,&pos_lemma);
   if (semitic) {
     semitic_token_compression(tmp_inflected,tmp_lemma,tmp_compressed);
   } else {
     get_compressed_token(tmp_inflected,tmp_lemma,tmp_compressed);
   }
   u_strcat(result,tmp_compressed);
}
u_strcat(result,code_gramm);
return;
}


/**
 * This function takes a semitic inflected form and its associated compression code, and
 * it replaces 'inflected' by the lemma that is rebuilt.
 *
 * Example: inflected="yabodaOu"  compress_info="24W"
 *       => inflected="bdW"
 */
void rebuild_token_semitic(unichar* inflected,unichar* compress_info) {
unichar tmp[1024];
u_strcpy(tmp,inflected);
int size=u_strlen(tmp);
int i=0;
for (;compress_info[i]!='\0';i++) {
  if (compress_info[i]>='0' && compress_info[i]<='9') {
    /* If we have a consonant number */
    int n=compress_info[i]-'0';
    if (n>=size) {
      error("compress info=<%S>\n",compress_info);
      fatal_error("rebuild_token_semitic: consonant number #%d out of form <%S> of size %d\n",n,tmp,size);
    }
    inflected[i]=tmp[n];
  } else {
    inflected[i]=compress_info[i];
  }
}
inflected[i]='\0';
}


/**
 * This function takes an inflected form and its associated compression code, and
 * it replaces 'inflected' by the lemma that is rebuilt.
 *
 * Example: inflected="written"  compress_info="3e"
 *       => inflected="write"
 */
void rebuild_token(unichar* inflected,unichar* compress_info) {
if (inflected[0]=='\0') {
  fatal_error("Unexpected empty inflected form in rebuild_token\n");
}
int n=0;
int i,pos=0;
/* We count the number of characters to remove */
while (compress_info[pos]>='0' && compress_info[pos]<='9') {
   n=n*10+(compress_info[pos]-'0');
   pos++;
}
i=u_strlen(inflected)-n;
if (i<0) {
   /* This case should never happen */
   fatal_error("Internal error in rebuild_token:\ninflected=<%S>\ninf code=<%S>\n",inflected,compress_info);
}
/* Then we append the remaining suffix */
if (P_EOS==parse_string(compress_info,&pos,&(inflected[i]),P_EMPTY)) {
   /* If there is no suffix to add, we must terminate the string by a NULL char */
   inflected[i]='\0';
}
}


/**
 * This function takes an inflected form of the BIN file and a code
 * of the INF file. It stores in 'result' the rebuilt line.
 *
 * Example: entry="mains" + info="1.N:fs" ==> res="mains,main.N:fs"
 */
void uncompress_entry(const unichar* inflected,const unichar* INF_code,Ustring* result) {
if (inflected[0]=='\0') {
  fatal_error("Unexpected empty inflected form in uncompress_entry\n");
}
unsigned int n;
int pos;
empty(result);
/* The rebuilt line must start by the inflected form, followed by a comma */
escape(inflected,result,P_COMMA_DOT_EQUAL_BACKSLASH);
u_strcat(result,",");
if (INF_code[0]=='.') {
   /* First case: the lemma is the same than the inflected form
    * "write" + ".V:W" ==> "write,.V:W" */
   u_strcat(result,INF_code);
   return;
}
int semitic=(INF_code[0]=='_' && INF_code[1]=='_');
if (INF_code[0]=='_' && !semitic) {
   /* If we have a normal "_" compression, we rawly suppress chars, before adding some
    * "Albert Einstein" + "_15Einstein.N+Npr" => "Albert Einstein,Einstein.N+Npr" */
   pos=1;
   n=0;
   /* We read the number of chars to suppress */
   while (INF_code[pos]>='0' && INF_code[pos]<='9') {
      n=n*10+(INF_code[pos]-'0');
      pos++;
   }
   /* We add the inflected form */
   u_strcat(result,inflected);
   /* But we start copying the code at position length-n */
   if (n>result->len) {
     fatal_error("Unexpected problem in uncompress_entry: inflected=<%S> inf=<%S>\n",inflected,INF_code);
   }
   truncate(result,result->len-n);

   /* If a char is protected in the code, it must stay protected except if it's a digit,
    * because digit protection was only necessary because of the number after "_" in
    * lines like "_15Einstein.N+Npr" */
   unprotect(INF_code+pos,result,P_DIGITS);
   return;
}
/* Last case: we have to process token by token */
int pos_entry=0;
pos=(semitic?2:0);
while (INF_code[pos]!='.') {
   if (INF_code[pos]==' ' || INF_code[pos]=='-') {
      /* In the case of a separator, we copy the one of the INF code */
      u_strcat(result,INF_code[pos++]);
      pos_entry++;
   }
   else {
      unichar tmp[DIC_LINE_SIZE];
      unichar tmp_entry[DIC_LINE_SIZE];
      /* We read the compressed token */
      int j=0;
      while (INF_code[pos]!='.' && INF_code[pos]!=' ' && INF_code[pos]!='-') {
         if (INF_code[pos]=='\\') {
            /* If we find a protected char that is not a point, we let it protected */
            pos++;
            if (INF_code[pos]!='.') {tmp[j++]='\\';}
         }
         tmp[j++]=INF_code[pos++];
      }
      tmp[j]='\0';
      /* Now we read a token in the inflected form */
      j=0;
      while (inflected[pos_entry]!='\0' && inflected[pos_entry]!=' ' && inflected[pos_entry]!='-') {
         tmp_entry[j++]=inflected[pos_entry++];
      }
      tmp_entry[j]='\0';
      if (semitic) {
        rebuild_token_semitic(tmp_entry,tmp);
      } else {
        rebuild_token(tmp_entry,tmp);
      }
      j=0;
      /* Once we have rebuilt the token, we protect in it the following chars: . + \ /
       */
      escape(tmp_entry,result,P_DOT_PLUS_SLASH_BACKSLASH);
   }
}
/* Finally, we append the grammatical/inflectional information at the end
 * of the result line. */
while (INF_code[pos]!='\0') {
   u_strcat(result,INF_code[pos++]);
}
}


/**
 * function extracted from explore_all_paths to minimize stack uage of recursive
 *    function. Produce entries from the INF codes associated to this final state
 */
void uncompress_entry_and_print(unichar* content,struct list_ustring* tmp,U_FILE* output) {
Ustring* s=new_Ustring(DIC_WORD_SIZE);
while (tmp!=NULL) {
  uncompress_entry(content,tmp->string,s);
  u_fprintf(output,"%S\n",s->str);
  tmp=tmp->next;
}
free_Ustring(s);
}


/**
 * This function explores all the paths from the current state in the
 * .bin automaton and produces all the corresponding DELAF lines in the
 * 'output' file.
 * 'pos' is the offset in the byte array 'bin'. 'content' is the string
 * that contains the characters corresponding to the current position in the
 * automaton. 'string_pos' is the current position in 'content'.
 */
void explore_all_paths(int pos,unichar* content,int string_pos,Dictionary* d,U_FILE* output,Ustring* ustr,int base) {
int final,n_transitions;
int ref;
pos=read_dictionary_state(d,pos,&final,&n_transitions,&ref);
int z=save_output(ustr);
if (final) {
   /* If we are in a final state */
   content[string_pos]='\0';
   /* We produce entries from the INF codes associated to this final state */
   struct list_ustring* head;
   int to_be_freed=get_inf_codes(d,ref,ustr,&head,base);
   uncompress_entry_and_print(content,head,output);
   if (to_be_freed) free_list_ustring(head);
   base=ustr->len;
}
/* Nevermind the state finality, we explore all the reachable states */
int adr;
for (int i=0;i<n_transitions;i++) {
  pos=read_dictionary_transition(d,pos,&(content[string_pos]),&adr,ustr);
  explore_all_paths(adr,content,string_pos+1,d,output,ustr,base);
  restore_output(z,ustr);
}
}


/**
 * This function explores the automaton stored in the .bin and rebuilds
 * the original DELAF in the 'output' file.
 */
void rebuild_dictionary(Dictionary* d,U_FILE* output) {
unichar content[DIC_LINE_SIZE];
Ustring* ustr=new_Ustring();
explore_all_paths(d->initial_state_offset,content,0,d,output,ustr,0);
free_Ustring(ustr);
}


/**
 * This function parses a DELAF and stores all its grammatical and
 * semantic codes into the 'hash' structure. This structure is later
 * used in the Locate program in order to know if XYZ can be such a
 * code when there is a pattern like "<XYZ>".
 */
void extract_semantic_codes(const VersatileEncodingConfig* vec,const char* delaf,struct string_hash* hash) {
U_FILE* f=u_fopen(vec,delaf,U_READ);
if (f==NULL) return;
int i;
struct dela_entry* entry;
Ustring* line=new_Ustring(DIC_LINE_SIZE);
    
Abstract_allocator extract_semantic_codes_abstract_allocator=NULL;
extract_semantic_codes_abstract_allocator=create_abstract_allocator("extract_semantic_codes",
                                                                  AllocatorFreeOnlyAtAllocatorDelete|AllocatorTipGrowingOftenRecycledObject,
                                                                  0);
    
while (EOF!=readline(line,f)) {
   /* NOTE: DLF and DLC files are not supposed to contain comment
    *       lines, but we test them, just in the case */
   if (line->str[0]!='/') {
      entry=tokenize_DELAF_line(line->str,1,extract_semantic_codes_abstract_allocator);
      if (entry!=NULL) {
         for (i=0;i<entry->n_semantic_codes;i++) {
            get_value_index(entry->semantic_codes[i],hash);
         }
         free_dela_entry(entry,extract_semantic_codes_abstract_allocator);
      }
   }
}
free_Ustring(line);
u_fclose(f);
close_abstract_allocator(extract_semantic_codes_abstract_allocator);
return;
}


/**
 * This function checks the validity of a DELAF/DELAS line. If there are errors,
 * it prints error messages in the 'out' file.
 *
 * NOTE 1: as a side effect, this function stores the grammatical/semantic and inflectional
 *         codes into the 'semantic_codes' and 'inflectional_codes' structures. This is
 *         used to build the list of all the codes that are used in the dictionary.
 * NOTE 2: the 'alphabet' array is used to mark characters that are used in
 *         inflected forms and lemmas.
 * if strict_unprotected is not 0, we don't accept unprotected comma and dot (for CheckDic)
 */
void check_DELA_line(const unichar* DELA_line,U_FILE* out,int is_a_DELAF,int line_number,char* alphabet,
                     struct string_hash* semantic_codes,struct string_hash* inflectional_codes,
                     struct string_hash* simple_lemmas,struct string_hash* compound_lemmas,
                     int *n_simple_entries,int *n_compound_entries,Alphabet* alph2,int strict_unprotected,
                     Abstract_allocator prv_alloc) {
int i;
if (DELA_line==NULL) return;
int error_code;
struct dela_entry* entry;
if (is_a_DELAF) {
   entry=tokenize_DELAF_line(DELA_line,1,&error_code,strict_unprotected,prv_alloc);
} else {
   entry=tokenize_DELAS_line(DELA_line,&error_code,prv_alloc);
}
if (entry!=NULL) {
   /* If the line is correct, we just have to note its codes and the characters
    * that compose the inflected form and the lemma. */
   for (i=0;i<entry->n_semantic_codes;i++) {
      get_value_index(entry->semantic_codes[i],semantic_codes);
   }
   for (i=0;i<entry->n_inflectional_codes;i++) {
      get_value_index(entry->inflectional_codes[i],inflectional_codes);
   }
   
   int simple_entry;
   if (alph2!=NULL) {
      simple_entry=is_sequence_of_letters((is_a_DELAF)?entry->inflected:entry->lemma,alph2);
   } else {
      simple_entry=u_is_word((is_a_DELAF)?entry->inflected:entry->lemma);
   }
   if (simple_entry) {
      (*n_simple_entries)++;
      get_value_index(entry->lemma,simple_lemmas);
   } else {
      (*n_compound_entries)++;
      get_value_index(entry->lemma,compound_lemmas);
   }
   if (is_a_DELAF) {
      /* There is no inflected form to examine in a DELAS line */
      for (i=0;entry->inflected[i]!='\0';i++) {
         alphabet[entry->inflected[i]]=1;
      }
   }
   for (i=0;entry->lemma[i]!='\0';i++) {
      alphabet[entry->lemma[i]]=1;
   }
   free_dela_entry(entry,prv_alloc);
   return;
}
/**
 * If the entry is not correct, we must produce an appropriate error message.
 */
switch (error_code) {
   case P_UNEXPECTED_END_OF_LINE: {
      u_fprintf(out,"Line %d: unexpected end of line\n%S\n",line_number,DELA_line);
      return;
   }
   case P_BACKSLASH_AT_END: {
      u_fprintf(out,"Line %d: \\ at end of line\n%S\n",line_number,DELA_line);
      return;
   }
   case P_EMPTY_INFLECTED_FORM: {
      u_fprintf(out,"Line %d: empty inflected form\n%S\n",line_number,DELA_line);
      return;
   }
   case P_EMPTY_LEMMA: {
      u_fprintf(out,"Line %d: empty lemma\n%S\n",line_number,DELA_line);
      return;
   }
   case P_EMPTY_SEMANTIC_CODE: {
      u_fprintf(out,"Line %d: empty grammatical or semantic code\n%S\n",line_number,DELA_line);
      return;
   }
   case P_EMPTY_INFLECTIONAL_CODE: {
      u_fprintf(out,"Line %d: empty inflectional code\n%S\n",line_number,DELA_line);
      return;
   }
   case P_DUPLICATE_CHAR_IN_INFLECTIONAL_CODE: {
      u_fprintf(out,"Line %d: duplicate character in an inflectional\n%S\n",line_number,DELA_line);
      return;
   }
   case P_DUPLICATE_INFLECTIONAL_CODE: {
      u_fprintf(out,"Line %d: an inflectional code is a subset of another\n%S\n",line_number,DELA_line);
      return;
   }
   case P_DUPLICATE_SEMANTIC_CODE: {
      u_fprintf(out,"Line %d: duplicate semantic code\n%S\n",line_number,DELA_line);
      return;
   }
   case P_UNPROTECTED_DOT: {
      u_fprintf(out,"Line %d: unprotected dot in inflected form\n%S\n",line_number,DELA_line);
      return;
   }
   case P_UNPROTECTED_COMMA: {
      u_fprintf(out,"Line %d: unprotected comma in lemma\n%S\n",line_number,DELA_line);
      return;
   }
}
}


/**
 * This function tests if the tag token passed in parameter is valid. A tag token
 * is supposed to be like a DELAF without comment surrounded by round brackets like:
 *
 *    {being,be.V:G}
 *
 * It returns 1 on success. Otherwise, it prints an error message to the error output
 * if emit_error is non null and returns 0.
 */
int check_tag_token(const unichar* s,int emit_error) {
if (s==NULL) {
   /* This case should never happen */
   fatal_error("Interal NULL error in check_tag_token\n");
}
struct dela_entry* entry=tokenize_tag_token(s,emit_error);
if (entry==NULL) {
   return 0;
}
free_dela_entry(entry);
return 1;
}


/**
 * This function takes a char sequence supposed to represent a
 * gramatical, semantic or inflectional code;
 * if this code contains space, tabulation or any non-ASCII char
 * it returns 1 and stores a warning message in 'comment'; returns 0 otherwise
 */
int warning_on_code(const unichar* code,unichar* comment,int space_warnings) {
int i;
int space=0;
int tab=0;
int non_ascii=0;
int n=0;
int l=u_strlen(code);
for (i=0;i<l;i++) {
   if (code[i]==' ') {space++;n++;}
   else if (code[i]=='\t') {tab++;n++;}
   else if (code[i]>=128) {non_ascii++;n++;}
}
if ((space && space_warnings) || tab || non_ascii) {
   /* We build a message that indicates the number of suspect chars */
   unichar temp[DIC_LINE_SIZE];
   u_sprintf(temp,"warning: %d suspect char%s (",n,(n>1)?"s":"");
   u_strcpy(comment,temp);
   if (space && space_warnings) {
      u_sprintf(temp,"%d space%s, ",space,(space>1)?"s":"");
      u_strcat(comment,temp);
   }
   if (tab) {
      u_sprintf(temp,"%d tabulation%s, ",tab,(tab>1)?"s":"");
      u_strcat(comment,temp);
   }
   if (non_ascii) {
      u_sprintf(temp,"%d non ASCII char%s, ",non_ascii,(non_ascii>1)?"s":"");
      u_strcat(comment,temp);
   }
   comment[u_strlen(comment)-2]='\0';
   u_strcat(comment,"): (");
   unichar temp2[10];
   /* We explicit the content of the code. For instance, if the code is "� t",
    * the result will be: "E9 SPACE t" */
   for (i=0;i<l-1;i++) {
      if (code[i]==' ') u_sprintf(temp2,"SPACE");
      else if (code[i]=='\t') u_sprintf(temp2,"TABULATION");
      else if (code[i]<=128) u_sprintf(temp2,"%c",code[i]);
      else u_sprintf(temp2,"%04X",code[i]);
      u_strcat(comment,temp2);
      u_strcat(comment," ");
   }
   if (code[l-1]==' ') u_sprintf(temp2,"SPACE");
   else if (code[l-1]=='\t') u_sprintf(temp2,"TABULATION");
   else if (code[l-1]<=128) u_sprintf(temp2,"%c",code[l-1]);
   else u_sprintf(temp2,"%04X",code[l-1]);
   u_strcat(comment,temp2);
   u_strcat(comment,")");
   return 1;
}
return 0;
}


/**
 * Tests if the given sequence contains an unprotected = sign
 */
int contains_unprotected_equal_sign(const unichar* s) {
for (;;) {
  if (*(s+0)==0) return 0;
  if (*(s+0)==1) return 1;
  if (*(s+1)==0) return 0;
  if (*(s+1)==1) return 1;
  if (*(s+2)==0) return 0;
  if (*(s+2)==1) return 1;
  if (*(s+3)==0) return 0;
  if (*(s+3)==1) return 1;
  s+=4;
}
}


/**
 * Replaces all the unprotected = signs by the char 'c'.
 * Now, such '=' signs are supposed to have been previously
 * replaced by chars #1.
 */
void replace_unprotected_equal_sign(unichar* s,unichar c) {
for (;;) {
  if (*(s+0)==0) return ;
  if (*(s+0)==1) *(s+0) = c;
  if (*(s+1)==0) return ;
  if (*(s+1)==1) *(s+1) = c;
  if (*(s+2)==0) return ;
  if (*(s+2)==1) *(s+2) = c;
  if (*(s+3)==0) return ;
  if (*(s+3)==1) *(s+3) = c;
  s+=4;
}
}


/**
 * Takes a string containing protected equal signs and unprotects them
 * ex: E\=mc2 -> E=mc2
 */
void unprotect_equal_signs(unichar* s) {
int j=0;
int i=0;
for (;;) {
   /* There won't be segfault since s[i+1] will be \0 at worst */
   if (s[i]=='\0') break;
   if (s[i]=='\\' && s[i+1]=='=') {
   }
   else s[j++]=s[i];

   if (s[i+1]=='\0') break;
   if (s[i+1]=='\\' && s[i+1+1]=='=') {
   }
   else s[j++]=s[i+1];

   if (s[i+2]=='\0') break;
   if (s[i+2]=='\\' && s[i+2+1]=='=') {
   }
   else s[j++]=s[i+2];

   if (s[i+3]=='\0') break;
   if (s[i+3]=='\\' && s[i+3+1]=='=') {
   }
   else s[j++]=s[i+3];
   i+=4;
}
s[j]='\0';
}


/**
 * Frees all the memory consumed by the given dela_entry structure.
 */
void free_dela_entry(struct dela_entry* d,Abstract_allocator prv_alloc) {
if (d==NULL) return;
if (d->inflected!=NULL) free_cb(d->inflected,prv_alloc);
if (d->lemma!=NULL) free_cb(d->lemma,prv_alloc);
for (int i=0;i<d->n_semantic_codes;i++) {
   free_cb(d->semantic_codes[i],prv_alloc);
}
for (int i=0;i<d->n_inflectional_codes;i++) {
   free_cb(d->inflectional_codes[i],prv_alloc);
}

for (int i=0;i<d->n_filter_codes;i++) {
   free_cb(d->filter_codes[i],prv_alloc);
}


free_cb(d,prv_alloc);
}


/**
 * Returns 1 if the given entry contains the given grammatical code; 0 otherwise.
 */
int dic_entry_contain_gram_code(const struct dela_entry* entry,const unichar* code) {
for (int i=0;i<entry->n_semantic_codes;i++) {
   if (!u_strcmp(entry->semantic_codes[i],code)) {
      return 1;
   }
}
return 0;
}


/**
 * Returns 1 if the inflectional code 'a' contains the inflectional code 'b';
 * 0 otherwise.
 */
int one_inflectional_codes_contains_the_other(const unichar* a,const unichar* b) {
int i=0;
while (b[i]!='\0') {
   int j=0;
   while (a[j]!=b[i]) {
      if (a[j]=='\0') {
         /* If we have not found a character, we return */
         return 0;
      }
      j++;
   }
   i++;
}
return 1;
}


/**
 * Returns 1 if the given entry contains the given inflectional code; 0 otherwise.
 */
int dic_entry_contain_inflectional_code(const struct dela_entry* entry,const unichar* code) {
for (int i=0;i<entry->n_inflectional_codes;i++) {
   if (one_inflectional_codes_contains_the_other(entry->inflectional_codes[i],code)) {
      return 1;
   }
}
return 0;
}


/**
 * This function takes a string 's' representing the first code of a DELAS/DELAC
 * line and analyses it to determine the inflection_code and the grammatical code.
 * There are two possible conventions:
 * 1) The grammatical code, made of ANSI letters, is followed by an inflection code
 *    between parenthesis like "N(NC_XXX)"
 * 2) The grammatical code, made of ANSI letters, is followed by a suffix like "N32".
 *    In that case, the whole string is the inflection code.
 *
 * Moreover, if the code starts with a '$' we set the '*semitic' parameter to 1.
 *
 * The output strings are supposed to be allocated.
 *
 * Examples:
 *
 *    s="N32"       => inflection_code="N32"     code_gramm="N"
 *    s="N(NC_XXX)" => inflection_code="NC_XXX"  code_gramm="N"
 */
void get_inflection_code(unichar* s,char* inflection_code,unichar* code_gramm,int *semitic) {
int i=0;
if (s[0]=='$') {
   i++;
   (*semitic)=1;
} else {
   (*semitic)=0;
}
for (;(s[i]>='A' && s[i]<='Z')||(s[i]>='a' && s[i]<='z');i++) {}
if (s[i]=='\0') {
   /* If the whole string is made of ANSI letters, then the inflection and
    * grammmatical codes are identical */
    u_strcpy(code_gramm,&(s[*semitic]));
    u_to_char(inflection_code,&(s[*semitic]));
    return;
}
if (s[i]=='(' && s[u_strlen(s)-1]==')') {
   /* If we are in the case "N(NC_XXX)" */
   u_strcpy(code_gramm,&(s[*semitic]));
   code_gramm[i]='\0';
   int j=0;
   i++;
   while (s[i+1]!='\0') {
      /* We don't want to stop at ')' since a malicious code could be "N(NC_X(X))" */
      inflection_code[j++]=(char)s[i++];
   }
   inflection_code[j++]='\0';
   return;
}
/* If we are in the case "N32" */
u_to_char(inflection_code,&(s[*semitic]));
u_strcpy(code_gramm,&(s[*semitic]));
code_gramm[i-(*semitic)]='\0';
}


/**
 * This function takes a DELAF entry and builds from it a tag of
 * the form "{AM,be.V:P1s}". 'tag' is supposed to be large enough.
 * If 'token' is not NULL, it is used as the inflected form.
 */
void build_tag(struct dela_entry* entry,const unichar* token,unichar* tag) {
int i;
tag[0]='{';
/* We protect the comma and dot, if any, in the inflected form */
int l=1+escape(((token!=NULL)?token:entry->inflected),&(tag[1]),P_COMMA_DOT);
tag[l++]=',';
/* We protect the comma and dots, if any, in the lemma */
l=l+escape(entry->lemma,&(tag[l]),P_COMMA_DOT);
tag[l++]='.';
/* We protect the + and :, if any, in the grammatical code */
l=l+escape(entry->semantic_codes[0],&(tag[l]),P_PLUS_COLON);
for (i=1;i<entry->n_semantic_codes;i++) {
   tag[l++]='+';
   l=l+escape(entry->semantic_codes[i],&(tag[l]),P_PLUS_COLON);
}
for (i=0;i<entry->n_inflectional_codes;i++) {
   tag[l++]=':';
   l=l+escape(entry->inflectional_codes[i],&(tag[l]),P_COLON);
}
tag[l++]='}';
tag[l++]='\0';
}


/**
 * Returns 1 if a and b have exactly the same semantic codes; 0
 * otherwise. a and b are supposed to be non NULL.
 */
int same_semantic_codes(const struct dela_entry* a,const struct dela_entry* b) {
if (a->n_semantic_codes!=b->n_semantic_codes) return 0;
for (int i=0;i<b->n_semantic_codes;i++) {
   if (!dic_entry_contain_gram_code(a,b->semantic_codes[i])) return 0;
}
return 1;
}


/**
 * Returns 1 if a and b have exactly the same semantic codes; 0
 * otherwise. a and b are supposed to be non NULL.
 */
int same_inflectional_codes(const struct dela_entry* a,const struct dela_entry* b) {
if (a->n_inflectional_codes!=b->n_inflectional_codes) return 0;
for (int i=0;i<b->n_inflectional_codes;i++) {
   if (!dic_entry_contain_inflectional_code(a,b->inflectional_codes[i])) return 0;
}
return 1;
}


/**
 * Returns 1 if a and b have exactly the same semantic and inflectional codes; 0
 * otherwise. a and b are supposed to be non NULL.
 */
int same_codes(const struct dela_entry* a,const struct dela_entry* b) {
return same_semantic_codes(a,b) && same_inflectional_codes(a,b);
}

/**
 * Adds to dst all the inflectional codes of src, it not already present.
 * Both are supposed to be non NULL.
 */
void merge_inflectional_codes(struct dela_entry* dst,const struct dela_entry* src,Abstract_allocator prv_alloc) {
for (int i=0;i<src->n_inflectional_codes;i++) {
   if (!dic_entry_contain_inflectional_code(dst,src->inflectional_codes[i])) {
      /* If necessary, we add the code */
      dst->inflectional_codes[dst->n_inflectional_codes]=u_strdup(src->inflectional_codes[i],prv_alloc);
      dst->n_inflectional_codes++;
   }
}
}



/**
 * Looks for an exact match of the given string 'str'. Returns its INF code number or
 * -1 is not found.
 *
 * NOTE: this is an EXACT matching. No alphabet equivalency is used here.
 */
int explore_for_exact_match(const unsigned char* bin,int offset,unichar* str,int pos) {
/* We compute the number of transitions that outgo from the current node */
int n_transitions=((unsigned char)bin[offset])*256+(unsigned char)bin[offset+1];
offset=offset+2;
if (str[pos]=='\0') {
   /* If we are at the end of the token */
   if (!(n_transitions & 32768)) {
      /* If the node is final */
      int inf_number=((unsigned char)bin[offset])*256*256+((unsigned char)bin[offset+1])*256+(unsigned char)bin[offset+2];
      return inf_number;
   }
   /* If the string is not in the dictionary */
   return -1;
}
if ((n_transitions & 32768)) {
   /* If we are in a normal node, we remove the control bit to
    * have the good number of transitions */
   n_transitions=n_transitions-32768;
} else {
   /* If we are in a final node, we must jump after the reference to the INF
    * line number */
   offset=offset+3;
}
for (int i=0;i<n_transitions;i++) {
   /* For each outgoing transition, we look if the transition character is
    * the one we look for */
   unichar c=(unichar)(((unsigned char)bin[offset])*256+(unsigned char)bin[offset+1]);
   offset=offset+2;
   int offset_dest=((unsigned char)bin[offset])*256*256+((unsigned char)bin[offset+1])*256+(unsigned char)bin[offset+2];
   offset=offset+3;
   if (c==str[pos]) {
      return explore_for_exact_match(bin,offset_dest,str,pos+1);
   }
}
return -1;
}


/**
 * Returns the INF code associated to the given string or -1 if not found.
 *
 * NOTE: this is an EXACT matching. No alphabet equivalency is used here.
 */
int get_inf_code_exact_match(Dictionary* d,unichar* str) {
if (d->type!=BIN_CLASSIC) {
  fatal_error("get_inf_code_exact_match: unsupported dictionary type\n");
}
return explore_for_exact_match(d->bin,d->initial_state_offset,str,0);
}


/**
 * Prints the given entry to the error stream.
 *
 * WARNING: use for debug only, since it does not handle special char protection
 */
void debug_print_entry(struct dela_entry* e) {
error("%S,%S.%S",e->inflected,e->lemma,e->semantic_codes[0]);
for (int i=1;i<e->n_semantic_codes;i++) {
  error("+%S",e->semantic_codes[i]);
}
for (int i=0;i<e->n_inflectional_codes;i++) {
  error(":%S",e->inflectional_codes[i]);
}
}


/**
 * The same than above, with a new line
 */
void debug_println_entry(struct dela_entry* e) {
debug_print_entry(e);
error("\n");
}


/**
 * Prints the given entry to the given string.
 */
void dela_entry_to_string(Ustring* s,struct dela_entry* e) {
empty(s);
escape(e->inflected,s,P_COMMA);
u_strcat(s,",");
escape(e->lemma,s,P_DOT);
u_strcat(s,".");
escape(e->semantic_codes[0],s,P_PLUS_COMMA_COLON_SLASH_BACKSLASH);
for (int i=1;i<e->n_semantic_codes;i++) {
  u_strcat(s,"+");
  escape(e->semantic_codes[i],s,P_PLUS_COMMA_COLON_SLASH_BACKSLASH);
}
for (int i=0;i<e->n_inflectional_codes;i++) {
  u_strcat(s,":");
  escape(e->inflectional_codes[i],s,P_COLON_SLASH_BACKSLASH);
}
}


/**
 * This function replaces any '=' char by a char #1 if the '=' sign
 * is not protected by a \
 * For instance:
 * =,X.Y   =>   $,X.Y  ($ stands here for the char whose code is 1)
 * \=,X.Y   =>  \=,X.Y
 * \\=,X.Y   =>  \$,X.Y
 */
int replace_special_equal_signs(unichar* s) {
int i=0;
while (s[i]!='\0') {
  if (s[i]==PROTECTION_CHAR) {
    if (s[i+1]=='\0') {
      error("Unexpected \\ at end of string in replace_special_equal_signs\n");
      return DEFAULT_ERROR_CODE; 
    }
    i=i+2;
  } else {
    if (s[i]=='=') s[i]=1;
    i++;
  }
}
return SUCCESS_RETURN_CODE;
}

} // namespace unitex
