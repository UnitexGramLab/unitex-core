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


#define MAX_LINE_SIZE 4096


int tokenize_dic_line(unichar** part,unichar* line);

void create_mwu_dictionary(U_FILE* delas) {
unichar line[MAX_LINE_SIZE];
int size_line;
int line_number=-1;
unichar* part[32];
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
   if (line[0]!='{') {
      error("Invalid line %d: should start with '{'\n",line_number);
   }
   if ((n_parts=tokenize_dic_line(part,line))==-1) {
      /* If there is an error, we ignore this line */
      continue;
   }

}
}



int tokenize_dic_line(unichar** part,unichar* line) {

}
