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

#include "Alphabet.h"
#include "Error.h"


/**
 * Allocates, initializes an returns an 'Alphabet*' structure
 */
Alphabet* new_alphabet() {
Alphabet* alphabet=(Alphabet*)malloc(sizeof(Alphabet));
for (int i=0;i<0x10000;i++) {
    alphabet->t[i]=NULL;
    alphabet->t2[i]=0;
}
return alphabet;
}


/**
 * Frees a given 'Alphabet*' structure
 */
void free_alphabet(Alphabet* alphabet) {
if (alphabet==NULL) return;
for (int i=0;i<0x10000;i++) {
  if (alphabet->t[i]!=NULL)
    free(alphabet->t[i]);
}
free(alphabet);
}


/**
 * Adds a letter equivalence to the given alphabet. For instance,
 * if 'lower'="e" and 'upper'="E", "E" will now be considered as an
 * uppercase equivalent of "e".
 */
void add_letter_equivalence(Alphabet* alphabet,unichar lower,unichar upper) {
if (alphabet->t[lower]==NULL) {
   alphabet->t[lower]=(unichar*)malloc(2*sizeof(unichar));
   alphabet->t[lower][0]=upper;
   alphabet->t[lower][1]='\0';
   return;
}
int L=u_strlen(alphabet->t[lower]);
alphabet->t[lower]=(unichar*)realloc(alphabet->t[lower],(L+2)*sizeof(unichar));
alphabet->t[lower][L]=upper;
alphabet->t[lower][L+1]='\0';
}


/**
 * Loads an alphabet file and returns the associated 'Alphabet*' structure.
 */
Alphabet* load_alphabet(char* filename) {
FILE* f;
f=u_fopen(filename,U_READ);
if (f==NULL) return NULL;
Alphabet* alphabet=new_alphabet();
int c;
unichar lower,upper;
while ((c=u_fgetc(f))!=EOF) {
      upper=(unichar)c;
      if (upper=='#') {
         // we are in the case of an interval #AZ -> [A..Z]
         lower=(unichar)u_fgetc(f);
         upper=(unichar)u_fgetc(f);
         if (lower>upper) {
            error("Error in alphabet file: for an interval like #AZ, A must be before Z\n");
            u_fclose(f);
            return NULL;
         }
         for (c=lower;c<=upper;c++) {
           alphabet->t2[c]=(char)(alphabet->t2[c] | 1);
           alphabet->t2[c]=(char)(alphabet->t2[c] | 2);
           add_letter_equivalence(alphabet,(unichar)c,(unichar)c);
         }
         u_fgetc(f); // reading the \n
      }
      else {
        alphabet->t2[upper]=(char)(alphabet->t2[upper] | 1);
        lower=(unichar)u_fgetc(f);
        if (lower!='\n') {
          alphabet->t2[lower]=(char)(alphabet->t2[lower] | 2);
          u_fgetc(f); // reading the \n
          add_letter_equivalence(alphabet,lower,upper);
        }
        else {
          // we are in the case of a single (no min/maj distinction like in thai)
          alphabet->t2[upper]=(char)(alphabet->t2[upper] | 2);
          add_letter_equivalence(alphabet,upper,upper);
        }
      }
}
u_fclose(f);
return alphabet;
}


/**
 * Returns 1 if 'upper' is considered as an uppercase equivalent
 * of 'lower' for the given alphabet; returns 0 otherwise.
 */
int is_upper_of(unichar lower,unichar upper,Alphabet* alphabet) {
if (alphabet->t[lower]==NULL) return 0;
int i=0;
while (alphabet->t[lower][i]!='\0') {
      if (alphabet->t[lower][i]==upper) return 1;
      i++;
}
return 0;
}


/**
 * Returns a non-zero value if 'a' and 'b' are identical no matter the case;
 * 0 otherwise.
 */
int is_equal_ignore_case(unichar a,unichar b,Alphabet* alphabet) {
return a==b || is_upper_of(a,b,alphabet) || is_upper_of(b,a,alphabet);
}


/**
 * Returns a non-zero value if 'b' is identical to 'a' or if it is
 * an uppercase equivalent of 'a' according for the given alphabet; 
 * returns 0 otherwise.
 */
int is_equal_or_uppercase(unichar a,unichar b,Alphabet* alphabet) {
return (a==b || is_upper_of(a,b,alphabet));
}


/**
 * Returns a non-zero value if 'b' is identical to 'a' or if it is
 * an uppercase equivalent of 'a' according for the given alphabet; 
 * returns 0 otherwise.
 */
int is_equal_or_uppercase(unichar* a,unichar* b,Alphabet* alphabet) {
int i=0;
while (a[i] && is_equal_or_uppercase(a[i],b[i],alphabet)) {i++;}
return (a[i]=='\0' && b[i]=='\0');
}


/**
 * Returns 1 if 'c' is considered as an uppercase letter
 * in the given alphabet, 0 otherwise.
 */
int is_upper(unichar c,Alphabet* alphabet) {
return (alphabet->t2[c] & 1);
}


/**
 * Returns 2 if 'c' is considered as a lowercase letter
 * in the given alphabet, 0 otherwise.
 */
int is_lower(unichar c,Alphabet* alphabet) {
return (alphabet->t2[c] & 2);
}


/**
 * Returns a non-zero value if 'c' is considered
 * as a letter in for the given alphabet, 0 otherwise.
 */
int is_letter(unichar c,Alphabet* alphabet) {
return is_upper(c,alphabet)||is_lower(c,alphabet);
}


/**
 * Returns 1 if 'c' is a letter; 0 otherwise. If alphabet is non NULL,
 * it is used to determine which characters are letters; otherwise, the
 * 'u_is_letter' function is used.
 */
int is_letter2(unichar c,Alphabet* alphabet) {
if (alphabet==NULL) {
   return u_is_letter(c);
}
return is_letter(c,alphabet);
}


/**
 * Returns 1 if the string 's' is only made of letters,
 * according to the given alphabet, 0 otherwise.
 */
int is_sequence_of_letters(unichar* s,Alphabet* alphabet) {
for (int i=0;s[i]!='\0';i++) {
   if (!is_letter(s[i],alphabet)) return 0;
}
return 1;
}


/**
 * Returns 1 if the string 's' is only made of lowercase letters,
 * according to the given alphabet, 0 otherwise.
 */
int is_sequence_of_lowercase_letters(unichar* s,Alphabet* alphabet) {
int i=0;
while (s[i]!='\0') {
  if (!is_lower(s[i],alphabet)) return 0;
  i++;
}
return 1;
}


/**
 * Returns 1 if the string 's' is only made of uppercase letters,
 * according to the given alphabet, 0 otherwise.
 */
int is_sequence_of_uppercase(unichar* s,Alphabet* alphabet) {
int i=0;
while (s[i]!='\0') {
  if (!is_upper(s[i],alphabet)) return 0;
  i++;
}
return 1;
}


/**
 * This function turns a Portuguese letter sequence into a lowercase one.
 * It cannot be a general function because of potential ambiguities
 * like "A" -> "a" or "à" in French.
 * It works on Portuguese because the uppercase/lowercase relations are 
 * bijectives.
 */
void turn_portuguese_sequence_to_lowercase(unichar* s) {
int i=0;
while (s[i]!='\0') {
   switch (s[i]) {
      case 'A':s[i]='a'; break;
      case 0xc0: s[i]=0xe0; break;
      case 0xc1: s[i]=0xe1; break;
      case 0xc2: s[i]=0xe2; break;
      case 0xc3: s[i]=0xe3; break;
      case 0xc4: s[i]=0xe4; break;
      case 'B': s[i]='b'; break;
      case 'C': s[i]='c'; break;
      case 0xc7: s[i]=0xe7; break;
      case 'D': s[i]='d'; break;
      case 'E': s[i]='e'; break;
      case 0xc8: s[i]=0xe8; break;
      case 0xc9: s[i]=0xe9; break;
      case 0xca: s[i]=0xea; break;
      case 0xcb: s[i]=0xeb; break;
      case 'F': s[i]='f'; break;
      case 'G': s[i]='g'; break;
      case 'H': s[i]='h'; break;
      case 'I': s[i]='i'; break;
      case 0xcc: s[i]=0xec; break;
      case 0xcd: s[i]=0xed; break;
      case 0xce: s[i]=0xee; break;
      case 0xcf: s[i]=0xef; break;
      case 'J': s[i]='j'; break;
      case 'K': s[i]='k'; break;
      case 'L': s[i]='l'; break;
      case 'M': s[i]='m'; break;
      case 'N': s[i]='n'; break;
      case 'O': s[i]='o'; break;
      case 0xd2: s[i]=0xf2; break;
      case 0xd3: s[i]=0xf3; break;
      case 0xd4: s[i]=0xf4; break;
      case 0xd5: s[i]=0xf5; break;
      case 0xd6: s[i]=0xf6; break;
      case 'P': s[i]='p'; break;
      case 'Q': s[i]='q'; break;
      case 'R': s[i]='r'; break;
      case 'S': s[i]='s'; break;
      case 'T': s[i]='t'; break;
      case 'U': s[i]='u'; break;
      case 0xd9: s[i]=0xf9; break;
      case 0xda: s[i]=0xfa; break;
      case 0xdb: s[i]=0xfb; break;
      case 0xdc: s[i]=0xfc; break;
      case 'V': s[i]='v'; break;
      case 'W': s[i]='w'; break;
      case 'X': s[i]='x'; break;
      case 'Y': s[i]='y'; break;
      case 'Z': s[i]='z'; break;
      default:; // if we don't have an uppercase letter, we have nothing to do
   }
   i++;
}
}


/**
 * Takes a given unicode string 'dest' and
 * replaces any lowercase letter by the set made of itself and
 * its uppercase equivalent, surrounded with square brackets if
 * the letter was not already between square brackets.
 * Examples:
 * 
 * "For" => "F[oO][rR]"
 * "F[ao]r" => "F[aAoO][rR]"
 * 
 * The output is stored in 'src'. The function assumes that 'src' is
 * wide enough.
 * 
 * This function is used for morphological filter regular expressions.
 */
void replace_letter_by_letter_set(Alphabet* a,unichar* dest,unichar* src) {
int i=0,j=0;
char inside_a_set=0;
while (src[i]!='\0') {
   switch (src[i]) {
      case '\\': 
         if (src[i+1]=='\0') {
             // there is nothing after a backslash, then we stop,
             // and the RE compiler may indicate an error 
             dest[j++] = src[i++];
             dest[j] = src[i];
             return;
         }
         if (a->t2[src[i+1]] & 2) {
             // this is a lowercase letter in Unitex alphabet :
             // we don't need "\" and we make expansion "[eE]"
             ++i;
             if (!inside_a_set) dest[j++]='[';
             dest[j++]=src[i];
             int k=0;
             while (a->t[src[i]][k]!='\0') {
                dest[j++]=a->t[src[i]][k++];
             }
             if (!inside_a_set) dest[j++]=']';
             i++;
          } else {
             // others cases :
             // we keep the "\" and the letter
             dest[j++] = src[i++];
             dest[j++] = src[i++];
          }
          break;
       case '[':
          dest[j++]=src[i++];
          inside_a_set=1;
          break;
       case ']':
          dest[j++]=src[i++];
          inside_a_set=0;
          break;
       case '.': case '*': case '+': case '?': case '|': case '^': case '$':
       case ':': case '(': case ')': case '{': case '}': case '1': case '2':
       case '3': case '4': case '5': case '6': case '7': case '8': case '9': 
          dest[j++]=src[i++];
          break;
       default:
          if (a->t2[src[i]] & 2) {
             if (!inside_a_set) dest[j++]='[';
             dest[j++]=src[i];
             int k=0;
             while (a->t[src[i]][k]!='\0') {
                dest[j++]=a->t[src[i]][k++];
             }
             if (!inside_a_set) dest[j++]=']';
             i++;
          }
          else dest[j++]=src[i++];
   }       
}
dest[j]='\0';
}



/**
 * Returns the length of the longuest prefix common to the strings 'a' and 'b',
 * but allowing the following case variants:
 * 
 * ab OK
 * Ab OK
 * AB OK
 * aB X
 */
int get_longuest_prefix_ignoring_case(unichar* a,unichar* b,Alphabet* alphabet) {
if (a==NULL || b==NULL) {
   return 0;
}
int i=0;
while (is_equal_or_uppercase(b[i],a[i],alphabet) && a[i]!='\0') i++;
return i;
}
