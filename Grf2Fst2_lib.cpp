 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#include "Grf2Fst2_lib.h"
#include "FIFO.h"
#include "Error.h"
#include "File.h"
#include "Transitions.h"
#include "SingleGraph.h"


/* Maximum length for the content of a grf box */
#define MAX_GRF_BOX_CONTENT 10000

/* Maximum number of token in a box line */
#define MAX_TOKENS_IN_A_SEQUENCE 4096

/* Maximum number of graphs in a grammar */
#define MAX_NUMBER_OF_GRAPHS 10000

#define NON_PROCESSED_GRAPH 0
#define EMPTY_GRAPH 1
#define NON_EMPTY_GRAPH 2



/**
 * Allocates, initializes and returns a compilation information structure.
 */
struct compilation_info* new_compilation_info() {
struct compilation_info* infos=(struct compilation_info*)malloc(sizeof(struct compilation_info));
if (infos==NULL) {
   fatal_alloc_error("new_compilation_info");
}
infos->main_graph_path[0]='\0';
infos->repository[0]='\0';
infos->graph_names=new_string_hash(256);
/* As the graph numbers start at 1, we insert the empty string at position 0 */
get_value_index(U_EMPTY,infos->graph_names);
infos->tags=new_string_hash(256);
/* We insert <E> in the tags in order to ensure that its number is 0 */
get_value_index(EPSILON,infos->tags);
infos->nombre_graphes_comp=0;
infos->tokenization_policy=DEFAULT_TOKENIZATION;
infos->alphabet=NULL;
infos->fst2=NULL;
infos->no_empty_graph_warning=0;
infos->CONTEXT_COUNTER=0;
infos->encoding_output = DEFAULT_ENCODING_OUTPUT;
infos->bom_output = DEFAULT_BOM_OUTPUT;
infos->mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;

return infos;
}


/**
 * Frees the memory associate to the given information.
 */
void free_compilation_info(struct compilation_info* infos) {
if (infos==NULL) return;
free_string_hash(infos->graph_names);
free_string_hash(infos->tags);
free(infos);
}


/**
 * Returns 1 if the given character is a letter, according to the
 * tokenization policy; 0 otherwise.
 */
int is_letter_generic(unichar c,struct compilation_info* infos) {
switch(infos->tokenization_policy) {
case DEFAULT_TOKENIZATION: return u_is_letter(c);
case CHAR_BY_CHAR_TOKENIZATION: return 0; /* By convention */
case WORD_BY_WORD_TOKENIZATION: return is_letter(c,infos->alphabet);
default: fatal_error("Internal error in is_letter_generic\n");
}
return 0;
}


/**
 * Writes the given state into the given file.
 */
void write_state(U_FILE* f,SingleGraphState s) {
if (is_final_state(s)) u_fputc('t',f);
else u_fputc(':',f);
Transition* ptr=s->outgoing_transitions;
while (ptr!=NULL) {
   u_fprintf(f," %d %d",ptr->tag_number,ptr->state_number);
   ptr=ptr->next;
}
u_fprintf(f," \n");
}


/**
 * Writes the states and transitions of of the given graph #n into the given file.
 */
void write_graph(U_FILE* f,SingleGraph graph,int n,unichar* name) {
u_fprintf(f,"%d %S\n",n,name);
/* By convention, the empty automaton is represented by an initial state with no
 * transition */
if (graph->number_of_states==0) u_fprintf(f, ": \n");
/* Otherwisen we print all the states */
for (int i=0;i<graph->number_of_states;i++) {
   if (graph->states[i]==NULL) {
      fatal_error("NULL state error in write_graph\n");
   }
   write_state(f,graph->states[i]);
}
/* We mark the end of the graph */
u_fprintf(f,"f \n");
}


/**
 * Creates a new state and relies it to the origin state.
 */
void create_intermediate_state(SingleGraph graph,int origin_state,int tag_number) {
add_state(graph);
int dest_state=graph->number_of_states-1;
add_outgoing_transition(graph->states[origin_state],tag_number,dest_state);
}


#ifndef _NOT_UNDER_WINDOWS
/**
 * This function returns 1 if the given file name is an absolute
 * Windows-style one like "C:\tmp\foo.grf" or "C::tmp:foo.grf"
 */
int test4abs_windows_path_name(unichar* name) {
if (((name[0] >= 'A' && name[0] <= 'Z') ||
     (name[0] >= 'a' && name[0] <= 'z'))
    && (name[1] == ':') && ((name[2] == '\\') || (name[2] == ':'))) {
   return 1;
}
return 0;
}
#endif


/**
 * Computes the absolute path of the graph #n, taking into account references
 * to the graph repository, if any.
 */
void get_absolute_name(char* name,int n,struct compilation_info* infos) {
unichar temp[FILENAME_MAX];
int offset;
int abs_path_name_warning=0; // 1 windows, 2 unix
temp[0]='\0'; // necessary if we have an absolute path name
if (infos->graph_names->value[n][0]==':') {
   /* If the graph is located in the repository, then we must test if
    * the repository is defined. If not, an absolute path is tried
    * starting with '/' resp. '\\'. This enables absolute path names
    * under Unixes. */
   u_strcpy(temp,infos->repository);
   offset=(int)strlen(infos->repository);
   if (infos->repository[0]=='\0') {
      abs_path_name_warning=2;
   }
}
#ifndef _NOT_UNDER_WINDOWS
else if (test4abs_windows_path_name(infos->graph_names->value[n])) {
   /* We have an absolute windows path name ("C:\" but now "C::" ), so we will
    * start ':' replacements after the first ':' of "C:\...", since we want
    * "C:\path\graph" and not "C\\path\graph" */
   offset=2;
   abs_path_name_warning=1;
}
#endif
else {
   /* If the graph path is relative to its calling graph, then we have just
    * to concatenate it with the main graph path. For instance, if the main
    * graph is "/tmp/ABC.grf" and if the sugraph call is "my_dir/DEF", then
    * the absolute path will be "/tmp/my_dir/DEF.grf" */
   u_strcpy(temp,infos->main_graph_path);
   offset=u_strlen(infos->main_graph_path);
}
u_strcat(temp,infos->graph_names->value[n]);
u_strcat(temp,".grf");
/* Finally, we turn the file name into ISO-8859-1 */
u_to_char(name,temp);
if (abs_path_name_warning!=0) {
   error("Absolute path name detected (%s):\n"
         "%s\n"
         "Absolute path names are not portable!\n",
         ((abs_path_name_warning==1) ? "Windows" :
         "Unix: forgot to specify repository by -d <dir>?"),
         name);
}
/* Finally, we turn all the ':' into the system separator ('/' or '\'),
 * but we must ignore the ':' in "C:\...", so we start the
 * replacement after a shift offset */
name=name+offset;
replace_colon_by_path_separator(name);
}


/**
 * Takes a grf box content and splits it into an input and an output.
 * Note that the output can be empty.
 */
void split_input_output(unichar* box_content,unichar* input,unichar* output) {
int i=0;
while (box_content[i]!='\0' && box_content[i]!='/') {
   if (box_content[i]=='\\') {
      /* If we have a backslash with no backslash before */
      if (box_content[i+1]=='"' && (i==0 || (i>0 && box_content[i-1]!='\\'))) {
         /* If we have a \" we look for the next one */
         i=i+2;
         while (box_content[i]!='\0' && box_content[i]!='"') {
            if (box_content[i]=='\\' && box_content[i+1]!='"') i++;
            i++;
         }
      } else {
         /* If we have a normal backslash, we must jump after the protected character */
         i=i+2;
      }
   } else {
      i++;
   }
}
u_strcpy(input,box_content);
output[0]='\0';
/* If there is no output, we can return */
if (box_content[i]=='\0') {
   return;
}
/* Otherwise, we set the end of the input and we ignore the / */
input[i]='\0';
i++;
int j=0;
do {
   if (box_content[i]=='\\') {
      /* We may have to unprotect some characters */
      i++;
      if (box_content[i]=='\0') {
        fatal_error("Unexpected backslash at end of line\n");
      }
   }
   output[j] = box_content[i];
   j++;
} while (box_content[i++]!='\0');
}


/**
 * Gets one character from the input.
 */
void get_character(unichar* input,int *pos,unichar* dest) {
dest[0]=input[*pos];
dest[1]='\0';
(*pos)++;
}


/**
 * Gets a word from the input according to the default tokenization.
 */
void get_default_tokenization_word(unichar* input,int *pos,unichar* dest) {
int i=0;
do {
   dest[i++]=input[(*pos)++];
} while (u_is_letter(input[(*pos)-1]) && i<MAX_GRF_BOX_CONTENT);
if (i==MAX_GRF_BOX_CONTENT) {
   fatal_error("Word too long in get_default_tokenization_word\n");
}
dest[i-1]='\0';
(*pos)--;
}


/**
 * Gets a word from the input according to an alphabet tokenization.
 */
void get_alphabet_tokenization_word(unichar* input,int *pos,unichar* dest,Alphabet* alph) {
int i=0;
do {
   dest[i++]=input[(*pos)++];
} while (is_letter(input[(*pos)-1],alph) && i<MAX_GRF_BOX_CONTENT);
if (i==MAX_GRF_BOX_CONTENT) {
   fatal_error("Word too long in get_alphabet_tokenization_word\n");
}
dest[i-1]='\0';
(*pos)--;
}


/**
 * Gets a letter sequence from the input.
 */
void get_letter_sequence(unichar* input,int *pos,unichar* dest,struct compilation_info* infos) {
switch(infos->tokenization_policy) {
case DEFAULT_TOKENIZATION: get_default_tokenization_word(input,pos,dest); return;
case CHAR_BY_CHAR_TOKENIZATION: get_character(input,pos,dest); return;
case WORD_BY_WORD_TOKENIZATION: get_alphabet_tokenization_word(input,pos,dest,infos->alphabet); return;
default: fatal_error("Internal error in get_letter_sequence\n");
}
}


/**
 * Gets an angle bracket sequence from the input.
 */
void get_angle_bracket_sequence(unichar* input,int *pos,unichar* dest) {
int i=0;
do {
   dest[i++]=input[(*pos)++];
   if (dest[i-1]=='\\') {
      if (input[(*pos)]=='\0') {
         fatal_error("Backslash at end of <... sequence: %S\n",input);
      }
      dest[i++]=input[(*pos)++];
   }
} while (input[*pos]!='>' && input[*pos]!='\0' && i<MAX_GRF_BOX_CONTENT);
if (i==MAX_GRF_BOX_CONTENT) {
   fatal_error("Angle bracket sequence too long in get_angle_bracket_sequence\n");
}
if (input[*pos]=='\0') {
   fatal_error("Missing > at the end of an angle bracket sequence\n");
}
dest[i]='>';
/* Now, we will deal with the morphological filter, if any */
if (input[(*pos)+1] == '>') {
   /* If the <...> sequence was in fact a morphological filter, we
    * catch the second > */
   dest[++i]=input[++(*pos)];
}
else if (input[(*pos)+1]=='<' && input[(*pos)+2]=='<') {
   /* Otherwise, if we have a morphological filter */
   do {
      dest[++i]=input[++(*pos)];
   } while (input[*pos]!='>' && input[*pos]!='\0');
   if (input[*pos]=='\0' || input[(*pos)+1]!='>') {
      /* If we don't have a second > after the first > */
      fatal_error("Missing > at the end of a morphological filter\n");
   }
   dest[++i]=input[++(*pos)];
} else {
   /* If there was no morphological filter, we return */
   dest[i+1]='\0';
   (*pos)++;
   return;
}
/* If there is a '_' after a morphological filter, we catch the _x_ sequence */
if (input[(*pos)+1]=='_') {
   dest[++i]=input[++(*pos)];
   do {
      dest[++i]=input[++(*pos)];
   } while (input[*pos]!='_' && input[*pos]!='\0');
   if (input[*pos]=='\0') {
      fatal_error("Missing _ at the end of a morphological filter\n");
   }
}
dest[i+1]='\0';
(*pos)++;
}


/**
 * Gets a round bracket sequence from the input.
 */
void get_round_bracket_sequence(unichar* input,int *pos,unichar* dest) {
int i=0;
do {
   dest[i++]=input[(*pos)++];
} while ((input[*pos]!='}' || input[(*pos)-1]=='\\') && input[*pos]!='\0' && i<MAX_GRF_BOX_CONTENT);
if (i==MAX_GRF_BOX_CONTENT) {
   fatal_error("Round bracket sequence too long in get_round_bracket_sequence\n");
}
if (input[*pos]=='\0') {
   fatal_error("Missing } in round bracket sequence\n");
}
dest[i] = '}';
dest[i+1] = '\0';
if (!u_strcmp(dest,"{STOP}")) {
   /* If the graph contains the forbidden tag {STOP} */
   fatal_error("ERROR: a graph contains the forbidden tag {STOP}\n");
}
(*pos)++;
}


/**
 * This function assumes that the current position in the input is
 * inside a double quoted sequence and it gets a token. All tokens
 * that will be read here will be prefixed with '@' in order to
 * indicate that they don't tolerate case variations.
 * Returns 1 if the end of the double quoted sequence is found; 0 otherwise.
 */
int get_double_quoted_token(unichar* input,int *pos,unichar* dest,struct compilation_info* infos) {
dest[0]='@';
/* If we have a backslash */
if (input[*pos]=='\\') {
   (*pos)++;
   if (input[(*pos)]=='\\' && input[(*pos)+1]=='\\' && input[(*pos)+2]=='"') {
      /* If we have \\\" we must return the " character */
      (*pos)=(*pos)+3;
      u_strcpy(dest,"@\"");
      return 0;
   }
   if (input[*pos] == '"') {
      /* If we have \" in the grf, it means that we have the final " of the sequence */
      (*pos)++;
      return 1;
   }
   /* If we have \x we must get the x character */
   get_character(input,pos,&(dest[1]));
   return 0;
}
/* If we have a letter */
if (is_letter_generic(input[*pos],infos)) {
   get_letter_sequence(input,pos,&(dest[1]),infos);
   return 0;
}
/* If we have a non letter character */
get_character(input,pos,&(dest[1]));
return 0;
}


/**
 * This function reads a sequence between double quotes from  the input.
 */
void get_double_quoted_sequence(unichar* input,int *pos,
                                struct fifo* tokens,
                                struct compilation_info* infos) {
(*pos)++;
unichar tmp[MAX_GRF_BOX_CONTENT];
while (!get_double_quoted_token(input,pos,tmp,infos) && input[*pos]!='\0') {
   put_ptr(tokens,u_strdup(tmp));
}
}


/**
 * This function considers the given input from the given position
 * and copies everything to 'dest' until it finds a '+' or the end of string.
 * All '/' and '\' are turned into ':'.
 */
void get_subgraph_call(unichar* input,int *pos,unichar* dest) {
int i=0;
while (input[*pos]!='+' && input[*pos]!='\0' && i<MAX_GRF_BOX_CONTENT) {
   dest[i]=input[*pos];
   if (dest[i]=='/' || dest[i]=='\\') {
      dest[i]=':';
   }
   i++;
   (*pos)++;
}
if (i==MAX_GRF_BOX_CONTENT) {
   fatal_error("Graph call too long in get_subgraph_call\n");
}
dest[i]='\0';
if (u_ends_with(dest,".grf")) {
   dest[i-4]='\0';
}
}


/**
 * Takes the given input and tries to read a token from '*pos'.
 * Returns 1 if a '+' was found after the token that has been read; 0 otherwise.
 */
int process_box_line_token(unichar* input,int *pos,
                           struct fifo* tokens,
                           int n,struct compilation_info* infos) {
if (input[*pos]=='\0') {
   fatal_error("Empty string in process_box_line_token\n");
}
unichar token[MAX_GRF_BOX_CONTENT];
if (input[*pos]==':') {
   /* If we have a subgraph call */
   token[0]=':';
   int l;
   if (input[(*pos)+1]!= ':') {
      /* If the subgraph is not in the repository, we copy the path
       * of the current graph */
      u_strcpy(&(token[1]),infos->graph_names->value[n]);
      /* And we remove the current graph name */
      l=u_strlen(token);
      while (token[l]!=':') {
         l--;
      }
   }
   #ifndef _NOT_UNDER_WINDOWS
   else if (test4abs_windows_path_name(&(input[*pos]))) {
      /* If the subgraph has an absolute path */
      l=0;
   }
   #endif
   else {
      /* If the subgraph is in the repository */
      l=0;
      /* We keep the (second) colon in front of the graph name,
       * i.e. we don't change the name. This indicates that the path of
       * the main graph won't be prefixed in get_absolute_name. */
   }
   /* Now that we have the path prefix, we can add the subgraph call */
   get_subgraph_call(input,pos,&(token[l]));
   put_ptr(tokens,u_strdup(token));
   /* We add this graph name to the graph names, if not already present */
   get_value_index(token+1,infos->graph_names);
   return 0;
}
/* If we have found a '+' */
if (input[*pos]=='+') {
   (*pos)++;
   return 1;
}
/* If we have a space, we skip it */
if (input[*pos]==' ') {
   (*pos)++;
   return 0;
}
/* If we have a backslask */
if (input[*pos]=='\\') {
   (*pos)++;
   if (input[*pos]!='\\') {
      if (input[*pos] == '"') {
         /* If we have a sequence between double quotes */
         get_double_quoted_sequence(input,pos,tokens,infos);
         return 0;
      }
      /* If we have \x where x is not a \ neither a " */
      get_character(input,pos,token);
      put_ptr(tokens,u_strdup(token));
      return 0;
   }
   /* If we have \\ in the box input */
   if (input[(*pos)+1] != '\\') {
      /* If we have just a character to unprotect */
      get_character(input,pos,token);
      put_ptr(tokens,u_strdup(token));
      return 0;
   }
   /* If we have \\\" it means that we want to print a " */
   if (input[(*pos)+2] == '"') {
      (*pos)=(*pos)+2;
      get_character(input,pos,token);
      put_ptr(tokens,u_strdup(token));
      return 0;
   }
   /* If we have \\\x in means that we want to print \x */
   get_character(input,pos,token);
   put_ptr(tokens,u_strdup(token));
   return 0;
}
/* If we have a round bracket */
if (input[*pos]=='{') {
   get_round_bracket_sequence(input,pos,token);
   put_ptr(tokens,u_strdup(token));
   return 0;
}
/* If we have an angle bracket */
if (input[*pos]=='<') {
   get_angle_bracket_sequence(input,pos,token);
   put_ptr(tokens,u_strdup(token));
   return 0;
}
/* If we have a letter */
if (is_letter_generic(input[*pos],infos)) {
   get_letter_sequence(input,pos,token,infos);
   put_ptr(tokens,u_strdup(token));
   return 0;
}
/* If we have a character that is not a letter */
get_character(input,pos,token);
put_ptr(tokens,u_strdup(token));
return 0;
}


/**
 * Takes a token sequence and turns it into an integer sequence.
 */
void token_sequence_2_integer_sequence(struct fifo* u_tokens,unichar* output,
                                int* i_tokens,struct compilation_info* infos,
                                int *n_tokens) {
if (u_tokens==NULL) {
   fatal_error("NULL error in token_sequence_2_integer_sequence\n");
}
if (is_empty(u_tokens)) {
   fatal_error("Empty FIFO in token_sequence_2_integer_sequence\n");
}
*n_tokens=0;
unichar* token=(unichar*)take_ptr(u_tokens);
unichar tmp[MAX_GRF_BOX_CONTENT];
int is_an_output=(output!=NULL && output[0]!='\0');
if (token[0]==':' && token[1]!='\0') {
   /* If we have a subgraph call */
   if (is_an_output) {
      error("WARNING: ignoring output associated to subgraph call %S\n",token);
   }
   if (!is_empty(u_tokens)) {
      fatal_error("Unexpected token after subgraph call in token_sequence_2_integer_sequence\n");
   }
   i_tokens[(*n_tokens)++]=-get_value_index(&(token[1]),infos->graph_names);
   free(token);
   return;
}
if (is_an_output) {
   /* If there is an output, we associate it to the first token */
   u_sprintf(tmp,"%S/%S",token,output);
   i_tokens[(*n_tokens)++]=get_value_index(tmp,infos->tags);
} else {
   i_tokens[(*n_tokens)++]=get_value_index(token,infos->tags);
}
free(token);

/* Then, we process the rest of the tokens */
while (!is_empty(u_tokens)) {
   token=(unichar*)take_ptr(u_tokens);
   if (token[0]==':' && token[1]!='\0') {
      fatal_error("Unexpected subgraph call in token_sequence_2_integer_sequence\n");
   }
   u_sprintf(tmp,"%S",token);
   i_tokens[(*n_tokens)++]=get_value_index(tmp,infos->tags);
   free(token);
}
}


/**
 * This function takes a tag number sequence and a list of reachable states
 * and it creates pathes from the current state to each of the reachable states,
 * introducing at new intermediate states as needed.
 */
void write_transitions(SingleGraph graph,int* tag_numbers,struct list_int* transitions,
                      int current_state,int n_tag_numbers) {
int tmp_state;
while (transitions!=NULL) {
   tmp_state=current_state;
   for (int j=0;j<n_tag_numbers-1;j++) {
      /* If we are not on the last tag number, then we must introduce a new
       * intermediate state */
      create_intermediate_state(graph,tmp_state,tag_numbers[j]);
      tmp_state=graph->number_of_states-1;
   }
   /* Finally, we rely the state we are to the reachable state */
   add_outgoing_transition(graph->states[tmp_state],tag_numbers[n_tag_numbers-1],transitions->n);
   /* And we go again with the next reachable state */
   transitions=transitions->next;
}
}


/**
 * This function takes the input and the output that correspond to one box in the graph #n
 * and it adds the necessary states and transitions to
 * the given SingleGraph. '*pos' represents the current position in the input string.
 * For instance, if the current state corresponds to the box
 * content "abc+d e f/foo", this function will be called twice: one with
 * "abc+d e f/foo" and one with "d e f/foo" (more exactly, it's '*pos' that will
 * be 0 for the first call and 4 for the second call).
 */
void process_box_line(SingleGraph graph,unichar* input,unichar* output,struct list_int* transitions,
                     int *pos,int state,int n,struct compilation_info* infos) {
int result=0;
struct fifo* sequence=new_fifo();
while (result==0 && input[*pos]!='\0') {
   result=process_box_line_token(input,pos,sequence,n,infos);
}
int sequence_ent[MAX_TOKENS_IN_A_SEQUENCE];
int n_tokens;
token_sequence_2_integer_sequence(sequence,output,sequence_ent,infos,&n_tokens);
free_fifo(sequence);
write_transitions(graph,sequence_ent,transitions,state,n_tokens);
}


/**
 * If we have a variable or context mark, we store it in the tags, if not already
 * present, and we write the corresponding transitions.
 */
void process_variable_or_context(SingleGraph graph,unichar* input,
                                struct list_int* transitions,
                                int state,struct compilation_info* infos) {
struct fifo* tmp=new_fifo();
put_ptr(tmp,u_strdup(input));
int token[1];
int i;
token_sequence_2_integer_sequence(tmp,NULL,token,infos,&i);
free_fifo(tmp);
write_transitions(graph,token,transitions,state,1);
}


/**
 * Processes the given grf state of the graph #n.
 */
void process_grf_state(unichar* box_content,struct list_int* transitions,
                      SingleGraph graph,int current_state,
                      int n,struct compilation_info* infos) {
unichar input[MAX_GRF_BOX_CONTENT];
unichar output[MAX_GRF_BOX_CONTENT];
if (transitions==NULL) {
   /* If the state has not outoing transition, it will be discarded when the
    * graph is cleaned, so it's not necessary to process it. */
   return;
}
int length=u_strlen(box_content);
if ((length>2 && box_content[0]=='$' &&
       (box_content[length-1]=='('
        || box_content[length-1]==')'))
        || !u_strcmp(box_content,"$[")
        || !u_strcmp(box_content,"$![")
        || !u_strcmp(box_content,"$]")
        || !u_strcmp(box_content,"$<")
        || !u_strcmp(box_content,"$>")
        || !u_strcmp(box_content,"$*")) {
   /* If we have a variable or context mark, or a morphological mode delimiter */
   u_strcpy(input,box_content);
   if (box_content[1]=='!' || box_content[1]=='[') {
      /* If we have a context start mark, we adds a unique number to it
       * (see declaration of CONTEXT_COUNTER) */
      u_sprintf(input,"%s%d",(box_content[1]=='!')?"$![":"$[",(infos->CONTEXT_COUNTER)++);
   }
   u_strcpy(output,"");
   process_variable_or_context(graph,input,transitions,current_state,infos);
   return;
}
/* Otherwise, we deal with the output of the box, if any */
split_input_output(box_content,input,output);
/* And we process the box input */
int pos=0;
while (input[pos]!='\0') {
   process_box_line(graph,input,output,transitions,&pos,current_state,n,infos);
}
}


/**
 * Reads one line of the graph #n. The box content is stored into 'box_content' and
 * the outgoing transitions are stored into 'transitions'.
 * It returns 0 if the box is too large; 1 otherwise.
 */
int read_grf_line(U_FILE* f,unichar* box_content,struct list_int* *transitions,int n,struct compilation_info* infos) {
*transitions=NULL;
unichar c;
int n_sortantes;
int i=0;
/* We skip chars until we have read the '"' that starts the line */
while (u_fgetc(f)!='"') {}
/* Then we copy the box content, escaping the '"'found in it */
while (((c=(unichar)u_fgetc(f))!='"') && (i<MAX_GRF_BOX_CONTENT)) {
   box_content[i]=c;
   if ((box_content[i]=='\\') && (i<MAX_GRF_BOX_CONTENT)) {
	   i++;
	   box_content[i]=(unichar)u_fgetc(f);
	}
   i++;
}
/* If the box content is too long */
if (i>=MAX_GRF_BOX_CONTENT) {
   error("ERROR in graph %S.grf:\n"
         "Too many characters in box. The number of characters\n"
         "per box should be lower than %d\n",
         infos->graph_names->value[n],MAX_GRF_BOX_CONTENT);
   return 0;
}
box_content[i]='\0';
/* 3 %d because we skip the X and Y coordinates and then we read the number
 * of outgoing transitions */
u_fscanf(f,"%d%d%d",&n_sortantes,&n_sortantes,&n_sortantes);
/* Now, we read the transitions */
int j;
for (i=0;i<n_sortantes;i++) {
   u_fscanf(f,"%d",&j);
   *transitions=head_insert(j,*transitions);
}
/* Finally, we read the end of line character */
u_fgetc(f);
return 1;
}


/**
 * This function compiles the graph number #n and saves its states into the
 * output .fst2.
 */
int compile_grf(int n,struct compilation_info* infos) {
int i;
int n_states;
char name[FILENAME_MAX];
struct list_int* transitions;
unichar ligne[MAX_GRF_BOX_CONTENT];
SingleGraph graph=new_SingleGraph();
u_printf("Compiling graph %S\n",infos->graph_names->value[n]);
/* We get the absolute path of the graph */
get_absolute_name(name,n,infos);
U_FILE* f=u_fopen_existing_versatile_encoding(infos->mask_encoding_compatibility_input,name,U_READ);
if (f==NULL) {
   error("Cannot open the graph %S.grf\n(%s)\n",infos->graph_names->value[n],name);
   write_graph(infos->fst2,graph,-n,infos->graph_names->value[n]);
   if (n==0) return 0;
   return 1;
}
/* If we can open the .grf file, we start with skipping the header. We skip
 * the first '#' and then we look for the second. */
u_fgetc(f);
int c;
while ((c=u_fgetc(f))!=EOF && c!='#') {}
if (c==EOF) {
   error("Invalid graph %S.grf\n(%s)\n",infos->graph_names->value[n],name);
   write_graph(infos->fst2,graph,-n,infos->graph_names->value[n]);
   if (n==0) return 0;
   return 1;
}
/* Skip the newline and the number of states */
u_fscanf(f,"%d\n",&n_states);
/* If necessary, we resize the graph that it can hold all the states */
if (graph->capacity<n_states) {
   set_state_array_capacity(graph,n_states);
}
/* Now, every line represents a state of the automaton */
for (i=0;i<n_states;i++) {
   graph->states[i]=new_SingleGraphState();
}
graph->number_of_states=n_states;
for (i=0;i<n_states;i++) {
   /* We read one line and we process it */
   int result=read_grf_line(f,ligne,&transitions,n,infos);
   if (result==0) {
      /* In case of error, we dump the graph and return */
      write_graph(infos->fst2,graph,-n,infos->graph_names->value[n]);
      free_SingleGraph(graph);
      u_fclose(f);
      if (n==1) return 0;
      return 1;
   }
   /* We process the box */
   process_grf_state(ligne,transitions,graph,i,n,infos);
   free_list_int(transitions);
}
u_fclose(f);
/* Once we have loaded the graph, we process it. */
set_initial_state(graph->states[0]);
set_final_state(graph->states[1]);
compute_reverse_transitions(graph);
check_co_accessibility(graph->states,1);
remove_epsilon_transitions(graph,1);
check_accessibility(graph->states,0);
remove_useless_states(graph);
if (graph->states[0]==NULL) {
   /* If the graph has been emptied */
   write_graph(infos->fst2,graph,-n,infos->graph_names->value[n]);
   free_SingleGraph(graph);
   if (infos->no_empty_graph_warning) return 1;
   if (n==0) {
      error("ERROR: Main graph %S.grf has been emptied\n",infos->graph_names->value[n]);
      return 0;
   }
   error("WARNING: graph %S.grf has been emptied\n",infos->graph_names->value[n]);
   return 1;
}
/* Now, we minimize the automaton assuming that reversed transitions are still there */
minimize(graph,0);
/* And we save it */
write_graph(infos->fst2,graph,-n,infos->graph_names->value[n]);
free_SingleGraph(graph);
return 1;
}


/**
 * This function takes the main graph name as given to the program and
 * computes its path and its name without path and extension. Then, the
 * name without path and extension is added to the graph names string_hash.
 *
 * Examples:
 *
 *    "E:\Unitex\French\date.grf" => "E:\Unitex\French\" and "date"
 *    "/tmp/foo.grf" => "/tmp/" and "foo"
 *
 */
void extract_path_and_main_graph(char* main_graph,struct compilation_info* infos) {
char temp[FILENAME_MAX];
char temp2[FILENAME_MAX];
unichar temp3[FILENAME_MAX];
/* First, we copy the main graph path */
get_path(main_graph,temp);
u_strcpy(infos->main_graph_path,temp);
/* Then, we get the name of the main graph without its path and extension */
remove_path(main_graph,temp);
remove_extension(temp,temp2);
u_strcpy(temp3,temp2);
/* And we insert it into the graph names string_hash */
get_value_index(temp3,infos->graph_names);
}


/**
 * This is the main function that takes a main graph and compiles it.
 * It returns 1 in case of success; 0 otherwise.
 */
int compile_grf(char* main_graph,struct compilation_info* infos) {
int current_graph=1;
int result;
extract_path_and_main_graph(main_graph,infos);
do {
   result=compile_grf(current_graph,infos);
   if (result==0 && current_graph==1) {
      /* If the main graph has been emptied, then the compilation has failed */
      return 0;
   }
   if (result==-1) return 0;
   current_graph++;
} while (current_graph<infos->graph_names->size);
return 1;
}


/**
 * Prints the final number of graphs at the beginning of the .fst2 file.
 * This is safer to use this function instead of a raw fseek, since the fseek
 * must be at +2 if and only if we are in UTF16. If the encoding changes, the
 * fseek may also change (for instance, we would have +0 for UTF8). So, it is
 * safer to let the U_MODIFY mode do the job.
 */
void write_number_of_graphs(char* name,int n) {
U_FILE* f=u_fopen_existing_unitex_text_format(name,U_MODIFY);
/* And we print the number of graphs on 10 digits */
u_fprintf(f,"%010d",n);
u_fclose(f);
}


/**
 * Dumps into the given fst2 the tags contained in the given string_hash.
 */
void write_tags(U_FILE* fst2,struct string_hash* tags) {
int n_tags=tags->size;
for (int i=0;i<n_tags;i++) {
   if (tags->value[i][0]=='@') {
      /* During the construction of the tags, only strict-case tags are
       * prefixed with '@'. Tags that tolerate case variations are not
       * prefixed with '%' */
      if (tags->value[i][1]=='\0') {
         u_fprintf(fst2,"%%@\n");
      } else {
         u_fprintf(fst2,"%S\n",tags->value[i]);
      }
   } else {
      u_fprintf(fst2,"%%%S\n",tags->value[i]);
   }
}
u_fprintf(fst2,"f\n");
}



