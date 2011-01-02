/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

typedef struct {
   MAIN_FUNCTION main;
   vector_ptr* args;
} ProgramInvoker;


ProgramInvoker* new_ProgramInvoker(MAIN_FUNCTION f,const char* name);
void free_ProgramInvoker(ProgramInvoker*);
void add_argument(ProgramInvoker* invoker,const char* arg);
int invoke(ProgramInvoker* invoker);
int invoke_as_new_process(ProgramInvoker* invoker);
void build_command_line(ProgramInvoker* invoker,char* line);


#endif
