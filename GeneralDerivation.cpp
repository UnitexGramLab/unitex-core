 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

#include "GeneralDerivation.h"
#include "Error.h"
#include "Vector.h"


// main internal functions:
void analyse_word_list(unsigned char*, struct INF_codes*, U_FILE*, U_FILE*, U_FILE*, U_FILE*,Alphabet*,
                       bool*,bool*,struct utags,vector_ptr*,vector_ptr*);
int analyse_word(unichar*,unsigned char*,U_FILE*,U_FILE*,struct INF_codes*,bool*,bool*,Alphabet*,struct utags
      ,vector_ptr*,vector_ptr*);
void explore_state(int, unichar*, int, unichar*, unichar*, int, unichar*, unichar*,
                   struct decomposed_word_list**, int, struct rule_list*, struct dela_entry*,
                   unsigned char*,struct INF_codes*,bool*,bool*,Alphabet*,U_FILE*,struct utags,vector_ptr*,vector_ptr*);


// results of decomposition are written to
struct decomposed_word {
  int n_parts;
  unichar decomposition[2000]; // info about parts
  unichar dela_line[2000];     // new lexicon entry
};
// respectively a list of these
struct decomposed_word_list {
   struct decomposed_word* element;
   struct decomposed_word_list* suivant;
};
// necessary functions:
struct decomposed_word* new_decomposed_word ();
void free_decomposed_word (struct decomposed_word*);
struct decomposed_word_list* new_decomposed_word_list ();
void free_decomposed_word_list (struct decomposed_word_list*);


// names for affixes and rules can be choosen individually for each language
// definition in "<language>Compounds.cpp"
// struct _tags TAG and typedef tags declared in .h
// unicode version of names:
struct utags init_utags (tags T)
{
  struct utags UTAG;
  u_strcpy(UTAG.PREFIX, T.PREFIX);
  u_strcpy(UTAG.SUFFIX, T.SUFFIX);
  u_strcpy(UTAG.RULE,   T.RULE);
  return UTAG;
}



/* data and functions for rule matching */

// struct pattern holds one pattern to be matched between rule and dela_entry
// e.g.: "+Hum" => { YesNo=1, type=g (grammatical, not flexional), string="Hum" }
struct pattern {
  bool YesNo;
  unichar type;
  unichar string[MAX_COMPOSITION_RULE_LENGTH];
};
// associated functions:
void save_pattern (pattern*, bool, unichar, unichar*);

// struct change_code holds information for manipulating (parts of) words and information
// when decompositing words
struct change_code {
  unichar add[MAX_COMPOSITION_RULE_LENGTH];
  unichar del[MAX_COMPOSITION_RULE_LENGTH];
  unichar repl[MAX_COMPOSITION_RULE_LENGTH];
  unichar substr_act[MAX_COMPOSITION_RULE_LENGTH];
  unichar substr_next[MAX_COMPOSITION_RULE_LENGTH];
  unichar undo_substr_act[MAX_COMPOSITION_RULE_LENGTH];
  unichar undo_substr_next[MAX_COMPOSITION_RULE_LENGTH];
};

// struct composition_rule holds content of one rule
struct composition_rule {
  struct pattern before[MAX_NUMBER_OF_COMPOSITION_RULES];
  struct pattern after[MAX_NUMBER_OF_COMPOSITION_RULES];
  struct change_code then;
};
// associated functions:
struct composition_rule* new_composition_rule ();
void free_composition_rule (struct composition_rule*);
struct composition_rule* copy_composition_rule (struct composition_rule*,
						struct composition_rule*);
// all rules of one lexicon entry are stored in a list
struct rule_list {
  struct composition_rule* rule;
  struct rule_list* next;
};
struct rule_list* new_rule_list(vector_ptr*);
void free_rule_list (struct rule_list*);
void free_all_rule_lists (vector_ptr*);

// parse_condition parses condition parts of rules
void parse_condition (unichar*, pattern*);
// parse_then_code parses replacement part of a rule
void parse_then_code (unichar*_code, struct change_code*);
// parse_rules parses a rule
struct rule_list* parse_rules (unichar*,struct utags,vector_ptr*);
// composition_rule_matches_entry decides whether rule and entry match
int composition_rule_matches_entry (struct pattern*, struct dela_entry*,U_FILE*);
// substring_operation changes prefix or suffix of word given a substring-rule
void substring_operation (unichar*, unichar*);


void check_valid_INF_lines(unichar*, bool*, struct INF_codes*);
bool check_is_valid_for_an_INF_line(unichar*, struct list_ustring*);
int check_is_valid_for_one_INF_code(unichar* t, unichar* s);
int check_is_valid(unichar*, struct dela_entry*);



void init_tableaux (struct INF_codes* inf,bool* *tableau_prefix,
                    bool* *tableau_suffix,struct utags UTAG) {
  *tableau_prefix = (bool*)malloc(sizeof(bool)*(inf->N));
  if (*tableau_prefix==NULL) {
     fatal_alloc_error("init_tableaux");
  }
  *tableau_suffix = (bool*)malloc(sizeof(bool)*(inf->N));
  if (*tableau_suffix==NULL) {
     fatal_alloc_error("init_tableaux");
  }
  check_valid_INF_lines(UTAG.PREFIX, *tableau_prefix, inf);
  check_valid_INF_lines(UTAG.SUFFIX, *tableau_suffix, inf);
}


void free_tableaux (bool* tableau_prefix,bool* tableau_suffix) {
  free(tableau_prefix);
  free(tableau_suffix);
}



// stacks of dic_entries and rule_lists
// respectively garbage collection
void free_all_dic_entries(vector_ptr*);



//
// this function analyses russian compound words
//
void analyse_compounds(Alphabet* alph,
		       unsigned char* bin,
		       struct INF_codes* inf,
		       U_FILE* words,
		       U_FILE* result,
		       U_FILE* debug,
		       U_FILE* new_unknown_words,struct utags UTAG)
{
   bool* prefix;
   bool* suffix;
   vector_ptr* rules=new_vector_ptr(16);
   vector_ptr* entries=new_vector_ptr(16);
  init_tableaux(inf,&prefix,&suffix,UTAG);
  analyse_word_list(bin,inf,words,result,debug,new_unknown_words,alph,prefix,suffix,UTAG,rules,entries);
  free_tableaux(prefix,suffix);
  free_vector_ptr(rules);
  free_vector_ptr(entries);
}

//
// this function reads words in the word file and try analyse them
//
void analyse_word_list(unsigned char* tableau_bin,
			       struct INF_codes* inf,
			       U_FILE* words,
			       U_FILE* result,
			       U_FILE* debug,
			       U_FILE* new_unknown_words,
			       Alphabet* alph,
			       bool* prefix,bool* suffix,
			       struct utags UTAG,
			       vector_ptr* rules,
			       vector_ptr* entries)
{
  unichar s[MAX_WORD_LENGTH];
  u_printf("Analysing russian unknown words...\n");
  int n=0;
  int words_done = 0;
  while (EOF!=u_fgets_limit2(s,MAX_WORD_LENGTH,words)) {
    if (!analyse_word(s,tableau_bin,debug,result,inf,prefix,suffix,alph,UTAG,rules,entries)) {
      // if the analysis has failed, we store the word in the new unknown word file
      u_fprintf(new_unknown_words,"%S\n",s);
    } else {
      n++;
    }
    if ( (++words_done % 10000) == 0)
      u_printf("%d words done", words_done);
  }
  u_printf("%d words decomposed as compound words\n",n);
}



//
// this function try to analyse an unknown russian word
//
int analyse_word(unichar* mot,unsigned char* tableau_bin,U_FILE* debug,U_FILE* result_file,
                 struct INF_codes* inf_codes,bool* prefix,bool* suffix,Alphabet* alphabet,
                 struct utags UTAG,vector_ptr* rules,vector_ptr* entries)
{
#if DDEBUG > 0
  {
    u_fprintf(debug,"\n  %S\n",mot);
  }
#endif

  unichar decomposition[MAX_DICT_LINE_LENGTH];
  unichar dela_line[MAX_DICT_LINE_LENGTH];
  unichar correct_word[MAX_DICT_LINE_LENGTH];
  decomposition[0]='\0';
  dela_line[0]='\0';
  correct_word[0]='\0';
  struct decomposed_word_list* l = 0;
  explore_state(4,correct_word,0,mot,mot,0,decomposition,dela_line,&l,1,0,0,tableau_bin,
        inf_codes,prefix,suffix,alphabet,debug,UTAG,rules,entries);
  free_all_dic_entries(entries);
  free_all_rule_lists(rules);
  if ( l == 0 ) {
    return 0;
  }
  struct decomposed_word_list* tmp = l;
  while ( tmp != NULL ) {
	  if (debug!=NULL) {
	     u_fprintf(debug,"%S = %S\n",mot,tmp->element->decomposition);
	  }
	  u_fprintf(result_file,"%S\n",tmp->element->dela_line);
     tmp=tmp->suivant;
  }
  free_decomposed_word_list(l);
  return 1;
}


int check_is_valid(unichar* t, struct dela_entry* d)
{
  return dic_entry_contain_gram_code(d, t);
}

int check_is_valid_for_one_INF_code(unichar* t, unichar* s)
{
  unichar temp[MAX_DICT_LINE_LENGTH];
  u_strcpy(temp,"x,");
  u_strcat(temp,s);
  struct dela_entry* d = tokenize_DELAF_line(temp,0);
  int res = check_is_valid(t, d);
  free_dela_entry(d);
  return res;
}

bool check_is_valid_for_an_INF_line(unichar* t, struct list_ustring* l)
{
  while ( l != 0 ) {
    if (check_is_valid_for_one_INF_code(t, l->string)) {
      return 1;
    }
    l = l->next;
  }
  return 0;
}

void check_valid_INF_lines(unichar* t, bool* tableau, struct INF_codes* inf)
{
  u_printf("Check valid %S components...\n",t);
  for (int i=0;i<inf->N;i++) {
    tableau[i] = check_is_valid_for_an_INF_line(t, inf->codes[i]);
  }
}




struct decomposed_word* new_decomposed_word()
{
  struct decomposed_word* tmp;
  tmp=(struct decomposed_word*)malloc(sizeof(struct decomposed_word));
  if (tmp==NULL) {
     fatal_alloc_error("new_decomposed_word");
  }
  tmp->n_parts=0;
  tmp->decomposition[0]='\0';
  tmp->dela_line[0]='\0';
  return tmp;
}

void free_decomposed_word(struct decomposed_word* t)
{
  free(t);
}

struct decomposed_word_list* new_decomposed_word_list()
{
  struct decomposed_word_list* tmp;
  tmp = (struct decomposed_word_list*) malloc(sizeof(struct decomposed_word_list));
  if (tmp==NULL) {
       fatal_alloc_error("new_decomposed_word_list");
    }
  tmp->element = 0;
  tmp->suivant = 0;
  return tmp;
}

void free_decomposed_word_list(struct decomposed_word_list* l)
{
  struct decomposed_word_list* tmp;
  while ( l != 0 ) {
    free_decomposed_word(l->element);
    tmp=l->suivant;
    free(l);
    l=tmp;
  }
}



bool affix_is_valid (int index,bool* prefix,bool* suffix)
{
  return prefix[index] || suffix[index];
}

bool prefix_is_valid (int index,bool* prefix)
{
  return prefix[index];
}

bool suffix_is_valid (int index,bool* suffix)
{
  return suffix[index];
}



void save_pattern (pattern* patterns, bool YesNo, unichar type, unichar* patt)
{
  patterns->YesNo   = YesNo;
  patterns->type    = type;
  patterns->string[0] = '\0';
  u_strcpy(patterns->string, patt);
}


void parse_condition (unichar* condition, pattern* patterns)
{
  // parses condition for derivation and composition
  int j = 0;
  for (int i = 0; condition[i] != '\0'; i++) {
    if (condition[i]=='<') {
      // begin of "abstract" pattern
      i++;
      unichar type = 'g';
      bool YesNo = 1;
      unichar pattern[MAX_COMPOSITION_RULE_LENGTH];
      pattern[0] = '\0';
      int k = 0;
      while (condition[i] != '>' && condition[i] != '\0') {
	if (condition[i] == '+') { // grammatical code positive
	  pattern[k] = '\0';
	  save_pattern(&patterns[j++],YesNo,type,pattern);
	  YesNo = 1;
	  type = 'g';
	  i++;
	  k = 0;
	}
	else if (condition[i] == '-') { // grammatical code negative
	  pattern[k] = '\0';
	  save_pattern(&patterns[j++],YesNo,type,pattern);
	  YesNo = 0;
	  type = 'g';
	  i++;
	  k = 0;
	}
	else if (condition[i] == ':') { // flexional code
	  pattern[k] = '\0';
	  save_pattern(&patterns[j++],YesNo,type,pattern);
	  YesNo = 1;
	  type = 'f';
	  i++;
	  k = 0;
	}
	pattern[k++] = condition[i];
        i++;
      }
      if (pattern[0] != '\0')
	pattern[k] = '\0';
	save_pattern(&patterns[j++],YesNo,type,pattern);
    }
    else { // don't know if it is necessary to have concrete words or something else
    }
  }
  // last pattern in array must be empty
  unichar pattern[MAX_COMPOSITION_RULE_LENGTH];
  pattern[0] = '\0';
  save_pattern(&patterns[j],0,'\0',pattern);
}


struct composition_rule* new_composition_rule () {
  struct composition_rule* tmp
    = (struct composition_rule*)malloc(sizeof(struct composition_rule));
  if (tmp==NULL) {
     fatal_alloc_error("new_composition_rule");
  }
  tmp->before[0].string[0] = '\0';
  tmp->after[0].string[0] = '\0';
  tmp->then.add[0]    = '\0';
  tmp->then.del[0]    = '\0';
  tmp->then.repl[0]   = '\0';
  tmp->then.substr_act[0]   = '\0';
  tmp->then.substr_next[0]   = '\0';
  tmp->then.undo_substr_act[0]   = '\0';
  tmp->then.undo_substr_next[0]   = '\0';
  return tmp;
}



void free_composition_rule (struct composition_rule* t)
{
  if ( t == 0 ) return;
  free(t);
}

struct composition_rule* copy_composition_rule (struct composition_rule* a,
						struct composition_rule* b)
{
  for (int i = 0; i < MAX_NUMBER_OF_COMPOSITION_RULES; i++) {
    u_strcpy(a->before[i].string, b->before[i].string);
    a->before[i].YesNo = b->before[i].YesNo;
    a->before[i].type = b->before[i].type;
    if (a->before[i].string[0] == '\0')
      break;
  }
  for (int i = 0; i < MAX_NUMBER_OF_COMPOSITION_RULES; i++) {
    u_strcpy(a->after[i].string, b->after[i].string);
    a->after[i].YesNo = b->after[i].YesNo;
    a->after[i].type = b->after[i].type;
    if (a->after[i].string[0] == '\0')
      break;
  }
  u_strcpy(a->then.add, b->then.add);
  u_strcpy(a->then.del, b->then.del);
  u_strcpy(a->then.repl, b->then.repl);
  u_strcpy(a->then.substr_act, b->then.substr_act);
  u_strcpy(a->then.substr_next, b->then.substr_next);
  u_strcpy(a->then.undo_substr_act, b->then.undo_substr_act);
  u_strcpy(a->then.undo_substr_next, b->then.undo_substr_next);
  return a;
}




struct rule_list* new_rule_list(vector_ptr* rule_collection) {
  struct rule_list* tmp
    = (struct rule_list*)malloc(sizeof(struct rule_list));
  if (tmp==NULL) {
     fatal_alloc_error("new_rule_list");
  }
  tmp->rule = 0;
  tmp->next = 0;
  vector_ptr_add(rule_collection,tmp);
  return tmp;
}

void free_rule_list (struct rule_list* r)
{
  struct rule_list* tmp;
  while ( r != 0 ) {
    tmp = r;
    r = r->next;
    if ( tmp->rule != 0 ) free_composition_rule(tmp->rule);
    free(tmp);
  }
}
void free_rule_list2 (struct rule_list* r)
{
  if ( r != 0) {
    if ( r->rule != 0 ) {
      free_composition_rule(r->rule);
    }
    free(r);
  }
}

void free_all_rule_lists (vector_ptr* rule_collection) {
for (int i=0;i<rule_collection->nbelems;i++) {
   struct rule_list* r=(struct rule_list*)rule_collection->tab[i];
   free_rule_list2(r);
   /* We don't want the vector to be freed now, and we don't want free_vector_ptr to crash */
   rule_collection->tab[i]=NULL;
}
rule_collection->nbelems=0;
}


struct dela_entry* new_dic_entry (unichar* entry,vector_ptr* entry_collection) {
  struct dela_entry* d = tokenize_DELAF_line(entry,0);
  vector_ptr_add(entry_collection,d);
  return d;
}

void free_all_dic_entries (vector_ptr* entry_collection) {
   for (int i=0;i<entry_collection->nbelems;i++) {
      struct dela_entry* r=(struct dela_entry*)entry_collection->tab[i];
      free_dela_entry(r);
      /* We don't want the vector to be freed now, and we don't want free_vector_ptr to crash */
      entry_collection->tab[i]=NULL;
   }
   entry_collection->nbelems=0;
}


void parse_then_code (unichar* then_code, struct change_code* then)
{
  enum { BEGIN, SUBSTR_ACT, UNDO_ACT, SUBSTR_NEXT, UNDO_NEXT, CODE };
  int state = SUBSTR_ACT;
  int k = 0;
  for (int i = 0; then_code[i] != '\0'; i++) {
    if (then_code[i] == '\\') // escape char
      i++;
    if (then_code[i] == ';') {
      if (state == SUBSTR_ACT) {
 	then->substr_act[k] = '\0';
	k = 0;
	state = UNDO_ACT;
      }
      else if (state == SUBSTR_NEXT) {
 	then->substr_next[k] = '\0';
	k = 0;
	state = UNDO_NEXT;
      }
    }
    else if (then_code[i] == '.') { // begin of codes
      if (state == SUBSTR_ACT) {
	then->substr_act[k] = '\0';
	k = 0;
      }
      else if (state == SUBSTR_NEXT) {
	then->substr_next[k] = '\0';
	k = 0;
      }
      else if (state == UNDO_ACT) {
	then->undo_substr_act[k] = '\0';
	k = 0;
      }
      else if (state == UNDO_NEXT) {
	then->undo_substr_next[k] = '\0';
	k = 0;
      }
      state = CODE;
    }
    else if (then_code[i] == '!') { // begin of substr_next
      state = SUBSTR_NEXT;
      then->substr_act[k] = '\0';
      k = 0;
    }
    else if (then_code[i] == '<') { // codes replacing the old ones
      i++;
      int j = 0;
      while (then_code[i] != '>')
	then->repl[j++] = then_code[i++];
      then->repl[j] = '\0';
    }
    else if (state == CODE && then_code[i] == '+') { // feature to be added
      i++;
      int j = 0;
      while (then_code[i] != '+' && then_code[i] != '-'
	     && then_code[i] != '\0') {
	then->add[j++] = then_code[i];
        i++;
      }
      then->add[j] = '\0';
    }
    else if (state == CODE && then_code[i] == '-') { // feature to be deleted
      i++;
      int j = 0;
      while (then_code[i] != '+' && then_code[i] != '-'
	     && then_code[i] != '\0') {
	then->del[j++] = then_code[i];
        i++;
      }
      then->del[j] = '\0';
    }
    else if (state == SUBSTR_ACT) { // substring-operation actual element
      then->substr_act[k++] = then_code[i];
    }
    else if (state == SUBSTR_NEXT) { // substring-operation next element
      then->substr_next[k++] = then_code[i];
    }
    else if (state == UNDO_ACT) { // undo-substring-operation actual element
      then->undo_substr_act[k++] = then_code[i];
    }
    else if (state == UNDO_NEXT) { // undo-substring-operation next element
      then->undo_substr_next[k++] = then_code[i];
    }
  }
}

struct rule_list* parse_rules (unichar* entry,struct utags UTAG,vector_ptr* rules)
{
  // parses dictionary entry to extract rules for derivation and composition

  struct rule_list* rule_list = new_rule_list(rules);
  struct rule_list* actual_list_pos = rule_list;

  unichar cleaned_entry[MAX_DICT_LINE_LENGTH]; // rules will be stripped off
  unichar beforcond[MAX_COMPOSITION_RULE_LENGTH];
  unichar aftercond[MAX_COMPOSITION_RULE_LENGTH];
  unichar then_code[MAX_COMPOSITION_RULE_LENGTH];

  int bcpos, acpos, tpos;
  bcpos = acpos = tpos = 0;
  enum { BEGIN, BEFORE_COND, AFTER_COND, THEN };
  int state = 0;
  int k = 0;
  for (int i = 0; entry[i] != '\0'; i++) {
    if ( state != BEGIN ) { // inside a rule
      if (entry[i] == '\\')
	i++; // unescaping escaped chars in rule
      if (entry[i] == ')') {
	// end of rule
 	struct composition_rule* rule = new_composition_rule();
	beforcond[bcpos] = '\0';
	aftercond[acpos] = '\0';
	then_code[tpos]  = '\0';
	parse_condition(beforcond, rule->before);
	parse_condition(aftercond, rule->after);
	parse_then_code(then_code, &rule->then);
	bcpos = acpos = tpos = 0;
	if (actual_list_pos->rule != 0) { // not first rule
	  struct rule_list* tmp = new_rule_list(rules);
	  actual_list_pos->next = tmp;
	  actual_list_pos = tmp;
	}
	actual_list_pos->rule = rule;
	state = BEGIN;
      }
      else if (state == BEFORE_COND) {
	// condition before
	if (entry[i] == '#')
	  state = AFTER_COND;
	else
	  beforcond[bcpos++] = entry[i];
      }
      else if (state == AFTER_COND) {
	// condition after
	if (entry[i] == '=')
	  state = THEN;
	else
	  aftercond[acpos++] = entry[i];
      }
      else if (state == THEN)
	// then-code
	then_code[tpos++] = entry[i];
    }
    else { // not inside a rule
      if (entry[i] == '+') {
	unichar tmp[MAX_DICT_LINE_LENGTH];
	int j;
	for (j = i+1; ((entry[j] != '+') &&
		       (entry[j] != ':') &&
		       (entry[j] != '(') &&
		       (entry[j] != '\0')); j++)
	  tmp[j-(i+1)] = entry[j];
	tmp[j-(i+1)] = '\0';
	if ((!u_strcmp(tmp, UTAG.PREFIX)) ||
	    (!u_strcmp(tmp, UTAG.SUFFIX))) {
	  i = j-1;
	}
	else if (!u_strcmp(tmp, UTAG.RULE)) {
	  i = j; // including '('
	  state = BEFORE_COND;
	}
	else {
	  cleaned_entry[k++] = entry[i];
	}
      } else {
	cleaned_entry[k++] = entry[i];
      }
    }
  }
  cleaned_entry[k] = '\0';
  u_strcpy(entry, cleaned_entry);
  if (rule_list->rule == 0)
    rule_list->rule = new_composition_rule();
  return rule_list;
}


int composition_rule_matches_entry (struct pattern* rule,
				     struct dela_entry* d,U_FILE* 
#if DDEBUG > 1                         
				     debug_file
#endif
                     ) {
  int ok = 1;
  // "ok = 0;"  may be replaced by "return 0;"
  int flex_code_already_matched = 1;
  unichar tmp[MAX_DICT_LINE_LENGTH];
  tmp[0] = '\0';
#if DDEBUG > 1
    u_strcat(tmp, "   trying ");
#endif
  for (int i = 0; i < MAX_NUMBER_OF_COMPOSITION_RULES; i++) {
    if (rule[i].string[0] == '\0')
      break; // last rule reached: return 1
#if DDEBUG > 1
    {
      if (rule[i].type == 'f')
	u_strcat(tmp, ":");
      else if (rule[i].YesNo)
	u_strcat(tmp, "+");
      else
	u_strcat(tmp, "-");
      u_strcat(tmp, rule[i].string);
    }
#endif
    if (rule[i].YesNo) { // rule '+' => pattern must be in entry, too
      if (rule[i].type == 'g') {
	if (dic_entry_contain_gram_code(d,rule[i].string))
	  continue; // rule matched, try next one
	ok = 0;
      }
      else if (rule[i].type == 'f') {
	if (dic_entry_contain_inflectional_code(d,rule[i].string)) {
	  // rule matched, try next one, but mark flex codes as matched
	  flex_code_already_matched = 2;
	  continue;
	}
	else if (flex_code_already_matched == 2) {
	  // no matter if any flex code already matched
	  continue;
	}
	else {
	  // no-matches before first match
	  flex_code_already_matched = 0;
	}
      }
    }
    else { // rule '-' => pattern must not be in entry
      if (rule[i].type == 'g') {
	if (dic_entry_contain_gram_code(d,rule[i].string))
	  ok = 0;
      }
      else if (rule[i].type == 'f') {
	// implemented although not possible in rule syntax
	if (dic_entry_contain_inflectional_code(d,rule[i].string))
	  ok = 0;
      }
    }
  }
#if DDEBUG > 1
  {
    if (ok && flex_code_already_matched) u_fprintf(debug_file,"\n   === matched ");
    else u_fprintf(debug_file,"\n   === not matched ");
    if ( d->semantic_codes != 0 ) {
      for (int i = 0; i < d->n_semantic_codes; i++) {
         u_fprintf(debug_file,"+%S",d->semantic_codes[i]);
      }
    }
    if ( d->inflectional_codes != 0 ) {
      for (int i = 0; i < d->n_inflectional_codes; i++) {
         u_fprintf(debug_file,":%S",d->inflectional_codes[i]);
      }
    }
    u_fprintf(debug_file,"\n");
  }
#endif
  return (ok && flex_code_already_matched);
}

void substring_operation (unichar* affix, unichar* rule)
{
  if (rule[0] == '\0') { // no substring operation
    return;
  }
  else if (rule[0] == '-') { // prefix operation on affix
    int i, j, n;
    j = n = 0;
    i = 1;
    while (rule[i] >= '0' && rule[i] <= '9') {
      n = n*10+(rule[i]-'0');
      i++;
    }
    unichar new_affix[MAX_WORD_LENGTH];
    while (rule[i] != '\0')
      new_affix[j++] = rule[i++];
    i = 0; // now index of affix
    while (affix[i] != '\0') {
      if (i >= n)
	new_affix[j++] = affix[i];
      i++;
    }
    new_affix[j] = '\0';
    u_strcpy(affix, new_affix);
  }
  else {                 // suffix operation on affix
    int i, j, n;
    i = j = n = 0;
    while (rule[i] >= '0' && rule[i] <= '9') {
      n = n*10+(rule[i]-'0');
      i++;
    }
    j = u_strlen(affix)-n;
    while (rule[i] != '\0')
      affix[j++] = rule[i++];
    affix[j] = '\0';
  }
}


/**
 * This function parses a DELAF line and stores in the appropriate parameters
 * the inflected form, the lemma and the codes. If there is no lemma, it takes
 * the value of the inflected form. All these strings are unprotected:
 *
 * "3\,14,.PI" => inflected="3,14" lemma="3,14" codes="PI"
 */
void tokenize_DELA_line_into_3_parts(const unichar* line,unichar* inflected,unichar* lemma,unichar* codes) {
int i,j;
if (line==NULL) return;
/* We read the inflected form */
i=0;
j=0;
while (line[i]!='\0' && line[i]!=',') {
   if (line[i]=='\\') {
      /* We unprotect chars */
      i++;
      if (line[i]=='\0') {
         error("***Dictionary error: incorrect line\n%S\n",line);
         return;
      }
   }
   inflected[j++]=line[i++];
}
inflected[j]='\0';
if (line[i]=='\0') {
   error("***Dictionary error: incorrect line\n%S\n",line);
   return;
}
/* We read the lemma */
i++;
j=0;
while (line[i]!='\0' && line[i]!='.') {
   if (line[i]=='\\') {
      i++;
      if (line[i]=='\0') {
         error("***Dictionary error: incorrect line\n%S\n",line);
         return;
      }
   }
   lemma[j++]=line[i++];
}
lemma[j]='\0';
if (j==0) {
   /* If the lemma is not specified, we copy the inflected form */
   u_strcpy(lemma,inflected);
}
if (line[i]=='\0') {
   error("***Dictionary error: incorrect line\n%S\n",line);
   return;
}
/* We read the remaining part of the line */
i++;
j=0;
while (line[i]!='\0') {
   if (line[i]=='\\') {
      i++;
      if (line[i]=='\0') {
         error("***Dictionary error: incorrect line\n%S\n",line);
         return;
      }
   }
   codes[j++]=line[i++];
}
codes[j]='\0';
}



//
// this function explores the dictionary to decompose the word mot
//
void explore_state (int adresse,
		    unichar* current_component,
		    int pos_in_current_component,
		    unichar* original_word,
		    unichar* remaining_word,
		    int pos_in_remaining_word,
		    unichar* decomposition,
		    unichar* lemma_prefix,
		    struct decomposed_word_list** L,
		    int n_decomp,
		    struct rule_list* rule_list_called,
		    struct dela_entry* dic_entr_called,
		    unsigned char* tableau_bin,
		    struct INF_codes* inf_codes,
		    bool* prefix,bool* suffix,Alphabet* alphabet,
		    U_FILE* debug_file,struct utags UTAG,
		    vector_ptr* rules,vector_ptr* entries)
{

  int c = tableau_bin[adresse]*256+tableau_bin[adresse+1];
  int index;
  int t = 0;

  if ( !(c&32768) ) { // if we are in a terminal state

    index = tableau_bin[adresse+2]*256*256+tableau_bin[adresse+3]*256+tableau_bin[adresse+4];
    current_component[pos_in_current_component] = '\0';

    if (pos_in_current_component >= 1) {
      // go on if word length equals zero

#if DDEBUG > 0
      {
         u_fprintf(debug_file,". %S\n",current_component);
      }
#endif

      struct list_ustring* l = inf_codes->codes[index];
      while ( l != 0 ) {

//	int one_rule_already_matched = 0; // one rule matched each entry is enough

	unichar entry[MAX_DICT_LINE_LENGTH];
	uncompress_entry(current_component, l->string, entry);

#if DDEBUG > 0
	{
	  u_fprintf(debug_file,": %S\n",entry);
	}
#endif

	struct dela_entry* dic_entr = new_dic_entry(entry,entries);

	unichar lemma_prefix_new[MAX_DICT_LINE_LENGTH];
	struct rule_list* rule_list_new = 0;
	unichar next_remaining_word[MAX_WORD_LENGTH];

	struct rule_list* rule_list = 0;
	if (prefix_is_valid(index,prefix) || suffix_is_valid(index,suffix))
	  rule_list = parse_rules(entry,UTAG,rules);
	else {
	  rule_list = new_rule_list(rules);
	  rule_list->rule = new_composition_rule();
	}
	// entry is now cleaned from rules for composition and derivation

	// log decomposition of word
	// ("cleaned" entries for better overview)
	unichar decomposition_new[MAX_DICT_LINE_LENGTH];
	u_strcpy(decomposition_new, decomposition);
	if (decomposition_new[0] != '\0') u_strcat(decomposition_new, " +++ ");
	u_strcat(decomposition_new, entry);


	// loop on all composition_rules called
	struct rule_list* called = rule_list_called;
	do { // while ( rule_list* called != 0 )

// 	  if (one_rule_already_matched)
// 	    break;

 	  struct composition_rule* rule_called
	    = ( called != 0 ) ? called->rule : 0; // may be undefined

	  // loop on all actual composition_rules
	  struct rule_list* r_list = rule_list;
 	  while ( r_list != 0 ) {

// 	    if (one_rule_already_matched)
// 	      break;

	    struct composition_rule* rule = r_list->rule; // ever defined, see upwards

	    if (remaining_word[pos_in_remaining_word]=='\0' &&
		// we have explored the entire original word
		((((dic_entr_called != 0) &&
		   composition_rule_matches_entry(rule->before, dic_entr_called,debug_file))  &&
		  ((rule_called != 0) &&
		   composition_rule_matches_entry(rule_called->after, dic_entr,debug_file))) ||
		 // and we have a valid right component, i.e. rules match
		 ((dic_entr_called == 0) &&  // or a simple entry (i.e. no prefix),
		  (! affix_is_valid(index,prefix,suffix))) // but no affix
		 )
		)  {

//	      one_rule_already_matched = 1;

	      unichar inflected[MAX_WORD_LENGTH];
	      unichar lemma[MAX_WORD_LENGTH];
	      unichar codes[MAX_DICT_LINE_LENGTH];
	      tokenize_DELA_line_into_3_parts(entry, inflected, lemma, codes);

	      /* generating new lexicon entry */
	      unichar new_dela_line[MAX_DICT_LINE_LENGTH];

	      /* word form */
	      u_strcpy(new_dela_line, original_word);
	      u_strcat(new_dela_line, ",");

	      /* lemma */                           // lemmatize word
	      if (rule->then.repl[0] == '\0'	    // if there are no replace codes
		  && (rule_called != 0              // either in actual nor in preceeding rule
		      && rule_called->then.repl[0] == '\0')) {
		u_strcat(new_dela_line, lemma_prefix);
		unichar affix[MAX_WORD_LENGTH];
		u_strcpy(affix, lemma);
		substring_operation(affix, rule->then.substr_act);
		if (rule_called != 0 && rule_called->then.undo_substr_next[0] != '\0')
		  substring_operation(affix, rule_called->then.undo_substr_next);
		u_strcat(new_dela_line, affix);
	      } else {
		u_strcat(new_dela_line, original_word);
	      }

	      /* codes */
	      u_strcat(new_dela_line,".");
	      if (rule->then.repl[0] != '\0') {            // replacing codes by
		u_strcat(new_dela_line,rule->then.repl);   // suffix' ones
	      }
	      else if (rule_called == 0) { // prohibit SGV
		u_strcat(new_dela_line,codes);
	      }
	      else if (rule_called->then.repl[0] != '\0') {
		u_strcat(new_dela_line,rule_called->then.repl); // prefix' ones
	      }
	      // replace replaces all and blocks adding and deleting
	      // maybe this is not optimal ???
	      else {
		if (rule_called->then.add[0] != '\0') {        // add codes
		  if (!dic_entry_contain_gram_code(dic_entr, rule_called->then.add)) {
		    bool done = 0;
		    unichar tmp[MAX_COMPOSITION_RULE_LENGTH];
		    int j = 0;
		    for (int i = 0; codes[i] != '\0'; i++) {
		      if (codes[i] == ':' && (!done)) {
			tmp[j++] = '+';
			tmp[j] = '\0';
			u_strcat(new_dela_line,tmp);
			u_strcat(new_dela_line,rule_called->then.add);
			done = 1;
			j = 0;
		      }
		      tmp[j++] = codes[i];
		    }
		    tmp[j] = '\0';
		    u_strcat(new_dela_line,tmp);
		    if (!done) {
		      u_strcat(new_dela_line,"+");
		      u_strcat(new_dela_line,rule_called->then.add);
		    }
		  } else {
		    u_strcat(new_dela_line,codes);
		  }
		} else if (rule_called->then.del[0] != '\0') { // delete codes

		} else {
		  u_strcat(new_dela_line,codes);
		}
	      }

#if DDEBUG > 0
	      {
            u_fprintf(debug_file,"= %S\n",new_dela_line);
	      }
#endif

	      struct decomposed_word* wd = new_decomposed_word();
	      wd->n_parts = n_decomp;
	      u_strcpy(wd->decomposition,decomposition_new);
	      u_strcpy(wd->dela_line,new_dela_line);
	      struct decomposed_word_list* wdl=new_decomposed_word_list();
	      // unshift actual decomposition to decomposition list L
	      wdl->element = wd;
	      wdl->suivant = (*L);
	      (*L) = wdl;

	    } // end if end of word and valid right component
	    else if
	      // beginning or middle of word: explore the rest of the original word
	      (prefix_is_valid(index,prefix) &&
	       check_is_valid(UTAG.PREFIX, dic_entr) &&
	       // but only if the current component was a valid left one
	       // we go on with the next component
	       (
		(n_decomp == 1) // prefix as first part of a word: no rule matching
		||
		(               // prefix in the middle of a word
		 (rule_called &&
		  composition_rule_matches_entry(rule_called->after, dic_entr,debug_file)) &&
		 (dic_entr_called &&
		  composition_rule_matches_entry(rule->before, dic_entr_called,debug_file))
		)
	       )) {

//	      one_rule_already_matched = 1;

	      u_strcpy(lemma_prefix_new, lemma_prefix);
	      unichar affix[MAX_WORD_LENGTH];
	      u_strcpy(affix, current_component);
	      if (rule_called != 0 && rule_called->then.undo_substr_next[0] != '\0') {
            substring_operation(affix, rule_called->then.undo_substr_next);
            u_fprintf(debug_file,"yes\n");
	      }
	      substring_operation(affix, rule->then.substr_act);
	      u_strcat(lemma_prefix_new, affix);
	      int j = 0;
	      for (int i = pos_in_remaining_word; remaining_word[i] != '\0'; i++) {
            next_remaining_word[j++] = remaining_word[i];
         }
	      next_remaining_word[j] = '\0';
	      if (rule->then.substr_next[0] != '\0') {
            substring_operation(next_remaining_word, rule->then.substr_next);
#if DDEBUG > 0
            {
               u_fprintf(debug_file,"| %S|%S\n",affix,next_remaining_word);
            }
#endif
	      }

#if DDEBUG > 0
	      {
            u_fprintf(debug_file,"- %S\n",entry);
	      }
#endif
	      struct rule_list* tmp = new_rule_list(rules);
	      tmp->rule = new_composition_rule();
	      copy_composition_rule(tmp->rule, rule);
	      tmp->next = 0;
	      if ( rule_list_new == 0 ) {
            rule_list_new = tmp;
	      }
	      else {
            struct rule_list* t = rule_list_new;
            while ( t->next != 0 ) {
               t=t->next;
            }
            t->next = tmp;
	      }

	    }
	    else {
	      // no valid suffix nor prefix
	    }

	    r_list = r_list->next;
	  } // while ( rule_list* r_list != 0 )

	  if ( called != 0 )
	    called = called->next;
	} while ( called != 0 );

	// prefix found, try to decomposite rest of word
	if ( rule_list_new != 0 && dic_entr != 0 ) {
	  unichar next_component[MAX_WORD_LENGTH];
#if DDEBUG > 0
	  {
	    u_fprintf(debug_file,"> %S\n",next_remaining_word);
	  }
#endif
	  explore_state(4,
			next_component,
			0,
			original_word,
			next_remaining_word,
			0,
			decomposition_new,
			lemma_prefix_new,
			L,
			n_decomp+1,
			rule_list_new,
			dic_entr,
			tableau_bin,inf_codes,prefix,suffix,alphabet,debug_file,UTAG,rules,entries);
	}
	else {
// 	  free_dic_entry(dic_entr);
// 	  free_rule_list(rule_list);
	}

	l = l->next;

      } // end of while (token_list* l != 0)

      t = adresse+5;

    } // end of word length >= 1
  }
  else { // not a final state
    c = c-32768;
    t = adresse+2;
  }
  if (remaining_word[pos_in_remaining_word]=='\0') {
    // if we have finished, we return
//     free_dic_entry(dic_entr_called);
//     free_rule_list(rule_list_called);
    return;
  }
  // if not, we go on with the next letter
  for (int i=0;i<c;i++) {
    if (is_equal_or_uppercase((unichar)(tableau_bin[t]*256+tableau_bin[t+1]),
			       remaining_word[pos_in_remaining_word],
			       alphabet)
	||
	is_equal_or_uppercase(remaining_word[pos_in_remaining_word],
			       (unichar)(tableau_bin[t]*256+tableau_bin[t+1]),
			       alphabet)) {
      index = tableau_bin[t+2]*256*256+tableau_bin[t+3]*256+tableau_bin[t+4];
      current_component[pos_in_current_component] =
	(unichar)(tableau_bin[t]*256+tableau_bin[t+1]);
      explore_state(index,
		    current_component,
		    pos_in_current_component+1,
		    original_word,
		    remaining_word,
		    pos_in_remaining_word+1,
		    decomposition,
		    lemma_prefix,
		    L,
		    n_decomp,
		    rule_list_called,
		    dic_entr_called,
		    tableau_bin,
		    inf_codes,prefix,suffix,alphabet,debug_file,UTAG,rules,entries);
    }
    t += 5;
  }
}
