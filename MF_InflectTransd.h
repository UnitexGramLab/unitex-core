/*
  * Unitex 
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
  *
  */

/* Created by Agata Savary (savary@univ-tours.fr)
 * Last modification on July 22 2005
 */
//---------------------------------------------------------------------------

////////////////////////////////////////////////////////////
// Implementation of the management of inflectional tranducers
////////////////////////////////////////////////////////////
// Copy of some functions from Inflect.cpp

#include "Fst2.h"

//Maximum number of flexional transducers
#define N_FST2 3000

///////////////////////////////
// A node of the tree for inflection transducers' names
struct node {
  int final;  //-1 if the node is non final; otherwise gives the index
              //of the inflection transducer (whose name leads to this node) 
              //in the tranducer table
  struct transition* t;
};

///////////////////////////////
// A branch of the tree for inflection transducers' names
struct transition {
    char c;
    struct node* n;
    struct transition* suivant;
};

///////////////////////////////
//Initiate the tree for inflection transducers' names
//On succes return 0, 1 otherwise
int init_transducer_tree();

///////////////////////////////
// Free the transducer tree memory
void free_transducer_tree();

///////////////////////////////
// Try to load the transducer flex and returns its position in the
// 'fst2' array. Returns -1 if the transducer cannot be loaded
int get_transducer(char* flex);
