/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 //
#ifndef PackInfH
#define PackInfH

#include "AbstractAllocator.h"
#include "AbstractDelaLoad.h"

#include "Fst2.h"
#include "Unicode.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {


struct INF_codes* read_pack_inf_from_memory(const void* buf, size_t size_buf,
                                            Abstract_allocator prv_alloc);

struct INF_codes* read_pack_inf_from_permanent_memory(const void* buf, size_t size_buf,
                                                      Abstract_allocator prv_alloc,
                                                      bool permanentMemory);

struct INF_codes* read_pack_inf_from_permanent_memory_and_secondary_buffer(const void* buf, size_t size_buf,
                                                                           Abstract_allocator prv_alloc,
                                                                           bool permanentMemory,
                                                                           void* SecondaryBuffer,int sizeSecondaryBuffer);

int get_inp_secondary_buffer_size(const void* rawInp, int sizeBuf,
                                  bool fIsPermanentBinInpFile);

struct INF_codes* read_pack_inf_from_memory(const void* buf, size_t size_buf,
                                            Abstract_allocator prv_alloc);
bool convert_inf_to_inp_pack_file(const char* inf_name,
                                  const char* inf_pack_name);

bool write_pack_inp(struct INF_codes*, const char* inf_pack_name);


struct INF_codes* read_pack_inf_from_file(const char* filename,
                                          Abstract_allocator prv_alloc);

void free_pack_inf(struct INF_codes*, Abstract_allocator prv_alloc);

}
#endif
