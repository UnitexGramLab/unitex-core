/*
 * Cassys.h
 *
 *  Created on: 2 avr. 2010
 *      Author: David Nott
 */

#ifndef CASSYS_H_
#define CASSYS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Concord.h"
#include "Locate.h"

#include "ProgramInvoker.h"
#include "LocateConstants.h"
#include "Error.h"
#include "getopt.h"
#include "FIFO.h"
#include "Tokenize.h"
#include "Unicode.h"
#include "Alphabet.h"
#include "Cassys_tokens.h"




/**
 * Structure storing informations about a transducer
 */

typedef struct transducer{
	char *transducer_file_name;
	OutputPolicy output_policy;
}transducer;

/**
 * \struct locate_pos
 * \brief Concord structure information
 */
typedef struct locate_pos{

	/**
	 * position in token of the first token located by a transducer
	 */
	long token_start_offset;

	/**
	 * position in character of the first character in the first token located by a transducer
	 */
	long character_start_offset;

	/**
	 * used for thai. Not used by cassys
	 */
	long logical_start_offset;

	/**
	 * position in token of the last token located by a transducer
	 */
	long token_end_offset;

	/**
	 * position in character of the last character in the last token located by a transducer
	 */
	long character_end_offset;

	/**
	 * used for thai. Not used by cassys
	 */
	long logical_end_offset;
	unichar *label;
}locate_pos;



extern const char *optstring_Cassys;
extern const struct option_TS lopts_Cassys[];
extern const char* usage_Cassys;


int main_Cassys(int argc,char* const argv[]);

/**
 * \brief returns a fifo containing the list of transducers to be applied in the cascade
 *
 * \param[in] file_list_transducer_name user file containing the list of transducers
 */
struct fifo *load_transducer(const char *file_list_transducer_name);


/**
 * \brief function which makes the cascade
 *
 * \param[in] text
 * \param[in] transducer_list
 * \param[in] alphabet
 *
 * return 0 if correct
 */
int cascade(const char* text, int must_create_directory, fifo* transducer_list, const char*negation_operator,const char *alphabet,Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input);


/**
 * \brief Calls the 'locate' program
 *
 *	The locate program is called with the options '--longest_match' and '--all'. References of to the occurrences
 *	found are saved in a file 'concord.ind'
 *
 * \param text_name target text
 * \param transducer transducer to apply
 * \param file name of the alphabet of the target text
 */
int launch_locate_in_Cassys(const char *text_name, const transducer *transducer, const char* alphabet_name,
                            const char*negation_operator,
                            Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input);


/**
 * brief Calls the concord program
 *
 * The 'concord' program is called with the option '--merge'. It replaces all the occurence of matches in 'concord.ind'
 * by their output.
 *
 * \param text_name target text
 * \param index_file name of the file containing the matches
 * \param alphabet_name name of the file containing the alphabet of the text
 */
int launch_concord_in_Cassys(const char *text_name, const char* index_file, const char *alphabet_name,Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input);



/**
 * \brief Calls the program 'tokenize'
 *
 *	The program is called with the option 'word_by_word. The results of this program are the two files
 *	'tokens.txt' which contains all the tokens of 'text' and 'tokens.cod' which contains the text coded with
 *	'tokens.txt'.
 *
 * \param text_name target text
 * \param file name of the alphabet of the target text
 * \param token_txt_name name of the file containing tokens from a precedent call to tokenize
 *
 */
int launch_tokenize_in_Cassys(const char *text_name, const char *alphabet_name, const char *token_txt_name,Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input);



/**
 * \brief the 'fgets' function
 *
 * Written because it is unavailable in unitex environnement and because 'u_fgets' does not work on ascci files.
 */
char *cassys_fgets(char *line, int n, U_FILE *u);

/**
 * \brief Removes comments (end of line following a '#' char)
 */
void remove_cassys_comments(char *line);


/**
 * \brief Reads of line the name of a transducer in configuration file
 *
 * \param[in] line string line to be read
 *
 * \return an allocated string containing the name of transducer file or NULL if an error occurred
 */
char* extract_cassys_transducer_name(const char *line);


/**
 * \brief Reads the transducer policy in configuration file
 *
 * \param line text line to be read
 *
 * \return MERGE_OUTPUTS or REPLACE_OUTPUTS or IGNORE_OUTPUTS if an error occurred
 */
OutputPolicy extract_cassys_transducer_policy(const char *line);



/**
 * \brief Creates the directory \path
 *
 * @param path the directory to be created
 *
 * @todo Windows version
 */
int make_directory(const char *path);


/**
 * \brief Creates the cassys working directory and copies the target \b text and the associated \b snt directory
 * into the working directory with number label
 *
 * The working directory is the canonical name of the the target \b text file with suffix \b _csc. A numbered suffix
 * is also added to the copied \b text and to the associated \snt directory.
 *
 * If the \b text is \b foo.snt and the associated \b snt directory is \b foo_snt/, the function will create the
 * following files and directories :
 * - directory \b foo_csc/
 * - file \b foo_csc/foo_0.snt (copy of \b foo.snt)
 * - directory \b foo_csc/foo_0.snt/ (copy of \b foo_snt/ ant its content)
 *
 * @param[in] text the target text file
 */


int initialize_working_directory(const char *text,int must_create_directory);

/**
 * \brief
 *
 * \param[in] text
 * \param[in] next_transducer_label
 */
char* create_labeled_files_and_directory(const char *text, int next_transducer_label, int must_create_directory);

/**
 * \brief Copies the content of a snt directory \b src in the directory \b dest
 *
 * @param src the source directory
 * @param dest the destination directory
 *
 */
int copy_directory_snt_content(const char *dest, const char *src);

/**
 * \brief Reads the 'concord.ind' file and returns a FIFO list of locate_pos items
 *
 * \param concord_file_name file containing the matches
 */
struct fifo *read_concord_file(const char *concord_file_name,int mask_encoding_compatibility_input);

/**
 * \brief Reads a line of the 'concord.ind' file and and returns the content in a struct locate_pos
 *
 * \parameter[in] line unichar string containing the 'concord.ind' line
 *
 * \return a struct locate_pos
 */
locate_pos *read_concord_line(unichar *line);


/**
 * \brief Constructs the text representation in the cassys_tokens_list
 *
 * \param[in] token_txt_name name of the file containing the tokens of the text
 * \param[in] text_cod_name name of the containing the sequence of token representing the text
 * \param[out] tokens structure containing the token present in the text
 *
 * Each element of the cassys_tokens_list token field is pointing on a element of tokens.
 */
cassys_tokens_list *cassys_load_text(const char *token_text_name, const char * text_cod_name, struct text_tokens **tokens);




cassys_tokens_list *add_replaced_text(const char *text, cassys_tokens_list *list,
		 int transducer_id, const char *alphabet,int mask_encoding_compatibility_input);

/**
 * \brief Produces a concordance file with the matches found by all the locates program called during the cascade
 *
 * \param[in] list the token list representation of the text
 * \param[in] the text target
 * \param[in] the number of transducers applied during the cascade
 */
void construct_cascade_concord(cassys_tokens_list *list, const char *text_name, int number_of_transducer,
    Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input);

void follow_text(cassys_tokens_list *list, int transducer_id);
void display_locate_pos(const locate_pos *l);
void display_list_ustring(const struct list_ustring *l);

/**
 * \brief Adds protection character in braces before
 *
 * \param[in/out] text file name of the text to be protected
 */
void protect_special_characters(const char *text,Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input);

/**
 * \brief Adds protection characters in the lem zone. (currently does nothing)
 */
unichar *protect_lem_in_braced_string(const unichar *s);

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
 * \brief Returns all the characters encountered
 */
unichar *get_braced_string(U_FILE *u);

int token_length(list_ustring *l);


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

#endif /* CASSYS_H_ */


