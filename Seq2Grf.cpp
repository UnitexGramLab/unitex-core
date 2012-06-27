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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "Alphabet.h"
#include "Copyright.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "Grf_lib.h"
#include "GrfBeauty.h"
#include "Seq2Grf.h"
#include "Ustring.h"
#include "String_hash.h"
#include "File.h"
#include "SingleGraph.h"
#include "Sentence2Grf.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {


void build_sequence_automaton(char* txt,char* grf,Alphabet* alph,int only_stop,int max_wildcards,
		int n_delete,int n_insert,int n_replace,int beautify,VersatileEncodingConfig* vec);

const char* usage_Seq2Grf =
		"Usage: Seq2Grf [OPTIONS] <txt>\n"
		"\n"
		"  <txt> : the sequence text file\n"
		"\n"
		"OPTIONS:\n"
		"  -a ALPH/--alphabet=ALPH: the alphabet file\n"
		"  -o XXX/--output=XXX: the output GRF file\n"
		"  -s/--only-stop : only consider {STOP}-separated sequences\n"
		"  -b/--beautify: apply the grf beautifying algorithm\n"
		"  -n/--no_beautify: do not apply the grf beautifying algorithm (default)\n"
		"  -w n_wildcards\n"
		"  -i n_insertion\n"
		"  -r n_replace\n"
		"  -d n_delete\n"
		"  -h/--help: this help\n"
		"\n"
		"Constructs the sequence automaton : one single automaton that recognizes\n"
		"all the sequences from the given text file. The sequences must be delimited either\n"
		"with a newline or the special tag {STOP}. \n"
		;

static void usage() {
	u_printf("%S", COPYRIGHT);
	u_printf(usage_Seq2Grf);
}

const char* optstring_Seq2Grf = ":a:o:i:r:d:hbnk:q:w:sm";
const struct option_TS lopts_Seq2Grf[] = {
		{ "alphabet",required_argument_TS,    NULL, 'a' },
		{ "output", required_argument_TS, NULL, 'o' },
		{ "wildcards", required_argument_TS,NULL, 'w'},
		{ "insert", required_argument_TS,NULL, 'i'},
		{ "replace", required_argument_TS,NULL, 'r'},
		{ "delete", required_argument_TS,NULL, 'd'},
		{ "only-stop", no_argument_TS,NULL, 's'},
		{ "beautify", no_argument_TS,NULL, 'b'},
		{ "no_beautify", no_argument_TS,NULL, 'n'},
		{ "input_encoding", required_argument_TS, NULL, 'k' },
		{ "output_encoding", required_argument_TS, NULL, 'q' },
		{ "help", no_argument_TS, NULL, 'h' },
		{ NULL, no_argument_TS, NULL, 0 }
};

int main_Seq2Grf(int argc, char* const argv[]) {
if (argc==1) {
	usage();
	return 0;
}
char alphabet[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
char foo;
int max_wildcards=0,n_delete=0,n_replace=0,n_insert=0,only_stop=0,beautify=0;
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_Seq2Grf,lopts_Seq2Grf,&index,vars))) {
	switch (val) {
		case 'a': {
			if (vars->optarg[0] == '\0') {
				fatal_error("You must specify a non empty alphabet file name\n");
			}
			strcpy(alphabet, vars->optarg);
			break;
		}
		case 'o': {
			strcpy(output, vars->optarg);
			break;
		}
		case 'w': {
			if (1!=sscanf(vars->optarg,"%d%c",&max_wildcards,&foo)) {
				fatal_error("Invalid wildcards number argument: %s\n",vars->optarg);
			}
			break;
		}
		case 'i': {
			if (1!=sscanf(vars->optarg,"%d%c",&n_insert,&foo)) {
				fatal_error("Invalid insertions argument: %s\n",vars->optarg);
			}
			break;
		}
		case 'r': {
			if (1!=sscanf(vars->optarg,"%d%c",&n_replace,&foo)) {
				fatal_error("Invalid replacements argument: %s\n",vars->optarg);
			}
			break;
		}
		case 'd': {
			if (1!=sscanf(vars->optarg,"%d%c",&n_delete,&foo)) {
				fatal_error("Invalid deletions argument: %s\n",vars->optarg);
			}
			break;
		}
		case 's': {
			only_stop=1;
			break;
		}
		case 'b': {
			beautify=1;
			break;
		}
		case 'n': {
			beautify=0;
			break;
		}
		case 'h': {
			usage();
			return 0;
		}
		case 'k': {
			if (vars->optarg[0] == '\0') {
				fatal_error("Empty input_encoding argument\n");
			}
			decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input), vars->optarg);
			break;
		}
		case 'q': {
			if (vars->optarg[0] == '\0') {
				fatal_error("Empty output_encoding argument\n");
			}
			decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output), vars->optarg);
			break;
		}
		case ':': {
			if (index == -1)
				fatal_error("Missing argument for option -%c\n", vars->optopt);
			else
				fatal_error("Missing argument for option --%s\n",
						lopts_Seq2Grf[index].name);
			break;
		}
		case '?': {
			if (index == -1)
				fatal_error("Invalid option -%c\n", vars->optopt);
			else
				fatal_error("Invalid option --%s\n", vars->optarg);
			break;
		}
	}
	index=-1;
}
if (vars->optind!=argc-1) {
	fatal_error("Invalid arguments: rerun with --help\n");
}
if (output[0]=='\0') {
	fatal_error("Error: you must specify the output .grf file\n");
}
Alphabet* alph=NULL;
if (alphabet[0]!='\0' ) {
	alph=load_alphabet(&vec,alphabet,0);
}
build_sequence_automaton(argv[vars->optind],output,alph,only_stop,max_wildcards,n_delete,
		n_insert,n_replace,beautify,&vec);

free_alphabet(alph);
free_OptVars(vars);
return 0;
}


void read_letter_token(Ustring* tmp,unichar* line,unsigned int *i,Alphabet* alph) {
do {
	u_strcat(tmp,line[(*i)++]);
} while (is_letter(line[*i],alph));
}

const static char* special[]={"{S}",
		"<MIN>","<!MIN>",
		"<MAJ>","<!MAJ>",
		"<MOT>","<!MOT>",
		"<PRE>","<!PRE>",
		"<TOKEN>","<!TOKEN>",
		"<NB>",
		"<DIC>","<!DIC>",
		"<SDIC>","<!SDIC>",
		"<CDIC>","<!CDIC>",
		"<TDIC>","<!TDIC>",
		NULL
};


int read_special(vector_ptr* tokens,unichar* line,unsigned int *pos) {
for (int i=0;special[i]!=NULL;i++) {
	if (u_starts_with(line+(*pos),special[i])) {
		vector_ptr_add(tokens,u_strdup(special[i]));
		(*pos)+=strlen(special[i]);
		return 1;
	}
}
return 0;
}


vector_ptr* tokenize_sequence(Ustring* line,Alphabet* alph) {
vector_ptr* tokens=new_vector_ptr(32);
Ustring* tmp=new_Ustring(128);
unsigned int i=0;
while (i!=line->len) {
	empty(tmp);
	if (is_letter(line->str[i],alph)) {
		read_letter_token(tmp,line->str,&i,alph);
		vector_ptr_add(tokens,u_strdup(tmp->str));
	} else if (line->str[i]==' ' || line->str[i]=='\t'
			|| line->str[i]=='\r' || line->str[i]=='\n') {
		/* We ignore separators */
		i++;
	} else if (read_special(tokens,line->str,&i)) {
		/* Nothing to do */
	} else {
		/* We read a one-char token */
		u_strcat(tmp,line->str[i]);
		i++;
		vector_ptr_add(tokens,u_strdup(tmp->str));
	}
}
free_Ustring(tmp);
return tokens;
}


static unichar TOKEN[]={'<','T','O','K','E','N','>','\0'};

/**
 * Adds the given token sequence to the automaton
 */
void add_sequence(vector_ptr* seq,SingleGraph automaton,struct string_hash* tokens) {
int from_state=0,dest_state,token_number;
for (int i=0;i<seq->nbelems;i++) {
	dest_state=automaton->number_of_states;
	SingleGraphState s=add_state(automaton);
	if (i==(seq->nbelems-1)) {
		set_final_state(s);
	}
	token_number=get_value_index((unichar*)(seq->tab[i]),tokens);
	add_outgoing_transition(automaton->states[from_state],token_number,dest_state);
	from_state=dest_state;
}
}


/**
 * Adds the given token sequence to the automaton
 */
void add_sequence(unichar** seq,int size,SingleGraph automaton,struct string_hash* tokens) {
int from_state=0,dest_state,token_number;
for (int i=0;i<size;i++) {
	dest_state=automaton->number_of_states;
	SingleGraphState s=add_state(automaton);
	if (i==(size-1)) {
		set_final_state(s);
	}
	token_number=get_value_index((unichar*)(seq[i]),tokens);
	add_outgoing_transition(automaton->states[from_state],token_number,dest_state);
	from_state=dest_state;
}
}


/**
 * If there is less than 2 non <TOKEN> items and if the sequence length >=2,
 * then we consider that the sequence was too much transformed and so, we
 * return 0 to indicate that it should not be accepted; otherwise we return 1.
 */
int check_sequence(unichar* res[],int n,int size_origin) {
int n_tokens=0,n_lex;
for (int i=0;i<n;i++) {
	if (!u_strcmp(res[i],TOKEN)) {
		n_tokens++;
	}
}
n_lex=n-n_tokens;
if (n_lex<2 && size_origin>=2) return 0;
if (size_origin<=2 && n_tokens>0) return 0;
return 1;
}


void work(unichar* t[],int size,int current,int errors,int insert,int replace,int suppr,char last_op,
			unichar* res[],int pos_res,SingleGraph automaton,struct string_hash* tags) {
if (current==size) {
	/* We add the current sequence to the automaton, but only if it contains
	 * enough non <TOKEN> items
	 */
	if (!check_sequence(res,pos_res,size)) {
		return;
	}
	add_sequence(res,pos_res,automaton,tags);
	if (errors==0) {
		/* If we are done, we quit */
		return;
	}
	/* If we have reached the end of the array, we could only consider insertions,
	 * but the specifications forbids this */
	return;
}
/* Normal case */
res[pos_res]=t[current];
work(t,size,current+1,errors,insert,replace,suppr,0,res,pos_res+1,automaton,tags);
/* Now, we consider errors */
if (errors==0) return;
if (insert!=0 && last_op!='S' && pos_res>0) {
	/* We refuse insertions before the sequence */
	res[pos_res]=TOKEN;
	work(t,size,current,errors-1,insert-1,replace,suppr,'I',res,pos_res+1,automaton,tags);
}
if (suppr!=0 && last_op!='I') {
	work(t,size,current+1,errors-1,insert,replace,suppr-1,'S',res,pos_res,automaton,tags);
}
if (replace!=0) {
	res[pos_res]=TOKEN;
	work(t,size,current+1,errors-1,insert,replace-1,suppr,'R',res,pos_res+1,automaton,tags);
}
}


void process_sequence(Ustring* line,Alphabet* alph,int max_wildcards,int n_delete,int n_insert,
		int n_replace,SingleGraph automaton,struct string_hash* tokens) {
vector_ptr* tok=tokenize_sequence(line,alph);
unichar** variants=(unichar**)malloc(sizeof(unichar*)*(tok->nbelems+max_wildcards));
work((unichar**)(tok->tab),tok->nbelems,0,max_wildcards,n_insert,n_replace,n_delete,0,variants,0,automaton,tokens);
free_vector_ptr(tok,free);
free(variants);
}


void build_sequence_automaton(char* txt,char* output,Alphabet* alph,int only_stop,int max_wildcards,
		int n_delete,int n_insert,int n_replace,int do_beautify,VersatileEncodingConfig* vec) {
U_FILE* f=u_fopen(vec,txt,U_READ);
if (f==NULL) {
	fatal_error("Cannot open file %s\n",txt);
}
Ustring* line=new_Ustring(1024);
Ustring* seq=new_Ustring(1024);
SingleGraph automaton=new_SingleGraph(INT_TAGS);
/* We force state #0 to be the initial state */
SingleGraphState initial=add_state(automaton);
set_initial_state(initial);
struct string_hash* tokens=new_string_hash(128);
get_value_index(EPSILON,tokens);
int first_on_this_line=0,line_start;
while (EOF!=readline(line,f)) {
	/* If there is {STOP} tag, it is always a sequence delimiter */
	line_start=0;
	for (unsigned int i=0;i<line->len;i++) {
		if (u_starts_with(line->str+i,"{STOP}")) {
			/* If there is a previous buffer, we append the current
			 * part to the previous, with a \n if necessary
			 */
			if (first_on_this_line) {
				if (seq->len!=0) u_strcat(seq,"\n");
				first_on_this_line=0;
			}
			line->str[i]='\0';
			u_strcat(seq,line->str+line_start);
			process_sequence(seq,alph,max_wildcards,n_delete,n_insert,n_replace,automaton,tokens);
			empty(seq);
			i=i+5;
			line_start=i+1;
			continue;
		}
	}
	if (first_on_this_line) {
		if (seq->len!=0) u_strcat(seq,"\n");
		first_on_this_line=0;
	}
	u_strcat(seq,line->str+line_start);
	if (!only_stop) {
		process_sequence(seq,alph,max_wildcards,n_delete,n_insert,n_replace,automaton,tokens);
		empty(seq);
	}
	first_on_this_line=1;
}
process_sequence(seq,alph,max_wildcards,n_delete,n_insert,n_replace,automaton,tokens);
u_fclose(f);
free_Ustring(line);
free_Ustring(seq);
minimize(automaton,1);
Grf* grf=sentence_to_grf(automaton,tokens,12);
if (do_beautify) {
	beautify(grf,alph);
}
U_FILE* f_output=u_fopen(vec,output,U_WRITE);
if (f_output==NULL) {
	fatal_error("Cannot create file %s\n",output);
}
save_Grf(f_output,grf);
u_fclose(f_output);
free_Grf(grf);
free_string_hash(tokens);
free_SingleGraph(automaton,NULL);
}


} // namespace unitex
