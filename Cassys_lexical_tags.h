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
/*
 * Cassys_tags.h
 *
 *  Created on: 8 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#ifndef CASSYS_TAGS_H_
#define CASSYS_TAGS_H_

#include "FileEncoding.h"
#include "Unicode.h"
#include "List_ustring.h"
#include "Alphabet.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * \brief Returns the list of tokens contained in text.
 *
 * This function is different from tokenize_word_by_word in the treatment of braces. With this function, content
 * of braces, including the opening and losing braces, is a token.
 *
 * \param[in] text the text to be tokenized
 * \param[in] alphabet the alphabet of the text
 */
list_ustring *cassys_tokenize_word_by_word(const unichar* text,const Alphabet* alphabet);


/**
 * \brief Adds protection character in braces before
 *
 * \param[in/out] text file name of the text to be protected
 */
void protect_special_characters(const char *text, const VersatileEncodingConfig*);

/**
 * \brief Adds protection characters in the text zone.
 */
unichar *protect_text_in_braced_string(const unichar *s);

/**
 * \brief Adds protection character in a string contained in braces.
 *
 * The string s is supposed to be given without the external braces.
 */
unichar *protect_braced_string(const unichar *s);

/**
 * \brief Adds protection characters in the lem zone. (currently does nothing)
 */
unichar *protect_lem_in_braced_string(const unichar *s);


/**
 * \brief Returns all the characters encountered
 */
unichar *get_braced_string(U_FILE *u);

}


#endif /* CASSYS_TAGS_H_ */
