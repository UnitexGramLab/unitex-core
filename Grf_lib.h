/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#define GRF_HEADER_LINE_SIZE 128


typedef struct {
	unichar* box_content;
	int x,y;
	int n_transitions;
	int* transitions;
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

	int n_states;
	GrfState** states;
} Grf;


Grf* new_Grf();
void free_Grf(Grf*);
void free_GrfState(GrfState*);
Grf* load_Grf(const char*);
void save_Grf(U_FILE*,Grf*);
Grf* dup_Grf(Grf*);
GrfState* cpy_grf_state(GrfState*);
void cpy_grf_states(Grf* dst,Grf* src);

#endif

