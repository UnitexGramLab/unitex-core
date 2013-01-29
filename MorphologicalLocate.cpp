/*
 * Unitex
 *
 * Copyright (C) 2001-2013 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "Unicode.h"
#include "Text_tokens.h"
#include "TransductionStack.h"
#include "Text_parsing.h"
#include "Error.h"
#include "BitArray.h"
#include "Transitions.h"
#include "Alphabet.h"
#include "TransductionStack.h"
#include "DicVariables.h"
#include "ParsingInfo.h"
#include "UserCancelling.h"
#include "CompressedDic.h"
#include "Contexts.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/* see http://en.wikipedia.org/wiki/Variable_Length_Array . MSVC did not support it 
 see http://msdn.microsoft.com/en-us/library/zb1574zs(VS.80).aspx */
#if defined(_MSC_VER) && (!(defined(NO_C99_VARIABLE_LENGTH_ARRAY)))
#define NO_C99_VARIABLE_LENGTH_ARRAY 1
#endif

static void morphological_locate(/*int, */int, int, int, /*int, */struct parsing_info**, int,
		struct list_context*, struct locate_parameters*,
		unichar*, int, unichar*);
int input_is_token(Fst2Tag tag);
static void explore_dic_in_morpho_mode(struct locate_parameters* p, int pos,
		int pos_in_token, struct parsing_info* *matches,
		struct pattern* pattern, int save_dela_entry, unichar* jamo,
		int pos_in_jamo);
int is_entry_compatible_with_pattern(const struct dela_entry* entry,
		const struct pattern* pattern);

/**
 * Stores in 'content' the string corresponding to the given range.
 */
static void get_content(unichar* content, struct locate_parameters* p, int pos,
		int pos_in_token, int pos2, int pos_in_token2) {
	int i = 0;
	unichar* current_token =
			p->tokens->value[p->buffer[pos + p->current_origin]];
	while (!(pos == pos2 && pos_in_token == pos_in_token2)) {
		content[i++] = current_token[pos_in_token++];
		if (current_token[pos_in_token] == '\0' && !(pos == pos2
				&& pos_in_token == pos_in_token2)) {
			pos++;
			pos_in_token = 0;
			int token_number = p->buffer[pos + p->current_origin];
			if (token_number == -1 || token_number == p->STOP) {
				fatal_error("Unexpected end of array or {STOP} tag in get_content\n");
			}
			current_token = p->tokens->value[token_number];
		}
	}
	content[i] = '\0';
}

/**
 * Returns 1 if the given string is of the form $YYY$ where YYY is a valid
 * variable name; 0 otherwise.
 */
static int is_morpho_variable_output(unichar* s, unichar* var_name) {
	if (s == NULL || s[0] != '$')
		return 0;
	int i, j = 0;
	for (i = 1; s[i] != '$' && s[i] != '\0'; i++) {
		if (!is_variable_char(s[i]))
			return 0;
		var_name[j++] = s[i];
	}
	var_name[j] = '\0';
	return (i != 1 && s[i] == '$' && s[i + 1] == '\0');
}

/**
 * Tries to match all the given tag token against the given jamo sequence. Return values:
 * 0=failed
 * 1=the tag matches the whole jamo sequence (i.e. we will have to go on the next text token
 * 2=the tag matches a part of the jamo sequence
 */
static int get_jamo_longest_prefix(unichar* jamo, int *new_pos_in_jamo,
		int *new_pos_in_token, unichar* tag_token, struct locate_parameters* p,
		unichar* token) {
	unichar tmp[128];
	if (tag_token[0] == KR_SYLLABLE_BOUND && tag_token[1] == '\0') {
		u_strcpy(tmp, tag_token);
	} else {
		Hanguls_to_Jamos(tag_token, tmp, p->korean, 0);
		/*error("conversion depuis <%S> vers <%S><",tag_token,tmp);
		for (int i=0;tmp[i]!='\0';i++) {
			error("%X ",tmp[i]);
		}
		error(">\n");*/
	}
	int i = 0;
	if (token == NULL) {
		token = tag_token;
	}
	//set_debug(0 && token[0]==0xB2A5);
	//error("on compare text=_%S/%S_ et tag=_%S/%S\n",token,jamo/*+(*new_pos_in_jamo)*/,tag_token,tmp);
	/*error("on compare text=<%S><",token);
	for (int i=(*new_pos_in_jamo);jamo[i]!='\0';i++) {
		error("(%C) ",jamo[i]);
	}
	error("> et tag=<%S><",tag_token);
	for (int i=0;tmp[i]!='\0';i++) {
		error("(%C) ",tmp[i]);
	}
	error(">\n");*/
	while (tmp[i] != '\0' && jamo[(*new_pos_in_jamo)] != '\0') {
#if 2
		/* We ignore syllable bounds in both tfst and fst2 tags */
		if (tmp[i] == KR_SYLLABLE_BOUND && jamo[(*new_pos_in_jamo)] != KR_SYLLABLE_BOUND) {
			i++;
			//debug("ignoring . in tag: %S\n",tmp+i);
			continue;
		}
#endif
		if (jamo[(*new_pos_in_jamo)] == KR_SYLLABLE_BOUND) {
			if (tmp[i] != KR_SYLLABLE_BOUND) return 0;
			i++;
			(*new_pos_in_jamo)++;
			(*new_pos_in_token)++;
			//debug("ignoring . in text: %S\n",jamo+((*new_pos_in_jamo)));
			continue;
		}
		if (tmp[i] != jamo[(*new_pos_in_jamo)]) {
			/* If a character doesn't match */
			//debug("match failed between text=%S and fst2=%S\n",jamo,tmp);
			return 0;
		}
		i++;
		(*new_pos_in_jamo)++;
		//debug("moving in tag: %S\n",tmp+i);
		//debug("moving in text: %S\n",jamo+((*new_pos_in_jamo)));
	}
	if (tmp[i] == '\0' && jamo[(*new_pos_in_jamo)] == '\0') {
		/* If we are at both ends of strings, it's a full match */
		//debug("XX full match between text=%S and fst2=%S\n",jamo,tmp);
		return 1;
	}
	if (tmp[i] == '\0') {
		/* If the tag has not consumed all the jamo sequence, it's a partial match */
		//debug("XX partial match between text=%S and fst2=%S\n",jamo,tmp);
		return 2;
	}
	/* If we are at the end of the jamo sequence, but not at the end of the tag, it's a failure */
	//debug("match failed #2 between text=%S and fst2=%S\n",jamo,tmp);
	//set_debug(0);
	return 0;
}

static void update_last_position(struct locate_parameters* p, int pos) {
	if (pos > p->last_tested_position) {
		p->last_tested_position = pos;
	}
}

/**
 * This is the core function of the morphological mode.
 */
static void morphological_locate(/*int graph_depth, */ /* 0 means that we are in the top level graph */
int current_state_index, /* current state in the grammar */
int pos_in_tokens, /* position in the token buffer, relative to the current origin */
int pos_in_chars, /* position in the token in characters */
//int depth, /* number of nested calls to 'locate' */
struct parsing_info** matches, /* current match list. Irrelevant if graph_depth==0 */
int n_matches, /* number of sequences that have matched. It may be different from
 * the length of the 'matches' list if a given sequence can be
 * matched in several ways. It is used to detect combinatory
 * explosions due to bad written grammars. */
struct list_context* ctx, /* information about the current context, if any */
struct locate_parameters* p, /* miscellaneous parameters needed by the function */
unichar* jamo, int pos_in_jamo,
unichar* content_buffer /* reusable unichar 4096 buffer for content */
//,variable_backup_memory_reserve* backup_reserve
) {

	int old_weight=p->weight;
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
					fatal_alloc_error("morphological_locate");
				}
				lti->size_struct_locate_trace_info = (int)sizeof(locate_trace_info);
				lti->is_on_morphlogical = 1;

				lti->pos_in_tokens=pos_in_tokens;

				lti->current_state=NULL;

				lti->current_state_index=current_state_index;
				lti->pos_in_chars=pos_in_chars;

				lti->matches=matches;
				lti->n_matches=n_matches;
				lti->ctx=ctx;
				lti->p=p;

				lti->step_number=p->counting_step.count_call-p->counting_step_count_cancel_trying_real_in_debug_or_trace;

				lti->jamo=jamo;
				lti->pos_in_jamo=pos_in_jamo;

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


	OptimizedFst2State current_state = p->optimized_states[current_state_index];
	Fst2State current_state_old = p->fst2->states[current_state_index];
	int token;
	Transition* t;
	int stack_top = p->stack->stack_pointer;
	int captured_chars;
	/* The following static variable holds the number of matches at
	 * one position in text. */

	p->explore_depth ++ ;
    /*
	if (depth == 0) {
		// We reset if this is first call to 'locate' from a given position in the text 
		p->p_token_error_ctx->n_matches_at_token_pos__morphological_locate = 0;
	}*/

	if (p->explore_depth > STACK_MAX) {
		/* If there are too much recursive calls */
		error_at_token_pos("\nMaximal stack size reached!\n"
			"(There may be longer matches not recognized!)", p->current_origin,
				pos_in_tokens, p);
		p->explore_depth -- ;
		return;
	}
	if ((p->token_error_ctx.n_matches_at_token_pos__morphological_locate)
			> MAX_MATCHES_AT_TOKEN_POS) {
		/* If there are too much matches from the current origin in the text */
		error_at_token_pos(
				"\nToo many (ambiguous) matches starting from one position in text!",
				p->current_origin, pos_in_tokens, p);
		p->explore_depth -- ;
		return;
	}
	if (current_state->control & 1) {
		/* If we are in a final state... */
		/* In we are in the top level graph, it's an error, since we are not supposed
		 * to reach the end of the graph (we should find a $>) */
		if (p->graph_depth == 0) {
			error("Error in graph %S:\n",p->fst2->graph_names[get_graph_index(p->fst2,current_state_index)]);
			fatal_error("Unexpected end of graph in morphological mode!\n");
		} else {
			/* If we are in a subgraph */
			if (n_matches == MAX_MATCHES_PER_SUBGRAPH) {
				/* If there are too much matches, we suspect an error in the grammar
				 * like an infinite recursion */
				error_at_token_pos(
						"\nMaximal number of matches per subgraph reached!",
						p->current_origin, pos_in_tokens, p);
				p->explore_depth -- ;
				return;
			} else {
				/* If everything is fine, we add this match to the match list of the
				 * current graph level */
				n_matches++;
				p->stack->stack[stack_top + 1] = '\0';
				if (p->ambiguous_output_policy == ALLOW_AMBIGUOUS_OUTPUTS) {
					(*matches) = insert_if_different(pos_in_tokens, pos_in_chars, -1,
							(*matches), p->stack->stack_pointer,
							&(p->stack->stack[p->stack_base + 1]),
							p->input_variables, p->output_variables, p->dic_variables, -1, -1, jamo,
							pos_in_jamo, NULL, p->weight,p->prv_alloc_recycle,p->prv_alloc);
				} else {
					(*matches) = insert_if_absent(pos_in_tokens, pos_in_chars, -1,
							(*matches), p->stack->stack_pointer,
							&(p->stack->stack[p->stack_base + 1]),
							p->input_variables, p->output_variables, p->dic_variables, -1, -1, jamo,
							pos_in_jamo, NULL, p->weight,p->prv_alloc_recycle,p->prv_alloc);
				}
			}
		}
	}

	/* If we have reached the end of the token buffer, we indicate it by setting
	 * the current tokens to -1 */
	if (pos_in_tokens + p->current_origin >= p->buffer_size) {
		token = -1;
	} else {
		token = p->buffer[pos_in_tokens + p->current_origin];
	}
	unichar* current_token;
	int current_token_length = 0;
	if (token == -1 || token == p->STOP) {
		current_token = NULL;
		jamo = NULL;
	} else {
		current_token = p->tokens->value[p->buffer[p->current_origin + pos_in_tokens]];
		current_token_length = u_strlen(current_token);
	}

	/**
	 * SUBGRAPHS
	 */
	struct opt_graph_call* graph_call_list = current_state->graph_calls;
	if (graph_call_list != NULL) {
		/* If there are subgraphs, we process them */
		int old_StackBase = p->stack_base;
		int* save_previous_ptr_var = NULL;
		int* var_backup = NULL;
		//int create_new_reserve_done = 0;
		variable_backup_memory_reserve* reserve_previous = NULL;

		/* We save all kind of variables */
		struct dic_variable* dic_variables_backup = NULL;
		if (p->dic_variables != NULL) {
			dic_variables_backup = clone_dic_variable_list(p->dic_variables);
		}
		/* We do this to be sure there will be no complicated case leading to
		 * memory leaks */
		clear_dic_variable_list(&(p->dic_variables));

		unichar* output_variable_backup=NULL;
		if (p->output_policy!=IGNORE_OUTPUTS) {
			if (is_enough_memory_in_reserve_for_transduction_variable_set(p->input_variables,
					p->backup_memory_reserve) == 0) {
				reserve_previous = p->backup_memory_reserve;
				p->backup_memory_reserve = create_variable_backup_memory_reserve(
						p->input_variables,0);
				//create_new_reserve_done = 1;
			}
			if (p->graph_depth == 0) {
			  var_backup = create_variable_backup_using_reserve(p->input_variables,
					p->backup_memory_reserve);
			}
			if (p->nb_output_variables != 0) {
				output_variable_backup=create_output_variable_backup(p->output_variables);
			}
		}
		do {
			/* For each graph call, we look all the reachable states */
			t = graph_call_list->transition;
			if (p->output_policy!=IGNORE_OUTPUTS) {
				if (p->nb_output_variables != 0) {
				    install_output_variable_backup(p->output_variables,output_variable_backup);
				}
			}
			while (t != NULL) {
				struct parsing_info* L = NULL;
				p->stack_base = p->stack->stack_pointer;

				p->graph_depth ++ ;

				p->dic_variables = NULL;
				if (dic_variables_backup != NULL) {
					p->dic_variables = clone_dic_variable_list(dic_variables_backup);
				}

				p->weight=old_weight;
				morphological_locate(/*graph_depth + 1,*/ /* Exploration of the subgraph */
						p->fst2->initial_states[graph_call_list->graph_number], pos_in_tokens,
						pos_in_chars, &L, 0, NULL, p,
						jamo, pos_in_jamo, content_buffer);
				p->graph_depth -- ;

				clear_dic_variable_list(&(p->dic_variables));

				p->stack_base = old_StackBase;
				if (L != NULL) {
					struct parsing_info* L_first = L;
					/* If there is at least one match, we process the match list */
					do {
						/* We restore the settings that we had at the end of the subgraph exploration */
						if (p->output_policy != IGNORE_OUTPUTS) {
							u_strcpy(&(p->stack->stack[stack_top + 1]),
									L->stack);
							p->stack->stack_pointer = L->stack_pointer;
							p->dic_variables = clone_dic_variable_list(L->dic_variable_backup);
							if (p->nb_output_variables != 0) {
								install_output_variable_backup(p->output_variables,L->output_variable_backup);
							}
							if (save_previous_ptr_var == NULL && (var_backup != NULL)) {
								save_previous_ptr_var
										= install_variable_backup_preserving(
												p->input_variables, p->backup_memory_reserve,
												L->input_variable_backup);
							} else {
								install_variable_backup(p->input_variables,
										L->input_variable_backup);
							}
						}
						/* And we continue the exploration */
						/* We reset the weight since its a value that is graph specific */
						p->weight=old_weight;
						morphological_locate(/*graph_depth,*/ t->state_number,
								L->pos_in_tokens, L->pos_in_chars,
								matches, n_matches, ctx, p,
								L->jamo, L->pos_in_jamo, content_buffer);
						clear_dic_variable_list(&(p->dic_variables));
						p->stack->stack_pointer = stack_top;
						L = L->next;
					} while (L != NULL);
					/* We free all subgraph matches */
					free_parsing_info(L_first, p->prv_alloc_recycle, p->prv_alloc);
				}
				t = t->next;
			} /* end of while (t!=NULL) */
		} while ((graph_call_list = graph_call_list->next) != NULL);
		/* Finally, we have to restore the stack and other backup stuff */
		p->weight=old_weight;
		p->stack->stack_pointer = stack_top;
		p->stack_base = old_StackBase; /* May be changed by recursive subgraph calls */
		if (p->nb_output_variables != 0) {
			install_output_variable_backup(p->output_variables,output_variable_backup);
			free_output_variable_backup(output_variable_backup);
		}
		/* We restore the original dic variables */
		p->dic_variables=dic_variables_backup;
		if (save_previous_ptr_var != NULL) { 
			restore_variable_array(p->input_variables, p->backup_memory_reserve,
			                          save_previous_ptr_var);
			save_previous_ptr_var = NULL;
		}

        
		if (var_backup!=NULL) {
				int reserve_freeable = free_variable_backup_using_reserve(
						p->backup_memory_reserve);

				if (reserve_previous != NULL) {
					if (reserve_freeable == 0) {
							fatal_error("incoherent reserve free result\n");
					}
					free_reserve(p->backup_memory_reserve);
					p->backup_memory_reserve = reserve_previous;
				}
			}
	} /* End of processing subgraphs */

	/* This variable will be used to store the results provided by <DIC>. It
	 * is useful to cache the exploration of the morphological dictionaries,
	 * so that there will only be one even if there are several patterns requiring it,
	 * like <DIC>, <A>, <N:s>, etc */
	int DIC_cached=0;
	struct parsing_info* DIC_consultation = NULL;


	/**
	 * METAS
	 */
	struct opt_meta* meta_list = current_state->metas;
	while (meta_list != NULL) {
		/* We process all the meta of the list */
		t = meta_list->transition;
		int match_one_letter;
		while (t != NULL) {
			match_one_letter = 0;
			switch (meta_list->meta) {

			case META_EPSILON:
			case META_SHARP: /* # is no longer considered a problem in morphological mode, because
			                  * it may have been inserted by Grf2Fst2 */
				/* We could have an output associated to an epsilon or a #, so we handle this case */
				captured_chars=0;
				if (p->output_policy != IGNORE_OUTPUTS) {
					if (!deal_with_output(p->tags[t->tag_number]->output,p,&captured_chars)) {
						break;
					}
				}
				morphological_locate(/*graph_depth,*/
						t->state_number, pos_in_tokens, pos_in_chars,
						matches, n_matches, ctx, p,
						jamo, pos_in_jamo, content_buffer);
				p->weight=old_weight;
				p->stack->stack_pointer = stack_top;
				remove_chars_from_output_variables(p->output_variables,captured_chars);
				break;

			case META_SPACE:
				if (token == -1)
					break;
				update_last_position(p, pos_in_tokens);
				if (token == p->STOP)
					break;
				if (current_token[pos_in_chars] == ' ') {
					match_one_letter = 1;
				}
				break;

			case META_MOT: /* In morphological mode, this tag matches a letter, as defined in
			 the alphabet file */
				if (token == -1)
					break;
				update_last_position(p, pos_in_tokens);
				if (token == p->STOP)
					break;
				if (XOR(is_letter(current_token[pos_in_chars], p->alphabet),
						meta_list->negation)) {
					match_one_letter = 1;
				}
				break;

			case META_DIC: {
				if (token == -1)
					break;
				update_last_position(p, pos_in_tokens);
				if (token == p->STOP)
					break;
				struct parsing_info* L2 = NULL;
				int len_var_name = 0;
				if (p->tags[t->tag_number]->output != NULL) {
					len_var_name = 31;//u_strlen(p->tags[t->tag_number]->output);
				}
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
				unichar *var_name;
				var_name=(unichar*)malloc(sizeof(unichar)*(len_var_name+1));
				if (var_name==NULL) {
					fatal_alloc_error("morphological_locate");
				}
#else
				unichar var_name[len_var_name + 1];
#endif
				int save_dic_entry = (p->output_policy != IGNORE_OUTPUTS
						&& !capture_mode(p->output_variables)
						&& is_morpho_variable_output(
								p->tags[t->tag_number]->output, var_name));
				/*explore_dic_in_morpho_mode(p, pos_in_tokens, pos_in_chars, &L2, NULL,
						save_dic_entry, jamo, pos_in_jamo);*/
				/* In order to make the cache system work, we always have to save dic entries */
				if (!DIC_cached) {
					DIC_cached=1;
					explore_dic_in_morpho_mode(p, pos_in_tokens, pos_in_chars, &DIC_consultation, NULL,
											1, jamo, pos_in_jamo);
				}
				L2=DIC_consultation;
				unichar *content1 = content_buffer;
				if (L2 != NULL) {
					/* If there is at least one match, we process the match list */
					do {
						get_content(content1, p, pos_in_tokens, pos_in_chars,
								L2->pos_in_tokens, L2->pos_in_chars);
#ifdef REGEX_FACADE_ENGINE
						int filter_number =
								p->tags[t->tag_number]->filter_number;
						int morpho_filter_OK = (filter_number == -1
								|| string_match_filter(p->filters, content1,
										filter_number,
										p->recyclable_wchart_buffer));
						if (!morpho_filter_OK) {
							p->stack->stack_pointer = stack_top;
							L2 = L2->next;
							continue;
						}
#endif

						/* WARNING: we don't process the tag's output as usual if it
						 *          is a variable declaration like $abc$. Note that it could
						 *          make a difference if a variable with the same
						 *          name was declared before entering the morphological mode */
						captured_chars=0;
						if (!save_dic_entry && p->output_policy	!= IGNORE_OUTPUTS) {
							if (!deal_with_output(p->tags[t->tag_number]->output,p,&captured_chars)) {
								break;
							}
						}
						if (p->output_policy == MERGE_OUTPUTS) {
							push_input_string(p->stack, content1,
									p->protect_dic_chars);
						}
						unichar* reached_token =
								p->tokens->value[p->buffer[p->current_origin
										+ L2->pos_in_tokens]];
						int new_pos, new_pos_in_token;
						if (reached_token[L2->pos_in_chars] == '\0') {
							/* If we are at the end of the last token matched by the <DIC> tag */
							new_pos = L2->pos_in_tokens + 1;
							new_pos_in_token = 0;
						} else {
							/* If not */
							new_pos = L2->pos_in_tokens;
							new_pos_in_token = L2->pos_in_chars;
						}
						/* We continue the exploration */
						struct dela_entry* old_value = NULL;
						if (save_dic_entry) {
							old_value = clone_dela_entry(get_dic_variable(
									var_name, p->dic_variables));
							set_dic_variable(var_name, L2->dic_entry,
									&(p->dic_variables),1);
						}
						morphological_locate(/*graph_depth,*/
								t->state_number, new_pos,
								new_pos_in_token, matches,
								n_matches, ctx, p, L2->jamo,
								L2->pos_in_jamo, content_buffer);
						p->weight=old_weight;
						if (save_dic_entry) {
							set_dic_variable(var_name, old_value,
									&(p->dic_variables),0);
						}
						p->stack->stack_pointer = stack_top;
						remove_chars_from_output_variables(p->output_variables,captured_chars);
						L2 = L2->next;
					} while (L2 != NULL);
					/* With the cache system, we must not free the parsing information list
					 * free_parsing_info(L_first, p->prv_alloc_recycle, p->prv_alloc);
					 */
				}
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
				free(var_name);
#endif
				/* end of usage of content1 */
				break;
			}

			case META_SDIC:
				error("Error in graph %S:\n",p->fst2->graph_names[get_graph_index(p->fst2,current_state_index)]);
				fatal_error("Unexpected <SDIC> tag in morphological mode\n");
				break;

			case META_CDIC:
				error("Error in graph %S:\n",p->fst2->graph_names[get_graph_index(p->fst2,current_state_index)]);
				fatal_error("Unexpected <CDIC> tag in morphological mode\n");
				break;

			case META_TDIC:
				error("Error in graph %S:\n",p->fst2->graph_names[get_graph_index(p->fst2,current_state_index)]);
				fatal_error("Unexpected <TDIC> tag in morphological mode\n");
				break;

			case META_TEXT_START:
				error("Error in graph %S:\n",p->fst2->graph_names[get_graph_index(p->fst2,current_state_index)]);
				fatal_error("Unexpected <^> tag in morphological mode\n");
				break;

			case META_TEXT_END:
				error("Error in graph %S:\n",p->fst2->graph_names[get_graph_index(p->fst2,current_state_index)]);
				fatal_error("Unexpected <$> tag in morphological mode\n");
				break;

			case META_MAJ: /* In morphological mode, this tag matches an uppercase letter, as defined in
			 the alphabet file */
				if (token == -1)
					break;
				update_last_position(p, pos_in_tokens);
				if (token == p->STOP)
					break;
				if (XOR(is_upper(current_token[pos_in_chars], p->alphabet),
						meta_list->negation)) {
					match_one_letter = 1;
				}
				break;

			case META_MIN: /* In morphological mode, this tag matches a lowercase letter, as defined in
			 the alphabet file */
				if (token == -1)
					break;
				update_last_position(p, pos_in_tokens);
				if (token == p->STOP)
					break;
				if (XOR(is_lower(current_token[pos_in_chars], p->alphabet),
						meta_list->negation)) {
					match_one_letter = 1;
				}
				break;

			case META_PRE:
				error("Error in graph %S:\n",p->fst2->graph_names[get_graph_index(p->fst2,current_state_index)]);
				fatal_error("Unexpected <PRE> tag in morphological mode\n");
				break;

			case META_NB:
				error("Error in graph %S:\n",p->fst2->graph_names[get_graph_index(p->fst2,current_state_index)]);
				fatal_error("Unexpected <NB> tag in morphological mode\n");
				break;

			case META_TOKEN: {
				if (token == -1)
					break;
				update_last_position(p, pos_in_tokens);
				if (token == p->STOP)
					break;
				unichar one_letter[2];
				one_letter[0] = current_token[pos_in_chars];
				one_letter[1] = '\0';
#ifdef REGEX_FACADE_ENGINE
				int filter_number = p->tags[t->tag_number]->filter_number;
				int morpho_filter_OK = (filter_number == -1
						|| string_match_filter(p->filters, one_letter,
								filter_number, p->recyclable_wchart_buffer));
				if (morpho_filter_OK) {
					match_one_letter = 1;
				}
#endif
				break;
			}

			case META_LEFT_CONTEXT:
				error("Error in graph %S:\n",p->fst2->graph_names[get_graph_index(p->fst2,current_state_index)]);
				fatal_error("Unexpected left context mark in morphological mode\n");
				break;

			case META_BEGIN_MORPHO:
				error("Error in graph %S:\n",p->fst2->graph_names[get_graph_index(p->fst2,current_state_index)]);
				fatal_error("Unexpected morphological mode begin tag $<\n");
				break;

			case META_END_MORPHO:
				/* Should happen, but only at the same level than the $< tag */
				if ((p->graph_depth) != 0) {
					error("Error in graph %S:\n",p->fst2->graph_names[get_graph_index(p->fst2,current_state_index)]);
					fatal_error("Unexpected end of morphological mode at a different\nlevel than the $< tag\n");
				}
				if (pos_in_chars != 0 || (jamo != NULL && pos_in_jamo != 0)) {
					/* If the end of the morphological mode occurs in the middle
					 * of a token, we don't take this "match" into account */
					break;
				}
				/* If the end of the morphological mode occurs at the beginning of a token,
				 * then we have a match. We note the state number that corresponds to the state
				 * that follows the current $> tag transition, so that we know where to continue
				 * the exploration in the 'enter_morphological_mode' function. */
				p->stack->stack[stack_top + 1] = '\0';
				if (p->ambiguous_output_policy == ALLOW_AMBIGUOUS_OUTPUTS) {
					(*matches) = insert_if_different(pos_in_tokens, pos_in_chars,
							t->state_number, (*matches),
							p->stack->stack_pointer,
							&(p->stack->stack[p->stack_base + 1]),
							p->input_variables, p->output_variables, p->dic_variables, -1, -1, jamo,
							pos_in_jamo, NULL, p->weight,p->prv_alloc_recycle,p->prv_alloc);
				} else {
					(*matches) = insert_if_absent(pos_in_tokens, pos_in_chars,
							t->state_number, (*matches),
							p->stack->stack_pointer,
							&(p->stack->stack[p->stack_base + 1]),
							p->input_variables, p->output_variables, p->dic_variables, -1, -1, jamo,
							pos_in_jamo, NULL, p->weight,p->prv_alloc_recycle,p->prv_alloc);
				}
				break;

			} /* End of the switch */
			if (match_one_letter) {
				/* We factorize here the cases <MOT>, <MIN> and <MAJ> that all correspond
				 * to matching one letter */
				captured_chars=0;
				if (p->output_policy != IGNORE_OUTPUTS) {
					if (!deal_with_output(p->tags[t->tag_number]->output,p,&captured_chars)) {
						goto next;
					}
				}
				if (p->output_policy == MERGE_OUTPUTS) {
					/* If needed, we push the character that was matched */
					push_input_char(p->stack, current_token[pos_in_chars],
							p->protect_dic_chars);
				}
				int new_pos;
				int new_pos_in_token;
				unichar* new_jamo = NULL;
				int new_pos_in_jamo = 0;
				if (pos_in_chars + 1 == current_token_length) {
					/* If we are at the end of the current token */
					new_pos = pos_in_tokens + 1;
					new_pos_in_token = 0;
					/* We also update the Jamo things */
					new_jamo = NULL;
					if (p->jamo_tags != NULL) {
						new_jamo = p->jamo_tags[p->buffer[new_pos
								+ p->current_origin]];
					}
					new_pos_in_jamo = 0;
				} else {
					/* If not */
					new_pos = pos_in_tokens;
					new_pos_in_token = pos_in_chars + 1;
					new_jamo = jamo;
					/* If we are in Korean mode, we have to move in the Jamo sequence */
					if (jamo != NULL) {
						new_pos_in_jamo = pos_in_jamo + 1;
						while (jamo[new_pos_in_jamo] != '\0') {
							if (jamo[new_pos_in_jamo] == KR_SYLLABLE_BOUND) {
								/* A syllable bound is OK: we go on the following Jamo */
								new_pos_in_jamo++;
								if (!u_is_Hangul_Jamo(jamo[new_pos_in_jamo])) {
									error("Error in graph %S:\n",p->fst2->graph_names[get_graph_index(p->fst2,current_state_index)]);
									fatal_error("Unexpected non Jamo character after a syllable bound\n");
								}
								break;
							}
							if (u_is_Hangul(jamo[new_pos_in_jamo])) {
								/* A Hangul is OK */
								break;
							}
							if (u_is_Hangul_Jamo(jamo[new_pos_in_jamo])) {
								/* If we have a Jamo, we must go on */
								new_pos_in_jamo++;
							} else {
								/* Any other char is OK */
								break;
							}
						}
						/*if (jamo[new_pos_in_jamo] != '\0') {
							fatal_error("Unexpected end of Jamo sequence\n");
						}*/
					}
				}
				morphological_locate(/*graph_depth,*/ t->state_number, new_pos,
						new_pos_in_token, matches, n_matches, ctx,
						p, new_jamo, new_pos_in_jamo,
						content_buffer);
				p->weight=old_weight;
				p->stack->stack_pointer = stack_top;
				remove_chars_from_output_variables(p->output_variables,captured_chars);
			}
			next: t = t->next;
		}
		meta_list = meta_list->next;
	}

	/**
	 * OUTPUT VARIABLE STARTS
	 */
	struct opt_variable* variable_list = current_state->output_variable_starts;
	while (variable_list != NULL) {
		set_output_variable_pending(p->output_variables,variable_list->variable_number);
		morphological_locate(/*graph_depth,*/ variable_list->transition->state_number, pos_in_tokens,
				pos_in_chars, matches, n_matches, ctx,
				p, jamo, pos_in_jamo,
				content_buffer);
		p->weight=old_weight;
		p->stack->stack_pointer = stack_top;
		unset_output_variable_pending(p->output_variables,variable_list->variable_number);
		variable_list=variable_list->next;
	}

	/**
	 * OUTPUT VARIABLE ENDS
	 */
	variable_list = current_state->output_variable_ends;
	while (variable_list != NULL) {
		unset_output_variable_pending(p->output_variables,variable_list->variable_number);
		morphological_locate(/*graph_depth,*/ variable_list->transition->state_number, pos_in_tokens,
				pos_in_chars, matches, n_matches, ctx,
				p, jamo, pos_in_jamo,
				content_buffer);
		p->weight=old_weight;
		p->stack->stack_pointer = stack_top;
		set_output_variable_pending(p->output_variables,variable_list->variable_number);
		variable_list=variable_list->next;
	}

	/**
	 * VARIABLE STARTS
	 */
	variable_list = current_state->input_variable_starts;
	while (variable_list != NULL) {
		inc_dirty(p->backup_memory_reserve);
		int old_in_token = get_variable_start(p->input_variables,
				variable_list->variable_number);
		int old_in_char = get_variable_start_in_chars(p->input_variables,
						variable_list->variable_number);
		set_variable_start(p->input_variables, variable_list->variable_number, pos_in_tokens);
		set_variable_start_in_chars(p->input_variables, variable_list->variable_number, pos_in_chars);
		//error("var start=tok=%d char=%d\n",pos_in_tokens,pos_in_chars);
		morphological_locate(/*graph_depth,*/ variable_list->transition->state_number, pos_in_tokens,
						pos_in_chars, matches, n_matches, ctx,
						p, jamo, pos_in_jamo,
						content_buffer);
		p->weight=old_weight;
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
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	while (variable_list != NULL) {
		inc_dirty(p->backup_memory_reserve);
		int old_in_token =
				get_variable_end(p->input_variables, variable_list->variable_number);
		int old_in_char = get_variable_end_in_chars(p->input_variables, variable_list->variable_number);
		int new_end_in_token;
		int new_end_in_chars;
		if (pos_in_chars==0) {
			/* If we have just started a new token, then the variable
			 * ends on the end of the previous one */
			new_end_in_token=pos_in_tokens;
			new_end_in_chars=-1;
		} else  {
			new_end_in_token=pos_in_tokens+1;
			new_end_in_chars=pos_in_chars-1;
		}
		//error("var end=tok=%d char=%d\n",new_end_in_token,new_end_in_chars);
		set_variable_end(p->input_variables, variable_list->variable_number, new_end_in_token);
		set_variable_end_in_chars(p->input_variables, variable_list->variable_number,new_end_in_chars);
		morphological_locate(/*graph_depth,*/ variable_list->transition->state_number, pos_in_tokens,
						pos_in_chars, matches, n_matches, ctx,
						p, jamo, pos_in_jamo,
						content_buffer);
		p->weight=old_weight;
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

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	if (token == -1 || token == p->STOP) {
		/* We can't match anything if we are at the end of the buffer or if
		 * we have the forbidden token {STOP} that must never be matched. */
		p->explore_depth -- ;
		return;
	}

	/**
	 * CONTEXTS
	 */
	struct opt_contexts* contexts = current_state->contexts;
	if (contexts != NULL) {
		error("Error in graph %S:\n",p->fst2->graph_names[get_graph_index(p->fst2,current_state_index)]);
		fatal_error("Unexpected use of contexts in morphological mode\n");
	}

	/**
	 * TOKENS
	 */
	Transition* trans = current_state_old->transitions;
	while (trans != NULL) {
		if (trans->tag_number < 0) {
			/* We don't take subgraph transitions into account */
			trans = trans->next;
			continue;
		}
		Fst2Tag tag = p->tags[trans->tag_number];
		if (tag->pattern != NULL) {
			update_last_position(p, pos_in_tokens);
			if (tag->pattern->type == TOKEN_PATTERN) {
				unichar* tag_token = tag->pattern->inflected;
				int comma = -1;
				if (tag_token[0] == '{' && u_strcmp(tag_token, "{")
						&& u_strcmp(tag_token, "{S}")) {
					/* If we have a tag like {eats,eat.V:P3s} */
					tag_token++; /* We ignore the { */
					while (tag_token[comma] != ',') {
						comma++;
					}
					/* We want to avoid a copy */
					tag_token[comma] = '\0';
				}
				int tag_token_length = u_strlen(tag_token);
				if (jamo != NULL) {
					/* KOREAN token matching */
					//error("comparing txt=<%S> and tag=<%S>\n",jamo,tag_token);
					int new_pos_in_jamo = pos_in_jamo;
					int new_pos_in_token = pos_in_chars;
					int result = get_jamo_longest_prefix(jamo,
							&new_pos_in_jamo, &new_pos_in_token, tag_token, p,
							current_token);
					if (comma != -1) {
						/* If necessary, we restore the tag */
						tag_token[comma] = ',';
					}
					if (result != 0) {
						/* Nothing to do if the match failed */
						int new_pos = pos_in_tokens;
						unichar* new_jamo = jamo;
						if (result == 1) {
							/* The text token has been fully matched, so we go on the next one */
							new_pos_in_token = 0;
							new_pos = pos_in_tokens + 1;
							new_jamo = p->jamo_tags[p->buffer[new_pos
									+ p->current_origin]];
							new_pos_in_jamo = 0;
						}
						/* If we can match the tag's token, we process the output, if we have to */
						captured_chars=0;
						if (p->output_policy != IGNORE_OUTPUTS) {
							if (!deal_with_output(tag->output,p,&captured_chars)) {
								continue;
							}
						}
						if (p->output_policy == MERGE_OUTPUTS) {
							fatal_error(
									"Unsupported MERGE mode in Korean morphological mode\n");
							//push_input_substring(p->stack,current_token+pos_in_token,prefix_length,p->protect_dic_chars);
						}
						morphological_locate(/*graph_depth,*/ trans->state_number,
								new_pos, new_pos_in_token, matches,
								n_matches, ctx, p, new_jamo,
								new_pos_in_jamo, content_buffer);
						p->weight=old_weight;
						remove_chars_from_output_variables(p->output_variables,captured_chars);
						p->stack->stack_pointer = stack_top;
					}
					/* End of KOREAN token matching */
				} else {
					/* Non Korean token matching*/
					int prefix_length;
					if (tag->control & RESPECT_CASE_TAG_BIT_MASK) {
						/* If we must have a perfect match */
						prefix_length = get_longuest_prefix(current_token
								+ pos_in_chars, tag_token);
					} else {
						prefix_length = get_longuest_prefix_ignoring_case(
								current_token + pos_in_chars, tag_token,
								p->alphabet);
					}
					if (prefix_length == tag_token_length) {
						/* If we can match the tag's token, we process the output, if we have to */
						captured_chars=0;
						if (p->output_policy != IGNORE_OUTPUTS) {
							if (!deal_with_output(tag->output,p,&captured_chars)) {
								continue;
							}
						}
						if (p->output_policy == MERGE_OUTPUTS) {
							push_input_substring(p->stack, current_token
									+ pos_in_chars, prefix_length,
									p->protect_dic_chars);
						}
						int new_pos, new_pos_in_token;
						if (pos_in_chars + prefix_length < current_token_length) {
							/* If we didn't reach the end of the current token */
							new_pos = pos_in_tokens;
							new_pos_in_token = pos_in_chars + prefix_length;
						} else {
							/* If we must go on the next token */
							new_pos = pos_in_tokens + 1;
							new_pos_in_token = 0;
						}
						morphological_locate(/*graph_depth,*/ trans->state_number,
								new_pos, new_pos_in_token, matches,
								n_matches, ctx, p, jamo,
								pos_in_jamo, content_buffer);
						p->weight=old_weight;
						p->stack->stack_pointer = stack_top;
						remove_chars_from_output_variables(p->output_variables,captured_chars);
					} else {
						/* No else here, because a grammar tag is not supposed to match a sequence that
						 * overlaps two text tokens. */
					}
					/* End of non Korean token matching */
				}
			} else {
				/* Here, we deal with all the "real" patterns: <be>, <N+z1:ms>, <be.V:K> and <am,be.V> */
				struct parsing_info* L = NULL;
				int len_var_name = 0;
				if (tag->output != NULL) {
					len_var_name = 31;//u_strlen(tag->output);
				}
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
				unichar *var_name;
				var_name=(unichar*)malloc(sizeof(unichar)*(len_var_name+1));
				if (var_name==NULL) {
					fatal_alloc_error("morphological_locate");
				}
#else
				unichar var_name[len_var_name + 1];
#endif

				int save_dic_entry = (p->output_policy != IGNORE_OUTPUTS
						&& is_morpho_variable_output(tag->output, var_name));

				/*explore_dic_in_morpho_mode(p, pos_in_tokens, pos_in_chars, &L,
						tag->pattern, save_dic_entry, jamo, pos_in_jamo);*/
				if (!DIC_cached) {
					DIC_cached=1;
					explore_dic_in_morpho_mode(p, pos_in_tokens, pos_in_chars, &DIC_consultation,
											NULL, 1, jamo, pos_in_jamo);
				}
				L=DIC_consultation;
				unichar *content2 = content_buffer;
				if (L != NULL) {
					/* If there is at least one match, we process the match list */
					do {
						if (!is_entry_compatible_with_pattern(L->dic_entry,tag->pattern)) {
							/* We take all <DIC> entries from the cache, and we compare them
							 * with the actual <X> pattern of the current tag */
							p->stack->stack_pointer = stack_top;
							L = L->next;
							continue;
						}
						get_content(content2, p, pos_in_tokens, pos_in_chars,
								L->pos_in_tokens, L->pos_in_chars);
#ifdef REGEX_FACADE_ENGINE
						int filter_number =
								p->tags[trans->tag_number]->filter_number;
						int morpho_filter_OK = (filter_number == -1
								|| string_match_filter(p->filters, content2,
										filter_number,
										p->recyclable_wchart_buffer));
						if (!morpho_filter_OK) {
							p->stack->stack_pointer = stack_top;
							L = L->next;
							continue;
						}
#endif
						/* WARNING: we don't process the tag's output as usual if it
						 *          is a variable declaration like $abc$. Note that it could
						 *          make a difference if a variable with the same
						 *          name was declared before entering the morphological mode */
						captured_chars=0;
						if (!save_dic_entry && p->output_policy != IGNORE_OUTPUTS) {
							if (!deal_with_output(tag->output,p,&captured_chars)) {
								continue;
							}
						}
						if (p->output_policy == MERGE_OUTPUTS) {
							push_input_string(p->stack, content2,
									p->protect_dic_chars);
						}
						unichar* reached_token =
								p->tokens->value[p->buffer[p->current_origin
										+ L->pos_in_tokens]];
						int new_pos, new_pos_in_token, new_pos_in_jamo;
						//error("dic match output=<%S>\n",p->stack->stack);
						if (reached_token[L->pos_in_chars] == '\0' && (L->jamo==NULL || L->jamo[L->pos_in_jamo]=='\0')) {
							/* If we are at the end of the last token matched by the dictionary search */
							new_pos = L->pos_in_tokens + 1;
							new_pos_in_token = 0;
							new_pos_in_jamo=0;
							//error(" => end of the token <%S>  pos=%d  jamo=<%S> pos_jamo=%d\n",reached_token,L->pos_in_chars,L->jamo,L->pos_in_jamo);
						} else {
							/* If not */
							new_pos = L->pos_in_tokens;
							new_pos_in_token = L->pos_in_chars;
							new_pos_in_jamo=L->pos_in_jamo;
							//error(" => token=%d char=%d\n",new_pos,new_pos_in_token);
						}
						/* We continue the exploration */
						struct dela_entry* old_value = NULL;
						if (save_dic_entry) {
							old_value = clone_dela_entry(get_dic_variable(
									var_name, p->dic_variables));
							set_dic_variable(var_name, L->dic_entry,
									&(p->dic_variables),1);
						}
						morphological_locate(/*graph_depth,*/ trans->state_number,
								new_pos, new_pos_in_token, matches,
								n_matches, ctx, p, L->jamo,
								new_pos_in_jamo, content_buffer);
						p->weight=old_weight;
						if (save_dic_entry) {
							set_dic_variable(var_name, old_value,
									&(p->dic_variables),0);
						}
						remove_chars_from_output_variables(p->output_variables,captured_chars);
						p->stack->stack_pointer = stack_top;
						L = L->next;
					} while (L != NULL);
					/* No free, because of the cache system
					 * free_parsing_info(L_first, p->prv_alloc_recycle, p->prv_alloc);
					 */
				}
#ifdef NO_C99_VARIABLE_LENGTH_ARRAY
				free(var_name);
#endif
				/* end of usage of content2 */
			}
		}
		trans = trans->next;
	}
    p->explore_depth -- ;
    free_parsing_info(DIC_consultation, p->prv_alloc_recycle, p->prv_alloc);
}

/**
 * Tests whether this tag has an input that is a token and not something like <...>.
 * Returns 1 in that case; 0 otherwise.
 */
int input_is_token(Fst2Tag tag) {
	return (tag->input[0] != '<' || tag->input[1] == '\0');
}

/**
 * This is function that starts matching things in morphological mode.
 */
void enter_morphological_mode(/*int graph_depth,*/ /* 0 means that we are in the top level graph */
int state, /* current state in the grammar */
int pos, /* position in the token buffer, relative to the current origin */
//int depth, /* number of nested calls to 'locate' */
struct parsing_info** matches, /* current match list. Irrelevant if graph_depth==0 */
int *n_matches, /* number of sequences that have matched. It may be different from
 * the length of the 'matches' list if a given sequence can be
 * matched in several ways. It is used to detect combinatory
 * explosions due to bad written grammars. */
struct list_context* ctx, /* information about the current context, if any */
struct locate_parameters* p /* miscellaneous parameters needed by the function */
//,variable_backup_memory_reserve* backup_reserve_
) {
	int old_weight=p->weight;
    p->explore_depth ++ ;
	unichar* content_buffer = (unichar*) malloc(sizeof(unichar) * 4096);
	if (content_buffer == NULL) {
		fatal_alloc_error("enter_morphological_mode");
	}
	int* var_backup = NULL;
	unichar* output_variable_backup=NULL;
	int old_StackBase;
	int stack_top = p->stack->stack_pointer;
	old_StackBase = p->stack_base;
	if (p->output_policy != IGNORE_OUTPUTS) {
		/* For better performance when ignoring outputs */
		var_backup = create_variable_backup(p->input_variables,p->prv_alloc_recycle);
		if (p->nb_output_variables != 0) {
			output_variable_backup=create_output_variable_backup(p->output_variables);
		}
	}
	struct parsing_info* L = NULL;
	p->stack_base = p->stack->stack_pointer;
	struct dic_variable* dic_variable_backup = NULL;
	if (p->dic_variables != NULL) {
		dic_variable_backup = clone_dic_variable_list(p->dic_variables);
	}
	int current_token = p->buffer[pos + p->current_origin];
	//error("current token=%d/%S  jamo=%S\n",current_token,p->tokens->value[current_token],p->jamo_tags[current_token]);

    int backup_graph_depth = p->graph_depth;
    p->graph_depth = 0;
	morphological_locate(/*0,*/ state, pos, 0, &L, 0, NULL, p,
			(p->jamo_tags != NULL) ? p->jamo_tags[current_token] : NULL, 0,
			content_buffer);
    p->graph_depth = backup_graph_depth;

	clear_dic_variable_list(&(p->dic_variables));
	p->stack_base = old_StackBase;
	if (L != NULL) {
		struct parsing_info* L_first = L;
		/* If there is at least one match, we process the match list */
		do {
			/* We restore the settings that we had at the end of the exploration in morphological mode */
			if (p->output_policy != IGNORE_OUTPUTS) {
				u_strcpy(&(p->stack->stack[stack_top + 1]), L->stack);
				p->stack->stack_pointer = L->stack_pointer;
				install_variable_backup(p->input_variables, L->input_variable_backup);
				if (p->nb_output_variables != 0) {
					install_output_variable_backup(p->output_variables,L->output_variable_backup);
				}
			}
			p->dic_variables = NULL;
			if (L->dic_variable_backup != NULL) {
				p->dic_variables = clone_dic_variable_list(L->dic_variable_backup);
			}
			/* And we continue the exploration */

			variable_backup_memory_reserve* reserve_previous = NULL;

			if (is_enough_memory_in_reserve_for_transduction_variable_set(p->input_variables,
					p->backup_memory_reserve) == 0) {
				reserve_previous = p->backup_memory_reserve ;
				p->backup_memory_reserve = create_variable_backup_memory_reserve(
						p->input_variables,0);
			}


			p->weight=L->weight;
			locate(/*graph_depth, */p->optimized_states[L->state_number],
					L->pos_in_tokens, matches, n_matches, ctx, p);
            /*
			if ((p->max_count_call > 0) && (p->counting_step.count_call >= p->max_count_call)) {
				u_printf("stop computing token %u after %u step computing\n",
						p->current_origin, p->counting_step.count_call);
			} else if ((p->max_count_call_warning > 0) && (p->counting_step.count_call
					>= p->max_count_call_warning)) {
				u_printf(
						"warning : computing token %u take %u step computing\n",
						p->current_origin, p->counting_step.count_call);
			}
            */



			if (reserve_previous != NULL) {
					free_reserve(p->backup_memory_reserve);
					p->backup_memory_reserve = reserve_previous;
			}




			p->stack->stack_pointer = stack_top;
			if (p->graph_depth == 0) {
				/* If we are at the top graph level, we restore the variables */
				if (p->output_policy != IGNORE_OUTPUTS) {
					install_variable_backup(p->input_variables, var_backup);
				}
			}
			if ((ctx == NULL || L->next != NULL) && (p->dic_variables != NULL)) {
				/* If we are inside a context, we don't want to free all the dic_variables that
				 * have been set, in order to allow extracting morphological information from contexts.
				 * To do that, we arbitrarily keep the dic_variables of the last path match. */
				clear_dic_variable_list(&(p->dic_variables));
			}
			L = L->next;
		} while (L != NULL);
		free_parsing_info(L_first,p->prv_alloc_recycle, p->prv_alloc); /* free all morphological matches */
	}
	/* Finally, we have to restore the stack and other backup stuff */
	p->weight=old_weight;
	p->stack->stack_pointer = stack_top;
	p->stack_base = old_StackBase; /* May be changed by recursive subgraph calls */
	if (p->output_policy != IGNORE_OUTPUTS) { /* For better performance (see above) */
		install_variable_backup(p->input_variables, var_backup);
		free_variable_backup(var_backup,p->prv_alloc_recycle);
		if (p->nb_output_variables != 0) {
			install_output_variable_backup(p->output_variables,output_variable_backup);
			free_output_variable_backup(output_variable_backup);
		}
	}
	if (ctx == NULL) {
		p->dic_variables = dic_variable_backup;
	} else {
		if (dic_variable_backup != NULL) {
			clear_dic_variable_list(&dic_variable_backup);
		}
	}
	free(content_buffer);
	p->explore_depth -- ;
}

/**
 * Tries to find something in the dictionary that match both text content and
 * given pattern. This version of the function handles the all languages but Arabic.
 */
static void explore_dic_in_morpho_mode_standard(struct locate_parameters* p,
		Dictionary* d, int offset,
		unichar* current_token, unichar* inflected, int pos_in_current_token,
		int pos_in_inflected, int pos_offset, struct parsing_info* *matches,
		struct pattern* pattern, int save_dic_entry, unichar* jamo,
		int pos_in_jamo, Ustring *line_buffer,Ustring* ustr,int base) {
int final,n_transitions,inf_number;
int z=save_output(ustr);
offset=read_dictionary_state(d,offset,&final,&n_transitions,&inf_number);
	if (final) {
		//error("\narriba!\n\n\n");
		/* If this node is final, we get the INF line number */
		inflected[pos_in_inflected] = '\0';
		if (pattern == NULL && !save_dic_entry) {
			/* If any word will do with no entry saving */
			(*matches) = insert_morphological_match(pos_offset,
					pos_in_current_token, -1, (*matches), NULL, jamo,
					pos_in_jamo, p->prv_alloc_recycle,p->prv_alloc);
		} else {
			/* If we have to check the pattern */
			struct list_ustring* tmp = d->inf->codes[inf_number];
			while (tmp != NULL) {
				/* For each compressed code of the INF line, we save the corresponding
				 * DELAF line in 'info->dlc' */
				uncompress_entry(inflected, tmp->string, line_buffer);
				//error("\non decompresse la ligne _%S_\n",line_buffer->str);
				struct dela_entry* dela_entry = tokenize_DELAF_line_opt(line_buffer->str);
				if (dela_entry != NULL
						&& (pattern == NULL
								|| is_entry_compatible_with_pattern(dela_entry,
										pattern))) {
					//error("et ca matche!!\n");
					(*matches) = insert_morphological_match(pos_offset,
							pos_in_current_token, -1, (*matches),
							save_dic_entry ? dela_entry : NULL, jamo,
							pos_in_jamo, p->prv_alloc_recycle,p->prv_alloc);
				}
				free_dela_entry(dela_entry);
				tmp = tmp->next;
			}
		}
		base=ustr->len;
	}

	if (current_token[pos_in_current_token] == '\0') {
		if (jamo == NULL) {
			/* If we have reached the end of the current token in a non Korean language */
			pos_offset++;
			int token_number = (((int)(pos_offset + p->current_origin + 1)) <= (int)(p->buffer_size)) ?
				p->buffer[pos_offset + p->current_origin] : -1;
			
			if (token_number == -1 || token_number == p->STOP) {
				/* Remember 1) that we must not be out of the array's bounds and
				 *          2) that the token {STOP} must never be matched */
				return;
			}

			current_token = p->tokens->value[token_number];
			pos_in_current_token = 0;
		} else {
			/* We are in Korean mode */
			if (jamo[pos_in_jamo] == '\0') {
				/* In korean, we perform a token change only if we have used all the token's jamos */
				pos_offset++;
				int token_number = p->buffer[pos_offset + p->current_origin];
				if (token_number == -1 || token_number == p->STOP) {
					/* Remember 1) that we must not be out of the array's bounds and
					 *          2) that the token {STOP} must never be matched */
					return;
				}
				current_token = p->tokens->value[token_number];
				pos_in_current_token = 0;
				pos_in_jamo = 0;
				jamo = p->jamo_tags[token_number];
			}
		}
	}
	//int after_syllab_bound = 0;
	if (jamo != NULL) {
		/* We test whether we are in the middle of a syllable or just after a syllable bound */
		if (jamo[pos_in_jamo] == KR_SYLLABLE_BOUND) {
			/* If we have a syllable bound */
			//after_syllab_bound = 1;
			pos_in_current_token++;
			//pos_in_jamo++;
		} else if (pos_in_jamo > 0 && jamo[pos_in_jamo - 1]
				== KR_SYLLABLE_BOUND) {
			/* If we are just after a syllable bound */
			//after_syllab_bound = 1;
		} else {
			/* By default, we must be in the middle of a syllable, and there's nothing to do */
		}
	}

	/* We look for outgoing transitions */
	unichar c;
	int adr;
	for (int i = 0; i < n_transitions; i++) {
		update_last_position(p, pos_offset);
		offset=read_dictionary_transition(d,offset,&c,&adr,ustr);
		if (jamo == NULL) {
			/* Non Korean mode */
			if (is_equal_or_uppercase(c, current_token[pos_in_current_token],
					p->alphabet)) {
				/* We explore the rest of the dictionary only if the
				 * dictionary char is compatible with the token char. In that case,
				 * we copy in 'inflected' the exact character that is in the dictionary. */
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_standard(p, d, adr,
						current_token, inflected, pos_in_current_token + 1,
						pos_in_inflected + 1, pos_offset, matches, pattern,
						save_dic_entry, jamo, pos_in_jamo, line_buffer, ustr, base);
			}
		} else {
			//error("la: jamo du text<%S>=%C (%04X)   char du dico=%C (%04X)\n",jamo,jamo[pos_in_jamo],jamo[pos_in_jamo],c,c);
			/* Korean mode: we may match just the current jamo, or also the current hangul, but only if we are
			 * after a syllable bound */
			unichar c2[2];
			c2[0] = c;
			c2[1] = '\0';
			int syllable_bounds=0;
			/* We try to match all the jamo sequence found in the dictionary */
			if (jamo[pos_in_jamo]==KR_SYLLABLE_BOUND) {
				//error("jamo=<%S> %C bound dic=%C (%X)     pos_inflected=%d\n",jamo,jamo[pos_in_jamo],c,c,pos_in_inflected);
				if (c!=KR_SYLLABLE_BOUND) {
					/* no match */
					restore_output(z,ustr);
					continue;
				}
				syllable_bounds=1;
			} else {
				//error("jamo=%C (%X) dic=%C (%X)\n",jamo[pos_in_jamo],jamo[pos_in_jamo],c,c);
			}
			int new_pos_in_current_token = pos_in_current_token;
			int new_pos_in_jamo = pos_in_jamo;
			int result=0;
			if (!syllable_bounds) {
				result = get_jamo_longest_prefix(jamo, &new_pos_in_jamo,
					&new_pos_in_current_token, c2, p, current_token);
			}
			if (result != 0 || syllable_bounds) {
				if (syllable_bounds) {
					new_pos_in_jamo++;
				}
				//error("match next pos_in_jamo=%d (%C)\n",new_pos_in_jamo,jamo[new_pos_in_jamo]);
				//error("MATCH entre jamo du text=%C (%04X)   char du dico=%C (%04X)\n",jamo[pos_in_jamo],jamo[pos_in_jamo],c,c);
				/* Nothing to do if the match failed */
				int new_pos_offset = pos_offset;
				unichar* new_jamo = jamo;
				unichar* new_current_token = current_token;
				if (result == 1 || (syllable_bounds && jamo[new_pos_in_jamo]=='\0')) {
					/* The text token has been fully matched, so we go on the next one */
					new_pos_in_current_token = 0;
					new_pos_offset = pos_offset + 1;
					int token_number = p->buffer[new_pos_offset
							+ p->current_origin];
					if (token_number == -1 || token_number == p->STOP) {
						/* Remember 1) that we must not be out of the array's bounds and
						 *          2) that the token {STOP} must never be matched */
						restore_output(z,ustr);
						return;
					}
					new_current_token = p->tokens->value[token_number];
					new_jamo = p->jamo_tags[token_number];
					new_pos_in_jamo = 0;
				}
				/* If we have a jamo letter match */
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_standard(p, d, adr,
						new_current_token, inflected, new_pos_in_current_token,
						pos_in_inflected + 1, new_pos_offset, matches, pattern,
						save_dic_entry, new_jamo, new_pos_in_jamo, line_buffer, ustr, base);
			} else {
				//error("non match\n");
			}
			/* Then we try to match a hangul, but only if we are just after a syllable bound */
			//error("after syllable=%d:  text=%C (%04X)   dico=%C (%04X)\n",after_syllab_bound,current_token[pos_in_current_token],current_token[pos_in_current_token],c,c);
		}
		restore_output(z,ustr);
	}
}

static int shadda_may_be_omitted(ArabicTypoRules r) {
	return r.shadda_damma_omission || r.shadda_damma_omission_at_end
			|| r.shadda_dammatan_omission_at_end || r.shadda_fatha_omission
			|| r.shadda_fatha_omission_at_end
			|| r.shadda_fathatan_omission_at_end || r.shadda_kasra_omission
			|| r.shadda_kasra_omission_at_end
			|| r.shadda_kasratan_omission_at_end
			|| r.shadda_superscript_alef_omission;
}

/**
 * Tries to find something in the dictionary that matches both text content
 * and given pattern. This version of the function is dedicated to Arabic.
 */
#define NOTHING_EXPECTED 0
#define END_OF_WORD_EXPECTED 1
#define NO_END_OF_WORD_EXPECTED 2
#define AL_EXPECTED 4
#define AF_EXPECTED 8
#define YF_EXPECTED 16

static void explore_dic_in_morpho_mode_arabic(struct locate_parameters* p,
		Dictionary* d, int offset,
		unichar* current_token, unichar* inflected, int pos_in_current_token,
		int pos_in_inflected, int pos_offset, struct parsing_info* *matches,
		struct pattern* pattern, int save_dic_entry, Ustring* line_buffer,
		int expected, unichar last_dic_char, Ustring* ustr, int base) {
int old_offset=offset;
int final,n_transitions,inf_number;
int z=save_output(ustr);
offset=read_dictionary_state(d,offset,&final,&n_transitions,&inf_number);
	if (final) {
		if (expected & NO_END_OF_WORD_EXPECTED) {
			/* If we were not supposed to find the end of the word */
			return;
		}
		if (expected & AL_EXPECTED) {
			/* If we were only supposed to match a Al */
			if (pos_in_inflected!=2 || inflected[pos_in_inflected-1]!=AR_LAM) return;
		}
		/* If this node is final, we get the INF line number */
		inflected[pos_in_inflected] = '\0';
		if (pattern == NULL && !save_dic_entry) {
			/* If any word will do with no entry saving */
			(*matches) = insert_morphological_match(pos_offset,
					pos_in_current_token, -1, (*matches), NULL, NULL, 0, p->prv_alloc_recycle,p->prv_alloc);
		} else {
			/* If we have to check the pattern */
			struct list_ustring* tmp = d->inf->codes[inf_number];
			while (tmp != NULL) {
				/* For each compressed code of the INF line, we save the corresponding
				 * DELAF line in 'info->dlc' */
				uncompress_entry(inflected, tmp->string, line_buffer);
				//error("on a decompresse la ligne %S\n",line);
				struct dela_entry* dela_entry = tokenize_DELAF_line_opt(line_buffer->str);
				if (dela_entry != NULL
						&& (pattern == NULL
								|| is_entry_compatible_with_pattern(dela_entry,
										pattern))) {
					//error("et ca matche!\n");
					(*matches) = insert_morphological_match(pos_offset,
							pos_in_current_token, -1, (*matches),
							save_dic_entry ? dela_entry : NULL, NULL, 0, p->prv_alloc_recycle,p->prv_alloc);
				}
				free_dela_entry(dela_entry);
				tmp = tmp->next;
			}
		}
		base=ustr->len;
	}
	if (expected & END_OF_WORD_EXPECTED) {
		/* If we were supposed to reach the end of the word, then we don't have to
		 * explore more transitions */
		return;
	}
	if (current_token[pos_in_current_token] == '\0') {
		/* If we have reached the end of the current token */
		pos_offset++;
		int token_number = p->buffer[pos_offset + p->current_origin];
		if (token_number == -1 || token_number == p->STOP) {
			/* Remember 1) that we must not be out of the array's bounds and
			 *          2) that the token {STOP} must never be matched */
			return;
		}
		current_token = p->tokens->value[token_number];
		pos_in_current_token = 0;
	}

	unichar c;
	int adr;
	for (int i = 0; i < n_transitions; i++) {
		update_last_position(p, pos_offset);
		offset=read_dictionary_transition(d,offset,&c,&adr,ustr);
		/* Standard case: matching the char in the dictionary */
		if (c==current_token[pos_in_current_token]) {
			/* We explore the rest of the dictionary only if the
			 * dictionary char is compatible with the token char. In that case,
			 * we copy in 'inflected' the exact character that is in the dictionary. */
			int can_go_on=0;
			int next = NOTHING_EXPECTED;
			if (last_dic_char == AR_SHADDA
					&& (pos_in_current_token==0	|| current_token[pos_in_current_token-1]!=AR_SHADDA)) {
				/* Additional test: if we have matched X, we must check whether there was a shadda X
				 * rule pending with a omitted shadda. In that case, we must check whether we are
				 * allowed to go on or not. Moreover, we must determine the end of word is
				 * expected now or not */
				switch (c) {
				case AR_FATHATAN: {
					if (p->arabic.shadda_fathatan_omission_at_end) {
						can_go_on=1;
						next=next | END_OF_WORD_EXPECTED;
					}
					break;
				}
				case AR_DAMMATAN: {
					if (p->arabic.shadda_dammatan_omission_at_end) {
						can_go_on=1;
						next=next | END_OF_WORD_EXPECTED;
					}
					break;
				}
				case AR_KASRATAN: {
					if (p->arabic.shadda_kasratan_omission_at_end) {
						can_go_on=1;
						next=next | END_OF_WORD_EXPECTED;
					}
					break;
				}
				case AR_FATHA: {
					if (p->arabic.shadda_fatha_omission_at_end) {
						can_go_on=1;
						next=next | END_OF_WORD_EXPECTED;
					}
#if 0
					if (p->arabic.shadda_fatha_omission) {
						can_go_on=1;
						next=next | END_OF_WORD_EXPECTED;
					}
					if (p->arabic.shadda_fatha_omission_at_end) can_go_on=1;
#endif
					break;
				}
				case AR_DAMMA: {
					if (p->arabic.shadda_damma_omission_at_end) {
						can_go_on=1;
						next=next | END_OF_WORD_EXPECTED;
					}
#if 0
					if (p->arabic.shadda_damma_omission) {
						can_go_on=1;
						next=next | END_OF_WORD_EXPECTED;
					}
					if (p->arabic.shadda_damma_omission_at_end) can_go_on=1;
#endif
					break;
				}
				case AR_KASRA: {
					if (p->arabic.shadda_kasra_omission_at_end) {
						can_go_on=1;
						next=next | END_OF_WORD_EXPECTED;
					}
#if 0
					if (p->arabic.shadda_kasra_omission) {
						can_go_on=1;
						next=next | END_OF_WORD_EXPECTED;
					}
					if (p->arabic.shadda_kasra_omission_at_end) can_go_on=1;
#endif
					break;
				}
				case AR_SUPERSCRIPT_ALEF: {
					if (p->arabic.shadda_superscript_alef_omission) {
						can_go_on=1;
						next=next | END_OF_WORD_EXPECTED;
					}
					break;
				}
				default: break;
					//fatal_error("Invalid switch value %C in explore_dic_in_morpho_mode_arabic!\n",c);
				}
			} else {
				can_go_on=1;
			}

			/*error("on compare %C et %C pour le token %S\n",to_buckwalter_plusplus(c),to_buckwalter_plusplus(current_token[pos_in_current_token]),current_token);
			error("pos+1=%d => %X\n",pos_in_current_token+1,current_token[pos_in_current_token+1]);
			for (int i=0;current_token[i];i++) {
				error("%C",to_buckwalter_plusplus(current_token[i]));
			}
			error("\n");*/
			if (can_go_on) {
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_arabic(p, d, adr,
						current_token, inflected, pos_in_current_token + 1,
						pos_in_inflected + 1, pos_offset, matches, pattern,
						save_dic_entry, line_buffer, next, c, ustr, base);
			}
		} else {
			/* If the dictionary char and the text char have not matched:
			 *
			 * it may be because of Al written Ll */
			if (c==AR_ALEF && current_token[pos_in_current_token]==AR_ALEF_WASLA
					&& pos_in_inflected==0 && p->arabic.al_with_wasla) {
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_arabic(p, d, adr,
						current_token, inflected, pos_in_current_token + 1,
						pos_in_inflected + 1, pos_offset, matches, pattern,
						save_dic_entry, line_buffer, AL_EXPECTED, c, ustr, base);
			}
			/* or it may be because of an initial O written A */
			else if (c==AR_ALEF_WITH_HAMZA_ABOVE && current_token[pos_in_current_token]==AR_ALEF
					&& pos_in_inflected==0 && p->arabic.alef_hamza_above_O_to_A) {
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_arabic(p, d, adr,
						current_token, inflected, pos_in_current_token + 1,
						pos_in_inflected + 1, pos_offset, matches, pattern,
						save_dic_entry, line_buffer, NOTHING_EXPECTED, c, ustr, base);
			}
			/* or it may be because of an initial I written A */
			else if (c==AR_ALEF_WITH_HAMZA_BELOW && current_token[pos_in_current_token]==AR_ALEF
					&& pos_in_inflected==0 && p->arabic.alef_hamza_below_I_to_A) {
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_arabic(p, d, adr,
						current_token, inflected, pos_in_current_token + 1,
						pos_in_inflected + 1, pos_offset, matches, pattern,
						save_dic_entry, line_buffer, NOTHING_EXPECTED, c, ustr, base);
			}
			/* or it may be because of an initial I written L */
			else if (c==AR_ALEF_WITH_HAMZA_BELOW && current_token[pos_in_current_token]==AR_ALEF_WASLA
					&& pos_in_inflected==0 && p->arabic.alef_hamza_below_I_to_L) {
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_arabic(p, d, adr,
						current_token, inflected, pos_in_current_token + 1,
						pos_in_inflected + 1, pos_offset, matches, pattern,
						save_dic_entry, line_buffer, NOTHING_EXPECTED, c, ustr, base);
			}
			/* or it may be because of the FA -> AF rule, matching the first char */
			else if (c==AR_FATHATAN && current_token[pos_in_current_token]==AR_ALEF
					&& p->arabic.fathatan_alef_equiv_alef_fathatan) {
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_arabic(p, d, adr,
						current_token, inflected, pos_in_current_token + 1,
						pos_in_inflected + 1, pos_offset, matches, pattern,
						save_dic_entry, line_buffer, AF_EXPECTED, c, ustr, base);
			}
			/* or it may be because of the FA -> AF rule, matching the second char */
			else if (c==AR_ALEF && current_token[pos_in_current_token]==AR_FATHATAN
					&& expected==AF_EXPECTED
					&& p->arabic.fathatan_alef_equiv_alef_fathatan) {
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_arabic(p, d, adr,
						current_token, inflected, pos_in_current_token + 1,
						pos_in_inflected + 1, pos_offset, matches, pattern,
						save_dic_entry, line_buffer, END_OF_WORD_EXPECTED, c, ustr, base);
			}
			/* or it may be because of the FY -> YF rule, matching the first char */
			else if (c==AR_FATHATAN && current_token[pos_in_current_token]==AR_ALEF_MAQSURA
					&& p->arabic.fathatan_alef_maqsura_equiv_alef_maqsura_fathatan) {
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_arabic(p, d, adr,
						current_token, inflected, pos_in_current_token + 1,
						pos_in_inflected + 1, pos_offset, matches, pattern,
						save_dic_entry, line_buffer, YF_EXPECTED, c, ustr, base);
			}
			/* or it may be because of the FY -> YF rule, matching the second char */
			else if (c==AR_ALEF_MAQSURA && current_token[pos_in_current_token]==AR_FATHATAN
					&& expected==YF_EXPECTED
					&& p->arabic.fathatan_alef_maqsura_equiv_alef_maqsura_fathatan) {
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_arabic(p, d, adr,
						current_token, inflected, pos_in_current_token + 1,
						pos_in_inflected + 1, pos_offset, matches, pattern,
						save_dic_entry, line_buffer, END_OF_WORD_EXPECTED, c, ustr, base);
			}
			/* or it may be because of the solar assimilation rule */
			else if (p->arabic.solar_assimilation
					&& current_token[pos_in_current_token]==AR_SHADDA && pos_in_inflected==1
					&& is_solar(inflected[0])
					&& was_Al_before(current_token,pos_in_current_token-1,p->arabic)) {
				explore_dic_in_morpho_mode_arabic(p, d, old_offset,
						current_token, inflected, pos_in_current_token + 1,
						pos_in_inflected, pos_offset, matches, pattern,
						save_dic_entry, line_buffer,NOTHING_EXPECTED,last_dic_char, ustr, base);
			}
			/* Or it may be because of the lunar assimilation rule */
			else if (p->arabic.lunar_assimilation
					&& pos_in_inflected==0
					&& is_lunar(c)
					&& current_token[pos_in_current_token]==AR_SUKUN
					&& was_Al_before(current_token,pos_in_current_token,p->arabic)) {
				explore_dic_in_morpho_mode_arabic(p, d, old_offset,
						current_token, inflected, pos_in_current_token + 1,
						pos_in_inflected, pos_offset, matches, pattern,
						save_dic_entry, line_buffer,NOTHING_EXPECTED,last_dic_char,ustr, base);
			}
			/* Or it may be because of the Yc => e rule at the end of a word, when
			 * we are on the Y */
			else if (p->arabic.alef_maqsura_hamza_equiv_hamza_above_yeh
					&& current_token[pos_in_current_token]==AR_YEH_WITH_HAMZA_ABOVE
					&& c==AR_ALEF_MAQSURA) {
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_arabic(p, d, adr,
						current_token, inflected, pos_in_current_token,
						pos_in_inflected+1, pos_offset, matches, pattern,
						save_dic_entry, line_buffer,NOTHING_EXPECTED,last_dic_char,ustr, base);
			}
			/* Or it may be because of the Yc => e rule at the end of a word, when
			 * we are on the c */
			else if (p->arabic.alef_maqsura_hamza_equiv_hamza_above_yeh
					&& current_token[pos_in_current_token]==AR_YEH_WITH_HAMZA_ABOVE
					&& c==AR_HAMZA
					&& last_dic_char==AR_ALEF_MAQSURA) {
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_arabic(p, d, adr,
						current_token, inflected, pos_in_current_token,
						pos_in_inflected+1, pos_offset, matches, pattern,
						save_dic_entry, line_buffer,END_OF_WORD_EXPECTED,last_dic_char,ustr, base);
			}
		}
		/* Rule that always applies about tatweel */
		if (c != AR_TATWEEL && current_token[pos_in_current_token]
				== AR_TATWEEL) {
			explore_dic_in_morpho_mode_arabic(p, d, old_offset, current_token,
					inflected, pos_in_current_token + 1, pos_in_inflected,
					pos_offset, matches, pattern, save_dic_entry, line_buffer,
					NOTHING_EXPECTED, c, ustr, base);
		}
		if (p->arabic.fatha_omission && c == AR_FATHA
				&& current_token[pos_in_current_token] != AR_FATHA
				&& last_dic_char != AR_SHADDA) {
			inflected[pos_in_inflected] = c;
			explore_dic_in_morpho_mode_arabic(p, d, adr, current_token,
					inflected, pos_in_current_token, pos_in_inflected + 1,
					pos_offset, matches, pattern, save_dic_entry, line_buffer,
					NOTHING_EXPECTED, c, ustr, base);
		}
		if (p->arabic.damma_omission && c == AR_DAMMA
				&& current_token[pos_in_current_token] != AR_DAMMA
				&& last_dic_char != AR_SHADDA) {
			inflected[pos_in_inflected] = c;
			explore_dic_in_morpho_mode_arabic(p, d, adr, current_token,
					inflected, pos_in_current_token, pos_in_inflected + 1,
					pos_offset, matches, pattern, save_dic_entry, line_buffer,
					NOTHING_EXPECTED, c, ustr, base);
		}
		if (p->arabic.kasra_omission && c == AR_KASRA
				&& current_token[pos_in_current_token] != AR_KASRA
				&& last_dic_char != AR_SHADDA) {
			inflected[pos_in_inflected] = c;
			explore_dic_in_morpho_mode_arabic(p, d, adr, current_token,
					inflected, pos_in_current_token, pos_in_inflected + 1,
					pos_offset, matches, pattern, save_dic_entry, line_buffer,
					NOTHING_EXPECTED, c, ustr, base);
		}
		if (p->arabic.sukun_omission && c == AR_SUKUN
				&& current_token[pos_in_current_token] != AR_SUKUN) {
			inflected[pos_in_inflected] = c;
			explore_dic_in_morpho_mode_arabic(p, d, adr, current_token,
					inflected, pos_in_current_token, pos_in_inflected + 1,
					pos_offset, matches, pattern, save_dic_entry, line_buffer,
					NOTHING_EXPECTED, c, ustr, base);
		}
		if (p->arabic.superscript_alef_omission && c == AR_SUPERSCRIPT_ALEF
				&& current_token[pos_in_current_token] != AR_SUPERSCRIPT_ALEF
				&& last_dic_char != AR_SHADDA) {
			inflected[pos_in_inflected] = c;
			explore_dic_in_morpho_mode_arabic(p, d, adr, current_token,
					inflected, pos_in_current_token, pos_in_inflected + 1,
					pos_offset, matches, pattern, save_dic_entry, line_buffer,
					NOTHING_EXPECTED, c, ustr, base);
		}
		if (p->arabic.fathatan_omission_at_end && c == AR_FATHATAN
				&& current_token[pos_in_current_token] != AR_FATHATAN
				&& last_dic_char != AR_SHADDA) {
			inflected[pos_in_inflected] = c;
			explore_dic_in_morpho_mode_arabic(p, d, adr, current_token,
					inflected, pos_in_current_token, pos_in_inflected + 1,
					pos_offset, matches, pattern, save_dic_entry, line_buffer,
					END_OF_WORD_EXPECTED, c, ustr, base);
		}
		if (p->arabic.dammatan_omission_at_end && c == AR_DAMMATAN
				&& current_token[pos_in_current_token] != AR_DAMMATAN
				&& last_dic_char != AR_SHADDA) {
			inflected[pos_in_inflected] = c;
			explore_dic_in_morpho_mode_arabic(p, d, adr, current_token,
					inflected, pos_in_current_token, pos_in_inflected + 1,
					pos_offset, matches, pattern, save_dic_entry, line_buffer,
					END_OF_WORD_EXPECTED, c, ustr, base);
		}
		if (p->arabic.kasratan_omission_at_end && c == AR_KASRATAN
				&& current_token[pos_in_current_token] != AR_KASRATAN
				&& last_dic_char != AR_SHADDA) {
			inflected[pos_in_inflected] = c;
			explore_dic_in_morpho_mode_arabic(p, d, adr, current_token,
					inflected, pos_in_current_token, pos_in_inflected + 1,
					pos_offset, matches, pattern, save_dic_entry, line_buffer,
					END_OF_WORD_EXPECTED, c, ustr, base);
		}
		/* If the dictionary contains a shadda that we may be allowed to skip */
		if (c == AR_SHADDA && current_token[pos_in_current_token] != AR_SHADDA
				&& shadda_may_be_omitted(p->arabic)) {
			inflected[pos_in_inflected] = c;
			explore_dic_in_morpho_mode_arabic(p, d, adr, current_token,
					inflected, pos_in_current_token, pos_in_inflected + 1,
					pos_offset, matches, pattern, save_dic_entry, line_buffer,
					NOTHING_EXPECTED, c, ustr, base);
		}
		/* If the dictionary contains a shadda X, and the text contains either only shadda or nothing.
		 * The test (c!=current_token[pos_in_current_token]) is used to avoid
		 * duplicate calculus with the normal case */
		if (last_dic_char==AR_SHADDA
				/* The rules are the same regardless if the previous token char was a shadda or not,
				 * so the following test must not be performed. We keep it in comment to
				 * remind that it is pure luck that we can factorize two cases by skipping it
				 *
				 * && (pos_in_current_token>0 && current_token[pos_in_current_token-1]==AR_SHADDA)*/
				&& c!=current_token[pos_in_current_token]) {
			int can_go_on=0;
			int next=NOTHING_EXPECTED;
			switch(c) {
			case AR_FATHA: {
				if (p->arabic.shadda_fatha_omission || p->arabic.shadda_fatha_omission_at_end) can_go_on=1;
				break;
			}
			case AR_FATHATAN: {
				if (p->arabic.shadda_fathatan_omission_at_end) {
					can_go_on=1;
					/* I know a simple affectation could work, but I prefer doing it
					 * the general case to prevent bugs if one day, several flags can co-occur */
					next=next | END_OF_WORD_EXPECTED;
				}
				break;
			}
			case AR_DAMMA: {
				if (p->arabic.shadda_damma_omission || p->arabic.shadda_damma_omission_at_end) can_go_on=1;
				break;
			}
			case AR_DAMMATAN: {
				if (p->arabic.shadda_dammatan_omission_at_end) {
					can_go_on=1;
					next=next | END_OF_WORD_EXPECTED;
				}
				break;
			}
			case AR_KASRA: {
				if (p->arabic.shadda_kasra_omission || p->arabic.shadda_kasra_omission_at_end) can_go_on=1;
				break;
			}
			case AR_KASRATAN: {
				if (p->arabic.shadda_kasratan_omission_at_end) {
					can_go_on=1;
					next=next | END_OF_WORD_EXPECTED;
				}
				break;
			}
			case AR_SUPERSCRIPT_ALEF: {
				if (p->arabic.shadda_superscript_alef_omission) can_go_on=1;
				break;
			}
			default: break; /*fatal_error("Invalid switch value #2 %C in explore_dic_in_morpho_mode_arabic!\n",c);*/
			}
			if (can_go_on) {
				inflected[pos_in_inflected] = c;
				explore_dic_in_morpho_mode_arabic(p, d, adr, current_token,
						inflected, pos_in_current_token, pos_in_inflected + 1,
						pos_offset, matches, pattern, save_dic_entry, line_buffer,
						next, c, ustr, base);
			}
		}
		restore_output(z,ustr);
	}
}

/**
 * This function tries to find something in p's morphological dictionary that
 * matches the text content as well as the given pattern. If 'pattern' is NULL,
 * anything found in the dictionary will do. It is used to represent the <DIC>
 * pattern.
 */
static void explore_dic_in_morpho_mode(struct locate_parameters* p, int pos,
		int pos_in_token, struct parsing_info* *matches,
		struct pattern* pattern, int save_dic_entry, unichar* jamo,
		int pos_in_jamo) {
	unichar* buffer_line_buffer_inflected;
	buffer_line_buffer_inflected = (unichar*) malloc(sizeof(unichar) * (4096
			+ DIC_LINE_SIZE));
	if (buffer_line_buffer_inflected == NULL) {
		fatal_alloc_error("explore_dic_in_morpho_mode");
	}
	unichar* inflected = buffer_line_buffer_inflected;
	Ustring* line_buffer=new_Ustring(4096);
	Ustring* ustr=new_Ustring();
	for (int i = 0; i < p->n_morpho_dics; i++) {
		if (p->morpho_dic[i] != NULL) {
			/* Can't match anything in an empty dictionary */
			if (p->arabic.rules_enabled) {
				explore_dic_in_morpho_mode_arabic(p, p->morpho_dic[i],
						p->morpho_dic[i]->initial_state_offset,
						p->tokens->value[p->buffer[p->current_origin + pos]],
						inflected, pos_in_token, 0, pos, matches, pattern,
						save_dic_entry, line_buffer, NOTHING_EXPECTED, '\0', ustr,0);
			} else {
				explore_dic_in_morpho_mode_standard(p, p->morpho_dic[i],
						p->morpho_dic[i]->initial_state_offset,
						p->tokens->value[p->buffer[p->current_origin + pos]],
						inflected, pos_in_token, 0, pos, matches, pattern,
						save_dic_entry, jamo, pos_in_jamo, line_buffer, ustr, 0);
			}
		}
	}
	free_Ustring(ustr);
	free_Ustring(line_buffer);
	free(buffer_line_buffer_inflected);
}

} // namespace unitex
