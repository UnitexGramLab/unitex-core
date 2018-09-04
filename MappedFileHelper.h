/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * additional information: http://www.smartversion.com/unitex-contribution/
 * contact : info@winimage.com
 *
 */



#ifndef MappedFileHelperH
#define MappedFileHelperH

/**
 * This library provides an abstraction for mapping and unmapping files
 * in read-only mode in a portable way.
 */


struct _MAPFILE {
        void* dummy;
        };
typedef struct _MAPFILE MAPFILE;

MAPFILE* iomap_open_mapfile(const char*name,int /* option*/, size_t /*value_for_option*/);

size_t iomap_get_mapfile_size(MAPFILE*);
const void* iomap_get_mapfile_pointer(MAPFILE*, size_t pos=0, size_t sizemap=0);
void iomap_release_mapfile_pointer(MAPFILE*, const void*,size_t sizemap=0);

void iomap_close_mapfile(MAPFILE*);

#endif

