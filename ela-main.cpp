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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "general.h"
#include "convcomm.h"
#include "autalmot.h"
#include "list_aut.h"
#include "entrsort.h"
#include "evamble.h"

static tConvComm convComm ;
static tLigneComm ligneComm ;

int main(int argc, char * argv []) {
   tChargeurPhrases * avant, * apres ;
convComm.nbOpt = 3 ;
convComm.identif[0][0] = 0 ; /* parametre principal : premier fichier d'entree */
convComm.parametre[0] = 1 ;
strcpy(convComm.identif[2], "e") ; /* fichier d'erreurs */
convComm.parametre[2] = 1 ;
strcpy(convComm.identif[1], "s") ; /* deuxieme fichier d'entree */
convComm.parametre[1] = 1 ;
convComm.tiret = 'o' ;
convComm.separees = 1 ;
lireLigneComm(& convComm, & ligneComm, argc, argv) ;
if (ligneComm.activee[2] && ligneComm.parametre[2]) {
   fErr = fopen(ligneComm.parametre[2], "w") ;
   if (! fErr) {
      fprintf(stderr,"\nError: opening file %s\n", ligneComm.parametre[2]) ;
      return 0 ; } }
else {
      fprintf(stderr,"\nTo use Evamble, specify name of error file.\n") ;
      return 0 ; }
if (! ligneComm.activee[0] || ! ligneComm.parametre[0]) {
	fprintf(fErr, "\nError: name of first file not found\n") ;
	fclose(fErr) ;
	exit(1) ; }
avant = evamble(ligneComm.parametre[0]) ;
apres = NULL ;
if (ligneComm.activee[1] && ligneComm.parametre[1]) {
	apres = evamble(ligneComm.parametre[1]) ;
	compareAmbLex(avant, apres) ;
	destrChargeurPhrases(apres) ; }
destrChargeurPhrases(avant) ;
fclose(fErr) ;
return 0 ; }
