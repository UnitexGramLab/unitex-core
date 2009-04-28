 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#include "Unicode.h"
#include "Fst2.h"
#include "AbstractFst2Load.h"


Fst2* load_abstract_fst2(char* filename,int read_names,struct FST2_free_info* p_fst2_free_info)
{
	Fst2* res = NULL;
	res = load_fst2(filename, read_names);

	if ((res != NULL) && (p_fst2_free_info != NULL))
		p_fst2_free_info->must_be_free = 1;

	return res;
}

void free_abstract_Fst2(Fst2* fst2,struct FST2_free_info* p_fst2_free_info)
{
	if (fst2 != NULL)
		if (p_fst2_free_info->must_be_free != 0)
			free_Fst2(fst2);
}
