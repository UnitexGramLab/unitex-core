/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Korean.h"


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
void push_input_char(struct stack_unichar* s,unichar c,int protect_dic_chars) {
if (protect_dic_chars && (c==',' || c=='.')) {
	/* We want to protect dots and commas because the Locate program can be used
	 * by Dico to generate dictionary entries */
	push(s,'\\');
}
push(s,c);
}


/**
 * Pushes the given character.
 */
void push_output_char(struct stack_unichar* s,unichar c) {
push(s,c);
}


/**
 * Pushes the given string to the output, if not NULL.
 */
void push_input_string(struct stack_unichar* stack,unichar* s,int protect_dic_chars) {
int i;
if (s==NULL) {
   return;
}
for (i=0;s[i]!='\0';i++) {
    push_input_char(stack,s[i],protect_dic_chars);
}
}


/**
 * Pushes the given string to the output, if not NULL, in the limit of 'length' chars.
 */
void push_input_substring(struct stack_unichar* stack,unichar* s,int length,int protect_dic_chars) {
int i;
if (s==NULL) {
   return;
}
for (i=0;i<length && s[i]!='\0';i++) {
   push_input_char(stack,s[i],protect_dic_chars);
}
}


/**
 * Pushes the given string to the output, if not NULL.
 */
void push_output_string(struct stack_unichar* stack,unichar* s) {
push_input_string(stack,s,0);
}


/**
 * This function processes the given output string.
 * Returns 1 if OK; 0 otherwise (for instance, if a variable is
 * not correctly defined).
 */
int process_output(unichar* s,struct locate_parameters* p) {
int old_stack_pointer=p->stack->stack_pointer;
int i=0;
if (s==NULL) {
   /* We do nothing if there is no output */
   return 1;
}

for (;;) {
    /* First, we push all chars before '\0' or '$' */
    int char_to_push_count=0;
    while ((s[i+char_to_push_count]!='\0') && (s[i+char_to_push_count]!='$')) {
        char_to_push_count++;
    }
    if (char_to_push_count!=0) {
        push_array(p->stack,&s[i],char_to_push_count);
        i+=char_to_push_count;
    }

    if (s[i]=='\0') {
        break;
    }
    /* Now we are sure to have s[i]=='$' */
    {
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
            case BACKTRACK_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
         }
      }
      if (s[i]=='.') {
         /* Here we deal with the case of a field like $a.CODE$ */
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
         if (s[i]=='\0') {
            switch (p->variable_error_policy) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: missing closing $ after $%S.%S\n",name,field);
               case IGNORE_VARIABLE_ERRORS: continue;
               case BACKTRACK_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
            }
         }
         if (field[0]=='\0') {
            switch (p->variable_error_policy) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: empty field: $%S.$\n",name);
               case IGNORE_VARIABLE_ERRORS: continue;
               case BACKTRACK_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
            }
         }
         i++;
         if (!u_strcmp(field,"SET") || !u_strcmp(field,"UNSET")) {
            /* If we have $a.SET$, we must go on only if the variable a is set, no
             * matter if it is a normal or a dictionary variable */
            struct transduction_variable* v=get_transduction_variable(p->variables,name);
            if (v==NULL) {
               /* We do nothing, since this normal variable may not exist */
            } else {
               if (v->start==UNDEF_VAR_BOUND || v->end==UNDEF_VAR_BOUND
                     || v->start>v->end) {
                  /* If the variable is not defined properly */
                  if (field[0]=='S') {
                     /* $a.SET$ is false, we backtrack */
                     p->stack->stack_pointer=old_stack_pointer; return 0;
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
                     p->stack->stack_pointer=old_stack_pointer; return 0;
                  }
               }
            }
            /* If we arrive here, the variable was not a normal one, so we
             * try to match dictionary one */
            struct dela_entry* entry=get_dic_variable(name,p->dic_variables);
            if (entry==NULL) {
               /* If the variable is not defined properly */
               if (field[0]=='S') {
                  /* $a.SET$ is false, we backtrack */
                  p->stack->stack_pointer=old_stack_pointer; return 0;
               } else {
                  /* $a.UNSET$ is true, we go on */
                  continue;
               }
            }
            /* If the dictionary variable is defined */
            if (field[0]=='S') {
               /* $a.SET$ is true, we go on */
               continue;
            } else {
               /* $a.UNSET$ is false, we backtrack */
               p->stack->stack_pointer=old_stack_pointer; return 0;
            }
         }

         struct dela_entry* entry=get_dic_variable(name,p->dic_variables);
         if (entry==NULL) {
            switch (p->variable_error_policy) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: undefined morphological variable %S\n",name);
               case IGNORE_VARIABLE_ERRORS: continue;
               case BACKTRACK_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
            }
         }
         if (u_starts_with(field,"EQ=")) {
            /* We deal with restrictions on semantic codes, especially brewed for
             * Korean dictionary graphs */
            unichar* filter_code=field+3;
            int match=0;
            for (int i=0;!match && i<entry->n_semantic_codes;i++) {
               if (!u_strcmp(entry->semantic_codes[i],filter_code)) {
                  match=1;
               }
            }
            if (match) {
               /* If the filter match, we go on without outputing anything */
               continue;
            } else {
               /* Otherwise, we backtrack */
               p->stack->stack_pointer=old_stack_pointer; return 0;
            }
         } else if (!u_strcmp(field,"INFLECTED")) {
            /* We use push_input_string because it can protect special chars */
            if (p->korean!=NULL) {
               /* If we work in Korean mode, we must convert text into Hanguls */
               unichar z[1024];
               convert_jamo_to_hangul(entry->inflected,z,p->korean);
               push_input_string(p->stack,z,p->protect_dic_chars);
            } else {
               push_input_string(p->stack,entry->inflected,p->protect_dic_chars);
            }
         } else if (!u_strcmp(field,"LEMMA")) {
        	 push_input_string(p->stack,entry->lemma,p->protect_dic_chars);
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
         } else if (!u_strcmp(field,"CODE.GRAM")) {
            push_output_string(p->stack,entry->semantic_codes[0]);
         } else if (!u_strcmp(field,"CODE.SEM")) {
            if (entry->n_semantic_codes>1) {
               push_output_string(p->stack,entry->semantic_codes[1]);
               for (int i=2;i<entry->n_semantic_codes;i++) {
                  push_output_char(p->stack,'+');
                  push_output_string(p->stack,entry->semantic_codes[i]);
               }
            }
         } else if (!u_strcmp(field,"CODE.FLEX")) {
            if (entry->n_inflectional_codes>0) {
               push_output_string(p->stack,entry->inflectional_codes[0]);
               for (int i=1;i<entry->n_inflectional_codes;i++) {
                  push_output_char(p->stack,':');
                  push_output_string(p->stack,entry->inflectional_codes[i]);
               }
            }
         } else {
            switch (p->variable_error_policy) {
               case EXIT_ON_VARIABLE_ERRORS: fatal_error("Invalid morphological variable field $%S.%S$\n",name,field);
               case IGNORE_VARIABLE_ERRORS: continue;
               case BACKTRACK_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
            }
         }
         continue;
      }
      i++;
      if (l==0) {
         /* Case of $$ in order to print a $ */
    	  push_output_char(p->stack,'$');
         continue;
      }
      struct transduction_variable* v=get_transduction_variable(p->variables,name);
      if (v==NULL) {
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: undefined variable $%S$\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
         }
      }
      if (v->start==UNDEF_VAR_BOUND) {
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: starting position of variable $%S$ undefined\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
         }
      }
      if (v->end==UNDEF_VAR_BOUND) {
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position of variable $%S$ undefined\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
         }
      }
      if (v->start>v->end) {
         switch (p->variable_error_policy) {
            case EXIT_ON_VARIABLE_ERRORS: fatal_error("Output error: end position before starting position for variable $%S$\n",name);
            case IGNORE_VARIABLE_ERRORS: continue;
            case BACKTRACK_ON_VARIABLE_ERRORS: p->stack->stack_pointer=old_stack_pointer; return 0;
         }
      }
      /* If the variable definition is correct */
      for (int k=v->start;k<v->end;k++) {
         push_input_string(p->stack,p->tokens->value[p->buffer[k+p->current_origin]],p->protect_dic_chars);
      }
   }
}

return 1;
}

