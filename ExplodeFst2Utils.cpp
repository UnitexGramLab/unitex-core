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

#include "String_hash.h"
#include "ExplodeFst2Utils.h"
#include "utils.h"
#include "DELA.h"
#include "Transitions.h"


static unichar PONCTAB[] = {
  '"', '\'',
  '+', '-', '*', '\\', '=',
  '.', ',', ':', ';', '!', '?',
  '(', ')', '[', ']', '<', '>',
  '%', '#', '@', '/', '$', '&',
  '|', '_', 0
};



static int compte_flex(unichar * gramm) {

  int nb = 0;

  while (*gramm) {
    if (*gramm == ':') { nb++; }
    gramm++;
  }

  return nb;
}


// assume symb->sorteSymbole == ATOM

int symbole_developp(tAlphMot * alphabet, tSymbole * symb) {

  int nbflex = compte_flex(symb->gramm);

  if (nbflex < 2) {
    add_symbol(alphabet, symb);
    return 1;
  }

  int i, k, n;
  unichar common[maxGramm];
  unichar * p;

  for (i = 0; symb->gramm[i] != ':'; i++) { common[i] = symb->gramm[i]; }
  common[i] = 0;

  tSymbole * s = tSymbole_new();

  symbole_copy(s, symb);

  for (p = symb->gramm + i, n = 0; n < nbflex; n++) {

    unichar flex[maxGramm];
    k = 0; p++;

    while (*p && *p !=  ':') { flex[k++] = *(p++); }
    flex[k] = 0;

    u_sprintf(s->gramm, "%S:%S", common, flex);

    add_symbol(alphabet, s);
  }

  symbole_delete(s);

  return nbflex;
}




/* text label can be either a full DELA entry, either a punc symbol, either an unknow word
 */

static int check_text_label(const unichar * label) {
const unichar * p;
if (label==NULL) {error("check label: no label\n"); return -1; }

  if (*label == 0) { error("error: check label: label is empty\n"); return -1; }

if (label[0]=='{' && label[1]!='\0') {
   /* If we have something that looks like a tag token of the form {__,__.__} */
   return (check_tag_token((unichar*)label)==1)?0:-1;
}
  /* no spaces */

  for (p = label; *p; p++) {
    if ((*p == ' ') || (*p == '\t') || (*p == '\n')) { error("malformed label: '%S'.\n", label); return -1; }
  }

  return 0;
}




void load_text_symbol(tSymbole * symb, unichar * lex) {

  int i, j;

  if (check_text_label(lex) == -1) { fatal_error("bad text label: \"%S\"\n", lex); }

  symb->sorteSymbole = ATOME;


  if (*lex == '{') {   // {__,__.__}

    /* inflected form */

    i = 1, j = 0;

    while (*(lex + i) != ',') {

      symb->flechie[j++] = lex[i++];

      if (j >= maxMot) { fatal_error("inflected form '%S' is too long.\n", lex); }
    }

    symb->flechie[j] = 0;
    i++;


    /* forme canonique */

    for (j = i ; *(lex + j) != '.'; j++);
    symb->canonique = (unichar *) malloc((j + 1 - i) * sizeof(unichar));

    j = 0 ;
    while (*(lex + i) != '.') { symb->canonique[j++] = lex[i++]; }

    symb->canonique[j] = 0;
    i++;


    /* traits gramma ... */

    j = 0;
    while (lex[i] && lex[i] != '}' && j < maxGramm - 1) { symb->gramm[j++] = lex[i++]; }
    symb->gramm[j] = 0;


  } else {    /* mot inconnu dans un texte ou ponctuation (pas d'accolade, pas de point, pas de virgule) */


    if (lex[0] == '\\') {

      if (! lex[1] || lex[2]) { fatal_error("illegal label '%S'\n", lex); }

      *symb->flechie  = 0;
      symb->canonique = u_strdup(lex);

      u_strcpy(symb->gramm, "PNC");


    } else if (u_strchr(PONCTAB, *lex)) {

      if (lex[1]) { fatal_error("illegal label text: '%S'\n", symb->flechie); }

      *symb->flechie  = 0;
      symb->canonique = u_strdup(lex);

      u_strcpy(symb->gramm, "PNC");


    } else if (u_is_digit(*lex)) { /* chiffre arabe */

      if (lex[1]) { fatal_error("illegal label text: '%S'\n", symb->flechie); }

      *symb->flechie  = 0;
      symb->canonique = u_strdup(lex);

      u_strcpy(symb->gramm, "CHFA");


    } else { // mot inconnu

      i = 0;
      while (lex[i]) {
   if (i >= maxMot) { fatal_error("inflected form too long in '%S'.\n", lex); }
   symb->flechie[i] = lex[i];
   i++;
      }
      symb->flechie[i] = 0;

      symb->canonique    = (unichar *) xmalloc(sizeof(unichar));
      symb->canonique[0] = 0;

      symb->gramm[0] = '?';
      symb->gramm[1] = 0;
    }
  }
}


list_aut_old * load_text_automaton(char * fname, bool developp) {


  //unichar buf[1024];

  u_printf("Loading text automaton...\n");
  Fst2 * A = load_fst2(fname, 1);
  if (A == NULL) { fatal_error("cannot load %s\n", fname); }

  list_aut_old * res = (list_aut_old *) xmalloc(sizeof(list_aut_old));

  res->nb_aut    = A->number_of_graphs;
  res->les_aut   = (tAutAlMot **) xmalloc(sizeof(tAutAlMot *) * res->nb_aut);

  tAlphMot * alphabet = alphabet_new();

  for (int i = 0; i < res->nb_aut; i++) {

    int nb   = i + 1;  /* automate in fst2 start at index 1 */
    int base = A->initial_states[nb];

    tAutAlMot * aut = initAutAlMot(A->number_of_states_per_graphs[nb]);

    aut->name = u_strdup(A->graph_names[nb]);

    for (int q = 0; q < (int) aut->nbEtats; q++) {

      int qq = base + q;

      aut->type[q] = 0;

      if (is_final_state(A->states[qq])) { aut->type[q] |= AUT_TERMINAL; }
      if (is_initial_state(A->states[qq])) { aut->type[q] |= AUT_INITIAL;  }

      aut->etats[q] = NULL;

      for (Transition * trans = A->states[qq]->transitions; trans; trans = trans->next) {

	//tSymbole symb;

        tSymbole * symb = tSymbole_new();

	//u_strcpy(buf, A->tags[trans->tag_number]->input);
        //        debug("in=%S\n", A->tags[trans->tag_number]->input);
	load_text_symbol(symb, A->tags[trans->tag_number]->input);
        //debug("out=(%d:%S,%S.%S)\n", symb->sorteSymbole, symb->flex->str, symb->canonique, symb->gramm);

        if (developp) { // developp symbols

          alphabet_clear(alphabet);

          int nbflex = symbole_developp(alphabet, symb);

          if (nbflex == 0) { nouvTrans(aut, q, NULL, trans->state_number - base); }

          while (nbflex--) {
            nouvTrans(aut, q, alphabet->symb + nbflex, trans->state_number - base);
            free(alphabet->symb[nbflex].canonique);
            alphabet->symb[nbflex].canonique = NULL;
            free_Ustring(alphabet->symb[nbflex].flex);
            alphabet->symb[nbflex].flex  = NULL;
          }

        } else { nouvTrans(aut, q, symb, trans->state_number - base); }
        
        symbole_delete(symb);
      }
    }
    res->les_aut[i] = aut;
  }


  free_SymbolAlphabet(alphabet);
  free_Fst2(A);

  return res;
}



void text_output_fst2(list_aut_old * txt, FILE * f) {

  unichar buf[1024];
  u_strcpy(buf, "0000000000");

  int i = 9;
  int n = txt->nb_aut;

  while (n) {
    buf[i--] = '0' + (n % 10);
    n = n / 10;
  }

  u_fprintf(f, "%S\n", buf);


  string_hash * hash = new_string_hash();


  /* add epsilon before all other labels */

  u_strcpy(buf, "<E>");

  get_value_index(buf, hash);


  for (i = 0; i < txt->nb_aut; i++) {

    tAutAlMot * A = txt->les_aut[i];

    u_fprintf(f, "-%d %S\n", i + 1, A->name);

    for (int q = 0; q < A->nbEtats; q++) {

      u_fputc((A->type[q] & AUT_TERMINAL) ? 't' : ':', f);
      u_fputc(' ', f);

      for (tTransitions * t = A->etats[q]; t; t = t->suivant) {

	if (t->etiq == NULL) {

	  error("<def> trans on a txt automaton???\n");
	  u_strcpy(buf, "<def>");

	} else {

	  switch (t->etiq->sorteSymbole) {
	    
	  case ATOME:

	    //if (*t->etiq->flechie == 0) { // PNC, CHFA ...

            if (t->etiq->flex->len == 0) {
	      u_sprintf(buf, "%S", t->etiq->canonique);

	    } else if (*t->etiq->gramm == '?') { // unknow word
	      
	      u_sprintf(buf, "%S", t->etiq->flex->str);

	    } else {

	      u_sprintf(buf, "{%S,%S.%S}", t->etiq->flex->str, t->etiq->canonique, t->etiq->gramm);
	    }
	    break;

	  case SPECIAL:
	    error("SPECIAL label in text automaton???\n");
	    u_strcpy(buf, t->etiq->flex->str);
	    break;

	  case UNIVERSEL:
	    error("<.> label in text automaton???\n");
	    u_sprintf(buf, "<.>");
	    break;

	  case CODE_POUET:
	    error("CODE_POUET in text automaton???\n");
	    u_sprintf(buf, "<%S>", t->etiq->gramm);
	    break;

	  case NEGATIF:
	  case INCOMPLET:
	    error("INCOMPLET label in text automaton???\n");
	    u_sprintf(buf, "<%S.%S>", t->etiq->canonique, t->etiq->gramm);
	    break;

	  case INDETERMINE:
	    error("ouptut_fst2: symbol code is INDETERMINE\n", t->etiq->sorteSymbole);
	    u_sprintf(buf, "<INDETERMINE:%S,%S.%S>", t->etiq->flex->str, t->etiq->canonique, t->etiq->gramm);
	  }
	}

	u_fprintf(f, "%d %d ", get_value_index(buf, hash), t->but);
      }

      u_fputc('\n', f);
    }

    u_fprintf(f,"f \n");
  }
  output_fst2_labels(hash, f);
  free_string_hash(hash);
}



int text_output_fst2_fname(list_aut_old * txt, char * fname) {

  FILE * f = u_fopen(fname, U_WRITE);

  if (f == NULL) { error("output_fst2: unable to open %s for writing\n", fname); return -1; }

  text_output_fst2(txt, f);

  fclose(f);

  return 0;
}



/* Cree une nouvelle transition de source vers but etiquetee s. 
 * Il n'y a pas de partage de memoire entre s et l'etiquette de la transition.
 */

void nouvTrans(tAutAlMot * a, int source, tSymbole * s, int but) {

  tTransitions * t = (tTransitions *) xmalloc(sizeof(tTransitions)) ;

  //  t->etiq    = s ? copieSymbole(s) : NULL;
  t->etiq = symbole_dup(s);
  t->but     = but;
  t->suivant = a->etats[source];
  a->etats[source] = t;
}


/* Postconditions : la taille est egale au nombre d'etats ;
 * l'unique etat initial est 0, sauf s'il n'y a pas d'etats.
 */

tAutAlMot * initAutAlMot(int nbEtats) {

  tAutAlMot * aut = (tAutAlMot *) xcalloc(1, sizeof(tAutAlMot));

  aut->name           = NULL;
  aut->nbEtats        = nbEtats; 
  aut->taille         = nbEtats; 
  aut->entrantesEtats = NULL;

  if (nbEtats == 0) {

    error("automaton with no states.\n") ;
    aut->nbEtatsInitiaux = 0 ;

  } else {

    aut->etats = (tTransitions **) xcalloc(nbEtats, sizeof(tTransitions *)) ;
    aut->type  = (char *) xcalloc(nbEtats, sizeof(char)); 

    marqueEtatInitial(aut);
  }

  return aut ;
}


/* Marque 0 comme etat initial de l automate.
 */

void marqueEtatInitial(tAutAlMot * aut) {
  aut->nbEtatsInitiaux = 1;
  aut->initial = (int *) xcalloc(1, sizeof(int));
  aut->initial[0] = 0;
  //  aut->type[0] |= AUT_INITIAL; 
}



tSymbole * add_symbol(tAlphMot * alphabet, tSymbole * symb) {

  while (alphabet->nbSymboles >= alphabet->size) {
    alphabet->size = alphabet->size * 2;
    alphabet->symb    = (tSymbole *) xrealloc(alphabet->symb, alphabet->size * sizeof(tSymbole));
  }

  tSymbole * addr = alphabet->symb + alphabet->nbSymboles;
  addr->flex = new_Ustring();
  symbole_copy(addr, symb);

  alphabet->nbSymboles++;

  return addr;
}


tSymbole * tSymbole_new() {
  tSymbole * res = (tSymbole *) xmalloc(sizeof(tSymbole));
  res->sorteSymbole = UNIVERSEL;
  res->flex = new_Ustring();
  res->canonique = NULL;
  return res;
}

void symbole_delete(tSymbole * symb) {
  if (! symb) { return; }
  free_Ustring(symb->flex);
  free(symb->canonique); 
  free(symb);
}


void symbole_copy(tSymbole * dest, tSymbole * src) {
  dest->sorteSymbole = src->sorteSymbole;
  u_strcpy(dest->flex, src->flex);
  //u_strcpy(dest->flechie, src->flechie);
  dest->canonique = u_strdup(src->canonique);
  u_strcpy(dest->gramm, src->gramm);
}


tSymbole * symbole_dup(tSymbole * src) {

//  debug("symbole_dup: (%d:%S,%S.%S)\n", src->sorteSymbole, src->flex->str, src->canonique, src->gramm);

  if (src == NULL) { return NULL; }

  //tSymbole * dest = (tSymbole *) xmalloc(sizeof(tSymbole));
  tSymbole * dest = tSymbole_new();

  dest->sorteSymbole = src->sorteSymbole;
  //u_strcpy(dest->flechie, src->flechie);
  u_strcpy(dest->flex, src->flex);
  u_strcpy(dest->gramm, src->gramm);
  dest->canonique = u_strdup(src->canonique);

  return dest;
}


tAlphMot * alphabet_new(int size) {
  tAlphMot * alphabet  = (tAlphMot *) xmalloc(sizeof(tAlphMot));
  alphabet->symb       = (tSymbole *) xmalloc(size * sizeof(tSymbole));
  alphabet->size    = size;
  alphabet->nbSymboles = 0;
  return alphabet;
}


void free_SymbolAlphabet(tAlphMot * alpha) {
  free(alpha->symb);
  free(alpha);
}

void alphabet_clear(tAlphMot * alpha) {
  alpha->nbSymboles = 0;
}




/* Libere le contenu de l'automate sans liberer l'automate. */
/* Precondition : l automate a au moins un etat. */

void videAutomate(tAutAlMot * Aut) {

  int i ;
  tTransitions * t, * s ;       /* Transition courante et suivante */

  for (i = 0 ; i < Aut -> nbEtats ; i ++) {
    for (t = Aut->etats[i]; t; t = s) { /* Destruction des transitions sortantes pour chaque etat */
      s = t->suivant;
      //libereEtiq(t);
      symbole_delete(t->etiq);
      free(t);
    }
  }

  free(Aut->etats);
  Aut->etats = 0;

  free(Aut->type);
  Aut->type = 0;

  free(Aut->initial);
  Aut->initial = 0;
 
  if (Aut->entrantesEtats) libereEntrantesEtats(Aut);

  Aut->nbEtats = 0;
}



/* PrÝcondition : a -> entrantesEtats != 0. */

void libereEntrantesEtats(tAutAlMot * a) {

  int i ;
  tTransitions * pt, * temp ;

  for (i = 0; i < a->nbEtats; i ++) {
    for (pt = a->entrantesEtats[i] ; pt ; pt = temp) {
      temp = pt->suivant ;
      free(pt) ;
      pt = 0 ;
    }   /* pt -> etiq est partage avec Aut -> etats[i] */
  }

  free(a->entrantesEtats) ;
  a->entrantesEtats = 0 ; /* free() ne le fait pas */
}   



void libereAutAlMot(tAutAlMot * Aut) {

  if (Aut->nbEtats) { videAutomate(Aut); }

  free(Aut->name);
  free(Aut);
}


void list_aut_old_delete(list_aut_old * lst) {

  for (int i = 0; i < lst->nb_aut; ++i) {
    libereAutAlMot(lst->les_aut[i]);
  }
  free(lst->les_aut);
  free(lst);
}





void output_fst2_labels(string_hash* hash,FILE* f) {
for (int i=0;i<hash->size;i++) {
   u_fprintf(f,"%%%S\n",hash->value[i]);
}
u_fprintf(f,"f\n");
}

