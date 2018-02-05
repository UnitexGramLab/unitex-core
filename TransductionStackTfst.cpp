/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "TransductionStackTfst.h"
#include "Error.h"
#include "Match.h"
#include "Tfst.h"
#include "DELA.h"
#include "DebugMode.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This function returns a non zero value if c can be a part of a variable name;
 * 0 otherwise.
 */
static int is_variable_char(unichar c) {
return ((c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') || c=='_');
}


/**
 * Pushes the given character.
 */
void push_input_char_tfst(Ustring* s,unichar c,int protect_dic_chars) {
if (protect_dic_chars && (c==',' || c=='.')) {
    /* We want to protect dots and commas because the Locate program can be used
     * by Dico to generate dictionary entries */
    u_strcat(s,"\\");
}
u_strcat(s,c);
}


/**
 * Pushes the given character.
 */
void push_output_char_tfst(Ustring* s,unichar c) {
   u_strcat(s,c);
}


/**
 * Pushes the given string to the output, if not NULL.
 */
void push_input_string_tfst(Ustring* stack,const unichar* s,int protect_dic_chars) {
int i;
if (s==NULL) {
   return;
}
for (i=0;s[i]!='\0';i++) {
    push_input_char_tfst(stack,s[i],protect_dic_chars);
}
}


/**
 * Pushes the given string to the output, if not NULL, in the limit of 'length' chars.
 */
void push_input_substring_tfst(Ustring* stack,const unichar* s,int length) {
int i;
if (s==NULL) {
   return;
}
for (i=0;i<length && s[i]!='\0';i++) {
   push_input_char_tfst(stack,s[i],0);
}
}


/**
 * Pushes the given string to the output, if not NULL.
 */
void push_output_string_tfst(Ustring* stack,const unichar* s) {
u_strcat(stack,s);
}


/**
 * Inserts the text interval defined by the parameters into the given string.
 */
void insert_text_interval_tfst(struct locate_tfst_infos* infos,Ustring* s,int start_token,int start_char,
                               int end_token,int end_char) {
//error("de %d.%d a %d.%d\n",start_token,start_char,end_token,end_char);
if (start_token>end_token || (start_token==end_token && start_char>end_char)) {
   /* In Korean, when we have a match made of several TfstTags containing less than
    * a character (for instance, a logical letter), then we can arrive here with a
    * start position that has already been increased of 1 since the last TfstTag
    * was dealt with. So, we can have things like 0.2 -> 0.1 and such things must
    * be ignored */
   return;
}
int current_token=start_token;
int current_char=start_char;
unichar* token=infos->tfst->token_content[current_token];
for (;;) {
   u_strcat(s,token[current_char]);
   if (current_token==end_token && current_char==end_char) {
      /* Done */
      return;
   }
   current_char++;
   if (token[current_char]=='\0') {
      /* We go on the next token */
      current_token++;
      //error("current=%d  end=%d  max=%d\n",current_token,end_token,infos->tfst->token_sizes->nbelems);
      token=infos->tfst->token_content[current_token];
      current_char=0;
   }
}
}


/**
 * This function processes the given output string.
 * Returns 1 if OK; 0 otherwise (for instance, if a variable is
 * not correctly defined).
 */
int process_output_tfst(Ustring* stack,const unichar* s,struct locate_tfst_infos* p,
        int capture_in_debug_mode) {
int old_length=stack->len;
int i=0;
if (s==NULL) {
   /* We do nothing if there is no output */
   return 1;
}
if (capture_in_debug_mode) {
    /* If we have a capture in debug mode, we must skip the initial char #1 */
    i++;
}
while (s[i]!='\0') {
    if (s[i]==DEBUG_INFO_COORD_MARK) {
        /* If we have found the debug mark indicating the end of the real output,
         * we rawly copy the end of the output without interpreting it,
         * or return if we were in capture mode */
        if (capture_in_debug_mode) return 1;
        push_output_string_tfst(stack,s+i);
        return 1;
    }

   if (s[i]=='$') {
      /* Case of a variable name */
      unichar name[MAX_TRANSDUCTION_VAR_LENGTH];
      int l=0;
      i++;
      while (is_variable_char(s[i]) && l<MAX_TRANSDUCTION_VAR_LENGTH) {
         name[l++]=s[i++];
      }
      if (l==MAX_TRANSDUCTION_VAR_LENGTH) {
         fatal_error("Too long variable name (>%d chars) in following output:\n%S\n",MAX_TRANSDUCTION_VAR_LENGTH,s);
      }
      name[l]='\0';
      if (s[i]!='$' && s[i]!='.') {
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: missing closing $ after $%S\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: {
               stack->len=old_length;
               stack->str[old_length]='\0';
               return 0;
            }
         }
      }
      else if (s[i]=='.') {
         /* Here we deal with the case of a field like $a.SET$ */
         unichar field[MAX_TRANSDUCTION_FIELD_LENGTH];
         l=0;
         i++;
         while (s[i]!='\0' && s[i]!='$' && l<MAX_TRANSDUCTION_FIELD_LENGTH) {
            field[l++]=s[i++];
         }
         if (l==MAX_TRANSDUCTION_FIELD_LENGTH) {
            fatal_error("Too long field name (>%d chars) in following output:\n%S\n",MAX_TRANSDUCTION_FIELD_LENGTH,s);
         }
         field[l]='\0';
         if (!u_strcmp(field,"SET") || !u_strcmp(field,"UNSET")) {
            /* We only accept those 2 fields. We look if a variable exists */
            struct transduction_variable* v=get_transduction_variable(p->input_variables,name);
            TfstTag* first_tag=NULL;
            TfstTag* last_tag=NULL;
            if (v==NULL) {
                /* We do nothing, since this normal variable may not exist */
            } else {
               first_tag=(TfstTag*)(p->tfst->tags->tab[v->start_in_tokens]);
               last_tag=(TfstTag*)(p->tfst->tags->tab[v->end_in_tokens]);
               if (v->start_in_tokens==UNDEF_VAR_BOUND || v->end_in_tokens==UNDEF_VAR_BOUND
                  || !valid_text_interval_tfst(&(first_tag->m),&(last_tag->m))) {
                   /* If the variable is not defined properly */
                   if (field[0]=='S') {
                       /* $a.SET$ is false, we backtrack */
                       stack->len=old_length;
                       stack->str[old_length]='\0';
                       return 0;
                   } else {
                       /* $a.UNSET$ is true, we go on */
                       continue;
                   }
               } else {
                   /* If the variable is correctly defined */
                   if (field[0]=='S') {
                       /* $a.SET$ is true, we go on */
                       continue;
                   } else {
                       /* $a.UNSET$ is false, we backtrack */
                       stack->len=old_length;
                       stack->str[old_length]='\0';
                       return 0;
                   }
               }
            }
            /* If we arrive here, the variable was not a normal one, so we
             * try to match an output one */
            const Ustring* output=get_output_variable(p->output_variables,name);
            if (output==NULL) {
                /* We do nothing, since this output variable may not exist */
            } else {
               if (output->len==0) {
                  /* If the variable is empty */
                  if (field[0]=='S') {
                      /* $a.SET$ is false, we backtrack */
                      stack->len=old_length;
                      stack->str[old_length]='\0';
                      return 0;
                  } else {
                     /* $a.UNSET$ is true, we go on */
                     continue;
                  }
               } else {
                  /* If the variable is non empty */
                  if (field[0]=='S') {
                      /* $a.SET$ is true, we go on */
                      continue;
                  } else {
                      /* $a.UNSET$ is false, we backtrack */
                      stack->len=old_length;
                      stack->str[old_length]='\0';
                      return 0;
                  }
               }
            }
         } else {
             /* Here we deal with dictionary variable things like $a.CODE$ */
             struct dela_entry* entry=get_dic_variable(name,p->dic_variables);
             if (entry==NULL) {
                 switch (p->variable_error_policy) {
                     case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: undefined morphological variable %S\n",name);
                     case IGNORE_VARIABLE_ERRORS: continue;
                     case BACKTRACK_ON_VARIABLE_ERRORS: {
                         stack->len=old_length;
                         stack->str[old_length]='\0';
                         return 0;
                     }
                 }
             }
             if (!u_strcmp(field,"INFLECTED")) {
                 /* We use push_input_string because it can protect special chars */
                 push_input_string_tfst(stack,entry->inflected,1);
             } else if (!u_strcmp(field,"LEMMA")) {
                 push_input_string_tfst(stack,entry->lemma,1);
             } else if (!u_strcmp(field,"CODE")) {
                 push_output_string_tfst(stack,entry->semantic_codes[0]);
                 for (int il=1;il<entry->n_semantic_codes;il++) {
                     push_output_char_tfst(stack,'+');
                     push_output_string_tfst(stack,entry->semantic_codes[il]);
                 }
                 for (int il=0;il<entry->n_inflectional_codes;il++) {
                     push_output_char_tfst(stack,':');
                     push_output_string_tfst(stack,entry->inflectional_codes[il]);
                 }
             } else if (!u_strcmp(field,"CAT")) {
                 push_output_string_tfst(stack,entry->semantic_codes[0]);
                 for (int il=1;il<entry->n_semantic_codes;il++) {
                     push_output_char_tfst(stack,'+');
                     push_output_string_tfst(stack,entry->semantic_codes[il]);
                 }
             } else if (!u_strcmp(field,"CODE.GRAM")) {
                 push_output_string_tfst(stack,entry->semantic_codes[0]);
             } else if (!u_strcmp(field,"CODE.SEM")) {
                if (entry->n_semantic_codes > 1) {
                  push_output_string_tfst(stack,entry->semantic_codes[1]);
                  for (int il=2;il<entry->n_semantic_codes;il++) {
                    push_output_char_tfst(stack,'+');
                    push_output_string_tfst(stack,entry->semantic_codes[il]);
                  }
                }
             } else if (!u_strcmp(field,"CODE.FLEX")) {
                 for (int il=0;il<entry->n_inflectional_codes;il++) {
                     push_output_char_tfst(stack,':');
                     push_output_string_tfst(stack,entry->inflectional_codes[il]);
                 }
             } else if (u_starts_with(field,"CODE.ATTR=")) {
                 unichar* attr_name=field+10;
                 int attr_len=u_strlen(attr_name);
                 int j;
                 for (j=0;j<entry->n_semantic_codes;j++) {
                    if (u_starts_with(entry->semantic_codes[j],attr_name)) {
                       if (entry->semantic_codes[j][attr_len]!='='
                          || entry->semantic_codes[j][attr_len+1]=='\0') {
                          continue;
                       }
                       push_output_string_tfst(stack,entry->semantic_codes[j]+attr_len+1);
                    }
                 }
                 if (j==entry->n_semantic_codes) {
                    /* If the attribute was not found, it's an error case */
                    switch (p->variable_error_policy) {
                       case EXIT_ON_VARIABLE_ERRORS: fatal_error("Attribute %S not found in a captured entry\n",attr_name);
                       case IGNORE_VARIABLE_ERRORS: continue;
                       case BACKTRACK_ON_VARIABLE_ERRORS: {
                          stack->len=old_length;
                          stack->str[old_length]='\0';
                          return 0;
                       }
                    }
                 }
             } else {
                 switch (p->variable_error_policy) {
                     case EXIT_ON_VARIABLE_ERRORS: fatal_error("Invalid morphological variable field $%S.%S$\n",name,field);
                     case IGNORE_VARIABLE_ERRORS: continue;
                     case BACKTRACK_ON_VARIABLE_ERRORS: {
                         stack->len=old_length;
                         stack->str[old_length]='\0';
                         return 0;
                     }
                 }
             }
             i++;
             continue;
         }
      }
      i++;
      if (l==0) {
         /* Case of $$ in order to print a $ */
           push_output_char_tfst(stack,'$');
         continue;
      }
      struct transduction_variable* v=get_transduction_variable(p->input_variables,name);
      if (v==NULL) {
          /* Not a normal one ? Maybe an output one */
          const Ustring* output=get_output_variable(p->output_variables,name);
          if (output==NULL) {
              switch (p->variable_error_policy) {
                  case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: undefined variable $%S$\n",name);
                  case IGNORE_VARIABLE_ERRORS: continue;
                  case BACKTRACK_ON_VARIABLE_ERRORS:
                      stack->len=old_length;
                      stack->str[old_length]='\0';
                      return 0;
              }
          }
          push_output_string_tfst(stack,output->str);
          continue;
      }
      if (v->start_in_tokens==UNDEF_VAR_BOUND) {
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: starting position of variable $%S$ undefined\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: {
               stack->len=old_length;
               stack->str[old_length]='\0';
               return 0;
            }
         }
      }
      if (v->end_in_tokens==UNDEF_VAR_BOUND) {
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position of variable $%S$ undefined\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: {
               stack->len=old_length;
               stack->str[old_length]='\0';
               return 0;
            }
         }
      }
      TfstTag* first_tag=(TfstTag*)(p->tfst->tags->tab[v->start_in_tokens]);
      TfstTag* last_tag=(TfstTag*)(p->tfst->tags->tab[v->end_in_tokens]);
      if (!valid_text_interval_tfst(&(first_tag->m),&(last_tag->m))) {
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position before starting position for variable $%S$\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: {
               stack->len=old_length;
               stack->str[old_length]='\0';
               return 0;
            }
         }
      }
      /* If the variable definition is correct */
      insert_text_interval_tfst(p,stack,first_tag->m.start_pos_in_token,first_tag->m.start_pos_in_char,
                                last_tag->m.end_pos_in_token,last_tag->m.end_pos_in_char);
   }
   else {
      /* If we have a normal character */
      push_output_char_tfst(stack,s[i]);
      i++;
   }
}
return 1;
}


/**
 * This function deals with an output sequence, regardless there are pending
 * output variables or not.
 */
int deal_with_output_tfst(Ustring* stack,const unichar* output,struct locate_tfst_infos* p,int *captured_chars) {
if (output==NULL) return 1;
Ustring* stack_foo=stack;
int capture=capture_mode(p->output_variables);
if (capture) {
    stack_foo=new_Ustring(64);
    if (p->debug) {
        /* In debug mode, an output to be captured must still be
         * added to the normal stack, to trace the explored grammar
         * path. But, in this case, we remove the actual output part,
         * since no output is really produced there */
        push_output_char_tfst(stack,DEBUG_INFO_OUTPUT_MARK);
        int i;
        for (i=0;output[i]!=DEBUG_INFO_COORD_MARK;i++) {
        }
        push_output_string_tfst(stack,output+i);
    }
}
if (!process_output_tfst(stack_foo,output,p,capture && p->debug)) {
    return 0;
}
if (capture) {
    *captured_chars=add_raw_string_to_output_variables(p->output_variables,stack_foo->str);
    free_Ustring(stack_foo);
}
return 1;
}

} // namespace unitex
