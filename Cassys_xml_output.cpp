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
 * Cassys_xml_output.cpp
 *
 *  Created on: 4 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#include "Cassys_xml_output.h"
#include "File.h"
#include "Cassys.h"
#include "FIFO.h"
#include "Cassys_concord.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

void concordFileBracket2Xml(const char *concordBracketFileName,const VersatileEncodingConfig *vec) {

	char concordXmlFileName[FILENAME_MAX]="";

	remove_extension(concordBracketFileName, concordXmlFileName);
	U_FILE *concord_xml_desc = u_fopen(vec, concordXmlFileName, U_WRITE);
	if (concord_xml_desc == NULL) {
		fatal_error("Cannot open file %s\n", concordBracketFileName);
		exit(1);
	}


	struct fifo *stage_concord = read_concord_file(concordBracketFileName, vec);

	while (!is_empty(stage_concord)) {
		locate_pos *l=(locate_pos*)take_ptr(stage_concord);
		unichar *line;
		line = (unichar *)malloc(sizeof(unichar)*(u_strlen(l->label)+1));

		free(line);
		free(l->label);
		free(l);
	}

	free_fifo(stage_concord);
}




unichar* concordAnnotatedBracket2Xml(){

	unichar *token = NULL;

	return token;

}






}
