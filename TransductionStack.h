/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef TransductionStackH
#define TransductionStackH

#include "Unicode.h"
#include "LocatePattern.h"
#include "Stack_unichar.h"
#include "UnitexString.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

extern const int TRANSDUCTION_STACK_SIZE;
//extern const int EXTENDED_OUTPUT_STACK_SIZE;
//extern const int EXTENDED_FUNCTIONS_PER_TRANSDUCTION;
//#define TRANSDUCTION_STACK_SIZE 10000

// replaced by u_is_identifier
#if !UNITEX_USE(BASE_UNICODE)
int is_variable_char(unichar);
#endif

/* Every character or string that comes from the input text must be
 * pushed with the following functions, because some characters like dots
 * and commas may have to be protected when they come from the input. This
 * is useful when Locate is invoked from the Dico program in order to avoid
 * producing bad lines like:
 *
 *    3,14,PI.NUM   ==>  should be:  3\,14,PI.NUM
 */
void push_input_char(struct stack_unichar*,unichar,int);
void push_input_string(struct stack_unichar*,unichar*,int);
void push_input_substring(struct stack_unichar* stack,unichar* s,int length,int);

void push_output_char(struct stack_unichar*,unichar);
void push_output_string(struct stack_unichar*, const char*);
void push_output_string(struct stack_unichar*,unichar*);


int deal_with_extended_output(unichar*,struct locate_parameters*,int*);

} // namespace unitex

#endif

