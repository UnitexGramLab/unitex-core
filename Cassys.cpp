/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Offsets.h"
#include "Cassys_lexical_tags.h"
#include "Cassys_concord.h"
#include "UnusedParameter.h"
#include "SyncTool.h"
#include "StringParsing.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char *optstring_Cassys = ":bp:t:a:w:l:Vhk:q:g:dvuNncm:s:ir:f:T:L:C:O$:x:";
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
  {"display_time", no_argument_TS, NULL, 'c' },
  {"translate_path_separator_to_native", no_argument_TS, NULL, 'v' },
  {"working_dir", required_argument_TS, NULL, 'p' },
  {"cleanup_working_files", no_argument_TS, NULL, 'b' },
  {"tokenize_argument", required_argument_TS, NULL, 'T' },
  {"locate_argument", required_argument_TS, NULL, 'L' },
  {"concord_argument", required_argument_TS, NULL, 'C' },
  {"uima", required_argument_TS, NULL, 'f' },
  {"input_offsets",required_argument_TS,NULL,'$'},
  {"produce_offsets_file",no_argument_TS,NULL,'O'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"standoff",optional_argument_TS,NULL,'x'},
  {"lang",required_argument_TS,NULL,'A'},
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
        "--input_offsets=XXX base offset file to be used (optional)\n"
        "-O/--produce_offsets_file produce offsets file (automatic if --input_offsets=XXX is used)\n"
        "-t TXT/--text=TXT the text file to be modified, with extension .snt\n"
        "-i/--in_place mean uses the same csc/snt directories for each transducer\n"
        "-p X/--working_dir=X uses directory X for intermediate working file\n"
        "-b/--cleanup_working_files remove intermediate working file after usage\n"
        "-u/--dump_token_graph create a .dot file with graph dump infos\n"
        "-N/--no_dump_token_graph create a .dot file with graph dump infos\n"
        "-n/--realign_token_graph_pointer create a.dot file will not depends to pointer allocation to be deterministic\n"
        "-v/--translate_path_separator_to_native replace path separator in csc by native separator for portable csc file\n"
        "-c/--display_time display time used in each unitex tool\n"
        "-d/--no_create_directory mean the all snt/csc directories already exist and don't need to be created\n"
        "  -g minus/--negation_operator=minus: uses minus as negation operator for Unitex 2.0 graphs\n"
        "  -g tilde/--negation_operator=tilde: uses tilde as negation operator (default)\n"
        "-T ARG/--tokenize_argument=ARG specify additionnal argument for Tokenize internal call (several possible)\n"
        "-L ARG/--locate_argument=ARG specify additionnal argument for Locate internal call (several possible)\n"
        "-C ARG/--concord_argument=ARG specify additionnal argument for Concord internal call (several possible)\n"
        "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
        "  -h/--help display this help\n"
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
  display_copyright_notice();
  u_printf(usage_Cassys);
}

/**
 * This structure represent the information needed by the generic graph generation algorithm to create the box
 *
 * reference_location : The number of the line on which the first box of the graph is defined.
 * This value is used to allow relative location with other boxes
 *
 * begining_location  : The number of the line in the .grf file, based on reference_location, that represent the 'G' box in the graph
 * end_location       : The number of the line in the .grf file, based on reference_location, that represent the end box (following the 'G') in the graph
 *
 * category           : The category searched by the user, indicated below the end box
 * filter_category    : The category that indicates which kind of box must be found inside the whole token, indicated in the end box. May be empty
 * final_category     : The category that indicates which category will be written in the final generated graph.
                      : It is indicated after the filter category in the end box. May be empty
 */
typedef struct graphInfo {

  unsigned int reference_location;
  unsigned int begining_location;
  unsigned int end_location;

  unichar* category;
  unichar* filter_category;
  unichar* final_category;

} graphInfo;

/**
 * Creates a new graphInfo
 *
 * Returns the newly created graphInfo or NULL if it couldn't be created
 */
graphInfo* new_graph_info() {

    graphInfo* graph_info = (graphInfo*) malloc(sizeof(graphInfo));

    if (graph_info == NULL) {

        return NULL;
    }

    graph_info->reference_location = 0;
    graph_info->begining_location = 0;
    graph_info->end_location = 0;

    graph_info->category = NULL;
    graph_info->filter_category = NULL;
    graph_info->final_category = NULL;

    return graph_info;
}

/**
 * Frees a graphInfo
 *
 * graph_info : A pointer to the graphInfo that needs to be freed
 */
void free_graph_info(graphInfo* graph_info) {

  free(graph_info->category);
  free(graph_info->filter_category);
  free(graph_info->final_category);

  free(graph_info);
}

/**
 * This structure represent a list of graphInfo. Such a list is returned after anaysing a file because more than one generic graph can be declared
 * in a single file
 *
 * graph_list   : The list of graphInfo
 * graph_count  : The number of graphs in the list
 */
typedef struct graphInfoList {

  graphInfo** graph_list;
  size_t graph_count;

} graphInfoList;

/**
 * Creates a new graphInfoList
 *
 * Returns the newly created graphInfoList or NULL if the list couldn't be created
 */
graphInfoList* new_graph_info_list() {

    graphInfoList* graph_info_list = (graphInfoList*) malloc(sizeof(graphInfoList));

    if (graph_info_list == NULL) {

        return NULL;
    }

    graph_info_list->graph_list = NULL;
    graph_info_list->graph_count = 0;

    return graph_info_list;
}

/**
 * Adds a graphInfo to a graphInfoList
 *
 * graph_info       : The graphInfo to add in the list
 * graph_info_list  : The graphInfoList in which to add the graph info
 */
void add_graph_info_to_list(graphInfo* graph_info, graphInfoList* graph_info_list) {

    // The size of the list is reallocated with one more graphInfo slot
    graph_info_list->graph_list = (graphInfo**) realloc(graph_info_list->graph_list, sizeof(graphInfo*) * (graph_info_list->graph_count + 1));
    graph_info_list->graph_list[graph_info_list->graph_count] = graph_info;
    graph_info_list->graph_count++;
}

/**
 * Frees a list of graphInfo
 *
 * graph_info_list  : A pointer to the list of graphInfoList that needs to be freed.
 */
void free_graph_info_list(graphInfoList* graph_info_list) {

    // For each graph in the list
    for (unsigned int i = 0; i < graph_info_list->graph_count; i++) {

        free_graph_info(graph_info_list->graph_list[i]);
    }

    free(graph_info_list->graph_list);
    free(graph_info_list);
}

/**
 * This structure represent a file, as an array a strings
 *
 * lines        : The list of strings, representing the loaded file
 * lines_number : The number of lines inf the file
 */
typedef struct graphFile {

    unichar** lines;
    size_t lines_number;

} graphFile;

/**
 * Adds a new line to a file. The line is dupplicated, so the line that is sent to the function must be freed manually
 *
 * file : The file on which the line will be added
 * line : The line that will be added to the file
 *
 * Returns the number of the line that has beed added, which is the number of lines in the file minus 1
 */
unsigned int add_line_to_file(graphFile* file, const unichar* line) {

    file->lines = (unichar**) realloc(file->lines, sizeof(unichar*) * (file->lines_number + 1));
    file->lines[file->lines_number] = u_strdup(line);

    file->lines_number++;
    return file->lines_number - 1;
}

/**
 * char version of the add_line_to_file function
 */
inline unsigned int add_line_to_file(graphFile* file, const char* line) {

    unichar* copy = u_strdup(line);
    const unsigned int return_value =  add_line_to_file(file, copy);
    free(copy);
    return return_value;
}

/**
 * Creates a graphFile from a file
 *
 * file             : The file to be loaded in the graphFile
 * encoding_config  : The encoding config
 *
 * Returns the created graphFile or NULL if the loading failed
 */
graphFile* load_graph_file(const char* file, VersatileEncodingConfig* encoding_config) {

    U_FILE* loaded_file = u_fopen(encoding_config, file, U_READ);

    // If the file can't be loaded we stop there and return NULL to indicate that the loading went wrong
    if (loaded_file == NULL) {

        return NULL;
    }

    // Creation and initialization of the graph file
    graphFile* graph_file = (graphFile*) malloc(sizeof(graphFile));
    graph_file->lines = NULL;
    graph_file->lines_number = 0;

    unichar* line = NULL;
    size_t buffer_size = 0;

    // As long as we're getting new lines to read
    while (u_fgets_dynamic_buffer(&line, &buffer_size, loaded_file) != EOF) {

        add_line_to_file(graph_file, line);
    }

    if (line != NULL) {

        free(line);
    }

    u_fclose(loaded_file);

    return graph_file;
}

/**
 * Replaces one line of the file by a new one. The new line is copied, so the replacement line that is sent to the function must be freed manually
 *
 * file         : The file in which the line will be replaced
 * line_number  : The number of the line in the file that need to be replaced. The first line of the file is the number 0
 * line         : The new line that will replace the previous one
 */
void replace_graph_file_line(graphFile* file, const unsigned int line_number, const unichar* line) {

    free(file->lines[line_number]);
    file->lines[line_number] = u_strdup(line);
}

/**
 * char version of the replace_graph_file_line function
 */
inline void replace_graph_file_line(graphFile* file, const unsigned int line_number, const char* line) {

    unichar* copy = u_strdup(line);
    replace_graph_file_line(file, line_number, copy);
    free(copy);
}

/**
 * Creates a file on the disk from a graphFile
 *
 * file : The file that will be printed on the disk
 * new_graph_file_name  : The name of the new file that will be created
 * config               : The encoding config
 *
 * Returns 1 if the operation was successful, 0 otherwise
 */
unsigned int print_graph_file(graphFile* file, const char* new_graph_file_name, VersatileEncodingConfig* config) {

    enum Status { FAIL = 0, SUCCESS = 1 };

    U_FILE* new_graph_file = u_fopen(config, new_graph_file_name, U_WRITE);

    if (new_graph_file == NULL) {

        return FAIL;
    }

    for (unsigned int i = 0; i < file->lines_number; i++) {

        u_fprintf(new_graph_file, "%S\n", file->lines[i]);
    }

    u_fclose(new_graph_file);

    return SUCCESS;
}

/**
 * Frees a graphFile
 *
 * graph_file : The graphFile to be freed
 */
void free_graph_file(graphFile* graph_file) {

    // For each line of the graph file
    for (unsigned int i = 0; i < graph_file->lines_number; i++) {

        free(graph_file->lines[i]);
    }

    free(graph_file->lines);
    free(graph_file);
}

/**
 * This structure represents a list of token found in a string. Token are considered as elements separated by blanks,
 * except for the strings (delimited by "") that are considered tokens even if they contain blanks between the quotes
 *
 * token_list : The list of found tokens, an array of strings
 * list_size  : The number of tokens in the list
 */
typedef struct stringTokenList {

    unichar** token_list;
    size_t list_size;

} stringTokenList;

/**
 * Adds a token to a stringTokenList. To original token sent to the function is copied, so it must be freed manually
 *
 * token              : The token to add to the list
 * string_token_list  : The list in which to add the new token
 */
void add_token_to_token_list(const unichar* token, stringTokenList* string_token_list) {

    string_token_list->token_list = (unichar**) realloc(string_token_list->token_list, sizeof(unichar*) * (string_token_list->list_size + 1));
    string_token_list->token_list[string_token_list->list_size] = u_strdup(token);

    string_token_list->list_size++;
}

/**
 * char version of the add_token_to_token_list function
 */
inline void add_token_to_token_list(const char* token, stringTokenList* string_token_list) {

    unichar* copy = u_strdup(token);
    add_token_to_token_list(copy, string_token_list);
    free(copy);
}

/**
 * Repalces a token in a token list. The new token is copied, so the one that is sent to the function must be freed manually
 *
 * string_token_list  : The list in which the token will be replaced
 * token_number       : The number of the token that will be replaced. The first token is the number 0
 * new_token          : The token that will replace the previous one
 */
void replace_token_in_list(stringTokenList* string_token_list, const unsigned int token_number, const unichar* new_token) {

    free(string_token_list->token_list[token_number]);
    string_token_list->token_list[token_number] = u_strdup(new_token);
}

/**
 * char version of the replace_token_in_list function
 */
inline void replace_token_in_list(stringTokenList* string_token_list, const unsigned int token_number, const char* new_token) {

    unichar* copy = u_strdup(new_token);
    replace_token_in_list(string_token_list, token_number, copy);
    free(copy);
}

/**
 * Returns the token as a string, by concatenating all the tokens with the given separator between them
 *
 * string_token_list  : The token list to concatenate
 * separator          : The separator that will be place between each token
 *
 * Returns the concatenation of all the tokens in the list with the given separator between them
 */
unichar* token_list_to_string(stringTokenList* string_token_list, unichar separator) {

    if (string_token_list->list_size == 0) {

        return u_strdup("");
    }

    const unichar separator_string[] = { separator, '\0' };

    Ustring* final_string = new_Ustring(string_token_list->token_list[0]);

    for (unsigned int i = 1; i < string_token_list->list_size; i++) {

        u_strcat(final_string, separator_string);
        u_strcat(final_string, string_token_list->token_list[i]);
    }

    unichar* copy = u_strdup(final_string->str);
    free_Ustring(final_string);

    return copy;
}

/**
 * char version of the token_list_to_string function
 */
inline unichar* token_list_to_string(stringTokenList* string_token_list, char separator) {

    unichar* return_value = token_list_to_string(string_token_list, (unichar) separator);
    return return_value;
}

/**
 * Creates a new empty string token list
 *
 * Returns the newly created stringTokenList, or NULL if it couldn't be created
 */
stringTokenList* new_string_token_list() {

    stringTokenList* string_token_list = (stringTokenList*) malloc(sizeof(stringTokenList));

    if (string_token_list == NULL) {

        return NULL;
    }

    string_token_list->token_list = NULL;
    string_token_list->list_size = 0;

    return string_token_list;
}

/**
 * Creates a stringTokenList from a source string. Each found token in the source string is added to the returned stringTokenList
 *
 * string : The source string to extract the tokens from
 *
 * Return the newly created token list, of NULL if the list couldn't be created
 */
stringTokenList* new_token_list_from_string(const unichar* string) {

    // Initialization
    stringTokenList* string_token_list = (stringTokenList*) malloc(sizeof(stringTokenList));

    if (string_token_list == NULL) {

        return NULL;
    }

    string_token_list->token_list = NULL;
    string_token_list->list_size = 0;

    // Begining of the token finding algorithm
    const size_t string_length = u_strlen(string);

    /**
     * normal state : When the cursor is outside any token
     * in_string    : When the cursor is between quotes
     * in_token     : When the cursor is on characters composing a token that is not a string
     */
    enum tokenState { normal, in_string, in_token } current_state;
    current_state = normal;

    unsigned int token_beginning = 0;
    unsigned int cursor = 0;

    while (cursor < string_length) {

        switch (current_state) {

        case normal: {

            // Skip the blanks
            while (u_strchr(" \t", string[cursor]) != NULL) {

                cursor++;
            }

            token_beginning = cursor;

            if (string[cursor] == '"') {

                current_state = in_string;
            }
            else if (string[cursor] != '\0') {

                current_state = in_token;
            }

        } break;

        case in_string: {

            // Search the end of the string
            bool is_escaped = false;
            cursor++;

            // Until we reach a quote that is not escaped
            while (string[cursor] != '"' || is_escaped) {

                if (string[cursor] == '\\') {

                    is_escaped = !is_escaped;
                }

                cursor++;
            }

            // Creation of the token to add to the token list
            const size_t token_length = cursor - token_beginning + 1; // +1 to match the correct token length
            unichar* sub_string = u_strndup(string + token_beginning, token_length);

            add_token_to_token_list(sub_string, string_token_list);
            free(sub_string);

            current_state = normal;
            cursor++; // The cursor is placed just after the "

        } break;

        case in_token: {

            // Search the end of the token
            bool is_escaped = (string[cursor] == '\\');

            // As long as we're not reading a blank character, and the current character is not the beginning of a string
            while (u_strchr(" \t\n", string[cursor]) == NULL &&
                    !(string[cursor] == '"' && is_escaped == false) ) {

                if (string[cursor] == '\\') {

                    is_escaped = !is_escaped;
                }

                cursor++;
            }

            // Creation of the token to add to the token list
            const size_t token_length = cursor - token_beginning;
            unichar* sub_string = u_strdup(string + token_beginning, token_length);

            add_token_to_token_list(sub_string, string_token_list);
            free(sub_string);

            current_state = normal;

        } break;

        default: {
        } break;
        }
    }

    return string_token_list;
}

/**
 * Frees a stringTokenList
 *
 * string_token_list  : The token list that needs to be freed
 */
void free_string_token_list(stringTokenList* string_token_list) {

    // For each token in the list
    for (unsigned int i = 0; i < string_token_list->list_size; i++) {

        free(string_token_list->token_list[i]);
    }

    free(string_token_list->token_list);
    free(string_token_list);
}

/**
 * This structure represent a single match, composing a linked list
 *
 * content  : The content of the match
 * next     : The next tokenMatch in the linked list
 */
typedef struct tokenMatch {

    unichar* token;
    tokenMatch* next;

} tokenMatch;

/**
 * Creates a new tokenMatch with the given content
 *
 * content  : The content of the tokenMatch
 *
 * Returns the newly created tokenMatch, or NULL if it couldn't be created
 */
tokenMatch* new_token_match(const unichar* content) {

    tokenMatch* token_match = (tokenMatch*) malloc(sizeof(tokenMatch));

    if (token_match == NULL) {

        return NULL;
    }

    token_match->token = u_strdup(content);
    token_match->next = NULL;

    return token_match;
}

/**
 * Frees a tokenMatch
 *
 * token_match  : The tokenMatch that needs to be freed
 */
void free_token_match(tokenMatch* token_match) {

    free(token_match->token);
    free(token_match);
}

/**
 * This structure represents the list of the tokens in the token list that match a graphInfo
 *
 * info   : The associated graphInfo used to search matching tokens
 * first  : The first tokenMatch of the linked list
 */
typedef struct graphInfoMatch {

    graphInfo* info;
    tokenMatch* first;

} graphInfoMatch;

/**
 * Creates a new graphInfoMatch from an existing graphInfo.
 *
 * graph_info : The graphInfo associated that indicates which kind of tokens should be searched
 *
 * Returns the newly created graphInfoMatch, or NULL if it couldn't be created
 */
graphInfoMatch* new_graph_info_match(graphInfo* graph_info) {

    graphInfoMatch* graph_info_match = (graphInfoMatch*) malloc(sizeof(graphInfoMatch));

    if (graph_info_match == NULL) {

        return NULL;
    }

    graph_info_match->info = graph_info;
    graph_info_match->first = NULL;

    return graph_info_match;
}

/**
 * Adds a new match to the list of the given graphInfoMatch. The match is copied so the original needs to be freed manually
 *
 * match            : The match to add to the list of the graphInfoMatch
 * graph_info_match : The graphInfoMatch in which to add the new match
 */
void add_match_to_graph_info_match(const unichar* match, graphInfoMatch* graph_info_match) {

    tokenMatch* new_match = new_token_match(match);

    if (new_match == NULL) {

        return;
    }
    
    tokenMatch* current_match = graph_info_match->first;

    int comparison = -1;

    // If the current list is empty or if the new match should be placed at the beginning of the list
    if (current_match == NULL || (comparison = u_strcmp(new_match->token, current_match->token)) <= 0) {

        if (comparison == 0) {

            free_token_match(new_match);
            return;
        }

        graph_info_match->first = new_match;
        new_match->next = current_match;
        return;
    }

    while (current_match->next != NULL &&
          (comparison = u_strcmp(new_match->token, current_match->next->token)) > 0) {

        current_match = current_match->next;
    }

    if (comparison == 0) { // If the token already exist, it is not added a second time

        free_token_match(new_match);
    }
    else {

        new_match->next = current_match->next;
        current_match->next = new_match;
    }
}

/**
 * char version of the add_match_to_graph_info_match function
 */
inline void add_match_to_graph_info_match(const char* match, graphInfoMatch* graph_info_match) {

    unichar* copy = u_strdup(match);
    add_match_to_graph_info_match(copy, graph_info_match);
    free(copy);
}

/**
 * Frees a graphInfoMatch.
 * WARNING, this function DOES NOT free the associated graphInfo, since this one was created separately
 *
 * graph_info_match : The graphInfoMatch that needs to be freed
 */
void free_graph_info_match(graphInfoMatch* graph_info_match) {

    tokenMatch* to_delete = graph_info_match->first;

    while (to_delete != NULL) {

        tokenMatch* next = to_delete->next;
        free_token_match(to_delete);
        to_delete = next;
    }

    free(graph_info_match);
}

/**
 * This structure represents a list of graphInfoMatch
 *
 * match_list       : The list of graphInfoMatch
 * match_list_size  : The number of graphInfoMatch in the list
 */
typedef struct graphInfoMatchList {

    graphInfoMatch** match_list;
    size_t match_list_size;

} graphInfoMatchList;

/**
 * Creates a new graphInfoMatchList
 *
 * Returns the newly created graphInfoMatchList, or NULL if it couldn't be created
 */
graphInfoMatchList* new_graph_info_match_list() {

    graphInfoMatchList* graph_info_match_list = (graphInfoMatchList*) malloc(sizeof(graphInfoMatchList));

    if (graph_info_match_list == NULL) {

        return NULL;
    }

    graph_info_match_list->match_list = NULL;
    graph_info_match_list->match_list_size = 0;

    return graph_info_match_list;
}

/**
 * Adds a graphInfoMatch to a graphInfoMatchList
 *
 * graph_info_match : The graphInfoMatch to add to the list
 * graph_info_match_list  : The graphInfoMatchList in which to add the graphInfoMatch
 */
void add_graph_info_match_to_list(graphInfoMatch* graph_info_match, graphInfoMatchList* list) {

    list->match_list = (graphInfoMatch**) realloc(list->match_list, sizeof(graphInfoMatch*) * (list->match_list_size + 1));
    list->match_list[list->match_list_size] = graph_info_match;

    list->match_list_size++;
}

/**
 * Frees a graphInfoMatchList
 *
 * graph_info_match_list  : The graphInfoMatchList that needs to be freed
 */
void free_graph_info_match_list(graphInfoMatchList* graph_info_match_list) {

    // For each graphInfoMath in the list
    for (unsigned int i = 0; i < graph_info_match_list->match_list_size; i++) {

        free_graph_info_match(graph_info_match_list->match_list[i]);
    }

    free(graph_info_match_list->match_list);
    free(graph_info_match_list);
}


/**
 * Extracts the information inside the given line of the file, and fills the given graphInfo with them.
 *
 * file         : The file in which to extract the information
 * line_number  : The number of the line to inspect in the given file
 * reference_line_number  : The line number of the line on which the description of the graph boxes begins
 * graph_info             : The graphInfo to fill the extracted information with
 */
void extract_G_box_info(graphFile* file, const unsigned int line_number, const unsigned int reference_line_number,
                        graphInfo* graph_info) {

    graph_info->reference_location = reference_line_number;
    graph_info->begining_location = line_number - graph_info->reference_location;

    const unichar* current_line = file->lines[line_number];

    // We search the number of the line of the following box, indicated by the last number on the current line
    // The last token of the token list should be that last number on the line
    stringTokenList* string_token_list = new_token_list_from_string(current_line);

    // Extracting the number out of the token
    const unichar* last_token = string_token_list->token_list[string_token_list->list_size - 1];
    const size_t last_token_size = u_strlen(last_token);
    unsigned int next_box_line_number = 0;

    // For each digit composing the last number, we get their value as unsigned int by using the ASCII table
    for (unsigned int i = 0; i < last_token_size; i++) {

        const char c = (char) last_token[i];
        const unsigned int current_digit_value = (unsigned int) (c - '0'); // All the digits are following '0' in the ASCII table
        next_box_line_number = next_box_line_number * 10 + current_digit_value;
    }

    graph_info->end_location = next_box_line_number; // The location is already relative to reference_location here

    free_string_token_list(string_token_list);
}

/**
 * Extracts the information of the box following the 'G' box.
 *
 * file       : The file in which to extract the information
 * graph_info : The graphInfo to fill the information with
 */
void extract_following_box_info(graphFile* file, graphInfo* graph_info) {

    const unichar* current_line = file->lines[graph_info->reference_location + graph_info->end_location];

    stringTokenList* string_token_list = new_token_list_from_string(current_line);
    unichar* box_content = u_strdup(string_token_list->token_list[0]); // The first token is the one containing the content of the box

    /** A box content should have a form like :
     *
     * "filterCategory[.finalCategory]/,.searchedCategory"
     *
     * So the algorithm is the following :
     *
     * - We search the index of the '/' in the whole content
     * - We make a sub string with the part of the content to the left of the '/'
     * - We search the index of the '.' in that left part
     * - If the '.' is found, the filter category and the final category are extracted from the left and the right part of the '.'
     * - Otherwise the whole part is the filter category
     * - Another sub string is made from the content to the right of the '/'
     * - The index of the '.' in that sub string is searched
     * - The part to the right of the '.' is keeped as the searched_category
     */

    unichar* pointer_to_slash = u_strchr(box_content, '/');
    const size_t index_of_slash = (size_t) (pointer_to_slash - box_content);

    // The sub string begins one character after the beginning of the box content, bacause of the first "
    const size_t left_part_length = index_of_slash - 1;
    unichar* left_part = u_strdup(box_content + 1, left_part_length);

    // For the right part we need to ignore both the final " and the '/'
    const size_t right_part_length = u_strlen(box_content) - index_of_slash - 1 - 1;
    unichar* right_part = u_strdup(pointer_to_slash + 1, right_part_length);

    // Searching the '.' in the left part
    unichar* pointer_to_dot = u_strchr(left_part, '.');

    if (u_strcmp(left_part, "<E>") == 0) {

        graph_info->filter_category = u_strdup("");
        graph_info->final_category = u_strdup("");
    }
    else if (pointer_to_dot != NULL) { // The '.' was found in the left part

        const size_t index_of_dot = (size_t) (pointer_to_dot - left_part);

        const size_t filter_category_length = index_of_dot;
        const size_t final_category_length = left_part_length - index_of_dot - 1; // -1 to ignore the '.'

        graph_info->filter_category = u_strdup(left_part, filter_category_length);
        graph_info->final_category = u_strdup(pointer_to_dot + 1, final_category_length); // +1 to ignore the '.'
    }
    else {

        graph_info->filter_category = u_strdup(left_part);
        graph_info->final_category = u_strdup("");
    }

    // Seeking the searched_category
    unichar* pointer_to_category_beginning = u_strchr(right_part, '.');
    const size_t index_of_category_beginning = (size_t) (pointer_to_category_beginning - right_part);
    const size_t searched_category_length = right_part_length - index_of_category_beginning - 1; // -1 to ignore the '.'

    graph_info->category = u_strdup(pointer_to_category_beginning + 1, searched_category_length);

    free(box_content);
    free(left_part);
    free(right_part);
    free_string_token_list(string_token_list);
}

/**
 * Creates a graphInfoList that contains all the graphInfo for a single .grf file
 *
 * The algorithm is the following :
 * For each line of the graph file, we search those containing "$G", which represent the beginning of a generic graph
 * If such a line is found, a new graphInfo is created.
 * - The location of the "$G" box is stored
 * - The location of the box following the "$G" box is stored as well, it indicates the end of the generic graph
 * - Then the category, the filter category and the final category (see graphInfo definition) are seeked and stored
 *
 * See Unitex manual for description of the .grf file format
 *
 * The newly created graphInfo is then added to the list, and this list is returned at the end
 *
 * graph_file : The file in which the graphInfo are seeked
 *
 * Returns the list of all the found graphInfo for the given graphFile. May be empty, or NULL if the creation of the list has failed
 */
graphInfoList* get_graph_info_list_from_graph_file(graphFile* graph_file) {

    graphInfoList* graph_info_list = new_graph_info_list();

    if (graph_info_list == NULL) {

        return NULL;
    }

    unsigned int reference_line_number = 0;

    // For each line of the graph file
    for (unsigned int current_line_number = 0; current_line_number < graph_file->lines_number; current_line_number++) {

        const unichar* current_line = graph_file->lines[current_line_number];
        const size_t current_line_length = u_strlen(current_line);

        // If we reach the begining of the graph description ('#' line), we store the number of the line + 2
        // because the first line only indicates the number of box in the graph
        if (current_line_length == 1 && current_line[0] == '#') {

            reference_line_number = current_line_number + 2;
        }

        // If we find a 'G' box, a new graphInfo is created
        if (u_starts_with(current_line, "\"$G/{")) {

            graphInfo* graph_info = new_graph_info();

            if (graph_info == NULL) {

                free_graph_info_list(graph_info_list);
                return NULL;
            }

            extract_G_box_info(graph_file, current_line_number, reference_line_number, graph_info);
            extract_following_box_info(graph_file, graph_info);

            add_graph_info_to_list(graph_info, graph_info_list);
        }
    }

    return graph_info_list;
}

/**
 * Returns the first occurence of a specified token in a given string, or NULL if the token wasn't found in the string
 * The search starts from the given position and goes forward until it reaches the end of the string
 *
 * line           : The string in which the token will be searched
 * start_position : The starting position of the research, from which the cursor will go forward
 * token          : The sequance ton find in the given string
 *
 * Returns the first occurence of th researched token in the line, or NULL if the token wasn't found
 */
unichar* first_occurence_of(const unichar* line, unichar* start_position, const unichar* token) {

    const size_t line_length = u_strlen(line);
    const size_t token_length = u_strlen(token);
    unichar* cursor = start_position;
    unsigned int progress = 0;

    while (cursor < line + line_length) {

        while (cursor[progress] == token[progress]) {

            progress++;

            if (progress == token_length) {

                return cursor;
            }
        }

        progress = 0;
        cursor++;
    }

    return NULL;
}

/**
 * char version of the first_occurence_of function
 */
inline unichar* first_occurence_of(const unichar* line, unichar* start_position, const char* token) {

    unichar* copy = u_strdup(token);
    unichar* result = first_occurence_of(line, start_position, copy);
    free(copy);
    return result;
}

/**
 * Returns the last occurence of a specified token in a given string, or NULL if the token wasn't found in the string
 * The search starts from the given position and goes backward until it reaches the beginning of the string
 *
 * line           : The string in which the token will be searched
 * start_position : The starting poisition of the research, from which the cursor will go backward
 * token          : The sequence to find in the given string
 *
 * Returns the last occurence of the researched token in the line, or NULL if the token wasn't found
 */
unichar* last_occurence_of(const unichar* line, unichar* start_position, const unichar* token) {

    const size_t token_length = u_strlen(token);
    unichar* cursor = start_position;
    unsigned int progress = 0;

    while (cursor >= line) {

        while (cursor[progress] == token[progress]) {

            progress++;

            if (progress == token_length) {

                return cursor;
            }
        }

        progress = 0;
        cursor--;
    }

    return NULL;
}

/**
 * char version of the last_occurence_of function
 */
inline unichar* last_occurence_of(const unichar* line, unichar* start_position, const char* token) {

    unichar* copy = u_strdup(token);
    unichar* result = last_occurence_of(line, start_position, copy);
    free(copy);
    return result;
}

/**
 * Returns the first occurence of a character that is part of a set, in a given line. The research goes forward, and returns NULL if no occurence was found
 *
 * line           : The line in which the character will be searched
 * start_position : The position at which the research will start
 * set            : The est of characters that must be searched in the line
 *
 * Returns the first occurence of any character of the set in the given line, or NULL if nothing was found
 */
unichar* first_occurence_in_set_of(const unichar* line, unichar* start_position, const unichar* set) {

    const unichar* line_end = line + u_strlen(line);
    const size_t set_length = u_strlen(set);

    for (unichar* cursor = start_position; cursor < line_end; cursor++) {

        for (unsigned int i = 0; i < set_length; i++) {

            if (*cursor == set[i]) {

                return cursor;
            }
        }
    }

    return NULL;
}

/**
 * char version of the first_occurence_in_set_of function
 */
inline unichar* first_occurence_in_set_of(const unichar* line, unichar* start_position, const char* set) {

    unichar* copy = u_strdup(set);
    unichar* result = first_occurence_in_set_of(line, start_position, copy);
    free(copy);
    return result;
}

/**
 * Returns the last occurence of a character that is part of a set, in a given line. The research goes backward, and returns NULL if no occurence was found
 *
 * line           : The line in which the character will be searched
 * start_position : The position at which the research will start
 * set            : The est of characters that must be searched in the line
 *
 * Returns the last occurence of any character of the set in the given line, or NULL if nothing was found
 */
unichar* last_occurence_in_set_of(const unichar* line, unichar* start_position, const unichar* set) {

    const size_t set_length = u_strlen(set);

    for (unichar* cursor = start_position; cursor >= line; cursor--) {

        for (unsigned int i = 0; i < set_length; i++) {

            if (*cursor == set[i]) {

                return cursor;
            }
        }
    }

    return NULL;
}

/**
 * char version of the last_occurence_in_set_of function
 */
inline unichar* last_occurence_in_set_of(const unichar* line, unichar* start_position, const char* set) {

    unichar* copy = u_strdup(set);
    unichar* result = last_occurence_in_set_of(line, start_position, copy);
    free(copy);
    return result;
}

/**
 * Extracts the category of a token or a sub tokens. The research is done backward, the start position indicated must be placed after the category that needs to be found
 *
 * line           : The line representing the token or the sub token
 * start_position : The position in the line at which the research will begin
 * is_sub_token   : Indicates if the line represents a token or a sub token. It changes the way the category is searched in the line
 *
 * Returns the category of the token or sub token, or NULL if no category was found
 */
unichar* extract_token_category(const unichar* line, unichar* start_position, bool is_sub_token) {

    unichar* bracket_occurence = last_occurence_in_set_of(line, start_position, is_sub_token ? "\\}" : "}");

    if (bracket_occurence == NULL) {

        return NULL;
    }

    unichar* dot_occurence = last_occurence_of(line, bracket_occurence, is_sub_token ? "\\." : ".");

    if (dot_occurence == NULL) {

        return NULL;
    }

    unichar* category_end = first_occurence_in_set_of(line, dot_occurence + 1, is_sub_token ? "\\" : "+:/}");

    const size_t category_length = category_end - dot_occurence - (is_sub_token ? 2 : 1);
    unichar* category = u_strndup(dot_occurence + (is_sub_token ? 2 : 1), category_length);

    return category;
}

/**
 * Extracts the content of a token. If the token contains sub tokens, the content of the sub tokens is integrated in the final content of the token
 *
 * line     : The line that represent the content of the token before the extraction.
 * The line must contain only the internal content of the token, which is delimited by the first '{' and the ','
 *
 * line_end : The delimiter of the end of the line
 *
 * Returns the content of the token
 */
unichar* extract_token_content(unichar* line, unichar* line_end) {

    stringTokenList* token_list = new_string_token_list();
    unichar* cursor = line_end;
    unichar* part_ending = cursor;

    while (cursor >= line) {

        // If we encounter either a '\', a '{' or a '}', we have to skip the part
        if (u_strchr("\\{}", *cursor) != NULL) {

            unichar* part = u_strndup(cursor + 1, part_ending - cursor);
            //u_strcat(content, part);
            add_token_to_token_list(part, token_list);
            free(part);

            // Depending of the encountered character, the way to skip the part is different
            if (*cursor == '\\') {

                cursor--;
            }
            else if (*cursor == '}') {

                unichar* comma_index = last_occurence_of(line, cursor, ",");
                cursor = comma_index - 1;
            }
            else { // '{'

                cursor -= 2;
            }

            part_ending = cursor;
        }
        else {

            cursor--;
        }
    }

    unichar* part = u_strndup(cursor + 1, part_ending - cursor);
    add_token_to_token_list(part, token_list);
    free(part);
    Ustring* content = new_Ustring();

    // Reversing the found parts
    for (int i = token_list->list_size - 1; i >= 0; i--) {

        u_strcat(content, token_list->token_list[i]);
    }

    unichar* final_content = u_strdup(content->str);
    free_string_token_list(token_list);
    free_Ustring(content);

    return final_content;
}

/**
 * Adds the matching tokens in the given line to the graphInfoMatch
 *
 * line   : The line in which the tokens must be extracted
 * match  : The graphInfoMatch in which the found tokens will be added
 */
inline void extract_matching_tokens(unichar* line, graphInfoMatch* match) {

    const size_t line_length = u_strlen(line);
    graphInfo* info = match->info;

    unichar* comma_index = last_occurence_of(line, line + line_length, ",");

    if (u_strlen(info->filter_category) == 0) { // If there is no filter the whole token content is added to the match

        // +1 to skip the first '{'
        // -1 to skip the ','
        unichar* content = extract_token_content(line + 1, comma_index - 1);
        add_match_to_graph_info_match(content, match);
        free(content);
    }
    else { // Otherwise we need to look for sub tokens that match the filter

        unichar* filter = extract_token_category(line, comma_index, true);

        while (filter != NULL) {

            unichar* bracket_index = last_occurence_of(line, comma_index, "}");
            comma_index = last_occurence_of(line, bracket_index, ",");

            if (u_strcmp(filter, info->filter_category) == 0) {

                // We have to find the beginning of the sub token
                int opening_ending_counter = 0;
                unichar* cursor = comma_index;

                while (opening_ending_counter != 1) {

                    cursor--;

                    if (*cursor == '\\') {

                        if (*(cursor + 1) == '{') {

                            opening_ending_counter++;
                        }
                        else if (*(cursor + 1) == '}') {

                            opening_ending_counter--;
                        }
                    }
                }

                // +2 to skip the '\{'
                // -2 to skip the '\,'
                unichar* content = extract_token_content(cursor + 2, comma_index - 2);
                add_match_to_graph_info_match(content, match);
                free(content);

                comma_index = cursor;
            }

            free(filter);
            filter = extract_token_category(line, comma_index, true);
        }
    }
}

/**
 * Creates a list of graphInfoMatch, with each of them containing the matching tokens
 *
 * token_file       : The file containing all the tokens previously matched in the cascade
 * graph_info_list  : The list of graphInfo containing information about the tokens that should be matched
 *
 * Returns a graphInfoMatchList containing all the matched tokens for each generic graph
 */
graphInfoMatchList* get_matching_list(graphFile* token_file, graphInfoList* graph_info_list) {

    // Initialization *****************************************************************************************
    graphInfoMatchList* graph_info_match_list = new_graph_info_match_list();

    if (graph_info_match_list == NULL) {

        return NULL;
    }

    // Creation of the list of graphInfoMatch associated to the graphInfo
    for (unsigned int i = 0; i < graph_info_list->graph_count; i++)  {

        graphInfo* graph_info = graph_info_list->graph_list[i];
        graphInfoMatch* graph_info_match = new_graph_info_match(graph_info);

        if (graph_info_match == NULL) {

            free_graph_info_match_list(graph_info_match_list);
            return NULL;
        }

        add_graph_info_match_to_list(graph_info_match, graph_info_match_list);
    }

    // Algorithm **********************************************************************************************

    // For each line of the file, starting from the end of the file to the beginning because all the tokens are at the end
    for (unsigned int current_line_number = token_file->lines_number - 1; current_line_number >= 0; current_line_number--) {

        unichar* current_line = token_file->lines[current_line_number];
        const size_t current_line_length = u_strlen(current_line);

        // If the line is not a token created in a previous graph in the cascade
        // That means that we've reached the end of the token in that file, so we can leave the loop
        if (current_line[0] != '{') {

            break;
        }

        unichar* category = extract_token_category(current_line, current_line + current_line_length, false);

        if (category == NULL) {

            continue;
        }

        // For each graphInfo, if the category matches, we can start to search for filters and extract token content
        for (unsigned int i = 0; i < graph_info_match_list->match_list_size; i++) {

            graphInfoMatch* graph_info_match = graph_info_match_list->match_list[i];

            if (u_strcmp(category, graph_info_match->info->category) == 0) { // If the category doesn't match for this one, we skip to the next one

                extract_matching_tokens(current_line, graph_info_match);
            }
        }

        free(category);
    }

    return graph_info_match_list;
}

/**
 * Replaces the content of a box in a graphFile.
 * The content is copied so the string that is passed to the function must be freed manually
 *
 * file         : The graphFile in which the modification will be done
 * line_number  : The number of the line in the graph (not based on the reference_location) of the box to replace the content
 * content      : The content that will replace the previous box content
 */
inline void replace_box_content(graphFile* file, const unsigned int line_number, const unichar* content) {

    // Setting the content between quotes
    Ustring* new_line = new_Ustring();
    u_strcatf(new_line, "\"%S\" ", content);

    stringTokenList* string_token_list = new_token_list_from_string(file->lines[line_number]);

    for (unsigned int token_number = 1; token_number < string_token_list->list_size; token_number++) {

        u_strcatf(new_line, "%S ", string_token_list->token_list[token_number]);
    }

    replace_graph_file_line(file, line_number, new_line->str);

    free_Ustring(new_line);
    free_string_token_list(string_token_list);
}

/**
 * char version of the replace_box_content function
 */
inline void replace_box_content(graphFile* file, const unsigned int line_number, const char* content) {

    unichar* copy = u_strdup(content);
    replace_box_content(file, line_number, copy);
    free(copy);
}

/**
 * Creates a box with the given content, and leading to the indicated box. The content is copied so the original string sent to the function
 * must be freed manually.
 *
 * file : The file in which the box will be added
 * content  : The content of the box that will be created.
 * next_box_line_number : The number of the line on which the newly created box will leads to. This number must be the line based on
 * reference_location and not the absolute line location in the file
 */
inline void create_box_leading_to(graphFile* file, const unichar* content, const unsigned int next_box_line_number) {

    Ustring* new_box_content = new_Ustring();

    u_strcatf(new_box_content, "\"%S\" 100 100 1 %u ", content, next_box_line_number);
    add_line_to_file(file, new_box_content->str);

    free_Ustring(new_box_content);
}

/**
 * char version of the create_box_leading_to function
 */
inline void create_box_leading_to(graphFile* file, const char* content, const unsigned int next_box_line_number) {

    unichar* copy = u_strdup(content);
    create_box_leading_to(file, copy, next_box_line_number);
    free(copy);
}

/**
 * Creates the updated graph from the graphInfoMatchList extracted from the original generic graph
 *
 * The process is the following :
 * For each graphInfoMatch in the list, we get the useful info in the contained graphInfo
 * The G box is replaced by a negative right context
 * In between boxes are created to match the matched tokens
 * The content of the final box is replaced by the closure
 *
 * new_graph_file_name  : The name of the graph that is created
 * config               : The encoding config
 * read_file            : The original graph from which the graphInfo has been extracted
 * info_match_list      : The graphInfoMatchList created from the extracted graphInfo
 *
 * Returns 1 if the operation was successful, 0 otherwise
 */
unsigned int create_updated_graph(const char* new_graph_file_name, VersatileEncodingConfig* config,
                                  graphFile* read_file, graphInfoMatchList* info_match_list) {

    enum Status { FAIL = 0, SUCCESS = 1 };

    // Used to put the correct number at the end of lines that indicated the line of the next box
    unsigned int last_file_line_number = read_file->lines_number;

    // For each graphInfoMatch
    for (unsigned int i = 0; i < info_match_list->match_list_size; i++) {

        const graphInfoMatch* graph_info_match = info_match_list->match_list[i];
        const graphInfo* graph_info = graph_info_match->info;
        const unsigned int start_line_number = graph_info->reference_location;
        const unsigned int g_box_line_number = graph_info->begining_location + start_line_number;
        const unsigned int following_box_line_number = graph_info->end_location + start_line_number;

        // String buffer
        Ustring* box_content = new_Ustring();

        // The G box is replaced by a negative right context box, and the destination is changed to the last line of the file
        stringTokenList* g_box_token_list = new_token_list_from_string(read_file->lines[g_box_line_number]);

        u_strcatf(box_content, "\"$![\"");

        for (unsigned int token_number = 1; token_number < g_box_token_list->list_size - 1; token_number++) {

            u_strcatf(box_content, " %S", g_box_token_list->token_list[token_number]);
        }

        free_string_token_list(g_box_token_list);

        u_strcatf(box_content, " %u ", last_file_line_number - start_line_number); // Changing the destination

        replace_graph_file_line(read_file, g_box_line_number, box_content->str);

        // Creation of the new boxes ********************************************************************************

        // Negative right context content
        empty(box_content);

        u_strcatf(box_content, "<%S>", graph_info->category);

        if (u_strlen(graph_info->filter_category) > 0) { // If the filter category exist

           u_strcatf(box_content, "+<%S>", graph_info->filter_category);
        }

        if (u_strlen(graph_info->final_category) > 0) { // If the final category exist

            u_strcatf(box_content, "+<%S>", graph_info->final_category);
        }

        create_box_leading_to(read_file, box_content->str, last_file_line_number + 1 - start_line_number);
        last_file_line_number++;

        // End of the negative right context
        create_box_leading_to(read_file, "$]", last_file_line_number + 1 - start_line_number);
        last_file_line_number++;

        // Creation of the box containing the matched tokens
        empty(box_content);

        // If there is no match for a G box, an empty box pointing to itself is created to cut the path
        if (graph_info_match->first == NULL) {

            u_strcatf(box_content, "<E>");
            create_box_leading_to(read_file, box_content->str, last_file_line_number - start_line_number);
        }
        else {

            u_strcatf(box_content, "%S", graph_info_match->first->token);

            for (tokenMatch* token_match = graph_info_match->first->next; token_match != NULL; token_match = token_match->next) {

                u_strcatf(box_content, "+%S", token_match->token);
            }

            // Two '{' if the filter category exist, only one otherwise
            u_strcatf(box_content, "/%s", u_strlen(graph_info->filter_category) > 0 ? "{{" : "{");

            create_box_leading_to(read_file, box_content->str, graph_info->end_location); // This box points back to the previous end box
        }

        last_file_line_number++;

        // Finally, we need to replace the content of the previous end box by the closure of the tokens
        empty(box_content);

        u_strcatf(box_content, "<E>/");

        if (u_strlen(graph_info->filter_category) > 0) { // If the filter category exist

          u_strcatf(box_content, ",.%S}", graph_info->filter_category);
        }

        // If the final category exist, it replaces the normal category
        u_strcatf(box_content, ",.%S",
          u_strlen(graph_info->final_category) > 0 ? graph_info->final_category : graph_info->category);

        replace_box_content(read_file, following_box_line_number, box_content->str);

        // Finally, the number of boxes in the graph is updated
        empty(box_content);

        u_strcatf(box_content, "%u", read_file->lines_number - start_line_number);
        replace_graph_file_line(read_file, start_line_number - 1, box_content->str);

        free_Ustring(box_content);
    }

    unsigned int file_creation_status = print_graph_file(read_file, new_graph_file_name, config);
    return file_creation_status;
}

typedef struct {
    char transducer_list_file_name[FILENAME_MAX];
    char text_file_name[FILENAME_MAX];
    char alphabet_file_name[FILENAME_MAX];
    char name_uima_offsets_file[FILENAME_MAX];
    char name_input_offsets_file[FILENAME_MAX];
    char transducer_filename_prefix[FILENAME_MAX];
    char extension_text_name[FILENAME_MAX];
    char language[FILENAME_MAX];
    char stdoff_file[FILENAME_MAX];
} Cassys_text_buffer;

typedef struct {
    char last_labeled_text_name[FILENAME_MAX];
    char orig_grf[FILENAME_MAX];
    char template_name_without_extension[FILENAME_MAX];
    //              char graph_file_name_n[FILENAME_MAX];
    char result_file_name_XML[FILENAME_MAX];
    char text_name_without_extension[FILENAME_MAX];
    char path[FILENAME_MAX];
    char last_resulting_text_path[FILENAME_MAX];
    char result_file_name_path_XML[FILENAME_MAX];
    char result_file_name_path_offset[FILENAME_MAX + 0x20];
    char result_file_name_raw[FILENAME_MAX];
    char result_file_name_path_raw[FILENAME_MAX];
    char graph_file_name[FILENAME_MAX];
} Cascade_text_buffer;


int main_Cassys(int argc,char* const argv[]) {
    if (argc==1) {
        usage();
        return SUCCESS_RETURN_CODE;
    }

    Cassys_text_buffer* textbuf = (Cassys_text_buffer*)malloc(sizeof(Cassys_text_buffer));
    if (textbuf == NULL) {
        alloc_error("main_Cassys");
        return ALLOC_ERROR_CODE;
    }

    char* morpho_dic = NULL;

    bool has_transducer_list = false;

    bool has_text_file_name = false;

    bool has_alphabet = false;
    char negation_operator[0x20];

    VersatileEncodingConfig vec=VEC_DEFAULT;
    int must_create_directory = 1;
    int in_place = 0;
    int realign_token_graph_pointer = 0;
    int translate_path_separator_to_native = 0;
    int dump_graph = 0; // By default, don't build a .dot file.
    int display_perf = 0;
    int produce_offsets_file = 0;
    int istex_param = 0;
// define CASSYS_DEFAULT_TEMP_WORK_DIR with a default location (probably in virtual system file) to
//  build a version of k6 which uses this temp location and perform cleanup
#ifdef CASSYS_DEFAULT_TEMP_WORK_DIR
#define CASSYS_STRINGIZE(x) #x
#define CASSYS_STRINGIZE2(x) CASSYS_STRINGIZE(x)
#define CASSYS_DEFAULT_TEMP_WORK_DIR_AS_STRING CASSYS_STRINGIZE2(CASSYS_DEFAULT_TEMP_WORK_DIR)
#endif

#ifdef CASSYS_DEFAULT_TEMP_WORK_DIR_AS_STRING
    int must_do_temp_cleanup = 1;
    char* temp_work_dir = strdup(CASSYS_DEFAULT_TEMP_WORK_DIR_AS_STRING);
#else
    int must_do_temp_cleanup = 0;
    char* temp_work_dir = NULL;
#endif

    struct transducer_name_and_mode_linked_list* transducer_name_and_mode_linked_list_arg=NULL;

    vector_ptr* tokenize_additional_args = new_vector_ptr();
    if (tokenize_additional_args == NULL) {
        alloc_error("main_Cassys");
        free(temp_work_dir);
        free(textbuf);
        return ALLOC_ERROR_CODE;
    }

    vector_ptr* locate_additional_args   = new_vector_ptr();
    if (locate_additional_args   == NULL) {
        alloc_error("main_Cassys");
        free_vector_ptr(tokenize_additional_args, free);
        free(temp_work_dir);
        free(textbuf);
        return ALLOC_ERROR_CODE;
    }

    vector_ptr* concord_additional_args  = new_vector_ptr();
    if (tokenize_additional_args == NULL) {
        alloc_error("main_Cassys");
        free_vector_ptr(locate_additional_args, free);
        free_vector_ptr(tokenize_additional_args, free);
        free(temp_work_dir);
        free(textbuf);
        return ALLOC_ERROR_CODE;
    }

    // decode the command line
    int val;
    int index = 1;
    negation_operator[0]='\0';
    textbuf->transducer_filename_prefix[0]='\0';
    textbuf->name_uima_offsets_file[0] = '\0';
    textbuf->name_input_offsets_file[0] = '\0';
    textbuf->language[0] = '\0';
    textbuf->stdoff_file[0] = '\0';
    bool only_verify_arguments = false;
    UnitexGetOpt options;
    while (EOF != (val=options.parse_long(argc, argv, optstring_Cassys,
                                          lopts_Cassys, &index))) {
        switch (val) {
        case 'V': only_verify_arguments = true;
                  break;
        case 'h': usage();
                  free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                  free_vector_ptr(concord_additional_args, free);
                  free_vector_ptr(locate_additional_args, free);
                  free_vector_ptr(tokenize_additional_args, free);
                  free(temp_work_dir);
                  free(morpho_dic);
                  free(textbuf);
                  return SUCCESS_RETURN_CODE;
        case 'k': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_encoding argument\n");
                free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                free_vector_ptr(concord_additional_args, free);
                free_vector_ptr(locate_additional_args, free);
                free_vector_ptr(tokenize_additional_args, free);
                free(temp_work_dir);
                free(morpho_dic);
                free(textbuf);
                return USAGE_ERROR_CODE;
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
        case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                free_vector_ptr(concord_additional_args, free);
                free_vector_ptr(locate_additional_args, free);
                free_vector_ptr(tokenize_additional_args, free);
                free(temp_work_dir);
                free(morpho_dic);
                free(textbuf);
                return USAGE_ERROR_CODE;
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
        case 't': {
            if (options.vars()->optarg[0] == '\0') {
                error("Command line error : Empty file name argument\n");
                free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                free_vector_ptr(concord_additional_args, free);
                free_vector_ptr(locate_additional_args, free);
                free_vector_ptr(tokenize_additional_args, free);
                free(temp_work_dir);
                free(morpho_dic);
                free(textbuf);
                return USAGE_ERROR_CODE;
            }

            get_extension(options.vars()->optarg, textbuf->extension_text_name);
            if (strcmp(textbuf->extension_text_name, ".snt") != 0) {
                error("Command line error : File name argument %s must be a preprocessed snt file\n",
                        options.vars()->optarg);
                free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                free_vector_ptr(concord_additional_args, free);
                free_vector_ptr(locate_additional_args, free);
                free_vector_ptr(tokenize_additional_args, free);
                free(temp_work_dir);
                free(morpho_dic);
                free(textbuf);
                return USAGE_ERROR_CODE;
            }

            strcpy(textbuf->text_file_name, options.vars()->optarg);
            has_text_file_name = true;

            break;
        }
        case 'l': {
            if(options.vars()->optarg[0] == '\0'){
                error("Command line error : Empty transducer list argument\n");
                free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                free_vector_ptr(concord_additional_args, free);
                free_vector_ptr(locate_additional_args, free);
                free_vector_ptr(tokenize_additional_args, free);
                free(temp_work_dir);
                free(morpho_dic);
                free(textbuf);
                return USAGE_ERROR_CODE;
            } else {
                strcpy(textbuf->transducer_list_file_name, options.vars()->optarg);
                has_transducer_list = true;
            }
            break;
        }
        case 'r': {
            if(options.vars()->optarg[0] == '\0'){
                error("Command line error : Empty transducer directory argument\n");
                free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                free_vector_ptr(concord_additional_args, free);
                free_vector_ptr(locate_additional_args, free);
                free_vector_ptr(tokenize_additional_args, free);
                free(temp_work_dir);
                free(morpho_dic);
                free(textbuf);
                return USAGE_ERROR_CODE;
            } else {
                strcpy(textbuf->transducer_filename_prefix, options.vars()->optarg);
                has_transducer_list = true;
            }
            break;
        }
        case 's': {
            if(options.vars()->optarg[0] == '\0'){
                error("Command line error : Empty transducer filename argument\n");
                free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                free_vector_ptr(concord_additional_args, free);
                free_vector_ptr(locate_additional_args, free);
                free_vector_ptr(tokenize_additional_args, free);
                free(temp_work_dir);
                free(morpho_dic);
                free(textbuf);
                return USAGE_ERROR_CODE;
            } else {
                transducer_name_and_mode_linked_list_arg=add_transducer_linked_list_new_name(transducer_name_and_mode_linked_list_arg,options.vars()->optarg);
            }
            break;
        }
        case 'm': {
            if(options.vars()->optarg[0] == '\0'){
                error("Command line error : Empty transducer mode argument\n");
                free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                free_vector_ptr(concord_additional_args, free);
                free_vector_ptr(locate_additional_args, free);
                free_vector_ptr(tokenize_additional_args, free);
                free(temp_work_dir);
                free(morpho_dic);
                free(textbuf);
                return USAGE_ERROR_CODE;
            } else {
                set_last_transducer_linked_list_mode_by_string(transducer_name_and_mode_linked_list_arg,options.vars()->optarg);
            }
            break;
        }
        case 'a':{
            if (options.vars()->optarg[0] == '\0') {
                error("Command line error : Empty alphabet argument\n");
                free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                free_vector_ptr(concord_additional_args, free);
                free_vector_ptr(locate_additional_args, free);
                free_vector_ptr(tokenize_additional_args, free);
                free(temp_work_dir);
                free(morpho_dic);
                free(textbuf);
                return USAGE_ERROR_CODE;
            } else {
                strcpy(textbuf->alphabet_file_name, options.vars()->optarg);
                has_alphabet = true;
            }
            break;
        }
        case '$':{
            if (options.vars()->optarg[0] == '\0') {
                error("Command line error : Empty input offsets file argument\n");
                free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                free_vector_ptr(concord_additional_args, free);
                free_vector_ptr(locate_additional_args, free);
                free_vector_ptr(tokenize_additional_args, free);
                free(temp_work_dir);
                free(morpho_dic);
                free(textbuf);
                return USAGE_ERROR_CODE;
            } else {
                strcpy(textbuf->name_input_offsets_file, options.vars()->optarg);
                has_alphabet = true;
            }
            break;
        }
        case 'O': {
            produce_offsets_file = 1;
            break;
        }
        case 'f':{
            if (options.vars()->optarg[0] == '\0') {
                error("Command line error : Empty uima offsets file argument\n");
                free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                free_vector_ptr(concord_additional_args, free);
                free_vector_ptr(locate_additional_args, free);
                free_vector_ptr(tokenize_additional_args, free);
                free(temp_work_dir);
                free(morpho_dic);
                free(textbuf);
                return USAGE_ERROR_CODE;
            } else {
                strcpy(textbuf->name_uima_offsets_file, options.vars()->optarg);
                has_alphabet = true;
            }
            break;
        }
        case 'g': if (options.vars()->optarg[0]=='\0') {
                error("You must specify an argument for negation operator\n");
                free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                free_vector_ptr(concord_additional_args, free);
                free_vector_ptr(locate_additional_args, free);
                free_vector_ptr(tokenize_additional_args, free);
                free(temp_work_dir);
                free(morpho_dic);
                free(textbuf);
                return USAGE_ERROR_CODE;
             }
             if ((strcmp(options.vars()->optarg,"minus")!=0) && (strcmp(options.vars()->optarg,"-")!=0) &&
                 (strcmp(options.vars()->optarg,"tilde")!=0) && (strcmp(options.vars()->optarg,"~")!=0))
             {
                 error("You must specify a valid argument for negation operator\n");
                 free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                 free_vector_ptr(concord_additional_args, free);
                 free_vector_ptr(locate_additional_args, free);
                 free_vector_ptr(tokenize_additional_args, free);
                 free(temp_work_dir);
                 free(morpho_dic);
                 free(textbuf);
                 return USAGE_ERROR_CODE;
             }
             strcpy(negation_operator,options.vars()->optarg);
             break;
        case 'i': {
            in_place = 1;
            break;
        }
        case 'u': {
            dump_graph = 1;
            break;
        }
        case 'c': {
            display_perf = 1;
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
        case 'b': {
            must_do_temp_cleanup = 1;
            break;
        }
        case 'd': {
            must_create_directory = 0;
            break;
        }
        case 'p' : {
            if (options.vars()->optarg[0] != '\0') {
                   if (temp_work_dir != NULL) {
                      free(temp_work_dir);
                   }
                   temp_work_dir = strdup(options.vars()->optarg);
                    if (temp_work_dir == NULL) {
                        alloc_error("main_Cassys");
                        free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                        free_vector_ptr(concord_additional_args, free);
                        free_vector_ptr(locate_additional_args, free);
                        free_vector_ptr(tokenize_additional_args, free);
                        free(morpho_dic);
                        free(textbuf);
                        return ALLOC_ERROR_CODE;
                    }

            }
            break;
        }
        case 'w' : {
            if (options.vars()->optarg[0] != '\0') {
                if (morpho_dic == NULL) {
                    morpho_dic = strdup(options.vars()->optarg);
                    if (morpho_dic == NULL) {
                        alloc_error("main_Cassys");
                        free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                        free_vector_ptr(concord_additional_args, free);
                        free_vector_ptr(locate_additional_args, free);
                        free_vector_ptr(tokenize_additional_args, free);
                        free(temp_work_dir);
                        free(textbuf);
                        return ALLOC_ERROR_CODE;
                    }
                } else {
                    char* more_morpho_dics = (char*) realloc((void*) morpho_dic, strlen(
                            morpho_dic) + strlen(options.vars()->optarg) + 2);
                    if (more_morpho_dics != NULL) {
                        morpho_dic = more_morpho_dics;
                    } else {
                        alloc_error("main_Cassys");
                        free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                        free_vector_ptr(concord_additional_args, free);
                        free_vector_ptr(locate_additional_args, free);
                        free_vector_ptr(tokenize_additional_args, free);
                        free(temp_work_dir);
                        free(morpho_dic);
                        free(textbuf);
                        return ALLOC_ERROR_CODE;
                    }
                    strcat(morpho_dic, ";");
                    strcat(morpho_dic, options.vars()->optarg);
                }
            }
            break;
        }
        case 'T': {
            if (options.vars()->optarg[0] != '\0') {
                char * locate_arg = strdup(options.vars()->optarg);
                if (locate_arg == NULL) {
                    alloc_error("main_Cassys");
                    free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                    free_vector_ptr(concord_additional_args, free);
                    free_vector_ptr(locate_additional_args, free);
                    free_vector_ptr(tokenize_additional_args, free);
                    free(temp_work_dir);
                    free(textbuf);
                    return ALLOC_ERROR_CODE;
                }
                vector_ptr_add(tokenize_additional_args, locate_arg);
            }
            break;
        }
        case 'L': {
            if (options.vars()->optarg[0] != '\0') {
                char * locate_arg = strdup(options.vars()->optarg);
                if (locate_arg == NULL) {
                    alloc_error("main_Cassys");
                    free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                    free_vector_ptr(concord_additional_args, free);
                    free_vector_ptr(locate_additional_args, free);
                    free_vector_ptr(tokenize_additional_args, free);
                    free(temp_work_dir);
                    free(textbuf);
                    return ALLOC_ERROR_CODE;
                }
                vector_ptr_add(locate_additional_args, locate_arg);
            }
            break;
        }
        case 'C': {
            if (options.vars()->optarg[0] != '\0') {
                char * concord_arg = strdup(options.vars()->optarg);
                if (concord_arg == NULL) {
                    alloc_error("main_Cassys");
                    free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                    free_vector_ptr(concord_additional_args, free);
                    free_vector_ptr(locate_additional_args, free);
                    free_vector_ptr(tokenize_additional_args, free);
                    free(temp_work_dir);
                    free(textbuf);
                    return ALLOC_ERROR_CODE;
                }
                vector_ptr_add(concord_additional_args, concord_arg);
            }
            break;
        }
        case 'x': {
            if(options.vars() !=NULL && options.vars()->optarg !=NULL && options.vars()->optarg[0] != '\0') {
                strcpy(textbuf->stdoff_file, options.vars()->optarg);
            }
            istex_param = 1;
            break;
        }
        case 'A': {
            if (options.vars()->optarg[0] == '\0') {
                error("Command line error : Empty language argument\n");
                free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
                free_vector_ptr(concord_additional_args, free);
                free_vector_ptr(locate_additional_args, free);
                free_vector_ptr(tokenize_additional_args, free);
                free(temp_work_dir);
                free(morpho_dic);
                free(textbuf);
                return USAGE_ERROR_CODE;
            } else {
                strcpy(textbuf->language, options.vars()->optarg);
            }
            break;
        } 
        default :{
            error("Invalid option : %c\n",val);
            free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
            free_vector_ptr(concord_additional_args, free);
            free_vector_ptr(locate_additional_args, free);
            free_vector_ptr(tokenize_additional_args, free);
            free(temp_work_dir);
            free(textbuf);
            return USAGE_ERROR_CODE;
        }
        }
    }
    index = -1;

    int command_line_errors = 0;

    if(has_alphabet == false){
        error("Command line error : no alphabet provided\nRerun with --help\n");
        command_line_errors = 1;
    }
    if(has_text_file_name == false){
        error("Command line error : no text file provided\nRerun with --help\n");
        command_line_errors = 1;
    }
    if((has_transducer_list == false) && (transducer_name_and_mode_linked_list_arg == NULL)){
        error("Command line error : no transducer list provided\nRerun with --help\n");
        command_line_errors = 1;
    }

    if(command_line_errors || only_verify_arguments) {
        // freeing all allocated memory
        free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
        free_vector_ptr(concord_additional_args, free);
        free_vector_ptr(locate_additional_args, free);
        free_vector_ptr(tokenize_additional_args, free);
        free(temp_work_dir);
        free(morpho_dic);
        free(textbuf);

        if (command_line_errors) {
          return USAGE_ERROR_CODE;
        }

        return SUCCESS_RETURN_CODE;
    }

    // Load the list of transducers from the file transducer list and stores it in a list
    //struct fifo *transducer_list = load_transducer(transducer_list_file_name);
    if ((transducer_name_and_mode_linked_list_arg == NULL) && has_transducer_list) {
        transducer_name_and_mode_linked_list_arg = load_transducer_list_file(textbuf->transducer_list_file_name, translate_path_separator_to_native);
    }
    struct fifo *transducer_list=load_transducer_from_linked_list(transducer_name_and_mode_linked_list_arg, textbuf->transducer_filename_prefix);

    int return_value = cascade(textbuf->text_file_name, in_place, must_create_directory, must_do_temp_cleanup, temp_work_dir,
        transducer_list, textbuf->alphabet_file_name, textbuf->name_input_offsets_file, produce_offsets_file, textbuf->name_uima_offsets_file, negation_operator,
        &vec, morpho_dic,
        tokenize_additional_args, locate_additional_args, concord_additional_args,
        dump_graph, realign_token_graph_pointer, display_perf, istex_param, textbuf->language,textbuf->stdoff_file);

    free_fifo(transducer_list);
    free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
    free_vector_ptr(concord_additional_args, free);
    free_vector_ptr(locate_additional_args, free);
    free_vector_ptr(tokenize_additional_args, free);
    free(temp_work_dir);
    free(morpho_dic);
    free(textbuf);

    return return_value;
}


typedef struct {
    unsigned int elapsed_time;
    unsigned int index;
    char* name;
} locate_perf_info;


static int compare_perf_info(const locate_perf_info* p1, const locate_perf_info* p2)
{
    if (p1->elapsed_time < p2->elapsed_time)
        return -1;
    if (p1->elapsed_time > p2->elapsed_time)
        return 1;
    if (p1->index < p2->index)
        return -1;
    if (p1->index > p2->index)
        return 1;
    return 0;
}


/**
* Sorts the locate perf info
*/
static void quicksort_recursive(unsigned int nb_items, locate_perf_info* array)
{
    unsigned int i, j;
    locate_perf_info pivot;
    int fOnePermut, fMoveIJ;
    if (nb_items < 2) return;
    pivot = *(array + (nb_items / 2));
    i = 0;
    j = nb_items - 1;
    fOnePermut = 0;
    do
    {
        fMoveIJ = 0;
        while (compare_perf_info(array + i, &pivot) < 0)
        {
            i++;
            fMoveIJ = 1;
        }

        while ((j != 0) && (compare_perf_info(array + j, &pivot) > 0))
        {
            j--;
            fMoveIJ = 1;
        }

        if (i <= j)
        {
            locate_perf_info permut;

            permut = *(array + i);
            *(array + i) = *(array + j);
            *(array + j) = permut;
            if (!fOnePermut)
                if ((compare_perf_info(array + i, &pivot) != 0))
                fOnePermut = 1;
        }
    } while ((i < j) && (fMoveIJ));

    if (fOnePermut || (j + 1<nb_items))
        if (j >= 1) quicksort_recursive(j + 1, array);
    if (fOnePermut || (i>0))
        if (i<nb_items - 1) quicksort_recursive(nb_items - i, array + i);

    if ((i == 0) && (j == 0) && (!fOnePermut) && (nb_items>1))
        quicksort_recursive(nb_items - 1, array + 1);
    else if ((i == nb_items - 1) && (j == i) && (!fOnePermut) && (nb_items>1))
        quicksort_recursive(nb_items - 1, array);
}


static void quicksort(int nb_items, locate_perf_info* transitions) {
    quicksort_recursive(nb_items, transitions);
}


/**
 * The main function of the cascade
 *
 *
 */
int cascade(const char* original_text, int in_place, int must_create_directory,  int must_do_temp_cleanup, const char* temp_work_dir,
    fifo* transducer_list, const char *alphabet,
    const char*name_input_offsets_file, int produce_offsets_file, const char* name_uima_offsets_file,
    const char*negation_operator,
    VersatileEncodingConfig* vec,
    const char *morpho_dic, vector_ptr* tokenize_args, vector_ptr* locate_args, vector_ptr* concord_args,
    int dump_graph, int realign_token_graph_pointer, int display_perf, int istex_param, const char* lang, const char* stdoff_file) {

    unsigned int time_tokenize = 0;
    unsigned int time_grf2fst2 = 0;
    unsigned int time_locate = 0;
    unsigned int time_concord = 0;
    unsigned int time_cascade = 0;

    unsigned int nb_perf_info = 0;
    unsigned int nb_perf_info_allocated  = 0;
    locate_perf_info* p_locate_perf_info = NULL;

    hTimeElapsed htm_cascade = NULL;
    if (display_perf) {
        htm_cascade = SyncBuidTimeMarkerObject();
        nb_perf_info_allocated = 1;
        p_locate_perf_info = (locate_perf_info*)malloc(nb_perf_info_allocated*sizeof(locate_perf_info));
        if (p_locate_perf_info == NULL) {
            alloc_error("cascade");
            return ALLOC_ERROR_CODE;
        }
    }

    Cascade_text_buffer* textbuf = (Cascade_text_buffer*)malloc(sizeof(Cascade_text_buffer));
    if (textbuf == NULL) {
        alloc_error("cascade");
        free(p_locate_perf_info);
        return ALLOC_ERROR_CODE;
    }

    cassys_tokens_allocation_tool* tokens_allocation_tool = build_cassys_tokens_allocation_tool();

    if (must_do_temp_cleanup) {
        in_place = 1;
    }

    const char* text = original_text;
    char* build_text = NULL;
    char* build_work_text_snt_path = NULL;
    char* build_work_text_csc_path = NULL;
    char* build_work_text_csc_work_path = NULL;
    vector_int* uima_offsets = NULL;

    if (name_uima_offsets_file != NULL)
        if ((*name_uima_offsets_file) != 0) {
            uima_offsets = load_uima_offsets(vec, name_uima_offsets_file);
            if (uima_offsets == NULL) {
                error("invalid uima offset file %s\n", name_uima_offsets_file);
            }
        }
    size_t len_work_dir = (temp_work_dir == NULL) ? 0 : strlen(temp_work_dir);

    if ((len_work_dir > 0) || (must_do_temp_cleanup != 0)) {
        size_t len_temp_work_dir = (temp_work_dir == NULL) ? 0 : strlen(temp_work_dir);

            build_work_text_snt_path = (char*)malloc(len_temp_work_dir + (strlen(original_text) * 2) + 0x40);
            if (build_work_text_snt_path == NULL) {
                alloc_error("load_transducer_from_linked_list");
                free_vector_int(uima_offsets);
                free(textbuf);
                free(p_locate_perf_info);
                return ALLOC_ERROR_CODE;
            }

            build_work_text_csc_path = (char*)malloc(len_temp_work_dir + (strlen(original_text) * 2) + 0x40);
            if (build_work_text_csc_path == NULL) {
                alloc_error("load_transducer_from_linked_list");
                free(build_work_text_snt_path);
                free_vector_int(uima_offsets);
                free(textbuf);
                free(p_locate_perf_info);
                return ALLOC_ERROR_CODE;
            }

            build_work_text_csc_work_path = (char*)malloc(len_temp_work_dir + (strlen(original_text) * 2) + 0x40);
            if (build_work_text_csc_work_path == NULL) {
                alloc_error("load_transducer_from_linked_list");
                free(build_work_text_csc_path);
                free(build_work_text_snt_path);
                free_vector_int(uima_offsets);
                free(textbuf);
                free(p_locate_perf_info);
                return ALLOC_ERROR_CODE;
            }

            if (len_temp_work_dir > 0) {
                build_text = (char*)malloc(len_temp_work_dir + strlen(original_text) + 0x10);
                if (build_text == NULL) {
                    alloc_error("load_transducer_from_linked_list");
                    free(build_work_text_csc_work_path);
                    free(build_work_text_csc_path);
                    free(build_work_text_snt_path);
                    free_vector_int(uima_offsets);
                    free(textbuf);
                    free(p_locate_perf_info);
                    return ALLOC_ERROR_CODE;
                }

                strcpy(build_text, (len_temp_work_dir > 0) ? temp_work_dir : original_text);
                char latest_char = *(temp_work_dir + strlen(temp_work_dir) - 1);
                if ((latest_char != '\\') && (latest_char != '/')) {
                    strcat(build_text, PATH_SEPARATOR_STRING);
                }
                remove_path(original_text, build_text + strlen(build_text));


                char* build_original_text_snt_path = (char*)malloc(len_temp_work_dir + (strlen(original_text) * 2) + 0x40);
                if (build_original_text_snt_path == NULL) {
                    alloc_error("load_transducer_from_linked_list");
                    free(build_text);
                    free(build_work_text_csc_work_path);
                    free(build_work_text_csc_path);
                    free(build_work_text_snt_path);
                    free_vector_int(uima_offsets);
                    free(textbuf);
                    free(p_locate_perf_info);
                    return ALLOC_ERROR_CODE;
                }

                get_snt_path(text, build_original_text_snt_path);
                text = build_text;
                get_snt_path(text, build_work_text_snt_path);
                copy_file(text, original_text);
                copy_directory_snt_content(build_work_text_snt_path, build_original_text_snt_path, 0);
                free(build_original_text_snt_path);
            } else {
                get_snt_path(text, build_work_text_snt_path);
            }


            get_csc_wd_path(text, build_work_text_csc_path);
            get_snt_path(build_work_text_csc_path, build_work_text_csc_work_path);


            get_csc_path(text, build_work_text_csc_path);

            if (len_temp_work_dir > 0) {
                make_cassys_directory(build_work_text_snt_path);
            }
        }

    initialize_working_directory_before_tokenize(text, must_create_directory);
        /*Commenting out launch_tokenize_in_Cassys because the call is redundant
         *  and throws the error "Cannot write tok_by_freq.txt" */
    //launch_tokenize_in_Cassys(text,alphabet,NULL,vec,tokenize_args, display_perf);

    //if (in_place == 0)
    initialize_working_directory(text, must_create_directory);

    struct snt_files* snt_text_files = new_snt_files(text);

    struct text_tokens* tokens = NULL;
    cassys_tokens_list* tokens_list = cassys_load_text(vec,snt_text_files->tokens_txt, snt_text_files->text_cod,&tokens, uima_offsets,tokens_allocation_tool);

    u_printf("CasSys Cascade begins\n");

    int transducer_number = 1;
    char* labeled_text_name = NULL;

    if (in_place != 0){
       labeled_text_name = create_labeled_files_and_directory(text,
            0,0,0,0, must_create_directory,0);
    }

    if (is_empty(transducer_list)) {
         if (labeled_text_name != NULL) {
            free(labeled_text_name);
         }
         labeled_text_name = create_labeled_files_and_directory(text,
                 0,0,0,0, must_create_directory,0);
         sprintf(textbuf->last_labeled_text_name, "%s", labeled_text_name);
    }

    int previous_transducer_number = 0;
    int previous_iteration = 0;
    int iteration = 0;
    while (!is_empty(transducer_list)) {

        transducer *current_transducer =
                (transducer*) take_ptr(transducer_list);

        int is_template_grf = current_transducer->generic_graph;

        if ((!is_template_grf) && is_debug_mode(current_transducer, vec) == true) {
            error("graph %s has been compiled in debug mode. Please recompile it in normal mode\n", current_transducer->transducer_file_name);
            free(labeled_text_name);
            free_text_tokens(tokens);
            free_snt_files(snt_text_files);
            free(build_text);
            free(build_work_text_csc_work_path);
            free(build_work_text_csc_path);
            free(build_work_text_snt_path);
            free_vector_int(uima_offsets);
            free_cassys_tokens_allocation_tool(tokens_allocation_tool);
            free(textbuf);
            for (unsigned int loop_display_perf = 0; loop_display_perf < nb_perf_info; loop_display_perf++) {
                free((p_locate_perf_info + loop_display_perf)->name);
            }
            free(p_locate_perf_info);
            return DEFAULT_ERROR_CODE;
        }

        for (iteration = 0; current_transducer->repeat_mode == REPEAT_INFINITY || iteration < current_transducer->repeat_mode; iteration++) {
            if (in_place == 0) {

                if (labeled_text_name != NULL) {
                    free(labeled_text_name);
                }
                labeled_text_name = create_labeled_files_and_directory(text, previous_transducer_number,
                    transducer_number, previous_iteration, iteration, must_create_directory, 1);
            }

            launch_tokenize_in_Cassys(labeled_text_name, alphabet,
                snt_text_files->tokens_txt, vec, tokenize_args, display_perf, display_perf ? &time_tokenize : NULL);

            //int entity = 0;
            char* updated_grf_file_name = NULL;
            char* updated_fst2_file_name = NULL;

            // Used to know if the generic graph has been created or not. If there is at least one match list that is not empty, the graph has to be created
            bool at_least_one_match_list_not_empty = false;

            if (is_template_grf) {
                //int *entity_loc = NULL;
                //int num_annots = 0;
                //unichar *start_node_line = NULL;
                //int start_node_loc = -1;
                //int total_lines = 0;
                //int num_entities = 0;
                //struct grfInfo *grf_infos = NULL;

                remove_extension(current_transducer->transducer_file_name, textbuf->template_name_without_extension);
                sprintf(textbuf->orig_grf, "%s.grf", textbuf->template_name_without_extension);

                u_printf("Loaded file : %s\n", textbuf->orig_grf);
                //unichar **grf_lines = load_file_in_memory(textbuf->orig_grf, vec, &total_lines);
                graphFile* graph_file = load_graph_file(textbuf->orig_grf, vec);
                  
                //grf_infos = extract_info(grf_lines, &num_annots, total_lines, &start_node_loc, &start_node_line, &entity_loc);
                graphInfoList* graph_info_list = get_graph_info_list_from_graph_file(graph_file);

                //if (num_annots > 0) {
                if (graph_info_list->graph_count > 0) {

                    //char* token_list = get_file_in_current_snt(text, transducer_number, iteration, "tok_by_alph", ".txt");
                    char* token_list = get_file_in_current_snt(text, transducer_number, iteration, "tokens", ".txt");

                    graphFile* token_file = load_graph_file(token_list, vec);
                    //unichar**entity_string = extract_entities(token_list,snt_text_files->tok_by_alph_txt, vec, num_annots, &num_entities, grf_infos);
                    graphInfoMatchList* graph_info_match_list = get_matching_list(token_file, graph_info_list);
                    free(token_list);
                    //free(entity_string);

                    updated_grf_file_name = create_updated_graph_filename(text,
                            in_place ? 0 : transducer_number,
                            in_place ? 0 : iteration,
                            filename_without_path(textbuf->template_name_without_extension), ".grf");

                    //if (update_tmp_graph(updated_grf_file_name, vec, grf_lines, total_lines, start_node_line, start_node_loc, num_entities, num_annots, grf_infos, 1)) {
                    if (create_updated_graph(updated_grf_file_name, vec, graph_file, graph_info_match_list)) {

                        //if (num_entities > 0) {
                        for (unsigned int i = 0; i < graph_info_match_list->match_list_size; i++) {

                            graphInfoMatch* graph_info_match = graph_info_match_list->match_list[i];

                            if (graph_info_match->first != NULL) {

                                at_least_one_match_list_not_empty = true;
                                break;
                            }
                        }

                        if (at_least_one_match_list_not_empty) {

                            launch_grf2fst2_in_Cassys(updated_grf_file_name, alphabet, vec, display_perf, display_perf ? &time_grf2fst2 : NULL);

                            updated_fst2_file_name = create_updated_graph_filename(text,
                                in_place ? 0 : transducer_number,
                                in_place ? 0 : iteration,
                                filename_without_path(textbuf->template_name_without_extension), ".fst2");
                        }

                    }
                    //update_tmp_graph(orig_grf, vec, grf_lines, total_lines, NULL, -1, -1, -1, NULL, 0);
                    free_graph_info_match_list(graph_info_match_list);
                    free_graph_file(token_file);

                }
                /*free_file_in_memory(grf_lines);
                free_grf_info(grf_infos, num_annots);
                free(start_node_line);*/
                free_graph_info_list(graph_info_list);
                free_graph_file(graph_file);
            }

            if (is_template_grf != 1 || at_least_one_match_list_not_empty) {
                // apply transducer only if the graph is not generic or when the generic graph has been updated

                u_printf("Applying transducer %s (numbered %d)\n",
                    current_transducer->transducer_file_name, transducer_number);
                char* backup_transducer_filename = NULL;

                if (updated_fst2_file_name != NULL) {
                    backup_transducer_filename = current_transducer->transducer_file_name;
                    current_transducer->transducer_file_name = updated_fst2_file_name;
                }

                unsigned int time_this_locate = 0;
                launch_locate_in_Cassys(labeled_text_name, current_transducer,
                    alphabet, negation_operator, vec, morpho_dic, locate_args, display_perf, display_perf ? &time_this_locate : NULL);

                if (backup_transducer_filename != NULL)
                    current_transducer->transducer_file_name = backup_transducer_filename;

                if (display_perf) {
                    time_locate += time_this_locate;
                    if (nb_perf_info_allocated <= nb_perf_info) {
                        nb_perf_info_allocated *= 2;
                        locate_perf_info* p_locate_perf_info_more = (locate_perf_info*)realloc(p_locate_perf_info,nb_perf_info_allocated*sizeof(locate_perf_info));
                        if (p_locate_perf_info_more != NULL) {
                            p_locate_perf_info = p_locate_perf_info_more;
                        } else {
                            alloc_error("cascade");
                            free(labeled_text_name);
                            free_text_tokens(tokens);
                            free_snt_files(snt_text_files);
                            free(build_text);
                            free(build_work_text_csc_work_path);
                            free(build_work_text_csc_path);
                            free(build_work_text_snt_path);
                            free_vector_int(uima_offsets);
                            free_cassys_tokens_allocation_tool(tokens_allocation_tool);
                            free(textbuf);
                            for (unsigned int loop_display_perf = 0; loop_display_perf < nb_perf_info; loop_display_perf++) {
                                free((p_locate_perf_info + loop_display_perf)->name);
                            }
                            free(p_locate_perf_info);
                            return ALLOC_ERROR_CODE;
                        }
                    }
                    (p_locate_perf_info + nb_perf_info)->elapsed_time = time_this_locate;
                    (p_locate_perf_info + nb_perf_info)->name  = strdup(current_transducer->transducer_file_name);
                    (p_locate_perf_info + nb_perf_info)->index = nb_perf_info;
                    nb_perf_info++;
                }

                // add protection character in lexical tags when needed
                //u_printf("labeled_text_name = %s *******\n", labeled_text_name);
                free_snt_files(snt_text_files);
                snt_text_files = new_snt_files(labeled_text_name);
                protect_lexical_tag_in_concord(snt_text_files->concord_ind, current_transducer->output_policy, vec);
                // generate concordance for this transducer
                launch_concord_in_Cassys(labeled_text_name,
                    snt_text_files->concord_ind, alphabet, NULL, NULL, NULL, vec, concord_args, display_perf, display_perf ? &time_concord : NULL);

                //
                add_replaced_text(labeled_text_name, tokens_list, previous_transducer_number, previous_iteration,
                    transducer_number, iteration, alphabet, vec, tokens_allocation_tool);


                previous_transducer_number = transducer_number;
                previous_iteration = iteration;

                sprintf(textbuf->last_labeled_text_name, "%s", labeled_text_name);
            }


            if (updated_grf_file_name) {
                if (must_do_temp_cleanup) {
                    af_remove(updated_grf_file_name);
                }
                free(updated_grf_file_name);
            }

            if (updated_fst2_file_name) {
                if (must_do_temp_cleanup) {
                    af_remove(updated_fst2_file_name);
                }
                free(updated_fst2_file_name);
            }


            if (count_concordance(snt_text_files->concord_ind, vec) == 0) {
                break;
            }
            else {
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

    remove_extension(original_text, textbuf->text_name_without_extension);
    sprintf(textbuf->result_file_name_XML,"%s_csc.txt", textbuf->text_name_without_extension);

    if(istex_param == 1) {
        construct_istex_standoff(snt_files->concord_ind,vec,original_text,lang,stdoff_file);
    }
    // make a copy of the last resulting text of the cascade in the file named _csc.txt
    // this result his in XML form
    get_path(text, textbuf->path);
    sprintf(textbuf->last_resulting_text_path,"%s", textbuf->last_labeled_text_name);
    sprintf(textbuf->result_file_name_path_XML,"%s", textbuf->result_file_name_XML);
    copy_file(textbuf->result_file_name_path_XML, textbuf->last_resulting_text_path);

    textbuf->result_file_name_path_offset[0]='\0';
    if ((name_uima_offsets_file && (name_uima_offsets_file[0] != '\0')) || (name_input_offsets_file && (name_input_offsets_file[0] != '\0')) || (produce_offsets_file != 0)) {
        sprintf(textbuf->result_file_name_path_offset, "%s_csc_txt_offsets.txt", textbuf->text_name_without_extension);
    }

    // create the text file including XMLized concordance
    launch_concord_in_Cassys(textbuf->result_file_name_path_XML, snt_files->concord_ind, alphabet,
        name_input_offsets_file, name_uima_offsets_file, textbuf->result_file_name_path_offset, vec,concord_args, display_perf, display_perf ? &time_concord : NULL);

    // make a copy of the last resulting text of the cascade in the file named _csc.raw
    sprintf(textbuf->result_file_name_raw,"%s_csc.raw", textbuf->text_name_without_extension);
    sprintf(textbuf->result_file_name_path_raw,"%s", textbuf->result_file_name_raw);
    copy_file(textbuf->result_file_name_path_raw, textbuf->last_resulting_text_path);

    textbuf->result_file_name_path_offset[0] = '\0';
    if ((name_uima_offsets_file && (name_uima_offsets_file[0] != '\0')) || (name_input_offsets_file && (name_input_offsets_file[0] != '\0')) || (produce_offsets_file != 0)) {
        sprintf(textbuf->result_file_name_path_offset, "%s_csc_raw_offsets.txt", textbuf->text_name_without_extension);
    }

    // relaunch the construction of the concord file without XML
    construct_cascade_concord(tokens_list,text,transducer_number, iteration, vec);
    // relaunch the construction of the text file without XML
    launch_concord_in_Cassys(textbuf->result_file_name_path_raw, snt_files->concord_ind, alphabet,
        name_input_offsets_file, name_uima_offsets_file, textbuf->result_file_name_path_offset, vec,concord_args, display_perf, display_perf ? &time_concord : NULL);

    if (dump_graph) {
        sprintf(textbuf->graph_file_name, "%s.dot", textbuf->text_name_without_extension);
        cassys_tokens_2_graph(tokens_list, textbuf->graph_file_name, realign_token_graph_pointer);
    }

    //free_cassys_tokens_list(tokens_list);
    free_snt_files(snt_files);
    free_text_tokens(tokens);
    free_cassys_tokens_allocation_tool(tokens_allocation_tool);


    if (must_do_temp_cleanup != 0) {
        if (build_text != NULL) {
            af_remove(build_text);
        }
        if (labeled_text_name != NULL) {
            af_remove(labeled_text_name);
        }
        cleanup_work_directory_content(build_work_text_snt_path);
        cleanup_work_directory_content(build_work_text_csc_work_path);
        cleanup_work_directory_content(build_work_text_csc_path);
    }

    if (labeled_text_name != NULL) {
      free(labeled_text_name);
    }

    if (build_text != NULL) {
        free(build_text);
    }

    if (build_work_text_snt_path != NULL) {
        free(build_work_text_snt_path);
    }

    if (build_work_text_csc_path != NULL) {
        free(build_work_text_csc_path);
    }

    if (build_work_text_csc_work_path != NULL) {
        free(build_work_text_csc_work_path);
    }

    if (uima_offsets != NULL) {
        free_vector_int(uima_offsets);
    }
    free(textbuf);

    if (display_perf) {
        time_cascade = SyncGetMSecElapsed(htm_cascade);
        float ratio = (float)(time_cascade / 100.);
        float ratio_locate = (float)(time_locate / 100.);
        if (ratio == 0) ratio = 1;
        if (ratio_locate == 0) ratio_locate = 1;
        u_printf("time running cascade = %.3f sec\n", time_cascade / 1000.);

        u_printf("time on tokenize = %.3f sec, %.1f %% total\n", time_tokenize / 1000.,time_tokenize / ratio);
        u_printf("time on grf2fst2 = %.3f sec, %.1f %% total\n", time_grf2fst2 / 1000., time_grf2fst2 / ratio);
        u_printf("time on locate = %.3f sec, %.1f %% total\n", time_locate / 1000., time_locate / ratio);
        u_printf("time on concord = %.3f sec, %.1f %% total\n", time_concord / 1000., time_concord / ratio);

        u_printf("\nlocate time on transduced order:\n");
        for (unsigned int loop_display_perf = 0; loop_display_perf < nb_perf_info; loop_display_perf++)
            u_printf("locate %.3f sec (%.1f %%) for %s\n",
                (p_locate_perf_info + loop_display_perf)->elapsed_time / 1000.,
                (p_locate_perf_info + loop_display_perf)->elapsed_time / ratio_locate,
                (p_locate_perf_info + loop_display_perf)->name);

        quicksort(nb_perf_info, p_locate_perf_info);

        u_printf("\nlocate time sorted by time:\n");
        for (unsigned int loop_display_perf = 0; loop_display_perf < nb_perf_info; loop_display_perf++)
            u_printf("locate %.3f sec (%.1f %%) for %s\n",
                (p_locate_perf_info + loop_display_perf)->elapsed_time / 1000.,
                (p_locate_perf_info + loop_display_perf)->elapsed_time / ratio_locate,
                (p_locate_perf_info + loop_display_perf)->name);
    }

    if (p_locate_perf_info != NULL) {
        for (unsigned int loop_display_perf = 0; loop_display_perf < nb_perf_info; loop_display_perf++) {
            free((p_locate_perf_info + loop_display_perf)->name);
        }
        free(p_locate_perf_info);
    }

    return SUCCESS_RETURN_CODE;
}

} // namespace unitex
