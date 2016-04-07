/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
  
#ifndef HTMLCharactersH
#define HTMLCharactersH

#include "Unicode.h"
#include "AsciiSearchTree.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define UNKNOWN_CHARACTER -1
#define DO_NOT_DECODE_CHARACTER -2
#define MALFORMED_HTML_CODE -3

void* init_HTML_character_context();
void free_HTML_character_context(void*);
int get_HTML_character(const void* html_ctx,const char*,int);
int is_HTML_control_character(unichar);

} // namespace unitex

#endif


