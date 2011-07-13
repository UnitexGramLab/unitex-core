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
#include "DebugMode.h"


/* Delay between two prints (yyy% done) */
#define DELAY CLOCKS_PER_SEC

static int binary_search(int, int*, int);
static int find_compound_word(int, int, struct DLC_tree_info*,
		struct locate_parameters*);
static unichar* get_token_sequence(struct locate_parameters*, int, int);
void enter_morphological_mode(int, int, int, int, struct parsing_info**, int,
		struct list_int*, struct locate_parameters*, struct Token_error_ctx*);
void shift_variable_bounds(Variables*, int);
static void add_match(int, unichar*, struct locate_parameters*, Abstract_allocator);
static void real_add_match(struct match_list*, struct locate_parameters*, Abstract_allocator);
static struct match_list* eliminate_longer_matches(struct match_list*, int, int,
		unichar*, int*, struct locate_parameters*, Abstract_allocator);
static struct match_list* eliminate_shorter_matches(struct match_list*, int, int,
		unichar*, int*, struct locate_parameters*, Abstract_allocator);
static struct match_list* save_matches(struct match_list*, int, U_FILE*,
		struct locate_parameters*, Abstract_allocator);
static inline int at_text_start(struct locate_parameters*,int);


static long CalcPerfHalfHundred(long text_size, long matching_units) {
	unsigned long text_size_calc_per_halfhundred = text_size;
	unsigned long matching_units_per_halfhundred = (unsigned long)(matching_units);
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
        if (factor_matching_units != 0)
            if ((text_size_calc_per_halfhundred / factor) != 0)
	    per_halfhundred = (long)(((matching_units_per_halfhundred * multiplicator ) / factor_matching_units) / (text_size_calc_per_halfhundred / factor));
	}
	return per_halfhundred;
}

/**
 * Performs the Locate operation on the text, saving the occurrences
 * on the fly.
 */
void launch_locate(U_FILE* out, long int text_size, U_FILE* info,
		struct locate_parameters* p) {
	p->token_error_ctx.n_errors = 0;
	p->token_error_ctx.last_start = -1;
	p->token_error_ctx.last_length = 0;
	p->token_error_ctx.n_matches_at_token_pos__locate = 0;
	p->token_error_ctx.n_matches_at_token_pos__morphological_locate = 0;


	p->is_in_trace_state = 0;
	if ((p->fnc_locate_trace_step != NULL))
		p->is_in_trace_state = 1;

	//fill_buffer(p->token_buffer, f);
	OptimizedFst2State initial_state =
			p->optimized_states[p->fst2->initial_states[1]];
	p->current_origin = 0;
	int n_read = 0;
	int unite;
	clock_t startTime = clock();
	clock_t currentTime;
	unsigned long total_count_step = 0;

	unite = (int)(((text_size / 100) > 1000) ? (text_size / 100) : 1000);
	variable_backup_memory_reserve* backup_reserve =
			create_variable_backup_memory_reserve(p->input_variables,1);
    p->backup_memory_reserve = backup_reserve;
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

			int cache_found = 0;
            if (p->useLocateCache)
                cache_found =  consult_cache(p->buffer, p->current_origin,
					p->buffer_size, p->match_cache,
					p->cached_match_vector);
			if (cache_found) {
				/* If we have found matches in the cache, we use them */
				for (int i=0;i<p->cached_match_vector->nbelems;i++) {
					struct match_list* tmp=(struct match_list*)(p->cached_match_vector->tab[i]);
					while (tmp!=NULL) {
						/* We have to adjust the match coordinates */
						int size=tmp->m.end_pos_in_token-tmp->m.start_pos_in_token;
						tmp->m.start_pos_in_token=p->current_origin;
						tmp->m.end_pos_in_token=tmp->m.start_pos_in_token+size;
						real_add_match(tmp,p,p->prv_alloc);
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

                p->counting_step.count_call=0;
                p->counting_step.count_cancel_trying=0;
				p->last_tested_position = 0;
				p->last_matched_position = -1;
                p->graph_depth=0;
                p->explore_depth=-1;
                p->token_error_ctx.n_matches_at_token_pos__morphological_locate = 0;

                if (p->is_in_cancel_state == 1)
                  p->is_in_cancel_state = 0;
                p->counting_step_count_cancel_trying_real_in_debug_or_trace = 0;
                p->no_fail_fast=0;
                p->weight=-1;
                int n_matches=0;
				locate(/*0,*/ initial_state, 0,/* 0,*/ &matches, &n_matches, NULL, p);


				int count_call_real = p->counting_step.count_call;
				count_call_real -= (p->is_in_trace_state == 0) ? (p->counting_step.count_cancel_trying) : (p->counting_step_count_cancel_trying_real_in_debug_or_trace);


//u_printf("token number %d : %d step\n",p->current_origin,count_call_real,p->tokens);

				total_count_step += (unsigned long)count_call_real;

				if ((p->max_count_call > 0)
						&& (p->counting_step.count_call >= p->max_count_call)) {
					u_printf(
							"stop computing token %u after %u step computing.\n",
							p->current_origin, p->counting_step.count_call);
				} else if ((p->max_count_call_warning > 0) && (p->counting_step.count_call
						>= p->max_count_call_warning)) {
					u_printf(
							"warning : computing token %u take %u step computing.\n",
							p->current_origin, p->counting_step.count_call);
				}
				int can_cache_matches = 0;
				p->last_tested_position=p->last_tested_position+p->current_origin;
				if (p->last_matched_position == -1) {
					if (p->last_tested_position == p->current_origin
							&& !u_is_digit(p->tokens->value[current_token][0])
							&& !p->no_fail_fast) {
						/* We are in the fail fast case, nothing has been matched while
						 * looking only at the first current token. That means that no match
						 * could ever happen when this token is found in the text.
						 *
						 * NOTE: we add the digit test because if the fail came from
						 * something like <NB><<....>>, then it may have failed on a token
						 * because of the morphological filter, not because of the first
						 * token itself */
						set_value(p->failfast, current_token, 1);
					}
				} else {
					if (p->last_tested_position <= p->last_matched_position
							&& !at_text_start(p,0)) {
						/* If there are matches that could never be longer, we
						 * can cache them, BUT, we never cache a match that occurred
						 * at the beginning of the text, since it may be a contextual
						 * match depending on the {^} meta */
						can_cache_matches = 1;
					}
				}
				struct match_list* tmp;
				while (p->match_cache_first != NULL) {
					real_add_match(p->match_cache_first, p, p->prv_alloc);
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
								&(p->match_cache[current_token]), p->prv_alloc);
					} else {
						free_match_list_element(tmp, p->prv_alloc);
					}
				}
				p->match_cache_last = NULL;
				free_parsing_info(matches, p->prv_alloc_recycle,p->prv_alloc);
                if (p->dic_variables != NULL) {
                    clear_dic_variable_list(&(p->dic_variables));
                }
			}
		}
		p->match_list = save_matches(p->match_list,p->current_origin, out, p, p->prv_alloc);
		(p->current_origin)++;
	}
	free_reserve(backup_reserve);
    p->backup_memory_reserve = NULL;

	p->match_list = save_matches(p->match_list,p->current_origin+1, out, p, p->prv_alloc);
	u_printf("100%% done      \n\n");
	u_printf("%d match%s\n", p->number_of_matches,
			(p->number_of_matches == 1) ? "" : "es");
	if ((p->number_of_outputs != p->number_of_matches) && (p->number_of_outputs
			!= 0))
		u_printf("(%d output%s)\n", p->number_of_outputs, (p->number_of_outputs
				== 1) ? "" : "s");
	u_printf("%d recognized units\n", p->matching_units);

	long per_halfhundred=CalcPerfHalfHundred(text_size,p->matching_units);

	if (text_size != 0) {
		u_printf("(%2.3f%% of the text is covered)\n", (float) (((float)per_halfhundred)
				/ (float) 1000.0));
	}
	u_printf("%u exploration step\n",(unsigned int)total_count_step);

    /*
    {
        char sz[0x100];
        sprintf(sz,"\n%lu exploration step",total_count_step);
        puts(sz);
    }*/

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
					(float) (((float)per_halfhundred) / (float) 1000.0));
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
		struct locate_parameters* p) {
	//static int n_errors;
	//static int last_start=-1;
	//static int last_length;
	int i;
	if ((p->token_error_ctx.last_start) == start) {
		/* The context was already printed */
		return;
	}
	error("%s\n  ", message);
	for (i = (start - 4); (i <= (start + 20)) && (i < p->buffer_size); i++) {
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
	(p->token_error_ctx.n_errors)++;
	if (!p->debug && p->token_error_ctx.n_errors >= MAX_ERRORS) {
		/* In debug mode, we don't stop on such a problem */
		fatal_error("Too many errors, giving up!\n");
	}
	p->token_error_ctx.last_start = start;
	p->token_error_ctx.last_length = length;
}


static inline void update_last_tested_position(struct locate_parameters* p, int pos) {
	if (pos > p->last_tested_position) {
		p->last_tested_position = pos;
	}
}


//
// return 1 if s is a digit sequence, 0 else
//
static inline int local_is_not_a_digit_token(const unichar* s) {
int i=0;
while (s[i]!='\0') {
   if (s[i]<'0' || s[i]>'9') {
      return 1;
   }
   i++;
}
return 0;
}


static inline int at_text_start(struct locate_parameters* p,int pos) {
int ret=pos == 0 && (p->current_origin == 0
				|| (p->current_origin == 1 && p->buffer[0] == p->SPACE));
return ret;
}


/**
 * This is the core function of the Locate program.
 */
void locate(/*int graph_depth,*/ /* 0 means that we are in the top level graph */
OptimizedFst2State current_state, /* current state in the grammar */
int pos, /* position in the token buffer, relative to the current origin */
//int depth, /* number of nested calls to 'locate' */
struct parsing_info** matches, /* current match list. Irrelevant if graph_depth==0 */
int *n_matches, /* number of sequences that have matched. It may be different from
 * the length of the 'matches' list if a given sequence can be
 * matched in several ways. It is used to detect combinatorial
 * explosions due to bad written grammars. */
struct list_context* ctx, /* information about the current context, if any */
struct locate_parameters* p /* miscellaneous parameters needed by the function */
) {
#ifdef TRE_WCHAR
	int filter_number;
#endif
	int pos2 = -1, ctrl = 0, end_of_compound;
	int token, token2;
	Transition* t1;
	int stack_top = p->stack->stack_pointer;
	int old_weight1=p->weight;
	unichar* output;
	int captured_chars;

	if ((p->counting_step.count_cancel_trying) == 0) {

		if (p->is_in_cancel_state != 0)
			return;

		if (p->is_in_trace_state == 0) {

			if ((p->max_count_call) > 0) {
				if ((p->counting_step.count_call) >= (p->max_count_call)) {
					p->counting_step.count_cancel_trying = 0;
					p->is_in_cancel_state = 1;
					return;
				}
			}

			p->counting_step.count_cancel_trying = COUNT_CANCEL_TRYING_INIT_CONST;
			if (((p->max_count_call) > 0) && (((p->counting_step.count_call)
					+COUNT_CANCEL_TRYING_INIT_CONST) > (p->max_count_call))) {
				p->counting_step.count_cancel_trying = (p->max_count_call) - (p->counting_step.count_call);
			}

			if (is_cancelling_requested() != 0) {
				p->counting_step.count_cancel_trying = 0;
				p->is_in_cancel_state = 2;
				return;
			}
			(p->counting_step.count_call) += (p->counting_step.count_cancel_trying);
		}
		else
		{
			if ((p->fnc_locate_trace_step != NULL)) {
				locate_trace_info* lti;
				lti = (locate_trace_info*)malloc(sizeof(locate_trace_info));				
				if (lti==NULL) {
					fatal_alloc_error("locate");
				}
				lti->size_struct_locate_trace_info = (int)sizeof(locate_trace_info);
				lti->is_on_morphlogical = 0;

				lti->pos_in_tokens=pos;

				lti->current_state=current_state;

				lti->current_state_index=0;
				lti->pos_in_chars=0;

				lti->matches=matches;
				lti->n_matches=*n_matches;
				lti->ctx=ctx;
				lti->p=p;

				lti->step_number=p->counting_step.count_call-p->counting_step_count_cancel_trying_real_in_debug_or_trace;

				lti->jamo=NULL;
				lti->pos_in_jamo=0;

				p->is_in_cancel_state = (*(p->fnc_locate_trace_step))(lti,p->private_param_locate_trace);
				free(lti);
			}

			if ((p->counting_step_count_cancel_trying_real_in_debug_or_trace) == 0) {
				if ((p->max_count_call) > 0) {
					if ((p->counting_step.count_call) >= (p->max_count_call)) {
						p->counting_step.count_cancel_trying = 0;
						p->is_in_cancel_state = 1;
						return;
					}
				}

				p->counting_step_count_cancel_trying_real_in_debug_or_trace = COUNT_CANCEL_TRYING_INIT_CONST;
				if (((p->max_count_call) > 0) && (((p->counting_step.count_call)
						+COUNT_CANCEL_TRYING_INIT_CONST) > (p->max_count_call))) {
					p->counting_step_count_cancel_trying_real_in_debug_or_trace = (p->max_count_call) - (p->counting_step.count_call);
				}

				if (is_cancelling_requested() != 0) {
					p->counting_step.count_cancel_trying = 0;
					p->is_in_cancel_state = 2;
					return;
				}
				(p->counting_step.count_call) += (p->counting_step_count_cancel_trying_real_in_debug_or_trace);
			}

			p->counting_step_count_cancel_trying_real_in_debug_or_trace --;
			//  prepare now to be at 0 after the "--" (we don't want another test for performance */
			(p->counting_step.count_cancel_trying)++;
		}
	}
	(p->counting_step.count_cancel_trying)--;

	/* The following static variable holds the number of matches at
	 * one position in text. */
	//static int n_matches_at_token_pos;


	p->explore_depth ++ ;

/*
	if (p->explore_depth == 0) {
		// We reset if this is first call to 'locate' from a given position in the text
		p->p_token_error_ctx->n_matches_at_token_pos__morphological_locate = 0;
	}
*/


	if ((p->explore_depth) > STACK_MAX) {
		/* If there are too much recursive calls */
		error_at_token_pos("\nMaximal stack size reached!\n"
			"(There may be longer matches not recognized!)", p->current_origin,
				pos, p);
		p->explore_depth -- ;
		return;
	}
	if ((p->token_error_ctx.n_matches_at_token_pos__morphological_locate)
			> MAX_MATCHES_AT_TOKEN_POS) {
		/* If there are too much matches from the current origin in the text */
		error_at_token_pos(
				"\nToo many (ambiguous) matches starting from one position in text!",
				p->current_origin, pos, p);
		p->explore_depth -- ;
		return;
	}
	if (current_state->control & 1) {
		/* If we are in a final state... */
		if (ctx != NULL) {
			/* If we have reached the final state of a graph while
			 * looking for a context, it's an error because every
			 * opened context must be closed before the end of the graph. */
			fatal_error("ERROR: unclosed context in graph \"%S\"\n",
					p->fst2->graph_names[(p->graph_depth) + 1]);
		}
		/* In we are in the top level graph, we have a match */
		if ((p->graph_depth) == 0) {
			(p->token_error_ctx.n_matches_at_token_pos__morphological_locate)++;
			if (p->output_policy == IGNORE_OUTPUTS) {
				if (pos > 0) {
					add_match(pos + p->current_origin-1,NULL, p, p->prv_alloc);
				} else {
					add_match(pos + p->current_origin,NULL, p, p->prv_alloc);
				}
			} else {
				p->stack->stack[stack_top + 1] = '\0';
				if (pos > 0) {
					add_match(pos + p->current_origin-1,
							p->stack->stack + p->left_ctx_base, p, p->prv_alloc);
				} else {
					add_match(pos + p->current_origin,
							p->stack->stack + p->left_ctx_base, p, p->prv_alloc);
				}
			}
		} else {
			/* If we are in a subgraph */
			if (*n_matches >= MAX_MATCHES_PER_SUBGRAPH) {
				/* If there are too much matches, we suspect an error in the grammar
				 * like an infinite recursion */
				error_at_token_pos(
						"\nMaximal number of matches per subgraph reached!",
						p->current_origin, pos, p);
				p->explore_depth -- ;
				return;
			} else {
				/* If everything is fine, we add this match to the match list of the
				 * current graph level */
				(*n_matches)++;
				p->stack->stack[stack_top + 1] = '\0';
				if (p->ambiguous_output_policy == ALLOW_AMBIGUOUS_OUTPUTS) {
					(*matches) = insert_if_different(pos, -1, -1, (*matches),
							p->stack->stack_pointer,
							&(p->stack->stack[p->stack_base + 1]),
							p->input_variables, p->output_variables,p->dic_variables, p->left_ctx_shift,
							p->left_ctx_base, NULL, -1, NULL, p->weight,p->prv_alloc_recycle,p->prv_alloc);
				} else {
					(*matches) = insert_if_absent(pos, -1, -1, (*matches),
							p->stack->stack_pointer,
							&(p->stack->stack[p->stack_base + 1]),
							p->input_variables, p->output_variables,p->dic_variables, p->left_ctx_shift,
							p->left_ctx_base, NULL, -1, NULL, p->weight,p->prv_alloc_recycle,p->prv_alloc);
				}
			}
		}
	}
	int end_of_text = pos + p->current_origin == p->buffer_size;
	/* The same, but tolerant if there is a remaining space */
	int end_of_text2 = pos + p->current_origin + 1 == p->buffer_size
			&& p->buffer[pos + p->current_origin] == p->SPACE;

	/* If we have reached the end of the token buffer, we indicate it by setting
	 * the current tokens to -1 */
	if ((((pos + p->current_origin) >= p->buffer_size)) || (pos==-1)) {
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
		if ((pos2 + p->current_origin) >= p->buffer_size) {
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
		//int create_new_reserve_done = 0;
        variable_backup_memory_reserve* reserve_previous = NULL;

		struct dic_variable* dic_variables_backup = NULL;
		int old_StackBase = p->stack_base;
		unichar* output_var_backup=NULL;
		int old_weight2=p->weight;
		if (p->output_policy != IGNORE_OUTPUTS) {
			/* For better performance when ignoring outputs */

			if (is_enough_memory_in_reserve_for_transduction_variable_set(p->input_variables,
					p->backup_memory_reserve) == 0) {
				reserve_previous=p->backup_memory_reserve;
				p->backup_memory_reserve = create_variable_backup_memory_reserve(
						p->input_variables,0);
				//create_new_reserve_done = 1;
			}

			var_backup = create_variable_backup_using_reserve(p->input_variables,
					p->backup_memory_reserve);
			dic_variables_backup = p->dic_variables;
			if (p->nb_output_variables != 0) {
			    output_var_backup = create_output_variable_backup(p->output_variables);
			}
		}

		do {
			/* For each graph call, we look all the reachable states */
			t1 = graph_call_list->transition;
			while (t1 != NULL) {
				/* We reset some parameters before exploring the subgraph */
				struct parsing_info* L = NULL;
				p->stack_base = p->stack->stack_pointer;
				p->dic_variables = NULL;
				if (dic_variables_backup != NULL) {
				    p->dic_variables = clone_dic_variable_list(dic_variables_backup);
				}
				if (p->nb_output_variables != 0) {
				    install_output_variable_backup(p->output_variables,output_var_backup);
				}

				p->graph_depth ++ ;
				p->weight=-1;
				int n_matches_in_subgraph=0;
				locate(/*graph_depth + 1,*/ /* Exploration of the subgraph */
				       p->optimized_states[p->fst2->initial_states[graph_call_list->graph_number]],
				       pos, &L, &n_matches_in_subgraph, NULL, /* ctx is set to NULL because the end of a context must occur in the
						 * same graph than its beginning */
				       p);
				p->graph_depth -- ;

				p->stack_base = old_StackBase;
				if (p->dic_variables != NULL) {
				    clear_dic_variable_list(&(p->dic_variables));
				}
				if (L != NULL) {
					struct parsing_info* L_first = L;
					/* If there is at least one match, we process the match list */
					do {
						/* We restore the settings that we had at the end of the subgraph exploration */
						if (p->output_policy != IGNORE_OUTPUTS) {
							u_strcpy(&(p->stack->stack[stack_top + 1]),
									L->stack);
							p->stack->stack_pointer = L->stack_pointer;

							if (save_previous_ptr_var == NULL) {
								save_previous_ptr_var
										= install_variable_backup_preserving(
												p->input_variables, p->backup_memory_reserve,
												L->input_variable_backup);
							} else {
								install_variable_backup(p->input_variables,
										L->input_variable_backup);
							}

							p->dic_variables = NULL;
							if (L->dic_variable_backup != NULL) {
							    p->dic_variables = clone_dic_variable_list(L->dic_variable_backup);
							}

							if (p->nb_output_variables != 0) {
							    install_output_variable_backup(p->output_variables,L->output_variable_backup);
							}
						}
						/* And we continue the exploration */
						int old_left_ctx_shift = p->left_ctx_shift;
						int old_left_ctx_base = p->left_ctx_base;
						p->left_ctx_shift = L->left_ctx_shift;
						p->left_ctx_base = L->left_ctx_base;

//variable_backup_memory_reserve* reserve_new=p->backup_memory_reserve;
//p->backup_memory_reserve=reserve_previous;

						p->weight=old_weight2;
						locate(/*graph_depth,*/
								p->optimized_states[t1->state_number],
								L->pos_in_tokens, matches, n_matches,
								ctx, p);

//p->backup_memory_reserve=reserve_new;

						p->left_ctx_shift = old_left_ctx_shift;
						p->left_ctx_base = old_left_ctx_base;
						p->stack->stack_pointer = stack_top;
						if (p->dic_variables != NULL) {
						    clear_dic_variable_list(&(p->dic_variables));
						}
						if (p->graph_depth == 0) {
							/* If we are at the top graph level, we restore the variables */
							if (p->output_policy != IGNORE_OUTPUTS) {
								if (save_previous_ptr_var != NULL) {
									restore_variable_array(p->input_variables,
											p->backup_memory_reserve, save_previous_ptr_var);
									save_previous_ptr_var = NULL;
								}
							}
						}
						L = L->next;
					} while (L != NULL);
					free_parsing_info(L_first, p->prv_alloc_recycle,p->prv_alloc); //  free all subgraph matches
				}
				/* As free_parsing_info has freed p->dic_variables, we must restore it */
				t1 = t1->next;
			} /* end of while (t!=NULL) */
		} while ((graph_call_list = graph_call_list->next) != NULL);
		/* Finally, we have to restore the stack and other backup stuff */
		p->weight=old_weight2;
		p->stack->stack_pointer = stack_top;
		p->stack_base = old_StackBase; /* May be changed by recursive subgraph calls */
		if (p->output_policy != IGNORE_OUTPUTS) { /* For better performance (see above) */
			if (save_previous_ptr_var != NULL) {
				restore_variable_array(p->input_variables, p->backup_memory_reserve,
						save_previous_ptr_var);
				save_previous_ptr_var = NULL;
			}

			int reserve_freeable = free_variable_backup_using_reserve(
							p->backup_memory_reserve);
			if (reserve_previous != NULL) {
					if (reserve_freeable == 0) {
						fatal_error("incoherent reserve free result\n");
					}
					free_reserve(p->backup_memory_reserve);
					p->backup_memory_reserve = reserve_previous;
			}

			if (p->nb_output_variables != 0) {
			    install_output_variable_backup(p->output_variables,output_var_backup);
			    free_output_variable_backup(output_var_backup);
			}
		}
		p->dic_variables = dic_variables_backup;
	} /* End of processing subgraphs */

	/**
	 * METAS
	 */
	struct opt_meta* meta_list = current_state->metas;
	if (meta_list != NULL) {
		/* We cache the control bytes of the pos2 token. The pos token has not interest,
		 * because it is 1) a space or 2) equal to the pos2 one. */
		if (token2 != -1)
			ctrl = p->token_control[token2];
		else
			ctrl = 0;
	}
	while (meta_list != NULL) {
		/* We process all the meta of the list */
		t1 = meta_list->transition;
		while (t1 != NULL) {
			/* We cache the output of the current tag, as well as values indicating if the
			 * current pos2 tokens matches the tag's morphological filter, if any. */
			output = p->tags[t1->tag_number]->output;
			/* If there is no morphological filter, we act as if there was a matching one, except
			 * if we are at the end of the token buffer. With this trick, the morphofilter test
			 * will avoid overpassing the end of the token buffer. */
			int morpho_filter_OK = (token2 != -1) ? 1 : 0;
#ifdef TRE_WCHAR
			filter_number = p->tags[t1->tag_number]->filter_number;
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
				}
				if (token != -1) {
					/* We don't update the position if # has matched because of the
					 * end of the buffer */
					update_last_tested_position(p, pos);
				}
				break;

			case META_SPACE:
				if (token != -1 && token == p->SPACE) {
					/* The space meta can match only if there is a space at the current position.
					 * Note that we don't compare token and p->SPACE since p->SPACE can be -1 if
					 * the text contains no space. */
					start = pos;
					end = pos + 1;
				}
				update_last_tested_position(p, pos);
				break;

			case META_EPSILON:
				/* We can always match the empty word */
				start = pos;
				end = pos;
				break;

			case META_TEXT_START:
				/* We can match if and only if we are at the beginning of the text */
				p->no_fail_fast=1;
				if (at_text_start(p,pos)) {
					start = pos;
					end = pos;
				}
				break;

			case META_TEXT_END:
				p->no_fail_fast=1;
				if (end_of_text || end_of_text2) {
					start = pos;
					end = pos;
				}
				break;

			case META_MOT:
				update_last_tested_position(p, pos2);
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
				update_last_tested_position(p, pos2);
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
							unichar* sequence = get_token_sequence(p,
									pos2 + p->current_origin,
									end_of_compound + p->current_origin);
							OK = string_match_filter(p->filters, sequence,
									filter_number, p->recyclable_wchart_buffer);
						}
#endif
						if (OK) {
							/* <DIC> can match two things: a sequence of tokens or a single token
							 * As we don't want to process lists of [start,end[ ranges, we
							 * directly handle here the case of a token sequence. */
							if (p->output_policy == MERGE_OUTPUTS && pos2!=pos) {
								push_input_char(p->stack, ' ',
										p->protect_dic_chars);
							}
							captured_chars=0;
							if (p->output_policy != IGNORE_OUTPUTS) {
								if (!deal_with_output(output,p,&captured_chars)) {
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
							locate(/*graph_depth,*/
									p->optimized_states[t1->state_number],
									end_of_compound + 1, matches,
									n_matches, ctx, p);
							p->stack->stack_pointer = stack_top;
							p->weight=old_weight1;
							if (p->nb_output_variables != 0) {
							    remove_chars_from_output_variables(p->output_variables,captured_chars);
							}
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
				update_last_tested_position(p, pos2);
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
				update_last_tested_position(p, pos2);
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
						unichar* sequence = get_token_sequence(p,
								pos2 + p->current_origin,
								end_of_compound + p->current_origin);
						OK = string_match_filter(p->filters, sequence,
								filter_number, p->recyclable_wchart_buffer);
					}
#endif
					if (OK) {
						/* <CDIC> can match two things: a sequence of tokens or a compound
						 * tag token like "{black-eyed,.A}". As we don't want to process lists
						 * of [start,end[ ranges, we directly handle here the case of a
						 * token sequence. */
						if (p->output_policy == MERGE_OUTPUTS && pos2!=pos) {
							push_input_char(p->stack, ' ', p->protect_dic_chars);
						}
						captured_chars=0;
						if (p->output_policy != IGNORE_OUTPUTS) {
							if (!deal_with_output(output,p,&captured_chars)) {
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
						locate(/*graph_depth,*/
								p->optimized_states[t1->state_number],
								end_of_compound + 1, matches,
								n_matches, ctx, p);
						p->weight=old_weight1;
						if (p->nb_output_variables != 0) {
						    remove_chars_from_output_variables(p->output_variables,captured_chars);
						}
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
				update_last_tested_position(p, pos2);
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
				update_last_tested_position(p, pos2);
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
				update_last_tested_position(p, pos2);
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
				update_last_tested_position(p, pos2);
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
				update_last_tested_position(p, pos2);
				if (token2 == p->STOP) {
					break;
				}
				{ /* This block avoids visibility problem about 'z' */
                    

					// local_is_not_a_digit_token return 1 if s is a digit sequence, 0 else
					if (!(pos2 + p->current_origin < p->buffer_size
							&& (local_is_not_a_digit_token(p->tokens->value[p->buffer[pos2
									+ p->current_origin]]) == 0) )) {
						break;
					}

					/* If we have found a contiguous digit sequence */

					int z = pos2+1;
					int pos_limit = p->buffer_size - p->current_origin; 

					int next_pos_add = 0;
					while (z  < pos_limit
							&& ((next_pos_add = local_is_not_a_digit_token(p->tokens->value[p->buffer[z
									+ p->current_origin]])) == 0)) {
						z++;
					}

					// If we have stopped because of the end of the buffer, next_pos_add = 0
					// If we have stopped because of a non matching token, next_pos_add = 1
					start = pos2;
					end = z;

#ifdef TRE_WCHAR
					if (filter_number == -1) {
						update_last_tested_position(p,z+next_pos_add);
						break;
                    }

					int OK = 1;
					unichar* sequence = get_token_sequence(p,
							start + p->current_origin,
							end-1 + p->current_origin);
					OK = string_match_filter(p->filters, sequence,
							filter_number, p->recyclable_wchart_buffer);
					if (!OK) {
						start=-1;
						break;
					}
#endif
					update_last_tested_position(p,z+next_pos_add);
				}
				break;

			case META_TOKEN:
				if (token2 == -1)
					break;
				update_last_tested_position(p, pos2);
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
				/* we don't know if enter_morphological_mode will modify variable
				 so we increment the dirty flag, without any associated decrement, to be sure
				 having no problem */
				inc_dirty(p->backup_memory_reserve);
				enter_morphological_mode(/*graph_depth,*/ t1->state_number, pos2,
						matches, n_matches, ctx, p);
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
				locate(/*graph_depth,*/ p->optimized_states[t1->state_number], pos2,
						matches, n_matches, ctx, p);
				/*if (p->output_policy!=IGNORE_OUTPUTS) {
				 install_variable_backup(p->variables,var_backup);
				 free_variable_backup(var_backup);
				 }*/
				p->weight=old_weight1;
				p->left_ctx_shift = current_shift;
				p->left_ctx_base = old_left_ctx_stack_base;
				p->stack->stack_pointer = stack_top;
				break;

			} /* End of the switch */

			if (start != -1) {
				/* If the transition has matched */
				if (p->output_policy == MERGE_OUTPUTS && start == pos2 && pos2!=pos) {
					push_input_char(p->stack, ' ', p->protect_dic_chars);
				}
				captured_chars=0;
				if (p->output_policy != IGNORE_OUTPUTS) {
					/* We process its output */
					if (!deal_with_output(output,p,&captured_chars)) {
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
				locate(/*graph_depth,*/ p->optimized_states[t1->state_number], end,
						matches, n_matches, ctx, p);
				/* Once we have finished, we restore the stack */
				p->weight=old_weight1;
				p->stack->stack_pointer = stack_top;
				if (p->nb_output_variables != 0) {
				  remove_chars_from_output_variables(p->output_variables,captured_chars);
                }
			}
			next: t1 = t1->next;
		}
		meta_list = meta_list->next;
	}

/**
 * OUTPUT VARIABLE STARTS
 */
struct opt_variable* output_variable_list = current_state->output_variable_starts;
Ustring* recycle_Ustring=NULL;
while (output_variable_list != NULL) {
	Ustring* old_value=p->output_variables->variables[output_variable_list->variable_number];
	if (recycle_Ustring==NULL) {
		recycle_Ustring=new_Ustring();
	}
	else {
		empty(recycle_Ustring);
	}
	p->output_variables->variables[output_variable_list->variable_number]=recycle_Ustring;
	set_output_variable_pending(p->output_variables,output_variable_list->variable_number);
	locate(/*graph_depth,*/
			p->optimized_states[output_variable_list->transition->state_number],
			pos, matches, n_matches, ctx, p);
	p->weight=old_weight1;
	unset_output_variable_pending(p->output_variables,output_variable_list->variable_number);
	//free_Ustring(p->output_variables->variables[output_variable_list->variable_number]);
	p->output_variables->variables[output_variable_list->variable_number]=old_value;
	p->stack->stack_pointer = stack_top;
	output_variable_list=output_variable_list->next;
}
if (recycle_Ustring!=NULL) {
	free_Ustring(recycle_Ustring);
	recycle_Ustring=NULL;
}
/**
 * OUTPUT VARIABLE ENDS
 */
output_variable_list = current_state->output_variable_ends;
while (output_variable_list != NULL) {
	unset_output_variable_pending(p->output_variables,output_variable_list->variable_number);
	locate(/*graph_depth,*/
			p->optimized_states[output_variable_list->transition->state_number],
			pos, matches, n_matches, ctx, p);
	p->weight=old_weight1;
	set_output_variable_pending(p->output_variables,output_variable_list->variable_number);
	p->stack->stack_pointer = stack_top;
	output_variable_list=output_variable_list->next;
}

	/**
	 * VARIABLE STARTS
	 */
	struct opt_variable* variable_list = current_state->input_variable_starts;
	/* We don't start variables after the end of the buffer */
	if (token2!=-1) while (variable_list != NULL) {
		inc_dirty(p->backup_memory_reserve);
		int old_in_token = get_variable_start(p->input_variables,
				variable_list->variable_number);
		int old_in_char = get_variable_start_in_chars(p->input_variables,
						variable_list->variable_number);
		set_variable_start(p->input_variables, variable_list->variable_number, pos2);
		set_variable_start_in_chars(p->input_variables, variable_list->variable_number, 0);
		locate(/*graph_depth,*/
				p->optimized_states[variable_list->transition->state_number],
				pos, matches, n_matches, ctx, p);
		p->weight=old_weight1;
		p->stack->stack_pointer = stack_top;
		if (ctx == NULL) {
			/* We do not restore previous value if we are inside a context, in order
			 * to allow extracting things from contexts (see the
			 * "the cat is white" example in Unitex manual). */
			set_variable_start(p->input_variables, variable_list->variable_number,
					old_in_token);
			set_variable_start_in_chars(p->input_variables, variable_list->variable_number,
								old_in_char);
			dec_dirty(p->backup_memory_reserve);
			// restore dirty
		}
		variable_list = variable_list->next;
	}

	/**
	 * VARIABLE ENDS
	 */
	variable_list = current_state->input_variable_ends;
	int end=(token!=-1)?pos:(p->buffer_size-p->current_origin);
	while (variable_list != NULL) {
		inc_dirty(p->backup_memory_reserve);
		int old_in_token =
				get_variable_end(p->input_variables, variable_list->variable_number);
		int old_in_char =
						get_variable_end_in_chars(p->input_variables, variable_list->variable_number);
		set_variable_end(p->input_variables, variable_list->variable_number, /*pos*/end);
		set_variable_end_in_chars(p->input_variables, variable_list->variable_number,-1);
		locate(/*graph_depth,*/
				p->optimized_states[variable_list->transition->state_number],
				pos, matches, n_matches, ctx, p);
		p->weight=old_weight1;
		p->stack->stack_pointer = stack_top;
		if (ctx == NULL) {
			/* We do not restore previous value if we are inside a context, in order
			 * to allow extracting things from contexts (see the
			 * "the cat is white" example in Unitex manual). */
			set_variable_end(p->input_variables, variable_list->variable_number, old_in_token);
			set_variable_end_in_chars(p->input_variables, variable_list->variable_number, old_in_char);
			dec_dirty(p->backup_memory_reserve);
		}
		variable_list = variable_list->next;
	}

	/**
	 * CONTEXTS
	 */
	struct opt_contexts* contexts = current_state->contexts;
	if (contexts != NULL) {
		Transition* t2;
		for (int n_ctxt = 0; n_ctxt < contexts->size_positive; n_ctxt = n_ctxt
				+ 2) {
			t2 = contexts->positive_mark[n_ctxt];
			/* We look for a positive context from the current position */
			struct list_context* c = new_list_context(0, ctx);
			locate(/*graph_depth,*/ p->optimized_states[t2->state_number], pos,
					NULL, 0, c, p);
			p->weight=old_weight1;
			/* Note that there is no match to free since matches cannot be built within a context */
			p->stack->stack_pointer = stack_top;
			p->stack->stack[stack_top+1]='\0';
			if (c->n) {
				/* If the context has matched, then we can explore all the paths
				 * that starts from the context end */
				Transition* states = contexts->positive_mark[n_ctxt + 1];
				unichar* backup=NULL;
				if (p->debug) {
					/* In debug mode, we copy the debug output
					 * obtained while exploring the context,
					 * but we remove the input part in order
					 * not to produce an invalid concordance
					 */
					backup=u_strdup(p->stack->stack);
					u_strcat(p->stack->stack,c->output+stack_top+1);
					int pos3=u_strlen(p->stack->stack);
					p->stack->stack_pointer=pos3-1;
				}
				while (states != NULL) {
					locate(/*graph_depth,*/
							p->optimized_states[states->state_number], pos,
							matches, n_matches, ctx, p);
					p->weight=old_weight1;
					p->stack->stack_pointer = stack_top;
					states = states->next;
				}
				if (p->debug) {
					u_strcpy(p->stack->stack,backup);
					free(backup);
				}
			}
			free_list_context(c);
		}
		for (int n_ctxt = 0; n_ctxt < contexts->size_negative; n_ctxt = n_ctxt
				+ 2) {
			t2 = contexts->negative_mark[n_ctxt];
			/* We look for a negative context from the current position */
			struct list_context* c = new_list_context(0, ctx);
			locate(/*graph_depth,*/ p->optimized_states[t2->state_number], pos,
					NULL, 0, c, p);
			p->weight=old_weight1;
			/* Note that there is no matches to free since matches cannot be built within a context */
			p->stack->stack_pointer = stack_top;
			if (!c->n) {
				/* If the context has not matched, then we can explore all the paths
				 * that starts from the context end */
				Transition* states = contexts->negative_mark[n_ctxt + 1];
				while (states != NULL) {
					locate(/*graph_depth,*/
							p->optimized_states[states->state_number], pos,
							matches, n_matches, ctx, p);
					p->weight=old_weight1;
					p->stack->stack_pointer = stack_top;
					states = states->next;
				}
			}
			/* We just want to free the cell we created */
			free_list_context(c);
		}
		if ((t2 = contexts->end_mark) != NULL) {
			/* If we have a closing context mark */
			if (ctx == NULL) {
				/* If there was no current opened context, it's an error */
				error(
						"ERROR: unexpected closing context mark in graph \"%S\"\n",
						p->fst2->graph_names[(p->graph_depth) + 1]);
				p->explore_depth -- ;
				return;
			}
			/* Otherwise, we just indicate that we have found a context closing mark,
			 * and we return */
			ctx->n = 1;
			if (ctx->output==NULL) {
				p->stack->stack[p->stack->stack_pointer+1]='\0';
				ctx->output=u_strdup(p->stack->stack);
			}
			p->explore_depth -- ;
			return;
		}
	}

	/**
	 * Now that we have finished with meta symbols, there can be no chance to advance
	 * in the grammar if we are at the end of the token buffer.
	 */
	if (token2 == -1) {
		p->explore_depth -- ;
		return;
	}

	/**
	 * COMPOUND WORD PATTERNS:
	 * here, we deal with patterns that can only match compound sequences
	 */
	struct opt_pattern* pattern_list = current_state->compound_patterns;
	while (pattern_list != NULL) {
		t1 = pattern_list->transition;
		while (t1 != NULL) {
#ifdef TRE_WCHAR
			filter_number = p->tags[t1->tag_number]->filter_number;
#endif
			output = p->tags[t1->tag_number]->output;
			end_of_compound = find_compound_word(pos2,
					pattern_list->pattern_number, p->DLC_tree, p);
			if (end_of_compound != -1 && !(pattern_list->negation)) {
				int OK = 1;
#ifdef TRE_WCHAR
				if (filter_number == -1)
					OK = 1;
				else {
					unichar* sequence = get_token_sequence(p,
							pos2 + p->current_origin,
							end_of_compound + p->current_origin);
					OK = string_match_filter(p->filters, sequence,
							filter_number, p->recyclable_wchart_buffer);
				}
#endif
				if (OK) {
					if (p->output_policy == MERGE_OUTPUTS && pos2 != pos) {
						push_input_char(p->stack, ' ', p->protect_dic_chars);
					}
					captured_chars=0;
					if (p->output_policy != IGNORE_OUTPUTS) {
						if (!deal_with_output(output,p,&captured_chars)) {
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
					locate(/*graph_depth,*/ p->optimized_states[t1->state_number],
							end_of_compound + 1, matches, n_matches,
							ctx, p);
					p->weight=old_weight1;
					p->stack->stack_pointer = stack_top;
					remove_chars_from_output_variables(p->output_variables,captured_chars);
				}
			}
			next4: t1 = t1->next;
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
		t1 = pattern_list->transition;
		while (t1 != NULL) {
#ifdef TRE_WCHAR
			filter_number = p->tags[t1->tag_number]->filter_number;
#endif
			output = p->tags[t1->tag_number]->output;
			/* We try to match a compound word */
			end_of_compound = find_compound_word(pos2,
					pattern_list->pattern_number, p->DLC_tree, p);
			if (end_of_compound != -1 && !(pattern_list->negation)) {
				int OK = 1;
#ifdef TRE_WCHAR
				if (filter_number == -1)
					OK = 1;
				else {
					unichar* sequence = get_token_sequence(p,
							pos2 + p->current_origin,
							end_of_compound + p->current_origin);
					OK = string_match_filter(p->filters, sequence,
							filter_number, p->recyclable_wchart_buffer);
				}
#endif
				if (OK) {
					if (p->output_policy == MERGE_OUTPUTS && pos2 != pos) {
						push_input_char(p->stack, ' ', p->protect_dic_chars);
					}
					captured_chars=0;
					if (p->output_policy != IGNORE_OUTPUTS) {
						if (!deal_with_output(output,p,&captured_chars)) {
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
					locate(/*graph_depth,*/ p->optimized_states[t1->state_number],
							end_of_compound + 1, matches, n_matches,
							ctx, p);
					p->weight=old_weight1;
					p->stack->stack_pointer = stack_top;
					remove_chars_from_output_variables(p->output_variables,captured_chars);
				}
				/* This useless empty block is just here to enable the declaration of the next6 label */
				next6: {
				}
			}
			/* And now, we look for simple words */
			int OK = 1;
			update_last_tested_position(p, pos2);
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
						captured_chars=0;
						if (p->output_policy != IGNORE_OUTPUTS) {
							if (!deal_with_output(output,p,&captured_chars)) {
								goto next2;
							}
						}
						if (p->output_policy == MERGE_OUTPUTS) {
							push_input_string(p->stack,
									p->tokens->value[token2],
									p->protect_dic_chars);
						}
						locate(/*graph_depth,*/
								p->optimized_states[t1->state_number], pos2 + 1,
								matches, n_matches, ctx, p);
						p->weight=old_weight1;
						p->stack->stack_pointer = stack_top;
						remove_chars_from_output_variables(p->output_variables,captured_chars);
					}
				} else {
					/* If the token matches no pattern, then it can match a pattern negation
					 * like <!V> */
					if (pattern_list->negation && (p->token_control[token2]
							& MOT_TOKEN_BIT_MASK)) {
						if (p->output_policy == MERGE_OUTPUTS && pos2 != pos) {
							push_input_char(p->stack, ' ', p->protect_dic_chars);
						}
						captured_chars=0;
						if (p->output_policy != IGNORE_OUTPUTS) {
							if (!deal_with_output(output,p,&captured_chars)) {
								goto next2;
							}
						}
						if (p->output_policy == MERGE_OUTPUTS) {
							push_input_string(p->stack,
									p->tokens->value[token2],
									p->protect_dic_chars);
						}
						locate(/*graph_depth,*/
								p->optimized_states[t1->state_number], pos2 + 1,
								matches, n_matches, ctx, p);
						p->weight=old_weight1;
						p->stack->stack_pointer = stack_top;
						remove_chars_from_output_variables(p->output_variables,captured_chars);
					}
				}
			}
			next2: t1 = t1->next;
		}
		pattern_list = pattern_list->next;
	}

	/**
	 * TOKENS
	 */
	if (current_state->number_of_tokens != 0) {
		update_last_tested_position(p, pos2);
		int n = binary_search(token2, current_state->tokens,
				current_state->number_of_tokens);
		if (n != -1) {
			t1 = current_state->token_transitions[n];
			while (t1 != NULL) {
#ifdef TRE_WCHAR
				filter_number = p->tags[t1->tag_number]->filter_number;
				if (filter_number == -1 || token_match_filter(
						p->filter_match_index, token2, filter_number))
#endif
				{
					output = p->tags[t1->tag_number]->output;
					if (p->output_policy == MERGE_OUTPUTS && pos2 != pos) {
						push_input_char(p->stack, ' ', p->protect_dic_chars);
					}
					captured_chars=0;
					if (p->output_policy != IGNORE_OUTPUTS) {
						if (!deal_with_output(output,p,&captured_chars)) {
							goto next3;
						}
					}
					if (p->output_policy == MERGE_OUTPUTS) {
						push_input_string(p->stack, p->tokens->value[token2],
								p->protect_dic_chars);
					}
					locate(/*graph_depth,*/ p->optimized_states[t1->state_number],
							pos2 + 1, matches, n_matches, ctx, p);
					p->weight=old_weight1;
					p->stack->stack_pointer = stack_top;
					remove_chars_from_output_variables(p->output_variables,captured_chars);
				}
				next3: t1 = t1->next;
			}
		}
	}
	p->explore_depth -- ;
}


/**
 * Looks for 'a' in the given array. Returns its position or -1 if not found.
 */
static int binary_search(int a, int* t, int n) {
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
static int find_longest_compound_word(int pos, struct DLC_tree_node* node,
		int pattern_number, struct locate_parameters* p) {
	if (node == NULL) {
		fatal_error("NULL node in find_longest_compound_word\n");
	}
	update_last_tested_position(p, pos);
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
static int find_compound_word(int pos, int pattern_number,
		struct DLC_tree_info* DLC_tree, struct locate_parameters* p) {
	return find_longest_compound_word(pos, DLC_tree->root, pattern_number, p);
}


/**
 * Returns a string corresponding to the tokens in the range [start;end].
 */
static unichar* get_token_sequence(struct locate_parameters*p,
		int start, int end) {
	unichar* recyclable_buffer = p->recyclable_unichar_buffer;
	unichar* recyclable_buffer_limit = recyclable_buffer + p->size_recyclable_unichar_buffer ;
	unichar* fill_recyclable_buffer = recyclable_buffer;
	const int* token_array = p->buffer;
	const struct string_hash* tokens = p->tokens;
	int i;

	for (i = start; i <= end; i++) {
		const unichar* current_string = tokens->value[token_array[i]];
		unichar c;
		do
		{
			c=*(current_string++);

			if (fill_recyclable_buffer == recyclable_buffer_limit)
            {
				// we have reached the end of the recyclable buffer
				// we enlarge (*2) the size of recyclable buffer and try again with the new buffer
				p->size_recyclable_unichar_buffer *= 2;
				free(p->recyclable_unichar_buffer);
				p->recyclable_unichar_buffer=(unichar*)malloc(sizeof(unichar) * (p->size_recyclable_unichar_buffer));
				if (p->recyclable_unichar_buffer==NULL) {
					fatal_alloc_error("get_token_sequence");
				}
				return get_token_sequence(p,start,end);
            }

			*(fill_recyclable_buffer++) = c;
		} while (c != '\0');
		// next writting in buffer must overwrite the end of string
		fill_recyclable_buffer--;
	}
	return recyclable_buffer;
}


/**
 * Filters the given match list to remove all matches that have a lesser weight.
 * Note that the given match list is supposed to be made of matches that
 * all have the same weight.
 */
void remove_matches_with_lesser_weights(int weight,struct match_list* *match_cache_first,
										struct match_list* *match_cache_last,
										Abstract_allocator prv_alloc) {
if (*match_cache_first==NULL || (*match_cache_first)->weight>=weight) return;
/* If the new weight is bigger, we must free the whole match list */
free_match_list(*match_cache_first,prv_alloc);
*match_cache_first=NULL;
*match_cache_last=NULL;
}


/**
 * Stores the given match in a list. All matches will be processed later.
 */
static void add_match(int end, unichar* output, struct locate_parameters* p, Abstract_allocator prv_alloc) {
	if (end > p->last_matched_position) {
		p->last_matched_position = end;
	}
	int start = p->current_origin + p->left_ctx_shift;
	unichar* z=output;
	if (p->debug && p->left_ctx_base!=0) {
		/* In debug mode, we always use the whole output. It will be
		 * the match writing operation into the concordance index that
		 * will rebuild the actual output parsing the full debug one */
		z=p->stack->stack;
	}
	remove_matches_with_lesser_weights(p->weight,&(p->match_cache_first),&(p->match_cache_last),prv_alloc);
	if (p->match_cache_first!=NULL && p->match_cache_first->weight>p->weight) {
		/* If we have to ignore this match because its weight is lesser
		 * than the weight of the existing matches */
		return;
	}
	struct match_list* m = new_match(start, end, z, p->weight, NULL, prv_alloc);
	if (p->match_cache_first == NULL) {
		p->match_cache_first = p->match_cache_last = m;
		return;
	}
	p->match_cache_last->next = m;
	p->match_cache_last = m;
}


static void insert_in_all_matches_mode(struct match_list* m, struct locate_parameters* p, Abstract_allocator prv_alloc) {
int start = m->m.start_pos_in_token;
int end = m->m.end_pos_in_token;
unichar* output = m->output;
struct match_list* *L=&(p->match_list);
while (*L != NULL) {
	if ((*L)->m.start_pos_in_token == start
		&& (*L)->m.end_pos_in_token == end
		&& (p->ambiguous_output_policy != ALLOW_AMBIGUOUS_OUTPUTS || !u_strcmp((*L)->output, output))) {
		/* The match is already there, nothing to do */
		if (p->ambiguous_output_policy!=ALLOW_AMBIGUOUS_OUTPUTS && u_strcmp((*L)->output, output)) {
			/* If ambiguous outputs are forbidden, we emit an error message */
			error("Unexpected ambiguous outputs:\n<%S>\n<%S>\n",(*L)->output,output);
		}
		return;
	}
	if (start<(*L)->m.start_pos_in_token ||
			(start==(*L)->m.start_pos_in_token && end<(*L)->m.end_pos_in_token)) {
		/* We have to insert at the current position */
		(*L)=new_match(start, end, output, -1, *L, prv_alloc);
		return;
	}
	L = &((*L)->next);
}
/* End of list? We add the match */
(*L) = new_match(start, end, output, -1, NULL, prv_alloc);
}

static struct match_list* filter_on_weight_matches_with_same_start(struct match_list* l,int start,
			int weight,int *dont_add_new_match,Abstract_allocator prv_alloc) {
if (l==NULL) return NULL;
if (l->m.start_pos_in_token==start) {
	/* We have a match with the same start */
	if (l->weight>weight) {
		/* There is at least one match with a bigger weight, we must remove
		 * the new match */
		*dont_add_new_match=1;
		return l;
	}
	if (l->weight<weight) {
		/* The current match must be discarded */
		struct match_list* tmp=l->next;
		free_match_list_element(l,prv_alloc);
		return filter_on_weight_matches_with_same_start(tmp,start,weight,dont_add_new_match,prv_alloc);
	}
	/* If both matches have the same weight, we do nothing but exploring the
	 * remaining list */
}
l->next=filter_on_weight_matches_with_same_start(l->next,start,weight,dont_add_new_match,prv_alloc);
return l;
}


/**
 * Adds a match to the global list of matches. The function takes into
 * account the match policy. For instance, we don't take [2;3] into account
 * if we are in longest match mode and if we already have [2;5].
 *
 * # Changed to allow different outputs in merge/replace
 * mode when the grammar is an ambiguous transducer (S.N.) */
static void real_add_match(struct match_list* m, struct locate_parameters* p, Abstract_allocator prv_alloc) {
	int start = m->m.start_pos_in_token;
	int end = m->m.end_pos_in_token;
	unichar* output = m->output;
	int weight=m->weight;
	int dont_add_match;
	/* When we a conflict between two weighted paths, one with a left context and
	 * not the other, the two matches are processed at different times in this function,
	 * so their weights have not been compared yet. To address this issue, we always
	 * filter the existing match list when adding a new match according the following
	 * rule: if there is a match starting at the same position than the new one,
	 * then we filter them according to their weights */
	dont_add_match=0;
	p->match_list=filter_on_weight_matches_with_same_start(p->match_list,start,
			weight,&dont_add_match,prv_alloc);
	if (dont_add_match) {
		/* There is already a match better than the new one, so we are done */
		return;
	}
	if (p->match_list == NULL) {
		/* If the match list was empty, we always can put the match in the list */
		p->match_list = new_match(start, end, output, weight, NULL, prv_alloc);
		return;
	}
	switch (p->match_policy) {
	case LONGEST_MATCHES:
		/* We put the new match at the beginning of the list, but before, we
		 * test if the match we want to add may not be discarded because of
		 * a longest match that would already be in the list. By the way,
		 * we eliminate matches that are shorter than this one, if any. */
		dont_add_match = 0;
		p->match_list = eliminate_shorter_matches(p->match_list, start, end,
				output, &dont_add_match, p, prv_alloc);
		if (!dont_add_match) {
			p->match_list = new_match(start, end, output, weight, p->match_list, prv_alloc);
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
		dont_add_match = 0;
		p->match_list = eliminate_longer_matches(p->match_list, start, end,
				output, &dont_add_match, p, prv_alloc);
		if (!dont_add_match) {
			p->match_list = new_match(start, end, output, weight, p->match_list, prv_alloc);
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
static struct match_list* eliminate_longer_matches(struct match_list *ptr, int start,
		int end, unichar* output, int *dont_add_match,
		struct locate_parameters* p, Abstract_allocator prv_alloc) {
	struct match_list *l;
	if (ptr == NULL)
		return NULL;
	if (ptr->m.start_pos_in_token==start
		&& ptr->m.end_pos_in_token==end
		&& u_strcmp(ptr->output, output)) {
		if (p->ambiguous_output_policy == ALLOW_AMBIGUOUS_OUTPUTS) {
			/* In the case of ambiguous transductions producing different outputs,
			 * we accept matches with same range */
			ptr->next = eliminate_longer_matches(ptr->next, start, end, output,
					dont_add_match, p, prv_alloc);
			return ptr;
		} else {
			/* If we don't allow ambiguous outputs, we have to print an error message */
			error("Unexpected ambiguous outputs:\n<%S>\n<%S>\n",ptr->output,output);
		}
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
			free_cb(ptr->output,prv_alloc);
		ptr->output = u_strdup(output,prv_alloc);
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
 * Does the same as eliminate_longer_matches, but with shorter matches.
 */
static struct match_list* eliminate_shorter_matches(struct match_list *ptr, int start,
		int end, unichar* output, int *dont_add_match,
		struct locate_parameters* p, Abstract_allocator prv_alloc) {
	struct match_list *l;
	if (ptr == NULL)
		return NULL;
	if (ptr->m.start_pos_in_token==start
		&& ptr->m.end_pos_in_token==end
		&& u_strcmp(ptr->output, output)) {
		if (p->ambiguous_output_policy == ALLOW_AMBIGUOUS_OUTPUTS) {
			/* In the case of ambiguous transductions producing different outputs,
			 * we accept matches with same range */
			ptr->next = eliminate_shorter_matches(ptr->next, start, end, output,
					dont_add_match, p, prv_alloc);
			return ptr;
		} else {
			/* If we don't allow ambiguous outputs, we have to print an error message */
			error("Unexpected ambiguous outputs:\n<%S>\n<%S>\n",ptr->output,output);
		}
	}
	if (start <= ptr->m.start_pos_in_token && end >= ptr->m.end_pos_in_token) {
		/* If the new match is longer (or of equal length) than the current one
		 * in the list, we replace the match in the list by the new one */
		if (*dont_add_match) {
			/* If we have already noticed that the match mustn't be added
			 * to the list, we delete the current list element */
			l = ptr->next;
			free_match_list_element(ptr, prv_alloc);
			return eliminate_shorter_matches(l, start, end, output,
					dont_add_match, p, prv_alloc);
		}
		/* If the new match is longer than the current one in the list, then we
		 * update the current one with the value of the new match. */
		ptr->m.start_pos_in_token = start;
		ptr->m.end_pos_in_token = end;
		if (ptr->output != NULL)
			free_cb(ptr->output,prv_alloc);
		ptr->output = u_strdup(output,prv_alloc);
		/* We note that the match does not need anymore to be added */
		(*dont_add_match) = 1;
		ptr->next = eliminate_shorter_matches(ptr->next, start, end, output,
				dont_add_match, p, prv_alloc);
		return ptr;
	}
	if (start >= ptr->m.start_pos_in_token && end <= ptr->m.end_pos_in_token) {
		/* The new match is shorter than the one in list => we
		 * skip the new match */
		(*dont_add_match) = 1;
		return ptr;
	}
	/* If we have disjunct ranges or overlapping ranges without inclusion,
	 * we examine recursively the rest of the list */
	ptr->next = eliminate_shorter_matches(ptr->next, start, end, output,
			dont_add_match, p, prv_alloc);
	return ptr;
}


/**
 * Writes the matches to the file concord.ind. The matches are in
 * left-most longest order. 'current_position' represents the current
 * position in the text. It is used to determine when we can save matches:
 * when we are at position 246, matches that end before 246 cannot be
 * modified anymore by the shortest or longest match heuristic, so we can
 * save them.
 *
 * wrong results for all matches when modifying text ??
 * <E>/[[ <MIN>* <PRE> <MIN>* <E>/]]
 * left-most stehen am Anfang der Liste
 */
static struct match_list* save_matches(struct match_list* l, int current_position,
		U_FILE* f, struct locate_parameters* p, Abstract_allocator prv_alloc) {
struct match_list* ptr;

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
			if (!p->debug) {
				/* Normal mode */
				u_fprintf(f, " %S", l->output);
			} else {
				/* In debug mode, we save the normal (non debug) mode output,
				 * before the debug one */
				u_fprintf(f, " ");
				save_real_output_from_debug(f,p->real_output_policy,l->output);
				u_fprintf(f,"%S",l->output);
			}
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
		if (v->variables[i].start_in_tokens != UNDEF_VAR_BOUND) {
			v->variables[i].start_in_tokens += shift;
		}
		if (v->variables[i].end_in_tokens != UNDEF_VAR_BOUND) {
			v->variables[i].end_in_tokens += shift;
		}
	}
}

