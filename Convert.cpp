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


//--------------------------------------------------------------


//
// the usage function is in the file CodePages.cpp
//


int main(int argc, char **argv) {
setBufferMode();

FILE* entree=NULL;
FILE* sortie=NULL;
char res[1000];
int i;
int TYPE_SRC;
int TYPE_DEST;

if (argc<2) {                                                          //$CD:20021206
   usage();
   return 0;
}
else if (argc<3) {                                                     //$CD:20021206
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
    else usage();      
   return 0;                                                           //$CD:20021206
}                                                                      //$CD:20021206
//------- <src> ----------------------------
TYPE_SRC=ISO_XXX;
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
else if (!strcmp(argv[1],"UTF-8")) {
        //fprintf(stderr,"Error: UTF-8 encoding is not allowed for input files");
        //return 1;
        TYPE_SRC=UTF8;
     }
#ifndef HGH_INSERT
else if (!strcmp(argv[1],"KOREAN"))
     TYPE_SRC = MBCS_KR;
#endif // HGH_INSERT
else if (!strcmp(argv[1],"LITTLE-ENDIAN"))
     TYPE_SRC=UTF16_LE;
else if (!strcmp(argv[1],"BIG-ENDIAN"))
     TYPE_SRC=UTF16_BE;

else {
     fprintf(stderr,"Invalid language parameter %s\n",argv[1]);
     return 1;
}
//------- <dest> ----------------------------
TYPE_DEST=ISO_XXX; 
int PARAM=3;
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
else if (!strcmp(argv[2],"UTF-8"))
     TYPE_DEST=UTF8;
else if (!strcmp(argv[2],"LITTLE-ENDIAN")) {
        TYPE_DEST=UTF16_LE;
     }
else if (!strcmp(argv[2],"BIG-ENDIAN"))
     TYPE_DEST=UTF16_BE;
#ifndef HGH_INSERT
else if (!strcmp(argv[2],"KOREAN"))
     TYPE_DEST=MBCS_KR;
#endif // HGH_INSERT
else {
     // default case
     TYPE_DEST=UTF16_LE;
     PARAM--;     
}
// if the output encoding is a 8 bits one, we must 
// initialize the ascii_dest array
if (TYPE_DEST==ISO_XXX) {
   init_uni2asc_code_page_array(unicode_dest,ascii_dest);
}
//---------- -r -ps=PFX -pd=PFX -ss=SFX -sd=SFX ---------- 
int output_mode=-1;
char FX[128];
if (!strcmp(argv[PARAM],"-r")) output_mode=REPLACE_FILE;
else if (argv[PARAM][0]=='-' && argv[PARAM][1]=='p' 
         && argv[PARAM][2]=='s' && argv[PARAM][3]=='='
         && argv[PARAM][4]!='\0') {
         // we are in the case -ps=PFX
         output_mode=PREFIX_SRC;
         int i=-1;
         do {
            i++;
            FX[i]=argv[PARAM][4+i];
         } while (FX[i]!='\0');
}
else if (argv[PARAM][0]=='-' && argv[PARAM][1]=='p' 
         && argv[PARAM][2]=='d' && argv[PARAM][3]=='='
         && argv[PARAM][4]!='\0') {
         // we are in the case -pd=PFX
         output_mode=PREFIX_DEST;
         int i=-1;
         do {
            i++;
            FX[i]=argv[PARAM][4+i];
         } while (FX[i]!='\0');
}
else if (argv[PARAM][0]=='-' && argv[PARAM][1]=='s' 
         && argv[PARAM][2]=='s' && argv[PARAM][3]=='='
         && argv[PARAM][4]!='\0') {
         // we are in the case -ss=SFX
         output_mode=SUFFIX_SRC;
         int i=-1;
         do {
            i++;
            FX[i]=argv[PARAM][4+i];
         } while (FX[i]!='\0');
}
else if (argv[PARAM][0]=='-' && argv[PARAM][1]=='s' 
         && argv[PARAM][2]=='d' && argv[PARAM][3]=='='
         && argv[PARAM][4]!='\0') {
         // we are in the case -sd=SFX
         output_mode=SUFFIX_DEST;
         int i=-1;
         do {
            i++;
            FX[i]=argv[PARAM][4+i];
         } while (FX[i]!='\0');
}
else {
    fprintf(stderr,"Wrong argument: %s\n",argv[PARAM]);
}
//-----------------------------------------------------
PARAM++;
char input[1024];
char output[1024];
int error_code;
for (i=PARAM;i<argc;i++) {
    //--------- setting input and output file names --------------
    int problem=0;
    int file_type=u_is_a_unicode_file(argv[i]);
    if (file_type==UNICODE_LITTLE_ENDIAN_FILE && TYPE_DEST==UTF16_LE) {
       problem=1;
       fprintf(stderr,"%s is already a Unicode Little-Endian file\n",argv[i]);
    }
    if (file_type==UNICODE_BIG_ENDIAN_FILE && TYPE_DEST==UTF16_BE) {
       problem=1;
       fprintf(stderr,"%s is already a Unicode Big-Endian file\n",argv[i]);
    }
    if (!problem) {
    switch (output_mode) {
      case REPLACE_FILE: strcpy(input,argv[i]); 
                         add_suffix_to_file_name(output,input,"_TEMP"); break;
      case PREFIX_SRC: strcpy(output,argv[i]); 
                       add_prefix_to_file_name(input,output,FX); 
                       remove(input);
                       rename(argv[i],input); break;
      case SUFFIX_SRC: strcpy(output,argv[i]); 
                       add_suffix_to_file_name(input,output,FX); 
                       remove(input);
                       rename(argv[i],input); break;
      case PREFIX_DEST: strcpy(input,argv[i]); 
                        add_prefix_to_file_name(output,input,FX); break;
      case SUFFIX_DEST: strcpy(input,argv[i]); 
                        add_suffix_to_file_name(output,input,FX); break;
    }
    //--------- opening files --------------
    entree=fopen(input,"rb");
    if (entree==NULL) {
       fprintf(stderr,"Cannot open %s\n",argv[i]);
       problem=1;
    }
    else {
       sortie=fopen(output,"wb");
       if (sortie==NULL) {
          fprintf(stderr,"Cannot write %s\n",res);
          problem=1;
          fclose(entree);
       }
    }
    }
    //-------- if there is no problem, we do the conversion ---------
    if (!problem) {
       /*fprintf(sortie,"%c%c",0xFF,0xFE);
       if (code_page==ISO_XXX) {
          convert_ascii_to_unicode(entree,sortie);
       }
       else if (code_page==UTF16_BE) {
          convert_big_to_little_endian(entree,sortie,argv[i]);
       }*/
       error_code=convert(entree,sortie,TYPE_SRC,TYPE_DEST);
       fclose(entree);
       fclose(sortie);
       //--------- renaming files --------------
       switch(error_code) {
         case CONVERSION_OK: printf("%s converted\n",argv[i]);
                             if (output_mode==REPLACE_FILE) {
                                remove(argv[i]);
                                rename(output,argv[i]);
                             }
                             break;
         case INPUT_FILE_NOT_IN_UTF16_LE: 
           fprintf(stderr,"Error: %s is not a Unicode Little-Endian file\n",argv[i]); break;
         case INPUT_FILE_NOT_IN_UTF16_BE: 
           fprintf(stderr,"Error: %s is not a Unicode Big-Endian file\n",argv[i]); break;
       }
    }
}
return 0;
}



