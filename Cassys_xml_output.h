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
 * Cassys_xml_output.h
 *
 *  Created on: 4 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#ifndef CASSYS_XML_OUTPUT_H_
#define CASSYS_XML_OUTPUT_H_

#include "Cassys_lexical_tags.h"

#include "FileEncoding.h"
#include "Unicode.h"
#include "StringParsing.h"

#include <stdio.h>

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif




namespace unitex {

const unichar P_CASSYS_PROTECTED_CHARS[] ={'\\',',','.',':','+',0};

const unichar LEM_OPENING[] = {'<','l','e','m','m','a','>',0};
const unichar LEM_CLOSING[] = {'<','/','l','e','m','m','a','>',0};

const unichar FORM_OPENING[] = {'<','f','o','r','m','>',0};
const unichar FORM_CLOSING[] = {'<','/','f','o','r','m','>',0};

const unichar CODE_OPENING[] = {'<','c','o','d','e','>',0};
const unichar CODE_CLOSING[] = {'<','/','c','o','d','e','>',0};

const unichar INFLECTION_OPENING[] = {'<','i','n','f','l','e','c','t','>',0};
const unichar INFLECTION_CLOSING[] = {'<','/','i','n','f','l','e','c','t','>',0};

const unichar LEXICAL_OPENING[] = {'<','c','s','c','>',0};
const unichar LEXICAL_CLOSING[] = {'<','/','c','s','c','>',0};


void xmlizeConcordFile(const char *concordBracketFileName, const VersatileEncodingConfig *vec);
unichar* xmlizeConcordLine(const unichar *line);

unichar* xmlize(unichar *lexical_token);
unichar* xmlize(struct cassys_pattern *cp);

unichar *xmlize_element(const unichar *text, const unichar *opening_xml, const unichar *closing_xml);
unichar *xmlize_element(list_ustring *u, const unichar *opening_xml, const unichar *closing_xml);
}


#endif /* CASSYS_XML_OUTPUT_H_ */
