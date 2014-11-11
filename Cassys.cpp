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
 *
 *  Created on: 29 avr. 2010
 *  Authors: David Nott, Nathalie Friburger (nathalie.friburger@univ-tours.fr)
 */

#include "Text_tokens.h"
#include "Cassys.h"
#include <ctype.h>
#include "File.h"
#include "Snt.h"
#include "List_ustring.h"
#include "Tokenization.h"
#include "Copyright.h"
#include "DirHelper.h"

#include "Cassys_lexical_tags.h"
#include "Cassys_concord.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {




const char *optstring_Cassys = ":t:a:w:l:hk:q:g:dvuNnm:s:ir:";
const struct option_TS lopts_Cassys[] = {
        {"text", required_argument_TS, NULL, 't'},
        {"alphabet", required_argument_TS, NULL, 'a'},
        {"morpho",required_argument_TS,NULL,'w'},
        {"transducers_list", required_argument_TS, NULL,'l'},
        {"input_encoding",required_argument_TS,NULL,'k'},
        {"output_encoding",required_argument_TS,NULL,'q'},
        {"no_create_directory",no_argument_TS,NULL,'d'},
        {"negation_operator",required_argument_TS,NULL,'g'},

        {"transducer_policy",required_argument_TS,NULL,'m'},
        {"transducer_file",required_argument_TS,NULL,'s'},
        {"transducer_dir",required_argument_TS,NULL,'r'},
        {"in_place", no_argument_TS,NULL,'i'},
        {"dump_token_graph", no_argument_TS, NULL, 'u' },
        {"no_dump_token_graph", no_argument_TS, NULL, 'N' },
        {"realign_token_graph_pointer", no_argument_TS, NULL, 'n' },
        {"translate_path_separator_to_native", no_argument_TS, NULL, 'v' },
        {"help", no_argument_TS,NULL,'h'}
};

const char* usage_Cassys =
        "Usage : Cassys [options]\n"
        "\n"
        "OPTION :\n"
        "-a ALPH/--alphabet=ALPH: the language alphabet file\n"
        "-r X/--transducer_dir=X: take transducer on directory X (so you don't specify \n"
        "      full path for each transducer; note that X must be (back)slash terminated\n"
        "-w DIC/--morpho=DIC: specifies that DIC is a .bin dictionary\n"
                 "                       to use in morphological mode. Use as many\n"
                 "                       -m XXX as there are .bin to use. You can also\n"
                 "                       separate several .bin with semi-colons.\n"
        "-l TRANSDUCERS_LIST/--transducers_list=TRANSDUCERS_LIST the transducers list file with their output policy\n"
        "-s transducer.fst2/--transducer_file=transducer.fst2 a transducer to apply\n"
        "-m output_policy/--transducer_policy=output_policy the output policy of the transducer specified\n"
        "-t TXT/--text=TXT the text file to be modified, with extension .snt\n"
        "-i/--in_place mean uses the same csc/snt directories for each transducer\n"
        "-u/--dump_token_graph create a .dot file with graph dump infos\n"
        "-N/--no_dump_token_graph create a .dot file with graph dump infos\n"
        "-n/--realign_token_graph_pointer create a.dot file will not depends to pointer allocation to be deterministic\n"
        "-v/--translate_path_separator_to_native replace path separator in csc by native separator for portable csc file\n"
        "-d/--no_create_directory mean the all snt/csc directories already exist and don't need to be created\n"
        "  -g minus/--negation_operator=minus: uses minus as negation operator for Unitex 2.0 graphs\n"
        "  -g tilde/--negation_operator=tilde: uses tilde as negation operator (default)\n"
        "-h/--help display this help\n"
        "\n"
        "Applies a list of grammar to a text and saves the matching sequence index in a\n"
         "file named \"concord.ind\" stored in the text directory.\n\n"
         "The target text file has to be a preprocessed snt file with its _snt/ directory.\n"
         "The transducer list file is a file in which each line contains the path to a transducer.\n"
         "followed by the output policy to be applied to this transducer.\n"
         "Instead a list file, you can specify each file and each output policy by a set of couple\n"
         "of -s/--transducer_file and -m/--transducer_policy argument to enumerate the list\n"
         "The policy may be MERGE or REPLACE.\n"
         "The file option, the alphabet option and the transducer list file option are mandatory.\n"
         "\n";



static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Cassys);
}


int main_Cassys(int argc,char* const argv[]) {
    if (argc==1) {
        usage();
        return 0;
    }


    char* morpho_dic=NULL;

    char transducer_list_file_name[FILENAME_MAX];
    bool has_transducer_list = false;

    char text_file_name[FILENAME_MAX];
    bool has_text_file_name = false;

    char alphabet_file_name[FILENAME_MAX];
    char transducer_filename_prefix[FILENAME_MAX];
    bool has_alphabet = false;
    char negation_operator[0x20];

    VersatileEncodingConfig vec=VEC_DEFAULT;
    int must_create_directory = 1;
    int in_place = 0;
    int realign_token_graph_pointer = 0;
    int translate_path_separator_to_native = 0;
    int dump_graph = 0; // By default, don't build a .dot file.
    struct transducer_name_and_mode_linked_list* transducer_name_and_mode_linked_list_arg=NULL;

    // decode the command line
    int val;
    int index = 1;
    negation_operator[0]='\0';
    transducer_filename_prefix[0]='\0';
    struct OptVars* vars=new_OptVars();
    while (EOF != (val = getopt_long_TS(argc, argv, optstring_Cassys,
            lopts_Cassys, &index, vars))) {
        switch (val) {
        case 'h': usage();
                  free_OptVars(vars);
                  free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                  if (morpho_dic != NULL) {
                     free(morpho_dic);
                  }
                  return 0;
        case 'k': if (vars->optarg[0]=='\0') {
                fatal_error("Empty input_encoding argument\n");
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),vars->optarg);
             break;
        case 'q': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_encoding argument\n");
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),vars->optarg);
             break;
        case 't': {
            if (vars -> optarg[0] == '\0') {
                fatal_error("Command line error : Empty file name argument\n");
            }

            char extension_text_name[FILENAME_MAX];
            get_extension(vars -> optarg, extension_text_name);
            if (strcmp(extension_text_name, ".snt") != 0) {
                fatal_error(
                        "Command line error : File name argument %s must be a preprocessed snt file\n",
                        vars -> optarg);
            }

            strcpy(text_file_name, vars -> optarg);
            has_text_file_name = true;

            break;
        }
        case 'l': {
            if(vars -> optarg[0] == '\0'){
                fatal_error("Command line error : Empty transducer list argument\n");
            } else {
                strcpy(transducer_list_file_name, vars -> optarg);
                has_transducer_list = true;
            }
            break;
        }
        case 'r': {
            if(vars -> optarg[0] == '\0'){
                fatal_error("Command line error : Empty transducer directory argument\n");
            } else {
                strcpy(transducer_filename_prefix, vars -> optarg);
                has_transducer_list = true;
            }
            break;
        }
        case 's': {
            if(vars -> optarg[0] == '\0'){
                fatal_error("Command line error : Empty transducer filename argument\n");
            } else {
                transducer_name_and_mode_linked_list_arg=add_transducer_linked_list_new_name(transducer_name_and_mode_linked_list_arg,vars -> optarg);
            }
            break;
        }
        case 'm': {
            if(vars -> optarg[0] == '\0'){
                fatal_error("Command line error : Empty transducer mode argument\n");
            } else {
                set_last_transducer_linked_list_mode_by_string(transducer_name_and_mode_linked_list_arg,vars -> optarg);
            }
            break;
        }
        case 'a':{
            if (vars -> optarg[0] == '\0') {
                fatal_error("Command line error : Empty alphabet argument\n");
            } else {
                strcpy(alphabet_file_name, vars -> optarg);
                has_alphabet = true;
            }
            break;
        }
        case 'g': if (vars->optarg[0]=='\0') {
                fatal_error("You must specify an argument for negation operator\n");
             }
             if ((strcmp(vars->optarg,"minus")!=0) && (strcmp(vars->optarg,"-")!=0) &&
                 (strcmp(vars->optarg,"tilde")!=0) && (strcmp(vars->optarg,"~")!=0))
             {
                 fatal_error("You must specify a valid argument for negation operator\n");
             }
             strcpy(negation_operator,vars->optarg);
             break;
        case 'i': {
            in_place = 1;
            break;
        }
        case 'u': {
            dump_graph = 1;
            break;
        }
        case 'N': {
            dump_graph = 0;
            break;
        }
        case 'n': {
            realign_token_graph_pointer = dump_graph = 1;
            break;
        }
        case 'v': {
            translate_path_separator_to_native = 1;
            break;
        }
        case 'd': {
            must_create_directory = 0;
            break;
        }
        case 'w' : {
            if (vars->optarg[0] != '\0') {
                if (morpho_dic == NULL) {
                    morpho_dic = strdup(vars->optarg);
                    if (morpho_dic == NULL) {
                        fatal_alloc_error("main_Cassys");
                    }
                } else {
                    morpho_dic = (char*) realloc((void*) morpho_dic, strlen(
                            morpho_dic) + strlen(vars->optarg) + 2);
                    if (morpho_dic == NULL) {
                        fatal_alloc_error("main_Cassys");
                    }
                    strcat(morpho_dic, ";");
                    strcat(morpho_dic, vars->optarg);
                }
            }
            break;
        }
        default :{
            fatal_error("Unknown option : %c\n",val);
            break;
        }
        }
    }
    index = -1;

    if(has_alphabet == false){
        fatal_error("Command line error : no alphabet provided\nRerun with --help\n");
    }
    if(has_text_file_name == false){
        fatal_error("Command line error : no text file provided\nRerun with --help\n");
    }
    if((has_transducer_list == false) && (transducer_name_and_mode_linked_list_arg == NULL)){
        fatal_error("Command line error : no transducer list provided\nRerun with --help\n");
    }



    // Load the list of transducers from the file transducer list and stores it in a list
    //struct fifo *transducer_list = load_transducer(transducer_list_file_name);
    if ((transducer_name_and_mode_linked_list_arg == NULL) && has_transducer_list)
        transducer_name_and_mode_linked_list_arg = load_transducer_list_file(transducer_list_file_name, translate_path_separator_to_native);
    struct fifo *transducer_list=load_transducer_from_linked_list(transducer_name_and_mode_linked_list_arg,transducer_filename_prefix);

	cascade(text_file_name, in_place, must_create_directory, transducer_list, alphabet_file_name, negation_operator, &vec, morpho_dic, dump_graph, realign_token_graph_pointer);

    if(morpho_dic != NULL){
        free(morpho_dic);
    }
    free_fifo(transducer_list);
    free_OptVars(vars);
    free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
    return 0;
}


/**
 * The main function of the cascade
 *
 *
 */
int cascade(const char* text, int in_place, int must_create_directory, fifo* transducer_list, const char *alphabet,
    const char*negation_operator,
    VersatileEncodingConfig* vec,
    const char *morpho_dic, int dump_graph, int realign_token_graph_pointer) {

	cassys_tokens_allocation_tool* tokens_allocation_tool = build_cassys_tokens_allocation_tool();

    launch_tokenize_in_Cassys(text,alphabet,NULL,vec);

    //if (in_place == 0)
        initialize_working_directory(text, must_create_directory);

    struct snt_files *snt_text_files = new_snt_files(text);

    struct text_tokens *tokens = NULL;
    cassys_tokens_list *tokens_list = cassys_load_text(vec,snt_text_files->tokens_txt, snt_text_files->text_cod,&tokens, tokens_allocation_tool);

    u_printf("CasSys Cascade begins\n");

    int transducer_number = 1;
    char *labeled_text_name = NULL;
    char last_labeled_text_name[FILENAME_MAX];

    if (in_place != 0){
       labeled_text_name = create_labeled_files_and_directory(text,
            0,0,0,0, must_create_directory,0);
    }

    int previous_transducer_number = 0;
    int previous_iteration = 0;
    int iteration = 0;
    while (!is_empty(transducer_list)) {

        transducer *current_transducer =
                (transducer*) take_ptr(transducer_list);

        if(is_debug_mode(current_transducer, vec) == true){
            fatal_error("graph %s has been compiled in debug mode. Please recompile it in normal mode\n", current_transducer->transducer_file_name);
        }

        for (iteration = 0; current_transducer->repeat_mode == INFINITY || iteration < current_transducer->repeat_mode; iteration++) {

            if (in_place == 0) {
                labeled_text_name = create_labeled_files_and_directory(text, previous_transducer_number,
                        transducer_number, previous_iteration, iteration, must_create_directory, 1);
            }



            launch_tokenize_in_Cassys(labeled_text_name, alphabet,
                    snt_text_files->tokens_txt, vec);
            free_snt_files(snt_text_files);

            // apply transducer

            u_printf("Applying transducer %s (numbered %d)\n",
                    current_transducer->transducer_file_name, transducer_number);
            launch_locate_in_Cassys(labeled_text_name, current_transducer,
                    alphabet, negation_operator, vec, morpho_dic);

            // add protection character in lexical tags when needed
            snt_text_files = new_snt_files(labeled_text_name);
            protect_lexical_tag_in_concord(snt_text_files->concord_ind, current_transducer->output_policy, vec);
            // generate concordance for this transducer
            launch_concord_in_Cassys(labeled_text_name,
                    snt_text_files -> concord_ind, alphabet, vec);

            //
            add_replaced_text(labeled_text_name, tokens_list, previous_transducer_number, previous_iteration,
                    transducer_number, iteration, alphabet, vec, tokens_allocation_tool);


            previous_transducer_number = transducer_number;
            previous_iteration = iteration;

            sprintf(last_labeled_text_name, "%s", labeled_text_name);

            if (in_place == 0) {
                free(labeled_text_name);
            }

            if (count_concordance(snt_text_files->concord_ind, vec) == 0) {
                break;
            } else {
                u_printf("transducer %s\n %d iteration %d --> %d concordances\n",
                        current_transducer->transducer_file_name,
                        transducer_number,
                        iteration,
                        count_concordance(snt_text_files->concord_ind, vec));
//              char graph_file_name_n[FILENAME_MAX];
//                  sprintf(graph_file_name_n,"%s.dot", last_labeled_text_name);
//                  u_printf("writing graph = %s\n",graph_file_name_n);
//                  cassys_tokens_2_graph(tokens_list,graph_file_name_n, realign_token_graph_pointer);


                //u_printf("Displaying sparse matrix\n");
                //display_text(tokens_list, transducer_number);
            }
        }


        free(current_transducer -> transducer_file_name);
        free(current_transducer);

        transducer_number++;
    }


    free_snt_files(snt_text_files);

    // create the concord file with XML
    construct_cascade_concord(tokens_list,text,transducer_number, iteration, vec);
    // construct_xml_concord must be applied to create the xmlized concordance
    construct_xml_concord(text,vec);

    struct snt_files *snt_files = new_snt_files(text);

    char result_file_name_XML[FILENAME_MAX];
    char text_name_without_extension[FILENAME_MAX];
    remove_extension(text,text_name_without_extension);
    sprintf(result_file_name_XML,"%s_csc.txt",text_name_without_extension);

    // make a copy of the last resulting text of the cascade in the file named _csc.txt
    // this result his in XML form
    char path[FILENAME_MAX];
    get_path(text,path);
    char last_resulting_text_path[FILENAME_MAX];
    char result_file_name_path_XML[FILENAME_MAX];
    sprintf(last_resulting_text_path,"%s",last_labeled_text_name);
    sprintf(result_file_name_path_XML,"%s", result_file_name_XML);
    copy_file(result_file_name_path_XML, last_resulting_text_path);

    // create the text file including XMLized concordance
    launch_concord_in_Cassys(result_file_name_path_XML, snt_files->concord_ind, alphabet, vec);

    // make a copy of the last resulting text of the cascade in the file named _csc.raw
    char result_file_name_raw[FILENAME_MAX];
    sprintf(result_file_name_raw,"%s_csc.raw",text_name_without_extension);
    char result_file_name_path_raw[FILENAME_MAX];
    sprintf(result_file_name_path_raw,"%s", result_file_name_raw);
    copy_file(result_file_name_path_raw, last_resulting_text_path);

    // relaunch the construction of the concord file without XML
    construct_cascade_concord(tokens_list,text,transducer_number, iteration, vec);
    // relaunch the construction of the text file without XML
    launch_concord_in_Cassys(result_file_name_path_raw, snt_files->concord_ind, alphabet, vec);

    if (dump_graph)
    {
        char graph_file_name[FILENAME_MAX];
        sprintf(graph_file_name, "%s.dot", text_name_without_extension);
        cassys_tokens_2_graph(tokens_list, graph_file_name, realign_token_graph_pointer);
    }

    if (in_place != 0)
      free(labeled_text_name);


    //free_cassys_tokens_list(tokens_list);
    free_snt_files(snt_files);
    free_text_tokens(tokens);
	free_cassys_tokens_allocation_tool(tokens_allocation_tool);
    return 0;
}

} // namespace unitex
