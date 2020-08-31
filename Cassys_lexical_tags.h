/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


struct cassys_pattern {
    unichar *form;
    unichar *lem;
    list_ustring *code;
    list_ustring *inflection;
};


/**
 * \brief Returns the list of tokens contained in text.
 *
 * This function is different from tokenize_word_by_word in the treatment of braces.
 * With this function, content of braces, including the opening and losing braces, is a token.
 *
 * \param[in] text the text to be tokenized
 * \param[in] alphabet the alphabet of the text
 */
list_ustring *cassys_tokenize_word_by_word(const unichar* text,const Alphabet* alphabet);





struct cassys_pattern* load_cassys_pattern(unichar *string);

bool is_lexical_token(unichar *token);

int begin_of_lexical_tag(const unichar *text);
int end_of_lexical_tag(const unichar *text);

list_ustring *cassys_tokenize(const unichar* text);

unichar* cassys_pattern_2_lexical_tag(struct cassys_pattern *cp,
        bool to_protect) ;
unichar *unprotect_lexical_tag(const unichar *text);

unichar *protect_lexical_tag(const unichar *text, bool b);
unichar *protect_form(unichar *string);

int get_form_lemma_separator_position(unichar *text);
void free_cassys_pattern(cassys_pattern *cp);
}


#endif /* CASSYS_TAGS_H_ */
