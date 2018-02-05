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

#include "AutomatonDictionary2Bin.h"
#include "DictionaryTree.h"
#include "Error.h"
#include "CompressedDic.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * This function associates an offset to the given dictionary node and updates
 * the 'bin_size' value according to the number of bytes taken by this node
 * and its transitions. Then, the function is called recursively on all the
 * nodes that are children of 'node'.
 * If the node has already been numbered (we are in an automaton,
 * not a tree), the function does nothing.
 */
void number_node_old_style(struct dictionary_node* node,int *bin_size) {
if (node->offset!=-1) {
    /* Nothing to do if there is already an offset */
    return;
}
/* We give an offset to the node */
node->offset=(*bin_size);

/* And we update 'bin_size' with the 2 bytes that will be used
 * to code the number of transitions of this node */
(*bin_size)=(*bin_size)+2;

if (node->single_INF_code_list!=NULL) {
    /* If the node is a final one, we add 3 bytes for coding the INF line number */
    (*bin_size)+=3;
}
/* Then we count the transitions */
node->n_trans=0;
struct dictionary_node_transition* tmp;
tmp=node->trans;
while (tmp!=NULL) {
    /* For each transition, we count 2 bytes for the letter and 3 bytes for the
     * destination offset */
    (*bin_size)+=5;
    tmp=tmp->next;
    /* We update the number of transitions */
    (node->n_trans)++;
}
/* Finally, we number all the children of the node */
tmp=node->trans;
while (tmp!=NULL) {
  number_node_old_style(tmp->node,bin_size);
  tmp=tmp->next;
}
}


/**
 * This function computes node offsets in a new way, taking into account
 * the fact that information may be encoded in variable ways.
 */
void number_node_new_style(struct dictionary_node* node,int *bin_size,
        BinStateEncoding state_encoding,
        t_fnc_bin_write_bytes char_write_function,t_fnc_bin_write_bytes inf_number_write_function,t_fnc_bin_write_bytes offset_write_function,
        int* inf_indirection) {
if (node->offset!=-1) {
    /* Nothing to do if there is already an offset */
    return;
}
if (node->trans==NULL) {
    /* No transition? We can give an offset to this state */
    node->offset=(*bin_size);
    if (state_encoding==BIN_CLASSIC_STATE) {
        /* A classic final state with no transition takes 5 bytes */
        (*bin_size)+=5;
    } else if (state_encoding==BIN_NEW_STATE) {
        /* A new style final state starts with a variable length value
         * representing the finality and the number of transitions.
         * Since there is no transition, this means 1 byte.
         * We then add the space needed by the inf number, if any */
        (*bin_size)+=1+bin_get_value_length(inf_indirection[node->INF_code],inf_number_write_function);
    } else {
        /* A .bin2 state has no inf number */
        (*bin_size)+=1;
    }
    return;
}
/* If there are transitions, we have to attribute offsets to all pointed states first */
struct dictionary_node_transition* tmp;
tmp=node->trans;
while (tmp!=NULL) {
    number_node_new_style(tmp->node,bin_size,state_encoding,char_write_function,inf_number_write_function,offset_write_function,inf_indirection);
    (node->n_trans)++;
    tmp=tmp->next;
}
/* Now, we can securely assign an offset to the current node */
node->offset=(*bin_size);
if (state_encoding==BIN_CLASSIC_STATE) {
    /* A classic state takes 2 bytes */
    (*bin_size)+=2;
    /* Plus 3 bytes if it's final */
    if (node->single_INF_code_list!=NULL) (*bin_size)+=3;
} else if (state_encoding==BIN_NEW_STATE) {
    /* A new style state starts with a variable length value
     * representing the finality and the number of transitions.
     * Since there is no transition, this means 1 byte.
     * We then add the space needed by the inf number, if any */
    int value=node->n_trans<<1;
    (*bin_size)+=bin_get_value_length(value,BIN_VARIABLE);
    if (node->single_INF_code_list!=NULL) (*bin_size)+=bin_get_value_length(inf_indirection[node->INF_code],inf_number_write_function);
} else {
    /* A .bin2 state has no inf number */
    int value=node->n_trans<<1;
    (*bin_size)+=bin_get_value_length(value,BIN_VARIABLE);
}
/* Finally, we update the bin size by taking into account the space
 * needed by the outgoing transitions */
tmp=node->trans;
while (tmp!=NULL) {
    (*bin_size)+=bin_get_value_length(tmp->letter,char_write_function);
    if (state_encoding!=BIN_BIN2_STATE) {
        (*bin_size)+=bin_get_value_length(tmp->node->offset,offset_write_function);
    } else {
        /* For a .bin2 transition, we use an extra bit to note if there is an output */
        (*bin_size)+=bin_get_value_length(tmp->node->offset<<1,offset_write_function);
        if (tmp->output && tmp->output[0]!='\0') {
            (*bin_size)+=bin_get_string_length(tmp->output,char_write_function);
        }
    }
    tmp=tmp->next;
}
}


void number_node(struct dictionary_node* root,int *bin_size,int new_style_bin,
        BinStateEncoding state_encoding,
        t_fnc_bin_write_bytes char_write_function,t_fnc_bin_write_bytes inf_number_write_function,t_fnc_bin_write_bytes offset_write_function,
        int* inf_indirection) {
if (root==NULL) return;
if (!new_style_bin) {
    number_node_old_style(root,bin_size);
    return;
}
number_node_new_style(root,bin_size,state_encoding,char_write_function,inf_number_write_function,offset_write_function,
        inf_indirection);
}


/**
 * This function dumps the information relative to 'node' and its transitions
 * in the 'bin' array. Then, the function is called recursively on the children
 * nodes of 'node'. The function assumes that all nodes have previously been
 * numbered with the function 'number_node'.
 *
 * If the node has already been dumped, the function does nothing.
 *
 * The parameters 'n_states' and 'n_transitions' are used to count the states
 * and transitions of the dictionary automaton.
 */
void fill_bin_array(struct dictionary_node* node,int *n_states,int *n_transitions,
                        unsigned char* bin,int* inf_indirection,
                        BinStateEncoding state_encoding,
                        t_fnc_bin_write_bytes char_write_function,t_fnc_bin_write_bytes inf_number_write_function,t_fnc_bin_write_bytes offset_write_function,
                        BinType bin_type) {
if (node==NULL) return;
if (node->INF_code==-1) {
    /* We use this test to know if the node has already been dumped */
   return;
}
/* We increase the number of states */
(*n_states)++;

/* We take the offset of the node as a base */
int pos=node->offset;
int final=(node->single_INF_code_list!=NULL);
int inf_code=-1;
if (final && bin_type==BIN_CLASSIC) {
    inf_code=inf_indirection[node->INF_code];
    if (inf_code==-1) {
        fatal_error("fill_bin_array: Invalid INF line number redirection for code #%d\n",node->INF_code);
    }
}
write_dictionary_state(bin,state_encoding,inf_number_write_function,&pos,final,node->n_trans,inf_code);
/* Then, we assign -1 to 'node->INF_code' in order to mark that the node
 * has been dumped */
node->INF_code=-1;
/* And we process all the transitions of the node */
struct dictionary_node_transition* tmp;
tmp=node->trans;
while (tmp!=NULL) {
    /* We increase the number of transitions */
    (*n_transitions)++;
    write_dictionary_transition(bin,&pos,char_write_function,offset_write_function,tmp->letter,tmp->node->offset,
            bin_type,tmp->output);
    /* And we dump the destination node recursively */
    fill_bin_array(tmp->node,n_states,n_transitions,bin,inf_indirection,
            state_encoding,char_write_function,inf_number_write_function,offset_write_function,bin_type);
    tmp=tmp->next;
}
}


/**
 * This function saves the dictionary automaton whose initial state is 'root'
 * in a .bin file named 'output'.
 * The parameters 'n_states' and 'n_transitions' are used to count the states
 * and transitions of the dictionary automaton. 'bin_size' represents the size
 * of the resulting .bin file.
 */
void create_and_save_bin(struct dictionary_node* root,const char* output,int *n_states,
                        int *n_transitions,int *bin_size,int* inf_indirection,int new_style_bin,
                        BinType bin_type) {
U_FILE* f;
/* The output file must be opened as a binary one */
f=u_fopen(BINARY,output,U_WRITE);
if (f==NULL) {
  fatal_error("Cannot write automaton file %s\n",output);
}
/* The .bin size is initialized to 4, because of the 4 first bytes that will be
 * used to store the size of the .bin */
(*n_states)=0;
(*n_transitions)=0;
BinStateEncoding state_encoding=BIN_CLASSIC_STATE;
BinEncoding char_encoding=BIN_2BYTES;
BinEncoding inf_number_encoding=BIN_3BYTES;
BinEncoding offset_encoding=BIN_3BYTES;
(*bin_size)=BIN_V1_HEADER_SIZE;
if (new_style_bin) {
    state_encoding=BIN_NEW_STATE;
    char_encoding=BIN_VARIABLE;
    inf_number_encoding=BIN_VARIABLE;
    offset_encoding=BIN_VARIABLE;
    (*bin_size)=BIN_V2_HEADER_SIZE;
}
if (bin_type==BIN_BIN2) {
    state_encoding=BIN_BIN2_STATE;
}


t_fnc_bin_write_bytes char_write_function=get_bin_write_function_for_encoding(char_encoding) ;
t_fnc_bin_write_bytes inf_number_write_function=get_bin_write_function_for_encoding(inf_number_encoding) ;
t_fnc_bin_write_bytes offset_write_function=get_bin_write_function_for_encoding(offset_encoding) ;


/* We give offsets to the dictionary node and we get the .bin size */
number_node(root,bin_size,new_style_bin,state_encoding,char_write_function,inf_number_write_function,offset_write_function,
        inf_indirection);
if (!new_style_bin && (*bin_size >= (1<<24))) {
    fatal_error("The output .bin would be larger than 16Mb. Recompress this dictionary with --v2.\n");
}
/* An then we allocate a byte array of the correct size */
unsigned char* bin=(unsigned char*)malloc((*bin_size)*sizeof(unsigned char));
if (bin==NULL) {
   fatal_alloc_error("create_and_save_bin");
}
for (int i=0;i<*bin_size;i++) bin[i]=0xF1;
int pos=0;
if (!new_style_bin) {
    bin_write_4bytes(bin,*bin_size,&pos);
} else {
    write_new_bin_header(bin_type,bin,&pos,state_encoding,char_encoding,inf_number_encoding,offset_encoding,root->offset);
}
/* Then we fill the 'bin' array */
fill_bin_array(root,n_states,n_transitions,bin,inf_indirection,
        state_encoding,char_write_function,inf_number_write_function,offset_write_function,bin_type);
/* And we dump it to the output file */
if (fwrite(bin,1,(*bin_size),f)!=(unsigned)(*bin_size)) {
  fatal_error("Error while writing file %s\n",output);
}
if (new_style_bin) {
    /* Adding 4 null bytes to allow an optimization in bin_read_variable_length */
    uint32_t i=0;
    if (fwrite(&i,4,1,f)!=1) {
        fatal_error("Error while writing file %s\n",output);
    }
}
u_fclose(f);
free(bin);
}

} // namespace unitex
