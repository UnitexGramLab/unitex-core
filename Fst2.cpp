 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Universit� de Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#include "Fst2.h"
#include "Error.h"
#include "LocateConstants.h"
#include "BitMasks.h"


/*
 * These two constants are declared here instead of in the .h because
 * we don't want people to write tests like (e->control & FST2_FINAL_STATE_BIT_MASK)
 * We prefer writing: is_final_state(e)
 */
#define FST2_FINAL_STATE_BIT_MASK 1
#define FST2_INITIAL_STATE_BIT_MASK 2

/* The constant NO_TAG_LIMIT must have a negative value */
#define NO_TAG_LIMIT -1

/* The constant NO_GRAPH_NUMBER_SPECIFIED must have a value <=0 */
#define NO_GRAPH_NUMBER_SPECIFIED -1



/**
 * Frees a state and all its transitions.
 */
void free_Fst2State(Fst2State e) {
free_Transition_list(e->transitions);
free(e);
}


/**
 * Frees a tag.
 */
void free_Fst2Tag(Fst2Tag e) {
if (e->input!=NULL) free(e->input);
if (e->output!=NULL) free(e->output);
if (e->morphological_filter!=NULL) free(e->morphological_filter);
if (e->pattern!=NULL) free_pattern(e->pattern);
if (e->variable!=NULL) free(e->variable);
free_list_int(e->matching_tokens);
free(e);
}


/**
 * Allocates, initializes and returns an empty automaton.
 */
Fst2* new_Fst2() {
Fst2* a=(Fst2*)malloc(sizeof(Fst2));
a->states=NULL;
a->tags=NULL;
a->number_of_graphs=0;
a->number_of_states=0;
a->number_of_tags=0;
a->initial_states=NULL;
a->graph_names=NULL;
a->number_of_states_per_graphs=NULL;
a->variables=NULL;
return a;
}


/**
 * Frees a fst2. The function assumes that if 'fst2' is not NULL, all
 * its field are neither NULL nor already freed.
 */
void free_Fst2(Fst2* fst2) {
if (fst2==NULL) return;
int i;
for (i=0;i<fst2->number_of_states;i++) {
  free_Fst2State(fst2->states[i]);
}
free(fst2->states);
for (i=0;i<fst2->number_of_tags;i++)
  free_Fst2Tag(fst2->tags[i]);
free(fst2->tags);
if (fst2->graph_names!=NULL) {
  for ( i = 1;                       /* start at 1 because at pos 0
                                        there is no graph */
        i <= fst2->number_of_graphs; /* consequently the last pos is
                                        number_of_graphs+1 */
        i++ )
    {
      if (fst2->graph_names[i]!=NULL) free(fst2->graph_names[i]);
    }
  free(fst2->graph_names);
}
free(fst2->initial_states);
free(fst2->number_of_states_per_graphs);
free_list_ustring(fst2->variables);
free(fst2);
}


/**
 * Allocates, initializes and returns a new tag
 */
Fst2Tag new_Fst2Tag() {
Fst2Tag e;
e=(Fst2Tag)malloc(sizeof(struct fst2Tag));
if (e==NULL) {
  fatal_error("Not enough memory in new_Fst2Tag\n");
}
e->type=UNDEFINED_TAG;
e->control=0;
e->input=NULL;
e->output=NULL;
e->pattern=NULL;
e->variable=NULL;
e->matching_tokens=NULL;
e->compound_pattern=NO_COMPOUND_PATTERN;
e->morphological_filter=NULL;
e->filter_number=-1;
return e;
}


/**
 * Creates and returns the tag for the string 'line', representing a tag line
 * in a .fst2 file like "%<V>/VERB", without an ending '\n'.
 */
Fst2Tag create_tag(Fst2* fst2,unichar* line) {
unichar all_input[2048],output[2048];
int i=1,j=0,k=0;
/* First, we look if the tag must respect case */
int respect_case=(line[0]=='@');
/*
 * Then, we look for a '/' in order to see if there is an output. The first character
 * is necessary a part of the input since tags with no input like "%/OUTPUT" are not
 * allowed. We look for a '/' that is not preceeded by '\'. All the input is copied
 * into 'all_input'.
 */
all_input[k++]=line[i++];
while(line[i]!='\0' && !(line[i]=='/' && i>0 && line[i-1]!='\\')) {
	all_input[k++]=line[i++];
}
all_input[k]='\0';
/*
 * If we find an output, we copy it
 */
if(line[i]=='/') {
	i++;
	while(line[i]!='\0') {
		output[j++]=line[i++];
	}
}
/* Then, if there is an output, we have it, and if not, 'output'
 * contains an empty string */
output[j]='\0';
/*
 * Now, we will analyze the string 'all_input', in order to see if it
 * can be decomposed into an input and a morphological filter. A
 * morphological filter is of the form "<<.....>>_f_", but the "_..._" part
 * is optional.
 */
/* $CD$ begin */  
unichar input[2048],filter[2048];
input[0] = '\0';
filter[0] = '\0';
i = 0; j = 0;
/* We copy the real input into 'input' */
while (all_input[i]!='\0' && (all_input[i]!='<' || all_input[i+1]!='<')) {
	input[j++]=all_input[i++];
}
input[j]='\0';
/* If something remains, we have a morphological filter */
if (all_input[i]!='\0') {
	j = 0;
	/* We copy the "<<...>>"  sequence */
	while (all_input[i]!='\0' && (all_input[i]!='>' || all_input[i+1]!='>')) {
		filter[j++]=all_input[i++];
	}
    filter[j++]='>';filter[j++]='>';
    /* If something remains, then we have an optional part like "_f_",
     * and we add it to 'filter' */
    if (all_input[i]!='\0') {
		i=i+2;
		if (all_input[i]=='_') {
			do {
				filter[j++]=all_input[i++];
			} while (all_input[i]!='\0' && all_input[i]!= '_');
			filter[j++] = '_';
		}
	}
    filter[j] = '\0';
}
/* If there is a morphological filter but no input ("%<<^in>>/PFX"), then
 * we say that the input is any token */
if (input[0]=='\0') {u_strcpy(input,"<TOKEN>");}
Fst2Tag tag=new_Fst2Tag();
tag->input=u_strdup(input);
if (output[0]!='\0') {
   tag->output=u_strdup(output);
} else {tag->output=NULL;}
   
if (filter[0]!='\0') {
   tag->morphological_filter=u_strdup(filter);
} else {tag->morphological_filter=NULL;}
if (respect_case) {
   /* We set the case respect bit if necessary */
   tag->control=(unsigned char)(tag->control|RESPECT_CASE_TAG_BIT_MASK);
}
/* We ignore the numbers in the context start marks that are of
 * the form $[45 or $![12 */
if (input[0]=='$' && input[1]=='[') {
   tag->type=BEGIN_POSITIVE_CONTEXT_TAG;
   return tag;
}
if (input[0]=='$' && input[1]=='!' && input[2]=='[') {
   tag->type=BEGIN_NEGATIVE_CONTEXT_TAG;
   return tag;
}
if (!u_strcmp(input,"$]")) {
   tag->type=END_CONTEXT_TAG;
   return tag;
}
/* 
 * IMPORTANT: if the tag is a variable declaration, we must add this variable
 *            to the variable list of the fst2 .
 */
int length=u_strlen(input);
if (input[0]=='$' && 
    (input[length-1]=='(' || input[length-1]==')')) {
   tag->variable=u_strdup(&(input[1]),length-2);
   fst2->variables=sorted_insert(tag->variable,fst2->variables);
   if (input[length-1]=='(') {
      tag->type=BEGIN_VAR_TAG;
   }
   else {
      tag->type=END_VAR_TAG;
   }
}
return tag;
}


/**
 * Stringifies and writes a tag to a file including '\n'.
 * Opposite of "create_tag".
 */
void write_tag(FILE* f,Fst2Tag tag) {
if (tag->control & RESPECT_CASE_TAG_BIT_MASK) {
   u_fprintf(f,"@");
}
else {
   u_fprintf(f,"%%");
}
/* We print the content (label) of the tag */
u_fprintf(f,"%S",tag->input);
/* If any, we add the morphological filter: <A><<^pre>> */
if (tag->morphological_filter!=NULL &&
   tag->morphological_filter[0]!='\0') {
   u_fprintf(f,"%S",tag->morphological_filter);
}
/* If any, we add the output */
if (tag->output!=NULL) {
   if (tag->output[0]=='\0') {
      fatal_error("Invalid empty ouput in write_tag\n");
   }
   u_fprintf(f,"/%S",tag->output);
}
u_fprintf(f,"\n");
}


/**
 * Reads the tags of the .fst2 file 'f'. The function assumes that the
 * current position in the file is just before the first tag line.
 * For each tag line, a tag is created and inserted in the tag array of 
 * the given fst2.
 * 
 * The parameter 'limit' indicates that the function
 * may stop reading after the tag number 'limit'. NO_TAG_LIMIT indicates
 * that there is no limit. This is used to avoid loading the whole tag list
 * when you know the highest tag number that you will need, in particular
 * when you load just one sentence from a .fst2 that represents a text
 * automaton. 
 */
void read_fst2_tags(FILE *f,Fst2* fst2,int limit) {
int SIZE=2;
int i;
unichar c;
unichar line[10000];
int current_tag=0;
/* First, we allocate the tag array */
fst2->tags=(Fst2Tag*)malloc(SIZE*sizeof(Fst2Tag));
if (fst2->tags==NULL) {
   fatal_error("Not enough memory in read_fst2_tags\n");
}
/* If the position in the file is not correct we exit */
if (((c=(unichar)u_fgetc(f))!='%')&&(c!='@')) {
	fatal_error("Unexpected character in .fst2 file: %c (read tag)\n",c);
}
/* There cannot have no tag line, because by convention, every .fst2 file
 * must have "%<E>" as first tag. */
while (c!='f' && (limit==NO_TAG_LIMIT || current_tag<=limit)) {
	/* We read the line and copy it without the ending '\n' */
	i=0;
	do {
		line[i++]=c;
	} while ((c=(unichar)u_fgetc(f))!='\n');
	/* And we read the first character of the next line */
  	if (((c=(unichar)u_fgetc(f))!='f')&&(c!='%')&&(c!='@')) {
  		/* If the character is not an expected one we exit */
  		fatal_error("Unexpected character in .fst2 file: %c (read tag)\n",c);
  	}
	line[i]='\0';
	/* We create the tag and add it to the fst2 */
	fst2->tags[current_tag]=create_tag(fst2,line);
	/* We do not forget to increase the tag counter */
	current_tag++;
   if (current_tag==SIZE) {
      /* If necessary, we double the size of the array */
      SIZE=SIZE*2;
      fst2->tags=(Fst2Tag*)realloc(fst2->tags,SIZE*sizeof(Fst2Tag));
      if (fst2->tags==NULL) {
         fatal_error("Not enough memory in read_fst2_tags\n");
      }
   }
}
/* Finally, we set the number of tags of the fst2 */
fst2->number_of_tags=current_tag;
/* And we resize the array to the exact size */
fst2->tags=(Fst2Tag*)realloc(fst2->tags,current_tag*sizeof(Fst2Tag));
if (fst2->tags==NULL) {
   fatal_error("Not enough memory in read_fst2_tags\n");
}
}


/**
 * Reads all the tags of the .fst2 file 'f'.
 */
void read_fst2_tags(FILE *f,Fst2* fst2) {
read_fst2_tags(f,fst2,NO_TAG_LIMIT);
}


/**
 * Writes all the tags to the .fst2 file 'f'.
 * (opposite of "read_fst2_tags")
 */
void write_fst2_tags(FILE* f,Fst2* fst2) {
for (int i=0;i<fst2->number_of_tags;i++) {
   write_tag(f,fst2->tags[i]);
}
u_fprintf(f,"f\n");
}

/**
 * Writes one state of automaton to the .fst2 file 'f'.
 */
void write_fst2_state(FILE* f,Fst2State s) {
if (is_final_state(s))
   u_fprintf(f,"t ");
else u_fprintf(f,": ");
Transition* ptr=s->transitions;
while(ptr!=NULL) {
   u_fprintf(f," %d %d",ptr->tag_number,ptr->state_number);
   ptr=ptr->next;
}
u_fputc((unichar)' ',f);
u_fputc((unichar)'\n',f);
}


/**
 * Writes graph n of the grammar g to the .fst2 file 'f'.
 */
void write_graph(FILE* f,Fst2* fst2,int n) {
u_fprintf(f,
          "-1 flattened version of graph "
          "%S\n",
          fst2->graph_names[n]);
/* to be implemented */
fatal_error("function write_graph not fully implemented\n");
}


/**
 * Creates, initializes and returns a fst2 state.
 */
Fst2State new_Fst2State() {
Fst2State state;
state=(Fst2State)malloc(sizeof(struct fst2State));
if (state==NULL) {
  fatal_error("Not enough memory in new_Fst2State\n");
}
state->control=0;
state->transitions=NULL;
return state;
}


/**
 * Reads and returns a signed integer in a .fst2 file. If an end of line is
 * found, 'end_of_line' is setted to 1 and 0 is returned; Otherwise, 
 * 'end_of_line' is setted to 0.
 */
int read_int(FILE *f,int *end_of_line) {
register unichar c;
register int value;
int negative_number;
/* We ignore spaces */
do {
	c=(unichar)u_fgetc(f);
} while (c==' ');
/* If 'c' is neither a digit nor a minus sign nor an end of line, we exit */
if (((c<'0')||(c>'9'))&&(c!='-')&&(c!='\n')) {
	fatal_error("Unexpected character in fst2: %c (read int 1)\n",c);
}
if (c=='\n') {
  /* If we have an end of line */
  (*end_of_line)=1;
  return 0;
}
(*end_of_line)=0;
if (c=='-') {
  /* If we have a minus sign, we note that we have a negative number
   * and we read the next character that must be a digit */
  negative_number=1;
  c=(unichar)u_fgetc(f);
  if (c<'0' || c>'9') {fatal_error("Unexpected character in fst2 (read int 2): %c\n",c);}
} else {negative_number=0;}
/* We compute the value of the integer */
value=c-'0';
while(((c=(unichar)u_fgetc(f))>='0')&&(c<='9')) {
	value=value*10+(c-'0');
}
if (negative_number) value=-value;
return value;
}


/**
 * Creates and adds a transition to the given fst2 state.
 * 
 * NOTE: as a fst2 state is not supposed to contains duplicate 
 *       transitions, we do not check if the transition already
 *       exists before adding it.
 */
void add_transition_to_state(Fst2State state,int tag_number,int state_number) {
state->transitions=new_Transition(tag_number,state_number,state->transitions);
}


void set_initial_state(fst2State*,int);
void set_final_state(fst2State*,int);

/**
 * Reads fst2 states from the given file 'f' and stores them into
 * the given fst2. If 'read_names' is non null, graph names are
 * stored in the 'graph_names' array of the fst2.
 * 
 * The 'graph_number' parameter is used when you want to load only one
 * graph. In that case, its value is the number of the graph to load. The
 * value must be positive; the first graph is 1. If this parameter is used,
 * the 'max_tag_number' will be used to store the higher tag number used by
 * the specified graph, in order to avoid loading the whole tag list. These 
 * parameters are useful for loading one sentence from a .fst2 that represents
 * a text automaton. If the value of 'graph_number' is NO_GRAPH_NUMBER_SPECIFIED,
 * then all the fst2 is loaded, and the 'max_tag_number' parameter is ignored.
 */
void read_fst2_states(FILE *f,Fst2* fst2,int read_names,int graph_number,int *max_tag_number) {
int SIZE=256;
unichar c;
int i,end_of_line,tag_number,destination_state_number,current_graph;
int current_state=0;
fst2->states=(Fst2State*)malloc(SIZE*sizeof(Fst2State));
if (fst2->states==NULL) {
   fatal_error("Not enough memory in read_fst2_states\n");
}
/* We read all the graphs that make the fst2 */
for (i=0;i<fst2->number_of_graphs;i++) {
	/* We read the graph number and the space after it */
   u_fscanf(f,"%d ",&current_graph);
   /* And we make it positive */
   current_graph=current_graph*(-1);
	/* We set the initial state of the graph */
	fst2->initial_states[current_graph]=current_state;
	int relative_state=0;
	/*
	 * We read the graph name
	 */
	unichar graph_name[10000];
   u_fgets(graph_name,f);
	if (graph_number==NO_GRAPH_NUMBER_SPECIFIED || graph_number==current_graph) {
		/* If we must read the graph either because it is the one we look for
		 * or because we must read them all, then we initialize 'max_tag_number' */ 
    	(*max_tag_number)=0;
		/*
		 * We save the graph name if needed
		 */
		if (read_names) {
			fst2->graph_names[current_graph]=u_strdup(graph_name);
		}
		/* 
		 * We read the next char that must be 't' or ':' but not 'f', because 
		 * empty graphs are not allowed 
		 */
		c=(unichar)u_fgetc(f);
		if ((c!='t')&&(c!=':')) {fatal_error("Unexpected character in fst2: %c (read state)\n",c);}
		/*
		 * Then, we read the states of the graph, until we find a line beginning by 'f'.
		 */
		fst2->number_of_states_per_graphs[current_graph]=0;
		while (c!='f') {
			fst2->states[current_state]=new_Fst2State();
			fst2->number_of_states_per_graphs[current_graph]++;
			/*
			 * We set the finality and initiality bits of the state
			 */
			set_final_state(fst2->states[current_state],(c=='t'));
			set_initial_state(fst2->states[current_state],(relative_state==0));
			/*
			 * We read the tag number
			 */
			tag_number=read_int(f,&end_of_line);
			/* 
			 * We read transitions made of couple of integers (tag number/state number)
			 * until we find an end of line
			 */
			while (!end_of_line) {
				if (tag_number>(*max_tag_number)) {
					/* We update the highest tag number */
					(*max_tag_number)=tag_number;
				}
				/* We read the destination state number */
				destination_state_number=read_int(f,&end_of_line);
				if (end_of_line) {fatal_error("Missing state number in transition (graph %d, state %d)\n",current_graph,relative_state);}
				/* We adjust the destination state number in order to make it global */
				destination_state_number=destination_state_number+fst2->initial_states[current_graph];
				/* We add the transition to the current state */
				add_transition_to_state(fst2->states[current_state],tag_number,destination_state_number);
				/* And we do not forget to read the next integer */
				tag_number=read_int(f,&end_of_line);
			}
			if (((c=(unichar)u_fgetc(f))!=':')&&(c!='t')&&(c!='f')) {
            fatal_error("Unexpected character in fst2: %c\n",c);
			}
			current_state++;
         if (current_state==SIZE) {
            /* If necessary, we double the size of the state array */
            SIZE=SIZE*2;
            fst2->states=(Fst2State*)realloc(fst2->states,SIZE*sizeof(Fst2State));
            if (fst2->states==NULL) {
               fatal_error("Not enough memory in read_fst2_states\n");
            }
         }
			relative_state++;
		}
	}
	else {
		/*
		 * If we do not need to read this graph, then we just go to the 'f' that
		 * indicates the end of the graph. However, if the 'graph_names' array
		 * exists, we set the name of the current graph to NULL, in order to
		 * avoid memory error during the freeing of the fst2.
		 */
		if (read_names) {
			fst2->graph_names[current_graph]=NULL;
		}
		while(((c=(unichar)u_fgetc(f))!='f'));
	}
	/* We read the space and the '\n' that follows the final 'f' */
   u_fgetc(f);
   u_fgetc(f);
}
/* Finally, we set the number of states of the fst2, and we resize the state array
 * to the exact size */
fst2->number_of_states=current_state;
fst2->states=(Fst2State*)realloc(fst2->states,current_state*sizeof(Fst2State));
if (fst2->states==NULL) {
   fatal_error("Not enough memory in read_fst2_states\n");
}
}


/**
 * Reads fst2 states from the given file 'f' and stores them into
 * the given fst2. If 'read_names' is non null, graph names are
 * stored in the 'graph_names' array of the fst2.
 */
void read_fst2_states(FILE *f,Fst2* fst2,int read_names) {
read_fst2_states(f,fst2,read_names,NO_GRAPH_NUMBER_SPECIFIED,NULL);
}


/**
 * Loads a .fst2 file and returns its representation in a Fst2 structure.
 * 'read_names' indicates if graph names must be stored.
 * If 'graph_number' is setted to NO_GRAPH_NUMBER_SPECIFIED, all the fst2
 * is loaded; otherwise this parameter is taken as the number of the unique
 * graph to load. 
 */

#define GRAPH_IS_EMPTY 1
#define FILE_POINTER_NULL 2

Fst2* load_fst2(char* filename,int read_names,int graph_number) {

FILE* f;
f=u_fopen(filename,U_READ);
Fst2* fst2;
if (f==NULL) {
	error("Cannot open the file %s\n",filename);
	return NULL;
}

int ret=load_fst2_from_file(f,read_names, &fst2, graph_number);

switch (ret) {
case GRAPH_IS_EMPTY:
	error("Graph %s is empty\n",filename);
	return NULL;
}

return fst2;

}

int load_fst2_from_file(FILE *f,int read_names,Fst2** retval, int graph_number) {

if (! f) return 2;

Fst2 *fst2=new_Fst2();

/* We read the number of graphs contained in the fst2 */
u_fscanf(f,"%d\n",&(fst2->number_of_graphs));
if (fst2->number_of_graphs==0) {
	return GRAPH_IS_EMPTY;
}
/*
 * The 'initial_states' and 'number_of_states_per_graphs' arrays are
 * allocated with the correct size. We add +1 because graph numeration
 * starts at 1.
 */
fst2->initial_states=(int*)malloc((fst2->number_of_graphs+1)*sizeof(int));
if (fst2->initial_states==NULL) {fatal_error("Not enough memory in load_fst2\n");}
fst2->number_of_states_per_graphs=(int*)malloc((fst2->number_of_graphs+1)*sizeof(int));
if (fst2->number_of_states_per_graphs==NULL) {fatal_error("Not enough memory in load_fst2\n");}
/*
 * If needed, we allocate the 'graph_names' array. The +1 has the same motivation
 * than above.
 */
if (read_names) {
	fst2->graph_names=(unichar**)malloc((fst2->number_of_graphs+1)*sizeof(unichar*));
	if (fst2->graph_names==NULL) {fatal_error("Not enough memory in load_fst2\n");}
}
/*
 * Then we read the states of the fst2
 */
int max_tag_number;
read_fst2_states(f,fst2,read_names,graph_number,&max_tag_number);
/*
 * And we read the tags
 */
if (graph_number==NO_GRAPH_NUMBER_SPECIFIED) {
	read_fst2_tags(f,fst2);
} else {
	read_fst2_tags(f,fst2,max_tag_number);
}
u_fclose(f);
*retval=fst2;
return 0;
}


/**
 * Loads a .fst2 file and returns its representation in a Fst2 structure.
 */
Fst2* load_fst2(char* filename,int read_names) {
return load_fst2(filename,read_names,NO_GRAPH_NUMBER_SPECIFIED);
}

int load_fst2_from_file(FILE *f,int read_names, Fst2 **fst2) {
	
	return load_fst2_from_file(f,read_names,fst2,NO_GRAPH_NUMBER_SPECIFIED);

}


/**
 * Loads one graph from a .fst2 file that represents a text automaton,
 * and returns its representation in a Fst2 structure. The graph name
 * is stored because it represents the text of the sentence.
 */
Fst2* load_one_sentence_from_fst2(char* filename,int sentence_number) {
return load_fst2(filename,1,sentence_number);
}


/**
 * This function returns 0 if the given state is not initial and a non-zero value
 * if the state is final.
 */
int is_initial_state(Fst2State e) {
if (e==NULL) {
   fatal_error("NULL error in is_initial_state\n");
}
return e->control&FST2_INITIAL_STATE_BIT_MASK;
}


/**
 * This function sets the initiality of the given state.
 */
void set_initial_state(Fst2State e,int finality) {
if (e==NULL) {
   fatal_error("NULL error in set_initial_state\n");
}
/* First the compute the control byte without the initiality bit */
e->control=e->control & (0xFF-FST2_INITIAL_STATE_BIT_MASK);
/* And we add it if necessary*/
if (finality) {
	e->control=e->control | FST2_INITIAL_STATE_BIT_MASK;
}
}


/**
 * This function returns 0 if the given state is not final and a non-zero value
 * if the state is final.
 */
int is_final_state(Fst2State e) {
if (e==NULL) {
   fatal_error("NULL error in is_final_state\n");
}
return e->control&FST2_FINAL_STATE_BIT_MASK;
}


/**
 * This function sets the finality of the given state.
 */
void set_final_state(Fst2State e,int finality) {
if (e==NULL) {
   fatal_error("NULL error in set_final_state\n");
}
/* First the compute the control byte without the finality bit */
e->control=e->control & (0xFF-FST2_FINAL_STATE_BIT_MASK);
/* And we add it if necessary*/
if (finality) {
	e->control=e->control | FST2_FINAL_STATE_BIT_MASK;
}
}


