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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unicode.h"
#include "FileName.h"
#include "Copyright.h"
#include "IOBuffer.h"


//
// "E:\My Unitex\Greek\Corpus\31.txt" "E:\My Unitex\Greek\Graphs\31-PATRON.grf" "E:\My Unitex\Greek\Graphs\result.grf" "E:\My Unitex\Greek\Graphs\result@%.grf"
//


void usage() {
printf("%s",COPYRIGHT);
printf("Usage: Table2Grf <table> <graph> <result> [subgraph]\n");
printf("       <table> : unicode text table with tabs as separator\n");
printf("       <graph> : reference graph\n");
printf("       <result> : name of the result main graph\n");
printf("       [subgraph] : this optionnal parameter specify the name of the\n");
printf("                    subgraphs\n");
printf("Applies a reference graph to a lexicon-grammar table, producing a sub-graph\n");
printf("for each entry of the table.\n");
}


//---------------------------------------------------------------------------

void table2grf(FILE*,FILE*,FILE*,char*/*,char**/);

int main(int argc, char **argv) {
setBufferMode();

if (argc<4 || argc>5) {
   usage();
   return 0;
}
FILE* table=u_fopen(argv[1],U_READ);
if (table==NULL) {
   fprintf(stderr,"Cannot open table %s\n",argv[1]);
   return 1;
}
FILE* reference_graph=u_fopen(argv[2],U_READ);
if (reference_graph==NULL) {
   fprintf(stderr,"Cannot open reference graph %s\n",argv[2]);
   u_fclose(table);
   return 1;
}
if ((strlen(argv[3])-4)<=0) {
   fprintf(stderr,"Error in result graph name %s\n",argv[3]);
   u_fclose(table);
   return 1;
}
FILE* result_graph=u_fopen(argv[3],U_WRITE);
if (result_graph==NULL) {
   fprintf(stderr,"Cannot create result graph %s\n",argv[3]);
   u_fclose(table);
   u_fclose(reference_graph);
   return 1;
}
char subgraph[2000];
if (argc!=5) {
   // if no subgraph name is given for the graph TUTU.grf, we
   // take TUTU_xxxx as subgraph name
   //name_without_path(argv[3],subgraph);
   //subgraph[strlen(subgraph)-4]='\0';
   strcpy(subgraph,argv[3]);
   subgraph[strlen(subgraph)-4]='\0';
   strcat(subgraph,"_@%.grf");
} else {
   // if we do have a subgraph name, we just take it
   strcpy(subgraph,argv[4]);
}
//char chemin[1000];
//get_filename_path(argv[3],chemin);
table2grf(table,reference_graph,result_graph,subgraph/*,chemin*/);
return 0;
}



//---------------------------------------------------------------------------



struct etat {
  unichar contenu[5000];
  int x;
  int y;
  int n_trans;
  int marque;
  int trans[200];
};

struct graphe_patron {
  unichar en_tete[3000];
  int n_etats;
  struct etat* tab[2000];
};




void write_result_graph_header(FILE *f) {
u_fprints_char("#Unigraph\n",f);
u_fprints_char("SIZE 950 1328\n",f);
u_fprints_char("FONT Times New Roman:  12\n",f);
u_fprints_char("OFONT Times New Roman:B 12\n",f);
u_fprints_char("BCOLOR 16777215\n",f);
u_fprints_char("FCOLOR 0\n",f);
u_fprints_char("ACOLOR 12632256\n",f);
u_fprints_char("SCOLOR 16711680\n",f);
u_fprints_char("CCOLOR 255\n",f);
u_fprints_char("DBOXES y\n",f);
u_fprints_char("DFRAME y\n",f);
u_fprints_char("DDATE n\n",f);
u_fprints_char("DFILE y\n",f);
u_fprints_char("DDIR n\n",f);
u_fprints_char("DRIG n\n",f);
u_fprints_char("DRST n\n",f);
u_fprints_char("FITS 100\n",f);
u_fprints_char("PORIENT P\n",f);
u_fprints_char("#\n",f);
u_fprints_char("3\n",f);
u_fprints_char("\"<E>\" 68 368 1 2 \n",f);
u_fprints_char("\"\" 456 368 0 \n",f);
u_fprints_char("\"",f);
}


struct etat* nouvel_etat() {
struct etat* e;
e=(struct etat*)malloc(sizeof(struct etat));
e->contenu[0]='\0';
e->x=0;
e->y=0;
e->marque=0;
e->n_trans=0;
return e;
}



void lire_ligne(FILE *f,struct etat *e) {
int i;
int c;
while (u_fgetc(f)!='"');
i=0;
while ((c=u_fgetc(f))!='"') {
  e->contenu[i++]=(unichar)c;
  if (c=='\\') {
    // cas d'un caractere protege par un back-slash
    e->contenu[i++]=(unichar)u_fgetc(f);
  }
}
e->contenu[i]='\0';
// we read the space after the "
u_fgetc(f);
e->x=u_read_int(f);
e->y=u_read_int(f);
e->n_trans=u_read_int(f);
for (i=0;i<e->n_trans;i++) {
  e->trans[i]=u_read_int(f);
}
// we read the \n at the end of line
u_fgetc(f);
}



void charger_graphe_patron(FILE *f,struct graphe_patron *g) {
int i;
int c;
g->en_tete[0]=(unichar)u_fgetc(f);
i=1;
while ((c=u_fgetc(f))!='#')
  g->en_tete[i++]=(unichar)c;
g->en_tete[i]='#';
g->en_tete[i+1]='\0';
// we read th \n after the #
u_fgetc(f);
// the read the number of states
g->n_etats=u_read_int(f);
// we read the lines of the graph
for (i=0;i<g->n_etats;i++) {
  g->tab[i]=nouvel_etat();
  lire_ligne(f,g->tab[i]);
}
}



void read_table_first_line(FILE *f,int *n) {
int c;
(*n)=0;
while ((c=u_fgetc(f))!='\n')
  if (c=='\t') (*n)++;
(*n)++;
}



int read_field(FILE *f,unichar** ligne,int colonne,int delimiteur) {
int i;
int c;
unichar tmp[1000];
i=0;
while ((c=u_fgetc(f))!=EOF && c!=delimiteur && c!='\n') {
  tmp[i]=(unichar)c;
  i++;
}
if ((i==0 && c==EOF) || c!=delimiteur) return 0;
tmp[i]='\0';
ligne[colonne]=(unichar*)malloc(sizeof(unichar)*(i+1));
u_strcpy(ligne[colonne],tmp);
return 1;
}



int read_table_line(FILE *f,unichar** ligne,int n_champs) {
int i;
for (i=0;i<n_champs;i++) {
   if (!read_field(f,ligne,i,(i!=(n_champs-1))?'\t':'\n')) {
      if (i==0) {
         // case of an empty line
         return 0;
      } else {
         // case of missing fields at the end of line
         for (int j=i;j<n_champs;j++) {
            ligne[j]=(unichar*)malloc(sizeof(unichar));
            ligne[j][0]='\0';
         }
         return 1;
      }
   }
}
return 1;
}



void determine_subgraph_name(char* base,char* res,int n) {
sprintf(res,"%s_%04d",base,n);
}



int position(unichar* s,unichar c) {
int i;
i=0;
while (s[i]!=c && s[i]!='\0') i++;
if (s[i]=='\0') return -1;
return i;
}



int get_num(unichar m[]) {
if (m[1]=='\0')
  return (m[0]-'A');
if (m[2]=='\0')
  return ((m[0]-'A'+1)*26+(m[1]-'A'));
return -1;
}


int is_in_A_Z(int n) {
return (n>='A') && (n<='Z');
}


//
// this function takes a string and replace in it every reference to a table
// variable
//
void convertir(unichar* dest,unichar* source,unichar** champ,int n_champs,int ligne_courante) {
int pos_in_dest;
char tmp[5];
unichar line_number[5];
int row_number;
int negation=0;
int pos_in_src=0;

// we assume that a - reference can only appear in a single line box
// like @B, @AB  or @T/something
if (source[0]=='!') {
   negation=1;
   pos_in_src++;
}
if (source[pos_in_src]=='@' && is_in_A_Z(source[pos_in_src+1])) {
   // if we have a @A... variable
   row_number=-1;
   if (source[pos_in_src+2]=='\0' || source[pos_in_src+2]=='/') {
      // if we are in the case @A or @A/something
      row_number=source[pos_in_src+1]-'A';
   }
   else if (is_in_A_Z(source[pos_in_src+2]) && (source[pos_in_src+3]=='\0' || source[pos_in_src+3]=='/')) {
           // if we are in the case @AB or @AB/something
           row_number=(source[pos_in_src+1]-'A'+1)*(26)+(source[pos_in_src+2]-'A');
   }
   if (row_number!=-1) {
      // if we have a valid row reference
      if ((!negation && !u_strcmp_char(champ[row_number],"-"))
          || (negation && !u_strcmp_char(champ[row_number],"+"))) {
         // and if this reference points on a -,
         // then we must remove this state
         u_strcpy_char(dest,"");
         return;
      }
   }
}

// then, we can focus on the general case
sprintf(tmp,"%04d",ligne_courante);
u_strcpy_char(line_number,tmp);

pos_in_src=0;
pos_in_dest=0;
while (source[pos_in_src]!='\0') {
   if (source[pos_in_src]=='@') {
      // first, we test the presence of a negation sign
      if (pos_in_src>0 && source[pos_in_src-1]=='!') {
         negation=1;
      }
      else {
         negation=0;
      }
      pos_in_src++;
      if (source[pos_in_src]=='%') {
         // if are in the @% case
         // if we had a negation before, we restore the ! sign
         if (negation) {dest[pos_in_dest++]='!';}
         pos_in_src++;
         int i=0;
         while (line_number[i]!='\0') {
            dest[pos_in_dest++]=line_number[i++];
         }
      }
      else if (!is_in_A_Z(source[pos_in_src])) {
         // if we don't have a valid variable name, we produce the @ char
         // eventually preceeded by a ! char if a negation was found
         if (negation) {dest[pos_in_dest++]='!';}
         dest[pos_in_dest++]='@';
      }
      else {
         if (is_in_A_Z(source[pos_in_src+1])) {
            // if we are in the @AB case
            row_number=(source[pos_in_src]-'A'+1)*(26)+(source[pos_in_src+1]-'A');
            pos_in_src++;
         }
         else {row_number=source[pos_in_src]-'A';}
         pos_in_src++;
         if (!u_strcmp_char(champ[row_number],"+")) {
            if (negation) {
               // if we have a - sign, we do nothing
            }
            else {
               // if we have a + sign, we replace it by <E>
               dest[pos_in_dest++]='<';
               dest[pos_in_dest++]='E';
               dest[pos_in_dest++]='>';
            }
         }
         else if (!u_strcmp_char(champ[row_number],"-")) {
            if (!negation) {
               // if we have a - sign, we do nothing
            }
            else {
               // if we have a + sign, we replace it by <E>
               dest[pos_in_dest++]='<';
               dest[pos_in_dest++]='E';
               dest[pos_in_dest++]='>';
            }
         }
         else {
            // if we have a plain content, we must add it to dest
            // if we had a negation, we restore the ! char
            if (negation) {dest[pos_in_dest++]='!';}
            int i=0;
            while (champ[row_number][i]!='\0') {
               dest[pos_in_dest++]=champ[row_number][i++];
            }
         }
      }
   }
   else if (source[pos_in_src]=='!') {
           // if we find a !
           if (source[pos_in_src+1]=='@') {
              // and if it is followed by a @
              // then we ignore it
              pos_in_src++;
           }
           else {
              // else, we consider it as a single char
              dest[pos_in_dest++]=source[pos_in_src++];
           }
   }
   else {
      // normal case
      dest[pos_in_dest++]=source[pos_in_src++];
   }
}
dest[pos_in_dest]='\0';
}




//
// deprecated
//
void convertir_old(unichar* dest,unichar* source,unichar** champ,int n_champs,int ligne_courante) {
int pos;
int i;
unichar s[3000];
unichar tmp[20];
unichar num[3];
int numero;
int negation;
int plus;

u_int_to_string(ligne_courante,tmp);
u_strcpy(s,source);
u_strcpy(dest,s);

// on suppose que si on a une reference a une colonne +/-, elle est
// seule dans une boite

while ((pos=position(s,'@'))!=-1) {
  // we check if a ! sign preceed the @ sign
  if (pos>0 && s[pos-1]=='!') negation=1;
  else negation=0;

  if (s[pos+1]=='%') {
     // cas de @% qui doit être remplacé par le numéro de ligne courant
    if (negation==0) dest[pos]='\0';
    else dest[pos-1]='\0';
    u_strcat(dest,tmp);
    i=pos+u_strlen(tmp);
    pos=pos+2;
    while (s[pos]!='\0')
      dest[i++]=s[pos++];
    dest[i]='\0';
  }
  else {
    i=0;
    while (s[pos+1+i]>='A' && s[pos+1+i]<='Z') {
      num[i]=s[pos+1+i];
      i++;
    }
    if (i==0) {
       fprintf(stderr,"Error in the reference graph:\n");
       fprintf(stderr,"the @ sign must be followed by %% or by one or two letters within [A-Z]\n");
       exit(1);
    }
    num[i]='\0';
    numero=get_num(num);
    if (negation==0) dest[pos]='\0';
    else dest[pos-1]='\0';
    if (!u_strcmp_char(champ[numero],"+") || !u_strcmp_char(champ[numero],"-")) {
      // si on a une reference + ou -
      plus=(!u_strcmp_char(champ[numero],"+"));
      if ((plus && !negation)||(!plus && negation))
        u_strcpy_char(dest,"<E>");
      else u_strcpy_char(dest,"");
      return;
    }
    u_strcat(dest,champ[numero]);
    pos=pos+i+1;
    i=u_strlen(dest);
    while (s[pos]!='\0') {
      dest[i++]=s[pos++];
    }
    dest[i]='\0';
  }
  u_strcpy(s,dest);
}
}



int co_accessibilite(struct graphe_patron* g,int e) {
int i;
if (g->tab[e]->marque) return 1;
if (g->tab[e]->contenu[0]=='\0') return 0;
g->tab[e]->marque=1;
i=0;
while (i<g->tab[e]->n_trans) {
  if (!co_accessibilite(g,g->tab[e]->trans[i])) {
    g->tab[e]->trans[i]=g->tab[e]->trans[g->tab[e]->n_trans-1];
    g->tab[e]->n_trans--;
  } else i++;
}
return 1;
}



int nettoyer_graphe(struct graphe_patron *G) {
int i,n_etats,etat_courant,j;
int *t;

if (G->tab[0]->contenu==NULL) {
   printf("internal error in nettoyer_graphe: NULL for initial state content\n");
   exit(1);
}
if (G->tab[0]->contenu[0]=='\0') {
   // if the initial state is empty, we must remove the whole graph
   return 0;
}
n_etats=G->n_etats;
G->tab[1]->marque=1;
co_accessibilite(G,0);
if (G->tab[0]->n_trans==0) {
   // if there is no more transition out of the initial state, we return 0
   return 0;
}
t=(int*)malloc(sizeof(int)*n_etats);
for (i=0;i<n_etats;i++) {
  t[i]=i;
}
etat_courant=2; // we do not remove nor state 0 neither state 1
while (etat_courant<n_etats) {
  while (etat_courant<n_etats && G->tab[etat_courant]->marque==1)
    etat_courant++;
  while (n_etats>etat_courant && G->tab[n_etats-1]->marque==0) {
    // tant que le dernier etat est a virer, on le vire
    free(G->tab[n_etats-1]);
    n_etats--;
  }
  if (etat_courant==n_etats-1) {
    // si on doit virer le dernier etat sans faire d'echange
    free(G->tab[etat_courant]);
    n_etats--;
  }
  else
  if (etat_courant<n_etats) {
    // on est dans le cas ou l'etat courant est a virer
    free(G->tab[etat_courant]);
    G->tab[etat_courant]=G->tab[n_etats-1];
    G->tab[n_etats-1]=NULL;
    t[n_etats-1]=etat_courant;
    n_etats--;
  }
}
G->n_etats=n_etats;
for (i=0;i<n_etats;i++)
  for (j=0;j<G->tab[i]->n_trans;j++) {
    G->tab[i]->trans[j]=t[G->tab[i]->trans[j]];
  }
free(t);
return 1;
}



bool create_graph(int ligne_courante,unichar** ligne,int n_champs,struct graphe_patron* g,
                  char* nom_resultat,/*char* chemin,*/FILE *f_coord,int graphs_printed) {
char nom_res[1000];
//char nom[1000];
struct graphe_patron r;
struct graphe_patron* res;
int i,j;
FILE *f;
res=&r;
//determine_subgraph_name(nom_resultat,nom_res,ligne_courante);
res->n_etats=g->n_etats;
for (i=0;i<g->n_etats;i++) {
  res->tab[i]=nouvel_etat();
  res->tab[i]->x=g->tab[i]->x;
  res->tab[i]->y=g->tab[i]->y;
  res->tab[i]->n_trans=g->tab[i]->n_trans;
  for (j=0;j<g->tab[i]->n_trans;j++) {
    res->tab[i]->trans[j]=g->tab[i]->trans[j];
  }
  convertir(res->tab[i]->contenu,g->tab[i]->contenu,ligne,n_champs,ligne_courante);
}
// we compute the name of the subgraph
unichar tmp[2000];
unichar tmp2[2000];
u_strcpy_char(tmp,nom_resultat);
tmp2[0]='\0';
convertir(tmp2,tmp,ligne,n_champs,ligne_courante);
u_to_char(nom_res,tmp2);
if (!nettoyer_graphe(res)) {
   // if the graph has been emptied, we return
   return false;
} 
if (graphs_printed!=0) {
  /* print a "+" only if any subgraph already has been printed:
   *  - not in first line (ligne_courante==0)
   *  - not if only empty subgraphs have occured since */
  u_fprints_char("+",f_coord);
}
u_fprints_char(":",f_coord);
{
char tmp3[2000];
char tmp4[2000];
name_without_path(nom_res,tmp3);
name_without_extension(tmp3,tmp4);
u_fprints_char(tmp4,f_coord);
}
//strcpy(nom,chemin);
//strcat(nom,nom_res);
//strcat(nom,".grf");
f=u_fopen(/*nom*/nom_res,U_WRITE);
if (f==NULL) {
  fprintf(stderr,"Cannot create subgraph %s\n",/*nom*/nom_res);
  return false;
}
if (ligne_courante%10==0) {
   printf("%d table entries done...                \r",ligne_courante);
}
u_fprints(g->en_tete,f);
u_fprints_char("\n",f);
unichar et[10];
u_int_to_string(res->n_etats,et);
u_fprints(et,f);
u_fprints_char("\n",f);
char temp[1000];
for (i=0;i<res->n_etats;i++) {
  u_fprints_char("\"",f);
  u_fprints(res->tab[i]->contenu,f);
  sprintf(temp,"\" %d %d %d",res->tab[i]->x,res->tab[i]->y,res->tab[i]->n_trans);
  u_fprints_char(temp,f);
  for (j=0;j<res->tab[i]->n_trans;j++) {
     sprintf(temp," %d",res->tab[i]->trans[j]);
     u_fprints_char(temp,f);
  }
  u_fprints_char(" \n",f);
  free(res->tab[i]);
}
fclose(f);
return true;
}



void table2grf(FILE* table,FILE* reference_graph,FILE* result_graph,char* subgraph/*,
               char* chemin*/) {
int ligne_courante;
struct graphe_patron structure;
unichar* ligne[1000];
int n_champs;
int i;
int graphs_printed;
write_result_graph_header(result_graph);
printf("Loading reference graph...\n");
charger_graphe_patron(reference_graph,&structure);
u_fclose(reference_graph);
printf("Reading lexicon-grammar table...\n");
read_table_first_line(table,&n_champs);
ligne_courante=1;
graphs_printed=0;
while (read_table_line(table,(unichar**)ligne,n_champs)) {
  if (create_graph(ligne_courante,ligne,n_champs,&structure,subgraph,/*chemin,*/result_graph,graphs_printed))
    graphs_printed++;
  ligne_courante++;
}
u_fclose(table);
for (i=0;i<structure.n_etats;i++) {
  free(structure.tab[i]);
}
u_fprints_char("\" 216 368 1 1 \n",result_graph);
u_fclose(result_graph);
printf("Done.                                             \n");
}

