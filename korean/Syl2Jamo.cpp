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
#include "Unicode.h"
#include "File.h"
#include "Copyright.h"
#include "jamoCodage.h"
#include "etc.h"
#include "IOBuffer.h"
#include "Error.h"

// using namespace std;


//
//	change korean syllabes to korean alphabet
//	syl2alpakr.exe -m mapfile -o out_file input_file 
//
static int jamoFlag;


#define SZ1M	1024*1024
#define SZ8M	1024*1024*8
#define S64K	1024*64


const char* usage_Syl2Jamo =
         "Syl2Jamo [-m mapfile] [-o outfile] [-j] [-c cTable] [-e [m/j]] input_file\n" \
         "-m : convert table a sylabe to jamo\n" \
         "-o : name of output file\n" \
         "-O : remplace file \n" \
         "   : default outfile name \"input_fileJM.ext\"\n" \
         "-j : convert UNICODE compatibile jamos to UNICODE jamo of alphabet korean\n" \
         "-c : convert characters with cTable which have pairs of change set\n" \
         "   : format in the line \"[C/0xNNNN] [C/0xNNNN]\"  C: a character, NNNN : hex number\n" \
         "-e : m : display from a syllalbe to jamos table\n" \
         "-e : j : put jamoList to jamo.txt\n";


static void usage() {
u_printf("%S",COPYRIGHT);
u_printf(usage_Syl2Jamo);
}


#ifdef DELETE
void static setLocalKorean()
{
    return;
    /*
	char *langloc;
	langloc = strdup(setlocale(LC_CTYPE,"Korean_korea.949"));
	if(!langloc)
		fatal_error("lanque of the Korean did set fail\n");
	free(langloc);
        */
}
#endif





int main_Syl2Jamo(int argc,char *argv[]) {
	class jamoCodage hangul;

	char *ifilename = 0;
	char *ofilename =0;
	char extension[16];
	int iargIndex = 1;
	int remplaceFlag = 0;

    Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
    int bom_output = DEFAULT_BOM_OUTPUT;
    int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;

	U_FILE *ifile;
	U_FILE *ofile;

	jamoFlag  = 0;
	if(argc == 1) {
	   usage();
	   return 0;
	}
	while(iargIndex < argc){
		if(*argv[iargIndex] != '-') break;
		switch(argv[iargIndex][1]){
		case 'm': 
            iargIndex++;
			hangul.loadJamoMap(argv[iargIndex]);
			break;
		case 'o':iargIndex++; 
			ofilename = new char [strlen(argv[iargIndex])+1];
			strcpy(ofilename,argv[iargIndex]);
			break;
		case 'O':
		   remplaceFlag = 1; break; 
		case 'c':	// convertmap file for ideogramms
			iargIndex++;
			if(iargIndex >= argc) {
			   usage();
			   return 1;
			}
			hangul.loadHJAMap(argv[iargIndex]);
			break;
		case 'j': 
			jamoFlag = 1; break;
		case 'e': 
            iargIndex++;
			if(iargIndex >= argc) {
			   usage();
			   return 1;
			}
			switch(argv[iargIndex][0]){
			case 'm':
			{
//               setLocalKorean();
                ofile = u_fopen_creating_versatile_encoding(encoding_output,bom_output,"jamoTable.txt",U_WRITE);
                if(!ofile) fopenErrMessage("jamoTable.txt");
                int length = hangul.mbcs949clen((unsigned char *)defaultSylToJamoMap);
                unichar *outbuf = new unichar[length+1];
                hangul.mbcsToUniStr((unsigned char *)defaultSylToJamoMap,outbuf);
                u_fwrite_raw(outbuf,length,ofile);
				u_fclose(ofile);
//wprintf(L"%s",defaultSylToJamoMap);
                return(0);
            }
			case 'j':hangul.jamoMapOut(); return(0);
			}
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

	if(iargIndex != (argc-1)) {
	   return 1;
	}
//	setLocalKorean();

			
	ifilename = new char [strlen(argv[iargIndex])+1];
	strcpy(ifilename,argv[iargIndex]);

	ifile = u_fopen_existing_versatile_encoding(mask_encoding_compatibility_input,ifilename,U_READ);
	if(!ifile) {
	   usage();
	   return 1;
	}

	if(!ofilename){
		ofilename = new char [strlen(ifilename)+3];
		remove_extension(ifilename,ofilename);
		get_extension(ifilename,extension);
		strcat(ofilename,"jm");
		strcat(ofilename,extension);
	}
	 
	ofile = u_fopen_creating_versatile_encoding(encoding_output,bom_output,ofilename,U_WRITE);
	if(!ofile) { 
		fatal_error("Can't open %s file for output\n",ofilename);
	}

	int rsz;
	int hanjaCnt;
	unichar  *buff = new unichar[SZ1M+1];
	unichar  *hbuff = new unichar[SZ1M+1];
	unichar *obuff = new unichar [SZ8M+1];
	unichar *sbuff;
	if(!buff || !obuff) fatal_error("mem alloc fail\n");
	do{
		if((rsz = u_fread_raw(buff,SZ1M,ifile) ) ==  0) break;
		buff[rsz] = 0;
		hanjaCnt = hangul.convHJAtoHAN(buff,hbuff);
		if(hanjaCnt) sbuff = hbuff;
		else sbuff = buff;
		if(jamoFlag)
		  u_fwrite_raw(obuff,
                hangul.convertSyletCjamoToJamo(sbuff,obuff,rsz,SZ8M),ofile);
		else
		  u_fwrite_raw(obuff,
                hangul.convertSylToJamo(sbuff,obuff,rsz,SZ8M),ofile);
	} while(rsz == SZ1M);
	u_fclose(ifile);
	u_fclose(ofile);
	delete [] buff;
	delete [] hbuff;
	delete [] obuff;
	if(remplaceFlag){
	  remove(ifilename);
	  rename(ofilename,ifilename);
	}
	delete [] ifilename;
	delete [] ofilename;
	return(0);
}


