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

#include "Fst2.h"
#include "Error.h"
#include "LocateConstants.h"

//---------------------------------------------------------------------------



//-------CHARGEMENT DU FST2------------------------

Etat_fst* graphe_fst2;
Fst2Tag* etiquette_fst2;
int *debut_graphe_fst2;
int *nombre_etats_par_grf;
unichar** nom_graphe;
struct variable_list* liste_des_variables;
int nombre_etats_fst2;
int nombre_graphes_fst2;
int nombre_etiquettes_fst2;
int etiquette_courante;
int nombre_etiquettes_de_depart;
int etat_courant;





struct variable_list* new_variable(unichar* n) {
struct variable_list* v;
v=(struct variable_list*)malloc(sizeof(struct variable_list));
v->name=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(n)));
u_strcpy(v->name,n);
v->start=-1;
v->end=-1;
v->suivant=NULL;
return v;
}


void free_variable(struct variable_list* v) {
if (v->name!=NULL) free(v->name);
free(v);
}


void liberer_liste_variables(struct variable_list* l) {
struct variable_list* tmp;
while (l!=NULL) {
      tmp=l;
      l=l->suivant;
      free_variable(tmp);
}
}



struct variable_list* ajouter_variable(unichar* name,struct variable_list* v) {
if (v==NULL) return new_variable(name);
if (!u_strcmp(v->name,name)) return v;
v->suivant=ajouter_variable(name,v->suivant);
return v;
}


struct variable_list* get_variable(unichar* name,struct variable_list* v) {
while (v!=NULL) {
      if (!u_strcmp(name,v->name)) return v;
      v=v->suivant;
}
return NULL;
}


void free_transition(struct transition_fst* t) {
struct transition_fst* tmp;
while (t!=NULL) {
    tmp=t;
    t=t->suivant;
    free(tmp);
}
}


void free_etat(Etat_fst e) {
free_transition(e->trans);
free(e);
}


void free_etiquette(Fst2Tag e) {
if (e->input!=NULL) free(e->input);
if (e->output!=NULL) free(e->output);
if (e->inflected!=NULL) free(e->inflected);
if (e->lemma!=NULL) free(e->lemma);
if (e->codes!=NULL) free(e->codes);

/* $CD$ begin */
if (e -> contentGF != NULL) free(e -> contentGF);
/* $CD$ end   */

free(e);
}


//
// returns a new empty automaton
//
Automate_fst2* new_Automate_fst2() {
Automate_fst2* a=(Automate_fst2*)malloc(sizeof(Automate_fst2));
a->etat=NULL;
a->etiquette=NULL;
a->nombre_graphes=0;
a->nombre_etats=0;
a->nombre_etiquettes=0;
a->debut_graphe_fst2=NULL;
a->nom_graphe=NULL;
a->nombre_etats_par_grf=NULL;
a->variables=NULL;
return a;
}


//
// free the autoamata memory
//
void free_fst2(Automate_fst2* a) {
int i;
for (i=0;i<a->nombre_etats;i++) {
  free_etat(a->etat[i]);
}
free(a->etat);
for (i=0;i<a->nombre_etiquettes;i++)
  free_etiquette(a->etiquette[i]);
free(a->etiquette);
if (a->nom_graphe!=NULL) {
   for (i=0;i<a->nombre_graphes;i++) {
     if (a->nom_graphe[i]!=NULL) free(a->nom_graphe[i]);
   }
   free(a->nom_graphe);
}
free(a->debut_graphe_fst2);
free(a->nombre_etats_par_grf);
liberer_liste_variables(a->variables);
free(a);
}


//
// readjust the size of arrays
//
void resize(Automate_fst2* a) {
  // +1 because of numeration does not start at 0 but 1
int n_etats=a->nombre_etats+1;
int n_graphes=a->nombre_graphes+1;
int n_etiq=a->nombre_etiquettes+1;
a->etat=(Etat_fst*)realloc(a->etat,n_etats*sizeof(Etat_fst));
a->etiquette=(Fst2Tag*)realloc(a->etiquette,n_etiq*sizeof(Fst2Tag));
// we count +1 because we start the graph numerotation at 1
a->debut_graphe_fst2=(int*)realloc(a->debut_graphe_fst2,(1+n_graphes)*sizeof(int));
if (a->nom_graphe!=NULL) {
   // we reallocate only if there is something
   a->nom_graphe=(unichar**)realloc(a->nom_graphe,n_graphes*sizeof(unichar*));
}
a->nombre_etats_par_grf=(int*)realloc(a->nombre_etats_par_grf,n_graphes*sizeof(int));
}


//
// cree et renvoie une nouvelle etiquette
//
Fst2Tag nouvelle_etiquette_mat() {
Fst2Tag e;
e=(Fst2Tag)malloc(sizeof(struct fst2Tag));
if (e==NULL) {
  fprintf(stderr,"Probleme d'allocation memoire dans la fonction nouvelle_etiquette_mat\n");
  exit(1);
}
e->number=0;
e->control=0;
e->input=NULL;
e->output=NULL;
e->inflected=NULL;
e->lemma=NULL;
e->codes=NULL;
e->matching_tokens=NULL;
e->number_of_matching_tokens=0;
e->compound_pattern=NO_COMPOUND_PATTERN;

/* $CD$ begin */
e -> contentGF = NULL;
e -> entryMasterGF = -1;
/* $CD$ end   */

return e;
}



//
// gere les etiquettes <...> (<MOT>, <avoir>, <avoir.V>, <eu,avoir.V>, <V>)
//
void decomposer_angles_etiquette_fst2(Fst2Tag e) {
unichar temp[1000];
int i,j;
j=0;
i=1;
if (e->input[i]=='!') {
  i++;
  e->control=(char)(e->control|NEGATION_TAG_BIT_MASK);
}
while ((e->input[i]!=',')&&(e->input[i]!='.')&&(e->input[i]!='>'))
  temp[j++]=e->input[i++];
temp[j]='\0';
// cas <avoir>, <V> ou <MOT>
if (e->input[i]=='>') {
  if (!u_strcmp_char(temp,"MOT")) {
    return;
  }
  if (!u_strcmp_char(temp,"DIC")) {
    return;
  }
  if (!u_strcmp_char(temp,"MAJ")) {
    return;
  }
  if (!u_strcmp_char(temp,"MIN")) {
    return;
  }
  if (!u_strcmp_char(temp,"PRE")) {
    return;
  }
  if (!u_strcmp_char(temp,"NB")) {
    return;
  }
  
  /* $CD$ begin */
  if (!u_strcmp_char(temp,"TOKEN")) {
    return;
  }
  /* $CD$ end   */
  

  // on a <avoir> ou <V>
  e->inflected=(unichar*)malloc(sizeof(unichar)*(j+1));
  if (e->inflected==NULL) {
    fprintf(stderr,"Probleme d'allocation memoire dans la fonction decomposer_angles_etiquettes_fst2\n");
    exit(1);
  }
  u_strcpy(e->inflected,temp);
  return;
}
// cas <eu,avoir.V>
if (e->input[i]==',') {
  e->inflected=(unichar*)malloc(sizeof(unichar)*(j+1));
  if (e->inflected==NULL) {
    fprintf(stderr,"Probleme d'allocation memoire dans la fonction decomposer_angles_etiquettes_fst2\n");
    exit(1);
  }
  u_strcpy(e->inflected,temp);
  i++;
  j=0;
  while ((e->input[i]!='.')&&(e->input[i]!='>'))
    temp[j++]=e->input[i++];
  temp[j]='\0';
  if (e->input[i]=='>') {
  	char err[1000];
  	u_to_char(err,e->input);
    fprintf(stderr,"Invalid label %s\n",err);
    exit(1);
  }
  e->lemma=(unichar*)malloc(sizeof(unichar)*(j+1));
  if (e->lemma==NULL) {
    fprintf(stderr,"Probleme d'allocation memoire dans la fonction decomposer_angles_etiquettes_fst2\n");
    exit(1);
  }
  u_strcpy(e->lemma,temp);
  i++;
  j=0;
  while (e->input[i]!='>')
    temp[j++]=e->input[i++];
  temp[j]='\0';
  e->codes=(unichar*)malloc(sizeof(unichar)*(j+1));
  if (e->codes==NULL) {
    fprintf(stderr,"Probleme d'allocation memoire dans la fonction decomposer_angles_etiquettes_fst2\n");
    exit(1);
  }
  u_strcpy(e->codes,temp);
  return;
}
// cas <avoir.N>
e->lemma=(unichar*)malloc(sizeof(unichar)*(j+1));
if (e->lemma==NULL) {
  fprintf(stderr,"Probleme d'allocation memoire dans la fonction decomposer_angles_etiquettes_fst2\n");
  exit(1);
}
u_strcpy(e->lemma,temp);
i++;
j=0;
while (e->input[i]!='>')
  temp[j++]=e->input[i++];
temp[j]='\0';
e->codes=(unichar*)malloc(sizeof(unichar)*(j+1));
if (e->codes==NULL) {
  fprintf(stderr,"Probleme d'allocation memoire dans la fonction decomposer_angles_etiquettes_fst2\n");
  exit(1);
}
u_strcpy(e->codes,temp);
return;
}



//
// gere les etiquettes  {...}
//
void decomposer_accolades_etiquette_fst2(Fst2Tag e) {
unichar temp[1000];
char err[1000];
int i,j;
j=0;
i=1;
while ((e->input[i]!=',')&&(e->input[i]!='}')) {
  temp[j++]=e->input[i++];
}
if (e->input[i]=='}') {
  u_to_char(err,e->input);
  fprintf(stderr,"Invalid label %s: a tag must contain a valid DELAF line like {today,today.ADV}\n",err);
  exit(1);
}
temp[j]='\0';
e->inflected=(unichar*)malloc(sizeof(unichar)*(j+1));
u_strcpy(e->inflected,temp);
i++;
if (e->input[i]=='.') {
   e->lemma=(unichar*)malloc(sizeof(unichar)*(j+1));
   u_strcpy(e->lemma,temp);
}
else {
   j=0;
   while ((e->input[i]!='.')&&(e->input[i]!='}')) {
      temp[j++]=e->input[i++];
   }
   if (e->input[i]=='}') {
      u_to_char(err,e->input);
      fprintf(stderr,"Invalid label %s: a tag must contain a valid DELAF line like {today,today.ADV}\n",err);
      exit(1);
   }
   temp[j]='\0';
   e->lemma=(unichar*)malloc(sizeof(unichar)*(j+1));
   u_strcpy(e->lemma,temp);
}
i++;
j=0;
while (e->input[i]!='}') {
   temp[j++]=e->input[i++];
}
temp[j]='\0';
e->codes=(unichar*)malloc(sizeof(unichar)*(j+1));
u_strcpy(e->codes,temp);
}


void creer_etiquette_variable(int position,unichar mot[]) {
int L=u_strlen(mot);
Fst2Tag e=nouvelle_etiquette_mat();
e->input=(unichar*)malloc(L*sizeof(unichar));
for (int i=1;i<L-1;i++)
    e->input[i-1]=mot[i];
e->input[L-2]='\0';
if (mot[L-1]=='(') e->control=START_VAR_TAG_BIT_MASK;
else e->control=END_VAR_TAG_BIT_MASK;
liste_des_variables=ajouter_variable(e->input,liste_des_variables);
etiquette_fst2[position]=e;
}



//
// insere une etiquette dans le tableau
//
/* $CD$ begin */
//void creer_etiquette_fst2(int position,unichar mot[],unichar transduction[],int respect_min_maj) {
void creer_etiquette_fst2(int position, 
                          unichar mot[], unichar contentGF[], unichar transduction[],
                          int respect_min_maj) {
/* $CD$ end   */

Fst2Tag e;
int L=u_strlen(mot);
if (mot[0]=='$' && L>2 && (mot[L-1]=='(' || mot[L-1]==')')) {
   // on est dans le cas d'une variable $a( ou $a)
   creer_etiquette_variable(position,mot);
   return;
}

e=nouvelle_etiquette_mat();
if (e==NULL) {
   printf("oops");
}
e->input=(unichar*)malloc((u_strlen(mot)+1)*sizeof(unichar));
if (e->input==NULL) {
  fprintf(stderr,"Probleme d'allocation memoire dans la fonction creer_etiquette_fst2\n");
  exit(1);
}
u_strcpy(e->input,mot);

//---------------
if (!u_strcmp_char(mot,"$[")) {
   e->control=POSITIVE_CONTEXT_MASK;
   etiquette_fst2[position]=e;
   return;
}
if (!u_strcmp_char(mot,"$![")) {
   e->control=NEGATIVE_CONTEXT_MASK;
   etiquette_fst2[position]=e;
   return;
}
if (!u_strcmp_char(mot,"$]")) {
   e->control=CONTEXT_END_MASK;
   etiquette_fst2[position]=e;
   return;
}
//---------------

/* $CD$ begin */
if (u_strlen(contentGF) > 0) {
    e -> contentGF = (unichar *) malloc( (u_strlen(contentGF) + 1) * sizeof(unichar) );
    if (e -> contentGF == NULL) {
        fprintf(stderr,"Probleme d'allocation memoire dans la fonction creer_etiquette_fst2\n");
        exit(1);
        }       
    u_strcpy(e -> contentGF, contentGF);
    }
/* $CD$ end   */

e->output=(unichar*)malloc((u_strlen(transduction)+1)*sizeof(unichar));
if (e->output==NULL) {
  fprintf(stderr,"Probleme d'allocation memoire dans la fonction creer_etiquette_fst2\n");
  exit(1);
}
u_strcpy(e->output,transduction);
if (transduction[0]!=0)
  e->control=(unsigned char)(e->control|TRANSDUCTION_TAG_BIT_MASK);
if (respect_min_maj)
  e->control=(unsigned char)(e->control|RESPECT_CASE_TAG_BIT_MASK);

//--- on determine e->type
if ((mot[0]==' ')||(mot[0]=='#')||!u_strcmp_char(mot,"<E>")) {
  etiquette_fst2[position]=e;
  return;
}

if (mot[0]=='{') {
  if (!u_strcmp_char(mot,"{S}")) {
    // cas particulier du delimiteur de phrase {S}
    // NOTE: the {STOP} tag MUST NOT be handled here, since
    //       we do not want it to be matched
  }
  else {
    // on est soit dans le cas {....} soit dans le cas d'un char { seul
    if (mot[1]!='\0') {
       decomposer_accolades_etiquette_fst2(e);
    }
  }
  etiquette_fst2[position]=e;
  return;
}
// on est ou dans le cas <...> ou dans le cas lexical
if ((mot[0]!='<') || (mot[1]=='\0')) {
  etiquette_fst2[position]=e;
  return;
}
// on est forcement dans le cas <...>
decomposer_angles_etiquette_fst2(e);
etiquette_fst2[position]=e;
}


//
// ajoute les etiquettes correspondante a s
//
void ajouter_etiquette_fst2(unichar s[])
{
  unichar mot[500],transd[500];
  int i=1,j=0,k=0;
  int respect_min_maj;
  respect_min_maj=(s[0]=='@');
if ((!u_strcmp_char(s,"@/"))||(!u_strcmp_char(s,"%/"))) {
  mot[0]='/';
  mot[1]='\0';
  transd[0]='\0';
} else {
    mot[k++]=s[i++];
  while(s[i]!='\0' && !(s[i]=='/' && i>0 && s[i-1]!='\\'))
    mot[k++]=s[i++];
  mot[k]='\0';
  if(s[i]=='/')
    {
      i++;
      while(s[i]!='\0')
	{
	transd[j++]=s[i++];
	}
    }
  transd[j]='\0';
  }


/* $CD$ begin */  
//creer_etiquette_fst2(etiquette_courante,mot,transd,respect_min_maj);
unichar transitionContent[500], filterContent[500];

transitionContent[0] = '\0';
filterContent[0] = '\0';
i = 0; j = 0;

while ( mot[i] != '\0' && (mot[i] != '<' || mot[i+1] != '<') )
    transitionContent[j++] = mot[i++];
transitionContent[j] = '\0';
    
if (mot[i] != '\0') {
    j = 0;
    while ( mot[i] != '\0' && (mot[i] != '>' || mot[i+1] != '>') )
        filterContent[j++] = mot[i++];
    filterContent[j++] = '>'; filterContent[j++] = '>';
    if (mot[i] != '\0') {
        i += 2;
        if (mot[i] == '_') {
            do {
                filterContent[j++] = mot[i++];
                } while (mot[i] != '\0' && mot[i] != '_');
            filterContent[j++] = '_';
            }
        }
    filterContent[j] = '\0';
    }
    
if (transitionContent[0] == '\0') u_strcpy_char(transitionContent, "<TOKEN>");

creer_etiquette_fst2(etiquette_courante, 
                     transitionContent, filterContent, transd, respect_min_maj);
/* $CD$ end   */

etiquette_courante++;
}



//
// lit les etiquettes des transitions
//
void lire_etiquettes_fst2(FILE *f) {
int i;
unichar c;
unichar mot[10000];
while (((c=(unichar)u_fgetc(f))!='%')&&(c!='@'));
while (c!='f') {
  i=0;
  do {
    mot[i++]=c;
  } while ((c=(unichar)u_fgetc(f))!='\n');
  while (((c=(unichar)u_fgetc(f))!='f')&&(c!='%')&&(c!='@'));
  mot[i]='\0';
  ajouter_etiquette_fst2(mot);
}
nombre_etiquettes_de_depart=etiquette_courante;
nombre_etiquettes_fst2=etiquette_courante;
}



//
// lit les etiquettes des transitions jusqu'a l'etiquette d'indice ETIQ_MAX
//
void lire_etiquettes_fst2_under_limit(FILE *f,int ETIQ_MAX) {
int i;
unichar c;
unichar mot[10000];
int k=0;
while (((c=(unichar)u_fgetc(f))!='%')&&(c!='@'));
while (c!='f' && k<=ETIQ_MAX) {
  i=0;
  k++;
  do {
    mot[i++]=c;
  } while ((c=(unichar)u_fgetc(f))!='\n');
  while (((c=(unichar)u_fgetc(f))!='f')&&(c!='%')&&(c!='@')) {}
  mot[i]='\0';
  ajouter_etiquette_fst2(mot);
}
nombre_etiquettes_de_depart=etiquette_courante;
nombre_etiquettes_fst2=etiquette_courante;
}


//
// initialise le graphe_fst2
//
void initialiser_graphe_fst2() {
long int i;
for (i=0;i<MAX_FST2_STATES;i++)
  graphe_fst2[i]=NULL;
}

//
// initialise les etiquettes
//
void initialiser_etiquettes() {
long int i;
for (i=0;i<MAX_FST2_TAGS;i++)
  etiquette_fst2[i]=NULL;
}

//
// initialise les variables utilisees pour charger le FST2
//
void initialiser_variables_fst2(){
nombre_etiquettes_de_depart=0;
etiquette_courante=0;
nombre_etiquettes_fst2=0;
nombre_etats_fst2=0;
etiquette_courante=0;
etat_courant=0;
initialiser_graphe_fst2();
initialiser_etiquettes();
}


//
// cree et renvoie un etat vierge
//
Etat_fst nouvel_etat() {
Etat_fst e;
e=(Etat_fst)malloc(sizeof(struct etat_fst));
if (e==NULL) {
  fprintf(stderr,"Probleme d'allocation memoire dans la fonction nouvel_etat\n");
  exit(1);
}
e->controle=0;
e->trans=NULL;
return e;
}

//
// lit un entier relatif dans le fichier; si une fin de ligne est trouvee, ok vaut 0
//
int lire_entier_fst2(FILE *f,int *ok) {
unichar c;
int res,negatif;
do
  c=(unichar)u_fgetc(f);
while (((c<'0')||(c>'9'))&&(c!='-')&&(c!='\n'));
if (c=='\n') {
  // si fin de ligne, on arrete et ok vaut 0
  *ok=0;
  return 0;
}
*ok=1;
if (c=='-') {
  // on lit un nombre negatif
  negatif=1;
  c=(unichar)u_fgetc(f);
} else negatif=0;
res=c-'0';
while(((c=(unichar)u_fgetc(f))>='0')&&(c<='9')) {
  res=res*10+(c-'0');
}
if (negatif) res=-res;
return res;
}


//
// cree et renvoie une nouvelle transition
//
liste_transition nouvelle_transition_mat() {
liste_transition t;
t=(liste_transition)malloc(sizeof(struct transition_fst));
if (t==NULL) {
  fprintf(stderr,"Probleme d'allocation memoire dans la fonction nouvelle_transition_mat\n");
  exit(1);
}
t->etiquette=-1;
t->arr=-1;
t->suivant=NULL;
return t;
}



//
// ajoute une transition a l'etat courant
//
void ajouter_transition_mat(struct etat_fst *e,int etiq,int etarr)
{
  struct transition_fst *ptr;

  ptr=nouvelle_transition_mat();
  ptr->suivant=e->trans;
  ptr->etiquette=etiq;
  ptr->arr=etarr;
  e->trans=ptr;
}


//
// lit les etats puis les ajoute a l'automate
//
void lire_etats_fst2(FILE *f) {
unichar c;
int i,j,ok;
int imot,etarr,graphe_courant;
int etat_relatif;
debut_graphe_fst2=(int*)malloc((nombre_graphes_fst2+1)*sizeof(int));
nombre_etats_par_grf=(int*)malloc((nombre_graphes_fst2+1)*sizeof(int));
if (debut_graphe_fst2==NULL) {
  fprintf(stderr,"Probleme d'allocation memoire dans la fonction lire_etats_fst2\n");
  exit(1);
}
for (i=0;i<nombre_graphes_fst2+1;i++) {
  // j'ai ajoute +1 car les graphes sont numerotes de -1 a -nombre_graphes
  debut_graphe_fst2[i]=0;
  nombre_etats_par_grf[i]=0;
}
etat_courant=0;
// on lit tous les graphes
for (j=0;j<nombre_graphes_fst2;j++) {
  // on lit le -
  u_fgetc(f);
  // et le numero
  graphe_courant=u_read_int(f);
  debut_graphe_fst2[graphe_courant]=etat_courant;
  etat_relatif=0;
  // on saute le nom du graphe
  while ((c=(unichar)u_fgetc(f))!='\n');
  // on se place sur "t" ou ":" ("f" serait une erreur)
  do
    c=(unichar)u_fgetc(f);
  while ((c!='t')&&(c!=':'));
  // on lit les etats du graphe
  while (c!='f') {
    graphe_fst2[etat_courant]=nouvel_etat();
    nombre_etats_par_grf[graphe_courant]++;
    if(c=='t') graphe_fst2[etat_courant]->controle=1;
    if (etat_relatif==0) graphe_fst2[etat_courant]->controle=(unsigned char)((graphe_fst2[etat_courant]->controle)|2);
    imot=lire_entier_fst2(f,&ok);
    while (ok) {
	  etarr=lire_entier_fst2(f,&ok);
      // etarr est un numero relatif; on calcule sa position reelle dans
      // le tableau des etats
      etarr=etarr+debut_graphe_fst2[graphe_courant];
	  ajouter_transition_mat(graphe_fst2[etat_courant],imot,etarr);
      imot=lire_entier_fst2(f,&ok);
	}
    while(((c=(unichar)u_fgetc(f))!=':')&&(c!='t')&&(c!='f'));
    etat_courant++;
    etat_relatif++;
  }
  // on lit l'espace et le \n apres le f
  u_fgetc(f);
  u_fgetc(f);

}
nombre_etats_fst2=etat_courant;
}


//
// lit les etats puis les ajoute a l'automate
// RECUPERE EN PLUS LE NOM DE CHAQUE GRAPHE
//
void lire_etats_fst2_avec_noms(FILE *f) {
unichar c;
int i,j,ok;
int imot,etarr,graphe_courant;
int etat_relatif;
unichar temp[10000];
int tmp;
debut_graphe_fst2=(int*)malloc((nombre_graphes_fst2+1)*sizeof(int));
nombre_etats_par_grf=(int*)malloc((nombre_graphes_fst2+1)*sizeof(int));
nom_graphe=(unichar**)malloc((nombre_graphes_fst2+1)*sizeof(unichar*));
if (debut_graphe_fst2==NULL || (nom_graphe)==NULL) {
  fprintf(stderr,"Probleme d'allocation memoire dans la fonction lire_etats_fst2_avec_noms\n");
  exit(1);
}
for (i=0;i<nombre_graphes_fst2+1;i++) {
  // j'ai ajoute +1 car les graphes sont numerotes de -1 a -nombre_graphes
  debut_graphe_fst2[i]=0;
  nombre_etats_par_grf[i]=0;
  nom_graphe[i]=NULL;
}
etat_courant=0;
// on lit tous les graphes
for (j=0;j<nombre_graphes_fst2;j++) {
  // on lit le -
  u_fgetc(f);
  // et le numero
  graphe_courant=u_read_int(f);
  debut_graphe_fst2[graphe_courant]=etat_courant;
  etat_relatif=0;
  // on lit le nom du graphe
  tmp=0;
  while ((c=(unichar)u_fgetc(f))!='\n')
    temp[tmp++]=c;
  temp[tmp]='\0';
  nom_graphe[graphe_courant]=(unichar*)malloc(sizeof(unichar)*(tmp+1));
  u_strcpy(nom_graphe[graphe_courant],temp);

  // on se place sur "t" ou ":" ("f" serait une erreur)
  do
    c=(unichar)u_fgetc(f);
  while ((c!='t')&&(c!=':')&&(c!='f'));
  // on lit les etats du graphe
  while (c!='f') {
    graphe_fst2[etat_courant]=nouvel_etat();
    nombre_etats_par_grf[graphe_courant]++;
    if(c=='t') graphe_fst2[etat_courant]->controle=1;
    if (etat_relatif==0) graphe_fst2[etat_courant]->controle=(unsigned char)((graphe_fst2[etat_courant]->controle)|2);
    imot=lire_entier_fst2(f,&ok);
    while (ok) {
      etarr=lire_entier_fst2(f,&ok);
      // etarr est un numero relatif; on calcule sa position reelle dans
      // le tableau des etats
      etarr=etarr+debut_graphe_fst2[graphe_courant];
	  ajouter_transition_mat(graphe_fst2[etat_courant],imot,etarr);
      imot=lire_entier_fst2(f,&ok);
	}
    while(((c=(unichar)u_fgetc(f))!=':')&&(c!='t')&&(c!='f'));
    etat_courant++;
    etat_relatif++;
  }
  // on lit l'espace et le \n apres le f
  u_fgetc(f);
  u_fgetc(f);
}
nombre_etats_fst2=etat_courant;
}



//
// lit les etats puis les ajoute a l'automate
// RECUPERE EN PLUS LE NOM DE CHAQUE GRAPHE
//
void lire_etats_fst2_avec_noms_for_one_sentence(FILE *f,int SENTENCE,int* ETIQ_MAX,FILE* txt) {
unichar c;
int i,j,ok;
int imot,etarr,graphe_courant;
int etat_relatif;
unichar temp[100000];
int tmp;
(*ETIQ_MAX)=0;
debut_graphe_fst2=(int*)malloc((nombre_graphes_fst2+1)*sizeof(int));
nombre_etats_par_grf=(int*)malloc((nombre_graphes_fst2+1)*sizeof(int));
nom_graphe=(unichar**)malloc((nombre_graphes_fst2+1)*sizeof(unichar*));
if (debut_graphe_fst2==NULL || (nom_graphe)==NULL) {
  fprintf(stderr,"Probleme d'allocation memoire dans la fonction lire_etats_fst2_avec_noms\n");
  exit(1);
}
for (i=0;i<nombre_graphes_fst2+1;i++) {
  // j'ai ajoute +1 car les graphes sont numerotes de -1 a -nombre_graphes
  debut_graphe_fst2[i]=0;
  nombre_etats_par_grf[i]=0;
  nom_graphe[i]=NULL;
}
etat_courant=0;
// on lit tous les graphes
for (j=0;j<nombre_graphes_fst2;j++) {
  // on lit le -
  u_fgetc(f);
  // et le numero
  graphe_courant=u_read_int(f);
  debut_graphe_fst2[graphe_courant]=etat_courant;
  etat_relatif=0;
  // on lit le nom du graphe
  tmp=0;
  while ((c=(unichar)u_fgetc(f))!='\n')
    temp[tmp++]=c;
  temp[tmp]='\0';
  if ((j+1==SENTENCE)) {
     u_fprints(temp,txt);
     u_fprints_char("\n",txt);
     // on ne construit le graphe que si on est sur le bon indice
     // on se place sur "t" ou ":" ("f" serait une erreur)
     do
       c=(unichar)u_fgetc(f);
     while ((c!='t')&&(c!=':')&&(c!='f'));
     // on lit les etats du graphe
     while (c!='f') {
       graphe_fst2[etat_courant]=nouvel_etat();
       nombre_etats_par_grf[graphe_courant]++;
       if(c=='t') graphe_fst2[etat_courant]->controle=1;
       if (etat_relatif==0) graphe_fst2[etat_courant]->controle=(unsigned char)((graphe_fst2[etat_courant]->controle)|2);
       imot=lire_entier_fst2(f,&ok);
       while (ok) {
         if (imot>(*ETIQ_MAX)) (*ETIQ_MAX)=imot;
         etarr=lire_entier_fst2(f,&ok);
         // etarr est un numero relatif; on calcule sa position reelle dans
         // le tableau des etats
         etarr=etarr+debut_graphe_fst2[graphe_courant];
	     ajouter_transition_mat(graphe_fst2[etat_courant],imot,etarr);
         imot=lire_entier_fst2(f,&ok);
       }
       while(((c=(unichar)u_fgetc(f))!=':')&&(c!='t')&&(c!='f'));
       etat_courant++;
       etat_relatif++;
     }
  }
  else {
     while(((c=(unichar)u_fgetc(f))!='f'));
  }
  // on lit l'espace et le \n apres le f
  u_fgetc(f);
  u_fgetc(f);
}
nombre_etats_fst2=etat_courant;
}



//
// charge le FST2
//
void charger_graphe_fst2(FILE *f,Etat_fst graphe_fst2_[],Fst2Tag etiquette_[],
                         int *nombre_graphe_fst2s_,int *nombre_etats_,int *nombre_etiquettes_,
                         int **debut_graphe_fst2_,unichar ***nom_graphe_,int noms,
                         int **nombre_etats_par_grf_) {
nombre_graphes_fst2=u_read_int(f);
if (nombre_graphes_fst2==0) {
   fprintf(stderr,"Error: empty FST2\n");
   return;
}
graphe_fst2=graphe_fst2_;
etiquette_fst2=etiquette_;
debut_graphe_fst2=*debut_graphe_fst2_;
initialiser_variables_fst2();
nombre_etats_par_grf=*nombre_etats_par_grf_;
if (noms) {
   nom_graphe=*nom_graphe_;
   lire_etats_fst2_avec_noms(f);
   *nom_graphe_=nom_graphe;
}
else {
   lire_etats_fst2(f);
}
lire_etiquettes_fst2(f);
*nombre_etats_par_grf_=nombre_etats_par_grf;
*nombre_graphe_fst2s_=nombre_graphes_fst2;
*nombre_etats_=nombre_etats_fst2;
*nombre_etiquettes_=nombre_etiquettes_fst2;
*debut_graphe_fst2_=debut_graphe_fst2;
}






//
// loads an fst2 and returns its representation in an Automate_fst2 structure
//
Automate_fst2* load_fst2(char *file,int noms) {
FILE *f;
Automate_fst2* a=new_Automate_fst2();
f=u_fopen(file,U_READ);
if (f==NULL) {
  fprintf(stderr,"Cannot open the file %s\n",file);
  return NULL;
}
nombre_graphes_fst2=u_read_int(f);
if (nombre_graphes_fst2==0) {
   fprintf(stderr,"Graph %s is empty\n",file);
   return NULL;
}
a->etat=(Etat_fst*)malloc(MAX_FST2_STATES*sizeof(Etat_fst));
a->etiquette=(Fst2Tag*)malloc(MAX_FST2_TAGS*sizeof(Fst2Tag));
graphe_fst2=a->etat;
etiquette_fst2=a->etiquette;
debut_graphe_fst2=a->debut_graphe_fst2;
liste_des_variables=a->variables;
initialiser_variables_fst2();
nombre_etats_par_grf=a->nombre_etats_par_grf;
if (noms) {
   nom_graphe=a->nom_graphe;
   lire_etats_fst2_avec_noms(f);
   a->nom_graphe=nom_graphe;
}
else {
   lire_etats_fst2(f);
}
a->nombre_etats_par_grf=nombre_etats_par_grf;
lire_etiquettes_fst2(f);
u_fclose(f);
a->nombre_graphes=nombre_graphes_fst2;
a->nombre_etats=nombre_etats_fst2;
a->nombre_etiquettes=nombre_etiquettes_fst2;
a->debut_graphe_fst2=debut_graphe_fst2;
a->variables=liste_des_variables;
resize(a);
return a;
}



//
// loads one sentence of an fst2 and returns its representation in an Automate_fst2 structure
//
Automate_fst2* load_one_sentence_of_fst2(char *file,int SENTENCE,FILE* txt) {
FILE *f;
int ETIQ_MAX;
Automate_fst2* a=new_Automate_fst2();
f=u_fopen(file,U_READ);
if (f==NULL) {
  fprintf(stderr,"Cannot open the file %s\n",file);
  return NULL;
}
nombre_graphes_fst2=u_read_int(f);
if (nombre_graphes_fst2==0) {
   fprintf(stderr,"Graph %s is empty\n",file);
   return NULL;
}
a->etat=(Etat_fst*)malloc(MAX_FST2_STATES*sizeof(Etat_fst));
a->etiquette=(Fst2Tag*)malloc(MAX_FST2_TAGS*sizeof(Fst2Tag));
graphe_fst2=a->etat;
etiquette_fst2=a->etiquette;
debut_graphe_fst2=a->debut_graphe_fst2;
liste_des_variables=a->variables;
initialiser_variables_fst2();
nombre_etats_par_grf=a->nombre_etats_par_grf;
nom_graphe=a->nom_graphe;
lire_etats_fst2_avec_noms_for_one_sentence(f,SENTENCE,&ETIQ_MAX,txt);
a->nombre_etats_par_grf=nombre_etats_par_grf;
lire_etiquettes_fst2_under_limit(f,ETIQ_MAX);
u_fclose(f);
a->nom_graphe=nom_graphe;
a->nombre_graphes=nombre_graphes_fst2;
a->nombre_etats=nombre_etats_fst2;
a->nombre_etiquettes=nombre_etiquettes_fst2;
a->debut_graphe_fst2=debut_graphe_fst2;
a->variables=liste_des_variables;
resize(a);
return a;
}



int is_final_state(Etat_fst e) {
if (e==NULL) {
   fatal_error("NULL error in is_final_state\n");
}
return e->controle&1;
}




void unprotect_characters_in_sequence(unichar* s) {
int new_cursor=0;
int old_cursor=0;
while (s[old_cursor]!='\0') {
   if (s[old_cursor]=='\\') {old_cursor++;}
   if (s[old_cursor]=='\0') {
      fprintf(stderr,"ERROR: unprotected slash at the end of a sequence in unprotect_characters_in_sequence\n");
   }
   s[new_cursor++]=s[old_cursor++];
}
s[new_cursor]='\0';
}


//
// This function unprotects characters in non NULL flechi and canonique members
// of any tag of the given fst2
//
void unprotect_characters_in_fst2_tags(Automate_fst2* fst2) {
Fst2Tag etiq;
for (int i=0;i<fst2->nombre_etiquettes;i++) {
   etiq=fst2->etiquette[i];
   if (etiq!=NULL) {
      if (etiq->inflected!=NULL) {unprotect_characters_in_sequence(etiq->inflected);}
      if (etiq->lemma!=NULL) {unprotect_characters_in_sequence(etiq->lemma);}
   }
}
}
