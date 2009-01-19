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

/*
 *	unimap.h
 *   ver 3.0
 *    2000.4.27
 */
#ifndef UNIMAP_H
#define UNIMAP_H

#define ASCII_Org_L		0x0000
#define ASCII_Org_H		0x00ff

#define ASCII_Ext_L		0x0100
#define ASCII_Ext_H		0x024F

#define Hangul_Jamo_L	0x1100
#define Hangul_Jamo_H	0x11FF

/* table of the zone of  the unicode map
*/
#define UNIZONE_basic_ratin					0x0000
#define UNIZONE_latin_supplement				0x0080
#define UNIZONE_latin_extended_A				0x0100
#define UNIZONE_latin_extended_B				0x0180
#define UNIZONE_IPA_Extensions				0x0250
#define UNIZONE_Spacing_Modifier_Letters			0x02B0
#define UNIZONE_Combining_Diacritical_Marks		0x0300
#define UNIZONE_Greek					0x0370
#define UNIZONE_Cyrillic					0x0400
#define UNIZONE_Armenian					0x0530
#define UNIZONE_Hebrew					0x0590
#define UNIZONE_Arabic					0x0600
#define UNIZONE_Syriac					0x0700
#define UNIZONE_Thaana					0x0780
#define UNIZONE_Devangari					0x0900
#define UNIZONE_Bengali					0x0980
#define UNIZONE_Gurmukhi					0x0A00
#define UNIZONE_Gujarati					0x0B00
#define UNIZONE_Tamil					0x0B80
#define UNIZONE_Telugu					0x0C00
#define UNIZONE_kannada					0x0C80
#define UNIZONE_Malayalam					0x0D00
#define UNIZONE_Sinhala					0x0D80
#define UNIZONE_Thai						0x0E00
#define UNIZONE_Lao						0x0E80
#define UNIZONE_Tibetan					0x0F00
#define UNIZONE_Myanmar					0x1000
#define UNIZONE_Georgian					0x10A0
#define UNIZONE_Hangul_Jamo				0x1100
#define UNIZONE_Ethiopic					0x1200
#define UNIZONE_Cherokee					0x13A0
#define UNIZONE_Unified_Canadian_Aboriginal_Syllabic	0x1400
#define UNIZONE_Ogham					0x1680
#define UNIZONE_Runic					0x16A0
#define UNIZONE_Khmer					0x1780
#define UNIZONE_Mongolian					0x1800
#define UNIZONE_Latin_Extended_Additional			0x1E00
#define UNIZONE_Greek_Extended				0x1F00
#define UNIZONE_General_Punctuation			0x2000
#define UNIZONE_Superscripts_and_Subscripts		0x2070
#define UNIZONE_Currency_Symbols				0x20A0
#define UNIZONE_Combining_Marks_for_Symbols		0x20D0
#define UNIZONE_Letterlike_Symbols				0x2100
#define UNIZONE_Number_Forms				0x2150
#define UNIZONE_Arrows					0x2190
#define UNIZONE_Mathmatical_Operators			0x2200
#define UNIZONE_Miscellaneous_Technical			0x2300
#define UNIZONE_Control_Pictures				0x2400
#define UNIZONE_Optical_Character_Recongnition		0x2440
#define UNIZONE_Enclosed_Alphanumerics			0x2460
#define UNIZONE_Box_Drawing				0x2500
#define UNIZONE_Block_Elements				0x2580
#define UNIZONE_Geometric_Shapes				0x25A0
#define UNIZONE_Miscellaneus_Symbols			0x2600
#define UNIZONE_Dingbats					0x2700
#define UNIZONE_Braille_Patterns				0x2800
#define UNIZONE_CJK_Radicals_Supplement			0x2E80
#define UNIZONE_Kangxi_Radicals				0x2F00
#define UNIZONE_Ideographic_Description_Characters	0x2FF0
#define UNIZONE_CJK_Symbols_and_Punctuation		0x3000
#define UNIZONE_Hiragana					0x3040
#define UNIZONE_Katakana					0x30A0
#define UNIZONE_Bopomofo					0x3100
#define UNIZONE_Hangul_Compatiblity_Jamo			0x3130
#define UNIZONE_Kanbun					0x3190
#define UNIZONE_Bopomofo_Extended			0x31A0
#define UNIZONE_Enclosed_CJK_Letters_and_Months	0x3200
#define UNIZONE_CJK_Compatibility				0x3300
#define UNIZONE_CJK_Unified_Ideographs_Externsion_A	0x3400
#define UNIZONE_CJK_Unified_Ideographs			0x4E00
#define UNIZONE_Yi_Syllables					0xA000
#define UNIZONE_Yi_Radicals					0xA490
#define UNIZONE_Hangul_Syllables				0xAC00
#define UNIZONE_High_Surrogates				0xD800
#define UNIZONE_High_Private_Use_Surrogates		0xDB80
#define UNIZONE_Low_Surrogates				0xDC00
#define UNIZONE_Private_Use					0xE000
#define UNIZONE_CJK_Compatibility_Ideographs		0xF900
#define UNIZONE_Alphabet_Presentation_Forms		0xFB00
#define UNIZONE_Alphabet_Presentation_Forms_A		0xFB50
#define UNIZONE_Combining_Half_Marks_			0xFE20
#define UNIZONE_CJK_Compatibility_Forms			0xFE30
#define UNIZONE_Small_Form_Variants			0xFE50
#define UNIZONE_Arabic_Presentation_Forms_B		0xFE70
#define UNIZONE_Specials0					0xFEFF
#define UNIZONE_Halfwidth_and_Fullwidth_Forms		0xFF00
#define UNIZONE_Specials1					0xFFF0

#define END_Hangul_Jamo					0x11ff
#define END_Hangul_Compatiblity_Jamo			0x318F
#define END_Hangul_Syllables					0xD7A3
#endif

