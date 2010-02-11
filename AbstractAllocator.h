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


#ifndef _ABSTRACT_ALLOCATOR_H_INCLUDED
#define _ABSTRACT_ALLOCATOR_H_INCLUDED 1
#define _CALLBACK_ALLOCATOR_H_INCLUDED 1

#include "stdlib.h"
#include "Error.h"

#include "AbstractCallbackFuncModifier.h"

struct tag_abstract_allocator;

typedef void* (ABSTRACT_CALLBACK_UNITEX*fnc_alloc_t)(size_t,void*);
typedef void* (ABSTRACT_CALLBACK_UNITEX*fnc_realloc_t)(void*,size_t,size_t,void*);
typedef void (ABSTRACT_CALLBACK_UNITEX*fnc_free_t)(void*,void*);
typedef size_t (ABSTRACT_CALLBACK_UNITEX*fnc_get_size_allocator_t)(void*);

typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_delete_abstract_allocator)(struct tag_abstract_allocator*,void* privateAllocatorSpacePtr);


typedef struct tag_abstract_allocator
{
    fnc_alloc_t fnc_alloc;
    fnc_realloc_t fnc_realloc;
    fnc_free_t fnc_free;
    fnc_get_size_allocator_t fnc_get_size_allocator;
    void* abstract_allocator_ptr;

    /* only for delete */
    t_fnc_delete_abstract_allocator  fnc_delete_abstract_allocator;
    void* privateAllocatorSpacePtr;

} abstract_allocator;

typedef abstract_allocator* Abstract_allocator;


Abstract_allocator create_abstract_allocator(const char*creator,int flagAllocator=0,size_t expected_size=0);

void close_abstract_allocator(Abstract_allocator);

size_t get_allocator_size(Abstract_allocator);

#define STANDARD_ALLOCATOR ((Abstract_allocator)NULL)

#define malloc_cb(sz_alloc,prv_ptr) \
    ((prv_ptr==NULL) ? (malloc(sz_alloc)) : \
                       ((prv_ptr->fnc_alloc)((sz_alloc),(prv_ptr->abstract_allocator_ptr))))

#define free_cb(ptr_alloc,prv_ptr) \
    ((prv_ptr==NULL) ? (free(ptr_alloc)) : \
                       ((prv_ptr->fnc_free)((ptr_alloc),(prv_ptr->abstract_allocator_ptr))))

#define realloc_cb(ptr_alloc,sz_alloc_old,sz_alloc_new,prv_ptr) \
    ((prv_ptr==NULL) ? (realloc((ptr_alloc),(sz_alloc_new))) : \
                       ((prv_ptr->fnc_realloc)((ptr_alloc),(sz_alloc_old),(sz_alloc_new),(prv_ptr->abstract_allocator_ptr))))


#endif
