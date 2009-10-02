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

#include <stdlib.h>
// using namespace std;
#include "Unicode.h"
#include "etc.h"
#include "jamoCodage.h"
#include "File.h"
#include "state_machine_fst2.h"
#include "Copyright.h"
#include "IOBuffer.h"





const char* usage_ExtractChar =
"ExtractChar [-o outFile] [-c SS=0xxxx]* file.fs2 inputfile\n"\
" -o outFile : if this option not exist, save paths at \"file\"lst.txt\n"\
" -c SS=0xXXXX: change the symbol string between symbols < and >,\"<SS>\" \n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_ExtractChar);
}


int main_ExtractChar(int argc, char *argv[]) {
    unsigned short retvalue[3];

    char *ifilename;
  	char *ofilename =0;
	int iargIndex = 1;
	
	char extTmp[16];
	U_FILE *ifile = 0;
	U_FILE *ofile = 0;
	class state_machine extracter;
	class jamoCodage encode;

    Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
    int bom_output = DEFAULT_BOM_OUTPUT;
    int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;

	unichar tempBuff[1024];
	if(argc == 1) {
	   usage();
	   return 0;
	}
	retvalue[0] = 0x00d0;
	retvalue[1] = 0x00a0;
	retvalue[2] = 0;
	
	while(iargIndex < argc){
		if(*argv[iargIndex] != '-') break;
		switch(argv[iargIndex][1]){
		case 'o':iargIndex++; 
			ofilename = argv[iargIndex];
			break;
		case 'c':iargIndex++;
		    u_strcpy(tempBuff,argv[iargIndex]);
			if(!changeStrToVal(extracter.GetChangeStrContext(),(unsigned short*)tempBuff)) break;
        case 'k': iargIndex++;
                 if (argv[iargIndex][0]=='\0') {
                    fatal_error("Empty input_encoding argument\n");
                 }
                 decode_reading_encoding_parameter(&mask_encoding_compatibility_input,argv[iargIndex]);
                 break;
        case 'q': iargIndex++;
                 if (argv[iargIndex][0]=='\0') {
                    fatal_error("Empty output_encoding argument\n");
                 }
                 decode_writing_encoding_parameter(&encoding_output,&bom_output,argv[iargIndex]);
                 break;
		default:
			usage();
			return 1;
		}
		iargIndex++;
	}
//printf("langue change %s\n",setlocale(LC_ALL,"Korean_Korea.949"));
	if(iargIndex != (argc -2 )) {
	   usage();
	   return 1;
	}
	extracter.init_machine(argv[iargIndex],2);
	iargIndex++;
	ifilename = argv[iargIndex];
	if(!ofilename){
	   ofilename = new char [strlen(ifilename)+3];
	   get_extension(ifilename,extTmp);
	   remove_extension(ifilename,ofilename);
	   strcat(ofilename,"ex");
	   strcat(ofilename,extTmp);
	   
	}
	ofile = u_fopen_creating_versatile_encoding(encoding_output,bom_output,ofilename,U_WRITE);
	if(!ofile) fopenErrMessage(ofilename);

	ifile = u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,ifilename,U_READ);
	if(!ifile) fopenErrMessage(ifilename);

	unsigned short exWord[1024];
	unsigned short testWord[1024];
	
	unsigned short  *wp,*wwp;
	int ii;
    while(EOF!=u_fgets(tempBuff,ifile)){
	    wp = (unsigned short *)tempBuff;
	    if((*wp != '\0') && (*wp != ' ')){
             for (ii = 0; *wp && (*wp != ',');ii++)
                exWord[ii] = *wp++;
             while(*wp) wp++;
             encode.convertSylToJamo(exWord,testWord,ii,1023);
             wwp = testWord;
             do{
                extracter.curSMvalue(*wwp);
                for(ii = 0; ii < extracter.outCnt;ii++)
                  *wp++= (unsigned short)(extracter.uWord[ii]);
                extracter.outCnt=0;
                
             } while(*wwp++);
             *wp=0;
         }
         u_fprintf(ofile,"%S\n",tempBuff);
	}
  
  return 0;
}

