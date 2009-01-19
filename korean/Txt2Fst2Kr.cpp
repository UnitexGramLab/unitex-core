 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

// txt2fst2kr -m [word/morpheme/both]  text
//
// make fst2 graphe from les segmentation
// default input files :	text.cod token.txt soustoken.txt 
// before action       :  segmentation & consulation of simple words
//
//	korean text fst2
//	count of sentences
//	[automate of the sentence]*
//  [tokens]*
//	[morphemes]*     forme flechi , forme canonique, forme orgin, info

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "Unicode.h"
#include "etc.h"
#include "String_hash2.h"
#include "strToAuto.h"
#include "Copyright.h"
#include "IOBuffer.h"



static void usage(int flag) {
u_printf("%S",COPYRIGHT);
u_printf(
"Txt2Fst2Kr [[-e #sentence -p[m/a]]|[-c]|[-m/a fst2file] file\n"\
"Txt2Fst2Kr :  get a fst2 for the 'number'th sentence\n"\
"e : extract a automata with the form of fst2\n"\
"p : show the sentence \n"\
"  m  : morhpeme : with morphemes only\n"\
"  a  : both : with morphemes and word(default value)\n"\
"c : create automata of sentences from code de text\n"\
"m : modify a automata\n"\
"a : add a automata\n"\
"\n"
);
exit(flag);
}


int main(int argc,char **argv) {
setBufferMode();

	mkTxt2Fst2Kr txt;
	int argIdx = 1;
	int lineNumber= 0;
	int action = 0;	// code file
	char *fstname = 0;

//		printf("%s\n",setlocale(LC_ALL,"Korean_korea.949"));
    if(argc == 1) usage(0);
	while(argIdx < argc -1 ){
		if(argv[argIdx][0] != '-'){
			argIdx++;
			continue;
		}
		switch(argv[argIdx][1]){
		case 'p':
			switch(argv[argIdx][2]){
			case 'm':txt.modeSet(MORPHEMEONLY);break;
			case 'a':txt.modeSet(BOTHSET);break;
			default:
				usage(1);
			}
			argIdx++;
			lineNumber = atoi(argv[argIdx]);
			break;
		case 's':	// set blanc mark					// reserve
			break;
		case 'e':
			action = 2;
			argIdx++;
			lineNumber = atoi(argv[argIdx]);
			break;
		case 'c': action = 1;
			break;
		case 'a':
			argIdx++;
			fstname = argv[argIdx];
			action = 3; 
			break;
		case 'm':
			action = 4; 
			argIdx++;
			fstname = argv[argIdx];
			break;
		default:
			usage(1);
		}
		argIdx++;
	}
	if(argc -1  != argIdx) usage(1);
	// divide segments by morphemes
	// from token file
	unichar defaultSeparateurs[]={(unichar)' ',(unichar)'+',(unichar)'(',
                     (unichar)')',(unichar)'{',(unichar)'}',(unichar)'\0'};
	txt.pathNameSet(argv[argIdx]);
	switch(action){
	case 1:
		txt.convertIdxFileToFst2(argv[argIdx],defaultSeparateurs);
		break;
	case 2:
		txt.getUnePhraseFst3(argv[argIdx],lineNumber);
		break;
	case 3:
		if(fstname) usage(1);
		txt.addition(fstname,argv[argIdx]);
		break;
	case 4:
		if(fstname) usage(1);
		txt.modification(fstname,argv[argIdx]);
		break;
	default:
		usage(1);
	}
	return(0);
}
