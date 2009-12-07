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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "Fst2Automaton.h"
#include "ElagFstFilesIO.h"
#include "ElagRulesCompilation.h"
#include "File.h"
#include "String_hash.h"
#include "AutConcat.h"
#include "AutDeterminization.h"
#include "AutMinimization.h"
#include "AutComplementation.h"
#include "AutIntersection.h"
#include "ElagDebug.h"




/* Maximum number of states for a grammar before we split it in several fst2 */
#define MAX_GRAM_SIZE   128

/* This constant is used at the time of locating the bounds of Elag rules's parts */
#define ELAG_UNDEFINED (-1)


void split_elag_rule(elRule*,Encoding encoding_output,int bom_output,language_t*);
int count_constraints(Fst2Automaton*,int*);
Fst2Automaton* compile_elag_rule(elRule*,language_t*);
int get_sub_automaton(SingleGraph,SingleGraph,int,int,int);
int get_sub_automaton(SingleGraph,SingleGraph,int,SymbolType,int*);
int get_left_constraint_part(SingleGraph,SingleGraph,int,int,int*);
SingleGraph combine_constraints(elRule*,int,SingleGraph,SingleGraph,language_t*);



/**
 * Removes everything after a new line or a # in 's'.
 */
void chomp(char* s) {
while (*s!='\0') {
   if (*s=='\n' || *s=='\r' || *s=='#') {
      *s='\0';
   } else {
      s++;
   }
}
}


/**
 * Modify the given automaton so that it starts by a loop that can
 * recognize anything. If A is empty, it remains empty.
 */
void prefix_with_everything(SingleGraph A) {
struct list_int* list=get_initial_states(A);
while (list!=NULL) {
   add_outgoing_transition(A->states[list->n],new_symbol(S_LEXIC,-1),list->n);
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
      free_Transition_list(A->states[i]->outgoing_transitions,NULL);
      A->states[i]->outgoing_transitions=NULL;
      add_outgoing_transition(A->states[i],new_symbol(S_LEXIC,-1),i);
    }
  }
}


/**
 * Allocates, initializes and returns a new Elag rule from the given .fst2.
 * Returns NULL in case of error at loading the rule.
 */
elRule* new_elRule(char* fst2,Encoding encoding_output,int bom_output,language_t* language) {
elRule* rule=(elRule*)malloc(sizeof(elRule));
if (rule==NULL) {
   fatal_alloc_error("new_elRule");
}
rule->automaton=NULL;
rule->nbContexts=0;
rule->contexts=NULL;
rule->name=strdup(fst2);
if (rule->name==NULL) {
   fatal_alloc_error("new_elRule");
}
if ((rule->automaton=load_elag_grammar_automaton(rule->name,language))==NULL) {
   error("Cannot load '%s' automaton.\n", fst2);
   free(rule->name);
   free(rule);
   return NULL;
}
split_elag_rule(rule,encoding_output,bom_output,language);
return rule;
}


/**
 * Frees all the memory associated to the given rule, except its
 * automaton.
 */
void free_elRule(elRule* rule) {
if (rule==NULL) return;
if (rule->name!=NULL) free(rule->name);
for (int i=0;i<rule->nbContexts;i++) {
   free_SingleGraph(rule->contexts[i].right);
   free_SingleGraph(rule->contexts[i].left);
}
free(rule->contexts);
free(rule);
}


/**
 * This function takes an fst2 representing an Elag rule and returns
 * an automaton A so that the intersection of A and a sentence automaton
 * reject sequences that are not valid regarding this rule.
 */
Fst2Automaton* compile_elag_rule(elRule* rule,language_t* language) {
u_printf("Compiling %s... (%d context%s)\n",rule->name,rule->nbContexts,(rule->nbContexts>1)?"s":"");
/* Now, we will convert the automaton into the Elag format, i.e. with
 * transitions tagged with symbol_t* and not integers */
for (int c=0;c<rule->nbContexts;c++) {
   //convert_transitions_to_elag_ones(rule->contexts[c].left);
   elag_determinize(language,rule->contexts[c].left);
   trim(rule->contexts[c].left);
   //convert_transitions_to_elag_ones(rule->contexts[c].right);
   elag_determinize(language,rule->contexts[c].right);
   trim(rule->contexts[c].right);
}
/* We build A*.R1 */
prefix_with_everything(rule->contexts[0].left);
//u_printf("------------- anything_R1 -------------\n");
//print_graph(rule->contexts[0].left);
elag_determinize(language,rule->contexts[0].left);
//print_graph(rule->contexts[0].left);
elag_minimize(rule->contexts[0].left);
SingleGraph anything_R1=rule->contexts[0].left;
/* and R2.A* */
suffix_with_everything(rule->contexts[0].right);
elag_determinize(language,rule->contexts[0].right);
elag_minimize(rule->contexts[0].right);
SingleGraph R2_anything=rule->contexts[0].right;
/* We compute the number of constraint combinations */
int p=((rule->nbContexts-1)>=0) ? ((int)(1 << (rule->nbContexts-1))) : 0;
/* We allocate the resulting automaton */
SingleGraph result=new_SingleGraph();


for (int ens=0;ens<p;ens++) {
   /* For each combination of constraints, we produce an automaton a1
    * that does not match these constraints */
   SingleGraph a1=combine_constraints(rule,ens,anything_R1,R2_anything,language);
   /* And we make the union of it with the current automaton */
   build_union(result,a1);
   elag_determinize(language,result);
   elag_minimize(result);
}
/* Finally, we take the complement of the automaton that rejects wrong paths.
 * This new automaton recognizes correct paths, and so, the application of the
 * Elag rule will consists of intersecting this automaton with the sentence ones. */

//u_printf("------------- DUMP -------------\n");
//print_graph(result);

elag_complementation(language,result);

//u_printf("------------- AFTER COMPL -------------\n");
//print_graph(result);

trim(result);

if (result->number_of_states==0) {
   error("Grammar %s forbids everything\n",rule->name);
}
u_printf("Grammar %s compiled (%d states)\n",rule->name,result->number_of_states);
Fst2Automaton* Result=new_Fst2Automaton(rule->automaton->name,-1);
Result->automaton=result;
return Result;
}


/**
 * Compiles the given .fst2 grammar into the given .elg file.
 * Returns 0 in case of success; -1 otherwise.
 */
int compile_elag_grammar(char* grammar,char* elg_file,Encoding encoding_output,int bom_output,
                         language_t* language) {
elRule* rule=new_elRule(grammar,encoding_output,bom_output,language);
if (rule==NULL) {
   error("Unable to read grammar '%s'\n",grammar);
   return -1;
}
Fst2Automaton* A=compile_elag_rule(rule,language);
if (A==NULL) {
   fatal_error("Unable to compile rule '%s'\n",grammar);
}
free_elRule(rule);
save_automaton(A,elg_file,encoding_output,bom_output,FST_GRAMMAR);
free_Fst2Automaton(A);
return 0;
}


/**
 * This function reads a file that contains a list of Elag grammar names,
 * and it compiles them into the file 'outname'. However, if the result
 * automaton is too big, it will be saved in several automata inside
 * the output file.
 */
int compile_elag_rules(char* rulesname,char* outname,Encoding encoding_output,int bom_output,language_t* language) {
u_printf("Compilation of %s\n",rulesname);
U_FILE* f=NULL;
U_FILE* frules=u_fopen(ASCII,rulesname,U_READ);
if (frules==NULL) {
   fatal_error("Cannot open file '%s'\n",rulesname);
}
U_FILE* out=u_fopen(ASCII,outname,U_WRITE);
if (out==NULL) {
   fatal_error("cannot open file '%s'\n",outname);
}
/* Name of the file that contains the result automaton */
char fstoutname[FILENAME_MAX];
int nbRules=0;
char buf[FILENAME_MAX];
time_t start_time=time(0);
Fst2Automaton* res=NULL;
Fst2Automaton* A;
int fst_number=0;
Ustring* ustr=new_Ustring();
while (af_fgets(buf,FILENAME_MAX,frules->f)) {
   /* We read one by one the Elag grammar names in the .lst file */
   chomp(buf);
   if (*buf=='\0') {
      /* If we have an empty line */
      continue;
   }
   u_printf("\n%s...\n",buf);
   remove_extension(buf);
   strcat(buf,".elg");
   if ((f=u_fopen(ASCII,buf,U_READ))==NULL) {
      /* If the .elg file doesn't exist, we create one */
      remove_extension(buf);
      u_printf("Precompiling %s.fst2\n",buf);
      strcat(buf,".fst2");
      elRule* rule=new_elRule(buf,encoding_output,bom_output,language);
      if (rule==NULL) {
         fatal_error("Unable to read grammar '%s'\n",buf);
      }
      if ((A=compile_elag_rule(rule,language))==NULL) {
         fatal_error("Unable to compile rule '%s'\n",buf);
      }
      free_elRule(rule);
   } else {
      /* If there is already .elg, we use it */
      u_fclose(f);
      A=load_elag_grammar_automaton(buf,language);
      if (A==NULL) {
         fatal_error("Unable to load '%s'\n",buf);
      }
   }
   if (A->automaton->number_of_states==0) {
      error("Grammar %s forbids everything!\n",buf);
   }
   if (res!=NULL) {
      /* If there is already an automaton, we intersect it with the new one */
      SingleGraph tmp=res->automaton;
      res->automaton=elag_intersection(language,tmp,A->automaton,GRAMMAR_GRAMMAR);
      free_SingleGraph(tmp);
      free_Fst2Automaton(A);
      trim(res->automaton);
   } else {
      res=A;
   }
   nbRules++;
   if (res->automaton->number_of_states>MAX_GRAM_SIZE) {
      /* If the automaton is too large, we will split the grammar
       * into several automata */
      elag_minimize(res->automaton,1);
      sprintf(fstoutname,"%s-%d.elg",outname,fst_number++);
      u_fprintf(out,"<%s>\n",fstoutname);
      u_printf("Splitting big grammar in '%s' (%d states)\n",fstoutname,res->automaton->number_of_states);
      u_sprintf(ustr,"%s: compiled elag grammar",fstoutname);
      free(res->name);
      res->name=u_strdup(ustr->str);
      save_automaton(res,fstoutname,encoding_output,bom_output,FST_GRAMMAR);
      free_Fst2Automaton(res);
      res=NULL;
   }
}
if (res!=NULL) {
   /* We save the last automaton, if any */
   sprintf(fstoutname,"%s-%d.elg",outname,fst_number++);
   u_fprintf(out,"<%s>\n",fstoutname);
   u_printf("Saving grammar in '%s'(%d states)\n",fstoutname,res->automaton->number_of_states);
   elag_minimize(res->automaton,1);
   u_sprintf(ustr,"%s: compiled elag grammar",fstoutname);
   free(res->name);
   res->name=u_strdup(ustr->str);
   save_automaton(res,fstoutname,encoding_output,bom_output,FST_GRAMMAR);
   free_Fst2Automaton(res);
}
time_t end_time=time(0);
u_fclose(frules);
u_fclose(out);
free_Ustring(ustr);
u_printf("\nDone.\nElapsed time: %.0f s\n",difftime(end_time,start_time));
u_printf("\n%d rule%s from %s compiled in %s (%d automat%s)\n",
         nbRules,(nbRules>1)?"s":"",rulesname,outname,fst_number,
         (fst_number>1)?"a":"on");
return 0;
}


/**
 * This function builds and returns an automaton for pattern
 * matching of the rule's context.
 */
Fst2Automaton* make_locate_automaton(elRule* rule,language_t* language) {
Fst2Automaton* res=new_Fst2Automaton(NULL);
res->automaton=clone(rule->contexts[0].left);
/* We concatenate the left and right contexts */
elag_concat(language,res->automaton,rule->contexts[0].right);
/* Then we add loops with ignorable POS on each state */
for (int i=0;i<language->POSs->size;i++) {
   POS_t* PoS=(POS_t*)language->POSs->value[i];
   if (PoS->ignorable) {
      /* If we have a POS that can be ignored, we add a transition tagged
       * by this symbol to each state */
      for (int q=1;q<res->automaton->number_of_states;q++) {
         symbol_t* s=new_symbol_POS(PoS,-1);
         add_outgoing_transition(res->automaton->states[q],s,q);
      }
   }
}
return res;
}



/**
 * This function analyzes the given Elag rule automaton to find
 * where the rule and constraint parts are. As a side effect, it builds
 * a fst2 grammar ("foo.fst2" => "foo-conc.fst2") that can be used by
 * the Locate program to match the <!> .... <!> .... <!> part of the rule.
 */
void split_elag_rule(elRule* rule,Encoding encoding_output,int bom_output,language_t* language) {
int c;
/* This array contains the numbers of the states that are pointed to by
 * middle '<=>' of the constraints */
int constraints[ELAG_MAX_CONSTRAINTS];
int nbConstraints=count_constraints(rule->automaton,constraints);
/* +1 because we have to count the <!> .... <!> .... <!> part of the rule */
rule->nbContexts=nbConstraints+1;
rule->contexts=(elContext*)malloc(rule->nbContexts*sizeof(elContext));
if (rule->contexts==NULL) {
   fatal_alloc_error("split_elag_rule");
}
for (c=0;c<rule->nbContexts;c++) {
   rule->contexts[c].left=NULL;
   rule->contexts[c].right=NULL;
}
int endR1=ELAG_UNDEFINED;
int endR2=ELAG_UNDEFINED;
int endC2=ELAG_UNDEFINED;
for (Transition* t=rule->automaton->automaton->states[0]->outgoing_transitions;t!=NULL;t=t->next) {
   symbol_t* symbol=t->label;
   switch (symbol->type) {
      /* We split the unique <!> .... <!> .... <!> part */
      case S_EXCLAM:
         if (rule->contexts[0].left!=NULL) {
            fatal_error("Too much '<!>' tags\n",rule->name);
         }
         rule->contexts[0].left=new_SingleGraph();
         /* We look for the end of the first part of the rule */
         endR1=get_sub_automaton(rule->automaton->automaton,rule->contexts[0].left,t->state_number,0,S_EXCLAM);
         rule->contexts[0].right=new_SingleGraph();
         endR2=get_sub_automaton(rule->automaton->automaton,rule->contexts[0].right,endR1,0,S_EXCLAM);
         if (endR1==ELAG_UNDEFINED || endR2==ELAG_UNDEFINED
             || !is_final_state(rule->automaton->automaton->states[endR2])) {
            fatal_error("split_elag_rule: %s: parse error in <!> part\n",rule->name);
         }
         break;

      /* We split the nbConstraints <=> .... <=> .... <=> parts */
      case S_EQUAL:
         if (rule->contexts[1].left!=NULL) {
            fatal_error("Non deterministic .fst2 file\n");
         }
         for (c=0;c<nbConstraints;c++) {
            rule->contexts[c+1].left=new_SingleGraph();
            get_sub_automaton(rule->automaton->automaton,rule->contexts[c+1].left,t->state_number,1,constraints[c]);
            rule->contexts[c+1].right=new_SingleGraph();
            endC2=get_sub_automaton(rule->automaton->automaton,rule->contexts[c+1].right,constraints[c],0,S_EQUAL);
            if (endC2==ELAG_UNDEFINED || !is_final_state(rule->automaton->automaton->states[endC2])) {
               fatal_error("split_elag_rule: %s: parse error in <=> part\n",rule->name);
            }
         }
         break;

      default: fatal_error("Left delimitor '<!>' or '<=>' missing\n");
   }
}
if (rule->contexts[0].left==NULL) {
   fatal_error("In grammar '%s': symbol '<!>' not found.\n",rule->name);
}
char buf[FILENAME_MAX];
remove_extension(rule->name,buf);
strcat(buf,"-conc.fst2");

/* We create the.fst2 to be used by Locate */
Fst2Automaton* locate=make_locate_automaton(rule,language);
save_automaton(locate,buf,encoding_output,bom_output,FST_LOCATE);
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
   symbol=t->label;
   if (symbol->type==S_EQUAL) {
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
      symbol=t->label;
      if (t->state_number!=source && symbol->type==S_EQUAL && !is_final_state(automaton->states[t->state_number])) {
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
                      int left_constraint_part,int z) {
/* We create the initial state of the sub-automaton */
SingleGraphState state=add_state(dest);
set_initial_state(state);
/* We use this array to renumber states */
int* renumber=(int*)malloc(src->number_of_states*sizeof(int));
if (renumber==NULL) {
   fatal_alloc_error("in get_sub_automaton");
}
for (int e=0;e<src->number_of_states;e++) {
   renumber[e]=ELAG_UNDEFINED;
}
int end=ELAG_UNDEFINED;
renumber[start]=0;
if (left_constraint_part) {
   end=get_left_constraint_part(src,dest,start,z,renumber);
} else {
   end=get_sub_automaton(src,dest,start,(SymbolType)z,renumber);
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
                      int* renumber) {
int f;
int end=ELAG_UNDEFINED;
for (Transition* t=src->states[current_state]->outgoing_transitions;t!=NULL;t=t->next) {
   symbol_t* symbol=t->label;
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
         f=get_sub_automaton(src,aut_dest,t->state_number,delim,renumber);
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
                             int final,int* renumber) {
int found=0;
for (Transition* t=src->states[current_state]->outgoing_transitions;t!=NULL;t=t->next) {
   symbol_t* symbol=t->label;
   if (symbol->type==S_EQUAL) {
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
         if (get_left_constraint_part(src,dest,t->state_number,final,renumber)) {
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


/**
 * For a given set of constraints represented by the bit mask 'constraint_set',
 * this function returns an automaton A that recognizes all incorrect sequences.
 */
SingleGraph combine_constraints(elRule* rule,int constraint_set,SingleGraph anything_R1,
                                SingleGraph R2_anything,language_t* language) {
/* a1 will be the union of all the left parts of the constraints of the
 * given constraint set */
SingleGraph a1=new_SingleGraph();
/* a2 will be the union of all the right parts of the constraints that
 * are not in the given constraint set */
SingleGraph a2=new_SingleGraph();
/* dpc = pow(2,c-1) */
for (int c=1,dpc=1;c<rule->nbContexts;c++,dpc=dpc<<1) {
   if (dpc & constraint_set) {
      /* If the cth bit of the constraint set is set to 1, then we must
       * add the cth constraint's left part to a1 */
      build_union(a1,clone(rule->contexts[c].left));
      elag_determinize(language,a1);
      elag_minimize(a1);
   } else {
      /* Otherwise, we add the cth constraint's right part to a2 */
      build_union(a2,clone(rule->contexts[c].right));
      elag_determinize(language,a2);
      elag_minimize(a2);
   }
}

prefix_with_everything(a1);
elag_determinize(language,a1);
elag_minimize(a1);
elag_complementation(language,a1);
trim(a1);
SingleGraph tmp=a1;
a1=elag_intersection(language,a1,anything_R1,GRAMMAR_GRAMMAR);
free_SingleGraph(tmp);
trim(a1);
elag_minimize(a1);

suffix_with_everything(a2);
elag_determinize(language,a2);
elag_minimize(a2);
elag_complementation(language,a2);
trim(a2);
tmp=a2;
a2=elag_intersection(language,a2,R2_anything,GRAMMAR_GRAMMAR);
free_SingleGraph(tmp);
trim(a2);
elag_minimize(a2);
elag_concat(language,a1,a2);

free_SingleGraph(a2);
trim(a1);
//print_graph(a1);
elag_determinize(language,a1);
//print_graph(a1);

trim(a1);
elag_minimize(a1);
return a1;
}


