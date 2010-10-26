/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


/**
 * This function associates an offset to the given dictionary node and updates
 * the 'bin_size' value according to the number of bytes taken by this node
 * and its transitions. Then, the function is called recursively on all the
 * nodes that are children of 'node'.
 * If the node has already been numbered (we are in an automaton,
 * not a tree), the function does nothing.
 */
void number_node(struct dictionary_node* node,int *bin_size) {
if (node==NULL) return;
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
	(*bin_size)=(*bin_size)+3;
}
/* Then we count the transitions */
node->n_trans=0;
struct dictionary_node_transition* tmp;
tmp=node->trans;
while (tmp!=NULL) {
	/* For each transition, we count 2 bytes for the letter and 3 bytes for the
	 * destination offset */
  	(*bin_size)=(*bin_size)+5;
  	tmp=tmp->next;
  	/* We update the number of transitions */
	(node->n_trans)++;
}
/* Finally, we number all the children of the node */
tmp=node->trans;
while (tmp!=NULL) {
  number_node(tmp->node,bin_size);
  tmp=tmp->next;
}
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
						unsigned char* bin,int* inf_indirection) {
if (node==NULL) return;
if (node->INF_code==-1) {
	/* We use this test to know if the node has already been dumped */
   return;
}
/* We increase the number of states */
(*n_states)++;

/* We cast the number of transitions into an unsigned number on 2 bytes */
unichar n=(unichar)node->n_trans;
if (node->single_INF_code_list==NULL) {
   /* If the node is not a final one, we put to 1 the the heaviest bit */
   n=(unichar)(n|32768);
}

/* We take the offset of the node as a base */
int pos=node->offset;

/* We write the 2 bytes information about the node. We force the byte order
 * (higher byte first) to avoid little/big endian surprises */
bin[pos]=(unsigned char)(n/256);
bin[pos+1]=(unsigned char)(n%256);
pos=pos+2;
if (node->single_INF_code_list!=NULL) {
	/* If the node is a final one, we dump the number of the associated
	 * INF line on 3 bytes, forcing the order (higher byte first) */
	int INF_line_number=inf_indirection[node->INF_code];
	if (INF_line_number==-1) {
		fatal_error("fill_bin_array: Invalid INF line number redirection for code #%d\n",node->INF_code);
	}
	bin[pos++]=(unsigned char)(INF_line_number/(256*256));
	INF_line_number=INF_line_number%(256*256);
	bin[pos++]=(unsigned char)(INF_line_number/256);
	INF_line_number=INF_line_number%256;
	bin[pos++]=(unsigned char)(INF_line_number);
}
/* Then, we assign -1 to 'node->INF_code' in order to mark that the node
 * has been dumped */
node->INF_code=-1;
/* And we process all the transitions of the node */
struct dictionary_node_transition* tmp;
tmp=node->trans;
while (tmp!=NULL) {
	/* We increase the number of transitions */
	(*n_transitions)++;
	/* We dump the letter of the transition on 2 bytes, forcing the order (higher byte first) */
	bin[pos++]=(unsigned char)((tmp->letter)/256);
	bin[pos++]=(unsigned char)((tmp->letter)%256);
	/* We dump the offset of the destination node on 3 bytes, forcing the order (higher byte first) */
	int dest_offset=tmp->node->offset;
	bin[pos++]=(unsigned char)(dest_offset/(256*256));
	dest_offset=dest_offset%(256*256);
	bin[pos++]=(unsigned char)(dest_offset/256);
	dest_offset=dest_offset%256;
	bin[pos++]=(unsigned char)(dest_offset);
	/* And we dump the destination node recursively */
	fill_bin_array(tmp->node,n_states,n_transitions,bin,inf_indirection);
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
						int *n_transitions,int *bin_size,int* inf_indirection) {
U_FILE* f;
/* The output file must be opened as a binary one */
f=u_fopen(BINARY,output,U_WRITE);
if (f==NULL) {
  fatal_error("Cannot write automaton file %s\n",output);
}
/* The .bin size is initialized to 4, because of the 4 first bytes that will be
 * used to store the size of the .bin */
(*bin_size)=4;
(*n_states)=0;
(*n_transitions)=0;

/* We give offsets to the dictionary node and we get the .bin size */
number_node(root,bin_size);
/* An then we allocate a byte array of the correct size */
unsigned char* bin=(unsigned char*)malloc((*bin_size)*sizeof(unsigned char));
if (bin==NULL) {
   fatal_alloc_error("create_and_save_bin");
}
int n=(*bin_size);
/* We save the .bin size on the 4 first bytes, forcing the byte order
 * (higher byte first) */
bin[0]=(unsigned char)(n/(256*256*256));
n=n%(256*256*256);
bin[1]=(unsigned char)(n/(256*256));
n=n%(256*256);
bin[2]=(unsigned char)(n/256);
n=n%256;
bin[3]=(unsigned char)(n);
/* Then we fill the 'bin' array */
fill_bin_array(root,n_states,n_transitions,bin,inf_indirection);
/* And we dump it to the output file */
if (fwrite(bin,1,(*bin_size),f)!=(unsigned)(*bin_size)) {
  fatal_error("Error while writing file %s\n",output);
}
u_fclose(f);
free(bin);
}
