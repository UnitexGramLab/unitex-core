#ifndef PackFst2H
#define PackFst2H 

#include "Unicode.h"
#include "Alphabet.h"
#include "Fst2.h"
#include "AbstractAllocator.h"
#include "AbstractFst2Load.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

Fst2* read_pack_fst2_from_memory(const void* buf, size_t size_buf,
                                 Abstract_allocator prv_alloc);

bool convert_fst2_to_fst2_pack_file(const char* fst2_name,
                                    const char* fst2_pack_name, bool fVerbose);

bool write_pack_fst2(Fst2* fst2load, const char* fst2_pack_name, bool fVerbose);


Fst2* read_pack_fst2_from_file(const char* filename,
                               Abstract_allocator prv_alloc);

void free_pack_fst2(Fst2* fst2,Abstract_allocator prv_alloc);
}

#endif
