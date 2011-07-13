/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
	int length_max;
};

static Grf* build_grf_from_sequences(U_FILE* text, Alphabet* alph);

const char* old_usage_Seq2Grf = "Usage: Seq2Grf [OPTIONS] <txt>\n"
	"\n"
	"  <txt>: a sequence text file, one sequence per line\n"
	"\n"
	"OPTIONS:\n"
	"  -a ALPH/--alphabet=ALPH: the alphabet file\n"
	"  -o GRF/--output=GRF: the .grf file to be produced\n"
	"  -h/--help: this help\n"
	"\n"
	"Builds a .grf that recognizes sequences contained in the input file.\n";

const char
		* usage_Seq2Grf =
				"Usage: Seq2Tfst [OPTIONS] <snt>\n"
					"\n"
					"  <snt> : the .snt text file\n"
					"\n"
					"OPTIONS:\n"
					"  -a ALPH/--alphabet=ALPH: the alphabet file\n"
					"  -c/---clean: cleans each sentence automaton, keeping best paths\n"
					"  -n XXX/--normalization_grammar=XXX: the .fst2 grammar used to normalize the text automaton\n"
					"  -t XXX/--tagset=XXX: use the XXX ELAG tagset file to normalize the dictionary entries\n"
					//		"  -K/--korean: tells Txt2Tfst that it works on Korean\n"
					"  -h/--help: this help\n"
					"\n"
					"Constructs the sequences automaton. If the sequences must be delimited\n"
					"with the special tag {S}. The result files\n"
					"named \"text.tfst\" and \"text.tind\" are stored is the text directory.\n"
					"\n"
					"Note that the program will also take into account the file \"tags.ind\", if any.\n";

static void usage() {
	u_printf("%S", COPYRIGHT);
	u_printf(usage_Seq2Grf);
}

///**
// * This function tries to read a sentence from the given file.
// * It stops when it finds the EOF, a "{S}" or when MAX_TOKENS_IN_SENTENCE
// * tokens have been read. All the tokens that compose the sentence are stored
// * into 'buffer', except the "{S}", if any. The number of these tokens is stored
// * in '*N'. '*total' contains the whole number of integers read from the file.
// *
// * The function returns 1 if a sentence was read; 0 otherwise.
// */
//int read_sentence(int* buffer,int *N,int *total,U_FILE* f,int SENTENCE_MARKER) {
//	*total=0;
//	*N=0;
//	u_printf("read_sentence\n");
//	if (f==NULL) {
//		u_printf("f NULL\n");
//		fatal_error("Cannot open file\n");
//	}else{
//		u_printf("f not null\n");
//	}
//
//
//	if (1!=fread(buffer,sizeof(int),1,f)) {
//		/* If we are at the end of the file */
//		u_printf("we are at the end of the file \n");
//		return 0;
//	}
//	if (buffer==NULL){
//		u_printf("buffer NULL\n");
//	}
//	u_printf("sentence read\n");
//	*total=1;
//	int length;
//	if (buffer[0]==SENTENCE_MARKER) {
//		/* If the text starts by a {S}, we don't want to stop there */
//		length=0;
//	} else {
//		length=1;
//	}
//	int control=-1;
//	while (length<MAX_TOKENS_IN_SENTENCE && 1==(control=(int)fread(buffer+length,sizeof(int),1,f)) && buffer[length]!=SENTENCE_MARKER) {
//		length++;
//		(*total)++;
//	}
//	if (length<MAX_TOKENS_IN_SENTENCE && control==1 && buffer[length]==SENTENCE_MARKER) {
//		(*total)++;
//	}
//	u_printf("read_sentence : done\n");
//
//	if (length==0) return 0;
//	*N=length;
//	return 1;
//}
///**
// * This function returns the number of space tokens that are in the
// * given buffer.
// */
//int count_non_space_tokens(const int* buffer,int length,int SPACE) {
//	int n=0;
//	for (int i=0;i<length;i++) {
//		if (buffer[i]!=SPACE) {
//			n++;
//		}
//	}
//	return n;
//}

const char* optstring_Seq2Grf = ":a:o:hk:q:";
const struct option_TS lopts_Seq2Grf[] = { { "alphabet", required_argument_TS,
		NULL, 'a' }, { "output", required_argument_TS, NULL, 'o' }, {
		"input_encoding", required_argument_TS, NULL, 'k' }, {
		"output_encoding", required_argument_TS, NULL, 'q' }, { "help",
		no_argument_TS, NULL, 'h' }, { NULL, no_argument_TS, NULL, 0 } };

int main_Seq2Grf(int argc, char* const argv[]) {
	if (argc == 1) {
		usage();
		return 0;
	}

	char alphabet[FILENAME_MAX] = "";
	char output[FILENAME_MAX] = "";
	char norm[FILENAME_MAX] = "";
	char tagset[FILENAME_MAX] = "";
	int is_korean = 0;
	int CLEAN = 0;
	char* fontname = NULL;
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
		case 'f':
			if (vars->optarg[0] == '\0') {
				fatal_error("You must specify a non empty font name\n");
			}
			fontname = strdup(vars->optarg);//=strdup("Times New Roman");
			if (fontname == NULL) {
				fatal_alloc_error("main_Tfst2Grf");
			}
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
			decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),	vars->optarg);
			break;
		case ':':
			if (index == -1)
				fatal_error("Missing argument for option -%c\n", vars->optopt);
			else
				fatal_error("Missing argument for option --%s\n",
						lopts_Txt2Tfst[index].name);
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
	u_printf("\n\n\ntext_tind : %s\n\n\n", text_tind);

	U_FILE* tind = u_fopen(BINARY, text_tind, U_WRITE);
	struct match_list* tag_list = NULL;
	load_DELA(&vec,dlf,tree);
	load_DELA(&vec,dlc,tree);
	u_printf("Loading %s...\n", tags_ind);
	U_FILE* tag_file = u_fopen(&vec, tags_ind, U_READ);
	if (tag_file != NULL) {
		tag_list = load_match_list(tag_file, NULL);
		u_fclose(tag_file);
	}
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
	u_printf("test\n");
	char text_grf[FILENAME_MAX];
	get_snt_path(argv[vars->optind], text_grf);
	U_FILE* grf = u_fopen(&vec, output, U_WRITE);
	if (grf == NULL) {
		u_fclose(f);
		fatal_error("Cannot create %s\n", output);
	}
	out = u_fopen(&vec,	output, U_WRITE);
	if (out == NULL) {
		error("Cannot create file %s\n", &output);
		u_fclose(text);
		free_alphabet(alph);
		return 1;
	}
	U_FILE* tfst = u_fopen(&vec, text_tfst, U_WRITE);
	u_printf("text_tfst 1 : %s\n", text_tfst);
	if (tfst == NULL) {
		u_fclose(f);
		fatal_error("Cannot create %s\n", text_tfst);
	}
	u_printf("text_tfst 2 : %s\n", text_tfst);

	u_printf("text_tfst 3 : %s\n", text_tfst);
	struct hash_table* form_frequencies = new_hash_table(
			(HASH_FUNCTION) hash_unichar, (EQUAL_FUNCTION) u_equal,
			(FREE_FUNCTION) free, NULL, (KEYCOPY_FUNCTION) keycopy);
	u_printf("BUILD_SEQUENCES_AUTOMATON\tSTART\n");
	build_sequences_automaton(f, tokens, alph, tfst, tind, CLEAN,
			form_frequencies);
	u_printf("BUILD_SEQUENCES_AUTOMATON\tSTOP\n");
	/* Finally, we save statistics */
	char tfst_tags_by_freq[FILENAME_MAX];
	char tfst_tags_by_alph[FILENAME_MAX];
	u_printf("test 1 \n");
	get_snt_path(argv[vars->optind], tfst_tags_by_freq);
	strcat(tfst_tags_by_freq, "tfst_tags_by_freq.txt");
	get_snt_path(argv[vars->optind], tfst_tags_by_alph);
	strcat(tfst_tags_by_alph, "tfst_tags_by_alph.txt");
	U_FILE* f_tfst_tags_by_freq = u_fopen(&vec, tfst_tags_by_freq, U_WRITE);
	u_printf("test 2 \n");
	if (f_tfst_tags_by_freq == NULL) {
		error("Cannot open %s\n", tfst_tags_by_freq);
	}
	U_FILE* f_tfst_tags_by_alph = u_fopen(&vec, tfst_tags_by_alph, U_WRITE);
	if (f_tfst_tags_by_alph == NULL) {
		error("Cannot open %s\n", tfst_tags_by_alph);
	}
	u_printf("test 3 \n");
	sort_and_save_tfst_stats(form_frequencies, f_tfst_tags_by_freq,
			f_tfst_tags_by_alph);
	u_printf("test 4 \n");
	u_fclose(f_tfst_tags_by_freq);
	u_fclose(f_tfst_tags_by_alph);
	u_printf("test 5 \n");
	free_hash_table(form_frequencies);
	//free_Ustring(text);
	u_fclose(tfst);
	u_fclose(tind);

	///##########
	int is_sequence_automaton = 1;
	int SENTENCE = 1;
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
	u_printf("test 6 \n");
	Grf* grfObj = sentence_to_grf(tfstFile, fontname, size,
			is_sequence_automaton);
	beautify(grfObj, alph);
	u_printf("test 7 \n");
	save_Grf(out, grfObj);
	free_Grf(grfObj);
	u_fclose(f);
	u_printf("build_sequence_automaton: done \n");
	u_fclose(text);
	u_fclose(out);
	free_alphabet(alph);
	free_OptVars(vars);
	return 0;

}

/**
 * This function builds the sequences automaton that correspond to the
 * sequences from the input file. It saves it into the given file.
 */
// txt -> tfst (objet) tfst2grf -> .grf, pas de fichier .tfst
void build_sequences_automaton(U_FILE* f, const struct text_tokens* tokens,
		const Alphabet* alph, U_FILE* out_tfst, U_FILE* out_tind,
		int we_must_clean, struct hash_table* form_frequencies) {
	// New Automaton
	Tfst* tfst = new_Tfst(NULL, NULL, 0);
	tfst->current_sentence = 1;
	tfst->automaton = new_SingleGraph();
	tfst->offset_in_chars = 0;
	tfst->offset_in_tokens = 0;
	tfst->tokens = new_vector_int(2);
	tfst->token_sizes = new_vector_int(0);

	int current_state = 0;
	// New Initial State and Final state
	add_state(tfst->automaton); /* initial state */

	set_initial_state(tfst->automaton->states[current_state]);
	//tfst->tokens
	current_state++;
	//	add_state(tfst->automaton); /* final   state */
	//	current_state++;


	struct string_hash* tags = new_string_hash(132);
	struct string_hash* tmp_tags = new_string_hash(132);
	unichar EPSILON_TAG[] = { '@', '<', 'E', '>', '\n', '.', '\n', '\0' };
	/* The epsilon tag is always the first one */
	get_value_index(EPSILON_TAG, tmp_tags);
	get_value_index(EPSILON_TAG, tags);
	// Structured data used to give a set of parameters to some functions
	// like explore_*_tree
	struct info INFO;
	INFO.tok = tokens;
	INFO.alph = alph;
	INFO.SPACE = tokens->SPACE;
	INFO.length_max = 0;
	int N = 2;
	int total = 2;
	int buffer[MAX_TOKENS_IN_SENTENCE];
	int i;
	int num_state;
	Ustring* tmp_states = new_Ustring();
	Ustring* states = new_Ustring();
	Ustring* foo = new_Ustring(1);
	u_fprintf(out_tfst, "0000000001\n");
	u_printf("Constructing Sequence automaton...\n");
	if (f == NULL) {
		u_printf("f NULL\n");
		fatal_error("Cannot open file\n");
	}
	// N : nb tokens dans 1 sequence
	// total : nb token (non- espace ?) dans toutes les sequences
	// n_nodes : nb de noeuds à ajouter pour chaque sequence
	// ? : nb etats dans l'automate ?
	int nbsentence = 0;
	int last_transitions[MAX_FST2_STATES][2];
	//remplacer read_sentence
	// .txt .alph
	// readline, tokenization...
	int n_added_states = 0;
	while (read_sentence(buffer, &N, &total, f, tokens->SENTENCE_MARKER)) {
		u_printf("\n\n");
		u_printf("##################################################\n");
		u_printf("\t\tNBSENTENCE : %d\n", nbsentence);
		u_printf("##################################################\n");

		INFO.buffer = buffer;
		if (N > INFO.length_max)
			INFO.length_max = N;
		int n_nodes = count_non_space_tokens(buffer, N, tokens->SPACE);
		for (i = 0; i < n_nodes; i++) {
			add_state(tfst->automaton);
		}
		while (buffer[N - 1] == tokens->SPACE)
			N = N - 1;
		///*while(tokens->token[buffer[N-1]]==".")*/ N=N-1;
		for (int il = 0; il < N; il++) {
			//			if (buffer[il]!= tokens->SPACE){
			vector_int_add(tfst->tokens, buffer[il]);
			int l = u_strlen(tokens->token[buffer[il]]);
			vector_int_add(tfst->token_sizes, l);
			u_strcat(foo, tokens->token[buffer[il]], l);
			u_printf("tokens->token[buffer[%d]]='%S'\n", il,
					tokens->token[buffer[il]]);
			//tfst->automaton->number_of_states++;
			//			}else{
			//				int l = u_strlen(tokens->token[buffer[il]]);
			//				u_strcat(foo,tokens->token[buffer[il]],l);
			//			}
		}
		tfst->text = u_strdup(foo->str);
		/* Transitions */
		for (i = 0; i < N; i++) {
			if (buffer[i] == tokens->SENTENCE_MARKER) {
			} else {
				if (buffer[i] != tokens->SPACE) {
					//tfst->token_content[current_state]=tokens->token[buffer[i]];
					//u_printf("1166 - tokens->token[buffer[%d]]=%S\n",i,tokens->token[buffer[i]]);
					//tfst->token_content[current_state]=tokens->token[buffer[i]];
					//							tokens->token[buffer[i]][tfst->token_sizes->tab[i]-1]);
					//					u_sprintf(states,"@STD\n@%S\n@%d.0.0-%d.%d.%d\n.\n",tokens->token[buffer[i]],i,i,
					//							tfst->token_sizes->tab[i]-1,
					//							get_length_in_jamo(tokens->token[buffer[i]][tfst->token_sizes->tab[i]-1],korean)-1);
					u_sprintf(tmp_states, "@STD\n@%S\n@%d.0.0-%d.%d.%d\n.\n",
							tokens->token[buffer[i]], 0, //i,
							0, //i,
							1, //tfst->token_sizes->tab[i]-1,
							1 //get_length_in_jamo(tokens->token[buffer[i]][tfst->token_sizes->tab[i]-1],korean)-1
					);
					//u_strcat(&(tags[current_state],tmp_states);
					u_strcat(states, tmp_states);
					u_printf(">>>>new_state<<<<<<\n");
					u_printf("@STD\n@%S\n@%d.0.0-%d.%d.%d\n.\n",
							tokens->token[buffer[i]], i, i, 1, //tfst->token_sizes->tab[i]-1,
							1 //get_length_in_jamo(tokens->token[buffer[i]][tfst->token_sizes->tab[i]-1],korean)-1
					);
					u_printf(">>>>>>>>><<<<<<<<<<\n");
					int tag_number = get_value_index(tmp_states->str, tmp_tags);
					//					if (tfst->automaton->number_of_states>(sizeof(tfst->automaton->states))){
					//						add_state(tfst->automaton);
					//					}
					if (i == 0) {
						Transition
								* trans =
										tfst->automaton->states[current_state]->outgoing_transitions;
						add_outgoing_transition(tfst->automaton->states[0],
								tag_number, current_state + 1);
						trans
								= tfst->automaton->states[current_state]->outgoing_transitions;
						//					while (trans!=NULL) {
						//						u_printf(" %d %d -",trans->tag_number,trans->state_number);
						//						trans=trans->next;
						//					}
						//					u_printf("\n");
					} else if (i == N - 1) {
						last_transitions[nbsentence][0] = current_state;
						last_transitions[nbsentence][1] = tag_number;
						//						add_outgoing_transition(tfst->automaton->states[current_state],	1000+tag_number,1);
						u_printf("\n");
					} else {
						//						add_state()
						Transition
								* trans =
										tfst->automaton->states[current_state]->outgoing_transitions;
						//						add_outgoing_transition(tfst->automaton->states[current_state],	tag_number,current_state+1);
						//				add_outgoing_transition(tfst->automaton->states[current_state],	tag_number,current_state+1);
						add_outgoing_transition(
								tfst->automaton->states[current_state],
								get_value_index(tmp_states->str, tmp_tags),
								current_state + 1);
						trans
								= tfst->automaton->states[current_state]->outgoing_transitions;
						if (trans == NULL) {
							u_printf("ERROR 3 : transition not added\n");
						}
						u_printf(" >");
						while (trans != NULL) {
							u_printf(" %d %d -", trans->tag_number,
									trans->state_number);
							trans = trans->next;
						}
						u_printf("\n");
					}
					current_state++;
				}
			}
		}
		nbsentence++;
	}
	add_state(tfst->automaton);
	for (i = 0; i < nbsentence; i++) {
		add_outgoing_transition(
				tfst->automaton->states[last_transitions[i][0]],
				last_transitions[i][1], current_state);
	}
	set_final_state(tfst->automaton->states[current_state]);
	//minimize(tfst->automaton,1);
	if (we_must_clean) {
		/* If necessary, we apply the "good paths" heuristic */
		keep_best_paths(tfst->automaton, tmp_tags);
	}
	remove_epsilon_transitions(tfst->automaton, 0);
	//trim(tfst->automaton,NULL);
	if (tfst->automaton->number_of_states == 0) {
		/* Case 1: the automaton has been emptied because of the tagset filtering */
		error("Sentence %d is empty\n", tfst->current_sentence);
		SingleGraphState initial = add_state(tfst->automaton);
		set_initial_state(initial);
		free_vector_ptr(tfst->tags, (void(*)(void*)) free_TfstTag);
		tfst->tags = new_vector_ptr(1);
		vector_ptr_add(tfst->tags, new_TfstTag(T_EPSILON));
		save_current_sentence(tfst, out_tfst, out_tind, NULL, 0, NULL);
	} else { /* Case 2: the automaton is not empty */
		/* We minimize the sentence automaton. It will remove the unused states and may
		 * factorize suffixes introduced during the application of the normalization tree. */

		minimize(tfst->automaton, 1);
		/* We explore all the transitions of the automaton in order to renumber transitions */
		for (i = 0; i < tfst->automaton->number_of_states; i++) {
			Transition* trans =
					tfst->automaton->states[i]->outgoing_transitions;
			while (trans != NULL) {
				/* For each tag of the graph that is actually used, we put it in the main
				 * tags and we use this index in the tfst transition */
				trans->tag_number = get_value_index(
						tmp_tags->value[trans->tag_number], tags);
				trans = trans->next;
			}
		}
	}
	minimize(tfst->automaton, 1);
	save_current_sentence(tfst, out_tfst, out_tind, tags->value, tags->size,
			NULL);
	if (tfst->automaton == NULL) {
		u_printf("tfstFile->automaton==NULL !!!\n");
	} else {
		u_printf("GOOD  : tfstFile->automaton=/=NULL !!!\n");
	}

	close_text_automaton(tfst);
	free_Ustring(foo);
	free_Ustring(states);
	free_Ustring(tmp_states);
}
/**
 * Tokenizes the given sequence, ignoring spaces.
 */
static vector_ptr* tokenize_sequence(unichar* seq, Alphabet* alph) {
	if (seq == NULL)
		return NULL;
	vector_ptr* tokens = new_vector_ptr();
	Ustring* tmp = new_Ustring(256);
	int i = 0;
	while (seq[i] != '\0') {
		empty(tmp);
		if (seq[i] == ' ') {
			i++;
		} else if (!is_letter(seq[i], alph)) {
			/* Not a letter ? We may have to protect special .grf chars */
			switch (seq[i]) {
			case '<':
				u_strcpy(tmp, "\\<");
				break;
			case '"':
				u_strcpy(tmp, "\\\\\\\"");
				break;
			case '\\':
				u_strcpy(tmp, "\\\\");
				break;
			case '+':
				u_strcpy(tmp, "\\+");
				break;
			case ':':
				u_strcpy(tmp, "\\:");
				break;
			case '/':
				u_strcpy(tmp, "\\/");
				break;
			default:
				u_strcat(tmp, seq[i]);
				break;
			}
			vector_ptr_add(tokens, u_strdup(tmp->str));
			i++;
		} else {
			while (is_letter(seq[i], alph)) {
				u_strcat(tmp, seq[i]);
				i++;
			}
			vector_ptr_add(tokens, u_strdup(tmp->str));
		}
	}
	free_Ustring(tmp);
	return tokens;
}

/**
 * Creates a grf from a sequence text file.
 */
static Grf* build_grf_from_sequences(U_FILE* text, Alphabet* alph) {
	Grf* grf = NULL;
	/* ici, conversion du tfst vers le grf */
	Ustring* line = new_Ustring(1024);
	while (EOF != readline(line, text)) {
		vector_ptr* tokens = tokenize_sequence(line->str, alph);
		error("line=%S\n", line->str);
		for (int i = 0; i < tokens->nbelems; i++) {
			error("  [%S]\n", tokens->tab[i]);
		}

		/* ajoute ici ton code de construction des états dans le tfst */
		free_vector_ptr(tokens, free);
	}
	free_Ustring(line);
	beautify(grf, alph);
	return grf;
}

static Grf* build_grf_from_snt(U_FILE* text, Alphabet* alph) {
	Grf* grf = NULL;

	return grf;
}
