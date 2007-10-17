 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef __LIST_AUT_H_
#define __LIST_AUT_H_


#include "autalmot.h"
#include "vector.h"

/* Structure de liste d'automates */

typedef vector_t list_aut;



inline list_aut * list_aut_new(int size = 16) { return vector_new(size); }

inline void list_aut_delete(list_aut * l) { vector_delete(l, (release_f) free_Fst2Automaton); }

inline int list_aut_add(list_aut * l, Fst2Automaton * A) { return vector_add(l, (void *) A); }

#endif
