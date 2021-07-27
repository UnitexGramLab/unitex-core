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
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
 *
 */



#ifndef _ABSTRACT_ALLOCATOR_H_INCLUDED
#define _ABSTRACT_ALLOCATOR_H_INCLUDED 1
#define _CALLBACK_ALLOCATOR_H_INCLUDED 1

#include "stdlib.h"
#include "Error.h"

#include "AbstractCallbackFuncModifier.h"

//#ifndef HAS_UNITEX_NAMESPACE
//#define HAS_UNITEX_NAMESPACE 1
//#endif

//namespace unitex {

#define IS_ASTRACT_ALLOCATOR_EXTENSIBLE 1


struct tag_abstract_allocator;
struct tag_abstract_allocator_info_public_with_allocator;

typedef void* (ABSTRACT_CALLBACK_UNITEX*fnc_alloc_t)(size_t,void*);
typedef void* (ABSTRACT_CALLBACK_UNITEX*fnc_realloc_t)(void*,size_t,size_t,void*);
typedef void (ABSTRACT_CALLBACK_UNITEX*fnc_free_t)(void*,void*);
typedef int (ABSTRACT_CALLBACK_UNITEX*fnc_get_flag_allocator_t)(void*);
typedef void (ABSTRACT_CALLBACK_UNITEX*fnc_clean_allocator_t)(void*);

#define STATISTIC_NB_TOTAL_BYTE_ALLOCATED               0
#define STATISTIC_NB_TOTAL_CURRENT_LIVING_ALLOCATION    1
#define STATISTIC_NB_TOTAL_ALLOCATION_MADE              2


typedef int (ABSTRACT_CALLBACK_UNITEX*fnc_get_statistic_info_t)(int,size_t*,void*);

typedef void (ABSTRACT_CALLBACK_UNITEX* t_fnc_delete_abstract_allocator)(struct tag_abstract_allocator_info_public_with_allocator*,void* privateAllocatorSpacePtr);

typedef struct tag_abstract_allocator_info_public_with_allocator
{
    size_t size_abstract_allocator_info_size;
    fnc_alloc_t fnc_alloc;
    fnc_realloc_t fnc_realloc;
    fnc_free_t fnc_free;
    fnc_get_flag_allocator_t fnc_get_flag_allocator;
    fnc_get_statistic_info_t fnc_get_statistic_allocator_info;
    void* abstract_allocator_ptr;
    fnc_clean_allocator_t fnc_clean_allocator;
} abstract_allocator_info_public_with_allocator;

typedef struct tag_abstract_allocator
{
    size_t size_abstract_allocator;
    abstract_allocator_info_public_with_allocator pub;

    /* only for delete */
    t_fnc_delete_abstract_allocator  fnc_delete_abstract_allocator;
    void* privateAllocatorSpacePtr;

    size_t expected_creation_size;
    char* creator;
    int creation_flag;

} abstract_allocator;

typedef abstract_allocator* Abstract_allocator;

#define AllocatorCreationFlagAutoFreeUnused     0x000000
#define AllocatorCreationFlagAutoFreePrefered   0x000001
#define AllocatorCreationFlagAutoFreeNeeded     0x000002

#define AllocatorCreationFlagAutoFreeMask       0x000003

#define AllocatorFreeOnlyAtAllocatorDelete      0x000004

#define AllocatorCreationFlagCleanPrefered      0x000010

#define AllocatorGetFlagAutoFreePresent         0x000001


#define AllocatorTipOftenRecycledObject         0x000008


#define AllocatorCleanPresent                   0x000010


#define AllocatorTipGrowingOftenRecycledObject  0x000020


/*
 create_abstract_allocator is used when an function need an allocator
 creator is a string with the name of caller
 creationFlagAllocator : can contain a AllocatorCreationFlag* value
 expected_size_item : if non zero, the size of item which be allocated.
 This mean we will always request object of same size
 */


Abstract_allocator create_abstract_allocator(const char*creator,int creationFlagAllocator=0,size_t expected_size_item=0, const void* private_create_ptr=NULL);

void close_abstract_allocator(Abstract_allocator);

int get_allocator_flag(Abstract_allocator);


int get_allocator_creation_flag(Abstract_allocator);
size_t get_allocator_expected_creation_size(Abstract_allocator);
int get_allocator_statistic_info(Abstract_allocator,int iStatNum,size_t*p_value);
const char* get_allocator_creator(Abstract_allocator);
abstract_allocator_info_public_with_allocator* get_abstract_allocator_info_public_with_allocator(Abstract_allocator);

int clean_allocator(Abstract_allocator aa);

#define STANDARD_ALLOCATOR ((Abstract_allocator)NULL)

#define malloc_cb(sz_alloc,prv_ptr) \
    ((prv_ptr==NULL) ? (malloc(sz_alloc)) : \
                       ((prv_ptr->pub.fnc_alloc)((sz_alloc),(prv_ptr->pub.abstract_allocator_ptr))))

#define free_cb(ptr_alloc,prv_ptr) \
    ((prv_ptr==NULL) ? (free(ptr_alloc)) : \
                       ((prv_ptr->pub.fnc_free)((ptr_alloc),(prv_ptr->pub.abstract_allocator_ptr))))

#define realloc_cb(ptr_alloc,sz_alloc_old,sz_alloc_new,prv_ptr) \
    ((prv_ptr==NULL) ? (realloc((ptr_alloc),(sz_alloc_new))) : \
                       ((prv_ptr->pub.fnc_realloc)((ptr_alloc),(sz_alloc_old),(sz_alloc_new),(prv_ptr->pub.abstract_allocator_ptr))))

#define get_allocator_cb_flag(prv_ptr) \
    ((prv_ptr==NULL) ? (0) : \
                       ((prv_ptr->pub.fnc_get_flag_allocator)((prv_ptr->pub.abstract_allocator_ptr))))

//} // namespace unitex

#endif
