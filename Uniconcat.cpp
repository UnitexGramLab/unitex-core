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

#include "unicode.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define BUFFER_SIZE 4096


void usage() {
printf("%s",COPYRIGHT);
printf("Usage: Uniconcat <src> <dst>\n");
printf(" <src>: unicode text file\n");
printf(" <dst>: unicode text file\n\n");
printf("Appends the content of <src> at the end of <dst>. <src> and\n");
printf("<dst> must be different files.\n");
}


int main(int argc, char *argv[]) {
setBufferMode();

if (argc!=3) {
   usage();
   return 1;
}

if (!strcmp(argv[1],argv[2])) {
   fprintf(stderr,"<src> and <dst> files must be different\n");
   fatal_error(1);
}

FILE* src=u_fopen(argv[1],U_READ);
if (src==NULL) {
   fprintf(stderr,"Cannot open %s\n",argv[1]);
   fatal_error(1);
}

FILE* dst=u_fopen(argv[2],U_APPEND);
if (dst==NULL) {
   fprintf(stderr,"Cannot open %s\n",argv[2]);
   u_fclose(src);
   fatal_error(1);
}
unichar buffer[BUFFER_SIZE];
int n;
while (0!=(n=u_fread(buffer,BUFFER_SIZE,src))) {
   u_fwrite(buffer,n,dst);
}

u_fclose(dst);
u_fclose(src);
return 0;
}
