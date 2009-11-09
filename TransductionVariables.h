 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


/* Value to use to indicate that a variable bound is undefined */
#define UNDEF_VAR_BOUND -666666


/**
 * This structure is used to associates ranges to variable names.
 */
typedef struct {
   /* For a given variable A, this array gives an index a to be used in
    * the 'variables' array */
   struct string_hash* variable_index;
   /* variables[a] gives the range associated to the variable #a */
   struct transduction_variable* variables;
} Variables;


/**
 * This structure defines the range of a variable in the text tokens.
 *
 * Note: when used from LocateTfst, start and end represent the
 *       TfstTag indices of the first and last text dependent tags of
 *       a match. Moreover, 'end' must be taken into account, at the opposite
 *       of the Locate use of this structure.
 */
struct transduction_variable {
   /* Position of the first token of the sequence */
   int start;
   /* Position after the last token of the sequence, so that the sequence is in the
    * range [start;end[ */
   int end;
};



Variables* new_Variables(struct list_ustring*);
void free_Variables(Variables*);
struct transduction_variable* get_transduction_variable(Variables*,unichar*);
void set_variable_start(Variables*,int,int);
void set_variable_end(Variables*,int,int);
int get_variable_start(Variables*,int);
int get_variable_end(Variables*,int);

int* create_variable_backup(Variables*);
void free_variable_backup(int*);
void install_variable_backup(Variables*,int*);
void update_variable_backup(int*,Variables*);


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
variable_backup_memory_reserve* create_variable_backup_memory_reserve(Variables*);

/*
 * clear the reserve from memory
 */
void free_reserve(variable_backup_memory_reserve*r);

/* check if the reserve contain space and is correct to save variable v */
int is_enough_memory_in_reserve_for_two_set_variables(Variables* v,variable_backup_memory_reserve* r);

/*
 * create the backup, taking memory from reserve
 */
int* create_variable_backup_using_reserve(Variables* v,variable_backup_memory_reserve* r) ;

/*
 * free memory from reserve
 * return 0 if there is still used space in reserve
 * return 1 if the reserve can be free
 */
int free_variable_backup_using_reserve(variable_backup_memory_reserve* r) ;

/*
 * the function below are replaced by macro for performance
 */
/*
  
int get_dirty(variable_backup_memory_reserve* r) ;
void set_dirty(variable_backup_memory_reserve* r,int d) ;

void inc_dirty(variable_backup_memory_reserve* r) ;
void dec_dirty(variable_backup_memory_reserve* r) ;
*/


#define OFFSET_COUNTER (0)
#define OFFSET_DIRTY (1)
#define OFFSET_SWAPPER (2)
#define OFFSET_BACKUP (4)

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


int* install_variable_backup_preserving(Variables* v,variable_backup_memory_reserve* r,int*);
void restore_variable_array(Variables* v,variable_backup_memory_reserve* r,int*);

#endif
