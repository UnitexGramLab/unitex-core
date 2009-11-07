 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
#include "Buffer.h"
#include "Transitions.h"
#include "Alphabet.h"
#include "TransductionStack.h"
#include "DicVariables.h"
#include "ParsingInfo.h"

void morphological_locate(int,int,int,int,int,struct parsing_info**,int,struct list_int*,
		                  struct locate_parameters*,struct Token_error_ctx*,unichar*,int,
		                  unichar*);
void enter_morphological_mode(int,int,int,int,struct parsing_info**,int,struct list_int*,struct locate_parameters*,struct Token_error_ctx*);
int input_is_token(Fst2Tag tag);
void explore_dic_in_morpho_mode(struct locate_parameters* p,int pos,int pos_in_token,
                                struct parsing_info* *matches,struct pattern* pattern,
                                int save_dela_entry,unichar* jamo,int pos_in_jamo);
int is_entry_compatible_with_pattern(struct dela_entry* entry,struct pattern* pattern);


/**
 * Stores in 'content' the string corresponding to the given range.
 */
void get_content(unichar* content,struct locate_parameters* p,int pos,int pos_in_token,int pos2,int pos_in_token2) {
int i=0;
unichar* current_token=p->tokens->value[p->buffer[pos+p->current_origin]];
while (!(pos==pos2 && pos_in_token==pos_in_token2)) {
   content[i++]=current_token[pos_in_token++];
   if (current_token[pos_in_token]=='\0' && !(pos==pos2 && pos_in_token==pos_in_token2)) {
      pos++;
      pos_in_token=0;
      int token_number=p->buffer[pos+p->current_origin];
      if (token_number==-1 || token_number==p->STOP) {
         fatal_error("Unexpected end of array or {STOP} tag in get_content\n");
      }
      current_token=p->tokens->value[token_number];
   }
}
content[i]='\0';
}



/**
 * Returns 1 if the given string is of the form $XXX$ where XXX is a valid
 * variable name; 0 otherwise.
 */
int is_morpho_variable_output(unichar* s,unichar* var_name) {
if (s==NULL || s[0]!='$') return 0;
int i,j=0;
for (i=1;s[i]!='$' && s[i]!='\0';i++) {
   if (!is_variable_char(s[i])) return 0;
   var_name[j++]=s[i];
}
var_name[j]='\0';
return (i!=1 && s[i]=='$' && s[i+1]=='\0');
}


/**
 * Tries to match all the given tag token against the given jamo sequence. Return values:
 * 0=failed
 * 1=the tag matches the whole jamo sequence (i.e. we will have to go on the next text token
 * 2=the tag matches a part of the jamo sequence
 */
int get_jamo_longest_prefix(unichar* jamo,int *new_pos_in_jamo,int *new_pos_in_token,unichar* tag_token,
		struct locate_parameters* p,unichar* token) {
unichar tmp[128];
if (tag_token[0]==KR_SYLLAB_BOUND && tag_token[1]=='\0') {
   u_strcpy(tmp,tag_token);
} else {
   convert_Korean_text(tag_token,tmp,p->jamo,p->alphabet);
}
int i=0;
if (token==NULL) {
	token=tag_token;
}
set_debug(0 && token[0]==0xB2A5);
debug("on compare text=_%S/%S_ et tag=_%S/%S\n",token,jamo+(*new_pos_in_jamo),tag_token,tmp);
while (tmp[i]!='\0' && jamo[(*new_pos_in_jamo)]!='\0') {
	/* We ignore syllab bounds in both tfst and fst2 tags */
	if (tmp[i]==KR_SYLLAB_BOUND) {
		i++;
		debug("ignoring . in tag: %S\n",tmp+i);
		continue;
	}
	/*if (jamo[(*new_pos_in_jamo)]!=KR_SYLLAB_BOUND && !u_is_Hangul_Jamo(jamo[(*new_pos_in_jamo)])) {
		// If we have a non jamo character, then we have to go on in the text token
		(*new_pos_in_token)++;
		debug("new pos in token=%d\n",(*new_pos_in_token));
	}*/
	if (jamo[(*new_pos_in_jamo)]==KR_SYLLAB_BOUND) {
		(*new_pos_in_jamo)++;
		(*new_pos_in_token)++;
		debug("ignoring . in text: %S\n",jamo+((*new_pos_in_jamo)));
		continue;
	}
	if (tmp[i]!=jamo[(*new_pos_in_jamo)]) {
	   /* If a character doesn't match */
	   debug("match failed between text=%S and fst2=%S\n",jamo,tmp);
	   return 0;
	}
	i++;
	(*new_pos_in_jamo)++;
	debug("moving in tag: %S\n",tmp+i);
	debug("moving in text: %S\n",jamo+((*new_pos_in_jamo)));
}
if (tmp[i]=='\0' && jamo[(*new_pos_in_jamo)]=='\0') {
   /* If we are at both ends of strings, it's a full match */
   debug("XX full match between text=%S and fst2=%S\n",jamo,tmp);
   return 1;
}
if (tmp[i]=='\0') {
	/* If the tag has not consumed all the jamo sequence, it's a partial match */
   debug("XX partial match between text=%S and fst2=%S\n",jamo,tmp);
	return 2;
}
/* If we are at the end of the jamo sequence, but not at the end of the tag, it's a failure */
debug("match failed #2 between text=%S and fst2=%S\n",jamo,tmp);
set_debug(0);
return 0;
}



/**
 * This is the core function of the morphological mode.
 */
void morphological_locate(int graph_depth, /* 0 means that we are in the top level graph */
            int current_state_index, /* current state in the grammar */
            int pos, /* position in the token buffer, relative to the current origin */
            int pos_in_token, /* position in the token in characters */
            int depth, /* number of nested calls to 'locate' */
            struct parsing_info** matches, /* current match list. Irrelevant if graph_depth==0 */
            int n_matches, /* number of sequences that have matched. It may be different from
                            * the length of the 'matches' list if a given sequence can be
                            * matched in several ways. It is used to detect combinatory
                            * explosions due to bad written grammars. */
            struct list_int* ctx, /* information about the current context, if any */
            struct locate_parameters* p, /* miscellaneous parameters needed by the function */
            struct Token_error_ctx* p_token_error_ctx,
            unichar* jamo, int pos_in_jamo,
            unichar* content_buffer /* reusable unichar 4096 buffer for content */
            ) {
OptimizedFst2State current_state=p->optimized_states[current_state_index];
Fst2State current_state_old=p->fst2->states[current_state_index];
int token;
Transition* t;
int stack_top=p->stack->stack_pointer;
/* The following static variable holds the number of matches at
 * one position in text. */
if (depth==0) {
   /* We reset if this is first call to 'locate' from a given position in the text */
   p_token_error_ctx->n_matches_at_token_pos__morphological_locate=0;
}
if (depth>STACK_MAX) {
   /* If there are too much recursive calls */
   error_at_token_pos("\nMaximal stack size reached!\n"
                      "(There may be longer matches not recognized!)",
                      p->current_origin,pos,p,p_token_error_ctx);
   return;
}
if ((p_token_error_ctx->n_matches_at_token_pos__morphological_locate) > MAX_MATCHES_AT_TOKEN_POS) {
   /* If there are too much matches from the current origin in the text */
   error_at_token_pos("\nToo many (ambiguous) matches starting from one position in text!",
                      p->current_origin,pos,p,p_token_error_ctx);
   return;
}
if (current_state->control & 1) {
   /* If we are in a final state... */
   /* In we are in the top level graph, it's an error, since we are not supposed
    * to reach the end of the graph (we should find a $>) */
   if (graph_depth==0) {
      fatal_error("Unexpected end of graph in morphological mode!\n");
   }
   else {
      /* If we are in a subgraph */
      if (n_matches==MAX_MATCHES_PER_SUBGRAPH) {
         /* If there are too much matches, we suspect an error in the grammar
          * like an infinite recursion */
         error_at_token_pos("\nMaximal number of matches per subgraph reached!",
                            p->current_origin,pos,p,p_token_error_ctx);
         return;
      }
      else {
         /* If everything is fine, we add this match to the match list of the
          * current graph level */
         n_matches++;
         p->stack->stack[stack_top+1]='\0';
         if (p->ambiguous_output_policy==ALLOW_AMBIGUOUS_OUTPUTS) {
            (*matches)=insert_if_different(pos,pos_in_token,-1,(*matches),p->stack->stack_pointer,
            		                       &(p->stack->stack[p->stack_base+1]),p->variables,
            		                       p->dic_variables,-1,-1,jamo,pos_in_jamo);
         } else {
            (*matches)=insert_if_absent(pos,pos_in_token,-1,(*matches),p->stack->stack_pointer,
            		                    &(p->stack->stack[p->stack_base+1]),p->variables,
            		                    p->dic_variables,-1,-1,jamo,pos_in_jamo);
         }
      }
   }
}

/* If we have reached the end of the token buffer, we indicate it by setting
 * the current tokens to -1 */
if (pos+p->current_origin>=p->token_buffer->size) {
   token=-1;
} else {
   token=p->buffer[pos+p->current_origin];
}
unichar* current_token;
int current_token_length=0;
if (token==-1 || token==p->STOP) {
   current_token=NULL;
   jamo=NULL;
} else {
   current_token=p->tokens->value[p->buffer[p->current_origin+pos]];
   current_token_length=u_strlen(current_token);
}

/**
 * SUBGRAPHS
 */
struct opt_graph_call* graph_call_list=current_state->graph_calls;
if (graph_call_list!=NULL) {
   /* If there are subgraphs, we process them */
   int old_StackBase;
   old_StackBase=p->stack_base;
   /* In morphological mode, we cannot modify variables because $xxx( and $xxx) tags are
    * not allowed. However, we can have to modify DELAF entry variables */
   struct dic_variable* dic_variables_backup=clone_dic_variable_list(p->dic_variables);
   do {
      /* For each graph call, we look all the reachable states */
      t=graph_call_list->transition;
      while (t!=NULL) {
         struct parsing_info* L=NULL;
         p->stack_base=p->stack->stack_pointer;
         morphological_locate(graph_depth+1, /* Exploration of the subgraph */
                p->fst2->initial_states[graph_call_list->graph_number],
                pos,pos_in_token,depth+1,&L,0,NULL,p,p_token_error_ctx,jamo,pos_in_jamo,content_buffer);
         p->stack_base=old_StackBase;
         if (L!=NULL) {
            struct parsing_info* L_first=L;
            /* If there is at least one match, we process the match list */
            do  {
               /* We restore the settings of the current graph level */
               if (p->output_policy!=IGNORE_OUTPUTS) {
                  u_strcpy(&(p->stack->stack[stack_top+1]),L->stack);
                  p->stack->stack_pointer=L->stack_pointer;
                  p->dic_variables=L->dic_variable_backup;
               }
               /* And we continue the exploration */
               morphological_locate(graph_depth,t->state_number,L->position,L->pos_in_token,depth+1,
            		                matches,n_matches,ctx,p,p_token_error_ctx,L->jamo,L->pos_in_jamo,content_buffer);
               p->stack->stack_pointer=stack_top;
               L=L->next;
            } while (L!=NULL);
            /* We free all subgraph matches */
            free_parsing_info(L_first);
         }
         p->dic_variables=clone_dic_variable_list(dic_variables_backup);
         t=t->next;
      } /* end of while (t!=NULL) */
   } while ((graph_call_list=graph_call_list->next)!=NULL);
   /* Finally, we have to restore the stack and other backup stuff */
   p->stack->stack_pointer=stack_top;
   p->stack_base=old_StackBase; /* May be changed by recursive subgraph calls */

   /* NOTE: we don't have to take care of normal variables, since they cannot be
    *       modified in morphological mode */
   clear_dic_variable_list(&dic_variables_backup);
} /* End of processing subgraphs */


/**
 * METAS
 */
struct opt_meta* meta_list=current_state->metas;
while (meta_list!=NULL) {
   /* We process all the meta of the list */
   t=meta_list->transition;
   int match_one_letter;
   while (t!=NULL) {
      match_one_letter=0;
      switch (meta_list->meta) {

         case META_EPSILON:
            /* We could have an output associated to an epsilon, so we handle this case */
            if (p->output_policy!=IGNORE_OUTPUTS) {
               if (!process_output(p->tags[t->tag_number]->output,p)) {
                  break;
               }
            }
            morphological_locate(graph_depth,meta_list->transition->state_number,pos,pos_in_token,depth+1,
                                 matches,n_matches,ctx,p,p_token_error_ctx,jamo,pos_in_jamo,content_buffer);
            p->stack->stack_pointer=stack_top;
            break;

         case META_SHARP:
            fatal_error("Unexpected # tag in morphological mode\n");
            break;

         case META_SPACE:
            /* This case will be handled in the token case (see below) */
            break;

         case META_MOT: /* In morphological mode, this tag matches a letter, as defined in
                           the alphabet file */
            if (token==-1 || token==p->STOP) {break;}
            if (XOR(is_letter(current_token[pos_in_token],p->alphabet),meta_list->negation)) {
               match_one_letter=1;
            }
            break;

         case META_DIC: {
#ifdef __GNUC__
#warning todo
#elif ((defined(__VISUALC__)) || defined(_MSC_VER))
#pragma message("#warning todo")
#endif
            if (token==-1 || token==p->STOP) {break;}
            struct parsing_info* L2=NULL;
            explore_dic_in_morpho_mode(p,pos,pos_in_token,&L2,NULL,0,jamo,pos_in_jamo);
            unichar *content1=content_buffer;
            if (L2!=NULL) {
               struct parsing_info* L_first=L2;
               /* If there is at least one match, we process the match list */
               do  {
                  get_content(content1,p,pos,pos_in_token,L2->position,L2->pos_in_token);
                  #ifdef TRE_WCHAR
                  int filter_number=p->tags[t->tag_number]->filter_number;
                  int morpho_filter_OK=(filter_number==-1 || string_match_filter(p->filters,content1,filter_number));
                  if (!morpho_filter_OK) {
                     p->stack->stack_pointer=stack_top;
                     L2=L2->next;
                     continue;
                  }
                  #endif
                  if (p->output_policy!=IGNORE_OUTPUTS) {
                     if (!process_output(p->tags[t->tag_number]->output,p)) {
                        break;
                     }
                  }
                  if (p->output_policy==MERGE_OUTPUTS) {
                     push_input_string(p->stack,content1,p->protect_dic_chars);
                  }
                  unichar* reached_token=p->tokens->value[p->buffer[p->current_origin+L2->position]];
                  int new_pos,new_pos_in_token;
                  if (reached_token[L2->pos_in_token]=='\0') {
                     /* If we are at the end of the last token matched by the <DIC> tag */
#ifdef __GNUC__
#warning a modifier
#elif ((defined(__VISUALC__)) || defined(_MSC_VER))
#pragma message("#warning a modifier")
#endif
                     new_pos=L2->position+1;
                     new_pos_in_token=0;
                  } else {
                     /* If not */
                     new_pos=L2->position;
                     new_pos_in_token=L2->pos_in_token;
                  }
                  /* We continue the exploration */
                  morphological_locate(graph_depth,meta_list->transition->state_number,
                		               new_pos,new_pos_in_token,depth+1,matches,n_matches,ctx,
                		               p,p_token_error_ctx,
                		               L2->jamo,L2->pos_in_jamo,content_buffer);
                  p->stack->stack_pointer=stack_top;
                  L2=L2->next;
               } while (L2!=NULL);
               free_parsing_info(L_first);
            }
            /* end of usage of content1 */
            break;
         }

         case META_SDIC:
            fatal_error("Unexpected <SDIC> tag in morphological mode\n");
            break;

         case META_CDIC:
            fatal_error("Unexpected <CDIC> tag in morphological mode\n");
            break;

         case META_MAJ: /* In morphological mode, this tag matches an uppercase letter, as defined in
                           the alphabet file */
            if (token==-1 || token==p->STOP) {break;}
            if (XOR(is_upper(current_token[pos_in_token],p->alphabet),meta_list->negation)) {
               match_one_letter=1;
            }
            break;

         case META_MIN: /* In morphological mode, this tag matches a lowercase letter, as defined in
                           the alphabet file */
            if (token==-1 || token==p->STOP) {break;}
            if (XOR(is_lower(current_token[pos_in_token],p->alphabet),meta_list->negation)) {
               match_one_letter=1;
            }
            break;

         case META_PRE:
            fatal_error("Unexpected <PRE> tag in morphological mode\n");
            break;

         case META_NB:
            fatal_error("Unexpected <NB> tag in morphological mode\n");
            break;

         case META_TOKEN: {
            unichar one_letter[2];
            one_letter[0]=current_token[pos_in_token];
            one_letter[1]='\0';
            #ifdef TRE_WCHAR
            int filter_number=p->tags[t->tag_number]->filter_number;
            int morpho_filter_OK=(filter_number==-1 || string_match_filter(p->filters,one_letter,filter_number));
            if (morpho_filter_OK) {
               match_one_letter=1;
            }
            #endif
            break;
         }

         case META_LEFT_CONTEXT:
            fatal_error("Unexpected left context mark in morphological mode\n");
            break;

         case META_BEGIN_MORPHO:
            fatal_error("Unexpected morphological mode begin tag $<\n");
            break;

         case META_END_MORPHO:
            /* Should happen, but only at the same level than the $< tag */
            if (graph_depth!=0) {
               fatal_error("Unexpected end of morphological mode at a different\nlevel than the $< tag\n");
            }
            if (pos_in_token!=0 || (jamo!=NULL && pos_in_jamo!=0)) {
               /* If the end of the morphological mode occurs in the middle
                * of a token, we don't take this "match" into account */
               break;
            }
            /* If the end of the morphological mode occurs at the beginning of a token,
             * then we have a match. We note the state number that corresponds to the state
             * that follows the current $> tag transition, so that we know where to continue
             * the exploration in the 'enter_morphological_mode' function. */
            p->stack->stack[stack_top+1]='\0';
            if (p->ambiguous_output_policy==ALLOW_AMBIGUOUS_OUTPUTS) {
               (*matches)=insert_if_different(pos,pos_in_token,t->state_number,(*matches),p->stack->stack_pointer,
            		                          &(p->stack->stack[p->stack_base+1]),p->variables,
            		                          p->dic_variables,-1,-1,jamo,pos_in_jamo);
            } else {
               (*matches)=insert_if_absent(pos,pos_in_token,t->state_number,(*matches),p->stack->stack_pointer,
            		                       &(p->stack->stack[p->stack_base+1]),p->variables,
            		                       p->dic_variables,-1,-1,jamo,pos_in_jamo);
            }
            break;

      } /* End of the switch */
      if (match_one_letter) {
         /* We factorize here the cases <MOT>, <MIN> and <MAJ> that all correspond
          * to matching one letter */
         if (p->output_policy!=IGNORE_OUTPUTS) {
            if (!process_output(p->tags[t->tag_number]->output,p)) {
               goto next;
            }
         }
         if (p->output_policy==MERGE_OUTPUTS) {
            /* If needed, we push the character that was matched */
            push_input_char(p->stack,current_token[pos_in_token],p->protect_dic_chars);
         }
         int new_pos;
         int new_pos_in_token;
         unichar* new_jamo=NULL;
         int new_pos_in_jamo=0;
         if (pos_in_token+1==current_token_length) {
            /* If we are at the end of the current token */
            new_pos=pos+1;
            new_pos_in_token=0;
            /* We also update the Jamo things */
			   new_jamo=NULL;
			   if (p->jamo_tags != NULL) {
			      new_jamo=p->jamo_tags[p->buffer[new_pos+p->current_origin]];
			   }
            new_pos_in_jamo=0;
         } else {
            /* If not */
            new_pos=pos;
            new_pos_in_token=pos_in_token+1;
            new_jamo=jamo;
            /* If we are in Korean mode, we have to move in the jamo sequence */
            if (jamo!=NULL) {
               new_pos_in_jamo=pos_in_jamo+1;
               while (jamo[new_pos_in_jamo]!='\0') {
            	   if (jamo[new_pos_in_jamo]==KR_SYLLAB_BOUND) {
            		   /* A syllab bound is OK: we go on the following Jamo */
               		new_pos_in_jamo++;
               		if (!u_is_Hangul_Jamo(jamo[new_pos_in_jamo])) {
               			fatal_error("Unexpected non jamo character after a syllab bound\n");
            	   	}
            		   break;
            	   }
            	   if (u_is_korea_syllabe_letter(jamo[new_pos_in_jamo])) {
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
               if (jamo[new_pos_in_jamo]!='\0') {
            	   fatal_error("Unexpected end of Jamo sequence\n");
               }
            }
         }
         morphological_locate(graph_depth,t->state_number,new_pos,new_pos_in_token,depth+1,
                                 matches,n_matches,ctx,p,p_token_error_ctx,new_jamo,new_pos_in_jamo,content_buffer);
         p->stack->stack_pointer=stack_top;
      }
      next: t=t->next;
   }
   meta_list=meta_list->next;
}

if (token==-1 || token==p->STOP) {
   /* We can't match anything if we are at the end of the buffer or if
    * we have the forbidden token {STOP} that must never be matched. */
   return;
}


/**
 * VARIABLE STARTS
 */
struct opt_variable* variable_list=current_state->variable_starts;
if (variable_list!=NULL) {
   fatal_error("Unexpected use of variable in morphological mode\n");
}

/**
 * VARIABLE ENDS
 */
variable_list=current_state->variable_ends;
if (variable_list!=NULL) {
   fatal_error("Unexpected use of variable in morphological mode\n");
}

/**
 * CONTEXTS
 */
struct opt_contexts* contexts=current_state->contexts;
if (contexts!=NULL) {
   fatal_error("Unexpected use of contexts in morphological mode\n");
}

/**
 * TOKENS
 */
Transition* trans=current_state_old->transitions;
while (trans!=NULL) {
   if (trans->tag_number<0) {
      /* We don't take subgraph transitions into account */
      trans=trans->next;
      continue;
   }
   Fst2Tag tag=p->tags[trans->tag_number];
   if (tag->pattern!=NULL) {
      if (tag->pattern->type==TOKEN_PATTERN) {
         unichar* tag_token=tag->pattern->inflected;
         int comma=-1;
         if (tag_token[0]=='{' && u_strcmp(tag_token,"{") && u_strcmp(tag_token,"{S}")) {
        	 /* If we have a tag like {eats,eat.V:P3s} */
        	 tag_token++; /* We ignore the { */
        	 while (tag_token[comma]!=',') {
        		 comma++;
        	 }
        	 /* We want to avoid a copy */
        	 tag_token[comma]='\0';
         }

         int tag_token_length=u_strlen(tag_token);
         if (jamo!=NULL) {
        	/* KOREAN token matching */
            int new_pos_in_jamo=pos_in_jamo;
            int new_pos_in_token=pos_in_token;
            int result=get_jamo_longest_prefix(jamo,&new_pos_in_jamo,&new_pos_in_token,tag_token,p,current_token);
            if (comma!=-1) {
            	/* If necessary, we restore the tag */
            	tag_token[comma]=',';
            }
            if (result!=0) {
            	/* Nothing to do if the match failed */
            	int new_pos=pos;
            	unichar* new_jamo=jamo;
            	if (result==1) {
            		/* The text token has been fully matched, so we go on the next one */
            		new_pos_in_token=0;
            		new_pos=pos+1;
            	    new_jamo=p->jamo_tags[p->buffer[new_pos+p->current_origin]];
            	    new_pos_in_jamo=0;
                }
            	/* If we can match the tag's token, we process the output, if we have to */
            	if (p->output_policy!=IGNORE_OUTPUTS) {
            	    if (!process_output(tag->output,p)) {
            	        continue;
            	    }
            	}
            	if (p->output_policy==MERGE_OUTPUTS) {
            		fatal_error("Unsupported MERGE mode in Korean morphological mode\n");
            	    //push_input_substring(p->stack,current_token+pos_in_token,prefix_length,p->protect_dic_chars);
            	}
            	morphological_locate(graph_depth,trans->state_number,new_pos,new_pos_in_token,depth+1,
            	                                    matches,n_matches,ctx,p,p_token_error_ctx,new_jamo,new_pos_in_jamo,content_buffer);
            	p->stack->stack_pointer=stack_top;
            }
         	/* End of KOREAN token matching */
         } else {
            /* Non Korean token matching*/
        	int prefix_length;
            if (tag->control&RESPECT_CASE_TAG_BIT_MASK) {
               /* If we must have a perfect match */
               prefix_length=get_longuest_prefix(current_token+pos_in_token,tag_token);
            } else {
               prefix_length=get_longuest_prefix_ignoring_case(current_token+pos_in_token,tag_token,p->alphabet);
            }
            if (prefix_length==tag_token_length) {
               /* If we can match the tag's token, we process the output, if we have to */
               if (p->output_policy!=IGNORE_OUTPUTS) {
                  if (!process_output(tag->output,p)) {
                     continue;
                  }
               }
               if (p->output_policy==MERGE_OUTPUTS) {
                  push_input_substring(p->stack,current_token+pos_in_token,prefix_length,p->protect_dic_chars);
               }
               int new_pos,new_pos_in_token;
               if (pos_in_token+prefix_length<current_token_length) {
                  /* If we didn't reach the end of the current token */
                  new_pos=pos;
                  new_pos_in_token=pos_in_token+prefix_length;
               } else {
                  /* If we must go on the next token */
                  new_pos=pos+1;
                  new_pos_in_token=0;
               }
               morphological_locate(graph_depth,trans->state_number,new_pos,new_pos_in_token,depth+1,
                                    matches,n_matches,ctx,p,p_token_error_ctx,jamo,pos_in_jamo,content_buffer);
               p->stack->stack_pointer=stack_top;
            } else {
           	   /* No else here, because a grammar tag is not supposed to match a sequence that
        	    * overlaps two text tokens. */
            }
            /* End of non Korean token matching */
         }
      } else {
         /* Here, we deal with all the "real" patterns: <be>, <N+z1:ms>, <be.V:K> and <am,be.V> */
         struct parsing_info* L=NULL;
         unichar var_name[128];
         int save_dic_entry=(p->output_policy!=IGNORE_OUTPUTS && is_morpho_variable_output(tag->output,var_name));
         explore_dic_in_morpho_mode(p,pos,pos_in_token,&L,tag->pattern,save_dic_entry,jamo,pos_in_jamo);
         unichar *content2=content_buffer;
         if (L!=NULL) {
            struct parsing_info* L_first=L;
            /* If there is at least one match, we process the match list */
            do  {
               get_content(content2,p,pos,pos_in_token,L->position,L->pos_in_token);
               #ifdef TRE_WCHAR
               int filter_number=p->tags[trans->tag_number]->filter_number;
               int morpho_filter_OK=(filter_number==-1 || string_match_filter(p->filters,content2,filter_number));
               if (!morpho_filter_OK) {
                  p->stack->stack_pointer=stack_top;
                  L=L->next;
                  continue;
               }
               #endif
               /* WARNING: we don't process the tag's output as usual if it
                *          is a variable declaration like $abc$. Note that it could
                *          make a difference if a variable with the same
                *          name was declared before entering the morphological mode */
               if (!save_dic_entry && p->output_policy!=IGNORE_OUTPUTS) {
                  if (!process_output(tag->output,p)) {
                     continue;
                  }
               }
               if (p->output_policy==MERGE_OUTPUTS) {
                  push_input_string(p->stack,content2,p->protect_dic_chars);
               }
               unichar* reached_token=p->tokens->value[p->buffer[p->current_origin+L->position]];
               int new_pos,new_pos_in_token;
               if (reached_token[L->pos_in_token]=='\0') {
                  /* If we are at the end of the last token matched by the dictionary search */
                  new_pos=L->position+1;
                  new_pos_in_token=0;
               } else {
                  /* If not */
                  new_pos=L->position;
                  new_pos_in_token=L->pos_in_token;
               }
               /* We continue the exploration */
               struct dela_entry* old_value=NULL;
               if (save_dic_entry) {
                  old_value=clone_dela_entry(get_dic_variable(var_name,p->dic_variables));
                  set_dic_variable(var_name,L->dic_entry,&(p->dic_variables));
               }
               morphological_locate(graph_depth,trans->state_number,new_pos,new_pos_in_token,
            		                depth+1,matches,n_matches,ctx,p,p_token_error_ctx,L->jamo,L->pos_in_jamo,content_buffer);
               if (save_dic_entry) {
                  set_dic_variable(var_name,old_value,&(p->dic_variables));
                  free_dela_entry(old_value);
               }
               p->stack->stack_pointer=stack_top;
               L=L->next;
            } while (L!=NULL);
            free_parsing_info(L_first);
         }
         /* end of usage of content2 */
      }
   }
   trans=trans->next;
}
}



/**
 * Tests whether this tag has an input that is a token and not something like <...>.
 * Returns 1 in that case; 0 otherwise.
 */
int input_is_token(Fst2Tag tag) {
return (tag->input[0]!='<' || tag->input[1]=='\0');
}



/**
 * This is function that starts matching things in morphological mode.
 */
void enter_morphological_mode(int graph_depth, /* 0 means that we are in the top level graph */
            int state, /* current state in the grammar */
            int pos, /* position in the token buffer, relative to the current origin */
            int depth, /* number of nested calls to 'locate' */
            struct parsing_info** matches, /* current match list. Irrelevant if graph_depth==0 */
            int n_matches, /* number of sequences that have matched. It may be different from
                            * the length of the 'matches' list if a given sequence can be
                            * matched in several ways. It is used to detect combinatory
                            * explosions due to bad written grammars. */
            struct list_int* ctx, /* information about the current context, if any */
            struct locate_parameters* p, /* miscellaneous parameters needed by the function */
            struct Token_error_ctx* p_token_error_ctx
            ) {
unichar content_buffer[4096];
int* var_backup=NULL;
int old_StackBase;
int stack_top=p->stack->stack_pointer;
old_StackBase=p->stack_base;
if (p->output_policy!=IGNORE_OUTPUTS) {
   /* For better performance when ignoring outputs */
   var_backup=create_variable_backup(p->variables);
}
struct parsing_info* L=NULL;
p->stack_base=p->stack->stack_pointer;
struct dic_variable* dic_variable_backup=clone_dic_variable_list(p->dic_variables);
int current_token=p->buffer[pos+p->current_origin];
//error("current token=%d/%S  jamo=%S\n",current_token,p->tokens->value[current_token],p->jamo_tags[current_token]);
morphological_locate(0,state,pos,0,depth+1,&L,0,NULL,p,p_token_error_ctx,(p->jamo_tags!=NULL)?p->jamo_tags[current_token]:NULL,0,content_buffer);
clear_dic_variable_list(&(p->dic_variables));
p->stack_base=old_StackBase;
if (L!=NULL) {
   struct parsing_info* L_first=L;
   /* If there is at least one match, we process the match list */
   do  {
      /* We restore the settings of the current graph level */
      if (p->output_policy!=IGNORE_OUTPUTS) {
         u_strcpy(&(p->stack->stack[stack_top+1]),L->stack);
         p->stack->stack_pointer=L->stack_pointer;
         install_variable_backup(p->variables,L->variable_backup);
      }
      p->dic_variables=clone_dic_variable_list(L->dic_variable_backup);
      /* And we continue the exploration */

      int suggested_param_alloc_reserve=0;
      suggested_param_alloc_reserve = suggest_size_backup_reserve(p->variables);
      variable_backup_memory_reserve* backup_reserve = create_variable_backup_memory_reserve(suggested_param_alloc_reserve);
      locate(graph_depth,p->optimized_states[L->state_number],L->position,depth+1,matches,n_matches,ctx,p,p_token_error_ctx,backup_reserve);
      free_reserve(backup_reserve);

      p->stack->stack_pointer=stack_top;
      if (graph_depth==0) {
         /* If we are at the top graph level, we restore the variables */
         if (p->output_policy!=IGNORE_OUTPUTS) {
            install_variable_backup(p->variables,var_backup);
         }
      }
      if (ctx==NULL || L->next!=NULL) {
         /* If we are inside a context, we don't want to free all the dic_variables that
          * have been set, in order to allow extracting morphological information from contexts.
          * To do that, we arbitrarily keep the dic_variables of the last path match. */
         clear_dic_variable_list(&(p->dic_variables));
      }
      L=L->next;
   } while (L!=NULL);
   free_parsing_info(L_first); /* free all morphological matches */
}
/* Finally, we have to restore the stack and other backup stuff */
p->stack->stack_pointer=stack_top;
p->stack_base=old_StackBase; /* May be changed by recursive subgraph calls */
if (p->output_policy!=IGNORE_OUTPUTS) { /* For better performance (see above) */
   install_variable_backup(p->variables,var_backup);
   free_variable_backup(var_backup);
}
if (ctx==NULL) {
   p->dic_variables=dic_variable_backup;
} else {
   clear_dic_variable_list(&dic_variable_backup);
}
}



/**
 * Tries to find something in the dictionary that match both text content and given pattern.
 */
void explore_dic_in_morpho_mode__(struct locate_parameters* p,
                                unsigned char* bin,struct INF_codes* inf,
                                int offset,unichar* current_token,unichar* inflected,
                                int pos_in_current_token,
                                int pos_in_inflected,int pos_offset,
                                struct parsing_info* *matches,struct pattern* pattern,
                                int save_dic_entry,unichar* jamo,int pos_in_jamo) {
int n_transitions=((unsigned char)bin[offset])*256+(unsigned char)bin[offset+1];
offset=offset+2;
if (!(n_transitions & 32768)) {
	//error("\narriba!\n\n\n");
   /* If this node is final, we get the INF line number */
   inflected[pos_in_inflected]='\0';
   if (pattern==NULL) {
      /* If any word will do. Note that we don't save DELAF entries
       * for the <DIC> pattern */
      (*matches)=insert_morphological_match(pos_offset,pos_in_current_token,-1,(*matches),NULL,jamo,pos_in_jamo);
   } else {
      /* If we have to check the pattern */
      int inf_number=((unsigned char)bin[offset])*256*256+((unsigned char)bin[offset+1])*256+(unsigned char)bin[offset+2];
      unichar line[DIC_LINE_SIZE];
      struct list_ustring* tmp=inf->codes[inf_number];
      while (tmp!=NULL) {
         /* For each compressed code of the INF line, we save the corresponding
          * DELAF line in 'info->dlc' */
         uncompress_entry(inflected,tmp->string,line);
         //error("\non decompresse la ligne _%S_\n",line);
         struct dela_entry* dela_entry=tokenize_DELAF_line(line);
         if (dela_entry!=NULL && is_entry_compatible_with_pattern(dela_entry,pattern)) {
            //error("et ca matche!!\n");
            (*matches)=insert_morphological_match(pos_offset,pos_in_current_token,-1,(*matches),
            		                              save_dic_entry?dela_entry:NULL,jamo,pos_in_jamo);
         }
         free_dela_entry(dela_entry);
         tmp=tmp->next;
      }
   }
}

if (current_token[pos_in_current_token]=='\0') {
	if (jamo==NULL) {
		/* If we have reached the end of the current token in a non Korean language */
		pos_offset++;
		int token_number=p->buffer[pos_offset+p->current_origin];
		if (token_number==-1 || token_number==p->STOP) {
			/* Remember 1) that we must not be out of the array's bounds and
			 *          2) that the token {STOP} must never be matched */
			return;
		}
		current_token=p->tokens->value[token_number];
		pos_in_current_token=0;
	} else {
		/* We are in Korean mode */
		if (jamo[pos_in_jamo]=='\0') {
			/* In korean, we perform a token change only if we have used all the token's jamos */
			pos_offset++;
			int token_number=p->buffer[pos_offset+p->current_origin];
			if (token_number==-1 || token_number==p->STOP) {
				/* Remember 1) that we must not be out of the array's bounds and
				 *          2) that the token {STOP} must never be matched */
				return;
			}
			current_token=p->tokens->value[token_number];
			pos_in_current_token=0;
			pos_in_jamo=0;
			jamo=p->jamo_tags[token_number];
		}
	}
}
int after_syllab_bound=0;
if (jamo!=NULL) {
	/* We test wether we are in the middle of a syllab or just after a syllab bound */
    if (jamo[pos_in_jamo]==KR_SYLLAB_BOUND) {
    	/* If we have a syllab bound */
    	after_syllab_bound=1;
    	pos_in_jamo++;
    }
    else if (pos_in_jamo>0 && jamo[pos_in_jamo-1]==KR_SYLLAB_BOUND) {
    	/* If we are just after a syllab bound */
    	after_syllab_bound=1;
    }
    else {
    	/* By default, we must be in the middle of a syllab, and there's nothing to do */
    }
}

/* We look for outgoing transitions */
if (n_transitions & 32768) {
   /* If we are in a normal node, we remove the control bit to
    * have the good number of transitions */
   n_transitions=n_transitions-32768;
} else {
   /* If we are in a final node, we must jump after the reference to the INF line number */
   offset=offset+3;
}
for (int i=0;i<n_transitions;i++) {
   unichar c=(unichar)(((unsigned char)bin[offset])*256+(unsigned char)bin[offset+1]);
   offset=offset+2;
   int adr=((unsigned char)bin[offset])*256*256+((unsigned char)bin[offset+1])*256+(unsigned char)bin[offset+2];
   offset=offset+3;

   if (jamo==NULL) {
	   /* Non Korean mode */
	   if (is_equal_or_uppercase(c,current_token[pos_in_current_token],p->alphabet)) {
		   /* We explore the rest of the dictionary only if the
		    * dictionary char is compatible with the token char. In that case,
		    * we copy in 'inflected' the exact character that is in the dictionary. */
		   inflected[pos_in_inflected]=c;
		   explore_dic_in_morpho_mode__(p,bin,inf,adr,current_token,inflected,
				   pos_in_current_token+1,pos_in_inflected+1,
				   pos_offset,matches,pattern,save_dic_entry,
				   jamo,pos_in_jamo);
	   }
   } else {
	   debug("la: jamo du text=%C (%04X)   char du dico=%C (%04X)\n",jamo[pos_in_jamo],jamo[pos_in_jamo],c,c);
	   /* Korean mode: we may match just the current jamo, or also the current hangul, but only if we are
	    * after a syllab bound */
	   unichar c2[2];
	   c2[0]=c;
	   c2[1]='\0';
	   /* We try to match all the jamo sequence found in the dictionary */
	   int new_pos_in_current_token=pos_in_current_token;
	   int new_pos_in_jamo=pos_in_jamo;
	   int result=get_jamo_longest_prefix(jamo,&new_pos_in_jamo,&new_pos_in_current_token,c2,p,current_token);
	   if (result!=0) {
	      debug("MATCH entre jamo du text=%C (%04X)   char du dico=%C (%04X)\n",jamo[pos_in_jamo],jamo[pos_in_jamo],c,c);
		   /* Nothing to do if the match failed */
		   int new_pos_offset=pos_offset;
		   unichar* new_jamo=jamo;
		   unichar* new_current_token=current_token;
		   if (result==1) {
			   /* The text token has been fully matched, so we go on the next one */
			   new_pos_in_current_token=0;
			   new_pos_offset=pos_offset+1;

			   int token_number=p->buffer[new_pos_offset+p->current_origin];
				if (token_number==-1 || token_number==p->STOP) {
					/* Remember 1) that we must not be out of the array's bounds and
					 *          2) that the token {STOP} must never be matched */
					return;
				}
				new_current_token=p->tokens->value[token_number];
  			    new_jamo=p->jamo_tags[p->buffer[token_number]];
 			    new_pos_in_jamo=0;
           }
		   /* If we have a jamo letter match */
		   inflected[pos_in_inflected]=c;
		   explore_dic_in_morpho_mode__(p,bin,inf,adr,new_current_token,inflected,
		   				   new_pos_in_current_token,pos_in_inflected+1,
		   				   new_pos_offset,matches,pattern,save_dic_entry,
		   				   new_jamo,new_pos_in_jamo);
	   }
	   /* Then we try to match a hangul, but only if we are just after a syllab bound */
	   //error("after syllab=%d:  text=%C (%04X)   dico=%C (%04X)\n",after_syllab_bound,current_token[pos_in_current_token],current_token[pos_in_current_token],c,c);
#if 0
	   if (after_syllab_bound && c==current_token[pos_in_current_token]) {
			/* We explore the rest of the dictionary only if the
			 * dictionary char is compatible with the token char. In that case,
			 * we copy in 'inflected' the exact character that is in the dictionary. */
			inflected[pos_in_inflected]=c;
			//error("yes!\n\n\n");
			int new_pos_in_current_token=pos_in_current_token+1;
			int new_pos_in_jamo=pos_in_jamo;
			if (current_token[new_pos_in_current_token]=='\0') {
				/* If we matched the last character of the token, we must reset the jamo position to 0 */
				new_pos_in_jamo=0;
				//error("we did it\n");
			} else {
				/* Otherwise, we must update the jamo position */
				while (u_is_Hangul_Jamo(jamo[new_pos_in_jamo])) {
					new_pos_in_jamo++;
				}
			}
			explore_dic_in_morpho_mode__(p,bin,inf,adr,current_token,inflected,
					new_pos_in_current_token,pos_in_inflected+1,
					   pos_offset,matches,pattern,save_dic_entry,
					   jamo,new_pos_in_jamo);
	   }
#endif
   }
}
}



/**
 * This function tries to find something in p's morphological dictionary that
 * matches the text content as well as the given pattern. If 'pattern' is NULL,
 * anything found in the dictionary will do.
 */
void explore_dic_in_morpho_mode(struct locate_parameters* p,int pos,int pos_in_token,
                                struct parsing_info* *matches,struct pattern* pattern,
                                int save_dic_entry,unichar* jamo,int pos_in_jamo) {
unichar inflected[4096];
for (int i=0;i<p->n_morpho_dics;i++) {
   if (p->morpho_dic_bin[i]!=NULL) {
      /* Can't match anything in an empty dictionary */
      explore_dic_in_morpho_mode__(p,p->morpho_dic_bin[i],p->morpho_dic_inf[i],4,
    		                       p->tokens->value[p->buffer[p->current_origin+pos]],inflected,pos_in_token,
                                   0,pos,matches,pattern,save_dic_entry,jamo,pos_in_jamo);
   }
}
}



