 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Unicode.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"
#include "Thai.h"
#include "getopt.h"


#define DEFAULT 0
#define THAI 1

/* Maximum length of a line in the text file to be sorted */
#define LINE_LENGTH 10000



/**
 * This structure defines a list of couples (string,int). The integer
 * represents the number of occurrences of the string, so that the list
 * 
 * a -> a -> a -> b -> c -> c
 * 
 * will be represented by:
 * 
 * (a,3) -> (b,1) -> (c,2)
 */
struct couple {
   unichar* s;
   int n;
   struct couple* next;
};

/**
 * This represents a node of the sort tree. Each node contains a list of strings,
 * represented with the 'couple' structure'.
 */
struct sort_tree_node {
   struct couple* couples;
   struct sort_tree_transition* transitions;
};

/**
 * This is a transition in the sort tree, tagged with a unichar.
 */
struct sort_tree_transition {
   unichar c;
   struct sort_tree_node* node;
   struct sort_tree_transition* next;
};


void sort();
void sort_thai();
int read_line();
int read_line_thai();
void save();
struct sort_tree_node* new_sort_tree_node();
void free_sort_tree_node(struct sort_tree_node*);
void free_transition(struct sort_tree_transition*);
void free_couple(struct couple*);
void get_node(unichar*,int,struct sort_tree_node*);
void explore_node(struct sort_tree_node*);
void quicksort(struct sort_tree_transition**,int,int);
int char_cmp(unichar,unichar);
int strcmp2(unichar*,unichar*);
struct couple* insert_string_thai(unichar*,struct couple*);


FILE* f;
FILE* f_out;
char REMOVE_DUPLICATES=1;
int REVERSE=1;
int number_of_lines=0;
struct sort_tree_node* root=new_sort_tree_node();

/* The following array gives for each character its class number, i.e.
 * 0 if it's not a letter or the number of the line it appears in the
 * char order file. */
int class_numbers[0x10000];

/* This array gives for each character its canonical character, i.e. the first
 * character of its class, or itself if it is not a letter. */
unichar canonical[0x10000];

/* For a char that is a letter, this array indicates its position in its
 * class. This information will be used for comparing characters of the
 * same class. For instance, if we have the following char order file: 
 * 
 * Aaà
 * Bb
 * Ccç
 * ...
 * 
 * we will have class_numbers['c']=3, canonical['c']='C' and priority['c']=2
 */
int priority[0x10000];

/* This array will be used to sort transitions that outgo from a node */
struct sort_tree_transition* transitions[0x10000];

int resulting_line_number=0;




/**
 * This function initializes the arrays used to determine the char order.
 */
void init_char_arrays() {
for (int i=0;i<0x10000;i++) {
   class_numbers[i]=0;
   canonical[i]=i;
   priority[i]=0;
}
}


/**
 * This function reads the given char order file.
 */
void read_char_order(char* name) {
int c;
int current_line=1;
FILE* f=u_fopen(name,U_READ);
if (f==NULL) {
   error("Cannot open file %s\n",name);
   return;
}
unichar current_canonical='\0';
int current_priority=0;
while ((c=u_fgetc(f))!=EOF) {
   if (c!='\n') {
      /* we ignore the \n char */
      if (class_numbers[(unichar)c]!=0) {
         error("Error in %s: char 0x%x appears several times\n",name,c);
      } else {
         class_numbers[(unichar)c]=current_line;
         if (current_canonical=='\0') {
            current_canonical=(unichar)c;
         }
         canonical[(unichar)c]=current_canonical;
         priority[(unichar)c]=++current_priority;
      }
   } else {
     current_line++;
     current_canonical='\0';
     current_priority=0;
   }
}
u_fclose(f);
}


void usage_SortTxt() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: SortTxt [OPTIONS] <txt>\n"
         "\n"
         "  <txt>: any unicode text file\n"
         "\n"
         "OPTIONS:\n"
         "  -n/--no_duplicates: remove duplicates (default)\n"
         "  -d/--duplicates: do not remove duplicates\n"
         "  -r/--reverse: reverse the sort order\n"
         "  -o XXX/--sort_order=XXX: use a file describing the char order for sorting\n"
         "  -l XXX/--line_info: saves the resulting number of lines in file XXX\n"
         "  -t/--thai: sort thai text\n"
         "  -h/--help: this help\n"
         "\n"
         "By default, the sort is done according the Unicode char order, removing duplicates.\n");
}


/* 
 * This function behaves in the same way that a main one, except that it does
 * not invoke the setBufferMode function.
 */
int main_SortTxt(int argc,char* argv[]) {
if (argc==1) {
   usage_SortTxt();
   return 0;
}

const char* optstring=":ndr:o:l:th";
const struct option lopts[]= {
      {"no_duplicates",no_argument,NULL,'n'},
      {"duplicates",no_argument,NULL,'d'},
      {"reverse",no_argument,NULL,'r'},
      {"sort_order",required_argument,NULL,'o'},
      {"line_info",required_argument,NULL,'l'},
      {"thai",no_argument,NULL,'t'},
      {"help",no_argument,NULL,'h'},
      {NULL,no_argument,NULL,0}
};
REMOVE_DUPLICATES=1;
REVERSE=1;
int mode=DEFAULT;
char line_info[FILENAME_MAX]="";
char sort_order[FILENAME_MAX]="";
int val,index=-1;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch(val) {
   case 'n': REMOVE_DUPLICATES=1; break;
   case 'd': REMOVE_DUPLICATES=0; break;
   case 'r': REVERSE=-1; break;
   case 'o': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty sort order file name\n");
             }
             strcpy(sort_order,optarg);
             break;      
   case 'l': if (optarg[0]=='\0') {
                fatal_error("You must specify a non empty information file name\n");
             }
             strcpy(line_info,optarg);
             break;
   case 't': mode=THAI; break;
   case 'h': usage_SortTxt(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",optopt); 
             else fatal_error("Missing argument for option --%s\n",lopts[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",optopt); 
             else fatal_error("Invalid option --%s\n",optarg);
             break;
   }
   index=-1;
}

if (optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}   
if (sort_order[0]!='\0') {
   read_char_order(sort_order);
}

char new_name[FILENAME_MAX];
init_char_arrays();
strcpy(new_name,argv[optind]);
strcat(new_name,".new");
f=u_fopen(argv[optind],U_READ);
if (f==NULL) {
   error("Cannot open file %s\n",argv[optind]);
   return 1;
}
f_out=u_fopen(new_name,U_WRITE);
if (f_out==NULL) {
   error("Cannot open temporary file %s\n",new_name);
   u_fclose(f);
   return 1;
}
switch (mode) {
   case DEFAULT: sort(); break;
   case THAI: sort_thai(); break;
}
if (line_info[0]!='\0') {
   FILE* F=u_fopen(line_info,U_WRITE);
   if (F==NULL) {
      error("Cannot write %s\n",line_info);
   }
   else {
      u_fprintf(F,"%d\n",resulting_line_number);
      u_fclose(F);
   }
}
/* We don't have to free the sort tree since it's done while dumping it into
 * the sorted file */
u_fclose(f);
u_fclose(f_out);
remove(argv[optind]);
rename(new_name,argv[optind]);
u_printf("Done.\n");
return 0;
}


/**
 * Returns 0 if a==b, -i if a<b and +i if a>b, according 1) to
 * the char order file and 2) to the fact that the sort is ascending
 * or descending.
 */
int char_cmp(unichar a,unichar b) {
if (class_numbers[a]) {
   if (class_numbers[b]) {
      if (class_numbers[a]==class_numbers[b]) {
         return (priority[a]-priority[b]);
      }
      else return REVERSE*(class_numbers[a]-class_numbers[b]);
   }
   else {
      return REVERSE;
   }
} else {
   if (class_numbers[b]) {
      return -1*REVERSE;
   }
   else {
      return REVERSE*(a-b);
   }
}
}


/**
 * Does a unicode char to char comparison, according to the char_cmp function order.
 */
int strcmp2(unichar* a,unichar* b) {
int i=0;
while (a[i] && a[i]==b[i]) i++;
return (char_cmp(a[i],b[i]));
}


/**
 * Reads all the line and puts them in a sorted tree, then saves them
 * exploring the tree.
 */
void sort() {
u_printf("Loading text...\n");
while (read_line());
u_printf("%d lines read\n",number_of_lines);
save();
}


/**
 * Reads all the Thai lines and put them in a sorted tree, then saves them
 * exploring the tree.
 */
void sort_thai() {
u_printf("Loading text...\n");
while (read_line_thai());
u_printf("%d lines read\n",number_of_lines);
save();
}


/**
 * Reads and processes a line of the text file.
 * Returns 0 if the end of file has been reached; 1 otherwise.
 */
int read_line() {
unichar line[LINE_LENGTH];
int c;
int ret=1;
int i=0;
while ((c=u_fgetc(f))!='\n' && c!=EOF && i<LINE_LENGTH) {
   line[i++]=(unichar)c;
}
line[i]='\0';
if (c==EOF) ret=0;
else number_of_lines++;
if (i==0) {
   /* We ignore the empty line */
   return ret;
}
if (i==LINE_LENGTH) {
   /* Too long lines are not taken into account */
   error("Line %d: line too long\n",number_of_lines);
   return ret;
}
get_node(line,0,root);
return ret;
}


/**
 * Saves the lines.
 */
void save() {
u_printf("Sorting and saving...\n");
explore_node(root);
}


/**
 * Initializes, allocates and returns a new sort tree node.
 */
struct sort_tree_node* new_sort_tree_node() {
struct sort_tree_node* n=(struct sort_tree_node*)malloc(sizeof(struct sort_tree_node));
if (n==NULL) {
   fatal_error("Not enough memory in new_sort_tree_node\n");
}
n->couples=NULL;
n->transitions=NULL;
return n;
}


/**
 * Frees all the memory associated to the node n.
 */
void free_sort_tree_node(struct sort_tree_node* n) {
if (n==NULL) {
  return;
}
free_couple(n->couples);
free(n);
}


/**
 * Initializes, allocates and returns a new sort tree transition.
 */
struct sort_tree_transition* new_sort_tree_transition() {
struct sort_tree_transition* t=(struct sort_tree_transition*)malloc(sizeof(struct sort_tree_transition));
if (t==NULL) {
   fatal_error("Not enough memory in new_sort_tree_transition\n");
}
t->node=NULL;
t->next=NULL;
return t;
}


/**
 * Frees all the memory associated to the given transition list.
 */
void free_sort_tree_transition(struct sort_tree_transition* t) {
struct sort_tree_transition* tmp;
while (t!=NULL) {
   free_sort_tree_node(t->node);
   tmp=t;
   t=t->next;
   free(tmp);
}
}


/**
 * Allocates, initializes and returns a new couple.
 */
struct couple* new_couple(unichar* s) {
struct couple* c=(struct couple*)malloc(sizeof(struct couple));
if (c==NULL) {
   fatal_error("Not enough memory in new_couple\n");
}
c->next=NULL;
c->n=1;
c->s=u_strdup(s);
return c;
}


/**
 * Frees all the memory associated to the given couple list.
 */
void free_couple(struct couple* c) {
struct couple* tmp;
while (c!=NULL) {
   if (c->s!=NULL) free(c->s);
   tmp=c;
   c=c->next;
   free(tmp);
}
}


/**
 * Looks for a transition tagged with the character class of c. Inserts it if
 * absent.
 */
struct sort_tree_transition* get_transition(unichar c,struct sort_tree_transition** t) {
struct sort_tree_transition* tmp;
if (class_numbers[c]!=0) {
   c=(unichar)canonical[c];
}
tmp=*t;
while (tmp!=NULL) {
   if (tmp->c==c) return tmp;
   tmp=tmp->next;
}
tmp=new_sort_tree_transition();
tmp->c=c;
tmp->next=(*t);
tmp->node=NULL;
(*t)=tmp;
return tmp;
}


/**
 * Inserts the given string in the given couple list.
 */
struct couple* insert_string(unichar* s,struct couple* couple) {
struct couple* tmp;
if (couple==NULL || REVERSE*strcmp2(s,couple->s)<0) {
   /* If we are at the end of the list, or if we have to insert */
   tmp=new_couple(s);
   tmp->next=couple;
   return tmp;
}
if (!strcmp2(s,couple->s)) {
   /* If the string is already in the list */
   if (!REMOVE_DUPLICATES) (couple->n)++;
   return couple;
}
/* If we have to explore the tail of the list */
couple->next=insert_string(s,couple->next);
return couple;
}


/**
 * This function looks for the path to 'line', creating it if necessary
 * the current node is n, and pos is the position in the 'line' string.
 * Note that the branches of the tree are not tagged with letters but with
 * letter classes, so that "lé" and "le" correspond to the same node.
 */
void get_node(unichar* line,int pos,struct sort_tree_node* n) {
if (line[pos]=='\0') {
   /* We are at the final node for 'line' */
   n->couples=insert_string(line,n->couples);
   return;
}
/* If we are not at the end of the string */
struct sort_tree_transition* trans=get_transition(line[pos],&(n->transitions));
if (trans->node==NULL) {
   trans->node=new_sort_tree_node();
}
get_node(line,pos+1,trans->node);
}


/**
 * Explores the node n, dumps the corresponding lines to the output file,
 * and then frees the node. 'pos' is the current position in the string 's'.
 */
void explore_node(struct sort_tree_node* n) {
int i,N;
struct sort_tree_transition* t;
struct couple* couple;
struct couple* tmp;
if (n==NULL) {
   fatal_error("Internal error in explore_node\n");
}
if (n->couples!=NULL) {
   /* If the node is a final one, we print the corresponding lines */
   couple=n->couples;
   while (couple!=NULL) {
      for (i=0;i<couple->n;i++) {
         u_fprintf(f_out,"%S\n",couple->s);
         resulting_line_number++;
      }
      tmp=couple;
      couple=couple->next;
      free(tmp->s);
      free(tmp);
   }
   n->couples=NULL;
}
/* We convert the transition list into a sorted array */
t=n->transitions;
N=0;
while (t!=NULL && N<0x10000) {
   transitions[N++]=t;
   t=t->next;
}
if (N==0x10000) {
   fatal_error("Internal error in explore_node: more than 0x10000 nodes\n");
}
if (N>1) quicksort(transitions,0,N-1);
/* After sorting, we copy the result into the transitions of n */
for (int i=0;i<N-1;i++) {
   transitions[i]->next=transitions[i+1];
}
if (N>0) {
   transitions[N-1]->next=NULL;
   n->transitions=transitions[0];
}
/* Finally, we explore the outgoing transitions */
t=n->transitions;
while (t!=NULL) {
   explore_node(t->node);
   t=t->next;
}
/* And we free the node */
free_sort_tree_node(n);
}


/**
 * Partitions the given array for quicksort.
 */
int partition(struct sort_tree_transition** t,int m, int n) {
unichar pivot;
struct sort_tree_transition* tmp;
int i=m-1;
int j=n+1;         /* Final index of the pivot */
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


/**
 * Quicksorts the given sort tree transition array.
 */
void quicksort(struct sort_tree_transition** t,int m,int n) {
int p;
if (m<n) {
   p=partition(t,m,n);
   quicksort(t,m,p);
   quicksort(t,p+1,n);
}
}


/**
 * Converts the string 'src' into a string with no diacritic sign and
 * in which initial vowels and following consons have been swapped.
 */
void convert_thai(unichar* src,unichar* dest) {
int i=0,j=0;
while (src[i]!='\0') {
   if (is_Thai_diacritic(src[i])) {
      i++;
   }
   else if (is_Thai_initial_vowel(src[i])) {
      dest[j]=src[i+1];
      dest[j+1]=src[i];
      i=i+2;
      j=j+2;
   } else {
      dest[j++]=src[i++];
   }
}
dest[j]='\0';
}


/**
 * This function look for the path to 'line', creating it if necessary
 * the current node is n, and pos is the position in the 'line' string.
 * Note that the branches of the tree are not tagged with letters but with
 * letter classes, so that "lé" and "le" correspond to the same node.
 * When the final node is reached, we insert the real Thai string with diacritic
 * signs in the correct position.
 */

void get_node_thai(unichar* line,int pos,struct sort_tree_node* n,unichar* real_string) {
if (line[pos]=='\0') {
   /* We are at the final node for 'line' */
   n->couples=insert_string_thai(real_string,n->couples);
   return;
}
/* If we are not at the end of the string 'line' */
struct sort_tree_transition* trans=get_transition(line[pos],&(n->transitions));
if (trans->node==NULL) {
   trans->node=new_sort_tree_node();
}
get_node_thai(line,pos+1,trans->node,real_string);
}


/**
 * Reads and processes a line of the Thai text file.
 */
int read_line_thai() {
unichar line[LINE_LENGTH];
unichar thai_line[LINE_LENGTH];
int c;
int ret=1;
int i=0;
while ((c=u_fgetc(f))!='\n' && c!=EOF && i<LINE_LENGTH) {
   line[i++]=(unichar)c;
}
line[i]='\0';
if (c==EOF) ret=0;
else number_of_lines++;
if (i==0) {
   /* We ignore the empty line */
   return ret;
}
if (i==LINE_LENGTH) {
   error("Line %d: line too long\n",number_of_lines);
   return ret;
}
convert_thai(line,thai_line);
get_node_thai(thai_line,0,root,line);
return ret;
}


/**
 * Inserts the given string in the given couple list.
 */
struct couple* insert_string_thai(unichar* line,struct couple* couple) {
struct couple* tmp;
if (couple==NULL || REVERSE*u_strcmp(line,couple->s)<0) {
   /* If we are at the end of the list, or if we have to insert */
   tmp=new_couple(line);
   tmp->next=couple;
   return tmp;
}
if (!u_strcmp(line,couple->s)) {
   /* If 'line' is already in the list */
   if (!REMOVE_DUPLICATES) (couple->n)++;
   return couple;
}
/* If we have to explore the tail of the list */
couple->next=insert_string_thai(line,couple->next);
return couple;
}

