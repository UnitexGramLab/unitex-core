/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * Cassys_external_program.h
 *
 *  Created on: 8 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#ifndef CASSYS_EXTERNAL_PROGRAM_H_
#define CASSYS_EXTERNAL_PROGRAM_H_



#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * \brief Calls the 'locate' program
 *
 *	The locate program is called with the options '--longest_match' and '--all'. References of to the occurrences
 *	found are saved in a file 'concord.ind'
 *
 * \param text_name target text
 * \param transducer transducer to apply
 * \param file name of the alphabet of the target text
 */
int launch_locate_in_Cassys(const char *text_name,
                            const struct transducer *transducer,
                            const char* alphabet_name,
                            const char*negation_operator,
                            const VersatileEncodingConfig*,
                            const char *morpho_dic,
                            vector_ptr* additional_args);



/**
 * brief Calls the concord program
 *
 * The 'concord' program is called with the option '--merge'. It replaces all the occurence of matches in 'concord.ind'
 * by their output.
 *
 * \param text_name target text
 * \param index_file name of the file containing the matches
 * \param alphabet_name name of the file containing the alphabet of the text
 */
int launch_concord_in_Cassys(const char *text_name,
                            const char* index_file,
                            const char *alphabet_name,
                            VersatileEncodingConfig*,
                            vector_ptr* additional_args);







/**
 * \brief Calls the program 'tokenize'
 *
 *	The program is called with the option 'word_by_word. The results of this program are the two files
 *	'tokens.txt' which contains all the tokens of 'text' and 'tokens.cod' which contains the text coded with
 *	'tokens.txt'.
 *
 * \param text_name target text
 * \param file name of the alphabet of the target text
 * \param token_txt_name name of the file containing tokens from a precedent call to tokenize
 *
 */
int launch_tokenize_in_Cassys(const char *text_name,
                            const char *alphabet_name,
                            const char *token_txt_name,
                            VersatileEncodingConfig*,
                            vector_ptr* additional_args);

}
#endif /* CASSYS_EXTERNAL_PROGRAM_H_ */
