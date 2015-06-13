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
 * Cassys_external_program.cpp
 *
 *  Created on: 8 oct. 2012
 *      Author: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#include "Grf2Fst2.h"
#include "Cassys.h"
#include "Cassys_external_program.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {



/**
 * \brief Calls the tokenize program in Cassys
 *
 *	Tokenize is called with target text_name and options --word_by_word, --alphabet=alphabet_name, --token=token_txt_name if
 *	if token_txt_name is not NULL. For more information about tokenize, see the unitex manual.
 *
 * \param [in/out] text_name the name of the text
 * \param [in] alphabet the name of the alphabet
 * \param [in/out] token_txt_name the file containing all the of the text or
 *
 *
 *
 *
 */
int launch_tokenize_in_Cassys(const char *text_name, const char *alphabet_name, const char *token_txt_name,
    VersatileEncodingConfig* vec,
    vector_ptr* additional_args){

	u_printf("Launch tokenize in Cassys \n");
	ProgramInvoker *invoker = new_ProgramInvoker(main_Tokenize,"main_Tokenize");

    char tmp[FILENAME_MAX+20];
    {
        tmp[0]=0;
        get_reading_encoding_text(tmp,sizeof(tmp)-1,vec->mask_encoding_compatibility_input);
        if (tmp[0] != '\0') {
            add_argument(invoker,"-k");
            add_argument(invoker,tmp);
        }

        tmp[0]=0;
        get_writing_encoding_text(tmp,sizeof(tmp)-1,vec->encoding_output,vec->bom_output);
        if (tmp[0] != '\0') {
            add_argument(invoker,"-q");
            add_argument(invoker,tmp);
        }
    }

	// add the alphabet
	sprintf(tmp, "--alphabet=%s", alphabet_name);
	add_argument(invoker, tmp);

	// Tokenize word by word
	add_argument(invoker, "--word_by_word");

	// add the target text file
	add_argument(invoker,text_name);

	// if a token.txt file already exists, use it
	if(token_txt_name != NULL){
		sprintf(tmp,"--tokens=%s",token_txt_name);
		add_argument(invoker,tmp);
	}


	for (int i = 0; i<((additional_args == NULL) ? 0 : (additional_args->nbelems)); i++) {
		add_argument(invoker, (const char*)additional_args->tab[i]);
	}

	char* line_command = build_command_line_alloc(invoker);
	u_printf("%s\n", line_command);

	int result = invoke(invoker);
	free_command_line_alloc(line_command);
	free_ProgramInvoker(invoker);

	return result;
}






/**
 * \brief Calls the Locate program in Cassys
 *
 *	Locate is called with target the transducer file name of transudcer and options
 *  --text=text_name, --alphabet=alphabet_name, --longest_matches, --all and --merge or --replace
 *  depending of the output policy of the transducer.
 *
 *  For more information about Locate, see the unitex manual.
 *
 * \param [in/out] text_name the name of the text
 * \param [in] alphabet the name of the alphabet
 * \param [in] transducer structure containing information about the transducer to be applied
 *
 */
int launch_locate_in_Cassys(const char *text_name, const transducer *transducer, const char* alphabet_name,
    const char*negation_operator,
    const VersatileEncodingConfig* vec,
    const char *morpho_dic,
    vector_ptr* additional_args){

	ProgramInvoker *invoker = new_ProgramInvoker(main_Locate, "main_Locate");

    char tmp[FILENAME_MAX + 20];
    {
        tmp[0]=0;
        get_reading_encoding_text(tmp,sizeof(tmp)-1,vec->mask_encoding_compatibility_input);
        if (tmp[0] != '\0') {
            add_argument(invoker,"-k");
            add_argument(invoker,tmp);
        }

        tmp[0]=0;
        get_writing_encoding_text(tmp,sizeof(tmp)-1,vec->encoding_output,vec->bom_output);
        if (tmp[0] != '\0') {
            add_argument(invoker,"-q");
            add_argument(invoker,tmp);
        }
    }

    add_argument(invoker, transducer->transducer_file_name);

	// add the text
	sprintf(tmp,"--text=%s",text_name);
	add_argument(invoker, tmp);

	// add the merge or replace option
	switch (transducer ->output_policy) {
	   case MERGE_OUTPUTS: add_argument(invoker,"--merge"); break;
	   case REPLACE_OUTPUTS: add_argument(invoker,"--replace"); break;
	   default: add_argument(invoker,"--ignore"); break;
	}

	// add the alphabet
	sprintf(tmp,"--alphabet=%s",alphabet_name);
	add_argument(invoker, tmp);

	// look for the longest match argument
	add_argument(invoker, "--longest_matches");

	// look for all the occurrences
	add_argument(invoker, "--all");

	if(morpho_dic != NULL){
		sprintf(tmp,"--morpho=%s",morpho_dic);
		add_argument(invoker,tmp);
	}

    if ((*negation_operator) != 0) {
        sprintf(tmp,"--negation_operator=%s",negation_operator);
        add_argument(invoker,tmp);
    }


	for (int i = 0; i<((additional_args == NULL) ? 0 : (additional_args->nbelems)); i++) {
		add_argument(invoker, (const char*)additional_args->tab[i]);
	}

	char* line_command = build_command_line_alloc(invoker);
	u_printf("%s\n", line_command);

	int result = invoke(invoker);
	free_command_line_alloc(line_command);
	free_ProgramInvoker(invoker);

	return result;
}


/**
 * \brief Calls the Grf2fst2 program in Cassys
 *
 * Grf2fst2 is called with target graph (text_name) and option
 * --alphabet=alphabet_name
 *
 *  For more information about Grf2fst2, see the unitex manuak.
 *
 *  \param [in] text_name the name of the graph
 *  \param [in] alphabet the name of the alphabet
 *
 */

int launch_grf2fst2_in_Cassys(const char *text_name, const char *alphabet_name, VersatileEncodingConfig *vec, vector_ptr *additional_args) {
    ProgramInvoker *invoker = new_ProgramInvoker(main_Grf2Fst2, "main_Grf2Fst2");

    char tmp[FILENAME_MAX + 20];
	{
	    tmp[0] = 0;
	    get_reading_encoding_text(tmp, sizeof(tmp) - 1, vec->mask_encoding_compatibility_input);
	    if(tmp[0] != '\0') {
		add_argument(invoker,"-k");
		add_argument(invoker,tmp);
	    }
	    tmp[0] = 0;
	    get_writing_encoding_text(tmp, sizeof(tmp) - 1, vec->encoding_output, vec->bom_output);
	    if(tmp[0] != '\0') {
		add_argument(invoker,"-q");
		add_argument(invoker,tmp);
	    }
	}

	add_argument(invoker,text_name);
	add_argument(invoker,"-y");

	sprintf(tmp,"--alphabet=%s",alphabet_name);
	add_argument(invoker,tmp);

	for (int arg = 0; arg<((additional_args == NULL) ? 0 : (additional_args->nbelems)); arg++) {
		add_argument(invoker, (const char*)additional_args->tab[arg]);
	}

	char *line_command = build_command_line_alloc(invoker);
	u_printf("%s\n",line_command);

	int result = invoke(invoker);
	free_ProgramInvoker(invoker);
	free_command_line_alloc(line_command);
	return result;
}

/**
 * \brief Calls the Concord program in Cassys
 *
 *	Concord is called with target index_file and options
 *  --merge=text_name, --alphabet=alphabet_name.
 *
 *  For more information about Concord, see the unitex manual.
 *
 * \param [in/out] text_name the name of the text
 * \param [in] alphabet the name of the alphabet
 * \param [in] index_file file containing all the matches found by locate
 *
 */
int launch_concord_in_Cassys(const char *text_name, const char *index_file, const char *alphabet_name,
	const char *uima_name, const char *output_offsets_name,
	VersatileEncodingConfig* vec,
	vector_ptr* additional_args){
	ProgramInvoker *invoker = new_ProgramInvoker(main_Concord, "main_Concord");

	// verify the braces in concordance
			U_FILE *concord;
			unichar* line = NULL;
			size_t size_buffer_line = 0;
			int brace_level;
			int i;
			int l;

			concord = u_fopen(vec, index_file, U_READ);
			if( concord == NULL){
				fatal_error("Cannot open file %s\n",index_file);
				exit(1);
			}
			while (u_fgets_dynamic_buffer(&line, &size_buffer_line, concord) != EOF) {
				

				brace_level = 0;
				i=0;
				l=u_strlen(line);
				while(i<l) {
					if(line[i]=='{')
						brace_level++;
					else if(line[i]=='}')
						brace_level--;
					i++;
				}
				if( brace_level!=0){
					error("File %s\nProblem of brackets in line %S\n", index_file, line);
					fatal_error("cassys_tokenize_word_by_word : correct the current graph if possible.\n");
				}
			}
			u_fclose(concord);

	// end of veifying braces

	add_argument(invoker,index_file);

    char tmp[FILENAME_MAX + 20];
    {
        tmp[0]=0;
        get_reading_encoding_text(tmp,sizeof(tmp)-1,vec->mask_encoding_compatibility_input);
        if (tmp[0] != '\0') {
            add_argument(invoker,"-k");
            add_argument(invoker,tmp);
        }

        tmp[0]=0;
        get_writing_encoding_text(tmp,sizeof(tmp)-1,vec->encoding_output,vec->bom_output);
        if (tmp[0] != '\0') {
            add_argument(invoker,"-q");
            add_argument(invoker,tmp);
        }
    }

	if ((uima_name != NULL) && (uima_name[0] != '\0')) {
		sprintf(tmp, "--uima=%s", uima_name);
		add_argument(invoker, tmp);
	}

	sprintf(tmp,"--merge=%s",text_name);
	add_argument(invoker,tmp);


	if ((output_offsets_name != NULL) && (output_offsets_name[0] != '\0')) {
		sprintf(tmp, "--output_offsets=%s", output_offsets_name);
		add_argument(invoker, tmp);
	}


	sprintf(tmp,"--alphabet=%s",alphabet_name);
	add_argument(invoker, tmp);

	for (int arg = 0; arg<((additional_args == NULL) ? 0 : (additional_args->nbelems)); arg++) {
		add_argument(invoker, (const char*)additional_args->tab[arg]);
	}

	char* line_command = build_command_line_alloc(invoker);
	u_printf("%s\n", line_command);

	int result = invoke(invoker);
	free_command_line_alloc(line_command);
	free_ProgramInvoker(invoker);
	if (line != NULL) {
		free(line);
	}
	return result;
}

}
