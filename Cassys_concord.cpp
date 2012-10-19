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
 * Cassys_concord.cpp
 *
 *  Created on: 8 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#include "Cassys_concord.h"
#include "Cassys_xml_output.h"

#include "FIFO.h"
#include "Snt.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * \brief Reads a 'concord.ind' file and returns a fifo list of all matches found and their replacement
 *
 * \param[in] concord_file_name the name of the concord.ind file
 *
 * \return a fifo list of all the matches found with their replacement sentences. Each element is
 * stored in a locate_pos structure
 */
struct fifo *read_concord_file(const char *concord_file_name, const VersatileEncodingConfig* vec){
	unichar line[4096];

	struct fifo *f = new_fifo();

	U_FILE *concord_desc_file;
	concord_desc_file = u_fopen(vec,concord_file_name,U_READ);
	if( concord_desc_file == NULL){
		fatal_error("Cannot open file %s\n",concord_file_name);
		exit(1);
	}

	if(u_fgets(line,4096,concord_desc_file)==EOF){
		fatal_error("Malformed concordance file %s",concord_file_name);
	}

	while(u_fgets(line,4096,concord_desc_file)!=EOF){

		// we don't want the end of line char
		line[u_strlen(line)-1]='\0';
		locate_pos *l = read_concord_line(line);
		put_ptr(f,l);

	}

	u_fclose(concord_desc_file);
	return f;
}



/**
 * \brief Reads an line of a concord.ind file.
 *
 * \param[in] line the unichar string containing the line
 *
 * The line is expected to be in the the format : n.n.n n.n.n t where n are integers and t is string
 *
 * \return The information read in a locate_pos structure
 */
locate_pos *read_concord_line(const unichar *line) {

	locate_pos *l;
	l = (locate_pos*) malloc(sizeof(locate_pos) * 1);
	if (l == NULL) {
		fatal_alloc_error("read_concord_line");
		exit(1);
	}
	l->label = (unichar*) malloc(sizeof(unichar) * (u_strlen(line) + 1));
	if (l->label == NULL) {
		fatal_alloc_error("read_concord_line");
		exit(1);
	}

	// format of a line : n.n.n n.n.n t where n are integers and t is string
	const unichar **next;

	const unichar *current = line;
	next = &line; // make next not NULL
	l->token_start_offset = (long)u_parse_int(current, next);

	current = (*next)+1;
	l->character_start_offset = (long)u_parse_int(current, next);

	current = (*next)+1;
	l->logical_start_offset = (long)u_parse_int(current, next);

	current = (*next)+1;
	l->token_end_offset = (long)u_parse_int(current, next);

	current = (*next)+1;
	l-> character_end_offset = (long)u_parse_int(current, next);

	current = (*next)+1;
	l-> logical_end_offset = (long)u_parse_int(current, next);

	current = (*next)+1;
	u_strcpy(l->label,current);

	return l;
}





void construct_cascade_concord(cassys_tokens_list *list, const char *text_name, int number_of_transducer,
    VersatileEncodingConfig* vec){

	fprintf(stdout, "Construct cascade concord\n");

	struct snt_files *snt_file = new_snt_files(text_name);

	U_FILE *concord_desc_file = u_fopen(vec, snt_file->concord_ind,U_WRITE);
	if( concord_desc_file == NULL){
		fatal_error("Cannot open file %s\n",snt_file->concord_ind);
		exit(1);
	}

	fprintf(stdout, "Concord File %s successfully opened\n",snt_file->concord_ind);

	if (list == NULL) {
		fatal_error("empty text");
	}

	u_fprintf(concord_desc_file,"#M\n");

	cassys_tokens_list *current_pos_in_original_text = list;

	cassys_tokens_list *output=get_output(list, number_of_transducer);
	struct list_ustring *sentence = NULL;
	bool output_detected = false;
	long token_position=0;

	while(current_pos_in_original_text != NULL && output != NULL){
		if(output -> transducer_id == 0){
			if(output_detected){
				int start_position = token_position;
				int last_token_length = 0;
				while(current_pos_in_original_text != output){
					token_position ++;
					last_token_length = u_strlen(current_pos_in_original_text -> token)-1;
					current_pos_in_original_text = current_pos_in_original_text -> next_token;
				}

				// token position pointe sur le token suivant�
				int end_position=token_position-1;

				if(sentence == NULL){
					fatal_error("construct_cassys_concordance : Phrase de remplacement vide\n");
				}

				struct list_ustring *iterator = sentence;
				while(iterator -> next != NULL){
					iterator = iterator -> next;
				}
				//display_list_ustring(iterator);

				iterator = sentence;
				u_fprintf(concord_desc_file, "%d.0.0 %d.%d.0 ",start_position,end_position,last_token_length);
				while(iterator != NULL){
					u_fprintf(concord_desc_file,"%S",iterator->string);
					//u_fprintf(concord_desc_file,"concord.ind : %S %S %S\n",iterator->string, previous_pos_in_original_text->token, current_pos_in_original_text->token);
					iterator = iterator -> next;
				}
				u_fprintf(concord_desc_file,"\n");

				current_pos_in_original_text = current_pos_in_original_text -> next_token;
				output = get_output(current_pos_in_original_text, number_of_transducer);
				token_position++;

				free_list_ustring(sentence);
				sentence = NULL;

				output_detected = false;
			} else {
				current_pos_in_original_text = current_pos_in_original_text -> next_token;
				output = get_output(current_pos_in_original_text,number_of_transducer);
				token_position++;
			}
		}
		else {
			//u_printf("insert new sentence\n");

			sentence = insert_at_end_of_list(output->token, sentence);
			output = output -> next_token;
			output = get_output(output, number_of_transducer);
			output_detected = true;
		}

	}

	free_list_ustring(sentence);

	u_fclose(concord_desc_file);
	free(snt_file);

}



void construct_xml_concord(const char *text_name, VersatileEncodingConfig* vec){

	fprintf(stdout,"Building xml concord\n");

	struct snt_files *snt_file = new_snt_files(text_name);

	xmlizeConcordFile(snt_file->concord_ind, vec);


	free(snt_file);
}


}
