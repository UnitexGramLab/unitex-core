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

#include "unicode.h"
#include "DELA.h"


//
// reads a line in a unicode dictionary
// return 1 if a line was read; 0 otherwise
//
int read_DELA_line(FILE *f,unichar *t) {
int c;
int i=0;
while ((c=u_fgetc(f))!='\n' && c!=EOF) {
  t[i++]=(unichar)c;
}
t[i]='\0';
if (i==0 && c==EOF) {
    return 0; 
}
//if (c==EOF) return 0; // EOF
return 1;
}



//
// tokenizes a DELA line and returns the information in a dic_entry structure
//
dic_entry* tokenize_DELA_line(unichar *s) {
dic_entry* res;
char err[1000];
unichar temp[DIC_LINE_SIZE];
int i,j;
if (s==NULL) return NULL;
// initialization of the result structure
res=(dic_entry*)malloc(sizeof(dic_entry));
if (res==NULL) {
  fprintf(stderr,"Memory allocation error!\n");
  return NULL;
}
res->inflected=NULL;
res->lemma=NULL;
res->n_semantic_codes=1;   // 0 would be an error (no grammatical code)
res->semantic_codes[0]=NULL;
res->n_flexional_codes=0;
res->flexional_codes[0]=NULL;

// reading the inflected part
i=0;
j=0;
while (s[i]!='\0' && s[i]!=',') {
  if (s[i]=='\\') { // case of the special char
    i++;
    if (s[i]=='\0') {
      u_to_char(err,s);
      fprintf(stderr,"***Dictionary error: incorrect line\n_%s_\n",err);
      return NULL;
    }
    else if (s[i]=='=') {
         temp[j++]='\\';
    }
  }
  temp[j++]=s[i++];
}
temp[j]='\0';
res->inflected=(unichar*)malloc(sizeof(unichar)*(u_strlen(temp)+1));
if (res->inflected==NULL) {
  fprintf(stderr,"Memory allocation error!\n");
  return NULL;
}
u_strcpy(res->inflected,temp);
if (s[i]=='\0') {
  u_to_char(err,s);
  fprintf(stderr,"***Dictionary error: incorrect line\n_%s_\n",err);
  return NULL;
}

// reading the lemma part
i++;
j=0;
while (s[i]!='\0' && s[i]!='.') {
  if (s[i]=='\\') {
    i++;
    if (s[i]=='\0') {
      u_to_char(err,s);
      fprintf(stderr,"***Dictionary error: incorrect line\n_%s_\n",err);
      return NULL;
    } else if (s[i]=='=') {
         temp[j++]='\\';
    }
  }
  temp[j++]=s[i++];
}
temp[j]='\0';
if (j==0) {
  res->lemma=(unichar*)malloc(sizeof(unichar)*(u_strlen(res->inflected)+1));
  if (res->lemma==NULL) {
    fprintf(stderr,"Memory allocation error!\n");
    return NULL;
  }
  u_strcpy(res->lemma,res->inflected);
}
else {
  res->lemma=(unichar*)malloc(sizeof(unichar)*(u_strlen(temp)+1));
  if (res->lemma==NULL) {
    fprintf(stderr,"Memory allocation error!\n");
    return NULL;
  }
  u_strcpy(res->lemma,temp);
}
if (s[i]=='\0') {
  u_to_char(err,s);
  fprintf(stderr,"***Dictionary error: incorrect line\n_%s_\n",err);
  return NULL;
}
// reading the grammatical code
i++;
j=0;
while (s[i]!='\0' && s[i]!='+' && s[i]!='/' && s[i]!=':') {
  if (s[i]=='\\') {
    i++;
    if (s[i]=='\0') {
      u_to_char(err,s);
      fprintf(stderr,"***Dictionary error: incorrect line\n_%s_\n",err);
      return NULL;
    }
  }
  temp[j++]=s[i++];
}
temp[j]='\0';
res->semantic_codes[0]=(unichar*)malloc(sizeof(unichar)*(u_strlen(temp)+1));
if (res->semantic_codes[0]==NULL) {
  fprintf(stderr,"Memory allocation error!\n");
  return NULL;
}
u_strcpy(res->semantic_codes[0],temp);
// reading the semantic codes
while (res->n_semantic_codes<MAX_SEMANTIC_CODES && s[i]=='+') {
  i++;
  j=0;
  while (s[i]!='\0' && s[i]!='+' && s[i]!=':' && s[i]!='/') {
    if (s[i]=='\\') {
      i++;
      if (s[i]=='\0') {
        u_to_char(err,s);
        fprintf(stderr,"***Dictionary error: incorrect line\n_%s_\n",err);
        return NULL;
      }
    }
    temp[j++]=s[i++];
  }
  temp[j]='\0';
  res->semantic_codes[res->n_semantic_codes]=(unichar*)malloc(sizeof(unichar)*(u_strlen(temp)+1));
  if (res->semantic_codes[res->n_semantic_codes]==NULL) {
    fprintf(stderr,"Memory allocation error!\n");
    return NULL;
  }
  u_strcpy(res->semantic_codes[res->n_semantic_codes],temp);
  (res->n_semantic_codes)++;
}
// reading the flexional codes
while (res->n_flexional_codes<MAX_FLEXIONAL_CODES && s[i]==':') {
  i++;
  j=0;
  while (s[i]!='\0' && s[i]!=':' && s[i]!='/') {
    if (s[i]=='\\') {
      i++;
      if (s[i]=='\0') {
        u_to_char(err,s);
        fprintf(stderr,"***Dictionary error: incorrect line\n_%s_\n",err);
        return NULL;
      }
    }
    temp[j++]=s[i++];
  }
  temp[j]='\0';
  res->flexional_codes[res->n_flexional_codes]=(unichar*)malloc(sizeof(unichar)*(u_strlen(temp)+1));
  if (res->flexional_codes[res->n_flexional_codes]==NULL) {
    fprintf(stderr,"Memory allocation error!\n");
    return NULL;
  }
  u_strcpy(res->flexional_codes[res->n_flexional_codes],temp);
  (res->n_flexional_codes)++;
}
return res;
}



//
// this function tokenizes a tag token like {today,.ADV}
//
dic_entry* tokenize_tag_token(unichar* s) {
if (s==NULL || s[0]!='{') {
   return NULL;
}
int i=1;
unichar temp[2000];
while (i<2000 && s[i]!='}' && s[i]!='\0') {
  temp[i-1]=s[i];
  i++;
}
temp[i-1]='\0';
return tokenize_DELA_line(temp);
}



//
// this function tokenizes a tag token like {today,.ADV} into 3 strings
//
void tokenize_tag_token_into_3_parts(unichar* s,unichar* inflected,unichar* lemma,unichar* code_gramm) {
if (s==NULL || s[0]!='{') {
   return;
}
int i=1;
unichar temp[2000];
while (i<2000 && s[i]!='}' && s[i]!='\0') {
  temp[i-1]=s[i];
  i++;
}
temp[i-1]='\0';
tokenize_DELA_line_into_3_parts(temp,inflected,lemma,code_gramm);
}



void get_code_gramm(dic_entry* e,unichar* res) {
int i;
u_strcpy_char(res,".");
u_strcat(res,e->semantic_codes[0]);
for (i=1;i<e->n_semantic_codes;i++) {
    u_strcat_char(res,"+");
    u_strcat(res,e->semantic_codes[i]);
}
for (i=0;i<e->n_flexional_codes;i++) {
    u_strcat_char(res,":");
    u_strcat(res,e->flexional_codes[i]);
}
}



//
// counts the tokens of the string s; we delimit tokens by space or -
// because this is a formal and language independent criterion
//
int get_number_of_tokens(unichar* s) {
int n=0;
int lettre=0;
if (s==NULL) return 0;
int i=0;
while (s[i]!='\0') {
    if (s[i]==' ' || s[i]=='-') {
        n++;
        lettre=0;
    }
    else if (!lettre) {
            n++;
            lettre=1;
         }
    i++;
}
return n;
}


//
// returns the length of the longuest prefix common to the 2 strings
//
int get_longuest_prefix(unichar* inflected,unichar* lemma) {
int i=0;
while (inflected[i]==lemma[i] && inflected[i]!='\0') i++;
return i;
}


void get_compressed_token(unichar* inflected,unichar* lemma,unichar* res) {
int prefix=get_longuest_prefix(inflected,lemma);
int a_effacer=u_strlen(inflected)-prefix;
int l_lemma=u_strlen(lemma);


unichar buf[10];



if (l_lemma==1 && (lemma[0]==' ' || lemma[0]=='-') &&
    u_strlen(inflected)==1 && (inflected[0]==' ' || inflected[0]=='-')) {
    // if we have 2 separators, we write it rawly to make the INF file visible
    // ex: "jean-pierre,jean-pierre.N" => "0-0.N" instead of "000.N"
    res[0]=lemma[0];
    res[1]='\0';
    return;
}
u_int_to_string(a_effacer,buf);
u_strcpy(res,buf);
 int l_NB=u_strlen(buf);
int i;
int j=0;
for (i=0;i<(l_lemma-prefix);i++) {
    if ((lemma[i+prefix]>='0' && lemma[i+prefix]<='9')
        || lemma[i+prefix]==',' || lemma[i+prefix]=='.'
        || lemma[i+prefix]=='\\') {
       res[j+l_NB]='\\';
       j++;
    }
    res[j+l_NB]=lemma[i+prefix];
    j++;
}
res[j+l_NB]='\0';
}



void get_token(unichar* s,unichar* token,int *pos) {
if (s[*pos]==' ' || s[*pos]=='-') {
   // case of a separator
   token[0]=s[*pos];
   token[1]='\0';
   (*pos)++;
   return;
}
int j=0;
while (s[*pos]!=' ' && s[*pos]!='-' && s[*pos]!='\0') {
  token[j++]=s[*pos];
  (*pos)++;
}
token[j]='\0';
}



void get_compressed_line(dic_entry* e,unichar* res) {
unichar tmp[1000];
unichar code_gramm[1000];
get_code_gramm(e,code_gramm);

// if the 2 strings are identical, we just return the grammatical code => .N+z1:ms
if (!u_strcmp(e->inflected,e->lemma)) {
   u_strcpy(res,code_gramm);
   return;
}
// we test if the 2 strings have the same number of tokens
int n_inflected=get_number_of_tokens(e->inflected);
int n_lemma=get_number_of_tokens(e->lemma);
if (n_inflected!=n_lemma) {
    // if the 2 strings have not the same number of tokens
    get_compressed_token(e->inflected,e->lemma,tmp);
    u_strcpy_char(res,"_");
    u_strcat(res,tmp);
    u_strcat(res,code_gramm);
    return;
}

// we process now the case of 2 strings that have the same number of tokens
int pos_inflected=0;
int pos_lemma=0;
unichar tmp_inflected[1000];
unichar tmp_lemma[1000];
unichar tmp_compressed[1000];
tmp[0]='\0';
for (int i=0;i<n_inflected;i++) {
  get_token(e->inflected,tmp_inflected,&pos_inflected);
  get_token(e->lemma,tmp_lemma,&pos_lemma);
  get_compressed_token(tmp_inflected,tmp_lemma,tmp_compressed);
  u_strcat(tmp,tmp_compressed);
}
u_strcat(tmp,code_gramm);
u_strcpy(res,tmp);
return;
}



struct token_list* inserer_token(unichar* token,struct token_list* l) {
struct token_list* tmp;
tmp=(struct token_list*)malloc(sizeof(struct token_list));
tmp->token=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(token)));
u_strcpy(tmp->token,token);
tmp->suivant=l;
return tmp;
}


void free_token_list(struct token_list* l) {
struct token_list* tmp;
while (l!=NULL) {
  tmp=l;
  l=l->suivant;
  free(tmp->token);
  free(tmp);
}
}



//
// this function takes a line of a .INF file and tokenize it into
// several single codes.
// Example: .N,.V  =>  token 0=".N" ; token 1=".V"
//
struct token_list* tokenize_compressed_info(unichar* line) {
struct token_list* res=NULL;
unichar tmp[1000];
int pos,i;
pos=0;
i=0;
while (line[pos]!='\0') {
   if (line[pos]==',') {
      // if we are at the end of a token
      tmp[i]='\0';
      res=inserer_token(tmp,res);
      i=0;
      pos++;
   }
   else {
      if (line[pos]=='\\') {
         // if we find a backslash, we take it with the following char
         // doing like this avoids to take a protected comma for a token separator
         tmp[i++]=line[pos++];
      }
      tmp[i++]=line[pos++];
   }
}
tmp[i]='\0';
res=inserer_token(tmp,res);
return res;
}


//
// this function replace res by the rebuilt token
//
void rebuild_token(unichar* res,unichar* info) {
int n=0;
int i,pos=0;
while (info[pos]>='0' && info[pos]<='9') {
  n=n*10+(info[pos]-'0');
  pos++;
}
i=u_strlen(res)-n;
if (i<0) {
   i=0;
}
while (info[pos]!='\0') {
  if (info[pos]=='\\') {
     pos++;
  }
  res[i++]=info[pos++];
}
res[i]='\0';
}



//
// copy the string src to dest add a backslash if a comma is found
//
void copy_inflected(unichar* dest,unichar* src) {
int i=0;
int j=0;
while (src[i]!='\0') {
  if (src[i]==',' || src[i]=='.'
      /* commented by Sébastien Paumier
      || src[i]==':' || src[i]=='+' 
      || src[i]=='-' || src[i]=='/'*/) {dest[j++]='\\';}
  dest[j++]=src[i++];
}
dest[j]='\0';
}



//
// this function takes an entry of the BIN file and a code of the INF file
// it returns in res the rebuilt line
// Example: entry="mains" & info="1.N:fs"  ==>  res="mains,main.N:fs"
//
void uncompress_entry(unichar* entry,unichar* info,unichar* res) {
int n;
int pos,i;
res[0]='\0';

copy_inflected(res,entry);
u_strcat_char(res,",");

if (info[0]=='.') {
   // first case: lemma is the same that entry
   u_strcat(res,info);
   return;
}
if (info[0]=='_') {
   // in this case we rawly suppress chars, before adding some
   pos=1;
   n=0;
   // we read the number of chars to suppress
   while (info[pos]>='0' && info[pos]<='9') {
      n=n*10+(info[pos]-'0');
      pos++;
   }
   u_strcat(res,entry);
   i=u_strlen(res)-n;
   while (info[pos]!='\0') {
     /*if (info[pos]=='\\') {
        pos++;
     }*/
     res[i++]=info[pos++];
   }
   res[i]='\0';
   return;
}
// last case: we have to process token by token
int pos_entry=0;
pos=0;
i=u_strlen(res);
while (info[pos]!='.') {
  if (info[pos]==' ' || info[pos]=='-') {
     // case of a separator
     res[i++]=info[pos++];
     pos_entry++;
  }
  else {
     unichar tmp[10000];
     unichar tmp_entry[10000];
     // we read the compressed token
     int j=0;
     while (info[pos]!='.' && info[pos]!=' ' && info[pos]!='-') {
        /*if (info[pos]=='\\') {
           tmp[j++]=info[pos++];
        }*/
        if (info[pos]=='\\') {
           pos++;
           if (info[pos]!='.') tmp[j++]='\\';
        }
        tmp[j++]=info[pos++];
     }
     tmp[j]='\0';
     // and now the entry token
     j=0;
     while (entry[pos_entry]!='\0' && entry[pos_entry]!=' ' && entry[pos_entry]!='-') {
        tmp_entry[j++]=entry[pos_entry++];
     }
     tmp_entry[j]='\0';
     rebuild_token(tmp_entry,tmp);
     j=0;
     while (tmp_entry[j]!='\0') {
        if (tmp_entry[j]=='.' || tmp_entry[j]=='+' || tmp_entry[j]=='\\'
            || tmp_entry[j]=='/') {
            res[i++]='\\';
        }
        res[i++]=tmp_entry[j++];
     }
  }
}
while (info[pos]!='\0') {
  res[i++]=info[pos++];
}
res[i]='\0';
}



struct INF_codes* load_INF_file(char* nom) {
struct INF_codes* res;
FILE *f=u_fopen(nom,U_READ);
if (f==NULL) {
   fprintf(stderr,"Cannot open %s\n",nom);
   return NULL;
}
res=(struct INF_codes*)malloc(sizeof(struct INF_codes));
res->N=u_read_int(f);
res->tab=(struct token_list**)malloc(sizeof(struct token_list*)*(res->N));
unichar s[4000];
int i=0;
while (read_DELA_line(f,s)) {
  res->tab[i++]=tokenize_compressed_info(s);
}
u_fclose(f);
return res;
}


void free_INF_codes(struct INF_codes* INF) {
if (INF==NULL) return;
for (int i=0;i<INF->N;i++) {
  free_token_list(INF->tab[i]);
}
free(INF);
}



unsigned char* load_BIN_file(char* nom) {
FILE* f;
f=fopen(nom,"rb");
unsigned char* tab;
if (f==NULL) {
   fprintf(stderr,"Cannot open %s\n",nom);
   return NULL;
}
int a,b,c,d;
a=(unsigned char)fgetc(f);
b=(unsigned char)fgetc(f);
c=(unsigned char)fgetc(f);
d=(unsigned char)fgetc(f);
int taille=d+256*c+256*256*b+256*256*256*a;
fclose(f);
f=fopen(nom,"rb");

tab=(unsigned char*)malloc(sizeof(unsigned char)*taille);
if (tab==NULL) {
   fprintf(stderr,"Memory error: cannot load %s\n",nom);
   return NULL;
}
if (taille!=(int)fread(tab,sizeof(char),taille,f)) {
   fprintf(stderr,"Error while reading %s\n",nom);
   free(tab);
   fclose(f);
   return NULL;
}
fclose(f);
return tab;
}


int m=0;

void explore_bin_node(int pos,unichar* contenu,int string_pos,unsigned char* bin,struct INF_codes* inf,FILE *f) {
int n_transitions;
int ref;
if (string_pos>m) {
   m=string_pos;
   //printf("%d\n",m);
   /*if (m>=30) {
      contenu[string_pos]='\0';
      u_prints(contenu);
      getchar();
   }*/
}
n_transitions=((unsigned char)bin[pos])*256+(unsigned char)bin[pos+1];
pos=pos+2;
if (!(n_transitions & 32768)) {
   // we are in a final node
   ref=((unsigned char)bin[pos])*256*256+((unsigned char)bin[pos+1])*256+(unsigned char)bin[pos+2];
   pos=pos+3;
   contenu[string_pos]='\0';
   struct token_list* tmp=inf->tab[ref];
   while (tmp!=NULL) {
      unichar res[1000];
      uncompress_entry(contenu,tmp->token,res);
      u_fprints(res,f);
      u_fprints_char("\n",f);
      tmp=tmp->suivant;
   }
}
else {
   // if we are in a normal node, we remove the control bit to
   // have the good number of transitions
   n_transitions=n_transitions-32768;
}
for (int i=0;i<n_transitions;i++) {
  contenu[string_pos]=(unichar)(((unsigned char)bin[pos])*256+(unsigned char)bin[pos+1]);
  pos=pos+2;
  int adr=((unsigned char)bin[pos])*256*256+((unsigned char)bin[pos+1])*256+(unsigned char)bin[pos+2];
  pos=pos+3;
  explore_bin_node(adr,contenu,string_pos+1,bin,inf,f);
}
}



//
// this function explore the automaton stored in the bin and rebuild
// the original DELA
//
void rebuild_dictionary(unsigned char* bin,struct INF_codes* inf,FILE* f) {
unichar contenu[10000];
explore_bin_node(4,contenu,0,bin,inf,f);
}



//
// tokenizes a DELA line into the inflected part and the remaining part
// avons,avoir.V+z1:P1p -> "avons" and ",avoir.V+z1:P1p"
//
// This function is used to generate the dictionary tree for
// constructing the text automaton 
//
void tokenize_DELA_line_into_inflected_and_code(unichar* s,unichar* inflected,unichar* code) {
int i,j;
char err[1000];
unichar inflected_with_backslash[1024];
int y=0;
if (s==NULL) return;
// reading the inflected part
i=0;
j=0;
y=0;
while (s[i]!='\0' && s[i]!=',') {
  if (s[i]=='\\') { // case of the special char 
    inflected_with_backslash[y++]='\\';
    i++;
    if (s[i]=='\0') {
      u_to_char(err,s);
      fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
      return;
    }
  }
  inflected_with_backslash[y++]=s[i];
  inflected[j++]=s[i++];
}
inflected[j]='\0';
inflected_with_backslash[y]='\0';

if (s[i]=='\0') {
  u_to_char(err,s);
  fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
  return;
}

// reading the lemma part
i++;
code[0]=',';
j=1;
while (s[i]!='\0' && s[i]!='.') {
  if (s[i]=='\\') {
    code[j++]='\\';
    i++;
    if (s[i]=='\0') {
      u_to_char(err,s);
      fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
      return;
    }
  }
  code[j++]=s[i++];
}
code[j]='\0';
if (j==1) {
   // if the lemma is not specified, we copy the inflected form
  u_strcat(code,inflected_with_backslash);
}
if (s[i]=='\0') {
  u_to_char(err,s);
  fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
  return;
}

// reading the remaining part of the line
i++;
j=u_strlen(code);
code[j++]='.';
while (s[i]!='\0') {
  if (s[i]=='\\') {
    code[j++]='\\';
    i++;
    if (s[i]=='\0') {
      u_to_char(err,s);
      fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
      return;
    }
  }
  code[j++]=s[i++];
}
code[j]='\0';

}



void extract_semantic_codes(char* nom,struct string_hash* hash) {
FILE* f=u_fopen(nom,U_READ);
if (f==NULL) return;
unichar s[1000];
unichar temp[1000];
char err[1000];
int i,j;
while (read_DELA_line(f,s)) {
  if (s[0]!='\0') {
  i=0;
  while (s[i]!='\0' && s[i]!=',') {
     if (s[i]=='\\') {
        i++;
        if (s[i]=='\0') {
           u_to_char(err,s);
	       fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
           break;
        }
     }
     if (s[i]=='\0') {
     	u_to_char(err,s);
        fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
        break;
     }
     i++;
  }
  while (s[i]!='\0' && s[i]!='.') {
     if (s[i]=='\\') {
        i++;
        if (s[i]=='\0') {
           u_to_char(err,s);
           fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
           break;
        }
     }
     if (s[i]=='\0') {
     	u_to_char(err,s);
        fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
        break;
     }
     i++;
  }
  // reading the grammatical code
  i++;
  j=0;
  while (s[i]!='\0' && s[i]!='+' && s[i]!='/' && s[i]!=':') {
     if (s[i]=='\\') {
        i++;
        if (s[i]=='\0') {
           u_to_char(err,s);
           fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
           break;
        }
     }
     temp[j++]=s[i++];
  }
  temp[j]='\0';
  get_hash_number(temp,hash);
  // reading the semantic codes
  while (s[i]=='+') {
     i++;
     j=0;
     while (s[i]!='\0' && s[i]!='+' && s[i]!=':' && s[i]!='/') {
        if (s[i]=='\\') {
           i++;
           if (s[i]=='\0') {
              u_to_char(err,s);
              fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
              break;
           }
        }
        temp[j++]=s[i++];
     }
     temp[j]='\0';
     get_hash_number(temp,hash);
    }
  }
}
fclose(f);
}



void tokenize_DELA_line_into_3_parts(unichar* s,unichar* inflected,unichar* lemma,unichar* code) {
int i,j;
char err[1000];
if (s==NULL) return;
// reading the inflected part
i=0;
j=0;
while (s[i]!='\0' && s[i]!=',') {
  if (s[i]=='\\') { // case of the special char
    i++;
    if (s[i]=='\0') {
      u_to_char(err,s);
      fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
      return;
    }
  }
  inflected[j++]=s[i++];
}
inflected[j]='\0';

if (s[i]=='\0') {
  u_to_char(err,s);
  fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
  return;
}

// reading the lemma part
i++;
j=0;
while (s[i]!='\0' && s[i]!='.') {
  if (s[i]=='\\') {
    i++;
    if (s[i]=='\0') {
      u_to_char(err,s);
      fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
      return;
    }
  }
  lemma[j++]=s[i++];
}
lemma[j]='\0';
if (j==0) {
   // if the lemma is not specified, we copy the inflected form
   u_strcpy(lemma,inflected);
}
if (s[i]=='\0') {
  u_to_char(err,s);
  fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
  return;
}
// reading the remaining part of the line
i++;
j=0;
while (s[i]!='\0') {
  if (s[i]=='\\') {
    i++;
    if (s[i]=='\0') {
       u_to_char(err,s);
       fprintf(stderr,"***Dictionary error: incorrect line\n%s\n",err);
       return;
    }
  }
  code[j++]=s[i++];
}
code[j]='\0';
}



void check_DELAS_line(unichar* s,FILE* out,int line,char* alphabet,
                      struct string_hash* semantic,struct string_hash* inflectional) {
char err[5000];
unichar temp[DIC_LINE_SIZE];
int i,j;
if (s==NULL) return;
// reading the inflected part
i=0;
j=0;
while (s[i]!='\0' && s[i]!=',') {
  if (s[i]=='\t') {
     sprintf(err,"Line %d: tabulation in lemma\n",line);
     u_fprints_char(err,out);
     u_fprints(s,out);
     u_fprints_char("\n",out);
     return;
  }
  if (s[i]=='\\') { // case of the special char
    i++;
    if (s[i]=='\0') {
       sprintf(err,"Line %d: \\ at end of line\n",line);
       u_fprints_char(err,out);
       u_fprints(s,out);
       u_fprints_char("\n",out);
       return;
    }
  }
  alphabet[s[i]]=1;
  temp[j++]=s[i++];
}
if (s[i]=='\0') {
   sprintf(err,"Line %d: no comma found\n",line);
   u_fprints_char(err,out);
   u_fprints(s,out);
   u_fprints_char("\n",out);
   return;
}
temp[j]='\0';
if (j==0) {
   sprintf(err,"Line %d: empty canonical form\n",line);
   u_fprints_char(err,out);
   u_fprints(s,out);
   u_fprints_char("\n",out);
   return;
}

// reading the grammatical code
i++;
j=0;
while (s[i]!='\0' && s[i]!='+' && s[i]!=':' && s[i]!='/') {
  if (s[i]=='\t') {
     sprintf(err,"Line %d: tabulation in gramatical or semantic code\n",line);
     u_fprints_char(err,out);
     u_fprints(s,out);
     u_fprints_char("\n",out);
     return;
  }
  if (s[i]=='\\') {
    i++;
    if (s[i]=='\0') {
       sprintf(err,"Line %d: \\ at end of line\n",line);
       u_fprints_char(err,out);
       u_fprints(s,out);
       u_fprints_char("\n",out);
       return;
    }
  }
  temp[j++]=s[i++];
}
temp[j]='\0';
if (j==0) {
   sprintf(err,"Line %d: no grammatical code\n",line);
   u_fprints_char(err,out);
   u_fprints(s,out);
   u_fprints_char("\n",out);
   return;
}
get_hash_number(temp,semantic);
// reading the semantic codes
while (s[i]=='+') {
  i++;
  j=0;
  while (s[i]!='\0' && s[i]!='+' && s[i]!=':' && s[i]!='/') {
    if (s[i]=='\t') {
       sprintf(err,"Line %d: tabulation in gramatical or semantic code\n",line);
       u_fprints_char(err,out);
       u_fprints(s,out);
        u_fprints_char("\n",out);
       return;
    }
    if (s[i]=='\\') {
      i++;
      if (s[i]=='\0') {
         sprintf(err,"Line %d: \\ at end of line\n",line);
         u_fprints_char(err,out);
         u_fprints(s,out);
         u_fprints_char("\n",out);
         return;
      }
    }
    temp[j++]=s[i++];
  }
  if (j==0) {
     sprintf(err,"Line %d: empty gramatical or semantic code\n",line);
     u_fprints_char(err,out);
     u_fprints(s,out);
     u_fprints_char("\n",out);
     return;
  }
  temp[j]='\0';
  get_hash_number(temp,semantic);
}
// reading the flexional codes
while (s[i]==':') {
  i++;
  j=0;
  while (s[i]!='\0' && s[i]!=':' && s[i]!='/') {
    if (s[i]=='\t') {
       sprintf(err,"Line %d: tabulation in inflexional code\n",line);
       u_fprints_char(err,out);
       u_fprints(s,out);
        u_fprints_char("\n",out);
       return;
    }
    if (s[i]=='\\') {
      i++;
      if (s[i]=='\0') {
         sprintf(err,"Line %d: \\ at end of line\n",line);
         u_fprints_char(err,out);
         u_fprints(s,out);
         u_fprints_char("\n",out);
         return;
      }
    }
    temp[j++]=s[i++];
  }
  if (j==0) {
     sprintf(err,"Line %d: empty inflectional code\n",line);
     u_fprints_char(err,out);
     u_fprints(s,out);
     u_fprints_char("\n",out);
     return;
  }
  temp[j]='\0';
  get_hash_number(temp,inflectional);
}
}



void check_DELAF_line(unichar* s,FILE* out,int line,char* alphabet,
                      struct string_hash* semantic,struct string_hash* inflectional) {
char err[5000];
unichar temp[DIC_LINE_SIZE];
int i,j;
if (s==NULL) return;
// reading the inflected part
i=0;
j=0;
while (s[i]!='\0' && s[i]!=',') {
  if (s[i]=='\t') {
     sprintf(err,"Line %d: tabulation in inflected form\n",line);
     u_fprints_char(err,out);
     u_fprints(s,out);
     u_fprints_char("\n",out);
     return;
  }
  if (s[i]=='\\') { // case of the special char
    i++;
    if (s[i]=='\0') {
       sprintf(err,"Line %d: \\ at end of line\n",line);
       u_fprints_char(err,out);
       u_fprints(s,out);
       u_fprints_char("\n",out);
       return;
    }
  }
  alphabet[s[i]]=1;
  temp[j++]=s[i++];
}
if (s[i]=='\0') {
   sprintf(err,"Line %d: no comma found\n",line);
   u_fprints_char(err,out);
   u_fprints(s,out);
   u_fprints_char("\n",out);
   return;
}
temp[j]='\0';
if (j==0) {
   sprintf(err,"Line %d: empty inflected form\n",line);
   u_fprints_char(err,out);
   u_fprints(s,out);
   u_fprints_char("\n",out);
   return;
}
// reading the lemma part
i++;
j=0;
int virgule=0;
while (s[i]!='\0' && s[i]!='.') {
  if (s[i]=='\t') {
     sprintf(err,"Line %d: tabulation in lemma\n",line);
     u_fprints_char(err,out);
     u_fprints(s,out);
     u_fprints_char("\n",out);
     return;
  }
  if (s[i]==',') virgule=1;
  else if (s[i]=='\\') {
    i++;
    if (s[i]=='\0') {
       sprintf(err,"Line %d: \\ at end of line\n",line);
       u_fprints_char(err,out);
       u_fprints(s,out);
       u_fprints_char("\n",out);
       return;
    }
  }
  alphabet[s[i]]=1;
  temp[j++]=s[i++];
}
temp[j]='\0';
if (s[i]=='\0') {
   sprintf(err,"Line %d: no point found\n",line);
   u_fprints_char(err,out);
   u_fprints(s,out);
   u_fprints_char("\n",out);
   return;
}
if (virgule) {
   sprintf(err,"Line %d: unprotected comma in lemma\n",line);
   u_fprints_char(err,out);
   u_fprints(s,out);
   u_fprints_char("\n",out);
}
// reading the grammatical code
i++;
j=0;
while (s[i]!='\0' && s[i]!='+' && s[i]!=':' && s[i]!='/') {
  if (s[i]=='\t') {
     sprintf(err,"Line %d: tabulation in gramatical or semantic code\n",line);
     u_fprints_char(err,out);
     u_fprints(s,out);
     u_fprints_char("\n",out);
     return;
  }
  if (s[i]=='\\') {
    i++;
    if (s[i]=='\0') {
       sprintf(err,"Line %d: \\ at end of line\n",line);
       u_fprints_char(err,out);
       u_fprints(s,out);
       u_fprints_char("\n",out);
       return;
    }
  }
  temp[j++]=s[i++];
}
temp[j]='\0';
if (j==0) {
   sprintf(err,"Line %d: no grammatical code\n",line);
   u_fprints_char(err,out);
   u_fprints(s,out);
   u_fprints_char("\n",out);
   return;
}
get_hash_number(temp,semantic);
// reading the semantic codes
while (s[i]=='+') {
  i++;
  j=0;
  while (s[i]!='\0' && s[i]!='+' && s[i]!=':' && s[i]!='/') {
    if (s[i]=='\t') {
     sprintf(err,"Line %d: tabulation in gramatical or semantic code\n",line);
     u_fprints_char(err,out);
     u_fprints(s,out);
     u_fprints_char("\n",out);
     return;
  }
  if (s[i]=='\\') {
      i++;
      if (s[i]=='\0') {
         sprintf(err,"Line %d: \\ at end of line\n",line);
         u_fprints_char(err,out);
         u_fprints(s,out);
         u_fprints_char("\n",out);
         return;
      }
    }
    temp[j++]=s[i++];
  }
  if (j==0) {
     sprintf(err,"Line %d: empty gramatical or semantic code\n",line);
     u_fprints_char(err,out);
     u_fprints(s,out);
     u_fprints_char("\n",out);
     return;
  }
  temp[j]='\0';
  get_hash_number(temp,semantic);
}
// reading the inflectional codes
while (s[i]==':') {
  i++;
  j=0;
  while (s[i]!='\0' && s[i]!=':' && s[i]!='/') {
    if (s[i]=='\t') {
     sprintf(err,"Line %d: tabulation in inflectional code\n",line);
     u_fprints_char(err,out);
     u_fprints(s,out);
     u_fprints_char("\n",out);
     return;
  }
  if (s[i]=='\\') {
      i++;
      if (s[i]=='\0') {
         sprintf(err,"Line %d: \\ at end of line\n",line);
         u_fprints_char(err,out);
         u_fprints(s,out);
         u_fprints_char("\n",out);
         return;
      }
    }
    temp[j++]=s[i++];
  }
  if (j==0) {
     sprintf(err,"Line %d: empty inflectional code\n",line);
     u_fprints_char(err,out);
     u_fprints(s,out);
     u_fprints_char("\n",out);
     return;
  }
  temp[j]='\0';
  get_hash_number(temp,inflectional);
}
}



//
// this function takes a char sequence supposed to represent a
// gramatical, semantic or inflectional code;
// if this code contains space, tabulation or any non-ASCII char
// it returns 1 and stores a warning message in comment; returns 0 else
//
int warning_on_code(unichar* code,unichar* comment) {
int i;
int space=0;
int tab=0;
int non_ascii=0;
int n=0;
int l=u_strlen(code);
for (i=0;i<l;i++) {
   if (code[i]==' ') {space++;n++;}
   else if (code[i]=='\t') {tab++;n++;}
   else if (code[i]>=128) {non_ascii++;n++;}
}
if (space || tab || non_ascii) {
   char temp[1000];
   sprintf(temp,"warning: %d suspect char%s (",n,(n>1)?"s":"");
   u_strcpy_char(comment,temp);
   if (space) {
      sprintf(temp,"%d space%s, ",space,(space>1)?"s":"");
      u_strcat_char(comment,temp);
   }
   if (tab) {
      sprintf(temp,"%d tabulation%s, ",tab,(tab>1)?"s":"");
      u_strcat_char(comment,temp);
   }
   if (non_ascii) {
      sprintf(temp,"%d non ASCII char%s, ",non_ascii,(non_ascii>1)?"s":"");
      u_strcat_char(comment,temp);
   }
   comment[u_strlen(comment)-2]='\0';
   u_strcat_char(comment,"): (");
   unichar temp2[10];
   for (i=0;i<l-1;i++) {
      u_char_to_hexa_or_code(code[i],temp2);
      u_strcat(comment,temp2);
      u_strcat_char(comment," ");
   }
   u_char_to_hexa_or_code(code[l-1],temp2);
   u_strcat(comment,temp2);
   u_strcat_char(comment,")");
   return 1;
}
else return 0;
}



//
// test if the sequence contains an unprotected = sign
//
int contains_unprotected_equal_sign(unichar* s) {
if (s[0]=='=') {
   return 1;
}
for (int i=1;s[i]!='\0';i++) {
   if (s[i]=='=' && s[i-1]!='\\') {
      return 1;
   }
}
return 0;
}


//
// replaces all the unprotected = sign by the char c
//
void replace_unprotected_equal_sign(unichar* s,unichar c) {
if (s[0]=='=') {
   s[0]=c;
}
for (int i=1;s[i]!='\0';i++) {
   if (s[i]=='=' && s[i-1]!='\\') {
      s[i]=c;
   }
}
}



//
// takes a string containing protected equal sign and unprotect them
// ex: E\=mc2 -> E=mc2 
//
void unprotect_equal_signs(unichar* s) {
unichar temp[1000];
int j=0;
for (int i=0;s[i]!='\0';i++) {
   if (s[i]=='\\' && s[i+1]=='=') {
   }
   else temp[j++]=s[i];
}
temp[j]='\0';
u_strcpy(s,temp);
}



void free_dic_entry(dic_entry* d) {
if (d==NULL) return;
if (d->inflected!=NULL) free(d->inflected);
if (d->lemma!=NULL) free(d->lemma);
for (int i=0;i<d->n_semantic_codes;i++) {
   free(d->semantic_codes[i]);
}
for (int i=0;i<d->n_flexional_codes;i++) {
   free(d->flexional_codes[i]);
}
free(d);
}



//
// this function takes a string like ":F1s:F2s" and builds an array
// like this: *N -> 2,   tab[0]="F1s",    tab[1]="F2s"
//
void tokenize_inflectional_codes(unichar* s,int* N,unichar** tab) {
int i=0;
(*N)=0;
while (s[i]==':') {
   i++;
   int j=0;
   unichar temp[1000];
   while (s[i]!='\0' && s[i]!=':') {
      temp[j++]=s[i++];
   }
   temp[j]='\0';
   tab[*N]=(unichar*)malloc(sizeof(unichar)*(1+u_strlen(temp)));
   u_strcpy(tab[*N],temp);
   (*N)++;
}
}



//
// this function tests if the tag token passed in parameter is correct
// it returns 1 on success, 0 else
//
int check_tag_token(unichar* s) {
int i,j;
if (s==NULL || s[0]!='{' || s[u_strlen(s)-1]!='}') {
   return 0;
}
// reading the inflected part
i=1;
j=0;
while (s[i]!='\0' && s[i]!='}' && s[i]!=',') {
  if (s[i]=='\t') {
     fprintf(stderr,"Error: tabulation found in a tag\n");
     return 0;
  }
  if (s[i]=='\\') { // case of the special char
    i++;
    if (s[i]=='\0' || s[i]=='}') {
       fprintf(stderr,"Error: \\ at end of a tag\n");
       return 0;
    }
  }
  j++; i++;
}
if (s[i]=='\0' || s[i]=='}') {
   fprintf(stderr,"Error: tag without comma\n");
   return 0;
}
if (j==0) {
   fprintf(stderr,"Error: empty inflected form in a tag\n");
   return 0;
}
// reading the lemma part
i++;
int virgule=0;
while (s[i]!='\0' && s[i]!='}' && s[i]!='.') {
  if (s[i]=='\t') {
     fprintf(stderr,"Error: tabulation found in the lemma of a tag\n");
     return 0;
  }
  if (s[i]==',') virgule=1;
  else if (s[i]=='\\') {
    i++;
    if (s[i]=='\0' || s[i]=='}') {
       fprintf(stderr,"Error: \\ at end of a tag\n");
       return 0;
    }
  }
  i++;
}
if (s[i]=='\0' || s[i]=='}') {
   fprintf(stderr,"Error: tag without point\n");
   return 0;
}
if (virgule) {
   fprintf(stderr,"Error: unprotected comma in the lemma of a tag\n");
}
// reading the grammatical code
i++;
j=0;
while (s[i]!='\0' && s[i]!='}' && s[i]!='+' && s[i]!=':' && s[i]!='/') {
  if (s[i]=='\t') {
     fprintf(stderr,"Error: tabulation found in a gramatical or semantic code of a tag\n");
     return 0;
  }
  if (s[i]=='\\') {
    i++;
    if (s[i]=='\0' || s[i]=='}') {
       fprintf(stderr,"Error: \\ at end of a tag\n");
       return 0;
    }
  }
  j++; i++;
}
if (j==0) {
   fprintf(stderr,"Error: tag with no grammatical code\n");
   return 0;
}
// reading the semantic codes
while (s[i]=='+') {
  i++;
  j=0;
  while (s[i]!='\0' && s[i]!='}' && s[i]!='+' && s[i]!=':' && s[i]!='/') {
    if (s[i]=='\t') {
     fprintf(stderr,"Error: tabulation found in a gramatical or semantic code of a tag\n");
     return 0;
  }
  if (s[i]=='\\') {
      i++;
      if (s[i]=='\0' || s[i]=='}') {
         fprintf(stderr,"Error: \\ at end of a tag\n");
         return 0;
      }
    }
    j++; i++;
  }
  if (j==0) {
     fprintf(stderr,"Error: empty gramatical or semantic code in a tag\n");
     return 0;
  }
}
// reading the inflectional codes
while (s[i]==':') {
  i++;
  j=0;
  while (s[i]!='\0' && s[i]!='}' && s[i]!=':' && s[i]!='/') {
    if (s[i]=='\t') {
     fprintf(stderr,"Error: tabulation found in an inflectional code of a tag\n");
     return 0;
  }
  if (s[i]=='\\') {
      i++;
      if (s[i]=='\0' || s[i]=='}') {
         fprintf(stderr,"Error: \\ at end of a tag\n");
         return 0;
      }
    }
    j++; i++;
  }
  if (j==0) {
     fprintf(stderr,"Error: empty inflectional code in a tag\n");
     return 0;
  }
}
return 1;
}



//
// returns 1 if the entry d contains the grammatical code s
//
int dic_entry_contain_gram_code(dic_entry* d,unichar* s) {
for (int i=0;i<d->n_semantic_codes;i++) {
   if (!u_strcmp(d->semantic_codes[i],s)) {
      return 1;
   }
}
return 0;
}



//
// returns 1 if the first inflectional code a contains the second b
//
int one_flexional_codes_contains_the_other(unichar* a,unichar* b) {
int i=0;
while (b[i]!='\0') {
   int j=0;
   while (a[j]!=b[i]) {
      if (a[j]=='\0') {
         // if we have not found a character, we return
         return 0;
      }
      j++;
   }

   i++;
}
return 1;
}



//
// returns 1 if the entry d contains the inflectional code s
//
int dic_entry_contain_flex_code(dic_entry* d,unichar* s) {
for (int i=0;i<d->n_flexional_codes;i++) {
   if (one_flexional_codes_contains_the_other(d->flexional_codes[i],s)) {
      return 1;
   }
}
return 0;
}



int exploreBinDic(int pos,unichar* word,int string_pos,unsigned char* BIN,Alphabet* alph) {
int n_transitions;
n_transitions=((unsigned char)BIN[pos])*256+(unsigned char)BIN[pos+1];
pos=pos+2;
if (word[string_pos]=='\0') {
   // if we are at the end of the word, we just check
   // if the state is final, i.e. if the word belongs
   // to the dictionary
   if (!(n_transitions & 32768)) {
      // if the state is final
      return ((unsigned char)BIN[pos])*256*256+((unsigned char)BIN[pos+1])*256+(unsigned char)BIN[pos+2];
   }
   else {
      return NOT_IN_DICTIONARY;
   }
}
if ((n_transitions & 32768)) {
   // if we are in a normal node, we remove the control bit to
   // have the good number of transitions
   n_transitions=n_transitions-32768;
} else {
  // if we are in a final node, we must jump after the reference to the INF line number
  pos=pos+3;
}
for (int i=0;i<n_transitions;i++) {
  unichar c=(unichar)(((unsigned char)BIN[pos])*256+(unsigned char)BIN[pos+1]);
  pos=pos+2;
  int adr=((unsigned char)BIN[pos])*256*256+((unsigned char)BIN[pos+1])*256+(unsigned char)BIN[pos+2];
  pos=pos+3;
  if (is_equal_or_uppercase(c,word[string_pos],alph)) {
     int res=exploreBinDic(adr,word,string_pos+1,BIN,alph);
     if (res!=NOT_IN_DICTIONARY) return res;
  }
}
return NOT_IN_DICTIONARY;
}


//
// Checks if a given word 'w' is in the dictionary 'bin'. If not,
// the function returns NOT_IN_DICTIONARY; otherwise, the INF line
// number of this entry is returned.
//
int get_INF_number(unichar *w,unsigned char* bin,Alphabet* alph) {
return exploreBinDic(4,w,0,bin,alph);
}

