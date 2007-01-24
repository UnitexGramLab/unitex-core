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
#include "Regular_expression.h"
//---------------------------------------------------------------------------


int reg2grf(unichar* s,char* nom_grf) {
int i;
int e1;
int e2;
FILE* fichier_reg;
Etat_reg tab_reg[N_ETATS_REG];
int etat_courant_reg;

if (s[0]=='\0') {
  fprintf(stderr,"You must specify a non empty regular expression\n");
  return 0;
}
fichier_reg=u_fopen(nom_grf,U_WRITE);
if (fichier_reg==NULL) {
  fprintf(stderr,"Cannot open the output file for the regular expression\n");
  return 0;
}
etat_courant_reg=2;
for (i=0;i<N_ETATS_REG;i++)
  tab_reg[i]=NULL;
unichar z[10];
u_strcpy_char(z,"<E>");
tab_reg[0]=nouvel_etat_reg(z);
u_strcpy_char(z,"");
tab_reg[1]=nouvel_etat_reg(z);

u_fprints_char("#Unigraph\n",fichier_reg);
u_fprints_char("SIZE 1313 950\n",fichier_reg);
u_fprints_char("FONT Times New Roman:  12\n",fichier_reg);
u_fprints_char("OFONT Times New Roman:B 12\n",fichier_reg);
u_fprints_char("BCOLOR 16777215\n",fichier_reg);
u_fprints_char("FCOLOR 0\n",fichier_reg);
u_fprints_char("ACOLOR 12632256\n",fichier_reg);
u_fprints_char("SCOLOR 16711680\n",fichier_reg);
u_fprints_char("CCOLOR 255\n",fichier_reg);
u_fprints_char("DBOXES y\n",fichier_reg);
u_fprints_char("DFRAME y\n",fichier_reg);
u_fprints_char("DDATE y\n",fichier_reg);
u_fprints_char("DFILE y\n",fichier_reg);
u_fprints_char("DDIR y\n",fichier_reg);
u_fprints_char("DRIG n\n",fichier_reg);
u_fprints_char("DRST n\n",fichier_reg);
u_fprints_char("FITS 100\n",fichier_reg);
u_fprints_char("PORIENT L\n",fichier_reg);
u_fprints_char("#\n",fichier_reg);

if (!reg_2_grf(s,&e1,&e2,tab_reg,&etat_courant_reg)) {
  u_fclose(fichier_reg);
  remove(nom_grf);
  //liberer_etats_reg(tab_reg);
  return 0;
}
relier_reg(0,e1,tab_reg);
relier_reg(e2,1,tab_reg);
sauver_etats_reg(fichier_reg,tab_reg,etat_courant_reg);
//liberer_etats_reg(tab_reg);
u_fclose(fichier_reg);
return 1;
}




//
// cree un nouvel etat
//
Etat_reg nouvel_etat_reg(unichar s[]) {
Etat_reg e;
e=(Etat_reg)malloc(sizeof(struct etat_reg));
e->contenu=(unichar*)malloc((u_strlen(s)+1)*sizeof(unichar));
u_strcpy(e->contenu,s);
e->l=NULL;
e->nombre_trans=0;
return e;
}



//
// relie l'etat e1 a l'etat e2
//
void relier_reg(int e1,int e2,Etat_reg tab_reg[]) {
struct liste_transitions_reg *l;
l=(struct liste_transitions_reg*)malloc(sizeof(struct liste_transitions_reg));
l->numero=e2;
l->suivant=tab_reg[e1]->l;
tab_reg[e1]->l=l;
tab_reg[e1]->nombre_trans++;
}



//
// convertit l'expression reguliere s en un graphe equivalent
//
int reg_2_grf(unichar s[],int *initial,int *final,Etat_reg tab_reg[],int* etat_courant_reg) {
int niveau,i,j;
unichar e1[1000];
unichar e2[1000];
int etat1;
int etat2;
int etat3;
int etat4;
int a,b;

/*printf("expression=|");
u_prints(s);
printf("|");
getchar(); */

remove_prefix(s,(unichar)' ');

if ((s[0]=='\0')||(s[0]==')')||(s[0]=='+')||(s[0]=='.')||(s[0]=='*')) {
  // en cas d'erreur...
  return 0;
}

if (s[0]=='(') {
  // si l'expression commence par une parenthese...
  niveau=1;
  i=1;
  int k=0;
  while (s[i] &&  !((s[i-1]==')')&&(niveau==0))) {
    e1[k++]=s[i];
    if (s[i]=='\\') {
      i++;
      e1[k++]=s[i];
    }
    if (s[i]=='(') niveau++;
    if (s[i]==')' && s[i-1]!='\\') niveau--;
    if (s[i]=='<') {
      do {
        i++;
        e1[k++]=s[i];
      } while ((s[i]!='>')&&(s[i]!='\0'));
      if (s[i]=='\0') return 0;

      /* $CD$ begin */
      if ( (u_strlen(s)-1 - i) > 2 && s[i+1] == '<' && s[i+2] == '<' ) {
        do {
            i++;
            e1[k++] = s[i];
            } while (s[i] != '>' && s[i] != '\0');
        if (s[i] == '\0' || s[i+1] == '\0' || s[i+1] != '>') return 0;
        i++;
        e1[k++] = s[i];
        }
      /* $CD$ end   */

    }
    else if (s[i]=='{') {
      do {
        i++;
        e1[k++]=s[i];
      } while ((s[i]!='}')&&(s[i]!='\0'));
      if (s[i]=='\0') return 0;
    }
    else if (s[i]=='"') {
      printf("ici\n");
      do {
        i++;
        e1[k++]=s[i];
      } while ((s[i]!='"') && (s[i]!='\0'));
      if (s[i]=='\0') return 0;
    }
    i++;
  }
  if (!((s[i-1]==')')&&(niveau==0))) {
    //erreur
    return 0;
  }
  e1[k-1]='\0';
  /*printf("Cas ():  e1=|");
  u_prints(e1);
  printf("|\n");*/
  if (!reg_2_grf(e1,&etat1,&etat2,tab_reg,etat_courant_reg))
    return 0;
}
else {
  i=0;
  int k=0;
  while ((s[i]!='+') && (s[i]!='.') && (s[i]!='*') && (s[i]!='\0') && (s[i]!='(') && (s[i]!=')') && (s[i]!=' ') && (!(s[i]=='<' && i>0)) && (!(s[i]=='{' && i>0))) {
    e1[k]=s[i];
    if (s[i]=='\\') {
      // si on lit un \ alors on lit le caractere suivant
      i++;
      if (s[i]=='"') {
         // case of \" that must be coded in GRF by \\\"
         e1[k+1]='\\';k=k+2;
      }
      else if (s[i]=='<') {
         // case of character < that must be coded in GRF by \<
         k++;
      }
      e1[k]=s[i];
    }
    else if (s[i]=='<') {
      // si on lit un pattern...
      e1[k]='<';
      i++;
      k++;
      while (s[i]!='>') {
        e1[k]=s[i];
        i++;
        k++;
      }
      e1[k]=s[i];

      /* $CD$ begin */
      if ( (u_strlen(s)-1 - i) > 2 && s[i+1] == '<' && s[i+2] == '<' ) {
        ++k;
        do {
            i++;
            e1[k++] = s[i];
            } while (s[i] != '>' && s[i] != '\0');
        if (s[i] == '\0' || s[i+1] == '\0' || s[i+1] != '>') return 0;
        i++;
        e1[k] = s[i];
        }
      /* $CD$ end   */

    }
    else if (s[i]=='"') {
      // si on lit un pattern...
      e1[k]='"';
      i++;
      k++;
      while (s[i]!='"') {
        e1[k]=s[i];
        i++;
        k++;
      }
      e1[k]='"';
    }
    else if (s[i]=='{') {
      // si on lit un pattern...
      e1[k]='{';
      i++;
      k++;
      while (s[i]!='}') {
        e1[k]=s[i];
        i++;
        k++;
      }
      e1[k]=s[i];
    }
    i++;
    k++;
  }
  e1[k]='\0';
  etat1=(*etat_courant_reg)++;
  tab_reg[etat1]=nouvel_etat_reg(e1);
  etat2=etat1;
}
if (s[i]==')') {
   fprintf(stderr,"Unexpected closing parenthesis\n");
   return 0;
}

if (s[i]=='\0') {
  *initial=etat1;
  *final=etat2;
  return 1;
}
if (s[i]=='*') {
  relier_reg(etat2,etat1,tab_reg);
  a=(*etat_courant_reg)++;
  unichar z[10];
  u_strcpy_char(z,"<E>");
  tab_reg[a]=nouvel_etat_reg(z);
  b=(*etat_courant_reg)++;
  tab_reg[b]=nouvel_etat_reg(z);
  relier_reg(a,etat1,tab_reg);
  relier_reg(etat2,b,tab_reg);
  relier_reg(a,b,tab_reg);
  *initial=a;
  *final=b;
  etat1=a;
  etat2=b;
  i++;
}
if (s[i]=='\0') {
  *initial=etat1;
  *final=etat2;
  return 1;
}
switch (s[i]) {
  case '.': {
              i++;
              j=0;
              while (s[i]!='\0')
                e2[j++]=s[i++];
              e2[j]='\0';
              if (!reg_2_grf(e2,&etat3,&etat4,tab_reg,etat_courant_reg))
                return 0;
              relier_reg(etat2,etat3,tab_reg);
              *initial=etat1;
              *final=etat4;
              return 1;
            }
  case ' ': {
              i++;
              j=0;
              while (s[i]!='\0')
                e2[j++]=s[i++];
              e2[j]='\0';
              if (j!=0) {
                 if (!reg_2_grf(e2,&etat3,&etat4,tab_reg,etat_courant_reg))
                    return 0;
                 relier_reg(etat2,etat3,tab_reg);
                 *initial=etat1;
                 *final=etat4;
                 return 1;
              } else {
                 // if we are in the case of a single space as e2
                 *initial=etat1;
                 *final=etat2;
                 return 1;
              }
            }
  case '+': {
              i++;
              j=0;
              while (s[i]!='\0')
                e2[j++]=s[i++];
              e2[j]='\0';
              /*printf("+ le e2 lu est _");
              u_prints(e2);
              printf("_\n");*/
              if (!reg_2_grf(e2,&etat3,&etat4,tab_reg,etat_courant_reg))
                return 0;
              a=(*etat_courant_reg)++;
              unichar z[10];
              u_strcpy_char(z,"<E>");
              tab_reg[a]=nouvel_etat_reg(z);
              b=(*etat_courant_reg)++;
              tab_reg[b]=nouvel_etat_reg(z);
              relier_reg(a,etat1,tab_reg);
              relier_reg(etat2,b,tab_reg);
              relier_reg(a,etat3,tab_reg);
              relier_reg(etat4,b,tab_reg);
              *initial=a;
              *final=b;
              return 1;
            }
  default: {
              j=0;
              while (s[i]!='\0')
                e2[j++]=s[i++];
              e2[j]='\0';
              if (!reg_2_grf(e2,&etat3,&etat4,tab_reg,etat_courant_reg))
                return 0;
              relier_reg(etat2,etat3,tab_reg);
              *initial=etat1;
              *final=etat4;
              return 1;
           }
}
}



//
// this function copy src into dest converting special chars to grf format
//
void convert_to_grf_format(unichar* dest,unichar* src) {
int i=0;
int j=0;
while (src[i]!='\0') {
  if (src[i]=='"') {
     dest[j++]='\\';
  }
  dest[j++]=src[i++];
}
dest[j]='\0';
}



//
// sauvegarde le graphe tab_reg
//
void sauver_etats_reg(FILE* fichier_reg,Etat_reg tab_reg[],int etat_courant_reg) {
int i;
struct liste_transitions_reg *l;
struct liste_transitions_reg *tmp;
unichar z[10000];
u_int_to_string(etat_courant_reg,z);
u_strcat_char(z,"\n");
u_fprints(z,fichier_reg);

for (i=0;i<etat_courant_reg;i++) {
  u_strcpy_char(z,"\"");
  unichar temp[1000];
  convert_to_grf_format(temp,tab_reg[i]->contenu);
  u_strcat(z,temp);
  u_strcat_char(z,"\" 100 100 ");
  u_fprints(z,fichier_reg);
  u_int_to_string(tab_reg[i]->nombre_trans,z);
  u_fprints(z,fichier_reg);

  l=tab_reg[i]->l;
  while (l!=NULL) {
    tmp=l->suivant;
    u_fprints_char(" ",fichier_reg);
    u_int_to_string(l->numero,z);
    u_fprints(z,fichier_reg);
    l=tmp;
  }
  u_fprints_char(" \n",fichier_reg);
}
}

