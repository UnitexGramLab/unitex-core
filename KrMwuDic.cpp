 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Error.h"
#include "KrMwuDic.h"
#include "StringParsing.h"
#include "DELA.h"


#define MAX_LINE_SIZE 4096
#define MAX_PARTS 32

int tokenize_kr_mwu_dic_line(unichar** part,unichar* line);
void produce_mwu_entries(int n_parts,struct dela_entry** entries);



/**
 * Builds the .grf dictionary corresponding to the given Korean compound DELAS.
 */
void create_mwu_dictionary(U_FILE* delas) {
unichar line[MAX_LINE_SIZE];
int size_line;
int line_number=-1;
unichar* part[MAX_PARTS];
struct dela_entry* entries[MAX_PARTS];
int n_parts;
while ((size_line=u_fgets(line,MAX_LINE_SIZE,delas))!=EOF) {
   /* We place the line counter here, so we can use 'continue' */
   line_number++;
   if (size_line==MAX_LINE_SIZE-1) {
      error("Line %d ignored because it is too long (>%d chars)\n",line_number,MAX_LINE_SIZE-1);
      int foo;
      /* We skip the remaining characters of the line */
      while ((foo=u_fgetc(delas))!=EOF && foo!='\n');
      if (foo==EOF) {
         /* If the failing line was the last one, we exit the main loop */
         break;
      }
      continue;
   }
   if (size_line>0) {
      if (line[size_line-1]=='\n') {
        line[size_line-1]='\0';
      }
   }
   if (line[0]=='\0') {
      continue;
   }
   /* We split the line */
   if ((n_parts=tokenize_kr_mwu_dic_line(part,line))==-1) {
      /* If there is an error, we ignore this line */
      continue;
   }
   /* Then we check that the n_parts-1 first ones are valid DELAF entries
    * and that the last one is a valid DELAS entry. But first, we initialize
    * the array */
   for (int i=0;i<n_parts;i++) {
      entries[i]=NULL;
   }
   int OK=1;
   for (int i=0;i<n_parts-1;i++) {
      entries[i]=tokenize_DELAF_line(part[i]);
      if (entries==NULL) {
         OK=0;
         break;
      }
   }
   if (OK) {
      entries[n_parts-1]=tokenize_DELAS_line(part[n_parts-1],NULL);
      OK=(entries[n_parts-1]!=NULL);
   }
   if (OK) {
      /* If everything went OK, we can start inflecting the root of the last
       * component */
      produce_mwu_entries(n_parts,entries);
   }
   /* We free the 'part' and 'entries' tab*/
   for (int i=0;i<n_parts;i++) {
      free(part[i]);
      free_dela_entry(entries[i]);
   }
}
}


/**
 * 'line' is supposed to be of the form '{AA}{BB}{CC}...'. This function
 * tries to split 'line' into as many parts as needed, without the round brackets.
 * The parts are stored in 'part'. The size of 'part' is returned.
 * For '{AA}{BB}{CC}', we have:
 *
 * part[0]="AA"   part[1]="BB"   part[2]="CC"   return value=3
 *
 * Returns -1 in case of error.
 */
int tokenize_kr_mwu_dic_line(unichar** part,unichar* line) {
if (line==NULL) {
   fatal_error("NULL error in tokenize_kr_mwu_dic_line\n");
}
if (line[0]=='\0') {
   fatal_error("Empty line error in tokenize_kr_mwu_dic_line\n");
}
int pos=0,n_parts=0,new_pos;
unichar temp[MAX_LINE_SIZE];
while (line[pos]!='\0') {
   if (line[pos]!='{') {
      error("Line should start with '{':\n%S\n",line);
      for (int i=0;i<n_parts;i++) {free(part[i]);}
      return -1;
   }
   pos++;
   new_pos=pos;
   int res=parse_string(line,&new_pos,temp,P_CLOSING_ROUND_BRACKET,P_EMPTY,NULL);
   if (res!=P_OK || line[new_pos]=='\0') {
      error("Invalid DELAS line:\n%S\n",line);
      for (int i=0;i<n_parts;i++) {free(part[i]);}
      return -1;
   }
   if (n_parts==MAX_PARTS) {
      error("Invalid DELAS line with too many components (>%d):\n%S\n",MAX_PARTS,line);
      for (int i=0;i<n_parts;i++) {free(part[i]);}
      return -1;
   }
   part[n_parts++]=u_strdup(temp);
   pos=new_pos+1;
}
if (n_parts==1) {
   error("Invalid DELAS line with only one component:\n%S\n",line);
   free(part[0]);
   return -1;
}
return n_parts;
}


/**
 * TO DO
 */
void produce_mwu_entries(int n_parts,struct dela_entry** entries) {

}

