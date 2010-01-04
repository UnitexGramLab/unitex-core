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

#include "Diff.h"
#include "Unicode.h"
#include "File.h"
#include "Concord.h"
#include "Concordance.h"


/**
 * This function takes two concordance index 'in1' and 'in2', and builds
 * the associated concordacnces 'out1' and 'out2'.
 */
void create_text_concordances(Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input,const char* in1,const char* in2,const char* out1,const char* out2) {
pseudo_main_Concord(encoding_output,bom_output,mask_encoding_compatibility_input,in1,NULL,0,20,40,NULL,"--text",NULL,NULL,0);
char f[FILENAME_MAX];
get_path(in1,f);
strcat(f,"concord.txt");
af_remove(out1);
af_rename(f,out1);

pseudo_main_Concord(encoding_output,bom_output,mask_encoding_compatibility_input,in2,NULL,0,20,40,NULL,"--text",NULL,NULL,0);
af_remove(out2);
af_rename(f,out2);
}


/**
 * Printing the header of the HTML file.
 */
void print_diff_HTML_header(U_FILE* f,const char* font,int size) {
u_fprintf(f,"<html>\n");
u_fprintf(f,"<head>\n");
u_fprintf(f,"   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
u_fprintf(f,"   <style type=\"text/css\">\n");
u_fprintf(f,"   a.blue {color:blue; text-decoration:underline;}\n");
u_fprintf(f,"   a.red {color:red; text-decoration:underline;}\n");
u_fprintf(f,"   a.green {color:green; text-decoration:underline;}\n");
u_fprintf(f,"   </style>\n");
u_fprintf(f,"</head>\n");
u_fprintf(f,"<body>\n");
u_fprintf(f,"<h4>\n");
u_fprintf(f,"<font color=\"blue\">Blue:</font> identical sequences<br>\n");
u_fprintf(f,"<font color=\"red\">Red:</font> similar but different sequences<br>\n");
u_fprintf(f,"<font color=\"green\">Green:</font> sequences that occur in only one of the two concordances<br>\n");
u_fprintf(f,"<table border=\"1\" cellpadding=\"0\" style=\"font-family: %s; font-size: %d\">\n",font,size);
}


/**
 * Printing the closing tags of the HTML file.
 */
void print_diff_HTML_end(U_FILE* f) {
u_fprintf(f,"</table>\n</body>\n</html>\n");
}


/**
 * This function takes two concordance index (in1 and in2) and
 * produces a HTML file (out) that shows the differences between
 * those two concordances.
 */
int diff(Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input,const char* in1,const char* in2,const char* out,const char* font,int size) {
char concor1[FILENAME_MAX];
char concor2[FILENAME_MAX];
get_path(in1,concor1);
strcat(concor1,"concord-1.txt");
get_path(in2,concor2);
strcat(concor2,"concord-2.txt");
/* First, we build the two concordances */
create_text_concordances(encoding_output,bom_output,mask_encoding_compatibility_input,in1,in2,concor1,concor2);
/* Then, we load the two index */
U_FILE* f1=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,in1,U_READ);
if (f1==NULL) return 0;
struct match_list* l1=load_match_list(f1,NULL);
u_fclose(f1);
U_FILE* f2=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,in2,U_READ);
if (f2==NULL) {
   return 0;
}
struct match_list* l2=load_match_list(f2,NULL);
u_fclose(f2);
/* We open the output file */
U_FILE* output=u_fopen(UTF8,out,U_WRITE);
if (output==NULL) {
   fatal_error("Cannot open output file %s\n",out);
   return 0;
}
/* We open the two concordance files */
f1=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,concor1,U_READ);
f2=u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,concor2,U_READ);
/* And then we fill the output file with the differences
 * between the two concordances */
print_diff_HTML_header(output,font,size);
compute_concordance_differences(l1,l2,f1,f2,output);
print_diff_HTML_end(output);
free_match_list(l1);
free_match_list(l2);
u_fclose(f1);
u_fclose(f2);
u_fclose(output);
/* We remove the tmp files */
af_remove(concor1);
af_remove(concor2);
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
                                     U_FILE* f1,
                                     U_FILE* f2,
                                     U_FILE* output) {
/* We look both match index entirely */
while (!(list1==NULL && list2==NULL)) {
   if (list1==NULL) {
      /* If the first list is empty, then the current match in the second list
       * must be green */
      print_diff_matches(output,NULL,f2,"green");
      list2=list2->next;
      continue;
   }
   if (list2==NULL) {
      /* If the second list is empty, then the current match in the first list
       * must be green */
      print_diff_matches(output,f1,NULL,"green");
      list1=list1->next;
      continue;
   }
   switch (compare_matches(&(list1->m),&(list2->m))) {
      case A_BEFORE_B: {
         /* list1 has no common part with list2:
          * abcd,efgh */
         print_diff_matches(output,f1,NULL,"green");
         list1=list1->next;
         break;
      }
      case A_INCLUDES_B: {
         /* list2 is included in list1:
          * abcdef,cdef */
         print_diff_matches(output,f1,f2,"red");
         list1=list1->next;
         list2=list2->next;
         break;
      }
      case A_BEFORE_B_OVERLAP: {
         /* list2 overlaps list1:
          * abcdef,cdefgh
          *
          * We consider that they are two distinct lines, and we
          * print the first */
         print_diff_matches(output,f1,NULL,"green");
         list1=list1->next;
         break;
      }
      case A_AFTER_B: {
         /* list2 has no common part with list1:
          * abcd,efgh */
         print_diff_matches(output,NULL,f2,"green");
         list2=list2->next;
         break;
      }
      case B_INCLUDES_A: {
         /* list1 is included in list2:
          * abcd,abcdef */
         print_diff_matches(output,f1,f2,"red");
         list1=list1->next;
         list2=list2->next;
         break;
      }
      case A_AFTER_B_OVERLAP: {
         /* list1 overlaps list2
          * abcdef,cdefgh
          * We consider that they are two distinct lines, and we
          * print the first */
         print_diff_matches(output,NULL,f2,"green");
         list2=list2->next;         
         break;
      }
      case A_EQUALS_B: {
         /* list1 == list2:
          * abcd,abcd */
         print_diff_matches(output,f1,f2,"blue");
         list1=list1->next;
         list2=list2->next;
         break;
      }
   }
}
}


/**
 * This function reads one concordance line from 'f', and splits its
 * components into 'left', 'middle' and 'right'.
 */
void read_concordance_line(U_FILE* f,unichar* left,unichar* middle,unichar* right) {
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
void print_diff_matches(U_FILE* output,U_FILE* f1,U_FILE* f2,const char* color) {
unichar left[MAX_CONTEXT_IN_UNITS];
unichar middle[MAX_CONTEXT_IN_UNITS];
unichar right[MAX_CONTEXT_IN_UNITS];
/* We print the line from the first file, if needed */
u_fprintf(output,"<tr><td width=\"450\"><font color=\"%s\">",color);
if (f1!=NULL) {
   read_concordance_line(f1,left,middle,right);
   u_fprintf(output,"%HS<u>%HS</u>%HS",left,middle,right);
   /*u_fprints_html(left,output);
   fprintf(output,"<u>");
   u_fprints_html(middle,output);
   fprintf(output,"</u>");
   u_fprints_html(right,output);*/
}
u_fprintf(output,"</font></td>");
u_fprintf(output,"<td width=\"450\"><font color=\"%s\">",color);
/* We print the line from the second file, if needed */
if (f2!=NULL) {
   read_concordance_line(f2,left,middle,right);
   u_fprintf(output,"%HS<u>%HS</u>%HS",left,middle,right);
   /*u_fprints_html(left,output);
   fprintf(output,"<u>");
   u_fprints_html(middle,output);
   fprintf(output,"</u>");
   u_fprints_html(right,output);*/
}
u_fprintf(output,"</font></td></tr>\n");
}
