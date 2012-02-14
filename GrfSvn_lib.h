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

#ifndef GrfSvn_libH
#define GrfSvn_libH

#include "Grf_lib.h"
#include "Vector.h"

namespace unitex {

typedef enum {
	DIFF_PROPERTY='P',
	DIFF_BOX_MOVED='M',
	DIFF_BOX_CONTENT_CHANGED='C',
	DIFF_BOX_ADDED='A',
	DIFF_BOX_REMOVED='R',
	DIFF_TRANSITION_ADDED='T',
	DIFF_TRANSITION_REMOVED='X'
} DiffOpType;

typedef struct {
	/* Diff operation type */
	DiffOpType op_type;
	/* If we have a difference on properties, we need the property name */
	char property[16];
	/* Box concerned by the diff operation, with its number in the
	 * base grf and the one in the dest grf */
	int box_base;
	int box_dest;
	/* If we have a transition change, the origin of the transition is the
	 * current box, and its destination is dest. As for the box id,
	 * we store the destination state with its base and dest numbers */
	int dest_base;
	int dest_dest;
} DiffOp;


typedef struct {
	/* The diff operations */
	vector_ptr* diff_ops;
	/* A renumber array indicating for each base state its corresponding
	 * state in the dest grf, or -1 if the box was deleted */
	int* base_to_dest;
	int size_base_to_dest;
	/* A renumber array indicating for dest base state its corresponding
	 * state in the base grf, or -1 if the box was added */
	int* dest_to_base;
	int size_dest_to_base;
	/* We also need reverse transitions */
	ReverseTransitions* reverse_transitions_base;
	ReverseTransitions* reverse_transitions_dest;
} GrfDiff;


int diff3(U_FILE* f,U_FILE* conflicts,Grf* mine,Grf* base,Grf* other,int only_cosmetics);
void normalize_grf(Grf* grf);
GrfDiff* grf_diff(Grf* base,Grf* dest);
void free_GrfDiff(GrfDiff*);
void print_diff(U_FILE* f,GrfDiff* diff);

} // namespace unitex

#endif

