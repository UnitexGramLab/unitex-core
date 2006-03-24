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

/********************************************************/
/*  NOM : convcomm.c               */
/*  analyse de la ligne de commande          */
/*  Date  : juillet 1998               */
/*  Auteur : Eric Laporte         */

#ifdef __cplusplus
//extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "general.h"
#include "convcomm.h"

static int trouveOption(tConvComm * conventions, char ident);

  
  /* reste a traiter le cas ou separees == 0 */

void lireLigneComm(tConvComm * conventions, tLigneComm * ligne, int c, char * argv[]) {
   int n ; /* nombre d'elements de la ligne de commande deja traites */
   int option, ident ;
if (! conventions -> nbOpt)
   return ;
for (n = conventions -> nbOpt ; n ; ) {
   n -- ;
   ligne -> activee[n] = 0 ;
   ligne -> parametre[n] = 0 ; }  /* quand on sort du for, n == 0 */
if (conventions -> identif[0][0] == 0 && c > 1) /* parametre principal */
   switch(conventions -> tiret) {
      case 'o' : case 'f' :
         if (argv[1][0] != '-') {
            ligne -> activee[0] = 1 ;
            ligne -> parametre[0] = argv[1] ;
            n = 1 ; }
      break ;
      case 'i' :
         ligne -> activee[0] = 1 ;
         ligne -> parametre[0] = argv[1] ;
         n = 1 ;
      break ;
      default : printf("Erreur interne [lireLigneComm].\n") ; exit(0) ; }
if (conventions -> separees)
   switch(conventions -> tiret) {
      case 'o' :
         while (++ n < c) {
            if (argv[n][0] == '-' && strlen(argv[n]) == 2)
               option = trouveOption(conventions, argv[n][1]) ;
            else {
               printf("Option illÝgale : %s.\n", argv[n]) ;
               exit(0) ; }
            ligne -> activee[option] = 1 ;
            if (conventions -> parametre[option] && ++ n < c)
               ligne -> parametre[option] = argv[n] ; }
      break ;
      case 'f' :
         while (++ n < c) {
            if (argv[n][0] == '-' && strlen(argv[n]) == 2)
               option = trouveOption(conventions, argv[n][1]) ;
            else if (strlen(argv[n]) == 1)
               option = trouveOption(conventions, argv[n][0]) ;
            else {
               printf("Option illÝgale : %s.\n", argv[n]) ;
               exit(0) ; }
            ligne -> activee[option] = 1 ;
            if (conventions -> parametre[option] && ++ n < c)
               ligne -> parametre[option] = argv[n] ; }
      break ;
      case 'i' :
         while (++ n < c) {
            if (strlen(argv[n]) == 1)
               option = trouveOption(conventions, argv[n][0]) ;
            else {
               printf("Option illÝgale : %s.\n", argv[n]) ;
               exit(0) ; }
            ligne -> activee[option] = 1 ;
            if (conventions -> parametre[option] && ++ n < c)
               ligne -> parametre[option] = argv[n] ; }
      break ;
      default : printf("Erreur interne [lireLigneComm].\n") ; exit(0) ; }
else   /* conventions -> separees == 0 */
   switch(conventions -> tiret) {
      case 'o' :
         while (++ n < c) {
            if (argv[n][0] == '-' && strlen(argv[n]) > 1)
               if (strlen(argv[n]) == 2) {
                  option = trouveOption(conventions, argv[n][1]) ;
                  ligne -> activee[option] = 1 ;
                  if (conventions -> parametre[option] && ++ n < c)
                     ligne -> parametre[option] = argv[n] ; }
               else
                  for (ident = 1 ; argv[n][ident] ; ident ++) {
                     option = trouveOption(conventions, argv[n][ident]) ;
                     ligne -> activee[option] = 1 ; }
            else {
               printf("Option illÝgale : %s.\n", argv[n]) ;
               exit(0) ; } }
      break ;
      case 'f' :
         while (++ n < c) {
            if (argv[n][0] != '-')
               if (strlen(argv[n]) == 1) {
                  option = trouveOption(conventions, argv[n][0]) ;
                  ligne -> activee[option] = 1 ;
                  if (conventions -> parametre[option] && ++ n < c)
                     ligne -> parametre[option] = argv[n] ; }
               else
                  for (ident = 1 ; argv[n][ident] ; ident ++) {
                     option = trouveOption(conventions, argv[n][ident]) ;
                     ligne -> activee[option] = 1 ; }
            else if (strlen(argv[n]) == 2) {
               option = trouveOption(conventions, argv[n][1]) ;
               ligne -> activee[option] = 1 ;
               if (conventions -> parametre[option] && ++ n < c)
                  ligne -> parametre[option] = argv[n] ; }
            else if (strlen(argv[n]) > 2)
               for (ident = 1 ; argv[n][ident] ; ident ++) {
                  option = trouveOption(conventions, argv[n][ident]) ;
                  ligne -> activee[option] = 1 ; }
            else {
               printf("Option illÝgale : %s.\n", argv[n]) ;
               exit(0) ; } }
      break ;
      case 'i' :
         while (++ n < c) {
            if (strlen(argv[n]) == 1) {
               option = trouveOption(conventions, argv[n][0]) ;
               ligne -> activee[option] = 1 ;
               if (conventions -> parametre[option] && ++ n < c)
                  ligne -> parametre[option] = argv[n] ; }
            else
               for (ident = 1 ; argv[n][ident] ; ident ++) {
                  option = trouveOption(conventions, argv[n][ident]) ;
                  ligne -> activee[option] = 1 ; } }
      break ;
      default : printf("Erreur interne [lireLigneComm].\n") ; exit(0) ; } }

static int trouveOption(tConvComm * conventions, char ident) {
   int option, variante ;
for (option = 0 ; option < conventions -> nbOpt ; option ++)
   for (variante = 0 ;
        variante < maxVar && conventions -> identif[option][variante] ;
        variante ++)
      if (conventions -> identif[option][variante] == ident)
         return option ;
printf("Option %c inconnue\n", ident) ;
exit(0) ;
}

#ifdef __cplusplus
//}
#endif

