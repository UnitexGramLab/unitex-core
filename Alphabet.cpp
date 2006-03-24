 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

//---------------------------------------------------------------------------
#include "Alphabet.h"
//---------------------------------------------------------------------------



Alphabet* new_alphabet() {
Alphabet* a=(Alphabet*)malloc(sizeof(Alphabet));
for (int i=0;i<0x10000;i++) {
    a->t[i]=NULL;
    a->t2[i]=0;
}
return a;
}


void free_alphabet(Alphabet* a) {
free(a);
}


void ajouter_min_maj(Alphabet* a,unichar min,unichar maj) {
if (a->t[min]==NULL) {
   a->t[min]=(unichar*)malloc(2*sizeof(unichar));
   a->t[min][0]=maj;
   a->t[min][1]='\0';
   return;
}
int L=u_strlen(a->t[min]);
a->t[min]=(unichar*)realloc(a->t[min],(L+2)*sizeof(unichar));
a->t[min][L]=maj;
a->t[min][L+1]='\0';
}



Alphabet* load_alphabet(char* n) {
FILE* f;
f=u_fopen(n,U_READ);
if (f==NULL) return NULL;
Alphabet* a=new_alphabet();
int c;
unichar min,maj;
while ((c=u_fgetc(f))!=EOF) {
      maj=(unichar)c;
      if (maj=='#') {
         // we are in the case of an interval #AZ -> [A..Z]
         min=(unichar)u_fgetc(f);
         maj=(unichar)u_fgetc(f);
         if (min>maj) {
            fprintf(stderr,"Error in alphabet file: for an interval like #AZ, A must be before Z\n");
            u_fclose(f);
            return NULL;
         }
         for (c=min;c<=maj;c++) {
           a->t2[c]=(char)(a->t2[c] | 1);
           a->t2[c]=(char)(a->t2[c] | 2);
           ajouter_min_maj(a,(unichar)c,(unichar)c);
         }
         u_fgetc(f); // reading the \n
      }
      else {
        a->t2[maj]=(char)(a->t2[maj] | 1);
        min=(unichar)u_fgetc(f);
        if (min!='\n') {
          a->t2[min]=(char)(a->t2[min] | 2);
          u_fgetc(f); // reading the \n
          ajouter_min_maj(a,min,maj);
        }
        else {
          // we are in the case of a single (no min/maj distinction like in thai)
          a->t2[maj]=(char)(a->t2[maj] | 2);
          ajouter_min_maj(a,maj,maj);
        }
      }
}
u_fclose(f);
return a;
}



int is_upper_of(unichar min,unichar maj,Alphabet* a) {
if (a->t[min]==NULL) return 0;
int i=0;
while (a->t[min][i]!='\0') {
      if (a->t[min][i]==maj) return 1;
      i++;
}
return 0;
}


int is_equal_or_case_equal(unichar dic_letter,unichar text_letter,Alphabet* a) {
return (dic_letter==text_letter || is_upper_of(dic_letter,text_letter,a));
}


int is_upper(unichar c,Alphabet* a) {
return (a->t2[c] & 1);
}



int is_lower(unichar c,Alphabet* a) {
return (a->t2[c] & 2);
}


int is_letter(unichar c,Alphabet* a) {
return is_upper(c,a)||is_lower(c,a);
}

int all_are_letters(unichar* s,Alphabet* a) {
for (int i=0;s[i]!='\0';i++) {
   if (!is_letter(s[i],a)) return 0;
}
return 1;
}


int all_in_lower(unichar* s,Alphabet* a) {
int i=0;
while (s[i]!='\0') {
  if (!is_lower(s[i],a)) return 0;
  i++;
}
return 1;
}



int all_in_upper(unichar* s,Alphabet* a) {
int i=0;
while (s[i]!='\0') {
  if (!is_upper(s[i],a)) return 0;
  i++;
}
return 1;
}



int is_equal_ignore_case_and_quotes(unichar* dic,unichar* text,Alphabet* a) {
int i=0;
while (dic[i] && (is_equal_or_case_equal(dic[i],text[i],a)
                  || are_equivalent_quotes(dic[i],text[i]))) {i++;}
return (dic[i]=='\0' && text[i]=='\0');
}


int is_equal_ignore_case(unichar* dic,unichar* text,Alphabet* a) {
int i=0;
while (dic[i] && is_equal_or_case_equal(dic[i],text[i],a)) {i++;}
return (dic[i]=='\0' && text[i]=='\0');
}



//
// this function turns a Portuguese letter sequence into a lowercase one
// it cannot be a general function, because of potential ambiguities
// like A -> a or à in French
// it works on Portuguese because the uppercase/lowercase are bijectives
//
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



//
// replace ".+e" par ".+[eE]" for regular expressions
//
void replaceLetterByLetterSet(Alphabet* a,unichar* dest,unichar* src) {
int i=0,j=0;
char inside_a_set=0;
while (src[i]!='\0') {
   if (src[i]=='\\') {
      dest[j++]=src[i++];
      if (src[i]=='\0') {
         // is there is nothing after a backslash, then we stop,
         // and the RE compiler may indicate an error 
         dest[j]='\0';
         return;
      }
      else {
         dest[j++]=src[i++];
      }
   }
   if (src[i]=='[') {
      dest[j++]=src[i++];
      inside_a_set=1;
   }
   else if (src[i]==']') {
           dest[j++]=src[i++];
           inside_a_set=0;
        }
   else if (a->t2[src[i]] & 2) {
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
dest[j]='\0';
}


