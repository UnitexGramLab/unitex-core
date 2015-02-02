/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef ProgramInvokerH
#define ProgramInvokerH

/**
 * This library provides an easy way to build the argc and argv parameters
 * needed by any main_Foo function.
 */

#include "Vector.h"
#include "Ustring.h"

typedef int (*MAIN_FUNCTION)(int argc,char* const argv[]);

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

typedef struct {
   MAIN_FUNCTION main;
   vector_ptr* args;
} ProgramInvoker;


ProgramInvoker* new_ProgramInvoker(MAIN_FUNCTION f,const char* name);
void free_ProgramInvoker(ProgramInvoker*);
void add_argument(ProgramInvoker* invoker,const char* arg);
void add_long_option(ProgramInvoker* invoker,const char* opt_name,const char* opt_value);
void remove_last_argument(ProgramInvoker* invoker);
int invoke(ProgramInvoker* invoker);
void build_command_line(ProgramInvoker* invoker,char* line);
char* build_command_line_alloc(ProgramInvoker* invoker);
void free_command_line_alloc(char* line);
int exec_unitex_command(MAIN_FUNCTION f,const char* name,...);

} // namespace unitex

#endif
