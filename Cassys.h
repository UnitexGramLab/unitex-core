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
/*
 *
 *  Created on: 29 avr. 2010
 *  Authors: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */


#ifndef CASSYS_H_
#define CASSYS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Concord.h"
#include "Locate.h"

#include "ProgramInvoker.h"
#include "LocateConstants.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "FIFO.h"
#include "Tokenize.h"
#include "Unicode.h"
#include "Alphabet.h"

#include "Cassys_tokens.h"
#include "Cassys_transducer.h"
#include "Cassys_external_program.h"
#include "Cassys_io.h"
#include "Cassys_concord.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {






extern const char *optstring_Cassys;
extern const struct option_TS lopts_Cassys[];
extern const char* usage_Cassys;


int main_Cassys(int argc,char* const argv[]);



/**
 * \brief function which makes the cascade
 *
 * \param[in] text
 * \param[in] transducer_list
 * \param[in] alphabet
 *
 * return 0 if correct
 */
int cascade(const char* text, int in_place, int must_create_directory, int must_do_cleanup, const char* tmp_work_dir,
	fifo* transducer_list, const char *alphabet,
	const char*name_input_offsets_file, int produce_offsets_file, const char* name_uima_offsets_file,
	const char*negation_operator,
	VersatileEncodingConfig*, 
	const char *morpho_dic,
	vector_ptr* tokenize_args, vector_ptr* locate_args, vector_ptr* concord_args,
	int dump_graph, int realign_token_graph_pointer, int display_perf);




void display_list_ustring(const struct list_ustring *l);




} // namespace unitex

#endif /* CASSYS_H_ */


