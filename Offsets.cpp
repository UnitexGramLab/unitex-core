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

#include "Offsets.h"
#include "Unicode.h"
#include "Error.h"

/**
 * Loads the given offset file. Returns NULL in case of error.
 */
vector_offset* load_offsets(char* name,int MASK_ENCODING_COMPATIBILITY) {
U_FILE* f=u_fopen_existing_versatile_encoding(MASK_ENCODING_COMPATIBILITY,name,U_READ);
if (f==NULL) return NULL;
int a,b,c,d,n;
vector_offset* res=new_vector_offset();
while ((n=u_fscanf(f,"%d%d%d%d",&a,&b,&c,&d))!=EOF) {
	if (n!=4) {
		fatal_error("Corrupted offset file %s\n",name);
	}
	vector_offset_add(res,a,b,c,d);
}
u_fclose(f);
return res;
}
