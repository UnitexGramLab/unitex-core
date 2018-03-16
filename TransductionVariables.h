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

#ifndef TransductionVariablesH
#define TransductionVariablesH

#include "Unicode.h"
#include "Fst2.h"
#include "String_hash.h"
#include "List_ustring.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/* Value to use to indicate that a variable bound is undefined */
#define UNDEF_VAR_BOUND -666666

/* Maximum length in chars of a transduction variable or field */
#define MAX_TRANSDUCTION_VAR_LENGTH 127
#define MAX_TRANSDUCTION_FIELD_LENGTH 127

#define NB_INT_BY_VARIABLES (4)

/**
 * This structure is used to associates ranges to variable names.
 */
typedef struct {
   /* For a given variable A, this array gives an index a to be used in
    * the 'variables' array */
   struct string_hash* variable_index;
   /* variables[a] gives the range associated to the variable #a */
   struct transduction_variable* variables;
} InputVariables;



/**
 * This structure defines the range of a variable in the text tokens.
 *
 * Note: when used from LocateTfst, start_in_tokens and end_in_tokens represent the
 *       TfstTag indices of the first and last text dependent tags of
 *       a match. Moreover, 'end_in_tokens' must be taken into account, at the opposite
 *       of the Locate use of this structure.
 */
struct transduction_variable {
   /* Position of the first token of the sequence */
   int start_in_tokens;
   /* Position after the last token of the sequence, so that the sequence is in the
    * range [start_in_tokens;end_in_tokens[ */
   int end_in_tokens;
   /* Starting position in chars in the first token of the sequence */
   int start_in_chars;
   /* Ending position in chars in the last token of the sequence. -1 means
    * that the whole token must be taken into account */
   int end_in_chars;
};



InputVariables* new_Variables(const struct list_ustring*,int* p_nb_variable=NULL);
void free_Variables(InputVariables*);
struct transduction_variable* get_transduction_variable(InputVariables*,const unichar*);
struct transduction_variable* get_transduction_variable(InputVariables* v, const unichar* name, int* value_index);

/*
 * the function below are replaced by macro for performance
 */
/*
void set_variable_start(Variables*,int,int);
void set_variable_start_in_chars(Variables*,int,int);
void set_variable_end(Variables*,int,int);
void set_variable_end_in_chars(Variables*,int,int);

int get_variable_start(const Variables*,int);
int get_variable_start_in_chars(const Variables*,int);
int get_variable_end(const Variables*,int);
int get_variable_end_in_chars(const Variables*,int);
*/

#define set_variable_start(v,n,value) ((v)->variables[(n)].start_in_tokens=(value))
#define set_variable_start_in_chars(v,n,value) ((v)->variables[(n)].start_in_chars=(value))
#define set_variable_end(v,n,value) ((v)->variables[(n)].end_in_tokens=(value))
#define set_variable_end_in_chars(v,n,value) ((v)->variables[(n)].end_in_chars=(value))

#define get_variable_start(v,n) ((v)->variables[(n)].start_in_tokens)
#define get_variable_start_in_chars(v,n) ((v)->variables[(n)].start_in_chars)
#define get_variable_end(v,n) ((v)->variables[(n)].end_in_tokens)
#define get_variable_end_in_chars(v,n) ((v)->variables[(n)].end_in_chars)

size_t get_expected_variable_backup_size_in_byte_for_nb_variable(int nb);
size_t get_variable_backup_size_in_byte(const InputVariables* v);
void init_variable_backup(int* backup,const InputVariables* v);
int* create_variable_backup(const InputVariables*,Abstract_allocator);
void free_variable_backup(int*,Abstract_allocator);

size_t get_expected_variable_backup_size_in_byte_for_nb_variable(int nb);

void install_variable_backup(InputVariables*,const int*);
void update_variable_backup(int*,const InputVariables*);
void reset_Variables(InputVariables* v);

int same_input_variables(int* input_variable_backup,InputVariables* v);

/* to limit number of malloc, we define a pool of memory
   we use this thing : free will be done in reverse order than alloc

   we want help the compiler to understand we want array_int[0] aligned on
   16 bytes boundaries. This is very important for memcpy size.
   We don't care lost somes bytes : there will a very small number of
   variable_backup_memory_reserve structure allocated
   */


typedef struct {
    int size_variable_index;
    int pos_used;
    int size_aligned;
    int size_copydata;
    int nb_backup_possible_array;
    int dummy_align1,dummy_align2,dummy_align3;

    int array_int[4];
} variable_backup_memory_reserve;

/*
 * create_variable_backup_memory_reserve : build a reserve of memory
 * with space for nb_item_allocated int
 */
variable_backup_memory_reserve* create_variable_backup_memory_reserve(const InputVariables*,int is_first);

/*
 * clear the reserve from memory
 */
void free_reserve(variable_backup_memory_reserve*r);





#define OFFSET_COUNTER (0)
#define OFFSET_DIRTY (1)
#define OFFSET_SWAPPER (2)
#define OFFSET_BACKUP (4)

/*
 * check if the reserve contain space and is correct to save variable v
 */
static inline int is_enough_memory_in_reserve_for_transduction_variable_set(const InputVariables* v,const variable_backup_memory_reserve* r)
{
    return (((r->pos_used+1) < r->nb_backup_possible_array) && (v->variable_index->size == r->size_variable_index));
}



/*
 * create the backup, taking memory from reserve
 * we assume is_enough_memory_in_reserve_for_Variable was already called to verify
 */
static inline int* create_variable_backup_using_reserve(const InputVariables* v,variable_backup_memory_reserve* r) {

/* DIRTY==0 mean :
   - there is a least one backup
   - there is no modification made on the variable array since last backup
   - the last used array is not a "swapper"
    so we can reuse the backup previously made !
 */
if (r->array_int[OFFSET_DIRTY+(r->pos_used * r->size_aligned)] == 0)
{
    r->array_int[OFFSET_COUNTER+(r->pos_used * r->size_aligned)]++;
    int* prev = &(r->array_int[OFFSET_BACKUP+((r->pos_used-1) * r->size_aligned)]);
    return prev;
}

/* v->variables is an array of struct transduction_variable
   which is a structure of (two before) NB_INT_BY_VARIABLES int */
int* ret = &(r->array_int[OFFSET_BACKUP+(r->pos_used * r->size_aligned)]);
memcpy((void*)ret,(const void*)(&(v->variables[0])),r->size_copydata);

r->pos_used ++;
r->array_int[OFFSET_DIRTY+(r->pos_used * r->size_aligned)] = 0;
r->array_int[OFFSET_COUNTER+(r->pos_used * r->size_aligned)] = 0;
r->array_int[OFFSET_SWAPPER+(r->pos_used * r->size_aligned)] = 0;

return ret;
}

/*
 * free memory from reserve
 * return 0 if there is still used space in reserve
 * return 1 if the reserve can be free
 */
static inline int free_variable_backup_using_reserve(variable_backup_memory_reserve* r)
{
if (r->array_int[OFFSET_COUNTER+(r->pos_used * r->size_aligned)]>0)
{
    r->array_int[OFFSET_COUNTER+(r->pos_used * r->size_aligned)]--;
    return 0;
}

r->pos_used --;
return (r->pos_used == 0) ? 1 : 0;
}


/*
 * the function below are replaced by macro for performance
 */
/*

int get_dirty(variable_backup_memory_reserve* r) ;
void set_dirty(variable_backup_memory_reserve* r,int d) ;

void inc_dirty(variable_backup_memory_reserve* r) ;
void dec_dirty(variable_backup_memory_reserve* r) ;
*/


/*
 * get the dirty counter of the current slot
 */

#define get_dirty(r) \
 ( ((r)->array_int[OFFSET_DIRTY+((r)->pos_used * (r)->size_aligned)]) )


#define set_dirty(r,d) \
{ \
    ((r)->array_int[OFFSET_DIRTY+((r)->pos_used * (r)->size_aligned)]=(d)); \
}

#define inc_dirty(r) \
{ \
    ((r)->array_int[OFFSET_DIRTY+((r)->pos_used * (r)->size_aligned)]++); \
}

#define dec_dirty(r) \
{ \
    ((r)->array_int[OFFSET_DIRTY+((r)->pos_used * (r)->size_aligned)]--); \
}


int* install_variable_backup_preserving(InputVariables* v,variable_backup_memory_reserve* r,const int*);
void restore_variable_array(InputVariables* v,variable_backup_memory_reserve* r,int*);

} // namespace unitex

#endif
