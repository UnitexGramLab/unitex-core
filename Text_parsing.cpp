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
#include "LocatePattern.h"
#include "Text_tokens.h"
#include "Text_parsing.h"
#include "Context.h"
#include "Error.h"
#include <time.h>

#define DELAY CLOCKS_PER_SEC // delay between two prints (yyy% done)
//---------------------------------------------------------------------------

/**
 * This structure is used for setting global values for the 'parcourir_opt'
 * function.
 */
struct locate_infos {
	/* The compound word tree */
	struct DLC_tree_info* DLC_tree;
};

void parcourir_opt(int,int,int,int,struct liste_num**,int,struct context*,struct locate_infos*);



int GESTION_DE_L_ESPACE=MODE_NON_MORPHO;
int texte[BUFFER_SIZE];
int LENGTH;
int N_INT_ALLREADY_READ;
int origine_courante;
long int nombre_unites_reconnues=0;
int* debut_graphe;
struct string_hash* TOKENS;
struct text_tokens* tokens;
Fst2Tag* ETIQUETTE;
int SENTENCE_DELIMITER_INDICE=-1;
int STOP_MARKER_INDICE=-1;
Fst2* current_fst2=NULL;


void block_change(FILE* f) {
int i;
for (i=origine_courante;i<BUFFER_SIZE;i++) {
  // first, we copy the end of the buffer at the beginning
  texte[i-origine_courante]=texte[i];
}
N_INT_ALLREADY_READ=N_INT_ALLREADY_READ+origine_courante;
int N=BUFFER_SIZE-origine_courante;
int l=fread(texte+N,sizeof(int),origine_courante,f);
origine_courante=0;
LENGTH=N+l;
if (LENGTH<BUFFER_SIZE) {
}
}



void launch_locate(FILE* f,Fst2* automate,int mode,struct string_hash* tok,FILE* out,
                   int output_mode,long int text_size,FILE* info,
                   struct DLC_tree_info* DLC_tree) {
LENGTH=fread(texte,sizeof(int),BUFFER_SIZE,f);
statut_match=mode;
transduction_mode=output_mode;
init_matches();
if (transduction_mode != IGNORE_TRANSDUCTIONS) // there may be transducer output
  ambig_transduction_mode = ALLOW_AMBIG_TRANSDUCTIONS; // so we allow different output
debut_graphe=automate->initial_states;
int debut=debut_graphe[1];
current_fst2=automate;
N_INT_ALLREADY_READ=0;
origine_courante=0;
nombre_unites_reconnues=0;
TOKENS=tok;
ETIQUETTE=automate->tags;
int n_read=0;
int unite;
clock_t startTime = clock();
clock_t currentTime ;
/*
 * Initialization of some global settings 
 */
struct locate_infos infos;
infos.DLC_tree=DLC_tree;


unite=((text_size/100)>1000)?(text_size/100):1000;
while (origine_courante<LENGTH && nombre_match!=SEARCH_LIMITATION) {
   if (LENGTH==BUFFER_SIZE && origine_courante>(LENGTH-2000)) {
      // if must change of block
      block_change(f);
   }
   if (unite!=0) {
      n_read=((origine_courante+N_INT_ALLREADY_READ)%unite);
      if (n_read==0 && ((currentTime=clock())-startTime > DELAY) ) {
         startTime=currentTime;
         printf("%2.0f%% done        \r",100.0*(float)(N_INT_ALLREADY_READ+origine_courante)/(float)text_size);
      }
   }
   if (!(texte[origine_courante]==ESPACE && GESTION_DE_L_ESPACE==MODE_NON_MORPHO)) {
      StackBase = StackPointer = 0;
      parcourir_opt(0,debut,0,0,NULL,0,NULL,&infos);
   }
   liste_match=ecrire_index_des_matches(liste_match,N_INT_ALLREADY_READ+origine_courante,&nombre_unites_reconnues,out);
   origine_courante++;
}
liste_match=ecrire_index_des_matches(liste_match,N_INT_ALLREADY_READ+origine_courante,&nombre_unites_reconnues,out);
printf("100%% done      \n\n");
printf("%d match%s\n",nombre_match,(nombre_match==1)?"":"es");
if ((nombre_output != nombre_match)
    && (nombre_output != 0))
  printf("(%d output%s)\n",nombre_output,(nombre_output==1)?"":"s");
printf("%ld recognized units\n",nombre_unites_reconnues);
if (text_size!=0) {
   printf("(%2.3f%% of the text is covered)\n",((float)nombre_unites_reconnues*100.0)/text_size);
}
if (info!=NULL) {
   char tmp[3000];
   unichar unitmp[3000];
   sprintf(tmp,"%d match%s\n",nombre_match,(nombre_match==1)?"":"es");
   u_strcpy_char(unitmp,tmp);
   if ((nombre_output != nombre_match)
       && (nombre_output != 0))
     {
       sprintf(tmp,"(%d output%s)\n",nombre_output,(nombre_output==1)?"":"s");
       u_strcat_char(unitmp,tmp);
     }
   sprintf(tmp,"%ld recognized units\n",nombre_unites_reconnues);
   u_strcat_char(unitmp,tmp);
   if (text_size!=0) {
      sprintf(tmp,"(%2.3f%% of the text is covered)\n",((float)nombre_unites_reconnues*100.0)/text_size);
   }
   u_strcat_char(unitmp,tmp);
   u_fprints(unitmp,info);
}
}


/**
 *  Prints the current context to stderr,
 *  unless it wasn't already printed.
 *  If there are more than MAX_ERRORS errors,
 *  exit the programm by calling "fatal_error".
 */
static void error_at_token_pos (char* message, int start, int length) {

  static int n_errors;
  static int last_start = -1;
  static int last_length;

  int i;

  if (last_start == start /* && last_length == length */) {
    return; // context already printed
  }

  fprintf(stderr, message);

  fprintf(stderr, "\n  ");
  for (i = (start-4); i <= (start+20); i++) {
    if (i < 0)
      continue;
    if (i == start)
      fprintf(stderr, "<<HERE>>");
    u_fprints_html_ascii(TOKENS->tab[texte[i]], stderr);
    if (i == (start+length))
      fprintf(stderr, "<<END>>");
  }
  if (i < (start+length))
    fprintf(stderr, " ...");
  fprintf(stderr, "\n");

  if (++n_errors >= MAX_ERRORS)
    fatal_error("Too many errors, giving up!\n");

  last_start  = start;
  last_length = length;

}



void parcourir_opt(int numero_graphe_courant,
                   int numero_etat_courant,
                   int pos,
                   int profondeur,
                   struct liste_num** LISTE,
                   int n_matches,
                   struct context* ctx,
                   struct locate_infos* infos) {
Etat_opt etat_courant;
int pos2,k,pos3,pos4,k_pas_decale;
struct liste_arrivees* arr;
struct appel_a_sous_graphe* a_sous_graphe;
struct appel_a_meta* a_meta;
struct appel_a_pattern* a_pattern;
int SOMMET=StackPointer;
unichar* sortie;

/* $CD$ begin */
int iMasterGF;
/* $CD$ end   */

static int n_matches_at_token_pos; // holds the number of matches at one position in text (static!)
if (profondeur == 0)               // reset if main graph is called first
  n_matches_at_token_pos = 0;

if ((profondeur++)>TAILLE_PILE) {
  error_at_token_pos("\n"
                     "Maximal stack size reached!\n"
                     "(There may be longer matches not recognized!)",
                     origine_courante, pos);
  return;
}

if (n_matches_at_token_pos > MAX_MATCHES_AT_TOKEN_POS) {
  error_at_token_pos("\n"
                     "To many (ambiguous) matches starting from one position in text!",
                     origine_courante, pos);
  return;
}


etat_courant=graphe_opt[numero_etat_courant];


// if we are looking for the end of a failed negative context
if (ctx!=NULL && ctx->contextMode==FAILED_IN_NEGATIVE_CONTEXT) {
   // then we explore all the output transitions without matching
   // anything, in order to find the context end mark
   struct liste_nombres* liste=NULL;
   struct liste_nombres* tmp;
   
   //******* subgraphs ************
   // we make the list of the states that can be reached via 
   // a subgraph call transition
   a_sous_graphe=etat_courant->liste_sous_graphes;
   while (a_sous_graphe != NULL) {
      arr=a_sous_graphe->liste_arr;
      while (arr!=NULL) {
         liste=inserer_dans_liste_nombres(arr->arr,liste);
         arr=arr->suivant;
      }
      a_sous_graphe=a_sous_graphe->suivant;
   }
   //******* metas ************
   a_meta=etat_courant->liste_metas;
   while (a_meta!=NULL) {
      arr=a_meta->liste_arr;
      while (arr!=NULL) {
            if (a_meta->numero_de_meta==CONTEXT_END_MARK && ctx->depth==0) {
               // We have found a closing context mark.
               // if we have not recognized the negative context,
               // then we can take the match into account
               //printf("end of fail context in state %d",numero_etat_courant); getchar();
               pos=ctx->continue_position;
               u_strcpy(stack,ctx->stack);
               StackPointer=ctx->stack_pointer;
               // if there is a matches list, 
               // we must ignore all matches informations that have been 
               // compute inside the context
               if (LISTE!=NULL) {
                  free_list_num(*LISTE);
               }
               LISTE=ctx->list_of_matches;
               n_matches=ctx->n_matches;
               parcourir_opt(numero_graphe_courant,arr->arr,pos,profondeur,LISTE,n_matches,
               				ctx->next,infos);
            }
            else {
               // we handle separately the cases of context marks, because
               // they involve a modification of ctx->depth
               if (a_meta->numero_de_meta==CONTEXT_END_MARK) {
                  (ctx->depth)--;
                  parcourir_opt(numero_graphe_courant,arr->arr,pos,profondeur,LISTE,n_matches,
                  				ctx,infos);
                  (ctx->depth)++;
               }
               else
               if (a_meta->numero_de_meta==POSITIVE_CONTEXT_MARK
                  || a_meta->numero_de_meta==NEGATIVE_CONTEXT_MARK) {
                  (ctx->depth)++;
                  parcourir_opt(numero_graphe_courant,arr->arr,pos,profondeur,LISTE,n_matches,
                  				ctx,infos);
                  (ctx->depth)--;
               }
               else {
                  // if we have another meta, we handle it normally
                  liste=inserer_dans_liste_nombres(arr->arr,liste);
               }
            }
            arr=arr->suivant;
      }
      a_meta=a_meta->suivant;
   }
   //******* compound patterns ************
   a_pattern=etat_courant->liste_patterns_composes;
   while (a_pattern!=NULL) {
      arr=a_pattern->liste_arr;
      while (arr!=NULL) {
            liste=inserer_dans_liste_nombres(arr->arr,liste);
            arr=arr->suivant;
      }
      a_pattern=a_pattern->suivant;
   }
   //******* simple patterns ************
   a_pattern=etat_courant->liste_patterns;
   while (a_pattern!=NULL) {
      arr=a_pattern->liste_arr;
      while (arr!=NULL) {
            liste=inserer_dans_liste_nombres(arr->arr,liste);
            arr=arr->suivant;
      }
      a_pattern=a_pattern->suivant;
   }
   //******* tokens ************
   for (int k=0;k<etat_courant->nombre_de_tokens;k++) {
      arr=etat_courant->tableau_liste_arr[k];
      while (arr!=NULL) {
            liste=inserer_dans_liste_nombres(arr->arr,liste);
            arr=arr->suivant;
      }
   }
   // finally, we explore all the states that can be reached from this state
   tmp=liste;
   while (liste!=NULL) {
      parcourir_opt(numero_graphe_courant,liste->n,pos,profondeur,LISTE,n_matches,ctx,infos);
      liste=liste->suivant;
   }
   free_liste_nombres(tmp);
   return;
}

// if we are in a final state...
if (etat_courant->controle & 1) {
  if (ctx!=NULL) {
     // if we have reached the final state of a graph while
     // looking for a context, it's an error because every
     // opened context must be closed before the end of the graph
     char tmp[1024];
     u_to_char(tmp,current_fst2->graph_names[numero_graphe_courant+1]);
     fprintf(stderr,"ERROR: unclosed context in graph \"%s\"\n",tmp);
     free_context_list(ctx);
     return;
  }

  if (numero_graphe_courant == 0) { // in main graph : hooray, got a match!
    n_matches_at_token_pos++;
    if (transduction_mode == IGNORE_TRANSDUCTIONS) {
      if (pos>0) {
        afficher_match_fst2(pos+origine_courante+N_INT_ALLREADY_READ-1,NULL);
      }
      else {
        afficher_match_fst2(pos+origine_courante+N_INT_ALLREADY_READ,NULL);
      }
    } else {
      stack[SOMMET]='\0';
      if (pos>0) {
        afficher_match_fst2(pos+origine_courante+N_INT_ALLREADY_READ-1,stack);
      }
      else {
        afficher_match_fst2(pos+origine_courante+N_INT_ALLREADY_READ,stack);
      }
    }
  } else { // in a subgraph
    if (/**last*/n_matches==(NBRE_ARR_MAX-1)) {
      // presumably an infinite recursion
      error_at_token_pos("\n"
                         "Maximal number of matches per subgraph reached!",
                         // print also name of subgraph, but how?
                         origine_courante, pos);
      return;
    }
    else {
      n_matches++;
      if (ambig_transduction_mode == ALLOW_AMBIG_TRANSDUCTIONS) {
        (*LISTE)=inserer_si_different(pos,(*LISTE),StackPointer,&stack[StackBase]);
      }
      else {
        (*LISTE)=inserer_si_absent(pos,(*LISTE),StackPointer,&stack[StackBase]);
      }
    }
  }
}

if (pos+origine_courante>LENGTH) return;

// sous-graphes
if ((a_sous_graphe=etat_courant->liste_sous_graphes) != NULL) {
  // if there are subgraphs, we must process them

  int* var_backup=NULL;
  int old_StackBase;
  old_StackBase = StackBase;
  if (transduction_mode != IGNORE_TRANSDUCTIONS) {
    // for better performance when ignoring outputs
    var_backup=create_variable_backup();
  }

  do
    { // process list of subgraphs
      arr=a_sous_graphe->liste_arr;
      while (arr!=NULL) {
        struct liste_num* L = NULL;
        StackBase = StackPointer;
        parcourir_opt(a_sous_graphe->numero_de_graphe,
                      debut_graphe[a_sous_graphe->numero_de_graphe],
                      pos,profondeur,&L,0,NULL,infos);
        StackBase = old_StackBase;
        if (L != NULL) { // at least one match by the called subgraph
          do
            { // process all matches in L
              if (transduction_mode != IGNORE_TRANSDUCTIONS) {
                u_strcpy(&stack[SOMMET],L->pile);
                StackPointer = L->sommet;
                install_variable_backup(L->variable_backup);
              }
              parcourir_opt(numero_graphe_courant,arr->arr,L->n,profondeur,LISTE,n_matches,
              				ctx,infos);
              StackPointer=SOMMET;
              if (numero_graphe_courant == 0) // necessary only in main graph
                install_variable_backup(var_backup);
              struct liste_num* l_tmp = L;
              L = L->suivant;
              free_variable_backup(l_tmp->variable_backup);
              free(l_tmp);
            }
          while (L != NULL);
        }
        arr=arr->suivant;
      } // end of while (arr!=NULL)
    }
  while ((a_sous_graphe=a_sous_graphe->suivant) != NULL);

  // finally we have to restore the stack and other backup stuff
  StackPointer=SOMMET;
  StackBase=old_StackBase; // may be changed by recursive subgraph calls !
  if (transduction_mode != IGNORE_TRANSDUCTIONS) { // for better performance (see above)
    install_variable_backup(var_backup);
    free_variable_backup(var_backup);
  }
} // end of processing subgraphs


/////////////////////////////////////////////////
// gestion de l'espace
if (texte[pos+origine_courante]==ESPACE) {
   pos2=pos+1;
}
else pos2=pos;
if (pos2+origine_courante>LENGTH) return;
pos4=pos2+1;



/////////////////////////////////////////////////
// metas
a_meta=etat_courant->liste_metas;
if (a_meta!=NULL) {
  k=index_controle[texte[pos2+origine_courante]];
  k_pas_decale=index_controle[texte[pos+origine_courante]];
}
while (a_meta!=NULL) {
  arr=a_meta->liste_arr;
  while (arr!=NULL) {

    /* $CD$ begin */
    iMasterGF = ETIQUETTE[arr->etiquette_origine]->entryMasterGF;
    /* $CD$ end   */

    sortie=ETIQUETTE[arr->etiquette_origine]->output;
    switch (a_meta->numero_de_meta) {
      case SPACE_TAG:if (texte[pos+origine_courante]==ESPACE) {
                        if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                        if (transduction_mode==MERGE_TRANSDUCTIONS) push_char(' ');
                        parcourir_opt(numero_graphe_courant,arr->arr,pos+1,profondeur,LISTE,
                        				n_matches,ctx,infos);
                        StackPointer=SOMMET;
                      }
                      break;
      case DIESE: if (texte[pos+origine_courante]!=ESPACE) {
                     if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                     parcourir_opt(numero_graphe_courant,arr->arr,pos,profondeur,LISTE,
                     				n_matches,ctx,infos);
                     StackPointer=SOMMET;
                  }
                  break;
      case EPSILON: if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                    parcourir_opt(numero_graphe_courant,arr->arr,pos,profondeur,LISTE,
                    			n_matches,ctx,infos);
                    StackPointer=SOMMET;
                    break;
      case POSITIVE_CONTEXT_MARK: {
                    // We look for a positive context from the current position.
                    stack[StackPointer]='\0';
                    struct context* c=new_context(INSIDE_POSITIVE_CONTEXT,
                                                  pos,
                                                  stack,
                                                  StackPointer,
                                                  LISTE,
                                                  n_matches,
                                                  ctx);
                    parcourir_opt(numero_graphe_courant,arr->arr,pos,profondeur,NULL,0,c,infos);
                    remove_context(c);
                    break;
                    }
      case NEGATIVE_CONTEXT_MARK: {
                    // We look for a negative context from the current position.
                    stack[StackPointer]='\0';
                    struct context* c=new_context(INSIDE_NEGATIVE_CONTEXT,
                                                  pos,
                                                  stack,
                                                  StackPointer,
                                                  LISTE,
                                                  n_matches,
                                                  ctx);
                    parcourir_opt(numero_graphe_courant,arr->arr,pos,profondeur,NULL,0,c,infos);
                    if (c->contextMode!=NEGATIVE_CONTEXT_HAS_MATCHED) {
                       // if we have not matched anything in the negative 
                       // context, then we can bypass anything looking for 
                       // the context end mark
                       c->contextMode=FAILED_IN_NEGATIVE_CONTEXT;
                       c->depth=0;
                       parcourir_opt(numero_graphe_courant,arr->arr,pos,profondeur,NULL,0,c,infos);
                    }
                    remove_context(c);
                    break;
                    }
      case CONTEXT_END_MARK: {
                    // We have found a closing context mark.
                    if (ctx==NULL) {
                       // if there was no current opened context
                       char tmp[1024];
                       u_to_char(tmp,current_fst2->graph_names[numero_graphe_courant+1]);
                       fprintf(stderr,"ERROR: unexpected closing context mark in graph \"%s\"\n",tmp);
                       return;
                    }
                    //***************************************************************
                    if (ctx->contextMode==INSIDE_POSITIVE_CONTEXT) {
                       // if we were inside a positive context
                       pos=ctx->continue_position;
                       u_strcpy(stack,ctx->stack);
                       StackPointer=ctx->stack_pointer;
                       // if there is a matches list, 
                       // we must ignore all matches informations that have been 
                       // compute inside the context
                       if (LISTE!=NULL) {
                          free_list_num(*LISTE);
                       }
                       LISTE=ctx->list_of_matches;
                       n_matches=ctx->n_matches;
                       parcourir_opt(numero_graphe_courant,arr->arr,pos,profondeur,LISTE,
                       				n_matches,ctx->next,infos);
                       break;
                    }
                    //***************************************************************
                    if (ctx->contextMode==INSIDE_NEGATIVE_CONTEXT) {
                       // if we have recognized a negative context
                       // we must ignore the match
                       ctx->contextMode=NEGATIVE_CONTEXT_HAS_MATCHED;
                       return;
                    }
                    //***************************************************************
                    if (ctx->contextMode==FAILED_IN_NEGATIVE_CONTEXT) {
                       fatal_error("ERROR: unexpected FAILED_IN_NEGATIVE_CONTEXT\n");
                    }
                    break;
                    }
      case VAR_START: {
                      int old=get_variable_start(a_meta->numero_de_variable);
                      set_variable_start(a_meta->numero_de_variable,pos2);
                      /*if (transduction_mode==MERGE_TRANSDUCTIONS) {
                         if (pos2!=pos) push_char(' ');
                         
                         // modification made by Sébastien Paumier
                      }*/
                      parcourir_opt(numero_graphe_courant,arr->arr,pos/*pos2*/,profondeur,LISTE,
                      				n_matches,ctx,infos);
                      StackPointer=SOMMET;
                      set_variable_start(a_meta->numero_de_variable,old);
                      }
                      break;
      case VAR_END:   {
                      int old=get_variable_end(a_meta->numero_de_variable);
                      set_variable_end(a_meta->numero_de_variable,pos-1);
                      /*if (transduction_mode==MERGE_TRANSDUCTIONS) {
                         if (pos2!=pos) push_char(' ');
                         
                         // modification made by Sébastien Paumier
                      }*/
                      parcourir_opt(numero_graphe_courant,arr->arr,pos/*pos2*/,profondeur,LISTE,
                      				n_matches,ctx,infos);
                      StackPointer=SOMMET;
                      set_variable_end(a_meta->numero_de_variable,old);
                      }
                      break;
      case NB: {     int z=pos2;
                     while (is_a_digit_token(TOKENS->tab[texte[z+origine_courante]])) z++;
                     if (z!=pos2) {
                        if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                        if (transduction_mode==MERGE_TRANSDUCTIONS) {
                           if (pos2!=pos) push_char(' ');
                           for (int y=pos2;y<z;y++) {
                              push_string(TOKENS->tab[texte[y+origine_courante]]);
                           }
                        }
                        parcourir_opt(numero_graphe_courant,arr->arr,z,profondeur,LISTE,n_matches,ctx,infos);
                        StackPointer=SOMMET;
                     }
               }
               break;
               
               
      /* $CD$ begin */
      case TOKEN:   if (texte[pos2+origine_courante]==STOP_MARKER_INDICE) {
                       // the {STOP} tag must NEVER be matched by any pattern
                       break;
                    }
#ifdef TRE_WCHAR 
                    if (!(iMasterGF == -1 || OptMatchGF(indexGF, texte[pos2+origine_courante], iMasterGF) == 0)) break;
#endif 
                    if (transduction_mode != IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                    if (transduction_mode == MERGE_TRANSDUCTIONS) {
                        if (pos2 != pos) push_char(' ');
                        push_string(TOKENS->tab[texte[pos2+origine_courante]]);
                        }
                    parcourir_opt(numero_graphe_courant,arr->arr, pos4,profondeur,LISTE,
                    			n_matches,ctx,infos);
                    StackPointer = SOMMET;
                    break;
      /* $CD$ end   */
      
      
      case MOT: 
#ifdef TRE_WCHAR
                if (!(iMasterGF == -1 || OptMatchGF(indexGF, texte[pos2+origine_courante], iMasterGF) == 0)) break;
#endif
                if ((GESTION_DE_L_ESPACE==MODE_MORPHO)&&(pos2!=pos)&&(!(k_pas_decale&MOT_TOKEN_BIT_MASK))
                    &&(a_meta->negation)) {
                   // on est dans le cas ou on veut attraper l'espace avec <!MOT>
                   if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                   if (transduction_mode==MERGE_TRANSDUCTIONS) {
                      if (pos2!=pos) push_char(' ');
                   }
                   parcourir_opt(numero_graphe_courant,arr->arr,pos+1,profondeur,LISTE,
                   				n_matches,ctx,infos);
                   StackPointer=SOMMET;
                }
                else if (((k&MOT_TOKEN_BIT_MASK)&&!(a_meta->negation))
                    ||((!(k&MOT_TOKEN_BIT_MASK))&&(a_meta->negation)&&(texte[pos2+origine_courante]!=SENTENCE_DELIMITER_INDICE)
                    
                             // the {STOP} tag must NEVER be matched by any pattern         
                             &&(texte[pos2+origine_courante]!=STOP_MARKER_INDICE))) {
                   if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                   if (transduction_mode==MERGE_TRANSDUCTIONS) {
                      if (pos2!=pos) push_char(' ');
                      push_string(TOKENS->tab[texte[pos2+origine_courante]]);
                   }
                   parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
                   				n_matches,ctx,infos);
                   StackPointer=SOMMET;
                }
                break;
      case DIC: if (!(a_meta->negation)) {
                  pos3=trouver_mot_compose_DIC(pos2,UNDEFINED_COMPOUND_PATTERN,infos->DLC_tree);
                  if (pos3!=-1) {
                     int OK=1;
#ifdef TRE_WCHAR
                     if (iMasterGF == -1 ) OK=1;
                     else {
                        unichar tmp[512];
                        tmp[0]='\0';
                        for (int x=pos2;x<=pos3;x++) {
                           u_strcat(tmp,TOKENS->tab[texte[x+origine_courante]]);
                        }
                        OK=(MatchRawGF(masterGF, tmp,iMasterGF)==0);
                     }
#endif
                     if (OK) {
                        if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                        if (transduction_mode==MERGE_TRANSDUCTIONS) {
                           if (pos2!=pos) push_char(' ');
                           for (int x=pos2;x<=pos3;x++)
                              push_string(TOKENS->tab[texte[x+origine_courante]]);
                        }
                        parcourir_opt(numero_graphe_courant,arr->arr,pos3+1,profondeur,LISTE,
                        				n_matches,ctx,infos);
                        StackPointer=SOMMET;
                     }
                  }
                  if (k&DIC_TOKEN_BIT_MASK) {
#ifdef TRE_WCHAR
                     if (!(iMasterGF == -1 || OptMatchGF(indexGF, texte[pos2+origine_courante], iMasterGF) == 0)) break;
#endif
                     if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                     if (transduction_mode==MERGE_TRANSDUCTIONS) {
                        if (pos2!=pos) push_char(' ');
                        push_string(TOKENS->tab[texte[pos2+origine_courante]]);
                     }
                     parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
                     				n_matches,ctx,infos);
                     StackPointer=SOMMET;
                     break;
                  }
      			}
                else { // we are in the case <!DIC>
                       if (k&NOT_DIC_TOKEN_BIT_MASK) {
#ifdef TRE_WCHAR
                          if (!(iMasterGF == -1 || OptMatchGF(indexGF, texte[pos2+origine_courante], iMasterGF) == 0)) break;
#endif
                          if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                          if (transduction_mode==MERGE_TRANSDUCTIONS) {
                             if (pos2!=pos) push_char(' ');
                             push_string(TOKENS->tab[texte[pos2+origine_courante]]);
                          }
                          parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
                          				n_matches,ctx,infos);
                          StackPointer=SOMMET;
                       }
                       break;
                } break;
      case SDIC: 
#ifdef TRE_WCHAR
                 if (!(iMasterGF == -1 || OptMatchGF(indexGF, texte[pos2+origine_courante], iMasterGF) == 0)) break;
#endif
                 if ((k&DIC_TOKEN_BIT_MASK) && !(k&CDIC_TOKEN_BIT_MASK)) {
                     if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                     if (transduction_mode==MERGE_TRANSDUCTIONS) {
                        if (pos2!=pos) push_char(' ');
                        push_string(TOKENS->tab[texte[pos2+origine_courante]]);
                     }
                     parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
                     			n_matches,ctx,infos);
                     StackPointer=SOMMET;
                  }
                  break;
      case CDIC: pos3=trouver_mot_compose_DIC(pos2,UNDEFINED_COMPOUND_PATTERN,infos->DLC_tree);
                  if (pos3!=-1) {
                     int OK=1;
#ifdef TRE_WCHAR
                     if (iMasterGF == -1 ) OK=1;
                     else {
                        unichar tmp[512];
                        tmp[0]='\0';
                        for (int x=pos2;x<=pos3;x++) {
                           u_strcat(tmp,TOKENS->tab[texte[x+origine_courante]]);
                        }
                        OK=(MatchRawGF(masterGF,tmp,iMasterGF)==0);
                     }
#endif
                     if (OK) {
                        if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                        if (transduction_mode==MERGE_TRANSDUCTIONS) {
                           if (pos2!=pos) push_char(' ');
                           for (int x=pos2;x<=pos3;x++)
                              push_string(TOKENS->tab[texte[x+origine_courante]]);
                        }
                        parcourir_opt(numero_graphe_courant,arr->arr,pos3+1,profondeur,LISTE,
                        				n_matches,ctx,infos);
                        StackPointer=SOMMET;
                     }
                  }
                  // case {aujourd'hui,.ADV}
#ifdef TRE_WCHAR
                  if (!(iMasterGF == -1 || OptMatchGF(indexGF, texte[pos2+origine_courante], iMasterGF) == 0)) break;
#endif
                  if (k&CDIC_TOKEN_BIT_MASK) {
                     if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                     if (transduction_mode==MERGE_TRANSDUCTIONS) {
                        if (pos2!=pos) push_char(' ');
                        push_string(TOKENS->tab[texte[pos2+origine_courante]]);
                     }
                     parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
                     			n_matches,ctx,infos);
                     StackPointer=SOMMET;
                     break;
                  }
                 break;
      case MAJ: 
#ifdef TRE_WCHAR
                if (!(iMasterGF == -1 || OptMatchGF(indexGF, texte[pos2+origine_courante], iMasterGF) == 0)) break;
#endif
                if (!(a_meta->negation)) {
                  if (k&MAJ_TOKEN_BIT_MASK) {
                     if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                     if (transduction_mode==MERGE_TRANSDUCTIONS) {
                        if (pos2!=pos) push_char(' ');
                        push_string(TOKENS->tab[texte[pos2+origine_courante]]);
                     }
                     parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
                     				n_matches,ctx,infos);
                     StackPointer=SOMMET;
                     break;
                  }
      			}
                else {if ((!(k&MAJ_TOKEN_BIT_MASK))&&(k&MOT_TOKEN_BIT_MASK)) {
                         if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                         if (transduction_mode==MERGE_TRANSDUCTIONS) {
                             if (pos2!=pos) push_char(' ');
                             push_string(TOKENS->tab[texte[pos2+origine_courante]]);
                         }
                         parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
                         				n_matches,ctx,infos);
                         StackPointer=SOMMET;
                      }
                      break;
                } break;
      case MIN: 
#ifdef TRE_WCHAR
                if (!(iMasterGF == -1 || OptMatchGF(indexGF, texte[pos2+origine_courante], iMasterGF) == 0)) break;
#endif
                if (!(a_meta->negation)) {
                  if (k&MIN_TOKEN_BIT_MASK) {
                     if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                     if (transduction_mode==MERGE_TRANSDUCTIONS) {
                        if (pos2!=pos) push_char(' ');
                        push_string(TOKENS->tab[texte[pos2+origine_courante]]);
                     }
                     parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
                     				n_matches,ctx,infos);
                     StackPointer=SOMMET;
                     break;
                  }
      			}
                else {if ((!(k&MIN_TOKEN_BIT_MASK))&&(k&MOT_TOKEN_BIT_MASK)) {
                         if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                         if (transduction_mode==MERGE_TRANSDUCTIONS) {
                             if (pos2!=pos) push_char(' ');
                             push_string(TOKENS->tab[texte[pos2+origine_courante]]);
                         }
                         parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
                         				n_matches,ctx,infos);
                         StackPointer=SOMMET;
                      }
                      break;
                } break;
      case PRE: 
#ifdef TRE_WCHAR 
                if (!(iMasterGF == -1 || OptMatchGF(indexGF, texte[pos2+origine_courante], iMasterGF) == 0)) break;
#endif
                if (!(a_meta->negation)) {
                  if (k&PRE_TOKEN_BIT_MASK) {
                     if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                     if (transduction_mode==MERGE_TRANSDUCTIONS) {
                        if (pos2!=pos) push_char(' ');
                        push_string(TOKENS->tab[texte[pos2+origine_courante]]);
                     }
                     parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
                     			n_matches,ctx,infos);
                     StackPointer=SOMMET;
                     break;
                  }
      			}
                else {if ((!(k&PRE_TOKEN_BIT_MASK))&&(k&MOT_TOKEN_BIT_MASK)) {
                         if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
                         if (transduction_mode==MERGE_TRANSDUCTIONS) {
                             if (pos2!=pos) push_char(' ');
                             push_string(TOKENS->tab[texte[pos2+origine_courante]]);
                         }
                         parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
                         			n_matches,ctx,infos);
                         StackPointer=SOMMET;
                      }
                      break;
                } break;
    }
    arr=arr->suivant;
  }
  a_meta=a_meta->suivant;
}


/////////////////////////////////////////////////////////
// patterns composes
//
a_pattern=etat_courant->liste_patterns_composes;
while (a_pattern!=NULL) {
  arr=a_pattern->liste_arr;
  while (arr!=NULL) {

    /* $CD$ begin */
#ifdef TRE_WCHAR
    iMasterGF = ETIQUETTE[arr->etiquette_origine]->entryMasterGF;  
#endif
    /* $CD$ end   */

    sortie=ETIQUETTE[arr->etiquette_origine]->output;
    k=a_pattern->numero_de_pattern;
    pos3=trouver_mot_compose_DIC(pos2,k,infos->DLC_tree);
    if (pos3!=-1 && !(a_pattern->negation)) {
       int OK=1;
#ifdef TRE_WCHAR
       if (iMasterGF == -1 ) OK=1;
       else {
          unichar tmp[512];
          tmp[0]='\0';
          for (int x=pos2;x<=pos3;x++) {
            u_strcat(tmp,TOKENS->tab[texte[x+origine_courante]]);
          }
          OK=(MatchRawGF(masterGF,tmp,iMasterGF)==0);
       }
#endif
       if (OK){
          if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
          if (transduction_mode==MERGE_TRANSDUCTIONS) {
             if (pos2!=pos) push_char(' ');
             for (int x=pos2;x<=pos3;x++)
                push_string(TOKENS->tab[texte[x+origine_courante]]);
          }
          parcourir_opt(numero_graphe_courant,arr->arr,pos3+1,profondeur,LISTE,
          				n_matches,ctx,infos);
          StackPointer=SOMMET;
       }
    }
    arr=arr->suivant;
  }
  a_pattern=a_pattern->suivant;
}


///////////////////////////////////////////////
// patterns simples
//
a_pattern=etat_courant->liste_patterns;
while (a_pattern!=NULL) {
  arr=a_pattern->liste_arr;
  while (arr!=NULL) {

    /* $CD$ begin */
#ifdef TRE_WCHAR
    iMasterGF = ETIQUETTE[arr->etiquette_origine]->entryMasterGF;  
#endif
    /* $CD$ end   */
    sortie=ETIQUETTE[arr->etiquette_origine]->output;
    k=a_pattern->numero_de_pattern;
    //---mots composes
    pos3=trouver_mot_compose_DIC(pos2,k,infos->DLC_tree);
    if (pos3!=-1 && !(a_pattern->negation)) {
      int OK=1;
#ifdef TRE_WCHAR
      if (iMasterGF == -1 ) OK=1;
      else {
        unichar tmp[512];
        tmp[0]='\0';
        for (int x=pos2;x<=pos3;x++) {
          u_strcat(tmp,TOKENS->tab[texte[x+origine_courante]]);
        }
        OK=(MatchRawGF(masterGF,tmp,iMasterGF)==0);
    }
#endif
      if (OK) {
        if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
        if (transduction_mode==MERGE_TRANSDUCTIONS) {
          if (pos2!=pos) push_char(' ');
          for (int x=pos2;x<=pos3;x++)
            push_string(TOKENS->tab[texte[x+origine_courante]]);
        }
        parcourir_opt(numero_graphe_courant,arr->arr,pos3+1,profondeur,LISTE,
        			n_matches,ctx,infos);
        StackPointer=SOMMET;
      }
    }
    //---mots simples

#ifdef TRE_WCHAR
    if (iMasterGF == -1 || OptMatchGF(indexGF, texte[pos2+origine_courante], iMasterGF) == 0) 
#endif
    {
      if (index_code_gramm[texte[pos2+origine_courante]]!=NULL) {
        if ((index_code_gramm[texte[pos2+origine_courante]][k/8]&(1<<(k%8)) && !a_pattern->negation)
            || (!(index_code_gramm[texte[pos2+origine_courante]][k/8]&(1<<(k%8))) && a_pattern->negation)) {
          if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
          if (transduction_mode==MERGE_TRANSDUCTIONS) {
            if (pos2!=pos) push_char(' ');
            push_string(TOKENS->tab[texte[pos2+origine_courante]]);
          }
          parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
          				n_matches,ctx,infos);
          StackPointer=SOMMET;
        }
      } else {
        // if there is no code, we can try to look for a negation
        if (a_pattern->negation && (index_controle[texte[pos2+origine_courante]]&MOT_TOKEN_BIT_MASK)) {
          if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
          if (transduction_mode==MERGE_TRANSDUCTIONS) {
            if (pos2!=pos) push_char(' ');
            push_string(TOKENS->tab[texte[pos2+origine_courante]]);
          }
          parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
          				n_matches,ctx,infos);
          StackPointer=SOMMET;
        }
      }
    }
    arr=arr->suivant;
  }
  a_pattern=a_pattern->suivant;
}

// tokens
if (etat_courant->nombre_de_tokens!=0) {
  k=dichotomie(texte[pos2+origine_courante],etat_courant->tableau_de_tokens,etat_courant->nombre_de_tokens);
  if (k!=-1) {
    arr=etat_courant->tableau_liste_arr[k];
    while (arr!=NULL) {

      /* $CD$ begin */
#ifdef TRE_WCHAR
      iMasterGF = ETIQUETTE[arr->etiquette_origine]->entryMasterGF;  
      if (iMasterGF == -1 || OptMatchGF(indexGF, texte[pos2+origine_courante], iMasterGF) == 0) 
#endif
      {
      /* $CD$ end   */

      sortie=ETIQUETTE[arr->etiquette_origine]->output;
      if (transduction_mode!=IGNORE_TRANSDUCTIONS) process_transduction(sortie);
      if (transduction_mode==MERGE_TRANSDUCTIONS) {
         if (pos2!=pos) push_char(' ');
         push_string(TOKENS->tab[texte[pos2+origine_courante]]);
      }
      parcourir_opt(numero_graphe_courant,arr->arr,pos4,profondeur,LISTE,
      				n_matches,ctx,infos);
      StackPointer=SOMMET;
    
      /* $CD$ begin */
      } // MatchGF
      /* $CD$ end   */
    
      arr=arr->suivant;
    }
  }
}
}



int appartient_deja_aux_matches(int t[],int max,int n) {
int i;
for (i=0;i<max;i++)
  if (t[i]==n) return 1;
return 0;
}




int dichotomie(int a,int* t,int n) {
int debut,milieu;
if (n==0||t==NULL) return -1;
if (a<t[0]) return -1;
if (a>t[n-1]) return -1;
n=n-1;
debut=0;
while (debut<=n) {
  milieu=(debut+n)/2;
  if (t[milieu]==a) return milieu;
  if (t[milieu]<a) {
    debut=milieu+1;
  }
  else {
    n=milieu-1;
  }
}
return -1;
}




int trouver_mot_compose(int pos,struct DLC_tree_node* n,int code) {
int position_max,p,res;
if (n==NULL)
  return -1;
if (-1!=dichotomie(code,n->array_of_patterns,n->number_of_patterns))
  position_max=pos-1;
else position_max=-1;
if (pos+origine_courante==LENGTH)
  return position_max;
res=dichotomie(texte[pos+origine_courante],n->destination_tokens,n->number_of_transitions);
if (res==-1)
  return position_max;
p=trouver_mot_compose(pos+1,n->destination_nodes[res],code);
if (p>position_max)
  return p;
else return position_max;
}



int trouver_mot_compose_DIC(int pos,int code,struct DLC_tree_info* DLC_tree) {
int position_max,p,res;
struct DLC_tree_node *n;

if (pos+origine_courante==LENGTH) {
   return -1;
}
if ((n=DLC_tree->index[texte[pos+origine_courante]])==NULL) {
   return -1;
}
if (-1!=dichotomie(code,n->array_of_patterns,n->number_of_patterns)) {
   position_max=pos;
}
else position_max=-1;
pos++;
if (pos+origine_courante==LENGTH) {
   return -1;
}
res=dichotomie(texte[pos+origine_courante],n->destination_tokens,n->number_of_transitions);
if (res==-1) {
   return position_max;
}
p=trouver_mot_compose(pos+1,n->destination_nodes[res],code);
if (p>position_max) {
   return p;
}
else return position_max;
}

