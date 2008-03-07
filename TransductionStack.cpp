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


int DISPLAY_VARIABLE_ERRORS=0;



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
 * This function processes the given output string.
 */
void process_output(unichar* s,struct locate_parameters* p) {
int i=0;
if (s==NULL || !u_strcmp(s,"<E>")) {
   /* We do nothing if the output is <E> */
   return;
}
while (s[i]!='\0') {
   if (s[i]=='$') {
      /* Case of a variable name */
      unichar name[100];
      int l=0;
      i++;
      while (is_variable_char(s[i])) {
         name[l++]=s[i++];
      }
      name[l]='\0';
      if (s[i]!='$') {
         if (DISPLAY_VARIABLE_ERRORS) {
            error("Error: missing closing $ after $%S\n",name);
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
         if (DISPLAY_VARIABLE_ERRORS) {
            error("Error: undefined variable $%S\n",name);
         }
         continue;
      }
      if (v->start==-1) {
         if (DISPLAY_VARIABLE_ERRORS) {
            error("Error: starting position of variable $%S undefined\n",name);
         }
         continue;
      }
      if (v->end==-1) {
         if (DISPLAY_VARIABLE_ERRORS) {
            error("Error: end position of variable $%S undefined\n",name);
         }
         continue;
      }
      if (v->start>v->end) {
         if (DISPLAY_VARIABLE_ERRORS) {
            error("Error: end position before starting position for variable $%S\n",name);
         }
         continue;
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
}

