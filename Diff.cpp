 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Diff.h"
#include "Unicode.h"
#include "File.h"
#include "ConcordMain.h"
#include "Concordance.h"


/**
 * This function takes two concordance index 'in1' and 'in2', and builds
 * the associated concordacnces 'out1' and 'out2'.
 */
void create_text_concordances(char* in1,char* in2,char* out1,char* out2) {
char** argv;
argv=(char**)malloc(9*sizeof(char*));
argv[0]=strdup(" ");
argv[1]=strdup(in1);
argv[2]=strdup("Courier New");
argv[3]=strdup("10");
argv[4]=strdup("20");
argv[5]=strdup("40");
argv[6]=strdup("TO");
argv[7]=strdup("text");
argv[8]=strdup("NULL");
main_Concord(9,argv);
char f[FILENAME_MAX];
get_path(in1,f);
strcat(f,"concord.txt");
rename(f,out1);
free(argv[1]);
argv[1]=strdup(in2);
main_Concord(9,argv);
rename(f,out2);
for (int i=0;i<9;i++) {
   free(argv[i]);
}
free(argv);
}


/**
 * Printing the header of the HTML file.
 */
void print_diff_HTML_header(FILE* f,char* font,char* size) {
u_fprintf(UTF8,f,"<html>\n");
u_fprintf(UTF8,f,"<head>\n");
u_fprintf(UTF8,f,"   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
u_fprintf(UTF8,f,"   <style type=\"text/css\">\n");
u_fprintf(UTF8,f,"   a.blue {color:blue; text-decoration:underline;}\n");
u_fprintf(UTF8,f,"   a.red {color:red; text-decoration:underline;}\n");
u_fprintf(UTF8,f,"   a.green {color:green; text-decoration:underline;}\n");
u_fprintf(UTF8,f,"   </style>\n");
u_fprintf(UTF8,f,"</head>\n");
u_fprintf(UTF8,f,"<body>\n");
u_fprintf(UTF8,f,"<h4>\n");
u_fprintf(UTF8,f,"<font color=\"blue\">Blue:</font> identical sequences<br>\n");
u_fprintf(UTF8,f,"<font color=\"red\">Red:</font> similar but different sequences<br>\n");
u_fprintf(UTF8,f,"<font color=\"green\">Green:</font> sequences that occur in only one of the two concordances<br>\n");
u_fprintf(UTF8,f,"<table border=\"1\" cellpadding=\"0\" style=\"font-family: %s; font-size: %s\">\n",font,size);
}


/**
 * Printing the closing tags of the HTML file.
 */
void print_diff_HTML_end(FILE* f) {
u_fprintf(UTF8,f,"</table>\n</body>\n</html>\n");
}


/**
 * This function takes two concordance index (in1 and in2) and
 * produces a HTML file (out) that shows the differences between
 * those two concordances.
 */
int diff(char* in1,char* in2,char* out,char* font,char* size) {
char concor1[FILENAME_MAX];
char concor2[FILENAME_MAX];
get_path(in1,concor1);
strcat(concor1,"concord-1.txt");
get_path(in2,concor2);
strcat(concor2,"concord-2.txt");
/* First, we build the two concordances */
create_text_concordances(in1,in2,concor1,concor2);
/* Then, we load the two index */
FILE* f1=u_fopen(in1,U_READ);
if (f1==NULL) return 0;
struct match_list* l1=load_match_list(f1,NULL);
u_fclose(f1);
FILE* f2=u_fopen(in2,U_READ);
if (f2==NULL) {
   return 0;
}
struct match_list* l2=load_match_list(f2,NULL);
u_fclose(f2);
/* We open the output file */
FILE* output=fopen(out,"w");
if (output==NULL) {
   return 0;
}
/* We open the two concordance files */
f1=u_fopen(concor1,U_READ);
f2=u_fopen(concor2,U_READ);
/* And then we fill the output file with the differences 
 * between the two concordances */
print_diff_HTML_header(output,font,size);
compute_concordance_differences(l1,l2,f1,f2,output);
print_diff_HTML_end(output);
u_fclose(f1);
u_fclose(f2);
fclose(output);
/* We remove the tmp files */
remove(concor1);
remove(concor2);
return 1;
}


/**
 * This function explores the two given match index and prints
 * the differences into 'output'. 'f1' and 'f2' are the concordance
 * files that are used to get the text sequences to put in 'output'.
 * Matches that appear in only one file are printed in green.
 * Matches that are identical in both files are printed in blue.
 * Matches that are different but with a non empty intersection like 
 * "the blue car" and "blue car" are printed in red.
 */
void compute_concordance_differences(struct match_list* list1,
                                     struct match_list* list2,
                                     FILE* f1,
                                     FILE* f2,
                                     FILE* output) {
/* We look both match index entirely */
while (!(list1==NULL && list2==NULL)) {
   if (list1==NULL) {
      /* If the first list is empty, then the current match in the second list
       * must be green */
      print_diff_matches(output,NULL,f2,"green");
      list2=list2->next;
   }
   else if (list2==NULL) {
      /* If the second list is empty, then the current match in the first list
       * must be green */
      print_diff_matches(output,f1,NULL,"green");
      list1=list1->next;
   }
   else if (list1->start < list2->start) {
           if (list1->end < list2->start) {
              /* list1 has no common part with list2:
               * abcd,efgh */
              print_diff_matches(output,f1,NULL,"green");
              list1=list1->next;
           }
           else {
              /* list1 and list2 have something in common */
              if (list2->end <= list1->end) {
                 /* list2 is included in list1:
                  * abcdef,cdef */
                 print_diff_matches(output,f1,f2,"red");
                 list1=list1->next;
                 list2=list2->next;
              }
              else {
                 /* list2 overlaps list1:
                  * abcdef,cdefgh
                  * 
                  * We consider that they are two distinct lines, and we
                  * print the first */
                 print_diff_matches(output,f1,NULL,"green");
                 list1=list1->next;
              }
           }
   }
   else if (list2->start < list1->start) {
           if (list2->end < list1->start) {
              /* list2 has no common part with list1:
               * abcd,efgh */
              print_diff_matches(output,NULL,f2,"green");
              list2=list2->next;
           }
           else {
              /* list1 and list2 have something in common */
              if (list1->end <= list2->end) {
                 /* list1 is included in list2:
                  * abcd,abcdef */
                 print_diff_matches(output,f1,f2,"red");
                 list1=list1->next;
                 list2=list2->next;
              }
              else {
                 /* list1 overlaps list2
                  * abcdef,cdefgh
                  * We consider that they are two distinct lines, and we
                  * print the first */
                 print_diff_matches(output,NULL,f2,"green");
                 list2=list2->next;
              }
           }
   }
   else {
      if (list1->end == list2->end) {
         /* list1 == list2:
          * abcd,abcd */
         print_diff_matches(output,f1,f2,"blue");
      }
      else {
         /* abcd,abcdef or abcedf,abcd */
         print_diff_matches(output,f1,f2,"red");
      }
      list1=list1->next;
      list2=list2->next;
   }
}
}


/**
 * This function reads one concordance line from 'f', and splits its
 * components into 'left', 'middle' and 'right'.
 */
void read_concordance_line(FILE* f,unichar* left,unichar* middle,unichar* right) {
int i,c;
i=0;
while ((c=u_fgetc(f))!='\t') {
   left[i++]=(unichar)c;
}
left[i]='\0';
i=0;
while ((c=u_fgetc(f))!='\t') {
   middle[i++]=(unichar)c;
}
middle[i]='\0';
i=0;
while ((c=u_fgetc(f))!='\n') {
   right[i++]=(unichar)c;
}
right[i]='\0';
}


/**
 * This function loads concordance lines from 'f1' and/or 'f2' and prints them to 
 * 'output' in the given color.
 */
void print_diff_matches(FILE* output,FILE* f1,FILE* f2,char* color) {
unichar left[MAX_CONTEXT_IN_UNITS];
unichar middle[MAX_CONTEXT_IN_UNITS];
unichar right[MAX_CONTEXT_IN_UNITS];
/* We print the line from the first file, if needed */
u_fprintf(UTF8,output,"<tr><td width=\"450\"><font color=\"%s\">",color);
if (f1!=NULL) {
   read_concordance_line(f1,left,middle,right);
   u_fprintf(UTF8,output,"%HS<u>%HS</u>%HS",left,middle,right);
   /*u_fprints_html(left,output);
   fprintf(output,"<u>");
   u_fprints_html(middle,output);
   fprintf(output,"</u>");
   u_fprints_html(right,output);*/
}
u_fprintf(UTF8,output,"</font></td>");
u_fprintf(UTF8,output,"<td width=\"450\"><font color=\"%s\">",color);
/* We print the line from the second file, if needed */
if (f2!=NULL) {
   read_concordance_line(f2,left,middle,right);
   u_fprintf(UTF8,output,"%HS<u>%HS</u>%HS",left,middle,right);
   /*u_fprints_html(left,output);
   fprintf(output,"<u>");
   u_fprints_html(middle,output);
   fprintf(output,"</u>");
   u_fprints_html(right,output);*/
}
u_fprintf(UTF8,output,"</font></td></tr>\n");
}

