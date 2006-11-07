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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unicode.h"
#include "FileName.h"
#include "Copyright.h"
#include "DELA.h"
#include "String_hash.h"
#include "IOBuffer.h"


#define MAX_TAG_LENGTH 4000
#define KEEP_CARRIDGE_RETURN 0
#define REMOVE_CARRIDGE_RETURN 1



void usage() {
printf("%s",COPYRIGHT);
printf("Usage: Normalize <text> [-no_CR]\n");
printf("     <text> : text file to be normalized\n");
printf("     -no_CR :this optional parameter indicates that every separator\n");
printf("             sequence will be turned into a single space\n\n");
printf("Turns every sequence of separator chars (space, tab, new line) into one.\n");
printf("If a separator sequence contains a new line char, it is turned to a single new\n");
printf("line (except with -no_CR); if not, it is turned into a single space.\n");
printf("The result is stored in a file named file_name.snt.\n");
}


int N;


void update_position_in_file() {
N=N+2;
if ((N%(1024*1024))==0) {
   int l=N/(1024*1024);
   printf("%d megabyte%s read...      \r",l,(l>1)?"s":"");
}
}



void normalize(FILE* f,FILE* f_out,int mode) {
int c;
c=u_fgetc_normalized_carridge_return(f);
// 2 bytes for the unicode header and 2 bytes for the first character
N=4;
while (c!=EOF) {
   if (c==' ' || c=='\t' || c=='\n') {
      int enter=(c=='\n');
      if (enter) {
         // the new line sequence is coded with 2 characters
         update_position_in_file();
      }
      while ((c=u_fgetc_normalized_carridge_return(f))==' ' || c=='\t' || c=='\n') {
         update_position_in_file();
         if (c=='\n') {
            enter=1;
            update_position_in_file();
         }
      }
      update_position_in_file();
      if (enter && mode==KEEP_CARRIDGE_RETURN) u_fputc((unichar)'\n',f_out);
         else u_fputc((unichar)' ',f_out);
   }
   else if (c=='{') {
      unichar s[MAX_TAG_LENGTH+1];
      s[0]='{';
      int z=1;
      while (z<(MAX_TAG_LENGTH-1) && (c=u_fgetc_normalized_carridge_return(f))!='}' && c!='{' && c!='\n') {
         s[z++]=(unichar)c;
         update_position_in_file();
      }
      update_position_in_file();
      if (z==(MAX_TAG_LENGTH-1) || c!='}') {
         // if the tag has no ending }
         fprintf(stderr,"\nError at char %d: a tag without ending } has been found\n",(N-1)/2);
         fprintf(stderr,"The { char was replaced by a [  char\n");
         s[0]='[';
         if (c=='{') {
            c='[';
         }
         s[z]=(unichar)c;
         s[z+1]='\0';
      }
      else if (c=='\n') {
         // if the tag contains a return
         fprintf(stderr,"\nError at char %d: a tag containing a new-line sequence has been found\n",(N-1)/2);
         fprintf(stderr,"The { char was replaced by a [ char\n");
         s[0]='[';
         s[z]='\n';
         s[z+1]='\0';
      }
      else {
         s[z]='}';
         s[z+1]='\0';
         if (!u_strcmp_char(s,"{S}") || !u_strcmp_char(s,"{STOP}")) {
            // if we have found a sentence delimiter or the STOP marker
            // we have nothing special to do
         } else {
           if (!check_tag_token(s)) {
              // if a tag is incorrect, we exit
              fprintf(stderr,"\nError at char %d: the text contains an invalid tag\n",(N-1)/2);
              fprintf(stderr,"The { and } chars were replaced by the [ and ] chars\n");
              s[0]='[';
              s[z]=']';
           }
         }
      }
      u_fprints(s,f_out);
      c=u_fgetc_normalized_carridge_return(f);
      update_position_in_file();
   } else {
      u_fputc((unichar)c,f_out);
      c=u_fgetc_normalized_carridge_return(f);
      update_position_in_file();
   }
}
}




int main(int argc, char **argv) {
setBufferMode();

FILE* f;
FILE* f_out;
char n[1000];
if (argc!=2 && argc !=3) {
   usage();
   return 0;
}
int mode=KEEP_CARRIDGE_RETURN;
if (argc==3) {
   if (strcmp(argv[2],"-no_CR")) {
      fprintf(stderr,"Wrong parameter: %s\n",argv[2]);
      return 1;
   }
   else {
      mode=REMOVE_CARRIDGE_RETURN;
   }
}

char ext[1000];
get_extension(argv[1],ext);
if (!strcmp(ext,".snt")) {
   remove_extension(argv[1],ext);
   strcat(ext,".biniou");
   rename(argv[1],ext);
   f=u_fopen(ext,U_READ);
}
else {
   ext[0]='\0';
   f=u_fopen(argv[1],U_READ);
}
if (f==NULL) {
   fprintf(stderr,"Cannot open file %s\n",argv[1]);
   return 1;
}
remove_extension(argv[1],n);
strcat(n,".snt");
f_out=u_fopen(n,U_WRITE);
if (f_out==NULL) {
   fprintf(stderr,"Cannot create file %s\n",n);
   u_fclose(f);
   return 1;
}
printf("Normalizing %s...\n",argv[1]);
normalize(f,f_out,mode);
printf("\n");
u_fclose(f);
u_fclose(f_out);
printf("\n");
if (ext[0]!='\0') {
   remove(ext);
}
return 0;
}
//---------------------------------------------------------------------------
