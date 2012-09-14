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

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define CASSYS_DIRECTORY_EXTENSION "_csc"


const char *optstring_Cassys = ":t:a:w:l:hk:q:g:dm:s:ir:";
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
		{"help", no_argument_TS,NULL,'h'}
};

const char* usage_Cassys =
		"Usage : Cassys [options]\n"
		"\n"
		"OPTION :\n"
		"-a ALPH/--alphabet=ALPH: the language alphabet file\n"
        "-r X/--transducer_dir=X: take tranducer on directory X (so you don't specify \n"
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

OutputPolicy GetOutputPolicyFromString(const char*option_name)
{
	if (strcmp(option_name, "M") == 0 || strcmp(option_name, "MERGE") == 0 || strcmp(
			option_name, "Merge") == 0) {
		return MERGE_OUTPUTS;
	}
	if (strcmp(option_name, "R") == 0 || strcmp(option_name, "REPLACE") == 0
			|| strcmp(option_name, "Replace") == 0) {
		return REPLACE_OUTPUTS;
	}
	return IGNORE_OUTPUTS;
}


struct transducer_name_and_mode_linked_list* add_transducer_linked_list_new_name(struct transducer_name_and_mode_linked_list *current_list,const char*filename)
{
    struct transducer_name_and_mode_linked_list* new_item=(struct transducer_name_and_mode_linked_list*)malloc(sizeof(struct transducer_name_and_mode_linked_list));
    if (new_item==NULL) {
		fatal_alloc_error("add_transducer_linked_list_new_name");
		exit(1);
	}
    
    new_item->transducer_filename = strdup(filename);
    new_item->transducer_mode=IGNORE_OUTPUTS;
    new_item->next=NULL;
    if (new_item->transducer_filename==NULL) {
		fatal_alloc_error("add_transducer_linked_list_new_name");
		exit(1);
	}

    if (current_list==NULL)
        return new_item;

    struct transducer_name_and_mode_linked_list *browse_current_list = current_list;

    while (browse_current_list->next != NULL)
        browse_current_list = browse_current_list->next;
    browse_current_list->next=new_item;
    return current_list;
}

void set_last_transducer_linked_list_mode(struct transducer_name_and_mode_linked_list *current_list,OutputPolicy mode)
{
    struct transducer_name_and_mode_linked_list *browse_current_list = current_list;
    if (current_list == NULL)
        return;
    while (browse_current_list->next != NULL)
        browse_current_list = browse_current_list->next;
    browse_current_list->transducer_mode = mode;
}

void set_last_transducer_linked_list_mode_by_string(struct transducer_name_and_mode_linked_list *current_list,const char*option_name)
{
    set_last_transducer_linked_list_mode(current_list,GetOutputPolicyFromString(option_name));
}

void free_transducer_name_and_mode_linked_list(struct transducer_name_and_mode_linked_list *list)
{
    while (list!=NULL) {
        struct transducer_name_and_mode_linked_list *list_next = list->next;
        free(list->transducer_filename);
        free(list);
        list = list_next;
    }
}




struct transducer_name_and_mode_linked_list *load_transducer_list_file(const char *transducer_list_name) {

	U_FILE *file_transducer_list;
    struct transducer_name_and_mode_linked_list * res=NULL;

	file_transducer_list = u_fopen(ASCII, transducer_list_name,U_READ);
	if( file_transducer_list == NULL){
		fatal_error("Cannot open file %s\n",transducer_list_name);
		exit(1);
	}

    char line[1024];
    int i=1;
	while (cassys_fgets(line,1024,file_transducer_list) != NULL){
		char *transducer_file_name;
		OutputPolicy transducer_policy;	

		remove_cassys_comments(line);

		transducer_file_name = extract_cassys_transducer_name(line);
		//fprintf(stdout, "transducer name read =%s\n",transducer_file_name);

		transducer_policy = extract_cassys_transducer_policy(line);


		if (transducer_file_name != NULL && transducer_policy != IGNORE_OUTPUTS) {
			res=add_transducer_linked_list_new_name(res,transducer_file_name);
            set_last_transducer_linked_list_mode(res,transducer_policy);
		}
		else {
			if (transducer_file_name == NULL) {
				fprintf(stdout, "Line %d : Empty line\n",i);
			} else if (transducer_policy == IGNORE_OUTPUTS) {
				fprintf(stdout, "Line %d : Transducer policy not recognized\n",i);
			}
		}
        free(transducer_file_name);
		i++;
	}
    u_fclose(file_transducer_list);

	return res;
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
        transducer_name_and_mode_linked_list_arg = load_transducer_list_file(transducer_list_file_name);
    struct fifo *transducer_list=load_transducer_from_linked_list(transducer_name_and_mode_linked_list_arg,transducer_filename_prefix);

	cascade(text_file_name, in_place, must_create_directory, transducer_list, alphabet_file_name,negation_operator,&vec, morpho_dic);

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
    char *morpho_dic) {

	launch_tokenize_in_Cassys(text,alphabet,NULL,vec);

    //if (in_place == 0)
        initialize_working_directory(text, must_create_directory);

	struct snt_files *snt_text_files = new_snt_files(text);

	struct text_tokens *tokens = NULL;
	cassys_tokens_list *tokens_list = cassys_load_text(vec,snt_text_files->tokens_txt, snt_text_files->text_cod,&tokens);

	fprintf(stdout,"CasSys Cascade begins\n");

	int transducer_number = 1;
    char *labeled_text_name = NULL;
    char last_labeled_text_name[FILENAME_MAX];

    if ((in_place != 0))
	   labeled_text_name = create_labeled_files_and_directory(text,
		    transducer_number*0, must_create_directory,0);


	while(!is_empty(transducer_list)){

        if (in_place == 0)
		    labeled_text_name = create_labeled_files_and_directory(text,
				    transducer_number, must_create_directory,1);
        /*
        else
        {
            labeled_text_name = strdup(text);
        }*/

		launch_tokenize_in_Cassys(labeled_text_name,alphabet,snt_text_files->tokens_txt,vec);
		free_snt_files(snt_text_files);

		// apply transducer
		transducer *current_transducer = (transducer*)take_ptr(transducer_list);
		fprintf(stdout,"Applying transducer %s (numbered %d)\n", current_transducer->transducer_file_name,  transducer_number);
		launch_locate_in_Cassys(labeled_text_name, current_transducer, alphabet, negation_operator,vec,morpho_dic);

		// generate concordance for this transducer
		snt_text_files = new_snt_files(labeled_text_name);
		launch_concord_in_Cassys(labeled_text_name,
				snt_text_files -> concord_ind, alphabet,vec);

		//
		add_replaced_text(labeled_text_name,tokens_list,transducer_number,alphabet,vec);

		// add protection character in braces when needed
		protect_special_characters(labeled_text_name,vec);

		transducer_number++;

		free(current_transducer -> transducer_file_name);
		free(current_transducer);

		sprintf(last_labeled_text_name, "%s", labeled_text_name);
        if (in_place == 0)
		       free(labeled_text_name);

	}
    if ((in_place != 0))
		    free(labeled_text_name);

	free_snt_files(snt_text_files);

	construct_cascade_concord(tokens_list,text,transducer_number,vec);


	struct snt_files *snt_files = new_snt_files(text);

	char result_file_name[FILENAME_MAX];
	char text_name_without_extension[FILENAME_MAX];
	remove_extension(text,text_name_without_extension);
	sprintf(result_file_name,"%s_csc.txt",text_name_without_extension);

	// make a copy of the last resulting text of the cascade in the file named text.csc (in the same directory than text.snt)
	char path[FILENAME_MAX];
	get_path(text,path);
	char last_resulting_text_path[FILENAME_MAX];
	char result_file_name_path[FILENAME_MAX];
	sprintf(last_resulting_text_path,"%s",last_labeled_text_name);
	sprintf(result_file_name_path,"%s", result_file_name);
	copy_file(result_file_name_path, last_resulting_text_path);
	//copy_file(result_file_name,text);
	//launch_concord_in_Cassys(result_file_name,snt_files->concord_ind,alphabet,vec);

    free_cassys_tokens_list(tokens_list);
	free_snt_files(snt_files);
	free_text_tokens(tokens);
	return 0;
}


cassys_tokens_list *cassys_load_text(const VersatileEncodingConfig* vec,const char *tokens_text_name, const char *text_cod_name, struct text_tokens **tokens){

	*tokens = load_text_tokens(vec,tokens_text_name);

	U_FILE *f = u_fopen(BINARY, text_cod_name,U_READ);
	if( f == NULL){
		fatal_error("Cannot open file %s\n",text_cod_name);
		exit(1);
	}

	cassys_tokens_list *list = NULL;
	cassys_tokens_list *temp = list;

	int token_id;
	int char_read = (int)fread(&token_id,sizeof(int),1,f);
	while(char_read ==1){
		if(list==NULL){
			list = new_element((*tokens)->token[token_id],0);
			temp = list;
		}
		else {
			temp ->next_token = new_element((*tokens)->token[token_id],0);
			temp = temp -> next_token;
		}

		char_read = (int)fread(&token_id,sizeof(int),1,f);
	}
	u_fclose(f);

	return list;
}


cassys_tokens_list *add_replaced_text( const char *text, cassys_tokens_list *list,
		 int transducer_id, const char *alphabet_name, const VersatileEncodingConfig* vec) {

	locate_pos *prev_l=NULL;
	Alphabet *alphabet = load_alphabet(vec,alphabet_name);

	struct snt_files *snt_text_files = new_snt_files(text);

	struct fifo *stage_concord = read_concord_file(snt_text_files->concord_ind,vec);

	// performance enhancement
	cassys_tokens_list *current_list_position = list;
	long current_token_position = 0;

	while (!is_empty(stage_concord)) {

		locate_pos *l=(locate_pos*)take_ptr(stage_concord);
		if(prev_l!=NULL){ // manage the fact that when writing a text merging the concord.ind,
			// and when there is more than one pattern beginning at the same position in the text,
			// the behavior of concord.exe is to take the first of those patterns.
			// when we create the concordance of a cascade, it is needed to chose the same path (the first)
			// as in concord.exe
			if(prev_l->token_start_offset==l->token_start_offset){
				while(prev_l!=NULL && l!=NULL && prev_l->token_start_offset==l->token_start_offset){
					free(prev_l->label);
					free(prev_l);
					prev_l=l;
					if(!is_empty(stage_concord))
						l=(locate_pos*)take_ptr(stage_concord);
					else l=NULL;
				}
			}
			else {
				free(prev_l->label);
				free(prev_l);
				prev_l=NULL;
			}
		}
		if(l!=NULL){
			struct list_ustring *new_sentence_lu = cassys_tokenize_word_by_word(l->label, alphabet);
			cassys_tokens_list *new_sentence_ctl =
				new_list(new_sentence_lu, transducer_id);

			// performance enhancement :
			// Since matches are sorted, we begin the search from the last known position in the list.
			// We have to substract from the text position the current token position.
			cassys_tokens_list *list_position = get_element_at(current_list_position, transducer_id - 1,
				l->token_start_offset - current_token_position);
			int replaced_sentence_length = l->token_end_offset - l->token_start_offset+1;
			int new_sentence_length = length(new_sentence_lu);

			add_output(list_position, new_sentence_ctl, transducer_id,
				replaced_sentence_length, new_sentence_length-1);

			// performance enhancement
			current_list_position = list_position;
			current_token_position = l-> token_start_offset;
			prev_l=l;
			free_list_ustring(new_sentence_lu);
		}
		if(l!=NULL && is_empty(stage_concord)){
			free(l->label);
			free(l);
		}
	}
	free_fifo(stage_concord);
	free_snt_files(snt_text_files);
    free_alphabet(alphabet);

	return list;
}




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


struct fifo *load_transducer_from_linked_list(const struct transducer_name_and_mode_linked_list *list,const char* transducer_filename_prefix){
	struct fifo *transducer_fifo = new_fifo();

	int i=1;
	while (list != NULL){
		char *transducer_file_name;
		OutputPolicy transducer_policy;
		transducer *t;

        transducer_file_name = list->transducer_filename;
		//fprintf(stdout, "transducer name read =%s\n",transducer_file_name);

        transducer_policy = list->transducer_mode;

		if (transducer_file_name != NULL && transducer_policy != IGNORE_OUTPUTS) {
			//fprintf(stdout,"transducer to be loaded\n");
			t = (transducer*) malloc(sizeof(transducer) * 1);
			if (t == NULL) {
				fatal_alloc_error("load_transducer_from_linked_list");
				exit(1);
			}
            size_t transducer_filename_prefix_len = 0;
            if (transducer_filename_prefix != NULL)
                transducer_filename_prefix_len = strlen(transducer_filename_prefix);
			t->transducer_file_name = (char*)malloc(sizeof(char)*(transducer_filename_prefix_len+strlen(transducer_file_name)+1));
			if(t->transducer_file_name == NULL){
				fatal_alloc_error("load_transducer_from_linked_list");
				exit(1);
			}

            t->transducer_file_name[0] = '\0';
            if (transducer_filename_prefix != NULL)
                strcpy(t->transducer_file_name, transducer_filename_prefix);
			strcat(t->transducer_file_name, transducer_file_name);

			t->output_policy = transducer_policy;


			struct any value;
			value._ptr = t;
			put_any(transducer_fifo,value);
			if (!is_empty(transducer_fifo)) {
				fprintf(stdout, "transducer %s successfully loaded\n",
						t->transducer_file_name);
			}
		}
		else {
			if (transducer_file_name == NULL) {
				fprintf(stdout, "Transducer %d : Empty filename\n",i);
			} else if (transducer_policy == IGNORE_OUTPUTS) {
				fprintf(stdout, "Transducer %d : Transducer mode not recognized\n",i);
			}
		}
		i++;
        list=list->next;
	}
    

	return transducer_fifo;
}

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
    VersatileEncodingConfig* vec){

	fprintf(stdout,"Launch tokenize in Cassys \n");
	ProgramInvoker *invoker = new_ProgramInvoker(main_Tokenize,"main_Tokenize");

    char tmp[FILENAME_MAX];
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
	char alphabet_argument[FILENAME_MAX + 11];
	sprintf(alphabet_argument, "--alphabet=%s", alphabet_name);
	add_argument(invoker, alphabet_argument);

	// Tokenize word by word
	add_argument(invoker, "--word_by_word");

	// add the target text file
	add_argument(invoker,text_name);

	// if a token.txt file already exists, use it
	if(token_txt_name != NULL){
		char token_argument[FILENAME_MAX + 9];
		sprintf(token_argument,"--tokens=%s",token_txt_name);
		add_argument(invoker,token_argument);
	}

	char line_command[4096];
	build_command_line(invoker, line_command);
	fprintf(stdout, "%s\n", line_command);

	int result = invoke(invoker);
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
    char *morpho_dic){

	ProgramInvoker *invoker = new_ProgramInvoker(main_Locate, "main_Locate");

    char tmp[FILENAME_MAX];
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
	char text_argument[FILENAME_MAX+7];
	sprintf(text_argument,"--text=%s",text_name);
	add_argument(invoker, text_argument);

	// add the merge or replace option
	switch (transducer ->output_policy) {
	   case MERGE_OUTPUTS: add_argument(invoker,"--merge"); break;
	   case REPLACE_OUTPUTS: add_argument(invoker,"--replace"); break;
	   default: add_argument(invoker,"--ignore"); break;
	}

	// add the alphabet
	char alphabet_argument[FILENAME_MAX+11];
	sprintf(alphabet_argument,"--alphabet=%s",alphabet_name);
	add_argument(invoker, alphabet_argument);

	// look for the longest match argument
	add_argument(invoker, "--longest_matches");

	// look for all the occurrences
	add_argument(invoker, "--all");

	if(morpho_dic != NULL){
		char morpho_dic_argument[FILENAME_MAX+20];
		sprintf(morpho_dic_argument,"--morpho=%s",morpho_dic);
		add_argument(invoker,morpho_dic_argument);
	}

    if ((*negation_operator) != 0) {
        char negation_operator_argument[0x40];
        sprintf(negation_operator_argument,"--negation_operator=%s",negation_operator);
        add_argument(invoker,negation_operator_argument);
    }

	char line_command[4096];
	build_command_line(invoker,line_command);
	fprintf(stdout, "%s\n",line_command);

	int result = invoke(invoker);
	free_ProgramInvoker(invoker);
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
    VersatileEncodingConfig* vec){
	ProgramInvoker *invoker = new_ProgramInvoker(main_Concord, "main_Concord");

	// verify the braces in concordance
			U_FILE *concord;
			unichar line[4096];
			int brace_level;
			int i;
			int l;

			//concord = u_fopen_exis(mask_encoding_compatibility_input, index_file,U_READ);
			concord = u_fopen(vec, index_file, U_READ);
			if( concord == NULL){
				fatal_error("Cannot open file %s\n",index_file);
				exit(1);
			}
			while(!u_feof(concord)){
				u_fgets(line, concord);
				brace_level=0;
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

    char tmp[FILENAME_MAX];
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

	char text_argument[FILENAME_MAX+7];
	sprintf(text_argument,"--merge=%s",text_name);
	add_argument(invoker,text_argument);

	char alphabet_argument[FILENAME_MAX+11];
	sprintf(alphabet_argument,"--alphabet=%s",alphabet_name);

	char line_command[4096];
	build_command_line(invoker, line_command);
	fprintf(stdout, "%s\n", line_command);

	int result = invoke(invoker);
	free_ProgramInvoker(invoker);
	return result;
}


/**
 * \brief Suppress Cassys comment line.
 */
void remove_cassys_comments(char *line){
	int i=0;
	while(line[i] != '\0' && line[i] != '#'){
		i++;
	}
	line[i]='\0';
}

/**
 *
 */
char* extract_cassys_transducer_name(const char *line){
	char *transducer_name;
	int i=0;
	while(line[i]!='"' && line[i] != '\0'){
		i++;
	}
	if(line[i] == '\0'){
		return NULL;
	}
	i++;
	int j=i;
	while(line[i]!='"' && line[i] != '\0'){
			i++;
		}
	if(line[i] == '\0'){
			return NULL;
		}

	transducer_name = (char*)malloc(sizeof(char)*((i-j)+1));
	if(transducer_name == NULL){
		fatal_alloc_error("extract_cassys_transducer_name");
		exit(1);
	}
	strncpy(transducer_name,line+j,(i-j));
	transducer_name[i-j]='\0';

	return transducer_name;
}


OutputPolicy extract_cassys_transducer_policy(const char *line) {
	int i = 0;
	while (line[i] != '"' && line[i] != '\0') {
		i++;
	}
	i++;
	while (line[i] != '"' && line[i] != '\0') {
		i++;
	}
	i++;
	while(isspace(line[i])){
		i++;
	}

	char option_name[FILENAME_MAX];
	int j=0;
	while(isalpha(line[i])){
		option_name[j]=line[i];
		i++;j++;
	}

	option_name[j]='\0';

	//fprintf(stdout,"extract option =%s\n",option_name);

	if (strcmp(option_name, "M") == 0 || strcmp(option_name, "MERGE") == 0 || strcmp(
			option_name, "Merge") == 0) {
		return MERGE_OUTPUTS;
	}
	if (strcmp(option_name, "R") == 0 || strcmp(option_name, "REPLACE") == 0
			|| strcmp(option_name, "Replace") == 0) {
		return REPLACE_OUTPUTS;
	}
	return IGNORE_OUTPUTS;
}



/**
 * \brief \b fgets working with \b U_FILE and storing \b char
 *
 * Needed to process configuration file
 *
 * @param[out] line the text read
 * @param[in] n max number of character read
 * @param[in] u file descriptor
 *
 * @return NULL if no character has been read before \c EOF has been encountered, \c line otherwise
 */
char *cassys_fgets(char *line, int n, U_FILE *u) {
	int i = 0;
	int c;

	c = u_fgetc(u);
	if (c == EOF) {
		return NULL;
	}
	while (c != EOF && c != '\n' && i < n) {
		line[i] = (char) c;
		c=u_fgetc(u);
		i++;
	}
	line[i] = '\0';
	//fprintf(stdout, "fgets result =%s\n",line);
	return line;
}



int make_directory(char *path){
	return mkDirPortable(path);
}


/**
 * \brief Copies the content of the directory src in the directory dest
 */
/*
int copy_directory_content(const char *dest, const char *src){
#ifdef _NOT_UNDER_WINDOWS
	char linux_command[FILENAME_MAX*2];
	sprintf(linux_command, "cp %s* %s -f", src, dest);

	return system(linux_command);
#else
    return 0;
#endif
}
*/

int copy_directory_snt_item(const char*dest_snt_dir,const char*src_snd_dir,const char*filename,int mandatory)
{
    char fullname_src[1024];
    char fullname_dest[1024];

    sprintf(fullname_src,"%s%s",src_snd_dir,filename);
    sprintf(fullname_dest,"%s%s",dest_snt_dir,filename);

    int ret_copy = af_copy(fullname_src,fullname_dest);
    if (!mandatory)
        return 1;
    return (ret_copy == 0);
}

int copy_directory_snt_content(const char*dest_snt_dir,const char*src_snd_dir)
{
    int result=1;

    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"concord.ind",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"concord.n",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"concord.txt",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"dlc",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"dlf",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"enter.pos",1);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"err",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"stat_dic.n",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"stats.n",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"tags.ind",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"text.cod",1);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"tok_by_alph.txt",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"tok_by_freq.txt",0);
    result = result && copy_directory_snt_item(dest_snt_dir,src_snd_dir,"tokens.txt",1);

    return result;
}


int initialize_working_directory(const char *text,int must_create_directory){
	char path[FILENAME_MAX];
	get_path(text,path);

	char canonical_name[FILENAME_MAX];
	remove_path_and_extension(text, canonical_name);

	char extension[FILENAME_MAX];
	get_extension(text,extension);

	char working_directory[FILENAME_MAX];
	sprintf(working_directory, "%s%s%s%c",path, canonical_name, CASSYS_DIRECTORY_EXTENSION, PATH_SEPARATOR_CHAR);

	if (must_create_directory != 0) {
        make_directory(working_directory);
    }

	char text_in_wd[FILENAME_MAX];
	sprintf(text_in_wd, "%s%s_0%s",working_directory,canonical_name,extension );
	copy_file(text_in_wd,text);

	char snt_dir_text_in_wd[FILENAME_MAX];
	get_snt_path(text_in_wd, snt_dir_text_in_wd);
    if (must_create_directory != 0) {
        make_directory(snt_dir_text_in_wd);
    }

	char original_snt_dir[FILENAME_MAX];
	get_snt_path(text,original_snt_dir);
	copy_directory_snt_content(snt_dir_text_in_wd, original_snt_dir);

	return 0;
}


char* create_labeled_files_and_directory(const char *text, int next_transducer_label,int must_create_directory,int must_copy_file) {
	char path[FILENAME_MAX];
	get_path(text, path);

	char canonical_text_name[FILENAME_MAX];
	remove_path_and_extension(text, canonical_text_name);

	char extension[FILENAME_MAX];
	get_extension(text, extension);

	char working_directory[FILENAME_MAX];
	sprintf(working_directory, "%s%s%s%c", path, canonical_text_name,
			CASSYS_DIRECTORY_EXTENSION, PATH_SEPARATOR_CHAR);

	// copy the text label i- to i
	char old_labeled_text_name[FILENAME_MAX];
	sprintf(old_labeled_text_name, "%s%s_%d%s", working_directory,
			canonical_text_name, next_transducer_label - 1, extension);

	char new_labeled_text_name[FILENAME_MAX];
	sprintf(new_labeled_text_name, "%s%s_%d%s", working_directory,
			canonical_text_name, next_transducer_label, extension);

	char new_labeled_snt_directory[FILENAME_MAX];
	get_snt_path(new_labeled_text_name, new_labeled_snt_directory);
    if (must_create_directory != 0) {
        make_directory(new_labeled_snt_directory);
    }

    if (must_copy_file != 0)
    {
	    copy_file(new_labeled_text_name, old_labeled_text_name);

	    // create snt directory labeled i
	    char old_labeled_snt_directory[FILENAME_MAX];
	    get_snt_path(old_labeled_text_name, old_labeled_snt_directory);


	    // copy dictionary files in the new snt directory
	    struct snt_files *old_snt_ = new_snt_files(old_labeled_text_name);
	    struct snt_files *new_snt_ = new_snt_files(new_labeled_text_name);

	    if (fexists(old_snt_->dlc)) {
		    copy_file(new_snt_->dlc, old_snt_->dlc);
	    }
	    if (fexists(old_snt_-> dlf)) {
		    copy_file(new_snt_->dlf, old_snt_->dlf);
	    }
	    if (fexists(old_snt_-> err)) {
		    copy_file(new_snt_->err, old_snt_->err);
	    }
	    if (fexists(old_snt_->dlc_n)) {
		    copy_file(new_snt_->dlc_n, old_snt_->dlc_n);
	    }
	    if (fexists(old_snt_->dlf_n)) {
		    copy_file(new_snt_->dlf_n, old_snt_->dlf_n);
	    }
	    if (fexists(old_snt_-> err_n)) {
		    copy_file(new_snt_->err_n, old_snt_->err_n);
	    }
	    if (fexists(old_snt_->stat_dic_n)) {
		    copy_file(new_snt_->stat_dic_n, old_snt_->stat_dic_n);
	    }
	    free_snt_files(old_snt_);
	    free_snt_files(new_snt_);
    }
	char *labeled_text_name;
	labeled_text_name = (char*)malloc(sizeof(char)*(strlen(new_labeled_text_name)+1));
	if(labeled_text_name == NULL){
		fatal_alloc_error("create_labeled_files_and_directory");
		exit(1);
	}
	strcpy(labeled_text_name, new_labeled_text_name);
	return labeled_text_name;
}


void protect_special_characters(const char *text, const VersatileEncodingConfig* vec){

	U_FILE *source;
	U_FILE *destination;

	//fprintf(stdout,"protect special character\n");

	char temp_name_file[FILENAME_MAX];
	char path[FILENAME_MAX];
	get_path(text,path);
	sprintf(temp_name_file,"%stemp",path);


	source = u_fopen(vec,text,U_READ);
	if( source == NULL){
		fatal_error("Cannot open file %s\n",text);
		exit(1);
	}

	destination = u_fopen(vec,temp_name_file,U_WRITE);
	if( destination == NULL){
		fatal_error("Cannot open file %s\n",temp_name_file);
		exit(1);
	}

	int a;
	a = u_fgetc(source);
	while(a!=EOF){
		u_fputc((unichar)a,destination);
		if(a=='{'){
			//fprintf(stdout,"opening bracket found\n");


			unichar *bracket_string = get_braced_string(source);
			unichar *protected_bracket_string = protect_braced_string(bracket_string);
			//u_fprints(protected_bracket_string,destination);
			u_fprintf(destination,"%S",protected_bracket_string);
			//u_printf("%S --- ",bracket_string);
			//u_printf("%S\n",protected_bracket_string);
			free(bracket_string);
			free(protected_bracket_string);
		}

		a = u_fgetc(source);
	}

	u_fclose(source);
	u_fclose(destination);

	copy_file(text,temp_name_file);

	// should delete the 'temp' file
}

/**
 *
 */
unichar *get_braced_string(U_FILE *u){

	//u_printf("get_braced string = ");
	int brace_level = 0; // already one brace opened

	long origin_position = ftell(u);
	if (origin_position == -1) {
		fatal_error("ftell");
	}

	int length = 0;
	int a = u_fgetc(u);
	bool protected_char = false;
	while (a != EOF) {
		//u_printf("%C",(unichar)a);
		unichar c = (unichar)a;
		if (protected_char) {
			protected_char = false;
		} else {
			if (c == '\\') {
				protected_char = true;
			} else {
				if (c == '}') {
					if (brace_level == 0) {
						break;
					}
					else {
						brace_level--;
					}
				}
				if(c=='{'){
					brace_level++;
				}

			}
		}
		length++;
		a = u_fgetc(u);
	}

	//u_printf("\n");


	if(a == EOF){
		fatal_error("Unexpected end of file");
	}

	unichar *result;
	result = (unichar*)malloc(sizeof(unichar)*(length+1));
	if(result == NULL){
		fatal_alloc_error("get_braced_string");
		exit(1);
	}

	int fseek_result = fseek(u,origin_position,SEEK_SET);
	if(fseek_result==-1){
		fatal_error("fseek");
	}

	for (int i = 0; i < length; ++i) {
		result[i]=(unichar)u_fgetc(u);
	}
	result[length]='\0';

	return result;
}


unichar *protect_braced_string(const unichar *s){
	unichar *result;
	unichar *stop_sentence;

	stop_sentence = (unichar*) malloc(sizeof(unichar) * (1 + 1));
	if (stop_sentence == NULL) {
		fatal_alloc_error("protect_braced_string");
		exit(1);
	}
	u_sprintf(stop_sentence, "S");

	if (u_strcmp(stop_sentence, s) == 0) {
		return stop_sentence;

	} else {
		unichar* text = protect_text_in_braced_string(s);
		unichar* lem = protect_lem_in_braced_string(s);

		//u_printf("text / lem = %S --- %S\n",text, lem);

		int length_t = u_strlen(text);
		int length_l = u_strlen(lem);

		result = (unichar*) malloc(sizeof(unichar) * (length_t + length_l + 2
				+ 1));
		if (result == NULL) {
			fatal_alloc_error("protect_braced_string");
			exit(1);
		}

		u_sprintf(result, "%S,.%S", text, lem);

		free(lem);
		free(text);
		free(stop_sentence);
	}
	return result;
}

unichar *protect_lem_in_braced_string(const unichar *s){
	int length = u_strlen(s);
	//u_printf("%S = length = %d\n", s,length);
	int i;
	// find the lemm/label separator
	for (i = length-1; i >= 0; i--) {

		if (s[i] == '.') {
			break;
		}
	}

	if (i < 0) {
		fatal_error("protect_text error : no dots in string %S", s);
	}


	// nothing to do, just copy the lem
	unichar *result = (unichar*)malloc(sizeof(unichar)*(length+1));
	if(result == NULL){
		fatal_alloc_error("protect_lem_in_braced_string");
		exit(1);
	}
	i++;
	int j=0;
	while(i<length){
		result[j]=s[i];
		i++;j++;
	}
	result[j]='\0';
	return result;
}


unichar *protect_text_in_braced_string(const unichar *s){
	int length = u_strlen(s);
	int i;
	// find the lemm/label separator
	for(i=length-1; i>=0; i--){
		if(s[i]=='.'){
			break;
		}
	}

	if(i<0){
		fatal_error("protect_text error : no dots in string %s",s);
	}

	i--;
	if(s[i] != ','){
		fatal_error("protect_text error : no comma in string %s",s);
	}


	unichar *result;
	// Alloc twice the memory of s to be sure to have enough space for escape chars.
	result = (unichar*)malloc(sizeof(unichar)*(length*2+1));
	if(result == NULL){
		fatal_alloc_error("protect_text_in_braced_string");
		exit(1);
	}

	// j for s and k for result
	int j,k;
	for(j=0,k=0; j<i; k++,j++){
		if(s[j] == '\\'){
			result[k]=s[j];
			j++;k++;
			result[k]=s[j];
			continue;
		}

		if(s[j] == '{'|| s[j] == '}' || s[j] == '+' || s[j] == ',' || s[j] == '.' || s[j] == ':'){
			result[k] = '\\';
			k++;
			result[k] = s[j];
			continue;
		}
		result[k] = s[j];
	}
	result[k]='\0';
	return result;

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


	u_fclose(concord_desc_file);
	free(snt_file);

}




void display_locate_pos(const locate_pos *l){
	u_printf("locate_pos = %d.%d.%d %d.%d.%d %S\n",l->token_start_offset,l->character_start_offset,l->logical_start_offset,
			l->token_end_offset, l->character_end_offset, l->logical_end_offset,l->label);
}

void display_list_ustring(const struct list_ustring *l){
	u_printf("list_ustring = ");
	while(l!=NULL){
		u_printf("%S",l->string);
		l=l->next;
	}
	u_printf("\n");
}


list_ustring *cassys_tokenize_word_by_word(const unichar* text,const Alphabet* alphabet){

	list_ustring *result = NULL;
	unichar token[4096];
	unichar braced_token[4096];
	int i=0,j=0;


	int opened_bracket = 0;
	bool protected_char = false;
	bool token_found = false;

	//u_printf("Cassys_tokenize : \n");
	//u_printf("chaine = %S\n",text);

	while(text[i]!='\0'){
		if (!opened_bracket) {
			token[j++]=text[i];
			if (is_letter(text[i], alphabet)) {
				if (!is_letter(text[i + 1], alphabet)) {
					token_found = true;

				}
			}
			else {
				if(text[i]=='{'){
					opened_bracket ++;
					j=0;
				}
				else {
					token_found = true;
				}
			}

		}
		else {
			braced_token[j++] = text[i];

			if(protected_char){
				protected_char = false;
			}
			else {
				if(text[i]=='\\'){
					protected_char = true;
				}
				if (text[i] == '}'){
					opened_bracket --;
					if(!opened_bracket){
						token_found = true;
						braced_token[--j]='\0';
						//u_printf("protect string : \n");
						unichar *protected_braced_string = protect_braced_string(braced_token);
						u_sprintf(token,"{%S}",protected_braced_string);
						free(protected_braced_string);
						j=u_strlen(token);
					}
				}
				if (text[i] == '{'){
					opened_bracket ++;
				}
			}
		}
		if(token_found){
			token[j] = '\0';
			result = insert_at_end_of_list(token,result);
			j=0;
			token_found = false;
			//u_printf("token = %S\n",token);
		}
		i++;

	}

	//display_list_ustring(result);
	return result;
}

} // namespace unitex
