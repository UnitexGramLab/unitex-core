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

#include <vector>

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

const char * usage_Seq2Grf =
		"Usage: Seq2Tfst [OPTIONS] <snt>\n"
		"\n"
		"TEEEEEEEEEEST"
		"  <snt> : the .snt text file\n"
		"\n"
		"OPTIONS:\n"
		"  -a ALPH/--alphabet=ALPH: the alphabet file\n"
		"  -c/---clean: cleans each sentence automaton, keeping best paths\n"
		"  -n XXX/--normalization_grammar=XXX: the .fst2 grammar used to normalize the text automaton\n"
		"  -t XXX/--tagset=XXX: use the XXX ELAG tagset file to normalize the dictionary entries\n"
		//        "  -K/--korean: tells Txt2Tfst that it works on Korean\n"
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

const char* optstring_Seq2Grf = ":a:o:hk:q:";
const struct option_TS lopts_Seq2Grf[] = {
		{ "alphabet",required_argument_TS,    NULL, 'a' },
		{ "output", required_argument_TS, NULL, 'o' },
		{ "input_encoding", required_argument_TS, NULL, 'k' },
		{ "output_encoding", required_argument_TS, NULL, 'q' },
		{ "help", no_argument_TS, NULL, 'h' },
		{ NULL, no_argument_TS, NULL, 0 }
};

/*
 * pouvoir paramétrer seq2grf pour
 * 1 fonctionnement standard
 * 2 génerer un graphe de seq avec jokers :
 * 		pour tte seq de n termes : n sequences à n-1 termes
 * 								   n sequences avec 1 joker à la place d'un terme
 * 								   n-1 sequences avec un joker entre 2 termes
 */


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
		tag_list = load_match_list(tag_file, NULL);
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

	//////////////////////////////////////////////////////
	//////////////////////////////////////////////////////
	int n_op=1,n_replace=1,n_reduce=0,n_enlarge=0;
	//////////////////////////////////////////////////////
	//////////////////////////////////////////////////////
	build_sequences_automaton(f, tokens, alph, tfst, tind, CLEAN,form_frequencies, n_op,n_replace,n_reduce,n_enlarge);
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
	beautify(grfObj, alph);
	save_Grf(out, grfObj);
	free_Grf(grfObj);
	free_hash_table(form_frequencies);
	u_fclose(tag_file);
	close_text_automaton(tfstFile);
	free_DELA_tree(tree);
	u_fclose(out);
	u_fclose(grf);
	u_fclose(f);
	free_text_tokens(tokens);
	u_fclose(text);
	free_alphabet(alph);
	free_OptVars(vars);
	return 0;
}

int** concat_arrays(int N,int n,int **array_a, int a, int ** array_b,int b ){
	int** array_c=new int*[N];
	for (int i=0;i<N;i++){
		array_c[i]= new int[n];
	}
	for (int i=0;i<a;i++){
		array_c[i]=array_a[i];
	}
	for (int i=0;i<b;i++){
		array_c[i+a]=array_b[i];
	}
	return array_c;
}
/*
 * flatten an array of arrays of sequences
 * into an array of sequences
 * [[()()()][()()()()()][()()]]
 * =>
 * [()()()()()()()()()]
 */
int flatten_array(int*** buffers, int N,int** flatten_buffer){
	int s=0;
	u_printf("test 1\n");
	for(int i=0;i<N;i++){
		s+=sizeof(buffers[i])/sizeof(int);
	}
	u_printf("test 2\n");
	flatten_buffer= new int*[s];
	int k =0;
	for (int i=0;i<N;i++){
		u_printf("test\n");
		s=sizeof(buffers[i])/sizeof(int);
		u_printf("test\n");
		for (int j=0;j<s;j++){
			u_printf("toto\ts=%d\ti=%d\tj=%d\n",s,i,j);
			u_printf("buffers[i][j]=%d\t",buffers[i][j]);
			flatten_buffer[k]=buffers[i][j];
			k++;
		}
	}
	u_printf("test 3\n");
	return k;
}
//int** concat_arrays(int ** buffer1, int buffer2)

void build_reduced_buffer(int* buffer, int N,int **reduced_buffer, int n_reduce, int n_op){
	u_printf("[[[[[[[build_reduced_buffer]]]]]]]");
	for(int i=0;i<N-1;i++){
		u_printf("[%d]",buffer[i]);
	}
	u_printf("\n");
	u_printf("n_reduce = %d\tn_op = %d\tN=%d\tbuffer_length =%d\n",n_reduce,n_op,N,sizeof(buffer)/sizeof(int));
	if(n_reduce>0 && n_op>0){
		u_printf("\nGO\n");
		for (int i=0;i<N;i++){
			for (int j=0;j<N-1;j++){
				if(j<i){
					reduced_buffer[i][j]=buffer[j];
				}else if(j>i){
					reduced_buffer[i][j]=buffer[j+1];
				}else{
					if (j+1<N){
						reduced_buffer[i][j]=buffer[j+1];
					}
				}
				if(j==N-1){
					u_printf("buffer[%d]=%S\n\n",j,buffer[j]);
				}
			}
		}
	}

	u_printf("print reduced buffer :\n");
	u_printf("N=%d\n",N);
	for (int i=0;i<N;i++){
		u_printf("(%d)",buffer[i]);
	}
	u_printf("\n");
	u_printf("inside "
			"\tvoid build_reduced_buffer(int* buffer, int N,int **reduced_buffer, int n_reduce, int n_op){\n");
	for (int i=0;i<N;i++){
		u_printf("<");
		for (int j=0;j<N-1;j++){
			u_printf("%d ",reduced_buffer[i][j]);
		}
		u_printf(">\n");
	}

	u_printf("build_reduce_buffer : done\n");
}
//void build_reduced_buffer(int** buffer, int N,int ** reduced_buffer, int n_reduce, int n_op, struct info inf){
//	u_printf("\n\tEMPTY FUNCTION\n");
//}
void build_enlarged_buffer(int* buffer, int N, int **enlarged_buffer, int n_enlarge,int n_op){
	u_printf("build_enlarged_buffer\n");
	u_printf("n_enlarge=%d>0 && n_op=%d>0\n",n_enlarge,n_op);
	if(n_enlarge>0 &&n_op>0){
		//enlarged_buffer = new int*[N+1];
		for (int i=0;i<N;i++){
			enlarged_buffer[i] = new int[N-1];
			u_printf("%d ",buffer[i]);
		}
		for (int i=0;i<N;i++){
			for (int j=0;j<N+1;j++){
				if (j<i){
					enlarged_buffer[i][j]=buffer[j];
				}else if (j==i){
					enlarged_buffer[i][j]=-1;
				}else{
					if (i<N){
						enlarged_buffer[i][j]=buffer[j-1];
					}
				}
			}
		}
	}
	u_printf("inside \n\t"
			"void build_enlarged_buffer(int* buffer, int N, int** enlarged_buffer, int n_enlarge,int n_op){\n");
	for (int i=0;i<N;i++){
		u_printf("<");
		for (int j=0;j<N+1;j++){
			u_printf("%d ",enlarged_buffer[i][j]);
		}
		u_printf(">\n");
	}
	u_printf("\n");
}
void build_replaced_buffer(int* buffer, int N, int** replaced_buffer,int n_replace, int n_op){
	if(n_replace>0 && n_op>0){
		for (int i=0;i<N;i++){
			//		int k=0;
			for (int j=0;j<N;j++){
				if(i==j)	replaced_buffer[i][j]=-1;
				else		replaced_buffer[i][j]=buffer[j];
			}
		}
	}
}




// recursif
void build_derived_buffer(int** buffer,		//
		int N,								//
		int n_seq,
		int ** derived_buffer,				//
		int n_reduce,						//
		int n_enlarge,						//
		int n_replace,						//
		int n_op,							//
		struct info inf){
	u_printf("\n>>>>>>>>build_derived_buffer<<<<<<<<<<<\n");
	u_printf(	"\tn_reduce=%d\n\tn_enlarge=%d\n\tn_replace=%d\n\tn_op=%d\n",
			n_reduce,n_enlarge,n_replace,n_op);
	for (int i=0;i<N;i++){
		u_printf("%d",buffer[0][i]);
	}
	u_printf("\n");
	if (n_op>0){
		//		int** produced;
		u_printf("n_op=%d>0",n_op);
//REDUCE////////////////////////////
		if(
								n_reduce>0
//				false
		){
			//false){//
			u_printf("[[[[[[[[[[[REDUCE]]]]]]]]]]]\n");

			//			for (int i=0;i<N;i++){
			u_printf("n_reduce=%d>0\n",n_reduce);
			for (int i=0;i<N;i++){
				u_printf("%d ",buffer[0][i]);
			}
			u_printf("\n");
			int ** produced= new int*[N];
			for (int i=0;i<N;i++){
				produced[i]= new int[N-1];
			}
			for (int i=0;i<n_seq;i++){
				u_printf("\t\tderivation n°%d\n",i);
				for (int j= 0;j<N;j++){
					u_printf("%d-",buffer[i][j]);
				}
				u_printf("\n");
				build_reduced_buffer(	buffer[i], 	N, 	produced,			n_reduce, 						n_op);
				for (int j= 0;j<N;j++){
					u_printf("%d-",buffer[0][j]);
				}
				u_printf("\n");
				u_printf("\noutside\n");
				for (int j=0;j<N;j++){
					u_printf("[");
					for (int k=0;k<N;k++){
						u_printf("%d ",produced[j][k]);
					}
					u_printf("]\n");
				}
				u_printf("\nnow recursive call\n");
				//				derived += produced
				//				build_derived_buffer(	produced,	N,	derived_buffer,	n_reduce-1,n_enlarge, n_replace,n_op-1,	inf);
			}
			////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////
			derived_buffer = new int *[N];
			for (int i=0;i<N;i++){
				derived_buffer[i]= produced[i];
			}
						u_printf("N=%d\n",N);
						u_printf("test reduce delete produced[][]\n");
						for(int i=0;i<N;i++) delete produced[i];
						delete [] produced;
						u_printf("delete produced (reduced) ok\n");
			////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////
//			derived_buffer = concat_arrays(N,)
			u_printf("[[[[[[[[[[[[[[[[reduce : done]]]]]]]]]]]]]]\n");
		}else
			u_printf("n_reduce =0\n");
//ENLARGE////////////////////////////
		if(
								n_enlarge>0
//				false
		){
			u_printf("[[[[[[[[[[[ENLARGE]]]]]]]]]]]\n");

			u_printf("n_enlarge=%d>0\n",n_enlarge);
			u_printf("enlarge : produced = new int[%d]\n",N);
			////////////////////////////
			int ** produced2= new int*[N+1];
			for (int i=0;i<N;i++){
				produced2[i]= new int[N+1];
			}
			/////////////////////
			for (int i=0;i<n_seq;i++){
				//				u_printf("i=%d\n",i);
				//				for (int j= 0;j<N;j++){
				//					u_printf("%d.",j);
				//				}
				//				u_printf("\n");
				//				for (int j= 0;j<N;j++){
				//					u_printf("%d-",buffer[0][j]);
				//				}
				//				u_printf("\n");
				//				u_printf("\tN=%d\n\tn_enlarge=%d\n\tn_op=%d\n ",N,n_enlarge,n_op);
				build_enlarged_buffer(	buffer[i], 	N, 	produced2,				n_enlarge, 				n_op);
				//				derived += produced

				//build_derived_buffer(	produced,	N,n_seq,	derived_buffer,n_reduce,n_enlarge-1, n_replace,	n_op-1,	inf);

				//				u_printf("\noutside\n");
				for (int j=0;j<N;j++){
					u_printf("%d[",j);
					for (int k=0;k<N+1;k++){
						u_printf("%d ",produced2[j][k]);
					}
					u_printf("]\n");
				}
				u_printf("\n");
			}
			u_printf("now delete produced\n");
			u_printf("N=%d\n",N);
			////////////////////////////////////////////////////////////////			u_printf("test enlarge delete produced[][]\n");
			////////////////////////////////////////////////////////////////			u_printf("test enlarge delete produced[][]\n");
						for(int i=0;i<N;i++) {
							u_printf("i=%d\n",i);
							u_printf("%d[",i);
							for (int k=0;k<N+1;k++){
								u_printf("%d ",produced2[i][k]);
							}
							u_printf("]\n");
							free( produced2[i]);
//							delete produced2[i];

						}

						u_printf("delete produced 2\n");
						free( produced2);
//						delete [] produced;
						u_printf("[[[[[[[[[[[[[[[[enlarge : done]]]]]]]]]]]]]]\n");

			////////////////////////////////////////////////////////////////			u_printf("test enlarge delete produced[][]\n");
			////////////////////////////////////////////////////////////////			u_printf("test enlarge delete produced[][]\n");
			u_printf("[[[[[[[[[[[[[[[[enlarge : done]]]]]]]]]]]]]]\n");

		}
		//REPLACE///////////////////////
		u_printf("n_replace=%d\n",n_replace);
		if(
				n_replace>0
				//				false
		){
			u_printf("[[[[[[[[[[[REPLACE]]]]]]]]]]]\n");
			u_printf("1\n");
			int**
				produced= new int*[N];
			u_printf("1\n");
			for (int i=0;i<N;i++){
				produced[i]= new int[N];
			}
			for (int i=0;i<N;i++){
				u_printf("%d ",buffer[0][i]);
			}
			u_printf("\n");
			//				derived += produced
			for (int i=0;i<n_seq;i++){
				build_replaced_buffer(	buffer[i], 	N, 	produced,							n_replace, 	n_op);
				//				build_derived_buffer(	produced,	N,n_seq,	derived_buffer,n_reduce,n_enlarge, 	n_replace-1,n_op-1,	inf);
			}
			//			for(int i=0;i<N;i++) delete produced[i];
			//			delete [] produced;
			for (int j=0;j<N;j++){
				u_printf("[");
				for (int k=0;k<N;k++){
					u_printf("%d ",produced[j][k]);
				}
				u_printf("]\n");
			}
			u_printf("[[[[[[[[[[[[[[[[replace: done]]]]]]]]]]]]]]\n");
		}

		//		u_printf("N=%d",N);
		//		u_printf("test reduce delete produced[][]\n");
		//		for(int i=0;i<N;i++) delete produced[i];
		//		delete [] produced;
		//		u_printf("delete produced (reduced) ok\n");
	}
	else{
		u_printf("n_op=0\n");

	}
	u_printf("derived_buffer inside\n");
	for (int i=0;i<N;i++){
		u_printf("[");
		for (int j=0;j<N;j++){
			u_printf("%d ",derived_buffer[i][j]);
		}
		u_printf("]\n");
	}

	u_printf("build_derived_buffer out\n");
}

void remove_space_tokens(int buffer[],int N,int space_token_id, int* &cleaned_buffer){

	int n_ns_tok= count_non_space_tokens(buffer, N, space_token_id);
	cleaned_buffer = new int[n_ns_tok];
	//	u_printf("space_token_id : %d\n",space_token_id);
	int k=0;
	u_printf("\nremove_space_tokens\n");
	for (int i=0;i<N;i++){
		u_printf("(%d)",buffer[i]);
		if(buffer[i]!=space_token_id){
			cleaned_buffer[k]=buffer[i];
			u_printf("[%d]",cleaned_buffer[k]);
			k++;
		}
	}
	u_printf("\n\n");
	//	for (int i=0;i<n_ns_tok;i++){
	//		u_printf("[%d]",cleaned_buffer[i]);
	//	}
}
/**
 * This function builds the sequences automaton that correspond to the
 * sequences from the input file. It saves it into the given file.
 */
void build_sequences_automaton(U_FILE* f, const struct text_tokens* tokens,
		const Alphabet* alph, U_FILE* out_tfst, U_FILE* out_tind,
		int we_must_clean, struct hash_table* form_frequencies,
		int n_op, int n_reduce, int n_enlarge, int n_replace
) {
	u_printf("build_sequences_automaton START\n");
	u_printf("n_op=%d\n",n_op);
	//	u_printf("%d",info.SPACE);
	///////////////////////////////////////
	// New Automaton
	///////////////////////////////////////
	Tfst* tfst = new_Tfst(NULL, NULL, 0);
	tfst->current_sentence = 1;
	tfst->automaton = new_SingleGraph();
	tfst->offset_in_chars = 0;
	tfst->offset_in_tokens = 0;
	tfst->tokens = new_vector_int(2);
	tfst->token_sizes = new_vector_int(0);

	//	bool do_replace=true, do_reduce=true, do_enlarge=true;
	int current_state = 0;
	///////////////////////////////////////
	// New Initial State
	///////////////////////////////////////
	add_state(tfst->automaton); /* initial state */
	set_initial_state(tfst->automaton->states[current_state]);
	int initial_state = current_state;
	u_printf("current_state : %d\n",current_state);
	u_printf("intitial_state : %d\n",initial_state);

	current_state++;
	int tmp_final_state = 1;
	u_printf("current_state : %d\n",current_state);

	struct string_hash* tags = new_string_hash(132);
	struct string_hash* tmp_tags = new_string_hash(132);
	unichar EPSILON_TAG[] = { '@', '<', 'E', '>', '\n', '.', '\n', '\0' };
	/* The epsilon tag is always the first one */
	get_value_index(EPSILON_TAG, tmp_tags);
	get_value_index(EPSILON_TAG, tags);
	struct info INFO;
	INFO.tok = tokens;
	INFO.alph = alph;
	INFO.SPACE = tokens->SPACE;
	INFO.length_max = 0;
	int N = 2;
	int total = 2;
	int buffer[MAX_TOKENS_IN_SENTENCE];
	Ustring* tmp_states = new_Ustring();
	Ustring* states = new_Ustring();
	Ustring* foo = new_Ustring(1);
	u_fprintf(out_tfst, "0000000001\n");
	if (f == NULL) {
		u_printf("f NULL\n");
		fatal_error("Cannot open file\n");
	}
	int nbsentence = 0;
	//    int n_added_states = 0;
	bool linked;

	///////////////////////////////////////
	// While there is a sequence to read
	///////////////////////////////////////

	while (read_sentence(buffer, &N, &total, f, tokens->SENTENCE_MARKER)&& count_non_space_tokens(buffer, N, tokens->SPACE)>0) {
		u_printf("///////////////////////////////////////////////\n");
		u_printf("While : Start\tsentence = %d\n",nbsentence);
		u_printf("///////////////////////////////////////////////\n");
		//buffers / sequences
		linked = false;
		INFO.buffer = buffer;
		u_printf("N=%d\n",N);
		if (N > INFO.length_max)
			INFO.length_max = N;
		u_printf("test 1\n");
		int n_nodes = count_non_space_tokens(buffer, N, tokens->SPACE);
		u_printf("n_nodes : %d\n",n_nodes);
		u_printf(" test 2 : n_nodes=%d\n",n_nodes);
		int *clean_buffer =new int[n_nodes];

		///////////////////////////////////////
		// Enlarged sets of sequences
		// derived from the current sequence
		///////////////////////////////////////
		//		bool reduced = _reduced;
		//		bool replaced = _replaced;
		//		bool enlarged = _enlarged;
		//		if( reduced){
		remove_space_tokens(buffer, N, tokens->SPACE, clean_buffer);
		//		u_printf("\n\n");
		//		u_printf("remove_space_tokens:done\n");
		n_nodes = count_non_space_tokens(buffer, N, tokens->SPACE);
		u_printf("there's %d non space tokens\n", n_nodes);
		//		u_printf(" test 2 : n_nodes=%d\n",n_nodes);
		//
		//		u_printf("\n");
		//		for(int i=0;i<n_nodes;i++){
		//			u_printf("[%d:%S]",clean_buffer[i],tokens->token[clean_buffer[i]]);
		//		}
		//		u_printf("\n");

		//		u_printf("n_nodes = %d\n",n_nodes);
		//		u_printf("tokens->SPACE = %d\n",tokens->SPACE);
		//		u_printf("tokens->token[tokens->SPACE] ='%S'\n",tokens->token[tokens->SPACE] );
		//
		//		u_printf("tokens->token[0] ='%S'\n",tokens->token[0] );
		//		u_printf("tokens->token[1] ='%S'\n",tokens->token[1] );
		//		removing one token
		//		replacing one token
		//				int** reduced_buffer[N][N-1];
		//		u_printf("========================================\n");
		//		u_printf("clean_buffer :\n");
		//		for(int i=0;i<n_nodes;i++){
		//			u_printf("[%d:%S]",clean_buffer[i],tokens->token[clean_buffer[i]]);
		//		}
		//		u_printf("\n\n");
		//		int **reduced_buffer= new int* [N];
		//		for (int i = 0 ; i < N; i++){
		//			reduced_buffer[i] = new int [n_nodes-1];
		//		}
		//		u_printf("build_approx_buffer :\n");
		//
		//
		//
		//
		//		int ****approx_buffer= new int*** [3];
		//		approx_buffer[0]= new int**[n_reduce];
		//		int k=1;
		//		for (int i=0;i<n_reduce;i++){
		//			k=k*(N-i);
		//			approx_buffer[0][i]= new int*[k];
		//		}
		//		approx_buffer[1]=new int**[n_replace];
		//		approx_buffer[2]= new int**[n_enlarge];
		//		for (int i = 0 ; i < 3; i++){
		//			approx_buffer[i] = new int* [n_nodes-1];
		//		}

		//		u_printf("build approx buffer : \n");
		//
		//		build_approx_buffer(clean_buffer,n_nodes,approx_buffer,n_reduce, n_enlarge, n_replace,n_op, INFO);

		//		u_printf("build approx buffer : done \n\n\n");
		//		build_reduced_buffer(buffer, N,reduced_buffer,n_nodes,INFO);
		//		u_printf("build_reduced_buffer :\n");
		//		build_reduced_buffer(clean_buffer, n_nodes,reduced_buffer,n_reduce,n_op);
		//		u_printf("build_reduced_buffer : OK\n");
		//		if (replaced){
		//		int **replaced_buffer= new int* [N];
		//		for (int i = 0 ; i < N; i++){
		//			replaced_buffer[i] = new int [n_nodes];
		//		}
		//		build_replaced_buffer(buffer,N,replaced_buffer,INFO);
		//		build_replaced_buffer(clean_buffer,n_nodes,replaced_buffer,n_replace, n_op);
		//		}
		//		if(enlarged){
		//		int **enlarged_buffer= new int* [N];
		//		for (int i = 0 ; i < N; i++){
		//			enlarged_buffer[i] = new int [n_nodes+1];
		//		}
		//		build_enlarged_buffer(clean_buffer,n_nodes,enlarged_buffer,n_enlarge,n_op);
		u_printf("n_enlarge=%d\n\n\n\n",n_enlarge);
		int **derived_buffer = new int *[N+n_enlarge];
		for (int i=0;i<N;i++){
			derived_buffer[i]=new int[N+n_enlarge];
		}
		int ** buffers = new int *[1];
		buffers[0]= new int[N];
		u_printf("buffers init\n");
		for(int i=0; i<N;i++){
			buffers[0][i]=buffer[i];
			u_printf("%d ",buffers[0][i]);
		}


		u_printf("\nbuild_derived_buffer\n");
		int n_seq =1;
		build_derived_buffer(buffers, N,n_seq,derived_buffer,n_reduce,n_enlarge,n_replace,n_op,INFO);
		for (int i=0;i<N;i++){
			u_printf("[");
			for (int j=0;j<N;j++){
				u_printf("%d",derived_buffer[i][j]);
			}
			u_printf("]\n");
		}

		u_printf("build_derived_buffer : done\n");
		int n_derived_sequences=0;

		/////////////////////////////////////
		// check
		/////////////////////////////////////
		//		u_printf("\nbuffer\n");
		//		for (int i=0;i<N;i++){
		//			if (buffer[i]==tokens->SPACE) 	u_printf("_");
		//			else 							u_printf("%S",tokens->token[buffer[i]]);
		//		}
		//		u_printf("\n\n>>>>>reduced<<<<<\n");
		//		for (int i=0;i<n_nodes;i++){
		//			for(int j=0;j<n_nodes-1;j++){
		//				if (reduced_buffer[i][j]==tokens->SPACE) 	u_printf("_ ");
		//				else 										u_printf("%S ",tokens->token[reduced_buffer[i][j]]);
		//			}
		//			u_printf("\n");
		//		}
		//		u_printf("\n>>>>>replaced<<<<<\n");
		//		for (int i=0;i<n_nodes;i++){
		//			for(int j=0;j<n_nodes;j++){
		//				if (replaced_buffer[i][j]==tokens->SPACE)	u_printf("_ ");
		//				else if(replaced_buffer[i][j]==-1) 			u_printf("<E> ");
		//				else 										u_printf("%S ",tokens->token[replaced_buffer[i][j]]);
		//			}
		//			u_printf("\n");
		//		}
		//		u_printf("\n>>>>>enlarged<<<<<\n");
		//		for (int i=0;i<n_nodes+1;i++){
		//			for(int j=0;j<n_nodes+1;j++){
		//				if (enlarged_buffer[i][j]==tokens->SPACE)	u_printf("_ ");
		//				else if(enlarged_buffer[i][j]==-1)			u_printf("<E> ");
		//				else										u_printf("%S ",tokens->token[enlarged_buffer[i][j]]);
		//			}
		//			u_printf("\n");
		//		}
		//		u_printf("enlarged_buffer[%d][%d]=%S\n",n_nodes,n_nodes,tokens->token[enlarged_buffer[n_nodes][n_nodes]]);
		u_printf("========================================\n");
		///////////////////////////////////////
		// Count of the number of states to create
		///////////////////////////////////////
		// Sequences done
		//		int** buffer2D=new int*[n_nodes];
		//Automaton
		///////////////////////////////////////
		//	Adding States
		///////////////////////////////////////
		u_printf("Adding States !\t");
		for (int i = 0; i < n_nodes; i++) {
			add_state(tfst->automaton);
		}
		while (N>0 && buffer[N - 1] == tokens->SPACE){
			N = N - 1;
		}
		for (int il = 0; il < N; il++) {
			vector_int_add(tfst->tokens, buffer[il]);
			int l = u_strlen(tokens->token[buffer[il]]);
			vector_int_add(tfst->token_sizes, l);
			u_strcat(foo, tokens->token[buffer[il]], l);
		}
		tfst->text= foo->str;

		///////////////////////////////////////
		//	Adding Transitions
		///////////////////////////////////////
		u_printf("Adding Transitions !\t");
		for (int i = 0; i < N; i++) {
			if (buffer[i] == tokens->SENTENCE_MARKER) {
				u_printf(">>>#>>>\ti = %d ET buffer[%d] = %s\n",i,i,tokens->SENTENCE_MARKER);
			} else {
				if (buffer[i] != tokens->SPACE) {
					unichar* token=tokens->token[buffer[i]];
					u_sprintf(tmp_states, "@STD\n@%S\n@%d.0.0-%d.%d.%d\n.\n",
							token, 0, 0, 1, 1 );
					u_strcat(states, tmp_states);
					int tag_number = get_value_index(tmp_states->str, tmp_tags);
					if (linked==false) {
						Transition * trans = tfst->automaton->states[current_state]->outgoing_transitions;
						add_outgoing_transition(
								tfst->automaton->states[0],
								tag_number, current_state + 1);
						linked=true;
					} else if (i == N - 1) {
						//						u_printf("Last word Transition : \t");
						Transition *trans = tfst->automaton->states[current_state]->outgoing_transitions;
						add_outgoing_transition(
								tfst->automaton->states[current_state],
								tag_number, tmp_final_state);
					} else {
						Transition * trans = tfst->automaton->states[current_state]->outgoing_transitions;
						add_outgoing_transition(tfst->automaton->states[current_state],
								tag_number, current_state + 1);
						trans = tfst->automaton->states[current_state]->outgoing_transitions;
					}
					current_state++;
				}
			}
		}
		u_printf("nbstates =%d/%d\n",current_state,tfst->automaton->number_of_states);
		//		delete [] *reduced_buffer;
		//		delete [] reduced_buffer;
		//
		//		delete [] *replaced_buffer;
		//		delete [] replaced_buffer;
		//
		//		delete [] *enlarged_buffer;
		//		delete [] enlarged_buffer

		u_printf("DELETES !!!!!\n");
		u_printf("test reduced\n");
		//		for (int i = 0; i < N; i++) {
		//			delete[] reduced_buffer[i] ;
		//		}
		//		delete[] reduced_buffer;
		//
		//		for (int i = 0; i < N; i++) {
		//			delete[] replaced_buffer[i] ;
		//		}
		//		delete[] replaced_buffer;
		//		//			free(replaced_buffer);
		//		for (int i = 0; i < N; i++) {
		//			delete[] enlarged_buffer[i] ;
		//		}
		//		delete[] enlarged_buffer;
		//		for (int i = 0; i < N; i++) {
		//			delete clean_buffer[i] ;
		//		}

		//		for( int i=0;i<N;i++){
		//			delete [] approx_buffer[i];
		//		}
		//		delete [] approx_buffer;
		delete [] clean_buffer;
		nbsentence++;
		u_printf("DELETE OK !!\n");
		u_printf("OKOKOK\n");
	}
	u_printf("OKOKOK\n");
	u_printf("while : out \n");
	//	free_Ustring(foo);
	///////////////////////////////////////
	// adding final state and
	// declaring it as final state
	///////////////////////////////////////
	add_state(tfst->automaton);
	int final_state = current_state;

	u_printf("final_state : %d\n",final_state);
	set_final_state(tfst->automaton->states[tmp_final_state]);
	int tag_number = get_value_index(EPSILON_TAG, tmp_tags);
	if (we_must_clean) {
		/* If necessary, we apply the "good paths" heuristic */
		keep_best_paths(tfst->automaton, tmp_tags);
	}
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
		for (int i = 0; i < tfst->automaton->number_of_states; i++) {
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
	u_printf("save_current_sentence(tfst, out_tfst, out_tind, tags->value, tags->size,NULL)\n");
	save_current_sentence(tfst, out_tfst, out_tind, tags->value, tags->size,
			NULL);
	//
	free_Ustring(states);
	free_Ustring(tmp_states);
	free_Ustring(foo);
	////	foo->str=NULL;
	free_string_hash(tags);
	free_string_hash(tmp_tags);
	close_text_automaton(tfst);

}
