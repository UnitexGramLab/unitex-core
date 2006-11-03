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
#include "Error.h"
#include "LemmaTree.h"
//---------------------------------------------------------------------------






/**
 * Allocates, initializes and returns a new locate_paramters structure.
 */
struct locate_parameters* new_locate_parameters() {
struct locate_parameters* p=(struct locate_parameters*)malloc(sizeof(struct locate_parameters));
if (p==NULL) {
   fatal_error("Not enough memory in new_locate_parameters\n");
}
p->token_controle=NULL;
p->matching_patterns=NULL;
p->pattern_compose_courant=0;
p->racine_code_gramm=NULL;
/* We used -1 because there may be no space in the text */
p->ESPACE=-1;
p->tag_token_list=NULL;
#ifdef TRE_WCHAR
p->masterGF=NULL;
p->indexGF=NULL;
#endif
p->DLC_tree=NULL;
return p;
}


/**
 * Frees a locate_parameters structure.
 */
void free_locate_parameters(struct locate_parameters* p) {
if (p==NULL) return;
free(p);
}


int locate_pattern(char* text,char* tokens,char* fst2,char* dlf,char* dlc,char* err,
                   char* alphabet,int mode,int output_mode, char* dynamicDir,
                   int tokenization_mode) {
struct locate_parameters* parameters=new_locate_parameters();

FILE* text_file;
FILE* out;
FILE* info;
long int text_size=u_file_size(text)/4;
text_file=fopen(text,"rb");
if (text_file==NULL) {
   error("Cannot load %s\n",text);
   return 0;
}
char concord[1000];
char concord_info[1000];

strcpy(concord,dynamicDir);
strcat(concord,"concord.ind");

strcpy(concord_info,dynamicDir);
strcat(concord_info,"concord.n");

out=u_fopen(concord,U_WRITE);
if (out==NULL) {
   error("Cannot write %s\n",concord);
   fclose(text_file);
   return 0;
}
info=u_fopen(concord_info,U_WRITE);
if (info==NULL) {
   error("Cannot write %s\n",concord_info);
}
switch(output_mode) {
   case IGNORE_TRANSDUCTIONS: u_fprints_char("#I\n",out); break;
   case MERGE_TRANSDUCTIONS: u_fprints_char("#M\n",out); break;
   case REPLACE_TRANSDUCTIONS: u_fprints_char("#R\n",out); break;
}
printf("Loading alphabet...\n");
Alphabet* alph=load_alphabet(alphabet);
if (alph==NULL) {
   error("Cannot load alphabet file %s\n",alphabet);
   return 0;
}
struct string_hash* semantic_codes=new_string_hash();
extract_semantic_codes(dlf,semantic_codes);
extract_semantic_codes(dlc,semantic_codes);
printf("Loading fst2...\n");
Fst2* automate=load_fst2(fst2,1);
if (automate==NULL) {
   error("Cannot load grammar %s\n",fst2);
   free_alphabet(alph);
   free_string_hash(semantic_codes);
   return 0;
}
/* $CD$ begin */
#ifdef TRE_WCHAR
parameters->masterGF=CreateMasterGF(automate,alph);
if (parameters->masterGF==NULL) {
   error("Cannot compile filter(s)\n");
   free_alphabet(alph);
   free_string_hash(semantic_codes);
   return 0;
}
#endif
/* $CD$ end   */

printf("Loading token list...\n");
struct string_hash* tok=load_text_tokens_hash(tokens,&SENTENCE_DELIMITER_INDICE,&STOP_MARKER_INDICE);
if (tok==NULL) {
   error("Cannot load token list %s\n",tokens);
   free_alphabet(alph);
   free_string_hash(semantic_codes);
   free_Fst2(automate);
   return 0;
}

/* $CD$ begin */
#ifdef TRE_WCHAR
parameters->indexGF=CreateIndexGF(parameters->masterGF,tok);
if (parameters->indexGF==NULL) {
   error("Cannot optimize filter(s)\n");
   free_alphabet(alph);
   free_string_hash(semantic_codes);
   free_string_hash(tok);
   free_Fst2(automate);
   return 0;
}
#endif
/* $CD$ end   */

extract_semantic_codes_from_tokens(tok,semantic_codes);
parameters->token_controle=(unsigned char*)malloc(NUMBER_OF_TEXT_TOKENS*sizeof(unsigned char));
if (parameters->token_controle==NULL) {
   fatal_error("Not enough memory in locate_pattern\n");
}
parameters->matching_patterns=(struct bit_array**)malloc(NUMBER_OF_TEXT_TOKENS*sizeof(struct bit_array*));
if (parameters->matching_patterns==NULL) {
   fatal_error("Not enough memory in locate_pattern\n");
}
for (int i=0;i<NUMBER_OF_TEXT_TOKENS;i++) {
  parameters->token_controle[i]=0;
  parameters->matching_patterns[i]=NULL;
}
compute_token_controls(tok,alph,err,tokenization_mode,parameters);
int nombre_patterns=0;
parameters->racine_code_gramm=nouveau_noeud_code_gramm();
int existe_etiquette_DIC=0;
int existe_etiquette_CDIC=0;
int existe_etiquette_SDIC=0;
printf("Computing fst2 tags...\n");
numerote_tags(automate,tok,&nombre_patterns,semantic_codes,alph,&existe_etiquette_DIC,
              &existe_etiquette_CDIC,&existe_etiquette_SDIC,tokenization_mode,parameters);

// on calcule l'espace necessaire pour stocker un code grammatical
//
#warning ???? Verifier que ca marche meme si on augmente le nombre de patterns avec les composes
//int n_octet_code_gramm=((nombre_patterns+1)/8)+1;
//
// ????
//

// on demarre la numerotation des patterns composes apres celle des patterns
// simples

parameters->pattern_compose_courant=nombre_patterns+1;

parameters->DLC_tree=new_DLC_tree(tok->N);
struct lemma_node* root=new_lemma_node();
printf("Loading dlf...\n");
load_dic_for_locate(dlf,alph,tok,nombre_patterns,existe_etiquette_DIC,existe_etiquette_CDIC,
				existe_etiquette_SDIC,tokenization_mode,root,parameters);
printf("Loading dlc...\n");
load_dic_for_locate(dlc,alph,tok,nombre_patterns,existe_etiquette_DIC,existe_etiquette_CDIC,
				existe_etiquette_SDIC,tokenization_mode,root,parameters);
// we look if the tag tokens like {today,.ADV} verify some patterns

check_patterns_for_tag_tokens(alph,tok,nombre_patterns,tokenization_mode,root,parameters);

printf("Optimizing fst2 tags...\n");
replace_pattern_tags(automate,alph,tok,tokenization_mode,root,parameters);
printf("Optimizing compound word dictionary...\n");
optimize_DLC(parameters->DLC_tree);
free_string_hash(semantic_codes);
init_transduction_variable_index(automate->variables);
printf("Optimizing fst2...\n");
optimize_fst2(automate);
printf("Optimizing patterns...\n");
init_pattern_transitions(tok);
convert_pattern_lists(tok);
printf("Working...\n");
launch_locate(text_file,automate,mode,tok,out,output_mode,text_size,info,parameters);
free_transduction_variable_index();
fclose(text_file);
if (info!=NULL) u_fclose(info);
u_fclose(out);
printf("Freeing memory...\n");
free_DLC_tree(parameters->DLC_tree);
free_Fst2(automate);
free_alphabet(alph);
free_string_hash(tok);
free_list_int(parameters->tag_token_list);
free_lemma_node(root);
free(parameters->token_controle);
for (int i=0;i<NUMBER_OF_TEXT_TOKENS;i++) {
   free_bit_array(parameters->matching_patterns[i]);
}
free(parameters->matching_patterns);
/* $CD$ begin */
#ifdef TRE_WCHAR
FreeMasterGF(parameters->masterGF,0);
FreeIndexGF(parameters->indexGF,0);
#endif
/* $CD$ end   */
free_locate_parameters(parameters);
printf("Done.\n");
return 1;
}



void numerote_tags(Fst2* fst2,struct string_hash* tok,int* nombre_patterns,
                   struct string_hash* semantic_codes,Alphabet* alph,
                   int* existe_etiquette_DIC,int* existe_etiquette_CDIC,
                   int* existe_etiquette_SDIC,int tokenization_mode,
                   struct locate_parameters* parameters) {
int i,j,k;
unichar tmp[TAILLE_MOT];
unichar flechi[TAILLE_MOT];
unichar canonique[TAILLE_MOT];
unichar pattern[TAILLE_MOT];
Fst2Tag* etiquette=fst2->tags;
unichar t[2];
t[0]=' ';
t[1]='\0';
parameters->ESPACE=get_token_number(t,tok);
for (i=0;i<fst2->number_of_tags;i++) {
  if (etiquette[i]->control&START_VAR_TAG_BIT_MASK) {
     // case of $a(
     etiquette[i]->number=VAR_START;
     etiquette[i]->control=(unsigned char)((etiquette[i]->control-START_VAR_TAG_BIT_MASK)|CONTROL_TAG_BIT_MASK);
  }
  else
  if (etiquette[i]->control&END_VAR_TAG_BIT_MASK) {
     // case of $a)
     etiquette[i]->number=VAR_END;
     etiquette[i]->control=(unsigned char)((etiquette[i]->control-END_VAR_TAG_BIT_MASK)|CONTROL_TAG_BIT_MASK);
  }
  else
  if (etiquette[i]->control&POSITIVE_CONTEXT_MASK) {
     // case of $[
     etiquette[i]->number=POSITIVE_CONTEXT_MARK;
     etiquette[i]->control=(unsigned char)((etiquette[i]->control-POSITIVE_CONTEXT_MASK)|CONTROL_TAG_BIT_MASK);
  }
  else
  if (etiquette[i]->control&NEGATIVE_CONTEXT_MASK) {
     // case of $![
     etiquette[i]->number=NEGATIVE_CONTEXT_MARK;
     etiquette[i]->control=(unsigned char)((etiquette[i]->control-NEGATIVE_CONTEXT_MASK)|CONTROL_TAG_BIT_MASK);
  }
  else
  if (etiquette[i]->control&CONTEXT_END_MASK) {
     // case of $]
     etiquette[i]->number=CONTEXT_END_MARK;
     etiquette[i]->control=(unsigned char)((etiquette[i]->control-CONTEXT_END_MASK)|CONTROL_TAG_BIT_MASK);
  }
  else
  if (!u_strcmp_char(etiquette[i]->input,"#")) {
    if (etiquette[i]->control&RESPECT_CASE_TAG_BIT_MASK) {
       // on est dans le cas @#: # doit etre considere comme un token normal
       etiquette[i]->number=get_hash_number(etiquette[i]->input,tok);
       parameters->token_controle[tok->N]=get_controle(etiquette[i]->input,alph,NULL,tokenization_mode);
       etiquette[i]->control=(unsigned char)(etiquette[i]->control|TOKEN_TAG_BIT_MASK);
    }
    else {
       etiquette[i]->number=DIESE;
       etiquette[i]->control=(unsigned char)(etiquette[i]->control|CONTROL_TAG_BIT_MASK);
    }
  }
  else
  if (!u_strcmp_char(etiquette[i]->input,"<E>")) {
     etiquette[i]->number=EPSILON;
     etiquette[i]->control=(unsigned char)(etiquette[i]->control|CONTROL_TAG_BIT_MASK);
  }
  else  {
      etiquette[i]->number=get_token_number(etiquette[i]->input,tok);
      if ((etiquette[i]->number<0) && u_strcmp_char(etiquette[i]->input,"<")) {
        if (etiquette[i]->input[0]=='<') {
          if (etiquette[i]->input[1]=='!') {
            etiquette[i]->control=(unsigned char)(etiquette[i]->control|NEGATION_TAG_BIT_MASK);
            j=2;
          } else j=1;
          k=0;
          while (etiquette[i]->input[j]!='>')
            tmp[k++]=etiquette[i]->input[j++];
          tmp[k]='\0';
          if (!u_strcmp_char(tmp,"MOT")) {
            etiquette[i]->number=MOT;
    		etiquette[i]->control=(unsigned char)(etiquette[i]->control|CONTROL_TAG_BIT_MASK);
          } else if (!u_strcmp_char(tmp,"DIC")) {
            // <DIC> will be computed from 'dlf' and 'dlc'
            // <!DIC> will be computed from the 'err'
            // we do this because part of compound words that are not simple
            // words would recognized by <!DIC>, and we do not want 'aujourd' to
            // appear as an unknown word while it is part of "aujourd'hui"
            if (etiquette[i]->input[1]!='!') (*existe_etiquette_DIC)=1;
            etiquette[i]->number=DIC;
    		etiquette[i]->control=(unsigned char)(etiquette[i]->control|CONTROL_TAG_BIT_MASK);
          } else if (!u_strcmp_char(tmp,"CDIC")) {
            (*existe_etiquette_CDIC)=1;
            etiquette[i]->number=CDIC;
    		etiquette[i]->control=(unsigned char)(etiquette[i]->control|CONTROL_TAG_BIT_MASK);
          } else if (!u_strcmp_char(tmp,"SDIC")) {
            (*existe_etiquette_SDIC)=1;
            etiquette[i]->number=SDIC;
    		etiquette[i]->control=(unsigned char)(etiquette[i]->control|CONTROL_TAG_BIT_MASK);
          } else if (!u_strcmp_char(tmp,"MAJ")) {
            etiquette[i]->number=MAJ;
    		etiquette[i]->control=(unsigned char)(etiquette[i]->control|CONTROL_TAG_BIT_MASK);
          } else if (!u_strcmp_char(tmp,"MIN")) {
            etiquette[i]->number=MIN;
    		etiquette[i]->control=(unsigned char)(etiquette[i]->control|CONTROL_TAG_BIT_MASK);
          } else if (!u_strcmp_char(tmp,"PRE")) {
            etiquette[i]->number=PRE;
    		etiquette[i]->control=(unsigned char)(etiquette[i]->control|CONTROL_TAG_BIT_MASK);
          } else if (!u_strcmp_char(tmp,"NB")) {
            etiquette[i]->number=NB;
    		etiquette[i]->control=(unsigned char)(etiquette[i]->control|CONTROL_TAG_BIT_MASK);

            /* $CD$ begin */
          } else if (!u_strcmp_char(tmp,"TOKEN")) {
            etiquette[i]->number=TOKEN;
    		etiquette[i]->control=(unsigned char)(etiquette[i]->control|CONTROL_TAG_BIT_MASK);
            /* $CD$ end   */

          } else {
            // on a une expression entre angles
            // 4 cas possibles
            decouper_entre_angles(tmp,flechi,canonique,pattern,semantic_codes,alph);
            // 1er cas: <V>
            if ((pattern[0]!='\0')&&(flechi[0]=='\0')&&(canonique[0]=='\0')) {
               etiquette[i]->number=(*nombre_patterns);
    		      etiquette[i]->control=(unsigned char)(etiquette[i]->control|GRAMM_CODE_TAG_BIT_MASK);
               inserer_code_gramm(*nombre_patterns,pattern,NULL,parameters->racine_code_gramm);
               (*nombre_patterns)++;
            } else
            // 2eme cas: <manger.V>
            if ((flechi[0]=='\0')&&(canonique[0]!='\0')&&(pattern[0]!='\0')) {
               etiquette[i]->number=(*nombre_patterns);
    		   etiquette[i]->control=(unsigned char)(etiquette[i]->control|GRAMM_CODE_TAG_BIT_MASK);
               etiquette[i]->control=(unsigned char)(etiquette[i]->control|LEMMA_TAG_BIT_MASK);
               inserer_code_gramm(*nombre_patterns,pattern,canonique,parameters->racine_code_gramm);
               (*nombre_patterns)++;
               etiquette[i]->lemma=(unichar*)malloc((u_strlen(canonique)+1)*sizeof(unichar));
               u_strcpy(etiquette[i]->lemma,canonique);
            } else
            // 3eme cas: <mange,manger.V>
            if ((flechi[0]!='\0')&&(canonique[0]!='\0')&&(pattern[0]!='\0')) {
               etiquette[i]->number=(*nombre_patterns);
    		   etiquette[i]->control=(unsigned char)(etiquette[i]->control|GRAMM_CODE_TAG_BIT_MASK);
               etiquette[i]->control=(unsigned char)(etiquette[i]->control|LEMMA_TAG_BIT_MASK);
               inserer_code_gramm(*nombre_patterns,pattern,canonique,parameters->racine_code_gramm);
               (*nombre_patterns)++;
               etiquette[i]->lemma=(unichar*)malloc((u_strlen(canonique)+1)*sizeof(unichar));
               u_strcpy(etiquette[i]->lemma,canonique);
               etiquette[i]->inflected=(unichar*)malloc((u_strlen(flechi)+1)*sizeof(unichar));
               u_strcpy(etiquette[i]->inflected,flechi);
            } else
            // 4eme cas: <manger>
            if ((flechi[0]=='\0')&&(canonique[0]!='\0')&&(pattern[0]=='\0')) {
               etiquette[i]->number=LEXICAL_TAG;
               etiquette[i]->control=(unsigned char)(etiquette[i]->control|TOKEN_TAG_BIT_MASK);
               etiquette[i]->control=(unsigned char)(etiquette[i]->control|LEMMA_TAG_BIT_MASK);
               etiquette[i]->lemma=(unichar*)malloc((u_strlen(canonique)+1)*sizeof(unichar));
               u_strcpy(etiquette[i]->lemma,canonique);
            }// si on n'est dans aucun de ces 4 cas, c'est une erreur
          }
        }
        else {
          // si l'etiquette n'est pas dans les tokens, on l'y rajoute
          // a cause du feature B.C.
          etiquette[i]->number=get_hash_number(etiquette[i]->input,tok);
          parameters->token_controle[tok->N]=get_controle(etiquette[i]->input,alph,NULL,tokenization_mode);
          etiquette[i]->control=(unsigned char)(etiquette[i]->control|TOKEN_TAG_BIT_MASK);
        }
      } else {
          if (etiquette[i]->number!=parameters->ESPACE) {
            etiquette[i]->control=(unsigned char)(etiquette[i]->control|TOKEN_TAG_BIT_MASK);
          }
          else {
            etiquette[i]->number=SPACE_TAG;
            etiquette[i]->control=(unsigned char)(etiquette[i]->control|CONTROL_TAG_BIT_MASK);
          }
        }
    }
}
}



unsigned char get_controle(unichar* s,Alphabet* alph,struct string_hash* err,int tokenization_mode) {
int i;
int tmp;
unsigned char c=0;
if (s[0]=='\0') {
   return 0;
}
if (is_letter(s[0],alph)) {
  c=(unsigned char)(c|MOT_TOKEN_BIT_MASK);
  // if a token is a word, we check if it is in the 'err' word list
  // in order to answer the question <!DIC>
  if (err!=NULL && get_token_number(s,err)!=-1) {
     c=(unsigned char)(c|NOT_DIC_TOKEN_BIT_MASK);
  }
  if (is_upper(s[0],alph)) {
    c=(unsigned char)(c|PRE_TOKEN_BIT_MASK);
    i=0;
    tmp=0;
    while (s[i]!='\0') {
      if (is_lower(s[i],alph)) {
        tmp=1;
        break;
      }
      i++;
    }
    if (!tmp) {
      return (unsigned char)(c|MAJ_TOKEN_BIT_MASK);
    }
  return c;
  }
  else {
    i=0;
    tmp=0;
    while (s[i]!='\0') {
      if (is_upper(s[i],alph)) {
        tmp=1;
        break;
      }
      i++;
    }
    if (!tmp) return (unsigned char)(c|MIN_TOKEN_BIT_MASK);
    return c;
  }
}
else {
   // we check if the token is a tag like {today,.ADV}
   if (s[0]=='{' && u_strcmp_char(s,"{S}") && u_strcmp_char(s,"{STOP}")) {
      // anyway, such a tag is classed as verifying <MOT> and <DIC>
      c=(MOT_TOKEN_BIT_MASK|DIC_TOKEN_BIT_MASK);
      struct dela_entry* temp=tokenize_tag_token(s);
      if (is_upper(temp->inflected[0],alph)) {
         c=(unsigned char)(c|PRE_TOKEN_BIT_MASK);
         i=0;
         tmp=0;
         while (temp->inflected[i]!='\0') {
            if (is_letter(temp->inflected[i],alph) && is_lower(temp->inflected[i],alph)) {
               tmp=1;
               break;
            }
            i++;
         }
         if (!tmp) {
            c=(unsigned char)(c|MAJ_TOKEN_BIT_MASK);
         }
      }
      else {
         i=0;
         tmp=0;
         while (temp->inflected[i]!='\0') {
            if (is_letter(temp->inflected[i],alph) && is_upper(temp->inflected[i],alph)) {
               tmp=1;
               break;
            }
            i++;
         }
         if (!tmp) {
            c=(unsigned char)(c|MIN_TOKEN_BIT_MASK);
         }
      }
      if (!is_a_simple_word(temp->inflected,alph,tokenization_mode)) {
         // if the tag is a compound word, we say that it verifies the <CDIC> pattern
         c=(unsigned char)(c|CDIC_TOKEN_BIT_MASK);
      }
      free_dela_entry(temp);
      return c;
   }
}
return 0;
}


//
// renvoie 1 si s est une categorie grammaticale, 0 sinon
//
int est_categorie_grammaticale(unichar* s,struct string_hash* semantic_codes) {
if (get_token_number(s,semantic_codes)!=-1) {
   return 1;
}
return 0;
}



//
// renvoie 1 si s peut etre un pattern (V:Kms, N+Hum, ...), 0 sinon
//
int est_pattern(unichar* s,struct string_hash* semantic_codes) {
int i;
if ((s==NULL)||(s[0]=='\0')) {
  return 0;
}
i=0;
while ((s[i]!='\0')&&(s[i]!='+')&&(s[i]!='-')&&(s[i]!=':')) {
    if (s[i]=='\\') {
        i++;
        if (s[i]=='\0') {
            fatal_error("Unexpected backslash at end of line\n");
        }
    }
   i++;
}
// si on a un + un - ou un : on a affaire a un pattern
if (s[i]!='\0') {
   return 1;
}
// sinon, si s est une categorie, on dit que c'est un pattern
if (est_categorie_grammaticale(s,semantic_codes)) {
   return 1;
}
return 0;
}





void decouper_entre_angles(unichar* tmp,unichar* flechi,
                           unichar* canonique,unichar* pattern,
                           struct string_hash* semantic_codes,
                           Alphabet* alph) {
int i,j;
flechi[0]='\0';
canonique[0]='\0';
pattern[0]='\0';
if ((tmp==NULL)||(tmp[0]=='\0')) {
   fprintf(stderr,"The empty pattern <> has been found\n");
   return;
}

i=0;
int k=0;
while ((tmp[i]!=',')&&(tmp[i]!='.')&&(tmp[i]!='\0')) {
   if (tmp[i]=='\\') {i++;}
   flechi[k++]=tmp[i++];
}
flechi[k]='\0';
// 1er cas: <manger> -> on a juste une forme canonique
if (tmp[i]=='\0') {
   // we must test on tmp and NOT on flechi, because of patterns like
   // "<A+faux\-ami>". In fact, tmp contains "A+faux\-ami" and flechi contains
   // "A+faux-ami"
   // So, if we consider flechi instead of tmp, the minus will be taken as
   // a negation and not as a part of the code "faux-ami", and then, no
   // difference will be made between "<A+faux\-ami>" and "<A+faux-ami>".
   if (est_pattern(tmp,semantic_codes)) {
      pattern[0]='\0';
      u_strcpy(pattern,tmp);
      flechi[0]='\0';
      canonique[0]='\0';
      return;
   }
   else {
      u_strcpy(canonique,flechi);
      flechi[0]='\0';
      return;
   }
}
// on est ou dans le cas <manger.V> ou dans le cas <.V>
if (tmp[i]=='.') {
   u_strcpy(canonique,flechi);
   flechi[0]='\0';
   i++;
   j=0;
   while (tmp[i]!='\0') {
      pattern[j++]=tmp[i++];
   }
   pattern[j]='\0';
   return;
}
// on est dans le cas <mange,manger.V>
if (flechi[0]=='\0') {
   fprintf(stderr,"Invalid pattern has been found\n");
   return;
}
i++;
j=0;
while ((tmp[i]!='.')&&(tmp[i]!='\0')) {
   canonique[j++]=tmp[i++];
}
canonique[j]='\0';
if (j==0) {
   fprintf(stderr,"Invalid pattern has been found\n");
   return;
}
if (tmp[i]=='\0') {
   fprintf(stderr,"Invalid pattern has been found\n");
   return;
}
i++;
j=0;
while ((tmp[i]!='.')&&(tmp[i]!='\0')) {
   pattern[j++]=tmp[i++];
}
pattern[j]='\0';
}



void compute_token_controls(struct string_hash* tok,Alphabet* alph,char* err,int tokenization_mode,
                            struct locate_parameters* parameters) {
struct string_hash* ERR=load_word_list(err);
for (int i=0;i<tok->N;i++) {
   parameters->token_controle[i]=get_controle(tok->tab[i],alph,ERR,tokenization_mode);
}
free_string_hash(ERR);
}

