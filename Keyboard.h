/*
 * Unitex
 *
 * Copyright (C) 2001-2012 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef KeyboardH
#define KeyboardH

/*
 This library deals with non-special keys of keyboards.
 NOTE: we do not take into account dead keys, i.e. keys that
 must be combined with other keys to produce a symbol.
 Ex: the dead circumflex key must be combined with a letter to
     produce a letter, whereas the normal circumflex key prints
     the '^' char.

 As an example, here is the fr-latin1 keyboard:

      Keyboard name=fr-latin1

+------------------------------------------------------------------------+
|    | 1  | 2  | 3  | 4  | 5  | 6  | 7  | 8  | 9  | 0  | °  | +  | <---  |
| ²  | &  | é ~| " #| ' {| ( [| - || è  | _ \| ç ^| à @| ) ]| = }|       |
+------------------------------------------------------------------------|
| |<-  | A  | Z  | E  | R  | T  | Y  | U  | I  | O  | P  |    | £  |   | |
| ->|  | a  | z  | e €| r  | t  | y  | u  | i  | o  | p  |    | $ �| <-+ |
+--------------------------------------------------------------------+   |
| Caps   | Q  | S  | D  | F  | G  | H  | J  | K  | L  | M  | %  | µ  |   |
| Lock   | q  | s  | d  | f  | g  | h  | j  | k  | l  | m  | ù  | *  |   |
+------------------------------------------------------------------------|
|      | >  | W  | X  | C  | V  | B  | N  | ?  | .  | /  | §  |          |
| Shift| <  | w  | x  | c  | v  | b  | n  | ,  | ;  | :  | !  |  Shift   |
+------------------------------------------------------------------------+


 Here is the logical key numbers map:

+----------------------------------------------------------------+-------+
| 41 | 2  | 3  | 4  | 5  | 6  | 7  | 8  | 9  | 10 | 11 | 12 | 13 |       |
|    |    |    |    |    |    |    |    |    |    |    |    |    |       |
+-------------------------------------------------------------------+----+
|      | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 |     |
|      |    |    |    |    |    |    |    |    |    |    |    |    |     |
+------+-------------------------------------------------------------+   |
|        | 30 | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 | 40 | 43 |   |
|        |    |    |    |    |    |    |    |    |    |    |    |    |   |
+------+-------------------------------------------------------------+---|
|      | 86 | 44 | 45 | 46 | 47 | 48 | 49 | 50 | 51 | 52 | 53 |          |
|      |    |    |    |    |    |    |    |    |    |    |    |          |
+------+------------------------------------------------------+----------+
*/

#include "Unicode.h"

#define PC_KEYBOARD_UPPER_LINE_SIZE 13  // number of keys in the digits keyboard line
#define PC_KEYBOARD_LINE_SIZE 12        // number of keys in a keyboard line
#define PC_KEYBOARD_LOWER_LINE_SIZE 11  // number of keys in the lowest keyboard line

#define NO_KEY_DEFINED 0

#define N_KEYBOARDS 4

typedef struct {
   char name[128];
   char comment[1024];
   unichar low0[PC_KEYBOARD_UPPER_LINE_SIZE];
   unichar low1[PC_KEYBOARD_LINE_SIZE];
   unichar low2[PC_KEYBOARD_LINE_SIZE];
   unichar low3[PC_KEYBOARD_LOWER_LINE_SIZE];
   unichar up0[PC_KEYBOARD_UPPER_LINE_SIZE];
   unichar up1[PC_KEYBOARD_LINE_SIZE];
   unichar up2[PC_KEYBOARD_LINE_SIZE];
   unichar up3[PC_KEYBOARD_LOWER_LINE_SIZE];
   unichar alt0[PC_KEYBOARD_UPPER_LINE_SIZE];
   unichar alt1[PC_KEYBOARD_LINE_SIZE];
   unichar alt2[PC_KEYBOARD_LINE_SIZE];
   unichar alt3[PC_KEYBOARD_LOWER_LINE_SIZE];
} Keyboard;


Keyboard* get_Keyboard(const char*);
int areCloseOnKeyboard(unichar,unichar,Keyboard*);
void print_Keyboard(Keyboard*,U_FILE*);

void print_available_keyboards(U_FILE*);

#endif

