/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * Cassys_transducer.h
 *
 *  Created on: 8 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#ifndef CASSYS_TRANSDUCER_H_
#define CASSYS_TRANSDUCER_H_

#include <stdlib.h>
#include <stdio.h>


#include "LocateConstants.h"
#include "FileEncoding.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define INFINITY -1

/**
  * Structure storing the list of transducer
  */
struct transducer_name_and_mode_linked_list
{
    char* transducer_filename;
    OutputPolicy transducer_mode;
    int repeat_mode;
    int generic_graph;
    struct transducer_name_and_mode_linked_list* next;
};

/**
 * Structure storing informations about a transducer
 */

typedef struct transducer{
	char *transducer_file_name;
	OutputPolicy output_policy;
	int repeat_mode;
	int generic_graph;
}transducer;


struct transducer_name_and_mode_linked_list* add_transducer_linked_list_new_name(
			struct transducer_name_and_mode_linked_list *current_list,
			const char*filename);

struct transducer_name_and_mode_linked_list* add_transducer_linked_list_new_name(
		struct transducer_name_and_mode_linked_list *current_list,
		const char*filename,
		int repeat_mode,
		int generic_graph);

void set_last_transducer_linked_list_mode(
		struct transducer_name_and_mode_linked_list *current_list,
		OutputPolicy mode);

void set_last_transducer_linked_list_mode_by_string(
		struct transducer_name_and_mode_linked_list *current_list,
		const char*option_name);

void free_transducer_name_and_mode_linked_list(
		struct transducer_name_and_mode_linked_list *list);

struct transducer_name_and_mode_linked_list *load_transducer_list_file(
		const char *transducer_list_name, int translate_path_separator_to_native);

struct fifo *load_transducer_from_linked_list(
		const struct transducer_name_and_mode_linked_list *list,
		const char*transducer_filename_prefix);

/**
 * \brief returns a fifo containing the list of transducers to be applied in the cascade
 *
 * \param[in] file_list_transducer_name user file containing the list of transducers
 */
struct fifo *load_transducer(const VersatileEncodingConfig*,const char *file_list_transducer_name);

OutputPolicy GetOutputPolicyFromString(const char*option_name);

/**
 * \brief Removes comments (end of line following a '#' char)
 */
void remove_cassys_comments(char *line);

/**
 * \brief the 'fgets' function
 *
 * Written because it is unavailable in unitex environnement and because 'u_fgets' does not work on ascci files.
 */
char *cassys_fgets(char *line, int n, U_FILE *u);



/**
 * \brief Reads of line the name of a transducer in configuration file
 *
 * \param[in] line string line to be read
 *
 * \return an allocated string containing the name of transducer file or NULL if an error occurred
 */
char* extract_cassys_transducer_name(const char *line);


/**
 * \brief Reads the transducer policy in configuration file
 *
 * \param line text line to be read
 *
 * \return MERGE_OUTPUTS or REPLACE_OUTPUTS or IGNORE_OUTPUTS if an error occurred
 */
OutputPolicy extract_cassys_transducer_policy(const char *line);



char *extract_cassys_disabled(const char *line);

int extract_cassys_tranducer_star(const char *line);


bool is_debug_mode(transducer *t, const VersatileEncodingConfig* vec);

}

#endif /* CASSYS_TRANSDUCER_H_ */
