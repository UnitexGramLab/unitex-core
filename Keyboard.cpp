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

#include "Keyboard.h"
#include "Error.h"
#include "CharNames.h"

namespace unitex {

static Keyboard keyboards[N_KEYBOARDS];


/**
 * Returns 1 if 'a' and 'b' are close on the same line on the given
 * keyboard; 0 otherwise.
 */
int areCloseOnKeyboard(unichar a,unichar b,Keyboard* kbd) {
if (kbd==NULL) {
   /* No keyboard means that this function is deactivated */
   return 0;
}
unichar* tab[6];
unichar* key;
/*----- upper line -----------*/
tab[0]=kbd->low0;
tab[1]=kbd->up0;
tab[2]=kbd->alt0;
for (int k=0;k<3;k++) {
   key=tab[k];
   /* leftmost char */
   if (key[0]==a) {
      return b==key[1];
   }
   /* rightmost char */
   if (key[PC_KEYBOARD_UPPER_LINE_SIZE-1]==a) {
      return b==key[PC_KEYBOARD_UPPER_LINE_SIZE-2];
   }
   /* middle chars */
   for (int i=1;i<PC_KEYBOARD_UPPER_LINE_SIZE-1;i++) {
      if (key[i]==a) {
         return b==key[i-1] || b==key[i+1];
      }
   }
}
/*----- middle lines -----------*/
tab[0]=kbd->low1;
tab[1]=kbd->up1;
tab[2]=kbd->alt1;
tab[3]=kbd->low2;
tab[4]=kbd->up2;
tab[5]=kbd->alt2;
for (int k=0;k<6;k++) {
   key=tab[k];
   /* leftmost char */
   if (key[0]==a) {
      return b==key[1];
   }
   /* rightmost char */
   if (key[PC_KEYBOARD_LINE_SIZE-1]==a) {
      return b==key[PC_KEYBOARD_LINE_SIZE-2];
   }
   /* middle chars */
   for (int i=1;i<PC_KEYBOARD_LINE_SIZE-1;i++) {
      if (key[i]==a) {
         return b==key[i-1] || b==key[i+1];
      }
   }
}
/*----- lower line -----------*/
tab[0]=kbd->low3;
tab[1]=kbd->up3;
tab[2]=kbd->alt3;
for (int k=0;k<3;k++) {
   key=tab[k];
   /* leftmost char */
   if (key[0]==a) {
      return b==key[1];
   }
   /* rightmost char */
   if (key[PC_KEYBOARD_LOWER_LINE_SIZE-1]==a) {
      return b==key[PC_KEYBOARD_LOWER_LINE_SIZE-2];
   }
   /* middle chars */
   for (int i=1;i<PC_KEYBOARD_LOWER_LINE_SIZE-1;i++) {
      if (key[i]==a) {
         return b==key[i-1] || b==key[i+1];
      }
   }
}
return 0;
}


static void init_Keyboard(Keyboard* keyboard) {
if (keyboard==NULL) {
   fatal_error("Unexpected NULL error in init_keyboard\n");
}
strcpy(keyboard->name,"");
strcpy(keyboard->comment,"Empty keyboard");
for (int i=0;i<PC_KEYBOARD_UPPER_LINE_SIZE;i++) {
   keyboard->low0[i]=NO_KEY_DEFINED;
   keyboard->up0[i]=NO_KEY_DEFINED;
   keyboard->alt0[i]=NO_KEY_DEFINED;
}
for (int i=0;i<PC_KEYBOARD_LINE_SIZE;i++) {
   keyboard->low1[i]=NO_KEY_DEFINED;
   keyboard->up1[i]=NO_KEY_DEFINED;
   keyboard->alt1[i]=NO_KEY_DEFINED;
   keyboard->low2[i]=NO_KEY_DEFINED;
   keyboard->up2[i]=NO_KEY_DEFINED;
   keyboard->alt2[i]=NO_KEY_DEFINED;
}
for (int i=0;i<PC_KEYBOARD_LOWER_LINE_SIZE;i++) {
   keyboard->low3[i]=NO_KEY_DEFINED;
   keyboard->up3[i]=NO_KEY_DEFINED;
   keyboard->alt3[i]=NO_KEY_DEFINED;
}
}


static void u_strcpy_char_with_no_zero(unichar* dest,const char* src) {
int i=0;
while (src[i]!='\0') {
   dest[i]=(unichar)((unsigned char)src[i]);
   i++;
}
}


static void init_ansi_qwerty(Keyboard* keyboard) {
init_Keyboard(keyboard);
strcpy(keyboard->name,"ansi-qwerty");
strcpy(keyboard->comment,"Strict ANSI qwerty layout");
u_strcpy_char_with_no_zero(keyboard->low1,"qwertyuiop");
u_strcpy_char_with_no_zero(keyboard->low2,"asdfghjkl");
u_strcpy_char_with_no_zero(&(keyboard->low3[1]),"zxcvbnm");
u_strcpy_char_with_no_zero(keyboard->up1,"QWERTYUIOP");
u_strcpy_char_with_no_zero(keyboard->up2,"ASDFGHJKL");
u_strcpy_char_with_no_zero(&(keyboard->up3[1]),"ZXCVBNM");
}


static void init_ansi_azerty(Keyboard* keyboard) {
init_Keyboard(keyboard);
strcpy(keyboard->name,"ansi-azerty");
strcpy(keyboard->comment,"Strict ANSI azerty layout");
u_strcpy_char_with_no_zero(keyboard->low1,"azertyuiop");
u_strcpy_char_with_no_zero(keyboard->low2,"qsdfghjklm");
u_strcpy_char_with_no_zero(&(keyboard->low3[1]),"wxcvbn");
u_strcpy_char_with_no_zero(keyboard->up1,"AZERTYUIOP");
u_strcpy_char_with_no_zero(keyboard->up2,"QSDFGHJKLM");
u_strcpy_char_with_no_zero(&(keyboard->up3[1]),"WXCVBN");
}


static void init_extended_azerty(Keyboard* keyboard) {
init_ansi_azerty(keyboard);
strcpy(keyboard->name,"extended-azerty");
strcpy(keyboard->comment,"Azerty layout extended with latin accented letters");
keyboard->low0[0]=U_SUPERSCRIPT_TWO;
keyboard->low0[1]='&';
keyboard->low0[2]=U_ECUTE;
keyboard->low0[3]='"';
keyboard->low0[4]='\'';
keyboard->low0[5]='(';
keyboard->low0[6]=U_SECTION_MARK;
keyboard->low0[7]=U_EGRAVE;
keyboard->low0[8]='!';
keyboard->low0[9]=U_CCEDILLA;
keyboard->low0[10]=U_AGRAVE;
keyboard->low0[11]=')';
keyboard->low0[12]='-';
/*-----------------------------*/
u_strcpy_char_with_no_zero(&(keyboard->up0[1]),"1234567890");
keyboard->up0[11]=U_DEGREE;
keyboard->up0[12]='_';
/*-----------------------------*/
keyboard->alt0[1]='|';
keyboard->alt0[2]='@';
keyboard->alt0[6]='^';
keyboard->alt0[9]='{';
keyboard->alt0[10]='}';
keyboard->alt0[12]='\\';
/*-----------------------------
 * We do not take into account dead keys, i.e. keys that
 * must be combined with other keys to produce a symbol.
 * Ex: the dead circumflex key must be combined with a letter to
 *     produce a letter, whereas the normal circumflex key prints
 *     the '^' char.
 */
keyboard->alt1[10]='[';
keyboard->low1[11]='$';
keyboard->up1[11]='*';
keyboard->alt1[11]=']';
/*-----------------------------*/
keyboard->low2[10]=U_UGRAVE;
keyboard->up2[10]='%';
keyboard->alt2[10]=U_ACUTE;
keyboard->low2[11]=U_MU;
keyboard->up2[11]=U_STERLING;
/*-----------------------------*/
keyboard->low3[0]='<';
keyboard->up3[0]='>';
keyboard->alt3[0]='\\';
u_strcpy_char_with_no_zero(&(keyboard->low3[7]),",;:=");
u_strcpy_char_with_no_zero(&(keyboard->up3[7]),"?./+");
}


static void init_fr_latin1(Keyboard* keyboard) {
init_ansi_azerty(keyboard);
strcpy(keyboard->name,"fr-latin1");
strcpy(keyboard->comment,"Keyboard used for French of France");
keyboard->low0[0]=U_SUPERSCRIPT_TWO;
keyboard->low0[1]='&';
keyboard->low0[2]=U_ECUTE;
keyboard->low0[3]='"';
keyboard->low0[4]='\'';
keyboard->low0[5]='(';
keyboard->low0[6]='-';
keyboard->low0[7]=U_EGRAVE;
keyboard->low0[8]='_';
keyboard->low0[9]=U_CCEDILLA;
keyboard->low0[10]=U_AGRAVE;
keyboard->low0[11]=')';
keyboard->low0[12]='=';
/*-----------------------------*/
u_strcpy_char_with_no_zero(&(keyboard->up0[1]),"1234567890");
keyboard->up0[11]=U_DEGREE;
keyboard->up0[12]='+';
/*-----------------------------*/
keyboard->alt0[2]=U_TILDE;
keyboard->alt0[3]=U_NUMBER_SIGN;
keyboard->alt0[4]='{';
keyboard->alt0[5]='[';
keyboard->alt0[6]='|';
keyboard->alt0[8]='\\';
keyboard->alt0[9]=U_CIRCUMFLEX;
keyboard->alt0[10]='@';
keyboard->alt0[11]=']';
keyboard->alt0[12]='}';
/*-----------------------------*/
keyboard->alt1[2]=U_EURO;
keyboard->low1[11]='$';
keyboard->up1[11]=U_STERLING;
keyboard->alt1[11]=U_CURRENCY;
/*-----------------------------*/
keyboard->low2[10]=U_UGRAVE;
keyboard->up2[10]='%';
keyboard->low2[11]='*';
keyboard->up2[11]=U_MU;
/*-----------------------------*/
keyboard->low3[0]='<';
keyboard->up3[0]='>';
u_strcpy_char_with_no_zero(&(keyboard->low3[7]),",;:!");
u_strcpy_char_with_no_zero(&(keyboard->up3[7]),"?./");
keyboard->up3[10]=U_SECTION_MARK;
}


static int init_keyboards() {
init_ansi_qwerty(&(keyboards[0]));
init_ansi_azerty(&(keyboards[1]));
init_extended_azerty(&(keyboards[2]));
init_fr_latin1(&(keyboards[3]));
return 0;
}


/* This line is used to force the initialization of the keyboards */
static int phantom_var=init_keyboards();



Keyboard* get_Keyboard(const char* name) {
for (int i=0;i<N_KEYBOARDS;i++) {
   if (!strcmp(name,keyboards[i].name)) {
      return &(keyboards[i]);
   }
}
return NULL;
}


/**
 * Displays the letter keys of a given keyboard
 */
void print_Keyboard(Keyboard* keyboard,U_FILE* out) {
if (keyboard==NULL) {
   fatal_error("Unexpected NULL keyboard error in print_Keyboard\n");
}
u_fprintf(out,"      Keyboard name=%s\n\n",keyboard->name);
u_fprintf(out,"      %s\n\n",keyboard->comment);
/*----- First keyboard line -------*/
u_fputc(0x250C,out);
for (int i=0;i<PC_KEYBOARD_UPPER_LINE_SIZE;i++) {
   u_fputc(0x2500,out);
   u_fputc(0x2500,out);
   u_fputc(0x2500,out);
   u_fputc(0x2500,out);
   u_fputc(0x252C,out);
}
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2510,out);
u_fputc('\n',out);
/*-----------------------------*/
u_fputc(0x2502,out);
for (int i=0;i<PC_KEYBOARD_UPPER_LINE_SIZE;i++) {
   u_fputc(' ',out);
   u_fputc((keyboard->up0[i]!=NO_KEY_DEFINED)?keyboard->up0[i]:' ',out);
   u_fputc(' ',out);
   u_fputc(' ',out);
   u_fputc(0x2502,out);
}
u_fprintf(out," <");
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fprintf(out,"  ");
u_fputc(0x2502,out);
u_fputc('\n',out);
/*-----------------------------*/
u_fputc(0x2502,out);
for (int i=0;i<PC_KEYBOARD_UPPER_LINE_SIZE;i++) {
   u_fputc(' ',out);
   u_fputc((keyboard->low0[i]!=NO_KEY_DEFINED)?keyboard->low0[i]:' ',out);
   u_fputc(' ',out);
   u_fputc((keyboard->alt0[i]!=NO_KEY_DEFINED)?keyboard->alt0[i]:' ',out);
   u_fputc(0x2502,out);
}
u_fprintf(out,"       ");
u_fputc(0x2502,out);
u_fputc('\n',out);
/*-------- Second keyboard line -----------*/
u_fputc(0x251C,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2534,out);
for (int i=0;i<PC_KEYBOARD_LINE_SIZE;i++) {
   u_fputc(0x2500,out);
   u_fputc(0x252C,out);
   u_fputc(0x2500,out);
   u_fputc(0x2500,out);
   u_fputc(0x2534,out);
}
u_fputc(0x2500,out);
u_fputc(0x252C,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2524,out);
u_fputc('\n',out);
/*-----------------------------*/
u_fputc(0x2502,out);
u_fprintf(out," |<-  ");
u_fputc(0x2502,out);
for (int i=0;i<PC_KEYBOARD_LINE_SIZE;i++) {
   u_fputc(' ',out);
   u_fputc((keyboard->up1[i]!=NO_KEY_DEFINED)?keyboard->up1[i]:' ',out);
   u_fputc(' ',out);
   u_fputc(' ',out);
   u_fputc(0x2502,out);
}
u_fprintf(out,"   ");
u_fputc(0x2502,out);
u_fputc(' ',out);
u_fputc(0x2502,out);
u_fputc('\n',out);
/*-----------------------------*/
u_fputc(0x2502,out);
u_fprintf(out," ->|  ");
u_fputc(0x2502,out);
for (int i=0;i<PC_KEYBOARD_LINE_SIZE;i++) {
   u_fputc(' ',out);
   u_fputc((keyboard->low1[i]!=NO_KEY_DEFINED)?keyboard->low1[i]:' ',out);
   u_fputc(' ',out);
   u_fputc((keyboard->alt1[i]!=NO_KEY_DEFINED)?keyboard->alt1[i]:' ',out);
   u_fputc(0x2502,out);
}
u_fprintf(out," <");
u_fputc(0x2500,out);
u_fputc(0x2518,out);
u_fputc(' ',out);
u_fputc(0x2502,out);
u_fputc('\n',out);
/*-------- Third keyboard line --------*/
u_fputc(0x251C,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2534,out);
for (int i=0;i<PC_KEYBOARD_LINE_SIZE;i++) {
   u_fputc(0x2500,out);
   u_fputc(0x252C,out);
   u_fputc(0x2500,out);
   u_fputc(0x2500,out);
   u_fputc(0x2534,out);
}
u_fputc(0x2500,out);
u_fputc(0x2510,out);
u_fputc(' ',out);
u_fputc(' ',out);
u_fputc(' ',out);
u_fputc(0x2502,out);
u_fputc('\n',out);
/*-----------------------------*/
u_fputc(0x2502,out);
u_fprintf(out," Caps   ");
u_fputc(0x2502,out);
for (int i=0;i<PC_KEYBOARD_LINE_SIZE;i++) {
   u_fputc(' ',out);
   u_fputc((keyboard->up2[i]!=NO_KEY_DEFINED)?keyboard->up2[i]:' ',out);
   u_fputc(' ',out);
   u_fputc(' ',out);
   u_fputc(0x2502,out);
}
u_fprintf(out,"   ");
u_fputc(0x2502,out);
u_fputc('\n',out);
/*-----------------------------*/
u_fputc(0x2502,out);
u_fprintf(out," Lock   ");
u_fputc(0x2502,out);
for (int i=0;i<PC_KEYBOARD_LINE_SIZE;i++) {
   u_fputc(' ',out);
   u_fputc((keyboard->low2[i]!=NO_KEY_DEFINED)?keyboard->low2[i]:' ',out);
   u_fputc(' ',out);
   u_fputc((keyboard->alt2[i]!=NO_KEY_DEFINED)?keyboard->alt2[i]:' ',out);
   u_fputc(0x2502,out);
}
u_fprintf(out,"   ");
u_fputc(0x2502,out);
u_fputc('\n',out);
/*-------- Fourth keyboard line ----------*/
u_fputc(0x251C,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x252C,out);
u_fputc(0x2500,out);
u_fputc(0x2534,out);
for (int i=0;i<PC_KEYBOARD_LOWER_LINE_SIZE;i++) {
   u_fputc(0x2500,out);
   u_fputc(0x2500,out);
   u_fputc(0x252C,out);
   u_fputc(0x2500,out);
   u_fputc(0x2534,out);
}
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2534,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2524,out);
u_fputc('\n',out);
/*-----------------------------*/
u_fputc(0x2502,out);
u_fprintf(out,"      ");
u_fputc(0x2502,out);
for (int i=0;i<PC_KEYBOARD_LOWER_LINE_SIZE;i++) {
   u_fputc(' ',out);
   u_fputc((keyboard->up3[i]!=NO_KEY_DEFINED)?keyboard->up3[i]:' ',out);
   u_fputc(' ',out);
   u_fputc(' ',out);
   u_fputc(0x2502,out);
}
u_fprintf(out,"          ");
u_fputc(0x2502,out);
u_fputc('\n',out);
/*-----------------------------*/
u_fputc(0x2502,out);
u_fprintf(out," Shift");
u_fputc(0x2502,out);
for (int i=0;i<PC_KEYBOARD_LOWER_LINE_SIZE;i++) {
   u_fputc(' ',out);
   u_fputc((keyboard->low3[i]!=NO_KEY_DEFINED)?keyboard->low3[i]:' ',out);
   u_fputc(' ',out);
   u_fputc((keyboard->alt3[i]!=NO_KEY_DEFINED)?keyboard->alt3[i]:' ',out);
   u_fputc(0x2502,out);
}
u_fprintf(out,"  Shift   ");
u_fputc(0x2502,out);
u_fputc('\n',out);
/*-----------------------------*/
u_fputc(0x2514,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2534,out);
for (int i=0;i<PC_KEYBOARD_LOWER_LINE_SIZE-1;i++) {
   u_fputc(0x2500,out);
   u_fputc(0x2500,out);
   u_fputc(0x2500,out);
   u_fputc(0x2500,out);
   u_fputc(0x2534,out);
}
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2534,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2500,out);
u_fputc(0x2518,out);
u_fputc('\n',out);
}


/**
 * A debug function to print all available keyboards.
 */
void print_available_keyboards(U_FILE* out) {
for (int i=0;i<N_KEYBOARDS;i++) {
   print_Keyboard(&(keyboards[i]),out);
   u_fprintf(out,"\n\n--------------------------------------------------------------\n\n");
}
}

} // namespace unitex
