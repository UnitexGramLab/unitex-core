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

#ifndef GrfTest_libH
#define GrfTest_libH

#include "Unicode.h"
#include "Grf_lib.h"
#include "Vector.h"
#include "LocateConstants.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define GRF_UNIT_TEST_PFX "@TEST:"

typedef struct {
    unichar* text;
    int start,end;
    int must_match;
    OutputPolicy output_policy;
    MatchPolicy match_policy;
    unichar* expected_output;
} GrfUnitTest;


GrfUnitTest* new_GrfUnitTest();
void free_GrfUnitTest(GrfUnitTest* test);
vector_ptr* get_grf_unit_tests(Grf* grf,const char* grf_name,U_FILE* f_error);
int check_test_results(GrfUnitTest* t,const char* concord,const char* grf,U_FILE* f_error);

} // namespace unitex

#endif

