 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef _TAUTOMATON_H_
#define _TAUTOMATON_H_

/* T comme template ? */

#include "unicode.h"

#define T_INITIAL   (1)
#define T_FINAL     (2)
#define T_TERMINAL  T_FINAL


class Ttransition {

public:

   int to;
   unichar * label;

   Ttransition(int _to, unichar * _label) : to(_to), label(_label) {}
   ~Ttransition() { debug("Ttransition: Ouch!\n"); }
};




class TState {

public:

   int flags;
   list<Ttransition> trans;

   TState(int _flags = 0) : flags(_flags), trans() {}
   ~TState() {}
};


class TAutomaton {

public:

  unichar * name;

  vector<TState> states;
  
  vector<int> initials;

  bool labels_are_mine; /* true if we should free labels on deletion */

  //  void * data;

  TAutomaton(unichar * _name = 0) : name(u_strdup(_name)), states(), initials() {}
  ~TAutomaton() { free(name); }

  inline void make_empty() {
    states.clear();
    initials.clear();
  }
  
  inline void reserve(int size) { states.reserve(size); }
  
  inline void rename(unichar * name) { free(name); name = u_strdup(name); }

  void set_initial(int stateno); 
  void unset_initial(int stateno);
  
  inline void set_final(int stateno)   { states[stateno].set_final(); }
  inline void unset_final(int stateno) { states[stateno].unset_final(); }

  inline void is_initial(int q) { return states[q].flags & AUT_INITIAL; }
  inline void is_final(int q) { return states[q].flags & AUT_FINAL; }
  
  int add_state(int no, int flags = 0);
  void add_trans(int from, unichar * label, int to);

  void dump(ostream & os = cerr);
  void dump_dot(ostream & os = cerr);
  void dump_dot(char * fname);
};


#endif

