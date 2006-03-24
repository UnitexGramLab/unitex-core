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

/* compgr.cpp */

/* Date 	: juin 98 */
/* Auteur(s) 	: MAYER Laurent et al */
/* compilation des grammaires de levee d ambiguites */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


#include "autalmot.h"
#include "fst_file.h"
#include "list_aut.h"
#include "compgr.h"

#include "utils.h"



/* maximum number of state for a grammar before we split it in several fst2 */

#define MAX_GRAM_SIZE   128



static bool prepareRegle(tRegle * regle) ;
static int compteContraintes(autalmot_t * aut, etat * contrainte) ;
static autalmot_t * compileRegle(tRegle * regle);
static int separeAut(autalmot_t * autLu, autalmot_t * r, etat depart, int symbtype) ;
static void separeCont(autalmot_t * autLu, autalmot_t * r, etat depart, etat arrivee) ;
static int suivre(autalmot_t * autLu, autalmot_t * r, etat e, int delim, int * corresp) ;
static bool suivreCont(autalmot_t * autLu, autalmot_t * r, etat depart, etat arrivee, int * corresp) ;
static autalmot_t * combinaison(tRegle * regle, int ens, autalmot_t * AetoileR1, autalmot_t * R2Aetoile) ;




static inline void chomp(char * s) {
  while (*s) {
    if (*s == '\n' || *s == '\r' || *s == '#') {
      *s = 0;
    } else { s++; }
  }
}


static inline void strip_extension(char * s) {

  char * p = s + strlen(s) - 1;
  
  while (p > s) {
    if (*p == '.') { *p = 0; break; }
    if (*p == '/' || *p == '\\') { break; }
    p--;
  }
}




/* sortie: L(aut) = A* . L(aut)
 * si A est vide alors le resultat est vide: A* . empty = empty
 */

static inline void AStar_aut(autalmot_t * A) {

  symbol_t * UNIV = symbol_LEXIC_new();

  for (int i = 0; i < A->nbinitials; i++) { autalmot_add_trans(A, A->initials[i], UNIV, A->initials[i]); }

  symbol_delete(UNIV);
}



/* sortie: L(aut) = L(aut) . A*
 */

static inline void aut_AStar(autalmot_t * A) {

  symbol_t * UNIV = symbol_LEXIC_new();

  for (int i = 0; i < A->nbstates; i++) {

    if (A->states[i].flags & AUT_TERMINAL) {

      /* on detruit les transitions sortantes */

      transitions_delete(A->states[i].trans);
      A->states[i].trans = NULL;

      /* remplace par un boucle */
      autalmot_add_trans(A, i, UNIV, i);
    }
  }

  symbol_delete(UNIV);
}





static tRegle * newRegle(char * fname) {

  tRegle * regle = (tRegle *) xmalloc(sizeof(tRegle));

  regle->autLu = NULL;
  regle->nbContextes = 0;
  regle->contexte =  NULL;

  regle->nom = strdup(fname);

  if ((regle->autLu = load_grammar_automaton(regle->nom)) == NULL) {
    error("cannot load '%s' automaton.\n", fname);
    free(regle->nom);
    free(regle);
    return NULL;
  }

  prepareRegle(regle);

  return regle;
}



static void deleteRegle(tRegle * regle) {

  free(regle->nom);

  for (int i = 0; i < regle->nbContextes; i++) {
    autalmot_delete(regle->contexte[i].D);
    autalmot_delete(regle->contexte[i].G);
  }

  free(regle->contexte);

  free(regle);
}





static autalmot_t * compileRegle(tRegle * regle) {

  int c, p, ens ;

  printf("Compiling %s ... (%d contextes)\n", regle->nom, regle->nbContextes);

  for (c = 0; c < regle->nbContextes; c++) {

    autalmot_determinize(regle->contexte[c].G);
    autalmot_emonde(regle->contexte[c].G);

    autalmot_determinize(regle->contexte[c].D);
    autalmot_emonde(regle->contexte[c].D);
  }


  /* Construction de A*R1 : */

  AStar_aut(regle->contexte[0].G);
  autalmot_determinize(regle->contexte[0].G);
  autalmot_minimize(regle->contexte[0].G);

  autalmot_t * AetoileR1 = regle->contexte[0].G;


  /* Construction de R2A* : */

  aut_AStar(regle->contexte[0].D);
  autalmot_determinize(regle->contexte[0].D);
  autalmot_minimize(regle->contexte[0].D);

  autalmot_t * R2Aetoile = regle->contexte[0].D;


  p = (int) pow(2, regle->nbContextes - 1);

  autalmot_t * res = autalmot_new();

  for (ens = 0 ; ens < p ; ens++) {

    autalmot_t * a1 = combinaison(regle, ens, AetoileR1, R2Aetoile);

    res = autalmot_union(res, a1);

    printtime(autalmot_determinize(res));
    printtime(autalmot_minimize(res));
  }

  debug("compileRegle: out of combis (%d states)\n", res->nbstates);

  debug("compl\n");
  printtime(autalmot_complementation(res));

  debug("emonde\n");
  printtime(autalmot_emonde(res));

  if (res->nbstates == 0) { warning("grammar %s forbids everything.\n", regle->nom); }

  printf("grammar %s compiled. (%d states)\n", regle->nom, res->nbstates);

  return res;
}




int compile_grammar(char * gram, char * outname) {

  tRegle * regle = newRegle(gram);
  if (regle == NULL) { error("unable to read grammar '%s'\n", gram); return -1; }

  debug("regle\n");

  autalmot_t * A;
  if ((A = compileRegle(regle)) == NULL) { die("unable to compile rule '%s'\n", gram); return -1; }

  debug("after compile\n");

  deleteRegle(regle);

  debug("after delete\n");

  autalmot_output_fst2(A, outname, FST_GRAMMAR);
  autalmot_delete(A);

  debug("endofcompile\n");

  return 0;
}


/* Lit les regles de levee d'ambiguites et les compile sous la forme d'automates. 
 * nomFichNomRegles est le fichier des noms des grammaires. 
 * nomFichNomGramm est le fichier de la composition des grammaires compilées.
 */


int compile_rules(char * rulesname, char * outname) {

  printf("Compilation of %s\n", rulesname);

  FILE * f = NULL;
  FILE * frules = fopen(rulesname, "r");
  if (frules == NULL) { die("cannot open file '%s'\n", rulesname); }

  FILE * out = fopen(outname, "w");
  if (out == NULL) { die("cannot open file '%s'\n", outname); }

  char fstoutname[MAX_PATH]; // le nom du fichier qui contient l'automate resultat (fst2)

  int nbregles = 0;
  char buf[MAX_PATH];

  time_t debut = time(0);

  autalmot_t * res = NULL, * A;
  int fstno = 0;
  ustring_t * ustr = ustring_new();




  while (fgets(buf, MAX_PATH, frules)) {

    chomp(buf);
    if (*buf == 0) { continue; }

    printf("\n\n%s ...\n", buf);

    strip_extension(buf);
    strcat(buf, ".elg");

    if ((f = fopen(buf, "r")) == NULL) { // .elg doesn't exist, making one

      strip_extension(buf);
      printf("precompiling %s.fst2\n", buf);

      strcat(buf, ".fst2");
      
      tRegle * regle = newRegle(buf);
      if (regle == NULL) { die("unable to read grammar '%s'\n", buf); }
      
      if ((A = compileRegle(regle)) == NULL) { die("unable to compile rule '%s'\n", buf); }

      deleteRegle(regle);

      /* saving result in name.elg  ? */
      /*
      strip_extension(buf);
      strcat(buf, ".elg");
      autalmot_output_fst2(A, buf, FST_GRAMMAR);
      */
    } else {
      fclose(f);
      printf("using already exiting %s\n", buf);
      A = load_grammar_automaton(buf);      
      if (A == NULL) { die("unable to load '%s'\n", buf); }
    }


    if (A->nbstates == 0) { error("grammar %s forbids everything!\n", buf); }
 
    debug("regroupe\n");

    printtime(if (res) {

      autalmot_t * tmp = res;
      res = autalmot_intersection(tmp, A);
      autalmot_delete(tmp);
      autalmot_delete(A);
      autalmot_emonde(res);

    } else { res = A; });

    debug("regroupe done (nbstates = %d)\n", res->nbstates);

    fprintf(out, "\t%s\n", buf);
    nbregles++;


    if (res->nbstates > MAX_GRAM_SIZE) {

      autalmot_minimize(res, 1);

      sprintf(fstoutname, "%s-%d.elg", outname, fstno++);
      fprintf(out, "<%s>\n", fstoutname);

      printf("splitting big grammar in '%s' (%d states)\n", fstoutname, res->nbstates);

      ustring_printf(ustr, "%s: compiled elag grammar", fstoutname);
      free(res->name);
      res->name = u_strdup(ustr->str);

      autalmot_output_fst2(res, fstoutname, FST_GRAMMAR);

      autalmot_delete(res);
      res = NULL;
    }
  }


  
  if (res) {

    sprintf(fstoutname, "%s-%d.elg", outname, fstno++);
    fprintf(out, "<%s>\n", fstoutname);

    printf("outputing grammar in '%s'(%d states)\n", fstoutname, res->nbstates);

    autalmot_minimize(res, 1);

    ustring_printf(ustr, "%s: compiled elag grammar", fstoutname);
    free(res->name);
    res->name = u_strdup(ustr->str);

    autalmot_output_fst2(res, fstoutname, FST_GRAMMAR);

    autalmot_delete(res);
  }

  time_t fin = time(0);

  fclose(frules);
  fclose(out);

  ustring_delete(ustr);

  printf("\ndone.\nElapsed time: %.0f s.\n", difftime(fin, debut));

  printf("\n%d rule%s from %s compiled in %s (%d automat%s).\n", nbregles, (nbregles > 1) ? "s" : "", rulesname, outname,
	 fstno, (fstno > 1) ? "a" : "on");

  return 0;
}



/* make autalmot for pattern matching of context ...
 */

static autalmot_t * make_locate_auto(tRegle * regle) {

  autalmot_t * res = autalmot_dup(regle->contexte[0].G);
  autalmot_concat(res, regle->contexte[0].D);

  return res;
}



/* Reconnait les 4 parties d'une regle
 */

static bool prepareRegle(tRegle * regle) {

  debug("PrepareRegle(%s)\n", regle->nom);

  int finR1, finR2, finC2, nbContraintes, c;
  /* Etats buts des transitions etiquetees par les '=' medians des contraintes. */
  int contrainte[maxContraintes];
  bool succes = true;

  nbContraintes = compteContraintes(regle->autLu, contrainte);

  regle->nbContextes = nbContraintes + 1;
  regle->contexte    = (tContexte *) xmalloc(regle->nbContextes * sizeof(tContexte));

  for (c = 0; c < regle->nbContextes; c++) { regle->contexte[c].G  = regle->contexte[c].D = NULL; }

  finR1 = finR2 = finC2 = NEXISTEPAS ;


  for (transition_t * t = regle->autLu->states[0].trans; t ; t = t->next) {

    switch (t->label->type) {

    case EXCLAM:
      if (regle->contexte[0].G) { die("too much '!'\n", regle->nom); }

      regle->contexte[0].G = autalmot_new();
      finR1 = separeAut(regle->autLu, regle->contexte[0].G, t->to, EXCLAM);

      regle->contexte[0].D = autalmot_new();
      finR2 = separeAut(regle->autLu, regle->contexte[0].D, finR1, EXCLAM);

      if (finR1 == NEXISTEPAS || finR2 == NEXISTEPAS
          || ! (regle->autLu->states[finR2].flags & AUT_FINAL)) {
	succes = false;
      }
      break;


    case EQUAL:
      if (regle->contexte[1].G) { die("nondeterministic .fst file\n") ; }

      for (c = 0; c < nbContraintes; c++) {

	regle->contexte[c + 1].G = autalmot_new();
	separeCont(regle->autLu, regle->contexte[c + 1].G, t->to, contrainte[c]);

	regle->contexte[c + 1].D = autalmot_new();
	finC2 = separeAut(regle->autLu, regle->contexte[c + 1].D, contrainte[c], EQUAL) ;

	if (finC2 == NEXISTEPAS || ! autalmot_is_final(regle->autLu, finC2)) { succes = false ; }
      }
      break ;

    default :
      debug("ohoh\n");
      symbol_dump(t->label);
      die("in grammar: left delimitor '!' or '=' lacking\n");
    }
  }

  autalmot_delete(regle->autLu);
  regle->autLu = NULL;

  if (! regle->contexte[0].G) {
    die("in grammar '%s': symbol <!> not found.\n", regle->nom);
  }
  if (! succes) { die("prepareRegle: %s: parse error\n", regle->nom) ; }


  char buf[MAX_PATH];
  strcpy(buf, regle->nom);
  strcpy(buf + strlen(buf) - 5, "-conc.fst2");

  /* make concordance auto for Locate */

  autalmot_t * locate = make_locate_auto(regle);

  autalmot_output_fst2(locate, buf, FST_LOCATE);
  autalmot_delete(locate);

  debug("end of prepare\n");

  return succes;
}








/* Compte les contraintes dans aut.
 * Place dans "contrainte" les etats buts des transitions
 * etiquetees par les '=' medians des contraintes.
 * Verifie que toutes les transitions entrant dans ces etats
 * sont etiquetees par '='.
 */

static int compteContraintes(autalmot_t * aut, int * contrainte) {

  int source = 0, e;
  transition_t * t;
  int c, nbContraintes = 0;
  
  for (t = aut->states[0].trans; t && ! source; t = t->next) {
    if (t->label->type == EQUAL) {
      if (t->to == 0) { die("illegal cycle in grammar\n"); }
      source = t->to;
    }
  }


  if (! source) { // no contraint
    return 0;
  }

  for (e = 1; e < aut->nbstates; e++) {

    for (t = aut->states[e].trans; t; t = t->next) {

      if (t->to != source && t->label->type == EQUAL && ! autalmot_is_final(aut, t->to)) {

	for (c = 0; c < nbContraintes; c++) {
	  if (contrainte[c] == t->to) { break; }
	}

	if (c == nbContraintes) {
	  if (++nbContraintes >= maxContraintes) {
            die("too many constraints with same condition\n");
          }
	  contrainte[c] = t->to;
	}
      }
    }
  }

  if (nbContraintes == 0) { die("middle delimitor '=' not found\n"); }

  return nbContraintes;
}



/* Copie autLu dans r depuis depart jusqu a delim. 
 * Ne copie pas les transitions etiquetees par delim.
 * Preconditions : autLu a une transition etiquetee delim et dont le but est depart.
 * Renvoie l'etat but des transitions etiquetees par delim.
 */

static int separeAut(autalmot_t * autLu, autalmot_t * r, etat depart, int delim) {

  int * corresp;                /* et non unsigned int, car NEXISTEPAS < 0 */
  int fin = NEXISTEPAS;         /* Etat but des transitions etiquetees par delim. */
  int e;                        /* Etat de autLu. */

  autalmot_empty(r);
  autalmot_resize(r, autLu->nbstates);
  autalmot_add_state(r, AUT_INITIAL);

  corresp = (int *) xmalloc(autLu->nbstates * sizeof(int));

  for (e = 0; e < autLu->nbstates ; e++) { corresp[e] = NEXISTEPAS; }

  corresp[depart] = 0;
  fin = suivre(autLu, r, depart, delim, corresp);
  free(corresp);

  if (fin == NEXISTEPAS) { /* suivre n'a pas rencontre delim. */
    die("in grammar: middle or end delimitor lacking\n");
  }

  /* NE PAS Verifier que toutes les transitions entrantes dans fin
   * sont etiquetees par delim.
   * (ce n'est pas necessaire a la correction de la grammaire)
   */

  /*
  if (! autalmot_is_final(autLu, fin))
    for (e = 0; e < autLu->nbstates; e++)
      for (t = autLu->states[e].trans; t; t = t->next)
	if (t->to == fin && t->label->type != delim)
	  die("\nInternal error [separeAut], '%d'\n", delim);
  */
  return fin;
}


/* Copie autLu dans r depuis depart jusqu a arrivee
 * Ne copie pas les transitions etiquetees par '='.
 * Preconditions : dans autLu, une transition etiquetee '=' entre dans
 * depart et une autre dans arrivee ; depart est dans la partie gauche
 * d'une contrainte et arrivee dans la partie droite d'une contrainte.
 */

static void separeCont(autalmot_t * autLu, autalmot_t * r, etat depart, etat arrivee) {

  int * corresp ;   /* et non unsigned int, car NEXISTEPAS < 0 */
  etat e ;   /* Etat de autLu. */
  bool trouve ;


  autalmot_empty(r);
  autalmot_resize(r, autLu->nbstates);

  autalmot_add_state(r, AUT_INITIAL);

  corresp = (int *) xcalloc(autLu->nbstates, sizeof(int)) ;

  for (e = 0; e < autLu->nbstates; e++) { corresp[e] = NEXISTEPAS; }

  corresp[depart] = 0 ;

  trouve = suivreCont(autLu, r, depart, arrivee, corresp) ;

  if (! trouve) { die("separeCont"); } /* suivreCont n'a pas rencontre arrivee. */

  free(corresp);
}



/* Copie autLu dans r depuis l'etat e de autLu jusqu'a delim. 
 * corresp donne pour chaque etat de autLu son equiv. dans r. 
 * Ne copie pas les transitions etiquetees par delim. 
 * Rend l'etat but des transitions etiquetees par delim 
 * s'il est rencontre. Rend NEXISTEPAS s'il n'est pas rencontre.
 */


static int suivre(autalmot_t * autLu, autalmot_t * r, etat e, int delim, int * corresp) {

  transition_t * t;
  int f, fin = NEXISTEPAS;

  //  debug("suivre(%d,'%c')\n", e, delim);


  for (t = autLu->states[e].trans; t; t = t->next) {

    if (t->label->type == delim) { /* on a trouve delim */

      //      debug("delim! (at %d)\n", t->to);

      if (fin != NEXISTEPAS && fin != t->to) { die("[suivre]: too much '%c' in rule, %d, %d\n", delim, fin, t->to) ; }
      fin = t->to;
      r->states[corresp[e]].flags |= AUT_TERMINAL;    /* corresp[e] devient terminal */

    } else { /* transition normale, on va la copier */

      if (corresp[t->to] == NEXISTEPAS) {   /* nouvel etat */

	corresp[t->to] = autalmot_add_state(r);

	autalmot_add_trans(r, corresp[e], t->label, corresp[t->to]);

	f = suivre(autLu, r, t->to, delim, corresp);

	if (f != NEXISTEPAS) {

	  if ((fin != NEXISTEPAS) && (f != fin)) { die("suivre: too much '%C' in rule (in state %d & %d)\n", delim, fin, f); }

	  fin = f;

	}

      } else { autalmot_add_trans(r, corresp[e], t->label, corresp[t->to]); }
    }
  }

  /* Si toutes les transitions sortant de e entrent dans des etats deja
   * explores auparavant, on peut avoir une activation de suivre qui
   * ne rencontre pas le delimiteur de fin. Dans ce cas, fin == NEXISTEPAS.
   */

  return fin;
}


/* Copie autLu dans r depuis l etat depart de autLu jusqu a arrivee. */
/* corresp donne pour chaque etat de autLu son equiv. dans r. */
/* Ne copie pas les transitions etiquetees par '='. */

static bool suivreCont(autalmot_t * autLu, autalmot_t * r, etat depart, etat arrivee, int * corresp) {

  bool trouve = false;
  transition_t * t;

  for (t = autLu->states[depart].trans; t; t = t->next) {

    if (t->label->type == '=') {

      if (t->to == arrivee && ! (autalmot_is_final(r, corresp[depart]))) {
	trouve = true;
	autalmot_set_final(r, corresp[depart]);    /* corresp[e] devient terminal */
      }


    } else {                   /* transition normale, on va la copier */

      if (corresp[t->to] == NEXISTEPAS) {   /* nouvel etat */

	corresp[t->to] = autalmot_add_state(r);

	autalmot_add_trans(r, corresp[depart], t->label, corresp[t->to]);

	if (suivreCont(autLu, r, t->to, arrivee, corresp)) { trouve = true; }

      } else { autalmot_add_trans(r, corresp[depart], t->label, corresp[t->to]); }
    }
  }

  /* Si toutes les transitions sortant de e entrent dans des etats deja
   * explores auparavant, on peut avoir une activation de suivreCont qui
   * ne rencontre pas arrivee. Dans ce cas, trouve == false.
   */

  return trouve;
}






/* Parcours de l'ensemble des contraintes. Chaque contrainte est
 * representee par un bit dans la representation binaire de ens.
 * Postcondition : l'automate resultat est deterministe et complet.
 * Il peut ne pas avoir d'etats.
 * 
 * retourne:  (A*.R1 \ (U_{i in ens} (A*.Ci,1))) (R2.A* \ (U_{i not in ens} (Ci,2.A*)))
 */


static autalmot_t * combinaison(tRegle * regle, int ens, autalmot_t * AetoileR1, autalmot_t * R2Aetoile) {

  // if (ens > 0) { die("combinaison: enough!\n"); }

  errprintf("\ncombinaison(%d)\n", ens);

  autalmot_t * a1 = autalmot_new();
  autalmot_t * a2 = autalmot_new();

  int c, dpc; /* dpc = pow(2, c - 1) */

  for (c = 1, dpc = 1; c < regle->nbContextes; c++, dpc = dpc << 1) {

    if (dpc & ens) {    /* le c-ieme bit de ens vaut 1 */

      a1 = autalmot_union(a1, autalmot_dup(regle->contexte[c].G));

      autalmot_determinize(a1);
      autalmot_minimize(a1);

    } else {    /* le c-ieme bit de ens vaut 0 */

      a2 = autalmot_union(a2, autalmot_dup(regle->contexte[c].D));

      autalmot_determinize(a2);
      autalmot_minimize(a2);
    }
  }

  errprintf("\nI\n");

  //  debug("U_{i in E} (Ci,1):\n"); autalmot_dump(a1);
  //  debug("U_{i not in E} (Ci,2):\n"); autalmot_dump(a2);

  /* L(a1) = U_{i in I} R1_i
   * L(a2) = U_{i not in I} R2_i */

  printtime(AStar_aut(a1));
  
  printtime(autalmot_determinize(a1));

  printtime(autalmot_minimize(a1));

  printtime(autalmot_complementation(a1));

  printtime(autalmot_emonde(a1));

  autalmot_t * tmp = a1;
  printtime(a1 = autalmot_intersection(a1, AetoileR1));

  printtime(autalmot_delete(tmp));

  printtime(autalmot_emonde(a1));

  printtime(autalmot_minimize(a1));  /* emonder a3 ? */

  /* L(a1) =  (A*.R1 \ (U_{i in ens} (A*.Ci,1))) */

  errprintf("\nII\n");

  printtime(aut_AStar(a2));

  printtime(autalmot_determinize(a2));

  printtime(autalmot_minimize(a2));
    
  printtime(autalmot_complementation(a2));

  printtime(autalmot_emonde(a2));

  tmp = a2;
  printtime(a2 = autalmot_intersection(a2, R2Aetoile));
  autalmot_delete(tmp);


  /* autalmot_determinize(a2) ; */

  printtime(autalmot_emonde(a2));
  printtime(autalmot_minimize(a2));

  /* L(a2) == (R2.A* \ (U_{i not in ens} (Ci,2.A*))) */

  errprintf("\nIII\n");

  printtime(autalmot_concat(a1, a2));

  autalmot_delete(a2);

  printtime(autalmot_emonde(a1));

  printtime(autalmot_determinize(a1));

  printtime(autalmot_emonde(a1));  /* utile quand tous les etats sont inutiles */

  printtime(autalmot_minimize(a1));

  debug("end of combi(%d)\n\n", ens);

//  autalmot_dump(a1);

  return a1;
}
