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
//---------------------------------------------------------------------------
#ifndef Grf2Fst2_libH
#define Grf2Fst2_libH
//---------------------------------------------------------------------------

#include "unicode.h"
#include "Alphabet.h"
#include "Fst2.h"
#include "Error.h"
#include "FileName.h"
#include "Limits.h"
#include "SingleGraph.h"

#define DEFAULT_TOKENIZATION 0
#define CHAR_BY_CHAR_TOKENIZATION 1
#define ALPHABET_TOKENIZATION 2

#define NOMBRE_GRAPHES_COMP 5000        // nombre max de graphes
#define N_CAR_MAX_COMP 300               // nombre max de caracteres d'un mot normal
#define TAILLE_MOT_GRAND_COMP 10000       // nombre max de caracteres d'un long mot
#define NOMBRE_TRANSITIONS_COMP 10000    // nombre max de transitions
#define L_ASCII 256 //taille de l'alphabet
#define TAILLE_SEQUENCE_COMP 200        // nombre de mots max dans une sequence de mots entre deux + d'une boite
#define NBRE_ET  20000  /* maximal number of states per automaton/subgraph */
#define NBRE_ETIQ_TRANSITION_COMP MAX_FST2_TAGS+NOMBRE_GRAPHES_COMP
/* NBRE_ETIQ_TRANSITION_COMP: stack size of transition table: 
   one part for etiquettes, one for subgraph transitions */

//
// structure d'un ensemble d'etats
//
struct char_etats_det {
  unsigned int num_char;     //  numero du char dont chaque bit est un etat
  unsigned int valeur;       //   valeur du char
  struct char_etats_det *suivant;
};
typedef struct char_etats_det* ensemble_det;


//
// structure d'un etat
//
struct etat_fst_det
{
  unsigned char  controle;        // etat final ou pas
  ensemble_det   ens;            //ensemble des etats de graphe regroupes dans cet etat
  Fst2Transition trans;     // transition_fsts partant de cet etat
};
typedef struct etat_fst_det* Etat_fst_det;


//
// structure d'un noeud num_char
//
struct noeud_num_char_det {
  int num_char;
  struct liste_branches_valeur_det *liste;
};


struct liste_branches_valeur_det {
  struct noeud_valeur_det *n;
  struct liste_branches_valeur_det *suivant;
};

struct noeud_valeur_det {
  int valeur;
  int indice;
  struct liste_branches_num_char_det *liste;
};


struct liste_branches_num_char_det {
  struct noeud_num_char_det *n;
  struct liste_branches_num_char_det *suivant;
};






//
// structures servant a manipuler l'arbre des graphes
//
struct noeud_g_comp
{
  int fin;
  unsigned char lettre;
  struct liste_feuilles_g_comp *l;
};

struct liste_feuilles_g_comp
{
  struct noeud_g_comp *node;
  struct liste_feuilles_g_comp *suivant;
};


//
// structures servant a manipuler l'arbre des etiquettes
//
struct noeud_comp
{
  int fin;
  unichar lettre;
  struct liste_feuilles_comp *l;
};

struct liste_feuilles_comp
{
  struct noeud_comp *node;
  struct liste_feuilles_comp *suivant;
};


struct donnees_comp
{
  unichar nom_graphe[NOMBRE_GRAPHES_COMP][N_CAR_MAX_COMP];
  signed char statut_graphe[NOMBRE_GRAPHES_COMP];     //0=vide 1=non_vide -1=non traite
  unichar Etiquette_comp[MAX_FST2_TAGS][N_CAR_MAX_COMP];
  unichar chemin_alphabet[TAILLE_MOT_GRAND_COMP];
  unichar chemin_graphe_principal[TAILLE_MOT_GRAND_COMP];
};




extern struct donnees_comp *donnees;
extern int nombre_graphes_comp;
extern int nombre_etiquettes_comp;
extern struct noeud_g_comp *rac_graphe_comp; //racine de l'arbre des graphes
extern struct noeud_comp *rac_comp; //racine de l'arbre des étiquettes
extern int EPSILON_comp;   // etiquette pour <E>
extern int compteur_char;
extern int compteur_free_char;
extern unichar pckg_path[TAILLE_MOT_GRAND_COMP];

void vider_noeud_num_char_det(struct noeud_num_char_det *);

void liberer_graphe_comp(SingleGraphState *etat);
struct noeud_valeur_det* nouveau_noeud_valeur_det();
void init_graphe_mat_det(Etat_fst_det resultat[]);
void init_resultat_det(Etat_fst_det resultat[],struct noeud_valeur_det *racine,int dernier_etat_res);
void init_stock_det(ensemble_det s[]);
void init_hachage_det(int*,int);
void liberer_etat_det(Etat_fst_det e);
void liberer_char_etat_det(ensemble_det ptr);
void ajouter_etat_dans_ensemble_det(int netat,ensemble_det *e);
int numero_ensemble_det(ensemble_det e,struct noeud_valeur_det *node,int dernier_etat_res);
void ajouter_transition_mat_det(struct etat_fst_det *e,int etiq,int etarr);
void liberer_arbre_det(struct noeud_valeur_det *racine);
Etat_fst_det nouvel_etat_mat_det();
ensemble_det copie_det(ensemble_det e);
void sauvegarder_etat_det(FILE *f,Etat_fst_det e);
int determinisation(SingleGraph);
int minimisation(SingleGraph);
int write_graph_comp(FILE*,SingleGraph,int,unichar*);
void *malloc_comp(int n);
void init_generale_comp();
void init_arbres_comp();
void libere_arbres_comp();
int compilation(char *nom_graphe_principal,int mode,Alphabet* alph,FILE* fs_comp);
void sauvegarder_etiquettes_comp(FILE* fs_comp);
void ecrire_fichier_sortie_nb_graphes(char name[],FILE* fs_comp);



#endif

