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

//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unicode.h"
#include "Copyright.h"
#include "IOBuffer.h"


#define DEFAULT 0
#define THAI 1


FILE* f;
FILE* f_out;
char VIRER_DOUBLONS=1;
int REVERSE=1;
int line=0;
struct node* root;
int char_tab[0xFFFF];
int char_priorite[0xFFFF];
int langue=DEFAULT;
char line_number[5000];
int resulting_line_number=0;


void sort();
void sort_thai();
int read_line();
int read_line_thai();
void insert(unichar*);
void save();
void init_tree();
struct node* new_node();
void free_node(struct node*);
void free_transition(struct transition*);
void get_node(unichar*,int,struct node*);
void explore_node(struct node*,unichar*,int,int);
void quicksort(struct transition**,int,int);
int char_cmp(unichar,unichar);
int strcmp2(unichar*,unichar*);
struct couple_final* inserer_chaine_thai(unichar*,struct couple_final*);



void init_char_tab() {
for (int i=0;i<0xFFFF;i++) {
  char_tab[i]=0;
  char_priorite[i]=0;
}
}



//
// this function try to read the file name
//
void read_char_order(char* name) {
int c;
int ligne=1;
int priorite;
FILE* f=u_fopen(name,U_READ);
if (f==NULL) {
   fprintf(stderr,"Cannot open file %s\n",name);
   return;
}
priorite=0;
while ((c=u_fgetc(f))!=EOF) {
      if (c!='\n') {
         // we ignore the \n char
         if (char_tab[(unichar)c]!=0) {
            fprintf(stderr,"Error in %s: char 0x%x appears several times\n",name,c);
         }
         else {
              char_tab[(unichar)c]=ligne;
              if (priorite==0) priorite=(unichar)c;
              char_priorite[(unichar)c]=priorite;
         }
      } else {
        ligne++;
        priorite=0;
      }
}
u_fclose(f);
}



void traiter_parametres(int n,char **p) {
for (int i=2;i<n;i++) {
    if (!strcmp(p[i],"-y") || !strcmp(p[i],"-Y")) {
       VIRER_DOUBLONS=1;
       }
    else if (!strcmp(p[i],"-n") || !strcmp(p[i],"-N")) {
       VIRER_DOUBLONS=0;
    }
    else if (!strcmp(p[i],"-o") || !strcmp(p[i],"-O")) {
       if (i==n-1) {
          fprintf(stderr,"Missing char order file name after -o\n");
          return;
       }
       i++;
       read_char_order(p[i]);
    }
    else if (!strcmp(p[i],"-l") || !strcmp(p[i],"-L")) {
       if (i==n-1) {
          fprintf(stderr,"Missing file name after -l\n");
          return;
       }
       i++;
       strcpy(line_number,p[i]);
    }
    else if (!strcmp(p[i],"-r") || !strcmp(p[i],"-R")) REVERSE=-1;
    else if (!strcmp(p[i],"-thai")) langue=THAI;
    else fprintf(stderr,"Invalid parameter: %s\n",p[i]);
}
}


/* 
 * This function behaves in the same way that a main one, except that it does
 * not invoke the setBufferMode function and that it does not print the
 * synopsis.
 */
int main_SortTxt(int argc, char **argv) {
char new_name[1000];
init_char_tab();
line_number[0]='\0';
traiter_parametres(argc,argv);
strcpy(new_name,argv[1]);
strcat(new_name,".new");
f=u_fopen(argv[1],U_READ);
if (f==NULL) {
   fprintf(stderr,"Cannot open file %s\n",argv[1]);
   return 1;
}
f_out=u_fopen(new_name,U_WRITE);
if (f_out==NULL) {
   fprintf(stderr,"Cannot open temporary file %s\n",new_name);
   u_fclose(f);
   return 1;
}
init_tree();
switch (langue) {
       case DEFAULT: sort(); break;
       case THAI: sort_thai(); break;
}
if (line_number[0]!='\0') {
   FILE* F=u_fopen(line_number,U_WRITE);
   if (F==NULL) {
      fprintf(stderr,"Cannot write %s\n",line_number);
   }
   else {
      unichar tmp[100];
      u_int_to_string(resulting_line_number,tmp);
      u_fprints(tmp,F);
      u_fprints_char("\n",F);
      u_fclose(F);
   }
}
u_fclose(f);
u_fclose(f_out);
remove(argv[1]);
rename(new_name,argv[1]);
return 0;
}
//---------------------------------------------------------------------------



//
// returns 0 if a==b, -i if a<b and +i if a>b
//
int char_cmp(unichar a,unichar b) {
if (char_tab[a])
   if (char_tab[b]) {
      if (char_tab[a]==char_tab[b]) {
         return (a-b);
      }
      else return REVERSE*(char_tab[a]-char_tab[b]);
   }
   else {
        return REVERSE;
   }
else
    if (char_tab[b]) {
       return -1*REVERSE;
    }
    else {
         return REVERSE*(a-b);
    }
}



//
// do a unicode char to char comparison, according to the char_cmp function order
//
int strcmp2(unichar* a,unichar* b) {
int i=0;
while (a[i] && a[i]==b[i]) i++;
return (char_cmp(a[i],b[i]));
}




//
// reads all the line and put them in a sorted tree, then saves them
// exploring the tree
//
void sort() {
printf("Loading text...\n");
while (read_line());
printf("%d lines read\n",line);
save();
}


//
// reads all the line and put them in a sorted tree, then saves them
// exploring the tree
//
void sort_thai() {
printf("Loading text...\n");
while (read_line_thai());
printf("%d lines read\n",line);
save();
}



unichar ligne[10000];

//
// reads and processes a line of the text file
//
int read_line() {
int c;
int i;
int fini=1;
i=0;
while ((c=u_fgetc(f))!='\n' && c!=EOF && i<10000)
      ligne[i++]=(unichar)c;
ligne[i]='\0';
if (c==EOF) fini=0;
else line++;
if (i==0) {
   // we ignore the empty line
   return fini;
}
if (i==10000) {
   fprintf(stderr,"Line %d: line too long\n",line);
   return fini;
}
insert(ligne);
return fini;
}




//
// insert the line in the tree
//
void insert(unichar* ligne) {
get_node(ligne,0,root);
}




//
// save the lines
//
void save() {
unichar s[5000];
printf("Sorting and saving...\n");
explore_node(root,s,0,0);
}









///////////////////////////////////////////////////



struct couple_final {
       unichar* chaine;
       int n;
       struct couple_final* suivant;
};


struct node {
    struct couple_final* couple;
    struct transition* t;
};

struct transition {
    unichar c;
    struct node* n;
    struct transition* suivant;
};



struct node* new_node() {
struct node* n=(struct node*)malloc(sizeof(struct node));
n->couple=NULL;
n->t=NULL;
return n;
}


struct transition* new_transition() {
struct transition* t=(struct transition*)malloc(sizeof(struct transition));
t->n=NULL;
t->suivant=NULL;
return t;
}



void init_tree() {
root=new_node();
}


void free_couple(struct couple_final* c) {
struct couple_final* tmp;
while (c!=NULL) {
  if (c->chaine!=NULL) free(c->chaine);
  tmp=c;
  c=c->suivant;
  free(tmp);
}
}


void free_transitions(struct transition* t) {
struct transition* tmp;
while (t!=NULL) {
   free_node(t->n);
   tmp=t;
   t=t->suivant;
   free(tmp);
}
}

//
// free the node n
//
void free_node(struct node* n) {
if (n==NULL) {
  return;
}
free_couple(n->couple);
free(n);
}


//
// free the transducer tree memory
//
void free_transducer_tree() {
free_node(root);
}



//
// looks for a transition by the char c
// insert it at its sorted place if it does not exists
//
struct transition* get_transition(unichar c,struct transition* t,struct node** n) {
struct transition* tmp;
if (char_tab[c]!=0) c=(unichar)char_priorite[c];

while (t!=NULL) {
      if (t->c==c) return t;
      t=t->suivant;
}
tmp=new_transition();
tmp->c=c;
tmp->suivant=(*n)->t;
tmp->n=NULL;
(*n)->t=tmp;
return tmp;
}


//
// returns a new couple_final with the string chaine 
//
struct couple_final* new_couple_final(unichar* chaine) {
struct couple_final* c;
c=(struct couple_final*)malloc(sizeof(struct couple_final));
c->suivant=NULL;
c->n=1;
c->chaine=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(chaine)));
u_strcpy(c->chaine,chaine);
return c;
}



//
// insert the string chaine in the node n final list
//
struct couple_final* inserer_chaine(unichar* chaine,struct couple_final* couple) {
struct couple_final* tmp;

if (couple==NULL || REVERSE*strcmp2(chaine,couple->chaine)<0) {
  // if we are at the end of the list, or if we have to insert
  tmp=new_couple_final(chaine);
  tmp->suivant=couple;
  return tmp;
}

if (!strcmp2(chaine,couple->chaine)) {
   // if chaine is allready in the list
   if (!VIRER_DOUBLONS) (couple->n)++;
   return couple;
}

// if we have to explore the tail of the list
couple->suivant=inserer_chaine(chaine,couple->suivant);
return couple;
}


//
// this function look for the path to 'chaine', creating it if necessary
// the current node is n, and pos is the position in the flex string
//
void get_node(unichar* chaine,int pos,struct node* n) {
if (chaine[pos]=='\0') {
    // we are at the final node for chaine
    n->couple=inserer_chaine(chaine,n->couple);
    return;
}
// if we are not at the end of the string chaine
struct transition* trans=get_transition(chaine[pos],n->t,&n);
if (trans->n==NULL) {
    trans->n=new_node();
}
get_node(chaine,pos+1,trans->n);
}



int z=0;



#define N_NODES 2000

struct transition* tab[N_NODES];
int COMPTEUR_MAX=0;

//
// explore the node n, and then free it
//
void explore_node(struct node* n,unichar s[],int pos,int COMPTEUR) {
int i,N;
struct transition* t;
struct couple_final* couple;
struct couple_final* tmp;
struct transition *tab_temp;
if (COMPTEUR>COMPTEUR_MAX) {
   COMPTEUR_MAX=COMPTEUR;
}
if (n==NULL) {
   fprintf(stderr,"Internal error in explore_node\n");
   exit(1);
}
if (n->couple!=NULL) {
   z++;
   couple=n->couple;
   while (couple!=NULL) {
     for (i=0;i<couple->n;i++) {
       u_fprints(couple->chaine,f_out);
       u_fputc('\n',f_out);
       resulting_line_number++;
     }
     tmp=couple;
     couple=couple->suivant;
     free(tmp->chaine);
     free(tmp);
   }
   n->couple=NULL;
}
t=n->t;
N=0;
while (t!=NULL && N<N_NODES) {
      tab[N++]=t;
      t=t->suivant;
}
if (N==N_NODES) {
   fprintf(stderr,"Internal error in explore_node: more than %d nodes\n",N_NODES);
   exit(1);
}
quicksort(tab,0,N-1);
////////////////
// after sorting, we copy the result into a temporary array
tab_temp=(struct transition*)malloc(N*sizeof(struct transition));
for (int i=0;i<N;i++) {
   tab_temp[i].c=tab[i]->c;
   tab_temp[i].n=tab[i]->n;
}
////////////////
for (int i=0;i<N;i++) {
  //printf("pos = %d     N=%d\n",pos,N);
  s[pos]=tab_temp[i].c;
  explore_node(tab_temp[i].n,s,pos+1,COMPTEUR+1);
}
free(tab_temp);
free_node(n);
}





//
// on partitionne le tableau t
//
int partition_pour_quicksort(struct transition** t,int m, int n) {
unichar pivot;
struct transition* tmp;
int i = m-1;
int j = n+1;         // indice final du pivot
pivot=t[(m+n)/2]->c;
while (true) {
  do j--;
  while ((j>(m-1))&&(char_cmp(pivot,t[j]->c) < 0));
  do i++;
  while ((i<n+1)&&(char_cmp(t[i]->c,pivot) < 0));
  if (i<j) {
    tmp=t[i];
    t[i]=t[j];
    t[j]=tmp;
  } else return j;
}
}

//
// trie la liste de transition de t
//
void quicksort(struct transition** t,int m,int n) {
int p;
if (m<n) {
  p=partition_pour_quicksort(t,m,n);
  quicksort(t,m,p);
  quicksort(t,p+1,n);
}
}













/////////////////////////////////////////////////////////
//                       THAI SORT


unichar ligne_thai[10000];



//
// convert the string src into a string with no diacritic sign and
// in which initial vowels and following consons have been reversed
//
void convert_thai(unichar* src,unichar* dest) {
int i=0,j=0;;
while (src[i]!='\0') {
      if (u_is_diacritic_thai(src[i])) {
         i++;
      }
      else if (u_is_vowel_thai(src[i])) {
           dest[j]=src[i+1];
           dest[j+1]=src[i];
           i++;
           i++;
           j++;
           j++;
      }
      else {
           dest[j++]=src[i++];
      }
}
dest[j]='\0';
}



//
// this function look for the path to 'chaine', creating it if necessary
// the current node is n, and pos is the position in the flex string
//
void get_node_thai(unichar* chaine,int pos,struct node* n,unichar* real_string) {
if (chaine[pos]=='\0') {
    // we are at the final node for chaine
    n->couple=inserer_chaine_thai(real_string,n->couple);
    return;
}
// if we are not at the end of the string chaine
struct transition* trans=get_transition(chaine[pos],n->t,&n);
if (trans->n==NULL) {
    trans->n=new_node();
}
get_node_thai(chaine,pos+1,trans->n,real_string);
}





//
// reads and processes a line of the text file
//
int read_line_thai() {
int c;
int i;
int fini=1;
i=0;
while ((c=u_fgetc(f))!='\n' && c!=EOF && i<10000)
      ligne[i++]=(unichar)c;
ligne[i]='\0';
if (c==EOF) fini=0;
else line++;
if (i==0) {
   // we ignore the empty line
   return fini;
}
if (i==10000) {
   fprintf(stderr,"Line %d: line too long\n",line);
   return fini;
}
convert_thai(ligne,ligne_thai);
get_node_thai(ligne_thai,0,root,ligne);
return fini;
}



//
// insert the string chaine in the node n final list
//
struct couple_final* inserer_chaine_thai(unichar* chaine,struct couple_final* couple) {
struct couple_final* tmp;

if (couple==NULL || REVERSE*u_strcmp(chaine,couple->chaine)<0) {
  // if we are at the end of the list, or if we have to insert
  tmp=new_couple_final(chaine);
  tmp->suivant=couple;
  return tmp;
}

if (!u_strcmp(chaine,couple->chaine)) {
   // if chaine is allready in the list
   if (!VIRER_DOUBLONS) (couple->n)++;
   return couple;
}

// if we have to explore the tail of the list
couple->suivant=inserer_chaine_thai(chaine,couple->suivant);
return couple;
}

