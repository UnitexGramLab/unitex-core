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

/* compgr.cpp */

/* Date 	: juin 98 */
/* Auteur(s) 	: MAYER Laurent et al */
/*                Olivier Blanc 2002-2006 */
/* compilation des grammaires de levee d ambiguites */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "autalmot.h"
#include "fst_file.h"
#include "list_aut.h"
#include "compgr.h"
#include "utils.h"
#include "File.h"
#include "String_hash.h"
#include "AutConcat.h"
#include "AutDeterminization.h"
#include "AutMinimization.h"
#include "AutComplementation.h"



/* maximum number of state for a grammar before we split it in several fst2 */

#define MAX_GRAM_SIZE   128



void split_elag_rule(elRule*);
int count_constraints(Fst2Automaton*,int*);
Fst2Automaton* compile_elag_rule(elRule*);
int get_sub_automaton(SingleGraph,SingleGraph,int,int,int,struct string_hash_ptr*);
int get_sub_automaton(SingleGraph,SingleGraph,int,SymbolType,int*,struct string_hash_ptr*);
int get_left_constraint_part(SingleGraph,SingleGraph,int,int,int*,struct string_hash_ptr*);
SingleGraph combine_constraints(elRule*,int,SingleGraph,SingleGraph,struct string_hash_ptr*);




static inline void chomp(char * s) {
  while (*s) {
    if (*s == '\n' || *s == '\r' || *s == '#') {
      *s = 0;
    } else { s++; }
  }
}


static inline void strip_extension(char * s) {

  char * p = s + strlen(s) - 1;
  
  while (p > s) {
    if (*p == '.') { *p = 0; break; }
    if (*p == '/' || *p == '\\') { break; }
    p--;
  }
}


/**
 * Modify the given automaton so that it starts by a loop that can
 * recognize anything. If A is empty, it remains empty.
 */
void prefix_with_everything(SingleGraph A) {
struct list_int* list=get_initial_states(A);
while (list!=NULL) {
   add_outgoing_transition(A->states[list->n],new_symbol(LEXIC),list->n);
   list=list->next;
}
free_list_int(list);
}


/**
 * Modify the given automaton so that it ends by a loop that can
 * recognize anything. If A is empty, it remains empty.
 */
void suffix_with_everything(SingleGraph A) {
for (int i=0;i<A->number_of_states;i++) {
   if (is_final_state(A->states[i])) {
      /* If we have reached a final state, we don't need to keep
       * its transitions, since the Kleene star transition will
       * include them. We use NULL since we don't want to NULL the
       * symbols that tag transitions, because these symbols can be shared */
      free_Transition(A->states[i]->outgoing_transitions,NULL);
      add_outgoing_transition(A->states[i],new_symbol(LEXIC),i);
    }
  }
}




/**
 * Allocates, initializes and returns a new Elag rule from the given .fst2.
 * Returns NULL in case of error at loading the rule.
 */
elRule* new_elRule(char* fst2) {
elRule* rule=(elRule*)malloc(sizeof(elRule));
if (rule==NULL) {
   fatal_error("Not enough memory in new_elRule\n");
}
rule->automaton=NULL;
rule->nbContexts=0;
rule->contexts=NULL;
rule->name=strdup(fst2);
if ((rule->automaton=load_elag_grammar_automaton(rule->name))==NULL) {
   error("Cannot load '%s' automaton.\n", fst2);
   free(rule->name);
   free(rule);
   return NULL;
}
split_elag_rule(rule);
return rule;
}



static void deleteRegle(elRule * regle) {

  free(regle->name);

  for (int i = 0; i < regle->nbContexts; i++) {
    free_SingleGraph(regle->contexts[i].right);
    free_SingleGraph(regle->contexts[i].left);
  }

  free(regle->contexts);

  free(regle);
}


/**
 * This function takes an automaton with transitions tagged with integers
 * that are indices in the symbols->value array, and it replaces them
 * with transitions tagged with the corresponding symbols.
 */
void convert_transitions_to_elag_ones(SingleGraph g,struct string_hash_ptr* symbols) {
Transition* t;
for (int i=0;i<g->number_of_states;i++) {
   t=g->states[i]->outgoing_transitions;
   while (t!=NULL) {
      /* I'm not sure: dup_symbol or dup_symbols ? */
      t->label=dup_symbol((symbol_t*)symbols->value[t->tag_number]);
      t=t->next;
   }
}
}


/**
 * 
 */
Fst2Automaton* compile_elag_rule(elRule* rule) {
u_printf("Compiling %s... (%d context%s)\n",rule->name,rule->nbContexts,(rule->nbContexts>1)?"s":"");
/* Now, we will convert the automaton into the Elag format, i.e. with
 * transitions tagged with symbol_t* and not integers */
for (int c=0;c<rule->nbContexts;c++) {
   convert_transitions_to_elag_ones(rule->contexts[c].left,rule->automaton->symbols);
   elag_determinize(rule->contexts[c].left);
   trim(rule->contexts[c].left);
   convert_transitions_to_elag_ones(rule->contexts[c].right,rule->automaton->symbols);
   elag_determinize(rule->contexts[c].right);
   trim(rule->contexts[c].right);
}
/* We build A*.R1 */
prefix_with_everything(rule->contexts[0].left);
elag_determinize(rule->contexts[0].left);
elag_minimize(rule->contexts[0].left,rule->automaton->symbols);
SingleGraph anything_R1=rule->contexts[0].left;
/* and R2.A* */
suffix_with_everything(rule->contexts[0].right);
elag_determinize(rule->contexts[0].right);
elag_minimize(rule->contexts[0].right,rule->automaton->symbols);
SingleGraph R2_anything=rule->contexts[0].right;
/* We compute the number of constraint combinations */
int p=(int)pow(2,rule->nbContexts-1);
/* We allocate the resulting automaton */
SingleGraph result=new_SingleGraph();
for (int ens=0;ens<p;ens++) {
   /* For each combination of constraints, we produce an automaton a1
    * that does not match these constraints */
   SingleGraph a1=combine_constraints(rule,ens,anything_R1,R2_anything,rule->automaton->symbols);
   /* And we make the union of it with the current automaton */
   build_union(result,a1);
   elag_determinize(result);
   elag_minimize(result,rule->automaton->symbols);
}
/* Finally, we take the complement of the automaton that rejects wrong paths.
 * This new automaton recognizes correct paths, and so, the application of the
 * Elag rule will consists of intersecting this automaton with the sentence ones. */
elag_complementation(result);
trim(result);
if (result->number_of_states==0) {
   error("Grammar %s forbids everything\n",rule->name);
}
u_printf("Grammar %s compiled (%d states)\n",rule->name,result->number_of_states);
Fst2Automaton* Result=new_Fst2Automaton(NULL,-1);
Result->symbols=rule->automaton->symbols;
Result->automaton=result;
return Result;
}



/**
 * Compiles the given .fst2 grammar into the given .elg file.
 * Returns 0 in case of success; -1 otherwise.
 */
int compile_elag_grammar(char* grammar,char* elg_file) {
elRule* rule=new_elRule(grammar);
if (rule==NULL) {
   error("Unable to read grammar '%s'\n",grammar);
   return -1;
}
Fst2Automaton* A=compile_elag_rule(rule);
if (A==NULL) {
   fatal_error("Unable to compile rule '%s'\n",grammar);
}
error("after compile\n");
deleteRegle(rule);
error("after delete\n");
save_automaton(A,elg_file,FST_GRAMMAR);
free_Fst2Automaton(A);
error("endofcompile\n");
return 0;
}


/* Lit les regles de levee d'ambiguites et les compile sous la forme d'automates. 
 * nomFichNomRegles est le fichier des noms des grammaires. 
 * nomFichNomGramm est le fichier de la composition des grammaires compilées.
 */


int compile_rules(char * rulesname, char * outname) {

  u_printf("Compilation of %s\n", rulesname);

  FILE * f = NULL;
  FILE * frules = fopen(rulesname, "r");
  if (frules == NULL) { fatal_error("cannot open file '%s'\n", rulesname); }

  FILE * out = fopen(outname, "w");
  if (out == NULL) { fatal_error("cannot open file '%s'\n", outname); }

  char fstoutname[FILENAME_MAX]; // le nom du fichier qui contient l'automate resultat (fst2)

  int nbregles = 0;
  char buf[FILENAME_MAX];

  time_t debut = time(0);

  Fst2Automaton * res = NULL, * A;
  int fstno = 0;
  Ustring * ustr = new_Ustring();




  while (fgets(buf, FILENAME_MAX, frules)) {

    chomp(buf);
    if (*buf == 0) { continue; }

    u_printf("\n\n%s ...\n", buf);

    strip_extension(buf);
    strcat(buf, ".elg");

    if ((f = fopen(buf, "r")) == NULL) { // .elg doesn't exist, making one

      strip_extension(buf);
      u_printf("precompiling %s.fst2\n", buf);

      strcat(buf, ".fst2");
      
      elRule * regle = new_elRule(buf);
      if (regle == NULL) { fatal_error("unable to read grammar '%s'\n", buf); }
      
      if ((A = compile_elag_rule(regle)) == NULL) { fatal_error("unable to compile rule '%s'\n", buf); }

      deleteRegle(regle);

      /* saving result in name.elg  ? */
      /*
      strip_extension(buf);
      strcat(buf, ".elg");
      autalmot_output_fst2(A, buf, FST_GRAMMAR);
      */
    } else {
      fclose(f);
      u_printf("using already exiting %s\n", buf);
      A = load_elag_grammar_automaton(buf);      
      if (A == NULL) { fatal_error("unable to load '%s'\n", buf); }
    }


    if (A->nbstates == 0) { error("grammar %s forbids everything!\n", buf); }
 
    error("regroupe\n");

    printtime(if (res) {

      Fst2Automaton * tmp = res;
      res = autalmot_intersection(tmp, A);
      free_Fst2Automaton(tmp);
      free_Fst2Automaton(A);
      elag_trim(res);

    } else { res = A; });

    error("regroupe done (nbstates = %d)\n", res->nbstates);

    fprintf(out, "\t%s\n", buf);
    nbregles++;


    if (res->nbstates > MAX_GRAM_SIZE) {

      elag_minimize(res->automaton,res->symbols,1);

      sprintf(fstoutname, "%s-%d.elg", outname, fstno++);
      fprintf(out, "<%s>\n", fstoutname);

      u_printf("splitting big grammar in '%s' (%d states)\n", fstoutname, res->nbstates);

      u_sprintf(ustr, "%s: compiled elag grammar", fstoutname);
      free(res->name);
      res->name = u_strdup(ustr->str);

      save_automaton(res, fstoutname, FST_GRAMMAR);

      free_Fst2Automaton(res);
      res = NULL;
    }
  }


  
  if (res) {

    sprintf(fstoutname, "%s-%d.elg", outname, fstno++);
    fprintf(out, "<%s>\n", fstoutname);

    u_printf("outputing grammar in '%s'(%d states)\n", fstoutname, res->nbstates);

    elag_minimize(res->automaton,res->symbols,1);

    u_sprintf(ustr, "%s: compiled elag grammar", fstoutname);
    free(res->name);
    res->name = u_strdup(ustr->str);

    save_automaton(res, fstoutname, FST_GRAMMAR);

    free_Fst2Automaton(res);
  }

  time_t fin = time(0);

  fclose(frules);
  fclose(out);

  free_Ustring(ustr);

  u_printf("\ndone.\nElapsed time: %.0f s.\n", difftime(fin, debut));

  u_printf("\n%d rule%s from %s compiled in %s (%d automat%s).\n",
         nbregles, (nbregles > 1) ? "s" : "", rulesname, outname, fstno, (fstno > 1) ? "a" : "on");
  return 0;
}


/**
 * This function builds and returns an automaton for pattern
 * matching of the rule's context.
 */
Fst2Automaton* make_locate_automaton(elRule* rule) {
Fst2Automaton* res=new_Fst2Automaton(NULL);
res->symbols=rule->automaton->symbols;
res->automaton=clone(rule->contexts[0].left);
/* We concatenate the left and right contexts */
elag_concat(res->automaton,rule->contexts[0].right,res->symbols);
/* Then we add loops with ignorable POS on each state */
language_t* lang=get_current_language();
struct list_int* list=NULL;
for (int i=0;i<lang->POSs->size;i++) {
   POS_t* PoS=(POS_t*)lang->POSs->value[i];
   if (PoS->ignorable) {
      /* If we have a POS that can be ignored, we build
       * the corresponding tag like <XXX> and we add it to
       * the symbols of the grammar */
      unichar temp[1024];
      u_sprintf(temp,"<%S>",PoS->name);
      int size=res->symbols->size;
      symbol_t* s=new_symbol_POS(PoS);
      int n=get_value_index(temp,res->symbols,INSERT_IF_NEEDED,s);
      if (n!=size) {
         /* If the symbol was already there, we free it */
         free_symbols(s);
      }
      list=sorted_insert(n,list);
   }
}
for (int q=1;q<res->automaton->number_of_states;q++) {
   struct list_int* l=list;
   while (l!=NULL) {
      add_outgoing_transition(res->automaton->states[q],l->n,q);
      l=l->next;
   }
}
free_list_int(list);
return res;
}



/**
 * This function analyzes the given Elag rule automaton to find
 * where the rule and constraint parts are. As a side effect, it builds
 * a fst2 grammar ("foo.fst2" => "foo-conc.fst2") that can be used by
 * the Locate program to match the <!> .... <!> .... <!> part of the rule.
 */
void split_elag_rule(elRule* rule) {
int c;
/* This array contains the numbers of the states that are pointed to by
 * middle '<=>' of the constraints */
int constraints[ELAG_MAX_CONSTRAINTS];
int nbConstraints=count_constraints(rule->automaton,constraints);
/* +1 because we have to count the <!> .... <!> .... <!> part of the rule */
rule->nbContexts=nbConstraints+1;
rule->contexts=(elContext*)malloc(rule->nbContexts*sizeof(elContext));
if (rule->contexts==NULL) {
   fatal_error("Not enough memory in split_elag_rule\n");
}
for (c=0;c<rule->nbContexts;c++) {
   rule->contexts[c].left=NULL;
   rule->contexts[c].right=NULL;
}
int endR1=ELAG_UNDEFINED;
int endR2=ELAG_UNDEFINED;
int endC2=ELAG_UNDEFINED;
for (Transition* t=rule->automaton->automaton->states[0]->outgoing_transitions;t!=NULL;t=t->next) {
   symbol_t* symbol=(symbol_t*)rule->automaton->symbols->value[t->tag_number];
   switch (symbol->type) {
      /* We split the unique <!> .... <!> .... <!> part */
      case EXCLAM:
         if (rule->contexts[0].left!=NULL) {
            fatal_error("Too much '<!>' tags\n",rule->name);
         }
         rule->contexts[0].left=new_SingleGraph();
         /* We look for the end of the first part of the rule */
         endR1=get_sub_automaton(rule->automaton->automaton,rule->contexts[0].left,t->state_number,0,EXCLAM,rule->automaton->symbols);
         rule->contexts[0].right=new_SingleGraph();
         endR2=get_sub_automaton(rule->automaton->automaton,rule->contexts[0].right,endR1,0,EXCLAM,rule->automaton->symbols);
         if (endR1==ELAG_UNDEFINED || endR2==ELAG_UNDEFINED
             || !is_final_state(rule->automaton->automaton->states[endR2])) {
            fatal_error("split_elag_rule: %s: parse error in <!> part\n",rule->name);
         }
         break;

      /* We split the nbConstraints <=> .... <=> .... <=> parts */
      case EQUAL:
         if (rule->contexts[1].left!=NULL) {
            fatal_error("Non deterministic .fst2 file\n");
         }
         for (c=0;c<nbConstraints;c++) {
            rule->contexts[c+1].left=new_SingleGraph();
            get_sub_automaton(rule->automaton->automaton,rule->contexts[c+1].left,t->state_number,1,constraints[c],rule->automaton->symbols);
            rule->contexts[c+1].right=new_SingleGraph();
            endC2=get_sub_automaton(rule->automaton->automaton,rule->contexts[c+1].right,constraints[c],0,EQUAL,rule->automaton->symbols);
            if (endC2==ELAG_UNDEFINED || !is_final_state(rule->automaton->automaton->states[endC2])) {
               fatal_error("split_elag_rule: %s: parse error in <=> part\n",rule->name);
            }
         }
         break;
         
      default: fatal_error("Left delimitor '<!>' or '<=>' missing\n");
   }
}
free_Fst2Automaton(rule->automaton);
rule->automaton=NULL;
if (rule->contexts[0].left==NULL) {
   fatal_error("In grammar '%s': symbol '<!>' not found.\n",rule->name);
}
char buf[FILENAME_MAX];
remove_extension(rule->name,buf);
strcat(buf,"-conc.fst2");
/* We create the.fst2 to be used by Locate */
Fst2Automaton* locate=make_locate_automaton(rule);
save_automaton(locate,buf,FST_LOCATE);
free_Fst2Automaton(locate);
}


/**
 * An Elag constraints is of the form: <=> .... <=> .... <=>
 * This function looks for states that are pointed to by middle '<=>' transitions.
 * It places their numbers into 'constraints' and it returns the size of this
 * array, i.e. the number of constraints expressed by this Elag rule.
 */
int count_constraints(Fst2Automaton* aut,int* constraints) {
int source=0;
int e;
Transition* t;
int c;
int nbConstraints=0;
symbol_t* symbol;
SingleGraph automaton=aut->automaton;
for (t=automaton->states[0]->outgoing_transitions;t!=NULL && source==0;t=t->next) {
   symbol=(symbol_t*)aut->symbols->value[t->tag_number];
   if (symbol->type==EQUAL) {
      if (t->state_number==0) {
         fatal_error("Illegal cycle in grammar\n");
      }
      source=t->tag_number;
   }
}
if (source==0) {
   /* If there are no contraints */
   return 0;
}
/* We look for '<=>' transitions, but only from the state 1, because
 * from state 0, we would take into account all the '<=>' that begin rules. */
for (e=1;e<automaton->number_of_states;e++) {
   for (t=automaton->states[e]->outgoing_transitions;t!=NULL;t=t->next) {
      symbol=(symbol_t*)aut->symbols->value[t->tag_number];
      if (t->state_number!=source && symbol->type==EQUAL && !is_final_state(automaton->states[t->state_number])) {
         /* We don't take into account '<=>' transitions that go to final states because
          * they are not middle '<=>' transitions. */
         for (c=0;c<nbConstraints;c++) {
            if (constraints[c]==t->state_number) {
               /* We stop if the constraint is already in the list */
               break;
            }
         }
         if (c==nbConstraints) {
	         if (++nbConstraints>=ELAG_MAX_CONSTRAINTS) {
               fatal_error("Too many constraints with same condition\n");
            }
	         constraints[c]=t->state_number;
	      }
      }
   }
}
if (nbConstraints==0) {
   fatal_error("Middle delimitor '<=>' not found\n");
}
return nbConstraints;
}


/**
 * This function copies into 'aut_dest' the sub-automaton of 'aut_src' that
 * starts at the state #start and that ends at:
 * 
 * - the state #z pointed by transitions, if left_constraint_part=1;
 * - the state pointed by transitions tagged with the 'z' symbol otherwise.
 * 
 * Note that these transitions are not copied. The function returns the number of 
 * the ending state.
 * 
 * 'aut_dest' is supposed to have been allocated and to be empty.
 */
int get_sub_automaton(SingleGraph src,SingleGraph dest,int start,
                      int left_constraint_part,int z,struct string_hash_ptr* symbols) {
/* We create the initial state of the sub-automaton */
SingleGraphState state=add_state(dest);
set_initial_state(state);
/* We use this array to renumber states */
int* renumber=(int*)malloc(src->number_of_states*sizeof(int));
if (renumber==NULL) {
   fatal_error("Not enough memory in get_sub_automaton\n");
}
for (int e=0;e<src->number_of_states;e++) {
   renumber[e]=ELAG_UNDEFINED;
}
int end=ELAG_UNDEFINED;
renumber[start]=0;
if (left_constraint_part) {
   end=get_left_constraint_part(src,dest,start,z,renumber,symbols);
} else {
   end=get_sub_automaton(src,dest,start,(SymbolType)z,renumber,symbols);
}
free(renumber);
if (left_constraint_part) {
   if (!end) {
      /* This should never happen */
      fatal_error("Internal error in get_sub_automaton\n");
   }
} else if (end==ELAG_UNDEFINED) {
   /* If we have not found the delimitor */
   fatal_error("In grammar: middle or end delimitor missing\n");
}
return end;
}


/**
 * This function copies into 'aut_dest' the sub-automaton of 'aut_src' that
 * starts at the state #current_state and that ends at the state #z pointed
 * by transitions tagged with the given symbol. Note that these transitions are not copied.
 * The function returns z if such a state is found, ELAG_UNDEFINED otherwise.
 * The 'renumber' array is updated each time a new state is copied into 'aut_dest'.
 */
int get_sub_automaton(SingleGraph src,SingleGraph aut_dest,int current_state,SymbolType delim,
                      int* renumber,struct string_hash_ptr* symbols) {
int f;
int end=ELAG_UNDEFINED;
for (Transition* t=src->states[current_state]->outgoing_transitions;t!=NULL;t=t->next) {
   symbol_t* symbol=(symbol_t*)symbols->value[t->tag_number];
   if (symbol->type==delim) {
      /* If we have found a transition tagged by the delimitor  */
      if (end!=ELAG_UNDEFINED && end!=t->state_number) {
         /* For a given rule part, all the delimitors all supposed to
          * point on the same state */
         fatal_error("get_sub_automaton: too much '<%c>' delimitors in rule\n",delim);
      }
      end=t->state_number;
      /* We set final the corresponding state in 'aut_dest' */
      set_final_state(aut_dest->states[renumber[current_state]]);
    } else {
      /* If we have a normal transition, we just copy it */
      if (renumber[t->state_number]==ELAG_UNDEFINED) {
         /* If we have to create a new state */
         renumber[t->state_number]=aut_dest->number_of_states;
         add_state(aut_dest);
         SingleGraphState state=aut_dest->states[renumber[current_state]];
         add_outgoing_transition(state,t->tag_number,renumber[t->state_number]);
         /* We copy recursively this part of 'aut_src' that we don't yet know */
         f=get_sub_automaton(src,aut_dest,t->state_number,delim,renumber,symbols);
         if (f!=ELAG_UNDEFINED) {
            if (end!=ELAG_UNDEFINED && f!=end) {
               fatal_error("get_sub_automaton: too much '<%c>' delimitors in rule\n",delim);
            }
            end=f;
         }
      } else {
         /* If the state already exists, we just add a transition, because
          * there is no need to explore again the state. */
         SingleGraphState state=aut_dest->states[renumber[current_state]];
         add_outgoing_transition(state,t->tag_number,renumber[t->state_number]);
      }
   }
}
return end;
}


/**
 * This function copies into 'aut_dest' the sub-automaton of 'aut_src' that
 * starts at the state #current_state and that ends at the state #final.
 * Note that <=> transitions leading to the state #final are not copied.
 * The function returns 1 if the state #final is found, 0 otherwise.
 * The 'renumber' array is updated each time a new state is copied into 'aut_dest'.
 */
int get_left_constraint_part(SingleGraph src,SingleGraph dest,int current_state,
                             int final,int* renumber,struct string_hash_ptr* symbols) {
int found=0;
for (Transition* t=src->states[current_state]->outgoing_transitions;t!=NULL;t=t->next) {
   symbol_t* symbol=(symbol_t*)symbols->value[t->tag_number];
   if (symbol->type==EQUAL) {
      /* If we find a <=> transition */
      if (t->state_number==final && !is_final_state(dest->states[renumber[current_state]])) {
         /* If we find the final state for the first time */
         found=1;
         set_final_state(dest->states[renumber[current_state]]);
      }
   } else {
      /* If we have a normal transition */
      if (renumber[t->state_number]==ELAG_UNDEFINED) {
         /* If we have to create a new state */
         renumber[t->state_number]=dest->number_of_states;
         add_state(dest);
         SingleGraphState state=dest->states[renumber[current_state]];
         add_outgoing_transition(state,t->tag_number,renumber[t->state_number]);
         if (get_left_constraint_part(src,dest,t->state_number,final,renumber,symbols)) {
            found=1;
         }
      } else {
         SingleGraphState state=dest->states[renumber[current_state]];
         add_outgoing_transition(state,t->tag_number,renumber[t->state_number]);
      }
   }
}
return found;
}






/* Parcours de l'ensemble des contraintes. Chaque contrainte est
 * representee par un bit dans la representation binaire de ens.
 * Postcondition : l'automate resultat est deterministe et complet.
 * Il peut ne pas avoir d'etats.
 * 
 * retourne:  (A*.R1 \ (U_{i in ens} (A*.Ci,1))) (R2.A* \ (U_{i not in ens} (Ci,2.A*)))
 */


SingleGraph combine_constraints(elRule* rule,int constraint_set,SingleGraph anything_R1,
                                SingleGraph R2_anything,struct string_hash_ptr* symbols) {
/* a1 will be the union of all the constraint of the given constraint set */
SingleGraph a1=new_SingleGraph();
SingleGraph a2=new_SingleGraph();
/* dpc = pow(2,c-1) */
for (int c=1,dpc=1;c<rule->nbContexts;c++,dpc=dpc<<1) {
   if (dpc & constraint_set) {
      /* If the cth bit of the constraint set is set to 1, then we must 
       * add the cth constraint to a1 */
      build_union(a1,clone(rule->contexts[c].left));
      elag_determinize(a1);
      elag_minimize(a1,symbols);
    } else {    /* le c-ieme bit de ens vaut 0 */

      build_union(a2,clone(rule->contexts[c].right));
      elag_determinize(a2);
      elag_minimize(a2,symbols);
    }
  }

  error("\nI\n");

  //  debug("U_{i in E} (Ci,1):\n"); autalmot_dump(a1);
  //  debug("U_{i not in E} (Ci,2):\n"); autalmot_dump(a2);

  /* L(a1) = U_{i in I} R1_i
   * L(a2) = U_{i not in I} R2_i */

  printtime(prefix_with_everything(a1));
  
  printtime(elag_determinize(a1));

  printtime(elag_minimize(a1,symbols));

  printtime(elag_complementation(a1));

  printtime(trim(a1));

  SingleGraph tmp = a1;
  // ZZZ printtime(a1=autalmot_intersection(a1,anything_R1));

  printtime(free_SingleGraph(tmp));

  printtime(trim(a1));

  printtime(elag_minimize(a1,symbols));  /* emonder a3 ? */

  /* L(a1) =  (A*.R1 \ (U_{i in ens} (A*.Ci,1))) */

  error("\nII\n");

  printtime(suffix_with_everything(a2));

  printtime(elag_determinize(a2));

  printtime(elag_minimize(a2,symbols));
    
  printtime(elag_complementation(a2));

  printtime(trim(a2));

  tmp = a2;
  // ZZZ printtime(a2 = autalmot_intersection(a2, R2_anything));
  free_SingleGraph(tmp);


  /* autalmot_determinize(a2) ; */

  printtime(trim(a2));
  printtime(elag_minimize(a2,symbols));

  /* L(a2) == (R2.A* \ (U_{i not in ens} (Ci,2.A*))) */

  error("\nIII\n");

  elag_concat(a1,a2,symbols);

  free_SingleGraph(a2);

  printtime(trim(a1));

  printtime(elag_determinize(a1));

  printtime(trim(a1));  /* utile quand tous les etats sont inutiles */

  printtime(elag_minimize(a1,symbols));
  return a1;
}
