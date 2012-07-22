/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define RED "#FF0000"
#define GREEN "#008000"
#define BLUE "#0000FF"
#define VIOLET "#800080"


/**
 * This function takes two concordance index 'in1' and 'in2', and builds
 * the associated concordances 'out1' and 'out2'.
 */
void create_text_concordances(const VersatileEncodingConfig* vec,const char* in1,const char* in2,const char* out1,const char* out2) {
pseudo_main_Concord(vec,in1,NULL,0,20,80,NULL,"--diff",NULL,NULL,0,0,0);
char f[FILENAME_MAX];
get_path(in1,f);
strcat(f,"concord.txt");
af_remove(out1);
af_rename(f,out1);
pseudo_main_Concord(vec,in2,NULL,0,20,80,NULL,"--diff",NULL,NULL,0,0,0);
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
u_fprintf(f,"   a.violet {color:violet; text-decoration:underline;}\n");
u_fprintf(f,"   a.red {color:red; text-decoration:underline;}\n");
u_fprintf(f,"   a.green {color:green; text-decoration:underline;}\n");
u_fprintf(f,"   </style>\n");
u_fprintf(f,"</head>\n");
u_fprintf(f,"<body>\n");
u_fprintf(f,"<h4>\n");
u_fprintf(f,"<font color=\"blue\">Blue:</font> identical sequences<br>\n");
u_fprintf(f,"<font color=\"#800080\">Violet:</font> identical sequences with different outputs<br>\n");
u_fprintf(f,"<font color=\"red\">Red:</font> similar but different sequences<br>\n");
u_fprintf(f,"<font color=\"green\">Green:</font> sequences that occur in only one of the two concordances<br>\n");
u_fprintf(f,"<table border=\"1\" cellpadding=\"5\" style=\"font-family: %s; font-size: %d\">\n",font,size);
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
int diff(const VersatileEncodingConfig* vec,const char* in1,const char* in2,const char* out,
		const char* font,int size,int diff_only) {
char concor1[FILENAME_MAX];
char concor2[FILENAME_MAX];
get_path(in1,concor1);
strcat(concor1,"concord-1.txt");
get_path(in2,concor2);
strcat(concor2,"concord-2.txt");
/* First, we build the two concordances */
create_text_concordances(vec,in1,in2,concor1,concor2);
/* Then, we load the two index */
U_FILE* f1=u_fopen(vec,in1,U_READ);
if (f1==NULL) return 0;
struct match_list* l1=load_match_list(f1,NULL,NULL);
u_fclose(f1);
U_FILE* f2=u_fopen(vec,in2,U_READ);
if (f2==NULL) {
   return 0;
}
struct match_list* l2=load_match_list(f2,NULL,NULL);
u_fclose(f2);
/* We open the output file in UTF8, because the GUI expects this file
 * to be that encoded */
U_FILE* output=u_fopen(UTF8,out,U_WRITE);
if (output==NULL) {
   fatal_error("Cannot open output file %s\n",out);
   return 0;
}
/* We open the two concordance files */
f1=u_fopen(vec,concor1,U_READ);
f2=u_fopen(vec,concor2,U_READ);
/* And then we fill the output file with the differences
 * between the two concordances */
print_diff_HTML_header(output,font,size);
compute_concordance_differences(l1,l2,f1,f2,output,diff_only);
print_diff_HTML_end(output);
free_match_list(l1);
free_match_list(l2);
u_fclose(f1);
u_fclose(f2);
u_fclose(output);
/* We remove the tmp files */
//af_remove(concor1);
//af_remove(concor2);
return 1;
}


/**
 * Looks in the given list for a match identical to m. In case
 * of success, it returns 1 and the list is modified so that
 * its first element is the identical one. Returns 0 otherwise.
 *
 * NOTE: the return value is different from look_for_same_outputs!
 */
static int look_for_same_outputs_(struct match_list* m,
							struct match_list* list) {
struct match_list* head=list;
while (list!=NULL && compare_matches(&(m->m),&(list->m))==A_EQUALS_B) {
	if (!u_strcmp(m->output,list->output)) {
		/* We have an identical match */
		unichar* tmp=head->output;
		head->output=list->output;
		list->output=tmp;
		return 1;
	}
	list=list->next;
}
return 0;
}


/**
 * If a we have at the same position list1=xA and list2=xB,xA
 * we want to avoid the following diff analysis:
 *
 * 1) xA != xB
 * 2)    != xA
 *
 * Instead, we want to reorder the matches so that we have:
 *
 * 1) xA == xA
 * 2)    != xB
 *
 * This functions looks for such reordering. If possible,
 * the concerned outputs are swapped so that the lists' first
 * items have identical outputs and the function returns 0.
 * If no identical outputs are found, it returns 1 and leaves the
 * lists unmodified.
 */
int look_for_same_outputs(struct match_list* list1,
							struct match_list* list2) {
if (look_for_same_outputs_(list1,list2)) return 0;
if (look_for_same_outputs_(list2,list1)) return 0;
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
                                     U_FILE* output,
                                     int diff_only) {
/* We look both match index entirely */
while (!(list1==NULL && list2==NULL)) {
   if (list1==NULL) {
      /* If the first list is empty, then the current match in the second list
       * must be green */
      print_diff_matches(output,NULL,f2,GREEN,NULL,list2->output);
      list2=list2->next;
      continue;
   }
   if (list2==NULL) {
      /* If the second list is empty, then the current match in the first list
       * must be green */
      print_diff_matches(output,f1,NULL,GREEN,list1->output,NULL);
      list1=list1->next;
      continue;
   }
   switch (compare_matches(&(list1->m),&(list2->m))) {
      case A_BEFORE_B: {
         /* list1 has no common part with list2:
          * abcd,efgh */
         print_diff_matches(output,f1,NULL,GREEN,list1->output,list2->output);
         list1=list1->next;
         break;
      }
      case A_INCLUDES_B: {
         /* list2 is included in list1:
          * abcdef,cdef */
         print_diff_matches(output,f1,f2,RED,list1->output,list2->output);
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
         print_diff_matches(output,f1,NULL,GREEN,list1->output,list2->output);
         list1=list1->next;
         break;
      }
      case A_AFTER_B: {
         /* list2 has no common part with list1:
          * abcd,efgh */
         print_diff_matches(output,NULL,f2,GREEN,list1->output,list2->output);
         list2=list2->next;
         break;
      }
      case B_INCLUDES_A: {
         /* list1 is included in list2:
          * abcd,abcdef */
         print_diff_matches(output,f1,f2,RED,list1->output,list2->output);
         list1=list1->next;
         list2=list2->next;
         break;
      }
      case A_AFTER_B_OVERLAP: {
         /* list1 overlaps list2
          * abcdef,cdefgh
          * We consider that they are two distinct lines, and we
          * print the first */
         print_diff_matches(output,NULL,f2,GREEN,list1->output,list2->output);
         list2=list2->next;         
         break;
      }
      case A_EQUALS_B: {
         /* list1 == list2:
          * abcd,abcd */
    	 int different_outputs=u_strcmp(list1->output,list2->output);
    	 if (different_outputs) {
    		 /* If there are matches with ambiguous outputs, we may find
    		  * an exact match pair forward in the lists
    		  */
    		 different_outputs=look_for_same_outputs(list1,list2);
    	 }
    	 if (!diff_only || different_outputs) {
            print_diff_matches(output,f1,f2,different_outputs?VIOLET:BLUE,list1->output,list2->output);
    	 } else {
    		 /* We have to skip the unused lines */
    		 u_fskip_line(f1);
    		 u_fskip_line(f2);
    	 }
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
 *
 * IMPORTANT: in order to fix a bug, we may have to reorder some matches. However, we can't
 *            easily reorder lines in concordance files. So, we indicate the length of the
 *            expected match instead of relying on the length of the match that was actually read.
 *            This is why there is the (++total>=60) hack.
 */
void read_concordance_line(U_FILE* f,unichar* left,unichar* middle,unichar* right,unichar* indices,int
		expected_match_length) {
int i,c;
i=0;
while ((c=u_fgetc(f))!='\t') {
   left[i++]=(unichar)c;
}
left[i]='\0';
int total=i+expected_match_length;
i=0;
while ((c=u_fgetc(f))!='\t') {
   middle[i++]=(unichar)c;
}
middle[i]='\0';
i=0;
while ((c=u_fgetc(f))!='\t') {
	if (++total>=60) {
		c='\0';
	}
   right[i++]=(unichar)c;
}
right[i]='\0';
i=0;
while ((c=u_fgetc(f))!='\n') {
   indices[i++]=(unichar)c;
}
indices[i]='\0';
}


/**
 * This function inserts the nth first chars of src at the beginning of
 * dst whose content is shifted to the right.
 */
static void adjust(unichar* src,unichar* dst,int n) {
int l=u_strlen(dst);
for (int i=l+1;i>=0;i--) {
	dst[i+n]=dst[i];
}
for (int i=0;i<n;i++) {
	dst[i]=src[i];
}
}

/**
 * This function loads concordance lines from 'f1' and/or 'f2' and prints them to
 * 'output' in the given color.
 */
void print_diff_matches(U_FILE* output,U_FILE* f1,U_FILE* f2,const char* color,
		unichar* match1,unichar* match2) {
unichar left1[MAX_CONTEXT_IN_UNITS];
unichar middle1[MAX_CONTEXT_IN_UNITS];
unichar right1[MAX_CONTEXT_IN_UNITS];
unichar indices1[MAX_CONTEXT_IN_UNITS];
unichar left2[MAX_CONTEXT_IN_UNITS];
unichar middle2[MAX_CONTEXT_IN_UNITS];
unichar right2[MAX_CONTEXT_IN_UNITS];
unichar indices2[MAX_CONTEXT_IN_UNITS];
if (f1!=NULL) {
   read_concordance_line(f1,left1,middle1,right1,indices1,u_strlen(match1));
}
if (f2!=NULL) {
   read_concordance_line(f2,left2,middle2,right2,indices2,u_strlen(match2));
}
if (match1!=NULL) u_strcpy(middle1,match1);
if (match2!=NULL) u_strcpy(middle2,match2);
if (!strcmp(color,RED)) {
	/* If we have one match included in the another, we want to align
	 * them. We do that by adjusting their left contexts */
	int pos1,pos2;
	u_sscanf(indices1,"%d",&pos1);
	u_sscanf(indices2,"%d",&pos2);
	if (pos1<pos2) {
		adjust(left1,left2,pos2-pos1);
	} else if (pos1>pos2) {
		adjust(left2,left1,pos1-pos2);
	} /*  Nothing to adjust if pos1==pos2 */
}
/* We print the line from the first file, if needed */
u_fprintf(output,"<tr><td nowrap bgcolor=\"#FFE4C4\"><font color=\"%s\">",color);
if (f1!=NULL) {
   u_fprintf(output,"%HS<a href=\"%S\" style=\"color:%s\">%HS</a>%HS",left1,indices1,color,middle1,right1);
} else {
	u_fprintf(output,"&nbsp;");
}
u_fprintf(output,"</font></td></tr>\n");
u_fprintf(output,"<tr><td nowrap bgcolor=\"#90EE90\"><font color=\"%s\">",color);
/* We print the line from the second file, if needed */
if (f2!=NULL) {
   u_fprintf(output,"%HS<a href=\"%S\" style=\"color:%s\">%HS</a>%HS",left2,indices2,color,middle2,right2);
} else {
	u_fprintf(output,"&nbsp;");
}
u_fprintf(output,"</font></td></tr>\n");
}

} // namespace unitex
