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

#include "Diff.h"
#include "unicode.h"
#include "FileName.h"

#include "ConcordMain.h"

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
char f[2000];
get_filename_path(in1,f);
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

void print_diff_HTML_header(FILE* f,char* font,char* size) {
fprintf(f,"<html>\n");
fprintf(f,"<head>\n");
fprintf(f,"   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
fprintf(f,"   <style type=\"text/css\">\n");
fprintf(f,"   a.blue {color:blue; text-decoration:underline;}\n");
fprintf(f,"   a.red {color:red; text-decoration:underline;}\n");
fprintf(f,"   a.green {color:green; text-decoration:underline;}\n");
fprintf(f,"   </style>\n");
fprintf(f,"</head>\n");
fprintf(f,"<body>\n");
fprintf(f,"<h4>\n");
fprintf(f,"<font color=\"blue\">Blue:</font> identical sequences<br>\n");
fprintf(f,"<font color=\"red\">Red:</font> similar but different sequences<br>\n");
fprintf(f,"<font color=\"green\">Green:</font> sequences that occur in only one of the two concordances<br>\n");
fprintf(f,"<table border=\"1\" cellpadding=\"0\" style=\"font-family: %s; font-size: %s\">\n",font,size);
}

void print_diff_HTML_end(FILE* f) {
fprintf(f,"</table>\n</body>\n</html>\n");
}


int diff(char* in1,char* in2,char* out,char* font,char* size) {
char concor1[2000];
char concor2[2000];
get_filename_path(in1,concor1);
strcat(concor1,"concord-1.txt");
get_filename_path(in2,concor2);
strcat(concor2,"concord-2.txt");
create_text_concordances(in1,in2,concor1,concor2);

FILE* f1=u_fopen(in1,U_READ);
if (f1==NULL) return 0;
FILE* f2=u_fopen(in2,U_READ);
if (f2==NULL) {
   u_fclose(f1);
   return 0;
}
FILE* output=fopen(out,"w");
if (output==NULL) {
   u_fclose(f1);
   u_fclose(f2);
   return 0;
}
int MODE1,MODE2;
struct liste_matches* l1=load_match_list(f1,&MODE1);
u_fclose(f1);
struct liste_matches* l2=load_match_list(f2,&MODE2);
u_fclose(f2);

f1=u_fopen(concor1,U_READ);
f2=u_fopen(concor2,U_READ);

print_diff_HTML_header(output,font,size);
compute_concordance_differences(l1,l2,f1,f2,output);
print_diff_HTML_end(output);

u_fclose(f1);
u_fclose(f2);
remove(concor1);
remove(concor2);
fclose(output);
return 1;
}





void compute_concordance_differences(struct liste_matches* list1,
                                     struct liste_matches* list2,
                                     FILE* f1,
                                     FILE* f2,
                                     FILE* output) {
while (!(list1==NULL && list2==NULL)) {
   if (list1==NULL) {
      print_diff_matches(output,NULL,f2,"green",list1,list2);
      list2=list2->suivant;
   }
   else if (list2==NULL) {
      print_diff_matches(output,f1,NULL,"green",list1,list2);
      list1=list1->suivant;
   }
   else if (list1->debut < list2->debut) {
           if (list1->fin < list2->debut) {
              // list1 has no common part with list2
              // abcd,efgh
              print_diff_matches(output,f1,NULL,"green",list1,list2);
              list1=list1->suivant;
           }
           else {
              // list1 and list2 have something in common
              if (list2->fin <= list1->fin) {
                 // list2 is included in list1
                 // abcdef,cdef
                 print_diff_matches(output,f1,f2,"red",list1,list2);
                 list1=list1->suivant;
                 list2=list2->suivant;
              }
              else {
                 // list2 overlaps list1
                 // abcdef,cdefgh
                 // we consider that they are two distinct lines, and we
                 // print the first
                 print_diff_matches(output,f1,NULL,"green",list1,list2);
                 list1=list1->suivant;
              }
           }
   }
   else if (list2->debut < list1->debut) {
           if (list2->fin < list1->debut) {
              // list2 has no common part with list1
              // abcd,efgh
              print_diff_matches(output,NULL,f2,"green",list1,list2);
              list2=list2->suivant;
           }
           else {
              // list1 and list2 have something in common
              if (list1->fin <= list2->fin) {
                 // list1 is included in list2
                 // abcd,abcdef
                 print_diff_matches(output,f1,f2,"red",list1,list2);
                 list1=list1->suivant;
                 list2=list2->suivant;
              }
              else {
                 // list1 overlaps list2
                 // abcdef,cdefgh
                 // we consider that they are two distinct lines, and we
                 // print the first
                 print_diff_matches(output,NULL,f2,"green",list1,list2);
                 list2=list2->suivant;
              }
           }
   }
   else {
      // list1->debut == list2->debut
      if (list1->fin == list2->fin) {
         // list1 == list2
         // abcd,abcd
         print_diff_matches(output,f1,f2,"blue",list1,list2);
      }
      else {
         // abcd,abcdef or abcedf,abcd
         print_diff_matches(output,f1,f2,"red",list1,list2);
      }
      list1=list1->suivant;
      list2=list2->suivant;
   }
}
}



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


void print_diff_matches(FILE* output,FILE* f1,FILE* f2,char* color,
                        struct liste_matches* l1,
                        struct liste_matches* l2) {
unichar left[2000];
unichar middle[2000];
unichar right[2000];

fprintf(output,"<tr><td width=\"450\"><font color=\"%s\">",color);
if (f1!=NULL) {
   read_concordance_line(f1,left,middle,right);
   u_fprints_html(left,output);
   //fprintf(output,"<a href=\"%d %d -1\" class=\"%s\">",l1->debut,l1->fin,color);
   fprintf(output,"<u>");
   u_fprints_html(middle,output);
   fprintf(output,"</u>");
   //fprintf(output,"</a>");
   u_fprints_html(right,output);
}
fprintf(output,"</font></td>");
fprintf(output,"<td width=\"450\"><font color=\"%s\">",color);
if (f2!=NULL) {
   read_concordance_line(f2,left,middle,right);
   u_fprints_html(left,output);
   //fprintf(output,"<a href=\"%d %d -1\" class=\"%s\">",l2->debut,l2->fin,color);
   fprintf(output,"<u>");
   u_fprints_html(middle,output);
   fprintf(output,"</u>");
   //fprintf(output,"</a>");
   u_fprints_html(right,output);
}
fprintf(output,"</font></td></tr>\n");
}
