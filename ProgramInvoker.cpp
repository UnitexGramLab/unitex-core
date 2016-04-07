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

#include <string.h>
#include "ProgramInvoker.h"
#include "UnusedParameter.h"
#include "Error.h"
#include "Ustring.h"
#include "SyncTool.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

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
 * Add the given option to the invoker as an argument of the form:
 *
 *    --opt_name=opt_value
 *
 * or
 *    --opt_name
 *
 * if opt_value is NULL.
 */
void add_long_option(ProgramInvoker* invoker,const char* opt_name,const char* opt_value) {
if (opt_name==NULL) {
   fatal_error("NULL argument in new_ProgramInvoker\n");
}
int size=2+(int)strlen(opt_name)+1;
if (opt_value!=NULL) {
	size=size+1+(int)strlen(opt_value);
}
char* tmp=(char*)malloc(size*sizeof(char));
if (tmp==NULL) {
   fatal_alloc_error("add_long_option");
}
if (opt_value==NULL) {
	sprintf(tmp,"--%s",opt_name);
} else {
	sprintf(tmp,"--%s=%s",opt_name,opt_value);
}
vector_ptr_add(invoker->args,tmp);
}


/**
 * Removes the last argument of the given invoker.
 */
void remove_last_argument(ProgramInvoker* invoker) {
if (invoker->args->nbelems==0) {
	fatal_error("remove_last_argument: cannot remove an argument from an empty invoker\n");
}
free(invoker->args->tab[--(invoker->args->nbelems)]);
invoker->args->tab[invoker->args->nbelems]=NULL;
}



/**
 * Invoke the main function.
 */
int invoke(ProgramInvoker* invoker) {
if (invoker->main==NULL) {
   fatal_error("NULL main pointer in invoke\n");
}
int argc=invoker->args->nbelems;
/* We duplicate the argv array, since the caller may not want
 * the arguments to be reordered by getopt */
char** argv=(char**)malloc((argc+1)*sizeof(char*));
if (argv==NULL) {
	fatal_alloc_error("invoke");
}
for (int i=0;i<argc;i++) {
	argv[i]=strdup((char*)(invoker->args->tab[i]));
	if (argv[i]==NULL) {
		fatal_alloc_error("invoke");
	}
}
argv[argc]=NULL;
int ret=invoker->main(argc,argv);
for (int i=0;i<argc;i++) {
	free(argv[i]);
}
free(argv);
return ret;
}


/**
 * Builds and returns a command line ready to be used with a 'system' call.
 */
void build_command_line(ProgramInvoker* invoker,char* line) {
/* If we are under Windows, we have to surround the whole command line with an
 * additional pair of double quotes */
#ifdef _NOT_UNDER_WINDOWS
const char* protection="";
#else
const char* protection="\"";
#endif
sprintf(line,"%s\"%s\"",protection,(const char*)(invoker->args->tab[0]));
for (int i=1;i<invoker->args->nbelems;i++) {
   strcat(line," \"");
   strcat(line,(const char*)(invoker->args->tab[i]));
   strcat(line,"\"");
}
strcat(line,protection);
}


/**
 * arg_need_protection return 1 if argument need quote protection
 */
static int arg_need_protection(const char* arg)
{
	for (;;)
	{
		char c = *(arg++);
		if (c == '\0')
			return 0;
		if ((c == '-') || (c == ',') || (c == '/') || (c == '_'))
			continue;
		if ((c >= 'A') && (c <= 'Z'))
			continue;
		if ((c >= 'a') && (c <= 'z'))
			continue;
		if ((c >= '0') && (c <= '9'))
			continue;
		return 1;
	}
}


/**
 * Builds and returns a command line ready to be used with a 'system' call, alloc a memory buffer.
 */
char* build_command_line_alloc(ProgramInvoker* invoker, int line_protection, int skip_main) {
/* If we are under Windows, we have to surround the whole command line with an
 * additional pair of double quotes */
#ifdef _NOT_UNDER_WINDOWS
const char* protection="";
DISCARD_UNUSED_PARAMETER(line_protection)
#else
const char* protection= line_protection ? "\"" : "";
#endif

size_t totalize_size=2;
for (int count_for_totalize=0;count_for_totalize<invoker->args->nbelems;count_for_totalize++) {
  totalize_size+=strlen((const char*)(invoker->args->tab[count_for_totalize]))+4;
}

char* line=(char*)malloc(totalize_size);
if (line==NULL) {
   fatal_alloc_error("build_command_line_alloc");
}

const char* name_tool = (const char*)(invoker->args->tab[0]);
if ((skip_main) && (strlen(name_tool) > 5) && (memcmp(name_tool,"main_",5)==0)) {
   sprintf(line, "%s", name_tool + 5);
} else {
   sprintf(line, "%s\"%s\"", protection, (const char*)(invoker->args->tab[0]));
}
for (int i=1;i<invoker->args->nbelems;i++) {
   const char* cur_arg = (const char*)(invoker->args->tab[i]);
   int cur_arg_need_protection = arg_need_protection(cur_arg);
   strcat(line, cur_arg_need_protection ? " \"" : " ");
   strcat(line, cur_arg);
   strcat(line, cur_arg_need_protection ? "\"" : "");
}
strcat(line,protection);
return line;
}


/**
 * free a command line allocated memory buffer.
 */
void free_command_line_alloc(char* line) {
	free(line);
}


/**
 * The full version, receiving all arguments with NULL to end the list.
 */
int exec_unitex_command(MAIN_FUNCTION f,const char* name,...) {
ProgramInvoker* invoker=new_ProgramInvoker(f,name);
va_list list;
va_start(list,name);
char* arg;
while ((arg=va_arg(list,char*))!=NULL) {
	add_argument(invoker,arg);
}
va_end(list);
int ret=invoke(invoker);
free_ProgramInvoker(invoker);
return ret;
}

} // namespace unitex
