 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
void push_input_char_tfst(Ustring* s,unichar c) {
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
void push_input_string_tfst(Ustring* stack,unichar* s) {
int i;
if (s==NULL) {
   return;
}
for (i=0;s[i]!='\0';i++) {
    push_input_char_tfst(stack,s[i]);
}
}


/**
 * Pushes the given string to the output, if not NULL, in the limit of 'length' chars.
 */
void push_input_substring_tfst(Ustring* stack,unichar* s,int length) {
int i;
if (s==NULL) {
   return;
}
for (i=0;i<length && s[i]!='\0';i++) {
   push_input_char_tfst(stack,s[i]);
}
}


/**
 * Pushes the given string to the output, if not NULL.
 */
void push_output_string_tfst(Ustring* stack,unichar* s) {
u_strcat(stack,s);
}


/**
 * Inserts the text interval defined by the parameters into the given string. 
 */
void insert_text_interval_tfst(struct locate_tfst_infos* infos,Ustring* s,int start_token,int start_char,
                               int end_token,int end_char) {
//error("de %d.%d a %d.%d\n",start_token,start_char,end_token,end_char);
if (start_token==end_token && start_char==end_char+1) {
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
while (1) {
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
int process_output_tfst(Ustring* stack,unichar* s,struct locate_tfst_infos* p) {
int old_length=stack->len;
int i=0;
if (s==NULL) {
   /* We do nothing if there is no output */
   return 1;
}
while (s[i]!='\0') {
   if (s[i]=='$') {
      /* Case of a variable name */
      unichar name[128];
      int l=0;
      i++;
      while (is_variable_char(s[i])) {
         name[l++]=s[i++];
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
      if (s[i]=='.') {
         /* Dic variables are not currently not handled */
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: dictionary variables not allowed by LocateTfst\n");
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: {
               stack->len=old_length;
               stack->str[old_length]='\0';
               return 0;
            }
         }
      #if 0
         /* Here we deal with the case of a field like $a.CODE$ */
         unichar field[128];
         l=0;
         i++;
         while (s[i]!='\0' && s[i]!='$') {
            field[l++]=s[i++];
         }
         field[l]='\0';
         if (s[i]=='\0') {
            switch (p->variable_error_policy) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: missing closing $ after $%S.%S\n",name,field);
               case IGNORE_VARIABLE_ERRORS: continue;
               case BACKTRACK_ON_VARIABLE_ERRORS: {
                  stack->len=old_length;
                  stack->str[old_length]='\0';
                  return 0;
               }
            }
         }
         if (field[0]=='\0') {
            switch (p->variable_error_policy) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: empty field: $%S.$\n",name);
               case IGNORE_VARIABLE_ERRORS: continue;
               case BACKTRACK_ON_VARIABLE_ERRORS: {
                  stack->len=old_length;
                  stack->str[old_length]='\0';
                  return 0;
               }
            }
         }
         i++;
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
            push_input_string(p->stack,entry->inflected,1);
         } else if (!u_strcmp(field,"LEMMA")) {
        	 push_input_string(p->stack,entry->lemma,1);
         } else if (!u_strcmp(field,"CODE")) {
        	 push_output_string(p->stack,entry->semantic_codes[0]);
            for (int i=1;i<entry->n_semantic_codes;i++) {
               push_output_char(p->stack,'+');
               push_output_string(p->stack,entry->semantic_codes[i]);
            }
            for (int i=0;i<entry->n_inflectional_codes;i++) {
            	push_output_char(p->stack,':');
               push_output_string(p->stack,entry->inflectional_codes[i]);
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
         continue;
      #endif
      }
      i++;
      if (l==0) {
         /* Case of $$ in order to print a $ */
     	   push_output_char_tfst(stack,'$');
         continue;
      }
      struct transduction_variable* v=get_transduction_variable(p->variables,name);
      if (v==NULL) {
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: undefined variable $%S$\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: {
               stack->len=old_length;
               stack->str[old_length]='\0';
               return 0;
            }
         }
      }
      if (v->start==UNDEF_VAR_BOUND) {
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
      if (v->end==UNDEF_VAR_BOUND) {
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
      TfstTag* first_tag=(TfstTag*)(p->tfst->tags->tab[v->start]);
      TfstTag* last_tag=(TfstTag*)(p->tfst->tags->tab[v->end]);
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

