 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Copyright.h"
#include "Unicode.h"
#include "CodePages.h"
#include "File.h"
#include "IOBuffer.h"
#include "Error.h"


#define REPLACE_FILE 0
#define PREFIX_SRC 1
#define SUFFIX_SRC 2
#define PREFIX_DEST 3
#define SUFFIX_DEST 4


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Convert <src> [<dest>] <mode> [HTML OPTIONS] <text_1> [<text_2> <text_3> ...]\n");
u_printf(" <src> : encoding of the text file to be converted\n");
u_printf("<dest> : optional encoding of the destination text file. The default value \n");
u_printf("         is LITTLE-ENDIAN\n");
u_printf("<mode> : this parameter specifies how the source/destination files\n");
u_printf("         must be named. The possible values are:\n");
u_printf("   -r : sources files will be replaced by destination files\n");
u_printf("   -ps=PFX : source files will be renamed with the prefix PFX\n");
u_printf("   -pd=PFX : destination files will be named with the prefix PFX\n");
u_printf("   -ss=SFX : source files will be renamed with the suffix SFX\n");
u_printf("   -sd=SFX : destination files will be named with the suffix SFX\n");
u_printf("<text_i> : text file to be converted\n");
u_printf("\n");
u_printf("Optional HTML OPTIONS can be a combination of the followings:\n");
u_printf("   -dnc (Decode Normal Chars): things like '&eacute;' '&#120;' and '&#xF8;'\n");
u_printf("        will be decoded as the one equivalent unicode character, except if\n");
u_printf("        it represents an HTML control character\n");
u_printf("   -dcc (Decode Control Chars): '&lt;' '&gt;' '&amp;' and '&quot;'\n");
u_printf("        will be decoded as '<' '>' '&' and the quote (the same for their\n");
u_printf("        decimal and hexadecimal representations)\n");
u_printf("   -eac (Encode All Chars): every character that is not supported by the\n");
u_printf("        output encoding will be coded as a string like '&#457;'\n");
u_printf("   -ecc (Encode Control Chars): '<' '>' '&' and the quote will be encoded\n");
u_printf("         by '&lt;' '&gt;' '&amp;' and '&quot;'\n");
u_printf("All the HTML options are deactivated by default.\n");
u_printf("\n");
u_printf("Converts a text file into another encoding.\n");
u_printf("You can also launch the program with only one parameter:\n\n");
u_printf("'-main-names' to get the list of the encoding main names\n");
u_printf("'-aliases' to get the list of the encoding aliases\n");
u_printf("'-all-infos' to display all the information about all the encodings\n");
u_printf("'XXX' to get information about the encoding XXX\n");
}



int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

if (argc<2) {
	usage();
	return 0;
}
/* First, we install all the available encoding */
install_all_encodings();
/* And we analyze the parameters */
if (argc<3) {
	/* If the user has called the program with just one parameter */
	if (!strcmp(argv[1],"-main-names")) {
		/* We do not print the copyright message since this list
		 * can be read by another program. */
		print_encoding_main_names();
		return 0;
	}
	if (!strcmp(argv[1],"-aliases")) {
		/* We do not print the copyright message since this list
		 * can be read by another program. */
		print_encoding_aliases();
		return 0;
	}
	if (!strcmp(argv[1],"-all-infos")) {
		/* We do not print the copyright message since this list
		 * can be read by another program. */
		u_printf("%S",COPYRIGHT);
		print_information_for_all_encodings();
		return 0;
	}
	/* We look if it corresponds to the name of a code page. In that
	 * case, we print the appropriate information.
	 */
	print_encoding_infos(argv[1]);
	return 0;
}
/*
 * First, we set the properties of the input encoding. By default,
 * we say that it is an encoding on one byte. Then, we initialize
 * the characters equivalence table according to the encoding.
 */
struct encoding* src_encoding=get_encoding(argv[1]);
if (src_encoding==NULL) {
	fatal_error("%s is not a valid encoding name\n",argv[1]);
} 
if (argc==3) {
	fatal_error("Invalid parameters.\n");
}
struct encoding* dest_encoding=get_encoding(argv[2]);
int param=3;
if (dest_encoding==NULL) {
	/* If the 2nd parameter is not an encoding name, we assume that it
	 * is the <mode> parameter, and we set the output encoding to
	 * UTF16 Little-Endian by default.
	 */
	 param--;
	 dest_encoding=get_encoding("utf16-le");
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
/*
 * We read the HTML options if any
 */
int decode_normal_characters=0;
int decode_control_characters=0;
int encode_all_characters=0;
int encode_control_characters=0;
int done;
do {
	if (param==argc) {
		/* If there is no more parameter, we raise an error */
		fatal_error("Invalid parameters.\n");
	}
	if (!strcmp(argv[param],"-dnc")) {
		decode_normal_characters=1;
		param++;
	}
	else if (!strcmp(argv[param],"-dcc")) {
		decode_control_characters=1;
		param++;
	}
	else if (!strcmp(argv[param],"-eac")) {
		encode_all_characters=1;
		param++;
	}
	else if (!strcmp(argv[param],"-ecc")) {
		encode_control_characters=1;
		param++;
	}
	else {
		done=1;
	}
} while (!done);

char input_name[2048];
char output_name[2048];
FILE* input=NULL;
FILE* output=NULL;
int error_code;
for (int i=param;i<argc;i++) {
	/*
	 * We set input and output file names according to the output mode
	 */
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
	int problem=0;
	if (input==NULL) {
		error("Cannot open %s\n",argv[i]);
		problem=1;
	}
	else {
		output=fopen(output_name,"wb");
		if (output==NULL) {
			error("Cannot write to file %s\n",output_name);
			problem=1;
			fclose(input);
		}
	}
	/*
	 * We do the conversion and we close the files.
	 */
	if (!problem) {
		error_code=convert(input,output,src_encoding,dest_encoding,
							decode_normal_characters,
							decode_control_characters,
							encode_all_characters,
							encode_control_characters);
		fclose(input);
		fclose(output);
		switch(error_code) {
			case CONVERSION_OK: u_printf("%s converted\n",argv[i]);
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



