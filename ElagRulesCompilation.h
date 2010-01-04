 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#ifndef ElagRulesCompilationH
#define ElagRulesCompilationH

#include "Fst2Automaton.h"


/**
 * This library provides the tools for compiling Elag rules.
 *
 * Authors: Laurent Mayer et al (1998)
 *          Olivier Blanc (2002-2006)
 * Modified by S�bastien Paumier
 */


/**
 * This is the maximum number of constraints for an Elag rule.
 */
#define ELAG_MAX_CONSTRAINTS 8


/**
 * This structure defines a couple of automata. It is used
 * to store the left and right parts of an Elag rule.
 */
typedef struct {
   SingleGraph left;
   SingleGraph right;
} elContext;


/**
 * This structure defines a single ELAG rule.
 */
typedef struct {
   /* Name of the .fst2 file that contains this rule */
   char* name;

   /* The automaton that corresponds to the rule */
   Fst2Automaton* automaton;

   /* Number of contexts, at least 1 */
   int nbContexts;

   /* Array of the contexts */
   elContext* contexts;
} elRule;


int compile_elag_grammar(char*,char*,Encoding,int,language_t*);
int compile_elag_rules(char*,char*,Encoding,int,language_t*);

#endif
