 /*
  * Unitex
  *
  * Copyright (C) 2001-2008 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

using namespace std;
#include <locale.h>
#include <stdlib.h>

#include "Unicode.h"
#include "File.h"
#include "etc.h"
#include "bitmap.h"
#include "Copyright.h"
#include "jamoCodage.h"
#include "state_machine_fst2.h"
#include "IOBuffer.h"



#define SZ1M	1024*1024
#define SZ8M	1024*1024*8
#define S64K	1024*64

static void usage(int flag)
{
u_printf("%S",COPYRIGHT);
u_printf("Jamo2Syl  [-m mapfile][-c Str=0xNNNN]* [-o outfile] transducter.fst2 input_file\n");
u_printf("-m : encodage file\n");
u_printf("-c : change pre-define value in transducter with '<' et '>' <Str> to hexdemal value\n");
u_printf("-o : name of output file\n");
u_printf("   : default outfile name \"input_fileSyl.ext\"\n");
u_printf("transducter.fst2 : decoder \n");
exit(flag);
}


class localtemp : public state_machine, public jamoCodage
{
public:
    localtemp(){
	};
    ~localtemp(){};
} trans;



int main(int argc, char *argv[]) {
setBufferMode();
    
  	char *ifilename = 0;
	char *ofilename =0;
	
	char extension[16];
	unichar temp[256];
	int iargIndex = 1;
	FILE *ifile;
	FILE *ofile;
	debugPrFlag = 0;



	if(argc == 1) usage(0);
	while(iargIndex < argc){
		if(*argv[iargIndex] != '-') break;
		switch(argv[iargIndex][1]){
		case 'm':iargIndex++;
		        trans.loadJamoMap(argv[iargIndex]);
		        setStrToVal(trans.sylMarkStr,trans.sylMark);
		    break;
		case 'c': iargIndex++;
            trans.mbcsToUniStr((unsigned char *)argv[iargIndex],temp);
		    if(changeStrToVal(temp)) usage(1);
			break;
		case 'o':iargIndex++; 
			ofilename = new char [strlen(argv[iargIndex])+1];
			strcpy(ofilename,argv[iargIndex]);
			break;
		default:usage(1);
		}
		iargIndex++;
	}
	if(iargIndex != (argc -2 )) usage(1);

	trans.init_machine(argv[iargIndex],2);
	iargIndex++;			
	ifilename = new char [strlen(argv[iargIndex])+1];
	strcpy(ifilename,argv[iargIndex]);
	if(!(ifile = u_fopen(ifilename,U_READ)))usage(1);

	if(!ofilename){
		ofilename = new char [strlen(ifilename)+4];
		remove_extension(ifilename,ofilename);
		get_extension(ifilename,extension);
		strcat(ofilename,"syl");
		strcat(ofilename,extension);
	}
	 
	if(!(ofile = u_fopen(ofilename,U_WRITE))) { 
		fatal_error("Can't open %s file for output\n",ofilename);
	}
	trans.convFile(ifilename,ofilename);

	delete ifilename;
	delete ofilename;
  return 0;
}
