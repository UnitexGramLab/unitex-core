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
#include "Unicode.h"
#include "FileName.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"

#define MAX_LINES_IN_TABLE 1024  


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Table2Grf <table> <graph> <result> [subgraph]\n"
       "       <table> : unicode text table with tabs as separator\n"
       "       <graph> : reference graph\n"
       "       <result> : name of the result main graph\n"
       "       [subgraph] : this optionnal parameter specifies the name of the\n"
       "                    subgraphs. Use \"@%%\" to insert the id (line number)\n"
       "                    to get unique names, e.g. \"sub_@%%.grf\".\n"
       "Applies a reference graph to a lexicon-grammar table, producing a sub-graph\n"
       "for each entry of the table.\n");
}


void table2grf(FILE*,FILE*,FILE*,char*,char*);


int main(int argc, char **argv) {
setBufferMode();

if (argc<4 || argc>5) {
   usage();
   return 0;
}
FILE* table=u_fopen(argv[1],U_READ);
if (table==NULL) {
   fatal_error("Cannot open table %s\n",argv[1]);
}
FILE* reference_graph=u_fopen(argv[2],U_READ);
if (reference_graph==NULL) {
   error("Cannot open reference graph %s\n",argv[2]);
   u_fclose(table);
   return 1;
}
if ((strlen(argv[3])-4)<=0) {
   error("Error in result graph name %s\n",argv[3]);
   u_fclose(table);
   return 1;
}
FILE* result_graph=u_fopen(argv[3],U_WRITE);
if (result_graph==NULL) {
   error("Cannot create result graph %s\n",argv[3]);
   u_fclose(table);
   u_fclose(reference_graph);
   return 1;
}
char subgraph[FILENAME_MAX];
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
char chemin[FILENAME_MAX];
get_path(argv[3],chemin);
table2grf(table,reference_graph,result_graph,subgraph,chemin);
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
u_fprintf(f,"#Unigraph\n"
               "SIZE 950 1328\n"
               "FONT Times New Roman:  12\n"
               "OFONT Times New Roman:B 12\n"
               "BCOLOR 16777215\n"
               "FCOLOR 0\n"
               "ACOLOR 12632256\n"
               "SCOLOR 16711680\n"
               "CCOLOR 255\n"
               "DBOXES y\n"
               "DFRAME y\n"
               "DDATE n\n"
               "DFILE y\n"
               "DDIR n\n"
               "DRIG n\n"
               "DRST n\n"
               "FITS 100\n"
               "PORIENT P\n"
               "#\n"
               "3\n"
               "\"<E>\" 68 368 1 2 \n"
               "\"\" 456 368 0 \n"
               "\"");
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
/*u_fgetc(f);
e->x=u_read_int(f);
e->y=u_read_int(f);
e->n_trans=u_read_int(f);*/
u_fscanf(f,"%d%d%d",&(e->x),&(e->y),&(e->n_trans));
for (i=0;i<e->n_trans;i++) {
  //e->trans[i]=u_read_int(f);
  u_fscanf(f,"%d",&(e->trans[i]));
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
//u_fgetc(f);
// we read the number of states
//g->n_etats=u_read_int(f);
u_fscanf(f,"%d\n",&(g->n_etats));
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
      if (row_number > n_champs)
        fatal_error("error: row #%d (@%c) not defined in table\n",
                    row_number,source[pos_in_src+1]);
   }
   else if (is_in_A_Z(source[pos_in_src+2]) && (source[pos_in_src+3]=='\0' || source[pos_in_src+3]=='/')) {
           // if we are in the case @AB or @AB/something
           row_number=(source[pos_in_src+1]-'A'+1)*(26)+(source[pos_in_src+2]-'A');
           if (row_number > n_champs)
             fatal_error("error: row #%d (@%c%c) not defined in table\n",
                         row_number,source[pos_in_src+1],source[pos_in_src+2]);
   }
   if (row_number!=-1) {
      // if we have a valid row reference
      if ((!negation && !u_strcmp(champ[row_number],"-"))
          || (negation && !u_strcmp(champ[row_number],"+"))) {
         // and if this reference points on a -,
         // then we must remove this state
         u_strcpy(dest,"");
         return;
      }
   }
}

// then, we can focus on the general case
u_sprintf(line_number,"%04d",ligne_courante);

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
            if (row_number > n_champs)
              fatal_error("Error: row #%d (@%c%c) not defined in table\n",
                          row_number,source[pos_in_src],source[pos_in_src+1]);
            pos_in_src++;
         }
         else {
           row_number=source[pos_in_src]-'A';
           if (row_number > n_champs)
             fatal_error("Error: row #%d (@%c) not defined in table\n",
                         row_number,source[pos_in_src]);
         }
         pos_in_src++;
         if (!u_strcmp(champ[row_number],"+")) {
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
         else if (!u_strcmp(champ[row_number],"-")) {
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
  }
  else i++;
}
return 1;
}



int nettoyer_graphe(struct graphe_patron *G) {
int i,n_etats,etat_courant,j;
int *t;

if (G->tab[0]->contenu==NULL) {
   fatal_error("internal error in nettoyer_graphe: NULL for initial state content\n");
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
                  char* nom_resultat,char* chemin,FILE *f_coord,int graphs_printed) {

char nom_res[FILENAME_MAX];
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

// we print the name of the subgraph
unichar tmp[FILENAME_MAX];
unichar tmp2[FILENAME_MAX];
u_strcpy(tmp,nom_resultat);
tmp2[0]='\0';
convertir(tmp2,tmp,ligne,n_champs,ligne_courante);
if (!nettoyer_graphe(res)) {
  // if the graph has been emptied, we return
  error("%S has been emptied\n",tmp2);
  return false;
} 
if (graphs_printed!=0) {
  /* print a "+" only if any subgraph already has been printed:
   *  - not in first line (ligne_courante==0)
   *  - not if only empty subgraphs have occured since */
  u_fprintf(f_coord,"+");
}
u_fprintf(f_coord,":");
{
char tmp3[FILENAME_MAX];
char tmp4[FILENAME_MAX];
get_path(nom_res,tmp3);
if ( ! strncmp(tmp3,chemin,strlen(chemin)) ) /* the subgraph is in a subdirectory
                                                relative to the path of the result graph:
                                                we strip the common path */
  {
    strcpy(tmp3,&nom_res[strlen(chemin)]);
  }
else /* we take the full name (including the path) */
  strcpy(tmp3,nom_res);

/* Now we have to replace '/' and '\\' in the path to ':' */
replace_path_separator_by_colon(tmp3);

/* And finally we remove the extension ".grf" */
remove_extension(tmp3,tmp4);

u_fprintf(f_coord,"%s",tmp4);
}

f=u_fopen(nom_res,U_WRITE);
if (f==NULL) {
  error("Cannot create subgraph %s\n",nom_res);
  return false;
}
if (ligne_courante%10==0) {
   u_printf("%d table entries done...    \r",ligne_courante);
}
u_fprintf(f,"%S\n",g->en_tete);
u_fprintf(f,"%d\n",res->n_etats);
for (i=0;i<res->n_etats;i++) {
  u_fprintf(f,"\"%S",res->tab[i]->contenu);
  u_fprintf(f,"\" %d %d %d",res->tab[i]->x,res->tab[i]->y,res->tab[i]->n_trans);
  for (j=0;j<res->tab[i]->n_trans;j++) {
     u_fprintf(f," %d",res->tab[i]->trans[j]);
  }
  u_fprintf(f," \n");
  free(res->tab[i]);
}
fclose(f);
return true;
}



void table2grf(FILE* table,FILE* reference_graph,FILE* result_graph,char* subgraph,char* chemin) {
int ligne_courante;
struct graphe_patron structure;
unichar* ligne[MAX_LINES_IN_TABLE];
int n_champs;
int i;
int graphs_printed;
write_result_graph_header(result_graph);
u_printf("Loading reference graph...\n");
charger_graphe_patron(reference_graph,&structure);
u_fclose(reference_graph);
u_printf("Reading lexicon-grammar table...\n");
read_table_first_line(table,&n_champs);
ligne_courante=1;
graphs_printed=0;
while (read_table_line(table,(unichar**)ligne,n_champs)) {
  if (create_graph(ligne_courante,ligne,n_champs,&structure,subgraph,chemin,result_graph,graphs_printed))
    graphs_printed++;
  ligne_courante++;
}
u_fclose(table);
for (i=0;i<structure.n_etats;i++) {
  free(structure.tab[i]);
}
u_fprintf(result_graph,"\" 216 368 1 1 \n");
u_fclose(result_graph);
u_printf("Done.                                             \n");
}

