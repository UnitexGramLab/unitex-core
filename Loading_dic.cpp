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
#include "Loading_dic.h"
//---------------------------------------------------------------------------


struct noeud *racine=nouveau_noeud();

int est_un_mot_simple(unichar* m,Alphabet* alph) {
int i;
i=0;
if (CHAR_BY_CHAR==THAI && u_strlen(m)>1) return 0;

while (m[i]!='\0' /*&& m[i]!='<' && m[i]!='>' && m[i]!='_'*/ && is_letter(m[i],alph)) {
   i++;
}
if (m[i]=='\0') {
   return 1;
}
return 0;
}



void load_dic_for_locate(char* dico,Alphabet* alph,struct string_hash* tok,
                         int n_octet_code_gramm,int existe_etiquette_DIC,
                         int existe_etiquette_CDIC,int existe_etiquette_SDIC,
                         struct DLC_tree_info* DLC_tree) {
FILE* f;
unichar flechi[TAILLE_MOT];
unichar canonique[TAILLE_MOT];
unichar code_gramm[TAILLE_MOT];
unsigned char* code_gramm_temp;
unichar s[4000];
int i,j,non_nul,z;
struct liste_nombres* ptr;

f=u_fopen(dico,U_READ);
if (f==NULL) {
   fprintf(stderr,"Cannot open dictionary %s\n",dico);
   return;
}
while (read_DELA_line(f,s)) {
  tokenize_DELA_line_into_3_parts(s,flechi,canonique,code_gramm);
  ajouter_forme_flechie(canonique,0,racine,flechi);
  ptr=get_token_list_for_sequence(flechi,alph,tok);
  struct liste_nombres* ptr_copy = ptr; // s.n.
  while (ptr!=NULL) {
      i=ptr->n;
      index_controle[i]=(unsigned char)(get_controle(tok->tab[i],alph,NULL)|DIC_TOKEN_BIT_MASK);
      if (code_gramm[0]!='\0') {
         if (index_code_gramm[i]==NULL) {
            index_code_gramm[i]=nouveau_code_pattern(n_octet_code_gramm);
         }
         get_numeros_pattern(code_gramm,index_code_gramm[i],canonique,n_octet_code_gramm);
         if (index_code_gramm[i]!=NULL) {
            j=0;
            non_nul=0;
            while (j<n_octet_code_gramm) {
              if (index_code_gramm[i][j]!=0) {
                non_nul=1;
                break;
              }
              j++;
            }
            if (!non_nul) {
               free(index_code_gramm[i]);
               index_code_gramm[i]=NULL;
            }
         }
      }
      ptr=ptr->suivant;
  }
  free_liste_nombres(ptr_copy); // s.n.
  if (!est_un_mot_simple(flechi,alph)) {
    // on est dans le cas d'un mot composé
    if (existe_etiquette_DIC || existe_etiquette_CDIC) {
       // si on a <DIC> dans le graphe, on charge betement toutes
       // les formes flechies composes
       ajouter_a_dlc_sans_code(flechi,alph,tok,DLC_tree);
    }
    if (code_gramm[0]!='\0') {
      code_gramm_temp=nouveau_code_pattern(n_octet_code_gramm);
      get_numeros_pattern(code_gramm,code_gramm_temp,canonique,n_octet_code_gramm);
      if (code_gramm_temp!=NULL) {
        j=0;
        non_nul=0;
        while (j<n_octet_code_gramm) {
          if (code_gramm_temp[j]!=0) {
            non_nul=1;
            break;
          }
          j++;
        }
        if (!non_nul) {
           free(code_gramm_temp);
        }
        else {
          // si le mot verifie au moins un pattern, on le met dans
          // l'arbre dlc
          j=0;
          while (j<n_octet_code_gramm) {
            for (z=0;z<8;z++)
              if (code_gramm_temp[j]&(1<<z)) {
                ajouter_a_dlc_avec_code(flechi,j*8+z,alph,tok,DLC_tree);
              }
            j++;
          }
          free(code_gramm_temp);
        }
      }
    }
  }
}
u_fclose(f);
}



struct noeud* nouveau_noeud() {
struct noeud* n;
n=(struct noeud*)malloc(sizeof(struct noeud));
n->controle=0;
n->numero=-1;
n->formes_flechies=NULL;
n->lettre=1;
n->l=NULL;
return n;
}



struct noeud* get_sous_noeud(struct noeud *n,unichar c,int creer) {
struct liste_feuilles* ptr;
struct liste_feuilles* ptr2;
struct noeud* res;
ptr=n->l;
while ((ptr!=NULL)&& (ptr->node!=NULL) && ((ptr->node)->lettre!=c)) {
  ptr=ptr->suivant;
}
if (ptr==NULL) {        // si on veut juste savoir si le noeud existe
  if (!creer) {          // et que le noeud n'existe pas, on renvoie NULL
     return NULL;
  }
  res=nouveau_noeud();
  res->lettre=c;
  ptr2=(struct liste_feuilles*)malloc(sizeof(struct liste_feuilles));
  ptr2->node=res;
  ptr2->suivant=NULL;
  if (n->l==NULL)
    n->l=ptr2;
  else {
         ptr=n->l;
         while (ptr->suivant!=NULL)
           ptr=ptr->suivant;
         ptr->suivant=ptr2;
       }
  return res;
}
return ptr->node;
}



void ajouter_flechi(struct noeud* n,unichar* s) {
int i,j,l,l2,debut;
unichar tmp[20000];
if (n->formes_flechies==NULL) {
  n->formes_flechies=(unichar*)malloc((u_strlen(s)+1)*sizeof(unichar));
  u_strcpy(n->formes_flechies,s);
  return;
}
tmp[0]='\0';
u_strcpy(tmp,n->formes_flechies);
i=0;
j=0;
debut=0;
l=u_strlen(tmp);
l2=u_strlen(s);
while ((i<l) && (j<l2)) {
  if (tmp[i++]==s[j]) {
     j++;
  }
  else {j=0;debut=i;}
  if ((j==l2) && (tmp[i]!=SEPARATOR_CHAR) && (tmp[i]!='\0')) {
     j=0;
     debut=i;
  }
}
if (((i==l) && (j<l2)) || (debut!=0 && tmp[debut-1]!=SEPARATOR_CHAR)) {
   j=0;
}
if (!j) {
  char t[2];
  sprintf(t,"%c",SEPARATOR_CHAR);
  u_strcat_char(tmp,t);
  u_strcat(tmp,s);
  free(n->formes_flechies);
  n->formes_flechies=(unichar*)malloc((u_strlen(tmp)+1)*sizeof(unichar));
  u_strcpy(n->formes_flechies,tmp);
}
}



void ajouter_forme_flechie(unichar* canonique,int i,struct noeud* n,unichar* flechi) {
struct noeud* sous_noeud;
sous_noeud=get_sous_noeud(n,canonique[i],1);
i++;
if (canonique[i]=='\0') {
   sous_noeud->controle=(unsigned char)(sous_noeud->controle|1);
   ajouter_flechi(sous_noeud,flechi);
}
else ajouter_forme_flechie(canonique,i,sous_noeud,flechi);
}



unsigned char* nouveau_code_pattern(int n_octet_code_gramm) {
unsigned char *c;
int i;
c=(unsigned char*)malloc(sizeof(unsigned char)*n_octet_code_gramm);
for (i=0;i<n_octet_code_gramm;i++) {
  c[i]=0;
}
return c;
}



//
// this function checks for each tag token if it verifies some patterns
//
void check_patterns_for_tag_tokens(Alphabet* alph,struct string_hash* tok,int n_octet_code_gramm,
									struct DLC_tree_info* DLC_tree) {
for (int i=0;i<tok->N;i++) {
   if (tok->tab[i][0]=='{' && u_strcmp_char(tok->tab[i],"{S}")  && u_strcmp_char(tok->tab[i],"{STOP}")) {
      // if the token is tag like {today,.ADV}
      // we add its number to the tag token list
      struct liste_nombres* L=new_liste_nombres();
      L->n=i;
      L->suivant=tag_token_list;
      tag_token_list=L;
      // and we look for the patterns it verifies
      unichar flechi[2000];
      unichar canonique[2000];
      unichar code_gramm[2000];
      tokenize_tag_token_into_3_parts(tok->tab[i],flechi,canonique,code_gramm);
      ajouter_forme_flechie(canonique,0,racine,tok->tab[i]);
      index_controle[i]=(unsigned char)(get_controle(tok->tab[i],alph,NULL)|DIC_TOKEN_BIT_MASK);
      if (code_gramm[0]!='\0') {
         if (index_code_gramm[i]==NULL) {
            index_code_gramm[i]=nouveau_code_pattern(n_octet_code_gramm);
         }
         get_numeros_pattern(code_gramm,index_code_gramm[i],canonique,n_octet_code_gramm);
         if (index_code_gramm[i]!=NULL) {
            int j=0;
            int non_nul=0;
            while (j<n_octet_code_gramm) {
              if (index_code_gramm[i][j]!=0) {
                non_nul=1;
                break;
              }
              j++;
            }
            if (!non_nul) {
               free(index_code_gramm[i]);
               index_code_gramm[i]=NULL;
            }
         }
      }
      
    if ( 0 && !est_un_mot_simple(flechi,alph)) {
    // on est dans le cas d'un mot composé
    if (code_gramm[0]!='\0') {
      unsigned char* code_gramm_temp=nouveau_code_pattern(n_octet_code_gramm);
      get_numeros_pattern(code_gramm,code_gramm_temp,canonique,n_octet_code_gramm);
      if (code_gramm_temp!=NULL) {
        int j=0;
        int non_nul=0;
        while (j<n_octet_code_gramm) {
          if (code_gramm_temp[j]!=0) {
            non_nul=1;
            break;
          }
          j++;
        }
        if (!non_nul) {
           free(code_gramm_temp);
        }
        else {
          // si le mot verifie au moins un pattern, on le met dans
          // l'arbre dlc
          j=0;
          while (j<n_octet_code_gramm) {
            for (int z=0;z<8;z++)
              if (code_gramm_temp[j]&(1<<z)) {
                ajouter_a_dlc_avec_code(flechi,j*8+z,alph,tok,DLC_tree);
              }
            j++;
          }
          free(code_gramm_temp);
        }
      }
    }
  }



  }
}
}

