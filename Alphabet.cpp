/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Persistence.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

// 0x400 in final release, good for all langage
#define FIRST_SIZE_ARRAYCOLLECTION 0x400
#define ENLARGE_ARRAYCOLLECTION_FACTOR 0x10

void enlarge_buffer_alphabet(Alphabet* alphabet) {
    alphabet->i_nb_array_pos_allocated = alphabet->i_nb_array_pos_allocated * ENLARGE_ARRAYCOLLECTION_FACTOR;
    if (alphabet->i_nb_array_pos_allocated > 0x10000)
        alphabet->i_nb_array_pos_allocated = 0x10000;

    alphabet->t_array_collection=(unichar**)realloc(alphabet->t_array_collection,
                              alphabet->i_nb_array_pos_allocated*sizeof(unichar*));
    if (alphabet->t_array_collection == NULL) {
        fatal_alloc_error("enlarge_buffer_alphabet");
    }
}


/**
 * Allocates, initializes an returns an 'Alphabet*' structure
 */
Alphabet* new_alphabet(int korean) {
Alphabet* alphabet=(Alphabet*)malloc(sizeof(Alphabet));
if (alphabet==NULL) {
   fatal_alloc_error("new_alphabet");
}
memset(alphabet,0,sizeof(Alphabet));
//alphabet->higher_written = 0; // 0 is reserved
alphabet->i_nb_array_pos_allocated = FIRST_SIZE_ARRAYCOLLECTION;
alphabet->t_array_collection=(unichar**)malloc(alphabet->i_nb_array_pos_allocated*sizeof(unichar*));
if (alphabet->t_array_collection==NULL) {
   fatal_alloc_error("new_alphabet");
}
alphabet->t_array_collection[0] = NULL;
if (korean) {
   alphabet->korean_equivalent_syllable=(unichar*)malloc(0x10000*sizeof(unichar));
   if (alphabet->korean_equivalent_syllable==NULL) {
      fatal_alloc_error("new_alphabet");
   }
   memset(alphabet->korean_equivalent_syllable,0,0x10000*sizeof(unichar));
   /*
   for (int i=0;i<0x10000;i++) {
      alphabet->korean_equivalent_syllab[i]=0;
   }*/
} else {
   alphabet->korean_equivalent_syllable=NULL;
}
return alphabet;
}


/**
 * Frees a given 'Alphabet*' structure
 */
void free_alphabet(Alphabet* alphabet) {
if (alphabet==NULL || is_persistent_structure(alphabet)) return;
/*
for (int i=0;i<alphabet->higher_written;i++) {
  if (alphabet->t[i]!=NULL)
    free(alphabet->t[i]);
}*/
for (int i=1;i<=alphabet->i_last_array_pos_used;i++)
   free(alphabet->t_array_collection[i]);
free(alphabet->t_array_collection);
if (alphabet->korean_equivalent_syllable!=NULL) {
   free(alphabet->korean_equivalent_syllable);
}
free(alphabet);
}


/**
 * Adds a letter equivalence to the given alphabet. For instance,
 * if 'lower'="e" and 'upper'="E", "E" will now be considered as an
 * uppercase equivalent of "e".
 */
void add_letter_equivalence(Alphabet* alphabet,unichar lower,unichar upper) {
if (alphabet->pos_in_represent_list[lower]==0) {
   alphabet->i_last_array_pos_used++;
   int i_pos_in_array_of_string = alphabet->i_last_array_pos_used;
   if (i_pos_in_array_of_string >= alphabet->i_nb_array_pos_allocated) {
     enlarge_buffer_alphabet(alphabet);
   }
   alphabet->pos_in_represent_list[lower] = (uint16_t)i_pos_in_array_of_string;
   alphabet->t_array_collection[i_pos_in_array_of_string]=(unichar*)malloc(2*sizeof(unichar));
   if (alphabet->t_array_collection[i_pos_in_array_of_string]==NULL) {
      fatal_alloc_error("add_letter_equivalence");
   }
   alphabet->t_array_collection[i_pos_in_array_of_string][0]=upper;
   alphabet->t_array_collection[i_pos_in_array_of_string][1]='\0';
   return;
}
int i_pos_in_array_of_string = alphabet->pos_in_represent_list[lower];
int L=u_strlen(alphabet->t_array_collection[i_pos_in_array_of_string]);
alphabet->t_array_collection[i_pos_in_array_of_string]=(unichar*)realloc(alphabet->t_array_collection[i_pos_in_array_of_string],(L+2)*sizeof(unichar));
if (alphabet->t_array_collection[i_pos_in_array_of_string]==NULL) {
   fatal_alloc_error("add_letter_equivalence");
}
alphabet->t_array_collection[i_pos_in_array_of_string][L]=upper;
alphabet->t_array_collection[i_pos_in_array_of_string][L+1]='\0';
/* If needed, we look at the Korean case, but only if we have a Chinese character */
if (alphabet->korean_equivalent_syllable!=NULL) {
   if (u_is_CJK_Unified_Ideograph(upper) || u_is_CJK_compatibility_ideograph(upper)) {
      if (alphabet->korean_equivalent_syllable[upper]!=0) {
         fatal_error("add_letter_equivalence: Chinese character %C has several equivalent Hangul characters\n",upper);
      }
      alphabet->korean_equivalent_syllable[upper]=lower;
   }
}
}


int is_abstract_or_persistent_alphabet_filename(const char* filename)
{
    if (get_persistent_structure(filename))
        return 1;
    return 0;
}


/**
 * Loads an alphabet file and returns the associated 'Alphabet*' structure.
 * If 'korean' is non null, we compute the equivalences between Chinese and Hangul
 * characters.
 */
Alphabet* load_alphabet(const VersatileEncodingConfig* vec,const char* filename,int korean) {
void* a=get_persistent_structure(filename);
if (a!=NULL) {
    return (Alphabet*)a;
}
U_FILE* f;
f=u_fopen(vec,filename,U_READ);
if (f==NULL) {
   return NULL;
}
Alphabet* alphabet=new_alphabet(korean);
int c;
unichar lower,upper;
while ((c=u_fgetc(f))!=EOF) {
      upper=(unichar)c;
      if (upper=='\n') {
          /* We skip empty lines */
          continue;
      }
      if (upper=='#') {
         // we are in the case of an interval #AZ -> [A..Z]
         lower=(unichar)u_fgetc(f);
         upper=(unichar)u_fgetc(f);
         if (lower>upper) {
            error("Error in alphabet file: for an interval like #AZ, A must be before Z\n");
            free_alphabet(alphabet);
            u_fclose(f);
            return NULL;
         }
         for (c=lower;c<=upper;c++) {
           SET_CASE_FLAG_MACRO(c,alphabet,1|2);
           add_letter_equivalence(alphabet,(unichar)c,(unichar)c);
         }
         u_fgetc(f); // reading the \n
      }
      else {
        SET_CASE_FLAG_MACRO(upper,alphabet,1);
        lower=(unichar)u_fgetc(f);
        if (lower!='\n') {
          SET_CASE_FLAG_MACRO(lower,alphabet,2);
          u_fgetc(f); // reading the \n
          add_letter_equivalence(alphabet,lower,upper);
        }
        else {
          // we are in the case of a single (no min/maj distinction like in thai)
          SET_CASE_FLAG_MACRO(upper,alphabet,2);
          add_letter_equivalence(alphabet,upper,upper);
        }
      }
}
u_fclose(f);
return alphabet;
}


/**
 * Loads an alphabet file and returns the associated 'Alphabet*' structure.
 */
Alphabet* load_alphabet(const VersatileEncodingConfig* vec,const char* filename) {
return load_alphabet(vec,filename,0);
}


/**
 * Returns 1 if 'upper' is considered as an uppercase equivalent
 * of 'lower' for the given alphabet; returns 0 otherwise.
 */
int is_upper_of(unichar lower,unichar upper,const Alphabet* alphabet) {
if (alphabet==NULL) {
   return upper==u_toupper(lower);
}
int i_pos_in_array_of_string = alphabet->pos_in_represent_list[lower];
if (i_pos_in_array_of_string == 0) return 0;
int i=0;
while (alphabet->t_array_collection[i_pos_in_array_of_string][i]!='\0') {
      if (alphabet->t_array_collection[i_pos_in_array_of_string][i]==upper) return 1;
      i++;
}
return 0;
}


/**
 * Returns a non-zero value if 'a' and 'b' are identical no matter the case;
 * 0 otherwise.
 */
int is_equal_ignore_case(unichar a,unichar b,const Alphabet* alphabet) {
return a==b || is_upper_of(a,b,alphabet) || is_upper_of(b,a,alphabet);
}


/**
 * Returns a non-zero value if 'b' is identical to 'a' or if it is
 * an uppercase equivalent of 'a' according for the given alphabet;
 * returns 0 otherwise.
 */
int is_equal_or_uppercase(unichar a,unichar b,const Alphabet* alphabet) {
if (alphabet!=NULL) {
    if (a==b)
        return 1;

    int i_pos_in_array_of_string = alphabet->pos_in_represent_list[a];
    if (i_pos_in_array_of_string == 0) return 0;
    int i=0;
    while (alphabet->t_array_collection[i_pos_in_array_of_string][i]!='\0') {
          if (alphabet->t_array_collection[i_pos_in_array_of_string][i]==b) return 1;
          i++;
    }
    return 0;
}
return (a==b || is_upper_of(a,b,alphabet));
}


/**
 * Returns a non-zero value if 'b' is identical to 'a' or if it is
 * an uppercase equivalent of 'a' according for the given alphabet;
 * returns 0 otherwise.
 */
int is_equal_or_uppercase(const unichar* a,const unichar* b,const Alphabet* alphabet) {
int i=0;
while (a[i] && is_equal_or_uppercase(a[i],b[i],alphabet)) {i++;}
return (a[i]=='\0' && b[i]=='\0');
}



static int test_qp(unichar a,unichar b,int quotes,const Alphabet* alph) {
if (quotes) return a==b;
return is_equal_or_uppercase(a,b,alph);
}

/**
 * Returns a non-zero value if 'b' is identical to 'a' or if it is
 * an uppercase equivalent of 'a' according for the given alphabet;
 * returns 0 otherwise.
 *
 * The difference with is_equal_or_uppercase is that the function consider
 * case-protection with double quotes as in grf boxes. Every sequence surrounded by
 * double quotes will thus have to be match exactly.
 *
 * Examples: Anchor    anchor  => ok
 *          "Anchor"  "anchor" => X
 */
int is_equal_or_uppercase_qp(const unichar* a,const unichar* b,const Alphabet* alphabet) {
int i=0,quotes=0;
while (a[i] && b[i]) {
    if (!test_qp(a[i],b[i],quotes,alphabet)) return 0;
    if (a[i]=='"') {
        quotes=!quotes;
    } else if (a[i]=='\\') {
        i++;
        if (!test_qp(a[i],b[i],quotes,alphabet)) return 0;
    }
    i++;
}
return (a[i]=='\0' && b[i]=='\0');
}



/**
 * Returns 1 if 'c' is considered as an uppercase letter
 * in the given alphabet, 0 otherwise.
 */
int is_upper(unichar c,const Alphabet* alphabet) {
if (alphabet==NULL) {
    if (u_is_letter(c) == 0)
        return 0;
    return (c == u_toupper(c)) ? 1 : 0;
}
return IS_UPPER_MACRO(c,alphabet);
}


/**
 * Returns 2 if 'c' is considered as a lowercase letter
 * in the given alphabet, 0 otherwise.
 */
int is_lower(unichar c,const Alphabet* alphabet) {
if (alphabet==NULL) {
    if (u_is_letter(c) == 0)
        return 0;
    return (c == u_tolower(c)) ? 2 : 0;
}
return IS_LOWER_MACRO(c,alphabet);
}


/**
 * Returns a non-zero value if 'c' is considered
 * as a letter in for the given alphabet, 0 otherwise.
 */
static inline int is_letter__(unichar c,const Alphabet* alphabet) {
return CASE_FLAG_MACRO(c,alphabet) != 0;
}


/**
 * Returns 1 if 'c' is a letter; 0 otherwise. If alphabet is non NULL,
 * it is used to determine which characters are letters; otherwise, the
 * 'u_is_letter' function is used.
 */
int is_letter(unichar c,const Alphabet* alphabet) {
if (alphabet==NULL) {
   return u_is_letter(c);
}
return is_letter__(c,alphabet);
}


/**
 * Returns 1 if the string 's' is only made of letters,
 * according to the given alphabet, 0 otherwise.
 */
int is_sequence_of_letters(const unichar* s,const Alphabet* alphabet) {
for (int i=0;s[i]!='\0';i++) {
   if (!is_letter(s[i],alphabet)) return 0;
}
return 1;
}


/**
 * Returns 1 if the string 's' is only made of lowercase letters,
 * according to the given alphabet, 0 otherwise.
 */
int is_sequence_of_lowercase_letters(const unichar* s,const Alphabet* alphabet) {
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
int is_sequence_of_uppercase_letters(const unichar* s,const Alphabet* alphabet) {
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
 * like "A" -> "a" or "�" in French.
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
void replace_letter_by_letter_set(const Alphabet* a,unichar* dest,const unichar* src) {
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
         if (is_lower(src[i+1],a)) {
             // this is a lowercase letter in Unitex alphabet :
             // we don't need "\" and we make expansion "[eE]"
             ++i;
             if (!inside_a_set) dest[j++]='[';
             dest[j++]=src[i];
             if (a==NULL) {
                /* If there is no alphabet file, we just consider the unique
                 * uppercase variant of the letter */
                dest[j++]=u_toupper(src[i]);
             } else {
             unichar* tbrowse = NULL;
             int i_pos_in_array_of_string = a->pos_in_represent_list[src[i]];
             if (i_pos_in_array_of_string != 0)
                 tbrowse = a->t_array_collection[i_pos_in_array_of_string];
             if (tbrowse != NULL)
                 while ((*tbrowse) != '\0') {
                     dest[j++]=*(tbrowse++);
                 }
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
          if (is_lower(src[i],a)) {
             if (!inside_a_set) dest[j++]='[';
             dest[j++]=src[i];
             if (inside_a_set && src[i+1]=='-') {
                 /* Special case:
                  * if we had [a-d], we don't want to turn it into
                  * [aA-dD], but rather into [a-dA-D]. In such a case,
                  * we just use u_toupper
                  */
                 i=i+2;
                 dest[j++]='-';
                 dest[j++]=src[i++];
                 dest[j++]=u_toupper(dest[i-3]);
                 dest[j++]='-';
                 dest[j++]=u_toupper(src[i-1]);
                 continue;
             }

             if (a==NULL) {
                /* If there is no alphabet file, we just consider the unique
                 * uppercase variant of the letter */
                dest[j++]=u_toupper(src[i]);
             } else {
                /* If there is an alphabet file, we use it */
                unichar* tbrowse = NULL;
                int i_pos_in_array_of_string = a->pos_in_represent_list[src[i]];
                if (i_pos_in_array_of_string != 0) {
                   tbrowse = a->t_array_collection[i_pos_in_array_of_string];
                }
                if (tbrowse != NULL) {
                   while ((*tbrowse) != '\0') {
                      dest[j++]=*(tbrowse++);
                   }
                }
             }
             if (!inside_a_set) dest[j++]=']';
             i++;
         }
          else {
             /* Not a lower case letter */
             dest[j++]=src[i++];
          }
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
int get_longuest_prefix_ignoring_case(const unichar* a,const unichar* b,const Alphabet* alphabet) {
if (a==NULL || b==NULL) {
   return 0;
}
int i=0;
while (is_equal_or_uppercase(b[i],a[i],alphabet) && a[i]!='\0') i++;
return i;
}


int load_persistent_alphabet(const char* name) {
VersatileEncodingConfig vec=VEC_DEFAULT;
Alphabet* a=load_alphabet(&vec,name);
if (a==NULL) return 0;
set_persistent_structure(name,a);
return 1;
}


void free_persistent_alphabet(const char* name) {
Alphabet* a=(Alphabet*)get_persistent_structure(name);
set_persistent_structure(name,NULL);
free_alphabet(a);
}

}
