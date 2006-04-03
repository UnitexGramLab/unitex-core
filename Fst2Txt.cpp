 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//---------------------------------------------------------------------------

#include "unicode.h"
#include "Fst2.h"
#include "LiberationFst2.h"
#include "Alphabet.h"
#include "Liste_num.h"
#include "UnicharTree.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "TransductionVariables.h"
#include "LocateConstants.h"

// commmand line for testing:
//
// e:\unitex\data\french\corpus\femme.snt e:\unitex\data\french\graphs\preprocessing\sentence\sentence.fst2 e:\unitex\data\french\alphabet.txt -merge
//
// e:\femme.txt e:\replace.fst2 e:\alphabet.txt -replace
// e:\thai\thai_corpus.txt e:\thai\sentence.fst2 e:\thai\alphabet_thai.txt -merge -char_by_char

#define N_FST2 3000 // maximum number of flexional transducers
#define MERGE 0
#define REPLACE 1
#define BUFFER_SIZE 1000000
#define MAX_DEPTH 300
#define NORMAL_MODE 0
#define CHAR_BY_CHAR 1
#define CHAR_BY_CHAR_WITH_SPACE 2


void usage() {
printf("%s",COPYRIGHT);
printf("Usage: Fst2Txt <text> <fst2> <alphabet> <MODE> [-char_by_char|-char_by_char_with_space]\n");
printf("     <text> : the unicode text file to be parsed\n");
printf("     <fst2> : the grammar to be applied to the text\n");
printf("     <alphabet> : the alphabet file for the current language.\n");
printf("     <MODE> : the parsing can be done in merge mode or replace mode, using\n");
printf("              -merge or -replace\n");
printf("     -char_by_char : force the program to parse char by char. This\n");
printf("                     option is useful for languages like Thai.\n");
printf("     -char_by_char_with_space : force the program to parse char by char, allowing\n");
printf("                     the matching of expressions beginning by spaces.\n\n");
printf("Applies a grammar to a text. The text file is modified.\n");
}



FILE *f;
FILE *f_out;
Automate_fst2* fst2;
int MODE;
unichar buffer[BUFFER_SIZE];
Alphabet* alphabet;
int LENGTH;
int PARSING_MODE=NORMAL_MODE;
struct arbre_char** arbre_etiquettes;
int N;



void update_position_in_file() {
N=N+2;
if ((N%(1024*1024))==0) {
   int l=N/(1024*1024);
   printf("%d megabyte%s read...            \r",l,(l>1)?"s":"");
}
}



void update_position_in_file(int z) {
for (int i=0;i<z;i++) {
   N=N+2;
   if ((N%(1024*1024))==0) {
      int l=N/(1024*1024);
      printf("%d megabyte%s read...         \r",l,(l>1)?"s":"");
   }
}
}


void parse_text();
void pretraiter_etiquettes();
void free_arbre_etiquettes();



int main(int argc, char **argv) {
setBufferMode();

char temp[1000];
if (argc<5) {
  usage();
  return 0;
}
strcpy(temp,argv[1]);
strcat(temp,".tmp");
f=u_fopen(argv[1],U_READ);
if (f==NULL) {
   fprintf(stderr,"Cannot open file %s\n",argv[1]);
   return 1;
}
f_out=u_fopen(temp,U_WRITE);
if (f_out==NULL) {
   fprintf(stderr,"Cannot open temporary file %s\n",temp);
   u_fclose(f);
   return 1;
}
fst2=load_fst2(argv[2],1);
if (fst2==NULL) {
  fprintf(stderr,"Cannot load grammar %s\n",argv[2]);
  u_fclose(f);
  u_fclose(f_out);
  return 1;
}
alphabet=load_alphabet(argv[3]);
if (alphabet==NULL) {
  fprintf(stderr,"Cannot load alphabet file %s\n",argv[3]);
  u_fclose(f);
  u_fclose(f_out);
  return 1;
}
if (!strcmp(argv[4],"-merge")) {
   MODE=MERGE;
}
else if (!strcmp(argv[4],"-replace")) {
          MODE=REPLACE;
     }
     else {
          fprintf(stderr,"Invalid parameter %s : the mode must be -merge or -replace\n",argv[4]);
          u_fclose(f);
          u_fclose(f_out);
          free_fst2(fst2);
          free_alphabet(alphabet);
          return 1;
     }
if (argc>=6) {
   if (!strcmp(argv[5],"-char_by_char")) {
      PARSING_MODE=CHAR_BY_CHAR;
   } 
   else if (!strcmp(argv[5],"-char_by_char_with_space")) {
           PARSING_MODE=CHAR_BY_CHAR_WITH_SPACE;
   } 
   else {
     fprintf(stderr,"Invalid parameter: %s\n",argv[5]);
     u_fclose(f);
     u_fclose(f_out);
     free_fst2(fst2);
     free_alphabet(alphabet);
     return 1;
   }
}
arbre_etiquettes=(struct arbre_char**)malloc(sizeof(struct arbre_char*)*fst2->nombre_etats);
printf("Applying %s in %s mode...\n",argv[2],(MODE==MERGE)?"merge":"replace");
pretraiter_etiquettes();
parse_text();
printf("\n");
u_fclose(f);
u_fclose(f_out);
remove(argv[1]);
rename(temp,argv[1]);
free_fst2(fst2);
free_alphabet(alphabet);
free_arbre_etiquettes();
return 0;
}





////////////////////////////////////////////////////////////////////////
// TEXT PARSING
////////////////////////////////////////////////////////////////////////


void parcourir_sous_graphe(int,int,int,struct liste_num**);
void parcourir_initial(int,int,int);
int ecrire_transduction();


int origine_courante;
int taille_entree;
unichar output[10000];
unichar pile[10000];
int sommet;



int ecrire_sortie() {
u_fprints(output,f_out);
return (taille_entree);
}


void empiler(unichar c) {
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


void empiler_output_chaine(unichar s[]) {
int i=0;
if (!u_strcmp_char(s,"<E>")) {
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
            char NAME[100];
            u_to_char(NAME,name);
            fprintf(stderr,"Error: missing closing $ after $%s\n",NAME);
         }
         else {
             i++;
             if (L==0) {
                // case of $$ in order to print a $
                empiler('$');
             }
             else {
                 struct variable_list* v=get_variable(name,fst2->variables);
                 if (v==NULL) {
                    char NAME[100];
                    u_to_char(NAME,name);
                    fprintf(stderr,"Error: undefined variable $%s\n",NAME);
                 }
                 else if (v->start==-1) {
                    char NAME[100];
                    u_to_char(NAME,name);
                    fprintf(stderr,"Error: starting position of variable $%s undefined\n",NAME);
                 }
                 else if (v->end==-1) {
                    char NAME[100];
                    u_to_char(NAME,name);
                    fprintf(stderr,"Error: end position of variable $%s undefined\n",NAME);
                 }
                 else if (v->start > v->end) {
                    char NAME[100];
                    u_to_char(NAME,name);
                    fprintf(stderr,"Error: end position before starting position for variable $%s\n",NAME);
                 }
                 else {
                    // if the variable definition is correct
                    for (int k=v->start;k<=v->end;k++)
                      empiler(buffer[k+origine_courante]);
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


void traiter_transduction(unichar* s) {
if (s!=NULL) empiler_output_chaine(s);
}


void block_change() {
int i;
for (i=origine_courante;i<BUFFER_SIZE;i++) {
  // first, we copy the end of the buffer at the beginning
  buffer[i-origine_courante]=buffer[i];
}
int N=BUFFER_SIZE-origine_courante;
int l=u_fread(buffer+N/*,sizeof(unichar)*/,origine_courante,f);
origine_courante=0;
LENGTH=N+l;
}


void parse_text() {
LENGTH=u_fread(buffer/*,sizeof(unichar)*/,BUFFER_SIZE,f);
origine_courante=0;
N=2;
int debut=fst2->debut_graphe_fst2[1];
while (origine_courante<LENGTH) {
      if (LENGTH==BUFFER_SIZE && origine_courante>(LENGTH-200)) {
         // if must change of block
         block_change();
      }
      output[0]='\0';
      sommet=0;
      taille_entree=0;
      if (buffer[origine_courante]!=' ' || PARSING_MODE==CHAR_BY_CHAR_WITH_SPACE) {
         // we don't start a match on a space
         parcourir_initial(debut,0,0);
      }
      if (!ecrire_sortie()) {
         // if no input was read, we go on
         if (buffer[origine_courante]==0x0d) {origine_courante++;}
         u_fputc(buffer[origine_courante],f_out);
         origine_courante++;
         update_position_in_file();
      }
      else {
           // we increase origine_courante
           origine_courante=origine_courante+taille_entree;
           update_position_in_file(taille_entree);
           if (buffer[origine_courante]==0x0a) {
              // we don't want to separate 0d an 0a (\n)
              origine_courante++;
              update_position_in_file();
           }
      }
}
}



void parcourir_initial(int e,int pos,int profondeur) {
Fst2State etat_courant=fst2->etat[e];

if (profondeur > MAX_DEPTH) {
   fprintf(stderr,"Parsing error: there might be an infinite recursion in the grammar\n");
   exit(1);
}
if (etat_courant->control&FST2_FINAL_STATE_BIT_MASK) {
   // if we are in a final state
   if (pos>=taille_entree/*sommet>u_strlen(output)*/) {
      // and if the recognized input is longer than the current one, it replaces it
      pile[sommet]='\0';
      u_strcpy(output,pile);
      taille_entree=(pos);
   }
}

if (pos+origine_courante==LENGTH) {
   // if we are at the end of the text, we return
   return;
}

int SOMMET=sommet;
int pos2;

// if there are some letter sequence transitions like %hello, we process them
if (arbre_etiquettes[e]->trans!=NULL) {
   int position=0;
   unichar mot[1000];
   if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
   /* we don't keep this line because of problems occur in sentence tokenizing
      if the return sequence is defautly considered as a separator like space
   else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
   */
   else pos2=pos;

if (PARSING_MODE!=NORMAL_MODE 
    || (is_letter(buffer[pos2+origine_courante],alphabet) && (pos2+origine_courante==0 || !is_letter(buffer[pos2+origine_courante-1],alphabet)))) {
   while (pos2+origine_courante<LENGTH && is_letter(buffer[pos2+origine_courante],alphabet)) {
      mot[position++]=buffer[(pos2++)+origine_courante];
   }
   mot[position]='\0';
   if (position!=0 &&
       !(is_letter(mot[position-1],alphabet) && is_letter(buffer[pos2+origine_courante],alphabet))) {
       // we proceed only if we have exactly read the contenu sequence
       // in both modes MERGE and REPLACE, we process the transduction if any
       int SOMMET2=sommet;
       struct liste_nombres* RES=get_matching_etiquettes(mot,arbre_etiquettes[e],alphabet,PARSING_MODE);
       struct liste_nombres* TMP;
       while (RES!=NULL) {
          sommet=SOMMET2;
          Fst2Tag etiq=fst2->etiquette[RES->etiq];
          traiter_transduction(etiq->output);
          int longueur=u_strlen(etiq->input);
          unichar C=mot[longueur];
          mot[longueur]='\0';
          if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
             // if we are in MERGE mode, we add to ouput the char we have read
             empiler_chaine(mot);
          }
          mot[longueur]=C;
          parcourir_initial(RES->arr,pos2-(position-longueur),profondeur+1);
          TMP=RES;
          RES=RES->suivant;
          free(TMP);
       }
   }
}
}


struct transition_fst* t=etat_courant->transitions;
while (t!=NULL) {
      sommet=SOMMET;
      // we process the transition of the current state
      int n_etiq=t->etiquette;
      if (n_etiq<0) {
         // case of a sub-graph
         struct liste_num* liste=NULL;
         unichar pile_old[1000];
         u_strcpy(pile_old,pile);
         parcourir_sous_graphe(fst2->debut_graphe_fst2[-n_etiq],pos,profondeur+1,&liste);
         while (liste!=NULL) {
           sommet=liste->sommet;
           u_strcpy(pile,liste->pile);
           parcourir_initial(t->arr,liste->n,profondeur+1);
           struct liste_num* l_tmp=liste;
           liste=liste->suivant;
           free(l_tmp);
         }
         u_strcpy(pile,pile_old);
         sommet=SOMMET;
      }
      else {
         // case of a normal tag
         Fst2Tag etiq=fst2->etiquette[n_etiq];
         unichar* contenu=etiq->input;
         if (etiq->control & START_VAR_TAG_BIT_MASK) {
            // case of a $a( variable tag
            //int old;
            struct variable_list* L=get_variable(contenu,fst2->variables);
            //old=L->start;
            if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
            //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
            else pos2=pos;
            L->start=pos2;
            parcourir_initial(t->arr,pos2,profondeur+1);
            //L->start=old;
         }
         else if (etiq->control & END_VAR_TAG_BIT_MASK) {
              // case of a $a) variable tag
              //int old;
              struct variable_list* L=get_variable(contenu,fst2->variables);
              //old=L->end;
              if (pos>0)
                L->end=pos-1;
              else L->end=pos;
              // BUG: qd changement de buffer, penser au cas start dans ancien buffer et end dans nouveau
              parcourir_initial(t->arr,pos,profondeur+1);
              //L->end=old;
         }
         else if (!u_strcmp_char(contenu,"<MOT>")) {
              // case of transition by any sequence of letters
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              if (PARSING_MODE!=NORMAL_MODE ||
                  ((pos2+origine_courante)==0 || !is_letter(buffer[pos2+origine_courante-1],alphabet))) {
                     while (pos2+origine_courante<LENGTH && is_letter(buffer[pos2+origine_courante],alphabet)) {
                           mot[position++]=buffer[(pos2++)+origine_courante];
                     }
                     mot[position]='\0';
                     if (position!=0) {
                       // we proceed only if we have read a letter sequence
                       // in both modes MERGE and REPLACE, we process the transduction if any
                       traiter_transduction(etiq->output);
                       if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                         // if we are in MERGE mode, we add to ouput the char we have read
                         empiler_chaine(mot);
                       }
                       parcourir_initial(t->arr,pos2,profondeur+1);
                     }
              }
         }
         else if (!u_strcmp_char(contenu,"<NB>")) {
              // case of transition by any sequence of digits
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              while (pos2+origine_courante<LENGTH && (buffer[pos2+origine_courante]>='0')
                     && (buffer[pos2+origine_courante]<='9')) {
                 mot[position++]=buffer[(pos2++)+origine_courante];
              }
              mot[position]='\0';
              if (position!=0) {
                 // we proceed only if we have read a letter sequence
                 // in both modes MERGE and REPLACE, we process the transduction if any
                 traiter_transduction(etiq->output);
                 if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler_chaine(mot);
                 }
                 parcourir_initial(t->arr,pos2,profondeur+1);
              }
         }
         else if (!u_strcmp_char(contenu,"<MAJ>")) {
              // case of upper case letter sequence
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              if (PARSING_MODE!=NORMAL_MODE ||
                  ((pos2+origine_courante)==0 || !is_letter(buffer[pos2+origine_courante-1],alphabet))) {
                 while (pos2+origine_courante<LENGTH && is_upper(buffer[pos2+origine_courante],alphabet)) {
                    mot[position++]=buffer[(pos2++)+origine_courante];
                 }
                 mot[position]='\0';
                 if (position!=0 && !is_letter(buffer[pos2+origine_courante],alphabet)) {
                   // we proceed only if we have read an upper case letter sequence
                   // which is not followed by a lower case letter
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(etiq->output);
                   if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(mot);
                   }
                   parcourir_initial(t->arr,pos2,profondeur+1);
                 }
              }
         }
         else if (!u_strcmp_char(contenu,"<MIN>")) {
              // case of lower case letter sequence
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              if (PARSING_MODE!=NORMAL_MODE ||
                  (pos2+origine_courante==0 || !is_letter(buffer[pos2+origine_courante-1],alphabet))) {
                 while (pos2+origine_courante<LENGTH && is_lower(buffer[pos2+origine_courante],alphabet)) {
                    mot[position++]=buffer[(pos2++)+origine_courante];
                 }
                 mot[position]='\0';
                 if (position!=0 && !is_letter(buffer[pos2+origine_courante],alphabet)) {
                   // we proceed only if we have read a lower case letter sequence
                   // which is not followed by an upper case letter
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(etiq->output);
                   if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(mot);
                   }
                   parcourir_initial(t->arr,pos2,profondeur+1);
                 }
              }
         }
         else if (!u_strcmp_char(contenu,"<PRE>")) {
              // case of a sequence beginning by an upper case letter
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              if (PARSING_MODE!=NORMAL_MODE ||
                  (is_upper(buffer[pos2+origine_courante],alphabet) && (pos2+origine_courante==0 || !is_letter(buffer[pos2+origine_courante-1],alphabet)))) {
                 while (pos2+origine_courante<LENGTH && is_letter(buffer[pos2+origine_courante],alphabet)) {
                    mot[position++]=buffer[(pos2++)+origine_courante];
                 }
                 mot[position]='\0';
                 if (position!=0 && !is_letter(buffer[pos2+origine_courante],alphabet)) {
                   // we proceed only if we have read a letter sequence
                   // which is not followed by a letter
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(etiq->output);
                   if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(mot);
                   }
                   parcourir_initial(t->arr,pos2,profondeur+1);
                 }
              }
         }
         else if (!u_strcmp_char(contenu,"<PNC>")) {
              // case of a punctuation sequence
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar C=buffer[pos2+origine_courante];
              if (C==';' || C=='!' || C=='?' ||
                  C==':' ||  C==0xbf ||
                  C==0xa1 || C==0x0e4f || C==0x0e5a ||
                  C==0x0e5b || C==0x3001 || C==0x3002 ||
                  C==0x30fb) {
                 // in both modes MERGE and REPLACE, we process the transduction if any
                 traiter_transduction(etiq->output);
                 if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler(C);
                 }
                 parcourir_initial(t->arr,pos2+1,profondeur+1);
              }
              else {
                   // we consider the case of ...
                   // BUG: if ... appears at the end of the buffer
                   if (C=='.') {
                      if ((pos2+origine_courante+2)<LENGTH && buffer[pos2+origine_courante+1]=='.' && buffer[pos2+origine_courante+2]=='.') {
                         traiter_transduction(etiq->output);
                         if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                            // if we are in MERGE mode, we add to ouput the ... we have read
                            empiler(C);empiler(C);empiler(C);
                         }
                         parcourir_initial(t->arr,pos2+3,profondeur+1);
                      } else {
                        // we consider the . as a normal punctuation sign
                        traiter_transduction(etiq->output);
                        if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                          // if we are in MERGE mode, we add to ouput the char we have read
                          empiler(C);
                        }
                        parcourir_initial(t->arr,pos2+1,profondeur+1);
                      }
                   }
              }
         }
         else if (!u_strcmp_char(contenu,"<E>")) {
              // case of an empty sequence
              // in both modes MERGE and REPLACE, we process the transduction if any
              traiter_transduction(etiq->output);
              parcourir_initial(t->arr,pos,profondeur+1);
         }
         else if (!u_strcmp_char(contenu,"<^>")) {
              // case of a new line sequence
              if (buffer[pos+origine_courante]==0x0d) {
                 // in both modes MERGE and REPLACE, we process the transduction if any
                 traiter_transduction(etiq->output);
                 if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler(0x0a);
                 }
                 // we go on at pos+2 because \n is a two char code 0d 0a
                 parcourir_initial(t->arr,pos+2,profondeur+1);
              }
         }
         else if (!u_strcmp_char(contenu,"#")) {
              // case of a no space condition
              if (buffer[pos+origine_courante]!=' ') {
                // in both modes MERGE and REPLACE, we process the transduction if any
                traiter_transduction(etiq->output);
                parcourir_initial(t->arr,pos,profondeur+1);
              }
         }
         else if (!u_strcmp_char(contenu," ")) {
              // case of an obligatory space
              if (buffer[pos+origine_courante]==' ') {
                // in both modes MERGE and REPLACE, we process the transduction if any
                traiter_transduction(etiq->output);
                 if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler(' ');
                 }
                parcourir_initial(t->arr,pos+1,profondeur+1);
              }
         }
         else if (!u_strcmp_char(contenu,"<L>")) {
              // case of a single letter
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              if (is_letter(buffer[pos2+origine_courante],alphabet)) {
                // in both modes MERGE and REPLACE, we process the transduction if any
                traiter_transduction(etiq->output);
                 if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler(buffer[pos2+origine_courante]);
                 }
                parcourir_initial(t->arr,pos2+1,profondeur+1);
              }
         }
         else {
              // case of a normal letter sequence
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              if (etiq->control&RESPECT_CASE_TAG_BIT_MASK) {
                 // case of exact case match
                 int position=0;
                 while (pos2+origine_courante<LENGTH && buffer[pos2+origine_courante]==contenu[position]) {
                   pos2++; position++;
                 }
                 if (contenu[position]=='\0' && position!=0 &&
                     !(is_letter(contenu[position-1],alphabet) && is_letter(buffer[pos2+origine_courante],alphabet))) {
                   // we proceed only if we have exactly read the contenu sequence
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(etiq->output);
                   if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(contenu);
                   }
                   parcourir_initial(t->arr,pos2,profondeur+1);
                 }
              }
              else {
                 // case of variable case match
                 // the letter sequences may have been caught by the arbre_etiquette structure
                 int position=0;
                 unichar mot[1000];
                 while (pos2+origine_courante<LENGTH && is_equal_or_uppercase(contenu[position],buffer[pos2+origine_courante],alphabet)) {
                   mot[position++]=buffer[(pos2++)+origine_courante];
                 }
                 mot[position]='\0';
                 if (contenu[position]=='\0' && position!=0 &&
                     !(is_letter(contenu[position-1],alphabet) && is_letter(buffer[pos2+origine_courante],alphabet))) {
                   // we proceed only if we have exactly read the contenu sequence
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(etiq->output);
                   if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(mot);
                   }
                   parcourir_initial(t->arr,pos2,profondeur+1);
                 }
              }
         }
      }
      t=t->suivant;
}
}




void parcourir_sous_graphe(int e,int pos,int profondeur,struct liste_num** liste_arrivee) {
Fst2State etat_courant=fst2->etat[e];
if (profondeur > MAX_DEPTH) {
   fprintf(stderr,"Parsing error: there might be an infinite recursion in the grammar\n");
   exit(1);
}
if (etat_courant->control&FST2_FINAL_STATE_BIT_MASK) {
   // if we are in a final state
   pile[sommet]='\0';
   (*liste_arrivee)=inserer_si_absent(pos,(*liste_arrivee),sommet,pile);
}

if (pos+origine_courante==LENGTH) {
   // if we are at the end of the text, we return
   return;
}

int SOMMET=sommet;
int pos2;


// if there are some letter sequence transitions like %hello, we process them
if (arbre_etiquettes[e]->trans!=NULL) {
   int position=0;
   unichar mot[1000];

   if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
   /* we don't keep this line because of problems occur in sentence tokenizing
      if the return sequence is defautly considered as a separator like space
   else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
   */
   else pos2=pos;

if (PARSING_MODE!=NORMAL_MODE ||
    (is_letter(buffer[pos2+origine_courante],alphabet) && (pos2+origine_courante==0 || !is_letter(buffer[pos2+origine_courante-1],alphabet)))) {

   while (pos2+origine_courante<LENGTH && is_letter(buffer[pos2+origine_courante],alphabet)) {
     mot[position++]=buffer[(pos2++)+origine_courante];
   }
   mot[position]='\0';
   if (position!=0 &&
       !(is_letter(mot[position-1],alphabet) && is_letter(buffer[pos2+origine_courante],alphabet))) {
       // we proceed only if we have exactly read the contenu sequence
       // in both modes MERGE and REPLACE, we process the transduction if any
       struct liste_nombres* RES=get_matching_etiquettes(mot,arbre_etiquettes[e],alphabet,PARSING_MODE);
       struct liste_nombres* TMP;
       int SOMMET2=sommet;
       while (RES!=NULL) {
          sommet=SOMMET2;
          Fst2Tag etiq=fst2->etiquette[RES->etiq];
          traiter_transduction(etiq->output);
          int longueur=u_strlen(etiq->input);
          unichar C=mot[longueur];
          mot[longueur]='\0';
          if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
             // if we are in MERGE mode, we add to ouput the char we have read
             empiler_chaine(mot);
          }
          mot[longueur]=C;
          parcourir_sous_graphe(RES->arr,pos2-(position-longueur),profondeur+1,liste_arrivee);
          TMP=RES;
          RES=RES->suivant;
          free(TMP);
       }
   }
}
}

struct transition_fst* t=etat_courant->transitions;
while (t!=NULL) {
      sommet=SOMMET;
      // we process the transition of the current state
      int n_etiq=t->etiquette;
      if (n_etiq<0) {
         // case of a sub-graph
         struct liste_num* liste=NULL;
         unichar pile_old[1000];
         u_strcpy(pile_old,pile);
         parcourir_sous_graphe(fst2->debut_graphe_fst2[-n_etiq],pos,profondeur+1,&liste);
         while (liste!=NULL) {
           sommet=liste->sommet;
           u_strcpy(pile,liste->pile);
           parcourir_sous_graphe(t->arr,liste->n,profondeur+1,liste_arrivee);
           struct liste_num* l_tmp=liste;
           liste=liste->suivant;
           free(l_tmp);
         }
         u_strcpy(pile,pile_old);
      }
      else {
         // case of a normal tag
         Fst2Tag etiq=fst2->etiquette[n_etiq];
         unichar* contenu=etiq->input;
         if (etiq->control & START_VAR_TAG_BIT_MASK) {
            // case of a $a( variable tag
            //int old;
            struct variable_list* L=get_variable(contenu,fst2->variables);
            //old=L->start;
            if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
            //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
            else pos2=pos;
            L->start=pos2;
            parcourir_sous_graphe(t->arr,pos2,profondeur+1,liste_arrivee);
            //L->start=old;
         }
         else if (etiq->control & END_VAR_TAG_BIT_MASK) {
              // case of a $a) variable tag
              //int old;
              struct variable_list* L=get_variable(contenu,fst2->variables);
              //old=L->end;
              if (pos>0)
                L->end=pos-1;
              else L->end=pos;
              // BUG: qd changement de buffer, penser au cas start dans ancien buffer et end dans nouveau
              parcourir_sous_graphe(t->arr,pos,profondeur+1,liste_arrivee);
              //L->end=old;
         }
         else if (!u_strcmp_char(contenu,"<MOT>")) {
              // case of transition by any sequence of letters
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              if (PARSING_MODE!=NORMAL_MODE ||
                  (pos2+origine_courante==0 || !is_letter(buffer[pos2+origine_courante-1],alphabet))) {
                     while (pos2+origine_courante<LENGTH && is_letter(buffer[pos2+origine_courante],alphabet)) {
                           mot[position++]=buffer[(pos2++)+origine_courante];
                     }
                     mot[position]='\0';
                     if (position!=0) {
                       // we proceed only if we have read a letter sequence
                       // in both modes MERGE and REPLACE, we process the transduction if any
                       traiter_transduction(etiq->output);
                       if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                         // if we are in MERGE mode, we add to ouput the char we have read
                         empiler_chaine(mot);
                       }
                       parcourir_sous_graphe(t->arr,pos2,profondeur+1,liste_arrivee);
                     }
              }
         }
         else if (!u_strcmp_char(contenu,"<NB>")) {
              // case of transition by any sequence of digits
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              while (pos2+origine_courante<LENGTH && (buffer[pos2+origine_courante]>='0')
                     && (buffer[pos2+origine_courante]<='9')) {
                 mot[position++]=buffer[(pos2++)+origine_courante];
              }
              mot[position]='\0';
              if (position!=0) {
                 // we proceed only if we have read a letter sequence
                 // in both modes MERGE and REPLACE, we process the transduction if any
                 traiter_transduction(etiq->output);
                 if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler_chaine(mot);
                 }
                 parcourir_sous_graphe(t->arr,pos2,profondeur+1,liste_arrivee);
              }
         }
         else if (!u_strcmp_char(contenu,"<MAJ>")) {
              // case of upper case letter sequence
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              if (PARSING_MODE!=NORMAL_MODE ||
                  (pos2+origine_courante==0 || !is_letter(buffer[pos2+origine_courante-1],alphabet))) {
                 while (pos2+origine_courante<LENGTH && is_upper(buffer[pos2+origine_courante],alphabet)) {
                    mot[position++]=buffer[(pos2++)+origine_courante];
                 }
                 mot[position]='\0';
                 if (position!=0 && !is_letter(buffer[pos2+origine_courante],alphabet)) {
                   // we proceed only if we have read an upper case letter sequence
                   // which is not followed by a lower case letter
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(etiq->output);
                   if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(mot);
                   }
                   parcourir_sous_graphe(t->arr,pos2,profondeur+1,liste_arrivee);
                 }
              }
         }
         else if (!u_strcmp_char(contenu,"<MIN>")) {
              // case of lower case letter sequence
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              if (PARSING_MODE!=NORMAL_MODE ||
                  (pos2+origine_courante==0 || !is_letter(buffer[pos2+origine_courante-1],alphabet))) {
                 while (pos2+origine_courante<LENGTH && is_lower(buffer[pos2+origine_courante],alphabet)) {
                    mot[position++]=buffer[(pos2++)+origine_courante];
                 }
                 mot[position]='\0';
                 if (position!=0 && !is_letter(buffer[pos2+origine_courante],alphabet)) {
                   // we proceed only if we have read a lower case letter sequence
                   // which is not followed by an upper case letter
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(etiq->output);
                   if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(mot);
                   }
                   parcourir_sous_graphe(t->arr,pos2,profondeur+1,liste_arrivee);
                 }
              }
         }
         else if (!u_strcmp_char(contenu,"<PRE>")) {
              // case of a sequence beginning by an upper case letter
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar mot[1000];
              int position=0;
              if (PARSING_MODE!=NORMAL_MODE ||
                  (is_upper(buffer[pos2+origine_courante],alphabet) && (pos2+origine_courante==0 || !is_letter(buffer[pos2+origine_courante-1],alphabet)))) {
                 while (pos2+origine_courante<LENGTH && is_letter(buffer[pos2+origine_courante],alphabet)) {
                    mot[position++]=buffer[(pos2++)+origine_courante];
                 }
                 mot[position]='\0';
                 if (position!=0 && !is_letter(buffer[pos2+origine_courante],alphabet)) {
                   // we proceed only if we have read a letter sequence
                   // which is not followed by a letter
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(etiq->output);
                   if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(mot);
                   }
                   parcourir_sous_graphe(t->arr,pos2,profondeur+1,liste_arrivee);
                 }
              }
         }
         else if (!u_strcmp_char(contenu,"<PNC>")) {
              // case of a punctuation sequence
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              unichar C=buffer[pos2+origine_courante];
              if (C==';' || C=='!' || C=='?' ||
                  C==':' || C==0xbf ||
                  C==0xa1 || C==0x0e4f || C==0x0e5a ||
                  C==0x0e5b || C==0x3001 || C==0x3002 ||
                  C==0x30fb) {
                 // in both modes MERGE and REPLACE, we process the transduction if any
                 traiter_transduction(etiq->output);
                 if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler(C);
                 }
                 parcourir_sous_graphe(t->arr,pos2+1,profondeur+1,liste_arrivee);
              }
              else {
                   // we consider the case of ...
                   // BUG: if ... appears at the end of the buffer
                   if (C=='.') {
                      if ((pos2+origine_courante+2)<LENGTH && buffer[pos2+origine_courante+1]=='.' && buffer[pos2+origine_courante+2]=='.') {
                         traiter_transduction(etiq->output);
                         if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                            // if we are in MERGE mode, we add to ouput the ... we have read
                            empiler(C);empiler(C);empiler(C);
                         }
                         parcourir_sous_graphe(t->arr,pos2+3,profondeur+1,liste_arrivee);
                      } else {
                        // we consider the . as a normal punctuation sign
                        traiter_transduction(etiq->output);
                        if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                          // if we are in MERGE mode, we add to ouput the char we have read
                          empiler(C);
                        }
                        parcourir_sous_graphe(t->arr,pos2+1,profondeur+1,liste_arrivee);
                      }
                   }
              }
         }
         else if (!u_strcmp_char(contenu,"<E>")) {
              // case of an empty sequence
              // in both modes MERGE and REPLACE, we process the transduction if any
              traiter_transduction(etiq->output);
              parcourir_sous_graphe(t->arr,pos,profondeur+1,liste_arrivee);
         }
         else if (!u_strcmp_char(contenu,"<^>")) {
              // case of a new line sequence
              if (buffer[pos+origine_courante]==0x0d) {
                 // in both modes MERGE and REPLACE, we process the transduction if any
                 traiter_transduction(etiq->output);
                 if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler(0x0a);
                 }
                 // we go on at pos+2 because \n is a two char code 0d 0a
                 parcourir_sous_graphe(t->arr,pos+2,profondeur+1,liste_arrivee);
              }
         }
         else if (!u_strcmp_char(contenu,"#")) {
              // case of a no space condition
              if (buffer[pos+origine_courante]!=' ') {
                // in both modes MERGE and REPLACE, we process the transduction if any
                traiter_transduction(etiq->output);
                parcourir_sous_graphe(t->arr,pos,profondeur+1,liste_arrivee);
              }
         }
         else if (!u_strcmp_char(contenu," ")) {
              // case of an obligatory space
              if (buffer[pos+origine_courante]==' ') {
                // in both modes MERGE and REPLACE, we process the transduction if any
                traiter_transduction(etiq->output);
                 if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler(' ');
                 }
                parcourir_sous_graphe(t->arr,pos+1,profondeur+1,liste_arrivee);
              }
         }
         else if (!u_strcmp_char(contenu,"<L>")) {
              // case of a single letter
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              if (is_letter(buffer[pos2+origine_courante],alphabet)) {
                // in both modes MERGE and REPLACE, we process the transduction if any
                traiter_transduction(etiq->output);
                 if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                    // if we are in MERGE mode, we add to ouput the char we have read
                    empiler(buffer[pos2+origine_courante]);
                 }
                parcourir_sous_graphe(t->arr,pos2+1,profondeur+1,liste_arrivee);
              }
         }
         else {
              // case of a normal letter sequence
              if (buffer[pos+origine_courante]==' ') {pos2=pos+1;if (MODE==MERGE) empiler(' ');}
              //else if (buffer[pos+origine_courante]==0x0d) {pos2=pos+2;if (MODE==MERGE) empiler(0x0a);}
              else pos2=pos;
              if (etiq->control&RESPECT_CASE_TAG_BIT_MASK) {
                 // case of exact case match
                 int position=0;
                 while (pos2+origine_courante<LENGTH && buffer[pos2+origine_courante]==contenu[position]) {
                   pos2++; position++;
                 }
                 if (contenu[position]=='\0' && position!=0 &&
                     !(is_letter(contenu[position-1],alphabet) && is_letter(buffer[pos2+origine_courante],alphabet))) {
                   // we proceed only if we have exactly read the contenu sequence
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(etiq->output);
                   if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(contenu);
                   }
                   parcourir_sous_graphe(t->arr,pos2,profondeur+1,liste_arrivee);
                 }
              }
              else {
                 // case of variable case match
                 int position=0;
                 unichar mot[1000];
                 while (pos2+origine_courante<LENGTH && is_equal_or_uppercase(contenu[position],buffer[pos2+origine_courante],alphabet)) {
                   mot[position++]=buffer[(pos2++)+origine_courante];
                 }
                 mot[position]='\0';
                 if (contenu[position]=='\0' && position!=0 &&
                     !(is_letter(contenu[position-1],alphabet) && is_letter(buffer[pos2+origine_courante],alphabet))) {
                   // we proceed only if we have exactly read the contenu sequence
                   // in both modes MERGE and REPLACE, we process the transduction if any
                   traiter_transduction(etiq->output);
                   if (MODE==MERGE /*|| etiq->transduction==NULL || etiq->transduction[0]=='\0'*/) {
                     // if we are in MERGE mode, we add to ouput the char we have read
                     empiler_chaine(mot);
                   }
                   parcourir_sous_graphe(t->arr,pos2,profondeur+1,liste_arrivee);
                 }
              }
         }
      }
      t=t->suivant;
}
}




void free_arbre_etiquettes() {
int L=fst2->nombre_etats;
for (int i=0;i<L;i++)
  free_arbre_char(arbre_etiquettes[i]);
free(arbre_etiquettes);
}


int not_a_letter_sequence(Fst2Tag e) {
// we return false only if e is a letter sequence like %hello
if (e->control&RESPECT_CASE_TAG_BIT_MASK || e->control&START_VAR_TAG_BIT_MASK
    || e->control&END_VAR_TAG_BIT_MASK) {
   // case of @hello $a( and $a)
   return 1;
}
unichar* s=e->input;
if (!is_letter(s[0],alphabet)) return 1;
if (!u_strcmp_char(s,"<E>") ||
    !u_strcmp_char(s,"<MOT>") ||
    !u_strcmp_char(s,"<MAJ>") ||
    !u_strcmp_char(s,"<MIN>") ||
    !u_strcmp_char(s,"<PRE>") ||
    !u_strcmp_char(s,"<PNC>") ||
    !u_strcmp_char(s,"<L>") ||
    !u_strcmp_char(s,"<^>") ||
    !u_strcmp_char(s,"#") ||
    !u_strcmp_char(s," ")) {
    return 1;
}
return 0;
}



struct transition_fst* pretraiter_etiquette(struct arbre_char* racine,struct transition_fst* trans) {
// case 1: empty transition
if (trans==NULL) return NULL;
// case 2: transition by something else that a sequence of letter like %hello
//         or sub-graph call
if (trans->etiquette<0 || not_a_letter_sequence(fst2->etiquette[trans->etiquette])) {
   trans->suivant=pretraiter_etiquette(racine,trans->suivant);
   return trans;
}
struct transition_fst* tmp=pretraiter_etiquette(racine,trans->suivant);
inserer_etiquette(fst2->etiquette[trans->etiquette]->input,trans->etiquette,trans->arr,racine);
return tmp;
}


void pretraiter_etiquettes() {
int L=fst2->nombre_etats;
for (int i=0;i<L;i++) {
  arbre_etiquettes[i]=new_arbre_char();
  fst2->etat[i]->transitions=pretraiter_etiquette(arbre_etiquettes[i],fst2->etat[i]->transitions);
}
}
