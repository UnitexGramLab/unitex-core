/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"
#include "Thai.h"
#include "UnitexGetOpt.h"
#include "SortTxt.h"
#include "ProgramInvoker.h"
#include "DELA.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define DEFAULT 0
#define THAI 1

/* Maximum length of a line in the text file to be sorted */
#define LINE_LENGTH 10000

/**
 * This structure defines a list of couples (string,int). The integer
 * represents the number of occurrences of the string, so that the list
 *
 * a -> a -> a -> b -> c -> c
 *
 * will be represented by:
 *
 * (a,3) -> (b,1) -> (c,2)
 */
struct couple {
	unichar* s;
	int n;
	struct couple* next;
};

/**
 * This represents a node of the sort tree. Each node contains a list of strings,
 * represented with the 'couple' structure'.
 */
struct sort_tree_node {
	struct couple* couples;
	struct sort_tree_transition* transitions;
};

/**
 * This is a transition in the sort tree, tagged with a unichar.
 */
struct sort_tree_transition {
	unichar c;
	struct sort_tree_node* node;
	struct sort_tree_transition* next;
};

/**
 * All infos needed by sort operations.
 */
struct sort_infos {
	U_FILE* f;
	U_FILE* f_out;
	char REMOVE_DUPLICATES;
	int REVERSE;
	int number_of_lines;
	struct sort_tree_node* root;

	/* The following array gives for each character its class number, i.e.
	 * 0 if it's not a letter or the number of the line it appears in the
	 * char order file. */
	int class_numbers[0x10000];

	/* This array gives for each character its canonical character, i.e. the first
	 * character of its class, or itself if it is not a letter. */
	unichar canonical[0x10000];

	/* For a char that is a letter, this array indicates its position in its
	 * class. This information will be used for comparing characters of the
	 * same class. For instance, if we have the following char order file:
	 *
	 * Aa�
	 * Bb
	 * Cc�
	 * ...
	 *
	 * we will have class_numbers['c']=3, canonical['c']='C' and priority['c']=2
	 */
	int priority[0x10000];

	/* This array will be used to sort transitions that outgo from a node */
	struct sort_tree_transition* transitions[0x10000];

	int resulting_line_number;

	int factorize_inflectional_codes;
};

void sort(struct sort_infos*);
void sort_thai(struct sort_infos*);
int read_line(struct sort_infos* inf);
int read_line_thai(struct sort_infos* inf);
void save(struct sort_infos* inf);
struct sort_tree_node* new_sort_tree_node();
void free_sort_tree_node(struct sort_tree_node*);
void free_sort_tree_transition(struct sort_tree_transition*);
void free_couple(struct couple*);
void get_node(unichar*, int, struct sort_tree_node*, struct sort_infos* inf);
void explore_node(struct sort_tree_node*, struct sort_infos* inf,
		struct dela_entry**);
void quicksort(struct sort_tree_transition**, int, int, struct sort_infos* inf);
int char_cmp(unichar, unichar, struct sort_infos* inf);
int strcmp2(unichar*, unichar*, struct sort_infos* inf);
struct couple* insert_string_thai(unichar*, struct couple*,
		struct sort_infos* inf);

/**
 * Allocates, initializes and returns a new struct sort_infos*
 */
struct sort_infos* new_sort_infos() {
	struct sort_infos* inf = (struct sort_infos*) malloc(
			sizeof(struct sort_infos));
	if (inf == NULL) {
		fatal_alloc_error("new_sort_infos");
	}
	inf->f = NULL;
	inf->f_out = NULL;
	inf->REMOVE_DUPLICATES = 1;
	inf->REVERSE = 1;
	inf->number_of_lines = 0;
	inf->root = new_sort_tree_node();
	inf->resulting_line_number = 0;
	for (int i = 0; i < 0x10000; i++) {
		inf->class_numbers[i] = 0;
		inf->canonical[i] = (unichar) i;
		inf->priority[i] = 0;
	}
	inf->factorize_inflectional_codes = 0;
	return inf;
}

/**
 * Frees the memory associated to the given struct sort_infos*
 */
void free_sort_infos(struct sort_infos* inf) {
	if (inf == NULL)
		return;
	/* We don't have to free the sort tree since it's done while dumping it into
	 * the sorted file */
	free(inf);
}

/**
 * This function reads the given char order file.
 */
void read_char_order(const VersatileEncodingConfig* vec, const char* name,
		struct sort_infos* inf) {
	int c;
	int current_line = 1;
	U_FILE* f = u_fopen(vec, name, U_READ);
	if (f == NULL) {
		error("Cannot open file %s\n", name);
		return;
	}
	unichar current_canonical = '\0';
	int current_priority = 0;
	while ((c = u_fgetc(f)) != EOF) {
		if (c != '\n') {
			/* we ignore the \n char */
			if (inf->class_numbers[(unichar) c] != 0) {
				error("Error in %s: char 0x%x appears several times\n", name, c);
			} else {
				inf->class_numbers[(unichar) c] = current_line;
				if (current_canonical == '\0') {
					current_canonical = (unichar) c;
				}
				inf->canonical[(unichar) c] = current_canonical;
				inf->priority[(unichar) c] = ++current_priority;
			}
		} else {
			current_line++;
			current_canonical = '\0';
			current_priority = 0;
		}
	}
	u_fclose(f);
}

const char
		* usage_SortTxt =
				"Usage: SortTxt [OPTIONS] <txt>\n"
					"\n"
					"  <txt>: any unicode text file\n"
					"\n"
					"OPTIONS:\n"
					"  -n/--no_duplicates: remove duplicates (default)\n"
					"  -d/--duplicates: do not remove duplicates\n"
					"  -r/--reverse: reverse the sort order\n"
					"  -o XXX/--sort_order=XXX: use a file describing the char order for sorting\n"
					"  -l XXX/--line_info: saves the resulting number of lines in file XXX\n"
					"  -t/--thai: sorts thai text\n"
					"  -f/--factorize_inflectional_codes: makes two entries XXX,YYY.ZZZ:A and XXX,YYY.ZZZ:B\n"
					"                                   become a single entry XXX,YYY.ZZZ:A:B\n"
					"  -h/--help: this help\n"
					"\n"
					"By default, the sort is done according the Unicode char order, removing duplicates.\n";

static void usage() {
	u_printf("%S", COPYRIGHT);
	u_printf(usage_SortTxt);
}

int pseudo_main_SortTxt(const VersatileEncodingConfig* vec, int duplicates,
		int reverse, char* sort_alphabet, char* line_info, int thai,
		char* text, int factorize) {
	ProgramInvoker* invoker = new_ProgramInvoker(main_SortTxt, "main_SortTxt");
	char tmp[200];
	{
		tmp[0] = 0;
		get_reading_encoding_text(tmp, sizeof(tmp) - 1,
				vec->mask_encoding_compatibility_input);
		if (tmp[0] != '\0') {
			add_argument(invoker, "-k");
			add_argument(invoker, tmp);
		}

		tmp[0] = 0;
		get_writing_encoding_text(tmp, sizeof(tmp) - 1, vec->encoding_output,
				vec->bom_output);
		if (tmp[0] != '\0') {
			add_argument(invoker, "-q");
			add_argument(invoker, tmp);
		}
	}
	if (duplicates) {
		add_argument(invoker, "-d");
	} else {
		add_argument(invoker, "-n");
	}
	if (reverse) {
		add_argument(invoker, "-r");
	}
	if (sort_alphabet != NULL) {
		add_argument(invoker, "-o");
		add_argument(invoker, sort_alphabet);
	}
	if (line_info != NULL) {
		add_argument(invoker, "-l");
		add_argument(invoker, line_info);
	}
	if (thai) {
		add_argument(invoker, "-t");
	}
	if (factorize) {
		add_argument(invoker, "-f");
	}
	add_argument(invoker, text);
	int ret = invoke(invoker);
	free_ProgramInvoker(invoker);
	return ret;
}

const char* optstring_SortTxt = ":ndr:o:l:tfhk:q:";
const struct option_TS
		lopts_SortTxt[] = { { "no_duplicates", no_argument_TS, NULL, 'n' }, {
				"duplicates", no_argument_TS, NULL, 'd' }, { "reverse",
				no_argument_TS, NULL, 'r' }, { "sort_order",
				required_argument_TS, NULL, 'o' }, { "line_info",
				required_argument_TS, NULL, 'l' }, { "thai", no_argument_TS,
				NULL, 't' }, { "factorize_inflectional_codes", no_argument_TS,
				NULL, 'f' }, { "input_encoding", required_argument_TS, NULL,
				'k' }, { "output_encoding", required_argument_TS, NULL, 'q' },
				{ "help", no_argument_TS, NULL, 'h' }, { NULL, no_argument_TS,
						NULL, 0 } };

int main_SortTxt(int argc, char* const argv[]) {
	if (argc == 1) {
		usage();
		return 0;
	}

	struct sort_infos* inf = new_sort_infos();
	int mode = DEFAULT;
	char line_info[FILENAME_MAX] = "";
	char sort_order[FILENAME_MAX] = "";
	VersatileEncodingConfig vec = { DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT,
			DEFAULT_ENCODING_OUTPUT, DEFAULT_BOM_OUTPUT };
	int val, index = -1;
	struct OptVars* vars = new_OptVars();
	while (EOF != (val = getopt_long_TS(argc, argv, optstring_SortTxt,
			lopts_SortTxt, &index, vars))) {
		switch (val) {
		case 'n':
			inf->REMOVE_DUPLICATES = 1;
			break;
		case 'd':
			inf->REMOVE_DUPLICATES = 0;
			break;
		case 'r':
			inf->REVERSE = -1;
			break;
		case 'o':
			if (vars->optarg[0] == '\0') {
				fatal_error(
						"You must specify a non empty sort order file name\n");
			}
			strcpy(sort_order, vars->optarg);
			break;
		case 'l':
			if (vars->optarg[0] == '\0') {
				fatal_error(
						"You must specify a non empty information file name\n");
			}
			strcpy(line_info, vars->optarg);
			break;
		case 't':
			mode = THAI;
			break;
		case 'f':
			inf->factorize_inflectional_codes = 1;
			break;
		case 'h':
			usage();
			return 0;
		case 'k':
			if (vars->optarg[0] == '\0') {
				fatal_error("Empty input_encoding argument\n");
			}
			decode_reading_encoding_parameter(
					&(vec.mask_encoding_compatibility_input), vars->optarg);
			break;
		case 'q':
			if (vars->optarg[0] == '\0') {
				fatal_error("Empty output_encoding argument\n");
			}
			decode_writing_encoding_parameter(&(vec.encoding_output),
					&(vec.bom_output), vars->optarg);
			break;
		case ':':
			if (index == -1)
				fatal_error("Missing argument for option -%c\n", vars->optopt);
			else
				fatal_error("Missing argument for option --%s\n",
						lopts_SortTxt[index].name);
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

	if (sort_order[0] != '\0') {
		read_char_order(&vec, sort_order, inf);
	}

	char new_name[FILENAME_MAX];
	strcpy(new_name, argv[vars->optind]);
	strcat(new_name, ".new");
	inf->f = u_fopen(&vec, argv[vars->optind], U_READ);
	if (inf->f == NULL) {
		error("Cannot open file %s\n", argv[vars->optind]);
		return 1;
	}
	inf->f_out = u_fopen(&vec, new_name, U_WRITE);
	if (inf->f_out == NULL) {
		error("Cannot open temporary file %s\n", new_name);
		u_fclose(inf->f);
		return 1;
	}
	switch (mode) {
	case DEFAULT:
		sort(inf);
		break;
	case THAI:
		sort_thai(inf);
		break;
	}
	if (line_info[0] != '\0') {
		U_FILE* F = u_fopen(&vec, line_info, U_WRITE);
		if (F == NULL) {
			error("Cannot write %s\n", line_info);
		} else {
			u_fprintf(F, "%d\n", inf->resulting_line_number);
			u_fclose(F);
		}
	}
	u_fclose(inf->f);
	u_fclose(inf->f_out);
	af_remove(argv[vars->optind]);
	af_rename(new_name, argv[vars->optind]);
	free_sort_infos(inf);
	free_OptVars(vars);
	u_printf("Done.\n");
	return 0;
}

/**
 * Returns 0 if a==b, -i if a<b and +i if a>b, according 1) to
 * the char order file and 2) to the fact that the sort is ascending
 * or descending.
 */
int char_cmp(unichar a, unichar b, struct sort_infos* inf) {
	if (inf->class_numbers[a]) {
		if (inf->class_numbers[b]) {
			if (inf->class_numbers[a] == inf->class_numbers[b]) {
				return (inf->priority[a] - inf->priority[b]);
			} else
				return inf->REVERSE * (inf->class_numbers[a]
						- inf->class_numbers[b]);
		} else {
			return inf->REVERSE;
		}
	} else {
		if (inf->class_numbers[b]) {
			return -1 * inf->REVERSE;
		} else {
			return inf->REVERSE * (a - b);
		}
	}
}

/**
 * Does a unicode char to char comparison, according to the char_cmp function order.
 */
int strcmp2(unichar* a, unichar* b, struct sort_infos* inf) {
	int i = 0;
	while (a[i] && a[i] == b[i])
		i++;
	return char_cmp(a[i], b[i], inf);
}

/**
 * Reads all the line and puts them in a sorted tree, then saves them
 * exploring the tree.
 */
void sort(struct sort_infos* inf) {
	u_printf("Loading text...\n");
	while (read_line(inf)) {
	}
	u_printf("%d lines read\n", inf->number_of_lines);
	save(inf);
}

/**
 * Reads all the Thai lines and put them in a sorted tree, then saves them
 * exploring the tree.
 */
void sort_thai(struct sort_infos* inf) {
	u_printf("Loading text...\n");
	while (read_line_thai(inf)) {
	}
	u_printf("%d lines read\n", inf->number_of_lines);
	save(inf);
}

/**
 * Reads and processes a line of the text file.
 * Returns 0 if the end of file has been reached; 1 otherwise.
 */
int read_line(struct sort_infos* inf) {
	unichar line[LINE_LENGTH];
	int c;
	int ret = 1;
	int i = 0;
	while ((c = u_fgetc(inf->f)) != '\n' && c != EOF && i < LINE_LENGTH) {
		line[i++] = (unichar) c;
	}
	line[i] = '\0';
	if (c == EOF)
		ret = 0;
	else
		(inf->number_of_lines)++;
	if (i == 0) {
		/* We ignore the empty line */
		return ret;
	}
	if (i == LINE_LENGTH) {
		/* Too long lines are not taken into account */
		error("Line %d: line too long\n", inf->number_of_lines);
		return ret;
	}
	get_node(line, 0, inf->root, inf);
	return ret;
}

/**
 * Saves the lines.
 */
void save(struct sort_infos* inf) {
	u_printf("Sorting and saving...\n");
	/* -1 means that no line at all was already printed */
	struct dela_entry* last = (struct dela_entry*)-1;
	explore_node(inf->root, inf, &last);
	if (last != NULL && last!=(struct dela_entry*)-1) {
		u_fprintf(inf->f_out, "\n");
		free_dela_entry(last);
	}
}

/**
 * Initializes, allocates and returns a new sort tree node.
 */
struct sort_tree_node* new_sort_tree_node() {
	struct sort_tree_node* n = (struct sort_tree_node*) malloc(
			sizeof(struct sort_tree_node));
	if (n == NULL) {
		fatal_alloc_error("new_sort_tree_node");
	}
	n->couples = NULL;
	n->transitions = NULL;
	return n;
}

/**
 * Frees all the memory associated to the node n.
 */
void free_sort_tree_node(struct sort_tree_node* n) {
	if (n == NULL) {
		return;
	}
	free_couple(n->couples);
	free_sort_tree_transition(n->transitions);
	free(n);
}

/**
 * Initializes, allocates and returns a new sort tree transition.
 */
struct sort_tree_transition* new_sort_tree_transition() {
	struct sort_tree_transition* t = (struct sort_tree_transition*) malloc(
			sizeof(struct sort_tree_transition));
	if (t == NULL) {
		fatal_alloc_error("new_sort_tree_transition");
	}
	t->node = NULL;
	t->next = NULL;
	return t;
}

/**
 * Frees all the memory associated to the given transition list.
 */
void free_sort_tree_transition(struct sort_tree_transition* t) {
	struct sort_tree_transition* tmp;
	while (t != NULL) {
		/* We MUST NOT call 'free_sort_tree_node' since we are
		 * freeing while recursively exploring the node tree. So,
		 * when we free a node, its child have already been freed. */
		//free_sort_tree_node(t->node);
		tmp = t;
		t = t->next;
		free(tmp);
	}
}

/**
 * Allocates, initializes and returns a new couple.
 */
struct couple* new_couple(unichar* s) {
	struct couple* c = (struct couple*) malloc(sizeof(struct couple));
	if (c == NULL) {
		fatal_alloc_error("new_couple");
	}
	c->next = NULL;
	c->n = 1;
	c->s = u_strdup(s);
	return c;
}

/**
 * Frees all the memory associated to the given couple list.
 */
void free_couple(struct couple* c) {
	struct couple* tmp;
	while (c != NULL) {
		if (c->s != NULL)
			free(c->s);
		tmp = c;
		c = c->next;
		free(tmp);
	}
}

/**
 * Looks for a transition tagged with the character class of c. Inserts it if
 * absent.
 */
struct sort_tree_transition* get_transition(unichar c,
		struct sort_tree_transition** t, struct sort_infos* inf) {
	struct sort_tree_transition* tmp;
	if (inf->class_numbers[c] != 0) {
		c = (unichar) inf->canonical[c];
	}
	tmp = *t;
	while (tmp != NULL) {
		if (tmp->c == c)
			return tmp;
		tmp = tmp->next;
	}
	tmp = new_sort_tree_transition();
	tmp->c = c;
	tmp->next = (*t);
	tmp->node = NULL;
	(*t) = tmp;
	return tmp;
}

/**
 * Inserts the given string in the given couple list.
 */
struct couple* insert_string(unichar* s, struct couple* couple,
		struct sort_infos* inf) {
	struct couple* tmp;
	if (couple == NULL || inf->REVERSE * strcmp2(s, couple->s, inf) < 0) {
		/* If we are at the end of the list, or if we have to insert */
		tmp = new_couple(s);
		tmp->next = couple;
		return tmp;
	}
	if (!strcmp2(s, couple->s, inf)) {
		/* If the string is already in the list */
		if (!inf->REMOVE_DUPLICATES)
			(couple->n)++;
		return couple;
	}
	/* If we have to explore the tail of the list */
	couple->next = insert_string(s, couple->next, inf);
	return couple;
}

/**
 * This function looks for the path to 'line', creating it if necessary
 * the current node is n, and pos is the position in the 'line' string.
 * Note that the branches of the tree are not tagged with letters but with
 * letter classes, so that "l�" and "le" correspond to the same node.
 */
void get_node(unichar* line, int pos, struct sort_tree_node* n,
		struct sort_infos* inf) {
	if (line[pos] == '\0') {
		/* We are at the final node for 'line' */
		n->couples = insert_string(line, n->couples, inf);
		return;
	}
	/* If we are not at the end of the string */
	struct sort_tree_transition* trans = get_transition(line[pos],
			&(n->transitions), inf);
	if (trans->node == NULL) {
		trans->node = new_sort_tree_node();
	}
	get_node(line, pos + 1, trans->node, inf);
}


static int are_compatible(struct dela_entry* a,struct dela_entry* b) {
return !u_strcmp(a->inflected,b->inflected)
	&& !u_strcmp(a->lemma,b->lemma)
	&& same_semantic_codes(a,b);
}


/**
 * Explores the node n, dumps the corresponding lines to the output file,
 * and then frees the node. 'pos' is the current position in the string 's'.
 */
void explore_node(struct sort_tree_node* n, struct sort_infos* inf,
		struct dela_entry* *last) {
	int i, N;
	struct sort_tree_transition* t;
	struct couple* couple;
	struct couple* tmp;
	if (n == NULL) {
		fatal_error("Internal error in explore_node\n");
	}
	if (n->couples != NULL) {
		/* If the node is a final one, we print the corresponding lines */
		couple = n->couples;
		while (couple != NULL) {
			if (inf->factorize_inflectional_codes) {
				/* We look if the previously printed line, if any, did share
				 * the same information. If so, we just append the new inflectional codes.
				 * Otherwise, we print the new line.
				 *
				 * NOTE: in factorize mode, we always ignore duplicates */
				int err;
				struct dela_entry* entry = tokenize_DELAF_line(couple->s,1,&err,0);
				if (entry==NULL) {
					/* We have a non DELAF entry line, like for instance a comment one */
					if (*last!=NULL && *last!=(struct dela_entry*)-1) {
						/* If there was at least one line already printed, then this line
						 * awaits for its \n */
						u_fprintf(inf->f_out, "\n");
					}
					/* Then we print the line */
					u_fprintf(inf->f_out, "%S\n",couple->s);
					/* And we reset *last */
					if (*last==(struct dela_entry*)-1) {
						*last=NULL;
					} else if (*last!=NULL) {
						free_dela_entry(*last);
						*last=NULL;
					}
				} else {
					/* So, we have a dic entry. Was there a previous one ? */
					if (*last==NULL || *last==(struct dela_entry*)-1) {
						/* No ? So we print the line, and the current entry becomes *last */
						u_fputs(couple->s, inf->f_out);
						*last=entry;
					} else {
						/* Yes ? We must compare if the codes are compatible */
						if (are_compatible(*last,entry)) {
							/* We look for any code of entry if it was already in *last */
							for (int j=0;j<entry->n_inflectional_codes;j++) {
								if (!dic_entry_contain_inflectional_code(*last,entry->inflectional_codes[j])) {
									u_fprintf(inf->f_out, ":%S",entry->inflectional_codes[j]);
									/* We also have to add the newly printed code to *last */
									(*last)->inflectional_codes[((*last)->n_inflectional_codes)++]=u_strdup(entry->inflectional_codes[j]);
								}
							}
							/* And we must free entry */
							free_dela_entry(entry);
						} else {
							/* If codes are not compatible, we print the \n for the previous
							 * line, then the current line that becomes *last */
							u_fprintf(inf->f_out, "\n%S",couple->s);
							free_dela_entry(*last);
							*last=entry;
						}
					}
				}
			} else {
				/* Normal way: we print each line one after the other */
				for (i = 0; i < couple->n; i++) {
					u_fprintf(inf->f_out, "%S\n", couple->s);
					(inf->resulting_line_number)++;
				}
			}
			tmp = couple;
			couple = couple->next;
			free(tmp->s);
			free(tmp);
		}
		n->couples = NULL;
	}
	/* We convert the transition list into a sorted array */
	t = n->transitions;
	N = 0;
	while (t != NULL && N < 0x10000) {
		inf->transitions[N++] = t;
		t = t->next;
	}
	if (N == 0x10000) {
		fatal_error("Internal error in explore_node: more than 0x10000 nodes\n");
	}
	if (N > 1)
		quicksort(inf->transitions, 0, N - 1, inf);
	/* After sorting, we copy the result into the transitions of n */
	for (int j = 0; j < N - 1; j++) {
		inf->transitions[j]->next = inf->transitions[j + 1];
	}
	if (N > 0) {
		inf->transitions[N - 1]->next = NULL;
		n->transitions = inf->transitions[0];
	}
	/* Finally, we explore the outgoing transitions */
	t = n->transitions;
	while (t != NULL) {
		explore_node(t->node, inf, last);
		t = t->next;
	}
	/* And we free the node */
	free_sort_tree_node(n);
}

/**
 * Partitions the given array for quicksort.
 */
int partition(struct sort_tree_transition** t, int m, int n,
		struct sort_infos* inf) {
	unichar pivot;
	struct sort_tree_transition* tmp;
	int i = m - 1;
	int j = n + 1; /* Final index of the pivot */
	pivot = t[(m + n) / 2]->c;
	for (;;) {
		do
			j--;
		while ((j > (m - 1)) && (char_cmp(pivot, t[j]->c, inf) < 0));
		do
			i++;
		while ((i < n + 1) && (char_cmp(t[i]->c, pivot, inf) < 0));
		if (i < j) {
			tmp = t[i];
			t[i] = t[j];
			t[j] = tmp;
		} else
			return j;
	}
}

/**
 * Quicksorts the given sort tree transition array.
 */
void quicksort(struct sort_tree_transition** t, int m, int n,
		struct sort_infos* inf) {
	int p;
	if (m < n) {
		p = partition(t, m, n, inf);
		quicksort(t, m, p, inf);
		quicksort(t, p + 1, n, inf);
	}
}

/**
 * Converts the string 'src' into a string with no diacritic sign and
 * in which initial vowels and following consons have been swapped.
 */
void convert_thai(unichar* src, unichar* dest) {
	int i = 0, j = 0;
	while (src[i] != '\0') {
		if (is_Thai_diacritic(src[i])) {
			i++;
		} else if (is_Thai_initial_vowel(src[i])) {
			dest[j] = src[i + 1];
			dest[j + 1] = src[i];
			i = i + 2;
			j = j + 2;
		} else {
			dest[j++] = src[i++];
		}
	}
	dest[j] = '\0';
}

/**
 * This function look for the path to 'line', creating it if necessary
 * the current node is n, and pos is the position in the 'line' string.
 * Note that the branches of the tree are not tagged with letters but with
 * letter classes, so that "l�" and "le" correspond to the same node.
 * When the final node is reached, we insert the real Thai string with diacritic
 * signs in the correct position.
 */

void get_node_thai(unichar* line, int pos, struct sort_tree_node* n,
		unichar* real_string, struct sort_infos* inf) {
	if (line[pos] == '\0') {
		/* We are at the final node for 'line' */
		n->couples = insert_string_thai(real_string, n->couples, inf);
		return;
	}
	/* If we are not at the end of the string 'line' */
	struct sort_tree_transition* trans = get_transition(line[pos],
			&(n->transitions), inf);
	if (trans->node == NULL) {
		trans->node = new_sort_tree_node();
	}
	get_node_thai(line, pos + 1, trans->node, real_string, inf);
}

/**
 * Reads and processes a line of the Thai text file.
 */
int read_line_thai(struct sort_infos* inf) {
	unichar line[LINE_LENGTH];
	unichar thai_line[LINE_LENGTH];
	int c;
	int ret = 1;
	int i = 0;
	while ((c = u_fgetc(inf->f)) != '\n' && c != EOF && i < LINE_LENGTH) {
		line[i++] = (unichar) c;
	}
	line[i] = '\0';
	if (c == EOF)
		ret = 0;
	else
		(inf->number_of_lines)++;
	if (i == 0) {
		/* We ignore the empty line */
		return ret;
	}
	if (i == LINE_LENGTH) {
		error("Line %d: line too long\n", inf->number_of_lines);
		return ret;
	}
	convert_thai(line, thai_line);
	get_node_thai(thai_line, 0, inf->root, line, inf);
	return ret;
}

/**
 * Inserts the given string in the given couple list.
 */
struct couple* insert_string_thai(unichar* line, struct couple* couple,
		struct sort_infos* inf) {
	struct couple* tmp;
	if (couple == NULL || inf->REVERSE * u_strcmp(line, couple->s) < 0) {
		/* If we are at the end of the list, or if we have to insert */
		tmp = new_couple(line);
		tmp->next = couple;
		return tmp;
	}
	if (!u_strcmp(line, couple->s)) {
		/* If 'line' is already in the list */
		if (!inf->REMOVE_DUPLICATES)
			(couple->n)++;
		return couple;
	}
	/* If we have to explore the tail of the list */
	couple->next = insert_string_thai(line, couple->next, inf);
	return couple;
}

} // namespace unitex
