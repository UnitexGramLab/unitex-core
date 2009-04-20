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

#include "Tfst.h"
#include "Fst2.h"
#include "Alphabet.h"
#include "LocateTfstMatches.h"
#include "ConcordanceTfst.h"
#include "DELA.h"
#include "Pattern.h"


#define OK_MATCH_STATUS 1
#define NO_MATCH_STATUS 2
#define TEXT_INDEPENDENT_MATCH 3


void explore_tfst(Tfst* tfst,Fst2* fst2,Alphabet* alph,int current_state_in_tfst,
		          int current_state_in_fst2,int depth,
		          struct tfst_match* match_element_list,
                  struct tfst_match_list** LIST,int *n_matches,U_FILE* output);

int match_between_text_and_grammar_tags(Tfst* tfst,TfstTag* text_tag,Fst2Tag grammar_tag,Alphabet* alphabet);
int same_codes(struct dela_entry* a,struct dela_entry* b);
struct pattern* tokenize_grammar_tag(unichar* tag,int *negation);
int is_space_on_the_left_in_tfst(Tfst* tfst,TfstTag* tag);


/**
 * This function applies the given grammar to the given text automaton.
 * It returns 1 in case of success; 0 otherwise.
 */
int locate_tfst(char* text,char* grammar,char* alphabet,char* output) {
Tfst* tfst=open_text_automaton(text);
if (tfst==NULL) {
	return 0;
}
Fst2* fst2=load_fst2(grammar,0);
if (fst2==NULL) {
	return 0;
}
Alphabet* alph=load_alphabet(alphabet);
if (alph==NULL) {
	error("Cannot load alphabet file: %s\n",alphabet);
	return 0;
}
U_FILE* out=u_fopen(UTF16_LE,output,U_WRITE);
if (out==NULL) {
	error("Cannot open %s\n",output);
}
int n_matches=0;
/* We launch the matching for each sentence */
for (int i=1;i<=tfst->N;i++) {
	load_sentence(tfst,i);
	compute_token_contents(tfst);
	/* Within a sentencce graph, we try to match from any state */
	for (int j=0;j<tfst->automaton->number_of_states;j++) {
		explore_tfst(tfst,fst2,alph,j,fst2->initial_states[1],0,NULL,NULL,&n_matches,out);
	}
}
/* And we free things */
u_fclose(out);
free_alphabet(alph);
free_Fst2(fst2);
close_text_automaton(tfst);
return 1;
}


/**
 * Explores in parallel the tfst and the fst2.
 */
void explore_tfst(Tfst* tfst,Fst2* fst2,Alphabet* alphabet,int current_state_in_tfst,
		          int current_state_in_fst2,int depth,
		          struct tfst_match* match_element_list,
                  struct tfst_match_list* *LIST,
                  int *n_matches,U_FILE* output) {
Fst2State current_state_in_grammar=fst2->states[current_state_in_fst2];

if (is_final_state(current_state_in_grammar)) {
   /* If the current state is final */
   if (depth==0) {
      /* If we are in the main graph, we produce a concordance line */
      compute_tfst_match_concordance_line(tfst,match_element_list,output);
      (*n_matches)++;
   } else {
      /* If we are in a subgraph, we add a match to the current match list */
      (*LIST)=add_match_in_list((*LIST),match_element_list);
   }
}

/* We test every transition that goes out the current state in the grammar */
Transition* grammar_transition=current_state_in_grammar->transitions;
while (grammar_transition!=NULL) {
   int e=grammar_transition->tag_number;
   if (e<0) {
      /* Case of a subgraph */
      struct tfst_match_list* list_for_subgraph=NULL;
      struct tfst_match_list* tmp;

      explore_tfst(tfst,fst2,alphabet,
    		  current_state_in_tfst,fst2->initial_states[-e],
              depth+1,match_element_list,&list_for_subgraph,n_matches,output);
      while (list_for_subgraph!=NULL) {
         tmp=list_for_subgraph->next;
         /* Before exploring an element that points on a subgraph match,
          * we decrease its 'pointed_by' variable that was previously increased
          * in the 'add_match_in_list' function */
         (list_for_subgraph->match->pointed_by)--;
         explore_tfst(tfst,fst2,alphabet,
                           list_for_subgraph->match->dest_state_text,
                           grammar_transition->state_number,
                           depth,list_for_subgraph->match,LIST,n_matches,output);
         /* Finally, we remove, if necessary, the list of match element
          * that was used for storing the subgraph match. This cleaning
          * will only free elements that are not involved in others
          * matches, that is to say element with pointed_by=0 */
         clean_tfst_match_list(list_for_subgraph->match,match_element_list);
         free(list_for_subgraph);
         list_for_subgraph=tmp;
      }
   }
   else {
      /* Normal case (not a subgraph call) */
      struct tfst_match* list=NULL;
      Transition* text_transition=tfst->automaton->states[current_state_in_tfst]->outgoing_transitions;
      /* For a given tag in the grammar, we test all the transitions in the
       * text automaton, and we note all the states we can reach */
      while (text_transition!=NULL) {
         int result=match_between_text_and_grammar_tags(tfst,(TfstTag*)(tfst->tags->tab[text_transition->tag_number]),
                                                 fst2->tags[grammar_transition->tag_number],alphabet);
         if (result==OK_MATCH_STATUS) {
            /* Case of a match with something in the text automaton (i.e. <V>) */
            list=insert_in_tfst_matches(list,current_state_in_tfst,text_transition->state_number,
                 grammar_transition->tag_number,text_transition->tag_number);
         }
         else if (result==TEXT_INDEPENDENT_MATCH) {
            /* Case of a match independent of the text automaton (i.e. <E>) */
            list=insert_in_tfst_matches(list,current_state_in_tfst,current_state_in_tfst,
                 grammar_transition->tag_number,NO_TEXT_TOKEN_WAS_MATCHED);
         }
         text_transition=text_transition->next;
      }
      struct tfst_match* tmp;
      /* Then, we continue the exploration from the reached states. This
       * procedure avoids exploring several times a same state when
       * it can be reached through various tags in the text automaton.
       * For instance, if we have "le" in the grammar and {le,.DET} and
       * {le,.PRO} in the text automaton that point to the same state XXX,
       * we will explore XXX just once. */
      while (list!=NULL) {
         tmp=list->next;
         list->next=match_element_list;
         /* match_element_list is pointed by one more element */
         if (match_element_list!=NULL) {(match_element_list->pointed_by)++;}
         explore_tfst(tfst,fst2,alphabet,list->dest_state_text,grammar_transition->state_number,
                           depth,list,LIST,n_matches,output);
         if (list->pointed_by==0) {
            /* If list is not blocked by being part of a match for the calling
             * graph, we can free it */
            list->next=NULL;
            if (match_element_list!=NULL) {(match_element_list->pointed_by)--;}
            free_tfst_match(list);
         }
         list=tmp;
      }
   }
   grammar_transition=grammar_transition->next;
}
}



/**
 * This function tests if a text tag can be matched by a grammar tag.
 */
int match_between_text_and_grammar_tags(Tfst* tfst,TfstTag* text_tag,Fst2Tag grammar_tag,Alphabet* alphabet) {
/**************************************************
 * We want to match something that is text independent */
if (grammar_tag->type==BEGIN_POSITIVE_CONTEXT_TAG
	|| grammar_tag->type==BEGIN_NEGATIVE_CONTEXT_TAG
	|| grammar_tag->type==END_CONTEXT_TAG
	|| grammar_tag->type==LEFT_CONTEXT_TAG
	|| grammar_tag->type==BEGIN_MORPHO_TAG
	|| grammar_tag->type==END_MORPHO_TAG
	|| grammar_tag->type==BEGIN_VAR_TAG
	|| grammar_tag->type==END_VAR_TAG
    || !u_strcmp(grammar_tag->input,"<E>")) {
   return TEXT_INDEPENDENT_MATCH;
}
/* Here we test the special case of the " " and # tags */
if (!u_strcmp(grammar_tag->input," ")) {
	if (is_space_on_the_left_in_tfst(tfst,text_tag)) {
		return TEXT_INDEPENDENT_MATCH;
	}
	return NO_MATCH_STATUS;
}
if (!u_strcmp(grammar_tag->input,"#")) {
	if (!is_space_on_the_left_in_tfst(tfst,text_tag)) {
		return TEXT_INDEPENDENT_MATCH;
	}
	return NO_MATCH_STATUS;
}

/* Now, we will compare the text tag and the grammar one */

if (!u_strcmp(text_tag->content,"{STOP}")) {
	/* {STOP} can NEVER be matched */
	return NO_MATCH_STATUS;
}

struct dela_entry* grammar_entry=NULL;
struct dela_entry* text_entry=NULL;

/**************************************************
 * We want to match a token like "le" */
if (is_letter(grammar_tag->input[0],alphabet)) {
   if (is_letter(text_tag->content[0],alphabet)) {
      /* text= "toto" */
      if (grammar_tag->control & RESPECT_CASE_TAG_BIT_MASK) {
         /* If we must respect case */
         if (!u_strcmp(grammar_tag->input,text_tag->content)) {return OK_MATCH_STATUS;}
         else {return NO_MATCH_STATUS;}
      } else {
         /* If case does not matter */
         if (is_equal_or_uppercase(grammar_tag->input,text_tag->content,alphabet)) {return OK_MATCH_STATUS;}
         else {return NO_MATCH_STATUS;}
      }
   } else if (text_tag->content[0]=='{' && text_tag->content[1]!='\0') {
      /* text={toto,tutu.XXX} */
	  text_entry=tokenize_tag_token(text_tag->content);
	  if (text_entry==NULL) {
		  fatal_error("NULL text_entry error in match_between_text_and_grammar_tags\n");
	  }
	  if (grammar_tag->control & RESPECT_CASE_TAG_BIT_MASK) {
         /* If we must respect case */
		 if (!u_strcmp(grammar_tag->input,text_entry->inflected)) {
			 goto ok_match;
		 } else {
			 goto no_match;
		 }
      } else {
         /* If case does not matter */
    	 if (is_equal_or_uppercase(grammar_tag->input,text_entry->inflected,alphabet)) {
    		 goto ok_match;
    	 } else {
    		 goto no_match;
    	 }
      }
   }
   return NO_MATCH_STATUS;
}

/**************************************************
 * We want to match a tag like "{tutu,toto.XXX}" */
if (grammar_tag->input[0]=='{' && u_strcmp(grammar_tag->input,"{S}")) {
   if (text_tag->content[0]!='{') {
      /* If the text tag is not of the form "{tutu,toto.XXX}" */
      return NO_MATCH_STATUS;
   }
   grammar_entry=tokenize_tag_token(grammar_tag->input);
   text_entry=tokenize_tag_token(text_tag->content);
   if (!is_equal_or_uppercase(grammar_entry->inflected,text_entry->inflected,alphabet)) {
      /* We allow case variations on the inflected form :
       * if there is "{tutu,toto.XXX}" in the grammar, we want it
       * to match "{Tutu,toto.XXX}" in the text automaton */
	  goto no_match;
   }
   if (u_strcmp(grammar_entry->lemma,text_entry->lemma)) {
      /* If lemmas are different,we don't match */
	  goto no_match;
   }
   if (!same_codes(grammar_entry,text_entry)) {
      /* If grammatical, semantical and inflectional informations
       * are different we don't match*/
	  goto no_match;
   }
   goto ok_match;
}

/**************************************************
 * We want to match something like "<....>". The
 * <E> case has already been handled in text independent matchings */
if (grammar_tag->input[0]=='<' && grammar_tag->input[1]!='\0') {
	/* We tokenize the text tag, if we have one */
	if (text_tag->content[0]=='{' && text_tag->content[1]!='\0') {
	   text_entry=tokenize_tag_token(text_tag->content);
	}
   if (!u_strcmp(grammar_tag->input,"<MOT>")) {
      /* <MOT> matches a sequence of letters or a tag like {tutu,toto.XXX}, even
       * if 'tutu' is not made of characters */
      if (is_letter(text_tag->content[0],alphabet) || text_entry!=NULL) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<!MOT>")) {
      /* <!MOT> matches the opposite of <MOT> */
      if (!is_letter(text_tag->content[0],alphabet)
          && text_entry->inflected==NULL) {
    	  goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<MIN>")) {
      /* <MIN> matches a sequence of letters or a tag like {tutu,toto.XXX}
       * only made of lower case letters */
      if (is_sequence_of_lowercase_letters(text_tag->content,alphabet) ||
          (text_entry!=NULL && is_sequence_of_lowercase_letters(text_entry->inflected,alphabet))) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<!MIN>")) {
      /* <!MIN> matches a sequence of letters or a tag like {tutu,toto.XXX}
       * that is not only made of lower case letters */
      if ((is_letter(text_tag->content[0],alphabet) && !is_sequence_of_lowercase_letters(text_tag->content,alphabet))
          || (text_entry!=NULL && !is_sequence_of_lowercase_letters(text_entry->inflected,alphabet))) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<MAJ>")) {
      /* <MAJ> matches a sequence of letters or a tag like {TUTU,toto.XXX}
       * only made of upper case letters */
      if (is_sequence_of_uppercase_letters(text_tag->content,alphabet) ||
          (text_entry!=NULL && is_sequence_of_uppercase_letters(text_entry->inflected,alphabet))) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<!MAJ>")) {
      /* <!MAJ> matches a sequence of letters or a tag like {tutu,toto.XXX}
       * that is not only made of upper case letters */
      if ((is_letter(text_tag->content[0],alphabet) && !is_sequence_of_uppercase_letters(text_tag->content,alphabet))
          || (text_entry!=NULL && !is_sequence_of_uppercase_letters(text_entry->inflected,alphabet))) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<PRE>")) {
      /* <PRE> matches a sequence of letters or a tag like {TUTU,toto.XXX}
       * that begins by an upper case letter */
      if (is_upper(text_tag->content[0],alphabet) ||
          (text_entry!=NULL && is_upper(text_entry->inflected[0],alphabet))) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<!PRE>")) {
      /* <!PRE> matches a sequence of letters or a tag like {TUTU,toto.XXX}
       * that does not begin by an upper case letter */
      if ((is_letter(text_tag->content[0],alphabet) && !is_upper(text_tag->content[0],alphabet)) ||
          (text_entry!=NULL && is_letter(text_entry->inflected[0],alphabet) && !is_upper(text_entry->inflected[0],alphabet))) {
         /* We test is_letter+!is_upper instead of is_lower in order to handle
          * cases where there is no difference between lower and upper case letters
          * (for instance Thai) */
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<TOKEN>")) {
      /* <TOKEN> matches anything but the {STOP} tag, and the {STOP} tag case
       * has already been dealt with */
      goto ok_match;
   }
   if (!u_strcmp(grammar_tag->input,"<DIC>")) {
      /* <DIC> matches any tag like {tutu,toto.XXX} */
      if (text_entry!=NULL) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<!DIC>")) {
      /* <DIC> matches any sequence of letters that is not a tag like {tutu,toto.XXX} */
      if (is_letter(text_tag->content[0],alphabet)) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<SDIC>")) {
      /* <SDIC> matches any tag like {tutu,toto.XXX} where tutu is simple word
       *
       * NOTE: according to the formal definition of simple words, something like
       *       {3,.NUMBER} is not considered as a simple word since 3 is not
       *       a letter. It will be considered as a compound word */
      if (text_entry!=NULL && is_sequence_of_letters(text_entry->inflected,alphabet)) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<CDIC>")) {
      /* <CDIC> matches any tag like {tutu,toto.XXX} where tutu is a compound word
       *
       * NOTE: see <SDIC> note */
      if (text_entry!=NULL && !is_sequence_of_letters(text_entry->inflected,alphabet)) {
         goto ok_match;
      }
      goto no_match;
   }
   if (!u_strcmp(grammar_tag->input,"<NB>")) {
      /* <NB> matches any tag like 3 or {37,toto.XXX} */
      if (u_are_digits(text_tag->content) ||
          (text_entry!=NULL && u_are_digits(text_entry->inflected))) {
         goto ok_match;
      }
      goto no_match;
   }
   /**************************************************************
    * If we arrive here, we are in the case of a pattern.
    * We will handle several cases, but only if the text contains
    * a tag like {tutu,toto.XXX} */
   if (text_entry==NULL) {
      goto no_match;
   }
   /* First, we tokenize the grammar pattern tag */
   int negation;
   struct pattern* pattern=tokenize_grammar_tag(grammar_tag->input,&negation);
   int ok=is_entry_compatible_with_pattern(text_entry,pattern);
   free_pattern(pattern);
   if ((ok && !negation) || (!ok && negation)) {
	   goto ok_match;
   }
   goto no_match;
}

/**************************************************
 * Last case: we want to match a non alphabetic token like "!" */
if (!u_strcmp(grammar_tag->input,text_tag->content)) {
   return OK_MATCH_STATUS;
}
return NO_MATCH_STATUS;

/* We arrive here when we have used dela_entry variables */
ok_match:
free_dela_entry(grammar_entry);
free_dela_entry(text_entry);
return OK_MATCH_STATUS;

no_match:
free_dela_entry(grammar_entry);
free_dela_entry(text_entry);
return NO_MATCH_STATUS;
}


/**
 * This function takes a tag of the form <.......> and tokenizes it.
 */
struct pattern* tokenize_grammar_tag(unichar* tag,int *negation) {
(*negation)=0;
if (tag==NULL) {
   error("NULL pattern error in tokenize_grammar_tag\n");
   return NULL;
}
if (tag[0]!='<') {
   error("ERROR: a pattern does not start with < in tokenize_grammar_tag\n");
   return NULL;
}
int l=u_strlen(tag);
if (tag[l-1]!='>') {
   error("ERROR: a pattern does not end with > in tokenize_grammar_tag\n");
   return NULL;
}
if (tag[1]=='!') {(*negation)=1;}
else {(*negation)=0;}
tag[l-1]='\0';
struct pattern* pattern=build_pattern(&(tag[1+(*negation)]),NULL);
tag[l-1]='>';
return pattern;
}


/**
 * Returns 1 if there is a space on the immediate left of the given tag.
 */
int is_space_on_the_left_in_tfst(Tfst* tfst,TfstTag* tag) {
if (tag->start_pos_char==0) {
	/* The tag starts exactly at the beginning of a token, so we just
	 * have to look if the previous token was ending with a space. By convention,
	 * we say that the token #0 has no space on its left */
	if (tag->start_pos_token==0) {
		return 0;
	}
	int size_of_previous=tfst->token_sizes->tab[tag->start_pos_token-1];
	return tfst->token_content[tag->start_pos_token-1][size_of_previous-1]==' ';
} else {
	/* The tag is in the middle of a token */
	return tfst->token_content[tag->start_pos_token][tag->start_pos_char-1]==' ';
}
}
