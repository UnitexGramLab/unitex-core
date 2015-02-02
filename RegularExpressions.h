/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef RegularExpressionsH
#define RegularExpressionsH

#include "Unicode.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define REG_EXP_MAX_LENGTH 10000


int reg2grf(const unichar*,const char*, const VersatileEncodingConfig*);

/**
 * The automaton coded in reg_2_grf is designed from the one produced
 * by the parser generator bison from the following regexp.y grammar:
 */
/*
%{
#include <stdlib.h>
int yyerror(char*);
int yylex();
%}
%token TOKEN

%left '+'
%left '.' ' '
%right '*'
%%
S : E '0' {}
      ;
E : E '+' E {}
  | E '.' E {}
  | E E {}
  | E Y E {}
  | '(' E ')' {}
  | E '*' {}
  | TOKEN {}
  ;

Y : ' ' {}
 | ' ' Y {}
 ;
%%

#include "lex.yy.c"

int yyerror(char* s) {
  fprintf(stderr,"%s\n",s);
  return 0;
}

int main(void) {
  yyparse();
  return 0;
}
*/

/**
 * Here is the resulting automaton, modified by hand to correct some bad conflict
 * resolutions:
 */
/*
State 12 conflicts: 6 shift/reduce
State 15 conflicts: 2 shift/reduce
State 16 conflicts: 2 shift/reduce
State 18 conflicts: 6 shift/reduce


Grammaire

    0 $accept: S $end

    1 S: E '0'

    2 E: E '+' E
    3  | E '.' E
    4  | E E
    5  | E Y E
    6  | '(' E ')'
    7  | E '*'
    8  | TOKEN

    9 Y: ' '
   10  | ' ' Y


Terminaux, suivis des règles où ils apparaissent

$end (0) 0
' ' (32) 9 10
'(' (40) 6
')' (41) 6
'*' (42) 7
'+' (43) 2
'.' (46) 3
'0' (48) 1
error (256)
TOKEN (258) 8


Non-terminaux, suivis des règles où ils apparaissent

$accept (11)
    � gauche: 0
S (12)
    � gauche: 1, � droite: 0
E (13)
    � gauche: 2 3 4 5 6 7 8, � droite: 1 2 3 4 5 6 7
Y (14)
    � gauche: 9 10, � droite: 5 10


état 0

    0 $accept: . S $end
    1 S: . E '0'
    2 E: . E '+' E
    3  | . E '.' E
    4  | . E E
    5  | . E Y E
    6  | . '(' E ')'
    7  | . E '*'
    8  | . TOKEN

    TOKEN  décalage et aller à l'état 1
    '('    décalage et aller à l'état 2

    S  aller à l'état 3
    E  aller à l'état 4


état 1

    8 E: TOKEN .

    $défaut  réduction par utilisation de la règle 8 (E)


état 2

    2 E: . E '+' E
    3  | . E '.' E
    4  | . E E
    5  | . E Y E
    6  | . '(' E ')'
    6  | '(' . E ')'
    7  | . E '*'
    8  | . TOKEN

    TOKEN  décalage et aller à l'état 1
    '('    décalage et aller à l'état 2

    E  aller à l'état 5


état 3

    0 $accept: S . $end

    $end  décalage et aller à l'état 6


état 4

    1 S: E . '0'
    2 E: . E '+' E
    2  | E . '+' E
    3  | . E '.' E
    3  | E . '.' E
    4  | . E E
    4  | E . E
    5  | . E Y E
    5  | E . Y E
    6  | . '(' E ')'
    7  | . E '*'
    7  | E . '*'
    8  | . TOKEN
    9 Y: . ' '
   10  | . ' ' Y

    TOKEN  décalage et aller à l'état 1
    '+'    décalage et aller à l'état 7
    '.'    décalage et aller à l'état 8
    ' '    décalage et aller à l'état 9
    '*'    décalage et aller à l'état 10
    '0'    décalage et aller à l'état 11
    '('    décalage et aller à l'état 2

    E  aller à l'état 12
    Y  aller à l'état 13


état 5

    2 E: . E '+' E
    2  | E . '+' E
    3  | . E '.' E
    3  | E . '.' E
    4  | . E E
    4  | E . E
    5  | . E Y E
    5  | E . Y E
    6  | . '(' E ')'
    6  | '(' E . ')'
    7  | . E '*'
    7  | E . '*'
    8  | . TOKEN
    9 Y: . ' '
   10  | . ' ' Y

    TOKEN  décalage et aller à l'état 1
    '+'    décalage et aller à l'état 7
    '.'    décalage et aller à l'état 8
    ' '    décalage et aller à l'état 9
    '*'    décalage et aller à l'état 10
    '('    décalage et aller à l'état 2
    ')'    décalage et aller à l'état 14

    E  aller à l'état 12
    Y  aller à l'état 13


état 6

    0 $accept: S $end .

    $défaut  accepter


état 7

    2 E: . E '+' E
    2  | E '+' . E
    3  | . E '.' E
    4  | . E E
    5  | . E Y E
    6  | . '(' E ')'
    7  | . E '*'
    8  | . TOKEN

    TOKEN  décalage et aller à l'état 1
    '('    décalage et aller à l'état 2

    E  aller à l'état 15


état 8

    2 E: . E '+' E
    3  | . E '.' E
    3  | E '.' . E
    4  | . E E
    5  | . E Y E
    6  | . '(' E ')'
    7  | . E '*'
    8  | . TOKEN

    TOKEN  décalage et aller à l'état 1
    '('    décalage et aller à l'état 2

    E  aller à l'état 16


état 9

    9 Y: . ' '  [TOKEN, '(']
    9  | ' ' .  [TOKEN, '(']
   10  | . ' ' Y
   10  | ' ' . Y

    ' '  décalage et aller à l'état 9

    $défaut  réduction par utilisation de la règle 9 (Y)

    Y  aller à l'état 17


état 10

    7 E: E '*' .

    $défaut  réduction par utilisation de la règle 7 (E)


état 11

    1 S: E '0' .

    $défaut  réduction par utilisation de la règle 1 (S)


état 12

    2 E: . E '+' E
    2  | E . '+' E
    3  | . E '.' E
    3  | E . '.' E
    4  | . E E  [TOKEN, '+', '.', ' ', '*', '0', '(', ')']
    4  | E . E  [TOKEN, '+', '.', ' ', '*', '0', '(', ')']
    4  | E E .  [TOKEN, '+', '.', ' ', '*', '0', '(', ')']
    5  | . E Y E
    5  | E . Y E
    6  | . '(' E ')'
    7  | . E '*'
    7  | E . '*'
    8  | . TOKEN
    9 Y: . ' '
   10  | . ' ' Y

    '*'    décalage et aller à l'état 10

    TOKEN    [réduction par utilisation de la règle 4 (E)]
    '+'      [réduction par utilisation de la règle 4 (E)]
    '.'      [réduction par utilisation de la règle 4 (E)]
    ' '      [réduction par utilisation de la règle 4 (E)]
    '('      [réduction par utilisation de la règle 4 (E)]
    $défaut  réduction par utilisation de la règle 4 (E)

    E  aller à l'état 12
    Y  aller à l'état 13


état 13

    2 E: . E '+' E
    3  | . E '.' E
    4  | . E E
    5  | . E Y E
    5  | E Y . E
    6  | . '(' E ')'
    7  | . E '*'
    8  | . TOKEN

    TOKEN  décalage et aller à l'état 1
    '('    décalage et aller à l'état 2

    E  aller à l'état 18


état 14

    6 E: '(' E ')' .

    $défaut  réduction par utilisation de la règle 6 (E)


état 15

    2 E: . E '+' E  [TOKEN, '+', '0', '(', ')']
    2  | E . '+' E  [TOKEN, '+', '0', '(', ')']
    2  | E '+' E .  [TOKEN, '+', '0', '(', ')']
    3  | . E '.' E
    3  | E . '.' E
    4  | . E E
    4  | E . E
    5  | . E Y E
    5  | E . Y E
    6  | . '(' E ')'
    7  | . E '*'
    7  | E . '*'
    8  | . TOKEN
    9 Y: . ' '
   10  | . ' ' Y

    TOKEN  décalage et aller à l'état 1
    '.'    décalage et aller à l'état 8
    ' '    décalage et aller à l'état 9
    '*'    décalage et aller à l'état 10
    '('    décalage et aller à l'état 2
    $défaut  réduction par utilisation de la règle 2 (E)

    E  aller à l'état 12
    Y  aller à l'état 13

Conflit entre la règle 2 et le jeton '+' résolu par réduction (%left '+').
Conflit entre la règle 2 et le jeton '.' résolu par décalage ('+' < '.').
Conflit entre la règle 2 et le jeton ' ' résolu par décalage ('+' < ' ').
Conflit entre la règle 2 et le jeton '*' résolu par décalage ('+' < '*').


état 16

    2 E: . E '+' E
    2  | E . '+' E
    3  | . E '.' E  [TOKEN, '+', '.', ' ', '0', '(', ')']
    3  | E . '.' E  [TOKEN, '+', '.', ' ', '0', '(', ')']
    3  | E '.' E .  [TOKEN, '+', '.', ' ', '0', '(', ')']
    4  | . E E
    4  | E . E
    5  | . E Y E
    5  | E . Y E
    6  | . '(' E ')'
    7  | . E '*'
    7  | E . '*'
    8  | . TOKEN
    9 Y: . ' '
   10  | . ' ' Y

    '*'    décalage et aller à l'état 10
   TOKEN   réduction par utilisation de la règle 3 (E)
    '('    réduction par utilisation de la règle 3 (E)
    $défaut  réduction par utilisation de la règle 3 (E)

    E  aller à l'état 12
    Y  aller à l'état 13

Conflit entre la règle 3 et le jeton '+' résolu par réduction ('+' < '.').
Conflit entre la règle 3 et le jeton '.' résolu par réduction (%left '.').
Conflit entre la règle 3 et le jeton ' ' résolu par réduction (%left ' ').
Conflit entre la règle 3 et le jeton '*' résolu par décalage ('.' < '*').


état 17

   10 Y: ' ' Y .

    $défaut  réduction par utilisation de la règle 10 (Y)


état 18

    2 E: . E '+' E
    2  | E . '+' E
    3  | . E '.' E
    3  | E . '.' E
    4  | . E E
    4  | E . E
    5  | . E Y E  [TOKEN, '+', '.', ' ', '*', '0', '(', ')']
    5  | E . Y E  [TOKEN, '+', '.', ' ', '*', '0', '(', ')']
    5  | E Y E .  [TOKEN, '+', '.', ' ', '*', '0', '(', ')']
    6  | . '(' E ')'
    7  | . E '*'
    7  | E . '*'
    8  | . TOKEN
    9 Y: . ' '
   10  | . ' ' Y

    '*'    décalage et aller à l'état 10
    TOKEN  réduction par utilisation de la règle 5 (E)
    '+'    réduction par utilisation de la règle 5 (E)
    '.'    réduction par utilisation de la règle 5 (E)
    ' '    réduction par utilisation de la règle 5 (E)
    '('    réduction par utilisation de la règle 5 (E)
    $défaut  réduction par utilisation de la règle 5 (E)

    E  aller à l'état 12
    Y  aller à l'état 13
*/

} // namespace unitex

#endif
