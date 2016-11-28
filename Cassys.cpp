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
    int negLeftContxt_loc;
};


unichar** load_file_in_memory(const char* tmp_file, VersatileEncodingConfig *vec,
        int *total_lines) {
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
        if (infos[i].entities!=NULL) free(infos[i].entities);

        if (infos[i].ignore) {
            unichar** walk = infos[i].ignore;
            while ((*walk) != NULL) {
                free(*walk);
                walk++;
            }
            free(infos[i].ignore);
        }

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


grfInfo *extract_info(unichar **lines, int *num_annot, int total_lines, int *loc,
        unichar **start_line, int **locations) {
    DISCARD_UNUSED_PARAMETER(locations)
    int start = -1;
    int num_info = 0;
    int count = 0;
    struct grfInfo *infos = NULL;
    if (lines != NULL) {
        int num_lines = 0;
        while (num_lines < total_lines && lines[num_lines] != NULL) {
            size_t num_char = u_strlen(lines[num_lines]);
            if (num_char == 1 && lines[num_lines][0] == '#') {
                start = num_lines;
            }
            else if (num_lines == start + 2) {
                *loc = num_lines;
                if ((*start_line) != NULL) free(*start_line);
                *start_line = (unichar*)malloc(sizeof(unichar) * (num_char + 1));
                u_strcpy(*start_line, lines[num_lines]);
            }
            else if (num_char > 3 && lines[num_lines][0] == '"' && lines[num_lines][1] == '$' && lines[num_lines][2] == 'G') {
                infos = (grfInfo*)realloc(infos, (num_info + 1) * sizeof(grfInfo));
                infos[num_info].accept = NULL;
                infos[num_info].ignore = NULL;
                infos[num_info].annotation = NULL;
                infos[num_info].entity_format = (unichar*)malloc(sizeof(unichar) * (num_char + 1));
                infos[num_info].entity_format[0] = '"';
                infos[num_info].entity_format[1] = '%';
                infos[num_info].entity_format[2] = 'S';
                infos[num_info].entity_loc = num_lines - (*loc);
                infos[num_info].annotation_loc = 0;
                infos[num_info].entity_count = 0;
                infos[num_info].entities = NULL;
                infos[num_info].negLeftContxt_loc = total_lines - (*loc) + count*3; //negative context is made up of 3 lines
                count++;
                int spaces = 0;
                for (size_t i = 3; i <= num_char; i++) {
                    infos[num_info].entity_format[i] = (unichar)lines[num_lines][i];
                    if (spaces == 4 && lines[num_lines][i] > 47 && lines[num_lines][i] < 58) { //is digit
                        infos[num_info].annotation_loc = 10 * infos[num_info].annotation_loc + lines[num_lines][i] - 48;
                    }
                    if (lines[num_lines][i] == ' ')
                        spaces++;
                }
                // start extract tag information
                int annot_pos = infos[num_info].annotation_loc + *loc;
                int n = u_strlen(lines[annot_pos]);
                int j, k;
                int division = -1;
                int annot_end = -1;
                int ignore_cnt = 0;
                int is_accept = 0;
                for (j = 0; j < n; j++) {
                    if (lines[annot_pos][j] == '/') {
                        division = j;
                    }
                    else if(lines[annot_pos][j] == '"') {
                        annot_end = j;
                        if(j > 0 && lines[annot_pos][j-1] == '}')
                            annot_end = j-1;
                    }
                }

                unichar *temp_annot = (unichar*)malloc(sizeof(unichar) * division);
                for (k = 1; k < division; k++)
                    temp_annot[k - 1] = lines[annot_pos][k];
                temp_annot[k - 1] = '\0';

                if (u_strcmp(temp_annot, "<E>") != 0) {
                    unichar *saveptr = NULL;
                    const unichar DELIMITER[] = { '~', 0 };
                    unichar *ignore_token = u_strtok_r(temp_annot, DELIMITER, &saveptr);
                    unichar *ignore = NULL;
                    while (ignore_token) {
                        ignore = ignore_token;
                        if (u_strcmp(ignore, temp_annot) == 0) {
                            is_accept = 1;
                            break;
                        }
                        ignore_token = u_strtok_r(NULL, DELIMITER, &saveptr);
                        infos[num_info].ignore = (unichar**)realloc(infos[num_info].ignore, sizeof(unichar*) * (ignore_cnt + 2));
                        infos[num_info].ignore[ignore_cnt] = (unichar*)malloc(sizeof(unichar) * (u_strlen(ignore) + 1));
                        u_strcpy(infos[num_info].ignore[ignore_cnt], ignore);
                        ignore_cnt++;
                        infos[num_info].ignore[ignore_cnt] = NULL;
                    }
                    if (is_accept && infos[num_info].ignore == NULL) {
                        int accept_cnt = 0;
                        int prev = 0;
                        for (k = 0; k < division; k++) {
                            if (k > 0 && temp_annot[k] == '+' && temp_annot[k - 1] != '\\') {
                                infos[num_info].accept = (unichar**)realloc(infos[num_info].accept, sizeof(unichar*) * (accept_cnt + 2));
                                infos[num_info].accept[accept_cnt] = (unichar*)malloc(sizeof(unichar) * (k - prev + 1));
                                infos[num_info].accept[accept_cnt + 1] = NULL;
                                n = 0;
                                for (j = prev; j < k; j++)
                                    if (temp_annot[j] != '\\') {
                                        infos[num_info].accept[accept_cnt][n++] = temp_annot[j];
                                    }
                                infos[num_info].accept[accept_cnt][n] = '\0';
                                prev = k+1;
                                accept_cnt++;
                            }
                        }
                        infos[num_info].accept = (unichar**)realloc(infos[num_info].accept, sizeof(unichar*) * (accept_cnt + 2));
                        infos[num_info].accept[accept_cnt] = (unichar*)malloc(sizeof(unichar) * (k - prev + 1));
                        infos[num_info].accept[accept_cnt + 1] = NULL;
                        n = 0;
                        for (j = prev; j < k; j++)
                            if (temp_annot[j] != '\\') {
                                infos[num_info].accept[accept_cnt][n++] = temp_annot[j];
                            }
                        infos[num_info].accept[accept_cnt][n] = '\0';
                        accept_cnt++;
                        infos[num_info].accept_count = accept_cnt;
                    }
                }
                free(temp_annot);
                infos[num_info].ignore_count = ignore_cnt;
                infos[num_info].annotation = (unichar*)malloc(sizeof(unichar) * (annot_end - division));

                for (k = 0, j = division + 1; j < annot_end; k++, j++)
                    infos[num_info].annotation[k] = lines[annot_pos][j];
                infos[num_info].annotation[k] = '\0';
                //end extract tag information

                num_info++;
            }
            num_lines++;
        }
    }
    *num_annot = num_info;
    return infos;
}


unichar **extract_entities(const char *token_list, const char *token_list_backup, VersatileEncodingConfig *vec, int num, int *updates, grfInfo *infos) {
    unichar **entity_string = NULL;
    entity_string = (unichar**) malloc(sizeof(unichar*) * num);
    for(int i = 0; i < num; i++)
        entity_string[i] = NULL;
    U_FILE *dico = u_fopen(vec, token_list, U_READ); //token list in the current _snt folder
    if(dico == NULL)
        dico = u_fopen(vec, token_list_backup, U_READ);   //token list in the previous _snt folder
    if(dico != NULL && infos != NULL) {
    int * num_entity = (int*)malloc(sizeof(int)*(num+1));
    for(int i = 0; i < num; i++)
        num_entity[i] = 0;
    unichar *line = NULL;
    size_t size_buffer_line = 0;
    while(u_fgets_dynamic_buffer(&line, &size_buffer_line, dico) != EOF) {
        int line_len = u_strlen(line);
        for(int k = 0; k < num; k++) {
            int annot_len = u_strlen(infos[k].annotation);
        if (line != NULL && line[0] == '{' && line_len > annot_len) {
            int j = 0;
            while(line[j] != '\0') {
            int i = 0;
            while(line[j] != '\0' && line[j] != infos[k].annotation[i])
                j++;
            while(line[j] != '\0' && infos[k].annotation[i] != '\0' && line[j] == infos[k].annotation[i]) {
                i++;
                j++;
                if(line[j]=='\\' && j+1<line_len) { //in case the annotation is protected
                    if(infos[k].annotation[i] == line[j+1])
                        j++;
                }
            }
            if(infos[k].annotation[i] == '\0') {
                int reverse_i =  j; //We will go in reverse to find the entity
                int num_paren = 1;
                while(reverse_i >= 0) {
                    if(line[reverse_i] =='{' && num_paren == 1) {
                        break;
                    }
                    else if(line[reverse_i] =='{' && num_paren > 0 ) {
                        num_paren--;
                    }
                    else if(line[reverse_i] =='}') {
                        num_paren++;
                    }
                    reverse_i--;
                }
                reverse_i++;
                int start = reverse_i;
                int end = 0;
                int annot_start = 0;
                int annot = -1;
                unichar *prev_char = NULL;
                unichar *entity_whole = NULL; // test on reverse_i is a quick fix to pass valgrind validation
                for(int x = (reverse_i > 0) ? reverse_i : 0; x < j; x++) {
                    if(line[x] == '{') {
                        start = x+1;
                        if (x > 0) {
                            if (prev_char!=NULL) free(prev_char);
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
                    else if(line[x] == '\\' || line[x] == '+' || line[x] == '}') {
                        if(annot_start > 0 && start > 0) {
                            annot = -1;
                            int matches = 0;
                            unichar *annot_ = (unichar*) malloc(sizeof(unichar) * ((x - annot_start)+1));
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
                                entity = (unichar*) malloc(sizeof(unichar) * ((end - start)+1));
                                z = 0;
                                int nb_expand=0;
                                for(int y = start; y < end; y++) {
                                    if(line[y] != '\\') {
                                        if(line[y] == '/') {
                                           nb_expand++;
                                           entity = (unichar*) realloc(entity,sizeof(unichar) * ((end - start)+1+nb_expand));
                                           entity[z++] = '\\';
                                        }
                                        entity[z++] = line[y];
                                    }
                                }
                                entity[z] = '\0';
                                int entity_len = z + 1;
                                if(entity_whole == NULL) {
                                entity_whole = (unichar*) malloc(sizeof(unichar) * (entity_len+1));
                                u_strcpy(entity_whole,entity);
                                }
                                else {
                                int current_len = u_strlen(entity_whole);
                                entity_whole =  (unichar*) realloc(entity_whole,sizeof(unichar) * (current_len + entity_len + 2));
                                if(prev_char[0] == '{') { //In case of multi level annotation, there could be multiple '\{' characters before the entity
                                    prev_char[0] = ' ' ;
                                }
                                if(infos[k].accept_count==1) {
                                    //if same element appears more than once then we store
                                    // them separately
                                    prev_char[0] = '+' ;
                                }
                                u_strcat(entity_whole,prev_char);
                                u_strcat(entity_whole,entity);
                                }
                                if(entity != NULL)
                                free(entity);
                                start = -1;
                            }
                            if(annot_ != NULL)
                                free(annot_);
                            annot_start = -1;
                        }
                    }
                }
                if(entity_whole != NULL) {
                    int entity_len = u_strlen(entity_whole);
                    unichar* protected_entity = NULL;
                    protected_entity = (unichar*) malloc(sizeof(unichar) * ((1+entity_len)*2));
                    entity_len = escape(entity_whole,protected_entity,P_PLUS_COLON_SLASH);
                    if(infos[k].entity_count == 0) {
                        infos[k].entities = (unichar*) malloc(sizeof(unichar) * (entity_len+1));
                        u_strcpy(infos[k].entities,protected_entity);
                        infos[k].entity_count++;
                        *updates = *updates + 1;
                    }
                    else {
                        int current_len = u_strlen(infos[k].entities);
                        infos[k].entities = (unichar*) realloc(infos[k].entities, sizeof(unichar) * (current_len + entity_len + 2));
                        infos[k].entities[current_len] = '+';
                        for(int a = current_len + 1, b = 0; a < current_len + entity_len + 2; a++)
                        infos[k].entities[a] = protected_entity[b++];
                        infos[k].entities[current_len + entity_len + 1] = '\0';
                    }
                    free(entity_whole);
                    free(protected_entity);
                }
                if (prev_char != NULL)
                    free(prev_char);
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
    /*if ((i + 1) == len_format) {
        u_fprintf(out, "%S", format);
        return;
    }*/
    u_fprintf(out, "\"%S", param);
    //*(format + i) = '\0';
    //u_fprintf(out, "%S", format);
    //u_printf( "%S", format);
    //*(format + i) = '%';

    u_fprintf(out, "%S", format+3);
}


int update_tmp_graph(const char *transducer, VersatileEncodingConfig *vec, unichar **lines, int total_lines, const unichar *start, int start_loc, int num_entities, int num_info, grfInfo *infos, int mode) {
    int status = 0;
    U_FILE *graph_file = u_fopen(vec,transducer,U_WRITE);
    if (graph_file !=NULL) {
    if(lines != NULL) {
        int num_lines = 0;
        int updates = 0;
        while(num_lines < total_lines && lines[num_lines] !=NULL) {
        size_t num_char = u_strlen(lines[num_lines]);
        if (num_lines == start_loc -1 && mode) {
            u_fprintf(graph_file,"%d\n",total_lines-start_loc+3*num_info); //total number of lines should be updated to reflect the newly added negative contexts
        }
        else if(mode && num_lines == start_loc) {
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
                if(infos[i].entity_count > 0) {
                    u_fprintf(graph_file,"%d ",infos[i].negLeftContxt_loc); //point to negative context
                }
            }
            u_fprintf(graph_file,"\n");
        }
        else if(mode && num_char > 3 && lines[num_lines][0] == '"' && lines[num_lines][1] == '$' && lines[num_lines][2] == 'G') {
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
                for(; j<(int)u_strlen(lines[num_lines]); j++) {
                    u_fprintf(graph_file,"%C",lines[num_lines][j]);
                }
                u_fprintf(graph_file,"\n");
                found++;
            }
            }
            if(found == 0) {
                u_fprintf(graph_file,"%S\n",lines[num_lines]);
            }
        }
        else {
            u_fprintf(graph_file,"%S\n",lines[num_lines]);
        }
        num_lines++;
        }
        if(mode) { // add the negative left context for each path
            for(int i = 0; i < num_info; i++) {
                u_fprintf(graph_file,"\"$![\" 10 20 1"); //use random coordinates
                u_fprintf(graph_file," %d \n",infos[i].negLeftContxt_loc+1);
                u_fprintf(graph_file,"\"<");
                for(int j=2; infos[i].annotation[j]!='\0'; j++ ) {
                    u_fprintf(graph_file,"%C",infos[i].annotation[j]);
                }
                u_fprintf(graph_file,">\" 10 20 1 %d \n",infos[i].negLeftContxt_loc+2);
                u_fprintf(graph_file,"\"$]\" 10 20 1 %d \n",infos[i].entity_loc); // point back to the entity
            }
        }
    }
    u_fclose(graph_file);
    status = 1;
    }
    return status;
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

        for (iteration = 0; current_transducer->repeat_mode == INFINITY || iteration < current_transducer->repeat_mode; iteration++) {
            if (in_place == 0) {

                if (labeled_text_name != NULL) {
                    free(labeled_text_name);
                }
                labeled_text_name = create_labeled_files_and_directory(text, previous_transducer_number,
                    transducer_number, previous_iteration, iteration, must_create_directory, 1);
            }

            launch_tokenize_in_Cassys(labeled_text_name, alphabet,
                snt_text_files->tokens_txt, vec, tokenize_args, display_perf, display_perf ? &time_tokenize : NULL);

            int entity = 0;
            char* updated_grf_file_name = NULL;
            char* updated_fst2_file_name = NULL;
            if (is_template_grf) {
                int *entity_loc = NULL;
                int num_annots = 0;
                unichar *start_node_line = NULL;
                int start_node_loc = -1;
                int total_lines = 0;
                int num_entities = 0;
                struct grfInfo *grf_infos = NULL;

                remove_extension(current_transducer->transducer_file_name, textbuf->template_name_without_extension);
                sprintf(textbuf->orig_grf, "%s.grf", textbuf->template_name_without_extension);

                unichar **grf_lines = load_file_in_memory(textbuf->orig_grf, vec, &total_lines);
                grf_infos = extract_info(grf_lines, &num_annots, total_lines, &start_node_loc, &start_node_line, &entity_loc);

                if (num_annots > 0) {
                    char* token_list = get_file_in_current_snt(text, transducer_number, iteration, "tok_by_alph", ".txt");
                    unichar**entity_string = extract_entities(token_list,snt_text_files->tok_by_alph_txt, vec, num_annots, &num_entities, grf_infos);
                    free(token_list);
                    free(entity_string);

                    updated_grf_file_name = create_updated_graph_filename(text,
                            in_place ? 0 : transducer_number,
                            in_place ? 0 : iteration,
                            filename_without_path(textbuf->template_name_without_extension), ".grf");

                    if (update_tmp_graph(updated_grf_file_name, vec, grf_lines, total_lines, start_node_line, start_node_loc, num_entities, num_annots, grf_infos, 1)) {
                        if (num_entities > 0) {
                            launch_grf2fst2_in_Cassys(updated_grf_file_name, alphabet, vec, display_perf, display_perf ? &time_grf2fst2 : NULL);

                            updated_fst2_file_name = create_updated_graph_filename(text,
                                in_place ? 0 : transducer_number,
                                in_place ? 0 : iteration,
                                filename_without_path(textbuf->template_name_without_extension), ".fst2");

                            entity = num_entities;
                        }

                    }
                    //update_tmp_graph(orig_grf, vec, grf_lines, total_lines, NULL, -1, -1, -1, NULL, 0);
                }
                free_file_in_memory(grf_lines);
                free_grf_info(grf_infos, num_annots);
                free(start_node_line);
            }

            if (is_template_grf != 1 || entity > 0) {
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
