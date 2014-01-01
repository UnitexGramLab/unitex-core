/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef MetaSymbolsH
#define MetaSymbolsH


/**
 * This enumeration represents all the meta symbols that can be
 * found in pattern within a .fst2 grammar.
 */
enum meta_symbol {
   META_SHARP, // #: used to forbid spaces
   META_SPACE, // the space character
   META_EPSILON, // the empty word
   META_MOT, // <MOT>: used to match a token made of letters
   META_DIC, // <DIC>: used to match a simple or compound word present in the text dictionaries (dlf/dlc)
   META_SDIC, // <SDIC>: used to match a simple word present in the text dlf dictionary
   META_CDIC, // <CDIC>: used to match a compound word present in the text dlc dictionary
   META_TDIC, // <TDIC>: used to match a tag like {XXX,XXX.XXX}
   META_MAJ, // <MAJ>: used to match a token made of uppercase letters
   META_MIN, // <MIN>: used to match a token made of lowercase letters
   META_PRE, // <PRE>: used to match a token made of letters that starts with an uppercase one
   META_NB,  // <NB>: used to match a contiguous sequence of latin digits (0-9)
   META_TOKEN, // <TOKEN>: used to match any token that is not the space
   META_BEGIN_MORPHO, // $<: used to enter the morphological mode
   META_END_MORPHO,   // $>: used to leave the morphological mode
   META_LEFT_CONTEXT, // $*: used to specify the end of a left context
   META_TEXT_START,   // {^}: matches the text start
   META_TEXT_END      // {$}: matches the end of the text
};

#endif

