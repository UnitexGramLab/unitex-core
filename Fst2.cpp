 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
//---------------------------------------------------------------------------
#include "Fst2.h"
#include "Error.h"
#include "LocateConstants.h"
//---------------------------------------------------------------------------


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


Fst2State* graphe_fst2;
Fst2Tag* etiquette_fst2;
int *debut_graphe_fst2;
int *nombre_etats_par_grf;
unichar** nom_graphe;
struct variable_list* liste_des_variables;
int nombre_etats_fst2;
int nombre_graphes_fst2;
int etat_courant;



/**
 * Allocates, initializes and returns a variable list item
 */
struct variable_list* new_variable_list(unichar* name) {
struct variable_list* v;
v=(struct variable_list*)malloc(sizeof(struct variable_list));
if (v==NULL) {
	fatal_error("Not enough memory in new_variable_list\n");
}
v->name=u_strdup(name);
v->start=-1;
v->end=-1;
v->next=NULL;
return v;
}


/**
 * Frees one variable list item
 */
void free_variable(struct variable_list* v) {
if (v->name!=NULL) free(v->name);
free(v);
}


/**
 * Frees a variable list
 */
void free_variable_list(struct variable_list* l) {
struct variable_list* tmp;
while (l!=NULL) {
	tmp=l;
	l=l->next;
	free_variable(tmp);
}
}


/**
 * Adds a variable at the end of the list, if it is not already in the list.
 * The function returns the list, modified or not.
 */
struct variable_list* add_variable_to_list(unichar* name,struct variable_list* v) {
if (v==NULL) return new_variable_list(name);
if (!u_strcmp(v->name,name)) return v;
v->next=add_variable_to_list(name,v->next);
return v;
}


/**
 * This function returns the variable list item corresponding to 'name'.
 * If it is not in the list, NULL is returned.
 */
struct variable_list* get_variable(unichar* name,struct variable_list* v) {
while (v!=NULL) {
	if (!u_strcmp(name,v->name)) return v;
	v=v->next;
}
return NULL;
}


/**
 * Frees a transition list
 */
void free_Fst2Transition(Fst2Transition t) {
struct fst2Transition* tmp;
while (t!=NULL) {
    tmp=t;
    t=t->next;
    free(tmp);
}
}


/**
 * Frees a state and all its transitions
 */
void free_Fst2State(Fst2State e) {
free_Fst2Transition(e->transitions);
free(e);
}


/**
 * Frees a tag
 */
void free_Fst2Tag(Fst2Tag e) {
if (e->input!=NULL) free(e->input);
if (e->output!=NULL) free(e->output);
if (e->inflected!=NULL) free(e->inflected);
if (e->lemma!=NULL) free(e->lemma);
if (e->codes!=NULL) free(e->codes);
#warning matching_tokens should maybe be freed here
/* $CD$ begin */
if (e -> contentGF != NULL) free(e->contentGF);
/* $CD$ end   */
free(e);
}


/**
 * Allocates, initializes and returns an empty automaton
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
a->number_of_states_by_graphs=NULL;
a->variables=NULL;
return a;
}


/**
 * Frees a fst2. The function assumes that if 'fst2' is not NULL, all
 * its field are neither NULL nor already freed.
 */
void free_fst2(Fst2* fst2) {
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
   for (i=0;i<fst2->number_of_graphs;i++) {
     if (fst2->graph_names[i]!=NULL) free(fst2->graph_names[i]);
   }
   free(fst2->graph_names);
}
free(fst2->initial_states);
free(fst2->number_of_states_by_graphs);
free_variable_list(fst2->variables);
free(fst2);
}


/**
 * Readjusts the size of the arrays, because arrays have a
 * big default size.
 */
void resize(Fst2* fst2) {
#warning replace resize by calls to realloc with size*2
/*
 * NOTE: there were +1 on fst2->number_of_states and fst2->number_of_tags,
 *       but I removed them because it seemed to be unnecessary.
 *       S. Paumier
 */
fst2->states=(Fst2State*)realloc(fst2->states,fst2->number_of_states*sizeof(Fst2State));
fst2->tags=(Fst2Tag*)realloc(fst2->tags,fst2->number_of_tags*sizeof(Fst2Tag));
/*
 * We add +1 because we start the graph numerotation at 1 
 */
int n=fst2->number_of_graphs+1;
fst2->initial_states=(int*)realloc(fst2->initial_states,n*sizeof(int));
if (fst2->graph_names!=NULL) {
   /* We reallocate only if there is something */
   fst2->graph_names=(unichar**)realloc(fst2->graph_names,n*sizeof(unichar*));
}
fst2->number_of_states_by_graphs=(int*)realloc(fst2->number_of_states_by_graphs,n*sizeof(int));
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
e->number=0;
e->control=0;
e->input=NULL;
e->output=NULL;
e->inflected=NULL;
e->lemma=NULL;
e->codes=NULL;
e->matching_tokens=NULL;
e->number_of_matching_tokens=0;
e->compound_pattern=NO_COMPOUND_PATTERN;
/* $CD$ begin */
e->contentGF = NULL;
e->entryMasterGF = -1;
/* $CD$ end   */
return e;
}


/**
 * Analyzes the input of the tag of the form "<...>" . If some fields
 * need to be filled, the function fills them.
 * 
 * Examples: <be.V:P> => lemma="be", codes="V:P"
 *           <!MOT>   => negation=true
 */
void analyze_tag_with_angles(Fst2Tag e) {
unichar temp[2048];
int i,j;
j=0;
i=1;
if (e->input[i]=='!') {
  i++;
  /* We set the negation bit if there is the negation sign '!' */
  e->control=(char)(e->control|NEGATION_TAG_BIT_MASK);
}
/* Then, we look for a separator, copying what we find in 'temp' */
while ((e->input[i]!=',')&&(e->input[i]!='.')&&(e->input[i]!='>')) {
  temp[j++]=e->input[i++];
}
temp[j]='\0';
/*
 * If we find a closing angle, we have three cases: <build>, <V> or <MOT>
 */
if (e->input[i]=='>') {
	/* If we have a meta symbol, we do nothing */
	if (!u_strcmp_char(temp,"MOT")) {return;}
	if (!u_strcmp_char(temp,"DIC")) {return;}
	if (!u_strcmp_char(temp,"MAJ")) {return;}
	if (!u_strcmp_char(temp,"MIN")) {return;}
	if (!u_strcmp_char(temp,"PRE")) {return;}
	if (!u_strcmp_char(temp,"NB")) {return;}
	/* $CD$ begin */
	if (!u_strcmp_char(temp,"TOKEN")) {return;}
	/* $CD$ end   */
	/* 
	 * If we have a tag like <build> or <V>, we copy the content (build or V)
	 * into the inflected field of the tag. This is a pure convention that we use 
	 * because at this step, we cannot decide if we have a lemma or a grammatical
	 * code. This problem is supposed to be resolved by a later function.
	 */
	e->inflected=u_strdup(temp);
	return;
}
if (e->input[i]==',') {
	/*
	 * If we find a comma, we have a tag like <built,build.V> and we copy the
	 * sequence before the comma in the inflected field.
	 */
  	e->inflected=u_strdup(temp);
	i++;
	j=0;
	/* Then we look for the next separator */
	while ((e->input[i]!='.')&&(e->input[i]!='>')) {
    	temp[j++]=e->input[i++];
	}
	temp[j]='\0';
	/* A closing angle is an error since tags like <built,build> are not allowed */
	if (e->input[i]=='>') {
		/* We try to convert the invalid tag into ASCII to print it
		 * in the error message */
		char err[1024];
		u_to_char(err,e->input);
		fatal_error("Invalid label %s\n",err);
	}
	/* We copy the part between the comma and the point into the lemma field */
	e->lemma=u_strdup(temp);
	i++;
	j=0;
	while (e->input[i]!='>') {
		temp[j++]=e->input[i++];
	}
	temp[j]='\0';
	/* And we copy the remaining part of the tag (without the closing angle)
	 * into the codes field */
	e->codes=u_strdup(temp);
	return;
}
/*
 * If we find a point, we have a tag like <build.V>, so we copy the sequence
 * before the point into the lemma field.
 */
e->lemma=u_strdup(temp);
i++;
j=0;
while (e->input[i]!='>') {
	temp[j++]=e->input[i++];
}
temp[j]='\0';
/* And we copy the remaining part of the tag (without the closing angle)
 * into the codes field */
e->codes=u_strdup(temp);
}


/**
 * Analyzes the input of the tag of the form "{...}", different from the
 * sentence delimiter "{S}". At the opposite of the tags of the form "<...>",
 * a tag between round brackets must be of the form "{built,build.V}".
 */
void analyze_tag_with_round_brackets(Fst2Tag e) {
unichar temp[2048];
char err[2048];
int i,j;
j=0;
i=1;
/* We look for the comma */
while ((e->input[i]!=',')&&(e->input[i]!='}')) {
	temp[j++]=e->input[i++];
}
if (e->input[i]=='}') {
	/* If we find the closing bracket, it is an error */
	u_to_char(err,e->input);
	fatal_error("Invalid label %s: a tag must contain a valid DELAF line like {today,today.ADV}\n",err);
}
temp[j]='\0';
/* We copy the inflected form and we look for the point */
e->inflected=u_strdup(temp);
i++;
if (e->input[i]=='.') {
	/* If the lemma is an empty sequence, it means that the 
	 * lemma is identical to the inflected form, so we copy it*/
   e->lemma=u_strdup(e->inflected);
}
else {
	j=0;
	/* If the lemma is not empty, we copy it into temp */
	while ((e->input[i]!='.')&&(e->input[i]!='}')) {
		temp[j++]=e->input[i++];
	}
	temp[j]='\0';
	if (e->input[i]=='}') {
		/* If we find the closing bracket, it is an error */
		u_to_char(err,e->input);
		fatal_error("Invalid label %s: a tag must contain a valid DELAF line like {today,today.ADV}\n",err);
	}
	e->lemma=u_strdup(temp);
}
i++;
j=0;
/* Finally, we copy the remaining sequence (without the closing bracket) into
 * the lemma field */
while (e->input[i]!='}') {
	temp[j++]=e->input[i++];
}
temp[j]='\0';
e->codes=u_strdup(temp);
}


/**
 * This function creates and returns a tag for a variable declaration of the form
 * "$a(" or "$a)".
 * 
 * IMPORTANT: This function does not add the variable to the variable list of 
 *            the enclosing fst2.
 */
Fst2Tag create_variable_tag(unichar* input) {
int length=u_strlen(input);
Fst2Tag tag=new_Fst2Tag();
/*
 * We copy the variable name into the input field
 * length-1 = length - 2(ignoring '$' and '('or ')') + 1(for '\0') 
 */
tag->input=(unichar*)malloc((length-1)*sizeof(unichar));
for (int i=1;i<length;i++) {
	tag->input[i-1]=input[i];
}
tag->input[length-2]='\0';
/*
 * And we indicate if it is a variable start or end
 */
if (input[length-1]=='(') {tag->control=START_VAR_TAG_BIT_MASK;}
else {tag->control=END_VAR_TAG_BIT_MASK;}
return tag;
}


/**
 * This function creates and returns a fst2 tag. 'input', 'contentGF' and
 * 'output' and not supposed to be NULL. 'input' is not supposed to be an
 * empty string. After the execution, the field 'contentGF' of the created tag
 * can be NULL, but 'output' cannot. No output is indicated by an empty string.
 */
Fst2Tag create_tag(unichar* input,unichar* contentGF,unichar* output,int respect_case) {
int L=u_strlen(input);
if (input[0]=='$' && L>2 && (input[L-1]=='(' || input[L-1]==')')) {
   /* If we have a variable declaration like $a( or $a) */
   return create_variable_tag(input);
}
Fst2Tag tag=new_Fst2Tag();
tag->input=u_strdup(input);
/* First, we test if we have a context mark */
if (!u_strcmp_char(input,"$[")) {
   tag->control=POSITIVE_CONTEXT_MASK;
   return tag;
}
if (!u_strcmp_char(input,"$![")) {
   tag->control=NEGATIVE_CONTEXT_MASK;
   return tag;
}
if (!u_strcmp_char(input,"$]")) {
   tag->control=CONTEXT_END_MASK;
   return tag;
}
/*
 * If we have a morphological filter, we copy it
 */
/* $CD$ begin */
if (u_strlen(contentGF) > 0) {
    tag->contentGF=u_strdup(contentGF);
}
/* $CD$ end   */
/*
 * We copy the output, even if empty
 */
tag->output=u_strdup(output);
if (output[0]!=0) {
	/* But we set the output bit only if the output is not empty */
	tag->control=(unsigned char)(tag->control|TRANSDUCTION_TAG_BIT_MASK);
}
if (respect_case) {
	/* We set the case respect bit if necessary */
	tag->control=(unsigned char)(tag->control|RESPECT_CASE_TAG_BIT_MASK);
}
/*
 * If the input is ' ', # or <E>, we do nothing
 */
if ((input[0]==' ')||(input[0]=='#')||!u_strcmp_char(input,"<E>")) {
  //fst2->tags[position]=tag;
  return tag;
}
/*
 * We handle inputs that start with a round bracket
 */
if (input[0]=='{') {
  if (!u_strcmp_char(input,"{S}")) {
  	/*
  	 * If we have the sentence delimiter {S}, we do nothing.
  	 * IMPORTANT: the {STOP} tag MUST NOT be handled here, since
  	 *            we do not want it to be matched
  	 */
  }
  else {
    /* Here we a lexical tag like {built,build.V} or a single '{' character */
    if (input[1]!='\0') {
    	/* If we have not a single '{', we analyze the lexical tag */
       analyze_tag_with_round_brackets(tag);
    }
  }
  return tag;
}
if ((input[0]!='<') || (input[1]=='\0')) {
	/*
	 * If we have a single '<' character or an input that does not start with '<',
	 * there is nothing special to do.
	 */
	return tag;
}
/*
 * Finally, if we have an input that starts with '<' that is not a single '<',
 * we analyze the tag assuming that it is of the form <...>
 */
analyze_tag_with_angles(tag);
return tag;
}


/**
 * Creates and returns the tag for the string 'line', representing a tag line
 * in a .fst2 file like "%<V>/VERB", without an ending '\n'.
 */
Fst2Tag create_tag(unichar* line) {
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
if (input[0]=='\0') {u_strcpy_char(input,"<TOKEN>");}
return create_tag(input,filter,output,respect_case);
/* $CD$ end   */
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
int i;
unichar c;
unichar line[10000];
int current_tag=0;
/* If the position in the file is not correct we exit */
if (((c=(unichar)u_fgetc(f))!='%')&&(c!='@')) {
	fatal_error("Unexpected character in .fst2 file: %c\n",c);
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
  		fatal_error("Unexpected character in .fst2 file: %c\n",c);
  	}
	line[i]='\0';
	/* We create the tag and add it to the fst2 */
	fst2->tags[current_tag]=create_tag(line);
	/* 
	 * IMPORTANT: if the tag is a variable declaration, we must add this variable
	 *            to the variable list of the fst2 .
	 */
	if (fst2->tags[current_tag]->control & (START_VAR_TAG_BIT_MASK+END_VAR_TAG_BIT_MASK)) {
		fst2->variables=add_variable_to_list(fst2->tags[current_tag]->input,fst2->variables);
	}
	/* We do not forget to increase the tag counter */
	current_tag++;
}
/* Finally, we set the number of tags of the fst2 */
fst2->number_of_tags=current_tag;
}


/**
 * Reads all the tags of the .fst2 file 'f'.
 */
void read_fst2_tags(FILE *f,Fst2* fst2) {
read_fst2_tags(f,fst2,NO_TAG_LIMIT);
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
unichar c;
int value,negative_number;
/* We ignore spaces */
do {
	c=(unichar)u_fgetc(f);
} while (c==' ');
/* If 'c' is neither a digit nor a minus sign nor an end of line, we exit */
if (((c<'0')||(c>'9'))&&(c!='-')&&(c!='\n')) {
	fatal_error("Unexpected character in fst2: %c\n",c);
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
  if (c<'0' || c>'9') {fatal_error("Unexpected character in fst2: %c\n",c);}
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
 * Creates, initializes and returns a fst2 transition
 */
Fst2Transition new_Fst2Transition() {
Fst2Transition transition;
transition=(Fst2Transition)malloc(sizeof(struct fst2Transition));
if (transition==NULL) {
  fatal_error("Not enough memory in new_Fst2Transition\n");
}
/*
 * There is no sense to give default values to the
 * fields 'tag_number' and 'state_number'
 */
transition->next=NULL;
return transition;
}


/**
 * Creates and adds a transition to the given fst2 state.
 * 
 * NOTE: as a fst2 state is not supposed to contains duplicate 
 *       transitions, we do not check if the transition already
 *       exists before adding it.
 */
void add_transition_to_state(Fst2State state,int tag_number,int state_number) {
Fst2Transition transition=new_Fst2Transition();
transition->next=state->transitions;
transition->tag_number=tag_number;
transition->state_number=state_number;
state->transitions=transition;
}


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
unichar c;
int i,end_of_line,tag_number,destination_state_number,current_graph;
int current_state=0;
/* We read all the graphs that make the fst2 */
for (i=0;i<fst2->number_of_graphs;i++) {
	/* We ignore the minus sign that preceeds the number of the graph */
	u_fgetc(f);
	/* And we read the graph number */
	current_graph=u_read_int(f);
	/* We set the initial state of the graph */
	fst2->initial_states[current_graph]=current_state;
	int relative_state=0;
	/*
	 * We read the graph name
	 */
	int tmp=0;
	unichar graph_name[10000];
	while ((c=(unichar)u_fgetc(f))!='\n') {
		graph_name[tmp++]=c;
	}
	graph_name[tmp]='\0';
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
		if ((c!='t')&&(c!=':')) {fatal_error("Unexpected character in fst2: %c\n",c);}
		/*
		 * Then, we read the states of the graph, until we find a line beginning by 'f'.
		 */
		while (c!='f') {
			fst2->states[current_state]=new_Fst2State();
			fst2->number_of_states_by_graphs[current_graph]++;
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
			relative_state++;
		}
	}
	else {
		/*
		 * If we do not need to read this graph, then we just go to the 'f' that
		 * indicates the end of the graph.
		 */
		while(((c=(unichar)u_fgetc(f))!='f'));
	}
	/* We read the space and the '\n' that follows the final 'f' */
	u_fgetc(f);
	u_fgetc(f);
}
/* Finally, we set the number of states of the fst2 */
fst2->number_of_states=current_state;
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
Fst2* load_fst2(char* filename,int read_names,int graph_number) {
FILE* f;
f=u_fopen(filename,U_READ);
if (f==NULL) {
  fprintf(stderr,"Cannot open the file %s\n",filename);
  return NULL;
}
Fst2* fst2=new_Fst2();
/* We read the number of graphs contained in the fst2 */
fst2->number_of_graphs=u_read_int(f);
if (fst2->number_of_graphs==0) {
   fprintf(stderr,"Graph %s is empty\n",filename);
   return NULL;
}
/*
 * We allocates 'states' and 'tags' arrays with a big default size.
 */
fst2->states=(Fst2State*)malloc(MAX_FST2_STATES*sizeof(Fst2State));
if (fst2->states==NULL) {fatal_error("Not enough memory in load_fst2\n");}
fst2->tags=(Fst2Tag*)malloc(MAX_FST2_TAGS*sizeof(Fst2Tag));
if (fst2->tags==NULL) {fatal_error("Not enough memory in load_fst2\n");}
/*
 * The 'initial_states' and 'number_of_states_per_graphs' arrays are
 * allocated with the correct size. We add +1 because graph numeration
 * starts at 1.
 */
fst2->initial_states=(int*)malloc((fst2->number_of_graphs+1)*sizeof(int));
if (fst2->initial_states==NULL) {fatal_error("Not enough memory in load_fst2\n");}
fst2->number_of_states_by_graphs=(int*)malloc((fst2->number_of_graphs+1)*sizeof(int));
if (fst2->number_of_states_by_graphs==NULL) {fatal_error("Not enough memory in load_fst2\n");}
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
/*
 * Finally, we resize the 'states' and 'tags' array at the correct size.
 */
resize(fst2);
return fst2;
}


/**
 * Loads a .fst2 file and returns its representation in a Fst2 structure.
 */
Fst2* load_fst2(char* filename,int read_names) {
return load_fst2(filename,read_names,NO_GRAPH_NUMBER_SPECIFIED);
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



void unprotect_characters_in_sequence(unichar* s) {
int new_cursor=0;
int old_cursor=0;
while (s[old_cursor]!='\0') {
   if (s[old_cursor]=='\\') {old_cursor++;}
   if (s[old_cursor]=='\0') {
      fprintf(stderr,"ERROR: unprotected slash at the end of a sequence in unprotect_characters_in_sequence\n");
   }
   s[new_cursor++]=s[old_cursor++];
}
s[new_cursor]='\0';
}


//
// This function unprotects characters in non NULL flechi and canonique members
// of any tag of the given fst2
//
void unprotect_characters_in_fst2_tags(Fst2* fst2) {
Fst2Tag etiq;
for (int i=0;i<fst2->number_of_tags;i++) {
   etiq=fst2->tags[i];
   if (etiq!=NULL) {
      if (etiq->inflected!=NULL) {unprotect_characters_in_sequence(etiq->inflected);}
      if (etiq->lemma!=NULL) {unprotect_characters_in_sequence(etiq->lemma);}
   }
}
}
