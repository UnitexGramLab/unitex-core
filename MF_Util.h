/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
/* Operations sur des strings unicode			        */
/****************************************************************/

#ifndef UtilH
#define UtilH

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "Unicode.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define MAX_STR_LEN 2000  //Maximum length of strings treated by this library

/****************************************************************/
/* Retourner 1 si str1 contient str2, sinon 0.			*/
int u_contient(const unichar *str1,const unichar *str2);

/****************************************************************/
/* Retourne 1 si str1 est une permutation de str2, sinon 0.  	*/
int u_is_permut(const unichar *str1,const unichar *str2);

/****************************************************************/
/* Verifier si la chaine unicode str contient le caractere c.	*/
int u_is_in(unichar c,const unichar *str);

/****************************************************************/
/* Verifier si la chaine char str contient le caractere c.	*/
int u_is_in_char(unichar c,const char *str);

/********************************************************************************/
/* Scan the prefix of "source" string until the first non protected             */
/* deliminator belonging to "delim" or the end of "source".                     */        
/* The scanned sequence is copied to "dest" which must have its space allocated */
/* (at least max+1 unichars).                                                   */
/* The length of the copied sequence is no higner than "max" unicode characters.*/
/* If "eliminate_bcksl" is set to 1 each protecting backslash is omitted in the */
/* copied sequence.                                                             */
/* Return the length of the scanned sequence.                                   */
int u_scan_until(unichar *dest,const unichar *source, int max,const unichar *delim, int eliminate_bcksl);

/***********************************************************************************************************/
/* Scan the prefix of "source" string until the first non protected deliminator belonging to "delim"       */
/* or the end of "source". the scanned sequence is copied to "dest" which must have its space allocated.   */
/* (at least max+1 unichars). The length of the copied sequence is no higner than "max" unicode characters.*/
/* If "eliminate_bcksl" is set to 1 each protecting backslash is omitted in the copied sequence.           */
/* Return the length of the scanned sequence.                                                              */
int u_scan_until_char(unichar *dest,const unichar *source, int max,const char *delim, int eliminate_bcksl);

/********************************************************************************/
/* Scan the prefix of "source" string as long as the current character belongs  */
/* to 'admitted'.                                                               */
/* The scanned sequence is copied to "dest" which must have its space allocated */
/* (at least max+1 unichars).                                                   */
/* The length of the copied sequence is no higher than "max" unicode characters.*/
/* Return the length of the scanned sequence.                                   */
int u_scan_while(unichar *dest,const unichar *source, int max,const unichar *admitted);

/********************************************************************************/
/* Scan the prefix of "source" string as long as the current character belongs  */
/* to 'admitted'.                                                               */
/* The scanned sequence is copied to "dest" which must have its space allocated */
/* (at least max+1 unichars).                                                   */
/* The length of the copied sequence is no higher than "max" unicode characters.*/
/* Return the length of the scanned sequence.                                   */
int u_scan_while_char(unichar *dest,const unichar *source, int max,const char *admitted);


/****************************************************************/
/* Trier un tableau de chaines de charact�res et enlever les	*/
/* doublons (bubble sort). len = longeur d'un �l�ment du tableau*/
void u_tri_a(void *TAB, int *cnt, int len);

///////////////////////////////////////////////////////////
//Table of unicode strings with its cardinality
typedef struct {
  unichar** t;
  int n;       
} unitab_t;

//////////////////////////////////////////////////////////
// Add a new unichar string to the dynamic unichar table
// while reallocating memory space and updating the 
// cardinality. Space for the new form must be allocated.
// Returns 0 on success, 1 otherwise.
int u_add_unitab_elem(unichar* form,unitab_t* tab);

///////////////////////////////////////////////////////////////////////////
// Print the contents of a table of unicode strings to the standard output.
void u_print_unitab(unitab_t tab);

} // namespace unitex

#endif
