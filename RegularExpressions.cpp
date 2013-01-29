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

#include "RegularExpressions.h"
#include "List_int.h"
#include "Stack_int.h"
#include "Error.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define MAX_REG2GRF_STATES 10000

/**
 * This structure defines a grf state built from a regular expression.
 * It is very simple since we don't take care to the grf boxes positions.
 */
struct reg2grf_state {
   unichar* content;
   struct list_int* transitions;
};


/**
 * This structure represents the things needed during the compilation
 * of the regular expression. It contains the array of the states being
 * created.
 */
struct reg2grf_info {
   struct reg2grf_state states[MAX_REG2GRF_STATES];
   int n_states;
};



int reg_2_grf(const unichar*,int*,int*,struct reg2grf_info*);
void save_states(U_FILE*,const struct reg2grf_info*);



/**
 * Allocates, initializes and returns a new reg2grf_info structure.
 */
struct reg2grf_info* new_reg2grf_info() {
struct reg2grf_info* info=(struct reg2grf_info*)malloc(sizeof(struct reg2grf_info));
if (info==NULL) {
   fatal_alloc_error("new_reg2grf_info");
}
info->n_states=0;
return info;
}


/**
 * Frees all the memory associated to the given reg2grf_info structure.
 */
void free_reg2grf_info(struct reg2grf_info* info) {
if (info==NULL) return;
for (int i=0;i<info->n_states;i++) {
   if (info->states[i].content!=NULL) free(info->states[i].content);
   free_list_int(info->states[i].transitions);
}
free(info);
}


/**
 * Creates and adds a new state with the given content. The
 * function returns the index of the new state.
 *
 * WARNING: note that the content is not duplicated, so you
 *          need to provide a persistent pointer to the function,
 *          for instance, by calling it with u_strdup(...)
 */
int add_state(struct reg2grf_info* info,unichar* s) {
int i=(info->n_states)++;
info->states[i].content=s;
info->states[i].transitions=NULL;
return i;
}


/**
 * Adds a transition from the src state to the dest one.
 */
void add_transition(int src,int dest,struct reg2grf_info* info) {
info->states[src].transitions=sorted_insert(dest,info->states[src].transitions);
}


/**
 * This function takes a unicode string representing a regular expression and
 * compiles it into a .grf file. It returns 1 in case of success; 0 otherwise.
 */
int reg2grf(const unichar* regexp,const char* name_grf, const VersatileEncodingConfig* vec) {
if (regexp[0]=='\0') {
   error("You must specify a non empty regular expression\n");
   return 0;
}
U_FILE* out=u_fopen(vec,name_grf,U_WRITE);
if (out==NULL) {
   error("Cannot open the output file for the regular expression\n");
   return 0;
}
struct reg2grf_info* INFO=new_reg2grf_info();
/* We create the initial and final states that must have numbers 0 and 1 */
add_state(INFO,u_strdup("<E>"));
add_state(INFO,u_strdup(""));
/* We print the grf header */
u_fprintf(out,"#Unigraph\n");
u_fprintf(out,"SIZE 1313 950\n");
u_fprintf(out,"FONT Times New Roman:  12\n");
u_fprintf(out,"OFONT Times New Roman:B 12\n");
u_fprintf(out,"BCOLOR 16777215\n");
u_fprintf(out,"FCOLOR 0\n");
u_fprintf(out,"ACOLOR 12632256\n");
u_fprintf(out,"SCOLOR 16711680\n");
u_fprintf(out,"CCOLOR 255\n");
u_fprintf(out,"DBOXES y\n");
u_fprintf(out,"DFRAME y\n");
u_fprintf(out,"DDATE y\n");
u_fprintf(out,"DFILE y\n");
u_fprintf(out,"DDIR y\n");
u_fprintf(out,"DRIG n\n");
u_fprintf(out,"DRST n\n");
u_fprintf(out,"FITS 100\n");
u_fprintf(out,"PORIENT L\n");
u_fprintf(out,"#\n");

int input_state;
int output_state;
int result=reg_2_grf(regexp,&input_state,&output_state,INFO);
if (result!=1) {
   u_fclose(out);
   af_remove(name_grf);
   free_reg2grf_info(INFO);
   if (result==0) {
      error("Syntax error in regular expression\n");
   }
   return 0;
}
/* If the compilation has successed, we must link the resulting automaton piece
 * to the grf's initial and final states */
add_transition(0,input_state,INFO);
add_transition(output_state,1,INFO);
save_states(out,INFO);
free_reg2grf_info(INFO);
u_fclose(out);
return 1;
}


/**
 * This function copy src into dest converting special chars to grf format
 */
void convert_to_grf_format(unichar* dest,unichar* src) {
int i=0;
int j=0;
while (src[i]!='\0') {
   if (src[i]=='"') {
      if (i>0 && src[i-1]=='\\') {
         /* If the double quote was a protected one, we must add one
          * more protection \ to save it into the grf */
         dest[j++]='\\';
      }
      dest[j++]='\\';
   }
   dest[j++]=src[i++];
}
dest[j]='\0';
}


/**
 * Saves the given states in the given grf file.
 */
void save_states(U_FILE* f,const struct reg2grf_info* info) {
unichar temp[4096];
struct list_int* transitions;
/* We print the number of states */
u_fprintf(f,"%d\n",info->n_states);
for (int i=0;i<info->n_states;i++) {
   convert_to_grf_format(temp,info->states[i].content);
   /* Each grf line starts with the content between double quotes
    * followed by the number of outgoing transitions */
   u_fprintf(f,"\"%S\" 100 100 %d",temp,length(info->states[i].transitions));
   transitions=info->states[i].transitions;
   /* Then we print all the outgoing transitions */
   while (transitions!=NULL) {
      u_fprintf(f," %d",transitions->n);
      transitions=transitions->next;
   }
   /* And we mark the end of line with a space before it for
    * backward compatibility reasons */
   u_fprintf(f," \n");
}
}


/**
 * This function takes an input string that is supposed to start with a "
 * and it reads it until a non protected " is found. All characters read
 * are copied into 'token'. Characters are not unspecialized.
 * The function returns 1 in case of success or 0 if the end of string
 * is found.
 */
int read_double_quoted_sequence(const unichar* input,int *pos,unichar* token,int *pos_in_token) {
token[(*pos_in_token)++]=input[(*pos)++];
while (input[*pos]!='\0' && input[*pos]!='"') {
   if (input[*pos]=='\\') {
      /* If there is a \ a copy it and we take the next character */
      token[(*pos_in_token)++]=input[(*pos)++];
      if (input[*pos]=='\0') return 0;
   }
   token[(*pos_in_token)++]=input[(*pos)++];
}
if (input[*pos]=='\0') {
   error("Unterminated double quoted sequence\n");
   return 0;
}
token[(*pos_in_token)++]=input[(*pos)++];
return 1;
}


/**
 * This function takes an input string that is supposed to start with a <
 * and it reads it until a non protected > is found. If the input is of the
 * form <<...>> we also read the second >. Note that it is not necessary to
 * read in one pass a sequence of the form <...><<...>> since no difference
 * will be made with reading it in two pass, because all the elements
 * of a token are concatenated in read_angle_bracketed_sequence. All characters read
 * are copied into 'token'. Characters are not unspecialized.
 * The function returns 1 in case of success or 0 if the end of string
 * is found.
 */
int read_angle_bracketed_sequence_single(const unichar* input,int *pos,unichar* token,int *pos_in_token) {
token[(*pos_in_token)++]=input[(*pos)++];
int tmp=(*pos_in_token);
while (input[*pos]!='\0' && input[*pos]!='>') {
   if (input[*pos]=='\\') {
      /* If there is a backslash we a copy it and we take the next character */
      token[(*pos_in_token)++]=input[(*pos)++];
      if (input[*pos]=='\0') return 0;
   }
   token[(*pos_in_token)++]=input[(*pos)++];
}
if (input[*pos]=='\0') return 0;
token[(*pos_in_token)++]=input[(*pos)++];
/* Now, we have read a sequence of the form <...> and we test if it was of the form
 * <<...> because in that case, we will try to read a second > */
if (token[tmp]=='<') {
   if ((token[(*pos_in_token)++]=input[(*pos)++])!='>') {
      error("Morphological filter with no >> at end\n");
      return 0;
   }
   /* Now, we may have to read the extra information _f_ or _b_ */
   if (input[*pos]!='_') {
	   return 1;
   }
   token[(*pos_in_token)++]=input[(*pos)++];
   if (input[*pos]!='f' && input[*pos]!='b') {
	  error("Invalid morphological filter option: should be _f_ or _b_\n");
	  return 0;
   }
   token[(*pos_in_token)++]=input[(*pos)++];
   if (input[*pos]!='_') {
	   error("Invalid morphological filter option: should be _f_ or _b_\n");
	   return 0;
   }
   token[(*pos_in_token)++]=input[(*pos)++];
   return 1;
}
return 1;
}


/**
 * We try to read one of the following sequences:
 *
 *    <...>
 *    <<...>>
 *    <...><<...>>
 */
int read_angle_bracketed_sequence(const unichar* input,int *pos,unichar* token,int *pos_in_token) {
int original_pos_in_token=*pos_in_token;
if (!read_angle_bracketed_sequence_single(input,pos,token,pos_in_token)) {
   return 0;
}
if (token[original_pos_in_token]=='<' && token[original_pos_in_token+1]=='<') {
   /* If we have read a <<...>> sequence, we have finished */
   return 1;
}
if (input[*pos]=='<' && input[(*pos)+1]=='<') {
   /* If we have read a <...> sequence and if the input contains <<, then
    * we try to read a <<...>> sequence */
   if (!read_angle_bracketed_sequence_single(input,pos,token,pos_in_token)) {
      return 0;
   }
}
return 1;
}


/**
 * We try to read a tag like {eats,eat.V:P3s}
 */
int read_round_bracketed_sequence(const unichar* input,int *pos,unichar* token,int *pos_in_token) {
token[(*pos_in_token)++]=input[(*pos)++];
while (input[*pos]!='\0' && input[*pos]!='}') {
   if (input[*pos]=='\\') {
      /* If there is a backslash we a copy it and we take the next character */
      token[(*pos_in_token)++]=input[(*pos)++];
      if (input[*pos]=='\0') return 0;
   }
   token[(*pos_in_token)++]=input[(*pos)++];
}
int OK=(input[*pos]=='}');
token[(*pos_in_token)++]=input[(*pos)++];
return OK;
}


/**
 * This function reads a token from the regular expression and
 * copies it into 'token'. The function returns 1 in case of success; 0
 * otherwise.
 */
int read_token(const unichar* input,int *pos,unichar* token) {
if (input[*pos]=='\0') {
   fatal_error("read_token should not have been called with an empty input\n");
}
int i=0;
if (input[*pos]=='"') {
   /* If there is a " we try to read a sequence between double quotes */
   if (!read_double_quoted_sequence(input,pos,token,&i)) {
      return 0;
   }
   token[i]='\0';
   return 1;
}
if (input[*pos]=='<') {
   /* If there is a < we try to read a pattern of the form <....> */
   if (!read_angle_bracketed_sequence(input,pos,token,&i)) {
      return 0;
   }
   token[i]='\0';
   return 1;
}
if (input[*pos]=='{') {
   /* If there is a { we try to read a tag of the form  {....} */
   if (!read_round_bracketed_sequence(input,pos,token,&i)) {
      return 0;
   }
   token[i]='\0';
   return 1;
}

while (input[*pos]!='\0' && input[*pos]!='+' && input[*pos]!='.' && input[*pos]!='(' && input[*pos]!=')'
       && input[*pos]!='*' && input[*pos]!=' ' && input[*pos]!='<' && input[*pos]!='"') {
   if (input[*pos]=='\\') {
      /* If we find a \ we despecialize the next character if it is an operator of
       * the grammar */
      (*pos)++;
      if (input[*pos]=='\0') {
         error("Backslash at end of input in read_token\n");
         return 0;
      }
      if (input[*pos]!='+' && input[*pos]!='.' && input[*pos]!='(' && input[*pos]!=')' && input[*pos]!='*') {
         token[i++]='\\';
      }
      token[i++]=input[(*pos)++];
      continue;
   }
   /* Otherwise, if we have a single character */
   token[i++]=input[(*pos)++];
}
token[i]='\0';
return 1;
}


/**
 * This function takes a regular expression and builds the corresponding
 * Thompson automaton. To do that, we use the following grammar:
 *
 * S -> E \0
 * E -> E + E
 * E -> E . E
 * E -> E E
 * E -> E Y E
 * E -> (E)
 * E -> E*
 * E -> TOKEN
 * Y -> SPACE
 * Y -> SPACE Y
 *
 * with priority(*) > priority(.) > priority(+)
 *
 * If we have the following expression:
 *
 * <DET> (very <A>+<E>).<N> of <<es$>>
 *
 * tokens will be "<DET>", "very", "<A>", "<E>", "<N>", "of" and "<<es$>>"
 *
 * See comments in "RegularExpressions.h" for more details.
 *
 * We use two integer stacks: one for the LR analyzer that contains the item automaton
 * states number and another that contains, for each piece of automaton being built, the
 * numbers of the input and output states of this piece.
 * The function returns 1 in case of success; 0 in case of a syntax error in the
 * expression and 2 in case of a malformed token.
 */
int reg_2_grf(const unichar* regexp,int *input_state,int *output_state,struct reg2grf_info* info) {
/* transE represents the transitions tagged with the non terminal E in the LR table.
 * transY does sthe same for Y  */
int transE[19]={4,-1,5,-1,12,12,-1,15,16,-1,-1,-1,12,18,-1,12,12,-1,12};
int transY[19]={-1,-1,-1,-1,13,13,-1,-1,-1,17,-1,-1,13,-1,-1,13,13,-1,13};
struct stack_int* stack=new_stack_int(1024);
struct stack_int* couples=new_stack_int(1024);
int value=-1;
int pos=0;
unichar token[REG_EXP_MAX_LENGTH];
/* We initialize the LR analyze stack */
stacki_push(stack,0);
while (value==-1) {
   int state=stack->stack[stack->stack_pointer];
   switch (state) {
      /* state 0 */
      case 0: {
         switch(regexp[pos]) {
            case '+':
            case ')':
            case '*':
            case '.':
            case ' ':
            case '\0': {
               /* Failure */
               value=0; break;
            }
            case '(': {
               pos++;
               stacki_push(stack,2);
               break;
            }
            default: {
               /* When we read a token, we create a state with this content */
               if (!read_token(regexp,&pos,token)) {
                  /* Failure */
                  value=2; break;
               }
               int n=add_state(info,u_strdup(token));
               stacki_push(couples,n);
               stacki_push(couples,n);
               stacki_push(stack,1);
               break;
            }
         }
         break;
      }
      /* state 1 */
      case 1: {
         /* If we must reduce with the rule E -> TOKEN,
          * there is nothing to pop/push in the couple stack, but
          * we pop 1 right member */
          stacki_pop(stack);
          /* We look in the LR table where to go */
          int next_state=transE[stack->stack[stack->stack_pointer]];
          if (next_state==-1) {
             /* Failure */
             value=0; break;
             }
          stacki_push(stack,next_state);
          break;
      }
      /* state 2 */
      case 2: {
         switch(regexp[pos]) {
            case '+':
            case ')':
            case '*':
            case '.':
            case ' ':
            case '\0': {
               /* Failure */
               value=0; break;
            }
            case '(': {
               pos++;
               stacki_push(stack,2);
               break;
            }
            default: {
               /* When we read a token, we create a state with this content */
               if (!read_token(regexp,&pos,token)) {
                  /* Failure */
                  value=2; break;
               }
               int n=add_state(info,u_strdup(token));
               stacki_push(couples,n);
               stacki_push(couples,n);
               stacki_push(stack,1);
               break;
            }
         }
         break;
      }
      /* state 3 */
      case 3: {
         /* Should not appear, since we quit the automaton when we have read the final '\0' */
         fatal_error("reg_2_grf: illegal position in state 3\n");
      }
      /* state 4 */
      case 4: {
         switch(regexp[pos]) {
            case '+': {
               pos++;
               stacki_push(stack,7);
               break;
            }
            case '.': {
               pos++;
               stacki_push(stack,8);
               break;
            }
            case ' ': {
               pos++;
               stacki_push(stack,9);
               break;
            }
            case '*': {
               pos++;
               stacki_push(stack,10);
               break;
            }
            case '\0': {
               pos++;
               stacki_push(stack,11);
               break;
            }
            case '(': {
               pos++;
               stacki_push(stack,2);
               break;
            }
            case ')': {
               /* Failure */
               value=0; break;
            }
            default: {
               /* When we read a token, we create a state with this content */
               if (!read_token(regexp,&pos,token)) {
                  /* Failure */
                  value=2; break;
               }
               int n=add_state(info,u_strdup(token));
               stacki_push(couples,n);
               stacki_push(couples,n);
               stacki_push(stack,1);
               break;
            }
         }
         break;
      }
      /* state 5 */
      case 5: {
         switch(regexp[pos]) {
            case '+': {
               pos++;
               stacki_push(stack,7);
               break;
            }
            case '.': {
               pos++;
               stacki_push(stack,8);
               break;
            }
            case ' ': {
               pos++;
               stacki_push(stack,9);
               break;
            }
            case '*': {
               pos++;
               stacki_push(stack,10);
               break;
            }
            case '(': {
               pos++;
               stacki_push(stack,2);
               break;
            }
            case ')': {
               pos++;
               stacki_push(stack,14);
               break;
            }
            case '\0': {
               /* Failure */
               value=0; break;
            }
            default: {
               /* When we read a token, we create a state with this content */
               if (!read_token(regexp,&pos,token)) {
                  /* Failure */
                  value=2; break;
               }
               int n=add_state(info,u_strdup(token));
               stacki_push(couples,n);
               stacki_push(couples,n);
               stacki_push(stack,1);
               break;
            }
         }
         break;
      }
      /* state 6 */
      case 6: {
         /* Should not appear, since we quit the automaton when we have read the final '\0' */
         fatal_error("reg_2_grf: illegal position in state 6\n");
      }
      /* state 7 */
      case 7: {
         switch(regexp[pos]) {
            case '+':
            case ')':
            case '*':
            case '.':
            case ' ':
            case '\0': {
               /* Failure */
               value=0; break;
            }
            case '(': {
               pos++;
               stacki_push(stack,2);
               break;
            }
            default: {
               /* When we read a token, we create a state with this content */
               if (!read_token(regexp,&pos,token)) {
                  /* Failure */
                  value=2; break;
               }
               int n=add_state(info,u_strdup(token));
               stacki_push(couples,n);
               stacki_push(couples,n);
               stacki_push(stack,1);
               break;
            }
         }
         break;
      }
      /* state 8 */
      case 8: {
         switch(regexp[pos]) {
            case '+':
            case ')':
            case '*':
            case '.':
            case ' ':
            case '\0': {
               /* Failure */
               value=0; break;
            }
            case '(': {
               pos++;
               stacki_push(stack,2);
               break;
            }
            default: {
               /* When we read a token, we create a state with this content */
               if (!read_token(regexp,&pos,token)) {
                  /* Failure */
                  value=2; break;
               }
               int n=add_state(info,u_strdup(token));
               stacki_push(couples,n);
               stacki_push(couples,n);
               stacki_push(stack,1);
               break;
            }
         }
         break;
      }
      /* state 9 */
      case 9: {
         switch(regexp[pos]) {
            case ' ': {
               pos++;
               stacki_push(stack,9);
               break;
            }
            default:  {
               /* If we must reduce with the rule Y -> SPACE,
                * there is nothing to pop/push in the couple stack, but
                * we pop 1 right member */
               stacki_pop(stack);
               /* We look in the LR table where to go */
               int next_state=transY[stack->stack[stack->stack_pointer]];
               if (next_state==-1) {
                  /* Failure */
                  value=0; break;
               }
               stacki_push(stack,next_state);
            }
         }
         break;
      }
      /* state 10 */
      case 10: {
         /* If we must reduce with the rule rule E -> E*,
          * we pop 2 right members */
         stacki_pop(stack);
         stacki_pop(stack);
         /* We look in the LR table where to go */
         int next_state=transE[stack->stack[stack->stack_pointer]];
         if (next_state==-1) {
            /* Failure */
            value=0;
            break;
         }
         stacki_push(stack, next_state);
         /* We pop the automaton A ->.....-> B */
         int B=stacki_pop(couples);
         int A=stacki_pop(couples);
         /* Then we create 2 empty states */
         int input=add_state(info, u_strdup("<E>"));
         int output=add_state(info, u_strdup("<E>"));
         /* We rely them to A and B */
         add_transition(input, A, info);
         add_transition(B, output, info);
         /* We create a loop relying B to A */
         add_transition(B, A, info);
         /* And we rely input to output since the Kleene star matches the
          * empty word */
         add_transition(input, output, info);
         /* Finally we push the automaton input ->.....-> output */
         stacki_push(couples, input);
         stacki_push(couples, output);
         break;
      }
      /* state 11 */
      case 11: {
         /* If we must reduce by S -> E '0', we have matched the whole expression */
         /* We return the input and output state numbers of the automaton */
         *output_state=stacki_pop(couples);
         *input_state=stacki_pop(couples);
         value=1;
         break;
      }
      /* state 12 */
      case 12: {
         switch(regexp[pos]) {
            case '*': {
               pos++;
               stacki_push(stack,10);
               break;
            }
            default: {
               /* If we must reduce with the rule E -> EE,
                * we pop 2 right members */
               stacki_pop(stack);
               stacki_pop(stack);
               /* We look in the LR table where to go */
               int next_state=transE[stack->stack[stack->stack_pointer]];
               if (next_state==-1) {
                  /* Failure */
                  value=0;
                  break;
               }
               stacki_push(stack, next_state);
               /* We pop 2 automata A ->.....-> B and C ->.....-> D */
               int D=stacki_pop(couples);
               int C=stacki_pop(couples);
               int B=stacki_pop(couples);
               int A=stacki_pop(couples);
               /* We rely B to C */
               add_transition(B, C, info);
               /* And we push the automaton A ->.....-> D */
               stacki_push(couples, A);
               stacki_push(couples, D);
               break;
            }
         }
         break;
      }
      /* state 13 */
      case 13: {
         switch(regexp[pos]) {
            case '+':
            case ')':
            case '*':
            case '.':
            case ' ':
            case '\0': {
               /* Failure */
               value=0; break;
            }
            case '(': {
               pos++;
               stacki_push(stack,2);
               break;
            }
            default: {
               /* When we read a token, we create a state with this content */
               if (!read_token(regexp,&pos,token)) {
                  /* Failure */
                  value=2; break;
               }
               int n=add_state(info,u_strdup(token));
               stacki_push(couples,n);
               stacki_push(couples,n);
               stacki_push(stack,1);
               break;
            }
         }
         break;
      }
      /* state 14 */
      case 14: {
         /* If we must reduce with the rule E -> (E),
          * we pop 3 right members */
         stacki_pop(stack);
         stacki_pop(stack);
         stacki_pop(stack);
         /* We look in the LR table where to go */
         int next_state=transE[stack->stack[stack->stack_pointer]];
         if (next_state==-1) {
            /* Failure */
            value=0; break;
         }
         stacki_push(stack,next_state);
         /* And we have nothing else to do */
         break;
      }
      /* state 15 */
      case 15: {
         switch(regexp[pos]) {
            case '+':
            case ')':
            case '\0': {
               /* If we must reduce with the rule E -> E+E,
                * we pop 3 right members */
               stacki_pop(stack);
               stacki_pop(stack);
               stacki_pop(stack);
               /* We look in the LR table where to go */
               int next_state=transE[stack->stack[stack->stack_pointer]];
               if (next_state==-1) {
                  /* Failure */
                  value=0; break;
               }
               stacki_push(stack,next_state);
               /* We pop 2 automata A ->.....-> B and C ->.....-> D */
               int D=stacki_pop(couples);
               int C=stacki_pop(couples);
               int B=stacki_pop(couples);
               int A=stacki_pop(couples);
               /* Then we create 2 empty states */
               int input=add_state(info,u_strdup("<E>"));
               int output=add_state(info,u_strdup("<E>"));
               /* We rely input to A and C */
               add_transition(input,A,info);
               add_transition(input,C,info);
               /* We rely B and D to output */
               add_transition(B,output,info);
               add_transition(D,output,info);
               /* Finally we push the automaton input ->.....-> output */
               stacki_push(couples,input);
               stacki_push(couples,output);
               break;
            }
            case '*': {
               pos++;
               stacki_push(stack,10);
               break;
            }
            case '.': {
               pos++;
               stacki_push(stack,8);
               break;
            }
            case ' ': {
               pos++;
               stacki_push(stack,9);
               break;
            }
            case '(': {
               pos++;
               stacki_push(stack,2);
               break;
            }
            default: {
               /* When we read a token, we create a state with this content */
               if (!read_token(regexp,&pos,token)) {
                  /* Failure */
                  value=2; break;
               }
               int n=add_state(info,u_strdup(token));
               stacki_push(couples,n);
               stacki_push(couples,n);
               stacki_push(stack,1);
               break;
            }
         }
         break;
      }
      /* state 16 */
      case 16: {
         switch(regexp[pos]) {
            case '*': {
               pos++;
               stacki_push(stack,10);
               break;
            }
            default: {
               /* If we must reduce with the rule E -> E.E,
                * we pop 3 right members */
               stacki_pop(stack);
               stacki_pop(stack);
               stacki_pop(stack);
               /* We look in the LR table where to go */
               int next_state=transE[stack->stack[stack->stack_pointer]];
               if (next_state==-1) {
                  /* Failure */
                  value=0; break;
               }
               stacki_push(stack,next_state);
               /* We pop 2 automata A ->.....-> B and C ->.....-> D */
               int D=stacki_pop(couples);
               int C=stacki_pop(couples);
               int B=stacki_pop(couples);
               int A=stacki_pop(couples);
               /* We rely B to C */
               add_transition(B,C,info);
               /* And we push the automaton A ->.....-> D */
               stacki_push(couples,A);
               stacki_push(couples,D);
               break;
            }
         }
         break;
      }
      /* state 17 */
      case 17: {
         /* If we must reduce with the rule Y -> SPACE Y,
          * there is nothing to pop/push in the couple stack, but
          * we pop 2 right members */
         stacki_pop(stack);
         stacki_pop(stack);
         /* We look in the LR table where to go */
         int next_state=transY[stack->stack[stack->stack_pointer]];
         if (next_state==-1) {
            /* Failure */
            value=0;
            break;
         }
         stacki_push(stack, next_state);
         break;
      }
      /* state 18 */
      case 18: {
         switch(regexp[pos]) {
            case '*': {
               pos++;
               stacki_push(stack,10);
               break;
            }
            default: {
               /* If we must reduce with the rule E -> E Y E,
                * we pop 3 right members */
               stacki_pop(stack);
               stacki_pop(stack);
               stacki_pop(stack);
               /* We look in the LR table where to go */
               int next_state=transE[stack->stack[stack->stack_pointer]];
               if (next_state==-1) {
                  /* Failure */
                  value=0; break;
               }
               stacki_push(stack,next_state);
               /* We pop 2 automata A ->.....-> B and C ->.....-> D */
               int D=stacki_pop(couples);
               int C=stacki_pop(couples);
               int B=stacki_pop(couples);
               int A=stacki_pop(couples);
               /* We rely B to C */
               add_transition(B,C,info);
               /* And we push the automaton A ->.....-> D */
               stacki_push(couples,A);
               stacki_push(couples,D);
               break;
            }
         }
         break;
      }
   }
}
free_stack_int(stack);
free_stack_int(couples);
return value;
}

} // namespace unitex

