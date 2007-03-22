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

#include "Unicode.h"
#include "Fst2.h"
#include "Alphabet.h"
#include "ParsingInfo.h"
#include "Fst2Txt_TokenTree.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "TransductionVariables.h"
#include "LocateConstants.h"
#include "Error.h"
#include "Buffer.h"


/**
 * This enumeration defines the available parsing modes.
 */
typedef enum {
   PARSING_WORD_BY_WORD,
   PARSING_CHAR_BY_CHAR,
   /* This mode allows to start parsing on a space */
   PARSING_CHAR_BY_CHAR_WITH_SPACE,
} ParsingMode;


/**
 * This structure represents the parameters required by Fst2Txt.
 */
struct fst2txt_parameters {
   FILE* input;
   FILE* output;
   Fst2* fst2;
   Alphabet* alphabet;
   OutputPolicy output_policy;
   struct fst2txt_token_tree** token_tree;
   /* n_token_trees corresponds to the number of states in the fst2, but
    * we cache it here, in order to avoid problems if the fst2 is freed
    * before 'token_tree'. */
   int n_token_trees;
   Variables* variables;
   ParsingMode parsing_mode;
   /* Here are the text buffer and the current origin in it */
   struct buffer* text_buffer;
   int current_origin;
   /* It's just a shortcut to the data array */
   unichar* buffer;
   /* This is the absolute offset of the first character in the buffer */
   int absolute_offset;
};

struct fst2txt_parameters* new_fst2txt_parameters();
void free_fst2txt_parameters(struct fst2txt_parameters*);
void build_state_token_trees(struct fst2txt_parameters*);
void parse_text(struct fst2txt_parameters*);



#define BUFFER_SIZE 1000000
#define MAX_DEPTH 300
#define MAX_OUTPUT_LENGTH 10000


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Fst2Txt <text> <fst2> <alphabet> <MODE> [-char_by_char|-char_by_char_with_space]\n"
       "     <text> : the unicode text file to be parsed\n"
       "     <fst2> : the grammar to be applied to the text\n"
       "     <alphabet> : the alphabet file for the current language.\n"
       "     <MODE> : the parsing can be done in merge mode or replace mode, using\n"
       "              -merge or -replace\n"
       "     -char_by_char : force the program to parse char by char. This\n"
       "                     option is useful for languages like Thai.\n"
       "     -char_by_char_with_space : force the program to parse char by char, allowing\n"
       "                     the matching of expressions beginning by spaces.\n\n"
       "Applies a grammar to a text. The text file is modified.\n");
}



int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

if (argc<5) {
   usage();
   return 0;
}
struct fst2txt_parameters* p=new_fst2txt_parameters();
char temp[FILENAME_MAX];
sprintf(temp,"%s.tmp",argv[1]);
p->input=u_fopen(argv[1],U_READ);
if (p->input==NULL) {
   error("Cannot open file %s\n",argv[1]);
   return 1;
}
p->output=u_fopen(temp,U_WRITE);
if (p->output==NULL) {
   error("Cannot open temporary file %s\n",temp);
   u_fclose(p->input);
   return 1;
}
p->fst2=load_fst2(argv[2],1);
if (p->fst2==NULL) {
   error("Cannot load grammar %s\n",argv[2]);
   u_fclose(p->input);
   u_fclose(p->output);
   return 1;
}
p->alphabet=load_alphabet(argv[3]);
if (p->alphabet==NULL) {
   error("Cannot load alphabet file %s\n",argv[3]);
   u_fclose(p->input);
   u_fclose(p->output);
   free_Fst2(p->fst2);
   return 1;
}
if (!strcmp(argv[4],"-merge")) {
   p->output_policy=MERGE_OUTPUTS;
}
else if (!strcmp(argv[4],"-replace")) {
   p->output_policy=REPLACE_OUTPUTS;
} else {
   error("Invalid parameter %s : the mode must be -merge or -replace\n",argv[4]);
   u_fclose(p->input);
   u_fclose(p->output);
   free_Fst2(p->fst2);
   free_alphabet(p->alphabet);
   return 1;
}
if (argc>=6) {
   if (!strcmp(argv[5],"-char_by_char")) {
      p->parsing_mode=PARSING_CHAR_BY_CHAR;
   } 
   else if (!strcmp(argv[5],"-char_by_char_with_space")) {
      p->parsing_mode=PARSING_CHAR_BY_CHAR_WITH_SPACE;
   } 
   else {
      error("Invalid parameter: %s\n",argv[5]);
      u_fclose(p->input);
      u_fclose(p->output);
      free_Fst2(p->fst2);
      free_alphabet(p->alphabet);
      return 1;
   }
}
u_printf("Applying %s in %s mode...\n",argv[2],(p->output_policy==MERGE_OUTPUTS)?"merge":"replace");
build_state_token_trees(p);
parse_text(p);
u_fclose(p->input);
u_fclose(p->output);
remove(argv[1]);
rename(temp,argv[1]);
free_Fst2(p->fst2);
free_alphabet(p->alphabet);
free_fst2txt_parameters(p);
u_printf("Done.\n");
return 0;
}


/**
 * Allocates, initializes and returns a new fst2txt_parameters structure.
 */
struct fst2txt_parameters* new_fst2txt_parameters() {
struct fst2txt_parameters* p=(struct fst2txt_parameters*)malloc(sizeof(struct fst2txt_parameters));
if (p==NULL) {
   fatal_error("Not enoug memory in new_fst2txt_parameters\n");
}
p->input=NULL;
p->output=NULL;
p->fst2=NULL;
p->alphabet=NULL;
p->token_tree=NULL;
p->n_token_trees=0;
p->variables=NULL;
p->parsing_mode=PARSING_WORD_BY_WORD;
p->text_buffer=new_buffer(BUFFER_SIZE,UNICHAR_BUFFER);
p->buffer=p->text_buffer->unichar_buffer;
p->current_origin=0;
p->absolute_offset=0;
return p;
}


/**
 * Frees the given structure
 */
void free_fst2txt_parameters(struct fst2txt_parameters* p) {
if (p==NULL) return;
for (int i=0;i<p->n_token_trees;i++) {
   free_fst2txt_token_tree(p->token_tree[i]);
}
if (p->token_tree!=NULL) {
   free(p->token_tree);
}
free_Variables(p->variables);
free_buffer(p->text_buffer);
free(p);
}


////////////////////////////////////////////////////////////////////////
// TEXT PARSING
////////////////////////////////////////////////////////////////////////


void parcourir_graphe(int,int,int,int,struct parsing_info**,struct fst2txt_parameters*);
int ecrire_transduction();


int origine_courante;
int taille_entree;
unichar output[MAX_OUTPUT_LENGTH];
unichar pile[MAX_OUTPUT_LENGTH];
int sommet;



int ecrire_sortie(FILE* f) {
u_fprintf(f,"%S",output);
return (taille_entree);
}


void empiler(unichar c) {
if (sommet > MAX_OUTPUT_LENGTH) {
  error("Maximal output stack size reached: ignoring output\n");
} else
  pile[sommet++]=c;
}


int is_variable_char(unichar c) {
return ((c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') || c=='_');
}

void empiler_chaine(unichar s[]) {
int i;
if (s==NULL) return;
for (i=0;s[i]!='\0';i++)
   empiler(s[i]);
}


void empiler_output_chaine(struct fst2txt_parameters* p,unichar s[]) {
int i=0;
if (!u_strcmp(s,"<E>")) {
  // we do nothing if the transduction is <E>
  return;
}
while (s[i]!='\0') {
      if (s[i]=='$') {
         // case of a variable name
         unichar name[100];
         int L=0;
         i++;
         while (is_variable_char(s[i])) {
           name[L++]=s[i++];
         }
         name[L]='\0';
         if (s[i]!='$') {
            error("Error: missing closing $ after $%S\n",name);
         }
         else {
             i++;
             if (L==0) {
                // case of $$ in order to print a $
                empiler('$');
             }
             else {
                 struct transduction_variable* v=get_transduction_variable(p->variables,name);
                 if (v==NULL) {
                    error("Error: undefined variable $%S\n",name);
                 }
                 else if (v->start==-1) {
                    error("Error: starting position of variable $%S undefined\n",name);
                 }
                 else if (v->end==-1) {
                    error("Error: end position of variable $%S undefined\n",name);
                 }
                 else if (v->start > v->end) {
                    error("Error: end position before starting position for variable $%S\n",name);
                 }
                 else {
                    // if the variable definition is correct
                    for (int k=v->start;k<=v->end;k++)
                      empiler(p->buffer[k+origine_courante]);
                 }
             }
         }
      }
      else {
         empiler(s[i]);
         i++;
      }
}
}


void traiter_transduction(struct fst2txt_parameters* p,unichar* s) {
if (s!=NULL) empiler_output_chaine(p,s);
}


void parse_text(struct fst2txt_parameters* p) {
fill_buffer(p->text_buffer,p->input);
int debut=p->fst2->initial_states[1];
p->variables=new_Variables(p->fst2->variables);
int n_blocks=0;
u_printf("Block %d",n_blocks);
while (p->current_origin<p->text_buffer->size) {
      if (!p->text_buffer->end_of_file
          && p->current_origin>(p->text_buffer->size-2000)) {
         /* If must change of block, we update the absolute offset, and we fill the
          * buffer. */
         p->absolute_offset=p->absolute_offset+p->current_origin;
         fill_buffer(p->text_buffer,p->current_origin,p->input);
         p->current_origin=0;
         n_blocks++;
         u_printf("\rBlock %d        ",n_blocks);
      }
      output[0]='\0';
      sommet=0;
      taille_entree=0;
      if (p->buffer[p->current_origin]!=' ' || p->parsing_mode==PARSING_CHAR_BY_CHAR_WITH_SPACE) {
         // we don't start a match on a space
        parcourir_graphe(0,debut,0,0,NULL,p);
      }
      if (!ecrire_sortie(p->output)) {
         // if no input was read, we go on
         u_fputc(p->buffer[p->current_origin],p->output);
         (p->current_origin)++;
      }
      else {
           // we increase origine_courante
           p->current_origin=p->current_origin+taille_entree;
          /* if (buffer[origine_courante]==0x0a) {
              // we don't want to separate 0d an 0a (\n)
              origine_courante++;
              update_position_in_file();
           }*/
      }
}
u_printf("\r                           \n");
free_Variables(p->variables);
p->variables=NULL;
}



void parcourir_graphe(int n_graph, // number of current graph
                     int e,       // number of current state
                     int pos,     //
                     int profondeur,
                     struct parsing_info** liste_arrivee,
                     struct fst2txt_parameters* p) {
Fst2State etat_courant=p->fst2->states[e];

if (profondeur > MAX_DEPTH) {
  
  error(  "\n"
          "Maximal stack size reached in graph %i!\n"
          "Recognized more than %i tokens starting from:\n"
          "  ",
          n_graph, MAX_DEPTH);
  for (int i=0; i<60; i++) {
    error("%S",p->buffer[p->current_origin+i]);
  }
  error("\nSkipping match at this position, trying from next token!\n");
  output[0] = '\0';  // clear output
  taille_entree = 0; // reset taille_entree
  pile[0] = '\0';    // clear output stack
  sommet = 0;        // dito
  if (liste_arrivee != NULL) {
    while (*liste_arrivee != NULL) { // free list of subgraph matches
      struct parsing_info* la_tmp=*liste_arrivee;
      *liste_arrivee=(*liste_arrivee)->next;
      free(la_tmp);
    }
  }
  return;
  //  exit(1); // don't exit, try at next position
}
profondeur++;

if (is_final_state(etat_courant)) {
   // if we are in a final state
  pile[sommet]='\0';
  if (n_graph == 0) { // in main graph
    if (pos>=taille_entree/*sommet>u_strlen(output)*/) {
      // and if the recognized input is longer than the current one, it replaces it
      u_strcpy(output,pile);
      taille_entree=(pos);
    }
  } else { // in a subgraph
    (*liste_arrivee)=insert_if_absent(pos,(*liste_arrivee),sommet,pile,p->variables);
  }
}

if (pos+p->current_origin==p->text_buffer->size) {
   // if we are at the end of the text, we return
   return;
}

int SOMMET=sommet;
int pos2;

/* If there are some letter sequence transitions like %hello, we process them */
if (p->token_tree[e]->transition_array!=NULL) {
   if (p->buffer[pos+p->current_origin]==' ') {pos2=pos+1;if (p->output_policy==MERGE_OUTPUTS) empiler(' ');}
   /* we don't keep this line because of problems occur in sentence tokenizing
    * if the return sequence is defautly considered as a separator like space
    else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
    */
   else pos2=pos;
   int position=0;
   unichar token[1000];
   if (p->parsing_mode!=PARSING_WORD_BY_WORD 
       || (is_letter(p->buffer[pos2+p->current_origin],p->alphabet) && (pos2+p->current_origin==0 || !is_letter(p->buffer[pos2+p->current_origin-1],p->alphabet)))) {
      /* If we are in character by character mode */
      while (pos2+p->current_origin<p->text_buffer->size && is_letter(p->buffer[pos2+p->current_origin],p->alphabet)) {
         token[position++]=p->buffer[(pos2++)+p->current_origin];
      }
      token[position]='\0';
      if (position!=0 &&
          !(is_letter(token[position-1],p->alphabet) && is_letter(p->buffer[pos2+p->current_origin],p->alphabet))) {
       // we proceed only if we have exactly read the contenu sequence
       // in both modes MERGE and REPLACE, we process the transduction if any
       int SOMMET2=sommet;
       Fst2Transition RES=get_matching_tags(token,p->token_tree[e],p->alphabet);
       Fst2Transition TMP;
       while (RES!=NULL) {
          sommet=SOMMET2;
          Fst2Tag etiq=p->fst2->tags[RES->tag_number];
          traiter_transduction(p,etiq->output);
          int longueur=u_strlen(etiq->input);
          unichar C=token[longueur];
          token[longueur]='\0';
          if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
             // if we are in MERGE mode, we add to ouput the char we have read
             empiler_chaine(token);
          }
          token[longueur]=C;
          parcourir_graphe(n_graph,RES->state_number,pos2-(position-longueur),profondeur,liste_arrivee,p);
          TMP=RES;
          RES=RES->next;
          free(TMP);
       }
   }
}
}

struct fst2Transition* t=etat_courant->transitions;
while (t!=NULL) {
      sommet=SOMMET;
      // we process the transition of the current state
      int n_etiq=t->tag_number;
      if (n_etiq<0) {
         // case of a sub-graph
         struct parsing_info* liste=NULL;
         unichar pile_old[MAX_OUTPUT_LENGTH];
         u_strcpy(pile_old,pile);
         parcourir_graphe((((unsigned)n_etiq)-1),p->fst2->initial_states[-n_etiq],pos,profondeur,&liste,p);
         while (liste!=NULL) {
           sommet=liste->stack_pointer;
           u_strcpy(pile,liste->stack);
           parcourir_graphe(n_graph,t->state_number,liste->position,profondeur,liste_arrivee,p);
           struct parsing_info* l_tmp=liste;
           liste=liste->next;
           free(l_tmp);
         }
         u_strcpy(pile,pile_old);
         sommet=SOMMET;
      }
      else {
         // case of a normal tag
         Fst2Tag etiq=p->fst2->tags[n_etiq];
         unichar* contenu=etiq->input;
         if (etiq->type==BEGIN_VAR_TAG) {
            // case of a $a( variable tag
            //int old;
            struct transduction_variable* L=get_transduction_variable(p->variables,etiq->variable);
            if (L==NULL) {
               fatal_error("Unknown variable: %S\n",etiq->variable);
            }
            //old=L->start;
            if (p->buffer[pos+p->current_origin]==' ' && pos+p->current_origin+1<p->text_buffer->size) {
               pos2=pos+1;
               if (p->output_policy==MERGE_OUTPUTS) empiler(' ');
            }
            //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
            else pos2=pos;
            L->start=pos2;
            parcourir_graphe(n_graph,t->state_number,pos2,profondeur,liste_arrivee,p);
            //L->start=old;
         }
         else if (etiq->type==END_VAR_TAG) {
              // case of a $a) variable tag
              //int old;
              struct transduction_variable* L=get_transduction_variable(p->variables,etiq->variable);
              if (L==NULL) {
                 fatal_error("Unknown variable: %S\n",etiq->variable);
              }
              //old=L->end;
              if (pos>0)
                L->end=pos-1;
              else L->end=pos;
              // BUG: qd changement de buffer, penser au cas start dans ancien buffer et end dans nouveau
              parcourir_graphe(n_graph,t->state_number,pos,profondeur,liste_arrivee,p);
              //L->end=old;
         }
         else if (!u_strcmp(contenu,"<MOT>")) {
              // case of transition by any sequence of letters
              if (p->buffer[pos+p->current_origin]==' ' && pos+p->current_origin+1<p->text_buffer->size) {
                 pos2=pos+1;
                 if (p->output_policy==MERGE_OUTPUTS) empiler(' ');
              }
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              if (p->parsing_mode!=PARSING_WORD_BY_WORD ||
                  ((pos2+p->current_origin)==0 || !is_letter(p->buffer[pos2+p->current_origin-1],p->alphabet))) {
                     while (pos2+p->current_origin<p->text_buffer->size && is_letter(p->buffer[pos2+p->current_origin],p->alphabet)) {
                           mot[position++]=p->buffer[(pos2++)+p->current_origin];
                     }
                     mot[position]='\0';
                     if (position!=0) {
                       // we proceed only if we have read a letter sequence
                       // in both modes MERGE and REPLACE, we process the transduction if any
                       traiter_transduction(p,etiq->output);
                       if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                         // if we are in MERGE mode, we add to ouput the char we have read
                         empiler_chaine(mot);
                       }
                       parcourir_graphe(n_graph,t->state_number,pos2,profondeur,liste_arrivee,p);
                     }
              }
         }
         else if (!u_strcmp(contenu,"<NB>")) {
              // case of transition by any sequence of digits
              if (p->buffer[pos+p->current_origin]==' ') {
                 pos2=pos+1;
                 if (p->output_policy==MERGE_OUTPUTS) empiler(' ');
              }
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              while (pos2+p->current_origin<p->text_buffer->size && (p->buffer[pos2+p->current_origin]>='0')
                     && (p->buffer[pos2+p->current_origin]<='9')) {
                 mot[position++]=p->buffer[(pos2++)+p->current_origin];
              }
              mot[position]='\0';
              if (position!=0) {
                 // we proceed only if we have read a letter sequence
                 // in both modes MERGE and REPLACE, we process the transduction if any
                 traiter_transduction(p,etiq->output);
                 if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler_chaine(mot);
                 }
                 parcourir_graphe(n_graph,t->state_number,pos2,profondeur,liste_arrivee,p);
              }
         }
         else if (!u_strcmp(contenu,"<MAJ>")) {
              // case of upper case letter sequence
              if (p->buffer[pos+p->current_origin]==' ') {pos2=pos+1;if (p->output_policy==MERGE_OUTPUTS) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              if (p->parsing_mode!=PARSING_WORD_BY_WORD ||
                  ((pos2+p->current_origin)==0 || !is_letter(p->buffer[pos2+p->current_origin-1],p->alphabet))) {
                 while (pos2+p->current_origin<p->text_buffer->size && is_upper(p->buffer[pos2+p->current_origin],p->alphabet)) {
                    mot[position++]=p->buffer[(pos2++)+p->current_origin];
                 }
                 mot[position]='\0';
                 if (position!=0 && !is_letter(p->buffer[pos2+p->current_origin],p->alphabet)) {
                   // we proceed only if we have read an upper case letter sequence
                   // which is not followed by a lower case letter
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(p,etiq->output);
                   if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(mot);
                   }
                   parcourir_graphe(n_graph,t->state_number,pos2,profondeur,liste_arrivee,p);
                 }
              }
         }
         else if (!u_strcmp(contenu,"<MIN>")) {
              // case of lower case letter sequence
              if (p->buffer[pos+p->current_origin]==' ') {pos2=pos+1;if (p->output_policy==MERGE_OUTPUTS) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              if (p->parsing_mode!=PARSING_WORD_BY_WORD ||
                  (pos2+p->current_origin==0 || !is_letter(p->buffer[pos2+p->current_origin-1],p->alphabet))) {
                 while (pos2+p->current_origin<p->text_buffer->size && is_lower(p->buffer[pos2+p->current_origin],p->alphabet)) {
                    mot[position++]=p->buffer[(pos2++)+p->current_origin];
                 }
                 mot[position]='\0';
                 if (position!=0 && !is_letter(p->buffer[pos2+p->current_origin],p->alphabet)) {
                   // we proceed only if we have read a lower case letter sequence
                   // which is not followed by an upper case letter
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(p,etiq->output);
                   if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(mot);
                   }
                   parcourir_graphe(n_graph,t->state_number,pos2,profondeur,liste_arrivee,p);
                 }
              }
         }
         else if (!u_strcmp(contenu,"<PRE>")) {
              // case of a sequence beginning by an upper case letter
              if (p->buffer[pos+p->current_origin]==' ') {pos2=pos+1;if (p->output_policy==MERGE_OUTPUTS) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              if (p->parsing_mode!=PARSING_WORD_BY_WORD ||
                  (is_upper(p->buffer[pos2+p->current_origin],p->alphabet) && (pos2+p->current_origin==0 || !is_letter(p->buffer[pos2+p->current_origin-1],p->alphabet)))) {
                 while (pos2+p->current_origin<p->text_buffer->size && is_letter(p->buffer[pos2+p->current_origin],p->alphabet)) {
                    mot[position++]=p->buffer[(pos2++)+p->current_origin];
                 }
                 mot[position]='\0';
                 if (position!=0 && !is_letter(p->buffer[pos2+p->current_origin],p->alphabet)) {
                   // we proceed only if we have read a letter sequence
                   // which is not followed by a letter
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(p,etiq->output);
                   if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(mot);
                   }
                   parcourir_graphe(n_graph,t->state_number,pos2,profondeur,liste_arrivee,p);
                 }
              }
         }
         else if (!u_strcmp(contenu,"<PNC>")) {
              // case of a punctuation sequence
              if (p->buffer[pos+p->current_origin]==' ') {pos2=pos+1;if (p->output_policy==MERGE_OUTPUTS) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar C=p->buffer[pos2+p->current_origin];
              if (C==';' || C=='!' || C=='?' ||
                  C==':' ||  C==0xbf ||
                  C==0xa1 || C==0x0e4f || C==0x0e5a ||
                  C==0x0e5b || C==0x3001 || C==0x3002 ||
                  C==0x30fb) {
                 // in both modes MERGE and REPLACE, we process the transduction if any
                 traiter_transduction(p,etiq->output);
                 if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler(C);
                 }
                 parcourir_graphe(n_graph,t->state_number,pos2+1,profondeur,liste_arrivee,p);
              }
              else {
                   // we consider the case of ...
                   // BUG: if ... appears at the end of the buffer
                   if (C=='.') {
                      if ((pos2+p->current_origin+2)<p->text_buffer->size && p->buffer[pos2+p->current_origin+1]=='.' && p->buffer[pos2+p->current_origin+2]=='.') {
                         traiter_transduction(p,etiq->output);
                         if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                            // if we are in MERGE mode, we add to ouput the ... we have read
                            empiler(C);empiler(C);empiler(C);
                         }
                         parcourir_graphe(n_graph,t->state_number,pos2+3,profondeur,liste_arrivee,p);
                      } else {
                        // we consider the . as a normal punctuation sign
                        traiter_transduction(p,etiq->output);
                        if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                          // if we are in MERGE mode, we add to ouput the char we have read
                          empiler(C);
                        }
                        parcourir_graphe(n_graph,t->state_number,pos2+1,profondeur,liste_arrivee,p);
                      }
                   }
              }
         }
         else if (!u_strcmp(contenu,"<E>")) {
              // case of an empty sequence
              // in both modes MERGE and REPLACE, we process the transduction if any
              traiter_transduction(p,etiq->output);
              parcourir_graphe(n_graph,t->state_number,pos,profondeur,liste_arrivee,p);
         }
         else if (!u_strcmp(contenu,"<^>")) {
              // case of a new line sequence
              if (p->buffer[pos+p->current_origin]=='\n') {
                 // in both modes MERGE and REPLACE, we process the transduction if any
                 traiter_transduction(p,etiq->output);
                 if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler('\n');
                 }
                 parcourir_graphe(n_graph,t->state_number,pos+1,profondeur,liste_arrivee,p);
              }
         }
         else if (!u_strcmp(contenu,"#")) {
              // case of a no space condition
              if (p->buffer[pos+p->current_origin]!=' ') {
                // in both modes MERGE and REPLACE, we process the transduction if any
                traiter_transduction(p,etiq->output);
                parcourir_graphe(n_graph,t->state_number,pos,profondeur,liste_arrivee,p);
              }
         }
         else if (!u_strcmp(contenu," ")) {
              // case of an obligatory space
              if (p->buffer[pos+p->current_origin]==' ') {
                // in both modes MERGE and REPLACE, we process the transduction if any
                traiter_transduction(p,etiq->output);
                 if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler(' ');
                 }
                parcourir_graphe(n_graph,t->state_number,pos+1,profondeur,liste_arrivee,p);
              }
         }
         else if (!u_strcmp(contenu,"<L>")) {
              // case of a single letter
              if (p->buffer[pos+p->current_origin]==' ') {pos2=pos+1;if (p->output_policy==MERGE_OUTPUTS) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              if (is_letter(p->buffer[pos2+p->current_origin],p->alphabet)) {
                // in both modes MERGE and REPLACE, we process the transduction if any
                traiter_transduction(p,etiq->output);
                 if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler(p->buffer[pos2+p->current_origin]);
                 }
                parcourir_graphe(n_graph,t->state_number,pos2+1,profondeur,liste_arrivee,p);
              }
         }
         else {
              // case of a normal letter sequence
              if (p->buffer[pos+p->current_origin]==' ') {pos2=pos+1;if (p->output_policy==MERGE_OUTPUTS) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              if (etiq->control&RESPECT_CASE_TAG_BIT_MASK) {
                 // case of exact case match
                 int position=0;
                 while (pos2+p->current_origin<p->text_buffer->size && p->buffer[pos2+p->current_origin]==contenu[position]) {
                   pos2++; position++;
                 }
                 if (contenu[position]=='\0' && position!=0 &&
                     !(is_letter(contenu[position-1],p->alphabet) && is_letter(p->buffer[pos2+p->current_origin],p->alphabet))) {
                   // we proceed only if we have exactly read the contenu sequence
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(p,etiq->output);
                   if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(contenu);
                   }
                   parcourir_graphe(n_graph,t->state_number,pos2,profondeur,liste_arrivee,p);
                 }
              }
              else {
                 // case of variable case match
                 // the letter sequences may have been caught by the arbre_etiquette structure
                 int position=0;
                 unichar mot[1000];
                 while (pos2+p->current_origin<p->text_buffer->size && is_equal_or_uppercase(contenu[position],p->buffer[pos2+p->current_origin],p->alphabet)) {
                   mot[position++]=p->buffer[(pos2++)+p->current_origin];
                 }
                 mot[position]='\0';
                 if (contenu[position]=='\0' && position!=0 &&
                     !(is_letter(contenu[position-1],p->alphabet) && is_letter(p->buffer[pos2+p->current_origin],p->alphabet))) {
                   // we proceed only if we have exactly read the contenu sequence
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(p,etiq->output);
                   if (p->output_policy==MERGE_OUTPUTS /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(mot);
                   }
                   parcourir_graphe(n_graph,t->state_number,pos2,profondeur,liste_arrivee,p);
                 }
              }
         }
      }
      t=t->next;
}
}




int not_a_letter_sequence(Fst2Tag e,Alphabet* alphabet) {
// we return false only if e is a letter sequence like %hello
if (e->control&RESPECT_CASE_TAG_BIT_MASK || e->type==BEGIN_VAR_TAG
    || e->type==END_VAR_TAG) {
   // case of @hello $a( and $a)
   return 1;
}
unichar* s=e->input;
if (!is_letter(s[0],alphabet)) return 1;
if (!u_strcmp(s,"<E>") ||
    !u_strcmp(s,"<MOT>") ||
    !u_strcmp(s,"<MAJ>") ||
    !u_strcmp(s,"<MIN>") ||
    !u_strcmp(s,"<PRE>") ||
    !u_strcmp(s,"<PNC>") ||
    !u_strcmp(s,"<L>") ||
    !u_strcmp(s,"<^>") ||
    !u_strcmp(s,"#") ||
    !u_strcmp(s," ")) {
    return 1;
}
return 0;
}



struct fst2Transition* add_tag_to_token_tree(struct fst2txt_token_tree* tree,struct fst2Transition* trans,
                                             struct fst2txt_parameters* p) {
// case 1: empty transition
if (trans==NULL) return NULL;
// case 2: transition by something else that a sequence of letter like %hello
//         or sub-graph call
if (trans->tag_number<0 || not_a_letter_sequence(p->fst2->tags[trans->tag_number],p->alphabet)) {
   trans->next=add_tag_to_token_tree(tree,trans->next,p);
   return trans;
}
struct fst2Transition* tmp=add_tag_to_token_tree(tree,trans->next,p);
add_tag(p->fst2->tags[trans->tag_number]->input,trans->tag_number,trans->state_number,tree);
return tmp;
}


/**
 * For each state of the fst2, this function builds a tree
 * containing all the outgoing tokens. This will be used to
 * speed up the exploration when there is a large number
 * of tokens in a same box of a grf.
 */
void build_state_token_trees(struct fst2txt_parameters* p) {
p->n_token_trees=p->fst2->number_of_states;
p->token_tree=(struct fst2txt_token_tree**)malloc(p->n_token_trees*sizeof(struct fst2txt_token_tree*));
if (p->token_tree==NULL) {
   fatal_error("Not enough memory in preprocess_tags\n");
}
for (int i=0;i<p->n_token_trees;i++) {
   p->token_tree[i]=new_fst2txt_token_tree();
   p->fst2->states[i]->transitions=add_tag_to_token_tree(p->token_tree[i],p->fst2->states[i]->transitions,p);
}
}
