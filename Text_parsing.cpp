/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <time.h>
#include "Text_tokens.h"
#include "Text_parsing.h"
#include "Error.h"
#include "BitArray.h"
#include "Transitions.h"
#include "MorphologicalLocate.h"
#include "DicVariables.h"
#include "UserCancelling.h"
#include "File.h"
#include "MappedFileHelper.h"


/* Delay between two prints (yyy% done) */
#define DELAY CLOCKS_PER_SEC

int binary_search(int, int*, int);
int find_compound_word(int, int, struct DLC_tree_info*,
		struct locate_parameters*);
unichar* get_token_sequence(const int*, struct string_hash*, int, int);
void enter_morphological_mode(int, int, int, int, struct parsing_info**, int,
		struct list_int*, struct locate_parameters*, struct Token_error_ctx*, Abstract_allocator);
void shift_variable_bounds(Variables*, int);
void add_match(int, unichar*, struct locate_parameters*, Abstract_allocator);
void real_add_match(struct match_list*, struct locate_parameters*, Abstract_allocator);
struct match_list* eliminate_longer_matches(struct match_list*, int, int,
		unichar*, int*, struct locate_parameters*, Abstract_allocator);
struct match_list* save_matches(struct match_list*, int, U_FILE*,
		struct locate_parameters*, Abstract_allocator);

/**
 * Performs the Locate operation on the text, saving the occurrences
 * on the fly.
 */
void launch_locate(U_FILE* out, long int text_size, U_FILE* info,
		struct locate_parameters* p, Abstract_allocator prv_alloc) {
	struct Token_error_ctx token_error_ctx;
	token_error_ctx.n_errors = 0;
	token_error_ctx.last_start = -1;
	token_error_ctx.last_length = 0;
	token_error_ctx.n_matches_at_token_pos__locate = 0;
	token_error_ctx.n_matches_at_token_pos__morphological_locate = 0;

	//fill_buffer(p->token_buffer, f);
	OptimizedFst2State initial_state =
			p->optimized_states[p->fst2->initial_states[1]];
	p->current_origin = 0;
	int n_read = 0;
	int unite;
	clock_t startTime = clock();
	clock_t currentTime;


	unite = ((text_size / 100) > 1000) ? (text_size / 100) : 1000;
	variable_backup_memory_reserve* backup_reserve =
			create_variable_backup_memory_reserve(p->variables);
	int current_token;
	while (p->current_origin < p->buffer_size && p->number_of_matches
			!= p->search_limit) {
		if (unite != 0) {
			n_read = p->current_origin % unite;
			if (n_read == 0 && ((currentTime = clock()) - startTime > DELAY)) {
				startTime = currentTime;
				u_printf("%2.0f%% done        \r", 100.0
						* (float) (p->current_origin)
						/ (float) text_size);
			}
		}
		current_token = p->buffer[p->current_origin];
		if (!(current_token == p->SPACE && p->space_policy
				== DONT_START_WITH_SPACE) && !get_value(p->failfast,
				current_token)) {

			if (consult_cache(p->buffer, p->current_origin,
					p->buffer_size, p->match_cache,
					p->cached_match_vector)) {
				/* If we have found matches in the cache, we use them */
				for (int i=0;i<p->cached_match_vector->nbelems;i++) {
					struct match_list* tmp=(struct match_list*)(p->cached_match_vector->tab[i]);
					while (tmp!=NULL) {
						/* We have to adjust the match coordinates */
						int size=tmp->m.end_pos_in_token-tmp->m.start_pos_in_token;
						tmp->m.start_pos_in_token=p->current_origin;
						tmp->m.end_pos_in_token=tmp->m.start_pos_in_token+size;
						real_add_match(tmp,p,prv_alloc);
						tmp=tmp->next;
					}
				}
			} else {
				/* Standard locate procedure */
				p->stack_base = -1;
				p->stack->stack_pointer = -1;
				struct parsing_info* matches = NULL;
				p->left_ctx_shift = 0;
				p->left_ctx_base = 0;

				int count_cancel_trying = 0;
				int count_call = 0;
				p->last_tested_position = 0;
				p->last_matched_position = -1;
				locate(0, initial_state, 0, 0, &matches, 0, NULL, p,
						&token_error_ctx, backup_reserve, &count_cancel_trying,
						&count_call, prv_alloc);
				if ((p->max_count_call > 0)
						&& (count_call >= p->max_count_call)) {
					u_printf(
							"stop computing token %u after %u step computing.\n",
							p->current_origin, count_call);
				} else if ((p->max_count_call_warning > 0) && (count_call
						>= p->max_count_call_warning)) {
					u_printf(
							"warning : computing token %u take %u step computing.\n",
							p->current_origin, count_call);
				}
				int can_cache_matches = 0;
				p->last_tested_position=p->last_tested_position+p->current_origin;
				if (p->last_matched_position == -1) {
					if (p->last_tested_position == p->current_origin) {
						/* We are in the fail fast case, nothing has been matched while
						 * looking only at the first current token. That means that no match
						 * could ever happen when this token is found in the text */
						set_value(p->failfast, current_token, 1);
					}
				} else {
					if (p->last_tested_position <= p->last_matched_position) {
						/* If there are matches that could never be longer, we
						 * can cache them */
						can_cache_matches = 1;
					}
				}
				struct match_list* tmp;
				while (p->match_cache_first != NULL) {
					real_add_match(p->match_cache_first, p, prv_alloc);
					tmp = p->match_cache_first;
					p->match_cache_first = p->match_cache_first->next;
					if (can_cache_matches &&
					      tmp->m.start_pos_in_token==p->current_origin) {
						/* We have to test the start position, because a match obtained using a left
						 * context could cause problems. We have to set tmp->next to NULL because
						 * we just want to consider this single match */
						tmp->next=NULL;
						/* We have to cache the match using the longest possible context and not
						 * only the end of the match. Imagine that the text contains the
						 * sequence "...volley-ball..." with the matches "volley" and
						 * "volley-ball". If we cache these two matches with their own ends,
						 * then, if the text contains "volley ball meeting", we will find
						 * "volley" in cache and skip longer matches like "volley ball".
						 */
						cache_match(tmp, p->buffer,
								tmp->m.start_pos_in_token,
								p->last_matched_position,
								&(p->match_cache[current_token]), prv_alloc);
					} else {
						free_match_list_element(tmp, prv_alloc);
					}
				}
				p->match_cache_last = NULL;
				free_parsing_info(matches);
				clear_dic_variable_list(&(p->dic_variables));
			}
		}
		p->match_list = save_matches(p->match_list,p->current_origin, out, p, prv_alloc);
		(p->current_origin)++;
	}
	free_reserve(backup_reserve);

	p->match_list = save_matches(p->match_list,p->current_origin+1, out, p, prv_alloc);
	u_printf("100%% done      \n\n");
	u_printf("%d match%s\n", p->number_of_matches,
			(p->number_of_matches == 1) ? "" : "es");
	if ((p->number_of_outputs != p->number_of_matches) && (p->number_of_outputs
			!= 0))
		u_printf("(%d output%s)\n", p->number_of_outputs, (p->number_of_outputs
				== 1) ? "" : "s");
	u_printf("%d recognized units\n", p->matching_units);

	unsigned long text_size_calc_per_halfhundred = text_size;
	unsigned long matching_units_per_halfhundred = (unsigned long)(p->matching_units);
	unsigned long factor = 1;
	while (text_size_calc_per_halfhundred / factor >= 21473) {
	    factor *= 10;
	}

	long per_halfhundred=0;
	if ((text_size_calc_per_halfhundred / factor) != 0)
	{
	    unsigned long multiplicator = 100000 ;
        unsigned long factor_matching_units = factor;
        while ((multiplicator > 1) && (factor_matching_units > 1)) {
          multiplicator /= 10;
          factor_matching_units /= 10;
        }
	    per_halfhundred = (long)(((matching_units_per_halfhundred * multiplicator ) / factor_matching_units) / (text_size_calc_per_halfhundred / factor));
	}

	if (text_size != 0) {
		u_printf("(%2.3f%% of the text is covered)\n", (float) (per_halfhundred
				/ (float) 1000.0));
	}
	if (info != NULL) {
		u_fprintf(info, "%d match%s\n", p->number_of_matches,
				(p->number_of_matches <= 1) ? "" : "es");
		if ((p->number_of_outputs != p->number_of_matches)
				&& (p->number_of_outputs != 0)) {
			u_fprintf(info, "(%d output%s)\n", p->number_of_outputs,
					(p->number_of_outputs <= 1) ? "" : "s");
		}
		u_fprintf(info, "%d recognized units\n", p->matching_units);
		if (text_size != 0) {
			u_fprintf(info, "(%2.3f%% of the text is covered)\n",
					(float) (per_halfhundred / (float) 1000.0));
		}
	}
}

/**
 *  Prints the current context to stderr,
 *  except if it was already printed.
 *  If there are more than MAX_ERRORS errors,
 *  exit the programm by calling "fatal_error".
 */
void error_at_token_pos(const char* message, int start, int length,
		struct locate_parameters* p, struct Token_error_ctx* p_token_error_ctx) {
	//static int n_errors;
	//static int last_start=-1;
	//static int last_length;
	int i;
	if ((p_token_error_ctx->last_start) == start) {
		/* The context was already printed */
		return;
	}
	error("%s\n  ", message);
	for (i = (start - 4); i <= (start + 20); i++) {
		if (i < 0) {
			continue;
		}
		if (i == start) {
			error("<<HERE>>");
		}
		error("%S", p->tokens->value[p->buffer[i]]);
		if (i == (start + length)) {
			error("<<END>>");
		}
	}
	if (i < (start + length)) {
		error(" ...");
	}
	error("\n");
	if (++(p_token_error_ctx->n_errors) >= MAX_ERRORS) {
		fatal_error("Too many errors, giving up!\n");
	}
	p_token_error_ctx->last_start = start;
	p_token_error_ctx->last_length = length;
}

/**
 * The logical XOR.
 */
int XOR(int a, int b) {
	return (a && !b) || (!a && b);
}

static void update_last_position(struct locate_parameters* p, int pos) {
	if (pos > p->last_tested_position) {
		p->last_tested_position = pos;
	}
}

/**
 * This is the core function of the Locate program.
 */
void locate(int graph_depth, /* 0 means that we are in the top level graph */
OptimizedFst2State current_state, /* current state in the grammar */
int pos, /* position in the token buffer, relative to the current origin */
int depth, /* number of nested calls to 'locate' */
struct parsing_info** matches, /* current match list. Irrelevant if graph_depth==0 */
int n_matches, /* number of sequences that have matched. It may be different from
 * the length of the 'matches' list if a given sequence can be
 * matched in several ways. It is used to detect combinatorial
 * explosions due to bad written grammars. */
struct list_int* ctx, /* information about the current context, if any */
struct locate_parameters* p, /* miscellaneous parameters needed by the function */
struct Token_error_ctx* p_token_error_ctx,
		variable_backup_memory_reserve*backup_reserve,
		int* p_count_cancel_trying, int* p_count_call, Abstract_allocator prv_alloc) {
#ifdef TRE_WCHAR
	int filter_number;
#endif
	int pos2 = -1, ctrl = 0, end_of_compound;
	int token, token2;
	Transition* t;
	int stack_top = p->stack->stack_pointer;
	unichar* output;

	//(*p_count_call)++;

	if ((*p_count_cancel_trying) == -1) {
		return;
	}

	if ((*p_count_cancel_trying) == 0) {

		if ((p->max_count_call) > 0) {
			if ((*p_count_call) >= (p->max_count_call)) {
				*p_count_cancel_trying = -1;
				return;
			}
		}

		*p_count_cancel_trying = COUNT_CANCEL_TRYING_INIT_CONST;
		if (((p->max_count_call) > 0) && (((*p_count_call)
				+COUNT_CANCEL_TRYING_INIT_CONST) > (p->max_count_call))) {
			*p_count_cancel_trying = (p->max_count_call) - (*p_count_call);
		}

		if (is_cancelling_requested() != 0) {
			*p_count_cancel_trying = -1;
			return;
		}
		(*p_count_call) += (*p_count_cancel_trying);
	}
	(*p_count_cancel_trying)--;

	/* The following static variable holds the number of matches at
	 * one position in text. */
	//static int n_matches_at_token_pos;
	if (depth == 0) {
		/* We reset if this is first call to 'locate' from a given position in the text */
		p_token_error_ctx->n_matches_at_token_pos__morphological_locate = 0;
	}
	if (depth > STACK_MAX) {
		/* If there are too much recursive calls */
		error_at_token_pos("\nMaximal stack size reached!\n"
			"(There may be longer matches not recognized!)", p->current_origin,
				pos, p, p_token_error_ctx);
		return;
	}
	if ((p_token_error_ctx->n_matches_at_token_pos__morphological_locate)
			> MAX_MATCHES_AT_TOKEN_POS) {
		/* If there are too much matches from the current origin in the text */
		error_at_token_pos(
				"\nToo many (ambiguous) matches starting from one position in text!",
				p->current_origin, pos, p, p_token_error_ctx);
		return;
	}
	if (current_state->control & 1) {
		/* If we are in a final state... */
		if (ctx != NULL) {
			/* If we have reached the final state of a graph while
			 * looking for a context, it's an error because every
			 * opened context must be closed before the end of the graph. */
			fatal_error("ERROR: unclosed context in graph \"%S\"\n",
					p->fst2->graph_names[graph_depth + 1]);
		}
		/* In we are in the top level graph, we have a match */
		if (graph_depth == 0) {
			(p_token_error_ctx->n_matches_at_token_pos__morphological_locate)++;
			if (p->output_policy == IGNORE_OUTPUTS) {
				if (pos > 0) {
					add_match(pos + p->current_origin-1,NULL, p, prv_alloc);
				} else {
					add_match(pos + p->current_origin,NULL, p, prv_alloc);
				}
			} else {
				p->stack->stack[stack_top + 1] = '\0';
				if (pos > 0) {
					add_match(pos + p->current_origin-1,
							p->stack->stack + p->left_ctx_base, p, prv_alloc);
				} else {
					add_match(pos + p->current_origin,
							p->stack->stack + p->left_ctx_base, p, prv_alloc);
				}
			}
		} else {
			/* If we are in a subgraph */
			if (n_matches == MAX_MATCHES_PER_SUBGRAPH) {
				/* If there are too much matches, we suspect an error in the grammar
				 * like an infinite recursion */
				error_at_token_pos(
						"\nMaximal number of matches per subgraph reached!",
						p->current_origin, pos, p, p_token_error_ctx);
				return;
			} else {
				/* If everything is fine, we add this match to the match list of the
				 * current graph level */
				n_matches++;
				p->stack->stack[stack_top + 1] = '\0';
				if (p->ambiguous_output_policy == ALLOW_AMBIGUOUS_OUTPUTS) {
					(*matches) = insert_if_different(pos, -1, -1, (*matches),
							p->stack->stack_pointer,
							&(p->stack->stack[p->stack_base + 1]),
							p->variables, p->dic_variables, p->left_ctx_shift,
							p->left_ctx_base, NULL, -1);
				} else {
					(*matches) = insert_if_absent(pos, -1, -1, (*matches),
							p->stack->stack_pointer,
							&(p->stack->stack[p->stack_base + 1]),
							p->variables, p->dic_variables, p->left_ctx_shift,
							p->left_ctx_base, NULL, -1);
				}
			}
		}
	}
	/* If we have reached the end of the token buffer, we indicate it by setting
	 * the current tokens to -1 */
	if (pos + p->current_origin >= p->buffer_size) {
		token = -1;
		token2 = -1;
	} else {
		token = p->buffer[pos + p->current_origin];
		if (token == p->SPACE) {
			pos2 = pos + 1;
		}
		/* Now, we deal with the SPACE, if any. To do that, we use several variables:
		 * pos: current position in the token buffer, relative to the current origin
		 * pos2: position of the first non-space token from 'pos'.
		 * token2: token at pos2  or -1 if 'pos2' is outside the token buffer */
		else {
			pos2 = pos;
		}
		if (pos2 + p->current_origin >= p->buffer_size) {
			token2 = -1;
		} else {
			token2 = p->buffer[pos2 + p->current_origin];
		}
	}

	/**
	 * SUBGRAPHS
	 */
	struct opt_graph_call* graph_call_list = current_state->graph_calls;
	if (graph_call_list != NULL) {
		/* If there are subgraphs, we process them */

		int* save_previous_ptr_var = NULL;
		int* var_backup = NULL;
		int create_new_reserve_done = 0;
		variable_backup_memory_reserve* reserve_used = backup_reserve;
		struct dic_variable* dic_variables_backup = NULL;
		int old_StackBase = p->stack_base;
		if (p->output_policy != IGNORE_OUTPUTS) {
			/* For better performance when ignoring outputs */

			if (is_enough_memory_in_reserve_for_two_set_variables(p->variables,
					reserve_used) == 0) {
				reserve_used = create_variable_backup_memory_reserve(
						p->variables);
				create_new_reserve_done = 1;
			}

			var_backup = create_variable_backup_using_reserve(p->variables,
					reserve_used);
			dic_variables_backup = p->dic_variables;
		}

		do {
			/* For each graph call, we look all the reachable states */
			t = graph_call_list->transition;
			while (t != NULL) {
				struct parsing_info* L = NULL;
				p->stack_base = p->stack->stack_pointer;
				p->dic_variables
						= clone_dic_variable_list(dic_variables_backup);

				locate(
						graph_depth + 1, /* Exploration of the subgraph */
						p->optimized_states[p->fst2->initial_states[graph_call_list->graph_number]],
						pos, depth + 1, &L, 0, NULL, /* ctx is set to NULL because the end of a context must occur in the
						 * same graph than its beginning */
						p, p_token_error_ctx, reserve_used,
						p_count_cancel_trying, p_count_call, prv_alloc);

				p->stack_base = old_StackBase;
				clear_dic_variable_list(&(p->dic_variables));
				if (L != NULL) {
					struct parsing_info* L_first = L;
					/* If there is at least one match, we process the match list */
					do {

						/* We restore the settings of the current graph level */
						if (p->output_policy != IGNORE_OUTPUTS) {
							u_strcpy(&(p->stack->stack[stack_top + 1]),
									L->stack);
							p->stack->stack_pointer = L->stack_pointer;

							if (save_previous_ptr_var == NULL) {
								save_previous_ptr_var
										= install_variable_backup_preserving(
												p->variables, reserve_used,
												L->variable_backup);
							} else {
								install_variable_backup(p->variables,
										L->variable_backup);
							}

							p->dic_variables = clone_dic_variable_list(
									L->dic_variable_backup);
						}
						/* And we continue the exploration */
						int old_left_ctx_shift = p->left_ctx_shift;
						int old_left_ctx_base = p->left_ctx_base;
						p->left_ctx_shift = L->left_ctx_shift;
						p->left_ctx_base = L->left_ctx_base;

						locate(graph_depth,
								p->optimized_states[t->state_number],
								L->position, depth + 1, matches, n_matches,
								ctx, p, p_token_error_ctx, backup_reserve,
								p_count_cancel_trying, p_count_call, prv_alloc);

						p->left_ctx_shift = old_left_ctx_shift;
						p->left_ctx_base = old_left_ctx_base;
						p->stack->stack_pointer = stack_top;
						clear_dic_variable_list(&(p->dic_variables));
						if (graph_depth == 0) {
							/* If we are at the top graph level, we restore the variables */
							if (p->output_policy != IGNORE_OUTPUTS) {
								if (save_previous_ptr_var != NULL) {
									restore_variable_array(p->variables,
											reserve_used, save_previous_ptr_var);
									save_previous_ptr_var = NULL;
								}
							}
						}
						L = L->next;
					} while (L != NULL);
					free_parsing_info(L_first); //  free all subgraph matches
				}
				/* As free_parsing_info has freed p->dic_variables, we must restore it */
				t = t->next;
			} /* end of while (t!=NULL) */
		} while ((graph_call_list = graph_call_list->next) != NULL);
		/* Finally, we have to restore the stack and other backup stuff */
		p->stack->stack_pointer = stack_top;
		p->stack_base = old_StackBase; /* May be changed by recursive subgraph calls */
		if (p->output_policy != IGNORE_OUTPUTS) { /* For better performance (see above) */
			if (save_previous_ptr_var != NULL) {
				restore_variable_array(p->variables, reserve_used,
						save_previous_ptr_var);
				save_previous_ptr_var = NULL;
			}

			int reserve_freeable = free_variable_backup_using_reserve(
					reserve_used);
			if ((reserve_freeable == 0) && (create_new_reserve_done != 0)) {
				fatal_error("incoherent reserve free result\n");
			}
			if (create_new_reserve_done == 1)
				free_reserve(reserve_used);

		}
		p->dic_variables = dic_variables_backup;
	} /* End of processing subgraphs */

	/**
	 * METAS
	 */
	struct opt_meta* meta_list = current_state->metas;
	if (meta_list != NULL) {
		/* We cache the control bytes of the pos2 token. The pos token has not interest,
		 * because it is 1) a space  or 2) equal to the pos2 one. */
		if (token2 != -1)
			ctrl = p->token_control[token2];
		else
			ctrl = 0;
	}
	while (meta_list != NULL) {
		/* We process all the meta of the list */
		t = meta_list->transition;
		while (t != NULL) {
			/* We cache the output of the current tag, as well as values indicating if the
			 * current pos2 tokens matches the tag's morphological filter, if any. */
			output = p->tags[t->tag_number]->output;
			/* If there is no morphological filter, we act as if there was a matching one, except
			 * if we are at the end of the token buffer. With this trick, the morphofilter test
			 * will avoid overpassing the end of the token buffer. */
			int morpho_filter_OK = (token2 != -1) ? 1 : 0;
#ifdef TRE_WCHAR
			filter_number = p->tags[t->tag_number]->filter_number;
			if (token2 != -1) {
				morpho_filter_OK = (filter_number == -1 || token_match_filter(
						p->filter_match_index, token2, filter_number));
			}
#endif
			int negation = meta_list->negation;
			/* We use these variables to deal with match cases:
			 * the matching sequence is in the range [start;end[
			 * start=end means that the transition matches but that no token is read in the text,
			 * which is the case, for instance, with epsilon transitions. start=-1 means that the
			 * transition does not match. 'end' is the position for the next call
			 * to 'locate'. */
			int start = -1;
			int end = -1;
			switch (meta_list->meta) {

			case META_SHARP:
				if (token == -1 || token != p->SPACE) {
					/* # can match only if there is no space at the current position or
					 * if we are at the end of the token buffer. */
					start = pos;
					end = pos;
					if (token != -1) {
						/* We don't update the position if # has matched because of the
						 * end of the buffer */
						update_last_position(p, pos);
					}
				}
				break;

			case META_SPACE:
				if (token != -1 && token == p->SPACE) {
					/* The space meta can match only if there is a space at the current position.
					 * Note that we don't compare token and p->SPACE since p->SPACE can be -1 if
					 * the text contains no space. */
					start = pos;
					end = pos + 1;
					update_last_position(p, pos);
				}
				break;

			case META_EPSILON:
				/* We can always match the empty word */
				start = pos;
				end = pos;
				break;

			case META_MOT:
				update_last_position(p, pos2);
				if (!morpho_filter_OK || token2 == p->SENTENCE || token2
						== p->STOP) {
					/* <MOT> and <!MOT> must NEVER match {S} and {STOP}! */
					break;
				}
				if ((p->space_policy == START_WITH_SPACE) && (token2
						== p->SPACE) && negation) {
					/* If we want to catch a space with <!MOT> */
					start = pos;
					end = pos + 1;
				} else if (XOR(negation, ctrl & MOT_TOKEN_BIT_MASK)) {
					start = pos2;
					end = pos2 + 1;
				}
				break;

			case META_DIC:
				if (token2 == -1)
					break;
				update_last_position(p, pos2);
				if (token2 == p->STOP) {
					break;
				}
				if (!negation) {
					/* If there is no negation on DIC, we can look for a compound word */
					end_of_compound = find_compound_word(pos2,
							COMPOUND_WORD_PATTERN, p->DLC_tree, p);
					if (end_of_compound != -1) {
						/* If we find one, we must test if it matches the morphological filter, if any */
						int OK = 1;
#ifdef TRE_WCHAR
						if (filter_number == -1)
							OK = 1;
						else {
							unichar* sequence = get_token_sequence(p->buffer,
									p->tokens, pos2 + p->current_origin,
									end_of_compound + p->current_origin);
							OK = string_match_filter(p->filters, sequence,
									filter_number, p->recyclable_wchart_buffer);
							free(sequence);
						}
#endif
						if (OK) {
							/* <DIC> can match two things: a sequence of tokens or a single token
							 * As we don't want to process lists of [start,end[ ranges, we
							 * directly handle here the case of a token sequence. */
							if (p->output_policy == MERGE_OUTPUTS && pos2
									!= pos) {
								push_input_char(p->stack, ' ',
										p->protect_dic_chars);
							}
							if (p->output_policy != IGNORE_OUTPUTS) {
								if (!process_output(output, p)) {
									break;
								}
							}
							if (p->output_policy == MERGE_OUTPUTS) {
								for (int x = pos2; x <= end_of_compound; x++) {
									push_input_string(p->stack,
											p->tokens->value[p->buffer[x
													+ p->current_origin]],
											p->protect_dic_chars);
								}
							}
							locate(graph_depth,
									p->optimized_states[t->state_number],
									end_of_compound + 1, depth + 1, matches,
									n_matches, ctx, p, p_token_error_ctx,
									backup_reserve, p_count_cancel_trying,
									p_count_call, prv_alloc);
							p->stack->stack_pointer = stack_top;
						}
					}
					/* Now, we look for a simple word */
					if (ctrl & DIC_TOKEN_BIT_MASK && morpho_filter_OK) {
						start = pos2;
						end = pos2 + 1;
					}
					break;
				}
				/* We have the meta <!DIC> */
				if (ctrl & NOT_DIC_TOKEN_BIT_MASK && morpho_filter_OK) {
					start = pos2;
					end = pos2 + 1;
				}
				break;

			case META_SDIC:
				if (token2 == -1)
					break;
				update_last_position(p, pos2);
				if (token2 == p->STOP) {
					break;
				}
				if (morpho_filter_OK && (ctrl & DIC_TOKEN_BIT_MASK) && !(ctrl
						& CDIC_TOKEN_BIT_MASK)) {
					/* We match only simple words */
					start = pos2;
					end = pos2 + 1;
				}
				break;

			case META_CDIC:
				if (token2 == -1)
					break;
				update_last_position(p, pos2);
				if (token2 == p->STOP) {
					break;
				}
				end_of_compound = find_compound_word(pos2,
						COMPOUND_WORD_PATTERN, p->DLC_tree, p);
				if (end_of_compound != -1) {
					/* If we find a compound word */
					int OK = 1;
#ifdef TRE_WCHAR
					if (filter_number == -1)
						OK = 1;
					else {
						unichar* sequence = get_token_sequence(p->buffer,
								p->tokens, pos2 + p->current_origin,
								end_of_compound + p->current_origin);
						OK = string_match_filter(p->filters, sequence,
								filter_number, p->recyclable_wchart_buffer);
						free(sequence);
					}
#endif
					if (OK) {
						/* <CDIC> can match two things: a sequence of tokens or a compound
						 * tag token like "{black-eyed,.A}". As we don't want to process lists
						 * of [start,end[ ranges, we directly handle here the case of a
						 * token sequence. */
						if (p->output_policy == MERGE_OUTPUTS && pos2 != pos) {
							push_input_char(p->stack, ' ', p->protect_dic_chars);
						}
						if (p->output_policy != IGNORE_OUTPUTS) {
							if (!process_output(output, p)) {
								break;
							}
						}
						if (p->output_policy == MERGE_OUTPUTS) {
							for (int x = pos2; x <= end_of_compound; x++) {
								push_input_string(p->stack,
										p->tokens->value[p->buffer[x
												+ p->current_origin]],
										p->protect_dic_chars);
							}
						}
						locate(graph_depth,
								p->optimized_states[t->state_number],
								end_of_compound + 1, depth + 1, matches,
								n_matches, ctx, p, p_token_error_ctx,
								backup_reserve, p_count_cancel_trying,
								p_count_call, prv_alloc);
						p->stack->stack_pointer = stack_top;
					}
				}
				/* Anyway, we could have a tag compound word like {aujourd'hui,.ADV} */
				if (morpho_filter_OK && ctrl & CDIC_TOKEN_BIT_MASK) {
					start = pos2;
					end = pos2 + 1;
				}
				break;

			case META_TDIC:
				if (token2 == -1)
					break;
				update_last_position(p, pos2);
				if (token2 == p->STOP) {
					break;
				}
				if (morpho_filter_OK && (ctrl & TDIC_TOKEN_BIT_MASK)) {
					start = pos2;
					end = pos2 + 1;
				}
				break;

			case META_MAJ:
				if (token2 == -1)
					break;
				update_last_position(p, pos2);
				if (token2 == p->STOP) {
					break;
				}
				if (!morpho_filter_OK)
					break;
				if (!negation) {
					if (ctrl & MAJ_TOKEN_BIT_MASK) {
						start = pos;
						end = pos2 + 1;
					}
					break;
				}
				if (!(ctrl & MAJ_TOKEN_BIT_MASK) && (ctrl & MOT_TOKEN_BIT_MASK)) {
					/* If we have <!MAJ>, the matching token must be matched by <MOT> */
					start = pos2;
					end = pos2 + 1;
				}
				break;

			case META_MIN:
				if (token2 == -1)
					break;
				update_last_position(p, pos2);
				if (token2 == p->STOP) {
					break;
				}
				if (!morpho_filter_OK)
					break;
				if (!negation) {
					if (ctrl & MIN_TOKEN_BIT_MASK) {
						start = pos2;
						end = pos2 + 1;
					}
					break;
				}
				if (!(ctrl & MIN_TOKEN_BIT_MASK) && (ctrl & MOT_TOKEN_BIT_MASK)) {
					/* If we have <!MIN>, the matching token must be matched by <MOT> */
					start = pos2;
					end = pos2 + 1;
				}
				break;

			case META_PRE:
				if (token2 == -1)
					break;
				update_last_position(p, pos2);
				if (token2 == p->STOP) {
					break;
				}
				if (!morpho_filter_OK)
					break;
				if (!negation) {
					if (ctrl & PRE_TOKEN_BIT_MASK) {
						start = pos;
						end = pos2 + 1;
					}
					break;
				}
				if (!(ctrl & PRE_TOKEN_BIT_MASK) && (ctrl & MOT_TOKEN_BIT_MASK)) {
					/* If we have <!PRE>, the matching token must be matched by <MOT> */
					start = pos2;
					end = pos2 + 1;
				}
				break;

			case META_NB:
				if (token2 == -1)
					break;
				update_last_position(p, pos2);
				if (token2 == p->STOP) {
					break;
				}
				{ /* This block avoids visibility problem about 'z' */
					int z = pos2;
					while (z + p->current_origin < p->buffer_size
							&& is_a_digit_token(p->tokens->value[p->buffer[z
									+ p->current_origin]])) {
						z++;
					}
					if (z != pos2) {
						/* If we have found a contiguous digit sequence */
						start = pos2;
						end = z;
						if (z + p->current_origin == p->buffer_size) {
							/* If we have stopped because of the end of the buffer */
							update_last_position(p, pos2);
						} else {
							/* Otherwise, we have stopped because of a non matching token */
							update_last_position(p, pos2 + 1);
						}
					}
				}
				break;

			case META_TOKEN:
				if (token2 == -1)
					break;
				update_last_position(p, pos2);
				if (token2 == p->STOP || !morpho_filter_OK) {
					/* The {STOP} tag must NEVER be matched by any pattern */
					break;
				}
				start = pos2;
				end = pos2 + 1;
				break;

			case META_BEGIN_MORPHO:
				if (token2 == -1)
					break;
				if (token2 == p->STOP) {
					/* The {STOP} tag must NEVER be matched by any pattern */
					break;
				}
				if (p->output_policy == MERGE_OUTPUTS) {
					if (pos2 != pos)
						push_input_char(p->stack, ' ', p->protect_dic_chars);
				}
				/* we don't known if enter_morphological_mode will modify variable
				 so we increment the dirty flag, without any associated decrement, to be sure
				 having no problem */
				inc_dirty(backup_reserve)
				;
				enter_morphological_mode(graph_depth, t->state_number, pos2,
						depth + 1, matches, n_matches, ctx, p,
						p_token_error_ctx, prv_alloc);
				p->stack->stack_pointer = stack_top;
				break;

			case META_END_MORPHO:
				/* Should not happen */
				fatal_error("Unexpected morphological mode end tag $>\n");
				break;

			case META_LEFT_CONTEXT:
				int current_shift = p->left_ctx_shift;
				if (p->space_policy == START_WITH_SPACE) {
					p->left_ctx_shift = pos;
				} else {
					p->left_ctx_shift = pos2;
				}
				int old_left_ctx_stack_base = p->left_ctx_base;
				p->left_ctx_base = p->stack->stack_pointer + 1;
				/*int* var_backup=NULL;
				 if (p->output_policy!=IGNORE_OUTPUTS) {
				 var_backup=create_variable_backup(p->variables);
				 shift_variable_bounds(p->variables,real_origin-p->current_origin);
				 }*/
				locate(graph_depth, p->optimized_states[t->state_number], pos2,
						depth + 1, matches, n_matches, ctx, p,
						p_token_error_ctx, backup_reserve,
						p_count_cancel_trying, p_count_call, prv_alloc);
				/*if (p->output_policy!=IGNORE_OUTPUTS) {
				 install_variable_backup(p->variables,var_backup);
				 free_variable_backup(var_backup);
				 }*/
				p->left_ctx_shift = current_shift;
				p->left_ctx_base = old_left_ctx_stack_base;
				p->stack->stack_pointer = stack_top;
				break;

			} /* End of the switch */

			if (start != -1) {
				/* If the transition has matched */
				if (p->output_policy == MERGE_OUTPUTS && start == pos2 && pos2
						!= pos) {
					push_input_char(p->stack, ' ', p->protect_dic_chars);
				}
				if (p->output_policy != IGNORE_OUTPUTS) {
					/* We process its output */
					if (!process_output(output, p)) {
						goto next;
					}
				}
				if (p->output_policy == MERGE_OUTPUTS) {
					/* Then, if we are in merge mode, we push the tokens that have
					 * been read to the output */
					for (int y = start; y < end; y++) {
						push_input_string(p->stack,
								p->tokens->value[p->buffer[y
										+ p->current_origin]],
								p->protect_dic_chars);
					}
				}
				/* Then, we continue the exploration of the grammar */
				locate(graph_depth, p->optimized_states[t->state_number], end,
						depth + 1, matches, n_matches, ctx, p,
						p_token_error_ctx, backup_reserve,
						p_count_cancel_trying, p_count_call, prv_alloc);
				/* Once we have finished, we restore the stack */
				p->stack->stack_pointer = stack_top;
			}
			next: t = t->next;
		}
		meta_list = meta_list->next;
	}

	/**
	 * VARIABLE STARTS
	 */
	struct opt_variable* variable_list = current_state->variable_starts;
	while (variable_list != NULL) {

		inc_dirty(backup_reserve);
		int old = get_variable_start(p->variables,
				variable_list->variable_number);
		set_variable_start(p->variables, variable_list->variable_number, pos2);

		locate(graph_depth,
				p->optimized_states[variable_list->transition->state_number],
				pos, depth + 1, matches, n_matches, ctx, p, p_token_error_ctx,
				backup_reserve, p_count_cancel_trying, p_count_call, prv_alloc);
		p->stack->stack_pointer = stack_top;
		if (ctx == NULL) {
			/* We do not restore previous value if we are inside a context, in order
			 * to allow extracting things from contexts (see the
			 * "the cat is white" example in Unitex manual). */
			set_variable_start(p->variables, variable_list->variable_number,
					old);
			dec_dirty(backup_reserve);
			// restore dirty
		}
		variable_list = variable_list->next;
	}

	/**
	 * VARIABLE ENDS
	 */
	variable_list = current_state->variable_ends;
	while (variable_list != NULL) {

		inc_dirty(backup_reserve);
		int old =
				get_variable_end(p->variables, variable_list->variable_number);
		set_variable_end(p->variables, variable_list->variable_number, pos);

		locate(graph_depth,
				p->optimized_states[variable_list->transition->state_number],
				pos, depth + 1, matches, n_matches, ctx, p, p_token_error_ctx,
				backup_reserve, p_count_cancel_trying, p_count_call, prv_alloc);
		p->stack->stack_pointer = stack_top;
		if (ctx == NULL) {
			/* We do not restore previous value if we are inside a context, in order
			 * to allow extracting things from contexts (see the
			 * "the cat is white" example in Unitex manual). */
			set_variable_end(p->variables, variable_list->variable_number, old);
			dec_dirty(backup_reserve);
		}
		variable_list = variable_list->next;
	}

	/**
	 * CONTEXTS
	 */
	struct opt_contexts* contexts = current_state->contexts;
	if (contexts != NULL) {
		Transition* t;
		for (int n_ctxt = 0; n_ctxt < contexts->size_positive; n_ctxt = n_ctxt
				+ 2) {
			t = contexts->positive_mark[n_ctxt];
			/* We look for a positive context from the current position */
			struct list_int* c = new_list_int(0, ctx);
			locate(graph_depth, p->optimized_states[t->state_number], pos,
					depth + 1, NULL, 0, c, p, p_token_error_ctx,
					backup_reserve, p_count_cancel_trying, p_count_call, prv_alloc);
			/* Note that there is no matches to free since matches cannot be built within a context */
			p->stack->stack_pointer = stack_top;
			if (c->n) {
				/* If the context has matched, then we can explore all the paths
				 * that starts from the context end */
				Transition* states = contexts->positive_mark[n_ctxt + 1];
				while (states != NULL) {
					locate(graph_depth,
							p->optimized_states[states->state_number], pos,
							depth + 1, matches, n_matches, ctx, p,
							p_token_error_ctx, backup_reserve,
							p_count_cancel_trying, p_count_call, prv_alloc);
					p->stack->stack_pointer = stack_top;
					states = states->next;
				}
			}
			free(c);
		}
		for (int n_ctxt = 0; n_ctxt < contexts->size_negative; n_ctxt = n_ctxt
				+ 2) {
			t = contexts->negative_mark[n_ctxt];
			/* We look for a negative context from the current position */
			struct list_int* c = new_list_int(0, ctx);
			locate(graph_depth, p->optimized_states[t->state_number], pos,
					depth + 1, NULL, 0, c, p, p_token_error_ctx,
					backup_reserve, p_count_cancel_trying, p_count_call, prv_alloc);
			/* Note that there is no matches to free since matches cannot be built within a context */
			p->stack->stack_pointer = stack_top;
			if (!c->n) {
				/* If the context has not matched, then we can explore all the paths
				 * that starts from the context end */
				Transition* states = contexts->negative_mark[n_ctxt + 1];
				while (states != NULL) {
					locate(graph_depth,
							p->optimized_states[states->state_number], pos,
							depth + 1, matches, n_matches, ctx, p,
							p_token_error_ctx, backup_reserve,
							p_count_cancel_trying, p_count_call, prv_alloc);
					p->stack->stack_pointer = stack_top;
					states = states->next;
				}
			}
			/* We just want to free the cell we created */
			free(c);
		}
		if ((t = contexts->end_mark) != NULL) {
			/* If we have a closing context mark */
			if (ctx == NULL) {
				/* If there was no current opened context, it's an error */
				error(
						"ERROR: unexpected closing context mark in graph \"%S\"\n",
						p->fst2->graph_names[graph_depth + 1]);
				return;
			}
			/* Otherwise, we just indicate that we have found a context closing mark,
			 * and we return */
			ctx->n = 1;
			return;
		}
	}

	/**
	 * Now that we have finished with meta symbols, there can be no chance to advance
	 * in the grammar if we are at the end of the token buffer.
	 */
	if (token2 == -1) {
		return;
	}

	/**
	 * COMPOUND WORD PATTERNS:
	 * here, we deal with patterns that can only match compound sequences
	 */
	struct opt_pattern* pattern_list = current_state->compound_patterns;
	while (pattern_list != NULL) {
		t = pattern_list->transition;
		while (t != NULL) {
#ifdef TRE_WCHAR
			filter_number = p->tags[t->tag_number]->filter_number;
#endif
			output = p->tags[t->tag_number]->output;
			end_of_compound = find_compound_word(pos2,
					pattern_list->pattern_number, p->DLC_tree, p);
			if (end_of_compound != -1 && !(pattern_list->negation)) {
				int OK = 1;
#ifdef TRE_WCHAR
				if (filter_number == -1)
					OK = 1;
				else {
					unichar* sequence = get_token_sequence(p->buffer,
							p->tokens, pos2 + p->current_origin,
							end_of_compound + p->current_origin);
					OK = string_match_filter(p->filters, sequence,
							filter_number, p->recyclable_wchart_buffer);
					free(sequence);
				}
#endif
				if (OK) {
					if (p->output_policy == MERGE_OUTPUTS && pos2 != pos) {
						push_input_char(p->stack, ' ', p->protect_dic_chars);
					}
					if (p->output_policy != IGNORE_OUTPUTS) {
						if (!process_output(output, p)) {
							goto next4;
						}
					}
					if (p->output_policy == MERGE_OUTPUTS) {
						for (int x = pos2; x <= end_of_compound; x++) {
							push_input_string(p->stack,
									p->tokens->value[p->buffer[x
											+ p->current_origin]],
									p->protect_dic_chars);
						}
					}
					locate(graph_depth, p->optimized_states[t->state_number],
							end_of_compound + 1, depth + 1, matches, n_matches,
							ctx, p, p_token_error_ctx, backup_reserve,
							p_count_cancel_trying, p_count_call, prv_alloc);
					p->stack->stack_pointer = stack_top;
				}
			}
			next4: t = t->next;
		}
		pattern_list = pattern_list->next;
	}

	/**
	 * SIMPLE WORD PATTERNS:
	 * here, we deal with patterns that can match both simple and
	 * compound words, like "<N>".
	 */
	pattern_list = current_state->patterns;
	while (pattern_list != NULL) {
		t = pattern_list->transition;
		while (t != NULL) {
#ifdef TRE_WCHAR
			filter_number = p->tags[t->tag_number]->filter_number;
#endif
			output = p->tags[t->tag_number]->output;
			/* We try to match a compound word */
			end_of_compound = find_compound_word(pos2,
					pattern_list->pattern_number, p->DLC_tree, p);
			if (end_of_compound != -1 && !(pattern_list->negation)) {
				int OK = 1;
#ifdef TRE_WCHAR
				if (filter_number == -1)
					OK = 1;
				else {
					unichar* sequence = get_token_sequence(p->buffer,
							p->tokens, pos2 + p->current_origin,
							end_of_compound + p->current_origin);
					OK = string_match_filter(p->filters, sequence,
							filter_number, p->recyclable_wchart_buffer);
					free(sequence);
				}
#endif
				if (OK) {
					if (p->output_policy == MERGE_OUTPUTS && pos2 != pos) {
						push_input_char(p->stack, ' ', p->protect_dic_chars);
					}
					if (p->output_policy != IGNORE_OUTPUTS) {
						if (!process_output(output, p)) {
							goto next6;
						}
					}
					if (p->output_policy == MERGE_OUTPUTS) {
						for (int x = pos2; x <= end_of_compound; x++) {
							push_input_string(p->stack,
									p->tokens->value[p->buffer[x
											+ p->current_origin]],
									p->protect_dic_chars);
						}
					}
					locate(graph_depth, p->optimized_states[t->state_number],
							end_of_compound + 1, depth + 1, matches, n_matches,
							ctx, p, p_token_error_ctx, backup_reserve,
							p_count_cancel_trying, p_count_call, prv_alloc);
					p->stack->stack_pointer = stack_top;
				}
				/* This useless empty block is just here to enable the declaration of the next6 label */
				next6: {
				}
			}
			/* And now, we look for simple words */
			int OK = 1;
			update_last_position(p, pos2);
#ifdef TRE_WCHAR
			OK = (filter_number == -1 || token_match_filter(
					p->filter_match_index, token2, filter_number));
#endif
			if (OK) {
				if (p->matching_patterns[token2] != NULL) {
					if (XOR(get_value(p->matching_patterns[token2],
							pattern_list->pattern_number),
							pattern_list->negation)) {
						if (p->output_policy == MERGE_OUTPUTS && pos2 != pos) {
							push_input_char(p->stack, ' ', p->protect_dic_chars);
						}
						if (p->output_policy != IGNORE_OUTPUTS) {
							if (!process_output(output, p)) {
								goto next2;
							}
						}
						if (p->output_policy == MERGE_OUTPUTS) {
							push_input_string(p->stack,
									p->tokens->value[token2],
									p->protect_dic_chars);
						}
						locate(graph_depth,
								p->optimized_states[t->state_number], pos2 + 1,
								depth + 1, matches, n_matches, ctx, p,
								p_token_error_ctx, backup_reserve,
								p_count_cancel_trying, p_count_call, prv_alloc);
						p->stack->stack_pointer = stack_top;
					}
				} else {
					/* If the token matches no pattern, then it can match a pattern negation
					 * like <!V> */
					if (pattern_list->negation && (p->token_control[token2]
							& MOT_TOKEN_BIT_MASK)) {
						if (p->output_policy == MERGE_OUTPUTS && pos2 != pos) {
							push_input_char(p->stack, ' ', p->protect_dic_chars);
						}
						if (p->output_policy != IGNORE_OUTPUTS) {
							if (!process_output(output, p)) {
								goto next2;
							}
						}
						if (p->output_policy == MERGE_OUTPUTS) {
							push_input_string(p->stack,
									p->tokens->value[token2],
									p->protect_dic_chars);
						}
						locate(graph_depth,
								p->optimized_states[t->state_number], pos2 + 1,
								depth + 1, matches, n_matches, ctx, p,
								p_token_error_ctx, backup_reserve,
								p_count_cancel_trying, p_count_call, prv_alloc);
						p->stack->stack_pointer = stack_top;
					}
				}
			}
			next2: t = t->next;
		}
		pattern_list = pattern_list->next;
	}

	/**
	 * TOKENS
	 */
	if (current_state->number_of_tokens != 0) {
		update_last_position(p, pos2);
		int n = binary_search(token2, current_state->tokens,
				current_state->number_of_tokens);
		if (n != -1) {
			t = current_state->token_transitions[n];
			while (t != NULL) {
#ifdef TRE_WCHAR
				filter_number = p->tags[t->tag_number]->filter_number;
				if (filter_number == -1 || token_match_filter(
						p->filter_match_index, token2, filter_number))
#endif
				{
					output = p->tags[t->tag_number]->output;
					if (p->output_policy == MERGE_OUTPUTS && pos2 != pos) {
						push_input_char(p->stack, ' ', p->protect_dic_chars);
					}
					if (p->output_policy != IGNORE_OUTPUTS) {
						if (!process_output(output, p)) {
							goto next3;
						}
					}
					if (p->output_policy == MERGE_OUTPUTS) {
						push_input_string(p->stack, p->tokens->value[token2],
								p->protect_dic_chars);
					}
					locate(graph_depth, p->optimized_states[t->state_number],
							pos2 + 1, depth + 1, matches, n_matches, ctx, p,
							p_token_error_ctx, backup_reserve,
							p_count_cancel_trying, p_count_call, prv_alloc);
					p->stack->stack_pointer = stack_top;
				}
				next3: t = t->next;
			}
		}
	}
}

//modifier mode morpho
//ajouter fail fast si on n'a pas dépassé le premier token et qu'on n'a rien matché


/**
 * Looks for 'a' in the given array. Returns its position or -1 if not found.
 */
int binary_search(int a, int* t, int n) {
	register int start, middle;
	if (n == 0 || t == NULL)
		return -1;
	if (a < t[0])
		return -1;
	if (a > t[n - 1])
		return -1;
	n = n - 1;
	start = 0;
	while (start <= n) {
		middle = ((start + n) >> 1); // like /2, but faster (depends on compiler)
		if (t[middle] == a)
			return middle;
		if (t[middle] < a) {
			start = middle + 1;
		} else {
			n = middle - 1;
		}
	}
	return -1;
}

/**
 * This function compares the text to the compound word tree in order to find
 * the longest compound word thay have in common. It returns the position
 * of the last token of the compound word, or -1 if no compound word is found.
 */
int find_longest_compound_word(int pos, struct DLC_tree_node* node,
		int pattern_number, struct locate_parameters* p) {
	if (node == NULL) {
		fatal_error("NULL node in find_longest_compound_word\n");
	}
	update_last_position(p, pos);
	int current_token = p->buffer[pos + p->current_origin];
	int position_max;
	if (-1 != binary_search(pattern_number, node->array_of_patterns,
			node->number_of_patterns))
		position_max = pos - 1;
	else
		position_max = -1;
	if (pos + p->current_origin == p->buffer_size)
		return position_max;
	/* As the 'node->destination_tokens' array may contain duplicates, we look for
	 * one, and then we look before and after it, in order to examine all the
	 * duplicates. */
	int position = binary_search(current_token, node->destination_tokens,
			node->number_of_transitions);
	if (position == -1)
		return position_max;
	/* We look after it... */
	for (int i = position; i < node->number_of_transitions && current_token
			== node->destination_tokens[i]; i++) {
		/* If we can follow a transition tagged with the good token */
		int m = find_longest_compound_word(pos + 1, node->destination_nodes[i],
				pattern_number, p);
		if (m > position_max)
			position_max = m;
	}
	/* ...and before it */
	for (int i = position - 1; i >= 0 && current_token
			== node->destination_tokens[i]; i--) {
		/* If we can follow a transition tagged with the good token */
		int m = find_longest_compound_word(pos + 1, node->destination_nodes[i],
				pattern_number, p);
		if (m > position_max)
			position_max = m;
	}
	return position_max;
}

/**
 * Looks for a compound word from the position 'pos' in the text, that matches the
 * given pattern_number. Returns the position of the last token of the compound word
 * or -1 if no compound word is found. In case of a compound word that is a prefix
 * of another, the function considers the longest one.
 */
int find_compound_word(int pos, int pattern_number,
		struct DLC_tree_info* DLC_tree, struct locate_parameters* p) {
	return find_longest_compound_word(pos, DLC_tree->root, pattern_number, p);
}

/**
 * This function compares the text to the compound word tree in order to find
 * the longest compound word thay have in common. It returns the position
 * of the last token of the compound word, or -1 if no compound word is found.
 */
int find_compound_word_old_(int pos, struct DLC_tree_node* node,
		int pattern_number, struct locate_parameters* p) {
	int position_max, m, res;
	if (node == NULL)
		return -1;
	if (-1 != binary_search(pattern_number, node->array_of_patterns,
			node->number_of_patterns))
		position_max = pos - 1;
	else
		position_max = -1;
	if (pos + p->current_origin == p->buffer_size)
		return position_max;
	res = binary_search(p->buffer[pos + p->current_origin],
			node->destination_tokens, node->number_of_transitions);
	if (res == -1)
		return position_max;
	m = find_compound_word_old_(pos + 1, node->destination_nodes[res],
			pattern_number, p);
	if (m > position_max)
		return m;
	return position_max;
}

/**
 * Looks for a compound word from the position 'pos' in the text, that matches the
 * given pattern_number. Returns the position of the last token of the compound word
 * or -1 if no compound word is found. In case of a compound word that is a prefix
 * of another, the function considers the longest one.
 */
int find_compound_word_old(int pos, int pattern_number,
		struct DLC_tree_info* DLC_tree, struct locate_parameters* p) {
	int position_max, m, res;
	struct DLC_tree_node *n;
	if (pos + p->current_origin == p->buffer_size) {
		return -1;
	}
	if ((n = DLC_tree->index[p->buffer[pos + p->current_origin]]) == NULL) {
		return -1;
	}
	if (-1 != binary_search(pattern_number, n->array_of_patterns,
			n->number_of_patterns)) {
		position_max = pos;
	} else
		position_max = -1;
	pos++;
	if (pos + p->current_origin == p->buffer_size) {
		return -1;
	}
	res = binary_search(p->buffer[pos + p->current_origin],
			n->destination_tokens, n->number_of_transitions);
	if (res == -1) {
		return position_max;
	}
	m = find_compound_word_old_(pos + 1, n->destination_nodes[res],
			pattern_number, p);
	if (m > position_max) {
		return m;
	}
	return position_max;
}

/**
 * Returns a string corresponding to the tokens in the range [start;end].
 */
unichar* get_token_sequence(const int* token_array, struct string_hash* tokens,
		int start, int end) {
	int i;
	int l = 0;
	for (i = start; i <= end; i++) {
		l = l + u_strlen(tokens->value[token_array[i]]);
	}
	unichar* res = (unichar*) malloc((l + 1) * sizeof(unichar));
	if (res == NULL) {
		fatal_alloc_error("get_token_sequence");
	}
	l = 0;
	for (i = start; i <= end; i++) {
		u_strcpy(&(res[l]), tokens->value[token_array[i]]);
		l = l + u_strlen(tokens->value[token_array[i]]);
	}
	return res;
}

/**
 * Stores the given match in a list. All matches will be processed later.
 */
void add_match(int end, unichar* output, struct locate_parameters* p, Abstract_allocator prv_alloc) {
	if (end > p->last_matched_position) {
		p->last_matched_position = end;
	}
	int start = p->current_origin + p->left_ctx_shift;
	struct match_list* m = new_match(start, end, output, NULL, prv_alloc);
	if (p->match_cache_first == NULL) {
		p->match_cache_first = p->match_cache_last = m;
		return;
	}
	p->match_cache_last->next = m;
	p->match_cache_last = m;
}


void insert_in_all_matches_mode(struct match_list* m, struct locate_parameters* p, Abstract_allocator prv_alloc) {
int start = m->m.start_pos_in_token;
int end = m->m.end_pos_in_token;
unichar* output = m->output;
struct match_list* *L=&(p->match_list);
while (*L != NULL) {
	if ((*L)->m.start_pos_in_token == start
		&& (*L)->m.end_pos_in_token == end
		&& (p->ambiguous_output_policy != ALLOW_AMBIGUOUS_OUTPUTS || !u_strcmp((*L)->output, output))) {
		/* The match is already there, nothing to do */
		return;
	}
	if (start<(*L)->m.start_pos_in_token ||
			(start==(*L)->m.start_pos_in_token && end<(*L)->m.end_pos_in_token)) {
		/* We have to insert at the current position */
		(*L)=new_match(start, end, output, *L, prv_alloc);
		return;
	}
	L = &((*L)->next);
}
/* End of list? We add the match */
(*L) = new_match(start, end, output, NULL, prv_alloc);
}


/**
 * Adds a match to the global list of matches. The function takes into
 * account the match policy. For instance, we don't take [2;3] into account
 * if we are in longest match mode and if we already have [2;5].
 *
 * # Changed to allow different outputs in merge/replace
 * mode when the grammar is an ambiguous transducer (S.N.) */
void real_add_match(struct match_list* m, struct locate_parameters* p, Abstract_allocator prv_alloc) {
	//int start=p->current_origin+p->absolute_offset+p->left_ctx_shift;
	int start = m->m.start_pos_in_token;
	int end = m->m.end_pos_in_token;
	unichar* output = m->output;
	struct match_list *l;
	if (p->match_list == NULL) {
		/* If the match list was empty, we always can put the match in the list */
		p->match_list = new_match(start, end, output, NULL, prv_alloc);
		return;
	}
	switch (p->match_policy) {
	case LONGEST_MATCHES:
		/* We put new matches at the beginning of the list */
		if (end > p->match_list->m.end_pos_in_token) {
			/* In longest match mode, we only consider matches ending
			 * later. Moreover, we allow just one match from a given
			 * start position, except if ambiguous outputs are allowed. */
			if (p->match_list->m.start_pos_in_token == start) {
				/* We overwrite matches starting at same position but ending earlier.
				 * We do this by deleting the actual head element of the list
				 * and calling the function recursively.
				 * This works also for different outputs from ambiguous transducers,
				 * i.e. we may delete more than one match in the list. */
				l = p->match_list;
				p->match_list = p->match_list->next;
				free_match_list_element(l, prv_alloc);
				real_add_match(m, p, prv_alloc);
				return;
			}
			/* We allow add shorter matches but with other start position.
			 * Note that, by construction, we have start>p->match_list->start,
			 * that is to say that we have two matches that overlap: ( [ ) ]
			 */
			p->match_list = new_match(start, end, output, p->match_list, prv_alloc);
			return;
		}
		/* If we have the same start and the same end, we consider the
		 * new match only if ambiguous outputs are allowed */
		if (p->ambiguous_output_policy == ALLOW_AMBIGUOUS_OUTPUTS
				&& p->match_list->m.end_pos_in_token == end
				&& p->match_list->m.start_pos_in_token == start && u_strcmp(
				p->match_list->output, output)) {
			/* Because matches with same range and same output may not come
			 * one after another, we have to look if a match with same output
			 * already exists */
			l = p->match_list;
			while (l != NULL && u_strcmp(l->output, output)) {
				l = l->next;
			}
			if (l == NULL) {
				p->match_list = new_match(start, end, output, p->match_list, prv_alloc);
			}
		}
		break;

	case ALL_MATCHES:
		/* We unify identical matches, i.e. matches with same range (start and end),
		 * taking the output into account if ambiguous outputs are allowed. */
		insert_in_all_matches_mode(m,p,prv_alloc);
		return;

	case SHORTEST_MATCHES:
		/* We put the new match at the beginning of the list, but before, we
		 * test if the match we want to add may not be discarded because of
		 * a shortest match that would already be in the list. By the way,
		 * we eliminate matches that are longer than this one, if any. */
		int dont_add_match = 0;
		p->match_list = eliminate_longer_matches(p->match_list, start, end,
				output, &dont_add_match, p, prv_alloc);
		if (!dont_add_match) {
			p->match_list = new_match(start, end, output, p->match_list, prv_alloc);
		}
		break;
	}
}

/**
 * For given actual match:
 * 1. checks if there are "longer" matches in the list and eliminates them
 * 2. if there is no "shorter" match in the list, adds the actual match to the list
 *
 * # added for support of ambiguous transducers:
 * 3. matches with same range but different output are also accepted
 *
 * 'dont_add_match' is set to 1 if any shorter match is found, i.e. if we
 * won't have to insert the new match into the list; to 0 otherwise.
 * NOTE: 'dont_add_match' is supposed to be initialized at 0 before this
 *       funtion is called.
 */
struct match_list* eliminate_longer_matches(struct match_list *ptr, int start,
		int end, unichar* output, int *dont_add_match,
		struct locate_parameters* p, Abstract_allocator prv_alloc) {
	struct match_list *l;
	if (ptr == NULL)
		return NULL;
	if (p->ambiguous_output_policy == ALLOW_AMBIGUOUS_OUTPUTS
			&& ptr->m.start_pos_in_token == start && ptr->m.end_pos_in_token
			== end && u_strcmp(ptr->output, output)) {
		/* In the case of ambiguous transductions producing different outputs,
		 * we accept matches with same range */
		ptr->next = eliminate_longer_matches(ptr->next, start, end, output,
				dont_add_match, p, prv_alloc);
		return ptr;
	}
	if (start >= ptr->m.start_pos_in_token && end <= ptr->m.end_pos_in_token) {
		/* If the new match is shorter (or of equal length) than the current one
		 * in the list, we replace the match in the list by the new one */
		if (*dont_add_match) {
			/* If we have already noticed that the match mustn't be added
			 * to the list, we delete the current list element */
			l = ptr->next;
			free_match_list_element(ptr, prv_alloc);
			return eliminate_longer_matches(l, start, end, output,
					dont_add_match, p, prv_alloc);
		}
		/* If the new match is shorter than the current one in the list, then we
		 * update the current one with the value of the new match. */
		ptr->m.start_pos_in_token = start;
		ptr->m.end_pos_in_token = end;
		if (ptr->output != NULL)
			free(ptr->output);
		ptr->output = u_strdup(output);
		/* We note that the match does not need anymore to be added */
		(*dont_add_match) = 1;
		ptr->next = eliminate_longer_matches(ptr->next, start, end, output,
				dont_add_match, p, prv_alloc);
		return ptr;
	}
	if (start <= ptr->m.start_pos_in_token && end >= ptr->m.end_pos_in_token) {
		/* The new match is longer than the one in list => we
		 * skip the new match */
		(*dont_add_match) = 1;
		return ptr;
	}
	/* If we have disjunct ranges or overlapping ranges without inclusion,
	 * we examine recursively the rest of the list */
	ptr->next = eliminate_longer_matches(ptr->next, start, end, output,
			dont_add_match, p, prv_alloc);
	return ptr;
}

/**
 * Writes the matches to the file concord.ind. The matches are in
 * left-most longest order. 'current_position' represents the current
 * position in the text. It is used to determine when we can save matches:
 * when we are at position 246, matches that end before 246 cannot be
 * modifyied anymore by the shortest or longest match heuristic, so we can
 * save them.
 *
 * wrong results for all matches when modifying text ??
 * <E>/[[ <MIN>* <PRE> <MIN>* <E>/]]
 * left-most stehen am Anfang der Liste
 */
struct match_list* save_matches(struct match_list* l, int current_position,
		U_FILE* f, struct locate_parameters* p, Abstract_allocator prv_alloc) {
struct match_list *ptr;

if (l == NULL)
	return NULL;
	if (l->m.end_pos_in_token < current_position) {
		/* we can save the match (necessary for SHORTEST_MATCHES: there
		 * may be no shorter match) */

		/* We save the match according to the new concord.ind format
		 * that takes into account 3 kinds of information:
		 *   1) offset in token
		 *   2) offset in char inside the token
		 *   3) offset in logical letter inside the current char (for Korean) */
		u_fprintf(f, "%d.0.0 %d.%d.0", l->m.start_pos_in_token,
				l->m.end_pos_in_token, u_strlen(
						p->tokens->value[p->buffer[l->m.end_pos_in_token]]) - 1);
		if (l->output != NULL) {
			/* If there is an output */
			u_fprintf(f, " %S", l->output);
		}
		u_fprintf(f, "\n");
		if (p->ambiguous_output_policy == ALLOW_AMBIGUOUS_OUTPUTS) {
			(p->number_of_outputs)++;
			/* If we allow different outputs for ambiguous transducers,
			 * we have to distinguish between matches and outputs
			 * The algorithm is based on the following considerations:
			 *  - l has all matches with same starting point in one block,
			 *    because they are inserted in one turn (Locate runs from left
			 *    to right through the text)
			 *  - since we consider only matches right from actual position,
			 *    matches with same range (start and end position) always follow consecutively.
			 *  - the start and end positions of the last printed match are stored in the
			 *    Locate parameters
			 *  - if the range differs (start and/or end position are different),
			 *    a new match is counted
			 */
			if (!(p->start_position_last_printed_match
					== l->m.start_pos_in_token
					&& p->end_position_last_printed_match
							== l->m.end_pos_in_token)) {
				(p->number_of_matches)++;
			}
		} else {
			/* If we don't allow ambiguous outputs, we count the matches */
			(p->number_of_matches)++;
		}
		// To count the number of matched tokens this won't work:
		//  p->matching_units=p->matching_units+(l->end+1)-(l->start);
		// or you get outputs like:
		//  1647 matches
		//  4101 recognized units
		//  (221.916% of the text is covered)
		// because of overlapping matches or the option "All matches" is choosed.
		// For options "Shortest" and "Longest matches", the last start and end
		// position are sufficient to calculate the correct coverage.
		// For all matches this is not the case. Suppose you have the matches at token pos:
		//  0 1 2 3 4 5
		//  XXX
		//    YYY
		//  ZZZZZ
		// The resulting concord.ind file will look like (for sort ordering see above)
		//   0 1 X
		//   1 2 Y
		//   0 2 Z
		// So when processing match Z we don't know, that token 0 has been already counted.
		// I guess a bit array is needed to count correctly.
		// But since for "Longest matches" only Z, and for "Shortest" only X and Y are
		// accepted, and the option "All matches" is rarely used, I (S.N.) propose:
		if (p->end_position_last_printed_match != current_position - 1) {
			// initial (non-recursive) call of function:
			// then check if match is out of range of previous matches
			if (p->end_position_last_printed_match < l->m.start_pos_in_token) { // out of range
				p->matching_units += (l->m.end_pos_in_token + 1)
						- (l->m.start_pos_in_token);
			} else {
				p->matching_units += (l->m.end_pos_in_token + 1)
						- (p->end_position_last_printed_match + 1);
			}
		}
		// else:
		//  recursive call, i.e. end position of match was already counted:
		//  for "longest" and "shortest" matches all is done, for option "all"
		//  it is possible that a token won't be counted (in the above example,
		//  when there is no match X), this will lead to an incorrect displayed
		//  coverage, lower than correct.
		else {
			// this may make the coverage greater than correct:
			if (p->start_position_last_printed_match > l->m.start_pos_in_token) {
				p->matching_units += p->start_position_last_printed_match
						- (l->m.start_pos_in_token);
			}
		}
		p->start_position_last_printed_match = l->m.start_pos_in_token;
		p->end_position_last_printed_match = l->m.end_pos_in_token;
		if (p->number_of_matches == p->search_limit) {
			/* If we have reached the search limitation, we free the remaining
			 * matches and return */
			while (l != NULL) {
				ptr = l;
				l = l->next;
				free_match_list_element(ptr, prv_alloc);
			}
			return NULL;
		}
		ptr = l->next;
		free_match_list_element(l, prv_alloc);
		return save_matches(ptr, current_position, f, p, prv_alloc);
	}
	l->next = save_matches(l->next, current_position, f, p, prv_alloc);
	return l;
}

/**
 * Apply the given shift to all variable bounds in the given variable set.
 */
void shift_variable_bounds(Variables* v, int shift) {
	if (v == NULL) {
		return;
	}
	int l = v->variable_index->size;
	for (int i = 0; i < l; i++) {
		if (v->variables[i].start != UNDEF_VAR_BOUND) {
			v->variables[i].start += shift;
		}
		if (v->variables[i].end != UNDEF_VAR_BOUND) {
			v->variables[i].end += shift;
		}
	}
}

