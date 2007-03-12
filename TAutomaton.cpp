 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

/* T comme template ? */

#include "Unicode.h"

#define T_INITIAL   (1)
#define T_FINAL     (2)
#define T_TERMINAL  T_FINAL


class Ttransition {

public:

   int to;
   unichar * label;

   Ttransition(int _to, unichar * _label) : to(_to), label(_label) {}
   ~Ttransition() { debug("Ttransition: Ouch!\n"); }

} Ttransision;




class TState {

public:
   int flags;
   list<Ttransition> trans;

   TState(int _flags = 0) : flags(_flags), trans() {}
   ~TState() {}
} TState;


typedef struct TAutomaton {

  unichar * name;

  TState * states;
  int nbstates;

  int size;
  
  int * initials;
  int nbinitials;

  bool labels_are_mine; /* true if we should free labels on deletion */

  //  void * data;

} TAutomaton;
