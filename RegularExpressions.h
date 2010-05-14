/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


#define REG_EXP_MAX_LENGTH 10000


int reg2grf(unichar*,char*,Encoding,int);

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


Terminaux, suivis des r�gles o� ils apparaissent

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


Non-terminaux, suivis des r�gles o� ils apparaissent

$accept (11)
    � gauche: 0
S (12)
    � gauche: 1, � droite: 0
E (13)
    � gauche: 2 3 4 5 6 7 8, � droite: 1 2 3 4 5 6 7
Y (14)
    � gauche: 9 10, � droite: 5 10


�tat 0

    0 $accept: . S $end
    1 S: . E '0'
    2 E: . E '+' E
    3  | . E '.' E
    4  | . E E
    5  | . E Y E
    6  | . '(' E ')'
    7  | . E '*'
    8  | . TOKEN

    TOKEN  d�calage et aller � l'�tat 1
    '('    d�calage et aller � l'�tat 2

    S  aller � l'�tat 3
    E  aller � l'�tat 4


�tat 1

    8 E: TOKEN .

    $d�faut  r�duction par utilisation de la r�gle 8 (E)


�tat 2

    2 E: . E '+' E
    3  | . E '.' E
    4  | . E E
    5  | . E Y E
    6  | . '(' E ')'
    6  | '(' . E ')'
    7  | . E '*'
    8  | . TOKEN

    TOKEN  d�calage et aller � l'�tat 1
    '('    d�calage et aller � l'�tat 2

    E  aller � l'�tat 5


�tat 3

    0 $accept: S . $end

    $end  d�calage et aller � l'�tat 6


�tat 4

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

    TOKEN  d�calage et aller � l'�tat 1
    '+'    d�calage et aller � l'�tat 7
    '.'    d�calage et aller � l'�tat 8
    ' '    d�calage et aller � l'�tat 9
    '*'    d�calage et aller � l'�tat 10
    '0'    d�calage et aller � l'�tat 11
    '('    d�calage et aller � l'�tat 2

    E  aller � l'�tat 12
    Y  aller � l'�tat 13


�tat 5

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

    TOKEN  d�calage et aller � l'�tat 1
    '+'    d�calage et aller � l'�tat 7
    '.'    d�calage et aller � l'�tat 8
    ' '    d�calage et aller � l'�tat 9
    '*'    d�calage et aller � l'�tat 10
    '('    d�calage et aller � l'�tat 2
    ')'    d�calage et aller � l'�tat 14

    E  aller � l'�tat 12
    Y  aller � l'�tat 13


�tat 6

    0 $accept: S $end .

    $d�faut  accepter


�tat 7

    2 E: . E '+' E
    2  | E '+' . E
    3  | . E '.' E
    4  | . E E
    5  | . E Y E
    6  | . '(' E ')'
    7  | . E '*'
    8  | . TOKEN

    TOKEN  d�calage et aller � l'�tat 1
    '('    d�calage et aller � l'�tat 2

    E  aller � l'�tat 15


�tat 8

    2 E: . E '+' E
    3  | . E '.' E
    3  | E '.' . E
    4  | . E E
    5  | . E Y E
    6  | . '(' E ')'
    7  | . E '*'
    8  | . TOKEN

    TOKEN  d�calage et aller � l'�tat 1
    '('    d�calage et aller � l'�tat 2

    E  aller � l'�tat 16


�tat 9

    9 Y: . ' '  [TOKEN, '(']
    9  | ' ' .  [TOKEN, '(']
   10  | . ' ' Y
   10  | ' ' . Y

    ' '  d�calage et aller � l'�tat 9

    $d�faut  r�duction par utilisation de la r�gle 9 (Y)

    Y  aller � l'�tat 17


�tat 10

    7 E: E '*' .

    $d�faut  r�duction par utilisation de la r�gle 7 (E)


�tat 11

    1 S: E '0' .

    $d�faut  r�duction par utilisation de la r�gle 1 (S)


�tat 12

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

    '*'    d�calage et aller � l'�tat 10

    TOKEN    [r�duction par utilisation de la r�gle 4 (E)]
    '+'      [r�duction par utilisation de la r�gle 4 (E)]
    '.'      [r�duction par utilisation de la r�gle 4 (E)]
    ' '      [r�duction par utilisation de la r�gle 4 (E)]
    '('      [r�duction par utilisation de la r�gle 4 (E)]
    $d�faut  r�duction par utilisation de la r�gle 4 (E)

    E  aller � l'�tat 12
    Y  aller � l'�tat 13


�tat 13

    2 E: . E '+' E
    3  | . E '.' E
    4  | . E E
    5  | . E Y E
    5  | E Y . E
    6  | . '(' E ')'
    7  | . E '*'
    8  | . TOKEN

    TOKEN  d�calage et aller � l'�tat 1
    '('    d�calage et aller � l'�tat 2

    E  aller � l'�tat 18


�tat 14

    6 E: '(' E ')' .

    $d�faut  r�duction par utilisation de la r�gle 6 (E)


�tat 15

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

    TOKEN  d�calage et aller � l'�tat 1
    '.'    d�calage et aller � l'�tat 8
    ' '    d�calage et aller � l'�tat 9
    '*'    d�calage et aller � l'�tat 10
    '('    d�calage et aller � l'�tat 2
    $d�faut  r�duction par utilisation de la r�gle 2 (E)

    E  aller � l'�tat 12
    Y  aller � l'�tat 13

Conflit entre la r�gle 2 et le jeton '+' r�solu par r�duction (%left '+').
Conflit entre la r�gle 2 et le jeton '.' r�solu par d�calage ('+' < '.').
Conflit entre la r�gle 2 et le jeton ' ' r�solu par d�calage ('+' < ' ').
Conflit entre la r�gle 2 et le jeton '*' r�solu par d�calage ('+' < '*').


�tat 16

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

    '*'    d�calage et aller � l'�tat 10
   TOKEN   r�duction par utilisation de la r�gle 3 (E)
    '('    r�duction par utilisation de la r�gle 3 (E)
    $d�faut  r�duction par utilisation de la r�gle 3 (E)

    E  aller � l'�tat 12
    Y  aller � l'�tat 13

Conflit entre la r�gle 3 et le jeton '+' r�solu par r�duction ('+' < '.').
Conflit entre la r�gle 3 et le jeton '.' r�solu par r�duction (%left '.').
Conflit entre la r�gle 3 et le jeton ' ' r�solu par r�duction (%left ' ').
Conflit entre la r�gle 3 et le jeton '*' r�solu par d�calage ('.' < '*').


�tat 17

   10 Y: ' ' Y .

    $d�faut  r�duction par utilisation de la r�gle 10 (Y)


�tat 18

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

    '*'    d�calage et aller � l'�tat 10
    TOKEN  r�duction par utilisation de la r�gle 5 (E)
    '+'    r�duction par utilisation de la r�gle 5 (E)
    '.'    r�duction par utilisation de la r�gle 5 (E)
    ' '    r�duction par utilisation de la r�gle 5 (E)
    '('    r�duction par utilisation de la r�gle 5 (E)
    $d�faut  r�duction par utilisation de la r�gle 5 (E)

    E  aller � l'�tat 12
    Y  aller � l'�tat 13
*/

#endif
