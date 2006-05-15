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
#include "Grammatical_codes.h"
//---------------------------------------------------------------------------



struct noeud_code_gramm* nouveau_noeud_code_gramm() {
struct noeud_code_gramm *n;
n=(struct noeud_code_gramm*)malloc(sizeof(struct noeud_code_gramm));
n->liste=NULL;
n->mot=NULL;
n->l=NULL;
return n;
}



void inserer_code_gramm(int numero_pattern,unichar* s,unichar* canonique) {
unichar* t[MAX_FLEXIONAL_CODES_LENGTH];
unichar* t2[MAX_FLEXIONAL_CODES_LENGTH];
struct facteurs_interdits *f;
Code_flexion c;
int i;
for (i=0;i<MAX_FLEXIONAL_CODES_LENGTH;i++) {
  t[i]=NULL;
  t2[i]=NULL;
}
c=NULL;
f=nouveaux_facteurs_interdits();
decouper_code_gramm(s,t,t2,f);
if (f->nbre_facteurs==0) {
  liberer_facteurs_interdits(f);
  f=NULL;
}
if (t[0]==NULL) {
  return;
}
if (t2[0]!=NULL) {
  c=calculer_code_flexion(t2);
}
for (i=0;i<MAX_FLEXIONAL_CODES_LENGTH;i++)
  if (t2[i]!=NULL) {
    free(t2[i]);
    t2[i]=NULL;
  }
ajouter_combinaisons_code_gramm(t,c,numero_pattern,f,canonique);
for (i=0;i<MAX_FLEXIONAL_CODES_LENGTH;i++)
  if (t[i]!=NULL) {
    free(t[i]);
    t[i]=NULL;
  }
if (c!=NULL) free(c);
if (f!=NULL) liberer_facteurs_interdits(f);
}



void decouper_code_gramm(unichar* s,unichar**t,
                         unichar** t2,struct facteurs_interdits *f) {
int pos,i;
int courant_t;
int courant_t2;
int fin;
int moins;
unichar tmp[5000];
courant_t=0;
courant_t2=0;
i=0;
if ((s==NULL)||(s[0]=='\0')) {
   return;
}
// lecture des i-categories
moins=0;
if (s[i]=='-') {
    moins=1;
    i++;
  }
fin=0;
while ((!fin)&&(s[i]!=':')) {
  pos=0;
  tmp[0]='\0';
  do {
    if (s[i]=='\\') {
       i++;
       if (s[i]!='\0') {
          tmp[pos++]=s[i];
          i++;
       }
       else {
          fprintf(stderr,"Error in a pattern: \\ at the end of a grammatical or semantic code\n");
       }
    }
    else tmp[pos++]=s[i++];
  }
  while ((s[i]!='+')&&(s[i]!=':')&&(s[i]!='-')&&(s[i]!='\0'));
  tmp[pos]='\0';
  if (!moins) {
    t[courant_t]=(unichar*)malloc((u_strlen(tmp)+1)*sizeof(unichar));
    t[courant_t][0]='\0';
    if (courant_t==(MAX_SEMANTIC_CODES-1)) {
      return;
    }
    u_strcpy(t[courant_t++],tmp);
  }
  else {
    ajouter_facteur_interdit(f,tmp);
  }
  moins=(s[i]=='-');
  if (s[i]=='\0') return;
  fin=(s[i]==':');
  i++;
}
if (s[i]==':') i++;
// lecture des codes de flexion

fin=0;
while (!fin) {
  pos=0;
  tmp[0]='\0';
  do
    tmp[pos++]=s[i++];
  while ((s[i]!=':')&&(s[i]!='\0'));
  tmp[pos]='\0';
  t2[courant_t2]=(unichar*)malloc((u_strlen(tmp)+1)*sizeof(unichar));
  t2[courant_t2][0]='\0';
  if (courant_t2==(MAX_INFLECTIONAL_CODES-1)) {
    return;
  }
  u_strcpy(t2[courant_t2++],tmp);
  if (s[i]=='\0') return;
  i++;
}
}



void ajouter_combinaisons_code_gramm(unichar** t,Code_flexion c,int numero_pattern,
                                     struct facteurs_interdits *f,unichar* canonique) {
int n;
int marque[MAX_FLEXIONAL_CODES_LENGTH];
int i;
unichar* t2[MAX_FLEXIONAL_CODES_LENGTH];
for (i=0;i<MAX_FLEXIONAL_CODES_LENGTH;i++) {
   marque[i]=0;
   t2[i]=NULL;
}
n=0;
while (t[n]!=NULL) {
   n++;
}
creer_ensemble_code_gramm(marque,t,t2,c,numero_pattern,n,0,f,canonique);
}



void creer_ensemble_code_gramm(int* marque,unichar** t,unichar** t2,Code_flexion c,
                               int numero_pattern,int n,int compteur,
                               struct facteurs_interdits* f,unichar* canonique) {
int i;
if (compteur==n) {
   ajouter_element_code_gramm(t2,0,racine_code_gramm,c,numero_pattern,f,canonique);
   return;
}
for (i=0;i<n;i++)
  if (marque[i]==0) {
     marque[i]=1;
     t2[compteur]=(unichar*)malloc((u_strlen(t[i])+1)*sizeof(unichar));
     u_strcpy(t2[compteur],t[i]);
     creer_ensemble_code_gramm(marque,t,t2,c,numero_pattern,n,compteur+1,f,canonique);
     if (t2[compteur]!=NULL) {
        free(t2[compteur]);
        t2[compteur]=NULL;
     }
     marque[i]=0;
  }
}



struct noeud_code_gramm* get_sous_noeud_code_gramm(struct noeud_code_gramm* n,
                                                   unichar* mot,int creer) {
struct liste_feuilles_code_gramm *ptr;
struct liste_feuilles_code_gramm *ptr2;
struct noeud_code_gramm* res;
ptr=n->l;
while ((ptr!=NULL)&&u_strcmp((ptr->node)->mot,mot))
  ptr=ptr->suivant;
if (ptr==NULL) {        // si on veut juste savoir si le noeud existe
   if (!creer) {           // et que le noeud n'existe pas, on renvoie NULL
      return NULL;
   }
   res=nouveau_noeud_code_gramm();
   res->mot=(unichar*)malloc((u_strlen(mot)+1)*sizeof(unichar));
   u_strcpy(res->mot,mot);
   ptr2=(struct liste_feuilles_code_gramm*)malloc(sizeof(struct liste_feuilles_code_gramm));
   ptr2->node=res;
   ptr2->suivant=n->l;
   n->l=ptr2;
   return res;
}
return ptr->node;
}




void ajouter_element_code_gramm(unichar** s,int i,struct noeud_code_gramm* n,
                                Code_flexion c,int numero_pattern,
                                struct facteurs_interdits* f,unichar* canonique) {
struct noeud_code_gramm* sous_noeud;
sous_noeud=get_sous_noeud_code_gramm(n,s[i],1);
i++;
if (s[i]==NULL) {
   ajouter_a_liste_code_flexion(sous_noeud,c,numero_pattern,f,canonique);
}
else ajouter_element_code_gramm(s,i,sous_noeud,c,numero_pattern,f,canonique);
}




void decouper_code_gramm_dic(unichar* s,unichar** t,
                         unichar** t2,struct facteurs_interdits* f,int probleme) {
int pos,i;
int courant_t;
int courant_t2;
int fin;
unichar tmp[5000];
courant_t=0;
courant_t2=0;
i=0;

if ((s==NULL)||(s[0]=='\0')) {
   return;
}
fin=0;
while ((!fin) && (s[i]!=':')) {
  pos=0;
  tmp[0]='\0';
  do {
     tmp[pos++]=s[i++];
  }
  while ((s[i]!='+') && (s[i]!=':') && (s[i]!='/') && (s[i]!='\0'));
  tmp[pos]='\0';
  t[courant_t]=(unichar*)malloc((u_strlen(tmp)+1)*sizeof(unichar));
  t[courant_t][0]='\0';
  if (courant_t==(MAX_SEMANTIC_CODES-1)) {
     return;
  }
  u_strcpy(t[courant_t++],tmp);
  if (s[i]=='\0') {
     return;
  }
  if (s[i]==':') {
     fin=1;
  }
  else {
       i++;
  }
}
if (s[i]==':') {
   i++;
}

// lecture des codes de flexion
fin=0;
while (!fin) {
  pos=0;
  tmp[0]='\0';
  do {
     tmp[pos++]=s[i++];
  }
  while ((s[i]!=':') && (s[i]!='/') && (s[i]!='\0'));
  tmp[pos]='\0';
  t2[courant_t2]=(unichar*)malloc((u_strlen(tmp)+1)*sizeof(unichar));
  t2[courant_t2][0]='\0';
  if (courant_t2==(MAX_INFLECTIONAL_CODES-1)) {
     return;
  }
  u_strcpy(t2[courant_t2++],tmp);
  if (s[i]=='\0'|| s[i]=='/') {
     return;
  }
  i++;
}
}



int comparer_canoniques(unichar* a,unichar* forme_du_dico) {
if (a==NULL) return 1;
if (forme_du_dico==NULL) return 0;
if (!u_strcmp(a,forme_du_dico)) return 1;
return 0;
}



int est_compatible_sous_code(unichar* code,unichar* pattern) {
int i,j;
j=0;
while (pattern[j]!='\0') {
  i=0;
  while ((code[i]!='\0')&&(pattern[j]!=code[i])) i++;
  if (code[i]=='\0') return 0;
  j++;
}
return 1;
}



int est_compatible2(unichar* code,unichar* pattern) {
int code_i,pattern_i,i,j,debut_pattern;
unichar a[100],b[100];
// si le pattern est vide, n'importe quoi le vérifie
if (pattern[0]=='\0') return 1;
// mais si le pattern est non vide et que le code est vide
// il y a forcement incompatibilite
if (code[0]=='\0') return 0;
// on saute les : pour se placer sur les premiers caracteres
// significatifs
if (code[0]==':')
  code_i=1;
else code_i=0;
if (pattern[0]==':')
  debut_pattern=1;
else debut_pattern=0;

do {
  i=0;
  do a[i++]=code[code_i++];
  while ((code[code_i]!=':')&&(code[code_i]!='\0'));
  a[i]='\0';
  pattern_i=debut_pattern;
  do {
    j=0;
    do b[j++]=pattern[pattern_i++];
    while ((pattern[pattern_i]!=':')&&(pattern[pattern_i]!='\0'));
    b[j]='\0';
    if (est_compatible_sous_code(a,b)) return 1;
  } while (pattern[pattern_i++]!='\0');
} while (code[code_i++]!='\0');
return 0;
}



int est_compatible(Code_flexion c,Code_flexion d) {
return est_compatible2(c->s,d->s);
}



void explorer_codes_flexion(struct liste_code_flexion* l,Code_flexion c,
							unichar** t,unsigned char* res,
                            unichar* canonique,int n_octet_code_gramm) {
struct liste_code_flexion* ptr;
int i,j,ok,k;
ptr=l;
while (ptr!=NULL) {
if (comparer_canoniques(ptr->canonique,canonique))
  if (ptr->code==NULL) {
    if (ptr->f==NULL) {
       if (res==NULL) {
          res=nouveau_code_pattern(n_octet_code_gramm);
       }
       k=ptr->numero_pattern;
       res[k/8]=(unsigned char)(res[k/8]|(1<<(k%8)));
    }
    else {
       ok=1;
       for (i=0;i<(ptr->f)->nbre_facteurs;i++) {
         j=0;
         while ((t[j]!=NULL) && u_strcmp(t[j],(ptr->f)->facteur[i])) {
            j++;
         }
         ok=(t[j]==NULL);
         if (!ok) break;
       }
       if (ok) {
          if (res==NULL) {
             res=nouveau_code_pattern(n_octet_code_gramm);
          }
          k=ptr->numero_pattern;
          res[k/8]=(unsigned char)(res[k/8]|(1<<(k%8)));
       }
    }
  }
  else if ((c!=NULL) && est_compatible(c,ptr->code)) {
         if (ptr->f==NULL) {
            if (res==NULL) res=nouveau_code_pattern(n_octet_code_gramm);
            k=ptr->numero_pattern;
            res[k/8]=(unsigned char)(res[k/8]|(1<<(k%8)));
         }
         else {
           ok=1;
           for (i=0;i<(ptr->f)->nbre_facteurs;i++) {
             j=0;
             while ((t[j]!=NULL) && u_strcmp(t[j],((ptr->f)->facteur[i]))) {
                j++;
             }
             ok=(t[j]==NULL);
             if (!ok) break;
           }
         if (ok) {
           if (res==NULL) res=nouveau_code_pattern(n_octet_code_gramm);
           k=ptr->numero_pattern;
           res[k/8]=(unsigned char)(res[k/8]|(1<<(k%8)));
         }
         }
  	   }
  ptr=ptr->suivant;
}
}




void trouver_numeros_pattern(struct noeud_code_gramm* n,unichar** t,int niveau,
                             Code_flexion c,unsigned char* res,unichar* canonique,
                             int n_octet_code_gramm) {
struct noeud_code_gramm* sous_noeud;
struct liste_code_flexion *l;
if (t[niveau]==NULL) {
   return;
}
sous_noeud=get_sous_noeud_code_gramm(n,t[niveau],0);
if (t[niveau+1]!=NULL) {
   trouver_numeros_pattern(n,t,niveau+1,c,res,canonique,n_octet_code_gramm);
}
if (sous_noeud!=NULL) {
   l=sous_noeud->liste;
   if (l!=NULL) {
      explorer_codes_flexion(l,c,t,res,canonique,n_octet_code_gramm);
   }
   trouver_numeros_pattern(sous_noeud,t,niveau+1,c,res,canonique,n_octet_code_gramm);
}
}




void get_numeros_pattern(unichar* s,unsigned char* res,
                         unichar* canonique,int n_octet_code_gramm) {
unichar* t[MAX_FLEXIONAL_CODES_LENGTH];
unichar* t2[MAX_FLEXIONAL_CODES_LENGTH];
struct facteurs_interdits* f;
Code_flexion c;
unichar tmp[TAILLE_MOT];
int i;
for (i=0;i<MAX_FLEXIONAL_CODES_LENGTH;i++) {
  t[i]=NULL;
  t2[i]=NULL;
}
c=NULL;
f=nouveaux_facteurs_interdits();
decouper_code_gramm_dic(s,t,t2,f,0);
if (f->nbre_facteurs==0) {
   liberer_facteurs_interdits(f);
   f=NULL;
}
if (t2[0]!=NULL) {
   c=calculer_code_flexion(t2);
}
for (i=0;i<MAX_FLEXIONAL_CODES_LENGTH;i++) {
  if (t2[i]!=NULL) {
     free(t2[i]);
     t2[i]=NULL;
  }
}
if (f!=NULL && f->nbre_facteurs) {
   liberer_facteurs_interdits(f);
   f=NULL;
   tmp[0]='\0';
}
if (f!=NULL) {
   liberer_facteurs_interdits(f);
}
trouver_numeros_pattern(racine_code_gramm,t,0,c,res,canonique,n_octet_code_gramm);
if (c!=NULL) {
   free(c);
}
for (i=0;i<100;i++) {
  if (t[i]!=NULL) {
     free(t[i]);
     t[i]=NULL;
  }
}
}

