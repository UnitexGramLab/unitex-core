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
//	
//	segtype.h
//	read the file that has the list of seperateur on the form text
//
//
//
//	tables for classifier the type of character
//
//	unsigned char charMapBelow256[];			// U+0000 ~ U+024F
//												// U+FF00 ~ U+FFEF
// unsigned char spacingModifierTable[];		// U+02B0 ~ U+02FF
//  unsigned char symbolsPunctuationTable[];	// U+3000 ~ U+303F
//
//	Default Table for Separateur
//	for pair

// 
// type define each charactere
//
#ifndef SEGTYPE_H
#define SEGTYPE_H

#define NUM_OF_GROUP_SEG	8


#define		 HAN_SYL       0
#define 	 HJA_SYL       1
#define 	 ASC_SYL       2
#define		 ESC_SYL       3	// extend latin or forign language
#define		 NUL_SYL	   4	// 
#define		 SYM_SYL       5	//
#define		 TPH_SYL       6	//
#define 	 NUM_SYL       7

#define SYM_CHAR	0
#define HAN_CHAR	1
#define ASC_CHAR	2
#define HJA_CHAR	4
#define NUL_CHAR	0x10
#define ESC_CHAR	0x20
#define TPH_CHAR	0x40
#define NUM_CHAR	0x80


#define		 NOTSTR_SYL		0x4	// mask indicate this character which
								// can be a element of string 

#define		 T00_SYL       0x08	// symbol for the separater : space, tab
#define		 T01_SYL       0x10	// symbol for the separater : line feed,  form feed
#define		 T10_SYL       0x18	// symbol for the delimiter : 
#define		 T20_SYL       0x20 // symbol for the start of the citation with pairs
#define		 T21_SYL       0x28	// symbol for the end of the citation with pairs
#define		 T30_SYL	   0x30 // symbol for use for the citation without pairs

#define TPS_SYL		(TPH_SYL|T20_SYL)
#define TPE_SYL		(TPH_SYL|T21_SYL)
#define TPM_SYL		(TPH_SYL|T30_SYL)
#define TPT_SYL		(TPH_SYL|T10_SYL)

#define SPA_SYL		(NUL_SYL|T00_SYL)

#define RET_SYL		(NUL_SYL|T01_SYL)
#define FRF_SYL		(NUL_SYL|T01_SYL)
#define LIF_SYL		(NUL_SYL|T01_SYL)

#define TYPE_MASK			0x07
#define TYPE_MASK_DETAIL	0x38

#define END_MARK_OFFSET		0xffffffff
	/// if you want to change the map for handling the class of
	//	segmentation change the map	at file "segtype.cpp"


#define sizeOfSeg sizeof(struct indexOfSegment)
typedef struct indexOfSegment SegIndex;

#define MASK_OFFS_STR		0x40000000
#define MASK_OFFS_SINDEX	0x0fffffff
#define MASK_CONN_SINDEX	0x80000000
#define MASK_TYPE_SINDEX	0x70000000

#define MAX_SEG_COUNT		0x10000000	// 28 bit 256*M

#define EX_TYPE_SINDEX(x)		( ( (x) >> 28 ) & 0x07)
#define EX_GROUP_SINDEX(x)		( ( (x) >> 28 ) & 0x07)
#define IN_TYPE_SINEX(x,y)		( (((x)<< 28) & MASK_TYPE_SINDEX ) | (y))
#define TYPE_SET_SINDEX(x)		( ((x) << 28) & MASK_TYPE_SINDEX )

#define DEF_CONN_SPA		1

struct table_of_separator {
	unsigned char type;
	unsigned char index;	// if this sepeateur is this  variable, it use for indicater of aneother pair
	unsigned short value;
	unsigned int LSindex;
	unsigned int LEend;
	unsigned int lastPair;	// if this seperateur use with the form pair							// this variable indicate last pair indicate
};
struct indexOfSegment {
	unsigned type:4;	// the symbol which occure the separation
	unsigned Index:28;
};
struct offsetString {
	unsigned follow:1;
	unsigned type:3;
	unsigned offset:28;
};

#define UNI_SP	0x20

//#pragma pack(push, temp_pragma_save)
//#pragma pack(push(temp_pragma_save))
//#pragma pack(1)

#ifdef _MSC_VER
# pragma pack( push, packing )
# pragma pack( 1 )
# define PACK_STRUCT_ST
#else
#if defined( __GNUC__ )
# define PACK_STRUCT_ST __attribute__((packed))
#else
# error you must byte-align these structures with the appropriate compiler directives
#endif
#endif

struct tmplettre {
	unsigned type:4;
	unsigned index:28;
	unsigned int Offset;
} PACK_STRUCT_ST;
struct tmpSymbole
{
	unsigned type:4;
	unsigned vide:12;
	unsigned symNum:16;
	unsigned int Offset;
} PACK_STRUCT_ST;
struct tokenTemplate {
	union {
		struct tmplettre txt;
		struct tmpSymbole sym;
	} a;
} PACK_STRUCT_ST;
//#pragma pack(pop,temp_pragma_save)
//#pragma pack(pop(temp_pragma_save))
#ifdef _MSC_VER
# pragma pack( pop, packing )
#endif

class wideCharTable {
public:
	static unsigned char charMapBelow256[256];
	static unsigned char spacingModifierTable[0x50];
	static unsigned char symbolsPunctuationTable[0x40];
	static unsigned char unicode_bankmap[256];
	static unsigned char HalfwidthFllwidthForms[256];

	wideCharTable();
	~wideCharTable();
	int check_range_character(unichar compareValue);
};

#endif
