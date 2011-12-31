/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef Fst2CheckH
#define Fst2CheckH

#include <stdio.h>
#include <stdlib.h>
#include "Unicode.h"
#include "Fst2.h"
#include "AbstractFst2Load.h"


int OK_for_Locate(const VersatileEncodingConfig*,const char*,char);
int OK_for_Locate_write_error(const VersatileEncodingConfig*,const char*,char,U_FILE*);
int valid_sentence_automaton(const VersatileEncodingConfig*,const char*);
int valid_sentence_automaton_write_error(const VersatileEncodingConfig*,const char*,U_FILE*);


#endif
