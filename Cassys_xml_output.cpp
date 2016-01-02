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
 * Cassys_xml_output.cpp
 *
 *  Created on: 4 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#include "Cassys_xml_output.h"
#include "Cassys_concord.h"

#include "File.h"
#include "FIFO.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {





void xmlizeConcordFile(const char *concordBracketFileName, const VersatileEncodingConfig *vec) {

	//char concordXmlFileName[FILENAME_MAX]="";

//	remove_extension(concordBracketFileName, concordXmlFileName);
//	strcat(concordXmlFileName,".xml");

	struct fifo *stage_concord = read_concord_file(concordBracketFileName, vec);


	U_FILE *concord_xml_desc = u_fopen(vec, concordBracketFileName, U_WRITE);
		if (concord_xml_desc == NULL) {
			fatal_error("Cannot open file %s\n", concordBracketFileName);
			exit(1);
		}
	u_fprintf(concord_xml_desc,"#M\n");

	while (!is_empty(stage_concord)) {

		locate_pos *l=(locate_pos*)take_ptr(stage_concord);

		//u_printf("string = %S\n",l->label);

		unichar *xml_line = xmlizeConcordLine(l->label);

		u_fprintf(concord_xml_desc,"%ld.%ld.%ld %ld.%ld.%ld %S\n",
				l->token_start_offset,
				l->character_start_offset,
				l->logical_start_offset,
				l->token_end_offset,
				l->character_end_offset,
				l->logical_end_offset,
				xml_line);

		free(xml_line);
		free(l->label);
		free(l);
	}
	u_fclose(concord_xml_desc);
	free_fifo(stage_concord);
}



unichar* xmlizeConcordLine(const unichar *line){

	list_ustring *lu = cassys_tokenize(line);

	int size = u_strlen(line);
	unichar *result = (unichar*) malloc(sizeof(unichar) * (size+1));
	if (result == NULL) {
		fatal_alloc_error("malloc\n");
	}
	result[0] = '\0';

	for(list_ustring *ite = lu; ite!=NULL; ite=ite->next){
		if(is_lexical_token(ite->string)){

			unichar *unprotected = unprotect_lexical_tag(ite->string);

			unichar *xml_string = xmlize(unprotected);
			size = size - u_strlen(ite->string) + u_strlen(xml_string);

			result = (unichar*)realloc(result, sizeof(unichar) * (size +1));
			if (result == NULL) {
				fatal_alloc_error("realloc");
			}
			u_strcat(result, xml_string);
			free(unprotected);
			free(xml_string);

		} else {
			u_strcat(result,ite->string);
		}

	}
	free_list_ustring(lu);
	return result;

}


unichar* xmlize(unichar *lexical_token){

	struct cassys_pattern *cp = load_cassys_pattern(lexical_token);

	unichar *form = xmlizeConcordLine(cp->form);
	free(cp->form);
	cp->form = form;

	unichar *result = xmlize(cp);
	free_cassys_pattern(cp);

	return result;
}

/**
 * Return a xml unichar string of the cassys_pattern cp
 */
unichar* xmlize(struct cassys_pattern *cp){


	unichar *xml_form = xmlize_element(cp->form, FORM_OPENING, FORM_CLOSING);
	unichar *xml_lem = xmlize_element(cp->lem, LEM_OPENING, LEM_CLOSING);
	unichar *xml_code = xmlize_element(cp->code, CODE_OPENING, CODE_CLOSING);
	unichar *xml_inflex = xmlize_element(cp->inflection, INFLECTION_OPENING, INFLECTION_CLOSING);

	int size = u_strlen(LEXICAL_OPENING) + u_strlen(xml_form) + u_strlen(xml_lem)
			+ u_strlen(xml_code) + u_strlen(xml_inflex) + u_strlen(LEXICAL_CLOSING);
	unichar *result = (unichar*)malloc(sizeof(unichar)*(size+1));
	if(result==NULL) {
		fatal_alloc_error("malloc");
	}
	result[0]='\0';


	u_strcat(result, LEXICAL_OPENING);
	u_strcat(result, xml_form);
	u_strcat(result, xml_lem);
	u_strcat(result, xml_code);
	u_strcat(result, xml_inflex);
	u_strcat(result, LEXICAL_CLOSING);

	free(xml_form);
	free(xml_code);
	free(xml_lem);
	free(xml_inflex);

	return result;
}

/**
 * Return the text enclosed between opening_xml and closing_xml if text not empty.
 * Return empty string if text is empty
 */
unichar *xmlize_element(const unichar *text, const unichar *opening_xml, const unichar *closing_xml) {

int size = 0;

if(text != NULL && text[0]!='\0') {
	size = u_strlen(opening_xml)+u_strlen(closing_xml)+ u_strlen(text);
}

unichar *result = (unichar*)malloc(sizeof(unichar)*(size+1));
if(result==NULL) {
	fatal_alloc_error("malloc");
}
result[0]='\0';

if(text != NULL && text[0]!='\0') {
	u_strcat(result, opening_xml);
	u_strcat(result, text);
	u_strcat(result, closing_xml);
}

return result;

}


/**
 * Return a unichar string where each string of u is enclosed with opening_xml and closing_xml
 * Return empty string if u is empty
 */
unichar *xmlize_element(list_ustring *u, const unichar *opening_xml, const unichar *closing_xml){

	int size = 0;
	list_ustring *ite = u;

	while(ite!=NULL){
		size += u_strlen(ite->string) + u_strlen(opening_xml) + u_strlen(closing_xml);
		ite = ite->next;
	}

	unichar *result = (unichar*)malloc(sizeof(unichar)*(size+1));
	if(result==NULL) {
		fatal_alloc_error("malloc");
	}
	result[0]='\0';

	ite = u;
	while(ite!=NULL){
		unichar *single_element = xmlize_element(ite->string, opening_xml, closing_xml);
		u_strcat(result, single_element);
		free(single_element);
		ite = ite->next;
	}

	return result;
}


}
