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

#include "Cassys_tokens.h"
#include "Cassys.h"
#include "Cassys_concord.h"
#include "Cassys_lexical_tags.h"
#include "Snt.h"
#include "UnusedParameter.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

void cassys_tokens_2_graph(cassys_tokens_list *c,const char *fileName, int realign_token_graph_pointer){

    U_FILE *dot_desc_file = u_fopen(ASCII, fileName, U_WRITE);
    if (dot_desc_file == NULL) {
        fatal_error("Cannot open file %s\n",fileName);
        exit(1);
    }

    u_fprintf(dot_desc_file,"digraph text {\n");
    //u_fprintf(dot_desc_file,"\trankdir=\"LR\"");   //pour changer l'orientation, decommenter
    u_fprintf(dot_desc_file,"\tbgcolor=lightblue1\n");
    u_fprintf(dot_desc_file,"node [color=lightblue2, style=filled]\n");

    //u_fprintf(dot_desc_file,"\tbgcolor=antiqueblue\n");
    cassys_tokens_2_graph_subgraph(c, dot_desc_file, realign_token_graph_pointer);

    cassys_tokens_2_graph_walk_for_subgraph(c, dot_desc_file, NULL, realign_token_graph_pointer);

    u_fprintf(dot_desc_file, "\tNULL[label=\"END\" shape=Mdiamond]\n");
    u_fprintf(dot_desc_file,"}\n");

    u_fclose(dot_desc_file);
}

void cassys_tokens_2_graph_walk_for_subgraph(cassys_tokens_list *c, U_FILE *u, cassys_tokens_list *predecessor, int realign_token_graph_pointer){

    cassys_tokens_list *i;
    cassys_tokens_list *p = predecessor;
    unsigned int count = 0;

    for(i=c; i != NULL && i->transducer_id == c->transducer_id; i = i->next_token) {

        if(i->output != NULL) {
            cassys_tokens_2_graph_subgraph(i->output, u, realign_token_graph_pointer);
            cassys_tokens_2_graph_walk_for_subgraph(i->output, u, p, realign_token_graph_pointer);


            if (realign_token_graph_pointer == 0) {
                if (p != NULL){
                    u_fprintf(u, "\t_%p -> _%p\n", p, i->output);
                }
                else {
                    u_fprintf(u, "\tstart -> _%p\n", i->output);
                }

                u_fprintf(u, "\t_%p -> _%p [style=invis]\n", i, i->output);
            }
            else
            {
                if (p != NULL){
                    u_fprintf(u, "\t_%08x -> _%08x\n", (unsigned int)0, 0);
                }
                else {
                    u_fprintf(u, "\tstart -> _%08x\n", 0);
                }

                u_fprintf(u, "\t_%08x -> _%08x [style=invis]\n", (unsigned int)(count++), 0);
            }


        }

        p = i;
    }
}

void cassys_tokens_2_graph_subgraph(cassys_tokens_list *c, U_FILE *u, int realign_token_graph_pointer){

    static int cluster_number = 0;
    cassys_tokens_list *i;
    unsigned int count = 0;
    u_fprintf(u, "\tsubgraph cluster%d {\n", cluster_number);
    u_fprintf(u,"\tbgcolor=cornflowerblue\n");
    u_fprintf(u, "\tlabel = \"transducer %d_%d\"\n", c->transducer_id, c->iteration);

    if(c->transducer_id==0){
        u_fprintf(u,"\tstart[shape=Mdiamond]\n");
        if(c!=NULL){
            if (realign_token_graph_pointer == 0) {
                u_fprintf(u, "\tstart -> _%p\n", c);
            }
            else {
                u_fprintf(u, "\tstart -> _%08x\n", 0);
            }
        }
    }

    for (i = c; i != NULL && i->transducer_id == c->transducer_id; i = i->next_token) {

        if(is_lexical_token(i->token)){

            cassys_pattern *cp = load_cassys_pattern(i->token);
            unichar *unprotected_form = unprotect_lexical_tags(cp->form);
            unichar *form = protect_from_record(unprotected_form);

            //u_fprintf(u, "\t\t_%p[label=\"%S\" shape=record ]\n",i,form);

            u_fprintf(u, "\t\t_%p[fillcolor=steelblue1 label=\"",i);
            u_fprintf(u,"{ %S",form);
            u_fprintf(u,"| ");
            if(cp->lem!=NULL && cp->lem[0]!='\0'){
                unichar *lem = protect_from_record(cp->lem);
                u_fprintf(u," %S \\n",lem);
                free(lem);
            }
            u_fprintf(u,"| ");
            if(cp->code!=NULL){
                list_ustring *lu=cp->code;
                while(lu!=NULL){
                    u_fprintf(u," %S \\n",lu->string);
                    lu=lu->next;
                }
            }
            u_fprintf(u,"| ");
            if (cp->inflection != NULL) {
                list_ustring *lu = cp->inflection;
                while (lu != NULL) {
                    u_fprintf(u, " %S \\n", lu->string);
                    lu = lu->next;
                }
            }
            u_fprintf(u, "}\" shape=Mrecord ]\n");

            free(unprotected_form);
            free(form);
            free_cassys_pattern(cp);
        } else {
            unichar *label = protect_quote(i->token);

            if (realign_token_graph_pointer == 0) {
                u_fprintf(u, "\t\t_%p[fillcolor=steelblue label=\"%S\"]\n", i, label);
            }
            else {
                u_fprintf(u, "\t\t_%08x[fillcolor=steelblue label=\"%S\"]\n", (unsigned int)(count++), label);
            }
            free(label);
        }

        if (realign_token_graph_pointer == 0) {
            if (i->next_token != NULL) {
                u_fprintf(u, "\t\t_%p -> _%p\n", i, i->next_token);
            }
            else {
                u_fprintf(u, "\t\t_%p -> NULL\n", i);
            }
        }
        else {

            if (i->next_token != NULL) {
                u_fprintf(u, "\t\t_%08x -> _%08x\n", (unsigned int)(count++), 0);
            }
            else {
                u_fprintf(u, "\t\t_%08x -> NULL\n", (unsigned int)(count++));
            }
        }
    }
    u_fprintf(u,"\t}\n");
    cluster_number++;
}




unichar *protect_from_record(const unichar *u){

    int size = u_strlen(u);
    unichar *r=(unichar *)malloc(sizeof(unichar)*size*2+1);
    if(r == NULL){
        fatal_alloc_error("malloc");
    }

    int i,j;
    for(i=0, j=0; i<=size; i++){
        if(u[i]=='"' || u[i]=='<' || u[i]=='>' || u[i]=='{'||u[i]=='}'||u[i]=='|'){
            r[i+j]='\\';
            j++;
        }
        r[i+j]=u[i];
    }

    return r;
}


unichar *protect_quote(const unichar *u){

    int size = u_strlen(u);
    unichar *r=(unichar *)malloc(sizeof(unichar)*(size*2+1));
    if(r == NULL){
        fatal_alloc_error("malloc");
    }

    int i,j;
    for(i=0, j=0; i<=size; i++){
        if(u[i]=='"'){
            r[i+j]='\\';
            j++;
        }
        r[i+j]=u[i];
    }

    return r;
}


unichar *unprotect_lexical_tags(unichar *u){

    int size = u_strlen(u);

    unichar *result=(unichar *)malloc(sizeof(unichar)*(size+1));
    if (result == NULL) {
        fatal_alloc_error("malloc");
    }
    int i, j;
    for(i=0;i<=size;i++){
        result[i]='\0';
    }



    for (i = 0, j = 0; i <= size; i++) {
        if (u[i] == '\\') {
            if(u[i+1]=='.'|| u[i+1]=='+'|| u[i+1]==':' || u[i+1]=='{' ||  u[i+1]=='}'|| u[i+1]=='\\'){
                j++;
                continue;
            }
        }
        result[i-j] = u[i];
    }

    return result;
}


cassys_tokens_list *cassys_load_text(const VersatileEncodingConfig* vec, const char *tokens_text_name, const char *text_cod_name,
         struct text_tokens **tokens, const vector_int* uima_offset, cassys_tokens_allocation_tool * allocation_tool) {

    DISCARD_UNUSED_PARAMETER(uima_offset)

    *tokens = load_text_tokens(vec, tokens_text_name);
    if ((*tokens) == NULL)
        return NULL;

    ABSTRACTMAPFILE* map_text_cod = af_open_mapfile(text_cod_name, MAPFILE_OPTION_READ, 0);

    if (map_text_cod == NULL){
        fatal_error("Cannot open file %s\n", text_cod_name);
        exit(1);
    }
    const int* text_cod_buf = (const int*)af_get_mapfile_pointer(map_text_cod);
    unsigned int text_cod_size_nb_int = (unsigned int)(af_get_mapfile_size(map_text_cod) / sizeof(int));



    cassys_tokens_list *list = NULL;
    cassys_tokens_list *temp = list;


    for (unsigned int i = 0; i<text_cod_size_nb_int; i++) {
        int token_id = *(text_cod_buf + i);
        if (list == NULL){
            list = new_element((*tokens)->token[token_id], 0, 0, allocation_tool);
            temp = list;
        }
        else {
            temp->next_token = new_element((*tokens)->token[token_id], 0, 0, allocation_tool);
            temp = temp->next_token;
        }
    }

    af_release_mapfile_pointer(map_text_cod, text_cod_buf);
    af_close_mapfile(map_text_cod);

    return list;
}



/**
 *
 */
cassys_tokens_list *add_replaced_text(
            const char *text, cassys_tokens_list *list,
            int previous_transducer, int previous_iteration,
            int transducer_id, int iteration,
            const char *alphabet_name,
            const VersatileEncodingConfig* vec, cassys_tokens_allocation_tool * allocation_tool) {

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
                new_list(new_sentence_lu, transducer_id, iteration, allocation_tool);

            // performance enhancement :
            // Since matches are sorted, we begin the search from the last known position in the list.
            // We have to substract from the text position the current token position.
            cassys_tokens_list *list_position = get_element_at(current_list_position, previous_transducer, previous_iteration,
                (int)(l->token_start_offset - current_token_position));

            int replaced_sentence_length = (int)(l->token_end_offset - l->token_start_offset+1);
            int new_sentence_length = length(new_sentence_lu);

            add_output(list_position, new_sentence_ctl, previous_transducer, previous_iteration, transducer_id, iteration,
                replaced_sentence_length, new_sentence_length-1);

            // performance enhancement
            current_list_position = list_position;
            current_token_position = l->token_start_offset;

            if (prev_l != NULL){
                free(prev_l->label);
                free(prev_l);
            }
            prev_l=l;
            free_list_ustring(new_sentence_lu);
        }
    }
    if (prev_l != NULL){
        free(prev_l->label);
        free(prev_l);
    }
    free_fifo(stage_concord);
    free_snt_files(snt_text_files);
    free_alphabet(alphabet);

    return list;
}





cassys_tokens_list *next_element(cassys_tokens_list *list, int transducer_id, int iteration){
    if(list->next_token == NULL){
        return NULL;
    }

    cassys_tokens_list *temp = list->next_token;
    temp = get_output(temp,transducer_id, iteration);

    return temp;
}


const unichar *next_token(cassys_tokens_list *list, int transducer_id, int iteration){
    cassys_tokens_list *temp = next_element(list,transducer_id, iteration);

    if(temp == NULL){
        return NULL;
    }
    return temp -> token;
}


cassys_tokens_list *get_output(cassys_tokens_list *list, int transducer_id, int iteration){
    cassys_tokens_list *temp = list;

    if(list == NULL){
        return NULL;
    }

    while (temp -> output != NULL && ((temp -> output -> transducer_id
            == transducer_id && temp->output->iteration <= iteration)|| temp -> output -> transducer_id< transducer_id)) {
        temp = temp -> output;
    }

    return temp;
}


cassys_tokens_list *get_element_at(cassys_tokens_list *list, int transducer_id, int iteration, int position){
    int current_position = 0;
    cassys_tokens_list *temp = list;

    if (temp==NULL)
        return NULL;

    while (temp -> output != NULL && ((temp -> output -> transducer_id
            == transducer_id && temp->output->iteration <= iteration)|| temp -> output -> transducer_id< transducer_id)) {
        temp = temp -> output;
    }

    while(current_position < position){
        if(temp->next_token != NULL)
        {
            temp=temp->next_token;

            while (temp -> output != NULL && ((temp -> output -> transducer_id
                    == transducer_id && temp->output->iteration <= iteration)|| temp -> output -> transducer_id< transducer_id)) {
                temp = temp -> output;
            }
        }
        else
        {
            return NULL;
        }

        current_position++;
    }

    return temp;
}


cassys_tokens_list *add_output(cassys_tokens_list *list,
        cassys_tokens_list *output, int previous_transducer, int previous_iteration,
        int transducer_id, int iteration,
        int number_of_tokens_replaced, int number_of_output_tokens) {


    DISCARD_UNUSED_PARAMETER(transducer_id)
    DISCARD_UNUSED_PARAMETER(iteration)

    if (list == NULL) {
        return NULL;
    }

    list ->output = output;
    cassys_tokens_list *replacement_end = get_element_at(list, previous_transducer, previous_iteration, number_of_tokens_replaced);
    cassys_tokens_list *output_end =NULL;
    if(list->output!=NULL)
        output_end = get_element_at(list->output, list->output->transducer_id, list->output->iteration, number_of_output_tokens);
    if (output_end == NULL) {
        return NULL;
    }

    output_end -> next_token = replacement_end;

    return list;
}


cassys_tokens_list *new_list(list_ustring *l_u, int transducer_id, int iteration, cassys_tokens_allocation_tool * allocation_tool){
    cassys_tokens_list *head = NULL;

    if(l_u!=NULL){
        head = new_element(l_u -> string, transducer_id, iteration, allocation_tool);
        l_u=l_u->next;
    }

    cassys_tokens_list *current = head;

    while(l_u!=NULL){
        // free ajout� pour lib�rer next_token : verifier son utilit� !
        free(current->next_token);
        current -> next_token = new_element(l_u -> string, transducer_id, iteration, allocation_tool);


        current = current ->next_token;

        list_ustring *l_u_next = l_u->next;
        l_u= l_u_next;
    }

    return head;
}





cassys_tokens_allocation_tool *build_cassys_tokens_allocation_tool()
{
    cassys_tokens_allocation_tool *allocation_tool = (cassys_tokens_allocation_tool*)malloc(sizeof(cassys_tokens_allocation_tool));
    if (allocation_tool == NULL){
        fatal_alloc_error("build_cassys_tokens_allocation_tool");
        exit(1);
    }
    allocation_tool->first_item = NULL;
    allocation_tool->allocator_tokens_list = create_abstract_allocator("build_cassys_tokens_allocation_tool", AllocatorCreationFlagAutoFreePrefered);
    allocation_tool->must_free_tokens_list = (get_allocator_cb_flag(allocation_tool->allocator_tokens_list) & AllocatorGetFlagAutoFreePresent) ? 0 : 1;
    return allocation_tool;
}


void free_cassys_tokens_allocation_tool(cassys_tokens_allocation_tool * allocation_tool)
{
    struct cassys_tokens_allocation_tool_item* item = allocation_tool->first_item;
    while (item != NULL)
    {
        struct cassys_tokens_allocation_tool_item* next_item = item->next;
        free_cb(item->tokens_list_item->token, allocation_tool->allocator_tokens_list);
        free_cb(item->tokens_list_item, allocation_tool->allocator_tokens_list);
        free(item);
        item = next_item;
    }

    close_abstract_allocator(allocation_tool->allocator_tokens_list);
    free(allocation_tool);
}




cassys_tokens_list *new_element(const unichar *u, int transducer_id, int iteration, cassys_tokens_allocation_tool * allocation_tool){

    cassys_tokens_list *l = (cassys_tokens_list*)malloc_cb(sizeof(cassys_tokens_list) * 1, allocation_tool->allocator_tokens_list);
    if(l == NULL){
        fatal_alloc_error("new_element");
        exit(1);
    }
    if (allocation_tool->must_free_tokens_list) {
        cassys_tokens_allocation_tool_item* allocation_item = (cassys_tokens_allocation_tool_item*)malloc(sizeof(cassys_tokens_allocation_tool_item));
        if (allocation_item == NULL){
            fatal_alloc_error("new_element");
            exit(1);
        }

        allocation_item->tokens_list_item = l;
        allocation_item->next = allocation_tool->first_item;
        allocation_tool->first_item = allocation_item;
    }

    l->transducer_id = transducer_id;
    l->iteration = iteration;
    l->output = NULL;
    l->next_token = NULL;
    l->token=u_strdup(u, allocation_tool->allocator_tokens_list);
    return l;
}
/*
void free_cassys_tokens_list(cassys_tokens_list *l){
    while(l!=NULL){
        cassys_tokens_list *l_next_token=NULL;
        free(l->token);
        free_cassys_tokens_list(l->output);
        if(l->next_token!=NULL  && l->transducer_id == l->next_token -> transducer_id){
            l_next_token = l->next_token;
        }
        free(l);
        l=l_next_token;
    }
}*/

void display_text(cassys_tokens_list *l, int transducer_id, int iteration){
    u_printf("cassys_token_list = ");
    while(l!=NULL){
        u_printf("%S",l->token);
        l=next_element(l,transducer_id, iteration);
    }
    u_printf("\n");
}

bool u_isspace(const unichar *u){
    if ( u[0] == ' ' || u[0] == '\t' || u[0] == 0x0d|| u[0] == 0x0a) {
        return true;
    }
    else {
        return false;
    }
}


cassys_tokens_list* cassys_trim(cassys_tokens_list *l, unichar *string_before, unichar *string_next, cassys_tokens_allocation_tool * allocation_tool){

    cassys_tokens_list *last = NULL;
    // We deal with successive spaces in the new concordance
    for(cassys_tokens_list *ite_base = l; ite_base !=NULL; ite_base=ite_base->next_token){
        if (u_isspace(ite_base->token)) {

            cassys_tokens_list *ite_space = ite_base->next_token;
            while( ite_space !=NULL ){
                if (u_isspace(ite_space->token)) {
                    // suppress the node in the graph
                    ite_base -> next_token = ite_space->next_token;

                    cassys_tokens_list *to_be_freed = ite_space;
                    ite_space=ite_space->next_token;
                    free_cb(to_be_freed->token, allocation_tool->allocator_tokens_list);
                    free_cb(to_be_freed, allocation_tool->allocator_tokens_list);
                } else {
                    // not a space character
                    break;
                }
            }
        }

        // We remind the last token of the list to trim it with string_next
        if(ite_base->next_token==NULL){
            last = ite_base;
        }
    }

    if(string_before !=NULL){
        if(u_isspace(string_before)&& u_isspace(l->token)){
            cassys_tokens_list *to_be_freed = l;
            l=l->next_token;
            free_cb(to_be_freed->token, allocation_tool->allocator_tokens_list);
            free_cb(to_be_freed, allocation_tool->allocator_tokens_list);
        }
    }
    if(string_next !=NULL){
        if(u_isspace(string_next)&& u_isspace(last->token)){
            free_cb(last->token, allocation_tool->allocator_tokens_list);
            free_cb(last, allocation_tool->allocator_tokens_list);
            last=NULL;
        }
    }

    return l;
}


} // namespace unitex

