/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Error.h"
#include "Fst2TxtAsRoutine.h"
#include "TransductionStack.h"
#include "Overlap.h"
#include "Fst2Check_lib.h"
#include "File.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define MAX_DEPTH 300
#define MOT_BUFFER_TOKEN_SIZE (1000)

#define CAPACITY_LIMIT 16384
#define MINIMAL_SIZE_PRELOADED_TEXT (2048+1)

void build_state_token_trees(struct fst2txt_parameters*);
void parse_text(struct fst2txt_parameters*);

int main_fst2txt(struct fst2txt_parameters* p) {
	p->f_input = u_fopen(&(p->vec), p->text_file, U_READ);
	if (p->f_input == NULL) {
		error("Cannot open file %s\n", p->text_file);
		return 1;
	}

	p->text_buffer = new_buffer_for_file(UNICHAR_BUFFER, p->f_input,
			CAPACITY_LIMIT);
	p->buffer = p->text_buffer->unichar_buffer;

	p->f_output = u_fopen(&(p->vec), p->temp_file, U_WRITE);
	if (p->f_output == NULL) {
		error("Cannot open temporary file %s\n", p->temp_file);
		u_fclose(p->f_input);
		return 1;
	}

	struct FST2_free_info fst2load_free;
	Fst2* fst2load = load_abstract_fst2(&(p->vec), p->fst_file, 1, &fst2load_free);
	if (fst2load == NULL) {
		error("Cannot load grammar %s\n", p->fst_file);
		u_fclose(p->f_input);
		u_fclose(p->f_output);
		return 1;
	}
	if (fst2load->debug) {
		error("Cannot use debug fst2 %s\n", p->fst_file);
		u_fclose(p->f_input);
		u_fclose(p->f_output);
		free_abstract_Fst2(fst2load,&fst2load_free);
		return 1;
	}


    p->fst2txt_abstract_allocator=create_abstract_allocator("fst2txt_fst2",AllocatorCreationFlagAutoFreePrefered);


    p->fst2=new_Fst2_clone(fst2load,p->fst2txt_abstract_allocator);
    free_abstract_Fst2(fst2load,&fst2load_free);


	OK_for_Fst2Txt(p->fst2);

	if (p->alphabet_file != NULL && p->alphabet_file[0] != '\0') {
		p->alphabet = load_alphabet(&(p->vec), p->alphabet_file);
		if (p->alphabet == NULL) {
			error("Cannot load alphabet file %s\n", p->alphabet_file);
			u_fclose(p->f_input);
			u_fclose(p->f_output);
			free_abstract_Fst2(p->fst2, NULL);
			return 1;
		}
	}

	u_printf("Applying %s in %s mode...\n", p->fst_file, (p->output_policy
			== MERGE_OUTPUTS) ? "merge" : "replace");
	build_state_token_trees(p);
	parse_text(p);
	u_fclose(p->f_input);
	u_fclose(p->f_output);
	af_remove(p->text_file);
	af_rename(p->temp_file, p->text_file);
	/* And finally, we compute offsets */
	process_offsets(p->v_in_offsets, p->v_out_offsets, p->f_out_offsets);
	u_printf("Done.\n");
	return 0;
}

/**
 * Allocates, initializes and returns a new fst2txt_parameters structure.
 */
struct fst2txt_parameters* new_fst2txt_parameters() {
	struct fst2txt_parameters* p = (struct fst2txt_parameters*) malloc(
			sizeof(struct fst2txt_parameters));
	if (p == NULL) {
		fatal_alloc_error("new_fst2txt_parameters");
	}
	p->text_file = NULL;
	p->temp_file = NULL;
	p->fst_file = NULL;
	p->alphabet_file = NULL;
	p->f_input = NULL;
	p->f_output = NULL;
	p->fst2 = NULL;
	p->fst2txt_abstract_allocator = NULL;
	p->alphabet = NULL;
	p->text_file = NULL;
	p->temp_file = NULL;
	p->fst_file = NULL;
	p->alphabet_file = NULL;
	p->token_tree = NULL;
	p->n_token_trees = 0;
	p->variables = NULL;
	p->output_policy = MERGE_OUTPUTS;
	p->tokenization_policy = WORD_BY_WORD_TOKENIZATION;
	p->space_policy = DONT_START_WITH_SPACE;
	p->text_buffer = NULL;
	p->buffer = NULL;
	p->current_origin = 0;
	p->absolute_offset = 0;
	p->stack = new_stack_unichar(MAX_OUTPUT_LENGTH);
	p->input_length = 0;
	p->vec.mask_encoding_compatibility_input
				= DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
	p->vec.encoding_output = DEFAULT_ENCODING_OUTPUT;
	p->vec.bom_output = DEFAULT_BOM_OUTPUT;
	p->v_in_offsets = NULL;
	p->v_out_offsets = NULL;
	p->f_out_offsets = NULL;
	p->insertions = NULL;
	p->current_insertions = NULL;
	p->CR_shift = 0;
	p->new_absolute_origin = 0;
	return p;
}

/**
 * Frees the given structure
 */
void free_fst2txt_parameters(struct fst2txt_parameters* p) {
	if (p == NULL)
		return;
	free(p->text_file);
	free(p->temp_file);
	free(p->fst_file);
	free(p->alphabet_file);
	for (int i = 0; i < p->n_token_trees; i++) {
		free_fst2txt_token_tree(p->token_tree[i],p->fst2txt_abstract_allocator);
	}

	int free_abstract_allocator_item = (get_allocator_cb_flag(p->fst2txt_abstract_allocator) & AllocatorGetFlagAutoFreePresent) ? 0 : 1;

	if (p->token_tree != NULL) {
		free_cb(p->token_tree, p->fst2txt_abstract_allocator);
	}

	free_Variables(p->variables);
	free_buffer(p->text_buffer);

	free_alphabet(p->alphabet);
	free_stack_unichar(p->stack);
	free_vector_offset(p->v_in_offsets);
	free_vector_offset(p->v_out_offsets);
	if (free_abstract_allocator_item) {
		free_Fst2(p->fst2, p->fst2txt_abstract_allocator);
		free_vector_int(p->insertions, p->fst2txt_abstract_allocator);
		free_vector_int(p->current_insertions, p->fst2txt_abstract_allocator);
	}
	u_fclose(p->f_out_offsets);
	close_abstract_allocator(p->fst2txt_abstract_allocator);
	free(p);
}

////////////////////////////////////////////////////////////////////////
// TEXT PARSING
////////////////////////////////////////////////////////////////////////


static void scan_graph(int, int, int, int, struct parsing_info**, unichar*,
				struct fst2txt_parameters*);

int write_transduction();

static void push_output_string(struct fst2txt_parameters* p, unichar s[]) {
	int i = 0;
	while (s[i] != '\0') {
		if (s[i] == '$') {
			// case of a variable name
			unichar name[100];
			int L = 0;
			i++;
			while (is_variable_char(s[i])) {
				name[L++] = s[i++];
			}
			name[L] = '\0';
			if (s[i] != '$') {
				error("Error: missing closing $ after $%S\n", name);
			} else {
				i++;
				if (L == 0) {
					// case of $$ in order to print a $
					push(p->stack, '$');
				} else {
					struct transduction_variable* v =
							get_transduction_variable(p->variables, name);
					if (v == NULL) {
						error("Error: undefined variable $%S\n", name);
					} else if (v->start_in_tokens == -1) {
						error(
								"Error: starting position of variable $%S undefined\n",
								name);
					} else if (v->end_in_tokens == -1) {
						error(
								"Error: end position of variable $%S undefined\n",
								name);
					} else if (v->start_in_tokens > v->end_in_tokens) {
						error(
								"Error: end position before starting position for variable $%S\n",
								name);
					} else {
						// if the variable definition is correct
						if (p->current_origin<p->text_buffer->size) {
							for (int k = v->start_in_tokens; k <= v->end_in_tokens; k++)
								push(p->stack, p->buffer[k + p->current_origin]);
						}
					}
				}
			}
		} else {
			push(p->stack, s[i]);
			i++;
		}
	}
}

void emit_output(struct fst2txt_parameters* p, unichar* s, int pos) {
	if (s != NULL) {
		if (p->current_insertions != NULL && p->output_policy == MERGE_OUTPUTS) {
			/* In REPLACE mode, there will be only one big output, so there is
			 * no need for all those subtle calculus */
			int CR_shift = 0;
			if (pos + p->current_origin < p->text_buffer->size) {
				for (int i = p->current_origin; i < pos + p->current_origin; i++) {
					if (p->buffer[i] == '\n') {
						CR_shift++;
					}
				}
			}
			/* In case the match contains several insertions, we have to take their lengths
			 * into account for the next one */
			int output_shift=0;
			for (int i=0;i<p->current_insertions->nbelems;i+=4) {
				output_shift=output_shift+p->current_insertions->tab[i+3]-p->current_insertions->tab[i+2];
			}
			/* If there is a need to compute offsets, we store the position of the insertion */
			vector_int_add(p->current_insertions, p->CR_shift + CR_shift + pos
					+ p->current_origin + p->absolute_offset, p->fst2txt_abstract_allocator);
			vector_int_add(p->current_insertions, p->CR_shift + CR_shift + pos
					+ p->current_origin + p->absolute_offset, p->fst2txt_abstract_allocator);
			vector_int_add(p->current_insertions, CR_shift + pos
					+ p->new_absolute_origin+output_shift, p->fst2txt_abstract_allocator);
			vector_int_add(p->current_insertions, CR_shift + pos
					+ p->new_absolute_origin + u_strlen(s)+output_shift, p->fst2txt_abstract_allocator);
		}
		push_output_string(p, s);
	}
}


void parse_text(struct fst2txt_parameters* p) {
	fill_buffer(p->text_buffer, p->f_input);
	int debut = p->fst2->initial_states[1];
	p->variables = new_Variables(p->fst2->input_variables);
	int n_blocks = 0;
	u_printf("Block %d", n_blocks);
	int within_tag = 0;
	if (p->output_policy == MERGE_OUTPUTS /* && p->f_out_offsets!=NULL*/) {
		p->insertions = new_vector_int(2048, p->fst2txt_abstract_allocator);
		p->current_insertions = new_vector_int(2048, p->fst2txt_abstract_allocator);
	}
	p->v_out_offsets = new_vector_offset();
	/* The following test used to be a <, but now it's a <= because of the {$} tag
	 * that may be used even if the end of the text has already been reached */
	while (p->current_origin <= p->text_buffer->size) {
		if (!p->text_buffer->end_of_file && p->current_origin
				> (p->text_buffer->size - MINIMAL_SIZE_PRELOADED_TEXT)) {
			/* If we must change of block, we update the absolute offset, and we fill the
			 * buffer. */
			p->absolute_offset = p->absolute_offset + p->current_origin;
			fill_buffer(p->text_buffer, p->current_origin, p->f_input);
			p->current_origin = 0;
			n_blocks++;
			u_printf("\rBlock %d        ", n_blocks);
		}
		p->output[0] = '\0';
		empty(p->stack);
		p->input_length = 0;
		if (p->output_policy == MERGE_OUTPUTS) {
			p->insertions->nbelems = 0;
			p->current_insertions->nbelems = 0;
		}
		if (p->buffer[p->current_origin] == '{') {
			within_tag = 1;
		} else if (p->buffer[p->current_origin] == '}') {
			within_tag = 0;
		} else if (!within_tag && (p->buffer[p->current_origin] != ' '
				|| p->space_policy == START_WITH_SPACE)) {
			// we don't start a match on a space
			unichar mot_token_buffer[MOT_BUFFER_TOKEN_SIZE];
			scan_graph(0, debut, 0, 0, NULL, mot_token_buffer, p);
		}
		if (p->output_policy == MERGE_OUTPUTS) {
			/* If there was an insertion, we have to note it */
			if (p->insertions != NULL && p->insertions->nbelems != 0) {
				for (int i = 0; i < p->insertions->nbelems; i = i + 4) {
					vector_offset_add(p->v_out_offsets, p->insertions->tab[i],
							p->insertions->tab[i + 1],
							p->insertions->tab[i + 2],
							p->insertions->tab[i + 3]);
				}
			}
		} else if (p->output_policy == REPLACE_OUTPUTS && p->input_length != 0) {
			int a = p->current_origin + p->CR_shift + p->absolute_offset;
			int b = a + p->input_length;
			int output_length=u_strlen(p->output);
			int diff = 0;
			int i,j;
			for (i=0,j=0; i < p->input_length && j<output_length; i++,j++) {
				if (p->buffer[i + p->current_origin] == '\n')
					b++;
				if (!diff && p->buffer[i + p->current_origin] != p->output[j]) {
					diff = 1;
				}
			}
			if (p->input_length>0 && p->output[0]=='\0') {
				/* any input deletion must be considered */
				diff=1;
			}
			if (!diff && p->output[j] != '\0') {
				diff = 1;
			}
			if (diff) {
				/* There is no need to consider fake replace that happen when
				 * normalizing quotes or dashes */
				int c = p->new_absolute_origin;
				int d = c + u_strlen(p->output);
				vector_offset_add(p->v_out_offsets, a, b, c, d);
			}
		}
		u_fprintf(p->f_output, "%S", p->output);
		p->new_absolute_origin = p->new_absolute_origin + u_strlen(p->output);
		if (p->input_length == 0) {
			// if no input was read, we go on
			if (p->current_origin < p->text_buffer->size)
				u_fputc(p->buffer[p->current_origin], p->f_output);
			(p->new_absolute_origin)++;
			if (p->buffer[p->current_origin] == '\n') {
				/* If we just have skipped a \n, we note there is an offset shift of 1 */
				(p->CR_shift)++;
				(p->new_absolute_origin)++;
			}
			(p->current_origin)++;
		} else {
			// we increase current_origin
			int new_origin = p->current_origin + p->input_length;
			for (int i = p->current_origin; i < new_origin; i++) {
				if (p->buffer[i] == '\n') {
					/* If we just have skipped a \n, we note there is an offset shift of 1 */
					(p->CR_shift)++;
					if (p->output_policy == MERGE_OUTPUTS) {
						/* We consider the \n in the output only in MERGE mode */
						(p->new_absolute_origin)++;
					}
				}
			}
			p->current_origin = new_origin;
		}
	}
	u_printf("\r                           \n");
	free_Variables(p->variables);
	p->variables = NULL;
}

/*
 * scan_graph token a lot of time of comparing string against
 *  "<E>", "<MOT>", "<NB>", "<MAJ>", "<MIN>", "<PRE>", "<PNC>", "<L>", "<^>", "#", " "
 */
static const unichar ETIQ_E_LN3[] = { '<', 'E', '>', 0 };
static const unichar ETIQ_MOT_LN5[] = { '<', 'M', 'O', 'T', '>', 0 };
static const unichar ETIQ_NB_LN4[] = { '<', 'N', 'B', '>', 0 };
static const unichar ETIQ_MAJ_LN5[] = { '<', 'M', 'A', 'J', '>', 0 };
static const unichar ETIQ_MIN_LN5[] = { '<', 'M', 'I', 'N', '>', 0 };
static const unichar ETIQ_PRE_LN5[] = { '<', 'P', 'R', 'E', '>', 0 };
static const unichar ETIQ_PNC_LN5[] = { '<', 'P', 'N', 'C', '>', 0 };
static const unichar ETIQ_L_LN3[] = { '<', 'L', '>', 0 };
static const unichar ETIQ_CIRC_LN3[] = { '<', '^', '>', 0 };
static const unichar ETIQ_DIESE_LN1[] = { '#', 0 };
static const unichar ETIQ_SPACE_LN1[] = { ' ', 0 };

/*
 * give the len of possible match
 */
static int u_len_possible_match(const unichar* s) {
	if ((*(s + 0)) == 0)
		return 0;
	if ((*(s + 1)) == 0)
		return 1;

	// all match with len > 1 begin with '>'
	if ((*(s + 0)) != '<')
		return 0;

	if ((*(s + 2)) == 0)
		return 0;
	if ((*(s + 3)) == 0)
		return 3;
	if ((*(s + 4)) == 0)
		return 4;
	if ((*(s + 5)) == 0)
		return 5;
	return 0;
}

/*
 * superfast hardcoded compare with specific len of one string
 * we must call after u_len_possible_match as returned the possible
 * match length
 * u_len_possible_match also check string begin with '<' for len 3,4,5
 * we return 1 when different, 0 when equal
 */
static inline int u_trymatch_superfast1(const unichar* a, const unichar c) {
	if ((*(a + 0)) != (c))
		return 1;
	return 0;
}

static inline int u_trymatch_superfast3(const unichar* a, const unichar*b) {
	if ((*(a + 1)) != (*(b + 1)))
		return 1;
	if ((*(a + 2)) != (*(b + 2)))
		return 1;
	return 0;
}

static inline int u_trymatch_superfast4(const unichar* a, const unichar*b) {
	if ((*(a + 1)) != (*(b + 1)))
		return 1;
	if ((*(a + 2)) != (*(b + 2)))
		return 1;
	if ((*(a + 3)) != (*(b + 3)))
		return 1;
	return 0;
}

static inline int u_trymatch_superfast5(const unichar* a, const unichar*b) {
	if ((*(a + 1)) != (*(b + 1)))
		return 1;
	if ((*(a + 2)) != (*(b + 2)))
		return 1;
	if ((*(a + 3)) != (*(b + 3)))
		return 1;
	if ((*(a + 4)) != (*(b + 4)))
		return 1;
	return 0;
}

static int backup(vector_int* v) {
	if (v == NULL)
		return 0;
	return v->nbelems;
}

static void restore(vector_int* v, int n) {
	if (v == NULL) {
		if (n != 0)
			fatal_error("Unexpected error in restore");
		return;
	}
	v->nbelems = n;
}

static inline int at_text_start(struct fst2txt_parameters* p, int pos) {
	return p->absolute_offset == 0 && pos == 0 && (p->current_origin == 0
			|| (p->current_origin == 1 && p->buffer[0] == ' '));
}

static void scan_graph(
		int n_graph, // number of current graph
		int e, // number of current state
		int pos, //
		int depth, struct parsing_info** match_list,
		unichar* word_token_buffer, struct fst2txt_parameters* p) {
	Fst2State current_state = p->fst2->states[e];
	int old_nb_insert;
	if (depth > MAX_DEPTH) {

		error("\n"
			"Maximal stack size reached in graph %i!\n"
			"Recognized more than %i tokens starting from:\n"
			"  ", n_graph, MAX_DEPTH);
		int nb_max_char_display_error = (p->text_buffer->size >= p->current_origin) ? (p->text_buffer->size - p->current_origin) : 0;
		nb_max_char_display_error = (nb_max_char_display_error > 60) ? 60 : nb_max_char_display_error;
		for (int i = 0; i < nb_max_char_display_error; i++) {
			error("%C", p->buffer[p->current_origin + i]);
		}
		error("\nSkipping match at this position, trying from next token!\n");
		p->output[0] = '\0'; // clear output
		p->input_length = 0; // reset taille_entree
		empty(p->stack); // clear output stack
		if (match_list != NULL) {
			while (*match_list != NULL) { // free list of subgraph matches
				struct parsing_info* la_tmp = *match_list;
				*match_list = (*match_list)->next;
				la_tmp->next = NULL; // to don't free the next item
				free_parsing_info(la_tmp, p->fst2txt_abstract_allocator, NULL, NULL);
			}
		}
		return;
		//  exit(1); // don't exit, try at next position
	}
	depth++;

	if (is_final_state(current_state)) {

		// if we are in a final state
		p->stack->stack[p->stack->stack_pointer + 1] = '\0';
		if (n_graph == 0) { // in main graph
			if (pos >= p->input_length/*sommet>u_strlen(output)*/) {
				// and if the recognized input is longer than the current one, it replaces it
				u_strcpy(p->output, p->stack->stack);
				p->input_length = (pos);
				vector_int_copy(p->insertions, p->current_insertions, p->fst2txt_abstract_allocator);
			}
		} else { // in a subgraph
			(*match_list) = insert_if_absent(pos, -1, -1, (*match_list),
					p->stack->stack_pointer + 1, p->stack->stack, p->variables,
					NULL, NULL, -1, -1, NULL, -1, p->current_insertions,-1,
					p->fst2txt_abstract_allocator,p->fst2txt_abstract_allocator,NULL);
		}
	}

	int end_of_text = pos + p->current_origin == p->text_buffer->size;
	/* The same, but tolerant if there is a remaining space */
	int end_of_text2 = pos + p->current_origin + 1 == p->text_buffer->size
			&& p->buffer[pos + p->current_origin] == ' ';
	int SOMMET = p->stack->stack_pointer + 1;
	int pos2;
	/* If there are some letter sequence transitions like %hello, we process them */
	if (!end_of_text && p->token_tree[e]->transition_array != NULL) {
		if (p->buffer[pos + p->current_origin] == ' ') {
			pos2 = pos + 1;
			if (p->output_policy == MERGE_OUTPUTS)
				push(p->stack, ' ');
		}
		/* we don't keep this line because of problems occurring in sentence tokenizing
		 * if the return sequence is defautly considered as a separator like space
		 else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
		 */
		else {
			pos2 = pos;
		}

		int position = 0;
		int start_pos_ = pos2;
		unichar *token = word_token_buffer;
		if (p->tokenization_policy == CHAR_BY_CHAR_TOKENIZATION || (is_letter(
				p->buffer[pos2 + p->current_origin], p->alphabet) && (pos2
				+ p->current_origin == 0 || !is_letter(p->buffer[pos2
				+ p->current_origin - 1], p->alphabet)))) {
			/* If we are in character by character mode */
			while (pos2 + p->current_origin < p->text_buffer->size
					&& is_letter(p->buffer[pos2 + p->current_origin],
							p->alphabet)) {
				token[position++] = p->buffer[(pos2++) + p->current_origin];
				if (p->tokenization_policy == CHAR_BY_CHAR_TOKENIZATION) {
					break;
				}
			}
			token[position] = '\0';
			if (position != 0 && (p->tokenization_policy
					== CHAR_BY_CHAR_TOKENIZATION || !(is_letter(token[position
					- 1], p->alphabet) && is_letter(p->buffer[pos2
					+ p->current_origin], p->alphabet)))) {
				// we proceed only if we have exactly read the 'token' sequence
				// in both modes MERGE and REPLACE, we process the output if any
				int SOMMET2 = p->stack->stack_pointer;
				Transition* RES = get_matching_tags(token, p->token_tree[e],
						p->alphabet,p->fst2txt_abstract_allocator);
				Transition* TMP;
				unichar* mot_token_new_recurse_buffer = NULL;
				if (RES != NULL) {
					// we allocate a new mot_token_buffer for the scan_graph recursion because we need preserve current
					// token=mot_token_buffer
					mot_token_new_recurse_buffer = (unichar*) malloc_cb(
							MOT_BUFFER_TOKEN_SIZE * sizeof(unichar),p->fst2txt_abstract_allocator);
					if (mot_token_new_recurse_buffer == NULL) {
						fatal_alloc_error("scan_graph");
					}
				}
				while (RES != NULL) {
					p->stack->stack_pointer = SOMMET2;
					Fst2Tag etiq = p->fst2->tags[RES->tag_number];
					old_nb_insert = backup(p->current_insertions);
					emit_output(p, etiq->output, start_pos_);
					int longueur = u_strlen(etiq->input);
					unichar C = token[longueur];
					token[longueur] = '\0';
					if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
						// if we are in MERGE mode, we add to output the char we have read
						push_input_string(p->stack, token, 0);
					}
					token[longueur] = C;
					scan_graph(n_graph, RES->state_number, pos2 - (position
							- longueur), depth, match_list,
							mot_token_new_recurse_buffer, p);
					restore(p->current_insertions, old_nb_insert);
					TMP = RES;
					RES = RES->next;
					free_cb(TMP,p->fst2txt_abstract_allocator);
				}
				if (mot_token_new_recurse_buffer != NULL) {
					free_cb(mot_token_new_recurse_buffer,p->fst2txt_abstract_allocator);
				}
			}
		}
	}

	Transition* t = current_state->transitions;
	while (t != NULL) {
		p->stack->stack_pointer = SOMMET - 1;
		// we process the transition of the current state
		int n_etiq = t->tag_number;
		if (n_etiq < 0) {
			// case of a sub-graph
			struct parsing_info* liste = NULL;
			unichar* pile_old;
			p->stack->stack[p->stack->stack_pointer + 1] = '\0';
			pile_old = u_strdup(p->stack->stack);
			vector_int* old_insertions = vector_int_dup(p->current_insertions, p->fst2txt_abstract_allocator);
			scan_graph((((unsigned) n_etiq) - 1),
					p->fst2->initial_states[-n_etiq], pos, depth, &liste,
					word_token_buffer, p);
			free_vector_int(p->current_insertions, p->fst2txt_abstract_allocator);
			p->current_insertions = NULL;
			while (liste != NULL) {
				p->stack->stack_pointer = liste->stack_pointer - 1;
				u_strcpy(p->stack->stack, liste->stack);
				p->current_insertions = liste->insertions;
				liste->insertions = NULL;
				scan_graph(n_graph, t->state_number, liste->pos_in_tokens, depth,
						match_list, word_token_buffer, p);
				free_vector_int(p->current_insertions, p->fst2txt_abstract_allocator);
				struct parsing_info* l_tmp = liste;
				liste = liste->next;
				l_tmp->next = NULL; // in order not to free the next item
				free_parsing_info(l_tmp, p->fst2txt_abstract_allocator, NULL,NULL);
			}
			u_strcpy(p->stack->stack, pile_old);
			p->current_insertions = old_insertions;
			free(pile_old);
			p->stack->stack_pointer = SOMMET - 1;
		} else {
			// case of a normal tag
			Fst2Tag etiq = p->fst2->tags[n_etiq];
			unichar* contenu = etiq->input;
			int contenu_len_possible_match = u_len_possible_match(contenu);
			if (etiq->type == BEGIN_OUTPUT_VAR_TAG) {
				fatal_error("Unsupported $|XXX( tags in Fst2Txt\n");
			}
			if (etiq->type == END_OUTPUT_VAR_TAG) {
				fatal_error("Unsupported $|XXX) tags in Fst2Txt\n");
			}
			if (etiq->type == BEGIN_VAR_TAG) {
				// case of a $a( variable tag
				if (/*1 || */ !end_of_text) {
					/* There is no point in trying to start a variable capture at the end of the text */
					struct transduction_variable* L =
							get_transduction_variable(p->variables,
									etiq->variable);
					if (L == NULL) {
						fatal_error("Unknown variable: %S\n", etiq->variable);
					}
					if (p->buffer[pos + p->current_origin] == ' ' && pos
							+ p->current_origin + 1 < p->text_buffer->size
							&& p->space_policy!=START_WITH_SPACE) {
						pos2 = pos + 1;
						if (p->output_policy == MERGE_OUTPUTS)
							push(p->stack, ' ');
					} else
						pos2 = pos;
					L->start_in_tokens = pos2;
					if (end_of_text) L->start_in_tokens=pos;
					scan_graph(n_graph, t->state_number, pos2, depth,
							match_list, word_token_buffer, p);
				}
			} else if (etiq->type == END_VAR_TAG) {
				// case of a $a) variable tag
				struct transduction_variable* L = get_transduction_variable(
						p->variables, etiq->variable);
				if (L == NULL) {
					fatal_error("Unknown variable: %S\n", etiq->variable);
				}
				if (pos > 0)
					L->end_in_tokens = pos - 1;
				else
					L->end_in_tokens = pos;
				// BUG: qd changement de buffer, penser au cas start dans ancien buffer et end dans nouveau
				scan_graph(n_graph, t->state_number, pos, depth, match_list,
						word_token_buffer, p);
			} else if (etiq->type == TEXT_START_TAG) {
				// case of an empty sequence that should be matched, but only if we are at the beginning of the text
				// in both modes MERGE and REPLACE, we process the transduction if any
				if (at_text_start(p, pos)) {
					old_nb_insert = backup(p->current_insertions);
					emit_output(p, etiq->output, pos);
					scan_graph(n_graph, t->state_number, pos, depth,
							match_list, word_token_buffer, p);
					restore(p->current_insertions, old_nb_insert);
				}
			} else if (etiq->type == TEXT_END_TAG) {
				// case of an empty sequence that should be matched, but only if we are at the end of the text
				// in both modes MERGE and REPLACE, we process the transduction if any
				if (end_of_text || end_of_text2) {
					old_nb_insert = backup(p->current_insertions);
					emit_output(p, etiq->output, pos);
					scan_graph(n_graph, t->state_number, pos, depth,
							match_list, word_token_buffer, p);
					restore(p->current_insertions, old_nb_insert);
				}
			} else if ((contenu_len_possible_match == 5) // <MOT>
					&& (!u_trymatch_superfast5(contenu, ETIQ_MOT_LN5))) {
				// case of transition by any sequence of letters
				if (!end_of_text) {
					if (p->buffer[pos + p->current_origin] == ' ' && pos
							+ p->current_origin + 1 < p->text_buffer->size) {
						pos2 = pos + 1;
						if (p->output_policy == MERGE_OUTPUTS)
							push(p->stack, ' ');
					}
					//else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
					else
						pos2 = pos;
					unichar* mot = word_token_buffer;
					int position = 0;
					int start_pos = pos2;
					if (p->tokenization_policy == CHAR_BY_CHAR_TOKENIZATION
							|| ((pos2 + p->current_origin) == 0 || !is_letter(
									p->buffer[pos2 + p->current_origin - 1],
									p->alphabet))) {
						while (pos2 + p->current_origin < p->text_buffer->size
								&& is_letter(
										p->buffer[pos2 + p->current_origin],
										p->alphabet)) {
							mot[position++] = p->buffer[(pos2++)
									+ p->current_origin];
						}
						mot[position] = '\0';
						if (position != 0) {
							// we proceed only if we have read a letter sequence
							// in both modes MERGE and REPLACE, we process the transduction if any
							old_nb_insert = backup(p->current_insertions);
							emit_output(p, etiq->output, start_pos);
							if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
								// if we are in MERGE mode, we add to ouput the char we have read
								push_output_string(p->stack, mot);
							}
							scan_graph(n_graph, t->state_number, pos2, depth,
									match_list, word_token_buffer, p);
							restore(p->current_insertions, old_nb_insert);
						}
					}
				}
			} else if ((contenu_len_possible_match == 4) //<NB>
					&& (!u_trymatch_superfast4(contenu, ETIQ_NB_LN4))) {
				// case of transition by any sequence of digits
				if (!end_of_text) {
					if (p->buffer[pos + p->current_origin] == ' ') {
						pos2 = pos + 1;
						if (p->output_policy == MERGE_OUTPUTS)
							push(p->stack, ' ');
					}
					//else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
					else
						pos2 = pos;
					unichar* mot = word_token_buffer;
					int position = 0;
					int start_pos = pos2;
					while (pos2 + p->current_origin < p->text_buffer->size
							&& (p->buffer[pos2 + p->current_origin] >= '0')
							&& (p->buffer[pos2 + p->current_origin] <= '9')) {
						mot[position++] = p->buffer[(pos2++)
								+ p->current_origin];
					}
					mot[position] = '\0';
					if (position != 0) {
						// we proceed only if we have read a letter sequence
						// in both modes MERGE and REPLACE, we process the transduction if any
						old_nb_insert = backup(p->current_insertions);
						emit_output(p, etiq->output, start_pos);
						if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
							// if we are in MERGE mode, we add to ouput the char we have read
							push_output_string(p->stack, mot);
						}
						scan_graph(n_graph, t->state_number, pos2, depth,
								match_list, word_token_buffer, p);
						restore(p->current_insertions, old_nb_insert);
					}
				}
			} else if ((contenu_len_possible_match == 5) // <MAJ>
					&& (!u_trymatch_superfast5(contenu, ETIQ_MAJ_LN5))) {
				// case of upper case letter sequence
				if (!end_of_text) {
					if (p->buffer[pos + p->current_origin] == ' ') {
						pos2 = pos + 1;
						if (p->output_policy == MERGE_OUTPUTS)
							push(p->stack, ' ');
					}
					//else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
					else
						pos2 = pos;
					unichar* mot = word_token_buffer;
					int position = 0;
					int start_pos = pos2;
					if (p->tokenization_policy == CHAR_BY_CHAR_TOKENIZATION
							|| ((pos2 + p->current_origin) == 0 || !is_letter(
									p->buffer[pos2 + p->current_origin - 1],
									p->alphabet))) {
						while (pos2 + p->current_origin < p->text_buffer->size
								&& is_upper(
										p->buffer[pos2 + p->current_origin],
										p->alphabet)) {
							mot[position++] = p->buffer[(pos2++)
									+ p->current_origin];
						}
						mot[position] = '\0';
						if (position != 0 && !is_letter(p->buffer[pos2
								+ p->current_origin], p->alphabet)) {
							// we proceed only if we have read an upper case letter sequence
							// which is not followed by a lower case letter
							// in both modes MERGE and REPLACE, we process the transduction if any
							old_nb_insert = backup(p->current_insertions);
							emit_output(p, etiq->output, start_pos);
							if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
								// if we are in MERGE mode, we add to ouput the char we have read
								push_input_string(p->stack, mot, 0);
							}
							scan_graph(n_graph, t->state_number, pos2, depth,
									match_list, word_token_buffer, p);
							restore(p->current_insertions, old_nb_insert);
						}
					}
				}
			} else if ((contenu_len_possible_match == 5) // <MIN>
					&& (!u_trymatch_superfast5(contenu, ETIQ_MIN_LN5))) {
				// case of lower case letter sequence
				if (!end_of_text) {
					if (p->buffer[pos + p->current_origin] == ' ') {
						pos2 = pos + 1;
						if (p->output_policy == MERGE_OUTPUTS)
							push(p->stack, ' ');
					}
					//else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
					else
						pos2 = pos;
					unichar* mot = word_token_buffer;
					int position = 0;
					int start_pos = pos2;
					if (p->tokenization_policy == CHAR_BY_CHAR_TOKENIZATION
							|| (pos2 + p->current_origin == 0 || !is_letter(
									p->buffer[pos2 + p->current_origin - 1],
									p->alphabet))) {
						while (pos2 + p->current_origin < p->text_buffer->size
								&& is_lower(
										p->buffer[pos2 + p->current_origin],
										p->alphabet)) {
							mot[position++] = p->buffer[(pos2++)
									+ p->current_origin];
						}
						mot[position] = '\0';
						if (position != 0 && !is_letter(p->buffer[pos2
								+ p->current_origin], p->alphabet)) {
							// we proceed only if we have read a lower case letter sequence
							// which is not followed by an upper case letter
							// in both modes MERGE and REPLACE, we process the transduction if any
							old_nb_insert = backup(p->current_insertions);
							emit_output(p, etiq->output, start_pos);
							if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
								// if we are in MERGE mode, we add to ouput the char we have read
								push_input_string(p->stack, mot, 0);
							}
							scan_graph(n_graph, t->state_number, pos2, depth,
									match_list, word_token_buffer, p);
							restore(p->current_insertions, old_nb_insert);
						}
					}
				}
			} else if ((contenu_len_possible_match == 5) // <PRE>
					&& (!u_trymatch_superfast5(contenu, ETIQ_PRE_LN5))) {
				// case of a sequence beginning by an upper case letter
				if (!end_of_text) {
					if (p->buffer[pos + p->current_origin] == ' ') {
						pos2 = pos + 1;
						if (p->output_policy == MERGE_OUTPUTS)
							push(p->stack, ' ');
					}
					//else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
					else
						pos2 = pos;
					unichar* mot = word_token_buffer;
					int position = 0;
					int start_pos = pos2;
					if (p->tokenization_policy == CHAR_BY_CHAR_TOKENIZATION
							|| (is_upper(p->buffer[pos2 + p->current_origin],
									p->alphabet) && (pos2 + p->current_origin
									== 0 || !is_letter(p->buffer[pos2
									+ p->current_origin - 1], p->alphabet)))) {
						while (pos2 + p->current_origin < p->text_buffer->size
								&& is_letter(
										p->buffer[pos2 + p->current_origin],
										p->alphabet)) {
							mot[position++] = p->buffer[(pos2++)
									+ p->current_origin];
						}
						mot[position] = '\0';
						if (position != 0 && !is_letter(p->buffer[pos2
								+ p->current_origin], p->alphabet)) {
							// we proceed only if we have read a letter sequence
							// which is not followed by a letter
							// in both modes MERGE and REPLACE, we process the transduction if any
							old_nb_insert = backup(p->current_insertions);
							emit_output(p, etiq->output, start_pos);
							if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
								// if we are in MERGE mode, we add to ouput the char we have read
								push_input_string(p->stack, mot, 0);
							}
							scan_graph(n_graph, t->state_number, pos2, depth,
									match_list, word_token_buffer, p);
							restore(p->current_insertions, old_nb_insert);
						}
					}
				}
			} else if ((contenu_len_possible_match == 5)  // <PNC>
					&& (!u_trymatch_superfast5(contenu, ETIQ_PNC_LN5))) {
				// case of a punctuation sequence
				if (!end_of_text) {
					if (p->buffer[pos + p->current_origin] == ' ') {
						pos2 = pos + 1;
						if (p->output_policy == MERGE_OUTPUTS)
							push(p->stack, ' ');
					}
					//else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
					else
						pos2 = pos;
					int start_pos = pos2;
					unichar C = p->buffer[pos2 + p->current_origin];
					if (C == ';' || C == '!' || C == '?' || C == ':' || C
							== 0xbf || C == 0xa1 || C == 0x0e4f || C == 0x0e5a
							|| C == 0x0e5b || C == 0x3001 || C == 0x3002 || C
							== 0x30fb) {
						// in both modes MERGE and REPLACE, we process the transduction if any
						old_nb_insert = backup(p->current_insertions);
						emit_output(p, etiq->output, start_pos);
						if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
							// if we are in MERGE mode, we add to ouput the char we have read
							push(p->stack, C);
						}
						scan_graph(n_graph, t->state_number, pos2 + 1, depth,
								match_list, word_token_buffer, p);
						restore(p->current_insertions, old_nb_insert);
					} else {
						// we consider the case of ...
						// BUG: if ... appears at the end of the buffer
						if (C == '.') {
							if ((pos2 + p->current_origin + 2)
									< p->text_buffer->size && p->buffer[pos2
									+ p->current_origin + 1] == '.'
									&& p->buffer[pos2 + p->current_origin + 2]
											== '.') {
								old_nb_insert = backup(p->current_insertions);
								emit_output(p, etiq->output, start_pos);
								if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
									// if we are in MERGE mode, we add to ouput the ... we have read
									push(p->stack, C);
									push(p->stack, C);
									push(p->stack, C);
								}
								scan_graph(n_graph, t->state_number, pos2 + 3,
										depth, match_list, word_token_buffer, p);
								restore(p->current_insertions, old_nb_insert);
							} else {
								// we consider the . as a normal punctuation sign
								old_nb_insert = backup(p->current_insertions);
								emit_output(p, etiq->output, start_pos);
								if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
									// if we are in MERGE mode, we add to ouput the char we have read
									push(p->stack, C);
								}
								scan_graph(n_graph, t->state_number, pos2 + 1,
										depth, match_list, word_token_buffer, p);
								restore(p->current_insertions, old_nb_insert);
							}
						}
					}
				}
			} else if ((contenu_len_possible_match == 3)  // <E>
					&& (!u_trymatch_superfast3(contenu, ETIQ_E_LN3))) {
				// case of an empty sequence
				// in both modes MERGE and REPLACE, we process the transduction if any
				old_nb_insert = backup(p->current_insertions);
				emit_output(p, etiq->output, pos);
				scan_graph(n_graph, t->state_number, pos, depth, match_list,
						word_token_buffer, p);
				restore(p->current_insertions, old_nb_insert);
			} else if ((contenu_len_possible_match == 3)
					&& (!u_trymatch_superfast3(contenu, ETIQ_CIRC_LN3))) {
				// case of a new line sequence
				if (!end_of_text) {
					if (p->buffer[pos + p->current_origin] == '\n') {
						// in both modes MERGE and REPLACE, we process the transduction if any
						old_nb_insert = backup(p->current_insertions);
						emit_output(p, etiq->output, pos);
						if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
							// if we are in MERGE mode, we add to ouput the char we have read
							push(p->stack, '\n');
						}
						scan_graph(n_graph, t->state_number, pos + 1, depth,
								match_list, word_token_buffer, p);
						restore(p->current_insertions, old_nb_insert);
					}
				}
			} else if ((contenu_len_possible_match == 1)
					&& (!u_trymatch_superfast1(contenu, '#'))
					&& (!(etiq->control & RESPECT_CASE_TAG_BIT_MASK))) {
				// case of a no space condition
				if (end_of_text || p->buffer[pos + p->current_origin] != ' ') {
					// in both modes MERGE and REPLACE, we process the transduction if any
					old_nb_insert = backup(p->current_insertions);
					emit_output(p, etiq->output, pos);
					scan_graph(n_graph, t->state_number, pos, depth,
							match_list, word_token_buffer, p);
					restore(p->current_insertions, old_nb_insert);
				}
			} else if ((contenu_len_possible_match == 1)
					&& (!u_trymatch_superfast1(contenu, ' '))) {
				// case of an obligatory space
				if (!end_of_text && p->buffer[pos + p->current_origin] == ' ') {
					// in both modes MERGE and REPLACE, we process the transduction if any
					old_nb_insert = backup(p->current_insertions);
					emit_output(p, etiq->output, pos);
					if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
						// if we are in MERGE mode, we add to ouput the char we have read
						push(p->stack, ' ');
					}
					scan_graph(n_graph, t->state_number, pos + 1, depth,
							match_list, word_token_buffer, p);
					restore(p->current_insertions, old_nb_insert);
				}
			} else if ((contenu_len_possible_match == 3)
					&& (!u_trymatch_superfast5(contenu, ETIQ_L_LN3))) {
				// case of a single letter
				if (!end_of_text) {
					if (p->buffer[pos + p->current_origin] == ' ') {
						pos2 = pos + 1;
						if (p->output_policy == MERGE_OUTPUTS)
							push(p->stack, ' ');
					}
					//else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
					else
						pos2 = pos;
					int start_pos = pos2;
					if (is_letter(p->buffer[pos2 + p->current_origin],
							p->alphabet)) {
						// in both modes MERGE and REPLACE, we process the transduction if any
						old_nb_insert = backup(p->current_insertions);
						emit_output(p, etiq->output, start_pos);
						if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
							// if we are in MERGE mode, we add to ouput the char we have read
							push(p->stack, p->buffer[pos2 + p->current_origin]);
						}
						scan_graph(n_graph, t->state_number, pos2 + 1, depth,
								match_list, word_token_buffer, p);
						restore(p->current_insertions, old_nb_insert);
					}
				}
			} else {
				// case of a normal letter sequence
				if (!end_of_text) {
					if (p->buffer[pos + p->current_origin] == ' ') {
						pos2 = pos + 1;
						if (p->output_policy == MERGE_OUTPUTS)
							push(p->stack, ' ');
					}
					//else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
					else
						pos2 = pos;
					int start_pos__ = pos2;
					if (etiq->control & RESPECT_CASE_TAG_BIT_MASK) {
						// case of exact case match
						int position = 0;
						while (pos2 + p->current_origin < p->text_buffer->size
								&& p->buffer[pos2 + p->current_origin]
										== contenu[position]) {
							pos2++;
							position++;
						}
						if (contenu[position] == '\0' && position != 0
								&& (!(is_letter(contenu[position - 1],
										p->alphabet) && is_letter(
										p->buffer[pos2 + p->current_origin],
										p->alphabet)) || p->tokenization_policy==CHAR_BY_CHAR_TOKENIZATION)) {
							// we proceed only if we have exactly read the contenu sequence
							// in both modes MERGE and REPLACE, we process the transduction if any
							old_nb_insert = backup(p->current_insertions);
							emit_output(p, etiq->output, start_pos__);
							if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
								// if we are in MERGE mode, we add to ouput the char we have read
								push_input_string(p->stack, contenu, 0);
							}
							scan_graph(n_graph, t->state_number, pos2, depth,
									match_list, word_token_buffer, p);
							restore(p->current_insertions, old_nb_insert);
						}
					} else {
						// case of variable case match
						// the letter sequences may have been caught by the arbre_etiquette structure
						int position = 0;
						unichar* mot = word_token_buffer;
						int start_pos = pos2;
						while (pos2 + p->current_origin < p->text_buffer->size
								&& is_equal_or_uppercase(contenu[position],
										p->buffer[pos2 + p->current_origin],
										p->alphabet)) {
							mot[position++] = p->buffer[(pos2++)
									+ p->current_origin];
						}
						mot[position] = '\0';
						if (contenu[position] == '\0' && position != 0
								&& !(is_letter(contenu[position - 1],
										p->alphabet) && is_letter(
										p->buffer[pos2 + p->current_origin],
										p->alphabet))) {
							// we proceed only if we have exactly read the contenu sequence
							// in both modes MERGE and REPLACE, we process the transduction if any
							old_nb_insert = backup(p->current_insertions);
							emit_output(p, etiq->output, start_pos);
							if (p->output_policy == MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
								// if we are in MERGE mode, we add to ouput the char we have read
								push_input_string(p->stack, mot, 0);
							}
							scan_graph(n_graph, t->state_number, pos2, depth,
									match_list, word_token_buffer, p);
							restore(p->current_insertions, old_nb_insert);
						}
					}
				}
			}
		}
		t = t->next;
	}
}

int not_a_letter_sequence(Fst2Tag e, Alphabet* alphabet) {
	// we return false only if e is a letter sequence like %hello
	if (e->control & RESPECT_CASE_TAG_BIT_MASK || e->type == BEGIN_VAR_TAG
			|| e->type == END_VAR_TAG || e->type == BEGIN_OUTPUT_VAR_TAG
			|| e->type == END_OUTPUT_VAR_TAG) {
		// case of @hello $a( and $a)
		return 1;
	}
	unichar* s = e->input;
	if (!is_letter(s[0], alphabet))
		return 1;
	int s_len_possible_match = u_len_possible_match(s);
	if (s_len_possible_match == 1) {
		if ((!u_trymatch_superfast1(s, '#'))
				|| (!u_trymatch_superfast1(s, ' ')))
			return 1;
	}
	if (s_len_possible_match == 3) {
		if ((!u_trymatch_superfast3(s, ETIQ_L_LN3)) || (!u_trymatch_superfast3(
				s, ETIQ_E_LN3)) || (!u_trymatch_superfast3(s, ETIQ_CIRC_LN3)))
			return 1;
	}
	// added in revision 1028
	if (s_len_possible_match == 4) {
		if ((!u_trymatch_superfast4(s, ETIQ_NB_LN4)))
			return 1;
	}
	if (s_len_possible_match == 5) {
		if ((!u_trymatch_superfast5(s, ETIQ_MOT_LN5))
				|| (!u_trymatch_superfast5(s, ETIQ_MAJ_LN5))
				|| (!u_trymatch_superfast5(s, ETIQ_MIN_LN5))
				|| (!u_trymatch_superfast5(s, ETIQ_PRE_LN5))
				|| (!u_trymatch_superfast5(s, ETIQ_PNC_LN5)))
			return 1;
	}
	return 0;
}

Transition* add_tag_to_token_tree(struct fst2txt_token_tree* tree,
		Transition* trans, struct fst2txt_parameters* p) {
	// case 1: empty transition
	if (trans == NULL)
		return NULL;
	// case 2: transition by something else that a sequence of letter like %hello
	//         or sub-graph call
	if (trans->tag_number < 0 || not_a_letter_sequence(
			p->fst2->tags[trans->tag_number], p->alphabet)) {
		trans->next = add_tag_to_token_tree(tree, trans->next, p);
		return trans;
	}
	Transition* tmp = add_tag_to_token_tree(tree, trans->next, p);
	add_tag(p->fst2->tags[trans->tag_number]->input, trans->tag_number,
			trans->state_number, tree, p->fst2txt_abstract_allocator);
	free_cb(trans,p->fst2txt_abstract_allocator);
	return tmp;
}

/**
 * For each state of the fst2, this function builds a tree
 * containing all the outgoing tokens. This will be used to
 * speed up the exploration when there is a large number
 * of tokens in a same box of a grf.
 */
void build_state_token_trees(struct fst2txt_parameters* p) {
	p->n_token_trees = p->fst2->number_of_states;
	p->token_tree = (struct fst2txt_token_tree**) malloc_cb(p->n_token_trees
			* sizeof(struct fst2txt_token_tree*),p->fst2txt_abstract_allocator);
	if (p->token_tree == NULL) {
		fatal_alloc_error("build_state_token_trees\n");
	}
	for (int i = 0; i < p->n_token_trees; i++) {
		p->token_tree[i] = new_fst2txt_token_tree(p->fst2txt_abstract_allocator);
		p->fst2->states[i]->transitions = add_tag_to_token_tree(
				p->token_tree[i], p->fst2->states[i]->transitions, p);
	}
}

} // namespace unitex
