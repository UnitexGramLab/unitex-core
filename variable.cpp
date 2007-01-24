 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include <ctype.h>
#include "general.h"
#include "autalmot.h"
#include "deter.h"
#include "variable.h"

#include "utils.h"

static int appart_negatif(tSymbole * etiq_a, tSymbole * etiq_b) ;
static BOOL completPlusFinQuInc(tSymbole * etiq_a, tSymbole * etiq_b) ;
static tCode *traduit_symbole_en_code (tSymbole * s) ;
static tSymbole * traduit_code_en_symbole (tSymbole *r, tCode *code) ;
static tCode * Inter_Code (tCode *code1, tCode *code2) ;
static tSymbole * incANInterIncAN(tSymbole * etiq_a, tSymbole * etiq_b) ;
static tSymbole * incInterInc(tSymbole * etiq_a , tSymbole * etiq_b) ;
static tSymbole * codeInterInc(tSymbole * etiq_a , tSymbole * etiq_b) ;
static tSymbole * codeInterCode(tSymbole * etiq_a , tSymbole * etiq_b) ;
static tSymbole * negInterNeg(tSymbole * etiq_a, tSymbole * etiq_b ) ;
static BOOL incPlusFinQuInc(tSymbole * etiq_a, tSymbole * etiq_b) ;
static alphabet * codSaufCod(tSymbole * a, tSymbole * b) ;
static alphabet * codSaufInc(tSymbole * a, tSymbole * b) ;
static alphabet * codSaufNeg(tSymbole * a, tSymbole * b) ;
static alphabet * incSaufCod(tSymbole * a, tSymbole * b) ;
static alphabet * incSaufInc(tSymbole * a, tSymbole * b) ;
static alphabet * incSaufNeg(tSymbole * a, tSymbole * b) ;
static alphabet * negSaufNeg(tSymbole * a, tSymbole * b) ;
static alphabet * tCodeSaufTCode(tCode * c1, tCode * c2) ;
static alphabet * ANsaufAN(tCode * c1, tCode * c2) ;
static alphabet * VsaufV(tCode * c1, tCode * c2) ;
static BOOL plusFin(tCode * c1, tCode * c2) ;
static alphabet * unionListesDisj(alphabet * a1, alphabet * a2) ;
static alphabet * univSaufInc(tSymbole * b) ;		  
static alphabet * univSaufNeg(tSymbole * b) ;		  
static alphabet * univSaufCod(tSymbole * b) ;		  
static alphabet * univSaufAt(tSymbole * b) ;		  
static alphabet * codSaufAt(tSymbole * a, tSymbole * b) ;
static alphabet * incSaufAt(tSymbole * a, tSymbole * b) ;
//static void affCode(tCode * c) ;
static tListe * separeFormes(tSymbole * s) ;
static tListe * trier(tListe * liste) ;

static char * CG[nbCatGramm] = {
  "A", "ADV", "CONJC", "CONJS", "DET", "DET:fp", "DET:fs", "DET:mp", "DET:ms",
  "INTJ", "N", "PFX", "PNC",
  "PPV", "PPV:1p", "PPV:1s", "PPV:2p", "PPV:2s",
  "PPV:3fp", "PPV:3fs", "PPV:3mp", "PPV:3ms", "PPV:3p", "PPV:3s",
  "PREP",
  "PRO", "PRO:1p", "PRO:1s", "PRO:2p", "PRO:2s",
  "PRO:3fp", "PRO:3fs", "PRO:3mp", "PRO:3ms", "PRO:3p", "PRO:3s",
  "PRO:fp", "PRO:fs", "PRO:mp", "PRO:ms",
  "V", "XI", "XI:1s", "XV", "?"
};










/* etiqa est un atome et etiqb est un symbole incomplet
 * renvoie vrai si a appartient a b (est plus fin que)
 */


static int appart_inc(tSymbole * a, tSymbole * b) {

  int i;

  if (*b->canonique) {
    if (u_strcmp(a->canonique, b->canonique) != 0) { return 0; }
  }

  for (i = 0; a->gramm[i] != 0 && a->gramm[i] != ':'; i++) {
    if (a->gramm[i] != b->gramm[i]) { return 0; }
  }

  if (b->gramm[i] == 0) { return 1; }

  if (a->gramm[i] == 0) { // sanity check, not useful regarding next test
    error("appart_inc: inc symbol finest than atom???\n");
    Affiche_Symbole(a); fprintf(stderr, " and ");  Affiche_Symbole(b); fprintf(stderr, "\n");
    return 0;
  }

  if (b->gramm[i] != a->gramm[i]) { return 0; }

  i++;

  /* compare flexional code, all characters in b should be present in a */

  for (int j = i;  b->gramm[j]; j++) {

    for (int k = i; a->gramm[k] != b->gramm[j]; k++) {

      if (a->gramm[k] == 0) {
	// debug("appart_inc: char %C from ", b->gramm[j]); Affiche_Symbole(b); fprintf(stderr, " not found in "); Affiche_Symbole(a);
	// fprintf(stderr, "\n");
	return 0;
      }
    }

  }

  return 1;
}







/* etiq_a doit etre un atome, etiq_b une etiquette ensembliste
 * renvoie 1 si etiqa appartient a l'ensemble etiqb, et 0 sinon 
 */

int appartient(tSymbole * etiqa, tSymbole * etiqb) {


  //  debug("apartient("); Affiche_Symbole(etiqa); fprintf(stderr, ", "); Affiche_Symbole(etiqb); fprintf(stderr, ")\n");

  if (etiqa->sorteSymbole != ATOME) {
    error("appartient: l'etiquette (a) n'est pas un atome !!!\n");
    return 0 ;
  }

  switch(etiqb->sorteSymbole) {

  case ATOME : /* ON NE COMPARE PAS DEUX ETIQUETTES */
    printf("erreur(appartient) : L'etiquette (b) n'est pas ensembliste !!!\n") ;
    return 0 ;
    break ;

  case UNIVERSEL : 
    return 1 ;
    break ;

  case CODE : 
    return ! u_strcmp(etiqa -> gramm, etiqb -> gramm) ;
    break ;

  case NEGATIF :  
    return appart_negatif(etiqa, etiqb);
    break ;

  case INCOMPLET : 
    return appart_inc(etiqa, etiqb);
    // return completPlusFinQuInc(etiqa,etiqb) ;
    break ;

  default :
    printf("Erreur interne [appartient].\n") ;
    return 0 ;
  }
}




/* etiq_a est un atome 
 * renvoie 1 si etiqa appartient a etiqb, et 0 sinon
 */

int appartientBis(tSymbole * etiqa, tSymbole * etiqb) {

  switch (etiqb->sorteSymbole) {

  case ATOME:
    return ! u_strcmp(etiqa->canonique, etiqb->canonique) && ! u_strcmp(etiqa->gramm, etiqb->gramm);

  case UNIVERSEL:
    debug("UNIVERSEL\n");
    return 1;

  case CODE:
    return ! u_strcmp(etiqa->gramm, etiqb->gramm);

  case NEGATIF:
    return appart_negatif(etiqa, etiqb);

  case INCOMPLET:
    return appart_inc(etiqa, etiqb);
    // return completPlusFinQuInc(etiqa, etiqb);

  default:
    printf("Erreur interne [appartientBis].\n") ;
    return 0 ;
  }
}



/* etiq_a est un atome, etiq_b est une etiquette negative 
 * renvoie 1 ssi etiq_a->canonique ne figure pas dans etiq_b->canonique 
 *       et  etiq_a->gramm et etiq_b->gramm sont egaux
 * renvoie 0 sinon						
 */

static int appart_negatif(tSymbole * etiq_a, tSymbole * etiq_b) {

  unichar * chaine, * tampon;

  if (u_strcmp(etiq_a->gramm, etiq_b->gramm)) { return 0; }

  tampon = (unichar *) xcalloc(1 + u_strlen(etiq_b->canonique), sizeof(unichar));

  u_strcpy(tampon, etiq_b->canonique);
  chaine = u_strtok_char(tampon, "!");

  while (chaine) {
    if (! u_strcmp(etiq_a->canonique, chaine)) {
      free(tampon);
      return 0;
    }
    chaine = u_strtok_char(NULL, "!");
  }

  free(tampon);
  return 1;
}




/* FONCTION  PRINCIPALE D'INTERSECTION DE 2 ETIQUETTES ENSEMBLISTES
 * si compatibilite grammaticale des 2 etiquettes
 * alors retourne une etiquette sinon retourne le pointeur NULL
 */

tSymbole * inter(tSymbole * a , tSymbole * b) {

  tSymbole * r = NULL;


  //  Affiche_Symbole(a); fprintf(stderr, " inter "); Affiche_Symbole(b); fprintf(stderr, "\n");

  switch (a->sorteSymbole) {

  case ATOME:

    switch(b->sorteSymbole) {

    case ATOME:
      r = (compSymb(a, b) ? NULL : copieSymbole(a));
      break ;

    case UNIVERSEL:
      r = copieSymbole(a);
      break ;

    case INCOMPLET:
    case NEGATIF:
    case CODE:
      r = (appartient(a, b) ? copieSymbole(a) : NULL);
      break ;

    default : printf("Erreur interne [inter].\n") ;
    }
    break ;

  case UNIVERSEL:
    r = copieSymbole(b);
    break ;

  case INCOMPLET:  
    switch (b->sorteSymbole) {

    case ATOME:
      r = (appartient(b, a) ? copieSymbole(b) : NULL) ;
      break ;

    case UNIVERSEL:
      r = copieSymbole(a) ;		  
      break ;

    case INCOMPLET:
      r = incInterInc(a, b) ;
      break ;

    case NEGATIF:
    case CODE:
      r = codeInterInc(b,a) ;
      break ;

    default:
      printf("Erreur interne [inter].\n") ;
    }
    break ;


  case NEGATIF:     
    switch (b->sorteSymbole) {

    case ATOME :
      r = (appartient(b, a) ? copieSymbole(b) : NULL) ;
      break ;

    case UNIVERSEL:
      r = copieSymbole(a) ;
      break ;

    case INCOMPLET:
      r = codeInterInc(a, b) ;
      break ;
    case NEGATIF :
      r = negInterNeg(a, b) ;
      break ;
    case CODE :
      r = codeInterCode(b, a) ;
      break ;

    default :
      printf("Erreur interne [inter].\n") ;
    }
    break ;

  case CODE :       
    switch (b->sorteSymbole) {
    case ATOME :
      r = (appartient(b, a) ? copieSymbole(b) : NULL) ;
      break ;
    case UNIVERSEL:  r = copieSymbole(a) ;
      break ;
    case INCOMPLET:  r = codeInterInc(a, b) ;
      break ;
    case NEGATIF: case CODE:
      r = codeInterCode(a, b) ;
      break ;
    default:
      printf("Erreur interne [inter].\n") ;
    }
    break ;

  default:
    printf("Erreur interne [inter].\n") ;
  }


  /*
    if (r) {
    Affiche_Symbole(a) ;
    printf(" inter ") ;
    Affiche_Symbole(b) ;
    printf(" = ") ;
    Affiche_Symbole(r) ;
    printf("\n") ; }
  */

  return r ;
}



/* a est ensembliste ; b est soit un atome soit ensembliste.
 * Ne modifie ni a ni b.
 * Renvoie une liste chaînée de symboles dont l'union est a sauf b.
 * La liste vide est représentée par le pointeur nul.
 * Il n'y a pas de partage de mémoire entre a ou b et le résultat.
 */

alphabet * sauf(tSymbole * a , tSymbole * b) {

  alphabet * r ;

  /*
  debug("sauf: ");
  Affiche_Symbole(a); fprintf(stderr, " \\ "); Affiche_Symbole(b); fprintf(stderr, "\n");
  */

  switch (b->sorteSymbole) {

  case UNIVERSEL:
    r = NULL;
    break;


  case INCOMPLET:

    switch (a->sorteSymbole) {

    case UNIVERSEL:
      r = univSaufInc(b);		  
      break;

    case INCOMPLET:
      r = incSaufInc(a, b);
      break;

    case NEGATIF:
    case CODE:
      r = codSaufInc(a, b);
      break;

    default:
      erreurInt("sauf1");
    }
    break;


  case NEGATIF:     
    switch (a->sorteSymbole) {
    case UNIVERSEL :  r = univSaufNeg(b) ;
      break ;
    case INCOMPLET :  r = incSaufNeg(a, b) ;
      break ;
    case NEGATIF:     r = negSaufNeg(a, b) ;
      break ;
    case CODE :       r = codSaufNeg(a, b) ;
      break ;
    default :
      erreurInt("sauf2") ;
    }
    break ;

  case CODE :       
    switch(a -> sorteSymbole) {
    case UNIVERSEL :  r = univSaufCod(b) ;
      break ;
    case INCOMPLET :  r = incSaufCod(a, b) ;
      break ;
    case NEGATIF : case CODE : r = codSaufCod(a, b) ;
      break ;
    default : erreurInt("sauf3") ;
    }
    break ;

  case ATOME :
    if (! appartient(b, a)) {
      r = (alphabet *) calloc(1, sizeof(alphabet)) ;
      if (! r) erreurMem("sauf") ;
      r -> suiv = 0 ;
      r -> etiquette = copieSymbole(a) ;
      break ;
    }
    switch(a -> sorteSymbole) {
    case UNIVERSEL :  r = univSaufAt(b) ;
      break ;
    case INCOMPLET :  r = incSaufAt(a, b) ;
      break ;
    case NEGATIF : case CODE : r = codSaufAt(a, b) ;
      break ;
    default : erreurInt("sauf4") ;
    }
    break ;

  default : erreurInt("sauf5") ;
  }

  /*
    for (i = 0 ; i < * nbSymboles ; i ++)
    if (4 > r[i].sorteSymbole ||
    r[i].sorteSymbole > 8)
    printf("Erreur interne [sauf] 8.\n") ;
  */

  return r ;
}



/* Met en ordre les formes canoniques dans une etiquette NEGATIF */

void ordonne(tSymbole * s) {

  tListe * liste, * pos ;
  int compte = 1 ;  /* on compte 1 pour le zero de fin de chaine */
  unichar * temp ;

  liste = separeFormes(s) ;
  trier(liste) ;

  for (pos = liste; pos; pos = pos->suiv)
    compte += (1 + u_strlen(pos->forme)) ;  /* 1 pour le "!" */

  temp = (unichar *) xcalloc(compte, sizeof(unichar)) ;
  temp[0] = 0 ;

  for (pos = liste ; pos ; pos = pos -> suiv) {
    u_strcat_char(temp, "!") ;
    u_strcat(temp, pos->forme) ;
  }

  free(s->canonique) ;

  while (liste) {
    pos = liste->suiv ;
    free(liste) ;  /* liste -> forme est deja libere, il etait partage */
    liste = pos ;
  }

  s->canonique = temp ;
}




/* a utiliser avec deux etiquettes incompletes
 * renvoie TRUE si etiq_a est plus fine que etiq_b
 * ou si les deux symboles sont egaux ; renvoie FALSE sinon
 */

static BOOL incPlusFinQuInc(tSymbole * a, tSymbole * b) {


  if (*b->canonique) { /* a doit avoir la meme forme canonique que b pour etre au moins plus fin */
    if (u_strcmp(a->canonique, b->canonique) != 0) {
      return FALSE;
    }
  }


  for (int i = 0; b->gramm[i]; i++) {

    for (int j = 0; a->gramm[j] != b->gramm[i]; j++) {

      if (a->gramm[j] == 0) { return FALSE; }
    }
  }

  return TRUE;
}



/* a complet ou  negatif, b incomplet
 * renvoie TRUE si a->gramm est plus fin que b->gramm,
 * renvoie FALSE sinon.
 */

static BOOL completPlusFinQuInc(tSymbole * a, tSymbole * b) {

  int i = 0 ,j = 0, nbLettresCommunes = 0;

  if (a->gramm[1] == ':' &&  strchr("ANV", a->gramm[0])) {

    for (i = 0; b->gramm[i]; i++)
      for (j = 0; a->gramm[j]; j++)
	if (b->gramm[i] == a->gramm[j])
	  nbLettresCommunes++;

    return (nbLettresCommunes == i);

  } else { return FALSE; }
}



/* a est une etiquette CODE ou NEGATIF ; b est une etiq. CODE */

static alphabet * codSaufCod(tSymbole * a, tSymbole * b) {

  alphabet * r ;

  if (u_strcmp(a->gramm, b->gramm)) {  /* Codes distincts */
    r = (alphabet *) calloc(1, sizeof(alphabet)) ;
    if (! r) erreurMem("codSaufCod") ;
    r -> suiv = 0 ;
    r -> etiquette = copieSymbole(a) ;
    return r ;
  }else             /* Meme code */
    return NULL ;
}


/* Preconditions : a est une etiquette CODE ou NEGATIF ; b est un ATOME ; */
/* b appartient a a. Postcondition : renvoie une liste a un element unique */

static alphabet * codSaufAt(tSymbole * a, tSymbole * b) {

  tSymbole * s ;
  alphabet * r ;

  s = (tSymbole *) calloc(1, sizeof(tSymbole)) ;
  if (! s)
    erreurMem("codSaufAt") ;

  s -> sorteSymbole = NEGATIF ;
  s -> canonique = (unichar *) calloc(2 + u_strlen(b->canonique), sizeof(unichar)) ;

  if (! s -> canonique)
    erreurMem("codSaufAt") ;

  u_strcpy_char(s -> canonique, "!") ;
  u_strcat(s -> canonique, b -> canonique) ;

  if (a -> sorteSymbole == NEGATIF) {
    s->canonique = (unichar *) realloc(s->canonique, (1 + u_strlen(s->canonique) + u_strlen(a->canonique)) * sizeof(unichar));

    if (! s -> canonique)
      erreurMem("codSaufAt") ;

    u_strcat(s -> canonique, a -> canonique) ; }

  if (1 + u_strlen(a -> gramm) >= maxGramm)
    erreurInt("codSaufAt") ;

  u_strcpy(s -> gramm, a -> gramm) ;
  s -> flechie[0] = 0 ;
  ordonne(s) ;
  r = (alphabet *) calloc(1, sizeof(alphabet)) ;

  if (! r) erreurMem("codSaufAt") ;

  r -> suiv = 0 ;
  r -> etiquette = s ;
  return r ;
}



/* a est une etiquette CODE ou NEGATIF ; b est une etiq. INCOMPLET */
static alphabet * codSaufInc(tSymbole * a, tSymbole * b) {

  alphabet * r ;
  if (completPlusFinQuInc(a, b)) { /* a plus fin que b */
    return NULL ; }
  else { /* a et b sont disjoints */
    r = (alphabet *) calloc(1, sizeof(alphabet)) ;
    if (! r) erreurMem("codSaufInc") ;
    r -> suiv = 0 ;
    r -> etiquette = copieSymbole(a) ;
    return r ; } }



/* Ne modifie ni a ni b. */
/* Postcondition : la liste renvoyee contient au moins un element. */

static alphabet * codSaufNeg(tSymbole * a, tSymbole * b) {

  unichar * chaine, * tampon ;
  int compte ;
  tSymbole * s ;
  alphabet * r, * temp ;

  if (u_strcmp(a->gramm, b->gramm)) {  /* Codes distincts */
    r = (alphabet *) calloc(1, sizeof(alphabet)) ;
    if (! r) erreurMem("codSaufNeg") ;
    r -> suiv = 0 ;
    r -> etiquette = copieSymbole(a) ;
    return r ;
  } else {            /* Meme code */
    tampon = (unichar *) calloc(1 + u_strlen(b->canonique), sizeof(unichar)) ;
    if (! tampon)
      erreurMem("codSaufNeg") ;
    u_strcpy(tampon, b->canonique) ;
    for (compte = 0, chaine = tampon ; * chaine ; chaine ++)
      if (* chaine == '!')
	compte ++ ;
    if (! compte)
      erreurInt("codSaufNeg") ;
    r = 0 ;
    chaine = u_strtok_char(tampon, "!") ; 
    while (chaine) {
      if (! compte --)
	erreurInt("codSaufNeg") ;
      s = (tSymbole *) calloc(1, sizeof(tSymbole)) ;
      if (! s)
	erreurMem("codSaufNeg") ;
      s -> sorteSymbole = ATOME ;
      s -> canonique = (unichar *) calloc(1 + u_strlen(chaine), sizeof(unichar)) ;
      if (! s -> canonique)
	erreurMem("codSaufNeg") ;
      u_strcpy(s->canonique, chaine) ;
      if (1 + u_strlen(a -> gramm) >= maxGramm)
	erreurInt("codSaufNeg") ;
      u_strcpy(s -> gramm, a -> gramm) ;
      s -> flechie[0] = 0 ;
      temp = (alphabet *) calloc(1, sizeof(alphabet)) ;
      if (! temp) erreurMem("codSaufNeg") ;
      temp -> suiv = r ;
      r = temp ;
      r -> etiquette = s ;
      chaine = u_strtok_char(NULL, "!") ; }
    if (compte)
      erreurInt("codSaufNeg") ;
    free(tampon) ;
    return r ; } }

static alphabet * incSaufCod(tSymbole * a, tSymbole * b) {
  tCode * c1, * c2 ;
  alphabet * r ;
  /*printf("incSaufCod\n") ;*/
  if (completPlusFinQuInc(b, a)) {   /* b est plus fin que a */
    c1 = traduit_symbole_en_code(a) ;
    if (! c1)
      erreurInt("incSaufCod1") ;
    c2 = traduit_symbole_en_code(b) ;
    if (! c2)
      erreurInt("incSaufCod2") ;
    r = tCodeSaufTCode(c1, c2) ;
    /*printf(" = ") ;
      Affiche_Symbole(s) ;
      if (* nbSymboles > 1)
      printf("...[%d]\n", * nbSymboles) ;
      else printf("\n") ;*/
    return r ; }
  else { /* a et b sont disjoints */
    r = (alphabet *) calloc(1, sizeof(alphabet)) ;
    if (! r) erreurMem("incSaufCod") ;
    r -> suiv = 0 ;
    r -> etiquette = copieSymbole(a) ;
    return r ; } }



/* Postcondition : * nbSymboles n est pas nul. */

static alphabet * univSaufCod(tSymbole * b) {

  tSymbole * s ;
  tSymbole inc ; /* etiq. INCOMPLET "A", "N" ou "V" */
  alphabet * r, * temp ;
  int i, compte ;

  switch(b -> gramm[0]) {
  case 'A' : case 'N' : case 'V' :
    if (u_strcmp_char(b->gramm, "ADV")) { /* b -> gramm ne contient pas "ADV" */
      r = 0 ;
      for (i = compte = 0 ; i < nbCatGramm ; i ++)
	if (CG[i][0] != b->gramm[0] || ! strcmp(CG[i], "ADV")) {
	  s = (tSymbole *) calloc(1, sizeof(tSymbole)) ;
	  if (! s)
	    erreurMem("univSaufCod") ;
	  s -> sorteSymbole = CODE ;
	  s -> canonique = (unichar *) calloc(1, sizeof(unichar)) ;
	  if (! s -> canonique)
	    erreurMem("univSaufCod") ;
	  s -> canonique[0] = 0 ;
	  if (1 + strlen(CG[i]) >= maxGramm)
	    erreurInt("univSaufCod") ;
	  u_strcpy_char(s->gramm, CG[i]) ;
	  s -> flechie[0] = 0 ;
	  temp = (alphabet *) calloc(1, sizeof(alphabet)) ;
	  if (! temp) erreurMem("univSaufCod") ;
	  temp -> suiv = r ;
	  r = temp ;
	  r -> etiquette = s ;
	  compte ++ ; }
      if (compte != nbCatGramm - 1)
	erreurInt("univSaufCod") ;
      inc.sorteSymbole = INCOMPLET ;
      inc.canonique = (unichar *) calloc(1, sizeof(unichar)) ;
      if (! inc.canonique)
	erreurMem("univSaufCod") ;
      inc.canonique[0] = 0 ;
      u_strcpy_char(inc.gramm, "x") ;
      inc.gramm[0] = b -> gramm[0] ;
      inc.flechie[0] = 0 ;
      temp = incSaufCod(& inc, b) ;
      free(inc.canonique) ;
      return unionListesDisj(r, temp) ; }
  default :  /* si b -> gramm contient "ADV" on passe dans le default */
    r = 0 ;
    for (i = compte = 0 ; i < nbCatGramm ; i ++)
      if (u_strcmp_char(b -> gramm, CG[i])) {
	s = (tSymbole *) calloc(1, sizeof(tSymbole)) ;
	if (! s)
	  erreurMem("univSaufCod") ;
	s -> sorteSymbole = CODE ;
	s -> canonique = (unichar *) calloc(1, sizeof(unichar)) ;
	if (! s -> canonique)
	  erreurMem("univSaufCod") ;
	s -> canonique[0] = 0 ;
	if (1 + strlen(CG[i]) >= maxGramm)
	  erreurInt("univSaufCod") ;
	u_strcpy_char(s -> gramm, CG[i]) ;
	s -> flechie[0] = 0 ;
	temp = (alphabet *) calloc(1, sizeof(alphabet)) ;
	if (! temp) erreurMem("univSaufCod") ;
	temp -> suiv = r ;
	r = temp ;
	r -> etiquette = s ;
	compte ++ ; }
    if (compte != nbCatGramm - 1)
      erreurInt("univSaufCod") ;
    return r ; } }

static alphabet * univSaufInc(tSymbole * b) {
  tSymbole * s ;
  tSymbole inc ; /* etiq. INCOMPLET "A", "N" ou "V" */
  int i, compte ;
  alphabet * r, * temp ;
  r = 0 ;
  for (i = compte = 0 ; i < nbCatGramm ; i ++)
    if (CG[i][0] != b -> gramm[0] || ! strcmp("ADV", CG[i])) {
      s = (tSymbole *) calloc(1, sizeof(tSymbole)) ;
      if (! s)
	erreurMem("univSaufInc") ;
      s -> sorteSymbole = CODE ;
      s -> canonique = (unichar *) calloc(1, sizeof(unichar)) ;
      if (! s -> canonique)
	erreurMem("univSaufInc") ;
      s -> canonique[0] = 0 ;
      if (1 + strlen(CG[i]) >= maxGramm)
	erreurInt("univSaufInc") ;
      u_strcpy_char(s -> gramm, CG[i]) ;
      s -> flechie[0] = 0 ;
      temp = (alphabet *) calloc(1, sizeof(alphabet)) ;
      if (! temp) erreurMem("univSaufInc") ;
      temp -> suiv = r ;
      r = temp ;
      r -> etiquette = s ;
      compte ++ ; }
  if (compte != nbCatGramm - 1)
    erreurInt("univSaufInc") ;
  inc.sorteSymbole = INCOMPLET ;
  inc.canonique = (unichar *) calloc(1, sizeof(unichar)) ;
  if (! inc.canonique)
    erreurMem("univSaufInc") ;
  inc.canonique[0] = 0 ;
  u_strcpy_char(inc.gramm, "x") ;
  inc.gramm[0] = b -> gramm[0] ;
  inc.flechie[0] = 0 ;
  temp = incSaufInc(& inc, b) ;
  free(inc.canonique) ;
  return unionListesDisj(r, temp) ; }

static alphabet * univSaufNeg(tSymbole * b) {
  tSymbole c ;   /* CODE c qui correspond au NEGATIF */
  alphabet * r1, * r2 ;
  c.sorteSymbole = CODE ;
  c.canonique = (unichar *) calloc(1, sizeof(unichar)) ;
  if (! c.canonique)
    erreurMem("univSaufNeg") ;
  c.canonique[0] = 0 ;
  if (1 + u_strlen(b -> gramm) >= maxGramm)
    erreurInt("univSaufNeg") ;
  u_strcpy(c.gramm, b -> gramm) ;
  c.flechie[0] = 0 ;
  r1 = univSaufCod(& c) ;
  r2 = codSaufNeg(& c, b) ;
  free(c.canonique) ;
  return unionListesDisj(r1, r2) ; }

static alphabet * univSaufAt(tSymbole * b) {
  tSymbole c ;   /* CODE c qui correspond a l ATOME */
  alphabet * r1, * r2 ;
  c.sorteSymbole = CODE ;
  c.canonique = (unichar *) calloc(1, sizeof(unichar)) ;
  if (! c.canonique)
    erreurMem("univSaufAt") ;
  c.canonique[0] = 0 ;
  if (1 + u_strlen(b -> gramm) >= maxGramm)
    erreurInt("univSaufAt") ;
  u_strcpy(c.gramm, b -> gramm) ;
  c.flechie[0] = 0 ;
  r1 = univSaufCod(& c) ;
  r2 = codSaufAt(& c, b) ;
  free(c.canonique) ;
  return unionListesDisj(r1, r2) ; }


static alphabet * incSaufAt(tSymbole * a, tSymbole * b) {

  /* a est une etiquette INCOMPLET ; b est un ATOME */
  /* b appartient a a */

  tSymbole c ;   /* CODE c qui correspond a l ATOME */
  alphabet * r1, * r2 ;

  c.sorteSymbole = CODE ;
  c.canonique = (unichar *) calloc(1, sizeof(unichar)) ;

  if (! c.canonique)
    erreurMem("incSaufAt") ;
  c.canonique[0] = 0 ;
  if (1 + u_strlen(b -> gramm) >= maxGramm)
    erreurInt("incSaufAt") ;
  u_strcpy(c.gramm, b -> gramm) ;
  c.flechie[0] = 0 ;
  r1 = incSaufCod(a, & c) ;
  r2 = codSaufAt(& c, b) ;
  free(c.canonique) ;
  return unionListesDisj(r1, r2) ;
}


static alphabet * incSaufInc(tSymbole * a, tSymbole * b) {

  tCode * c1, * c2 ;
  alphabet * r ;

  if (a->gramm[0] != b->gramm[0]) {

    r = (alphabet *) xcalloc(1, sizeof(alphabet));
    r->suiv = 0;
    r->etiquette = symbole_dup(a);
    return r;

  } else if (incPlusFinQuInc(a, b)) { /* a plus fin que b */

    return NULL;

  } else {

    c1 = traduit_symbole_en_code(a);
    if (! c1) { Affiche_Symbole(a); Affiche_Symbole(b); erreurInt("incSaufInc1"); }

    c2 = traduit_symbole_en_code(b);
    if (! c2) { Affiche_Symbole(a); Affiche_Symbole(b); erreurInt("incSaufInc2") ; }

    r = tCodeSaufTCode(c1, c2);
    return r;
  }
}



static alphabet * incSaufNeg(tSymbole * a, tSymbole * b) {
  tSymbole c ;   /* CODE c qui correspond au NEG */
  alphabet * r1, * r2 ;
  /*printf("incSaufNeg\n") ;*/
  if (completPlusFinQuInc(b, a)) {   /* b est plus fin que a */
    c.sorteSymbole = CODE ;
    c.canonique = (unichar *) calloc(1, sizeof(unichar)) ;
    if (! c.canonique)
      erreurMem("incSaufNeg") ;
    c.canonique[0] = 0 ;
    if (1 + u_strlen(b -> gramm) >= maxGramm)
      erreurInt("incSaufNeg") ;
    u_strcpy(c.gramm, b -> gramm) ;
    c.flechie[0] = 0 ;
    r1 = incSaufCod(a, & c) ;
    r2 = codSaufNeg(& c, b) ;
    free(c.canonique) ;
    return unionListesDisj(r1, r2) ; }
  else { /* a et b sont disjoints */
    r1 = (alphabet *) calloc(1, sizeof(alphabet)) ;
    if (! r1) erreurMem("incSaufNeg") ;
    r1 -> suiv = 0 ;
    r1 -> etiquette = copieSymbole(a) ;
    return r1 ;
  }
}

static alphabet * negSaufNeg(tSymbole * a, tSymbole * b) {
  unichar * chaine ;
  unichar * FCa, * FCb ;
  unichar * * formes_a, * * formes_b ;
  int compte_a, compte_b, i ;
  tSymbole * s ;
  BOOL trouve ;
  alphabet * r, * temp ;

  if (u_strcmp(a -> gramm, b -> gramm)) {  /* Codes distincts */
    r = (alphabet *) calloc(1, sizeof(alphabet)) ;
    if (! r) erreurMem("negSaufNeg") ;
    r -> suiv = 0 ;
    r -> etiquette = copieSymbole(a) ;
    return r ;
  } else {            /* Meme code */
    /* on fait la liste des f.can. de b qui ne sont pas dans a. */
    /* En effet,   (C \ A) \ (C \ B) == (C inter B) \ A   */
    FCa = (unichar *) calloc(1 + u_strlen(a->canonique), sizeof(unichar)) ;
    if (! FCa)
      erreurMem("negSaufNeg") ;
    u_strcpy(FCa,  a->canonique) ;
    for (compte_a = 0, chaine = FCa ; * chaine ; chaine ++)
      if (* chaine == '!')
	compte_a ++ ;
    if (! compte_a)
      erreurInt("negSaufNeg") ;
    formes_a = (unichar **) calloc(compte_a, sizeof(unichar *)) ;
    if (! formes_a)
      erreurMem("negSaufNeg") ;
    for (compte_a = 0, chaine = FCa ; * chaine ; chaine ++)
      if (* chaine == '!') {
	* chaine = 0 ;
	formes_a[compte_a ++] = chaine + 1 ; }
    FCb = (unichar *) calloc(1 + u_strlen(b->canonique), sizeof(unichar)) ;
    if (! FCb)
      erreurMem("negSaufNeg") ;
    u_strcpy(FCb, b->canonique) ;
    for (compte_b = 0, chaine = FCb ; * chaine ; chaine ++)
      if (* chaine == '!')
	compte_b ++ ;
    if (! compte_b)
      erreurInt("negSaufNeg") ;
    formes_b = (unichar **) calloc(compte_b, sizeof(unichar *)) ;
    if (! formes_b)
      erreurMem("negSaufNeg") ;
    compte_b = 0 ;
    for (chaine = u_strtok_char(FCb, "!") ; chaine ; chaine = u_strtok_char(NULL, "!")) {
      for (i = 0, trouve = FALSE ; i < compte_a && ! trouve ; i ++)
	if (! u_strcmp(chaine, formes_a[i]))
	  trouve = TRUE ;
      if (! trouve)   /* chaine figure dans b mais pas dans a, on la garde */
	formes_b[compte_b ++] = chaine ; }
    r = 0 ;
    if (compte_b)
      while (compte_b --) {
	s = (tSymbole *) calloc(1, sizeof(tSymbole)) ;
	if (! s)
	  erreurMem("negSaufNeg") ;
	s -> sorteSymbole = ATOME ;
	s -> canonique = (unichar *) calloc(1 + u_strlen(formes_b[compte_b]), sizeof(unichar)) ;
	if (! s -> canonique)
	  erreurMem("negSaufNeg") ;
	u_strcpy(s -> canonique, formes_b[compte_b]) ;
	if (1 + u_strlen(a -> gramm) >= maxGramm)
	  erreurInt("negSaufNeg") ;
	u_strcpy(s -> gramm, a -> gramm) ;
	s -> flechie[0] = 0 ;
	temp = (alphabet *) calloc(1, sizeof(alphabet)) ;
	if (! temp) erreurMem("negSaufNeg") ;
	temp -> suiv = r ;
	r = temp ;
	r -> etiquette = s ; }
    free(formes_a) ;
    free(FCa) ;
    free(formes_b) ;
    free(FCb) ;
    return r ; } }

static alphabet * unionListesDisj(alphabet * a1, alphabet * a2) {
  /* reutilise et detruit les listes disjointes a1 et a2 */
  if (! a1)
    return a2 ;
  if (! a2)
    return a1 ;
  while (a1 -> suiv)
    a1 = a1 -> suiv ;
  a1 -> suiv = a2 ;
  return a1 ; }



/* c1 et c2 sont de la meme categorie grammaticale
 * c1 est incomplet ; c2 est complet ou incomplet
 * c1 n'est pas plus fin que c2 
 * c1 et c2 sont liberes apres usage
 */

static alphabet * tCodeSaufTCode(tCode * c1, tCode * c2) {

  tSymbole * s;
  alphabet * r;
  tCode * c3;

  if ((c3 = Inter_Code(c1, c2)) == NULL) {  /* intersection vide */

    s = (tSymbole *) xcalloc(1, sizeof(tSymbole)) ;

    if (! traduit_code_en_symbole(s, c1)) { erreurInt("tCodeSaufTCode"); }

    free(c1);
    free(c2);
    r = (alphabet *) xcalloc(1, sizeof(alphabet));
    r->suiv = 0;
    r->etiquette = s;
    return r;
  }

  free(c3);

  switch (c1->cat_gramm) {

  case 'A':
  case 'N':
    return ANsaufAN(c1, c2) ;

  case 'V':
    return VsaufV(c1, c2) ;

    /*
    case 'P:
    return PROsaufPRO(c1, c2);
    */

  default:
    die("tCodeSaufTCode: unexpected cat_gramm: '%C'\n", c1->cat_gramm);
    return 0 ;
  }
}




/* c1 et c2 sont de la meme categorie grammaticale
 * c1 est incomplet ; c2 est complet ou incomplet
 * c1 n'est pas plus fin que c2
 * c1 inter c2 n'est pas vide
 * Cette fonction libere c1 et c2 apres usage
 */

static alphabet * ANsaufAN(tCode * c1, tCode * c2) {

  tCode * c ;
  tSymbole * s ;
  alphabet * r1, * r2 ;

  /*
    affCode(c1);
    printf(" ANsaufAN");
    affCode(c2);
    printf("\n");
  */

  c = (tCode *) xcalloc(1, sizeof(tCode)) ;

  c->cat_gramm = c1->cat_gramm ;


  if (c1->genre) {

    c->genre = c1->genre ;

    if (c1->nombre) {
      c->nombre = c1->nombre ;
    } else {
      switch (c2->nombre) {
      case 's': c->nombre = 'p'; break;
      case 'p': c->nombre = 's'; break;
      default : erreurInt("ANSaufAN");
      }
    }

    free(c1); free(c2);

    s = (tSymbole *) xcalloc(1, sizeof(tSymbole)) ;

    if (! traduit_code_en_symbole(s, c)) { erreurInt("ANsaufAN"); }
    free(c);
    r1 = (alphabet *) xcalloc(1, sizeof(alphabet)) ;

    r1->etiquette = s;
    r1->suiv = 0;

    /*printf(" = ") ; Affiche_Symbole(s1) ; printf("\n") ;*/

    return r1;


  } else if (c1->nombre) {

    c->nombre = c1->nombre ;

    switch (c2->genre) {
    case 'm': c -> genre = 'f'; break;
    case 'f': c -> genre = 'm'; break;
    default : fprintf(stderr, "Erreur interne [ANSaufAN] 2\n");
    }
    free(c1); free(c2);

    s = (tSymbole *) xcalloc(1, sizeof(tSymbole)) ;
    if (! traduit_code_en_symbole(s, c)) { erreurInt("ANsaufAN"); }
    free(c);

    r1 = (alphabet *) xcalloc(1, sizeof(alphabet)) ;
    r1->suiv = 0 ;
    r1->etiquette = s ;
    /*printf(" = ") ; Affiche_Symbole(s1) ; printf("\n") ;*/
    return r1;

  } else {   /* c1 n'a ni genre ni nombre */

    if (c2->genre) {

      c->genre = ((c2 -> genre == 'm') ? 'f' : 'm') ;
      c->nombre = c1 -> nombre ;
      s = (tSymbole *) xcalloc(1, sizeof(tSymbole)) ;

      if (! traduit_code_en_symbole(s, c)) { erreurInt("ANsaufAN"); }
      free(c) ;

      r1 = (alphabet *) xcalloc(1, sizeof(alphabet)) ;
      r1->suiv = 0 ;
      r1->etiquette = s ;

      if (c2->nombre) {
	c1->genre = c2->genre;  /* c1 pas plus fin que c2 */
	r2 = ANsaufAN(c1, c2);
	return unionListesDisj(r1, r2);
      } else {
	free(c1);
	free(c2);
	/*printf(" = ") ; Affiche_Symbole(s1) ; printf("\n") ;*/
	return r1 ;
      }

    } else {

      c->genre = c1->genre ;
      c->nombre = ((c2 -> nombre == 's') ? 'p' : 's') ;
      free(c1); free(c2);
      s = (tSymbole *) xcalloc(1, sizeof(tSymbole)) ;
      if (! traduit_code_en_symbole(s, c)) { erreurInt("ANsaufAN"); }
      free(c);
      r1 = (alphabet *) xcalloc(1, sizeof(alphabet)) ;
      r1 -> suiv = 0 ;
      r1 -> etiquette = s ;

      /*printf(" = ") ; Affiche_Symbole(s1) ; printf("\n") ;*/

      return r1 ;
    }
  }
}



/* c1 est incomplet ; c2 est complet ou incomplet
 * c1 n'est pas plus fin que c2
 * c1 inter c2 n'est pas vide
 * Cette fonction libere c1 et c2 apres usage
 */

static alphabet * VsaufV(tCode * c1, tCode * c2) {

  tCode * c ;
  tSymbole * s ;
  int autre ;
  alphabet * r, * temp ;

  /*affCode(c1) ;
    printf(" VsaufV ") ;
    affCode(c2) ;
    printf("\n") ;*/

  c = (tCode *) calloc(1, sizeof(tCode)) ;
  if (! c)
    erreurMem("VsaufV") ;

  /*printf(" (%d %d) ", c1 -> temps, c2 -> temps) ;*/

  if (c1 -> temps) { /* si c2->temps est specifie, c'est le meme */

    if (c2->temps == 0)
      c2->temps = c1->temps ;

    c->temps = c1->temps ;

  }  else if (c2->temps) {

    tCode autresTemps[10] ;  /* il y a 11 temps */
    char * tps ;
    for (autre = 0 ; autre < 10 ; autre ++)
      autresTemps[autre] = * c1 ; /* bit a bit : autres temps, autres moeurs, ah ah */
    autre = 0 ;
    for (tps = "PIJFCSTYKGW" ; * tps ; tps ++) 
      if (c2 -> temps != * tps)
	autresTemps[autre ++].temps = * tps ;
    for (autre = 0, r = 0 ; autre < 10 ; autre ++) {
      s = (tSymbole *) calloc(1, sizeof(tSymbole)) ;
      if (! s)
	erreurMem("VsaufV") ;
      if (traduit_code_en_symbole(s, autresTemps + autre)) {
	temp = (alphabet *) calloc(1, sizeof(alphabet)) ;
	if (! temp) erreurMem("VsaufV") ;
	temp -> suiv = r ;
	r = temp ;
	r -> etiquette = s ; }
      else free(s) ;}
    c1 -> temps = c2 -> temps ;
    if (plusFin(c1, c2)) {
      free(c1) ;
      free(c2) ;
      return r ; }
    else {
      temp = VsaufV(c1, c2) ;
      return unionListesDisj(r, temp) ; } }
  else
    c -> temps = c1 -> temps ;
  /* maintenant c -> temps == c1 -> temps == c2 -> temps */
  /*printf(" (%d %d) ", c1 -> personne, c2 -> personne) ;*/
  if (c1 -> personne) {
    if (c2 -> personne == 0)
      c2 -> personne = c1 -> personne ;
    c -> personne = c1 -> personne ; }
  /* si c2->personne est specifiee, c'est la meme */
  else if (c2 -> personne) {
    tCode autresPersonnes[2] ;
    autresPersonnes[0] = * c1 ;
    autresPersonnes[1] = * c1 ;
    autre = 0 ;
    if (c2 -> personne != '1')
      autresPersonnes[autre ++].personne = '1' ;
    if (c2 -> personne != '2')
      autresPersonnes[autre ++].personne = '2' ;
    if (c2 -> personne != '3')
      autresPersonnes[autre ++].personne = '3' ;
    for (autre = 0, r = 0 ; autre < 2 ; autre ++) {
      s = (tSymbole *) calloc(1, sizeof(tSymbole)) ;
      if (! s)
	erreurMem("VsaufV") ;
      if (traduit_code_en_symbole(s, autresPersonnes + autre)) {
	temp = (alphabet *) calloc(1, sizeof(alphabet)) ;
	if (! temp) erreurMem("VsaufV") ;
	temp -> suiv = r ;
	r = temp ;
	r -> etiquette = s ; }
      else free(s) ; }
    c1 -> personne = c2 -> personne ;
    if (plusFin(c1, c2)) {
      free(c1) ;
      free(c2) ;
      return r ; }
    temp = VsaufV(c1, c2) ;
    return unionListesDisj(r, temp) ; }
  else
    c -> personne = c1 -> personne ;
  /* maintenant, c -> personne == c1 -> personne == c2 -> personne */
  /*printf(" (%d %d) ", c1 -> genre, c2 -> genre) ;*/
  if (c1 -> genre) {
    if (c2 -> genre == 0)
      c2 -> genre = c1 -> genre ;
    c -> genre = c1 -> genre ; } /* si c2->genre est specifie, c'est le meme */
  else if (c2 -> genre) {
    tCode autreGenre ;
    /* fait l'autre genre, intersecte avec c1 et */
    /* s'il n'est pas vide traduit en tSymbole dans s1 */
    autreGenre = * c1 ;   /* copie bit a bit */
    autreGenre.genre = (c2 -> genre == 'm' ? 'f' : 'm') ;
    s = (tSymbole *) calloc(1, sizeof(tSymbole)) ;
    if (! s)
      erreurMem("VsaufV") ;
    if (traduit_code_en_symbole(s, & autreGenre)) {
      r = (alphabet *) calloc(1, sizeof(alphabet)) ;
      if (! r) erreurMem("VsaufV") ;
      r -> suiv = 0 ;
      r -> etiquette = s ; }
    else {
      r = 0 ;
      free(s) ; }
    c1 -> genre = c2 -> genre ;
    if (plusFin(c1, c2)) {
      free(c1) ;
      free(c2) ;
      return r ; }
    temp = VsaufV(c1, c2) ;
    return unionListesDisj(r, temp) ; }
  else
    c -> genre = c1 -> genre ;
  /* maintenant, c -> genre == c1 -> genre == c2 -> genre */
  /*printf(" (%d %d) ", c1 -> nombre, c2 -> nombre) ;*/
  if (c1 -> nombre) {
    if (c2 -> nombre == 0)
      erreurInt("VsaufV1") ;
    else erreurInt("VsaufV2") ; }
  else if (c2 -> nombre) {
    c1 -> nombre = (c2 -> nombre == 's' ? 'p' : 's') ;
    s = (tSymbole *) calloc(1, sizeof(tSymbole)) ;
    if (! s)
      erreurMem("VsaufV") ;
    if (traduit_code_en_symbole(s, c1)) {
      r = (alphabet *) calloc(1, sizeof(alphabet)) ;
      if (! r) erreurMem("VsaufV") ;
      r -> suiv = 0 ;
      r -> etiquette = s ; }
    else {
      r = 0 ;
      free(s) ; }
    free(c1) ;
    free(c2) ;
    return r ; }
  else
    erreurInt("VsaufV3") ;
  return 0 ;
}


static BOOL plusFin(tCode * c1, tCode * c2) {
  /* Renvoie TRUE si c1 est plus fin que c2 */
  /* et FALSE sinon */
  /* c1 et c2 sont des codes de verbes */
  if (c2 -> temps && c1 -> temps != c2 -> temps)
    if (c2 -> temps != 'K' || c1 -> temps || ! c1 -> genre)
      return FALSE ;
  if (c2 -> personne && c1 -> personne != c2 -> personne)
    return FALSE ;
  if (c2 -> genre && c1 -> genre != c2 -> genre)
    return FALSE ;
  if (c2 -> nombre && c1 -> nombre != c2 -> nombre)
    return FALSE ;
  return TRUE ;
}



void affCode(tCode * c) {

  tSymbole * s = (tSymbole *) xcalloc(1, sizeof(tSymbole));

  traduit_code_en_symbole(s, c);
  Affiche_Symbole(s);
  free(s->canonique);
  free(s);
}





static tCode * traduit_symbole_en_code(tSymbole * s) {

  //  debug("traduit_symbole: "); Affiche_Symbole(s); fprintf(stderr, "\n");

  int cpt = 0 ; 
  unichar tampon[maxGramm];

  tCode * c = (tCode *) xcalloc(1, sizeof(tCode));

  if (1 + u_strlen(s->gramm) > maxGramm) {
    die("[traduit_symbole_en_code] code grammatical (%S) de longueur %d\n", s->gramm, u_strlen(s->gramm)) ;
  }

  u_strcpy(tampon, s->gramm);

  switch (c->cat_gramm = tampon[0]) {

  case 'A':
    if (tampon[1] == 'D') { // ADV
      warning("traduit_symbole_en_code: unexpected symbol:"); Affiche_Symbole(s); fprintf(stderr, "\n");
      free(c);
      return NULL;
    }

  case 'N':
    switch (tampon[2]) {
    case 'm':
    case 'f':
      c->genre = tampon[2];
      cpt++;
      break;
    default:
      c->genre = 0;
    }

    switch (tampon[2 + cpt]) {
    case 's':
    case 'p':
      c->nombre = tampon[2 + cpt];
      cpt++;
      break;
    default:
      c->nombre = 0;
    }
    c->temps    = 0;
    c->personne = 0;
    break;


  case 'V' : {

    if (tampon[2] && strchr("CFGIJKPSTWY", tampon[2])) {
      c->temps = tampon[2] ;
      cpt++;
    } else c->temps = 0 ;


    switch (tampon[2 + cpt]) {

    case '1':
    case '2':
    case '3':
      c->personne = tampon[2 + cpt];
      cpt++;
      break;

    default: 
      c->personne = 0;
    }


    switch (tampon[2 + cpt]) {

    case 'm':
    case 'f':
      c->genre = tampon[2 + cpt];     
      cpt++;
      break;

    default :
      c -> genre = 0;
    }

    switch (tampon[2 + cpt]) {

    case 's':
    case 'p':
      c->nombre = tampon[2 + cpt] ;
      cpt++;
      break;

    default:
      c->nombre = 0 ;
    }
  }
  break ;


  case 'P':
    if (tampon[1] != 'R' || tampon[2] != 'O') { 
      error("traduit_symbole_en_code: unexpected grammar code in "); Affiche_Symbole(s); fprintf(stderr,"\n");
      free(c);
      return NULL;
    }

    cpt = 4;
    switch (tampon[cpt]) {

    case '1':
    case '2':
    case '3':
      c->personne = tampon[cpt++];
      break;

    default: 
      c->personne = 0;
    }

    switch (tampon[cpt]) {

    case 'm':
    case 'f':
      c->genre = tampon[cpt++];
      break;

    default :
      c->genre = 0;
    }

    switch (tampon[cpt]) {
    case 's':
    case 'p':
      c->nombre = tampon[cpt++];
      break;
    default:
      c->nombre = 0;
    }
    break;

  default :
    printf("Erreur interne [traduit_symbole_en_code],\n");
    i_fprintf(stdout, "code grammatical inattendu (%S)\n", s->gramm);
    free(c);
    return NULL;
  }

  return c;
}




/* Traduit un code de type tCode en symbole de type tSymbole
 * Si le code a des contradictions internes, renvoie NULL.
 * Ne modifie pas le code donne en entree.
 */

static tSymbole * traduit_code_en_symbole(tSymbole * s, tCode * c) {
  int cpt ;
  unichar tampon[maxGramm] ;
  memset(tampon, 0, maxGramm) ;
  tampon[0] = c -> cat_gramm ;
  cpt = 0 ;
  switch (tampon[0]) {
  case 'A' : case 'N' :
    if (c -> genre) {
      tampon[2] = c -> genre ;
      cpt ++ ; }
    if (c -> nombre) {
      tampon[2 + cpt] = c -> nombre ;
      cpt ++ ; }
    s -> sorteSymbole = (cpt == 2 ? CODE : INCOMPLET) ;
    break ;
  case 'V' :
    if ((c -> temps == 'K') && (c -> personne))
      return NULL ;
    if (c -> temps == 'G' && 
	(c -> personne || c -> genre || c -> nombre))
      return NULL ;
    if (c -> temps && strchr("CFGIJPSTWY", c -> temps) && c -> genre)
      return NULL ;
    if (c->temps) {
      tampon[2 + cpt] = c->temps ;
      cpt ++ ; }
    if (c->personne) {
      tampon[2 + cpt] = c -> personne ;
      cpt ++ ; }
    if (c -> genre) {
      tampon[2 + cpt] = c -> genre ;
      cpt ++ ; }
    if (c -> nombre) {
      tampon[2 + cpt] = c -> nombre ;
      cpt  ++; }
    if (c -> temps && strchr("GW", c -> temps))
      s -> sorteSymbole = CODE ;
    else s -> sorteSymbole = (cpt == 3 ? CODE : INCOMPLET) ;
    break ;
  default :
    printf("Erreur interne [traduit_code_en_symbole].\n") ;
    exit(1) ; }
  if (cpt)
    tampon[1] = ':';
  s -> flechie[0] = 0 ;
  s -> canonique = (unichar *) calloc(1, sizeof(unichar)) ;
  if (! s -> canonique)
    erreurMem("traduit_code_en_symbole") ;
  s -> canonique[0] = 0 ;
  if (1 + u_strlen(tampon) >= maxGramm) {
    printf("Erreur interne [traduit_code_en_symbole].\n") ;
    exit(1) ; }
  u_strcpy(s -> gramm, tampon);
  return s ; }

static tCode * Inter_Code(tCode * code1, tCode * code2) {
  /* Renvoie l intersection si elle n©est pas vide */
  tCode * c ;
  c = (tCode *) calloc(1, sizeof(tCode)) ;
  if (! c)
    erreurMem("Inter_Code") ;
  if (code1 -> cat_gramm != code2 -> cat_gramm) {
    free(c) ;
    printf("Categories distinctes %c (%d), %c (%d)\n",
	   code1 -> cat_gramm, code1 -> cat_gramm,
	   code2 -> cat_gramm, code2 -> cat_gramm) ;
    return NULL ; }
  else c -> cat_gramm = code1 -> cat_gramm ;
  if ((c -> temps = code1 -> temps) == 0)
    c -> temps = code2 -> temps ;
  else if ((code2 -> temps != 0) && (code1 -> temps != code2 -> temps)) {
    free(c) ;
    /*printf("Temps distincts %c, %c\n",
      code1 -> temps, code2 -> temps) ;*/
    return NULL ; }
  if ((c -> personne = code1 -> personne) == 0)
    c -> personne = code2 -> personne ;
  else if ((code2 -> personne != 0) &&
	   (code1 -> personne != code2 -> personne)) {
    free(c) ;
    /*printf("Personnes distinctes %c, %c\n",
      code1 -> personne, code2 -> personne) ;*/
    return NULL ; }
  if ((c -> genre = code1 -> genre) == 0)
    c -> genre = code2 -> genre ;
  else if ((code2 -> genre != 0) && (code1 -> genre != code2 -> genre)) {
    free(c) ;
    /*printf("Genres distincts %c, %c\n",
      code1 -> genre, code2 -> genre) ;*/
    return NULL ; }
  if ((c -> nombre = code1 -> nombre) == 0)
    c -> nombre = code2 -> nombre ;
  else if ((code2 -> nombre != 0) && (code1 -> nombre != code2 -> nombre)) {
    free(c) ;
    /*printf("Nombres distincts %c, %c\n",
      code1 -> nombre, code2 -> nombre) ;*/
    return NULL ; }
  /* cas particuliers */
  if ((c -> temps == 'K') && (c -> personne)) {
    free(c) ;
    return NULL ; }   
  if (c -> temps == 'G' && 
      (c -> personne || c -> genre || c -> nombre)) {
    free(c) ;
    return NULL ; }
  if (c -> temps && strchr("CFGIJPSTWY", c -> temps) && c -> genre) {
    free(c) ;
    return NULL ; }
  return c ; }


/* Intersection de deux symboles incomplets de noms ou d'adjectifs. */
/* a et b sont incomplets, non egaux, aucun des deux n est plus fin */
/* que l autre et ils sont de la meme categorie grammaticale, */
/* donc chacun des deux a soit le genre, soit le nombre, mais */
/* pas les deux a la fois */


static tSymbole * incANInterIncAN( tSymbole * a, tSymbole * b) {

  tSymbole * s ;

  s = (tSymbole *) calloc(1, sizeof(tSymbole)) ;

  if (! s)
    erreurMem("incANInterIncAN") ;

  s -> gramm[0] = a -> gramm[0]  ;
  s -> gramm[1] = ':' ;

  if( (a -> gramm[2] == 'f') || (a -> gramm[2] == 'm') ) {

    if( (b -> gramm[2] == 's') || (b -> gramm[2] == 'p') ) {

      s -> gramm[2] = a -> gramm[2] ;
      s -> gramm[3] = b -> gramm[2] ;
      s -> gramm[4] = 0 ;

    } else {  /* b est de l'autre genre */

      free(s -> canonique) ;
      free(s) ;
      return NULL ;

    }

  } else if ((a->gramm[2] == 's') || (a->gramm[2] == 'p')) {

    if( (b -> gramm[2] == 'm') || (b -> gramm[2] == 'f') ) {

      s -> gramm[2] = b -> gramm[2] ;
      s -> gramm[3] = a -> gramm[2] ;
      s -> gramm[4] = 0 ;

    } else {  /* b est de l'autre nombre */

      free(s -> canonique) ;
      free(s) ;
      return NULL ;
    }
  }

  s->sorteSymbole = (u_strlen(s -> gramm) == 4 ? CODE : INCOMPLET) ;
  s->canonique    = (unichar *) calloc(1, sizeof(unichar)) ;

  if (! s->canonique)
    erreurMem("incANInterIncAN");

  s->canonique[0] = 0;

  return s;
}



/* intersection de deux symboles incomplets */

static tSymbole * incInterInc(tSymbole * a , tSymbole * b) {

  tCode * Code1, * Code2, * code3;
  tSymbole * r ;

  if (a->gramm[0] != b->gramm[0]) { return NULL; }

  if (*a->canonique && *b->canonique) {
    if (u_strcmp(a->canonique, b->canonique) != 0) { // a et b sont disjoints
      return NULL;
    }
  }


  if (incPlusFinQuInc(a, b)) { return symbole_dup(a); }

  if (incPlusFinQuInc(b, a)) { return symbole_dup(b); }


  switch (a->gramm[0]) {

  case 'N':
  case 'A':  
    return incANInterIncAN(a,b) ;
    break;

  case 'V' : {

    Code1 = traduit_symbole_en_code(a);

    if (! Code1) { die("Erreur interne [incInterInc].\n"); }

    Code2 = traduit_symbole_en_code(b);

    if (! Code2) { die("Erreur interne [incInterInc].\n"); }

    if ((code3 = Inter_Code(Code1, Code2))) {

      r = (tSymbole *) xcalloc(1, sizeof(tSymbole)) ;

      if (! traduit_code_en_symbole(r, code3)) { die("Erreur interne [incInterInc]\n"); }

      free(Code1);
      free(Code2);
      free(code3);
      return r;

    } else {

      free(Code1);
      free(Code2);
      return NULL;
    }
  }
  break;

  default:
    return NULL;
    break;
  }
}


static tSymbole * codeInterInc(tSymbole * a , tSymbole * b) {
  /* Intersection entre un symbole code (a) et un symbole incomplet (b) */
  /* ou entre un symbole negatif (a) et un symbole incomplet (b). */
  return (completPlusFinQuInc(a, b) ? copieSymbole(a) : NULL) ;
}


/*////////////////////////////////////////////////////////////////////// */
/* FONCT. D'INTERSECT. ENTRE 1 ETIQUETTE DE CODE COMPLET ET 1 NEGATIVE   */
/*    OU ENTRE 2  ETIQUETTES GRAMMATICALES COMPLETES                     */
/*    s'il y a etiq negative, c'est etiq_b                               */
/*////////////////////////////////////////////////////////////////////// */

static tSymbole * codeInterCode(tSymbole * a , tSymbole * b) {
  return (u_strcmp(a->gramm, b->gramm) ? NULL : copieSymbole(b));
}


/*////////////////////////////////////////////////////////////// */
/* FONCT. D'INTERSECT.   ENTRE 2  ETIQUETTES NEGATIVES           */
/*////////////////////////////////////////////////////////////// */
static tSymbole * negInterNeg(tSymbole * a, tSymbole * b ) {
  tSymbole * s ;
  if (! u_strcmp(a -> gramm, b -> gramm)) {
    if (! u_strcmp(a -> canonique, b -> canonique))
      return copieSymbole(a) ;
    else {
      s = (tSymbole *) calloc(1, sizeof(tSymbole)) ;
      if (! s)
	erreurMem("negInterNeg") ;
      s -> sorteSymbole = a -> sorteSymbole ;
      if (1 + u_strlen(a -> gramm) >= maxGramm) {
	printf("Erreur interne[negInterNeg].\n") ;
	exit(1) ; }
      u_strcpy(s -> gramm, a -> gramm) ;
      s -> canonique = (unichar *) calloc(1 + u_strlen(a -> canonique) + u_strlen(b -> canonique), sizeof(unichar)) ;
      if (! s -> canonique)
	erreurMem("negInterNeg") ;
      u_strcpy(s -> canonique, a -> canonique) ;
      u_strcat(s -> canonique, b -> canonique) ;
      ordonne(s) ;
      return s ; } }
  else
    return NULL ; }

/*************************************************************/

/* transforme s -> canonique en liste. Les caracteres sont partages.
 */

static tListe * separeFormes(tSymbole * s) {

  tListe * liste = NULL, * temp ;
  unichar * chaine ;

  for (chaine = s->canonique ; *chaine ; chaine++) {
    if (* chaine == '!') {
      *chaine = 0 ;
      temp = (tListe *) calloc(1, sizeof(tListe)) ;
      if (! temp)
	erreurMem("separeFormes") ;
      temp->suiv = liste ;
      liste = temp ;
      liste->forme = chaine + 1 ;
    }
  }
  return liste ;
}

/*************************************************************/
static tListe * trier(tListe * liste) {
  /* Trie une liste de formes dans l'ordre croissant. Supprime les doublons. */
  /* Modifie et renvoie la liste donnee en entree. */
  tListe * crt, * p, * temp ;
  unichar * fTemp ;
  /*affListe(liste) ;*/
  for (crt = liste ; crt ; crt = crt -> suiv)
    while (crt -> suiv &&
	   u_strcmp( crt -> suiv -> forme,  crt -> forme) < 0)
      if (u_strcmp( crt -> suiv -> forme,  liste -> forme) < 0) {
	/* insertion en debut */
	/*printf("%d < %d, insertion en tete\n",
	  crt -> suiv -> forme + 1, crt -> forme + 1) ;*/
	fTemp = liste -> forme ;   /* 1e forme */
	liste -> forme = crt -> suiv -> forme ;  /* nouvelle forme */
	crt -> suiv -> forme = fTemp ;
	if (liste != crt) {
	  temp = liste -> suiv ;  /* contient le 2eme */
	  liste -> suiv = crt -> suiv ;   /* contient le 1er */
	  crt -> suiv = liste -> suiv -> suiv ;
	  liste -> suiv -> suiv  = temp ; }
	/*affListe(liste) ;*/ }
      else { /* autres cas d'insertion */
	/*printf("%d < %d, insertion apres tete\n",
	  crt -> suiv -> forme + 1, crt -> forme + 1) ;*/
	for (p = liste ;
	     u_strcmp( p -> suiv -> forme,  crt -> suiv -> forme)
               < 0 ;
	     p = p -> suiv) ;
	/* p -> forme < crt -> suiv -> forme == p -> suiv -> forme */
	/* on va inserer le nouveau apres p */
	temp = p -> suiv ;  /* contient le successeur de p */
	p -> suiv = crt -> suiv ;   /* contient le nouveau */
	crt -> suiv = p -> suiv -> suiv ;
	p -> suiv -> suiv  = temp ;
	/*affListe(liste) ;*/ }
  /*affListe(liste) ;*/
  for (crt = liste ; crt ; crt = crt -> suiv)
    while (crt -> suiv &&
	   ! u_strcmp( crt -> forme,  crt -> suiv -> forme)) {
      temp = crt -> suiv ;
      crt -> suiv = temp -> suiv ;
      free(temp) ; }
  /*affListe(liste) ;*/
  return liste ; }
