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
#include "StringParsing.h"
#include "DELA_tree.h"
#include "DELA.h"
#include "BuildTextAutomaton.h"
#include "Sentence2Grf.h"
#include "String_hash.h"
#include "File.h"
#include "Tfst.h"
#include "Txt2Tfst.h"
#include "TfstStats.h"

#define STR_VALUE_MACRO(x) #x
#define STR_VALUE_MACRO_STRING(x) STR_VALUE_MACRO(x)

/**
 * This is an internal structure only used to give a set of parameters to some functions.
 */
struct info {
	const struct text_tokens* tok;
	const int* buffer;
	const Alphabet* alph;
	int SPACE;
	int TOKEN;
	int length_max;
};

const char
* usage_Seq2Grf =
		"Usage: Seq2Grf [OPTIONS] <snt>\n"
		"\n"
		"  <snt> : the .snt text file\n"
		"\n"
		"OPTIONS:\n"
		"  -a ALPH/--alphabet=ALPH: the alphabet file\n"
		"  -o XXX/--output=XXX: the output GRF ﬁle\n"
		"  -j n_jokers\n"
		"  -i n_insertion\n"
		"  -r n_replace\n"
		"  -d n_delete\n"
		"  -h/--help: this help\n"
		"\n"
		"Constructs the sequences automaton : one single automaton that recognizes\n"
		"all the sequences from the SNT. The sequences must be delimited with the\n"
		"special tag {S}. The result ﬁles, named text.tfst, text.tind and\n"
		"XXX, the GRF ﬁle are stored is the text directory.\n"
		"\n"
		;

static void usage() {
	u_printf("%S", COPYRIGHT);
	u_printf(usage_Seq2Grf);
}

const char* optstring_Seq2Grf = ":a:o:i:r:d:h:k:q:j:b:";
const struct option_TS lopts_Seq2Grf[] = {
		{ "alphabet",required_argument_TS,    NULL, 'a' },
		{ "output", required_argument_TS, NULL, 'o' },
		{ "jokers", required_argument_TS,NULL, 'j'},
		{ "insert", required_argument_TS,NULL, 'i'},
		{ "replace", required_argument_TS,NULL, 'r'},
		{ "delete", required_argument_TS,NULL, 'd'},
		{ "beautify", optional_argument_TS,NULL, 'b'},
		{ "input_encoding", required_argument_TS, NULL, 'k' },
		{ "output_encoding", required_argument_TS, NULL, 'q' },
		{ "help", no_argument_TS, NULL, 'h' },
		{ NULL, no_argument_TS, NULL, 0 }
};

int main_Seq2Grf(int argc, char* const argv[]) {
	if (argc == 1) {
		usage();
		return 0;
	}

	char alphabet[FILENAME_MAX] = "";
	char output[FILENAME_MAX] = "";
	char norm[FILENAME_MAX] = "";
	char tagset[FILENAME_MAX] = "";
	char foo;
	int n_op=0,n_sup=0,n_rep=0,n_ins=0;
	int is_korean = 0;
	int CLEAN = 0;
	char* fontname = NULL;
	int do_beautify=0;
	VersatileEncodingConfig vec = { DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT,
			DEFAULT_ENCODING_OUTPUT, DEFAULT_BOM_OUTPUT };
	int val, index = -1;
	struct OptVars* vars = new_OptVars();
	while (EOF != (val = getopt_long_TS(argc, argv, optstring_Seq2Grf,
			lopts_Seq2Grf, &index, vars))) {
		switch (val) {
		case 'a':
			if (vars->optarg[0] == '\0') {
				fatal_error("You must specify a non empty alphabet file name\n");
			}
			strcpy(alphabet, vars->optarg);
			u_printf("\t\talphabet : %s\n", alphabet);
			break;
		case 'c':
			CLEAN = 1;
			break;
		case 'n':
			if (vars->optarg[0] == '\0') {
				fatal_error(
						"You must specify a non empty normalization grammar name\n");
			}
			strcpy(norm, vars->optarg);
			u_printf("\t\tnorm : %d\n", norm);
			break;
		case 'o':
			strcpy(output, vars->optarg);
			u_printf("\t\toutput : %s\n", output);
			break;
			/////////////////////////////////////////////////
		case 'j':
			if (1!=sscanf(vars->optarg,"%d%c",&n_op,&foo)){
				fatal_error("Invalid jokers number argument: %s\n",vars->optarg);
			}
			break;
		case 'i':
			u_printf("option i \n");
			if (1!=sscanf(vars->optarg,"%d%c",&n_ins,&foo)) {
				fatal_error("Invalid insertions argument: %s\n",vars->optarg);
			}
			u_printf("vars->optarg = %s\n",vars->optarg);
			break;
		case 'r':
			u_printf("option r \n");
			if (1!=sscanf(vars->optarg,"%d%c",&n_rep,&foo)) {
				fatal_error("Invalid replacements argument: %s\n",vars->optarg);
			}
			u_printf("vars->optarg = %s\n",vars->optarg);
			break;
		case 'd':
			u_printf("option d \n");
			if (1!=sscanf(vars->optarg,"%d%c",&n_sup,&foo)) {
				fatal_error("Invalid deletions argument: %s\n",vars->optarg);
			}
			u_printf("vars->optarg = %s\n",vars->optarg);
			break;
		case 'b':
			u_printf("beautify \n");
			do_beautify = 1;
			break;
			/////////////////////////////////////////////////
		case 'f':
			if (vars->optarg[0] == '\0') {
				fatal_error("You must specify a non empty font name\n");
			}
			fontname = strdup(vars->optarg);//=strdup("Times New Roman");
			if (fontname == NULL) {
				fatal_alloc_error("main_Tfst2Grf");
			}
			break;
		case 't':
			if (vars->optarg[0] == '\0') {
				fatal_error("You must specify a non empty tagset file name\n");
			}
			strcpy(tagset, vars->optarg);
			break;
		case 'K':
			is_korean = 1;
			break;
		case 'h':
			usage();
			return 0;
		case 'k':
			if (vars->optarg[0] == '\0') {
				fatal_error("Empty input_encoding argument\n");
			}
			decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input), vars->optarg);
			break;
		case 'q':
			if (vars->optarg[0] == '\0') {
				fatal_error("Empty output_encoding argument\n");
			}
			decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),    vars->optarg);
			break;
		case ':':
			if (index == -1)
				fatal_error("Missing argument for option -%c\n", vars->optopt);
			else
				fatal_error("Missing argument for option --%s\n",
						lopts_Txt2Tfst[index].name);
			break;
		case '?':
			if (index == -1)
				fatal_error("Invalid option -%c\n", vars->optopt);
			else
				fatal_error("Invalid option --%s\n", vars->optarg);
			break;
		}

		index = -1;
	}

	if (vars->optind != argc - 1) {
		fatal_error("Invalid arguments: rerun with --help\n");
	}
	struct DELA_tree* tree = new_DELA_tree();
	char tokens_txt[FILENAME_MAX]; //
	char text_cod[FILENAME_MAX]; //
	char dlf[FILENAME_MAX]; //
	char dlc[FILENAME_MAX]; //
	char tags_ind[FILENAME_MAX]; //
	char text_tind[FILENAME_MAX];
	char text_tfst[FILENAME_MAX];
	char grf_name[FILENAME_MAX];
	char txt_name[FILENAME_MAX];
	char tok_name[FILENAME_MAX];
	get_snt_path(argv[vars->optind], text_tfst);
	strcat(text_tfst, "text.tfst");
	get_snt_path(argv[vars->optind], tokens_txt);
	strcat(tokens_txt, "tokens.txt");
	get_snt_path(argv[vars->optind], text_cod);
	strcat(text_cod, "text.cod");
	get_snt_path(argv[vars->optind], dlf);
	strcat(dlf, "dlf");
	get_snt_path(argv[vars->optind], dlc);
	strcat(dlc, "dlc");
	get_snt_path(argv[vars->optind], tags_ind);
	strcat(tags_ind, "tags.ind");
	get_snt_path(argv[vars->optind], text_tind);
	strcat(text_tind, "text.tind");
	U_FILE* tind = u_fopen(BINARY, text_tind, U_WRITE);
	struct match_list* tag_list = NULL;
	load_DELA(&vec,dlf,tree);
	load_DELA(&vec,dlc,tree);
	u_printf("Loading %s...\n", tags_ind);
	U_FILE* tag_file = u_fopen(&vec, tags_ind, U_READ);
	if (tag_file != NULL) {
		tag_list = load_match_list(tag_file,NULL,NULL);
		u_fclose(tag_file);
		tag_file =NULL;
	}
	get_path(argv[vars->optind],grf_name);
	get_path(argv[vars->optind],txt_name);
	get_path(argv[vars->optind],tok_name);
	if ((*output) == '\0') {
		strcat(grf_name, "cursentence.grf");
		strcat(txt_name, "cursentence.txt");
		strcat(tok_name, "cursentence.tok");
	} else {
		strcat(grf_name, output);
		strcat(grf_name, ".grf");
		strcat(txt_name, output);
		strcat(txt_name, ".txt");
		strcat(tok_name, output);
		strcat(tok_name, ".tok");
	}

	U_FILE* text=NULL;
	U_FILE* out;
	Alphabet* alph = NULL;
	if (alphabet[0] != '\0') {
		alph = load_alphabet(&vec,alphabet);
		if (alph == NULL) {
			error("Cannot load alphabet file %s\n", alphabet);
			u_fclose(text);
			return 1;
		}
	}

	text = u_fopen(&vec, argv[vars->optind], U_READ);
	if (text == NULL) {
		fatal_error("Cannot open text file %s\n", argv[vars->optind]);
	}
	struct text_tokens* tokens = load_text_tokens(&vec,tokens_txt);
	if (tokens == NULL) {
		fatal_error("Cannot open %s\n", tokens_txt);
	}
	U_FILE* f = u_fopen(BINARY, text_cod, U_READ);
	if (f == NULL) {
		fatal_error("Cannot open %s\n", text_cod);
	}
	char text_grf[FILENAME_MAX];
	get_snt_path(argv[vars->optind], text_grf);
	U_FILE* grf = u_fopen(&vec, output, U_WRITE);
	if (grf == NULL) {
		u_fclose(f);
		fatal_error("Cannot create %s\n", output);
	}
	out = u_fopen(&vec,    output, U_WRITE);
	if (out == NULL) {
		error("Cannot create file %s\n", &output);
		u_fclose(text);
		free_alphabet(alph);
		return 1;
	}
	U_FILE* tfst = u_fopen(&vec, text_tfst, U_WRITE);
	if (tfst == NULL) {
		u_fclose(f);
		fatal_error("Cannot create %s\n", text_tfst);
	}
	struct hash_table* form_frequencies = new_hash_table(
			(HASH_FUNCTION) hash_unichar,
			(EQUAL_FUNCTION) u_equal,
			(FREE_FUNCTION) free,
			NULL,
			(KEYCOPY_FUNCTION) keycopy);
//	u_printf("tokens :\n");
//	u_printf("%S \n",tokens->token[0]);
	u_printf("n_op=%d\n",n_op);
	u_printf("n_ins=%d\n",n_ins);
	u_printf("n_rep=%d\n",n_rep);
	u_printf("n_sup=%d\n",n_sup);
	build_sequences_automaton(f, tokens, alph, tfst, tind, CLEAN,form_frequencies,n_op,n_ins,n_rep,n_sup);
	//    /* Finally, we save statistics */
	char tfst_tags_by_freq[FILENAME_MAX];
	char tfst_tags_by_alph[FILENAME_MAX];
	get_snt_path(argv[vars->optind], tfst_tags_by_freq);
	strcat(tfst_tags_by_freq, "tfst_tags_by_freq.txt");
	get_snt_path(argv[vars->optind], tfst_tags_by_alph);
	strcat(tfst_tags_by_alph, "tfst_tags_by_alph.txt");
	U_FILE* f_tfst_tags_by_freq = u_fopen(&vec, tfst_tags_by_freq, U_WRITE);
	if (f_tfst_tags_by_freq == NULL) {
		error("Cannot open %s\n", tfst_tags_by_freq);
	}
	U_FILE* f_tfst_tags_by_alph = u_fopen(&vec, tfst_tags_by_alph, U_WRITE);
	if (f_tfst_tags_by_alph == NULL) {
		error("Cannot open %s\n", tfst_tags_by_alph);
	}
	sort_and_save_tfst_stats(form_frequencies, f_tfst_tags_by_freq,
			f_tfst_tags_by_alph);
	u_fclose(f_tfst_tags_by_freq);
	u_fclose(f_tfst_tags_by_alph);

	u_fclose(tfst);
	u_fclose(tind);
	int is_sequence_automaton = 1;
	int size = 10;

	u_printf("text_tfst : %s\n", text_tfst);
	Tfst* tfstFile = open_text_automaton(&vec,text_tfst);
	load_sentence(tfstFile, 1);
	if (tfstFile == NULL)
		u_printf("tfstFile==NULL !!! \n");
	if (tfstFile->automaton == NULL)
		u_printf("tfstFile->automaton==NULL !!! \n");
	if (tfstFile == NULL)
		u_printf("tfst NULL\n");
	Grf* grfObj = sentence_to_grf(tfstFile, fontname, size,
			is_sequence_automaton);
	if (do_beautify){
		u_printf("beautify = true\n");
		beautify(grfObj, alph);
	}
	else u_printf("beautify = false\n");
	save_Grf(out, grfObj);
	free_Grf(grfObj);
	free_hash_table(form_frequencies);
	u_fclose(tag_file);
	close_text_automaton(tfstFile);
	u_fclose(out);
	u_fclose(grf);
	u_fclose(f);
	free_text_tokens(tokens);
	u_fclose(text);
	free_alphabet(alph);
	free_DELA_tree(tree);
	free_OptVars(vars);
	return 0;
}
void add_path(Tfst * tfst,
		int seq[],
		int pos_res,
//		struct info INFO,
		const struct text_tokens* tokens,
		Ustring * text,
		int & current_state,
		struct string_hash *&tmp_tags ){
	u_printf("add_path2\t\t");
	for (int i=0;i<pos_res;i++){
		if (seq[i]==-1){
			u_strcat(text,"<TOKEN>");
			u_printf("<TOKEN> ");
			vector_int_add(tfst->tokens,1);
			vector_int_add(tfst->token_sizes, 7);
		}
		else{
			u_printf("%S ",tokens->token[seq[i]]);
			int ind=get_value_index(tokens->token[seq[i]],tmp_tags);
			if (ind!=0 && ind !=1){
				int l=u_strlen(tokens->token[seq[i]]);
				vector_int_add(tfst->tokens,ind);
				vector_int_add(tfst->token_sizes, l);
				u_strcat(text,tokens->token[seq[i]]);
				Ustring * _tag=new_Ustring();
				u_strcat(_tag, "@STD\n@");
				u_strcat(_tag, tokens->token[seq[i]]);
				u_strcat(_tag,"\n@0.0.0-0.1.1\n.\n");
				if (tmp_tags->value[ind] != NULL) {
					free(tmp_tags->value[ind]);
				}
				tmp_tags->value[ind]=(*_tag).str;
				// we want keep the buffer or (*_tag).str by calling free_Ustring
				(*_tag).str = NULL;
				free_Ustring(_tag);
			}
		}
		u_strcat(text," ");
	}
	u_printf("\t\t>>pos_res=%d\n",pos_res);
	bool linked = false;
	int n_nodes = pos_res;
	for (int i = 0; i < n_nodes; i++) {
		add_state(tfst->automaton);
	}
	tfst->text= text->str;
	Ustring* tmp_states = new_Ustring();
	Ustring* states = new_Ustring();
	int tmp_final_state = 1;
	for (int i = 0; i < pos_res; i++) {
		int k;
		if (seq[i]==-1) k=1;
		else k=get_value_index(tokens->token[seq[i]],tmp_tags);
		u_sprintf(tmp_states,"%S",tmp_tags->value[k]);
		int tag_number = get_value_index(tmp_states->str, tmp_tags);
		u_strcat(states, tmp_states->str);
		if (linked==false) {
			add_outgoing_transition(
					tfst->automaton->states[0],
					tag_number,
					current_state + 1);
			linked=true;
		} else if (i == pos_res - 1) {
			add_outgoing_transition(
					tfst->automaton->states[current_state],
					tag_number,
					tmp_final_state);
		} else {
			Transition * trans = tfst->automaton->states[current_state]->outgoing_transitions;
			if (	tfst->automaton->states[current_state]==NULL)
				u_printf("	tfst->automaton->states[current_state] = NULL \n");
			add_outgoing_transition(
					tfst->automaton->states[current_state],
					tag_number,
					current_state + 1);
			trans = tfst->automaton->states[current_state]->outgoing_transitions;
			if (trans == NULL) {
				u_printf("ERROR 3 : transition not added\n");
			}
		}
		current_state++;
	}
		free_Ustring(tmp_states);
		free_Ustring(states);
}

int work(	int t[],
		int size,		int current,
		int errors,		int insert,		int replace,		int suppr,
		char last_op,
		int res[],	//résultat
		int max_length,
		int pos_res,
		int & cur,
		Tfst * tfst,
		struct info INFO,
		const struct text_tokens* tokens,
		Ustring * text,
		int & current_state,
		struct string_hash* &tmp_tags
) {
		if (current==size) {
			//####################################
			// produce sequence
			//####################################
			u_printf("res =[");
			for (int i=0;i<pos_res;i++){
//				if(res[i]==-1) u_printf("%d:%s",res[i],"<TOKEN>");
//				else u_printf("%d:%S ",res[i],tokens->token[res[i]]);
				if(res[i]==-1) u_printf("%s ","<TOKEN>");
				else u_printf("%S ",tokens->token[res[i]]);
			}
			u_printf("]\n");
			add_path(
							tfst,
							res,
							pos_res,
//							INFO,
							tokens,
							text,
							current_state,
							tmp_tags );
					cur++;
					if (errors==0) {
						/* If we are done, we quit */
						return cur;
					}
					if (insert!=0 && last_op!='S') {
						res[pos_res]=INFO.TOKEN;
						//####################################
						// s'il reste un joker je l'ajoute au bout (utile ?)
						//####################################
						work(t,size,current,errors-1,insert-1,replace,suppr,'I',res,max_length,pos_res+1,cur//,total
								,tfst,	INFO,	tokens,	text,	current_state,
								tmp_tags);
					}
		return cur;
		}
		res[pos_res]=t[current];
		//####################################
		// ici j'ajoute le token courrant et je passe au suivant
		//####################################
		work(t,size,current+1,errors,insert,replace,suppr,0,res,max_length,pos_res+1,cur//,total
		,tfst,	INFO,	tokens,	text,	current_state,
		tmp_tags);
		/* Now, we consider errors */
		if (errors==0) return cur;
		if (insert!=0 && last_op!='S' /*&& pos_res>0*/) {
			//####################################
			//		insert
			//####################################
			res[pos_res]=INFO.TOKEN;
			work(t,size,current,errors-1,insert-1,replace,suppr,'I',res,max_length,pos_res+1,
					cur, tfst,	INFO,	tokens,	text,	current_state,
					tmp_tags);
		}
		if (suppr!=0 && last_op!='I') {
			//####################################
			//		suppr
			//####################################
			work(t,size,current+1,errors-1,insert,replace,suppr-1,'S',res,max_length,pos_res,
					cur ,tfst,	INFO,	tokens,	text,	current_state,
					tmp_tags);
		}
		if (replace!=0 /*&& pos_res>0*/) {
			//####################################
			//		replace
			//####################################
			res[pos_res]=INFO.TOKEN;
			work(t,size,current+1,errors-1,insert,replace-1,suppr,'R',res,max_length,pos_res+1,cur
					,tfst,	INFO,	tokens,	text,	current_state,
					tmp_tags);
		}
		return cur;
}

/**
 * This function builds the sequences automaton that correspond to the
 * sequences from the input file. It saves it into the given file.
 */

void build_sequences_automaton(U_FILE* f, const struct text_tokens* tokens,
		const Alphabet* alph, U_FILE* out_tfst, U_FILE* out_tind,
		int we_must_clean, struct hash_table* form_frequencies,
		int err,int insert, int replace, int suppr) {
	u_printf("build_sequences_automaton START\n");
	u_printf("err=%d\n",err);
	u_printf("insert=%d\n",insert);
	u_printf("replace=%d\n",replace);
	u_printf("suppr=%d\n",suppr);
	// New Automaton
	Tfst* tfst = new_Tfst(NULL, NULL, 0);
	tfst->current_sentence = 1;
	tfst->automaton = new_SingleGraph();
	tfst->offset_in_chars = 0;
	tfst->offset_in_tokens = 0;
	tfst->tokens = new_vector_int(2);
	// the line below is a temporary hack to ensure the result is always same with same input file
	tfst->tokens->tab[0] = tfst->tokens->tab[1] = 0;
	tfst->token_sizes = new_vector_int(0);
	int current_state = 0;
	// New Initial State
	add_state(tfst->automaton); /* initial state */
	set_initial_state(tfst->automaton->states[current_state]);
	current_state++;
	int final_state = 1;
	struct string_hash* tags = new_string_hash(132);
	struct string_hash* tmp_tags = new_string_hash(132);
	unichar EPSILON_TAG[] = { '@', '<', 'E', '>', '\n', '.', '\n', '\0' };
	unichar TOKEN_TAG[] = { '@','S','T','D','\n'
			,'@','<','T','O','K','E','N','>','\n'
			,'@','0','.','0','.','0','-','0','.','1','.','1','\n'
			,'.','\n'
			,'\0'};
	/* The epsilon tag is always the first one */
	get_value_index(EPSILON_TAG, tmp_tags);
	get_value_index(EPSILON_TAG, tags);
	get_value_index(TOKEN_TAG, tmp_tags);
	struct info INFO;
	INFO.tok = tokens;
	INFO.alph = alph;
	INFO.SPACE = tokens->SPACE;
	INFO.length_max = 0;
	INFO.TOKEN=-1;
	int N,total;
	int buffer[MAX_TOKENS_IN_SENTENCE];
	Ustring* foo = new_Ustring(1);
	u_printf(">>>here\n");
	u_fprintf(out_tfst, "0000000001\n");
	if (f == NULL) {
		u_printf("f NULL\n");
		fatal_error("Cannot open file\n");
	}
	u_printf("read_sentence :\n");
	while (read_sentence(buffer, &N, &total, f, tokens->SENTENCE_MARKER)) {
		u_printf("\tN=%d\n",N);
		u_printf("\tread_sentence : in\n");
		int nst=count_non_space_tokens(buffer, N, tokens->SPACE);
		int *non_space_buffer=new int[nst];
		int k=0;
		for (int i=0;i<N;i++){
			if (buffer[i]!=tokens->SPACE){
				non_space_buffer[k]=buffer[i];
				k++;
				u_printf("%S ", tokens->token[buffer[i]]);
			}
		}
		u_printf("\n");
		u_printf("buffer : \t");
		for (int i=0;i<N;i++){
			u_printf("%d ",buffer[i]);
		}
		u_printf("\nnon_space_buffer : \n");
		for (int i=0;i<nst;i++){
			u_printf("%d ",non_space_buffer[i]);
		}
		u_printf("\n");
		u_printf("EPS\n");
		int max_length=nst+insert;
		// sequences produites par dérivations :
//		unichar **res =new unichar*[max_length];

		int* res2 = (int*)calloc(max_length, sizeof(int));
		u_printf("max_length=%d+%d=%d\n",nst,insert,max_length);
		int curr=0;
		INFO.length_max=max_length;
//		int n_seq=work(non_space_buffer, nst,0, err,insert, replace,suppr,0, res,max_length,0,curr,//sequences,
//				tfst,	INFO,	tokens,	foo,	current_state,
//				tmp_tags);
		int n_seq=work(non_space_buffer, nst,0, err,insert, replace,suppr,0, res2,max_length,0,curr,//sequences,
				tfst,	INFO,	tokens,	foo,	current_state,
				tmp_tags);
		u_printf("work2 : done %d sequences produced \n",n_seq);
		free(res2);
//		delete [] res;
		delete [] non_space_buffer;
	}
	//    adding final state :
	u_printf("N=%d\n",N);
	for (int i=0;i<N;i++){
		u_printf("buffer[%d]=%d\n",i,buffer[i]);
	}
	add_state(tfst->automaton);
	// declaring it as final state
	u_printf("final_state : %d\n",final_state);
	set_final_state(tfst->automaton->states[final_state]);
	int tag_number = get_value_index(EPSILON_TAG, tmp_tags);
	u_printf("add_outgoing_transition(");
	u_printf("state(%d), ",final_state);
	u_printf("%d, ",tag_number);
	u_printf("%d)\n",final_state);
	u_printf("minimize\n");
	minimize(tfst->automaton,1);
	u_printf("minimize : done"
			"\n");
//	if (we_must_clean) {
//		/* If necessary, we apply the "good paths" heuristic */
//		keep_best_paths(tfst->automaton, tmp_tags);
//	}
	if (tfst->automaton->number_of_states == 0) {
		//        /* Case 1: the automaton has been emptied because of the tagset filtering */
		error("Sentence %d is empty\n", tfst->current_sentence);
		SingleGraphState initial = add_state(tfst->automaton);
		set_initial_state(initial);
		free_vector_ptr(tfst->tags, (void(*)(void*)) free_TfstTag);
		tfst->tags = new_vector_ptr(1);
		vector_ptr_add(tfst->tags, new_TfstTag(T_EPSILON));
		save_current_sentence(tfst, out_tfst, out_tind, NULL, 0, NULL);
	} else {
		/* Case 2: the automaton is not empty */
		/* We minimize the sentence automaton. It will remove the unused states and may
		 * factorize suffixes introduced during the application of the normalization tree. */

//		minimize(tfst->automaton, 1);
		/* We explore all the transitions of the automaton in order to renumber transitions */
		u_printf("tfst->automaton->number_of_states =%d\n",tfst->automaton->number_of_states);

		int k=0;
		for (int i = 0; i < tfst->automaton->number_of_states; i++) {
			Transition* trans =
					tfst->automaton->states[i]->outgoing_transitions;
			while (trans != NULL) {
				/* For each tag of the graph that is actually used, we put it in the main
				 * tags and we use this index in the tfst transition */
				trans->tag_number =
						get_value_index(tmp_tags->value[trans->tag_number], tags);
				trans = trans->next;
				k++;
			}
		}
	}
	u_printf("out\n");
//	minimize(tfst->automaton, 1);
	u_printf("\nsave_current_sentence(tfst, out_tfst, out_tind, tags->value, tags->size,NULL)\n");
	save_current_sentence(tfst, out_tfst, out_tind, tags->value, tags->size,
			NULL);
	foo->str=NULL;
	free_Ustring(foo);
	free_string_hash(tags);
	free_string_hash(tmp_tags);
	close_text_automaton(tfst);

}
