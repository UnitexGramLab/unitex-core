/*
 * Cassys.cpp
 *
 *  Created on: 2 avr. 2010
 *  Author: David Nott
 */

#include "Cassys.h"
#include <ctype.h>
#include "File.h"
#include "Snt.h"
#include "List_ustring.h"
#include "Tokenization.h"
#include "Text_tokens.h"

#include "DirHelper.h"


#define CASSYS_DIRECTORY_EXTENSION "_csc"


const char *optstring_Cassys = ":f:a:t:";
const struct option_TS lopts_Cassys[] = {
		{"file", required_argument_TS, NULL, 'f'},
		{"alphabet", required_argument_TS, NULL, 'a'},
		{"transducers_list", required_argument_TS, NULL,'t'}
};

const char* usage_Cassys ="usage";


int main_Cassys(int argc, char **argv){

	char transducer_list_file_name[FILENAME_MAX];
	bool has_transducer_list = false;

	char text_file_name[FILENAME_MAX];
	bool has_text_file_name = false;

	char alphabet_file_name[FILENAME_MAX];
	bool has_alphabet = false;

	// decode the command line
	int val;
	int index = 1;
	struct OptVars* vars=new_OptVars();
	while (EOF != (val = getopt_long_TS(argc, argv, optstring_Cassys,
			lopts_Cassys, &index, vars))) {
		switch (val) {
		case 'f': {
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
		case 't': {
			if(vars -> optarg[0] == '\0'){
				fatal_error("Command line error : Empty transducer list argument\n");
			} else {
				strcpy(transducer_list_file_name, vars -> optarg);
				has_transducer_list = true;
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
		default :{
			fatal_error("Unknown option : %c\n",val);
			break;
		}
		}
	}
	index = -1;

	if(has_alphabet == false){
		fatal_error("Command line error : no alphabet provided\n");
	}
	if(has_text_file_name == false){
		fatal_error("Command line error : no text file provided\n");
	}
	if(has_transducer_list == false){
		fatal_error("Command line error : no transducer list provided\n");
	}



	// Load the list of transducers from the file transducer list and stores it in a list
	struct fifo *transducer_list = load_transducer(transducer_list_file_name);

	cascade(text_file_name, transducer_list, alphabet_file_name);

	return 0;
}


/**
 * The main function of the cascade
 *
 *
 */
int cascade(char* text, fifo* transducer_list, char *alphabet){

	launch_tokenize_in_Cassys(text,alphabet,NULL);

	initialize_working_directory(text);

	struct snt_files *snt_text_files = new_snt_files(text);

	struct text_tokens *tokens = NULL;
	cassys_tokens_list *tokens_list = cassys_load_text(snt_text_files->tokens_txt, snt_text_files->text_cod,&tokens);

	fprintf(stdout,"Cascade begins\n");

	int transducer_number = 1;
	while(!is_empty(transducer_list)){
		char *labeled_text_name;

		labeled_text_name = create_labeled_files_and_directory(text,
				transducer_number);

		launch_tokenize_in_Cassys(labeled_text_name,alphabet,snt_text_files->tokens_txt);
		free_snt_files(snt_text_files);

		// apply transducer
		transducer *current_transducer = (transducer*)take_ptr(transducer_list);
		launch_locate_in_Cassys(labeled_text_name, current_transducer, alphabet);

		// generate concordance for this transducer
		snt_text_files = new_snt_files(labeled_text_name);
		launch_concord_in_Cassys(labeled_text_name,
				snt_text_files -> concord_ind, alphabet);

		//
		add_replaced_text(labeled_text_name,tokens_list,transducer_number,alphabet);

		// add protection character in braces when needed
		protect_special_characters(labeled_text_name);

		transducer_number++;

		free(labeled_text_name);

	}
	free_snt_files(snt_text_files);

	construct_cascade_concord(tokens_list,text,transducer_number);


	struct snt_files *snt_files = new_snt_files(text);

	char result_file_name[FILENAME_MAX];
	char text_name_without_extension[FILENAME_MAX];
	remove_extension(text,text_name_without_extension);
	sprintf(result_file_name,"%s.csc",text_name_without_extension);

	copy_file(result_file_name,text);

	launch_concord_in_Cassys(result_file_name,snt_files->concord_ind,alphabet);


	free_snt_files(snt_files);

	return 0;
}


cassys_tokens_list *cassys_load_text(const char *tokens_text_name, const char *text_cod_name, struct text_tokens **tokens){

	int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;

	*tokens = load_text_tokens(tokens_text_name,mask_encoding_compatibility_input);

	U_FILE *f = u_fopen(BINARY, text_cod_name,U_READ);
	if( f == NULL){
		perror("fopen\n");
		fprintf(stderr,"Impossible d'ouvrir le fichier %s\n",text_cod_name);
		exit(1);
	}

	cassys_tokens_list *list = NULL;
	cassys_tokens_list *temp = list;

	int token_id;
	int char_read = fread(&token_id,sizeof(int),1,f);
	while(char_read ==1){
		if(list==NULL){
			list = new_element((*tokens)->token[token_id],0);
			temp = list;
		}
		else {
			temp ->next_token = new_element((*tokens)->token[token_id],0);
			temp = temp -> next_token;
		}

		char_read = fread(&token_id,sizeof(int),1,f);
	}
	u_fclose(f);

	return list;
}


cassys_tokens_list *add_replaced_text( char *text, cassys_tokens_list *list,
		 int transducer_id, const char *alphabet_name) {

	fprintf(stdout,"Replacement text begins\n");

	Alphabet *alphabet = load_alphabet(alphabet_name);

	struct snt_files *snt_text_files = new_snt_files(text);

	struct fifo *stage_concord = read_concord_file(snt_text_files->concord_ind);
	fprintf(stdout,"Read concord file done\n");



	int nb_sentence = 0;
	while (!is_empty(stage_concord)) {
		nb_sentence++;
		//fprintf(stdout,"add_replaced : take sentence %d\n",nb_sentence);

		locate_pos *l = (locate_pos*) take_ptr(stage_concord);
		//display_locate_pos(l);

		struct list_ustring *new_sentence_lu = cassys_tokenize_word_by_word(l->label,
				alphabet);
		//u_printf("%d --> ",length(new_sentence_lu));
		//display_list_ustring(new_sentence_lu);

		cassys_tokens_list *new_sentence_ctl =
				new_list(new_sentence_lu, transducer_id);
		//display_text(new_sentence_ctl,transducer_id);

		cassys_tokens_list *list_position = get_element_at(list, transducer_id - 1,
				l->token_start_offset);
		//display_text(list_position,transducer_id-1);

		int replaced_sentence_length = l->token_end_offset
				- l->token_start_offset+1;
		int new_sentence_length = length(new_sentence_lu);

		//u_printf("lengths = %d %d\n",replaced_sentence_length, new_sentence_length);


		add_output(list_position, new_sentence_ctl, transducer_id,
				replaced_sentence_length, new_sentence_length-1);
		//follow_text(list,transducer_id);

		//free_list_ustring(new_sentence_lu);
		free(l->label);
		free(l);
		free_list_ustring(new_sentence_lu);
	}

	free_fifo(stage_concord);
	free_snt_files(snt_text_files);

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
struct fifo *read_concord_file(const char *concord_file_name){
	unichar line[4096];

	struct fifo *f = new_fifo();

	U_FILE *concord_desc_file;
	concord_desc_file = u_fopen(UTF16_LE, concord_file_name,U_READ);
	if( concord_desc_file == NULL){
		perror("u_fopen\n");
		fprintf(stderr,"Impossible d'ouvrir le fichier %s\n",concord_file_name);
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
locate_pos *read_concord_line(unichar *line) {

	locate_pos *l;
	l = (locate_pos*) malloc(sizeof(locate_pos) * 1);
	if (l == NULL) {
		perror("malloc\n");
		fprintf(stderr, "Impossible d'allouer de la mémoire\n");
		exit(1);
	}
	l->label = (unichar*) malloc(sizeof(unichar) * (u_strlen(line) + 1));
	if (l->label == NULL) {
		perror("malloc\n");
		fprintf(stderr, "Impossible d'allouer de la mémoire\n");
		exit(1);
	}

	// format of a line : n.n.n n.n.n t where n are integers and t is string
	unichar **next;

	unichar *current = line;
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


struct fifo *load_transducer(const char *transducer_list_name){

	U_FILE *file_transducer_list;

	file_transducer_list = u_fopen(ASCII, transducer_list_name,U_READ);
	if( file_transducer_list == NULL){
		perror("u_fopen\n");
		fprintf(stderr,"Impossible d'ouvrir le fichier %s\n",transducer_list_name);
		exit(1);
	}

	struct fifo *transducer_fifo = new_fifo();

	char line[1024];


	int i=1;
	while (cassys_fgets(line,1024,file_transducer_list) != NULL){
		char *transducer_file_name;
		OutputPolicy transducer_policy;
		transducer *t;

		remove_cassys_comments(line);

		transducer_file_name = extract_cassys_transducer_name(line);
		//fprintf(stdout, "transducer name read =%s\n",transducer_file_name);

		transducer_policy = extract_cassys_transducer_policy(line);

		if (transducer_file_name != NULL && transducer_policy != IGNORE_OUTPUTS) {
			//fprintf(stdout,"transducer to be loaded\n");
			t = (transducer*) malloc(sizeof(transducer) * 1);
			if (t == NULL) {
				perror("malloc\n");
				fprintf(stderr, "Impossible d'allouer de la mémoire\n");
				exit(1);
			}

			t->transducer_file_name = (char*)malloc(sizeof(char)*(strlen(transducer_file_name)+1));
			if(t->transducer_file_name == NULL){
				perror("malloc\n");
				fprintf(stderr,"Impossible d'allouer de la mémoire\n");
				exit(1);
			}

			strcpy(t->transducer_file_name, transducer_file_name);
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
				fprintf(stdout, "Line %d : Empty line\n",i);
			} else if (transducer_policy == IGNORE_OUTPUTS) {
				fprintf(stdout, "Line %d : Transducer mode not recognized\n",i);
			}
		}
		i++;
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
int launch_tokenize_in_Cassys(char *text_name, const char *alphabet_name, char *token_txt_name){

	fprintf(stdout,"Launch tokenize in Cassys\n");

	ProgramInvoker *invoker = new_ProgramInvoker(main_Tokenize,"main_Tokenize");

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
int launch_locate_in_Cassys(char *text_name, const transducer *transducer, const char* alphabet_name){

	ProgramInvoker *invoker = new_ProgramInvoker(main_Locate, "main_Locate");

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
int launch_concord_in_Cassys(char *text_name, char *index_file, const char *alphabet_name){
	ProgramInvoker *invoker = new_ProgramInvoker(main_Concord, "main_Concord");

	add_argument(invoker,index_file);

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

	transducer_name = (char*)malloc(sizeof(char)*(i-j)+1);
	if(transducer_name == NULL){
		perror("malloc\n");
		fprintf(stderr,"Impossible d'allouer de la mémoire\n");
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
int copy_directory_content(char *dest, const char *src){
#ifdef _NOT_UNDER_WINDOWS
	char linux_command[FILENAME_MAX*2];
	sprintf(linux_command, "cp %s* %s -f", src, dest);

	return system(linux_command);
#else
    return 0;
#endif
}



int initialize_working_directory(char *text){
	char path[FILENAME_MAX];
	get_path(text,path);

	char canonical_name[FILENAME_MAX];
	remove_path_and_extension(text, canonical_name);

	char extension[FILENAME_MAX];
	get_extension(text,extension);

	char working_directory[FILENAME_MAX];
	sprintf(working_directory, "%s%s%s%c",path, canonical_name, CASSYS_DIRECTORY_EXTENSION, PATH_SEPARATOR_CHAR);

	make_directory(working_directory);

	char text_in_wd[FILENAME_MAX];
	sprintf(text_in_wd, "%s%s_0%s",working_directory,canonical_name,extension );
	copy_file(text_in_wd,text);

	char snt_dir_text_in_wd[FILENAME_MAX];
	get_snt_path(text_in_wd, snt_dir_text_in_wd);
	make_directory(snt_dir_text_in_wd);

	char original_snt_dir[FILENAME_MAX];
	get_snt_path(text,original_snt_dir);
	copy_directory_content(snt_dir_text_in_wd, original_snt_dir);

	return 0;
}


char* create_labeled_files_and_directory(char *text, int next_transducer_label) {

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

	copy_file(new_labeled_text_name, old_labeled_text_name);

	// create snt directory labeled i
	char old_labeled_snt_directory[FILENAME_MAX];
	get_snt_path(old_labeled_text_name, old_labeled_snt_directory);

	char new_labeled_snt_directory[FILENAME_MAX];
	get_snt_path(new_labeled_text_name, new_labeled_snt_directory);
	make_directory(new_labeled_snt_directory);

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

	char *labeled_text_name;
	labeled_text_name = (char*)malloc(sizeof(char)*(strlen(new_labeled_text_name)+1));
	if(labeled_text_name == NULL){
		perror("malloc\n");
		fprintf(stderr,"Impossible d'allouer de la mémoire\n");
		exit(1);
	}
	strcpy(labeled_text_name, new_labeled_text_name);
	return labeled_text_name;
}


void protect_special_characters(char *text){

	U_FILE *source;
	U_FILE *destination;

	//fprintf(stdout,"protect special character\n");

	char temp_name_file[FILENAME_MAX];
	char path[FILENAME_MAX];
	get_path(text,path);
	sprintf(temp_name_file,"%stemp",path);


	source = u_fopen(UTF16_LE, text,U_READ);
	if( source == NULL){
		perror("u_fopen\n");
		fprintf(stderr,"Impossible d'ouvrir le fichier %s\n",text);
		exit(1);
	}

	destination = u_fopen(UTF16_LE, temp_name_file,U_WRITE);
	if( destination == NULL){
		perror("u_fopen\n");
		fprintf(stderr,"Impossible d'ouvrir le fichier %s\n",temp_name_file);
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
		perror("ftell\n");
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
		perror("malloc\n");
		fprintf(stderr,"Impossible d'allouer de la mémoire\n");
		exit(1);
	}

	int fseek_result = fseek(u,origin_position,SEEK_SET);
	if(fseek_result==-1){
		perror("fseek");
		fatal_error("fseek");
	}

	for (int i = 0; i < length; ++i) {
		result[i]=(unichar)u_fgetc(u);
	}
	result[length]='\0';

	return result;
}


unichar *protect_braced_string(unichar *s){
	unichar *result;
	unichar *stop_sentence;

	stop_sentence = (unichar*) malloc(sizeof(unichar) * (1 + 1));
	if (stop_sentence == NULL) {
		perror("malloc\n");
		fprintf(stderr, "Impossible d'allouer de la mémoire\n");
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
			perror("malloc\n");
			fprintf(stderr, "Impossible d'allouer de la mémoire\n");
			exit(1);
		}

		u_sprintf(result, "%S,.%S", text, lem);

		free(lem);
		free(text);
		free(stop_sentence);
	}
	return result;
}

unichar *protect_lem_in_braced_string(unichar *s){
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
		perror("malloc\n");
		fprintf(stderr,"Impossible d'allouer de la mémoire\n");
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


unichar *protect_text_in_braced_string(unichar *s){
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
		perror("malloc\n");
		fprintf(stderr,"Impossible d'allouer de la mémoire\n");
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


void construct_cascade_concord(cassys_tokens_list *list, char *text_name, int number_of_transducer){

	fprintf(stdout, "Construct cascade concord\n");

	struct snt_files *snt_file = new_snt_files(text_name);

	U_FILE *concord_desc_file = u_fopen(UTF16_LE, snt_file->concord_ind,U_WRITE);
	if( concord_desc_file == NULL){
		perror("u_fopen\n");
		fprintf(stderr,"Impossible d'ouvrir le fichier %s\n",snt_file->concord_ind);
		exit(1);
	}

	fprintf(stdout, "Concord File %s successfully opened\n",snt_file->concord_ind);

	if (list == NULL) {
		fatal_error("empty text");
	}

	u_fprintf(concord_desc_file,"#M\n");

	cassys_tokens_list *current_pos_in_original_text = list;
	cassys_tokens_list *output=get_output(list,number_of_transducer);
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

				// token position pointe sur le token suivant déjà
				int end_position=token_position-1;

				if(sentence == NULL){
					fatal_error("construct_cassys_concordance : Phrase de remplacement vide\n");
				}

				struct list_ustring *iterator = sentence;
				while(iterator -> next != NULL){
					iterator = iterator -> next;
				}

				//display_list_ustring(iterator);


				u_fprintf(concord_desc_file, "%d.0.0 %d.%d.0 ",start_position,end_position,last_token_length);
				//u_fprintf(concord_desc_file, "%d.0.0 %d.0.0 ",start_position,end_position);

				iterator = sentence;
				while(iterator != NULL){
					u_fprintf(concord_desc_file,"%S",iterator->string);
					//u_printf("concord.ind : %S\n",iterator->string);
					iterator = iterator -> next;
				}
				//u_printf("\n");
				u_fprintf(concord_desc_file,"\n");

				current_pos_in_original_text
						= current_pos_in_original_text -> next_token;
				output = get_output(current_pos_in_original_text,
						number_of_transducer);
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




void display_locate_pos(locate_pos *l){
	u_printf("locate_pos = %d.%d.%d %d.%d.%d %S\n",l->token_start_offset,l->character_start_offset,l->logical_start_offset,
			l->token_end_offset, l->character_end_offset, l->logical_end_offset,l->label);
}

void display_list_ustring(struct list_ustring *l){
	u_printf("list_ustring = ");
	while(l!=NULL){
		u_printf("%S",l->string);
		l=l->next;
	}
	u_printf("\n");
}


list_ustring *cassys_tokenize_word_by_word(unichar* text,Alphabet* alphabet){

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






