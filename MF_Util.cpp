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

/* Created by Agata Savary (savary@univ-mlv.fr)
 */

/****************************************************************/
/* Operations sur des chaines de caracteres			*/
/****************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "MF_Util.h"
#include "Alphabet.h"
#include "Error.h"

namespace unitex {

/****************************************************************/
/* Verifier si le caractere c a un correspondant parmi les 	*/
/* elements de la chaine qui ne sont pas encore marqu�s dans 	*/
/* "where". Si oui, inscrire 1 dans "where" a la position du	*/
/* correspondant et returner cette position. Sinon retourner -1.*/
int u_member(unichar c, const unichar *str, int *where) {

   unsigned int i;

   for (i=0; i<u_strlen(str); i++)
      if ( (c == str[i]) && (!where[i])) {
         where[i] = 1;
         return (i);
      }

   return (-1);

}

/****************************************************************/

/****************************************************************/
/* Retourner 1 si str1 contient str2, sinon 0.			*/
int u_contient(const unichar *str1, const unichar *str2) {

   unsigned int len, i;
   int w, where[MAX_STR_LEN];

   /*Si str1 plus court que str2, alors str1 ne contient pas str2*/
   if (u_strlen(str1) < (len = u_strlen(str2)))
      return (0);

   /*Initialiser le tableau d'elements de str1, qui ont des*/
   /*correspondants dans str2				*/
   for (i=0; i<u_strlen(str1); i++)
      where[i] = 0;
   for (i=u_strlen(str1); i<MAX_STR_LEN; i++)
      where[i] = -1;

   /*Chercher chaque element de str2 dans str1 et marquer les elements  */
   /* de str1 pour lesquels des correspondants ont ete trouves dans str2*/
   for (i=0; i<len; i++)
      /*Si un element de str2 n'a pas de correspondant dans*/
      /*str1 les deux chaines ne sont pas des permutations */
      if ((w = u_member(str2[i], str1, where)) == -1)
         return (0);

   return (1);
}

/****************************************************************/

/****************************************************************/
/* Retourner 1 si str1 est une permutation de str2, sinon 0.  	*/
int u_is_permut(const unichar *str1, const unichar *str2) {

   /*Si longeurs differentes - ce ne sont pas des permutations*/
   if (u_strlen(str1) != u_strlen(str2))
      return (0);

   return (u_contient(str1, str2));

}

/****************************************************************/

/****************************************************************/
/* Verifier si la chaine unicode str contient le caractere c.	*/
int u_is_in(unichar c, const unichar *str) {

   while (*str != 0) {
      if (*str == c)
         return (1);
      str++;
   }
   return (0);
}
/****************************************************************/

/****************************************************************/
/* Verifier si la chaine char str contient le caractere c.	*/
int u_is_in_char(unichar c, const char *str) {

   while (*str != 0) {
      if ((unichar) *str == c)
         return (1);
      str++;
   }
   return (0);
}
/****************************************************************/

/********************************************************************************/
/* Scan the prefix of "source" string until the first non protected             */
/* deliminator belonging to "delim" or the end of "source".                     */
/* The scanned sequence is copied to "dest" which must have its space allocated */
/* (at least max+1 unichars).                                                   */
/* The length of the copied sequence is no higher than "max" unicode characters.*/
/* If "eliminate_bcksl" is set to 1 each protecting backslash is omitted in the */
/* copied sequence.                                                             */
/* Return the length of the scanned sequence (which may be bigger than the      */
/* length the copied sequence if backslashes appeared and were to be elimitated).*/
int u_scan_until(unichar *dest, const unichar *source, int max,
      const unichar *delim, int eliminate_bcksl) {
   int s=0; //index of the next element in 'source'
   int d=0; //index of the next element in 'dest'
   int end=0;
   int bcksl_precedes=0; //1 if the character preceding the current one was a backslash
   int no_elim_bcksl=0; //number of eliminated backslashes

   while (!end && (d<max) && (source[s] != 0))
      if (source[s] == (unichar) '\\')
         if (!bcksl_precedes) {
            bcksl_precedes = 1;
            s++;
         } else if (u_is_in(source[s], delim))
            end=1;
         else {
            if (!eliminate_bcksl)
               dest[d++] = (unichar) '\\';
            else
               no_elim_bcksl++;
            dest[d++] = source[s++];
            bcksl_precedes = 0;
         }
      else {//Current character is not a backslash
         if (u_is_in(source[s], delim))
            end=1;
         else {
            if (bcksl_precedes) {
               if (!eliminate_bcksl)
                  dest[d++] = (unichar) '\\';
               else
                  no_elim_bcksl++;
            }
            dest[d++] = source[s++];
            bcksl_precedes = 0;
         }
      }
   dest[d] = (unichar) '\0';
   return d+no_elim_bcksl;
}

/****************************************************************/

/***********************************************************************************************************/
/* Scan the prefix of "source" string until the first non protected deliminator belonging to "delim"       */
/* or the end of "source". the scanned sequence is copied to "dest" which must have its space allocated.   */
/* (at least max+1 unichars). The length of the copied sequence is no higher than "max" unicode characters.*/
/* If "eliminate_bcksl" is set to 1 each protecting backslash is omitted in the copied sequence.           */
/* Return the length of the scanned sequence (which may be bigger than the length the copied sequence if   */
/* backslashes appeared and were to be elimitated).                                                        */
int u_scan_until_char(unichar *dest, const unichar *source, int max,
      const char *delim, int eliminate_bcksl) {
   int s=0; //index of the next element in 'source'
   int d=0; //index of the next element in 'dest'
   int end=0;
   int bcksl_precedes=0; //1 if the character preceding the current one was a backslash
   int no_elim_bcksl=0; //number of eliminated backslashes

   while (!end && (d<max) && (source[s] != 0))
      if (source[s] == (unichar) '\\')
         if (!bcksl_precedes) {
            bcksl_precedes = 1;
            s++;
         } else if (u_is_in_char(source[s], delim))
            end=1;
         else {
            if (!eliminate_bcksl)
               dest[d++] = (unichar) '\\';
            else
               no_elim_bcksl++;
            dest[d++] = source[s++];
            bcksl_precedes = 0;
         }
      else {//Current character is not a backslash
         if (u_is_in_char(source[s], delim))
            end=1;
         else {
            if (bcksl_precedes) {
               if (!eliminate_bcksl)
                  dest[d++] = (unichar) '\\';
               else
                  no_elim_bcksl++;
            }
            dest[d++] = source[s++];
            bcksl_precedes = 0;
         }
      }
   dest[d] = (unichar) '\0';
   return d+no_elim_bcksl;
}
/****************************************************************/

/********************************************************************************/
/* Scan the prefix of "source" string as long as the current character belongs  */
/* to 'admitted'.                                                               */
/* The scanned sequence is copied to "dest" which must have its space allocated */
/* (at least max+1 unichars).                                                   */
/* The length of the copied sequence is no higher than "max" unicode characters.*/
/* Return the length of the scanned sequence.                                   */
int u_scan_while(unichar *dest, const unichar *source, int max,
      const unichar *admitted) {
   int i=0;
   while (i<max && *source!=0 && u_is_in(*source, admitted)) {
      dest[i++] = *source;
      source++;
   }
   dest[i] = (unichar) '\0';
   return i;
}

/****************************************************************/

/********************************************************************************/
/* Scan the prefix of "source" string as long as the current character belongs  */
/* to 'admitted'.                                                               */
/* The scanned sequence is copied to "dest" which must have its space allocated */
/* (at least max+1 unichars).                                                   */
/* The length of the copied sequence is no higher than "max" unicode characters.*/
/* Return the length of the scanned sequence.                                   */
int u_scan_while_char(unichar *dest, const unichar *source, int max,
      const char *admitted) {
   int i=0;
   while (i<max && *source!=0 && u_is_in_char(*source, admitted)) {
      dest[i++] = *source;
      source++;
   }
   dest[i] = (unichar) '\0';
   return i;
}

/****************************************************************/

/****************************************************************/
/* Trier un tableau de chaines de charact�res et enlever les	*/
/* doublons (bubble sort). len = longeur d'un �l�ment du tableau*/
void u_tri_a(void *T, int *cnt, int len) {
   int i, j;
   unichar tmp[MAX_STR_LEN];
   unichar *TAB = (unichar*) T;

   if (*cnt < 1)
      return;

   /* Trier avec doublons (bubble sort) */
   for (i=1; i<*cnt; i++) {
      j = i;
      while ((0 < j) && (u_strcmp(TAB + (j-1)*len, TAB + j*len) >= 0)) {
         u_strcpy(tmp, TAB + j*len);
         u_strcpy(TAB + j*len, TAB + (j-1)*len);
         u_strcpy(TAB + (j-1)*len, tmp);
         j--;
      }
   }

   /* Enlever les doublons */
   for (i=0, j=1; i<*cnt; i++) {
      /* Omettre tous les �l�ments �guax � TAB[i].*/
      while ((j<*cnt) && (!u_strcmp(TAB + i*len, TAB + j*len)))
         j++;
      if (j == *cnt) // Fin du tableau
      {
         *cnt = i+1;
         u_strcpy(TAB + (*cnt)*len, "");
      } else {
         i++;
         if (i != j)
            u_strcpy(TAB + i*len, TAB + j*len);
         j++;
      }
   }
}

//////////////////////////////////////////////////////////
// Add a new unichar string to the dynamic unichar table
// while reallocating memory space and updating the 
// cardinality. Space for the new form must be allocated.
// Returns 0 on success, 1 otherwise.
int u_add_unitab_elem(unichar* form, unitab_t* tab) {
   tab->n++;

   //If the table doesn't exist
   if ((tab->n) == 1) {
      tab->t = (unichar**) malloc(sizeof(unichar**));
      if (!(tab->t)) {
         fatal_alloc_error("u_add_unitab_elem");
      }
   } else {
      tab->t = (unichar**) realloc(tab->t, tab->n * sizeof(unichar*));
      if (!(tab->t)) {
         fatal_alloc_error("u_add_unitab_elem");
      }
   }
   tab->t[tab->n-1]=u_strdup(form);
   return 0;
}

///////////////////////////////////////////////////////////////////////////
// Print the contents of a table of unicode strings to the standard output.
void u_print_unitab(unitab_t tab) {
   int i;
   u_printf("\n");
   for (i=0; i<tab.n; i++) {
      u_printf("%S ", tab.t[i]);
   }
   u_printf("\n");
}

} // namespace unitex
