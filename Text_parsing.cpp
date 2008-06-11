 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "MorphologicalLocate.h"
#include "DicVariables.h"


/* Delay between two prints (yyy% done) */
#define DELAY CLOCKS_PER_SEC


int binary_search(int,int*,int);
int find_compound_word(int,int,struct DLC_tree_info*,struct locate_parameters*);
unichar* get_token_sequence(int*,struct string_hash*,int,int);
void enter_morphological_mode(int,int,int,int,struct parsing_info**,int,struct list_int*,struct locate_parameters*);
void shift_variable_bounds(Variables*,int);


/**
 * Performs the Locate operation on the text, saving the occurrences
 * on the fly.
 */
void launch_locate(FILE* f,FILE* out,
                   long int text_size,FILE* info,
                   struct locate_parameters* p) {
fill_buffer(p->token_buffer,f);
if (p->output_policy!=IGNORE_OUTPUTS) {
   /* There may be transducer output, so we allow different output */
   p->ambiguous_output_policy=ALLOW_AMBIGUOUS_OUTPUTS;
}
OptimizedFst2State initial_state=p->optimized_states[p->fst2->initial_states[1]];
p->current_origin=0;
int n_read=0;
int unite;
clock_t startTime=clock();
clock_t currentTime ;
unite=((text_size/100)>1000)?(text_size/100):1000;
while (p->current_origin<p->token_buffer->size
       && p->number_of_matches!=p->search_limit) {
   if (!p->token_buffer->end_of_file
       && p->current_origin>(p->token_buffer->size-2000)) {
      /* If must change of block, we update the absolute offset, and we fill the
       * buffer. */
      p->absolute_offset=p->absolute_offset+p->current_origin;
      fill_buffer(p->token_buffer,p->current_origin,f);
      p->current_origin=0;
   }
   if (unite!=0) {
      n_read=((p->current_origin+p->absolute_offset)%unite);
      if (n_read==0 && ((currentTime=clock())-startTime > DELAY) ) {
         startTime=currentTime;
         u_printf("%2.0f%% done        \r",100.0*(float)(p->absolute_offset+p->current_origin)/(float)text_size);
      }
   }
   if (!(p->token_buffer->int_buffer[p->current_origin]==p->SPACE
       && p->space_policy==DONT_START_WITH_SPACE)) {
      p->stack_base=-1;
      p->stack->stack_pointer=-1;
      struct parsing_info* matches = NULL;
      locate(0,initial_state,0,0,&matches,0,NULL,p);
      free_parsing_info(matches);
      clear_dic_variable_list(&(p->dic_variables));
   }
   p->match_list=save_matches(p->match_list,p->absolute_offset+p->current_origin,out,p);
   (p->current_origin)++;
}
p->match_list=save_matches(p->match_list,p->absolute_offset+p->current_origin,out,p);
u_printf("100%% done      \n\n");
u_printf("%d match%s\n",p->number_of_matches,(p->number_of_matches==1)?"":"es");
if ((p->number_of_outputs != p->number_of_matches)
    && (p->number_of_outputs != 0))
  u_printf("(%d output%s)\n",p->number_of_outputs,(p->number_of_outputs==1)?"":"s");
u_printf("%d recognized units\n",p->matching_units);
if (text_size!=0) {
   u_printf("(%2.3f%% of the text is covered)\n",((float)p->matching_units*100.0)/text_size);
}
if (info!=NULL) {
   u_fprintf(info,"%d match%s\n",p->number_of_matches,(p->number_of_matches==1)?"":"es");
   if ((p->number_of_outputs != p->number_of_matches)
       && (p->number_of_outputs != 0))
     {
       u_fprintf(info,"(%d output%s)\n",p->number_of_outputs,(p->number_of_outputs==1)?"":"s");
     }
   u_fprintf(info,"%d recognized units\n",p->matching_units);
   if (text_size!=0) {
      u_fprintf(info,"(%2.3f%% of the text is covered)\n",((float)p->matching_units*100.0)/text_size);
   }
}
}


/**
 *  Prints the current context to stderr,
 *  except if it was already printed.
 *  If there are more than MAX_ERRORS errors,
 *  exit the programm by calling "fatal_error".
 */
void error_at_token_pos(char* message,int start,int length,struct locate_parameters* p) {
static int n_errors;
static int last_start=-1;
static int last_length;
int i;
if (last_start==start) {
   /* The context was already printed */
   return;
}
error("%s\n  ",message);
for (i=(start-4);i<=(start+20);i++) {
   if (i<0) {
      continue;
   }
   if (i==start) {
      error("<<HERE>>");
   }
   error("%S",p->tokens->value[p->buffer[i]]);
   if (i==(start+length)) {
      error("<<END>>");
   }
}
if (i<(start+length)) {
   error(" ...");
}
error("\n");
if (++n_errors>=MAX_ERRORS) {
   fatal_error("Too many errors, giving up!\n");
}
last_start=start;
last_length=length;
}



/**
 * The logical XOR.
 */
int XOR(int a,int b) {
return (a && !b) || (!a && b);
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
                            * matched in several ways. It is used to detect combinatory
                            * explosions due to bad written grammars. */
            struct list_int* ctx, /* information about the current context, if any */
            struct locate_parameters* p /* miscellaneous parameters needed by the function */
            ) {
#ifdef TRE_WCHAR
int filter_number;
#endif
int pos2,ctrl=0,end_of_compound;
int token,token2;
Transition* t;
int stack_top=p->stack->stack_pointer;
unichar* output;
/* The following static variable holds the number of matches at
 * one position in text. */
static int n_matches_at_token_pos;
if (depth==0) {
   /* We reset if this is first call to 'locate' from a given position in the text */
   n_matches_at_token_pos=0;
}
if (depth>STACK_MAX) {
   /* If there are too much recursive calls */
   error_at_token_pos("\nMaximal stack size reached!\n"
                      "(There may be longer matches not recognized!)",
                      p->current_origin,pos,p);
   return;
}
if (n_matches_at_token_pos>MAX_MATCHES_AT_TOKEN_POS) {
   /* If there are too much matches from the current origin in the text */
   error_at_token_pos("\nToo many (ambiguous) matches starting from one position in text!",
                      p->current_origin,pos,p);
   return;
}
if (current_state->control & 1) {
   /* If we are in a final state... */
   if (ctx!=NULL) {
      /* If we have reached the final state of a graph while
       * looking for a context, it's an error because every
       * opened context must be closed before the end of the graph. */
      error("ERROR: unclosed context in graph \"%S\"\n",p->fst2->graph_names[graph_depth+1]);
      free_list_int(ctx);
      return;
   }
   /* In we are in the top level graph, we have a match */
   if (graph_depth==0) {
      n_matches_at_token_pos++;
      if (p->output_policy==IGNORE_OUTPUTS) {
         if (pos>0) {add_match(pos+p->current_origin+p->absolute_offset-1,NULL,p);}
         else {add_match(pos+p->current_origin+p->absolute_offset,NULL,p);}
      } else {
         p->stack->stack[stack_top+1]='\0';
         if (pos>0) {add_match(pos+p->current_origin+p->absolute_offset-1,p->stack->stack,p);}
         else {add_match(pos+p->current_origin+p->absolute_offset,p->stack->stack,p);}
      }
   }
   else {
      /* If we are in a subgraph */
      if (n_matches==MAX_MATCHES_PER_SUBGRAPH) {
         /* If there are too much matches, we suspect an error in the grammar
          * like an infinite recursion */
         error_at_token_pos("\nMaximal number of matches per subgraph reached!",
                            p->current_origin,pos,p);
         return;
      }
      else {
         /* If everything is fine, we add this match to the match list of the
          * current graph level */
         n_matches++;
         p->stack->stack[stack_top+1]='\0';
         if (p->ambiguous_output_policy==ALLOW_AMBIGUOUS_OUTPUTS) {
            (*matches)=insert_if_different(pos,-1,-1,(*matches),p->stack->stack_pointer,&(p->stack->stack[p->stack_base+1]),p->variables,p->dic_variables);
         } else {
            (*matches)=insert_if_absent(pos,-1,-1,(*matches),p->stack->stack_pointer,&(p->stack->stack[p->stack_base+1]),p->variables,p->dic_variables);
         }
      }
   }
}
/* If we have reached the end of the token buffer, we indicate it by setting
 * the current tokens to -1 */
if (pos+p->current_origin>=p->token_buffer->size) {
   token=-1;
   token2=-1;
} else {
   token=p->buffer[pos+p->current_origin];
   if (token==p->SPACE) {pos2=pos+1;}
      /* Now, we deal with the SPACE, if any. To do that, we use several variables:
       * pos: current position in the token buffer, relative to the current origin
       * pos2: position of the first non-space token from 'pos'.
       * token2: token at pos2  or -1 if 'pos2' is outside the token buffer */
   else {pos2=pos;}
   if (pos2+p->current_origin>=p->token_buffer->size) {
      token2=-1;
   } else {
      token2=p->buffer[pos2+p->current_origin];
   }
}

/**
 * SUBGRAPHS
 */
struct opt_graph_call* graph_call_list=current_state->graph_calls;
if (graph_call_list!=NULL) {
   /* If there are subgraphs, we process them */
   int* var_backup=NULL;
   struct dic_variable* dic_variables_backup=NULL;
   int old_StackBase=p->stack_base;
   if (p->output_policy!=IGNORE_OUTPUTS) {
      /* For better performance when ignoring outputs */
      var_backup=create_variable_backup(p->variables);
      dic_variables_backup=p->dic_variables;
   }
   do {
      /* For each graph call, we look all the reachable states */
      t=graph_call_list->transition;
      while (t!=NULL) {
         struct parsing_info* L=NULL;
         p->stack_base=p->stack->stack_pointer;
         locate(graph_depth+1, /* Exploration of the subgraph */
                p->optimized_states[p->fst2->initial_states[graph_call_list->graph_number]],
                pos,depth+1,&L,0,NULL,p);
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
               p->dic_variables=L->dic_variable_backup;
             }
             /* And we continue the exploration */
             locate(graph_depth,p->optimized_states[t->state_number],L->position,depth+1,matches,n_matches,ctx,p);
             p->stack->stack_pointer=stack_top;
             if (graph_depth==0) {
                /* If we are at the top graph level, we restore the variables */
                if (p->output_policy!=IGNORE_OUTPUTS) {
                   install_variable_backup(p->variables,var_backup);
                }
             }
             L=L->next;
           }
           while (L!=NULL);
           free_parsing_info(L_first); //  free all subgraph matches
         }
         t=t->next;
      } /* end of while (t!=NULL) */
   } while ((graph_call_list=graph_call_list->next)!=NULL);
   /* Finally, we have to restore the stack and other backup stuff */
   p->stack->stack_pointer=stack_top;
   p->stack_base=old_StackBase; /* May be changed by recursive subgraph calls */
   if (p->output_policy!=IGNORE_OUTPUTS) { /* For better performance (see above) */
      install_variable_backup(p->variables,var_backup);
      free_variable_backup(var_backup);
      p->dic_variables=dic_variables_backup;
   }
} /* End of processing subgraphs */

/**
 * METAS
 */
struct opt_meta* meta_list=current_state->metas;
if (meta_list!=NULL) {
   /* We cache the control bytes of the pos2 token. The pos token has not interest,
    * because it is 1) a space  or 2) equal to the pos2 one. */
   if (token2!=-1) ctrl=p->token_control[token2];
   else ctrl=0;
}
while (meta_list!=NULL) {
   /* We process all the meta of the list */
   t=meta_list->transition;
   while (t!=NULL) {
      /* We cache the output of the current tag, as well as values indicating if the
       * current pos2 tokens matches the tag's morphological filter, if any. */
      output=p->tags[t->tag_number]->output;
      /* If there is no morphological filter, we act as if there was a matching one, except
       * if we are at the end of the token buffer. With this trick, the morphofilter test
       * will avoid overpassing the end of the token buffer. */
      int morpho_filter_OK=(token2!=-1)?1:0;
      #ifdef TRE_WCHAR
      filter_number=p->tags[t->tag_number]->filter_number;
      if (token2!=-1) {
         morpho_filter_OK=(filter_number==-1 || token_match_filter(p->filter_match_index,token2,filter_number));
      }
      #endif
      int negation=meta_list->negation;
      /* We use these variables to deal with match cases:
       * the matching sequence is in the range [start;end[
       * start=end means that the transition matches but that no token is read in the text,
       * which is the case, for instance, with epsilon transitions. start=-1 means that the
       * transition does not match. 'end' is the position for the next call
       * to 'locate'. */
      int start=-1;
      int end=-1;
      switch (meta_list->meta) {
         
         case META_SHARP:
            if (token==-1 || token!=p->SPACE) {
               /* # can match only if there is no space at the current position or
                * if we are at the end of the token buffer. */
               start=pos;
               end=pos;
            }
            break;
            
         case META_SPACE:
            if (token!=-1 && token==p->SPACE) {
               /* The space meta can match only if there is a space at the current position.
                * Note that we don't compare token and p->SPACE since p->SPACE can be -1 if
                * the text contains no space. */
               start=pos;
               end=pos+1;
            }
            break;
            
         case META_EPSILON:
            /* We can always match the empty word */
            start=pos;
            end=pos;
            break;
            
         case META_MOT:
            if (!morpho_filter_OK || token2==p->SENTENCE || token2==p->STOP) {
               /* <MOT> and <!MOT> must NEVER match {S} and {STOP}! */
               break;
            }
            if ((p->space_policy==START_WITH_SPACE) && (token2==p->SPACE) && negation) {
               /* If we want to catch a space with <!MOT> */
               start=pos;
               end=pos+1;
            }
            else if (XOR(negation,ctrl&MOT_TOKEN_BIT_MASK)) {
               start=pos;
               end=pos2+1;
            }
            break;
            
         case META_DIC:
            if (token2==-1 || token2==p->STOP) break;
            if (!negation) {
               /* If there is no negation on DIC, we can look for a compound word */
               end_of_compound=find_compound_word(pos2,COMPOUND_WORD_PATTERN,p->DLC_tree,p);
               if (end_of_compound!=-1) {
                  /* If we find one, we must test if it matches the morphological filter, if any */
                  int OK=1;
                  #ifdef TRE_WCHAR
                  if (filter_number==-1) OK=1;
                  else {
                     unichar* sequence=get_token_sequence(p->buffer,p->tokens,pos2+p->current_origin,end_of_compound+p->current_origin);
                     OK=(string_match_filter(p->filters,sequence,filter_number)==0);
                     free(sequence);
                  }
                  #endif
                  if (OK) {
                     /* <DIC> can match two things: a sequence of tokens or a single token
                      * As we don't want to process lists of [start,end[ ranges, we
                      * directly handle here the case of a token sequence. */
                     if (p->output_policy!=IGNORE_OUTPUTS) {
                        if (!process_output(output,p)) {
                           break;
                        }
                     }
                     if (p->output_policy==MERGE_OUTPUTS) {
                        for (int x=pos;x<=end_of_compound;x++) {
                           push_string(p->stack,p->tokens->value[p->buffer[x+p->current_origin]]);
                        }
                     }
                     locate(graph_depth,p->optimized_states[t->state_number],end_of_compound+1,depth+1,matches,n_matches,ctx,p);
                     p->stack->stack_pointer=stack_top;
                  }
               }
               /* Now, we look for a simple word */
               if (ctrl&DIC_TOKEN_BIT_MASK && morpho_filter_OK) {
                  start=pos;
                  end=pos2+1;
               }
               break;
            }
            /* We have the meta <!DIC> */
            if (ctrl&NOT_DIC_TOKEN_BIT_MASK && morpho_filter_OK) {
               start=pos;
               end=pos2+1;
            }
            break;
            
         case META_SDIC: 
            if (morpho_filter_OK && (ctrl&DIC_TOKEN_BIT_MASK) && !(ctrl&CDIC_TOKEN_BIT_MASK)) {
               /* We match only simple words */
               start=pos;
               end=pos2+1;
            }
            break;
            
         case META_CDIC:
            if (token2==-1 || token2==p->STOP) break;
            end_of_compound=find_compound_word(pos2,COMPOUND_WORD_PATTERN,p->DLC_tree,p);
            if (end_of_compound!=-1) {
               /* If we find a compound word */
               int OK=1;
               #ifdef TRE_WCHAR
               if (filter_number==-1) OK=1;
               else {
                  unichar* sequence=get_token_sequence(p->buffer,p->tokens,pos2+p->current_origin,end_of_compound+p->current_origin);
                  OK=(string_match_filter(p->filters,sequence,filter_number)==0);
                  free(sequence);
               }
               #endif
               if (OK) {
                  /* <CDIC> can match two things: a sequence of tokens or a compound
                   * tag token like "{black-eyed,.A}". As we don't want to process lists
                   * of [start,end[ ranges, we directly handle here the case of a
                   * token sequence. */
                  if (p->output_policy!=IGNORE_OUTPUTS) {
                     if (!process_output(output,p)) {
                        break;
                     }
                  }
                  if (p->output_policy==MERGE_OUTPUTS) {
                     for (int x=pos;x<=end_of_compound;x++) {
                        push_string(p->stack,p->tokens->value[p->buffer[x+p->current_origin]]);
                     }
                  }
                  locate(graph_depth,p->optimized_states[t->state_number],end_of_compound+1,depth+1,matches,n_matches,ctx,p);
                  p->stack->stack_pointer=stack_top;
               }
            }
            /* Anyway, we could have a tag compound word like {aujourd'hui,.ADV} */
            if (morpho_filter_OK && ctrl&CDIC_TOKEN_BIT_MASK) {
               start=pos;
               end=pos2+1;
            }
            break;

         case META_MAJ:
            if (!morpho_filter_OK) break;
            if (!negation) {
               if (ctrl&MAJ_TOKEN_BIT_MASK) {
                  start=pos;
                  end=pos2+1;
               }
               break;
            }
            if ( !(ctrl&MAJ_TOKEN_BIT_MASK) && (ctrl&MOT_TOKEN_BIT_MASK)) {
               /* If we have <!MAJ>, the matching token must be matched by <MOT> */
               start=pos;
               end=pos2+1;
            }
            break;

         case META_MIN:
            if (!morpho_filter_OK) break;
            if (!negation) {
               if (ctrl&MIN_TOKEN_BIT_MASK) {
                  start=pos;
                  end=pos2+1;
               }
               break;
            }
            if ( !(ctrl&MIN_TOKEN_BIT_MASK) && (ctrl&MOT_TOKEN_BIT_MASK)) {
               /* If we have <!MIN>, the matching token must be matched by <MOT> */
               start=pos;
               end=pos2+1;
            }
            break;
 
         case META_PRE: 
            if (!morpho_filter_OK) break;
            if (!negation) {
               if (ctrl&PRE_TOKEN_BIT_MASK) {
                  start=pos;
                  end=pos2+1;
               }
               break;
            }
            if ( !(ctrl&PRE_TOKEN_BIT_MASK) && (ctrl&MOT_TOKEN_BIT_MASK)) {
               /* If we have <!PRE>, the matching token must be matched by <MOT> */
               start=pos;
               end=pos2+1;
            }
            break;

      case META_NB: 
         if (token2==-1) break;
         { /* This block avoids visibility problem about 'z' */
            int z=pos2;
            while (z+p->current_origin<p->token_buffer->size 
                   && is_a_digit_token(p->tokens->value[p->buffer[z+p->current_origin]])) {
               z++;
            }
            if (z!=pos2) {
               /* If we have found a contiguous digit sequence */
               start=pos;
               end=z;
            }
         }
         break;

      case META_TOKEN:
         if (token2==-1 || token2==p->STOP || !morpho_filter_OK) {
            /* The {STOP} tag must NEVER be matched by any pattern */
            break;
         }
         start=pos;
         end=pos2+1;
         break;

      case META_BEGIN_MORPHO: 
         if (token2==-1 || token2==p->STOP || !morpho_filter_OK) {
            /* The {STOP} tag must NEVER be matched by any pattern */
            break;
         }
         if (p->output_policy==MERGE_OUTPUTS) {
            if (pos2!=pos) push_char(p->stack,' ');
         }
         enter_morphological_mode(graph_depth,t->state_number,pos2,depth+1,matches,n_matches,ctx,p);
         p->stack->stack_pointer=stack_top;
         break;
         
      case META_END_MORPHO: 
         /* Should not happen */
         fatal_error("Unexpected morphological mode end tag $>\n");
         break;

      case META_LEFT_CONTEXT: 
         int real_origin=p->current_origin;
         if (p->space_policy==START_WITH_SPACE) {
            p->current_origin+=pos;
         } else {
            p->current_origin+=pos2;
         }
         int real_stack_base=p->stack_base;
         p->stack_base=0;
         struct stack_unichar* real_stack=p->stack;
         p->stack=new_stack_unichar(real_stack->capacity);
         
         int* var_backup=NULL;
         if (p->output_policy!=IGNORE_OUTPUTS) {
        	 var_backup=create_variable_backup(p->variables);
        	 shift_variable_bounds(p->variables,real_origin-p->current_origin);
         }
         locate(graph_depth,p->optimized_states[t->state_number],0,depth+1,matches,n_matches,ctx,p);
         if (p->output_policy!=IGNORE_OUTPUTS) {
            install_variable_backup(p->variables,var_backup);
            free_variable_backup(var_backup);
         }
         p->current_origin=real_origin;
         p->stack_base=real_stack_base;
         free_stack_unichar(p->stack);
         p->stack=real_stack;
         p->stack->stack_pointer=stack_top;
         break;

      } /* End of the switch */

      if (start!=-1) {
         /* If the transition has matched */
         if (p->output_policy!=IGNORE_OUTPUTS) {
            /* We process its output */
            if (!process_output(output,p)) {
               goto next;
            }
         }
         if (p->output_policy==MERGE_OUTPUTS) {
            /* Then, if we are in merge mode, we push the tokens that have
             * been read to the output */
            for (int y=start;y<end;y++) {
               push_string(p->stack,p->tokens->value[p->buffer[y+p->current_origin]]);
            }
         }
         /* Then, we continue the exploration of the grammar */
         locate(graph_depth,p->optimized_states[t->state_number],end,depth+1,matches,n_matches,ctx,p);
         /* Once we have finished, we restore the stack */
         p->stack->stack_pointer=stack_top;
      }
      next: t=t->next;
   }
   meta_list=meta_list->next;
}

/**
 * VARIABLE STARTS
 */
struct opt_variable* variable_list=current_state->variable_starts;
while (variable_list!=NULL) {
   int old=get_variable_start(p->variables,variable_list->variable_number);
   set_variable_start(p->variables,variable_list->variable_number,pos);
   locate(graph_depth,p->optimized_states[variable_list->transition->state_number],pos,depth+1,matches,n_matches,ctx,p);
   p->stack->stack_pointer=stack_top;
   if (ctx==NULL) {
      /* We do not restore previous value if we are inside a context, in order
       * to allow extracting things from contexts (see the
       * "the cat is white" example in Unitex manual). */
      set_variable_start(p->variables,variable_list->variable_number,old);
   }
   variable_list=variable_list->next;
}

/**
 * VARIABLE ENDS
 */
variable_list=current_state->variable_ends;
while (variable_list!=NULL) {
   int old=get_variable_end(p->variables,variable_list->variable_number);
   set_variable_end(p->variables,variable_list->variable_number,pos);
   locate(graph_depth,p->optimized_states[variable_list->transition->state_number],pos,depth+1,matches,n_matches,ctx,p);
   p->stack->stack_pointer=stack_top;
   if (ctx==NULL) {
      /* We do not restore previous value if we are inside a context, in order
       * to allow extracting things from contexts (see the
       * "the cat is white" example in Unitex manual). */
      set_variable_end(p->variables,variable_list->variable_number,old);
   }
   variable_list=variable_list->next;
}

/**
 * CONTEXTS
 */
struct opt_contexts* contexts=current_state->contexts;
if (contexts!=NULL) {
   Transition* t;
   for (int n_ctxt=0;n_ctxt<contexts->size_positive;n_ctxt=n_ctxt+2) {
      t=contexts->positive_mark[n_ctxt];
      /* We look for a positive context from the current position */
      struct list_int* c=new_list_int(0,ctx);
      locate(graph_depth,p->optimized_states[t->state_number],pos,depth+1,NULL,0,c,p);
      /* Note that there is no matches to free since matches cannot be built within a context */
      p->stack->stack_pointer=stack_top;
      if (c->n) {
         /* If the context has matched, then we can explore all the paths
          * that starts from the context end */
         Transition* states=contexts->positive_mark[n_ctxt+1];
         while (states!=NULL) {
            locate(graph_depth,p->optimized_states[states->state_number],pos,depth+1,matches,n_matches,ctx,p);
            p->stack->stack_pointer=stack_top;
            states=states->next;
         }
      }
      free(c);
   }
   for (int n_ctxt=0;n_ctxt<contexts->size_negative;n_ctxt=n_ctxt+2) {
      t=contexts->negative_mark[n_ctxt];
      /* We look for a negative context from the current position */
      struct list_int* c=new_list_int(0,ctx);
      locate(graph_depth,p->optimized_states[t->state_number],pos,depth+1,NULL,0,c,p);
      /* Note that there is no matches to free since matches cannot be built within a context */
      p->stack->stack_pointer=stack_top;
      if (!c->n) {
         /* If the context has not matched, then we can explore all the paths
          * that starts from the context end */
         Transition* states=contexts->negative_mark[n_ctxt+1];
         while (states!=NULL) {
            locate(graph_depth,p->optimized_states[states->state_number],pos,depth+1,matches,n_matches,ctx,p);
            p->stack->stack_pointer=stack_top;
            states=states->next;
         }
      }
      free_list_int(c);
   }
   if ((t=contexts->end_mark)!=NULL) {
      /* If we have a closing context mark */
      if (ctx==NULL) {
         /* If there was no current opened context, it's an error */
         error("ERROR: unexpected closing context mark in graph \"%S\"\n",p->fst2->graph_names[graph_depth+1]);
         return;
      }
      /* Otherwise, we just indicate that we have found a context closing mark,
       * and we return */
      ctx->n=1;
      return;
   }
}

/**
 * Now that we have finished with meta symbols, there can be no chance to advance
 * in the grammar if we are at the end of the token buffer.
 */
if (token2==-1) {return;}

/**
 * COMPOUND WORD PATTERNS:
 * here, we deal with patterns that can only match compound sequences
 */
struct opt_pattern* pattern_list=current_state->compound_patterns;
while (pattern_list!=NULL) {
   t=pattern_list->transition;
   while (t!=NULL) {
      #ifdef TRE_WCHAR
      filter_number=p->tags[t->tag_number]->filter_number;
      #endif
      output=p->tags[t->tag_number]->output;
      end_of_compound=find_compound_word(pos2,pattern_list->pattern_number,p->DLC_tree,p);
      if (end_of_compound!=-1 && !(pattern_list->negation)) {
         int OK=1;
         #ifdef TRE_WCHAR
         if (filter_number==-1 ) OK=1;
         else {
            unichar* sequence=get_token_sequence(p->buffer,p->tokens,pos2+p->current_origin,end_of_compound+p->current_origin);
            OK=(string_match_filter(p->filters,sequence,filter_number)==0);
            free(sequence);
         }
         #endif
         if (OK){
            if (p->output_policy!=IGNORE_OUTPUTS) process_output(output,p);
            if (p->output_policy==MERGE_OUTPUTS) {
               for (int x=pos;x<=end_of_compound;x++) {
                  push_string(p->stack,p->tokens->value[p->buffer[x+p->current_origin]]);
               }
            }
            locate(graph_depth,p->optimized_states[t->state_number],end_of_compound+1,depth+1,matches,n_matches,ctx,p);
            p->stack->stack_pointer=stack_top;
         }
      }
      t=t->next;
   }
   pattern_list=pattern_list->next;
}

/**
 * SIMPLE WORD PATTERNS:
 * here, we deal with patterns that can match both simple and
 * compound words, like "<N>".
 */ 
pattern_list=current_state->patterns;
while (pattern_list!=NULL) {
   t=pattern_list->transition;
   while (t!=NULL) {
      #ifdef TRE_WCHAR
      filter_number=p->tags[t->tag_number]->filter_number;  
      #endif
      output=p->tags[t->tag_number]->output;
      /* We try to match a compound word */
      end_of_compound=find_compound_word(pos2,pattern_list->pattern_number,p->DLC_tree,p);
      if (end_of_compound!=-1 && !(pattern_list->negation)) {
         int OK=1;
         #ifdef TRE_WCHAR
         if (filter_number==-1) OK=1;
         else {
            unichar* sequence=get_token_sequence(p->buffer,p->tokens,pos2+p->current_origin,end_of_compound+p->current_origin);
            OK=(string_match_filter(p->filters,sequence,filter_number)==0);
            free(sequence);
         }
         #endif
         if (OK) {
            if (p->output_policy!=IGNORE_OUTPUTS) process_output(output,p);
            if (p->output_policy==MERGE_OUTPUTS) {
               for (int x=pos;x<=end_of_compound;x++) {
                  push_string(p->stack,p->tokens->value[p->buffer[x+p->current_origin]]);
               }
            }
            locate(graph_depth,p->optimized_states[t->state_number],end_of_compound+1,depth+1,matches,n_matches,ctx,p);
            p->stack->stack_pointer=stack_top;
         }
      }
      /* And now, we look for simple words */
      int OK=1;
      #ifdef TRE_WCHAR
      OK=(filter_number==-1 || token_match_filter(p->filter_match_index,token2,filter_number));
      #endif
      if (OK) {
         if (p->matching_patterns[token2]!=NULL) {
            if (XOR(get_value(p->matching_patterns[token2],pattern_list->pattern_number),pattern_list->negation)) {
               if (p->output_policy!=IGNORE_OUTPUTS) process_output(output,p);
               if (p->output_policy==MERGE_OUTPUTS) {
                  if (pos2!=pos) push_char(p->stack,' ');
                  push_string(p->stack,p->tokens->value[token2]);
               }
               locate(graph_depth,p->optimized_states[t->state_number],pos2+1,depth+1,matches,n_matches,ctx,p);
               p->stack->stack_pointer=stack_top;
            }
         } else {
            /* If the token matches no pattern, then it can match a pattern negation
             * like <!V> */
            if (pattern_list->negation && (p->token_control[token2] & MOT_TOKEN_BIT_MASK)) {
               if (p->output_policy!=IGNORE_OUTPUTS) {
                  if (!process_output(output,p)) {
                     goto next2;
                  }
               }
               if (p->output_policy==MERGE_OUTPUTS) {
                  if (pos2!=pos) push_char(p->stack,' ');
                  push_string(p->stack,p->tokens->value[token2]);
               }
               locate(graph_depth,p->optimized_states[t->state_number],pos2+1,depth+1,matches,n_matches,ctx,p);
               p->stack->stack_pointer=stack_top;
            }
         }
      }
      next2: t=t->next;
   }
   pattern_list=pattern_list->next;
}

/**
 * TOKENS
 */
if (current_state->number_of_tokens!=0) {
   int n=binary_search(token2,current_state->tokens,current_state->number_of_tokens);
   if (n!=-1) {
      t=current_state->token_transitions[n];
      while (t!=NULL) {
         #ifdef TRE_WCHAR
         filter_number=p->tags[t->tag_number]->filter_number;
         if (filter_number==-1 || token_match_filter(p->filter_match_index,token2,filter_number)) 
         #endif
         {
            output=p->tags[t->tag_number]->output;
            if (p->output_policy!=IGNORE_OUTPUTS) {
               if (!process_output(output,p)) {
                  goto next3;
               }
            }
            if (p->output_policy==MERGE_OUTPUTS) {
               if (pos2!=pos) push_char(p->stack,' ');
               push_string(p->stack,p->tokens->value[token2]);
            }
            locate(graph_depth,p->optimized_states[t->state_number],pos2+1,depth+1,matches,n_matches,ctx,p);
            p->stack->stack_pointer=stack_top;
         }
         next3: t=t->next;
      }
   }
}
}


/**
 * Looks for 'a' in the given array. Returns its position or -1 if not found.
 */
int binary_search(int a, int* t, int n) {
register int start,middle;
if (n==0||t==NULL) return -1;
if (a<t[0]) return -1;
if (a>t[n-1]) return -1;
n=n-1;
start=0;
while (start<=n) {
   middle=((start+n) >> 1); // like /2, but faster (depends on compiler)
   if (t[middle]==a) return middle;
   if (t[middle]<a) {
      start=middle+1;
   } else {
      n=middle-1;
   }
}
return -1;
}


/**
 * This function compares the text to the compound word tree in order to find
 * the longest compound word thay have in common. It returns the position
 * of the last token of the compound word, or -1 if no compound word is found.
 */
int find_longest_compound_word(int pos,struct DLC_tree_node* node,int pattern_number,struct locate_parameters* p) {
if (node==NULL) {
   fatal_error("NULL node in find_longest_compound_word\n");
}
int current_token=p->buffer[pos+p->current_origin];
int position_max;
if (-1!=binary_search(pattern_number,node->array_of_patterns,node->number_of_patterns))
   position_max=pos-1;
else position_max=-1;
if (pos+p->current_origin==p->token_buffer->size) return position_max;
/* As the 'node->destination_tokens' array may contain duplicates, we look for
 * one, and then we look before and after it, in order to examine all the
 * duplicates. */
int position=binary_search(current_token,node->destination_tokens,node->number_of_transitions);
if (position==-1) return position_max;
/* We look after it... */
for (int i=position;i<node->number_of_transitions && current_token==node->destination_tokens[i];i++) {
   /* If we can follow a transition tagged with the good token */
   int m=find_longest_compound_word(pos+1,node->destination_nodes[i],pattern_number,p);
   if (m>position_max) position_max=m;
}
/* ...and before it */
for (int i=position-1;i>=0 && current_token==node->destination_tokens[i];i--) {
   /* If we can follow a transition tagged with the good token */
   int m=find_longest_compound_word(pos+1,node->destination_nodes[i],pattern_number,p);
   if (m>position_max) position_max=m;
}
return position_max;
}


/**
 * Looks for a compound word from the position 'pos' in the text, that matches the
 * given pattern_number. Returns the position of the last token of the compound word
 * or -1 if no compound word is found. In case of a compound word that is a prefix 
 * of another, the function considers the longest one.
 */
int find_compound_word(int pos,int pattern_number,struct DLC_tree_info* DLC_tree,struct locate_parameters* p) {
return find_longest_compound_word(pos,DLC_tree->root,pattern_number,p);
}


/**
 * This function compares the text to the compound word tree in order to find
 * the longest compound word thay have in common. It returns the position
 * of the last token of the compound word, or -1 if no compound word is found.
 */
int find_compound_word_old_(int pos,struct DLC_tree_node* node,int pattern_number,struct locate_parameters* p) {
int position_max,m,res;
if (node==NULL) return -1;
if (-1!=binary_search(pattern_number,node->array_of_patterns,node->number_of_patterns))
   position_max=pos-1;
else position_max=-1;
if (pos+p->current_origin==p->token_buffer->size) return position_max;
res=binary_search(p->buffer[pos+p->current_origin],node->destination_tokens,node->number_of_transitions);
if (res==-1) return position_max;
m=find_compound_word_old_(pos+1,node->destination_nodes[res],pattern_number,p);
if (m>position_max) return m;
return position_max;
}


/**
 * Looks for a compound word from the position 'pos' in the text, that matches the
 * given pattern_number. Returns the position of the last token of the compound word
 * or -1 if no compound word is found. In case of a compound word that is a prefix 
 * of another, the function considers the longest one.
 */
int find_compound_word_old(int pos,int pattern_number,struct DLC_tree_info* DLC_tree,struct locate_parameters* p) {
int position_max,m,res;
struct DLC_tree_node *n;
if (pos+p->current_origin==p->token_buffer->size) {
   return -1;
}
if ((n=DLC_tree->index[p->buffer[pos+p->current_origin]])==NULL) {
   return -1;
}
if (-1!=binary_search(pattern_number,n->array_of_patterns,n->number_of_patterns)) {
   position_max=pos;
}
else position_max=-1;
pos++;
if (pos+p->current_origin==p->token_buffer->size) {
   return -1;
}
res=binary_search(p->buffer[pos+p->current_origin],n->destination_tokens,n->number_of_transitions);
if (res==-1) {
   return position_max;
}
m=find_compound_word_old_(pos+1,n->destination_nodes[res],pattern_number,p);
if (m>position_max) {
   return m;
}
return position_max;
}


/**
 * Returns a string corresponding to the tokens in the range [start;end].
 */
unichar* get_token_sequence(int* token_array,struct string_hash* tokens,int start,int end) {
int i;
int l=0;
for (i=start;i<=end;i++) {
   l=l+u_strlen(tokens->value[token_array[i]]);
}
unichar* res=(unichar*)malloc((l+1)*sizeof(unichar));
if (res==NULL) {
   fatal_error("Not enough memory in get_token_sequence\n");
}
l=0;
for (i=start;i<=end;i++) {
   u_strcpy(&(res[l]),tokens->value[token_array[i]]);
   l=l+u_strlen(tokens->value[token_array[i]]);
}
return res;
}



/**
 * Apply the given shift to all variable bounds in the given variable set.
 */
void shift_variable_bounds(Variables* v,int shift) {
if (v==NULL) {
	return;
}
int l=v->variable_index->size;
for (int i=0;i<l;i++) {
	if (v->variables[i].start!=UNDEF_VAR_BOUND) {
		v->variables[i].start+=shift;
	}
	if (v->variables[i].end!=UNDEF_VAR_BOUND) {
		v->variables[i].end+=shift;
	}
}
}


