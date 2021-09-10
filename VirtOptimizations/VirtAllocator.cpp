/*
 * Unitex - Performance optimization code 
 *
 * File created and contributed by Gilles Vollant, working with François Liger
 * as part of an UNITEX optimization and reliability effort, first descibed at
 * http://www.smartversion.com/unitex-contribution/Unitex_A_NLP_engine_from_the_lab_to_the_iPhone.pdf
 *
 * Free software when used with Unitex 3.2 or later
 *
 * Copyright (C) 2021-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


#include "AbstractCallbackFuncModifier.h"
#include "AbstractAllocatorPlugCallback.h"
#include "FuncDeclModifier.h"
#include "VirtAllocator.h"


#include "UnusedParameter.h"


#ifdef HAS_UNITEX_NAMESPACE
using namespace unitex;
#endif


//#define VERBOSE_VIRTALLOC
#define alloc_around(v,round) ((((v)+((round)-1)) / (round)) * (round))

//#define POOL_ATOM_SIZE (1024*64)
#define POOL_ATOM_SIZE (1024*64)
//#define POOL_ATOM_ALIGN (sizeof(void*))
#define POOL_ATOM_ALIGN (0x10)



#ifdef VERBOSE_VIRTALLOC
#include <stdio.h>
#include <string.h>
#endif

static size_t MyBigGetAtomSize()
{
    return POOL_ATOM_SIZE;
}

static void* MyBigMalloc(size_t size_alloc)
{
    return malloc(size_alloc);
}

static void MyBigFree(void*ptr)
{
    free(ptr);
}


struct my_allocator_pool_atom {
    void*buf;
    struct my_allocator_pool_atom* previous;
    size_t size_buf;
    size_t pos_buf;
    int is_mini_alloc;
} ;

struct my_allocator_pool {
    struct my_allocator_pool_atom *last;
    size_t pool_atom_size;

#ifdef VERBOSE_VIRTALLOC
    const char*creator_given;
    size_t site_item_given;
#endif
} ;




struct my_allocator_pool_atom* build_allocator_pool_atom(size_t size_buf)
{
    struct my_allocator_pool_atom* ret= (struct my_allocator_pool_atom*)malloc(sizeof(struct my_allocator_pool_atom));
    if (ret == NULL) {
        fatal_alloc_error("build_allocator_pool_atom");
    }
    ret -> pos_buf = 0;
    ret -> previous = NULL;
    ret -> size_buf = size_buf;
    
    ret -> is_mini_alloc = size_buf < MyBigGetAtomSize();
    if (ret -> is_mini_alloc)
        ret -> buf = malloc(ret -> size_buf);
    else
        ret -> buf = MyBigMalloc(ret -> size_buf);

    if (ret->buf == NULL) {
        fatal_alloc_error("build_allocator_pool_atom");
    }
    
    return ret;
}

struct my_allocator_pool* build_allocator_pool(size_t atom)
{
    struct my_allocator_pool* ret= (struct my_allocator_pool*)malloc(sizeof(struct my_allocator_pool));
    if (ret == NULL) {
        fatal_alloc_error("build_allocator_pool");
    }
    ret -> pool_atom_size = MyBigGetAtomSize();
    ret -> last = build_allocator_pool_atom((atom==0) ? ret -> pool_atom_size : atom);

#ifdef VERBOSE_VIRTALLOC
    {
        char sz[80]; sprintf(sz,"build pool %lx ",(unsigned long)ret); puts(sz);
    }
#endif
    return ret;
}

size_t get_allocator_pool_size(struct my_allocator_pool* pool);
void delete_allocator_pool(struct my_allocator_pool* pool)
{


#ifdef VERBOSE_VIRTALLOC
    {
        size_t sizepool=get_allocator_pool_size(pool);
        char sz[80]; sprintf(sz,"size of pool %lu for pool %lx ",(unsigned long)sizepool,(unsigned long)pool); puts(sz);
    }
#endif

    struct my_allocator_pool_atom* browse = pool -> last;
    while (browse != NULL)
    {
        struct my_allocator_pool_atom* tmp = browse -> previous;
        if (browse -> is_mini_alloc)
            free(browse->buf);
        else
            MyBigFree(browse->buf);

        free(browse);
        browse = tmp;
    }
    free(pool);
}

void ABSTRACT_CALLBACK_UNITEX clean_allocator_pool(void*pb)
{

    struct my_allocator_pool* pool = (struct my_allocator_pool*)pb;
#ifdef VERBOSE_VIRTALLOC
    {
        size_t sizepool=get_allocator_pool_size(pool);
        char sz[80]; sprintf(sz,"size of pool %lu for pool %lx ",(unsigned long)sizepool,(unsigned long)pool); puts(sz);
    }
#endif
	

    struct my_allocator_pool_atom* browse = pool -> last;
    while (browse -> previous != NULL)
    {
        struct my_allocator_pool_atom* tmp = browse -> previous;

        if (browse -> is_mini_alloc)
            free(browse->buf);
        else
            MyBigFree(browse->buf);

        free(browse);
        browse = tmp;
    }
	pool->last = browse;
	browse->pos_buf = 0;
}

size_t get_allocator_pool_size(struct my_allocator_pool* pool)
{
    if (pool == NULL)
        return 0;
    size_t total=0;
    struct my_allocator_pool_atom* browse = pool -> last;
    while (browse != NULL)
    {
        total += browse->pos_buf;
        browse = browse->previous;
    }
    return total;
}

int ABSTRACT_CALLBACK_UNITEX get_allocator_pool_flag(void*)
{
    return AllocatorGetFlagAutoFreePresent | AllocatorCleanPresent;
}




void* ABSTRACT_CALLBACK_UNITEX my_alloc_cb(size_t size,void* pb)
{ 
    size_t rounded_size = alloc_around(size,POOL_ATOM_ALIGN);
    struct my_allocator_pool* ap = (struct my_allocator_pool*)pb;

    if (((ap->last->pos_buf) + rounded_size) > (ap->last->size_buf)) {
        size_t size_build_atom = (rounded_size<(ap -> pool_atom_size)) ? 
                    (ap -> pool_atom_size) : alloc_around(rounded_size,ap -> pool_atom_size);
        if (ap->last->previous == NULL)
            if (ap->last->is_mini_alloc)
            {
                size_build_atom = ap->last->size_buf * 0x10;
            }
        struct my_allocator_pool_atom* new_atom = 
            build_allocator_pool_atom(size_build_atom);
        if (new_atom == NULL) {
            fatal_alloc_error("my_alloc_cb");    
        }
        new_atom->previous = ap->last;
        ap->last = new_atom;
    }
    
    struct my_allocator_pool_atom* work_atom = ap->last;

    void*ret = (void*)(((char*)work_atom->buf)+ (work_atom->pos_buf));
    work_atom->pos_buf += rounded_size;
    return ret;
}


void ABSTRACT_CALLBACK_UNITEX my_free_cb(void*,void*)
{
}


void* ABSTRACT_CALLBACK_UNITEX my_realloc_cb(void* oldptr,size_t oldsize,size_t newsize,void*pb)
{
    void*ret;
    if (oldsize >= newsize)
        return oldptr;
    ret = my_alloc_cb(newsize,pb);
    if ((oldptr!=NULL) && (ret != NULL)) {
        memcpy(ret,oldptr,oldsize);
    }
    return ret;
}


size_t ABSTRACT_CALLBACK_UNITEX get_size_abstract_allocator_pool(void* pb)
{
    struct my_allocator_pool* ap = (struct my_allocator_pool*)pb;

    size_t ret = 0;
    if (ap != NULL)
      ret = get_allocator_pool_size(ap);
    return ret;
}

int ABSTRACT_CALLBACK_UNITEX get_statistic_info_abstract_allocator_pool(int iStatInfoItem,size_t*p_value,void* pb)
{
    struct my_allocator_pool* ap = (struct my_allocator_pool*)pb;

    int ret = 0;
    if (ap != NULL)
    {
        if (iStatInfoItem==STATISTIC_NB_TOTAL_BYTE_ALLOCATED)
        {
            *p_value = get_allocator_pool_size(ap);
            ret=1;
        }
    }
    return ret;
}

int ABSTRACT_CALLBACK_UNITEX is_param_allocator_virt_pool_compatible(const char*creator,int flagAllocator,size_t,const void* private_create_ptr,void* privateAllocatorSpacePtr)
{
    DISCARD_UNUSED_PARAMETER(creator)
    DISCARD_UNUSED_PARAMETER(private_create_ptr)
    DISCARD_UNUSED_PARAMETER(privateAllocatorSpacePtr)
    if ((flagAllocator & AllocatorTipOftenRecycledObject) != 0)
        return 0;
    return 1;
}

int ABSTRACT_CALLBACK_UNITEX create_abstract_allocator_pool(abstract_allocator_info_public_with_allocator* aa,
                                                            const char*creator,int flagAllocator,size_t size_item,
                                                            const void* private_create_ptr,
                                                            void* privateAllocatorSpacePtr)
{
    size_t size_atom_pool=0;
    DISCARD_UNUSED_PARAMETER(creator)
    DISCARD_UNUSED_PARAMETER(flagAllocator)
    DISCARD_UNUSED_PARAMETER(size_item)
    DISCARD_UNUSED_PARAMETER(privateAllocatorSpacePtr)

    if (private_create_ptr != NULL)
    {
        VirtAllocatorPrivateStruct* pVirtAllocatorPrivateStruct=(VirtAllocatorPrivateStruct*)private_create_ptr;
        if (pVirtAllocatorPrivateStruct->expected_total_size != 0)
            size_atom_pool = pVirtAllocatorPrivateStruct->expected_total_size;
    }

    if ((size_atom_pool == 0) && (size_item!=0))
    {
        size_atom_pool = size_item*0x20;
    }

    struct my_allocator_pool* pool=build_allocator_pool(size_atom_pool);

    if (pool == NULL)
        return 0;

#ifdef VERBOSE_VIRTALLOC
    pool->creator_given=creator;
    pool->site_item_given=size_item;
#endif

#ifdef IS_ASTRACT_ALLOCATOR_EXTENSIBLE
	aa->size_abstract_allocator_info_size = sizeof(abstract_allocator_info_public_with_allocator);
#endif
    aa->fnc_alloc = my_alloc_cb;
    aa->fnc_free = my_free_cb;
    aa->fnc_realloc = my_realloc_cb;
    aa->fnc_get_flag_allocator = get_allocator_pool_flag;
	aa->fnc_clean_allocator = clean_allocator_pool;
    aa->fnc_get_statistic_allocator_info = get_statistic_info_abstract_allocator_pool;
    aa->abstract_allocator_ptr = pool;
    return 1;
}


void ABSTRACT_CALLBACK_UNITEX delete_abstract_allocator_pool(abstract_allocator_info_public_with_allocator* aa,void* privateAllocatorSpacePtr)
{
    DISCARD_UNUSED_PARAMETER(privateAllocatorSpacePtr)
    struct my_allocator_pool* ap = (struct my_allocator_pool*)aa->abstract_allocator_ptr;
#ifdef VERBOSE_VIRTALLOC
    size_t total_size = get_allocator_pool_size(ap);
    char msg[0x80];
    sprintf(msg," *POOL ALLOCATOR info = for '%s', size=%u (atom size %u,item size %u)\n",ap->creator_given,(unsigned int)total_size,(unsigned int)ap->pool_atom_size,(unsigned int)ap->site_item_given);
    puts(msg);
#endif

    if (ap != NULL)
      delete_allocator_pool(ap);
}

const t_allocator_func_array allocator_func_array =
{
    sizeof(t_allocator_func_array),
    NULL,
    NULL,

    is_param_allocator_virt_pool_compatible,


    create_abstract_allocator_pool,
    delete_abstract_allocator_pool,
};

ULB_VFFUNC const t_allocator_func_array * ULIB_CALL GetVirtAllocatorFuncArray()
{
    return &allocator_func_array;
}

class autoinstall_va {
public:
    autoinstall_va() {
        AddAllocatorSpace(&allocator_func_array,this);
    };

    ~autoinstall_va() {
        RemoveAllocatorSpace(&allocator_func_array,this);
    };

} ;

autoinstall_va autoinstall_va_instance;











/*****************************************************************************************/


#define my_around_align(x)  ((((x)+0x0f)/0x10)*0x10)



/*****************************************************************************************/

struct my_allocator_recyle_item
{
    struct my_allocator_recyle_item* next_free;
} ;

struct my_allocator_recycle {
    struct my_allocator_recyle_item* first_free;
    size_t size_item;
    
#ifdef VERBOSE_VIRTALLOC
    const char*creator_given;
    size_t site_item_given;
#endif
} ;







struct my_allocator_recycle* build_allocator_recycle(size_t atom)
{
    struct my_allocator_recycle* ret= (struct my_allocator_recycle*)malloc(sizeof(struct my_allocator_recycle));
    if (ret == NULL) {
        fatal_alloc_error("build_allocator_recycle");
    }
    ret -> size_item = atom;
    ret -> first_free = NULL;
    return ret;
}

size_t get_allocator_recycle_size(struct my_allocator_recycle* recycle);
void delete_allocator_recycle(struct my_allocator_recycle* recycle)
{


    struct my_allocator_recyle_item* browse=recycle->first_free;
    while (browse!=NULL)
    {
        struct my_allocator_recyle_item* next_browse = browse->next_free;
        free(browse);
        browse=next_browse;
    }
    free(recycle);
}


size_t get_allocator_recycle_size(struct my_allocator_recycle* recycle)
{
    if (recycle == NULL)
        return 0;
    return recycle->size_item;
}

int ABSTRACT_CALLBACK_UNITEX get_allocator_recycle_flag(void*)
{
    return AllocatorTipOftenRecycledObject;
}




void* ABSTRACT_CALLBACK_UNITEX my_alloc_recycle_cb(size_t size,void* pb)
{ 
    struct my_allocator_recycle* ap = (struct my_allocator_recycle*)pb;
    if (size > ap->size_item)
        return NULL;
    if (ap->first_free == NULL)
    {
        unsigned char*buf;
        buf=(unsigned char*)malloc(my_around_align(sizeof(struct my_allocator_recyle_item)) +
                                   my_around_align(ap->size_item));
        if (buf==NULL)
            return NULL;

        struct my_allocator_recyle_item* new_item=(struct my_allocator_recyle_item*)buf;
        new_item->next_free = NULL;
        return (void*)(buf + my_around_align(sizeof(struct my_allocator_recyle_item)));
    }

    struct my_allocator_recyle_item* ret_item = ap->first_free;
    ap->first_free = ret_item->next_free ;

    unsigned char*buf;
    buf=(unsigned char*)ret_item;
    return (void*)(buf + my_around_align(sizeof(struct my_allocator_recyle_item)));
}


void ABSTRACT_CALLBACK_UNITEX my_free_recycle_cb(void* oldptr,void* pb)
{
    struct my_allocator_recycle* ap = (struct my_allocator_recycle*)pb;
    if (oldptr==NULL)
        return;

    unsigned char*buf = (unsigned char*)oldptr;
    buf -= my_around_align(sizeof(struct my_allocator_recyle_item));

    struct my_allocator_recyle_item* recycle_item=(struct my_allocator_recyle_item*)buf;
    recycle_item->next_free = ap->first_free;
    ap->first_free = recycle_item;
}


void* ABSTRACT_CALLBACK_UNITEX my_realloc_recycle_cb(void* oldptr,size_t oldsize,size_t newsize,void*pb)
{
    struct my_allocator_recycle* ap = (struct my_allocator_recycle*)pb;
    if (oldsize >= newsize)
        return oldptr;
    if (newsize > ap->size_item)
        return NULL;

    if (oldptr==NULL)
        return my_alloc_recycle_cb(newsize,pb);
    return oldptr;
}


size_t ABSTRACT_CALLBACK_UNITEX get_size_abstract_allocator_recycle(void* pb)
{
    struct my_allocator_recycle* ap = (struct my_allocator_recycle*)pb;

    size_t ret = 0;
    if (ap != NULL)
      ret = get_allocator_recycle_size(ap);
    return ret;
}

int ABSTRACT_CALLBACK_UNITEX get_statistic_info_abstract_allocator_recycle(int /*iStatInfoItem*/,size_t*,void* )
{
    //struct my_allocator_recycle* ap = (struct my_allocator_recycle*)pb;

    int ret = 0;
    /*
    if (ap != NULL)
    {
        if (iStatInfoItem==STATISTIC_NB_TOTAL_BYTE_ALLOCATED)
        {
            *p_value = get_allocator_recycle_size(ap);
            ret=1;
        }
    }*/
    return ret;
}

int ABSTRACT_CALLBACK_UNITEX is_param_allocator_virt_recycle_compatible(const char*creator,int flagAllocator,size_t,const void* private_create_ptr,void* privateAllocatorSpacePtr)
{
    DISCARD_UNUSED_PARAMETER(creator)
    DISCARD_UNUSED_PARAMETER(private_create_ptr)
    DISCARD_UNUSED_PARAMETER(privateAllocatorSpacePtr)
    if ((flagAllocator & AllocatorTipOftenRecycledObject) != 0)
        return 8;

    return 0;
}

int ABSTRACT_CALLBACK_UNITEX create_abstract_allocator_recycle(abstract_allocator_info_public_with_allocator* aa,
                                                            const char*creator,int flagAllocator,size_t size_item,
                                                            const void* private_create_ptr,
                                                            void* privateAllocatorSpacePtr)
{
//    size_t size_atom_recycle=0;
    DISCARD_UNUSED_PARAMETER(creator)
    DISCARD_UNUSED_PARAMETER(flagAllocator)
    DISCARD_UNUSED_PARAMETER(size_item)
    DISCARD_UNUSED_PARAMETER(privateAllocatorSpacePtr)
    DISCARD_UNUSED_PARAMETER(private_create_ptr)
    
   
/*
    if (private_create_ptr != NULL)
    {
        VirtAllocatorPrivateStruct* pVirtAllocatorPrivateStruct=(VirtAllocatorPrivateStruct*)private_create_ptr;
        if (pVirtAllocatorPrivateStruct->expected_total_size != 0)
            size_atom_recycle = pVirtAllocatorPrivateStruct->expected_total_size;
    }

    if ((size_atom_recycle == 0) && (size_item!=0))
    {
        size_atom_recycle = size_item*0x20;
    }
*/
    struct my_allocator_recycle* recycle=build_allocator_recycle(size_item);

    if (recycle == NULL)
        return 0;

#ifdef VERBOSE_VIRTALLOC
    recycle->creator_given=creator;
    recycle->site_item_given=size_item;
#endif

#ifdef IS_ASTRACT_ALLOCATOR_EXTENSIBLE
	aa->size_abstract_allocator_info_size = sizeof(abstract_allocator_info_public_with_allocator);
#endif
	aa->fnc_alloc = my_alloc_recycle_cb;
    aa->fnc_free = my_free_recycle_cb;
    aa->fnc_realloc = my_realloc_recycle_cb;
    aa->fnc_get_flag_allocator = get_allocator_recycle_flag;
    aa->fnc_get_statistic_allocator_info = get_statistic_info_abstract_allocator_recycle;
	aa->fnc_clean_allocator = NULL;
    aa->abstract_allocator_ptr = recycle;
    return 1;
}


void ABSTRACT_CALLBACK_UNITEX delete_abstract_allocator_recycle(abstract_allocator_info_public_with_allocator* aa,void* privateAllocatorSpacePtr)
{
    DISCARD_UNUSED_PARAMETER(privateAllocatorSpacePtr)
    struct my_allocator_recycle* ap = (struct my_allocator_recycle*)aa->abstract_allocator_ptr;
#ifdef VERBOSE_VIRTALLOC
    size_t total_size = get_allocator_recycle_size(ap);
    char msg[0x80];
    sprintf(msg," *recycle ALLOCATOR info = for '%s', size=%u (item size %u)\n",ap->creator_given,(unsigned int)total_size,(unsigned int)ap->site_item_given);
    puts(msg);
#endif

    if (ap != NULL)
      delete_allocator_recycle(ap);
}

const t_allocator_func_array allocator_func_recycle_array =
{
    sizeof(t_allocator_func_array),
    NULL,
    NULL,

    is_param_allocator_virt_recycle_compatible,


    create_abstract_allocator_recycle,
    delete_abstract_allocator_recycle,
};



class autoinstall_var {
public:
    autoinstall_var() {
        AddAllocatorSpace(&allocator_func_recycle_array,this);
    };

    ~autoinstall_var() {
        RemoveAllocatorSpace(&allocator_func_recycle_array,this);
    };

} ;

autoinstall_var autoinstall_var_instance;


/***********************************************************************************/

struct my_allocator_growing_recyle_item
{
    struct my_allocator_growing_recyle_item* next_free;
    size_t size_item;
} ;

struct my_allocator_growing_recycle {
    struct my_allocator_growing_recyle_item* first_free;
    size_t size_item;
    
#ifdef VERBOSE_VIRTALLOC
    const char*creator_given;
    size_t site_item_given;
#endif
} ;


//#define my_around_align_growing(x)  ((((x)+0x0f)/0x10)*0x10)

size_t my_around_align_growing(size_t original_size)
{
    size_t return_size = 0x20;
    while (return_size < original_size)
        return_size *= 2;
    return my_around_align(return_size);
}

struct my_allocator_growing_recycle* build_allocator_growing_recycle(size_t first_atom)
{
    struct my_allocator_growing_recycle* ret= (struct my_allocator_growing_recycle*)malloc(sizeof(struct my_allocator_growing_recycle));
    if (ret == NULL) {
        fatal_alloc_error("build_allocator_growing_recycle");
    }
    ret -> size_item = (first_atom == 0) ? my_around_align_growing(1) : first_atom;
    ret -> first_free = NULL;
    return ret;
}

size_t get_allocator_growing_recycle_size(struct my_allocator_growing_recycle* recycle);
void delete_allocator_growing_recycle(struct my_allocator_growing_recycle* recycle)
{
#ifdef VERBOSE_VIRTALLOC
    unsigned long nb_item_at_delete = 0;
#endif
    struct my_allocator_growing_recyle_item* browse=recycle->first_free;
    while (browse!=NULL)
    {
        struct my_allocator_growing_recyle_item* next_browse = browse->next_free;
        free(browse);
        browse=next_browse;

#ifdef VERBOSE_VIRTALLOC
        nb_item_at_delete ++;
#endif
    }
    

#ifdef VERBOSE_VIRTALLOC
    {
        char sz[80]; sprintf(sz,"nb items when delete growing recycle allocator: %lu ",(unsigned long)nb_item_at_delete); puts(sz);
    }
#endif
    free(recycle);
}


size_t get_allocator_growing_recycle_size(struct my_allocator_growing_recycle* recycle)
{
    if (recycle == NULL)
        return 0;
    return recycle->size_item;
}

int ABSTRACT_CALLBACK_UNITEX get_allocator_growing_recycle_flag(void*)
{
    return AllocatorTipOftenRecycledObject;
}




void* ABSTRACT_CALLBACK_UNITEX my_alloc_growing_recycle_cb(size_t size,void* pb)
{
    struct my_allocator_growing_recycle* ap = (struct my_allocator_growing_recycle*)pb;
    if (size > ap->size_item)
    {
        ap->size_item = my_around_align_growing(size);
        // we merge too_small list and next_free
        
        if (ap->first_free != NULL)
        {
            struct my_allocator_growing_recyle_item* browse_small=ap->first_free;
            while (browse_small!=NULL)
            {
                struct my_allocator_growing_recyle_item* next_browse = browse_small->next_free;
                free(browse_small);
                browse_small=next_browse;
            }
            ap->first_free = NULL;
        }
    }
    
    if (ap->first_free == NULL)
    {
        unsigned char*buf;
        buf=(unsigned char*)malloc(my_around_align(sizeof(struct my_allocator_growing_recyle_item)) +
                                   my_around_align(ap->size_item));
        if (buf==NULL)
            return NULL;
        
        struct my_allocator_growing_recyle_item* new_item=(struct my_allocator_growing_recyle_item*)buf;
        new_item->next_free = NULL;
        new_item->size_item = ap->size_item;
        return (void*)(buf + my_around_align(sizeof(struct my_allocator_growing_recyle_item)));
    }
    
    struct my_allocator_growing_recyle_item* ret_item = ap->first_free;
    ap->first_free = ret_item->next_free ;
    
    unsigned char*buf;
    buf=(unsigned char*)ret_item;
    return (void*)(buf + my_around_align(sizeof(struct my_allocator_growing_recyle_item)));
}


void ABSTRACT_CALLBACK_UNITEX my_free_growing_recycle_cb(void* oldptr,void* pb)
{
    struct my_allocator_growing_recycle* ap = (struct my_allocator_growing_recycle*)pb;
    if (oldptr==NULL)
        return;
    
    unsigned char*buf = (unsigned char*)oldptr;
    buf -= my_around_align(sizeof(struct my_allocator_growing_recyle_item));
    
    struct my_allocator_growing_recyle_item* recycle_item=(struct my_allocator_growing_recyle_item*)buf;
    if (recycle_item->size_item < ap->size_item)
    {
      free(buf);
    }
    else
    {
        recycle_item->next_free = ap->first_free;
        ap->first_free = recycle_item;
    }
}


void* ABSTRACT_CALLBACK_UNITEX my_realloc_growing_recycle_cb(void* oldptr,size_t oldsize,size_t newsize,void*pb)
{
    //struct my_allocator_growing_recycle* ap = (struct my_allocator_growing_recycle*)pb;
    if (oldsize >= newsize)
        return oldptr;
    
    if (oldptr==NULL)
        return my_alloc_growing_recycle_cb(newsize,pb);
    return oldptr;
}


size_t ABSTRACT_CALLBACK_UNITEX get_size_abstract_allocator_growing_recycle(void* pb)
{
    struct my_allocator_growing_recycle* ap = (struct my_allocator_growing_recycle*)pb;
    
    size_t ret = 0;
    if (ap != NULL)
        ret = get_allocator_growing_recycle_size(ap);
    return ret;
}

int ABSTRACT_CALLBACK_UNITEX get_statistic_info_abstract_allocator_growing_recycle(int /*iStatInfoItem*/,size_t*,void* )
{
    //struct my_allocator_growing_recycle* ap = (struct my_allocator_growing_recycle*)pb;
    
    int ret = 0;
    /*
     if (ap != NULL)
     {
     if (iStatInfoItem==STATISTIC_NB_TOTAL_BYTE_ALLOCATED)
     {
     *p_value = get_allocator_growing_recycle_size(ap);
     ret=1;
     }
     }*/
    return ret;
}

int ABSTRACT_CALLBACK_UNITEX is_param_allocator_virt_growing_recycle_compatible(const char*creator,int flagAllocator,size_t,const void* private_create_ptr,void* privateAllocatorSpacePtr)
{
    DISCARD_UNUSED_PARAMETER(creator)
    DISCARD_UNUSED_PARAMETER(private_create_ptr)
    DISCARD_UNUSED_PARAMETER(privateAllocatorSpacePtr)
    if ((flagAllocator & AllocatorTipOftenRecycledObject) != 0)
        return 0;
    
#ifdef AllocatorTipGrowingOftenRecycledObject
    if ((flagAllocator & AllocatorTipGrowingOftenRecycledObject) != 0)
    {
        return 2;
    }
#endif

    return 0;
}

int ABSTRACT_CALLBACK_UNITEX create_abstract_allocator_growing_recycle(abstract_allocator_info_public_with_allocator* aa,
                                                                       const char*creator,int flagAllocator,size_t size_item,
                                                                       const void* private_create_ptr,
                                                                       void* privateAllocatorSpacePtr)
{
    //    size_t size_atom_growing_recycle=0;
    DISCARD_UNUSED_PARAMETER(creator)
    DISCARD_UNUSED_PARAMETER(flagAllocator)
    DISCARD_UNUSED_PARAMETER(size_item)
    DISCARD_UNUSED_PARAMETER(privateAllocatorSpacePtr)
    DISCARD_UNUSED_PARAMETER(private_create_ptr)
    
    
    // test code
    /*
    if (size_item== 0)
    {
        puts("big size");
        size_item=2048;
    }
     */
    // end test code
    /*
     if (private_create_ptr != NULL)
     {
     VirtAllocatorPrivateStruct* pVirtAllocatorPrivateStruct=(VirtAllocatorPrivateStruct*)private_create_ptr;
     if (pVirtAllocatorPrivateStruct->expected_total_size != 0)
     size_atom_growing_recycle = pVirtAllocatorPrivateStruct->expected_total_size;
     }
     
     if ((size_atom_growing_recycle == 0) && (size_item!=0))
     {
     size_atom_growing_recycle = size_item*0x20;
     }
     */
    struct my_allocator_growing_recycle* recycle=build_allocator_growing_recycle(size_item);
    
    if (recycle == NULL)
        return 0;
    
#ifdef VERBOSE_VIRTALLOC
    recycle->creator_given=creator;
    recycle->site_item_given=size_item;
#endif
    
#ifdef IS_ASTRACT_ALLOCATOR_EXTENSIBLE
	aa->size_abstract_allocator_info_size = sizeof(abstract_allocator_info_public_with_allocator);
#endif
	aa->fnc_alloc = my_alloc_growing_recycle_cb;
    aa->fnc_free = my_free_growing_recycle_cb;
    aa->fnc_realloc = my_realloc_growing_recycle_cb;
    aa->fnc_get_flag_allocator = get_allocator_growing_recycle_flag;
    aa->fnc_get_statistic_allocator_info = get_statistic_info_abstract_allocator_growing_recycle;
	aa->fnc_clean_allocator = NULL;
    aa->abstract_allocator_ptr = recycle;
    return 1;
}


void ABSTRACT_CALLBACK_UNITEX delete_abstract_allocator_growing_recycle(abstract_allocator_info_public_with_allocator* aa,void* privateAllocatorSpacePtr)
{
    DISCARD_UNUSED_PARAMETER(privateAllocatorSpacePtr)
    struct my_allocator_growing_recycle* ap = (struct my_allocator_growing_recycle*)aa->abstract_allocator_ptr;
#ifdef VERBOSE_VIRTALLOC
    size_t total_size = get_allocator_growing_recycle_size(ap);
    char msg[0x80];
    sprintf(msg," *recycle ALLOCATOR info = for '%s', size=%u (item size %u)\n",ap->creator_given,(unsigned int)total_size,(unsigned int)ap->site_item_given);
    puts(msg);
#endif
    
    if (ap != NULL)
        delete_allocator_growing_recycle(ap);
}

const t_allocator_func_array allocator_func_growing_recycle_array =
{
    sizeof(t_allocator_func_array),
    NULL,
    NULL,
    
    is_param_allocator_virt_growing_recycle_compatible,
    
    
    create_abstract_allocator_growing_recycle,
    delete_abstract_allocator_growing_recycle,
};



class autoinstall_gr_var {
public:
    autoinstall_gr_var() {
        AddAllocatorSpace(&allocator_func_growing_recycle_array,this);
    };
    
    ~autoinstall_gr_var() {
        RemoveAllocatorSpace(&allocator_func_growing_recycle_array,this);
    };
    
} ;

autoinstall_gr_var autoinstall_gr_var_instance;

