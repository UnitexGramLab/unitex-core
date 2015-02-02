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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS) 
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
 *
 */



#ifndef ABSTRACT_FST2_LOAD_H
#define ABSTRACT_FST2_LOAD_H

#include "Fst2.h"

//#ifndef HAS_UNITEX_NAMESPACE
//#define HAS_UNITEX_NAMESPACE 1
//#endif

//namespace unitex {
using namespace unitex;
struct FST2_free_info
{
	void *func_free_fst2;
	void *private_ptr;
	void *privateSpacePtr;
	int must_be_free;
} ;

const struct FST2_free_info FST2_free_info_init={NULL,NULL,NULL,1};

int is_abstract_fst2_filename(const char* filename);
Fst2* load_abstract_fst2(const VersatileEncodingConfig*,const char* filename,int read_names,struct FST2_free_info*);
void free_abstract_Fst2(Fst2*,struct FST2_free_info*);

//} // namespace unitex

#endif
