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
extern const int EXTENDED_OUTPUT_STACK_SIZE;
extern const int EXTENDED_FUNCTIONS_PER_TRANSDUCTION;
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

/**
 *
 */
struct ExtendedOutputRender {
  int cardinality;
  struct stack_unichar* stack_template;
  struct stack_unichar* stack_render;
  vector_ptr* output_sets;
  vector_int* placeholders;
  vector_int* divisors;

  int new_output_set(int n_elements, int placeholder) const {
    vector_int_add(placeholders,placeholder);
    vector_int_add(divisors,0);
    return vector_ptr_add(output_sets, new_vector_ptr(n_elements));
  }

  // const char*, length
  int add_output(int set_number, const char* output, int length) const {
    return vector_ptr_add((vector_ptr*) output_sets->tab[set_number],
                          u_strndup(output, length));
  }

  // unichar, length
  int add_output(int set_number, const unichar* output, int length) const {
    return vector_ptr_add((vector_ptr*) output_sets->tab[set_number],
                          u_strndup(output, length));
  }

  // const char*
  int add_output(int set_number, const char* output) const {
    return add_output(set_number, output, strlen(output));
  }

  // unichar
  int add_output(int set_number, const unichar* output) const {
    return add_output(set_number, output, u_strlen(output));
  }

  // prepare the template to be rendered
  int prepare() {
    // there is no more chars to add to the output template,
    // hence we put a mark to indicate the end of the string
    push(stack_template, '\0');

    cardinality = 1;
    int i = output_sets->nbelems;

    // if output_sets contains 4 sets, e.g. A, B, C, D
    // the code below fills the "divisors" vector with the
    // cardinal product (the cardinality of the Cartesian product)
    // of the later sets of each set
    // D =          1   -> divisors[0]
    // C =         |D|  -> divisors[1]
    // B =     |C|x|D|  -> divisors[2]
    // A = |B|x|C|x|D|  -> divisors[3]
    while(i > 0) {
      --i;
      divisors->tab[i] = cardinality;
      cardinality = ((vector_ptr*) output_sets->tab[i])->nbelems * cardinality;
    }

    // will be equal to the cardinal product of all sets
    // e.g. the cardinal obtained from |A|x|B|x|C|x|D|
    return cardinality;
  }

  struct stack_unichar* render(int n) {
    // if there are no output sets (cardinality <= 1)  or the
    // given combination number (n) is out-of-bounds (0 <= n < cardinality)
    // then return the template without render it
    if (cardinality <= 1 || (n < 0 || n > cardinality)) {
      return stack_template;
    }

    return stack_render;
  }

  ExtendedOutputRender()
      : cardinality(0),
        stack_template(new_stack_unichar(EXTENDED_OUTPUT_STACK_SIZE)),
        stack_render(new_stack_unichar(EXTENDED_OUTPUT_STACK_SIZE)),
        output_sets(new_vector_ptr(EXTENDED_FUNCTIONS_PER_TRANSDUCTION)),
        placeholders(new_vector_int(EXTENDED_FUNCTIONS_PER_TRANSDUCTION)),
        divisors(new_vector_int(EXTENDED_FUNCTIONS_PER_TRANSDUCTION)) {
  }

  ~ExtendedOutputRender() {
    free_stack_unichar(stack_template);
    free_stack_unichar(stack_render);
    free_vector_int(placeholders);
    free_vector_int(divisors);
    free_vector_ptr_element(output_sets,
                            (release_p) free_vector_ptr,
                            (release_f) free);
  }

 private:
  UNITEX_DISALLOW_COPY_AND_ASSIGN(ExtendedOutputRender);
};

//struct {
//  int32_t length; // number of elemements
//  UnitexString extended_template;
//  UnitexString extended_output[length][];
//  int32_t dm[length][3]; // div, size, placeholder
//  int32_t total; // total of combinations
//  int32_t max_output_length;
//
//  UnitexString generate_literal_output(int32_t n) {
//    if (total == 0) return extended_template;
//
//    UnitexString literal_output(max_output_length);
//
//    int index = 0;
//
//    // 0 1 2 3 4 5 6 7
//    // f o o ? b a r 0
//    int32_t i;
//
//    for (i=0;i < length; i++) {
//      literal_output.append(extended_template.c_unichar()+index, dm[i][2]-index);
//      literal_output.append(extended_output[i][(int)(n/dm[i][0]) % dm[i][1]]);
//      index = dm[i][2] + 1;
//    }
//
//    literal_output.append(extended_template.c_unichar()+index, extended_template.len()-index);
//
//    return literal_output;
//  }
//};

// in:
// template
// sets
// n=number of sets
// [x_i,y_i] -> divisor_i, number of elements in set_i
// total=total of combinations
// out:
// literal_output


int deal_with_extended_output(unichar*,struct locate_parameters*,int*);

} // namespace unitex

#endif

