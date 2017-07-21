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

#include "DebugMode.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Adds debug information at the end of the given output string.
 */
void add_debug_infos(unichar* output,int graph,int box,int line) {
int l=u_strlen(output);
u_sprintf(output+l,"%C%d:%d:%d%C",DEBUG_INFO_COORD_MARK,graph,box,line,DEBUG_INFO_INPUT_MARK);
}


/**
 * This function takes a string supposed to contain a debug output and its
 * prints the expected normal (non debug) output that corresponds to it
 * into the given file. As a debug output is supposed to contain
 * all tags used to build a match, this function also deals with left
 * contexts and right contexts.
 */
void save_real_output_from_debug(U_FILE* f,OutputPolicy policy,unichar* s) {
int print_input=0;
int print_output=0;
switch (policy) {
case IGNORE_OUTPUTS: print_input=1; print_output=0; break;
case MERGE_OUTPUTS: print_input=1; print_output=1; break;
case REPLACE_OUTPUTS: print_input=0; print_output=1; break;
default:break;
}
Ustring* output=new_Ustring();
Ustring* tmp=new_Ustring();
int n_contexts=0;
while (*s!='\0') {
    /* Skipping char #1 */
    s++;
    while (*s!=DEBUG_INFO_COORD_MARK) {
        if (print_output && n_contexts==0) u_strcat(output,*s);
        s++;
    }
    /* Skipping everything until char #3 */
    while (*(s++)!=DEBUG_INFO_INPUT_MARK) {
    }
    empty(tmp);
    /* Skipping everything until char #4 */
    while (*s!=DEBUG_INFO_END_MARK) {
        u_strcat(tmp,*s);
        s++;
    }
    if (!u_strcmp(tmp->str,"$*")) {
        empty(output);
    } else if (!u_strcmp(tmp->str,"$[") || !u_strcmp(tmp->str,"$![")) {
        n_contexts++;
    } else if (!u_strcmp(tmp->str,"$]")) {
        n_contexts--;
    }
    s++;
    while (*s!='\0' && *s!=DEBUG_INFO_OUTPUT_MARK) {
        if (print_input && n_contexts==0) u_strcat(output,*s);
        s++;
    }
}
//u_fprintf(f,"%S",output->str);
u_fputs(output->str,f);
free_Ustring(tmp);
free_Ustring(output);
}


/**
 * This function takes a debug output and copies a modified version of it into dst,
 * so that dst will represent a debug tag associated to a graph call.
 */
void create_graph_call_debug_tag(unichar* dst,unichar* src,int graph_number,int before_call) {
u_sprintf(dst,"<E>/%C",DEBUG_INFO_OUTPUT_MARK);
int i;
for (i=0;src[i]!=DEBUG_INFO_COORD_MARK;i++) {
    if (src[i]=='\0') {
        fatal_error("Debug output error in create_graph_call_debug_tag\n");
    }
}
int n=5;
i--;
do {
    dst[n++]=src[++i];
} while (src[i]!=DEBUG_INFO_INPUT_MARK);
dst[n++]=DEBUG_INFO_GRAPHCALL_MARK;
dst[n++]=before_call?'<':'>';
u_sprintf(dst+n,"%d%C",graph_number,DEBUG_INFO_END_MARK);
}

} // namespace unitex

