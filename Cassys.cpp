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
#include "UnusedParameter.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {




const char *optstring_Cassys = ":bp:t:a:w:l:hk:q:g:dvuNnm:s:ir:T:L:C:";
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
        {"working_dir", required_argument_TS, NULL, 'p' },
        {"cleanup_working_files", no_argument_TS, NULL, 'b' },
        {"tokenize_argument", required_argument_TS, NULL, 'T' },
        {"locate_argument", required_argument_TS, NULL, 'L' },
        {"concord_argument", required_argument_TS, NULL, 'C' },
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
        "-p X/--working_dir=X: uses directory X for intermediate working file\n"
        "-b/--cleanup_working_files: remove intermediate working file after usage\n"
        "-u/--dump_token_graph create a .dot file with graph dump infos\n"
        "-N/--no_dump_token_graph create a .dot file with graph dump infos\n"
        "-n/--realign_token_graph_pointer create a.dot file will not depends to pointer allocation to be deterministic\n"
        "-v/--translate_path_separator_to_native replace path separator in csc by native separator for portable csc file\n"
        "-d/--no_create_directory mean the all snt/csc directories already exist and don't need to be created\n"
        "  -g minus/--negation_operator=minus: uses minus as negation operator for Unitex 2.0 graphs\n"
        "  -g tilde/--negation_operator=tilde: uses tilde as negation operator (default)\n"
        "-T ARG/--tokenize_argument=ARG specify additionnal argument for Tokenize internal call (several possible)\n"
        "-L ARG/--locate_argument=ARG specify additionnal argument for Locate internal call (several possible)\n"
        "-C ARG/--concord_argument=ARG specify additionnal argument for Concord internal call (several possible)\n"
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
display_copyright_notice();
u_printf(usage_Cassys);
}

struct grfInfo {
    int entity_loc;
    int annotation_loc;
    unichar *entity_format;
    unichar **ignore;
    int ignore_count;
    unichar **accept;
    int accept_count;
    unichar *annotation;
    unichar *entities;
    int entity_count;
};

int is_template_graph(const char *transducer) {
    int ret_value = 0;
    int pos = -1;
    for(int i = (int)strlen(transducer) - 1; i >= 0; i--)
    if(transducer[i] == PATH_SEPARATOR_CHAR) {
        pos = i;
        break;
    }

    if(pos >= 0 && transducer[pos+1] == '@')
    ret_value = 1;

    return ret_value;
}

unichar** load_file_in_memory(const char* tmp_file, VersatileEncodingConfig *vec, int *total_lines) {
    int num_lines = 0;
    unichar **grf_lines = NULL;
    unichar *line = NULL;
    size_t size_buffer_line = 0;
    U_FILE *grf_file = u_fopen(vec,tmp_file,U_READ);

    if(grf_file != NULL) {
    while(u_fgets_dynamic_buffer(&line, &size_buffer_line, grf_file) != EOF) {
        grf_lines = (unichar**) realloc(grf_lines, (num_lines+2) * sizeof(unichar*));
        grf_lines[num_lines] = (unichar*) malloc(sizeof(unichar) * (u_strlen(line) + 1));
        u_strcpy(grf_lines[num_lines],line);
        num_lines++;
		grf_lines[num_lines] = NULL;
    }
    if(line != NULL)
        free(line);
    u_fclose(grf_file);
    }
    *total_lines = num_lines;
    return grf_lines;
}


void free_file_in_memory(unichar** grf_lines)
{
	if (grf_lines == NULL) return;
	unichar** walk = grf_lines;
	while ((*walk) != NULL) {
		free(*walk);
		walk++;
	}
	free(grf_lines);
}


void free_grf_info(grfInfo *infos, int num) {
	for (int i = 0;i < num;i++) {
		free(infos[i].entity_format);
		free(infos[i].annotation);
		
		if (infos[i].accept) {
			unichar** walk = infos[i].accept;
			while ((*walk) != NULL) {
				free(*walk);
				walk++;
			}
			free(infos[i].accept);
		}
		
	}

	free(infos);
}

grfInfo *extract_info(unichar **lines, int *num_annot, int total_lines, int *loc, unichar **start_line, int **locations) {
    DISCARD_UNUSED_PARAMETER(locations)
    int start = -1;
    int num_info = 0;
    struct grfInfo *infos = NULL;
    if(lines != NULL) {
    int num_lines = 0;
    while(num_lines < total_lines && lines[num_lines] != NULL) {
        size_t num_char = u_strlen(lines[num_lines]);
        if(num_char == 1 && lines[num_lines][0] == '#') {
        start = num_lines;
        }
        else if(num_lines == start + 2) {
        *loc = num_lines;
        *start_line = (unichar*) malloc(sizeof(unichar) * (num_char + 2));
        u_strcpy(*start_line,lines[num_lines]);
        }
        else if(num_char > 2 && lines[num_lines][0] == '"' && lines[num_lines][1] == '@') {
        infos = (grfInfo*) realloc(infos, (num_info + 1) * sizeof(grfInfo));
        infos[num_info].entity_format = (unichar*) malloc(sizeof(unichar) * (num_char + 2));
        infos[num_info].entity_format[0] = '"';
        infos[num_info].entity_format[1] = '%';
        infos[num_info].entity_format[2] = 'S';
        infos[num_info].entity_loc = num_lines - (*loc);
        infos[num_info].annotation_loc = 0;
        infos[num_info].entity_count = 0;
        infos[num_info].entities = NULL;
        int spaces = 0;
        for(size_t i = 2; i <= num_char; i++) {
            infos[num_info].entity_format[i + 1] = (unichar) lines[num_lines][i];
            if(spaces == 4 && lines[num_lines][i] > 47 && lines[num_lines][i] < 58) { //is digit
                infos[num_info].annotation_loc = 10 * infos[num_info].annotation_loc + lines[num_lines][i] - 48;
            }
            if(lines[num_lines][i] == ' ')
                spaces++;
        }
        num_info++;
        }
        else {
        for(int i = 0; i < num_info; i++) {
            if(num_lines == infos[i].annotation_loc + *loc) {
            int n = u_strlen(lines[num_lines]);
            int j,k;
            int division = -1;
            int annot_end = -1;
            int ignore_cnt = 0;
            infos[i].ignore = NULL;
            infos[i].accept = NULL;
            int is_accept = 0;
            for(j = 0; j < n; j++) {
                if(lines[num_lines][j] == '/') {
                division = j;
                }
                else if(lines[num_lines][j] == '"') {
                annot_end = j;
                }
            }
            unichar *temp_annot = (unichar*) malloc(sizeof(unichar) * division);
            for(k = 1; k < division; k++)
                temp_annot[k - 1] = lines[num_lines][k];
            temp_annot[k - 1] = '\0';

            if(u_strcmp(temp_annot,"<E>") != 0) {
                unichar *saveptr = NULL;
                const unichar DELIMITER[] = { '~', 0 };
                unichar *ignore_token = u_strtok_r(temp_annot, DELIMITER, &saveptr);
                unichar *ignore = NULL;
                while(ignore_token) {
                ignore = ignore_token;
                if(u_strcmp(ignore,temp_annot) == 0) {
                    is_accept = 1;
                    break;
                }
                ignore_token = u_strtok_r(NULL, DELIMITER, &saveptr);
                infos[i].ignore = (unichar**) realloc(infos[i].ignore, sizeof(unichar*) * (ignore_cnt + 1));
                infos[i].ignore[ignore_cnt] =  (unichar*) malloc(sizeof(unichar) * (u_strlen(ignore) + 1));
                u_strcpy(infos[i].ignore[ignore_cnt],ignore);
                ignore_cnt++;
                }
                if(is_accept && infos[i].ignore == NULL) {
                int accept_cnt = 0;
                int prev = 0;
                for(k = 0; k < division; k++) {
                    if(k > 0 && temp_annot[k] == '+' && temp_annot[k - 1] != '\\') {
                    infos[i].accept = (unichar**) realloc(infos[i].accept, sizeof(unichar*) * (accept_cnt + 2));
                    infos[i].accept[accept_cnt] = (unichar*)malloc(sizeof(unichar) * (k - prev + 1));
					infos[i].accept[accept_cnt+1] = NULL;
                    n = 0;
                    for(j = prev; j < k; j++)
                        if(temp_annot[j] != '\\') {
                        infos[i].accept[accept_cnt][n++] = temp_annot[j];
                        }
                    infos[i].accept[accept_cnt][n] = '\0';
                    prev = k;
                    accept_cnt++;
                    }
                }
                infos[i].accept = (unichar**) realloc(infos[i].accept, sizeof(unichar*) * (accept_cnt + 2));
                infos[i].accept[accept_cnt] = (unichar*)malloc(sizeof(unichar) * (k - prev + 1));
				infos[i].accept[accept_cnt + 1] = NULL;
                n = 0;
                for(j = prev; j < k; j++)
                    if(temp_annot[j] != '\\') {
                    infos[i].accept[accept_cnt][n++] = temp_annot[j];
                    }
                infos[i].accept[accept_cnt][n] = '\0';
                accept_cnt++;
                infos[i].accept_count = accept_cnt;
                }
                if(ignore != NULL)
                free(ignore);
                //if(ignore_token != NULL)
                //free(ignore_token);
            }
			//free(temp_annot);
            infos[i].ignore_count = ignore_cnt;
            infos[i].annotation = (unichar*) malloc(sizeof(unichar) * (annot_end - division));

            for(k = 0, j = division + 1; j < annot_end; k++, j++)
                infos[i].annotation[k] = lines[num_lines][j];
            infos[i].annotation[k] = '\0';
            }
        }
        }
        num_lines++;
    }
    }
    *num_annot = num_info;
    return infos;
}

unichar **extract_entities(const char *token_list, VersatileEncodingConfig *vec, int num, int *updates, grfInfo *infos) {
    unichar **entity_string = NULL;
    entity_string = (unichar**) malloc(sizeof(unichar*) * num);
    for(int i = 0; i < num; i++)
        entity_string[i] = NULL;
    U_FILE *dico = u_fopen(vec, token_list, U_READ);
    if(dico != NULL && infos != NULL) {
    int * num_entity = (int*)malloc(sizeof(int)*(num+1));
    for(int i = 0; i < num; i++)
        num_entity[i] = 0;
    unichar *line = NULL;
    size_t size_buffer_line = 0;
    while(u_fgets_dynamic_buffer(&line, &size_buffer_line, dico) != EOF) {
        for(int k = 0; k < num; k++) {
        if (line != NULL && line[0] == '{' && u_strlen(line) > u_strlen(infos[k].annotation)) {
            int j = 0;
            while(line[j] != '\0') {
            int i = 0;
            while(line[j] != '\0' && line[j] != infos[k].annotation[i])
                j++;
            while(line[j] != '\0' && infos[k].annotation[i] != '\0' && line[j] == infos[k].annotation[i]) {
                i++;
                j++;
            }
            if(infos[k].annotation[i] == '\0') {
                int line_len = u_strlen(line);
                int start = 0;
                int end = 0;
                int annot_start = 0;
                int annot = -1;
                unichar *prev_char = NULL;
                unichar *entity_whole = NULL;
                for(int x = 0; x < line_len; x++) {
                if(line[x] == '{') {
                    start = x+1;
                    if (x > 0) {
                    prev_char = (unichar*) malloc(sizeof(unichar) * 2);
                    prev_char[0] = line[x - 1];
                    if(line[x - 1] == '\\')
                        prev_char[0] = line[x - 2];
                    prev_char[1] = '\0';
                    }
                }
                else if(line[x] == ',') {
                    annot = 0;
                    end = x;
                }
                else if(annot == 0 && line[x] == '.') {
                    annot = -1;
                    annot_start = x + 1;
                }
                else if(line[x] != '\\') {
                    annot = -1;
                    if(line[x] == '}' && start > 0) {
                    int matches = 0;
                    unichar *annot_ = (unichar*) malloc(sizeof(unichar) * (x - annot_start));
                    int z = 0;
                    for(int y = annot_start; y < x; y++)
                        if(line[y] != '\\') {
                        annot_[z++] = (unichar) line[y];
                        }
                    annot_[z] = '\0';
                    if(infos[k].ignore != NULL) {
                        for(int m = 0; m < infos[k].ignore_count; m++)
                        if(u_strncmp(annot_,infos[k].ignore[m],u_strlen(infos[k].ignore[m])) == 0) {
                            matches = 1;
                            break;
                        }
                    }
                    if(infos[k].accept != NULL) {
                        matches = 1;
                        for(int m = 0; m < infos[k].accept_count; m++)
                        if(u_strncmp(annot_,infos[k].accept[m],u_strlen(infos[k].accept[m])) == 0) {
                            matches = 0;
                            break;
                        }
                    }
                    if(matches == 0) {
                        unichar *entity = NULL;
                        entity = (unichar*) malloc(sizeof(unichar) * (end - start));
                        z = 0;
                        for(int y = start; y < end; y++)
                        if(line[y] != '\\')
                            entity[z++] = line[y];
                        entity[z] = '\0';
                        int entity_len = z + 1;
                        if(entity_whole == NULL) {
                        entity_whole = (unichar*) malloc(sizeof(unichar) * entity_len);
                        u_strcpy(entity_whole,entity);
                        }
                        else {
                        int current_len = u_strlen(entity_whole);
                        entity_whole =  (unichar*) realloc(entity_whole,sizeof(unichar) * (current_len + entity_len + 2));
                        u_strcat(entity_whole,prev_char);
                        u_strcat(entity_whole,entity);
                        }

                        if(entity != NULL)
                        free(entity);
                    }
                    if(annot_ != NULL)
                        free(annot_);
                    start = -1;
                    }
                }
                }
                if(entity_whole != NULL) {
                int entity_len = u_strlen(entity_whole);
                if(infos[k].entity_count == 0) {
                    infos[k].entities = (unichar*) malloc(sizeof(unichar) * entity_len);
                    u_strcpy(infos[k].entities,entity_whole);
                    infos[k].entity_count++;
                    *updates = *updates + 1;
                }
                else {
                    int current_len = u_strlen(infos[k].entities);
                    infos[k].entities = (unichar*) realloc(infos[k].entities, sizeof(unichar) * (current_len + entity_len + 2));
                    infos[k].entities[current_len] = '+';
                    for(int a = current_len + 1, b = 0; a < current_len + entity_len + 2; a++)
                    infos[k].entities[a] = entity_whole[b++];
                    infos[k].entities[current_len + entity_len + 1] = '\0';
                }
                free(entity_whole);
                }
            }
            }
        }
        }
    }
    if(line !=NULL)
        free(line);
    u_fclose(dico);
    free(num_entity);
    }
    return entity_string;
}


static void print_entity_param(U_FILE* out, unichar* format, const unichar*param)
{
    int len_format = (int)u_strlen(format);    
    int i;
    for (i = 0; (i + 1) < len_format; i++) {
        if (((*(format + i)) == '%') && ((*(format + i + 1)) == 'S')) {            
            break;
        }
    }
    if ((i + 1) == len_format) {
        u_fprintf(out, "%S", format);
        return;
    }

    *(format + i) = '\0';
    u_fprintf(out, "%S", format);
    *(format + i) = '%';
    u_fprintf(out, "%S", param);
    u_fprintf(out, "%S", format+2);
}




int update_tmp_graph(const char *transducer, VersatileEncodingConfig *vec, unichar **lines, int total_lines, unichar *start, int start_loc, int num_entities, int num_info, grfInfo *infos, int mode) {
    int status = 0;
    U_FILE *graph_file = u_fopen(vec,transducer,U_WRITE);
    if (graph_file !=NULL) {
    if(lines != NULL) {
        int num_lines = 0;
        int updates = 0;
        while(num_lines < total_lines && lines[num_lines] !=NULL) {
        size_t num_char = u_strlen(lines[num_lines]);
        if(mode && num_lines == start_loc) {
            int spaces = 0;
            for(int i = 0; start[i] !='\0'; i++) {
            u_fprintf(graph_file,"%C",start[i]);
            if(start[i] == ' ')
                spaces++;
            if(spaces == 3)
                break;
            }
            u_fprintf(graph_file,"%d ",num_entities);
            for(int i = 0; i < num_info; i++) {
            if(infos[i].entity_count > 0)
                u_fprintf(graph_file,"%d ",infos[i].entity_loc);
            }
            u_fprintf(graph_file,"\n");
        }
        else if(mode && num_char > 2 && lines[num_lines][0] == '"' && lines[num_lines][1] == '@') {
            int loc = num_lines - start_loc;
            for(int i = 0; i < num_info; i++) {
            if(infos[i].entity_loc == loc) {
                print_entity_param(graph_file,infos[i].entity_format, infos[i].entities);
                u_fprintf(graph_file,"\n");
            }
            }
            updates++;
        }
        else if(mode) {
            int loc = num_lines - start_loc;
            int found = 0;
            for(int i = 0; i < num_info; i++) {
            if(infos[i].annotation_loc == loc) {
                u_fprintf(graph_file, "\"<E>");
                int j = 0;
                while(lines[num_lines][j] != '\0' && lines[num_lines][j] != '/')
                j++;
                for(; j<(int)u_strlen(lines[num_lines]); j++)
                u_fprintf(graph_file,"%C",lines[num_lines][j]);
                u_fprintf(graph_file,"\n");
                found++;
            }
            }
            if(found == 0)
            u_fprintf(graph_file,"%S\n",lines[num_lines]);
        }
        else
            u_fprintf(graph_file,"%S\n",lines[num_lines]);
        num_lines++;
        }
    }
    u_fclose(graph_file);
    status = 1;
    }
    return status;
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

    vector_ptr * tokenize_additional_args = new_vector_ptr();
    vector_ptr * locate_additional_args = new_vector_ptr();
    vector_ptr * concord_additional_args = new_vector_ptr();

    if (locate_additional_args == NULL) {
        fatal_alloc_error("main_Cassys");
    }

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
                  if (temp_work_dir != NULL) {
                      free(temp_work_dir);
                  }
                  free_vector_ptr(tokenize_additional_args, free);
                  free_vector_ptr(locate_additional_args, free);
                  free_vector_ptr(concord_additional_args, free);
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
        case 'b': {
            must_do_temp_cleanup = 1;
            break;
        }
        case 'd': {
            must_create_directory = 0;
            break;
        }
        case 'p' : {
            if (vars->optarg[0] != '\0') {
                   if (temp_work_dir != NULL) {
                      free(temp_work_dir);
                   }
                   temp_work_dir = strdup(vars->optarg);
                    if (temp_work_dir == NULL) {
                        fatal_alloc_error("main_Cassys");
                    }

            }
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
        case 'T': {
            if (vars->optarg[0] != '\0') {
                char * locate_arg = strdup(vars->optarg);
                if (locate_arg == NULL) {
                    fatal_alloc_error("main_Cassys");
                }
                vector_ptr_add(tokenize_additional_args, locate_arg);
            }
            break;
        }
        case 'L': {
            if (vars->optarg[0] != '\0') {
                char * locate_arg = strdup(vars->optarg);
                if (locate_arg == NULL) {
                    fatal_alloc_error("main_Cassys");
                }
                vector_ptr_add(locate_additional_args, locate_arg);
            }
            break;
        }
        case 'C': {
            if (vars->optarg[0] != '\0') {
                char * locate_arg = strdup(vars->optarg);
                if (locate_arg == NULL) {
                    fatal_alloc_error("main_Cassys");
                }
                vector_ptr_add(concord_additional_args, locate_arg);
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

    cascade(text_file_name, in_place, must_create_directory, must_do_temp_cleanup, temp_work_dir,
        transducer_list, alphabet_file_name, negation_operator,
        &vec, morpho_dic,
        tokenize_additional_args, locate_additional_args, concord_additional_args,
        dump_graph, realign_token_graph_pointer);

    if (temp_work_dir != NULL){
        free(temp_work_dir);
    }
    if(morpho_dic != NULL){
        free(morpho_dic);
    }
    free_fifo(transducer_list);
    free_OptVars(vars);
    free_transducer_name_and_mode_linked_list(transducer_name_and_mode_linked_list_arg);
    free_vector_ptr(tokenize_additional_args, free);
    free_vector_ptr(locate_additional_args, free);
    free_vector_ptr(concord_additional_args, free);
    return 0;
}


/**
 * The main function of the cascade
 *
 *
 */
int cascade(const char* original_text, int in_place, int must_create_directory,  int must_do_temp_cleanup, const char* temp_work_dir,
    fifo* transducer_list, const char *alphabet,
    const char*negation_operator,
    VersatileEncodingConfig* vec,
    const char *morpho_dic, vector_ptr* tokenize_args, vector_ptr* locate_args, vector_ptr* concord_args,
    int dump_graph, int realign_token_graph_pointer) {


    cassys_tokens_allocation_tool* tokens_allocation_tool = build_cassys_tokens_allocation_tool();


    if (must_do_temp_cleanup) {
        in_place = 1;
    }
    const char* text = original_text;
    char * build_text = NULL;
    char * build_work_text_snt_path = NULL;
    char * build_work_text_csc_path = NULL;
    char * build_work_text_csc_work_path = NULL;
    size_t len_work_dir = (temp_work_dir == NULL) ? 0 : strlen(temp_work_dir);

    if ((len_work_dir > 0) || (must_do_temp_cleanup != 0)) {
        size_t len_temp_work_dir = (temp_work_dir == NULL) ? 0 : strlen(temp_work_dir);

            build_work_text_snt_path = (char*)malloc(len_temp_work_dir + (strlen(original_text) * 2) + 0x40);
            if (build_work_text_snt_path == NULL) {
                fatal_alloc_error("load_transducer_from_linked_list");
                exit(1);
            }

            build_work_text_csc_path = (char*)malloc(len_temp_work_dir + (strlen(original_text) * 2) + 0x40);
            if (build_work_text_csc_path == NULL) {
                fatal_alloc_error("load_transducer_from_linked_list");
                exit(1);
            }

            build_work_text_csc_work_path = (char*)malloc(len_temp_work_dir + (strlen(original_text) * 2) + 0x40);
            if (build_work_text_csc_work_path == NULL) {
                fatal_alloc_error("load_transducer_from_linked_list");
                exit(1);
            }

            if (len_temp_work_dir > 0) {
                build_text = (char*)malloc(len_temp_work_dir + strlen(original_text) + 0x10);
                if (build_text == NULL) {
                    fatal_alloc_error("load_transducer_from_linked_list");
                    exit(1);
                }

                strcpy(build_text, (len_temp_work_dir > 0) ? temp_work_dir : original_text);
                char latest_char = *(temp_work_dir + strlen(temp_work_dir) - 1);
                if ((latest_char != '\\') && (latest_char != '/')) {
                    strcat(build_text, PATH_SEPARATOR_STRING);
                }
                remove_path(original_text, build_text + strlen(build_text));


                char * build_original_text_snt_path = (char*)malloc(len_temp_work_dir + (strlen(original_text) * 2) + 0x40);
                if (build_original_text_snt_path == NULL) {
                    fatal_alloc_error("load_transducer_from_linked_list");
                    exit(1);
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
    //launch_tokenize_in_Cassys(text,alphabet,NULL,vec,tokenize_args);

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

    if (is_empty(transducer_list)) {
         if (labeled_text_name != NULL) {
            free(labeled_text_name);
         }
         labeled_text_name = create_labeled_files_and_directory(text,
                 0,0,0,0, must_create_directory,0);
         sprintf(last_labeled_text_name, "%s", labeled_text_name);
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

        int is_template_grf = is_template_graph(current_transducer->transducer_file_name);

        for (iteration = 0; current_transducer->repeat_mode == INFINITY || iteration < current_transducer->repeat_mode; iteration++) {
            if (in_place == 0) {

				if (labeled_text_name != NULL) {
					free(labeled_text_name);
				}
                labeled_text_name = create_labeled_files_and_directory(text, previous_transducer_number,
                    transducer_number, previous_iteration, iteration, must_create_directory, 1);
            }

            launch_tokenize_in_Cassys(labeled_text_name, alphabet,
                snt_text_files->tokens_txt, vec, tokenize_args);

            int entity = 0;
            if (is_template_grf) {
                int *entity_loc = NULL;
                int num_annots = 0;
                unichar *start_node_line = NULL;
                int start_node_loc = -1;
                int total_lines = 0;
                char orig_grf[FILENAME_MAX];
                char template_name_without_extension[FILENAME_MAX];
                int num_entities = 0;
                struct grfInfo *grf_infos = NULL;

                remove_extension(current_transducer->transducer_file_name, template_name_without_extension);
                sprintf(orig_grf, "%s.grf", template_name_without_extension);

                unichar **grf_lines = load_file_in_memory(orig_grf, vec, &total_lines);
                grf_infos = extract_info(grf_lines, &num_annots, total_lines, &start_node_loc, &start_node_line, &entity_loc);

                if (num_annots > 0) {
					unichar**entity_string = extract_entities(snt_text_files->tok_by_alph_txt, vec, num_annots, &num_entities, grf_infos);
					free(entity_string);
                    if (update_tmp_graph(orig_grf, vec, grf_lines, total_lines, start_node_line, start_node_loc, num_entities, num_annots, grf_infos, 1)) {
                        if (num_entities > 0)
                            launch_grf2fst2_in_Cassys(orig_grf, alphabet, vec, concord_args);
                        entity = num_entities;
                    }
                    update_tmp_graph(orig_grf, vec, grf_lines, total_lines, NULL, -1, -1, -1, NULL, 0);
                }
				free_file_in_memory(grf_lines);
				free_grf_info(grf_infos, num_annots);
            }

            if (is_template_grf != 1 || entity > 0) {
                // apply transducer only if the graph is not generic or when the generic graph has been updated

                u_printf("Applying transducer %s (numbered %d)\n",
                    current_transducer->transducer_file_name, transducer_number);
                launch_locate_in_Cassys(labeled_text_name, current_transducer,
                    alphabet, negation_operator, vec, morpho_dic, locate_args);

                // add protection character in lexical tags when needed
                //u_printf("labeled_text_name = %s *******\n", labeled_text_name);
				free_snt_files(snt_text_files);
                snt_text_files = new_snt_files(labeled_text_name);
                protect_lexical_tag_in_concord(snt_text_files->concord_ind, current_transducer->output_policy, vec);
                // generate concordance for this transducer
                launch_concord_in_Cassys(labeled_text_name,
                    snt_text_files->concord_ind, alphabet, vec, concord_args);

                //
                add_replaced_text(labeled_text_name, tokens_list, previous_transducer_number, previous_iteration,
                    transducer_number, iteration, alphabet, vec, tokens_allocation_tool);


                previous_transducer_number = transducer_number;
                previous_iteration = iteration;

                sprintf(last_labeled_text_name, "%s", labeled_text_name);
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

    char result_file_name_XML[FILENAME_MAX];
    char text_name_without_extension[FILENAME_MAX];
    remove_extension(original_text,text_name_without_extension);
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
    launch_concord_in_Cassys(result_file_name_path_XML, snt_files->concord_ind, alphabet, vec,concord_args);

    // make a copy of the last resulting text of the cascade in the file named _csc.raw
    char result_file_name_raw[FILENAME_MAX];
    sprintf(result_file_name_raw,"%s_csc.raw",text_name_without_extension);
    char result_file_name_path_raw[FILENAME_MAX];
    sprintf(result_file_name_path_raw,"%s", result_file_name_raw);
    copy_file(result_file_name_path_raw, last_resulting_text_path);

    // relaunch the construction of the concord file without XML
    construct_cascade_concord(tokens_list,text,transducer_number, iteration, vec);
    // relaunch the construction of the text file without XML
    launch_concord_in_Cassys(result_file_name_path_raw, snt_files->concord_ind, alphabet, vec,concord_args);

    if (dump_graph)
    {
        char graph_file_name[FILENAME_MAX];
        sprintf(graph_file_name, "%s.dot", text_name_without_extension);
        cassys_tokens_2_graph(tokens_list, graph_file_name, realign_token_graph_pointer);
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

    if (labeled_text_name != NULL)
      free(labeled_text_name);

    if (build_text != NULL)
        free(build_text);

    if (build_work_text_snt_path != NULL)
        free(build_work_text_snt_path);

    if (build_work_text_csc_path != NULL)
        free(build_work_text_csc_path);

    if (build_work_text_csc_work_path != NULL)
        free(build_work_text_csc_work_path);

    return 0;
}

} // namespace unitex
