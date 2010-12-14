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

#include "LocateTfstMatches.h"
#include "Error.h"
#include <stdlib.h>
#include "Tfst.h"
#include "List_pointer.h"
#include "TransductionStackTfst.h"
#include "DicVariables.h"


/* This special negative value is used to indicate that a $* tag was found, and that
 * we wait for a text dependent tag to set the actual beginning of the match to
 * be added to the concordance file */
#define LEFT_CONTEXT_PENDING -456


/**
 * Allocates, initializes and returns a new tfst_match.
 */
struct tfst_match* new_tfst_match(int source_state_text,
                                  int dest_state_text,
                                  Transition* fst2_transition,
                                  int pos_kr,
                                  int text_tag_number,
                                  int first_time) {
struct tfst_match* match=(struct tfst_match*)malloc(sizeof(struct tfst_match));
if (match==NULL) {
   fatal_alloc_error("new_tfst_match");
}
match->source_state_text=source_state_text;
match->dest_state_text=dest_state_text;
match->fst2_transition=fst2_transition;
match->pos_kr=pos_kr;
match->text_tag_numbers=sorted_insert(text_tag_number,NULL);
match->next=NULL;
match->pointed_by=0;
match->first_time=first_time;
return match;
}


/**
 * Inserts a tfst_match corresponding to the given information, if not already present.
 */
struct tfst_match* insert_in_tfst_matches(struct tfst_match* list,
                                          int source_state_text,
                                          int dest_state_text,
                                          Transition* fst2_transition,
                                          int pos_kr,
                                          int text_tag_number,
                                          int first_time) {
if (list==NULL) {
   return new_tfst_match(source_state_text,dest_state_text,
         fst2_transition,pos_kr,text_tag_number,first_time);
}
if (list->source_state_text==source_state_text
    && list->dest_state_text==dest_state_text
    && list->fst2_transition==fst2_transition
    && list->pos_kr==pos_kr) {
    list->text_tag_numbers=sorted_insert(text_tag_number,list->text_tag_numbers);
    return list;
}
list->next=insert_in_tfst_matches(list->next,
                         source_state_text,dest_state_text,
                         fst2_transition,pos_kr,text_tag_number,first_time);
return list;
}


/**
 * Frees the given tfst_match.
 */
void free_tfst_match(struct tfst_match* match) {
if (match==NULL) {
   fatal_error("NULL error in free_tfst_match");
}
/* We MUST NOT free 'fst2_transition' since it is just a copy of an actual transition
 * in the grammar */
free_list_int(match->text_tag_numbers);
free(match);
}


/**
 * Allocates, initializes and returns a tfst_match_list.
 */
struct tfst_match_list* new_tfst_match_list() {
struct tfst_match_list* l=(struct tfst_match_list*)malloc(sizeof(struct tfst_match_list));
if (l==NULL) {
   fatal_alloc_error("new_tfst_match_list");
}
l->match=NULL;
l->next=NULL;
return l;
}


/**
 * Adds a tfst_match to the given list, increasing its reference counter.
 */
struct tfst_match_list* add_match_in_list(struct tfst_match_list* list,
                                          struct tfst_match* match) {
struct tfst_match_list* l=new_tfst_match_list();
l->match=match;
l->next=list;
if (match!=NULL) (match->pointed_by)++;
return l;
}


/**
 * This function takes a list of match elements and free the first elements
 * until they have a non null pointed_by field. In other words, this function
 * releases all the first elements that are not involved anymore in any match.
 * The limit parameter refers to a stop element. In fact, when we want to
 * clean a list of match elements associated to a subgraph call, we do not want
 * to remove the elements that do not depend on this subgraph call.
 */
void clean_tfst_match_list(struct tfst_match* list,struct tfst_match* limit) {
struct tfst_match* tmp;
while (list!=NULL && list!=limit && list->pointed_by==0) {
   /* We have an element that we can free */
   if (list->next!=NULL) {
      /* If there is a next element, then we say
       * that it is now pointed by one element less */
      (list->next->pointed_by)--;
   }
   tmp=list->next;
   free_tfst_match(list);
   list=tmp;
}
}


/**
 * This function clones the given element. Note that we increase the 'pointed_by' field
 * of the tfst_match.
 */
struct tfst_simple_match_list* new_tfst_simple_match_list(struct tfst_simple_match_list* e,
		                                                  struct tfst_simple_match_list* next) {
struct tfst_simple_match_list* m=(struct tfst_simple_match_list*)malloc(sizeof(struct tfst_simple_match_list));
if (m==NULL) {
	fatal_alloc_error("new_tfst_simple_match_list");
}
memcpy(m,e,sizeof(struct tfst_simple_match_list));
if (m->output!=NULL) {
	/* If there was an output, we have to clone it */
	m->output=u_strdup(m->output);
}
if (m->match!=NULL) {
	(m->match->pointed_by)++;
}
m->next=next;
return m;
}


/**
 * Frees the given element. Note that we decrease the 'pointed_by' field
 * of the tfst_match.
 */
void free_tfst_simple_match_list(struct tfst_simple_match_list* m) {
if (m==NULL) {
	return;
}
if (m->match!=NULL) {
	(m->match->pointed_by)--;
}
free(m->output);
free(m);
}


/**
 * This function looks for the first element that is not text independent
 */
struct tfst_match* find_first_text_dependent_tag(struct tfst_match* list) {
while (list!=NULL) {
   struct list_int* l=list->text_tag_numbers;
   while (l!=NULL) {
      if (l->n!=-1) {
         /* If there is at least one text dependent tag, 'list' is the one */
         return list;
      }
      l=l->next;
   }
   list=list->next;
}
/* If the list has no text dependent tag, we fail */
return NULL;
}


/**
 * This function looks for the last element that is not text independent
 */
struct tfst_match* find_last_text_dependent_tag(struct tfst_match* m) {
if (m==NULL) {
   return NULL;
}
struct tfst_match* candidate=find_last_text_dependent_tag(m->next);
if (candidate!=NULL) {
   /* If there is a candidate, we return it */
   return candidate;
}
/* If there is no candidate, maybe 'm' could be the one */
struct list_int* list=m->text_tag_numbers;
while (list!=NULL) {
   if (list->n!=-1) {
      /* If there is at least one text dependent tag, 'm' is the one */
      return m;
   }
   list=list->next;
}
/* If 'm' has no text dependent tag, we fail */
return NULL;
}


/**
 * This function extracts simple information from a complex tfst_match list.
 */
static void fill_element(struct locate_tfst_infos* infos,struct tfst_simple_match_list* e,struct tfst_match* m) {
/* First, we compute the end offsets with the first element of the list
 * (remember: a match list is reversed) */
e->m.end_pos_in_token=-1;
e->m.end_pos_in_char=-1;
e->m.end_pos_in_letter=-1;
struct tfst_match* first_text_dependent_tag=find_first_text_dependent_tag(m);
if (first_text_dependent_tag==NULL) {
   /* If the whole match is text independent, we fail */
   e->m.end_pos_in_token=-1;
   return;
}
struct list_int* tags=first_text_dependent_tag->text_tag_numbers;
while (tags!=NULL) {
   if (tags->n!=-1) {
      /* We only consider tags that are not text independent */
	   TfstTag* t=(TfstTag*)(infos->tfst->tags->tab[tags->n]);
	   if (e->m.end_pos_in_token==-1 || e->m.end_pos_in_token<t->m.end_pos_in_token
	       || e->m.end_pos_in_char<t->m.end_pos_in_char
	       || e->m.end_pos_in_letter<t->m.end_pos_in_letter) {
		   e->m.end_pos_in_token=t->m.end_pos_in_token;
		   e->m.end_pos_in_char=t->m.end_pos_in_char;
		   e->m.end_pos_in_letter=t->m.end_pos_in_letter;
	   }
   }
	tags=tags->next;
}

/* Then, we locate the last element in order to get information about the start offset */
struct tfst_match* last_text_dependent_tag=find_last_text_dependent_tag(m);
if (last_text_dependent_tag==NULL) {
   /* If the whole match is text independent, we fail */
   e->m.end_pos_in_token=-1;
   return;
}
e->m.start_pos_in_token=-1;
e->m.start_pos_in_char=-1;
e->m.start_pos_in_letter=-1;
tags=last_text_dependent_tag->text_tag_numbers;
while (tags!=NULL) {
   if (tags->n!=-1) {
      /* We only consider tags that are not text independent */
	   TfstTag* t=(TfstTag*)(infos->tfst->tags->tab[tags->n]);
	   if (e->m.start_pos_in_token==-1 || e->m.start_pos_in_token>t->m.start_pos_in_token
	       || e->m.start_pos_in_char>t->m.start_pos_in_char
	       || e->m.start_pos_in_letter>t->m.start_pos_in_letter) {
		   e->m.start_pos_in_token=t->m.start_pos_in_token;
		   e->m.start_pos_in_char=t->m.start_pos_in_char;
		   e->m.start_pos_in_letter=t->m.start_pos_in_letter;
	   }
   }
	tags=tags->next;
}
/* Finally, we adjust offsets with the base offset of the current sentence */
e->m.start_pos_in_token+=infos->tfst->offset_in_tokens;
e->m.end_pos_in_token+=infos->tfst->offset_in_tokens;
e->output=NULL;
e->match=NULL;
e->next=NULL;
}


/**
 * We replace dest by a copy of src, freeing dest's previous values.
 */
void replace_match(struct tfst_simple_match_list* dest,struct tfst_simple_match_list* src) {
dest->m.start_pos_in_token=src->m.start_pos_in_token;
dest->m.end_pos_in_token=src->m.end_pos_in_token;
dest->m.start_pos_in_char=src->m.start_pos_in_char;
dest->m.end_pos_in_char=src->m.end_pos_in_char;
dest->m.start_pos_in_letter=src->m.start_pos_in_letter;
dest->m.end_pos_in_letter=src->m.end_pos_in_letter;
if (dest->match!=NULL) {
   (dest->match->pointed_by)--;
}
/* src->match->pointed_by does not need to be changed */
dest->match=src->match;
if (dest->output!=NULL) free(dest->output);
dest->output=u_strdup(src->output);
}


/**
 * Adds a match to the global list of matches. The function takes into
 * account the match policy. For instance, we don't take [2;3] into account
 * if we are in longest match mode and if we already have [2;5].
 *
 * This function is derived from the 'add_match' one in 'Matches.cpp', but it differs because
 * in Locate, we know exactly where we are in the text, so that we can filter matches easily. When
 * exploring a text automaton, it's not so easy to sort matches, because the state from where we
 * started the match may correspond to a position in text lower than the one of a previous match.
 *
 * IMPORTANT: 'e' is to be copied if the corresponding match must be added to the list
 */
struct tfst_simple_match_list* add_element_to_list(struct locate_tfst_infos* p,struct tfst_simple_match_list* list,struct tfst_simple_match_list* e) {
if (list==NULL) {
	/* We can always add a match to the empty list */
	return new_tfst_simple_match_list(e,NULL);
}
switch (compare_matches(&(list->m),&(e->m))) {
   case A_BEFORE_B:
   case A_BEFORE_B_OVERLAP: {
	   /* If the candidate starts after the end of the current match, then we have to go on,
	    * no matter the mode (shortest, longest or all matches) */
	   list->next=add_element_to_list(p,list->next,e);
	   return list;
   }

   case A_INCLUDES_B: {
	   if (p->match_policy==SHORTEST_MATCHES) {
		   /* e must replace the current match in the list */
		   replace_match(list,e);
		   return list;
	   } else if (p->match_policy==LONGEST_MATCHES) {
		   /* Our match is shorter than a match in the list, we discard it */
		   return list;
	   } else {
		   list->next=add_element_to_list(p,list->next,e);
		   return list;
	   }
   }

   case A_EQUALS_B: {
	   /* In any mode we replace the existing match by the new one, except if we allow
	    * ambiguous outputs */
	   if (u_strcmp(list->output,e->output)) {
		   if (p->ambiguous_output_policy==ALLOW_AMBIGUOUS_OUTPUTS) {
			   list=new_tfst_simple_match_list(e,list);
			   return list;
		   } else {
				/* If we don't allow ambiguous outputs, we have to print an error message */
				error("Unexpected ambiguous outputs:\n<%S>\n<%S>\n",list->output,e->output);
		   }
	   }
	   replace_match(list,e);
	   return list;
   }

   case B_INCLUDES_A: {
	   if (p->match_policy==SHORTEST_MATCHES) {
		   /* Our match is longer than a match in the list, we discard it */
		   return list;
   	   } else if (p->match_policy==LONGEST_MATCHES) {
   		   /* e must replace the current match in the list */
   		   replace_match(list,e);
   		   return list;
   	   } else {
   		   list->next=add_element_to_list(p,list->next,e);
   		   return list;
   	   }
   }

   case A_AFTER_B:
   case A_AFTER_B_OVERLAP: {
	   /* If the candidate ends before the start of the current match, then we have to insert it
	    * no matter the mode (shortest, longest or all matches) */
	   list=new_tfst_simple_match_list(e,list);
	   return list;
   }
}
/* Should not arrive here */
return NULL;
}


/**
 * Saves the elements of 'list' in reverse order into 'items'.
 */
static void fill_vector(vector_ptr* items,struct tfst_match* list) {
if (list==NULL) return;
fill_vector(items,list->next);
vector_ptr_add(items,list);
}


static int is_variable_char(unichar c) {
return ((c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') || c=='_');
}

/**
 * Returns 1 if output is of the form $:XYZ$ and saves XYZ in name; 0 otherwise.
 */
int is_capture_variable(unichar* output,unichar* name) {
if (output==NULL || output[0]!='$' || output[1]!=':') return 0;
int i=2,j=0;
while (is_variable_char(output[i])) {
	name[j++]=output[i++];
}
if (output[i]!='$' || output[i+1]!='\0') return 0;
name[j]='\0';
return 1;
}


/**
 * Performs the variable capture. Returns 1 in case of success; 0 otherwise.
 * Note that 1 is also returned if there is an error while the variable error
 * policy is IGNORE_VARIABLE_ERROR.
 */
int do_variable_capture(int tfst_tag_number, int fst2_tag_number,
		struct locate_tfst_infos* infos, unichar* name) {
if (tfst_tag_number == -1) {
	/* If we have a text independent match like <E>/$:X$, it's an error case */
	switch (infos->variable_error_policy) {
		case EXIT_ON_VARIABLE_ERRORS:
			fatal_error(
				"Should not have capture variable $:%S$ associated to text independent input %S\n",
				name, infos->fst2->tags[fst2_tag_number]->input);
		case IGNORE_VARIABLE_ERRORS: return 1;
		case BACKTRACK_ON_VARIABLE_ERRORS: return 0;
	}
}
TfstTag* tag = (TfstTag*) (infos->tfst->tags->tab[tfst_tag_number]);
if (tag->content[0] != '{' || tag->content[1] == '\0') {
	/* If we have a non tagged token like "foo" */
	switch (infos->variable_error_policy) {
		case EXIT_ON_VARIABLE_ERRORS:
			fatal_error(
				"Should not have capture variable $:%S$ associated to a tag that may capture untagged tokens: %S\n",
				name, infos->fst2->tags[fst2_tag_number]->input);
		case IGNORE_VARIABLE_ERRORS: return 1;
		case BACKTRACK_ON_VARIABLE_ERRORS: return 0;
	}
}
/* We can capture the tag */
struct dela_entry* e=tokenize_tag_token(tag->content);
if (e==NULL) {
	/* Should not happen */
	fatal_error("Unexpected tag tokenization error in do_variable_capture for tag:\n%S\n",tag->content);
}
set_dic_variable(name,e,&(infos->dic_variables),0);
return 1;
}


/**
 * Explores all the partial matches to produce outputs in MERGE or REPLACE mode.
 * 
 * If *var_starts!=NULL, it means that there are pending $var_start( tags
 * that wait for being taken into account when a text dependent tag is found.
 */
void explore_match_for_MERGE_or_REPLACE_mode(struct locate_tfst_infos* infos,
                                  struct tfst_simple_match_list* element,
                                  vector_ptr* items,int current_item,Ustring* s,
                                  int last_text_dependent_tfst_tag,
                                  struct list_pointer* *var_starts) {
if (current_item==items->nbelems) {
   /* If we have finished, we can save the current output */
   element->output=s->str;
   infos->matches=add_element_to_list(infos,infos->matches,element);
   element->output=NULL;
   return;
}
/* We save the length because it will be modified */
int len=s->len;
struct tfst_match* item=(struct tfst_match*)(items->tab[current_item]);
if (item==NULL) {
   fatal_error("Unexpected NULL item in explore_match_for_MERGE_mode\n");
}

unichar* output=infos->fst2->tags[item->fst2_transition->tag_number]->output;

unichar name[MAX_TRANSDUCTION_VAR_LENGTH];
int capture;
struct dela_entry* old_value_dela=NULL;
capture=is_capture_variable(output,name);
if (capture) {
	/* If we have a capture variable $:X$, we must save the previous value
	 * for this dictionary variable */
	old_value_dela=clone_dela_entry(get_dic_variable(name,infos->dic_variables));
}

Match saved_element=element->m;
struct list_int* text_tags=item->text_tag_numbers;
int captured_chars=0;
/* We explore all the text tags */
while (text_tags!=NULL) {
   /* First, we restore the output string */
   s->len=len;
   s->str[len]='\0';
   captured_chars=0;
   /* We deal with the fst2 tag output, if any */
   if (item->first_time) {
	   /* We only have to process the output only once,
	    * since it will have the same effect on all tfst tags.
	    *
	    * Example: the fst2 tag "cybercrime/ZZ" may match the two tfst tags "cyber" and
	    * "crime", but we must process the "ZZ" output only before the first tfst tag "cyber" */
	   if (capture) {
		   /* If we have a capture variable, then we have to check whether the tfst tag
	   	    * is a tagged token or not */
	   	   int tfst_tag_number=text_tags->n;
	   	   int fst2_tag_number=item->fst2_transition->tag_number;
	   	   if (!do_variable_capture(tfst_tag_number,fst2_tag_number,infos,name)) {
	   		   goto restore_dic_variable;
	   	   }
	   } else if (!deal_with_output_tfst(s,output,infos,&captured_chars)) {
         /* We do not take into account matches with variable errors if the
          * process_output_for_tfst_match function has decided that backtracking
          * was necessary, either because of a variable error of because of a
          * $a.SET$ or $a.UNSET$ test */
		  goto restore_dic_variable;
      }
   }
   int last_tag=last_text_dependent_tfst_tag;
   TfstTag* current_tag=NULL;
   if (text_tags->n==-1) {
      /* We have a text independent match */
      Fst2Tag fst2_tag=infos->fst2->tags[item->fst2_transition->tag_number];
      if (fst2_tag->type==BEGIN_OUTPUT_VAR_TAG) {
    	  /* If we an output variable start $|a( */
    	  set_output_variable_pending(infos->output_variables,fst2_tag->variable);
          explore_match_for_MERGE_or_REPLACE_mode(infos,element,items,current_item+1,s,last_tag,var_starts);
          unset_output_variable_pending(infos->output_variables,fst2_tag->variable);
          goto restore_dic_variable;
      } else if (fst2_tag->type==END_OUTPUT_VAR_TAG) {
    	  /* If we an output variable end $|a) */
    	  unset_output_variable_pending(infos->output_variables,fst2_tag->variable);
    	  explore_match_for_MERGE_or_REPLACE_mode(infos,element,items,current_item+1,s,last_tag,var_starts);
          set_output_variable_pending(infos->output_variables,fst2_tag->variable);
          goto restore_dic_variable;
      } else if (fst2_tag->type==BEGIN_VAR_TAG) {
         /* If we have a variable start tag $a(, we add it to our 
          * variable tag list */
         struct transduction_variable* v=get_transduction_variable(infos->input_variables,fst2_tag->variable);
         int old_value=v->start_in_tokens;
         /* We add the address of the start field to our list */
         (*var_starts)=new_list_pointer(&(v->start_in_tokens),(var_starts==NULL)?NULL:(*var_starts));
         /* Then, we go on the next item */
         explore_match_for_MERGE_or_REPLACE_mode(infos,element,items,current_item+1,s,last_tag,var_starts);
         /* After the exploration, there are 2 cases:
          * 1) *var_starts is NULL: nothing to do
          * 2) *var_starts is not NULL: we reached the end of the items without findind any
          *                             text dependent match, so we can free the list */
         free_list_pointer(*var_starts);
         (*var_starts)=NULL;
         v->start_in_tokens=old_value;
         /* If we have a $a( tag, we know that we can only have just one text tag 
          * with special value -1 */
         goto restore_dic_variable;
      } else if (fst2_tag->type==END_VAR_TAG) {
         /* If we have found a $a) tag */
         if (last_tag==-1) {
            /* If we have no tfst tag to use, then it's a variable definition error,
             * and we have nothing special to do */
            explore_match_for_MERGE_or_REPLACE_mode(infos,element,items,current_item+1,s,last_tag,var_starts);
            goto restore_dic_variable;
         } else {
            /* We can set the end of the variable, it's 'last_tag' */
            struct transduction_variable* v=get_transduction_variable(infos->input_variables,fst2_tag->variable);
            int old_value=v->end_in_tokens;
            v->end_in_tokens=last_tag;
            explore_match_for_MERGE_or_REPLACE_mode(infos,element,items,current_item+1,s,last_tag,var_starts);
            v->end_in_tokens=old_value;
            goto restore_dic_variable;
         }
      } else if (fst2_tag->type==LEFT_CONTEXT_TAG) {
         /* If we have found a $* tag, we must reset the stack string and the 
          * start position, so we save them */
         unichar* old_stack=u_strdup(s->str);
         int old_pos_token=element->m.start_pos_in_token;
         int old_pos_char=element->m.start_pos_in_char;
         int old_pos_letter=element->m.start_pos_in_letter;
         /* We set the new values */
         empty(s);
         element->m.start_pos_in_token=LEFT_CONTEXT_PENDING;
         /* We must reset last_tag to -1, because is not, we will have an 
          * extra space on the left of the match */
         explore_match_for_MERGE_or_REPLACE_mode(infos,element,items,current_item+1,s,-1,var_starts);
         
         /* And we restore previous values */
         element->m.start_pos_in_token=old_pos_token;
         element->m.start_pos_in_char=old_pos_char;
         element->m.start_pos_in_letter=old_pos_letter;
         u_strcpy(s,old_stack);
         free(old_stack);
         /* If we have a $* tag, we know that we can only have just one text tag 
          * with special value -1 */
         goto restore_dic_variable;
      }
   } else {
      current_tag=(TfstTag*)(infos->tfst->tags->tab[text_tags->n]);
      /* We update the last tag */
      last_tag=text_tags->n;
      /* If the current text tag is not a text independent one */
      
      /* If there are some pending $a( tags, we set them to the current tag */
      if (var_starts!=NULL) {
         struct list_pointer* ptr=(*var_starts);
         while (ptr!=NULL) {
            int* start=(int*)(ptr->pointer);
            (*start)=text_tags->n;
            ptr=ptr->next;
         }
      }
      int previous_start_token,previous_start_char; 
      if (last_text_dependent_tfst_tag!=-1) {
         /* If the item is not the first, we must insert the original text that is
          * between the end of the previous merged text and the beginning of the
          * current one, typically to insert spaces */
         TfstTag* previous_tag=(TfstTag*)(infos->tfst->tags->tab[last_text_dependent_tfst_tag]);
         previous_start_token=previous_tag->m.end_pos_in_token;
         previous_start_char=previous_tag->m.end_pos_in_char;
         /* We start just after the end of the previous match */
         if (infos->tfst->token_content[previous_start_token][previous_start_char+1]!='\0') {
            /* If we were not at the end of the previous text token, we just inscrease
             * the char position */
            previous_start_char++;
         } else {
            /* Otherwise, we go on the next token */
            previous_start_token++;
            previous_start_char=0;
         }
      } else {
         /* Otherwise, we start on the beginning of the current text tag */
         //error("current item=%d\n",text_tags->n);
         previous_start_token=current_tag->m.start_pos_in_token;
         previous_start_char=current_tag->m.start_pos_in_char;
      }
      /* Here we have to insert the text that is between current_start and current_end,
       * and then, the ouput of the fst2 transition */
      if (infos->output_policy==MERGE_OUTPUTS) {
    	  insert_text_interval_tfst(infos,s,previous_start_token,previous_start_char,
                 current_tag->m.end_pos_in_token,current_tag->m.end_pos_in_char);
      }
   }
   /* Then, we go on the next item */
   struct list_pointer* ptr2=NULL;
   if (element->m.start_pos_in_token==LEFT_CONTEXT_PENDING && current_tag!=NULL) {
      element->m.start_pos_in_token=infos->tfst->offset_in_tokens+current_tag->m.start_pos_in_token;
      element->m.start_pos_in_char=current_tag->m.start_pos_in_char;
      element->m.start_pos_in_letter=current_tag->m.start_pos_in_letter;
   }
   explore_match_for_MERGE_or_REPLACE_mode(infos,element,items,current_item+1,s,last_tag
         ,&ptr2 /* We have encountered a text dependent tag, so there is no
                 * more pending start tag like $a( */
         );
   element->m=saved_element;
   /* If there was a $* tag pending */
   free_list_pointer(ptr2);
   if (infos->ambiguous_output_policy==IGNORE_AMBIGUOUS_OUTPUTS) {
      /* If we don't want ambiguous outputs, then the first path is
       * enough for our purpose */ 
      goto restore_dic_variable;
   }
   text_tags=text_tags->next;
   remove_chars_from_output_variables(infos->output_variables,captured_chars);
   /* We reset to 0, because if we exit the while normally, we don't want to
    * modify output variables twice when reaching the 'restore_dic_variable'
    * label */
   captured_chars=0;
}
restore_dic_variable:
/* We redo this about output variables here, since we may have jumped here directly */
remove_chars_from_output_variables(infos->output_variables,captured_chars);
if (capture) {
	/* If we have a capture variable $:X$, we must restore the previous value
	 * for this dictionary variable */
	set_dic_variable(name,old_value_dela,&(infos->dic_variables),0);
}
}


/**
 * This function explores the partial matches that constitute the given match in order to produce
 * one or all possible outputs, depending on infos->ambiguous_output_policy.
 * The output(s) is(are) then used to add matches to the infos->matches list.
 */
void explore_match_to_get_outputs(struct locate_tfst_infos* infos,struct tfst_match* m,
		                          struct tfst_simple_match_list* element) {
/* As m is a reversed list, we first need to get its elements in the right order */
vector_ptr* items=new_vector_ptr(16);
fill_vector(items,m);
Ustring* s=new_Ustring(1024);
/* In MERGE/REPLACE mode, we have to explore the combination of partial matches */
struct list_pointer* ptr=NULL;
explore_match_for_MERGE_or_REPLACE_mode(infos,element,items,0,s,-1,&ptr);
free_list_pointer(ptr);
free_Ustring(s);
free_vector_ptr(items);
}


/**
 * This function takes a tfst_match list that represents a match. It turns it into
 * a tfst_simple_match_list element and inserts it into to the main tfst_simple_match_list,
 * according to the match filtering rules.
 */
void add_tfst_match(struct locate_tfst_infos* infos,struct tfst_match* m) {
struct tfst_simple_match_list element;
fill_element(infos,&element,m);
if (element.m.end_pos_in_token==-1) {
   /* If the match was in fact completely text independent, then we reject it */
   return;
}
//error("match from token %d.%d to %d.%d\n",element.m.start_pos_in_token,element.m.start_pos_in_char,
//      element.m.end_pos_in_token,element.m.end_pos_in_char);
if (infos->output_policy==IGNORE_OUTPUTS) {
	/* The simplest case */
	infos->matches=add_element_to_list(infos,infos->matches,&element);
} else {
	explore_match_to_get_outputs(infos,m,&element);
}
}


/**
 * This function saves the current match list in the concordance index file.
 * It is derived from the 'save_matches' from 'Matches.cpp'. At the opposite of
 * 'save_matches', this function is not parameterized by the current position in
 * the text. We assume that this function is called once per sentence automaton, after
 * all matches have been computed.
 */
void save_tfst_matches(struct locate_tfst_infos* p) {
struct tfst_simple_match_list* l=p->matches;
struct tfst_simple_match_list* ptr;
if (p->number_of_matches==p->search_limit) {
	/* If we have reached the limit, then we must free all the remaining matches */
	while (l!=NULL) {
		ptr=l;
		l=l->next;
		free_tfst_simple_match_list(ptr);
	}
	p->matches=NULL;
	return;
}
U_FILE* f=p->output;
if (l==NULL) return;
u_fprintf(f,"%d.%d.%d %d.%d.%d",l->m.start_pos_in_token,l->m.start_pos_in_char,
      l->m.start_pos_in_letter,l->m.end_pos_in_token,
      l->m.end_pos_in_char,l->m.end_pos_in_letter);
if (l->output!=NULL) {
	/* If there is an output */
	u_fprintf(f," %S",l->output);
}
u_fprintf(f,"\n");
if (p->ambiguous_output_policy==ALLOW_AMBIGUOUS_OUTPUTS) {
   (p->number_of_outputs)++;
   if (!(p->start_position_last_printed_match_token == l->m.start_pos_in_token
         && p->start_position_last_printed_match_char == l->m.start_pos_in_char
         && p->start_position_last_printed_match_letter == l->m.start_pos_in_letter
	      && p->end_position_last_printed_match_token == l->m.end_pos_in_token
	      && p->end_position_last_printed_match_char == l->m.end_pos_in_char
	      && p->end_position_last_printed_match_letter == l->m.end_pos_in_letter)) {
	   (p->number_of_matches)++;
   }
} else {
	/* If we don't allow ambiguous outputs, we count the matches */
	(p->number_of_matches)++;
}
p->start_position_last_printed_match_token=l->m.start_pos_in_token;
p->end_position_last_printed_match_token=l->m.end_pos_in_token;
p->start_position_last_printed_match_char=l->m.start_pos_in_char;
p->end_position_last_printed_match_char=l->m.end_pos_in_char;
p->start_position_last_printed_match_letter=l->m.start_pos_in_letter;
p->end_position_last_printed_match_letter=l->m.end_pos_in_letter;
if (p->number_of_matches==p->search_limit) {
	/* If we have reached the search limitation, we free the remaining
	 * matches and return */
	while (l!=NULL) {
		ptr=l;
		l=l->next;
		free_tfst_simple_match_list(ptr);
	}
	p->matches=NULL;
	return;
}
ptr=l->next;
free_tfst_simple_match_list(l);
p->matches=ptr;
save_tfst_matches(p);
return;
}
