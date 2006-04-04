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

//--------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Copyright.h"
#include "unicode.h"
#include "CodePages.h"
#include "FileName.h"
#include "IOBuffer.h"
#include "Error.h"


/**
 * Please do not forget to modify the 'usage' function when you add an
 * encoding in 'CodePages'.
 */
void usage() {
printf("%s",COPYRIGHT);
printf("Usage: Convert <src> [<dest>] <mode> <text_1> [<text_2> <text_3> ...]\n");
printf(" <src> : encoding of the text file to be converted\n");
printf("<dest> : optional encoding of the destination text file. The default value \n");
printf("         is LITTLE-ENDIAN\n");
printf("The following values are possible:\n");
printf(" FRENCH\n");
printf(" ENGLISH\n");
printf(" GREEK\n");
printf(" THAI\n");
printf(" CZECH\n");
printf(" GERMAN\n");
printf(" SPANISH\n");
printf(" PORTUGUESE\n");
printf(" ITALIAN\n");
printf(" NORWEGIAN\n");
printf(" LATIN (default latin codepage)\n");
printf(" windows-1252: Microsoft Windows Codepage 1252 - Latin I (Western Europe & USA)\n");   //$CD:20021206
printf(" windows-1250: Microsoft Windows Codepage 1250 - Central Europe\n");                               //$CD:20021206
printf(" windows-1257: Microsoft Windows Codepage 1257 - Baltic\n");                                       //$CD:20021206
printf(" windows-1251: Microsoft Windows Codepage 1251 - Cyrillic\n");                                     //$CD:20021206
printf(" windows-1254: Microsoft Windows Codepage 1254 - Turkish\n");                                      //$CD:20021206
printf(" windows-1258: Microsoft Windows Codepage 1258 - Viet Nam\n");                                     //$CD:20021206
printf(" iso-8859-1  : ISO Character Set 8859-1  - Latin 1 (Western Europe & USA)\n");         //$CD:20021206
printf(" iso-8859-15 : ISO Character Set 8859-15 - Latin 9 (Western Europe & USA)\n");         //$CD:20021206
printf(" iso-8859-2  : ISO Character Set 8859-2  - Latin 2 (Central & Eastern Europe)\n");               //$CD:20021206
printf(" iso-8859-3  : ISO Character Set 8859-3  - Latin 3 (South Europe)\n");                             //$CD:20021206
printf(" iso-8859-4  : ISO Character Set 8859-4  - Latin 4 (North Europe)\n");                             //$CD:20021206
printf(" iso-8859-5  : ISO Character Set 8859-5  - Cyrillic\n");                                           //$CD:20021206
printf(" iso-8859-7  : ISO Character Set 8859-7  - Greek\n");                                              //$CD:20021206
printf(" iso-8859-9  : ISO Character Set 8859-9  - Latin 5 (Turkish)\n");                                  //$CD:20021206
printf(" iso-8859-10 : ISO Character Set 8859-10 - Latin 6 (Nordic)\n");                                   //$CD:20021206
#ifndef HGH_INSERT
printf(" windows-949 : Microsoft Windows Codepage 949 (Korean)\n");
printf(" KOREAN      : \n");
#endif // HGH_INSERT
printf(" next-step   : NextStep Codepage\n");
printf(" UTF-8 (only for output)\n");
printf(" LITTLE-ENDIAN\n");
printf(" BIG-ENDIAN\n");
printf("\n");
printf("<mode> : this parameter specifies how the source/destination files\n");
printf("         must be named. The possible values are:\n");
printf("   -r : sources files will be replaced by destination files\n");
printf("   -ps=PFX : source files will be renamed with the prefix PFX\n");
printf("   -pd=PFX : destination files will be named with the prefix PFX\n");
printf("   -ss=SFX : source files will be renamed with the suffix SFX\n");
printf("   -sd=SFX : destination files will be named with the suffix SFX\n");
printf("<text_i> : text file to be converted\n");
printf("\n");
printf("Converts a text file into another encoding.\n");
printf("Type 'Convert <lang>' where <lang> is a valid codepage or character set to see\n");
printf("the supported languages.\n");      //$CD:20021206
}



int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();
int TYPE_SRC;
int TYPE_DEST;
if (argc<2) {
	usage();
	return 0;
}
if (argc<3) {
	/*
	 * If the user has called the program with just one parameter,
	 * we look if it corresponds to the name of a code page. In that
	 * case, we print the appropriate information.
	 */                                                                //$CD:20021206
	if (!strcmp(argv[1],"LATIN") || !strcmp(argv[1],"windows-1252"))   //$CD:20021206
		usage_LATIN();                                                 //$CD:20021206
	else if (!strcmp(argv[1],"windows-1250"))                          //$CD:20021206
		usage_windows1250();                                           //$CD:20021206
	else if (!strcmp(argv[1],"windows-1257"))                          //$CD:20021206
		usage_windows1257();                                           //$CD:20021206
	else if (!strcmp(argv[1],"windows-1251"))                          //$CD:20021206
		usage_windows1251();                                           //$CD:20021206
	else if (!strcmp(argv[1],"windows-1254"))                          //$CD:20021206
		usage_windows1254();                                           //$CD:20021206
	else if (!strcmp(argv[1],"windows-1258"))                          //$CD:20021206
		usage_windows1258();                                           //$CD:20021206
	else if (!strcmp(argv[1],"iso-8859-1"))                            //$CD:20021206
		usage_iso88591();                                              //$CD:20021206
	else if (!strcmp(argv[1],"iso-8859-15"))                           //$CD:20021206
		usage_iso885915();                                             //$CD:20021206
	else if (!strcmp(argv[1],"iso-8859-2"))                            //$CD:20021206
		usage_iso88592();                                              //$CD:20021206
	else if (!strcmp(argv[1],"iso-8859-3"))                            //$CD:20021206
		usage_iso88593();                                              //$CD:20021206
	else if (!strcmp(argv[1],"iso-8859-4"))                            //$CD:20021206
		usage_iso88594();                                              //$CD:20021206
	else if (!strcmp(argv[1],"iso-8859-5"))                            //$CD:20021206
		usage_iso88595();                                              //$CD:20021206
	else if (!strcmp(argv[1],"iso-8859-7"))                            //$CD:20021206
		usage_iso88597();                                              //$CD:20021206
	else if (!strcmp(argv[1],"iso-8859-9"))                            //$CD:20021206
		usage_iso88599();                                              //$CD:20021206
	else if (!strcmp(argv[1],"iso-8859-10"))                           //$CD:20021206
		usage_iso885910();                                             //$CD:20021206
	/* If the parameter is not a valid code page name, we print the
	 * default usage information. */
	else usage();      
	return 0;                                                           //$CD:20021206
}                                                                       //$CD:20021206
/*
 * First, we set the properties of the input encoding. By default,
 * we say that it is an encoding on one byte. Then, we initialize
 * the characters equivalence table according to the encoding.
 */
TYPE_SRC=ONE_BYTE_ENCODING;
if (!strcmp(argv[1],"FRENCH") ||
	!strcmp(argv[1],"ENGLISH") ||
	!strcmp(argv[1],"GERMAN") ||
	!strcmp(argv[1],"SPANISH") ||
	!strcmp(argv[1],"PORTUGUESE") ||
	!strcmp(argv[1],"ITALIAN") ||
	!strcmp(argv[1],"NORWEGIAN") ||
	!strcmp(argv[1],"LATIN") ||                                        //$CD:20021206
	!strcmp(argv[1],"windows-1252"))                                   //$CD:20021206
	init_ansi(unicode_src);
else if (!strcmp(argv[1],"THAI"))
	init_thai(unicode_src);
else if (!strcmp(argv[1],"GREEK"))
	init_grec(unicode_src);
else if (!strcmp(argv[1],"CZECH"))
	init_tcheque(unicode_src);
else if (!strcmp(argv[1],"windows-1250"))                              //$CD:20021206
	init_windows1250(unicode_src);                                               //$CD:20021206
else if (!strcmp(argv[1],"windows-1257"))                              //$CD:20021206
	init_windows1257(unicode_src);                                               //$CD:20021206
else if (!strcmp(argv[1],"windows-1251"))                              //$CD:20021206
	init_windows1251(unicode_src);                                               //$CD:20021206
else if (!strcmp(argv[1],"windows-1254"))                              //$CD:20021206
	init_windows1254(unicode_src);                                               //$CD:20021206
else if (!strcmp(argv[1],"windows-1258"))                              //$CD:20021206
	init_windows1258(unicode_src);                                               //$CD:20021206
else if (!strcmp(argv[1],"iso-8859-1"))                                //$CD:20021206
	init_iso88591(unicode_src);                                                  //$CD:20021206
else if (!strcmp(argv[1],"iso-8859-15"))                               //$CD:20021206
	init_iso885915(unicode_src);                                                 //$CD:20021206
else if (!strcmp(argv[1],"iso-8859-2"))                                //$CD:20021206
	init_iso88592(unicode_src);                                                  //$CD:20021206
else if (!strcmp(argv[1],"iso-8859-3"))                                //$CD:20021206
	init_iso88593(unicode_src);                                                  //$CD:20021206
else if (!strcmp(argv[1],"iso-8859-4"))                                //$CD:20021206
	init_iso88594(unicode_src);                                                  //$CD:20021206
else if (!strcmp(argv[1],"iso-8859-5"))                                //$CD:20021206
	init_iso88595(unicode_src);                                                  //$CD:20021206
else if (!strcmp(argv[1],"iso-8859-7"))                                //$CD:20021206
	init_iso88597(unicode_src);                                                  //$CD:20021206
else if (!strcmp(argv[1],"iso-8859-9"))                                //$CD:20021206
	init_iso88599(unicode_src);                                                  //$CD:20021206
else if (!strcmp(argv[1],"iso-8859-10"))                               //$CD:20021206
	init_iso885910(unicode_src);                                                 //$CD:20021206
else if (!strcmp(argv[1],"next-step"))
	init_nextstep(unicode_src);
/*
 * If we have an encoding that is not on one byte 
 */
else if (!strcmp(argv[1],"UTF-8")) {
        TYPE_SRC=UTF8;
     }
#ifndef HGH_INSERT
else if (!strcmp(argv[1],"KOREAN")) {
	TYPE_SRC = MBCS_KR;
}
#endif // HGH_INSERT
else if (!strcmp(argv[1],"LITTLE-ENDIAN")) {
	TYPE_SRC=UTF16_LE;
}
else if (!strcmp(argv[1],"BIG-ENDIAN")) {
	TYPE_SRC=UTF16_BE;
}
else {
	fatal_error("Invalid language parameter %s\n",argv[1]);
}
/*
 * Then we deal with the output encoding on the same principle.
 */
TYPE_DEST=ONE_BYTE_ENCODING;
/* We will need to count the parameters, since the destination encoding
 * be omitted (UTF-16 Little-Endian by default). */
int param=3;
if (!strcmp(argv[2],"FRENCH") ||
	!strcmp(argv[2],"ENGLISH") ||
	!strcmp(argv[2],"GERMAN") ||
	!strcmp(argv[2],"SPANISH") ||
	!strcmp(argv[2],"PORTUGUESE") ||
	!strcmp(argv[2],"ITALIAN") ||
	!strcmp(argv[2],"NORWEGIAN") ||
	!strcmp(argv[2],"LATIN") ||                                        //$CD:20021206
	!strcmp(argv[2],"windows-1252"))                                   //$CD:20021206
	init_ansi(unicode_dest);
else if (!strcmp(argv[2],"THAI"))
	init_thai(unicode_dest);
else if (!strcmp(argv[2],"GREEK"))
	init_grec(unicode_dest);
else if (!strcmp(argv[2],"CZECH"))
	init_tcheque(unicode_dest);
else if (!strcmp(argv[2],"windows-1250"))                              //$CD:20021206
	init_windows1250(unicode_dest);                                               //$CD:20021206
else if (!strcmp(argv[2],"windows-1257"))                              //$CD:20021206
	init_windows1257(unicode_dest);                                               //$CD:20021206
else if (!strcmp(argv[2],"windows-1251"))                              //$CD:20021206
	init_windows1251(unicode_dest);                                               //$CD:20021206
else if (!strcmp(argv[2],"windows-1254"))                              //$CD:20021206
	init_windows1254(unicode_dest);                                               //$CD:20021206
else if (!strcmp(argv[2],"windows-1258"))                              //$CD:20021206
	init_windows1258(unicode_dest);                                               //$CD:20021206
else if (!strcmp(argv[2],"iso-8859-1"))                                //$CD:20021206
	init_iso88591(unicode_dest);                                                  //$CD:20021206
else if (!strcmp(argv[2],"iso-8859-15"))                               //$CD:20021206
	init_iso885915(unicode_dest);                                                 //$CD:20021206
else if (!strcmp(argv[2],"iso-8859-2"))                                //$CD:20021206
	init_iso88592(unicode_dest);                                                  //$CD:20021206
else if (!strcmp(argv[2],"iso-8859-3"))                                //$CD:20021206
	init_iso88593(unicode_dest);                                                  //$CD:20021206
else if (!strcmp(argv[2],"iso-8859-4"))                                //$CD:20021206
	init_iso88594(unicode_dest);                                                  //$CD:20021206
else if (!strcmp(argv[2],"iso-8859-5"))                                //$CD:20021206
	init_iso88595(unicode_dest);                                                  //$CD:20021206
else if (!strcmp(argv[2],"iso-8859-7"))                                //$CD:20021206
	init_iso88597(unicode_dest);                                                  //$CD:20021206
else if (!strcmp(argv[2],"iso-8859-9"))                                //$CD:20021206
	init_iso88599(unicode_dest);                                                  //$CD:20021206
else if (!strcmp(argv[2],"iso-8859-10"))                               //$CD:20021206
	init_iso885910(unicode_dest);                                                 //$CD:20021206
else if (!strcmp(argv[2],"next-step"))
	init_nextstep(unicode_dest);
/*
 * If we have not an encoding on one byte
 */
else if (!strcmp(argv[2],"UTF-8")) {
	TYPE_DEST=UTF8;
}
else if (!strcmp(argv[2],"LITTLE-ENDIAN")) {
	TYPE_DEST=UTF16_LE;
}
else if (!strcmp(argv[2],"BIG-ENDIAN")) {
	TYPE_DEST=UTF16_BE;
}   
#ifndef HGH_INSERT
else if (!strcmp(argv[2],"KOREAN")) {
	TYPE_DEST=MBCS_KR;
}
#endif // HGH_INSERT
else {
	/* By default, the output encoding is UTF-16 Little-Endian*/
	TYPE_DEST=UTF16_LE;
	param--;     
}
/*
 * If the output encoding is a 1 byte one, we must 
 * initialize the ascii_dest array.
 */
if (TYPE_DEST==ONE_BYTE_ENCODING) {
	init_uni2asc_code_page_array();
}
/*
 * We analyze the next parameters.
 */
int output_mode;
char FX[128];
if (!strcmp(argv[param],"-r")) {
	/* If we must replace the input file */
	output_mode=REPLACE_FILE;
}
else if (argv[param][0]=='-' && argv[param][1]=='p' 
		&& argv[param][2]=='s' && argv[param][3]=='='
		&& argv[param][4]!='\0') {
		/* We are in the case -ps=PFX */
		output_mode=PREFIX_SRC;
		int i=-1;
		do {
			i++;
			FX[i]=argv[param][4+i];
		} while (FX[i]!='\0');
}
else if (argv[param][0]=='-' && argv[param][1]=='p' 
		&& argv[param][2]=='d' && argv[param][3]=='='
		&& argv[param][4]!='\0') {
		/* We are in the case -pd=PFX */
		output_mode=PREFIX_DEST;
		int i=-1;
		do {
			i++;
			FX[i]=argv[param][4+i];
		} while (FX[i]!='\0');
}
else if (argv[param][0]=='-' && argv[param][1]=='s' 
		&& argv[param][2]=='s' && argv[param][3]=='='
		&& argv[param][4]!='\0') {
		/* We are in the case -ss=SFX */
		output_mode=SUFFIX_SRC;
		int i=-1;
		do {
			i++;
			FX[i]=argv[param][4+i];
		} while (FX[i]!='\0');
}
else if (argv[param][0]=='-' && argv[param][1]=='s' 
		&& argv[param][2]=='d' && argv[param][3]=='='
		&& argv[param][4]!='\0') {
		/* We are in the case -sd=SFX */
		output_mode=SUFFIX_DEST;
		int i=-1;
		do {
			i++;
			FX[i]=argv[param][4+i];
		} while (FX[i]!='\0');
}
else {
	/* If there is an error */
    fatal_error("Wrong argument: %s\n",argv[param]);
}
/*
 * Now we will transcode all the files described by the remaining
 * parameters.
 */
param++;
char input_name[2048];
char output_name[2048];
FILE* input=NULL;
FILE* output=NULL;
int error_code;
for (int i=param;i<argc;i++) {
	/*
	 * We set input and output file names according to the output mode
	 */
	int problem=0;
	int file_type=u_is_a_unicode_file(argv[i]);
	if (file_type==UNICODE_LITTLE_ENDIAN_FILE && TYPE_DEST==UTF16_LE) {
		problem=1;
		error("%s is already a Unicode Little-Endian file\n",argv[i]);
	}
	if (file_type==UNICODE_BIG_ENDIAN_FILE && TYPE_DEST==UTF16_BE) {
		problem=1;
		error("%s is already a Unicode Big-Endian file\n",argv[i]);
	}
	if (!problem) {
		switch (output_mode) {
			case REPLACE_FILE: strcpy(input_name,argv[i]); 
							add_suffix_to_file_name(output_name,input_name,"_TEMP"); break;
			case PREFIX_SRC: strcpy(output_name,argv[i]); 
							add_prefix_to_file_name(input_name,output_name,FX); 
							remove(input_name);
							rename(argv[i],input_name); break;
			case SUFFIX_SRC: strcpy(output_name,argv[i]); 
							add_suffix_to_file_name(input_name,output_name,FX); 
							remove(input_name);
							rename(argv[i],input_name); break;
			case PREFIX_DEST: strcpy(input_name,argv[i]); 
							add_prefix_to_file_name(output_name,input_name,FX); break;
			case SUFFIX_DEST: strcpy(input_name,argv[i]); 
							add_suffix_to_file_name(output_name,input_name,FX); break;
			default: fatal_error("Internal error in Convert\n");
		}
		/*
		 * We open files as binary ones. Note that we do not read the 2-bytes
		 * header in the case of unicode files. This is delegated to the
		 * conversion function.
		 */
		input=fopen(input_name,"rb");
		if (input==NULL) {
			error("Cannot open %s\n",argv[i]);
			problem=1;
		}
		else {
			output=fopen(output_name,"wb");
			if (output==NULL) {
				error("Cannot write %s\n",output_name);
				problem=1;
				fclose(input);
			}
		}
		/*
		 * We do the conversion and we close the files.
		 */
		error_code=convert(input,output,TYPE_SRC,TYPE_DEST);
		fclose(input);
		fclose(output);
		switch(error_code) {
			case CONVERSION_OK: printf("%s converted\n",argv[i]);
								if (output_mode==REPLACE_FILE) {
									/* If we must replace the input file */
									remove(argv[i]);
									rename(output_name,argv[i]);
								}
								break;
			case INPUT_FILE_NOT_IN_UTF16_LE: 
								error("Error: %s is not a Unicode Little-Endian file\n",argv[i]); break;
			case INPUT_FILE_NOT_IN_UTF16_BE: 
								error("Error: %s is not a Unicode Big-Endian file\n",argv[i]); break;
			default: fatal_error("Internal error in Convert\n");
		}
	}
}
return 0;
}



