 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Matches.h"
#include "Error.h"



/**
 * Allocates, initializes and returns a new match list element.
 */
struct match_list* new_match(int start,int end,unichar* output,struct match_list* next) {
struct match_list *l;
l=(struct match_list*)malloc(sizeof(struct match_list));
if (l==NULL) {
   fatal_error("Not enough memory in new_match\n");
}
l->start=start;
l->end=end;
if (output==NULL) {
   l->output=NULL;
} else {
   l->output=u_strdup(output);
}
l->next=next;
return l;
}


/**
 * Frees a single match list element.
 */
void free_match_list_element(struct match_list* l) {
if (l==NULL) return;
if (l->output!=NULL) free(l->output);
free(l);
}


/** 
 * Adds a match to the global list of matches. The function takes into
 * account the match policy. For instance, we don't take [2;3] into account
 * if we are in longest match mode and if we already have [2;5].
 * 
 * # Changed to allow different outputs in merge/replace
 * mode when the grammar is an ambiguous transducer (S.N.) */
void add_match(int end,unichar* output,struct locate_parameters* p) {
int start=p->current_origin+p->absolute_offset;
struct match_list *l;
if (p->match_list==NULL) {
   /* If the match list was empty, we always can put the match in the list */
   p->match_list=new_match(start,end,output,NULL);
   return;
}
switch (p->match_policy) {
   case LONGEST_MATCHES:
      /* We put new matches at the beginning of the list */
      if (end>p->match_list->end) {
         /* In longest match mode, we only consider matches ending
          * later. Moreover, we allow just one match from a given
          * start position, except if ambiguous outputs are allowed. */
         if (p->match_list->start==start) {
            /* We overwrite matches starting at same position but ending earlier.
             * We do this by deleting the actual head element of the list
             * and calling the function recursively.
             * This works also for different outputs from ambiguous transducers,
             * i.e. we may delete more than one match in the list. */
            l=p->match_list;
            p->match_list=p->match_list->next;
            free_match_list_element(l);
            add_match(end,output,p);
            return;
         }
         /* We allow add shorter matches but with other start position.
          * Note that, by construction, we have start>p->match_list->start,
          * that is to say that we have two matches that overlap: ( [ ) ]
          */
         p->match_list=new_match(start,end,output,p->match_list);
         return;
      }
      /* If we have the same start and the same end, we consider the
       * new match only if ambiguous outputs are allowed */
      if (p->ambiguous_output_policy==ALLOW_AMBIGUOUS_OUTPUTS
         && p->match_list->end==end
         && p->match_list->start==start
         && u_strcmp(p->match_list->output,output)) {
         /* Because matches with same range and same output may not come
          * one after another, we have to look if a match with same output
          * already exists */
         l=p->match_list;
         while (l!=NULL && u_strcmp(l->output,output)) {
            l=l->next;
         }
         if (l==NULL) {
            p->match_list=new_match(start,end,output,p->match_list);
         }
      }
      break;
       
   case ALL_MATCHES:
      /* We put new matches at the beginning of the list */
      l=p->match_list;
      /* We unify identical matches, i.e. matches with same range (start and end),
       * taking the output into account if ambiguous outputs are allowed. */
      while (l!=NULL 
             && !(l->start==start && l->end==end 
                  && (p->ambiguous_output_policy!=ALLOW_AMBIGUOUS_OUTPUTS
                      || u_strcmp(l->output,output)))) {
         l=l->next;
      }
      if (l==NULL) {
         p->match_list=new_match(start,end,output,p->match_list);
      }
      break;

   case SHORTEST_MATCHES:
      /* We put the new match at the beginning of the list, but before, we
       * test if the match we want to add may not be discarded because of
       * a shortest match that would already be in the list. By the way,
       * we eliminate matches that are longer than this one, if any. */
      int dont_add_match=0;
      p->match_list=eliminate_longer_matches(p->match_list,start,end,output,&dont_add_match,p);
      if (!dont_add_match) {
         p->match_list=new_match(start,end,output,p->match_list);
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
struct match_list* eliminate_longer_matches(struct match_list *ptr,
                                            int start,int end,unichar* output,
                                            int *dont_add_match,
                                            struct locate_parameters* p) {
struct match_list *l;
if (ptr==NULL) return NULL;
if (p->ambiguous_output_policy==ALLOW_AMBIGUOUS_OUTPUTS
    && ptr->start==start && ptr->end==end
    && u_strcmp(ptr->output,output)) {
    /* In the case of ambiguous transductions producing different outputs,
     * we accept matches with same range */
   ptr->next=eliminate_longer_matches(ptr->next,start,end,output,dont_add_match,p);
   return ptr;
}
if (start>=ptr->start && end<=ptr->end) {
   /* If the new match is shorter (or of equal length) than the current one
    * in the list, we replace the match in the list by the new one */
   if (*dont_add_match) {
      /* If we have already noticed that the match mustn't be added
       * to the list, we delete the current list element */
      l=ptr->next;
      free_match_list_element(ptr);
      return eliminate_longer_matches(l,start,end,output,dont_add_match,p);
    }
    /* If the new match is shorter than the current one in the list, then we
     * update the current one with the value of the new match. */
    ptr->start=start;
    ptr->end=end;
    if (ptr->output!=NULL) free(ptr->output);
    ptr->output=u_strdup(output);
    /* We note that the match does not need anymore to be added */
    (*dont_add_match)=1;
    ptr->next=eliminate_longer_matches(ptr->next,start,end,output,dont_add_match,p);
    return ptr;
}
if (start<=ptr->start && end>=ptr->end) {
   /* The new match is longer than the one in list => we 
    * skip the new match */
   (*dont_add_match)=1;
   return ptr;
}
/* If we have disjunct ranges or overlapping ranges without inclusion,
 * we examine recursively the rest of the list */
ptr->next=eliminate_longer_matches(ptr->next,start,end,output,dont_add_match,p);
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
struct match_list* save_matches(struct match_list* l,int current_position,
                                FILE* f,struct locate_parameters* p) {
struct match_list *ptr;
if (l==NULL) return NULL;
if (l->end<current_position) {
   /* we can save the match (necessary for SHORTEST_MATCHES: there
    * may be no shorter match) */
   u_fprintf(f,"%d %d",l->start,l->end);
   if (l->output!=NULL) {
      /* If there is an output */
      u_fprintf(f," %S",l->output);
   }
   u_fprintf(f,"\n");
   if (p->ambiguous_output_policy==ALLOW_AMBIGUOUS_OUTPUTS) {
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
      if (!(p->start_position_last_printed_match == l->start
            && p->end_position_last_printed_match == l->end)) {
         (p->number_of_matches)++;
      }
   } else {
      /* If we don't allow ambiguous outputs, we count the matches */
      (p->number_of_matches)++;
   }
   // To count the number of matched tokens this won't work: 
   //  p->matching_units=p->matching_units+(l->end+1)-(l->start);
   // or you get ouputs like:
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
   // I guess an bit array is needed to count correctly.
   // But since for "Longest matches" only Z, and for "Shortest" only X and Y are
   // accepted, and the option "All matches" is rarely used, I (S.N.) propose:
   if (p->end_position_last_printed_match != current_position-1) {
      // initial (non-recursive) call of function:
      // then check if match is out of range of previous matches
      if (p->end_position_last_printed_match < l->start) { // out of range
         p->matching_units += (l->end+1)-(l->start);
      } else {
         p->matching_units += (l->end+1)-(p->end_position_last_printed_match+1);
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
      if (p->start_position_last_printed_match > l->start) {
         p->matching_units += p->start_position_last_printed_match-(l->start);
      }
   }
   p->start_position_last_printed_match=l->start;
   p->end_position_last_printed_match=l->end;
   if (p->number_of_matches==p->search_limit) {
      /* If we have reached the search limitation, we free the remaining
       * matches and return */
      while (l!=NULL) {
         ptr=l;
         l=l->next;
         free_match_list_element(ptr);
      }
      return NULL;
   }
   ptr=l->next;
   free_match_list_element(l);
   return save_matches(ptr,current_position,f,p);
}
l->next=save_matches(l->next,current_position,f,p);
return l;
}


/**
 * Loads a match list. Match lists are supposed to have been 
 * generated by the Locate program.
 */
struct match_list* load_match_list(FILE* f,OutputPolicy *output_policy) {
struct match_list* l=NULL;
struct match_list* end_of_list=NULL;
int c,start,end;
unichar output[3000];
char is_an_output;
/* We read the header */
u_fscanf(f,"#%C\n",&c);
if (output_policy!=NULL) {
   switch(c) {
      case 'I': *output_policy=IGNORE_OUTPUTS; break;
      case 'M': *output_policy=MERGE_OUTPUTS; break;
      case 'R': *output_policy=REPLACE_OUTPUTS; break;
	}
}
while (2==u_fscanf(f,"%d%d",&start,&end)) {
   /* We look if there is an output or not, i.e. a space or a new line */
   c=u_fgetc(f);
   if (c==' ') {
      /* If we have an output to read */
      int i=0;
      while ((c=u_fgetc(f))!='\n') {
         output[i++]=(unichar)c;
      }
      output[i]='\0';
      is_an_output=(i!=0);
   } else {
      is_an_output=0;
   }
   if (l==NULL) {
      l=new_match(start,end,is_an_output?output:NULL,NULL);
      end_of_list=l;
   } else {
      end_of_list->next=new_match(start,end,is_an_output?output:NULL,NULL);
      end_of_list=end_of_list->next;
   }
}
return l;
}


