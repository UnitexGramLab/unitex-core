/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "HashTable.h"
#include "String_hash.h"
#include "FIFO.h"
#include "Snt.h"
#include "UnitexRevisionInfo.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

void display_lu(struct list_ustring *l){
    struct list_ustring *u;

    u_printf("begin\n");
    for(u=l; u!=NULL; u=u->next){
        u_printf("%S\n",u->string);
    }
    u_printf("\n");
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
    unichar* line = NULL;
    size_t size_buffer_line = 0;

    struct fifo *f = new_fifo();

    U_FILE *concord_desc_file;
    concord_desc_file = u_fopen(vec,concord_file_name,U_READ);
    if( concord_desc_file == NULL){
        fatal_error("Cannot open file %s\n",concord_file_name);
        exit(1);
    }

    if (u_fgets_dynamic_buffer(&line, &size_buffer_line, concord_desc_file) == EOF){
        error("Malformed concordance file %s",concord_file_name);
    }

    while (u_fgets_dynamic_buffer(&line, &size_buffer_line, concord_desc_file) != EOF){
        locate_pos *l = read_concord_line(line);
        put_ptr(f,l);
    }
    if (line != NULL) {
        free(line);
    }
    u_fclose(concord_desc_file);
    return f;
}



int count_concordance(const char *concord_file_name, const VersatileEncodingConfig* vec){

    struct fifo *stage_concord = read_concord_file(concord_file_name, vec);

    int number_of_concordance = 0;
    while (!is_empty(stage_concord)) {

        locate_pos *l = (locate_pos*) take_ptr(stage_concord);
        free(l->label);
        free(l);

        number_of_concordance ++;
    }
    free_fifo(stage_concord);

    return number_of_concordance;
}




void protect_lexical_tag_in_concord(const char *concord_file_name, const OutputPolicy op, const VersatileEncodingConfig *vec) {

    struct fifo *stage_concord = read_concord_file(concord_file_name, vec);

    U_FILE *concord_xml_desc = u_fopen(vec, concord_file_name, U_WRITE);
    if (concord_xml_desc == NULL) {
        fatal_error("Cannot open file %s\n", concord_file_name);
        exit(1);
    }
    switch (op) {
        case MERGE_OUTPUTS: {
            u_fprintf(concord_xml_desc, "#M\n");
            break;
        }
        case REPLACE_OUTPUTS: {
            u_fprintf(concord_xml_desc, "#R\n");
            break;
        }
        default:
            fatal_error("Only Replace and merge output are currently allowed with transducers\n");
            break;
    }


    while (!is_empty(stage_concord)) {

        locate_pos *l = (locate_pos*) take_ptr(stage_concord);

        unichar *protected_line = protect_lexical_tag(l->label, false);

        u_fprintf(concord_xml_desc, "%ld.%ld.%ld %ld.%ld.%ld %S\n",
                l->token_start_offset, l->character_start_offset,
                l->logical_start_offset, l->token_end_offset,
                l->character_end_offset, l->logical_end_offset, protected_line);

        free(protected_line);
        free(l->label);
        free(l);
    }
    u_fclose(concord_xml_desc);
    free_fifo(stage_concord);
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

struct standOffInfo {
    unichar *type;
    unichar *subtype;
    struct hash_table *entList;
    struct string_hash* entity_count;
};

void free_standoff_info(standOffInfo *infos,int num) {
    for(int i=0; i<num; i++) {
        free(infos[i].subtype);
        free(infos[i].type);
        free_hash_table(infos[i].entList);
        free_string_hash(infos[i].entity_count);
    }
    free(infos);
}

void print_standoff(U_FILE *out,standOffInfo *infos, int num_info,const char* lang) {
    char* output_lang = NULL;
    const char* semver = get_unitex_semver_string();
    if (lang == NULL || strcmp("",lang) == 0) {
        output_lang = (char *)malloc(sizeof(char)*3);
        output_lang[0]='\0';
    }
    else {
        output_lang = (char *)malloc(sizeof(char)*(strlen(lang) + 14));
        sprintf(output_lang," xml:lang=\"%s\"",lang);
    }
    for(int i=0; i<num_info; i++) {
        int count = 0;
        int capacity = infos[i].entList->capacity;
        if(infos[i].entList->number_of_elements > 0) {
            u_fprintf(out,"<listAnnotation type=\"%S\"",infos[i].type);
            if(infos[i].subtype != NULL)
                u_fprintf(out," subtype=\"%S\"",infos[i].subtype);
            u_fprintf(out," %s>\n",output_lang);
            for(int j=0; j<capacity
                    && count <infos[i].entList->number_of_elements; j++){
                if(infos[i].entList->table[j] !=NULL) {
                    if(infos[i].entList->table[j]->ptr_key !=NULL) {
                        int term_len = u_strlen((const unichar *)infos[i].entList->table[j]->ptr_key);
                        unichar *term = (unichar *)malloc(sizeof(unichar)*(term_len+1));
                        u_strcpy(term,(const unichar *)infos[i].entList->table[j]->ptr_key);
                        u_fprintf(out,"\t<annotationBlock corresp=\"text\">\n");
                        u_fprintf(out,"\t\t<%S ",infos[i].type);
                        if(infos[i].subtype != NULL)
                            u_fprintf(out,"type=\"%S\" ",infos[i].subtype);
                        unichar *type_lower;
                        type_lower = (unichar *)malloc(sizeof(unichar) * (u_strlen(infos[i].type) + 1));
                        u_strcpy(type_lower, infos[i].type);
                        u_tolower(type_lower);
                        u_fprintf(out,"change=\"#Unitex-%s\" resp=\"istex-rd\" scheme=\"https://%S",semver, type_lower);
                        free(type_lower);
                        if(infos[i].subtype != NULL)
                            u_fprintf(out,"%S",infos[i].subtype);
                        u_fprintf(out,"-entity.data.istex.fr\">\n");
                        u_fprintf(out,"\t\t\t<term>");
                        int k=0;
                        for(; k<term_len && term[k]!='>'; k++)
                            ;
                        k += 1;
                        for(;k<term_len && term[k]!='<'; k++)
                            u_fprintf(out,"%C",term[k]);
                        u_fprintf(out,"</term>\n");
                        u_fprintf(out,"\t\t\t<fs type=\"statistics\">\n");
                        u_fprintf(out,"\t\t\t\t<f name=\"frequency\">\n");
                        u_fprintf(out,"\t\t\t\t\t<numeric value=\"");
                        int idx = get_value_index((const unichar *)(infos[i].entList->table[j]->ptr_key),infos[i].entity_count,DONT_INSERT);
                        if (idx>-1) {
                            int count_len = u_strlen(infos[i].entity_count->value[idx]);
                            u_fprintf(out,"%d",count_len);
                        }
                        else {
                            u_fprintf(out,"1");
                        }
                        u_fprintf(out,"\"/>\n");
                        u_fprintf(out,"\t\t\t\t</f>\n");
                        u_fprintf(out,"\t\t\t</fs>\n");
                        u_fprintf(out,"\t\t</%S>\n",infos[i].type);
                        u_fprintf(out,"\t</annotationBlock>\n");
                        count++;
                        free(term);
                    }
                }
            }
            u_fprintf(out,"</listAnnotation>\n");
        }
    }
    free(output_lang);
}

void getMetaInfo(unichar *e, unichar **s, unichar **t) {
    int entity_len = u_strlen(e);
    if(entity_len>2 && e[0]== '<' && e[entity_len-1]=='>') {
        int i=1;
        int t_start = i;
        int t_end = -1;
        int s_start = -1;
        int s_end = -1;
        int in_subt = 0;
        while(i<entity_len && e[i]!='>') {
            if(e[i]==' ' && t_end ==-1) {
                t_end = i;
                if(i+4<entity_len-1) {
                    if(e[i+1]=='t' && e[i+2]=='y' && e[i+3]=='p' && e[i+4]=='e')
                        in_subt=1;
                }
            }
            else if(in_subt == 1 && e[i] =='"') {
                if(s_start== -1)
                    s_start = i+1;
                else {
                    s_end = i;
                    in_subt = 0;
                }
            }
            i++;
        }
        if(t_end == -1) {
            t_end = i;
        }
        *t = (unichar*) malloc(sizeof(unichar)* (t_end-t_start+1));
        i=0;
        for(int j=t_start; j<t_end; j++,i++) {
            (*t)[i] = e[j];
        }
        (*t)[i] = '\0';

        if(s_start>=0 && s_end>s_start) {
            *s = (unichar*) malloc(sizeof(unichar)* (s_end-s_start+1));
            i=0;
            for(int j=s_start; j<s_end; j++,i++) {
                (*s)[i] = e[j];
            }
            (*s)[i] = '\0';
        }
    }
}

int findEntityList(standOffInfo *infos, int num,
        unichar *s, unichar *t) {
    int found = -1;
    for(int i=0; i<num; i++) {
        if(u_strcmp(t,infos[i].type) == 0) {
            if(s==NULL && infos[i].subtype==NULL) {
                found = i;
                break;
            }
            else if(s!=NULL && infos[i].subtype != NULL
                    && u_strcmp(s,infos[i].subtype) == 0) {
                    found = i;
                    break;
            }
        }
    }
    return found;
}

void construct_istex_standoff(const char *text_name,
        VersatileEncodingConfig* vec, const char* original_file, const char* lang, const char* stdoff_file) {
    char text_name_without_extension[FILENAME_MAX];
    char result_file[FILENAME_MAX];
    text_name_without_extension[0] = '\0';
    result_file[0] = '\0';
    size_t size_buffer_line = 0;
    unichar *line = NULL;
    struct standOffInfo *infos = NULL;
    int num_info = 0;
    U_FILE *concord_file = u_fopen(vec, text_name, U_READ);
    if(concord_file != NULL) {
        while(u_fgets_dynamic_buffer(&line, &size_buffer_line,
                concord_file) != EOF) {
            int limit = u_strlen(line);
            if(line != NULL && limit > 2 &&
                    line[limit - 1] == '>') {
                int pos = -1;
                for(int k = 0; k < limit; k++) {
                    if(line[k] == '<') {
                        pos = k;
                        break;
                    }
                }
                if (pos >= 0) {
                    unichar *entity = (unichar*) malloc(sizeof(unichar)*limit);
                    for(int k=pos,i=0; k<limit; k++,i++) {
                        entity[i] = line[k];
                    }
                    entity[limit-pos] = '\0';
                    unichar *type = NULL;
                    unichar *subtype= NULL;
                    getMetaInfo(entity,&subtype,&type);
                    int found = findEntityList(infos,num_info,subtype,type);
                    if(found > -1 && found<num_info) {
                        get_value(infos[found].entList,entity,HT_INSERT_IF_NEEDED);
                        int idx = get_value_index(entity,infos[found].entity_count,DONT_INSERT);
                        if(idx == -1) {
                            unichar count[2];
                            count[0] = '1';
                            count[1] = '\0';
                            get_value_index(entity,infos[found].entity_count,INSERT_IF_NEEDED,count);
                        }
                        else {
                            int count_len = u_strlen(infos[found].entity_count->value[idx]);
                            infos[found].entity_count->value[idx] = (unichar*)realloc(infos[found].entity_count->value[idx],sizeof(unichar)*(count_len + 2));
                            infos[found].entity_count->value[idx][count_len]='1';
                            infos[found].entity_count->value[idx][count_len+1]='\0';
                        }
                    }
                    else {
                        infos = (standOffInfo*)realloc(infos, (num_info + 1) * sizeof(standOffInfo));
                        infos[num_info].entList = new_hash_table((HASH_FUNCTION)hash_unichar,
                                    (EQUAL_FUNCTION)u_equal,(FREE_FUNCTION)free,
                                    NULL,(KEYCOPY_FUNCTION)keycopy);
                        infos[num_info].type = (unichar*)malloc(sizeof(unichar) * (u_strlen(type) + 1));
                        u_strcpy(infos[num_info].type,type);
                        infos[num_info].subtype = NULL;
                        infos[num_info].entity_count = NULL;
                        infos[num_info].entity_count = new_string_hash();
                        unichar *count = (unichar*)malloc(sizeof(unichar) * 3);
                        count[0] = '1';
                        count[1] = '\0';
                        get_value_index(entity,infos[num_info].entity_count,INSERT_IF_NEEDED,count);
                        free(count);
                        if(subtype !=NULL) {
                            infos[num_info].subtype = (unichar*)malloc(sizeof(unichar) * (u_strlen(subtype) + 1));
                            u_strcpy(infos[num_info].subtype,subtype);
                            get_value(infos[num_info].entList,entity,HT_INSERT_IF_NEEDED);
                        }
                        num_info++;
                    }
                    if(type!=NULL)
                        free(type);
                    if(subtype!=NULL)
                        free(subtype);
                    free(entity);
                }
            }
        }
        free(line);
        u_fclose(concord_file);
        remove_extension(original_file, text_name_without_extension);
        sprintf(result_file,"%s_standoff.txt",text_name_without_extension);
        U_FILE *out_file = u_fopen(vec, result_file, U_WRITE);
        if (stdoff_file !=NULL && strcmp("",stdoff_file)!=0) {
            unichar *line_2 = NULL;
            size_t size_buffer_line_2 = 0;
            U_FILE *header_file = u_fopen(vec,stdoff_file,U_READ);
            if(header_file != NULL) {
                u_fprintf(out_file,"<standOff>\n");
                while(u_fgets_dynamic_buffer(&line_2, &size_buffer_line_2, header_file) != EOF) {
                    u_fprintf(out_file,"%S\n",line_2);
                }
                u_fclose(header_file);
                free(line_2);
            }
        }
        print_standoff(out_file,infos,num_info,lang);
        if (stdoff_file !=NULL && strcmp("",stdoff_file)!=0)
            u_fprintf(out_file,"</standOff>\n");
        u_fclose(out_file);
        free_standoff_info(infos,num_info);
    }
}

void construct_istex_token(const char *text_name, VersatileEncodingConfig* vec,
        const char* original_file) {
    char text_name_without_extension[FILENAME_MAX];
    char result_file[FILENAME_MAX];
    text_name_without_extension[0] = '\0';
    result_file[0] = '\0';
    size_t size_buffer_line = 0;
    unichar *line = NULL;
    unichar *newline = NULL;
    U_FILE *token_file = u_fopen(vec, text_name, U_READ);
    remove_extension(original_file, text_name_without_extension);
    sprintf(result_file,"%s_token.txt",text_name_without_extension);
    U_FILE *out_file = u_fopen(vec, result_file, U_WRITE);
    if(token_file != NULL && out_file != NULL) {
        while(u_fgets_dynamic_buffer(&line,
                &size_buffer_line, token_file) != EOF) {
            if(line !=NULL && line[0] == '{') {
                int limit = u_strlen(line) - 1;
                int pos = limit;
                newline = (unichar*) malloc(sizeof(unichar)*(limit+2));
                for(int k=limit; k>0; k--) {
                    if(line[k] == '}') {
                        pos = k;
                        break;
                    }
                }
                for(int i=0; i<pos+1; i++) {
                    newline[i] = line[i];
                }
                newline[pos+1] = '\0';
                u_fprintf(out_file,"%S\n",xmlizeConcordLine(newline));
            }
        }
        free(line);
        free(newline);
        u_fclose(token_file);
        u_fclose(out_file);
    }
}



void construct_cascade_concord(cassys_tokens_list *list, const char *text_name,
        int number_of_transducer, int iteration,
    VersatileEncodingConfig* vec) {

    u_printf("Construct cascade concord\n");

    struct snt_files *snt_file = new_snt_files(text_name);

    U_FILE *concord_desc_file = u_fopen(vec, snt_file->concord_ind,U_WRITE);
    if( concord_desc_file == NULL){
        fatal_error("Cannot open file %s\n",snt_file->concord_ind);
        exit(1);
    }

    u_printf("Concord File %s successfully opened\n",snt_file->concord_ind);

    if (list == NULL) {
        error("empty text");

        u_fclose(concord_desc_file);
        free(snt_file);
        return;
    }

    u_fprintf(concord_desc_file,"#M\n");

    cassys_tokens_list *current_pos_in_original_text = list;

    cassys_tokens_list *output=get_output(list, number_of_transducer, iteration);
    struct list_ustring *sentence = NULL;
    struct list_ustring *latest_on_sentence = NULL;
    bool output_detected = false;
    long token_position=0;

    while(current_pos_in_original_text != NULL){
        if (output == NULL) {
            u_printf("output = NULL\n");
        }

        if(output == NULL || output -> transducer_id == 0){
            if(output_detected){
                int start_position = (int)token_position;
                int last_token_length = 0;
                while(current_pos_in_original_text != output){
                    token_position ++;
                    last_token_length = u_strlen(current_pos_in_original_text -> token)-1;
                    current_pos_in_original_text = current_pos_in_original_text -> next_token;
                }

                // token position pointe sur le token suivant�
                int end_position=(int)(token_position-1);

                if(sentence == NULL){
                    fatal_error("construct_cassys_concordance : Phrase de remplacement vide\n");
                }

                struct list_ustring *iterator = sentence;

                /*
                // remove code not useful...
                while(iterator -> next != NULL){
                    iterator = iterator -> next;
                }

                iterator = sentence;
                */

                u_fprintf(concord_desc_file, "%d.0.0 %d.%d.0 ",start_position,end_position,last_token_length);
                while(iterator != NULL){
                    u_fputs(iterator->string,concord_desc_file);
                    //u_printf("ccc iter = %S\n",iterator->string);
                    //u_fprintf(concord_desc_file,"concord.ind : %S %S %S\n",iterator->string, previous_pos_in_original_text->token, current_pos_in_original_text->token);
                    iterator = iterator -> next;
                }
                u_fprintf(concord_desc_file,"\n");

                free_list_ustring(sentence);
                sentence = NULL;
                latest_on_sentence = NULL;

                if(current_pos_in_original_text==NULL){
                    break;
                }

                current_pos_in_original_text = current_pos_in_original_text -> next_token;
                output = get_output(current_pos_in_original_text, number_of_transducer, iteration);
                token_position++;



                output_detected = false;
            } else {
                if (current_pos_in_original_text == NULL) {
                    break;
                }

                current_pos_in_original_text = current_pos_in_original_text -> next_token;
                output = get_output(current_pos_in_original_text,number_of_transducer, iteration);
                token_position++;
            }
        }
        else {
            //u_printf("insert new sentence\n");

            sentence = insert_at_end_of_list_with_latest(output->token, sentence, &latest_on_sentence);
            output = output -> next_token;
            output = get_output(output, number_of_transducer, iteration);
            output_detected = true;
            //display_lu(sentence);
        }

    }

    free_list_ustring(sentence);

    u_fclose(concord_desc_file);
    free(snt_file);
}



void construct_xml_concord(const char *text_name, VersatileEncodingConfig* vec){

    u_printf("Building xml concord\n");

    struct snt_files *snt_file = new_snt_files(text_name);

    xmlizeConcordFile(snt_file->concord_ind, vec);


    free(snt_file);
}





}
