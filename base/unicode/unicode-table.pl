#!/usr/bin/env perl

# Copyright (c) 2016 Cristian Martinez
# https://github.com/UnitexGramLab/unitex-core/tree/master/base/unicode
#
# Based on https://github.com/detomon/unicode-table (v0.3.2)
# https://github.com/detomon/unicode-table/blob/773e2d13/src/generate.pl
# Copyright (c) 2016 Simon Schoenenberger
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

# Changes introduced for the Unitex Unicode Library
# Script:
# - set tableSize to 0x10000, Unitex only handles code-points lower than 0xFFFF
# - set prefix to 'u'
# - set makeSnakeCase to 1
# - fix: avoid to set the flag letter for Nd, Nl or No catagories
# - fix:"my $title  = hex ($line [14])" if this field is null, then the
#   Simple_Titlecase_Mapping is the same as the Uppercase_Mapping
# - new variable 'maxValue' set to tableSize-1, used on table.h
# - add a new header info to reflect the introduced changes
# - set unicodeVersion to '9.0.0'
# - set outName to 'table'
# - add 'charType' variable
# - add 'charSize' variable
# - add variantsSize variable
# - print 2 spaces instead of a tabulation
# - fix 'Use of uninitialized value' calculating the hex value of upper, lower, title
# - new Unicode Blocks handling (using data from Blocks.txt)
# - new Unicode Scripts handling (using data from Scripts.txt)
# - new Unicode Decomposition Mappings (using data from UnicodeData.txt)
# - new Unicode CaseFolding (using data from CaseFolding.txt)
# - rename 'Casing' by 'Variants'
# - rename 'SpecialCases' 'SpecialVariants'
# - rename 'Glyph' by 'charName'
# - new Unnacent variant (removing the diacritical marks from canonical mappings)
# - update command line parsing
# - new Asciify variant (using a new table avalaible at variants/Asciify.txt)
# - add 'moAsciifyExpandsGlyphInfo' flag
# - new Unicode Bidi classes (using data from UnicodeData.txt)
# - new '{k:Foo}' constant style to create const 'kFoo' variables
# - new cache char sequences on makeCharSequence
# - new add Canonical_Combining_Class Values  (using data from UnicodeData.txt)
# - add 'moIdentifierGlyphInfo' flag (a-z,A-z,_)
# - update specialChars: now White_Space is on $extraProperty (computed using PropList.txt)
# - add 'moIgnorableGlyphInfo' flag (computed using data from DerivedCoreProperties.txt)
# Templates:
# - replace #pragma once by macro guards
# - replace #include 'stdint.h' by 'base/integer/types.h'
# - replace #include 'foo.h' by 'relative/path/foo.h'
# - rename 'u_lookup_glyph' by 'u_lookup'
# - rename 'case' by 'variant'
# - remove full stop from the end of comments
# - remove 'u_category_names' array
# - change 'char const*' by 'const char*'
# - change 'foo const ' by 'const foo'
# - change 'inline' by UNITEX_FORCE_INLINE
# - replace static function by an unnamed namespace
# - change 'uint32_t flags' by 'uint16_t flags'
# - change 'uint32_t category' by 'uint8_t category'
# - add unitex namespace
# - new 'unichar_info' macro
# - new 'UNITEX_UNICODE' and 'UNITEX_UNICODE_AT_LEAST' macros
# - new u_block, u_script, u_bidi structs
# - new add doxygen documentation
# =============================================================================
use strict;                               # Pragma to restrict unsafe constructs
use warnings;                             # Pragma to control optional warnings
# =============================================================================
use utf8;                                 # Enables to type utf8 in program code
use open IN  => ':encoding(utf8)';        # Input stream utf8
use open OUT => ':encoding(utf8)';        # Output stream utf8
use open        ':std';                   # STDIN, STDOUT, STDERR utf8
# =============================================================================
use Data::Dumper;                         # Stringified perl data structures
# =============================================================================
use POSIX qw(strftime);                   # strftime with GNU extensions
# =============================================================================

#-------------------------------------------------------------------------------
#
# Unicode definitions
#
#-------------------------------------------------------------------------------
use constant unicodeMajor   => 9;
use constant unicodeMinor   => 0;
use constant unicodePatch   => 0;
use constant unicodeVersion => sprintf("%d.%d.%d",unicodeMajor,unicodeMinor,unicodePatch);

# 0x110000 (10FFFF+1) Full Unicode support
#  0x10000 (  FFFF+1) Unitex support
use constant tableSize      => 0x10000;

use constant moLetterGlyphInfo         => 1 << 0;
use constant moUppercaseGlyphInfo      => 1 << 1;
use constant moLowercaseGlyphInfo      => 1 << 2;
use constant moTitlecaseGlyphInfo      => 1 << 3;
use constant moSpaceGlyphInfo          => 1 << 4;
use constant moLinebreakGlyphInfo      => 1 << 5;
use constant moPunctuationGlyphInfo    => 1 << 6;
use constant moDigitGlyphInfo          => 1 << 7;
# use constant moHexDigitGlyphInfo    => 1 << 8;
use constant moNumberGlyphInfo         => 1 << 8;
use constant moFractionGlyphInfo       => 1 << 9;
use constant moControlGlyphInfo        => 1 << 10;
use constant moSymbolGlyphInfo         => 1 << 11;
use constant moIdentifierGlyphInfo     => 1 << 12;
use constant moIgnorableGlyphInfo      => 1 << 13;
use constant moOtherGlyphInfo          => 1 << 14;
use constant moMapExpandsGlyphInfo     => 1 << 15;
use constant moUpperExpandsGlyphInfo   => 1 << 16;
use constant moLowerExpandsGlyphInfo   => 1 << 17;
use constant moTitleExpandsGlyphInfo   => 1 << 18;
use constant moFoldExpandsGlyphInfo    => 1 << 19;
use constant moAsciifyExpandsGlyphInfo => 1 << 20;


use constant caseUpper   => 0;
use constant caseLower   => 1;
use constant caseTitle   => 2;
use constant caseFold    => 3;
use constant caseUnacent => 5;
use constant caseAsciify => 4;

my %flagIndexes = (
 'Letter'         => moLetterGlyphInfo,
 'Uppercase'      => moUppercaseGlyphInfo,
 'Lowercase'      => moLowercaseGlyphInfo,
 'Titlecase'      => moTitlecaseGlyphInfo,
 'Space'          => moSpaceGlyphInfo,
 'Linebreak'      => moLinebreakGlyphInfo,
 'Punctuation'    => moPunctuationGlyphInfo,
 'Digit'          => moDigitGlyphInfo,
 'Number'         => moNumberGlyphInfo,
 'Fraction'       => moFractionGlyphInfo,
 'Control'        => moControlGlyphInfo,
 'Symbol'         => moSymbolGlyphInfo,
 'Identifier'     => moIdentifierGlyphInfo,
 'Ignorable'      => moIgnorableGlyphInfo,
 'Other'          => moOtherGlyphInfo,
 'MapExpands'     => moMapExpandsGlyphInfo,
 'UpperExpands'   => moUpperExpandsGlyphInfo,
 'LowerExpands'   => moLowerExpandsGlyphInfo,
 'TitleExpands'   => moTitleExpandsGlyphInfo,
 'FoldExpands'    => moFoldExpandsGlyphInfo,
 'AsciifyExpands' => moAsciifyExpandsGlyphInfo,
);

my %flagInUse = (
 'Letter'         => 1,
 'Uppercase'      => 1,
 'Lowercase'      => 1,
 'Titlecase'      => 1,
 'Space'          => 1,
 'Linebreak'      => 1,
 'Punctuation'    => 1,
 'Digit'          => 1,
 'Number'         => 1,
 'Fraction'       => 1,
 'Control'        => 1,
 'Symbol'         => 1,
 'Identifier'     => 1,
 'Ignorable'      => 1,
 'Other'          => 1,
 'MapExpands'     => 0,
 'UpperExpands'   => 0,
 'LowerExpands'   => 0,
 'TitleExpands'   => 0,
 'FoldExpands'    => 0,
 'AsciifyExpands' => 0,
);

my %flagDescription = (
 'Letter'         => 'Lu, Ll, Lt, Lm, Lo',
 'Uppercase'      => 'Lu',
 'Lowercase'      => 'Ll',
 'Titlecase'      => 'Lt',
 'Space'          => 'Zs, Zl, Zp',
 'Linebreak'      => 'Zl, Zp',
 'Punctuation'    => 'Pc, Pd, Ps, Pe, Pi, Pf, Po',
 'Digit'          => 'Nd',
 'Number'         => 'Nd, Nl, No',
 'Fraction'       => '-',
 'Control'        => 'Cc',
 'Symbol'         => 'Sm, Sc, Sk, So',
 'Identifier'     => '0-9, A-Z, a-z, _',
 'Ignorable'      => 'character should be ignored in processing',
 'Other'          => 'Mn, Mc, Me, Cf, Cs, Co, Cn',
 'MapExpands'     => 'decomposition mapping consist of 4 or more code points',
 'UpperExpands'   => 'uppercase expands to multiple characters',
 'LowerExpands'   => 'lowercase expands to multiple characters',
 'TitleExpands'   => 'titlecase expands to multiple characters',
 'FoldExpands'    => 'foldcase expands to multiple characters',
 'AsciifyExpands' => 'asciify expands to multiple characters',
);

my %categoryFlags = (
	''   => 0,
	'Lu' => moLetterGlyphInfo | moUppercaseGlyphInfo,
	'Ll' => moLetterGlyphInfo | moLowercaseGlyphInfo,
	'Lt' => moLetterGlyphInfo | moTitlecaseGlyphInfo,
	'Lm' => moLetterGlyphInfo,
	'Lo' => moLetterGlyphInfo,
	'Mn' => moOtherGlyphInfo,
	'Mc' => moOtherGlyphInfo,
	'Me' => moOtherGlyphInfo,
	'Nd' => moDigitGlyphInfo | moNumberGlyphInfo,
	'Nl' => moNumberGlyphInfo,
	'No' => moNumberGlyphInfo,
	'Pc' => moPunctuationGlyphInfo,
	'Pd' => moPunctuationGlyphInfo,
	'Ps' => moPunctuationGlyphInfo,
	'Pe' => moPunctuationGlyphInfo,
	'Pi' => moPunctuationGlyphInfo,
	'Pf' => moPunctuationGlyphInfo,
	'Po' => moPunctuationGlyphInfo,
	'Sm' => moSymbolGlyphInfo,
	'Sc' => moSymbolGlyphInfo,
	'Sk' => moSymbolGlyphInfo,
	'So' => moSymbolGlyphInfo,
	'Zs' => moSpaceGlyphInfo,
	'Zl' => moSpaceGlyphInfo | moLinebreakGlyphInfo,
	'Zp' => moSpaceGlyphInfo | moLinebreakGlyphInfo,
	'Cc' => moControlGlyphInfo,
	'Cf' => moOtherGlyphInfo,
	'Cs' => moOtherGlyphInfo,
	'Co' => moOtherGlyphInfo,
	'Cn' => moOtherGlyphInfo,
);

my %extraPropertyFlags = (
	''   => 0,
	'Default_Ignorable_Code_Point' => moIgnorableGlyphInfo,
  'White_Space' => moSpaceGlyphInfo,
);

my %categoryIndexes = (
	''   => 0,
	'Lu' => 1,
	'Ll' => 2,
	'Lt' => 3,
	'Lm' => 4,
	'Lo' => 5,
	'Mn' => 6,
	'Mc' => 7,
	'Me' => 8,
	'Nd' => 9,
	'Nl' => 10,
	'No' => 11,
	'Pc' => 12,
	'Pd' => 13,
	'Ps' => 14,
	'Pe' => 15,
	'Pi' => 16,
	'Pf' => 17,
	'Po' => 18,
	'Sm' => 19,
	'Sc' => 20,
	'Sk' => 21,
	'So' => 22,
	'Zs' => 23,
	'Zl' => 24,
	'Zp' => 25,
	'Cc' => 26,
	'Cf' => 27,
	'Cs' => 28,
	'Co' => 29,
	'Cn' => 30,
);

my %categoryName = (
	'',  => 'CategoryInvalid',
	'Lu' => 'CategoryLetterUppercase',
	'Ll' => 'CategoryLetterLowercase',
	'Lt' => 'CategoryLetterTitlecase',
	'Lm' => 'CategoryLetterModifier',
	'Lo' => 'CategoryLetterOther',
	'Mn' => 'CategoryMarkNonspacing',
	'Mc' => 'CategoryMarkSpacingCombining',
	'Me' => 'CategoryMarkEnclosing',
	'Nd' => 'CategoryNumberDecimalDigit',
	'Nl' => 'CategoryNumberLetter',
	'No' => 'CategoryNumberOther',
	'Pc' => 'CategoryPunctuationConnector',
	'Pd' => 'CategoryPunctuationDash',
	'Ps' => 'CategoryPunctuationOpen',
	'Pe' => 'CategoryPunctuationClose',
	'Pi' => 'CategoryPunctuationInitialQuote',
	'Pf' => 'CategoryPunctuationFinalQuote',
	'Po' => 'CategoryPunctuationOther',
	'Sm' => 'CategorySymbolMath',
	'Sc' => 'CategorySymbolCurrency',
	'Sk' => 'CategorySymbolModifier',
	'So' => 'CategorySymbolOther',
	'Zs' => 'CategorySeparatorSpace',
	'Zl' => 'CategorySeparatorLine',
	'Zp' => 'CategorySeparatorParagraph',
	'Cc' => 'CategoryOtherControl',
	'Cf' => 'CategoryOtherFormat',
	'Cs' => 'CategoryOtherSurrogate',
	'Co' => 'CategoryOtherPrivateUse',
	'Cn' => 'CategoryOtherNotAssigned',
);

my %bidiIndexes = (
  ''    => 0,
  'L'   =>  1,
  'R'   =>  2,
  'AL'  =>  3,
  'EN'  =>  4,
  'ES'  =>  5,
  'ET'  =>  6,
  'AN'  =>  7,
  'CS'  =>  8,
  'NSM' =>  9,
  'BN'  =>  10,
  'B'   =>  11,
  'S'   =>  12,
  'WS'  =>  13,
  'ON'  =>  14,
  'LRE' =>  15,
  'LRO' =>  16,
  'RLE' =>  17,
  'RLO' =>  18,
  'PDF' =>  19,
  'LRI' =>  20,
  'RLI' =>  21,
  'FSI' =>  22,
  'PDI' =>  23,
);

my %bidiName = (
  ''    =>  'BidiInvalid',
  'L'   =>  'BidiLeftToRight',
  'R'   =>  'BidiRightToLeft',
  'AL'  =>  'BidiArabicLetter',
  'EN'  =>  'BidiEuropeanNumber',
  'ES'  =>  'BidiEuropeanSeparator',
  'ET'  =>  'BidiEuropeanTerminator',
  'AN'  =>  'BidiArabicNumber',
  'CS'  =>  'BidiCommonSeparator',
  'NSM' =>  'BidiNonspacingMark',
  'BN'  =>  'BidiBoundaryNeutral',
  'B'   =>  'BidiParagraphSeparator',
  'S'   =>  'BidiSegmentSeparator',
  'WS'  =>  'BidiWhiteSpace',
  'ON'  =>  'BidiOtherNeutral',
  'LRE' =>  'BidiLeftToRightEmbedding',
  'LRO' =>  'BidiLeftToRightOverride',
  'RLE' =>  'BidiRightToLeftEmbedding',
  'RLO' =>  'BidiRightToLeftOverride',
  'PDF' =>  'BidiPopDirectionalFormat',
  'LRI' =>  'BidiLeftToRightIsolate',
  'RLI' =>  'BidiRightToLeftIsolate',
  'FSI' =>  'BidiFirstStrongIsolate',
  'PDI' =>  'BidiPopDirectionalIsolate',
);

my %blockIndexes = (
  'None'        => [-1,-1],
);

my %scriptIndexes = (
  'Unknown'     => -1,
);

# NormalizationMask
my %normalizationMaskIndexes = (
  'None'          => 0,
  'Compatibility' => 1,
  'Composition'   => 2,
);

my %normalizationMaskDescription = (
  'None'          => 'None',
  'Compatibility' => 'Compatibility Decomposition',
  'Composition'   => 'Canonical Composition',
);

# NormalizationForm
my %normalizationFormIndexes = (
  'D'  => $normalizationMaskIndexes{'None'},
  'Kd' => $normalizationMaskIndexes{'Compatibility'},
  'C'  => $normalizationMaskIndexes{'Composition'},
  'Kc' => $normalizationMaskIndexes{'Composition'} + $normalizationMaskIndexes{'Compatibility'},
);

my %normalizationFormDescription = (
  'D'  => 'Normalization Form D',
  'Kd' => 'Normalization Form KD',
  'C'  => 'Normalization Form C',
  'Kc' => 'Normalization Form KC',
);

# NormalizationQuickCheck
my %normalizationQuickCheckIndexes = (
  'Yes'   => 0,
  'No'    => 1,
  'Maybe' => 2,
);

my %normalizationQuickCheckDescription = (
  'Yes'   => 'Characters that always occur in normalization forms',
  'No'    => 'Characters that cannot ever occur in a normalization form',
  'Maybe' => 'Characters that may occur in a normalization form',
);

my %normalizationFormQuickCheckFlagsIndexes = (
  'NFD_QC_N'  => $normalizationQuickCheckIndexes{'No'}     <<  (2 * $normalizationFormIndexes{'D'}),
  'NFKD_QC_N' => $normalizationQuickCheckIndexes{'No'}     <<  (2 * $normalizationFormIndexes{'Kd'}),
  'NFC_QC_N'  => $normalizationQuickCheckIndexes{'No'}     <<  (2 * $normalizationFormIndexes{'C'}),
  'NFC_QC_M'  => $normalizationQuickCheckIndexes{'Maybe'}  <<  (2 * $normalizationFormIndexes{'C'}),
  'NFKC_QC_N' => $normalizationQuickCheckIndexes{'No'}     <<  (2 * $normalizationFormIndexes{'Kc'}),
  'NFKC_QC_M' => $normalizationQuickCheckIndexes{'Maybe'}  <<  (2 * $normalizationFormIndexes{'Kc'}),
);

my %normalizationFormQuickCheckFlagsName = (
  'NFD_QC_N'  => 'NFD_QuickCheckNo',
  'NFKD_QC_N' => 'NFKD_QuickCheckNo',
  'NFC_QC_N'  => 'NFC_QuickCheckNo',
  'NFC_QC_M'  => 'NFC_QuickCheckMaybe',
  'NFKC_QC_N' => 'NFKC_QuickCheckNo',
  'NFKC_QC_M' => 'NFKC_QuickCheckMaybe',
);

my %normalizationFormQuickCheckFlagsDescription = (
  'NFD_QC_N'  => 'NFD_QC_N  = No    << 2 * D',
  'NFKD_QC_N' => 'NFKD_QC_N = No    << 2 * KD',
  'NFC_QC_N'  => 'NFC_QC_N  = No    << 2 * C',
  'NFC_QC_M'  => 'NFC_QC_M  = Maybe << 2 * C',
  'NFKC_QC_N' => 'NFKC_QC_N = No    << 2 * KC',
  'NFKC_QC_M' => 'NFKC_QC_M = Maybe << 2 * KC',
);

my %decompositionCccIndexes = (
  'DecompositionCccNotReordered'       =>  0,
  'DecompositionCccOverlay'            =>  1,
  'DecompositionCccNukta'              =>  7,
  'DecompositionCccKanaVoicing'        =>  8,
  'DecompositionCccVirama'             =>  9,
  'DecompositionCccAttachedBelow'      =>  202,
  'DecompositionCccAttachedAbove'      =>  214,
  'DecompositionCccAttachedAboveRight' =>  216,
  'DecompositionCccBelowLeft'          =>  218,
  'DecompositionCccBelow'              =>  220,
  'DecompositionCccBelowRight'         =>  222,
  'DecompositionCccLeft'               =>  224,
  'DecompositionCccRight'              =>  226,
  'DecompositionCccAboveLeft'          =>  228,
  'DecompositionCccAbove'              =>  230,
  'DecompositionCccAboveRight'         =>  232,
  'DecompositionCccDoubleBelow'        =>  233,
  'DecompositionCccDoubleAbove'        =>  234,
  'DecompositionCccIotaSubscript'      =>  240,
);

my %decompositionCccName = (
  0    =>  'DecompositionCccNotReordered',
  1    =>  'DecompositionCccOverlay',
  7    =>  'DecompositionCccNukta',
  8    =>  'DecompositionCccKanaVoicing',
  9    =>  'DecompositionCccVirama',
  202  =>  'DecompositionCccAttachedBelow',
  214  =>  'DecompositionCccAttachedAbove',
  216  =>  'DecompositionCccAttachedAboveRight',
  218  =>  'DecompositionCccBelowLeft',
  220  =>  'DecompositionCccBelow',
  222  =>  'DecompositionCccBelowRight',
  224  =>  'DecompositionCccLeft',
  226  =>  'DecompositionCccRight',
  228  =>  'DecompositionCccAboveLeft',
  230  =>  'DecompositionCccAbove',
  232  =>  'DecompositionCccAboveRight',
  233  =>  'DecompositionCccDoubleBelow',
  234  =>  'DecompositionCccDoubleAbove',
  240  =>  'DecompositionCccIotaSubscript',
);

my %decompositionCccDescription = (
  0    =>  "Spacing and enclosing marks; also many vowel and consonant signs, even if nonspacing",
  1    =>  "Marks which overlay a base letter or symbol",
  7    =>  "Diacritic nukta marks in Brahmi-derived scripts",
  8    =>  "Hiragana/Katakana voicing marks",
  9    =>  "Viramas",
  202  =>  "Marks attached directly below",
  214  =>  "Marks attached directly above",
  216  =>  "Marks attached at the top right",
  218  =>  "Distinct marks at the bottom left",
  220  =>  "Distinct marks directly below",
  222  =>  "Distinct marks at the bottom right",
  224  =>  "Distinct marks to the left",
  226  =>  "Distinct marks to the right",
  228  =>  "Distinct marks at the top left",
  230  =>  "Distinct marks directly above",
  232  =>  "Distinct marks at the top right",
  233  =>  "Distinct marks subtending two bases",
  234  =>  "Distinct marks extending above two bases",
  240  =>  "Greek iota subscript only",
);

my %decompositionTagIndexes = (
  ''            =>  0,
  '<default>'   =>  1,
  '<canonical>' =>  2,
  '<font>'      =>  3,
  '<noBreak>'   =>  4,
  '<initial>'   =>  5,
  '<medial>'    =>  6,
  '<final>'     =>  7,
  '<isolated>'  =>  8,
  '<circle>'    =>  9,
  '<super>'     =>  10,
  '<sub>'       =>  11,
  '<vertical>'  =>  12,
  '<wide>'      =>  13,
  '<narrow>'    =>  14,
  '<small>'     =>  15,
  '<square>'    =>  16,
  '<fraction>'  =>  17,
  '<compat>'    =>  18,
);

my %decompositionTagName = (
  ''            =>  "DecompositionTagInvalid",
  '<default>'   =>  "DecompositionTagDefault",
  '<canonical>' =>  "DecompositionTagCanonical",
  '<font>'      =>  "DecompositionTagFont",
  '<noBreak>'   =>  "DecompositionTagNoBreak",
  '<initial>'   =>  "DecompositionTagInitial",
  '<medial>'    =>  "DecompositionTagMedial",
  '<final>'     =>  "DecompositionTagFinal",
  '<isolated>'  =>  "DecompositionTagIsolated",
  '<circle>'    =>  "DecompositionTagCircle",
  '<super>'     =>  "DecompositionTagSuper",
  '<sub>'       =>  "DecompositionTagSub",
  '<vertical>'  =>  "DecompositionTagVertical",
  '<wide>'      =>  "DecompositionTagWide",
  '<narrow>'    =>  "DecompositionTagNarrow",
  '<small>'     =>  "DecompositionTagSmall",
  '<square>'    =>  "DecompositionTagSquare",
  '<fraction>'  =>  "DecompositionTagFraction",
  '<compat>'    =>  "DecompositionTagCompat",
);

my %decompositionTagDescription = (
  ''            =>  "Invalid",
  '<default>'   =>  "The code point of the character itself",
  '<canonical>' =>  "Mapping is canonical",
  '<font>'      =>  "Font variant (for example, a blackletter form)",
  '<noBreak>'   =>  "No-break version of a space or hyphen",
  '<initial>'   =>  "Initial presentation form (Arabic)",
  '<medial>'    =>  "Medial presentation form (Arabic)",
  '<final>'     =>  "Final presentation form (Arabic)",
  '<isolated>'  =>  "Isolated presentation form (Arabic)",
  '<circle>'    =>  "Encircled form",
  '<super>'     =>  "Superscript form",
  '<sub>'       =>  "Subscript form",
  '<vertical>'  =>  "Vertical layout presentation form",
  '<wide>'      =>  "Wide (or zenkaku) compatibility character",
  '<narrow>'    =>  "Narrow (or hankaku) compatibility character",
  '<small>'     =>  "Small variant form (CNS compatibility)",
  '<square>'    =>  "CJK squared font variant",
  '<fraction>'  =>  "Vulgar fraction form",
  '<compat>'    =>  "Otherwise unspecified compatibility character",
);

my $outName       = 'table';
my $hdrFile       = "$outName.h";
my $hdrFileIn     = "$outName.h.in";
my $srcFile       = "$outName.cpp";
my $srcFileIn     = "$outName.cpp.in";

my $normName      = "normalization";
my $hdrNormFile   = "$normName.h";
my $hdrNormFileIn = "$normName.h.in";
my $srcNormFile   = "$normName.cpp";
my $srcNormFileIn = "$normName.cpp.in";

my $testFileIn    = "test.h.in";
my $testFile      = "test.h";

my $maxValue      = sprintf("0x%02X", tableSize-1);
my $maxHexValue   = hex($maxValue);

my $charName      = "unichar";

my $args = join ' ', @ARGV;
my $prefix = 'u';
my $prefix_k = 'k';
my $char = 'c';
my $makeSnakeCase = 1;
my $useCategories = 0;
my %useCategories = ();
my $useBlocks = 0;
my %useBlocks = ();
my $useScripts = 0;
my %useScripts = ();
my $useBidiClasses = 0;
my %useBidiClasses = ();
my $useDecomposition = 0;
my %useDecomposition = ();
my $useNormalization = 0;
my %useNormalization = ();
# flags,blocks,categories,scripts,bidiClasses,decomposition,normalization,variants,variant_unaccent,variant_asciify
my $includeInfos = 'flags,categories,scripts,bidiClasses,variants,variant_unaccent';
my %includeInfos = ();
my %conditionalFlags = ();
my %specialVariantsCache = ();
my %specialMappingsCache = ();
my $variantsSize = 0;

if ($args =~ /--symbol-prefix=([\w_]+)/) {
	$prefix = $1;
}

if ($args =~ /--snake-case=([\w_]+)/) {
	$makeSnakeCase = int $1;
}

if ($args =~ /--blocks=([\w_,]+)/) {
  foreach (split /,/, $1) {
    $useBlocks {$_} = 1;
  }

  $useBlocks = 1;
}

if ($args =~ /--categories=([\w_,]+)/) {
	foreach (split /,/, $1) {
		$useCategories {$_} = 1;
	}

	$useCategories = 1;
}

if ($args =~ /--scripts=([\w_,]+)/) {
  foreach (split /,/, $1) {
    $useScripts {$_} = 1;
  }

  $useScripts = 1;
}

if ($args =~ /--bidiClasses=([\w_,]+)/) {
  foreach (split /,/, $1) {
    $useBidiClasses {$_} = 1;
  }

  $useBidiClasses = 1;
}

if ($args =~ /--decomposition=([\w_,]+)/) {
  if ($args =~ /--normalization=([\w_,]+)/) {
    foreach (split /,/, $1) {
      $useNormalization {$_} = 1;
    }
    $useNormalization = 1;
  }

  foreach (split /,/, $1) {
    $useDecomposition {'<' . $_ . '>'} =  1;
  }

  $useDecomposition = 1;
}

if ($args =~ /--include-info=([\w_,]+)/) {
	$includeInfos = $1;
}

# full format: '{%1$5d,%2$3d,%3$4d,%4$4d,%5$4d,{%6$4d,%7$4d,%8$4d,{%9$6d,%10$6d,%11$6d}},{%12$6d,%13$6d,%14$6d,%15$6d,%16$6d,%17$6d},{%18$s}},'
my $infoFormat = '';
my %infoFormat = ();
my @infoFormat = ();

foreach (split /,/, $includeInfos) {
	$infoFormat {$_} = 1;
}

if (exists $infoFormat {'flags'}) {
	push @infoFormat, '%1$6d';
	$conditionalFlags {'addFlags'} = 1;
}

if (exists $infoFormat {'blocks'}) {
  push @infoFormat, '%2$4d';
  $conditionalFlags {'addBlocks'} = 1;
}

if (exists $infoFormat {'categories'}) {
	push @infoFormat, '%3$3d';
	$conditionalFlags {'addCategories'} = 1;
}

if (exists $infoFormat {'scripts'}) {
  push @infoFormat, '%4$3d';
  $conditionalFlags {'addScripts'} = 1;
}

if (exists $infoFormat {'bidiClasses'}) {
  push @infoFormat, '%5$3d';
  $conditionalFlags {'addBidiClasses'} = 1;
}

if (exists $infoFormat {'decomposition'}) {
  my $decomposition_format = "";

  if (exists $infoFormat {'normalization'}) {
    $decomposition_format .= '%6$4d,';
    $conditionalFlags {'addNormalization'} = 1;
  }

  $decomposition_format .= '%7$4d,%8$4d,{%9$7d,%10$6d,%11$6d}';

  $flagInUse{'MapExpands'} = 1;
  $conditionalFlags {'addDecomposition'} = 1;

  push @infoFormat, '{' . $decomposition_format . '}';
}

if (exists $infoFormat {'variants'}) {
  my $variants_format = '%12$6d,%13$6d,%14$6d,%15$6d';
  $variantsSize = 4;

  if(exists $infoFormat {'variant_unaccent'}) {
    $variants_format .= ',%16$6d';
    $conditionalFlags {'addVariantUnnacent'} = 1;
    $variantsSize++;
  }

  if(exists $infoFormat {'variant_asciify'}) {
    $variants_format .= ',%17$6d';
    $flagInUse{'AsciifyExpands'} = 1;
    $conditionalFlags {'addVariantAsciify'} = 1;
    $variantsSize++;
  }

  $flagInUse{'UpperExpands'} = 1;
  $flagInUse{'LowerExpands'} = 1;
  $flagInUse{'TitleExpands'} = 1;
  $flagInUse{'FoldExpands'}  = 1;
	$conditionalFlags {'addVariants'} = 1;
	push @infoFormat, '{' . $variants_format . '}';
}

if (exists $infoFormat {'numbers'}) {
	push @infoFormat, ' {%18$s}';
	$conditionalFlags {'addNumbers'} = 1;
}

$infoFormat = (join ',', @infoFormat);
$infoFormat =~ s/^\s+|\s+$//g;
$infoFormat = "{$infoFormat},";

#-------------------------------------------------------------------------------
#
# Check arguments
#
#-------------------------------------------------------------------------------

if (($#ARGV + 1) < 9) {
	print "usage $0 " . unicodeVersion . "/UnicodeData.txt "   .
	                    unicodeVersion . "/SpecialCasing.txt " .
	                    unicodeVersion . "/Blocks.txt "        .
	                    unicodeVersion . "/Scripts.txt "       .
                      unicodeVersion . "/CaseFolding.txt "   .
                      unicodeVersion . "/DerivedNormalizationProps.txt " .
                      unicodeVersion . "/DerivedCoreProperties.txt "     .
                      unicodeVersion . "/PropList.txt "                  .
	                    unicodeVersion . "/extra/Asciify.txt "             .
	                    "\n";
	exit 1;
}

#-------------------------------------------------------------------------------
#
# Prepare tables
#
#-------------------------------------------------------------------------------

my @data          = (0 .. (tableSize - 1));
my @block         = (0 .. (tableSize - 1));
my @script        = (0 .. (tableSize - 1));
my @casefold      = (0 .. (tableSize - 1));
my @asciify       = (0 .. (tableSize - 1));
my @extraProperty = (0 .. (tableSize - 1));
my @quickCheck    = (0 .. (tableSize - 1));
my %special       = ();
my %simple        = ();
my %types         = (sprintf ($infoFormat, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) => 0);
my @pages         = (0 .. (tableSize >> 8) - 1);
my %pageCache     = ();
my @specialVariants = ();
my @specialMappings = ();

for (my $i = 0; $i < tableSize; $i ++) {
	$data [$i] = 0;
	$block[$i] = 0;
	$script[$i] = 0;
 	$casefold[$i] = 0;
	$asciify[$i] = 0;
  $extraProperty[$i] = 0;
	$quickCheck[$i] = $normalizationQuickCheckIndexes{'Yes'};
}

for (my $i = 0; $i < tableSize >> 8; $i ++) {
	$pages [$i] = 0;
}

my @emptyPage = (0 .. 255);

for (my $i = 0; $i < 256; $i ++) {
	$emptyPage [$i] = 0;
}

$pageCache {join ',', @emptyPage} = 0;

#-------------------------------------------------------------------------------
#
# Functions
#
#-------------------------------------------------------------------------------

sub toCamelCase {
	my $var = shift;
	my $prefix = shift;

	$var = "$prefix$var" if ($prefix);

	$var =~ s/_([a-z])/uc($1)/ge;
	$var =~ s/^(\w)/lc($1)/ge unless ($prefix);

	return $var;
}

sub toSnakeCase {
	my $var = shift;
	my $prefix = shift;

	$var = "$prefix"."_$var" if ($prefix);

	$var =~ s/([a-z])([A-Z])/"$1_$2"/ge;
	$var = lc $var;

	return $var;
}

sub toUserCase {
	my $var = shift;

	if ($makeSnakeCase) {
		return toSnakeCase $var, $prefix;
	}
	else {
		return toCamelCase $var, $prefix;
	}
}

sub toConstant {
	my $var = shift;
	my $prefix = shift;

	$var = "$prefix"."_$var" if ($prefix);

	$var = uc (toSnakeCase $var);

	return $var;
}

sub toKConstant {
  my $var = shift;
  my $prefix = shift;
  my $constant = shift;

  $var = ucfirst("$prefix") . "$var" if ($prefix);

  $var = toCamelCase $var, $constant;

  return $var;
}

sub toComment {
  my $var = shift;
  $var =~ s/([a-z])([A-Z])/"$1 $2"/ge;
  $var =~ s/^[^ ]+ //g;
  return $var;
}

sub toUnderscoreConstant {
  my $var = shift;
  my $prefix = shift;

  $var =~ s/-/ /g;

  $var = "$prefix"."_$var" if ($prefix);

  $var = uc (toSnakeCase $var);
  $var =~ s/ /_/g;

  return $var;
}

#sub makeCharSequence {
#	my $codes = shift;
#	my $ignore_single = shift;
#	$ignore_single    = defined $ignore_single ? $ignore_single : 1;
#
#	$codes =~/\s*(.+)\s*/;
#
#  my $sequence = $1;
#  my $offset   = $#specialVariants + 1;
#
#  if (exists $specialVariantsCache{$sequence}) {
#    $offset = $specialVariantsCache{$sequence};
#  } else {
#  	my @sequence = split /\s+/, $sequence;
#
#  	# ignore single character
#  	return -1 if ($#sequence == 0 && $ignore_single);
#
#  	push @specialVariants, $#sequence + 1;
#  	push @specialVariants, hex ($_) foreach (@sequence);
#
#  	# cache char sequence
#  	$specialVariantsCache{$sequence} = $offset;
#  }
#
#	return $offset;
#}

sub makeSpecialMappingsSequence {
  my $codes = shift;

  $codes =~/\s*(.+)\s*/;

  my $sequence = $1;
  my $offset   = $#specialMappings + 1;

  if (exists $specialVariantsCache{$sequence}) {
    $offset = $specialVariantsCache{$sequence};
  } else {
    my @sequence = split /\s+/, $sequence;

    # ignore single character
    return -1 if ($#sequence == 0);

    push @specialMappings, $#sequence + 1;
    push @specialMappings, hex ($_) foreach (@sequence);

    # cache char sequence
    $specialVariantsCache{$sequence} = $offset;
  }

  return $offset;
}

sub makeSpecialVariantsSequence {
  my $codes  = shift;
  my $single = shift;
  my $code   = shift;

  $codes =~/\s*(.+)\s*/;

  my $sequence = $1 . ' ' . ($single ne 0 ? $single : $code);
  my $offset   = $#specialVariants + 1;

  if (exists $specialVariantsCache{$sequence}) {
    $offset = $specialVariantsCache{$sequence};
  } else {
    my @sequence = split /\s+/, $sequence;

    # ignore single character followed by single case
    return -1 if $#sequence == 1;

    push @specialVariants, $#sequence;
    push @specialVariants, hex ($_) foreach (@sequence);

    # cache char sequence
    $specialVariantsCache{$sequence} = $offset;
  }

  return $offset;
}

#-------------------------------------------------------------------------------
#
# Read UnicodeData.txt and save uppercase, lowercase and titlecase
# simple hex mappings
#
#-------------------------------------------------------------------------------

open DATA, "<$ARGV[0]" or die "File '$ARGV[0]' not found";

while (<DATA>) {
  chomp;

  my @line = split /;/, $_;
  my $code = hex ($line [0]);

  # ignore code points beyond $maxHexValue
  next if ($code > $maxHexValue);

  my $upper = defined($line [12]) && length($line [12]) ? $line [12] : 0;
  my $lower = defined($line [13]) && length($line [13]) ? $line [13] : 0;
  my $title = defined($line [14]) && length($line [14]) ? $line [14] : $upper;

  my @simple_cases = (-1) x 5;

  $simple_cases[caseUpper]   = $upper;
  $simple_cases[caseLower]   = $lower;
  $simple_cases[caseTitle]   = $title;
  $simple_cases[caseFold]    = 0;
  $simple_cases[caseAsciify] = 0;

  @{$simple {$code}} = @simple_cases;

}

close DATA;

#-------------------------------------------------------------------------------
#
# Read CaseFolding and save only simple hex mappings (C+S)
#
#-------------------------------------------------------------------------------
if(exists $conditionalFlags {'addVariants'} &&
  $conditionalFlags {'addVariants'} == 1) {
  open CASEFOLDING, "<$ARGV[4]" or die "File '$ARGV[4]' not found";

  while (<CASEFOLDING>) {
    chomp;

    # ignore empty lines and comments
    next if ($_ =~ /^$|^#/);

    my @line  = split /;/, $_;

    my $code  = hex($line[0]);
    (my $type = $line[1]) =~ s/\s//g;

    # ignore code points beyond $maxHexValue
    next if ($code > $maxHexValue);

    # C: common case folding, common mappings shared by both simple and full mapping
    # S: simple case folding, mappings to single characters where different from F
    next if ($type ne 'C' && $type ne 'S');

    (my $fold = $line[2]) =~ s/^\s+|\s+$//g;

    # if the simple case already exists
    if (exists($simple {$code})) {
      # assign the simple sequence
      $simple {$code}[caseFold] = $fold;
    } else {
      # create a new simple case
      my @simple_cases = (-1) x 5;
      $simple_cases[caseFold] = $fold;
      @{$simple {$code}} = @simple_cases;
    }
  }

  close CASEFOLDING;
}

#-------------------------------------------------------------------------------
#
# Read special cases
#
#-------------------------------------------------------------------------------

open SPECIAL, "<$ARGV[1]" or die "File '$ARGV[1]' not found";

while (<SPECIAL>) {
	chomp;

	# ignore empty lines and comments
	next if ($_ =~ /^$|^#/);

	$_ =~ /(.+);\s*#/;

	my @line = split ';', $1;

	# ignore conditional case-folding
	next if ($line [4]);

	my $code  = hex ($line [0]);

  # ignore code points beyond $maxHexValue
  next if ($code > $maxHexValue);

  my $lower = makeSpecialVariantsSequence $line[1], $simple{$code}[caseLower], $line [0];
  my $title = makeSpecialVariantsSequence $line[2], $simple{$code}[caseTitle], $line [0];
  my $upper = makeSpecialVariantsSequence $line[3], $simple{$code}[caseUpper], $line [0];

  my @cases = (-1) x 5;

	$cases[caseUpper]   = $upper;
	$cases[caseLower]   = $lower;
	$cases[caseTitle]   = $title;
	$cases[caseFold]    = -1;
	$cases[caseAsciify] = -1;

	@{$special {$code}} = @cases;

	@line = split ';', $_;
}

close SPECIAL;

#-------------------------------------------------------------------------------
#
# Read CaseFolding
#
#-------------------------------------------------------------------------------
if(exists $conditionalFlags {'addVariants'} &&
  $conditionalFlags {'addVariants'} == 1) {
  open CASEFOLDING, "<$ARGV[4]" or die "File '$ARGV[4]' not found";

  while (<CASEFOLDING>) {
    chomp;

    # ignore empty lines and comments
    next if ($_ =~ /^$|^#/);

    my @line  = split /;/, $_;

    my $code  = hex($line[0]);
    (my $type = $line[1]) =~ s/\s//g;

    # ignore code points beyond $maxHexValue
    next if ($code > $maxHexValue);

    # we do a full case folding using the mappings with status C + F
    # the mappings with status T will be omitted
    next if ($type ne 'C' && $type ne 'F');

    (my $fold = $line[2]) =~ s/^\s+|\s+$//g;

    if ($type eq 'C') {
      # common case folding, common mappings shared by both simple
      # and full mappings
      $casefold[$code] = hex($fold);
    } elsif  ($type eq 'F') {
      # full case folding, mappings that cause strings to grow in
      # length. Multiple characters are separated by spaces
      my $special_offset   = makeSpecialVariantsSequence $fold, $simple{$code}[caseFold], $line[0];
      # if the special case already exists
      if (exists($special {$code})) {
        # assign the folded sequence as an special case of character pointed by code
        $special {$code}[caseFold] = $special_offset;
      } else {
        # create a new special case
        my @cases = (-1) x 5;
        $cases[caseFold] = $special_offset;
        @{$special {$code}} = @cases;
      }
    }
  }


  close CASEFOLDING;
}

#-------------------------------------------------------------------------------
#
# Read DerivedNormalizationProps
#
#-------------------------------------------------------------------------------
if(exists $conditionalFlags {'addNormalization'} &&
  $conditionalFlags {'addNormalization'} == 1) {
  open DERIVED_NORMALIZATION_PROPS, "<$ARGV[5]" or die "File '$ARGV[5]' not found";

  while (<DERIVED_NORMALIZATION_PROPS>) {
    chomp;

    # ignore empty lines and comments
    next if ($_ =~ /^$|^#/);

    my @line  = split /;/, $_;

    # ignore lines with lest than 3 fiels
    next if (scalar(@line)<3);

    (my $block = $line[0]) =~ s/\s//g;
    (my $form  = $line[1]) =~ s/\s//g;
    (my $check = $line[2]) =~ s/(\s|#[^#]*$)//g;

    # ignore lines with a second field different from N (No) and M (Maybe)
    next if ($check ne 'N' && $check ne 'M');

    my $key    = $form . "_" . $check;

    # ignore invalid keys
    next if (!exists($normalizationFormQuickCheckFlagsIndexes{$key}));

    # block range
    my @range = split /\.\./, $block;
    $range[0] = defined($range[0]) ? hex ($range[0]) : 0;
    $range[1] = defined($range[1]) ? hex ($range[1]) : $range[0];

    # ignore code points beyond $maxHexValue
    next if ($range[0] > $maxHexValue);

    # update the quick check flag for codepoints in the defined range
    for (my $i = $range[0]; $i <= $range[1]; $i ++) {
      $quickCheck[$i] |= $normalizationFormQuickCheckFlagsIndexes{$key};
    }
  }

  close DERIVED_NORMALIZATION_PROPS;
}

#-------------------------------------------------------------------------------
#
# Read Asciify
#
#-------------------------------------------------------------------------------
if(exists $conditionalFlags {'addVariantAsciify'} &&
  $conditionalFlags {'addVariantAsciify'} == 1) {
  open ASCIIFY, "<$ARGV[8]" or die "File '$ARGV[8]' not found";

  while (<ASCIIFY>) {
    chomp;

    # ignore empty lines and comments
    next if ($_ =~ /^$|^#/);

    my @line  = split /;/, $_;
    my @codes = split ' ', $line[1];

    my $code  = hex($line[0]);

    # ignore code points beyond $maxHexValue
    next if ($code > $maxHexValue);

    # if the character can be converted to other ascii character
    if(scalar(@codes) == 1) {
      $asciify[$code] = hex($codes[0]);
    } else {
      # if the character can be converted to more than one ascii characters
      my $asciify_sequence = join(' ',@codes);
      my $special_offset   = makeSpecialVariantsSequence $asciify_sequence, 0, $line [0];
      # if the special case already exists
      if (exists($special {$code})) {
        # assign the asciify_sequence as an special case of character pointed by code
        $special {$code}[caseAsciify] = $special_offset;
      } else {
        # create a new special case
        my @cases = (-1) x 5;
        $cases[caseAsciify] = $special_offset;
        @{$special {$code}} = @cases;
      }
    }
  }

  close ASCIIFY;
}

#-------------------------------------------------------------------------------
#
# Read Blocks
#
#-------------------------------------------------------------------------------
# From @see http://unicode.org/faq/blocks_ranges.html#4
# Can blocks overlap?
# A: No. Every Unicode block is discrete, and cannot overlap with any other block.
# Also, every assigned character in the Unicode Standard has to be in a block
# (and only one block, of course). This ensures that when code charts are printed,
# no characters are omitted simply because they aren't in a block.

open BLOCK, "<$ARGV[2]" or die "File '$ARGV[2]' not found";

my $block_number = 1;

while (<BLOCK>) {
  chomp;

  # ignore empty lines and comments
  next if ($_ =~ /^$|^#/);

  my @line = split /;/, $_;

  # block range
  my @range = split /\.\./, $line[0];
  $range[0] = defined($range[0]) ? hex ($range[0]) : 0;
  $range[1] = defined($range[1]) ? hex ($range[1]) : 0;

  # ignore code points beyond $maxHexValue
  next if ($range[0] > $maxHexValue);

  my $name  = $line[1];
  $name =~ s/^\s+//g;  # ltrim
  $name =~ s/\s+$//g;  # rtrim

  for (my $i = $range[0]; $i <= $range[1]; $i ++) {
    $block[$i] = $block_number;
  }

  $blockIndexes{$name} = \@range;
  $block_number++;
}

close BLOCK;

#-------------------------------------------------------------------------------
#
# Read Scripts
#
#-------------------------------------------------------------------------------
open SCRIPT, "<$ARGV[3]" or die "File '$ARGV[3]' not found";

my $script_number = 1;

while (<SCRIPT>) {
  chomp;

  # ignore empty lines and comments
  next if ($_ =~ /^$|^#/);

  my @line = split /;/, $_;

  # script range
  $line[0] =~ s/^\s+//g;   # ltrim
  $line[0] =~ s/\s+$//g;   # rtrim
  my @range = split /\.\./, $line[0];
  $range[0] = defined($range[0]) ? hex ($range[0]) : 0;
  $range[1] = defined($range[1]) ? hex ($range[1]) : $range[0];

  my $name  = $line[1];
  $name =~ s/^\s+//g;   # ltrim
  $name =~ s/\s+$//g;   # rtrim
  $name =~ s/ #.*$//g;  # clean name

  my $number = $script_number;

  if(!exists($scriptIndexes{$name})) {
    $scriptIndexes{$name} = $script_number;
    $script_number++;
  } else {
    $number = $scriptIndexes{$name};
  }

  for (my $i = $range[0]; $i <= $range[1]; $i ++) {
    # ignore code points beyond $maxHexValue
    next if ($i > $maxHexValue);
    $script[$i] = $number;
  }
}

close SCRIPT;

#-------------------------------------------------------------------------------
#
# Read DerivedCoreProperties
#
#-------------------------------------------------------------------------------
open DERIVED_CORE_PROPERTIES, "<$ARGV[6]" or die "File '$ARGV[6]' not found";

while (<DERIVED_CORE_PROPERTIES>) {
  chomp;

  # ignore empty lines and comments
  next if ($_ =~ /^$|^#/);
  
  # parse the line
  my @line = split /;/, $_;
    
  # ignore unwanted properties
  next if not ($line[1] =~ /^\s(Default_Ignorable_Code_Point)/);
 
  # property range
  $line[0] =~ s/^\s+//g;   # ltrim
  $line[0] =~ s/\s+$//g;   # rtrim
  my @range = split /\.\./, $line[0];
  $range[0] = defined($range[0]) ? hex ($range[0]) : 0;
  $range[1] = defined($range[1]) ? hex ($range[1]) : $range[0];
  
  my $property_name  = $line[1];
  $property_name =~ s/^\s+//g;   # ltrim
  $property_name =~ s/\s+$//g;   # rtrim
  $property_name =~ s/ #.*$//g;  # clean name
  
  for (my $i = $range[0]; $i <= $range[1]; $i ++) {
    # ignore code points beyond $maxHexValue
    next if ($i > $maxHexValue);
    $extraProperty[$i] |= $extraPropertyFlags{$property_name};
  }
}

close DERIVED_CORE_PROPERTIES;
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#
# Read PropList
#
#-------------------------------------------------------------------------------
open PROP_LIST, "<$ARGV[7]" or die "File '$ARGV[7]' not found";

while (<PROP_LIST>) {
  chomp;

  # ignore empty lines and comments
  next if ($_ =~ /^$|^#/);
  
  # parse the line
  my @line = split /;/, $_;

  # ignore unwanted properties
  next if not ($line[1] =~ /^\s(White_Space)/);
 
  # property range
  $line[0] =~ s/^\s+//g;   # ltrim
  $line[0] =~ s/\s+$//g;   # rtrim
  my @range = split /\.\./, $line[0];
  $range[0] = defined($range[0]) ? hex ($range[0]) : 0;
  $range[1] = defined($range[1]) ? hex ($range[1]) : $range[0];
  
  my $property_name  = $line[1];
  $property_name =~ s/^\s+//g;   # ltrim
  $property_name =~ s/\s+$//g;   # rtrim
  $property_name =~ s/ #.*$//g;  # clean name
  
  for (my $i = $range[0]; $i <= $range[1]; $i ++) {
    # ignore code points beyond $maxHexValue
    next if ($i > $maxHexValue);
    $extraProperty[$i] |= $extraPropertyFlags{$property_name};
  }
}

close PROP_LIST;
#-------------------------------------------------------------------------------

my %specialChars = (
	0x000A => moLinebreakGlyphInfo,  # LINE FEED (LF)
	0x000D => moLinebreakGlyphInfo,  # CARRIAGE RETURN (CR)
);

#-------------------------------------------------------------------------------
#
# Read unicode data
#
#-------------------------------------------------------------------------------

open DATA, "<$ARGV[0]" or die "File '$ARGV[0]' not found";

while (<DATA>) {
	chomp;

	my @line = split /;/, $_;
	my $code = hex ($line [0]);

	# ignore code points beyond $maxHexValue
  next if ($code > $maxHexValue);

	my $cat  = $line [2];
	my $info = $categoryFlags {$cat};

  # Canonical combining class value
  my $ccc  = $line [3];

	# Bidi classes
	my $bidi = $line [4];

  # decomposition mappings
  my $decomposition_tag   = '';
  my $decomposition_map   = '';
  my @decomposition_map   = (0,0,0);
  my $decomposition_size  = 0;

	my $number = $line [8];

  my $upper     = length($line [12])  ? hex ($line [12]) : 0;
  my $lower     = length($line [13])  ? hex ($line [13]) : 0;
  my $title     = length($line [14])  ? hex ($line [14]) : $upper;
  my $fold      = 0;
	my $unaccent  = 0;
	my $asciified = 0;

	# unaccent
	# field 5: character's decomposition mapping
	if (length($line [5])) {
	  my $mapping = $line [5];
	  # only canonical mapping, i.e. avoid prefixed <tags>
	  if ($mapping !~ /^</) {
      # a canonical mapping is never longer than two characters
      # and we're looking exactly for two
      if ($mapping =~ /([^ ]+) ([^ ]+)/) {
        # only if the second character belongs to the
        # Combining Diacritical Marks block [0x0300..0x036f]
        if(hex($2) >= 768 && hex($2) <= 879) {
          # the unaccent character is equal to the first character
          # of the canonical mapping
          $unaccent = hex($1);
        }
      }
	  }
	}

	# decomposition mappings
	if (length($line [5])) {
    my $mapping         = $line [5];
    # where no a prefixed tag is given, the mapping is canonical
    $decomposition_tag  = '<canonical>';
    # get the prefixed tag if any
    $decomposition_tag  = $1 if $mapping =~ /^(<[^>]+>)/;
    # get the sequence of one or more unicode code points
    $decomposition_map  = $3 if $mapping =~ /^(<[^>]+>)?(\s)?(.+)$/;
    # convert the list of code points to an array
    @decomposition_map  = split / /, $decomposition_map;
    # the longest compatibility mapping is 18 code points long
    $decomposition_size = scalar(@decomposition_map);
    if ($decomposition_size < 4) {
      # in some instances a canonical mapping or a compatibility mapping
      # may consist of a single code point
      $decomposition_map[0] = hex($decomposition_map[0]);
      # mapping consist of a pair of code points
      $decomposition_map[1] = defined $decomposition_map[1] ? hex($decomposition_map[1]) : 0;
      # mapping consist of 3 code points
      $decomposition_map[2] = defined $decomposition_map[2] ? hex($decomposition_map[2]) : 0;
    } else {
      # set a flag to that the decomposition mapping consist of 4 or more code points
      $info |= moMapExpandsGlyphInfo if defined($info) && $conditionalFlags {'addDecomposition'};
      # set the offset of the full decomposition mapping at decomposition_map[1]
      $decomposition_map[0] = 0;
      $decomposition_map[1] = makeSpecialMappingsSequence $decomposition_map;
      $decomposition_map[2] = 0;
    }
	} else {
	  # the default value of the Decomposition_Mapping property is the
	  # code point of the character itself
	  $decomposition_tag = '<default>';
	}

  # if the character can be folded
  if($casefold[$code] != 0) {
    $fold = $casefold[$code];
  }

	# if the character can be converted to a ascii homoglyph
	if($asciify[$code] != 0) {
	  $asciified = $asciify[$code];
	}

	if ($useCategories && !exists $useCategories {$cat}) {
		$info   = moOtherGlyphInfo;
		$cat    = 'Cn';
		$bidi   = '';
		$decomposition_tag = '';
    @decomposition_map = (0,0,0);
		$number = 0;
		$upper  = 0;
		$lower  = 0;
		$title  = 0;
		$fold   = 0;
		$unaccent  = 0;
		$asciified = 0;
	}
	else {
		if (exists $specialChars {$code}) {
			$info |= $specialChars {$code};
		}
    
    # identifier
    if (($code >= 0x0030 && $code <= 0x0039) || # 0-9
        ($code >= 0x0041 && $code <= 0x005A) || # A-Z
        ($code >= 0x0061 && $code <= 0x007A) || # a-z
         $code == 0x005F ) {                    # _
      $info |= moIdentifierGlyphInfo;
    }

    #extraProperties
    if ($extraProperty[$code] > 0) {
      $info |= $extraProperty[$code];
    }

		$pages [$code >> 8] = 1;

	  # only for the first decomposition code point
    $decomposition_map[0] = $decomposition_map[0] - $code if ($decomposition_map[0]);

		$upper = $upper - $code if ($upper);
		$lower = $lower - $code if ($lower);
		$title = $title - $code if ($title);
		$fold  = $fold  - $code if ($fold);
    # uncomment this line to produce a bigger table
    # $unaccent = $unaccent - $code if ($unaccent);
		$asciified = $asciified - $code if ($asciified);

		if ($number =~ /\//) {
			my ($v1, $v2) = split '/', $number;

			$number = ".frac=\"$v1/$v2\"";
			$info |= moFractionGlyphInfo if defined($info) && $conditionalFlags {'addNumbers'};
		}
		elsif (defined($info) && $info & moNumberGlyphInfo && $conditionalFlags {'addNumbers'}) {
			$number = int ($number);
		}
		else {
			$number = 0;
		}

		if ($special {$code}) {
			my @cases = @{$special {$code}};

			if ($cases [0] != -1) {
				$upper = $cases [0];
				$info |= moUpperExpandsGlyphInfo if defined($info) && $conditionalFlags {'addVariants'};
			}

			if ($cases [1] != -1) {
				$lower = $cases [1];
				$info |= moLowerExpandsGlyphInfo if defined($info) && $conditionalFlags {'addVariants'};
			}

			if ($cases [2] != -1) {
				$title = $cases [2];
				$info |= moTitleExpandsGlyphInfo if defined($info) && $conditionalFlags {'addVariants'};
			}

      if ($cases [3] != -1) {
        $fold = $cases [3];
        $info |= moFoldExpandsGlyphInfo if defined($info) && $conditionalFlags {'addVariants'};
      }

      if ($cases [4] != -1) {
        $asciified = $cases [4];
        $info |= moAsciifyExpandsGlyphInfo if defined($info) && $conditionalFlags {'addVariantAsciify'};
      }
		}
	}

	my $type = sprintf ($infoFormat, defined($info) ? $info : 0, $block[$code], $categoryIndexes {$cat}, $script[$code], $bidiIndexes {$bidi}, $quickCheck[$code], $ccc, $decompositionTagIndexes {$decomposition_tag}, $decomposition_map[0], $decomposition_map[1], $decomposition_map[2], $upper, $lower, $title, $fold, $unaccent, $asciified, $number);

	if ($types {$type}) {
		$type = $types {$type};
	}
	else {
		my $count = keys %types;

		$types {$type} = $count;
		$type = $count;
	}

	$data [$code] = $type;

	# read range
	if ($line [1] =~ /First>$/) {
		$_ = <DATA>;
		chomp;

		my @line2 = split /;/, $_;
		my $code2 = hex ($line2 [0]);

		for (; $code <= $code2; $code ++) {
			$pages [$code >> 8] = 1;
			$data [$code] = $type;
		}
	}
}

close DATA;

#-------------------------------------------------------------------------------
#
# Build pages cache
#
#-------------------------------------------------------------------------------

my $cacheCount = 1;

for (my $i = 0; $i <= $#pages; $i ++) {
	next unless ($pages [$i]);

	my $index = 0;
	my $page  = join ',',  @data [($i << 8) .. ((($i + 1) << 8) - 1)];

	if ($pageCache {$page}) {
		$index = $pageCache {$page};
	}
	else {
		$index = $cacheCount ++;
		$pageCache {$page} = $index;
	}

	$pages [$i] = $index;
}

#-------------------------------------------------------------------------------
#
# Print
#
#-------------------------------------------------------------------------------

# table.h
open my $hdrin,  "<$hdrFileIn" or die "File '$hdrFileIn' not found";
open my $hdrout, ">$hdrFile" or die "File '$hdrFile' not found";

# table.cpp
open my $srcin,  "<$srcFileIn" or die "File '$srcFileIn' not found";
open my $srcout, ">$srcFile" or die "File '$srcFile' not found";

# normalization.h
open my $hdrnormin,  "<$hdrNormFileIn" or die "File '$hdrNormFileIn' not found";
open my $hdrnormout, ">$hdrNormFile" or die "File '$hdrNormFile' not found";

# normalization.cpp
open my $srcnormin,  "<$srcNormFileIn" or die "File '$srcNormFileIn' not found";
open my $srcnorout,  ">$srcNormFile" or die "File '$srcNormFile' not found";

# test.h
open my $testin,  "<$testFileIn" or die "File '$testFileIn' not found";
open my $testout, ">$testFile" or die "File '$testFile' not found";


my %vars = (
	'prefix'         => $prefix,
	'outName'        => $outName,
  'maxValue'       => $maxValue,
  'maxHexValue'    => $maxHexValue,
  'charName'       => $charName,
  'variantsSize'   => $variantsSize,
  'unicodeVersion' => unicodeMajor * 10000000 + unicodeMinor * 100000 + unicodePatch * 1,
);

my @infoKeys  = keys %types;
my $infoSize  = @infoKeys;
my $pagesSize = @pages;

$vars {'infoType'}  = $infoSize  >= 256   ? 'uint16_t' : 'uint8_t';
$vars {'pagesType'} = $pagesSize >= 256   ? 'uint16_t' : 'uint8_t';
$vars {'charType'}  = tableSize > 0x10000 ? 'uint32_t' : 'uint16_t';
$vars {'charSize'}  = tableSize > 0x10000 ? '32'       : '16';

my %printMethods = (
	'header' => sub {
		my $out  = shift;
		my $date = strftime "%B %Y", localtime;
    print $out " * \@file      " . $outName . "\n";
    print $out " * \@brief     Functions to lookup Unicode v" . unicodeVersion ." code points\n";
    print $out " *\n";
    print $out " * \@author    cristian.martinez\@univ-paris-est.fr (martinec)\n";
    print $out " *\n";
    print $out " * \@attention Do not include this file directly, rather include the\n";
    print $out " *            base/common.h header file to gain this file's functionality\n";
    print $out " *\n";
    print $out " * \@warning   unichar is the 16-bits type used to represent a unicode code point.\n";
    print $out " *            Internally, UTF16-LE is used to code each unichar. However,\n";
    print $out " *            for now, Unitex doesn't handle code points greater than 0xFFFF\n";
    print $out " *\n";
    print $out " * \@note      Use cpplint.py tool to detect style errors:\n";
    print $out " *            `cpplint.py --linelength=120 `" . $outName . "`\n";
    print $out " *\n";
    print $out " * \@date      " . $date . "\n";
    print $out " *\n";
    print $out " * This file was automatically generated using an enhanced version of unicode-table 0.3.2\n";
    print $out " * \@see https://github.com/UnitexGramLab/unitex-core/tree/master/base/unicode\n";
	},
	'categories' => sub {
		my $out = shift;

		foreach (sort { $categoryIndexes {$a} <=> $categoryIndexes {$b} } keys %categoryIndexes) {
			my $line = $categoryName {$_};
			my $comment = 'Invalid';

			# XXX(martinec) :~)
			if (length($_)) {
			  $comment =  ': ' . toComment $categoryName {$_};
			}

			$line =  toConstant $line, $prefix;
			$line = sprintf "  %-39s ///< %s%s", "$line,", $_, $comment;
			$line =~ s/\s+$//;

			print $out "$line\n";
		}
	},
  'flags' => sub {
    my $out = shift;

    foreach (sort { $flagIndexes {$a} <=> $flagIndexes {$b} } keys %flagIndexes) {
      my $line    = $_;
      my $comment = $flagDescription {$line};
      my $value   = $flagIndexes {$_};

      $line =  toConstant $line, $prefix . "Flag";
      $line = sprintf "  %-22s = 1 << %-2d,  ///< %s", "$line", log($value)/log(2), $comment;
      $line =~ s/\s+$//;

      if(exists $flagInUse{$_} && $flagInUse{$_} eq 1) {
        print $out "$line\n";
      }
    }
  },
  'blocks' => sub {
    my $out = shift;

    foreach (sort { $blockIndexes {$a}[0] <=> $blockIndexes {$b}[0] } keys %blockIndexes) {
      my $line  = $_;
      my $range = $blockIndexes {$_};

      $line = toUnderscoreConstant "block" . $line, $prefix;

      # XXX(martinec) :~)
      if(@$range[0] < 0) {
        $line = sprintf "  %-56s ///< %s", "$line,", $_;
      } else {
        $line = sprintf "  %-56s ///< [0x%04x..0x%04x] %s", "$line,", @$range[0], @$range[1], $_;
      }

      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
  'scripts' => sub {
    my $out = shift;

    foreach (sort { $scriptIndexes {$a} <=> $scriptIndexes {$b} } keys %scriptIndexes) {
      my $line  = $_;

      $line = toUnderscoreConstant "script" . $line, $prefix;
      $line = sprintf "  %-33s ///< %s", "$line,", $_;
      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
  'bidiClasses' => sub {
    my $out = shift;

    foreach (sort { $bidiIndexes {$a} <=> $bidiIndexes {$b} } keys %bidiIndexes) {
      my $line = $bidiName {$_};
      my $comment = 'Invalid';

      # XXX(martinec) :~)
      if (length($_)) {
        $comment =  ': ' . toComment $bidiName {$_};
      }

      $line = toConstant $line, $prefix;
      $line = sprintf "  %-33s ///< %s%s", "$line,", $_, $comment;
      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
  'normalizationMasks' => sub {
    my $out = shift;

    foreach (sort { $normalizationMaskIndexes {$a} <=> $normalizationMaskIndexes {$b} } keys %normalizationMaskIndexes) {
      my $line    = $_;
      my $comment = $normalizationMaskDescription {$line};
      my $value   = $normalizationMaskIndexes {$_};

      $line =  toConstant $line, $prefix . "NormalizationMask";
      $line = sprintf "  %-34s = %1d,  ///< %s", "$line", $value, $comment;
      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
  'normalizationForms' => sub {
    my $out = shift;

    foreach (sort { $normalizationFormIndexes {$a} <=> $normalizationFormIndexes {$b} } keys %normalizationFormIndexes) {
      my $line    = $_;
      my $comment = $normalizationFormDescription {$line};
      my $value   = $normalizationFormIndexes {$_};

      $line =  toConstant "NF" . $line, $prefix;
      $line = sprintf "  %-34s = %1d,  ///< %s", "$line", $value, $comment;
      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
  'normalizationFormQuickCheckValues' => sub {
    my $out = shift;

    foreach (sort { $normalizationQuickCheckIndexes {$a} <=> $normalizationQuickCheckIndexes {$b} } keys %normalizationQuickCheckIndexes) {
      my $line    = $_;
      my $comment = $normalizationQuickCheckDescription {$line};
      my $value   = $normalizationQuickCheckIndexes {$_};

      $line =  toConstant $line, $prefix . "NF_QuickCheck";
      $line = sprintf "  %-34s = %1d,  ///< %s", "$line", $value, $comment;
      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
  'normalizationFormQuickCheckFlags' => sub {
    my $out = shift;

    foreach (sort { $normalizationFormQuickCheckFlagsIndexes {$a} <=> $normalizationFormQuickCheckFlagsIndexes {$b} } keys %normalizationFormQuickCheckFlagsIndexes) {
      my $name    = $normalizationFormQuickCheckFlagsName{$_};
      my $line    = $_;
      my $comment = $normalizationFormQuickCheckFlagsDescription {$_};
      my $value   = $normalizationFormQuickCheckFlagsIndexes {$_};

      $line =  toConstant $name, $prefix;
      $line = sprintf "  %-34s = %3d,  ///< %s", "$line", $value, $comment;
      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
  'decompositionCccs' => sub {
    my $out = shift;

    foreach (sort { $decompositionCccIndexes {$a} <=> $decompositionCccIndexes {$b} } keys %decompositionCccIndexes) {
      my $line    = $_;
      my $number  = $decompositionCccIndexes {$_};
      my $comment = $decompositionCccDescription {$number};

      $line = toConstant $line, $prefix;
      $line = sprintf "  %-40s = %3d,  ///< %s", "$line", $number, $comment;
      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
  'decompositionTags' => sub {
    my $out = shift;

    foreach (sort { $decompositionTagIndexes {$a} <=> $decompositionTagIndexes {$b} } keys %decompositionTagIndexes) {
      my $line    = $decompositionTagName {$_};
      my $comment = $decompositionTagDescription {$_};

      $line = toConstant $line, $prefix;
      $line = sprintf "  %-33s ///< %s", "$line,", $comment;
      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
	'infos' => sub {
		my $out = shift;

		foreach (sort { $types {$a} <=> $types {$b} } keys %types) {
			print $out "	$_\n";
		}
	},
	'pageIndex' => sub {
		my $out = shift;
		my $i = 0;
		my $p = 0;

		print $out "  ";

		foreach (@pages) {
			print $out "\n  " if ($i > 0 && $i % 16 == 0);
			printf $out "%3d,", $_;

			$i ++;
		}

		print $out "\n";
	},
	'infoIndex' => sub {
		my $out = shift;
		my $p = 0;
		my $i = 0;

		foreach (sort { $pageCache {$a} <=> $pageCache {$b} } keys %pageCache) {
			print $out "  {" if ($p == 0);
			print $out "\n  }, {" if ($p > 0);

			foreach (split /,/, $_) {
				print $out "\n  " if ($i % 16 == 0);
				printf $out "%3d,", $_;

				$i ++;
			}

			$p ++;
		}

		print $out "\n  }\n";
	},
	'specialVariants' => sub {
		my $out = shift;
		my $line = '';

		foreach (@specialVariants) {
			my $data = sprintf "%d, ", $_;

			if (length ($line) + length ($data) >= 66) {
				$line =~ s/\s+$//;
				print $out "  $line\n";

				$line = $data;
			}
			else {
				$line .= $data;
			}
		}

		$line =~ s/\s+$//;
		print $out "  $line\n";
	},
  'specialMappings' => sub {
    my $out = shift;
    my $line = '';

    foreach (@specialMappings) {
      my $data = sprintf "%d, ", $_;

      if (length ($line) + length ($data) >= 66) {
        $line =~ s/\s+$//;
        print $out "  $line\n";

        $line = $data;
      }
      else {
        $line .= $data;
      }
    }

    $line =~ s/\s+$//;
    print $out "  $line\n";
  },
	'categoryNames' => sub {
		my $out = shift;

		foreach (sort { $categoryIndexes {$a} <=> $categoryIndexes {$b} } keys %categoryIndexes) {
			my $key = $categoryName {$_};

			$key = toConstant $key, $prefix;

			printf $out "  %-39s = \"%s\",\n", "[$key]", $_;
		}
	},
  'test_has_flag' => sub {
    my $out = shift;
    my $test_field = "Flag";
    my %indexes    = %flagIndexes;

    foreach (sort { $indexes {$a} <=> $indexes {$b} } keys %indexes) {
      my $line  = $_;
      my $range = $indexes {$_};


      my $macro_name = toUnderscoreConstant $test_field . $line,  $prefix . "Has";
      my $test_flag  = toUnderscoreConstant $test_field . $line,  $prefix;
      my $test_func  = toUnderscoreConstant "Test" . $test_field, $prefix;

      my $space_fill = 69 - length($macro_name);

      $line = sprintf "#define %s(%s)  %+${space_fill}s(%s,%s)", lc $macro_name, $char, lc $test_func, $char, $test_flag;

      $line =~ s/\s+$//;

      if(exists $flagInUse{$_} && $flagInUse{$_} eq 1) {
        print $out "$line\n";
      }
    }
  },
  'test_in_block' => sub {
    my $out = shift;
    my $test_field = "Block";
    my %indexes    = %blockIndexes;

    foreach (sort { $indexes {$a}[0] <=> $indexes {$b}[0] } keys %indexes) {
      my $line  = $_;
      my $range = $indexes {$_};


      my $macro_name = toUnderscoreConstant $test_field . $line,  $prefix . "In";
      my $test_flag  = toUnderscoreConstant $test_field . $line,  $prefix;
      my $test_func  = toUnderscoreConstant "Test" . $test_field, $prefix;

      my $space_fill = 70 - length($macro_name);

      $line = sprintf "#define %s(%s)  %+${space_fill}s(%s,%s)", lc $macro_name, $char, lc $test_func, $char, $test_flag;

      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
  'test_in_category' => sub {
    my $out = shift;
    my $test_field = "Category";
    my %indexes    = %categoryIndexes;

    foreach (sort { $indexes {$a} <=> $indexes {$b} } keys %indexes) {
      my $line  = $categoryName {$_};
      my $range = $indexes {$_};


      my $macro_name = toUnderscoreConstant $line,  $prefix . "In";
      my $test_flag  = toUnderscoreConstant $line,  $prefix;
      my $test_func  = toUnderscoreConstant "Test" . $test_field, $prefix;

      my $space_fill = 73 - length($macro_name);

      $line = sprintf "#define %s(%s)  %+${space_fill}s(%s,%s)", lc $macro_name, $char, lc $test_func, $char, $test_flag;

      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
  'test_in_script' => sub {
    my $out = shift;
    my $test_field = "Script";
    my %indexes    = %scriptIndexes;

    foreach (sort { $indexes {$a} <=> $indexes {$b} } keys %indexes) {
      my $line  = $_;
      my $range = $indexes {$_};


      my $macro_name = toUnderscoreConstant $test_field . $line,  $prefix . "In";
      my $test_flag  = toUnderscoreConstant $test_field . $line,  $prefix;
      my $test_func  = toUnderscoreConstant "Test" . $test_field, $prefix;

      my $space_fill = 71 - length($macro_name);

      $line = sprintf "#define %s(%s)  %+${space_fill}s(%s,%s)", lc $macro_name, $char, lc $test_func, $char, $test_flag;

      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
  'test_has_bidi' => sub {
    my $out = shift;
    my $test_field = "Bidi";
    my %indexes    = %bidiIndexes;

    foreach (sort { $indexes {$a} <=> $indexes {$b} } keys %indexes) {
      my $line  = $bidiName {$_};
      my $range = $indexes {$_};


      my $macro_name = toUnderscoreConstant $line,  $prefix . "Has";
      my $test_flag  = toUnderscoreConstant $line,  $prefix;
      my $test_func  = toUnderscoreConstant "Test" . $test_field, $prefix;

      my $space_fill = 69 - length($macro_name);

      $line = sprintf "#define %s(%s)  %+${space_fill}s(%s,%s)", lc $macro_name, $char, lc $test_func, $char, $test_flag;

      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
  'test_has_normalization_form_quick_check' => sub {
    my $out = shift;
    my $test_field = "NF_QuickCheck";
    my %indexes    = %normalizationFormQuickCheckFlagsIndexes;

    foreach (sort { $indexes {$a} <=> $indexes {$b} } keys %indexes) {
      my $line  = $normalizationFormQuickCheckFlagsName {$_};
      my $range = $indexes {$_};


      my $macro_name = toUnderscoreConstant $line,  $prefix . "Has";
      my $test_flag  = toUnderscoreConstant $line,  $prefix;
      my $test_func  = toUnderscoreConstant "Test" . $test_field, $prefix;

      my $space_fill = 79 - length($macro_name);

      $line = sprintf "#define %s(%s)  %+${space_fill}s(%s,%s)", lc $macro_name, $char, lc $test_func, $char, $test_flag;

      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
  'test_has_decomposition_tag' => sub {
    my $out = shift;
    my $test_field = "DecompositionTag";
    my %indexes    = %decompositionTagIndexes;

    foreach (sort { $indexes {$a} <=> $indexes {$b} } keys %indexes) {
      my $line  = $decompositionTagName {$_};
      my $range = $indexes {$_};


      my $macro_name = toUnderscoreConstant $line,  $prefix . "Has";
      my $test_flag  = toUnderscoreConstant $line,  $prefix;
      my $test_func  = toUnderscoreConstant "Test" . $test_field, $prefix;

      my $space_fill = 82 - length($macro_name);

      $line = sprintf "#define %s(%s)  %+${space_fill}s(%s,%s)", lc $macro_name, $char, lc $test_func, $char, $test_flag;

      $line =~ s/\s+$//;

      print $out "$line\n";
    }
  },
);

sub replaceName {
	my $type = shift;
	my $name = shift;

	if ($type eq 'n') {
		$name = toUserCase $name;
	}
	elsif ($type eq 'c') {
		$name = toConstant $name, $prefix;
	}
  elsif ($type eq 'k') {
    $name = toKConstant $name, $prefix, $prefix_k;
  }
	elsif ($type eq 'v') {
		$name = $vars {$name};
	}

	return $name;
}

sub handleLine {
	my $line = shift;
	my $out = shift;

	if ($line =~ /##([\w_]+)(\{([\w_]+)\})?/) {
		my $method = $1;

		die "Print method '$method' does not exist" unless (exists $printMethods {$method});

    if(!defined($3)) {
      $printMethods {$method} -> ($out);
    } else {
      $printMethods {$method} -> ($out,$3);
    }

		# @see http://stackoverflow.com/a/25808905
		no warnings "exiting";
		next;
	}

	# handle if:
	if ($line =~ /{(if:)([\w_]+)}/) {
		return exists $conditionalFlags {$2};
	}
	# ignore endif:
	elsif ($line =~ /{(endif:)([\w_]*)}/) {
		return 1;
	}

	$line =~ s/{((\w+):)([\w_]+)}/replaceName($2, $3)/ge;

	# unescape macro paste (##)
	$line =~ s/\\#\\#/##/g;

	print $out $line;

	return 1;
}

sub readToEndIf {
	my $file = shift;

	while (<$file>) {
		return if ($_ =~ /{(endif:)([\w_]*)}/);
	}
}

while (<$hdrin>) {
	if (!handleLine $_, $hdrout) {
		readToEndIf $hdrin;
	}
}

while (<$srcin>) {
	if (!handleLine $_, $srcout) {
		readToEndIf $srcin;
	}
}

while (<$testin>) {
  if (!handleLine $_, $testout) {
    readToEndIf $testin;
  }
}

close $hdrin;
close $hdrout;

close $srcin;
close $srcout;

close $testin;
close $testout;
