/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * Cassys_concord.h
 *
 *  Created on: 8 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#ifndef CASSYS_CONCORD_H_
#define CASSYS_CONCORD_H_

#include "Unicode.h"
#include "FileEncoding.h"
#include "Cassys_tokens.h"
#include "LocateConstants.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * \struct locate_pos
 * \brief Concord structure information
 */
typedef struct locate_pos{

    /**
     * position in token of the first token located by a transducer
     */
    long token_start_offset;

    /**
     * position in character of the first character in the first token located by a transducer
     */
    long character_start_offset;

    /**
     * used for thai. Not used by cassys
     */
    long logical_start_offset;

    /**
     * position in token of the last token located by a transducer
     */
    long token_end_offset;

    /**
     * position in character of the last character in the last token located by a transducer
     */
    long character_end_offset;

    /**
     * used for thai. Not used by cassys
     */
    long logical_end_offset;
    unichar *label;
}locate_pos;


/**
 * \brief Reads the 'concord.ind' file and returns a FIFO list of locate_pos items
 *
 * \param concord_file_name file containing the matches
 */
struct fifo *read_concord_file(const char *concord_file_name, const VersatileEncodingConfig*);

int count_concordance(const char *concord_file_name, const VersatileEncodingConfig* vec);

/**
 * \brief Reads a line of the 'concord.ind' file and and returns the content in a struct locate_pos
 *
 * \parameter[in] line unichar string containing the 'concord.ind' line
 *
 * \return a struct locate_pos
 */
locate_pos *read_concord_line(const unichar *line);


/**
 * \brief Produces a concordance file with the matches found by all the locates program called during the cascade
 *
 * \param[in] list the token list representation of the text
 * \param[in] the text target
 * \param[in] the number of transducers applied during the cascade
 */
void construct_cascade_concord(cassys_tokens_list *list, const char *text_name, int number_of_transducer, int iteration,
    VersatileEncodingConfig*);
void protect_lexical_tag_in_concord(const char *concord_file_name, const VersatileEncodingConfig *vec);

void protect_lexical_tag_in_concord(const char *concord_file_name, const OutputPolicy op, const VersatileEncodingConfig *vec);

void construct_xml_concord(const char *text_name, VersatileEncodingConfig* vec);

void construct_istex_token(const char *, VersatileEncodingConfig*, const char*);

void construct_istex_standoff(const char *, VersatileEncodingConfig*, const char*, const char*, const char*);
}




#endif /* CASSYS_CONCORD_H_ */
