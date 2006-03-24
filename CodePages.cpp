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

//---------------------------------------------------------------------------
#include "unicode.h"
#include "CodePages.h"
#include "Copyright.h"
#ifndef HGH_INSERT
#include "codePageKr.h"
#endif // HGH_INSERT

//---------------------------------------------------------------------------


unsigned short int unicode_src[256];
unsigned short int unicode_dest[256];
unsigned char ascii_dest[65536];


void init_thai(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0x80]=0x20ac;
unicode[0x85]=0x2026;
unicode[0x91]=0x2018;
unicode[0x92]=0x2019;
unicode[0x93]=0x201c;
unicode[0x94]=0x201d;
unicode[0x95]=0x2022;
unicode[0x96]=0x2013;
unicode[0x97]=0x2014;
for (i=0xa1;i<0xdb;i++)
  unicode[i]=(unsigned short int)(i+(0x0e00-0xa0));
for (i=0xdf;i<0xfc;i++)
  unicode[i]=(unsigned short int)(i+(0x0e00-0xa0));
}



void init_nextstep(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0xD5]=0xE0;
unicode[0xD7]=0xE2;
unicode[0xDB]=0xE7;
unicode[0xDC]=0xE8;
unicode[0xDD]=0xE9;
unicode[0xDE]=0xEA;
unicode[0xDF]=0xEB;
unicode[0xF4]=0xFB;
unicode[0xEE]=0xF4;
unicode[0xE4]=0xEE;
unicode[0xD9]=0xE4;
unicode[0xE5]=0xEF;
unicode[0xF6]=0xFC;
unicode[0xF0]=0xF6;
unicode[0xF2]=0xF9;
}



void init_ansi(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0x80]=0x20ac;
unicode[0x82]=0x201a;
unicode[0x83]=0x0192;
unicode[0x84]=0x201e;
unicode[0x85]=0x2026;
unicode[0x86]=0x2020;
unicode[0x87]=0x2021;
unicode[0x88]=0x02c6;
unicode[0x89]=0x2030;
unicode[0x8a]=0x0160;
unicode[0x8b]=0x2039;
unicode[0x8c]=0x0152;
unicode[0x8e]=0x017d;
unicode[0x91]=0x2018;
unicode[0x92]=0x2019;
unicode[0x93]=0x201c;
unicode[0x94]=0x201d;
unicode[0x95]=0x2022;
unicode[0x96]=0x2013;
unicode[0x97]=0x2014;
unicode[0x98]=0x02dc;
unicode[0x99]=0x2122;
unicode[0x9a]=0x0161;
unicode[0x9b]=0x203a;
unicode[0x9c]=0x0153;
unicode[0x9e]=0x017e;
unicode[0x9f]=0x0178;
}



void init_grec(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0x80]=0x20ac;
unicode[0x82]=0x201a;
unicode[0x83]=0x0192;
unicode[0x84]=0x201e;
unicode[0x85]=0x2026;
unicode[0x86]=0x2020;
unicode[0x87]=0x2021;
unicode[0x89]=0x2030;
unicode[0x8b]=0x2039;
unicode[0x91]=0x2018;
unicode[0x92]=0x2019;
unicode[0x93]=0x201c;
unicode[0x94]=0x201d;
unicode[0x95]=0x2022;
unicode[0x96]=0x2013;
unicode[0x97]=0x2014;
unicode[0x99]=0x2122;
unicode[0x9b]=0x203a;
unicode[0xa1]=0x0385;
unicode[0xa2]=0x0386;
unicode[0xaf]=0x2015;
unicode[0xb4]=0x0384;
unicode[0xb8]=0x0388;
unicode[0xb9]=0x0389;
unicode[0xba]=0x038a;
unicode[0xbc]=0x038c;
unicode[0xbe]=0x038e;
unicode[0xbf]=0x038f;
for (i=0xc0;i<=0xd1;i++)
   unicode[i]=(unsigned short int)(i+(0x0390-0xc0));
for (i=0xd3;i<=0xfe;i++)
   unicode[i]=(unsigned short int)(i+(0x0390-0xc0));
}



void init_tcheque(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
   unicode[i]=i;
unicode[0x80]=0x20ac;
unicode[0x82]=0x201a;
unicode[0x84]=0x201e;
unicode[0x85]=0x2026;
unicode[0x86]=0x2020;
unicode[0x87]=0x2021;
unicode[0x89]=0x2030;
unicode[0x8a]=0x0160;
unicode[0x8b]=0x2039;
unicode[0x8c]=0x015a;
unicode[0x8d]=0x0164;
unicode[0x8e]=0x017d;
unicode[0x8f]=0x0179;
unicode[0x91]=0x2018;
unicode[0x92]=0x2019;
unicode[0x93]=0x201c;
unicode[0x94]=0x201d;
unicode[0x95]=0x2022;
unicode[0x96]=0x2013;
unicode[0x97]=0x2014;
unicode[0x99]=0x2122;
unicode[0x9a]=0x0161;
unicode[0x9b]=0x203a;
unicode[0xa1]=0x02c7;
unicode[0xa2]=0x02d8;
unicode[0xa3]=0x0141;
unicode[0xa5]=0x0104;
unicode[0xaa]=0x015e;
unicode[0xaf]=0x017b;
unicode[0xb2]=0x02db;
unicode[0xb3]=0x0142;
unicode[0xb9]=0x0105;
unicode[0xba]=0x015f;
//unicode[0xbc]=0x0178;   //$CD:20021206 old
unicode[0xbc]=0x013d;     //$CD:20021206
unicode[0xbd]=0x02dd;
//unicode[0xbe]=0x013d;   //$CD:20021206 old
unicode[0xbe]=0x013e;     //$CD:20021206
unicode[0xbf]=0x017c;
unicode[0xc0]=0x0154;
unicode[0xc3]=0x0102;
unicode[0xc5]=0x0139;
unicode[0xc6]=0x0106;
unicode[0xc8]=0x010c;
unicode[0xca]=0x0118;
unicode[0xcc]=0x011a;
unicode[0xcf]=0x010e;
unicode[0xd0]=0x0110;
unicode[0xd1]=0x0143;
unicode[0xd2]=0x0147;
unicode[0xd5]=0x0150;
unicode[0xd8]=0x0158;
unicode[0xd9]=0x016e;
unicode[0xdb]=0x0170;
unicode[0xde]=0x0162;
unicode[0xe0]=0x0155;
unicode[0xe3]=0x0103;
unicode[0xe5]=0x013a;
unicode[0xe6]=0x0107;
unicode[0xe8]=0x010d;
unicode[0xea]=0x0119;
unicode[0xec]=0x011b;
unicode[0xef]=0x010f;
unicode[0xf0]=0x0111;
unicode[0xf1]=0x0144;
unicode[0xf2]=0x0148;
unicode[0xf5]=0x0151;
unicode[0xf8]=0x0159;
unicode[0xf9]=0x016f;
unicode[0xfb]=0x0171;
unicode[0xfe]=0x0163;
unicode[0xff]=0x02d9;
}



//$CD:20021206 begin
void init_windows1250(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
   unicode[i]=i;
unicode[0x80]=0x20ac; unicode[0x82]=0x201a; unicode[0x84]=0x201e; unicode[0x85]=0x2026;
unicode[0x86]=0x2020; unicode[0x87]=0x2021; unicode[0x89]=0x2030; unicode[0x8a]=0x0160;
unicode[0x8b]=0x2039; unicode[0x8c]=0x015a; unicode[0x8d]=0x0164; unicode[0x8e]=0x017d;
unicode[0x8f]=0x0179; 
unicode[0x91]=0x2018; unicode[0x92]=0x2019; unicode[0x93]=0x201c; unicode[0x94]=0x201d;
unicode[0x95]=0x2022; unicode[0x96]=0x2013; unicode[0x97]=0x2014; unicode[0x99]=0x2122;
unicode[0x9a]=0x0161; unicode[0x9b]=0x203a; unicode[0x9c]=0x015b; unicode[0x9d]=0x0165;
unicode[0x9e]=0x017e; unicode[0x9f]=0x017a;
unicode[0xa1]=0x02c7; unicode[0xa2]=0x02d8; unicode[0xa3]=0x0141; unicode[0xa5]=0x0104;
unicode[0xaa]=0x015e; unicode[0xaf]=0x017b;
unicode[0xb2]=0x02db; unicode[0xb3]=0x0142; unicode[0xb9]=0x0105; unicode[0xba]=0x015f;
unicode[0xbc]=0x013d; unicode[0xbd]=0x02dd; unicode[0xbe]=0x013e; unicode[0xbf]=0x017c;
unicode[0xc0]=0x0154; unicode[0xc3]=0x0102; unicode[0xc5]=0x0139; unicode[0xc6]=0x0106;
unicode[0xc8]=0x010c; unicode[0xca]=0x0118; unicode[0xcc]=0x011a; unicode[0xcf]=0x010e;
unicode[0xd0]=0x0110; unicode[0xd1]=0x0143; unicode[0xd2]=0x0147; unicode[0xd5]=0x0150;
unicode[0xd8]=0x0158; unicode[0xd9]=0x016e; unicode[0xdb]=0x0170; unicode[0xde]=0x0162;
unicode[0xe0]=0x0155; unicode[0xe3]=0x0103; unicode[0xe5]=0x013a; unicode[0xe6]=0x0107;
unicode[0xe8]=0x010d; unicode[0xea]=0x0119; unicode[0xec]=0x011b; unicode[0xef]=0x010f;
unicode[0xf0]=0x0111; unicode[0xf1]=0x0144; unicode[0xf2]=0x0148; unicode[0xf5]=0x0151;
unicode[0xf8]=0x0159; unicode[0xf9]=0x016f; unicode[0xfb]=0x0171; unicode[0xfe]=0x0163;
unicode[0xff]=0x02d9;
}

void init_windows1257(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
   unicode[i]=i;
unicode[0x80]=0x20ac; unicode[0x82]=0x201a; unicode[0x84]=0x201e; unicode[0x85]=0x2026;
unicode[0x86]=0x2020; unicode[0x87]=0x2021; unicode[0x89]=0x2030; unicode[0x8b]=0x2039;
unicode[0x8d]=0x00a8; unicode[0x8e]=0x02c7; unicode[0x8f]=0x00b8;
unicode[0x91]=0x2018; unicode[0x92]=0x2019; unicode[0x93]=0x201c; unicode[0x94]=0x201d;
unicode[0x95]=0x2022; unicode[0x96]=0x2013; unicode[0x97]=0x2014; unicode[0x99]=0x2122;
unicode[0x9b]=0x203a; unicode[0x9d]=0x00af; unicode[0x9e]=0x02db;
unicode[0xa8]=0x00d8; unicode[0xaa]=0x0156; unicode[0xaf]=0x00c6;
unicode[0xb8]=0x00f8; unicode[0xba]=0x0157; unicode[0xbf]=0x00e6;
unicode[0xc0]=0x0104; unicode[0xc1]=0x012e; unicode[0xc2]=0x0100; unicode[0xc3]=0x0106;
unicode[0xc6]=0x0118; unicode[0xc7]=0x0112; unicode[0xc8]=0x010c; unicode[0xca]=0x0179;
unicode[0xcb]=0x0116; unicode[0xcc]=0x0122; unicode[0xcd]=0x0136; unicode[0xce]=0x012a;
unicode[0xcf]=0x013b;
unicode[0xd0]=0x0160; unicode[0xd1]=0x0143; unicode[0xd2]=0x0145; unicode[0xd4]=0x014c;
unicode[0xd8]=0x0172; unicode[0xd9]=0x0141; unicode[0xda]=0x015a; unicode[0xdb]=0x016a;
unicode[0xdd]=0x017b; unicode[0xde]=0x017d;
unicode[0xe0]=0x0105; unicode[0xe1]=0x012f; unicode[0xe2]=0x0101; unicode[0xe3]=0x0107;
unicode[0xe6]=0x0119; unicode[0xe7]=0x0113; unicode[0xe8]=0x010d; unicode[0xea]=0x017a;
unicode[0xeb]=0x0117; unicode[0xec]=0x0123; unicode[0xed]=0x0137; unicode[0xee]=0x012b;
unicode[0xef]=0x013c;
unicode[0xf0]=0x0161; unicode[0xf1]=0x0144; unicode[0xf2]=0x0146; unicode[0xf4]=0x014d;
unicode[0xf8]=0x0173; unicode[0xf9]=0x0142; unicode[0xfa]=0x015b; unicode[0xfb]=0x016b;
unicode[0xfd]=0x017c; unicode[0xfe]=0x017e; unicode[0xff]=0x02d9;
}

void init_windows1251(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0x80]=0x0402; unicode[0x81]=0x0403; unicode[0x82]=0x201a; unicode[0x83]=0x0453;
unicode[0x84]=0x201e; unicode[0x85]=0x2026; unicode[0x86]=0x2020; unicode[0x87]=0x2021;
unicode[0x88]=0x20ac; unicode[0x89]=0x2030; unicode[0x8a]=0x0409; unicode[0x8b]=0x2039;
unicode[0x8c]=0x040a; unicode[0x8d]=0x040c; unicode[0x8e]=0x040b; unicode[0x8f]=0x040f;
unicode[0x90]=0x0452; unicode[0x91]=0x2018; unicode[0x92]=0x2019; unicode[0x93]=0x201c;
unicode[0x94]=0x201d; unicode[0x95]=0x2022; unicode[0x96]=0x2013; unicode[0x97]=0x2014;
unicode[0x99]=0x2122; unicode[0x9a]=0x0459; unicode[0x9b]=0x203a; unicode[0x9c]=0x045a;
unicode[0x9d]=0x045c; unicode[0x9e]=0x045b; unicode[0x9f]=0x045f;
unicode[0xa1]=0x040e; unicode[0xa2]=0x045e; unicode[0xa3]=0x0408; unicode[0xa5]=0x0490;
unicode[0xa8]=0x0401; unicode[0xaa]=0x0404; unicode[0xaf]=0x0407;
unicode[0xb2]=0x0406; unicode[0xb3]=0x0456; unicode[0xb4]=0x0491; unicode[0xb8]=0x0451;
unicode[0xb9]=0x2116; unicode[0xba]=0x0454; unicode[0xbc]=0x0458; unicode[0xbd]=0x0405;
unicode[0xbe]=0x0455; unicode[0xbf]=0x0457;
for (i=0xc0;i<=0xff;i++)
   unicode[i]=(unsigned short int)(i+(0x0410-0xc0));
}

void init_windows1254(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0x80]=0x20ac; unicode[0x82]=0x201a; unicode[0x83]=0x0192; unicode[0x84]=0x201e;
unicode[0x85]=0x2026; unicode[0x86]=0x2020; unicode[0x87]=0x2021; unicode[0x88]=0x02c6;
unicode[0x89]=0x2030; unicode[0x8a]=0x0160; unicode[0x8b]=0x2039; unicode[0x8c]=0x0152;
unicode[0x91]=0x2018; unicode[0x92]=0x2019; unicode[0x93]=0x201c; unicode[0x94]=0x201d;
unicode[0x95]=0x2022; unicode[0x96]=0x2013; unicode[0x97]=0x2014; unicode[0x98]=0x02dc;
unicode[0x99]=0x2122; unicode[0x9a]=0x0161; unicode[0x9b]=0x203a; unicode[0x9c]=0x0153;
unicode[0x9f]=0x0178;
unicode[0xd0]=0x011e; unicode[0xdd]=0x0130; unicode[0xde]=0x015e;
unicode[0xf0]=0x011f; unicode[0xfd]=0x0131; unicode[0xfe]=0x015f;
}

void init_windows1258(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0x80]=0x20ac; unicode[0x82]=0x201a; unicode[0x83]=0x0192; unicode[0x84]=0x201e;
unicode[0x85]=0x2026; unicode[0x86]=0x2020; unicode[0x87]=0x2021; unicode[0x88]=0x02c6;
unicode[0x89]=0x2030; unicode[0x8b]=0x2039; unicode[0x8c]=0x0152;
unicode[0x91]=0x2018; unicode[0x92]=0x2019; unicode[0x93]=0x201c; unicode[0x94]=0x201d;
unicode[0x95]=0x2022; unicode[0x96]=0x2013; unicode[0x97]=0x2014; unicode[0x98]=0x02dc;
unicode[0x99]=0x2122; unicode[0x9b]=0x203a; unicode[0x9c]=0x0153; unicode[0x9f]=0x0178;
unicode[0xc3]=0x0102; unicode[0xcc]=0x0300;
unicode[0xd0]=0x0110; unicode[0xd2]=0x0309; unicode[0xd5]=0x01a0; unicode[0xdd]=0x01af;
unicode[0xde]=0x0303;
unicode[0xe3]=0x0103; unicode[0xec]=0x0301;
unicode[0xf0]=0x0111; unicode[0xf2]=0x0323; unicode[0xf5]=0x01a1; unicode[0xfd]=0x01b0;
unicode[0xfe]=0x20ab;
}

void init_iso88591(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
}

void init_iso885915(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0xa4]=0x20ac; unicode[0xa6]=0x0160; unicode[0xa8]=0x0161;
unicode[0xb4]=0x017d; unicode[0xb8]=0x017e; unicode[0xbc]=0x0152; unicode[0xbd]=0x0153;
unicode[0xbe]=0x0178;
}

void init_iso88592(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0xa1]=0x0104; unicode[0xa2]=0x02d8; unicode[0xa3]=0x0141; unicode[0xa5]=0x013d;
unicode[0xa6]=0x015a; unicode[0xa9]=0x0160; unicode[0xaa]=0x015e; unicode[0xab]=0x0164;
unicode[0xac]=0x0179; unicode[0xae]=0x017d; unicode[0xaf]=0x017b;
unicode[0xb1]=0x0105; unicode[0xb2]=0x02db; unicode[0xb3]=0x0142; unicode[0xb5]=0x013e;
unicode[0xb6]=0x015b; unicode[0xb7]=0x02c7; unicode[0xb9]=0x0161; unicode[0xba]=0x015f;
unicode[0xbb]=0x0165; unicode[0xbc]=0x017a; unicode[0xbd]=0x02dd; unicode[0xbe]=0x017e;
unicode[0xbf]=0x017c;
unicode[0xc0]=0x0154; unicode[0xc3]=0x0102; unicode[0xc5]=0x0139; unicode[0xc6]=0x0106;
unicode[0xc8]=0x010c; unicode[0xca]=0x0118; unicode[0xcc]=0x011a; unicode[0xcf]=0x010e;
unicode[0xd0]=0x0110; unicode[0xd1]=0x0143; unicode[0xd2]=0x0147; unicode[0xd5]=0x0150;
unicode[0xd8]=0x0158; unicode[0xd9]=0x016e; unicode[0xdb]=0x0170; unicode[0xde]=0x0162;
unicode[0xe0]=0x0155; unicode[0xe3]=0x0103; unicode[0xe5]=0x013a; unicode[0xe6]=0x0107;
unicode[0xe8]=0x010d; unicode[0xea]=0x0119; unicode[0xec]=0x011b; unicode[0xef]=0x010f;
unicode[0xf0]=0x0111; unicode[0xf1]=0x0144; unicode[0xf2]=0x0148; unicode[0xf5]=0x0151;
unicode[0xf8]=0x0159; unicode[0xf9]=0x016f; unicode[0xfb]=0x0171; unicode[0xfe]=0x0163;
unicode[0xff]=0x02d9; 
}

void init_iso88593(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0xa1]=0x0126; unicode[0xa2]=0x02d8; unicode[0xa6]=0x0124; unicode[0xa9]=0x0130;
unicode[0xaa]=0x015e; unicode[0xab]=0x011e; unicode[0xac]=0x0134; unicode[0xaf]=0x017b;
unicode[0xb1]=0x0127; unicode[0xb6]=0x0125; unicode[0xb9]=0x0131; unicode[0xba]=0x015f;
unicode[0xbb]=0x011f; unicode[0xbc]=0x0135; unicode[0xbf]=0x017c;
unicode[0xc5]=0x010a; unicode[0xc6]=0x0108; 
unicode[0xd5]=0x0120; unicode[0xd8]=0x011c; unicode[0xdd]=0x016c; unicode[0xde]=0x015c;
unicode[0xe5]=0x010b; unicode[0xe6]=0x0109; 
unicode[0xf5]=0x0121; unicode[0xf8]=0x011d; unicode[0xfd]=0x016d; unicode[0xfe]=0x015d;
unicode[0xff]=0x02d9; 
}

void init_iso88594(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0xa1]=0x0104; unicode[0xa2]=0x0138; unicode[0xa3]=0x0156; unicode[0xa5]=0x0128;
unicode[0xa6]=0x013b; unicode[0xa9]=0x0160; unicode[0xaa]=0x0112; unicode[0xab]=0x0122;
unicode[0xac]=0x0166; unicode[0xae]=0x017d;
unicode[0xb1]=0x0105; unicode[0xb2]=0x02db; unicode[0xb3]=0x0157; unicode[0xb5]=0x0129;
unicode[0xb6]=0x013c; unicode[0xb7]=0x02c7; unicode[0xb9]=0x0161; unicode[0xba]=0x0113;
unicode[0xbb]=0x0123; unicode[0xbc]=0x0167; unicode[0xbd]=0x014a; unicode[0xbe]=0x017e;
unicode[0xbf]=0x014b;
unicode[0xc0]=0x0100; unicode[0xc7]=0x012e; unicode[0xc8]=0x010c; unicode[0xca]=0x0118;
unicode[0xcc]=0x0116; unicode[0xcf]=0x012a;
unicode[0xd0]=0x0110; unicode[0xd1]=0x0145; unicode[0xd2]=0x014c; unicode[0xd3]=0x0136;
unicode[0xd9]=0x0172; unicode[0xdd]=0x0168; unicode[0xde]=0x016a;
unicode[0xe0]=0x0101; unicode[0xe7]=0x012f; unicode[0xe8]=0x010d; unicode[0xea]=0x0119;
unicode[0xec]=0x0117; unicode[0xef]=0x012b;
unicode[0xf0]=0x0111; unicode[0xf1]=0x0146; unicode[0xf2]=0x014d; unicode[0xf3]=0x0137;
unicode[0xf9]=0x0173; unicode[0xfd]=0x0169; unicode[0xfe]=0x016b; unicode[0xff]=0x02d9;
}

void init_iso88595(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0xf0]=0x2116; unicode[0xfd]=0x00a7; unicode[0xfe]=0x045e; unicode[0xff]=0x045f;
for (i=0xa1;i<=0xac;i++)
   unicode[i]=(unsigned short int)(i+(0x0401-0xa1));
for (i=0xae;i<=0xef;i++)
   unicode[i]=(unsigned short int)(i+(0x040e -0xae));
for (i=0xf1;i<=0xfc;i++)
   unicode[i]=(unsigned short int)(i+(0x0451-0xf1));
}

void init_iso88597(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0xa1]=0x02bd; unicode[0xa2]=0x02bc; unicode[0xaf]=0x2015;
unicode[0xb4]=0x0384; unicode[0xb5]=0x0385; unicode[0xb6]=0x0386; unicode[0xb8]=0x0388;
unicode[0xb9]=0x0389; unicode[0xba]=0x038a; unicode[0xbc]=0x038c; 
for (i=0xbe;i<=0xd1;i++)
   unicode[i]=(unsigned short int)(i+(0x038e -0xbe));
for (i=0xd3;i<=0xfe;i++)
   unicode[i]=(unsigned short int)(i+(0x03a3-0xd3));
}

void init_iso88599(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0xd0]=0x011e; unicode[0xdd]=0x0130; unicode[0xde]=0x015e;
unicode[0xf0]=0x011f; unicode[0xfd]=0x0131; unicode[0xfe]=0x015f;
}

void init_iso885910(unsigned short int unicode[]) {
unsigned short int i;
for (i=0;i<256;i++)
  unicode[i]=i;
unicode[0xa1]=0x0104; unicode[0xa2]=0x0112; unicode[0xa3]=0x0122; unicode[0xa4]=0x012a;
unicode[0xa5]=0x0128; unicode[0xa6]=0x0136; unicode[0xa8]=0x013b; unicode[0xa9]=0x0110;
unicode[0xaa]=0x0160; unicode[0xab]=0x0166; unicode[0xac]=0x017d; unicode[0xae]=0x016a;
unicode[0xaf]=0x014a;
unicode[0xb1]=0x0105; unicode[0xb2]=0x0113; unicode[0xb3]=0x0123; unicode[0xb4]=0x012b;
unicode[0xb5]=0x0129; unicode[0xb6]=0x0137; unicode[0xb8]=0x013c; unicode[0xb9]=0x0111;
unicode[0xba]=0x0161; unicode[0xbb]=0x0167; unicode[0xbc]=0x017e; unicode[0xbd]=0x2015;
unicode[0xbe]=0x016b; unicode[0xbf]=0x014b;
unicode[0xc0]=0x0100; unicode[0xc7]=0x012e; unicode[0xc8]=0x010c; unicode[0xca]=0x0118;
unicode[0xcc]=0x0116; 
unicode[0xd1]=0x0145; unicode[0xd2]=0x014c; unicode[0xd7]=0x0168; unicode[0xd9]=0x0172;
unicode[0xe0]=0x0101; unicode[0xe7]=0x012f; unicode[0xe8]=0x010d; unicode[0xea]=0x0119;
unicode[0xec]=0x0117;
unicode[0xf1]=0x0146; unicode[0xf2]=0x014d; unicode[0xf7]=0x0169; unicode[0xf9]=0x0173;
unicode[0xff]=0x0138;
}

//$CD:20021206 end



//
// this function reads a unicode text file and produces an UTF-8 one
//
void convert_unicode_to_UTF_8(FILE *entree,FILE *sortie) {
int c;
while ((c=u_fgetc(entree))!=EOF) {
   u_fputc_utf8_diese((unichar)c,sortie);
}
}



void convert_unicode_to_ascii(FILE *entree,FILE *sortie) {
/*int c;
unsigned char n;
while ((c=u_fgetc(entree))!=EOF) {
   if (c=='\n') {
      n=0x0d;
      fwrite(&n,1,1,sortie);
      n=0x0a;
      fwrite(&n,1,1,sortie);
   } else {
      n=ascii[c];
      fwrite(&n,1,1,sortie);
   }
}
*/}



void convert_big_to_little_endian(FILE* entree,FILE* sortie,char* name) {
/*int c;
c=u_fgetc_big_endian(entree);
if (c!=U_BYTE_ORDER_MARK) {
   fprintf(stderr,"Error: file %s is not a big-endian one\n",name);
   return;
}
while ((c=u_fgetc_normalized_carridge_return_big_endian(entree))!=EOF) {
   u_fputc((unichar)c,sortie);
}
*/}



void convert_little_to_big_endian(FILE* entree,FILE* sortie) {
/*int c;
u_fputc(U_NOT_A_CHAR,sortie);
while ((c=u_fgetc(entree))!=EOF) {
   u_fputc_big_endian((unichar)c,sortie);
}
*/}



void convert_ascii_to_unicode(FILE *entree,FILE *sortie) {
/*int c;
while ((c=fgetc_normalized_carridge_return(entree))!=EOF) {
   u_fputc(unicode[c],sortie);
}
*/}



void init_uni2asc_code_page_array(unichar unicode[],unsigned char ascii[]) {
int i;
for (i=0;i<65536;i++) {
   ascii[i]='?';
}
for (i=0;i<256;i++) {
   ascii[unicode[i]]=(unsigned char)i;
}
}



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




//$CD:20021206 begin
void usage_LATIN() {
printf("LATIN : Microsoft Windows Codepage 1252 - Latin I (Western Europe & USA)\n");
printf("  Supported Languages :\n");
printf("    Afrikaans\n");
printf("    Basque\n");
printf("    Catalan\n");
printf("    Danish\n");
printf("    Dutch\n");
printf("    English\n");
printf("    Faroese\n");
printf("    Finnish\n");
printf("    French\n");
printf("    Galician\n");
printf("    German\n");
printf("    Icelandic\n");
printf("    Indonesian\n");
printf("    Italian\n");
printf("    Malay\n");
printf("    Norwegian\n");
printf("    Portuguese\n");
printf("    Spanish\n");
printf("    Swahili\n");
printf("    Swedish\n");
}


void usage_windows1250() {
printf("windows-1250 : Microsoft Windows Codepage 1250 - Central Europe\n");
printf("  Supported Languages :\n");
printf("    Albanian\n");
printf("    Croatian\n");
printf("    Czech\n");
printf("    Hungarian\n");
printf("    Polish\n");
printf("    Romanian\n");
printf("    Serbian (Latin)\n");
printf("    Slovak\n");
printf("    Slovenian\n");
}


void usage_windows1257() {
printf("windows-1257 : Microsoft Windows Codepage 1257 - Baltic\n");
printf("  Supported Languages :\n");
printf("    Estonian\n");
printf("    Latvian\n");
printf("    Lithuanian\n");
}



void usage_windows1251() {
printf("windows-1251 : Microsoft Windows Codepage 1251 - Cyrillic\n");
printf("  Supported Languages :\n");
printf("    Azeri (Cyrillic)\n");
printf("    Belarusian\n");
printf("    Bulgarian\n");
printf("    FYRO Macedonian\n");
printf("    Kazakh\n");
printf("    Kyrgyz\n");
printf("    Mongolian\n");
printf("    Russian\n");
printf("    Serbian (Cyrillic)\n");
printf("    Tatar\n");
printf("    Ukrainian\n");
printf("    Uzbek (Cyrillic)\n");
}



void usage_windows1254() {
printf("windows-1254 : Microsoft Windows Codepage 1254 - Turkish\n");
printf("  Supported Languages :\n");
printf("    Azeri (Latin)\n");
printf("    Turkish\n");
printf("    Uzbek (Latin)\n");
}



void usage_windows1258() {
printf("windows-1258 : Microsoft Windows Codepage 1258 - Viet Nam\n");
printf("  Supported Languages :\n");
printf("    Vietnamese\n");
}



void usage_iso88591() {
printf("iso-8859-1   : ISO Character Set 8859-1  - Latin 1 (Western Europe & USA)\n");
printf("  Supported Languages :\n");
printf("    Afrikaans\n");
printf("    Albanian\n");
printf("    Basque\n");
printf("    Catalan\n");
printf("    Danish\n");
printf("    Dutch\n");
printf("    English\n");
printf("    Faroese\n");
printf("    Finnish\n");
printf("    French\n");
printf("    German\n");
printf("    Icelandic\n");
printf("    Irish\n");
printf("    Italian\n");
printf("    Norwegian\n");
printf("    Portuguese\n");
printf("    Rhaeto-Romanic\n");
printf("    Scottish\n");
printf("    Spanish\n");
printf("    Swahili\n");
printf("    Swedish\n");
}



void usage_iso885915() {
printf("iso-8859-15  : ISO Character Set 8859-15 - Latin 9 (Western Europe & USA)\n");
printf("  Supported Languages :\n");
printf("    idem iso-8859-1 with the Euro sign and forgotten French and Finnish letters\n");
}



void usage_iso88592() {
printf("iso-8859-2   : ISO Character Set 8859-2  - Latin 2 (Central & Eastern Europe)\n");
printf("  Supported Languages :\n");
printf("    Albanian\n");
printf("    Croatian\n");
printf("    Czech\n");
printf("    English\n");
printf("    German\n");
printf("    Hungarian\n");
printf("    Polish\n");
printf("    Romanian\n");
printf("    Serbian (Latin)\n");
printf("    Slovak\n");
printf("    Slovenian\n");
}



void usage_iso88593() {
printf("iso-8859-3   : ISO Character Set 8859-3  - Latin 3 (South Europe)\n");
printf("  Supported Languages :\n");
printf("    Afrikaans\n");
printf("    Catalan\n");
printf("    Dutch\n");
printf("    English\n");
printf("    Esperanto\n");
printf("    Galician\n");
printf("    German\n");
printf("    Italian\n");
printf("    Maltese\n");
}



void usage_iso88594() {
printf("iso-8859-4   : ISO Character Set 8859-4  - Latin 4 (North Europe)\n");
printf("  Supported Languages :\n");
printf("    Danish\n");
printf("    English\n");
printf("    Estonian\n");
printf("    Finnish\n");
printf("    German\n");
printf("    Greenlandic\n");
printf("    Lappish\n");
printf("    Latvian\n");
printf("    Lithuanian\n");
printf("    Norwegian\n");
printf("    Swedish\n");
}



void usage_iso88595() {
printf("iso-8859-5   : ISO Character Set 8859-5  - Cyrillic\n");
printf("  Supported Languages :\n");
printf("    Bulgarian\n");
printf("    Byelorussian\n");
printf("    English\n");
printf("    Macedonian\n");
printf("    Russian\n");
printf("    Serbian\n");
printf("    Ukrainian\n");
}



void usage_iso88597() {
printf("iso-8859-7   : ISO Character Set 8859-7  - Greek\n");
printf("  Supported Languages :\n");
printf("    Greek modern monotonic\n");
}



void usage_iso88599() {
printf("iso-8859-9   : ISO Character Set 8859-9  - Latin 5 (Turkish)\n");
printf("  Supported Languages :\n");
printf("    Danish\n");
printf("    Dutch\n");
printf("    English\n");
printf("    Finnish\n");
printf("    French\n");
printf("    German\n");
printf("    Irish\n");
printf("    Italian\n");
printf("    Norwegian\n");
printf("    Portuguese\n");
printf("    Spanish\n");
printf("    Swedish\n");
printf("    Turkish\n");
}



void usage_iso885910() {
printf("iso-8859-10  : ISO Character Set 8859-10 - Latin 6 (Nordic)\n");
printf("  Supported Languages :\n");
printf("    Danish\n");
printf("    English\n");
printf("    Estonian\n");
printf("    Finnish\n");
printf("    German\n");
printf("    German\n");
printf("    Inuit (Greenlandic Eskimo)\n");
printf("    Lappish (non-Skolt Sami)\n");
printf("    Latvian\n");
printf("    Lithuanian\n");
printf("    Norwegian\n");
printf("    Swedish\n");
}

//$CD:20021206 end

#ifndef HGH_INSERT
convert_windows949kr_uni uniKoran949;
void usage_windows949() {
printf("Windows-949 : Microsoft Windows Codepage 949 - Korean\n");
printf("  Supported Languages :\n");
printf("    korean\n");
printf(" EUC-KR,ISO2022-kr,KSC5601\n");
printf("ex: Windows-949 [Korean/KOREAN/korean] [-pxx] filename ; wan-sung -> unicode\n");
printf("ex: [Korean/KOREAN/korean] Windows-949 [-pxx] filename ; unicode -> wan_sung\n");
}
void init_windows949()
{
    
}
int read_mbcs_char(FILE* f) {
    int page=fgetc(f);
    int off;
    if (page==EOF) return EOF;
    if(page & 0x80){
        off = fgetc(f);
        if(off == EOF) return EOF;  // obnomal case
        page &= 0x7f;
    } else {
        off = page;
        page = 0;
    }
    return uniKoran949.mbcsUni949Table[page*256+off];
}

int write_mbcs_char(unichar c,FILE* f) {
    int ret = fputc(uniKoran949.uniMbcs949Table[c*2]& 0xff,f);
    if(c < 128) return ret; 
    return fputc(uniKoran949.uniMbcs949Table[c*2+1] & 0xff,f);
}
#endif


int read_iso_char(FILE* f) {
int tmp=fgetc(f);
if (tmp==EOF) return EOF;
else return unicode_src[(unsigned char)tmp];
}

int write_iso_char(unichar c,FILE* f) {
return fputc(ascii_dest[(unichar)c],f);
}



int read_utf8_diese_char(FILE* f) {
int tmp=fgetc(f);
if (tmp==EOF) return EOF;
if (tmp!='&') return tmp;
tmp=fgetc(f);
if (tmp!='#') {
   // encoding error
   return '?';
}
int res=0;
while ((tmp=fgetc(f))!=';') {
   if (tmp<'0' || tmp>'9') {
      // encoding error
      return '?';
   }
   res=res*10+tmp-'0';
}
return res;
}



/*
 * converts a file to another encoding
 * @param input
 * @param output
 * @param INPUT_ENCODING
 * @param OUPUT_ENCODING
 * @return CONVERSION_OK if the conversion succeeded, an error code
 * otherwise
 **/
int convert(FILE* input,FILE* output,int INPUT_ENCODING,int OUTPUT_ENCODING) {
int tmp;
switch(INPUT_ENCODING) {
  case UTF16_LE: tmp=u_fgetc(input); 
                 if (tmp!=U_BYTE_ORDER_MARK) {
                    return INPUT_FILE_NOT_IN_UTF16_LE;
                 }
                 break;
  case UTF16_BE: tmp=u_fgetc_big_endian(input); 
                 if (tmp!=U_BYTE_ORDER_MARK) {
                    return INPUT_FILE_NOT_IN_UTF16_BE;
                 }
                 break;
  case UTF8: /*return UNSUPPORTED_INPUT_ENCODING;*/ break;
  default: break;
}
switch(OUTPUT_ENCODING) {
  case UTF16_LE: u_fputc((unichar)U_BYTE_ORDER_MARK,output); break;
  case UTF16_BE: u_fputc_big_endian((unichar)U_BYTE_ORDER_MARK,output); break;
  default: break;
}
int (*input_function)(FILE*);
int (*output_function)(unichar,FILE*);
switch (INPUT_ENCODING) {
  case ISO_XXX: input_function=read_iso_char; break;
  case UTF16_LE: input_function=u_fgetc_raw; break;
  case UTF16_BE: input_function=u_fgetc_raw_big_endian; break;
  case UTF8: input_function=read_utf8_diese_char; break;
#ifndef HGH_INSET
  case MBCS_KR: input_function=read_mbcs_char; break;
#endif // HGH_INSERT
  default: return UNSUPPORTED_INPUT_ENCODING; break;
}
switch (OUTPUT_ENCODING) {
  case ISO_XXX: output_function=write_iso_char; break;
  case UTF16_LE: output_function=u_fputc_raw; break;
  case UTF16_BE: output_function=u_fputc_raw_big_endian; break;
  case UTF8: output_function=u_fputc_utf8; break;
#ifndef HGH_INSET
  case MBCS_KR: output_function=write_mbcs_char; break;
#endif // HGH_INSERT
  default: return UNSUPPORTED_INPUT_ENCODING; break;
}
while ((tmp=input_function(input))!=EOF) {
   output_function((unichar)tmp,output);
}
return CONVERSION_OK;
}
