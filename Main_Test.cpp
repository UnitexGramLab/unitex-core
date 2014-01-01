/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Fst2.h"
#include "ProgramInvoker.h"
#include "Normalize.h"
#include "Fst2Txt.h"
#include "Tokenize.h"
#include "Dico.h"
#include "SortTxt.h"
#include "Locate.h"
#include "Concord.h"
#include "VirtualFiles.h"
#include <time.h>

using namespace unitex;

/**
 * This program is designed for test purpose only.
 */
int main(int argc,char* argv[]) {
setBufferMode();

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
#if 0


/* benchmark A: 80 jours, 10 itérations */

/*
 * benchmark A1: aucune optimisation
 *
real	0m34.424s
user	0m33.070s
sys		0m1.264s
*/

/*
 * benchmark A2: avec persistance des fichiers suivants:

load_persistent_alphabet("/home/paumier/unitex/French/Alphabet.txt");
load_persistent_dictionary("/home/paumier/Unitex3.0beta/French/Dela/dela-fr-public.bin");
load_persistent_dictionary("/home/paumier/unitex/French/Dela/communesFR+.bin");
load_persistent_fst2("/home/paumier/unitex/French/Graphs/Preprocessing/Sentence/Sentence.fst2");
load_persistent_fst2("/home/paumier/unitex/French/Graphs/Preprocessing/Replace/Replace.fst2");
load_persistent_fst2("/home/paumier/unitex/French/Dela/la_N-r+.fst2");
load_persistent_fst2("/home/paumier/unitex/French/Dela/fogg-r-.fst2");
load_persistent_fst2("/home/paumier/unitex/French/Dela/fogg-r.fst2");
load_persistent_fst2("/home/paumier/Unitex3.0beta/French/Dela/Suffixes+.fst2");
load_persistent_fst2("/home/paumier/unitex/French/Graphs/essai_poids.fst2");

real	0m31.826s
user	0m30.938s
sys		0m0.804s
*/

/*
 * benchmark A3: avec virtualisation

real	0m29.476s
user	0m29.446s
sys		0m0.024s
*/


/* benchmark B: 4k pris au début de 80 jours, 10 itérations


B1:
real	0m6.341s
user	0m5.852s
sys			0m0.428s

B2:
real	0m4.557s
user	0m4.084s
sys		0m0.092s

B3:
real	0m3.914s
user	0m3.896s
sys		0m0.016s

 */


/* benchmark C: 4k pris au début de 80 jours, 100 itérations


C1:
real	1m3.768s
user	0m59.080s
sys		0m3.996s

C2:
real	0m41.341s
user	0m39.054s
sys		0m0.700s

C3:
real	0m38.694s
user	0m38.546s
sys		0m0.136s

 */

#endif
#if 0
load_persistent_alphabet("/home/paumier/unitex/French/Alphabet.txt");
load_persistent_dictionary("/home/paumier/Unitex3.0beta/French/Dela/dela-fr-public.bin");
load_persistent_dictionary("/home/paumier/unitex/French/Dela/communesFR+.bin");
load_persistent_fst2("/home/paumier/unitex/French/Graphs/Preprocessing/Sentence/Sentence.fst2");
load_persistent_fst2("/home/paumier/unitex/French/Graphs/Preprocessing/Replace/Replace.fst2");
load_persistent_fst2("/home/paumier/unitex/French/Dela/la_N-r+.fst2");
load_persistent_fst2("/home/paumier/unitex/French/Dela/fogg-r-.fst2");
load_persistent_fst2("/home/paumier/unitex/French/Dela/fogg-r.fst2");
load_persistent_fst2("/home/paumier/Unitex3.0beta/French/Dela/Suffixes+.fst2");
load_persistent_fst2("/home/paumier/unitex/French/Graphs/essai_poids.fst2");

//init_virtual_files();

#define PFX ""
#define N 100

for (int i=0;i<N;i++) {
	exec_unitex_command(main_Normalize,"Normalize",PFX"/home/paumier/tmp/toto.txt","-r/home/paumier/unitex/French/Norm.txt","-qutf8-no-bom",NULL);
	exec_unitex_command(main_Fst2Txt,"Fst2Txt","-t"PFX"/home/paumier/tmp/toto.snt","/home/paumier/unitex/French/Graphs/Preprocessing/Sentence/Sentence.fst2","-a/home/paumier/unitex/French/Alphabet.txt","-M","-qutf8-no-bom",NULL);
	exec_unitex_command(main_Fst2Txt,"Fst2Txt","-t"PFX"/home/paumier/tmp/toto.snt","/home/paumier/unitex/French/Graphs/Preprocessing/Replace/Replace.fst2","-a/home/paumier/unitex/French/Alphabet.txt","-R","-qutf8-no-bom",NULL);
	exec_unitex_command(main_Tokenize,"Tokenize",PFX"/home/paumier/tmp/toto.snt","-a/home/paumier/unitex/French/Alphabet.txt","-qutf8-no-bom",NULL);
	exec_unitex_command(main_Dico,"Dico","-t"PFX"/home/paumier/tmp/toto.snt","-a/home/paumier/unitex/French/Alphabet.txt","-m/home/paumier/Unitex3.0beta/French/Dela/dela-fr-public.bin","-m/home/paumier/unitex/French/Dela/communesFR+.bin","/home/paumier/unitex/French/Dela/la_N-r+.fst2","/home/paumier/unitex/French/Dela/fogg-r-.fst2","/home/paumier/unitex/French/Dela/fogg-r.fst2","/home/paumier/Unitex3.0beta/French/Dela/Suffixes+.fst2","/home/paumier/Unitex3.0beta/French/Dela/dela-fr-public.bin","-qutf8-no-bom",NULL);
	exec_unitex_command(main_SortTxt,"SortTxt",PFX"/home/paumier/tmp/toto_snt/dlf","-l"PFX"/home/paumier/tmp/toto_snt/dlf.n","-o/home/paumier/unitex/French/Alphabet_sort.txt","-qutf8-no-bom",NULL);
	exec_unitex_command(main_SortTxt,"SortTxt",PFX"/home/paumier/tmp/toto_snt/dlc","-l"PFX"/home/paumier/tmp/toto_snt/dlc.n","-o/home/paumier/unitex/French/Alphabet_sort.txt","-qutf8-no-bom",NULL);
	exec_unitex_command(main_SortTxt,"SortTxt",PFX"/home/paumier/tmp/toto_snt/err","-l"PFX"/home/paumier/tmp/toto_snt/err.n","-o/home/paumier/unitex/French/Alphabet_sort.txt","-qutf8-no-bom",NULL);
	exec_unitex_command(main_SortTxt,"SortTxt",PFX"/home/paumier/tmp/toto_snt/tags_err","-l"PFX"/home/paumier/tmp/toto_snt/tags_err.n","-o/home/paumier/unitex/French/Alphabet_sort.txt","-qutf8-no-bom",NULL);
	exec_unitex_command(main_Locate,"Locate","-t"PFX"/home/paumier/tmp/toto.snt","/home/paumier/unitex/French/Graphs/essai_poids.fst2","-a/home/paumier/unitex/French/Alphabet.txt","-L","-M","--all","-m/home/paumier/Unitex3.0beta/French/Dela/dela-fr-public.bin","-m/home/paumier/unitex/French/Dela/communesFR+.bin","-b","-Y","-qutf8-no-bom",NULL);
	exec_unitex_command(main_Concord,"Concord",PFX"/home/paumier/tmp/toto_snt/concord.ind","-fCourier 10 Pitch","-s12","-l40","-r55","--html","-a/home/paumier/unitex/French/Alphabet_sort.txt","--CL","-qutf8-no-bom",NULL);
}
#endif

#if 0
VersatileEncodingConfig vec=VEC_DEFAULT;
/*
~/workspace/C++/bin/Test ~/unitex/French/Corpus/seq2grf_snt/seq2grf.grf ~/unitex/French/Corpus/seq2grf_snt/beautiful.grf

~/workspace/C++/bin/Test ~/unitex/French/Graphs/a.grf ~/unitex/French/Graphs/a_beautiful.grf
 */
if (argc!=3) {
	fatal_error("Usage: Test <grf> <output grf>\n");
	return 1;
}
Grf* grf=load_Grf(&vec,argv[1]);
if (grf==NULL) {
	fatal_error("Cannot load %s\n",argv[1]);
}
beautify(grf,NULL);
U_FILE* f=u_fopen(&vec,argv[2],U_WRITE);
if (f==NULL) {
	fatal_error("Cannot create %s\n",argv[2]);
}
save_Grf(f,grf);
u_fclose(f);
free_Grf(grf);

return 0;
#endif
}



