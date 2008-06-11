 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "TransductionStack.h"
#include "Error.h"
#include "DicVariables.h"

#warning add a parameter to Locate for variable error policy
int VARIABLE_ERROR_POLICY=IGNORE_VARIABLE_ERRORS;


/**
 * This function returns a non zero value if c can be a part of a variable name;
 * 0 otherwise.
 */
int is_variable_char(unichar c) {
return ((c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') || c=='_');
}


/**
 * Pushes the given character.
 */
void push_char(struct stack_unichar* s,unichar c) {
push(s,c);
}


/**
 * Pushes the given string to the output, if not NULL.
 */
void push_string(struct stack_unichar* stack,unichar* s) {
int i;
if (s==NULL) {
   return;
}
for (i=0;s[i]!='\0';i++) {
   push_char(stack,s[i]);
}
}


/**
 * Pushes the given string to the output, if not NULL, in the limit of 'length' chars.
 */
void push_substring(struct stack_unichar* stack,unichar* s,int length) {
int i;
if (s==NULL) {
   return;
}
for (i=0;i<length && s[i]!='\0';i++) {
   push_char(stack,s[i]);
}
}


/**
 * This function processes the given output string.
 * Returns 1 if OK; 0 otherwise (for instance, if a variable is 
 * not correctly defined).
 */
int process_output(unichar* s,struct locate_parameters* p) {
int old_stack_pointer=p->stack->stack_pointer;
int i=0;
if (s==NULL || !u_strcmp(s,"<E>")) {
   /* We do nothing if the output is <E> */
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
         switch (VARIABLE_ERROR_POLICY) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: missing closing $ after $%S\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case STOP_MATCH_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
         }
      }
      if (s[i]=='.') {
         /* Here we deal with the case of a field like $a.CODE$ */
         unichar field[128];
         l=0;
         i++;
         while (s[i]!='\0' && s[i]!='$') {
            field[l++]=s[i++];
         }
         field[l]='\0';
         if (s[i]=='\0') {
            switch (VARIABLE_ERROR_POLICY) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: missing closing $ after $%S.%S\n",name,field);
               case IGNORE_VARIABLE_ERRORS: continue;
               case STOP_MATCH_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
            }
         }
         if (field[0]=='\0') {
            switch (VARIABLE_ERROR_POLICY) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: empty field: $%S.$\n",name);
               case IGNORE_VARIABLE_ERRORS: continue;
               case STOP_MATCH_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
            }
         }
         i++;
         struct dela_entry* entry=(struct dela_entry*)get_dic_variable(name,p->dic_variables);
         if (entry==NULL) {
            switch (VARIABLE_ERROR_POLICY) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: undefined morphological variable %S\n",name);
               case IGNORE_VARIABLE_ERRORS: continue;
               case STOP_MATCH_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
            }
         }
         if (!u_strcmp(field,"INFLECTED")) {
            /* TODO: protect special chars */
            push_string(p->stack,entry->inflected);
         } else if (!u_strcmp(field,"LEMMA")) {
            push_string(p->stack,entry->lemma);
         } else if (!u_strcmp(field,"CODE")) {
            push_string(p->stack,entry->semantic_codes[0]);
            for (int i=1;i<entry->n_semantic_codes;i++) {
               push_char(p->stack,'+');
               push_string(p->stack,entry->semantic_codes[i]);
            }
            for (int i=0;i<entry->n_inflectional_codes;i++) {
               push_char(p->stack,':');
               push_string(p->stack,entry->inflectional_codes[i]);
            }
         } else {
            switch (VARIABLE_ERROR_POLICY) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Invalid morphological variable field $%S.%S$\n",name,field);
               case IGNORE_VARIABLE_ERRORS: continue;
               case STOP_MATCH_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
            }
         }
         continue;
      }
      i++;
      if (l==0) {
         /* Case of $$ in order to print a $ */
         push_char(p->stack,'$');
         continue;
      }
      struct transduction_variable* v=get_transduction_variable(p->variables,name);
      if (v==NULL) {
         switch (VARIABLE_ERROR_POLICY) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: undefined variable $%S$\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case STOP_MATCH_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
         }
      }
      if (v->start==UNDEF_VAR_BOUND) {
         switch (VARIABLE_ERROR_POLICY) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: starting position of variable $%S$ undefined\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case STOP_MATCH_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
         }
      }
      if (v->end==UNDEF_VAR_BOUND) {
         switch (VARIABLE_ERROR_POLICY) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position of variable $%S$ undefined\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case STOP_MATCH_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
         }
      }
      if (v->start>v->end) {
         switch (VARIABLE_ERROR_POLICY) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position before starting position for variable $%S$\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case STOP_MATCH_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
         }
      }
      /* If the variable definition is correct */
      for (int k=v->start;k<v->end;k++) {
         push_string(p->stack,p->tokens->value[p->buffer[k+p->current_origin]]);
      }
   }
   else {
      /* If we have a normal character */
      push_char(p->stack,s[i]);
      i++;
   }
}
return 1;
}

