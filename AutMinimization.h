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

#ifndef AutMinimizationH
#define AutMinimizationH

#include "SingleGraph.h"
#include "String_hash.h"


/**
 * This library provides a function for minimizing ELAG automata.
 * The minimization algorithm that is used is the one described
 * as "Algorithm 3.6" in the famous Dragon book from Aho, Sethi
 * and Ullman. This idea of this algorithm is to take a deterministic
 * automaton and to find out which states can be merged.
 * The color flavor of this implementation is dued to Eric Laporte.
 * 
 * Author: Eric Laporte
 * Cleaned and commented by Sébastien Paumier
 */

void elag_minimize(SingleGraph,int level=0);

#endif
