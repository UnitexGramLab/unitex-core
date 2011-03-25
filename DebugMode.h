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

#ifndef DebugModeH
#define DebugModeH

#include "LocatePattern.h"
#include "Unicode.h"

/**
 * This library is used to generate debug fst2 tags. In normal mode,
 * a fst2 tag may or may not have an output. In debug mode, all tags (except
 * special tag #0 that must always be "<E>") have an output of the following form:
 *
 *  1 output 2 graph:box:line 3 input 4
 *
 *  where:
 *   - 1, 2, 3 and 4 stands for the unicode characters whose value are 1, 2, 3 and 4
 *   - output=output of the tag
 *   - graph=number of the origin graph in the fst2
 *   - box=number of the origin box in the graph
 *   - line=number of the origin line in the box
 *   - input=input of tag, duplicated here for convenience
 */

#define DEBUG_INFO_OUTPUT_MARK 1
#define DEBUG_INFO_COORD_MARK 2
#define DEBUG_INFO_INPUT_MARK 3
#define DEBUG_INFO_END_MARK 4
#define DEBUG_INFO_GRAPHCALL_MARK 5


void add_debug_infos(unichar* output,int graph,int box,int line);
void save_real_output_from_debug(U_FILE* f,OutputPolicy policy,unichar* s);
void create_graph_call_debug_tag(unichar* dst,unichar* src,int graph_number,int before_call);

#endif

