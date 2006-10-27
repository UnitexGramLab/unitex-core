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
#include "DELA.h"
#include "Error.h"
#include "StringParsing.h"



/**
 * Returns a copy of the given entry.
 */
struct dela_entry* clone_dela_entry(struct dela_entry* entry) {
if (entry==NULL) return NULL;
int i;
struct dela_entry* res=(struct dela_entry*)malloc(sizeof(struct dela_entry));
if (res==NULL) {
   fatal_error("Not enough memory in clone_dela_entry\n");
}
res->inflected=u_strdup(entry->inflected);
res->lemma=u_strdup(entry->lemma);
res->n_semantic_codes=entry->n_semantic_codes;
for (i=0;i<res->n_semantic_codes;i++) {
   res->semantic_codes[i]=u_strdup(entry->semantic_codes[i]);
}
res->n_inflectional_codes=entry->n_inflectional_codes;
for (i=0;i<res->n_inflectional_codes;i++) {
   res->inflectional_codes[i]=u_strdup(entry->inflectional_codes[i]);
}
return res;
}


/**
 * Returns 1 if a and b are identical; 0 otherwise.
 * a and b are supposed to be valid entries, that is to say, entries
 * will non NULL inflected forms and lemmas.
 */
int equal(struct dela_entry* a,struct dela_entry* b) {
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
 * Tokenizes a DELAF line and returns the information in a dela_entry structure, or
 * NULL if there is an error in the line. The second parameter indicates if
 * comments are allowed at the end of the line or not. 'keep_equal_signs' indicates
 * if protected equal signs must be unprotected. This option is used by the Compress
 * program. If 'verbose' is NULL, the
 * function must print messages if there is an error; otherwise, the function prints
 * no error message and stores an error code in '*verbose'.
 */
struct dela_entry* tokenize_DELAF_line(unichar* line,int comments_allowed,int keep_equal_signs,
                                       int *verbose) {
struct dela_entry* res;
char err[DIC_LINE_SIZE];
unichar temp[DIC_LINE_SIZE];
int i,val;
if (line==NULL) {
	if (!verbose) error("Internal NULL error in tokenize_DELAF_line\n");
   else (*verbose)=P_NULL_STRING;
	return NULL;
}
/* Initialization of the result structure */
res=(struct dela_entry*)malloc(sizeof(struct dela_entry));
if (res==NULL) {
	fatal_error("Not enough memory in tokenize_DELAF_line\n");
}
res->inflected=NULL;
res->lemma=NULL;
res->n_semantic_codes=1;   /* 0 would be an error (no grammatical code) */
res->semantic_codes[0]=NULL;
res->n_inflectional_codes=0;
res->inflectional_codes[0]=NULL;
i=0;
/*
 * We read the inflected part
 */
val=parse_string(line,&i,temp,P_COMMA,P_EMPTY,keep_equal_signs?P_EQUAL:P_EMPTY);
if (val==P_BACKSLASH_AT_END) {
   if (!verbose) error("***Dictionary error: incorrect line\n_%s_\n",err);
   else (*verbose)=P_BACKSLASH_AT_END;
   return NULL;
}
/* If we are at the end of line, it's an error */
if (line[i]=='\0') {
   if (!verbose) {
      u_to_char(err,line);
      error("***Dictionary error: incorrect line\n_%s_\n",err);
   } else (*verbose)=P_UNEXPECTED_END_OF_LINE;
   return NULL;
}
/* The inflected form cannot be empty */
if (temp[0]=='\0') {
   if (!verbose) {
      u_to_char(err,line);
      error("***Dictionary error: incorrect line\n_%s_\n",err);
   } else (*verbose)=P_EMPTY_INFLECTED_FORM;
   return NULL;
}
res->inflected=u_strdup(temp);
/*
 * We read the lemma part
 */
i++;
val=parse_string(line,&i,temp,P_DOT,P_EMPTY,keep_equal_signs?P_EQUAL:P_EMPTY);
if (val==P_BACKSLASH_AT_END) {
   if (!verbose) error("***Dictionary error: incorrect line\n_%s_\n",err);
   else (*verbose)=P_BACKSLASH_AT_END;
   return NULL;
}
/* If we are at the end of line, it's an error */
if (line[i]=='\0') {
   if (!verbose) {
      u_to_char(err,line);
      error("***Dictionary error: incorrect line\n_%s_\n",err);
   } else (*verbose)=P_UNEXPECTED_END_OF_LINE;
   return NULL;
}
if (temp[0]=='\0') {
	/* If the lemma is empty like in "eat,.V:W", it is supposed to be
	 * the same as the inflected form. */
	res->lemma=u_strdup(res->inflected);
}
else {
	/* Otherwise, we copy it */
	res->lemma=u_strdup(temp);
}
/*
 * We read the grammatical code
 */
i++;
val=parse_string(line,&i,temp,P_PLUS_COLON_SLASH,P_EMPTY,P_EMPTY);
if (val==P_BACKSLASH_AT_END) {
   if (!verbose) error("***Dictionary error: incorrect line\n_%s_\n",err);
   else (*verbose)=P_BACKSLASH_AT_END;
   return NULL;
}
/* The grammatical code cannot be empty */
if (temp[0]=='\0') {
   if (!verbose) {
      u_to_char(err,line);
      error("***Dictionary error: incorrect line\n_%s_\n",err);
   } else (*verbose)=P_EMPTY_SEMANTIC_CODE;
   return NULL;
}
res->semantic_codes[0]=u_strdup(temp);
/*
 * Now we read the other gramatical and semantic codes if any
 */
while (res->n_semantic_codes<MAX_SEMANTIC_CODES && line[i]=='+') {
	i++;
   val=parse_string(line,&i,temp,P_PLUS_COLON_SLASH,P_EMPTY,P_EMPTY);
   if (val==P_BACKSLASH_AT_END) {
      if (!verbose) error("***Dictionary error: incorrect line\n_%s_\n",err);
      else (*verbose)=P_BACKSLASH_AT_END;
      return NULL;
   }
   /* A grammatical or semantic code cannot be empty */
   if (temp[0]=='\0') {
      if (!verbose) {
        u_to_char(err,line);
         error("***Dictionary error: incorrect line\n_%s_\n",err);
      } else (*verbose)=P_EMPTY_SEMANTIC_CODE;
      return NULL;
   }
   res->semantic_codes[res->n_semantic_codes]=u_strdup(temp);
	(res->n_semantic_codes)++;
}
/*
 * Then we read the inflectional codes if any
 */
while (res->n_inflectional_codes<MAX_INFLECTIONAL_CODES && line[i]==':') {
	i++;
   val=parse_string(line,&i,temp,P_COLON_SLASH,P_EMPTY,P_EMPTY);
   if (val==P_BACKSLASH_AT_END) {
      if (!verbose) error("***Dictionary error: incorrect line\n_%s_\n",err);
      else (*verbose)=P_BACKSLASH_AT_END;
      return NULL;
   }
      /* An inflectional code cannot be empty */
   if (temp[0]=='\0') {
      if (!verbose) {
        u_to_char(err,line);
         error("***Dictionary error: incorrect line\n_%s_\n",err);
      } else (*verbose)=P_EMPTY_INFLECTIONAL_CODE;
      return NULL;
   }
   res->inflectional_codes[res->n_inflectional_codes]=u_strdup(temp);
	(res->n_inflectional_codes)++;
}
/* Finally we check if there is a comment */
if (line[i]=='/' && !comments_allowed) {
   if (!verbose) error("***Dictionary error: unexpected comment at end of entry\n_%s_\n",err);
      else (*verbose)=P_UNEXPECTED_COMMENT;
      return NULL;
}
return res;
}


/**
 * Tokenizes a DELAF line and returns the information in a dela_entry structure, or
 * NULL if there is an error in the line. The second parameter indicates if
 * comments are allowed at the end of the line or not. The function prints
 * error message to the standard output in case of error.
 */
struct dela_entry* tokenize_DELAF_line(unichar* line,int comments_allowed) {
return tokenize_DELAF_line(line,comments_allowed,0,NULL);
}


/**
 * Tokenizes a DELAF line and returns the information in a dela_entry structure, or
 * NULL if there is an error in the line. Comments are allowed at the end of the
 * line.
 */
struct dela_entry* tokenize_DELAF_line(unichar* line) {
return tokenize_DELAF_line(line,1,0,NULL);
}


/**
 * This function tokenizes a tag token like {today,.ADV} and returns the
 * information in a dela_entry structure, or NULL if there is an error in
 * the tag.
 */
struct dela_entry* tokenize_tag_token(unichar* tag) {
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
unichar temp[DIC_LINE_SIZE];
int val=parse_string(tag,&i,temp,P_CLOSING_ROUND_BRACKET,P_EMPTY,NULL);
if (tag[i]!='}' || val!=P_OK) {
   error("Invalid tag in tokenize_tag_token\n");
   return NULL;
}
/* And we tokenize it as a normal DELAF line */
return tokenize_DELAF_line(temp,0);
}


/**
 * Tokenizes a DELAS line and returns the information in a dela_entry structure, or
 * NULL if there is an error in the line. If 'verbose' is NULL, the
 * function must print messages if there is an error; otherwise, the function prints
 * no error message and stores an error code in '*verbose'.
 */
struct dela_entry* tokenize_DELAS_line(unichar* line,int *verbose) {
struct dela_entry* res;
char err[DIC_LINE_SIZE];
unichar temp[DIC_LINE_SIZE];
int i,val;
if (line==NULL) {
   if (!verbose) error("Internal NULL error in tokenize_DELAS_line\n");
   else (*verbose)=P_NULL_STRING;
   return NULL;
}
/* Initialization of the result structure */
res=(struct dela_entry*)malloc(sizeof(struct dela_entry));
if (res==NULL) {
   fatal_error("Not enough memory in tokenize_DELAS_line\n");
}
res->inflected=NULL;
res->lemma=NULL;
res->n_semantic_codes=1;   /* 0 would be an error (no grammatical code) */
res->semantic_codes[0]=NULL;
res->n_inflectional_codes=0;
res->inflectional_codes[0]=NULL;
i=0;
/*
 * We read the inflected part
 */
val=parse_string(line,&i,temp,P_COMMA,P_EMPTY,P_EMPTY);
if (val==P_BACKSLASH_AT_END) {
   if (!verbose) error("***Dictionary error: incorrect line\n_%s_\n",err);
   else (*verbose)=P_BACKSLASH_AT_END;
   return NULL;
}
/* If we are at the end of line, it's an error */
if (line[i]=='\0') {
   if (!verbose) {
      u_to_char(err,line);
      error("***Dictionary error: incorrect line\n_%s_\n",err);
   } else (*verbose)=P_UNEXPECTED_END_OF_LINE;
   return NULL;
}
/* The lemma form cannot be empty */
if (temp[0]=='\0') {
   if (!verbose) {
      u_to_char(err,line);
      error("***Dictionary error: incorrect line\n_%s_\n",err);
   } else (*verbose)=P_EMPTY_LEMMA;
   return NULL;
}
res->lemma=u_strdup(temp);
/*
 * We read the grammatical code
 */
i++;
val=parse_string(line,&i,temp,P_PLUS_COLON_SLASH,P_EMPTY,P_EMPTY);
if (val==P_BACKSLASH_AT_END) {
   if (!verbose) error("***Dictionary error: incorrect line\n_%s_\n",err);
   else (*verbose)=P_BACKSLASH_AT_END;
   return NULL;
}
/* The grammatical code cannot be empty */
if (temp[0]=='\0') {
   if (!verbose) {
      u_to_char(err,line);
      error("***Dictionary error: incorrect line\n_%s_\n",err);
   } else (*verbose)=P_EMPTY_SEMANTIC_CODE;
   return NULL;
}
res->semantic_codes[0]=u_strdup(temp);
/*
 * Now we read the other gramatical and semantic codes if any
 */
while (res->n_semantic_codes<MAX_SEMANTIC_CODES && line[i]=='+') {
   i++;
   val=parse_string(line,&i,temp,P_PLUS_COLON_SLASH,P_EMPTY,P_EMPTY);
   if (val==P_BACKSLASH_AT_END) {
      if (!verbose) error("***Dictionary error: incorrect line\n_%s_\n",err);
      else (*verbose)=P_BACKSLASH_AT_END;
      return NULL;
   }
   /* A grammatical or semantic code cannot be empty */
   if (temp[0]=='\0') {
      if (!verbose) {
        u_to_char(err,line);
         error("***Dictionary error: incorrect line\n_%s_\n",err);
      } else (*verbose)=P_EMPTY_SEMANTIC_CODE;
      return NULL;
   }
   res->semantic_codes[res->n_semantic_codes]=u_strdup(temp);
   (res->n_semantic_codes)++;
}
/*
 * Then we read the inflectional codes if any
 */
while (res->n_inflectional_codes<MAX_INFLECTIONAL_CODES && line[i]==':') {
   i++;
   val=parse_string(line,&i,temp,P_COLON_SLASH,P_EMPTY,P_EMPTY);
   if (val==P_BACKSLASH_AT_END) {
      if (!verbose) error("***Dictionary error: incorrect line\n_%s_\n",err);
      else (*verbose)=P_BACKSLASH_AT_END;
      return NULL;
   }
      /* An inflectional code cannot be empty */
   if (temp[0]=='\0') {
      if (!verbose) {
        u_to_char(err,line);
         error("***Dictionary error: incorrect line\n_%s_\n",err);
      } else (*verbose)=P_EMPTY_INFLECTIONAL_CODE;
      return NULL;
   }
   res->inflectional_codes[res->n_inflectional_codes]=u_strdup(temp);
   (res->n_inflectional_codes)++;
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
void get_codes(struct dela_entry* e,unichar* codes) {
int i,l;
/* First, we add the grammatical and semantic code */
codes[0]='.';
escape(e->semantic_codes[0],&(codes[1]),P_PLUS_COLON_SLASH_BACKSLASH);
for (i=1;i<e->n_semantic_codes;i++) {
   l=u_strlen(codes);
   codes[l]='+';
   escape(e->semantic_codes[i],&(codes[l+1]),P_PLUS_COLON_SLASH_BACKSLASH);
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
 * This function compares a lemma and an inflected form. If both are spaces
 * or hyphens, it copies the lemma in 'result'. Otherwise, 'result' will
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
void get_compressed_token(unichar* inflected,unichar* lemma,unichar* result) {
int prefix=get_longuest_prefix(inflected,lemma);
int length_of_sfx_to_remove=u_strlen(inflected)-prefix;
int lemma_length=u_strlen(lemma);
if (lemma_length==1 && (lemma[0]==' ' || lemma[0]=='-') &&
   u_strlen(inflected)==1 && (inflected[0]==' ' || inflected[0]=='-')) {
   /* If we have 2 separators, we write the lemma one rawly in order 
    * to make the INF file visible.
    * Ex: "jean-pierre,jean-pierre.N" => "0-0.N" instead of "000.N" */
   result[0]=lemma[0];
   result[1]='\0';
   return;
}
/* We put the length to remove at the beginning of the result */
u_int_to_string(length_of_sfx_to_remove,result);
int j=u_strlen(result);
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
 * Builds the compressed line from the given DELAF entry, and
 * stores it into the string 'result'.
 */
void get_compressed_line(struct dela_entry* e,unichar* result) {
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
/* We test if the 2 strings have the same number of tokens */
int n_inflected=get_number_of_tokens(e->inflected);
int n_lemma=get_number_of_tokens(e->lemma);
if (n_inflected!=n_lemma) {
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
for (int i=0;i<n_inflected;i++) {
   /* Tokens are compressed one by one */
   get_token(e->inflected,tmp_inflected,&pos_inflected);
   get_token(e->lemma,tmp_lemma,&pos_lemma);
   get_compressed_token(tmp_inflected,tmp_lemma,tmp_compressed);
   u_strcat(result,tmp_compressed);
}
u_strcat(result,code_gramm);
return;
}


/**
 * This function takes a line of a .INF file and tokenize it into
 * several single codes.
 * Example: .N,.V  =>  code 0=".N" ; code 1=".V"
 */
struct word_list* tokenize_compressed_info(unichar* line) {
struct word_list* result=NULL;
unichar tmp[DIC_LINE_SIZE];
int pos=0;
/* Note: all protected characters must stay protected */
while (P_EOS!=parse_string(line,&pos,tmp,P_COMMA,P_EMPTY,NULL)) {
   result=new_word_list(tmp,result);
   if (line[pos]==',') pos++;
}
return result;
}


/**
 * This function takes an inflected form and its associated compression code, and
 * it replaces 'inflected' by the lemma that is rebuilt.
 * 
 * Example: inflected="written"  compress_info="3e"
 *       => inflected="write"
 */
void rebuild_token(unichar* inflected,unichar* compress_info) {
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
   fatal_error("Internal error in rebuild_token\n");
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
void uncompress_entry(unichar* inflected,unichar* INF_code,unichar* result) {
int n;
int pos,i;
/* The rebuilt line must start by the inflected form, followed by a comma */
escape(inflected,result,P_COMMA_DOT);
u_strcat_char(result,",");
if (INF_code[0]=='.') {
   /* First case: the lemma is the same than the inflected form
    * "write" + ".V:W" ==> "write,.V:W" */
   u_strcat(result,INF_code);
   return;
}
if (INF_code[0]=='_') {
   /* In this case, we rawly suppress chars, before adding some 
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
   i=u_strlen(result)-n;
   while (INF_code[pos]!='\0') {
      /* If a char is protected in the code, it must stay protected,
       * so there is nothing to do but a raw copy. */
      result[i++]=INF_code[pos++];
   }
   result[i]='\0';
   return;
}
/* Last case: we have to process token by token */
int pos_entry=0;
pos=0;
i=u_strlen(result);
while (INF_code[pos]!='.') {
   if (INF_code[pos]==' ' || INF_code[pos]=='-') {
      /* In the case of a separator, we copy the one of the INF code */
      result[i++]=INF_code[pos++];
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
      rebuild_token(tmp_entry,tmp);
      j=0;
      /* Once we have rebuilt the token, we protect in it the following chars: . + \ /
       * We must also update 'i'.
       */
      i+=escape(tmp_entry,&(result[i]),P_DOT_PLUS_SLASH_BACKSLASH);
   }
}
/* Finally, we append the grammatical/inflectional information at the end
 * of the result line. */
while (INF_code[pos]!='\0') {
   result[i++]=INF_code[pos++];
}
result[i]='\0';
}


/**
 * This function loads the content of an .inf file and returns 
 * a structure containing the lines of the file tokenized into INF
 * codes.
 */
struct INF_codes* load_INF_file(char* nom) {
struct INF_codes* res;
FILE *f=u_fopen(nom,U_READ);
if (f==NULL) {
   error("Cannot open %s\n",nom);
   return NULL;
}
res=(struct INF_codes*)malloc(sizeof(struct INF_codes));
if (res==NULL) {
   fatal_error("Not enough memory in load_INF_file\n");
}
res->N=u_read_int(f);
res->codes=(struct word_list**)malloc(sizeof(struct word_list*)*(res->N));
if (res->codes==NULL) {
   fatal_error("Not enough memory in load_INF_file\n");
}
unichar s[DIC_LINE_SIZE*10];
int i=0;
/* For each line of the .inf file, we tokenize it to get the single INF codes
 * it contains. */
while (u_read_line(f,s)) {
   res->codes[i++]=tokenize_compressed_info(s);
}
u_fclose(f);
return res;
}


/**
 * Frees all the memory allocated for the given structure.
 */
void free_INF_codes(struct INF_codes* INF) {
if (INF==NULL) {return;}
for (int i=0;i<INF->N;i++) {
   free_word_list(INF->codes[i]);
}
free(INF->codes);
free(INF);
}


/**
 * Loads a .bin file into an unsigned char array that is returned.
 * Returns NULL if an error occurs.
 */
unsigned char* load_BIN_file(char* name) {
FILE* f;
/* We open the file as a binary one */
f=fopen(name,"rb");
unsigned char* tab;
if (f==NULL) {
   error("Cannot open %s\n",name);
   return NULL;
}
/* We compute the size of the file that is encoded in the 4 first bytes.
 * This value could be used to check the integrity of the file. */
int a,b,c,d;
a=(unsigned char)fgetc(f);
b=(unsigned char)fgetc(f);
c=(unsigned char)fgetc(f);
d=(unsigned char)fgetc(f);
int file_size=d+256*c+256*256*b+256*256*256*a;
/* We come back to the beginning and we load rawly the file */
fseek(f,0,SEEK_SET);
tab=(unsigned char*)malloc(sizeof(unsigned char)*file_size);
if (tab==NULL) {
   error("Memory error: cannot load %s\n",name);
   return NULL;
}
if (file_size!=(int)fread(tab,sizeof(char),file_size,f)) {
   error("Error while reading %s\n",name);
   free(tab);
   fclose(f);
   return NULL;
}
fclose(f);
return tab;
}


/**
 * This function explores all the paths from the current state in the 
 * .bin automaton and produces all the corresponding DELAF lines in the
 * 'output' file.
 * 'pos' is the offset in the byte array 'bin'. 'content' is the string
 * that contains the characters corresponding to the current position in the
 * automaton. 'string_pos' is the current position in 'content'.
 */
void explore_all_paths(int pos,unichar* content,int string_pos,unsigned char* bin,
                      struct INF_codes* inf,FILE* output) {
int n_transitions;
int ref;
n_transitions=((unsigned char)bin[pos])*256+(unsigned char)bin[pos+1];
pos=pos+2;
if (!(n_transitions & 32768)) {
   /* If we are in a final state */
   ref=((unsigned char)bin[pos])*256*256+((unsigned char)bin[pos+1])*256+(unsigned char)bin[pos+2];
   pos=pos+3;
   content[string_pos]='\0';
   struct word_list* tmp=inf->codes[ref];
   /* We produce entries from the INF codes associated to this final state */
   while (tmp!=NULL) {
      unichar res[DIC_WORD_SIZE];
      uncompress_entry(content,tmp->word,res);
      u_fprints(res,output);
      u_fprints_char("\n",output);
      tmp=tmp->next;
   }
}
else {
   /* If we are in a normal node, we remove the control bit to
    * have the good number of transitions */
   n_transitions=n_transitions-32768;
}
/* Nevermind the state finality, we explore all the reachable states */
for (int i=0;i<n_transitions;i++) {
   content[string_pos]=(unichar)(((unsigned char)bin[pos])*256+(unsigned char)bin[pos+1]);
   pos=pos+2;
   int adr=((unsigned char)bin[pos])*256*256+((unsigned char)bin[pos+1])*256+(unsigned char)bin[pos+2];
   pos=pos+3;
   explore_all_paths(adr,content,string_pos+1,bin,inf,output);
}
}


/**
 * This function explores the automaton stored in the .bin and rebuilds
 * the original DELAF in the 'output' file.
 */
void rebuild_dictionary(unsigned char* bin,struct INF_codes* inf,FILE* output) {
unichar content[DIC_LINE_SIZE];
/* The offset of the initial state is 4 */
explore_all_paths(4,content,0,bin,inf,output);
}


/**
 * This function parses a DELAF and stores all its grammatical and
 * semantic codes into the 'hash' structure. This structure is later
 * used in the Locate program in order to know if XXX can be such a 
 * code when there is a pattern like "<XXX>".
 */
void extract_semantic_codes(char* delaf,struct string_hash* hash) {
FILE* f=u_fopen(delaf,U_READ);
if (f==NULL) return;
unichar line[DIC_LINE_SIZE];
int i;
struct dela_entry* entry;
while (u_read_line(f,line)) {
   /* NOTE: DLF and DLC files are not supposed to contain comment
    *       lines, but we test them, just in the case */
   if (line[0]!='/') {
      entry=tokenize_DELAF_line(line,1);
      for (i=0;i<entry->n_semantic_codes;i++) {
         get_hash_number(entry->semantic_codes[i],hash);
      }
      free_dic_entry(entry);
   }
}
fclose(f);
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
 */
void check_DELA_line(unichar* DELA_line,FILE* out,int is_a_DELAF,int line_number,char* alphabet,
                      struct string_hash* semantic_codes,struct string_hash* inflectional_codes) {
char err[DIC_LINE_SIZE];
int i;
if (DELA_line==NULL) return;
int error_code;
struct dela_entry* entry;
if (is_a_DELAF) {
   entry=tokenize_DELAF_line(DELA_line,1,0,&error_code);
} else {
   entry=tokenize_DELAS_line(DELA_line,&error_code);
}
if (entry!=NULL) {
   /* If the line is correct, we just have to note its codes and the characters 
    * that compose the inflected form and the lemma. */
   for (i=0;i<entry->n_semantic_codes;i++) {
      get_hash_number(entry->semantic_codes[i],semantic_codes);
   }
   for (i=0;i<entry->n_inflectional_codes;i++) {
      get_hash_number(entry->inflectional_codes[i],inflectional_codes);
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
   free_dic_entry(entry);
   return;
}
/**
 * If the entry is not correct, we must produce an appropriate error message. 
 */
switch (error_code) {
   case P_UNEXPECTED_END_OF_LINE: {
      sprintf(err,"Line %d: unexpected end of line\n",line_number);
      u_fprints_char(err,out);
      u_fprints(DELA_line,out);
      u_fprints_char("\n",out);
      return;
   }
   case P_BACKSLASH_AT_END: {
      sprintf(err,"Line %d: \\ at end of line\n",line_number);
      u_fprints_char(err,out);
      u_fprints(DELA_line,out);
      u_fprints_char("\n",out);
      return;
   }
   case P_EMPTY_INFLECTED_FORM: {
      sprintf(err,"Line %d: empty inflected form\n",line_number);
      u_fprints_char(err,out);
      u_fprints(DELA_line,out);
      u_fprints_char("\n",out);
      return;
   }
   case P_EMPTY_LEMMA: {
      sprintf(err,"Line %d: empty lemma\n",line_number);
      u_fprints_char(err,out);
      u_fprints(DELA_line,out);
      u_fprints_char("\n",out);
      return;
   }
   case P_EMPTY_SEMANTIC_CODE: {
      sprintf(err,"Line %d: empty grammatical or semantic code\n",line_number);
      u_fprints_char(err,out);
      u_fprints(DELA_line,out);
      u_fprints_char("\n",out);
      return;
   }
   case P_EMPTY_INFLECTIONAL_CODE: {
      sprintf(err,"Line %d: empty inflectional code\n",line_number);
      u_fprints_char(err,out);
      u_fprints(DELA_line,out);
      u_fprints_char("\n",out);
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
 * and returns 0.
 */
int check_tag_token(unichar* s) {
if (s==NULL) {
   /* This case should never happen */
   fatal_error("Interal NULL error in check_tag_token\n");
}
struct dela_entry* entry=tokenize_tag_token(s);
if (entry==NULL) {
   return 0;
}
free_dic_entry(entry);
return 1;
}


/**
 * This function takes a char sequence supposed to represent a
 * gramatical, semantic or inflectional code;
 * if this code contains space, tabulation or any non-ASCII char
 * it returns 1 and stores a warning message in 'comment'; returns 0 otherwise
 */
int warning_on_code(unichar* code,unichar* comment) {
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
if (space || tab || non_ascii) {
   /* We build a message that indicates the number of suspect chars */
   char temp[DIC_LINE_SIZE];
   sprintf(temp,"warning: %d suspect char%s (",n,(n>1)?"s":"");
   u_strcpy_char(comment,temp);
   if (space) {
      sprintf(temp,"%d space%s, ",space,(space>1)?"s":"");
      u_strcat_char(comment,temp);
   }
   if (tab) {
      sprintf(temp,"%d tabulation%s, ",tab,(tab>1)?"s":"");
      u_strcat_char(comment,temp);
   }
   if (non_ascii) {
      sprintf(temp,"%d non ASCII char%s, ",non_ascii,(non_ascii>1)?"s":"");
      u_strcat_char(comment,temp);
   }
   comment[u_strlen(comment)-2]='\0';
   u_strcat_char(comment,"): (");
   unichar temp2[10];
   /* We explicit the content of the code. For instance, if the code is "é t",
    * the result will be: "E9 SPACE t" */
   for (i=0;i<l-1;i++) {
      u_char_to_hexa_or_code(code[i],temp2);
      u_strcat(comment,temp2);
      u_strcat_char(comment," ");
   }
   u_char_to_hexa_or_code(code[l-1],temp2);
   u_strcat(comment,temp2);
   u_strcat_char(comment,")");
   return 1;
}
else return 0;
}


/**
 * Tests if the given sequence contains an unprotected = sign
 */
int contains_unprotected_equal_sign(unichar* s) {
if (s[0]=='=') {
   return 1;
}
for (int i=1;s[i]!='\0';i++) {
   if (s[i]=='=' && s[i-1]!='\\') {
      return 1;
   }
}
return 0;
}


/**
 * Replaces all the unprotected = signs by the char 'c'
 */
void replace_unprotected_equal_sign(unichar* s,unichar c) {
if (s[0]=='=') {
   s[0]=c;
}
for (int i=1;s[i]!='\0';i++) {
   if (s[i]=='=' && s[i-1]!='\\') {
      s[i]=c;
   }
}
}


/**
 * Takes a string containing protected equal signs and unprotects them
 * ex: E\=mc2 -> E=mc2 
 */
void unprotect_equal_signs(unichar* s) {
int j=0;
for (int i=0;s[i]!='\0';i++) {
   /* There won't be segfault since s[i+1] will be \0 at worst */
   if (s[i]=='\\' && s[i+1]=='=') {
   }
   else s[j++]=s[i];
}
s[j]='\0';
}


/**
 * Frees all the memory consumed by the given dela_entry structure.
 */
void free_dic_entry(struct dela_entry* d) {
if (d==NULL) return;
if (d->inflected!=NULL) free(d->inflected);
if (d->lemma!=NULL) free(d->lemma);
for (int i=0;i<d->n_semantic_codes;i++) {
   free(d->semantic_codes[i]);
}
for (int i=0;i<d->n_inflectional_codes;i++) {
   free(d->inflectional_codes[i]);
}
free(d);
}


/**
 * Returns 1 if the given entry contains the given grammatical code; 0 otherwise.
 */
int dic_entry_contain_gram_code(struct dela_entry* entry,unichar* code) {
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
int one_inflectional_codes_contains_the_other(unichar* a,unichar* b) {
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
int dic_entry_contain_inflectional_code(struct dela_entry* entry,unichar* code) {
for (int i=0;i<entry->n_inflectional_codes;i++) {
   if (one_inflectional_codes_contains_the_other(entry->inflectional_codes[i],code)) {
      return 1;
   }
}
return 0;
}


