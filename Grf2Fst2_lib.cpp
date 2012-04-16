/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "DebugMode.h"
#include "Grf_lib.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

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
infos->named_repositories=new_string_hash_ptr();
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
infos->vec.mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
infos->vec.encoding_output = DEFAULT_ENCODING_OUTPUT;
infos->vec.bom_output = DEFAULT_BOM_OUTPUT;
infos->verbose_name_grf=1;
infos->debug=0;
infos->renumber=new_vector_int();
/* We set a dummy #0 cell in order to avoid painful +/- 1 at each graph access */
vector_int_add(infos->renumber,0);
infos->part_of_precompiled_fst2=new_vector_int();
/* We set a dummy #0 cell in order to avoid painful +/- 1 at each graph access */
vector_int_add(infos->part_of_precompiled_fst2,0);
infos->current_saved_graph=0;
return infos;
}


/**
 * Frees the memory associate to the given information.
 */
void free_compilation_info(struct compilation_info* infos) {
if (infos==NULL) return;
free_string_hash_ptr(infos->named_repositories,free);
free_string_hash(infos->graph_names);
free_string_hash(infos->tags);
free_vector_int(infos->renumber);
free_vector_int(infos->part_of_precompiled_fst2);
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
void write_graph(U_FILE* f,SingleGraph graph,int n,unichar* name,char* full_name) {
u_fprintf(f,"%d %S",n,name);
if (full_name!=NULL) {
	/* In debug mode, we add the full file name, separated by char #1,
	 * but if the graph is empty, we don't print the full name */
	u_fprintf(f,"%C%s",1,(graph->number_of_states==0)?"":full_name);
}
u_fprintf(f,"\n");
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


static int is_variable_char(unichar c) {
return ((c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') || c=='_');
}


/**
 * Computes the absolute path of the graph #n, taking into account references
 * to the graph repository, if any.
 */
void get_absolute_name(int *called_from,char* name,int n,struct compilation_info* infos) {
unichar temp[FILENAME_MAX];
/* offset is the position where to start replacing ':' by '/' or '\' */
int offset;
int abs_path_name_warning=0; // 1 windows, 2 unix
temp[0]='\0'; // necessary if we have an absolute path name
int shift=0;
if (infos->graph_names->value[n][0]==':') {
	shift++;
	if (infos->graph_names->value[n][1]=='$') {
		/* If we have a graph call using a named repository */
		shift++;
		Ustring* foo=new_Ustring();
		for (unichar* s=infos->graph_names->value[n]+shift;(*s!=':' && *s!='/' && *s!='\\');s++,shift++) {
			if (!is_variable_char(*s)) {
				fatal_error("Invalid repository name in graph call: %S\n",infos->graph_names->value[n]);
			}
			u_strcat(foo,*s);
		}
		/* We skip the final file separator */
		shift++;
		int index=get_value_index(foo->str,infos->named_repositories,DONT_INSERT);
		if (index==-1) {
			fatal_error("Undefined repository name: %S\n",foo->str);
		}
		char* repository=(char*)(infos->named_repositories->value[index]);
		u_strcpy(temp,repository);
		free_Ustring(foo);
	} else {
		/* If the graph is located in the default repository, then we must test if
		 * the repository is defined. If not, an absolute path is tried
		 * starting with '/' resp. '\\'. This enables absolute path names
		 * under Unixes. */
		u_strcpy(temp,infos->repository);
		if (infos->repository[0]=='\0') {
			abs_path_name_warning=2;
		}
	}
	offset=(int)u_strlen(temp);
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
int l=u_strlen(temp)-1;
if (temp[l]!=':' && temp[l]!='/' && temp[l]!='\\') {
	u_strcat(temp,":");
}
u_strcat(temp,infos->graph_names->value[n]+shift);
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
replace_colon_by_path_separator(name+offset);
int pos=strlen(name)-1;
while (pos>=0 && name[pos]!=0x02) pos--;
if (called_from!=NULL) (*called_from)=-1;
if (pos==-1) return;
name[pos]='\0';
if (called_from!=NULL) sscanf(name+pos+1,"%d",called_from);
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
   Ustring* tmp=new_Ustring();
   u_sprintf(tmp,"%S%C%d",token+1,0x02,n);
   get_value_index(token+1,infos->graph_names,tmp->str);
   free_Ustring(tmp);
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
/* If we have a backslash */
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
      if (token[0]=='#') {
    	  /* Special case of \# that must be compiled as "#" */
    	  u_strcpy(token,"@#");
      }
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
                                int *n_tokens,int current_graph,int must_add_token_to_debug,
                                int must_keep_output) {
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
if (!must_keep_output && is_an_output && !u_strcmp(token,"<E>") && output[1]==DEBUG_INFO_COORD_MARK) {
	/* We don't want to produce debug outputs for <E> that had no actual
	 * outputs, in order not to introduce <E> loop mess, but we want to keep
	 * <E> associated to metas symbols $a( $< ... */
	is_an_output=0;
}
if (token[0]==':' && token[1]!='\0') {
   /* If we have a subgraph call */
	int graph_number=get_value_index(&(token[1]),infos->graph_names);
	if (infos->debug) {
		/* In debug mode, we generate two <E> tags: one before and
		 * one after the graph call */
		create_graph_call_debug_tag(tmp,output,graph_number,1);
		i_tokens[(*n_tokens)++]=get_value_index(tmp,infos->tags);
	}
	else if (is_an_output) {
      error("WARNING in %S: ignoring output associated to subgraph call %S\n",
            infos->graph_names->value[current_graph],token);
   }
   if (!is_empty(u_tokens)) {
      fatal_error("%S: unexpected token after subgraph call in token_sequence_2_integer_sequence\n",
            infos->graph_names->value[current_graph]);
   }
   i_tokens[(*n_tokens)++]=-graph_number;
   free(token);
	if (infos->debug) {
		/* In debug mode, we generate two <E> tags: one before and
		 * one after the graph call */
		create_graph_call_debug_tag(tmp,output,graph_number,0);
		i_tokens[(*n_tokens)++]=get_value_index(tmp,infos->tags);
	}
   return;
}
if (is_an_output) {
   /* If there is an output, we associate it to the first token */
   u_sprintf(tmp,"%S/%S",token,output);
   /* In debug mode, we have to add the input */
   if (infos->debug && must_add_token_to_debug) {
	   int start=0;
	   if (token[0]=='@' && token[1]!='\0') {
		   /* double-quoted token are prefixed by @, so we have to remove this
		    * @ in the debug output */
		   start++;
	   }
	   u_strcat(tmp,token+start);
	   int size=u_strlen(tmp);
	   tmp[size]=DEBUG_INFO_END_MARK;
	   tmp[size+1]='\0';
   }
   i_tokens[(*n_tokens)++]=get_value_index(tmp,infos->tags);
} else {
   i_tokens[(*n_tokens)++]=get_value_index(token,infos->tags);
}
free(token);

/* Then, we process the rest of the tokens */
while (!is_empty(u_tokens)) {
   token=(unichar*)take_ptr(u_tokens);
   if (token[0]==':' && token[1]!='\0') {
      fatal_error("%S: unexpected subgraph call in token_sequence_2_integer_sequence\n",
            infos->graph_names->value[current_graph]);
   }
   u_sprintf(tmp,"%S",token);
   if (infos->debug) {
	   /* In debug mode, we have to add an output, but only the part with the
	    * debug info */
	   u_strcat(tmp,"/");
	   unichar foo[2]={DEBUG_INFO_OUTPUT_MARK,0};
	   u_strcat(tmp,foo);
	   int i;
	   for (i=u_strlen(output)-1;output[i]!=DEBUG_INFO_COORD_MARK;i--) {
	   }
	   u_strcat(tmp,output+i);
	   int start=0;
	   if (token[0]=='@' && token[1]!='\0') {
		   /* double-quoted token are prefixed by @, so we have to remove this
		    * @ in the debug output */
		   start++;
	   }
	   u_strcat(tmp,token+start);
	   int size=u_strlen(tmp);
	   tmp[size]=DEBUG_INFO_END_MARK;
	   tmp[size+1]='\0';
   }
   i_tokens[(*n_tokens)++]=get_value_index(tmp,infos->tags);
   free(token);
}
}


/**
 * This function takes a tag number sequence and a list of reachable states
 * and it creates pathes from the current state to each of the reachable states,
 * introducing at new intermediate states as needed.
 */
void write_transitions(SingleGraph graph,int* tag_numbers,vector_int* transitions,
                      int current_state,int n_tag_numbers) {
int tmp_state;
for (int i=0;i<transitions->nbelems;i++) {
   tmp_state=current_state;
   for (int j=0;j<n_tag_numbers-1;j++) {
      /* If we are not on the last tag number, then we must introduce a new
       * intermediate state */
      create_intermediate_state(graph,tmp_state,tag_numbers[j]);
      tmp_state=graph->number_of_states-1;
   }
   /* Finally, we rely the state we are to the reachable state */
   add_outgoing_transition(graph->states[tmp_state],tag_numbers[n_tag_numbers-1],transitions->tab[i]);
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
void process_box_line(SingleGraph graph,unichar* input,unichar* output,vector_int* transitions,
                     int *pos,int current_state,int n,struct compilation_info* infos,int line,
                     GrfState* state) {
int result=0;
struct fifo* sequence=new_fifo();
while (result==0 && input[*pos]!='\0') {
   result=process_box_line_token(input,pos,sequence,n,infos);
}
int sequence_ent[MAX_TOKENS_IN_A_SEQUENCE];
int n_tokens;
if (!infos->debug) {
	token_sequence_2_integer_sequence(sequence,output,sequence_ent,infos,&n_tokens,n,1,0);
} else {
	unichar output2[MAX_GRF_BOX_CONTENT];
	u_strcpy(output2,output);
	add_debug_infos(output2,n,state->box_number,line);
	token_sequence_2_integer_sequence(sequence,output2,sequence_ent,infos,&n_tokens,n,1,0);
}
free_fifo(sequence);
write_transitions(graph,sequence_ent,transitions,current_state,n_tokens);
}


/**
 * If we have a variable or context mark, we store it in the tags, if not already
 * present, and we write the corresponding transitions.
 */
void process_variable_or_context(SingleGraph graph,unichar* input,
                                vector_int* transitions,
                                int current_state,struct compilation_info* infos,
                                int current_graph,unichar* debug_output) {
struct fifo* tmp=new_fifo();
int token[2];
int i;
int n=0;
if (debug_output[0]!='\0' && u_strcmp(input,"$]")) {
	put_ptr(tmp,u_strdup("<E>"));
	token_sequence_2_integer_sequence(tmp,debug_output,token+n,infos,&i,current_graph,0,1);
	n++;
}
put_ptr(tmp,u_strdup(input));
token_sequence_2_integer_sequence(tmp,NULL,token+n,infos,&i,current_graph,0,0);
n++;
if (debug_output[0]!='\0' && !u_strcmp(input,"$]")) {
	put_ptr(tmp,u_strdup("<E>"));
	token_sequence_2_integer_sequence(tmp,debug_output,token+n,infos,&i,current_graph,0,1);
	n++;
}
free_fifo(tmp);
write_transitions(graph,token,transitions,current_state,n);
}



/**
 * 's' is supposed to be a sequence found between two $ in an output.
 * We test here if this sequence is a valid one, raising a fatal error
 * if not.
 */
static void check_dollar_sequence(int current_graph,struct compilation_info* infos,
									unichar* s) {
int foo;
unichar c;
unichar* ptr=s;
if (s[0]=='\0') {
	/* $$ is fine */
	return;
}
if (u_sscanf(s,"{%d}%C",&foo,&c)==1) {
	/* If we have a valid weight sequence */
	return;
}
/* Any other sequence should start with a variable name */
while (is_variable_char(*s)) {s++;}
if (*s=='\0') {
	/* $a$ is fine */
	return;
}
if (*s!='.') {
	char name[FILENAME_MAX];
	get_absolute_name(NULL,name,current_graph,infos);
	fatal_error("Graph %s: invalid $...$ sequence in output:\n$%S$\n",name,ptr);
}
s++;
if (u_starts_with(s,"EQUAL=")
		|| u_starts_with(s,"EQUALcC=")
		|| u_starts_with(s,"UNEQUAL=")
		|| u_starts_with(s,"UNEQUALcC=")) {
	while ((*s)!='=') s++;
	s++;
	if ((*s)=='#') {
		/* If we have a comparison to a constant string, any string is acceptable,
		 * so it's a success
		 */
		return;
	}
	if ((*s)=='\0') {
		char name[FILENAME_MAX];
		get_absolute_name(NULL,name,current_graph,infos);
		fatal_error("Graph %s: empty variable name in output:\n$%S$\n",name,ptr);
	}
	while ((*s)!='\0') {
		if (!is_variable_char(*s)) {
			char name[FILENAME_MAX];
			get_absolute_name(NULL,name,current_graph,infos);
			fatal_error("Graph %s: invalid variable name in output:\n$%S$\n",name,ptr);
		}
		s++;
	}
	/* Everything is ok */
	return;
}
if (!u_strcmp(s,"EQ=")) {
	char name[FILENAME_MAX];
	get_absolute_name(NULL,name,current_graph,infos);
	fatal_error("Graph %s: missing code value in output:\n$%S$\n",name,ptr);
}
if (!u_strcmp(s,"CODE.ATTR=")) {
	char name[FILENAME_MAX];
	get_absolute_name(NULL,name,current_graph,infos);
	fatal_error("Graph %s: missing attribute value in output:\n$%S$\n",name,ptr);
}
if (!u_strcmp(s,"SET")
		|| !u_strcmp(s,"UNSET")
		|| u_starts_with(s,"EQ=")
		|| !u_strcmp(s,"INFLECTED")
		|| !u_strcmp(s,"LEMMA")
		|| !u_strcmp(s,"CODE")
		|| !u_strcmp(s,"CODE.GRAM")
		|| !u_strcmp(s,"CODE.SEM")
		|| !u_strcmp(s,"CODE.FLEX")
		|| u_starts_with(s,"CODE.ATTR=")) {
	/* Valid sequence */
	return;
}
char name[FILENAME_MAX];
get_absolute_name(NULL,name,current_graph,infos);
fatal_error("Graph %s: invalid $...$ sequence in output:\n$%S$\n",name,ptr);
}


/**
 * Checks whether the given output sequence is well formed and raises
 * a fatal error if necessary.
 */
static void check_output_validity(int current_graph,struct compilation_info* infos,
									unichar* output) {
if (output[0]=='\0') return;
int i=0;
Ustring* foo=new_Ustring();
while (output[i]!='\0') {
	if (output[i]!='$') {
		i++;
	} else {
		i++;
		empty(foo);
		while (output[i]!='\0' && output[i]!='$') {
			u_strcat(foo,output[i]);
			i++;
		}
		if (output[i]=='\0') {
			char name[FILENAME_MAX];
			get_absolute_name(NULL,name,current_graph,infos);
			fatal_error("Graph %s: invalid output with no closing $:\n%S\n",name,output);
		}
		check_dollar_sequence(current_graph,infos,foo->str);
		i++;
	}
}
free_Ustring(foo);
}


/**
 * Processes the given grf state of the graph #n.
 */
void process_grf_state(unichar* box_content,vector_int* transitions,
                      SingleGraph graph,int current_state,
                      int n,struct compilation_info* infos,
                      GrfState* state) {
unichar input[MAX_GRF_BOX_CONTENT];
unichar output[MAX_GRF_BOX_CONTENT];
if (transitions->nbelems==0) {
   /* If the state has no outgoing transition, it will be discarded when the
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
   unichar debug_output[MAX_GRF_BOX_CONTENT];
   debug_output[0]='\0';
   if (infos->debug) {
	   /* As a normal variable/context tag has no output, we have
	    * to create a <E> tag associated to this output */
	   unichar foo[2]={DEBUG_INFO_OUTPUT_MARK,0};
	   u_strcat(debug_output,foo);
	   add_debug_infos(debug_output,n,state->box_number,0);
	   u_strcat(debug_output,box_content);
	   int size=u_strlen(debug_output);
	   debug_output[size]=DEBUG_INFO_END_MARK;
	   debug_output[size+1]='\0';
   }
   process_variable_or_context(graph,input,transitions,current_state,infos,n,debug_output);
   return;
}
/* Otherwise, we deal with the output of the box, if any */
int shift=0;
if (infos->debug) {
	shift=1;
	output[0]=DEBUG_INFO_OUTPUT_MARK;
}
split_input_output(box_content,input,output+shift);
check_output_validity(n,infos,output+shift);
/* And we process the box input */
int pos=0;
int line=0;
while (input[pos]!='\0') {
   process_box_line(graph,input,output,transitions,&pos,current_state,n,infos,line,state);
   line++;
}
}


/**
 * Takes a string of the form $[...$ and tests if it is a valid range expression.
 * Returns 1 in case of success; 0 otherwise. For $[m,]$ n value is -1.
 */
static int test_range(unichar* s,int *m,int *n) {
int len=0;
if (1==u_sscanf(s,"$[%d]$%n",m,&len) && (*m)>=0 && s[len]=='\0') {
	*n=*m;
	return 1;
}
if (1==u_sscanf(s,"$[%d,]$%n",m,&len) && (*m)>=0 && s[len]=='\0') {
	*n=-1;
	return 1;
}
if (2==u_sscanf(s,"$[%d,%d]$%n",m,n,&len) && (*m)>=0 && (*n)>=0 && s[len]=='\0') {
	return 1;
}
if (1==u_sscanf(s,"$[,%d]$%n",n,&len) && (*n)>=0 && s[len]=='\0') {
	*m=1;
	return 1;
}
return 0;
}


/**
 * Do the actual duplication of the given box according to the given range.
 */
static void do_box_range_expansion(Grf* grf,ReverseTransitions* reverse,int box,int m,int n) {
if (m==0) {
	/* We first deal with the case of m=0 by adding transitions to skip the box */
	m=1;
	vector_int* v=reverse->t[box];
	for (int i=0;i<v->nbelems;i++) {
		int previous_state=v->tab[i];
		for (int j=0;j<grf->states[box]->transitions->nbelems;j++) {
			int dest_state=grf->states[box]->transitions->tab[j];
			vector_int_add_if_absent(grf->states[previous_state]->transitions,dest_state);
		}
	}
}
/* First, we set the m mandatory copies of the box */
int current=box;
for (int i=1;i<m;i++) {
	/* We copy the current box */
	GrfState* current_state=grf->states[current];
	GrfState* s=cpy_grf_state(grf->states[current]);
	/* And we replace all its transitions to a single transition to the new one */
	current=add_GrfState(grf,s);
	current_state->transitions->nbelems=0;
	vector_int_add(current_state->transitions,current);
}
/* Then, if n=-1 (no max limit), we just have to add a loop and return */
if (n==-1) {
	vector_int_add(grf->states[current]->transitions,current);
	return;
}
/* Otherwise, we add up to n boxes */
for (int i=m;i<n;i++) {
	/* We copy the current box */
	GrfState* current_state=grf->states[current];
	GrfState* s=cpy_grf_state(grf->states[current]);
	/* And we just add a transition to the new one */
	current=add_GrfState(grf,s);
	vector_int_add(current_state->transitions,current);
}
}


/**
 * If a box has an output starting with a range expression, it is replaced
 * by the equivalent box duplication. Possible ranges expressions are:
 * $[n]$ => exactly n times
 * $[m,]$ => at least n times
 * $[,n]$ => between 1 and n times
 * $[m,n]$ => between m and n times
 *
 * Note that the function raises a fatal error if the box contains a
 * transition to itself.
 *
 * Note also that this function does not apply to the initial and final states.
 */
static void expand_box_ranges(Grf* grf) {
ReverseTransitions* reverse=compute_reverse_transitions(grf);
int n=grf->n_states;
for (int i=2;i<n;i++) {
	GrfState* s=grf->states[i];
	int m=1,n=1;
	vector_ptr* lines=tokenize_box_content(s->box_content);
	unichar* last=(unichar*)lines->tab[lines->nbelems-1];
	if (last[0]=='/') {
		/* If the last line starts with a slash, then the box has an output,
		 * and we look for a range expression */
		if (last[1]=='$' && last[2]=='[') {
			int pos=3;
			while (last[pos]!='\0' && last[pos]!='$') {
				pos++;
			}
			if (last[pos]=='$') {
				/* We don't care if the end of line was found, it's not
				 * our business here */
				unichar old=last[pos+1];
				last[pos+1]='\0';
				int ok=test_range(last+1,&m,&n);
				if (!ok || (m==0 && n==0) || (n!=-1 && n<m)) {
					fatal_error("Invalid range expression: %S\n",last+1);
				}
				/* If we have a valid range, we first test if the box has a
				 * loop to itself, which would be an error */
				if (-1!=vector_int_contains(s->transitions,i)) {
					fatal_error("A box with a range %S cannot have a loop to itself\n",last+1);
				}
				last[pos+1]=old;
								/* Then we adjust the output of the box */
				int j=u_strlen(s->box_content)-u_strlen(last);
				pos++;
				do {
					s->box_content[j++]=last[pos++];
				} while (last[pos-1]!='\0');
				s->box_content[j-1]='"';
				s->box_content[j]='\0';
				/* Now we can actually duplicate the box */
				do_box_range_expansion(grf,reverse,i,m,n);
			}
		}
	}
	free_vector_ptr(lines,free);
}
free_ReverseTransitions(reverse);
}


/**
 * We have to copy the given fst2 as some graphs in the current .fst2
 * being saved.
 */
void save_compiled_fst2(char* name,Fst2* fst2,struct compilation_info* infos) {
Ustring* ustr=new_Ustring();
int n=infos->current_saved_graph;
for (int i=0;i<fst2->number_of_graphs;i++) {
	/* We indicate that we have a graph that is part of a .fst2 */
	vector_int_add(infos->part_of_precompiled_fst2,1);
	if (i==0) {
		u_fprintf(infos->fst2,"-%d %s\n",n++,name);
	} else {
		u_fprintf(infos->fst2,"-%d %s:<%S>\n",n++,name,fst2->graph_names[i+1]);
	}
	int N=fst2->initial_states[i+1];
	for (int j=0;j<fst2->number_of_states_per_graphs[i+1];j++) {
		Fst2State s=fst2->states[N+j];
		if (!is_final_state(s)) {
			u_fprintf(infos->fst2,": ");
		} else {
			u_fprintf(infos->fst2,"t ");
		}
		Transition* t=s->transitions;
		while (t!=NULL) {
			if (t->tag_number<0) {
				u_fprintf(infos->fst2,"-%d ",(infos->current_saved_graph-1)-t->tag_number);
				error("** -%d **\n",(infos->current_saved_graph-1)-t->tag_number);
			} else {
				empty(ustr);
				Fst2Tag tag=fst2->tags[t->tag_number];
				if (tag->control & RESPECT_CASE_TAG_BIT_MASK) {
				   u_strcat(ustr,"@");
				}
				u_strcatf(ustr,"%S",tag->input);
				if (tag->morphological_filter!=NULL) {
					u_strcatf(ustr,"%S",tag->morphological_filter);
				}
				if (tag->output!=NULL) {
					u_strcatf(ustr,"/%S",tag->output);
				}
				u_fprintf(infos->fst2,"%d ",get_value_index(ustr->str,infos->tags));
			}
			u_fprintf(infos->fst2,"%d ",t->state_number-N);
			t=t->next;
		}
		u_fprintf(infos->fst2,"\n");
	}
	u_fprintf(infos->fst2,"f \n");
}
free_Ustring(ustr);
}


/**
 * This function compiles the graph number #n and saves its states into the
 * output .fst2.
 */
int compile_grf(int n,struct compilation_info* infos) {
int i;
char called_from[FILENAME_MAX]="";
char name[FILENAME_MAX];
char name_fst2[FILENAME_MAX];
int n_caller;
SingleGraph graph=new_SingleGraph();
/* We get the absolute path of the graph */
get_absolute_name(&n_caller,name,n,infos);
if (n_caller!=-1) {
	get_absolute_name(NULL,called_from,n_caller,infos);
}
char* full_name=NULL;
if (infos->debug) {
	full_name=name;
}
infos->current_saved_graph++;
vector_int_add(infos->renumber,infos->current_saved_graph);
char foo[FILENAME_MAX];
get_extension(name,foo);
if (foo[0]=='\0') {
	strcat(name,".grf");
}
Grf* grf=NULL;
if (fexists(name)) {
	grf=load_Grf(&(infos->vec),name);
}
if (grf==NULL) {
	/* If we can't load the .grf, maybe we should try the .fst2 */
	remove_extension(name,name_fst2);
	strcat(name_fst2,".fst2");
	Fst2* fst2=NULL;
	if (n!=1 && fexists(name_fst2)) {
		/* We don't try to load the .fst2 for the main graph */
		fst2=load_fst2(&(infos->vec),name_fst2,1);
		if (fst2==NULL) {
			error("Cannot load %s\n",name_fst2);
			write_graph(infos->fst2,graph,-n,infos->graph_names->value[n],full_name);
			free_SingleGraph(graph,NULL);
			vector_int_add(infos->part_of_precompiled_fst2,0);
			if (n==1) return 0;
			return 1;
		}
	} else {
		error("Cannot open graph %s\n",name);
		if (called_from[0]!='\0') {
			error("which is called from %s\n",called_from);
		}
		write_graph(infos->fst2,graph,-n,infos->graph_names->value[n],full_name);
		free_SingleGraph(graph,NULL);
		vector_int_add(infos->part_of_precompiled_fst2,0);
		if (n==1) return 0;
		return 1;
	}
	if (infos->verbose_name_grf!=0) {
		u_printf("Loading compiled graph %s\n",name_fst2);
	}
	save_compiled_fst2(name_fst2,fst2,infos);
	/* -1 because we already made infos->current_saved_graph++ */
	infos->current_saved_graph+=(fst2->number_of_graphs-1);
	free_Fst2(fst2);
	free_SingleGraph(graph,NULL);
	return 1;
}
if (infos->verbose_name_grf!=0) {
  u_printf("Compiling graph %s\n",/*infos->graph_names->value[n]*/name);
}
/* We indicate that we have a .grf */
vector_int_add(infos->part_of_precompiled_fst2,0);
expand_box_ranges(grf);
/* If necessary, we resize the graph that it can hold all the states */
if (graph->capacity<grf->n_states) {
   set_state_array_capacity(graph,grf->n_states);
}
/* Now, every line represents a state of the automaton */
for (i=0;i<grf->n_states;i++) {
   graph->states[i]=new_SingleGraphState();
}
graph->number_of_states=grf->n_states;
for (i=0;i<grf->n_states;i++) {
   /* process_grf_state expect a box content without the surround double quotes */
   grf->states[i]->box_content[u_strlen(grf->states[i]->box_content)-1]='\0';
   /* To preserve previous behavior (for log consistency), we mirror the
    * transition list */
   for (int x=0,y=grf->states[i]->transitions->nbelems-1;x<y;x++,y--) {
	   int tmp=grf->states[i]->transitions->tab[x];
	   grf->states[i]->transitions->tab[x]=grf->states[i]->transitions->tab[y];
	   grf->states[i]->transitions->tab[y]=tmp;
   }
   process_grf_state(grf->states[i]->box_content+1,grf->states[i]->transitions,graph,i,n,infos,grf->states[i]);
}
free_Grf(grf);
/* Once we have loaded the graph, we process it. */
set_initial_state(graph->states[0]);
set_final_state(graph->states[1]);
compute_reverse_transitions(graph);
check_co_accessibility(graph->states,1);
remove_epsilon_transitions(graph,1);
check_accessibility(graph->states,0);
remove_useless_states(graph,NULL);
if (graph->states[0]==NULL) {
   /* If the graph has been emptied */
   write_graph(infos->fst2,graph,-n,infos->graph_names->value[n],full_name);
   free_SingleGraph(graph,NULL);
   if (n!=1 && infos->no_empty_graph_warning) return 1;
   if (n==1) {
      error("ERROR: Main graph %S.grf has been emptied\n",infos->graph_names->value[n]);
      return 0;
   }
   error("WARNING: graph %S.grf has been emptied\n",infos->graph_names->value[n]);
   return 1;
}
/* Now, we minimize the automaton assuming that reversed transitions are still there */
minimize(graph,0);
/* Finally we save the graph */
write_graph(infos->fst2,graph,-infos->current_saved_graph,infos->graph_names->value[n],full_name);
free_SingleGraph(graph,NULL);
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
 *
 * In debug mode, the first char is a 'd' followed by the number of
 * graphs written on 9 digits.
 */
void write_number_of_graphs(const VersatileEncodingConfig* vec,const char* name,int n,int debug) {
U_FILE* f=u_fopen(vec,name,U_MODIFY);
/* And we print the number of graphs on 10 digits */
u_fprintf(f,"%c%09d",debug?'d':'0',n);
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


/**
 * Performs the graph call renumbering that may have been made
 * necessary if some precompiled .fst2 were loaded
 */
void renumber_graph_calls(Fst2* fst2,vector_int* renumber,vector_int* part_of_precompiled_fst2) {
for (int i=1;i<=fst2->number_of_graphs;i++) {
	if (part_of_precompiled_fst2->tab[i]) continue;
	int n=fst2->number_of_states_per_graphs[i]+fst2->initial_states[i];
	for (int j=fst2->initial_states[i];j<n;j++) {
		Fst2State s=fst2->states[j];
		s->transitions=reverse_list(s->transitions);
		Transition* t=s->transitions;
		while (t!=NULL) {
			if (t->tag_number<0) {
				t->tag_number=-(renumber->tab[-(t->tag_number)]);
			}
			t=t->next;
		}
	}
}
}

} // namespace unitex
