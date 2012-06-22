/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef Grf_libH
#define Grf_libH

#include "Unicode.h"
#include "Vector.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define GRF_HEADER_LINE_SIZE 128


typedef struct {
	unichar* box_content;
	int x,y;
	vector_int* transitions;

	/* This information is useful when manipulating sentence grfs */
	int rank;

	/* box_number is used for artificial box duplication when
	 * specifying something like [2;4] to indicate that the given
	 * box can match 2, 3 or 4 times. In such a case, we will expend this
	 * request by creating all the boxes as a human would have created them
	 * by hand, but for consistency in debug mode, we want all those
	 * boxes to be considered as the original unique one in the grf. So,
	 * artificial boxes will have a number that is not their index in the
	 * state array of the grf object.
	 */
	int box_number;
} GrfState;


typedef struct {
	/* For the header part, we rawly store line contents, since
	 * there is no need to analyse them */
	unichar size[GRF_HEADER_LINE_SIZE];
	unichar font[GRF_HEADER_LINE_SIZE];
	unichar ofont[GRF_HEADER_LINE_SIZE];
	unichar bcolor[GRF_HEADER_LINE_SIZE];
	unichar fcolor[GRF_HEADER_LINE_SIZE];
	unichar acolor[GRF_HEADER_LINE_SIZE];
	unichar scolor[GRF_HEADER_LINE_SIZE];
	unichar ccolor[GRF_HEADER_LINE_SIZE];
	unichar dboxes[GRF_HEADER_LINE_SIZE];
	unichar dframe[GRF_HEADER_LINE_SIZE];
	unichar ddate[GRF_HEADER_LINE_SIZE];
	unichar dfile[GRF_HEADER_LINE_SIZE];
	unichar ddir[GRF_HEADER_LINE_SIZE];
	unichar drig[GRF_HEADER_LINE_SIZE];
	unichar drst[GRF_HEADER_LINE_SIZE];
	unichar fits[GRF_HEADER_LINE_SIZE];
	unichar porient[GRF_HEADER_LINE_SIZE];

	vector_ptr* metadata;

	int n_states;
	GrfState** states;
} Grf;


typedef struct {
	int n;
	vector_int** t;
} ReverseTransitions;


Grf* new_Grf();
Grf* new_Grf(int create_default_header);
void free_Grf(Grf*);
GrfState* new_GrfState(const unichar* content,int x,int y,int rank,int box_number);
GrfState* new_GrfState(const char* content,int x,int y,int rank,int box_number);
void free_GrfState(GrfState*);
int add_GrfState(Grf*,GrfState*);
Grf* load_Grf(const VersatileEncodingConfig*,const char*);
void save_Grf(U_FILE*,Grf*);
Grf* dup_Grf(Grf*);
GrfState* cpy_grf_state(GrfState*);
void cpy_grf_states(Grf* dst,Grf* src);
vector_ptr* tokenize_box_content(unichar* content);

ReverseTransitions* compute_reverse_transitions(Grf* g);
void free_ReverseTransitions(ReverseTransitions*);

int have_same_outgoing_transitions(GrfState*,GrfState*);
int have_same_outgoing_transitions(Grf*,int,int);
int have_same_incoming_transitions(int,int,ReverseTransitions*);
int have_same_transitions(Grf*,int,int,ReverseTransitions*);

} // namespace unitex

#endif

