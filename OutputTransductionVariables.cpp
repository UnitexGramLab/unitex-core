/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

// if you don't have stdint.h, define TYPE_PACK_MULTIBITS with type unsigned int 64 bits
// if you don't have unsigned int 64 bits, define TYPE_PACK_MULTIBITS_STRUCT

#ifdef TYPE_PACK_MULTIBITS_STRUCT
typedef struct
{
    unsigned char c[8];
} pack_8_bytes;
typedef pack_8_bytes uint_pack_multibits;
typedef pack_8_bytes uint_pack_multiunichar;

static inline uint_pack_multibits get_uint_pack_multibits_zero()
{
    uint_pack_multibits p;
    p.c[0] = p.c[1] = p.c[2] = p.c[3] = p.c[4] = p.c[5] = p.c[6] = p.c[7] = 0;
    return p;
}

#else
#ifdef TYPE_PACK_MULTIBITS
typedef TYPE_PACK_MULTIBITS uint_pack_multibits;
typedef TYPE_PACK_MULTIBITS uint_pack_multiunichar;
#else
#include <stdint.h>
typedef uint64_t uint_pack_multibits;
typedef uint64_t uint_pack_multiunichar;
#endif

static inline uint_pack_multibits get_uint_pack_multibits_zero()
{
    return (uint_pack_multibits)0;
}
#endif

#define ALIGN_BACKUP_STRING sizeof(uint_pack_multibits)

#include "OutputTransductionVariables.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {



/*
// unoptimized version
static inline void copy_string(unichar* dest, const unichar* src, unsigned int len) {
#ifdef COPY_STRING_USE_MEMCPY
    memcpy(dest, src, (len + 1) * sizeof(unichar));
#else
    #define factor_unichar_to_pack (sizeof(uint_pack_multiunichar)/sizeof(unichar))

    unsigned int size_in_byte = (len + 1)*sizeof(unichar);
    unsigned int len_in_pack_multiunichar = (size_in_byte + sizeof(uint_pack_multiunichar) - 1) / sizeof(uint_pack_multiunichar);

    for (unsigned int i = 0; i < len_in_pack_multiunichar; i++)
    {
        uint_pack_multiunichar mc = *(((uint_pack_multiunichar*)src) + i);
        *(((uint_pack_multiunichar*)dest) + i) = mc;
    }
#endif
}
*/

static inline void copy_string(unichar* dest, const unichar* src, unsigned int len) {
#ifdef COPY_STRING_USE_MEMCPY
        memcpy(dest, src, (len + 1) * sizeof(unichar));
#else
#define factor_unichar_to_pack (sizeof(uint_pack_multiunichar)/sizeof(unichar))

        unsigned int size_in_byte = (len + 1)*sizeof(unichar);
        unsigned int len_in_pack_multiunichar = (size_in_byte + sizeof(uint_pack_multiunichar) - 1) / sizeof(uint_pack_multiunichar);

        switch (len_in_pack_multiunichar)
        {
        case 0:
            break;
        case 1:
            *(((uint_pack_multiunichar*)dest) + 0) = *(((uint_pack_multiunichar*)src) + 0);
            break;

        case 2:
            *(((uint_pack_multiunichar*)dest) + 0) = *(((uint_pack_multiunichar*)src) + 0);
            *(((uint_pack_multiunichar*)dest) + 1) = *(((uint_pack_multiunichar*)src) + 1);
            break;

        case 3:
            *(((uint_pack_multiunichar*)dest) + 0) = *(((uint_pack_multiunichar*)src) + 0);
            *(((uint_pack_multiunichar*)dest) + 1) = *(((uint_pack_multiunichar*)src) + 1);
            *(((uint_pack_multiunichar*)dest) + 2) = *(((uint_pack_multiunichar*)src) + 2);
            break;

        case 4:
            *(((uint_pack_multiunichar*)dest) + 0) = *(((uint_pack_multiunichar*)src) + 0);
            *(((uint_pack_multiunichar*)dest) + 1) = *(((uint_pack_multiunichar*)src) + 1);
            *(((uint_pack_multiunichar*)dest) + 2) = *(((uint_pack_multiunichar*)src) + 2);
            *(((uint_pack_multiunichar*)dest) + 3) = *(((uint_pack_multiunichar*)src) + 3);
            break;

        case 5:
            *(((uint_pack_multiunichar*)dest) + 0) = *(((uint_pack_multiunichar*)src) + 0);
            *(((uint_pack_multiunichar*)dest) + 1) = *(((uint_pack_multiunichar*)src) + 1);
            *(((uint_pack_multiunichar*)dest) + 2) = *(((uint_pack_multiunichar*)src) + 2);
            *(((uint_pack_multiunichar*)dest) + 3) = *(((uint_pack_multiunichar*)src) + 3);
            *(((uint_pack_multiunichar*)dest) + 4) = *(((uint_pack_multiunichar*)src) + 4);
            break;

        case 6:
            *(((uint_pack_multiunichar*)dest) + 0) = *(((uint_pack_multiunichar*)src) + 0);
            *(((uint_pack_multiunichar*)dest) + 1) = *(((uint_pack_multiunichar*)src) + 1);
            *(((uint_pack_multiunichar*)dest) + 2) = *(((uint_pack_multiunichar*)src) + 2);
            *(((uint_pack_multiunichar*)dest) + 3) = *(((uint_pack_multiunichar*)src) + 3);
            *(((uint_pack_multiunichar*)dest) + 4) = *(((uint_pack_multiunichar*)src) + 4);
            *(((uint_pack_multiunichar*)dest) + 5) = *(((uint_pack_multiunichar*)src) + 5);
            break;

        case 7:
            *(((uint_pack_multiunichar*)dest) + 0) = *(((uint_pack_multiunichar*)src) + 0);
            *(((uint_pack_multiunichar*)dest) + 1) = *(((uint_pack_multiunichar*)src) + 1);
            *(((uint_pack_multiunichar*)dest) + 2) = *(((uint_pack_multiunichar*)src) + 2);
            *(((uint_pack_multiunichar*)dest) + 3) = *(((uint_pack_multiunichar*)src) + 3);
            *(((uint_pack_multiunichar*)dest) + 4) = *(((uint_pack_multiunichar*)src) + 4);
            *(((uint_pack_multiunichar*)dest) + 5) = *(((uint_pack_multiunichar*)src) + 5);
            *(((uint_pack_multiunichar*)dest) + 6) = *(((uint_pack_multiunichar*)src) + 6);
            break;

        default:
            for (unsigned int i = 0; i < len_in_pack_multiunichar; i++)
            {
                uint_pack_multiunichar mc = *(((uint_pack_multiunichar*)src) + i);
                *(((uint_pack_multiunichar*)dest) + i) = mc;
            }
        }
#endif
}


static inline void add_output_variable_to_pending_list(OutputVariables*v, OutputVarList* *list,Ustring* s);
//static inline void add_output_variable_to_pending_list_no_alloc_needed(OutputVariables*v, OutputVarList* *list, Ustring* s);
static inline void remove_output_variable_from_pending_list(OutputVariables*v, OutputVarList* *list,Ustring* s);



#define my_around_align_ispending_array(x)  ((((x)+0x0f)/0x10)*0x10)
#define my_around_align(x,sz)  ((((x)+((sz)-1))/(sz))*(sz))
#define my_around_align_intptr_size_ispending_array(x)  ((((x)+sizeof(uint_pack_multibits)-1)/sizeof(uint_pack_multibits))*sizeof(uint_pack_multibits))

#define STEP_UNROLL_FREE_VARIABLE 8

/**
 * Allocates and returns a structure representing the variables
 * whose names are in 'list'. The variable values are initialized
 * with empty strings.
 */
OutputVariables* new_OutputVariables(struct list_ustring* list,int* p_nbvar,vector_ptr* injected) {
struct string_hash* variable_index = new_string_hash(DONT_USE_VALUES);
while (list != NULL) {
    get_value_index(list->string, variable_index);
    list = list->next;
}
if (injected != NULL) {
    for (int i = 0;i<injected->nbelems;i += 2) {
        get_value_index((unichar*)(injected->tab[i]), variable_index);
    }
}
unsigned int nb_var = variable_index->size;
OutputVariables* v=(OutputVariables*)malloc(sizeof(OutputVariables)+((nb_var + STEP_UNROLL_FREE_VARIABLE)*sizeof(Ustring)));
if (v==NULL) {
   fatal_alloc_error("new_OutputVariables");
}
v->variable_index=variable_index;

v->nb_var = (size_t)nb_var;
//v->variables_=(Ustring**)malloc(nb_var*sizeof(Ustring*));
/*
for (unsigned int i=0;i<nb_var;i++) {
   v->variables_[i]=new_Ustring();
}*/
#define START_SIZE_VARIABLE 1
for (unsigned int i = 0; i < my_around_align(nb_var, STEP_UNROLL_FREE_VARIABLE); i++) {
    v->variables_[i].len=0;
    v->variables_[i].size = START_SIZE_VARIABLE;
    v->variables_[i].str = (unichar*)malloc(v->variables_[i].size*sizeof(unichar));
    if (v->variables_[i].str == NULL) {
        fatal_alloc_error("new_OutputVariables");
    }
    *(v->variables_[i].str)=0;
    resize(&v->variables_[i], 1);
}
v->pending=NULL;
v->is_pending=(char*)calloc(my_around_align_intptr_size_ispending_array(my_around_align_ispending_array(nb_var*sizeof(char))),1);
v->is_pending_array_size_intptr_size_rounded = my_around_align_intptr_size_ispending_array(my_around_align_ispending_array(nb_var*sizeof(char))) / sizeof(uint_pack_multibits);
// size for backup
size_t size_header = 2 * sizeof(int);
size_t pending_list_offset = size_header;
size_t size_pending_list = (nb_var + 1)*sizeof(int);
size_t size_string_index = ((nb_var + 1) * 2) * sizeof(int);
v->string_index_offset = pending_list_offset + size_pending_list;
v->unichars_offset = my_around_align(v->string_index_offset + size_string_index,0x10);

if (v->is_pending==NULL) {
   fatal_alloc_error("new_OutputVariables");
}
if (p_nbvar!=NULL) {
    *p_nbvar = nb_var;
}
if (injected!=NULL) {
    for (int i=0;i<injected->nbelems;i+=2) {
        int index=get_value_index((unichar*)(injected->tab[i]),v->variable_index);
        u_strcpy(&v->variables_[index],(unichar*)(injected->tab[i+1]));
    }
}

v->recycle_allocation = NULL;
return v;
}


/**
 * alloc_OutputVarList_from_recycle_reserve replace malloc(sizeof(OutputVarList))
 * by maintain a collection of already allocated structure to do less malloc/free
 */
static inline OutputVarList* alloc_OutputVarList_from_recycle_reserve(OutputVariables*v)
{
    OutputVarList* item = v->recycle_allocation;
    if (item != NULL) {
        v->recycle_allocation = item->next;
    } else {
        item = (OutputVarList*)malloc(sizeof(OutputVarList));
        if (item == NULL) {
            fatal_alloc_error("alloc_OutputVarList_from_recycle_reserve");
        }
    }

    return item;
}


/**
 * same function than alloc_OutputVarList_from_recycle_reserve, but used when restore a previous existing list
 *  we are sure alloc_OutputVarList_from_recycle_reserve will no do malloc
 */
/*
static inline OutputVarList* alloc_OutputVarList_from_recycle_reserve_no_alloc_needed(OutputVariables*v)
{
    OutputVarList* item = v->recycle_allocation;
    v->recycle_allocation = item->next;
    return item;
}
*/


/**
 * free an unused OutputVarList and add it on the recycling collection list
 */
static inline void free_OutputVarList_from_recycle_reserve(OutputVariables*v, OutputVarList* item)
{
    item->next = v->recycle_allocation;
    v->recycle_allocation = item;
}


/**
 * swap the string content of one variable
 * uses this function instead direct switching on another source file
 * the string in parameter will receive previous content (as backup)
 */
void swap_output_variable_content(OutputVariables*v, int index, Ustring* swap_string)
{
    Ustring t;
    if ((v->variables_[index].size) > (swap_string->size)) {
        resize(swap_string, v->variables_[index].size);
    }

    t = *swap_string;
    *swap_string = (v->variables_[index]);
    (v->variables_[index]) = t;
}


/**
 * Frees the memory associated to the given variables.
 */
void free_OutputVariables(OutputVariables* v) {
if (v==NULL) return;
int nb_var=v->variable_index->size;
free_string_hash(v->variable_index);
for (unsigned int i = 0; i < my_around_align((unsigned int)nb_var, STEP_UNROLL_FREE_VARIABLE); i++) {
    free(v->variables_[i].str);
}
OutputVarList* l=v->pending;
OutputVarList* tmp;
while (l!=NULL) {
    tmp=l;
    l=l->next;
    /* Don't free l->var, since it has been freed by free_Ustring above */
    free_OutputVarList_from_recycle_reserve(v,tmp);
}
// freeing the recycle collection of OutputVarList allocated structure
OutputVarList* tmp_recyling = v->recycle_allocation;
while (tmp_recyling != NULL) {
    OutputVarList* tmp_recyling_next = tmp_recyling->next;
    free(tmp_recyling);
    tmp_recyling = tmp_recyling_next;
}
free(v->is_pending);
free(v);
}


/**
 * Returns a pointer on the Ustring associated the variable whose name is 'name',
 * or NULL if the variable in not in the given variable set.
 */
const Ustring* get_output_variable(OutputVariables* v,const unichar* name) {
int n=get_value_index(name,v->variable_index,DONT_INSERT);
if (n==-1) {
   return NULL;
}
return &v->variables_[n];
}


#define OFFSET_NB_PENDING 0
#define OFFSET_NB_FILLED_STRING (sizeof(int))
#define OFFSET_PENDING_LIST (sizeof(int)*2)
/**
 * Allocates, initializes and returns a unichar array that is a copy of
 * the variable values. The array starts with a subarray of size 'n'
 * (n=number of variables) with for each variable 1 if it is pending and 0 otherwise.
 */
OutputVariablesBackup* create_output_variable_backup(OutputVariables* RESTRICT v,Abstract_allocator prv_alloc) {
if (v==NULL || v->variable_index==NULL) return NULL;
unsigned int nb_var=(unsigned int)v->nb_var;

if (nb_var==0) return NULL;
unsigned int size_strings=0;

for (unsigned int i=0;i<nb_var;i++) {
    unsigned int len = v->variables_[i].len;
    size_strings += len;
}
OutputVariablesBackup* backup=(OutputVariablesBackup*)malloc_cb(
    v->unichars_offset + (size_strings * sizeof(unichar)) +
        ((nb_var+1) * ( sizeof(uint_pack_multibits)+sizeof(unichar))),prv_alloc);
if (backup==NULL) {
   fatal_alloc_error("create_output_variable_backup");
}

unsigned int nb_pending = 0;
unsigned int * pending_var_list = (unsigned int *)(((char*)backup) + OFFSET_PENDING_LIST);

for (unsigned int i = 0; i<(unsigned int)nb_var; i++) {
    *(pending_var_list + nb_pending) = i;
    nb_pending+= v->is_pending[i];
}
*(pending_var_list + nb_pending) = nb_var;

*((unsigned int*)(((char*)backup) + OFFSET_NB_PENDING)) = nb_pending;

unsigned int* string_index = (unsigned int*)(((char*)backup) + (v->string_index_offset));

if ((!nb_pending) && (!size_strings)) {
    *string_index = nb_var;
    *((unsigned int*)(((char*)backup) + OFFSET_NB_FILLED_STRING)) = 0;
    return (OutputVariablesBackup*)backup;
}

unichar* backup_string = (unichar*)(((char*)backup) + v->unichars_offset);
unsigned int nb_string_filled = 0;
for (unsigned int i=0;i<(unsigned int)nb_var;i++) {
    {
        unsigned int len = v->variables_[i].len;
        if (len > 0)
        {
            *(string_index + (nb_string_filled * 2)) = i;
            *(string_index + (nb_string_filled * 2) + 1) = len;
            //memcpy(backup_string, v->variables[i]->str, sizeof(unichar)*(len + 1));
            copy_string(backup_string, v->variables_[i].str, len);
            backup_string += my_around_align(len + 1, sizeof(uint_pack_multibits));
            nb_string_filled++;
        }
    }
}

*(string_index + (nb_string_filled * 2)) = nb_var;

*((unsigned int*)(((char*)backup) + OFFSET_NB_FILLED_STRING)) = nb_string_filled;

return (OutputVariablesBackup*)backup;
}


/**
 * Frees the given variable backup.
 */
void free_output_variable_backup(OutputVariablesBackup* backup,Abstract_allocator prv_alloc) {
if (backup!=NULL) free_cb(backup,prv_alloc);
}


/**
 * Sets the variables with the values of the given backup.
 */
void install_output_variable_backup(OutputVariables* RESTRICT v,const OutputVariablesBackup* RESTRICT backup) {
if (backup==NULL) return;

/* First, we free the previous pending list */
OutputVarList* tmp;
while (v->pending!=NULL) {
    tmp=v->pending;
    v->pending=v->pending->next;
    /* We must not free tmp->var */
    free_OutputVarList_from_recycle_reserve(v,tmp);
}

uint_pack_multibits* is_pending_erase = (uint_pack_multibits*)v->is_pending;


const unsigned int * pending_var_list = (const unsigned int*)(((const char*)backup) + OFFSET_PENDING_LIST);
const unsigned int nb_pending = *((const unsigned int*)(((const char*)backup) + OFFSET_NB_PENDING)) ;
const unsigned int nb_filled_strings = *((const unsigned int*)(((const char*)backup) + OFFSET_NB_FILLED_STRING)) ;
unsigned int nb_var = (unsigned int)v->nb_var;


for (unsigned int i = 0; i < nb_var; i+=STEP_UNROLL_FREE_VARIABLE) {
    v->variables_[i].str[0] = 0;
    v->variables_[i].len = 0;
    v->variables_[i+1].str[0] = 0;
    v->variables_[i+1].len = 0;
    v->variables_[i+2].str[0] = 0;
    v->variables_[i+2].len = 0;
    v->variables_[i+3].str[0] = 0;
    v->variables_[i+3].len = 0;
    v->variables_[i+4].str[0] = 0;
    v->variables_[i+4].len = 0;
    v->variables_[i+5].str[0] = 0;
    v->variables_[i+5].len = 0;
    v->variables_[i+6].str[0] = 0;
    v->variables_[i+6].len = 0;
    v->variables_[i+7].str[0] = 0;
    v->variables_[i+7].len = 0;
    *(is_pending_erase + (i/8)) = get_uint_pack_multibits_zero();
}

// first int of backup is set to 0 if we have full empty backup
if ((!nb_pending) && (!nb_filled_strings)) {
    return;
}

for (unsigned int loop_pending = 0; loop_pending < nb_pending; loop_pending++)
{
    unsigned int cur_pending_item = *(pending_var_list + loop_pending);
    v->is_pending[cur_pending_item] = 1;
    add_output_variable_to_pending_list(v,&(v->pending),&v->variables_[cur_pending_item]);
}


const unsigned int* string_index = (const unsigned int*)(((const char*)backup) + (v->string_index_offset));
const unichar* backup_string = (const unichar*)(((const char*)backup) + v->unichars_offset);
int pos_in_index = 0;

for (;;) {
    unsigned int cur_item_in_index = *(string_index + pos_in_index);

    if (cur_item_in_index == nb_var)
        break;

    Ustring * cur_ustr = &(v->variables_[cur_item_in_index]);

    int cur_len_in_index = *(string_index + pos_in_index + 1);

    /*
    // this test is awful, because it break optimized IPO/LTO/WPO optimization having only internal code
    //  we can remove it, because we restore a previous value (so buffer size is already compatible), and
    //  when we replace with replace_output_variable_string, size became never smaller
    if (cur_ustr->size < (unsigned int)cur_len_in_index)
        resize(cur_ustr, cur_len_in_index + 1);
        */
    cur_ustr->len = cur_len_in_index;
    //memcpy(cur_ustr->str, backup_string, (cur_len_in_index + 1) * sizeof(unichar));
    copy_string(cur_ustr->str, backup_string, cur_len_in_index);
    //backup_string += cur_len_in_index + 1;
    backup_string += my_around_align(cur_len_in_index + 1, ALIGN_BACKUP_STRING);
    pos_in_index+=2;
}
}


/**
 * Returns 1 if the given backup correspond to the same values than the given
 * output variables; 0 otherwise.
 * This function is far less optimized than create_output_variable_backup and
 * install_output_variable_backup because less used.
 */
int same_output_variables(const OutputVariablesBackup* backup, OutputVariables* v) {
    if (v == NULL) {
        return backup == NULL;
    }

    unsigned int nb_var = (unsigned int)v->nb_var;

    if (nb_var == 0) return 1;


    const unsigned int * pending_var_list_backup = (const unsigned int*)(((const char*)backup) + OFFSET_PENDING_LIST);
    const unsigned int nb_pending_backup = *((const unsigned int*)(((const char*)backup) + OFFSET_NB_PENDING));
    const unsigned int nb_filled_strings_backup = *((const unsigned int*)(((const char*)backup) + OFFSET_NB_FILLED_STRING));



    // first int of backup is set to 0 if we have full empty backup
    if ((!nb_pending_backup) && (!nb_filled_strings_backup)) {
        for (unsigned int i = 0;i<nb_var;i++) {
            if (0 != v->is_pending[i]) {
                return 0;
            }
        }

        for (unsigned int i = 0;i<nb_var;i++) {
            if (v->variables_[i].str[0] != '\0') {
                return 0;
            }
        }

        return 1;
    }

    unsigned int nb_pending_v = 0;
    for (size_t i = 0; i<nb_var; i++) {
        if (v->is_pending[i]) {
            if (*(pending_var_list_backup + nb_pending_v) != i) {
                return 0;
            }
            nb_pending_v++;
        }
    }

    if (*(pending_var_list_backup + nb_pending_v) != nb_var) {
        return 0;
    }

    const unsigned int* string_index = (const unsigned int*)(((const char*)backup) + (v->string_index_offset));
    const unichar* backup_string = (const unichar*)(((const char*)backup) + v->unichars_offset);
    unsigned int pos_in_index = 0;
    unsigned int cur_item_in_index = *(string_index + (pos_in_index * 2));
    for (unsigned int i = 0; i<nb_var; i++) {
        Ustring * cur_ustr = &(v->variables_[i]);
        if (i == cur_item_in_index)
        {
            unsigned int cur_len_in_index = *(string_index + (pos_in_index * 2) + 1);
            if (cur_ustr->len != cur_len_in_index) {
                return 0;
            }
            if (memcmp(cur_ustr->str, backup_string, (cur_len_in_index + 1) * sizeof(unichar)) != 0) {
                return 0;
            }
            //backup_string += cur_len_in_index + 1;
            backup_string += my_around_align(cur_len_in_index + 1, ALIGN_BACKUP_STRING);

            pos_in_index++;
            cur_item_in_index = *(string_index + (pos_in_index * 2));
        }
        else
        {
            if (cur_ustr->len != 0) {
                return 0;
            }
        }
    }

    return 1;
}


/**
 * Adds the given Ustring pointer to the list, with no duplicates.
 */
static inline void add_output_variable_to_pending_list(OutputVariables*v, OutputVarList* *list,Ustring* s) {
while (*list != NULL) {
    if ((*list)->var==s) {
        /* The pointer is already in the list */
        return;
    }
    list=&((*list)->next);
}
(*list)=(OutputVarList*)alloc_OutputVarList_from_recycle_reserve(v);
(*list)->var=s;
(*list)->next=NULL;
}


/**
 * Adds the given Ustring pointer to the list, with no duplicates.
 * same function than add_output_variable_to_pending_list, but as we restore a previous existing list
 * we are sure alloc_OutputVarList_from_recycle_reserve will no do malloc
 */
// currently unused
/*
static inline void add_output_variable_to_pending_list_no_alloc_needed(OutputVariables*v, OutputVarList* *list,Ustring* s) {
while (*list != NULL) {
    if ((*list)->var==s) {
        // The pointer is already in the list
        return;
    }
    list=&((*list)->next);
}
(*list)=(OutputVarList*)alloc_OutputVarList_from_recycle_reserve_no_alloc_needed(v);
(*list)->var=s;
(*list)->next=NULL;
}
*/


/**
 * Removes the given Ustring pointer from the list.
 */
static inline void remove_output_variable_from_pending_list(OutputVariables*v, OutputVarList* *list,Ustring* s) {
while (*list != NULL) {
    if ((*list)->var==s) {
        /* We have found the pointer to be removed */
        OutputVarList* tmp=(*list);
        (*list)=(*list)->next;
        free_OutputVarList_from_recycle_reserve(v,tmp);
        return;
    }
    list=&((*list)->next);
}
fatal_error("Non-existent Ustring pointer in remove_output_variable\n");
}


/**
 * Concatenates the given string to all strings of the given list.
 * Returns the length of 's'.
 */
unsigned int add_raw_string_to_output_variables(OutputVariables* var,unichar* s) {
OutputVarList* list=var->pending;
if (list==NULL) {
    fatal_error("Should not invoke add_string_to_output_variables on an empty list\n");
}
if (s==NULL || s[0]=='\0') return 0;
unsigned int old=list->var->len;
OutputVarList* first=list;
while (list!=NULL) {
    u_strcat(list->var,s);
    list=list->next;
}
return first->var->len-old;
}


/**
 * Removes 'n' chars from all strings of the given list. If 'n'
 * is greater than the length of a string, the string is emptied.
 */
void remove_chars_from_output_variables(OutputVariables* var,unsigned int n) {
OutputVarList* list=var->pending;
if (n==0) return;
if (list==NULL) {
    fatal_error("Should not invoke remove_chars_from_output_variables on an empty list\n");
}
while (list!=NULL) {
    remove_n_chars(list->var,n);
    list=list->next;
}
}


/**
 * Sets the variable #index as being pending.
 */
void set_output_variable_pending(OutputVariables* var,int index) {
var->is_pending[index]=1;
add_output_variable_to_pending_list(var,&(var->pending),&(var->variables_[index]));
}


/**
 * Unsets the variable #index as being pending.
 */
void unset_output_variable_pending(OutputVariables* var,int index) {
var->is_pending[index]=0;
remove_output_variable_from_pending_list(var,&(var->pending),&(var->variables_[index]));
}


/**
 * Sets the given variable as being pending.
 */
void set_output_variable_pending(OutputVariables* var,const unichar* var_name) {
set_output_variable_pending(var,get_value_index(var_name,var->variable_index,DONT_INSERT));
}


/**
 * Unsets the given variable as being pending.
 */
void unset_output_variable_pending(OutputVariables* var,const unichar* var_name) {
unset_output_variable_pending(var,get_value_index(var_name,var->variable_index,DONT_INSERT));
}


} // namespace unitex
