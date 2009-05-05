 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "ProgramInvoker.h"
#include "Error.h"
#include "Ustring.h"
#include <string.h>

/**
 * Allocates, initializes and returns a new program invoker.
 */
ProgramInvoker* new_ProgramInvoker(MAIN_FUNCTION f,const char* name) {
if (name==NULL) {
   fatal_error("NULL program name in new_ProgramInvoker\n");
}
ProgramInvoker* res=(ProgramInvoker*)malloc(sizeof(ProgramInvoker));
if (res==NULL) {
   fatal_alloc_error("new_ProgramInvoker");
}
res->main=f;
res->args=new_vector_ptr(16);
add_argument(res,name);
return res;
}


/**
 * Frees all the memory associated to the given invoker.
 */
void free_ProgramInvoker(ProgramInvoker* i) {
if (i==NULL) return;
free_vector_ptr(i->args,free);
free(i);
}


/**
 * Add the given argument to the given invoker.
 */
void add_argument(ProgramInvoker* invoker,const char* arg) {
if (arg==NULL) {
   fatal_error("NULL argument in new_ProgramInvoker\n");
}
char* tmp=strdup(arg);
if (tmp==NULL) {
   fatal_alloc_error("add_argument");
}
vector_ptr_add(invoker->args,tmp);
}


/**
 * Invoke the main function.
 */
int invoke(ProgramInvoker* invoker) {
if (invoker->main==NULL) {
   fatal_error("NULL main pointer in invoke\n");
}
return invoker->main(invoker->args->nbelems,(char**)(invoker->args->tab));
}


/**
 * Invoke the main function.
 */
int invoke_as_new_process(ProgramInvoker* invoker) {
char line[4096];
build_command_line(invoker,line);
int ret=system(line);
return ret;
}


/**
 * Builds and returns a command line ready to be used with a 'system' call.
 */
void build_command_line(ProgramInvoker* invoker,char* line) {
sprintf(line,"%s",(char*)(invoker->args->tab[0]));
for (int i=1;i<invoker->args->nbelems;i++) {
   strcat(line," \"");
   strcat(line,(char*)(invoker->args->tab[i]));
   strcat(line,"\"");
}
}
