/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include <string.h>
#include <stdlib.h>
#include "IOBuffer.h"
#include "Unicode.h"
#include "Copyright.h"
#include "Error.h"
#include "AbstractFst2Load.h"
#include "Alphabet.h"
#include "Grf_lib.h"
#include "GrfBeauty.h"
#include "LocateConstants.h"
#include "Locate.h"
#include "Fst2.h"
#include "ProgramInvoker.h"
#include "Normalize.h"
#include "Fst2Txt.h"
#include "Tokenize.h"

/**
 * This program is designed for test purpose only.
 */
int main(int argc,char* argv[]) {
setBufferMode();
#if 0
if (argc!=4) {
	fatal_error("Usage: cmd <txt> start end\n");
}
U_FILE* f=u_fopen(UTF16_LE,argv[1],U_READ);
if (f==NULL) return 1;
int a,b;
if (1!=sscanf(argv[2],"%d",&a) || a <0) {
	fatal_error("Invalid start position %d\n",a);
}
if (1!=sscanf(argv[3],"%d",&b) || b <a) {
	fatal_error("Invalid end position %d\n",b);
}
int i;
for (i=0;i<a-40;i++) u_fgetc_raw(f);
int c;
for (;i<a;i++) {
	c=u_fgetc_raw(f);
	u_printf("%C",c);
}
u_printf("<$");
for (;i<b;i++) u_printf("%C",u_fgetc_raw(f));
u_printf("$>");
for (i=0;i<40;i++) {
	c=u_fgetc_raw(f);
	if (c==EOF) break;
	u_printf("%C",c);
}
u_printf("\n");

u_fclose(f);
return 0;
#endif

/*VersatileEncodingConfig vec=VEC_DEFAULT;*/

#define PFX ""

exec_unitex_command(main_Normalize,"Normalize",PFX"/home/paumier/tmp/toto.txt","-r/home/paumier/unitex/French/Norm.txt","-qutf8-no-bom",NULL);
exec_unitex_command(main_Fst2Txt,"Fst2Txt","-t"PFX"/home/paumier/tmp/toto.snt","/home/paumier/unitex/French/Graphs/Preprocessing/Sentence/Sentence.fst2","-a/home/paumier/unitex/French/Alphabet.txt","-M","-qutf8-no-bom",NULL);
exec_unitex_command(main_Fst2Txt,"Fst2Txt","-t"PFX"/home/paumier/tmp/toto.snt","/home/paumier/unitex/French/Graphs/Preprocessing/Replace/Replace.fst2","-a/home/paumier/unitex/French/Alphabet.txt","-R","-qutf8-no-bom",NULL);
exec_unitex_command(main_Tokenize,"Tokenize",PFX"/home/paumier/tmp/toto.snt","-a/home/paumier/unitex/French/Alphabet.txt","-qutf8-no-bom",NULL);
return 0;
}




