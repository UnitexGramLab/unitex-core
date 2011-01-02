/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <string.h>
#include <stdint.h>
#include "Tfst.h"
#include "File.h"
#include "TfstStats.h"


void free_current_sentence(Tfst*);



/**
 * Allocates, initializes and returns a Tfst*
 */
Tfst* new_Tfst(U_FILE* tfst,U_FILE* tind,int N) {
Tfst* t=(Tfst*)malloc(sizeof(Tfst));
if (t==NULL) {
   fatal_alloc_error("new_Tfst");
}
t->N=N;
t->tfst=tfst;
t->tind=tind;
t->current_sentence=NO_SENTENCE_LOADED;
t->text=NULL;
t->tokens=NULL;
t->token_sizes=NULL;
t->token_content=NULL;
t->offset_in_tokens=-1;
t->offset_in_chars=-1;
t->automaton=NULL;
t->tags=NULL;
return t;
}


/**
 * Frees all the memory associated to the given Tfst*
 */
void close_text_automaton(Tfst* t) {
if (t==NULL) return;
if (t->tfst!=NULL) u_fclose(t->tfst);
if (t->tind!=NULL) u_fclose(t->tind);
free_current_sentence(t);
free(t);
}


/**
 * Opens the given text automaton file, but loads no sentence.
 * Returns NULL if:
 *  - the .tfst does not exist
 *  - the .tind does not exist
 *  - the .tfst does not start by a number >0
 */
Tfst* open_text_automaton(char* tfst) {
char tind[FILENAME_MAX];
remove_extension(tfst,tind);
strcat(tind,".tind");
int size=(int)get_file_size(tind);
if (size==-1) {
   error("Cannot get size of file %s\n",tind);
   return NULL;
}
U_FILE* f=u_fopen_existing_unitex_text_format(tfst,U_READ);
if (f==NULL) {
   error("Cannot open file %s\n",tfst);
   return NULL;
}
U_FILE* f2=u_fopen(BINARY,tind,U_READ);
if (f2==NULL) {
   /* Should not happen since we checked the size of this file */
   error("Cannot open file %s\n",tind);
   u_fclose(f);
   return NULL;
}
int N;
if (1!=u_fscanf(f,"%d",&N) || N<=0 || N*4!=size) {
   error("Bad number of sentence: %d\n",N);
   return NULL;
}
return new_Tfst(f,f2,N);
}


/**
 * Frees the memory related to the current sentence.
 */
void free_current_sentence(Tfst* tfst) {
if (tfst==NULL) {
   fatal_error("NULL error in free_current_sentence\n");
}
tfst->current_sentence=NO_SENTENCE_LOADED;
free(tfst->text);
tfst->text=NULL;
if (tfst->token_content!=NULL) {
	for (int i=0;i<tfst->tokens->nbelems;i++) {
		free(tfst->token_content[i]);
	}
	free(tfst->token_content);
	tfst->token_content=NULL;
}
free_vector_int(tfst->tokens);
tfst->tokens=NULL;
free_vector_int(tfst->token_sizes);
tfst->token_sizes=NULL;
tfst->offset_in_tokens=-1;
tfst->offset_in_chars=-1;
free_SingleGraph(tfst->automaton,NULL);
tfst->automaton=NULL;
free_vector_ptr(tfst->tags,(void (*)(void*))free_TfstTag);
tfst->tags=NULL;
}


/**
 * Returns the offset of the given sentence in the .tfst of the
 * given text automaton. Remember that sentences are numbered from 1.
 */
long get_sentence_offset(Tfst* t,int n) {
long pos=4*(n-1);
fseek(t->tind,pos,SEEK_SET);
unsigned char tab[4];
if (4!=fread(tab,1,4,t->tind)) {
   fatal_error("Read error in get_sentence_offset\n");
}
long offset=tab[0]+(tab[1]<<8)+(tab[2]<<16)+(tab[3]<<24);
return offset;
}


/**
 * Tries to read a valid epsilon tag.
 */
TfstTag* read_epsilon_tag(Tfst* t,Ustring* foo) {
readline(foo,t->tfst);
if (u_strcmp(foo->str,".\n")) {
   fatal_error("read_epsilon_tag: malformed epsilon tag\n");
}
return new_TfstTag(T_EPSILON);
}


/**
 * Tries to read a valid STD tag.
 */
TfstTag* read_normal_tag(Tfst* t,Ustring* foo) {
/* We read the content */
readline(foo,t->tfst);
chomp_new_line(foo);
if (foo->str[0]!='@' || foo->str[1]=='\0') {
   fatal_error("read_normal_tag: invalid content line %S\n",foo->str);
}
unichar* content=u_strdup(foo->str+1);
/* We read the bounds that are supposed to be as follows:
 *
 *    @a.b.c-d.e.fZ
 *
 * a=start pos in tokens
 * b=start pos in characters
 * c=start pos in logical letters (for Korean)
 * d=end pos in tokens
 * e=end pos in characters
 * f=end pos in logical letters (for Korean)
 *
 * Z is used for Korean:
 *    Z='#' if there is no syllable bound at the end of the input sequence
 *    If there is a syllable bound, then Z is not set.
 */
readline(foo,t->tfst);
chomp_new_line(foo);
if (foo->str[0]!='@' || foo->str[1]=='\0') {
   fatal_error("read_normal_tag #1: invalid bounds line %S\n",foo->str);
}
int a=-1,b=-1,c=-1,d=-1,e=-1,f=-1,n;
unichar Z='\0';

/* We read the start position */
n=u_sscanf(&(foo->str[1]),"%d.%d.%d-%d.%d.%d%C",&a,&b,&c,&d,&e,&f,&Z);
if (n<6 || (n==7 && Z!='#')) {
   fatal_error("read_normal_tag #2: invalid bounds line %S\n",foo->str);
}
/* We don't test f<0, because f==-1 is the sign that the tag has an empty
 * surface form like {<E>,.JO} */
if (a<0 || b<0 || c<0 || d<0 || e<0 /* || f<0*/) {
   fatal_error("read_normal_tag: negative bound in line %S\n",foo->str);
}
/* Finally, we must check that we have the final line with a dot */
readline(foo,t->tfst);
if (u_strcmp(foo->str,".\n")) {
   fatal_error("read_normal_tag: invalid final line %S\n",foo->str);
}
TfstTag* tag=new_TfstTag(T_STD);
tag->content=content;
tag->m.start_pos_in_token=a;
tag->m.start_pos_in_char=b;
tag->m.start_pos_in_letter=c;
tag->m.end_pos_in_token=d;
tag->m.end_pos_in_char=e;
tag->m.end_pos_in_letter=f;
return tag;
}


/**
 * Loads the given sentence of the given text automaton.
 */
void load_sentence(Tfst* tfst,int n) {
if (tfst==NULL) {
   fatal_error("NULL error in load_sentence\n");
}
if (n<=0 || n>tfst->N) {
   fatal_error("Invalid sentence number %d in load_sentence: should be in [1;%d]\n",n,tfst->N);
}
if (tfst->current_sentence!=NO_SENTENCE_LOADED) {
   free_current_sentence(tfst);
}
tfst->current_sentence=n;
long offset=get_sentence_offset(tfst,n);
fseek(tfst->tfst,offset,SEEK_SET);
/* Now we can read the sentence */
int N=0;
unichar z;
if (2!=u_fscanf(tfst->tfst,"%C%d\n",&z,&N) || z!='$' || N!=n) {
   fatal_error("load_sentence: Invalid sentence header line: should be $%d\n",n);
}
Ustring* foo=new_Ustring(16);
readline(foo,tfst->tfst);
chomp_new_line(foo);
tfst->text=u_strdup(foo->str);
readline(foo,tfst->tfst);
tfst->tokens=new_vector_int(16);
tfst->token_sizes=new_vector_int(16);
int tmp,tmp2,pos=0,shift,ret;
while ((ret=u_sscanf(foo->str+pos,"%d/%d%n",&tmp,&tmp2,&shift))==2) {
   pos=pos+shift;
   vector_int_add(tfst->tokens,tmp);
   vector_int_add(tfst->token_sizes,tmp2);
}
if (ret==1 || (ret==0 && foo->str[pos]!='\n')) {
   /* If the token line was not fully read */
   fatal_error("load_sentence: malformed token line:\n%S\n",foo->str);
}
/* We read the offsets of the sentence */
readline(foo,tfst->tfst);
chomp_new_line(foo);
if (2!=u_sscanf(foo->str,"%d_%d%C",&(tfst->offset_in_tokens),&(tfst->offset_in_chars),&z)) {
   fatal_error("load_sentence: malformed offset line:\n%S\n",foo->str);
}
/* Now, we have to read the states */
tfst->automaton=new_SingleGraph(INT_TAGS);
readline(foo,tfst->tfst);
while (u_strcmp(foo->str,"f\n")) {
   SingleGraphState s=add_state(tfst->automaton);
   if (tfst->automaton->number_of_states==1) {
      /* By convention, the first state is initial */
      set_initial_state(s);
   }
   if (foo->str[0]=='t') {
      set_final_state(s);
   }
   else if (foo->str[0]!=':') {
      fatal_error("load_sentence: malformed state line:\n%S\n",foo->str);
   }
   pos=1;
   int tag,dest;
   while (2==u_sscanf(foo->str+pos,"%d %d%n",&tag,&dest,&shift)) {
      pos=pos+shift;
      add_outgoing_transition(s,tag,dest);
   }
   /* And we read the next line */
   readline(foo,tfst->tfst);
}
/* Now, we have to read the tags */
tfst->tags=new_vector_ptr(16);
readline(foo,tfst->tfst);
if (u_strcmp(foo->str,"@<E>\n")) {
   fatal_error("load_sentence: first tag should be the epsilon one\n");
}
vector_ptr_add(tfst->tags,read_epsilon_tag(tfst,foo));
readline(foo,tfst->tfst);
while (u_strcmp(foo->str,"f\n")) {
   if (!u_strcmp(foo->str,"@STD\n")) {
      vector_ptr_add(tfst->tags,read_normal_tag(tfst,foo));
   } else {
      fatal_error("load_sentence: Invalid tag type %S\n",foo->str);
   }
   /* And we read the next line */
   readline(foo,tfst->tfst);
}

/* We don't forget to free our useful Ustring */
free_Ustring(foo);
}


/**
 * Allocates, initializes and returns a TfstTag
 */
TfstTag* new_TfstTag(TfstTagType type) {
TfstTag* t=(TfstTag*)malloc(sizeof(TfstTag));
if (t==NULL) {
   fatal_alloc_error("new_TfstTag");
}
if (type!=T_EPSILON && type!=T_STD) {
   fatal_error("Invalid tag type in new_TfstTag\n");
}
t->type=type;
if (type==T_EPSILON) {
   t->content=u_strdup("<E>");
} else {
   t->content=NULL;
}
t->m.start_pos_in_token=-1;
t->m.start_pos_in_char=-1;
t->m.start_pos_in_letter=-1;
t->m.end_pos_in_token=-1;
t->m.end_pos_in_char=-1;
t->m.end_pos_in_letter=-1;
return t;
}


/**
 * Frees all the memory associated to the given TfstTag
 */
void free_TfstTag(TfstTag* t) {
if (t==NULL) return;
if (t->content!=NULL) free(t->content);
free(t);
}


/**
 * Dumps the given offset in the given, as an unsigned 4-byte
 * Little-Endian sequence.
 */
void dump_offset(long offset,U_FILE* tind) {
uint32_t n=(uint32_t)offset;
unsigned char t[4];
t[0]=(unsigned char)(n&0xFF);
t[1]=(unsigned char)((n&0xFF00) >> 8);
t[2]=(unsigned char)((n&0xFF0000) >> 16);
t[3]=(unsigned char)((n&0xFF000000) >> 24);
if (4!=fwrite(t,1,4,tind)) {
   fatal_error("dump_offset: Write error on .tind file\n");
}
}


/**
 * Saves the current sentence of the given tfst.
 * If 'tags' is not NULL, it is supposed to contain ready-to-dump tag labels that
 * will be used; otherwise, the function saves each TfstTag.
 *
 * The form_frequencies hash table is to be used when calling compute_form_frequencies.
 *
 * WARNING: if tags are provided, they are supposed to be \n terminated !
 */
void save_current_sentence(Tfst* tfst,U_FILE* out_tfst,U_FILE* out_tind,unichar** tags,int n_tags,
							struct hash_table* form_frequencies) {
if (tfst==NULL) {
   fatal_error("NULL tfst in save_current_sentence\n");
}
if (out_tfst==NULL) {
   fatal_error("NULL output .tfst file in save_current_sentence\n");
}
if (out_tind==NULL) {
   fatal_error("NULL output .tind file in save_current_sentence\n");
}
if (tfst->current_sentence==NO_SENTENCE_LOADED) {
   fatal_error("No sentence to save in save_current_sentence\n");
}
if (tags==NULL && tfst->tags==NULL) {
   fatal_error("No tag description in save_current_sentence\n");
}
if (tags!=NULL && n_tags<=0) {
   fatal_error("save_current_sentence: tag description array must have a size >0\n");
}
if (tags==NULL && tfst->tags->nbelems==0) {
   fatal_error("save_current_sentence: tag description vector must have a size >0\n");
}

/* We compute form frequencies */
if (form_frequencies!=NULL) {
	if (tags!=NULL) {
	   compute_form_frequencies(tfst->automaton,tags,n_tags,form_frequencies);
	} else {
	   compute_form_frequencies(tfst->automaton,(TfstTag**)(tfst->tags->tab),form_frequencies);
	}
}

/* First, we update the offset index in the .tind file */
long offset=ftell(out_tfst);
dump_offset(offset,out_tind);

/* Then we save the sentence automaton */
u_fprintf(out_tfst,"$%d\n",tfst->current_sentence);
/* its text the sentence automaton, */
u_fprintf(out_tfst,"%S\n",tfst->text);
/* We save the tokens */
u_fprintf(out_tfst,"%d/%d",tfst->tokens->tab[0],tfst->token_sizes->tab[0]);
for (int i=1;i<tfst->tokens->nbelems;i++) {
   u_fprintf(out_tfst," %d/%d",tfst->tokens->tab[i],tfst->token_sizes->tab[i]);
}
u_fprintf(out_tfst,"\n");
u_fprintf(out_tfst,"%d_%d\n",tfst->offset_in_tokens,tfst->offset_in_chars);
/* We save the states */
for (int i=0;i<tfst->automaton->number_of_states;i++) {
   if (is_final_state(tfst->automaton->states[i])) {
      u_fprintf(out_tfst,"t");
   } else {
      u_fprintf(out_tfst,":");
   }
   Transition* trans=tfst->automaton->states[i]->outgoing_transitions;
   while (trans!=NULL) {
      /* For each tag of the graph that is actually used, we put it in the main
       * tags and we use this index in the fst2 transition */
      u_fprintf(out_tfst," %d %d",trans->tag_number,trans->state_number);
      trans=trans->next;
   }
   u_fprintf(out_tfst,"\n");
}
u_fprintf(out_tfst,"f\n");
/* We save the tags */
if (tags!=NULL) {
   /* If there is a tag array, we use it */
   for (int i=0;i<n_tags;i++) {
      u_fprintf(out_tfst,"%S",tags[i]);
   }
} else {
   for (int i=0;i<tfst->tags->nbelems;i++) {
      TfstTag* t=(TfstTag*)(tfst->tags->tab[i]);
      unichar tmp[4096];
      TfstTag_to_string(t,tmp);
      u_fprintf(out_tfst,"%S",tmp);
   }
}
u_fprintf(out_tfst,"f\n");
}


/**
 * Builds a \n terminated string representation of the given TfstTag.
 */
void TfstTag_to_string(TfstTag* t,unichar* out) {
if (t==NULL) {
   fatal_error("NULL tag in TfstTag_to_string\n");
}
if (out==NULL) {
   fatal_error("NULL string in TfstTag_to_string\n");
}
if (t->type==T_EPSILON) {
   u_sprintf(out,"@<E>\n.\n");
   return;
}
if (t->type==T_STD) {
   int pos=0;
   int n=u_sprintf(out,"@STD\n@%S\n",t->content);
   pos=pos+n;
   if (t->m.start_pos_in_token<0 || t->m.start_pos_in_char<0 || t->m.start_pos_in_letter<0
         || t->m.end_pos_in_token<0 || t->m.end_pos_in_char<0 /*|| t->m.end_pos_in_letter<0*/) {
      /* We don't test t->m.end_pos_in_letter<0, because t->m.end_pos_in_letter==-1 is
       * the sign that the tag has an empty surface form like {<E>,.JO} */
      fatal_error("Invalid TfstTag information in TfstTag_to_string\n");
   }
   n=u_sprintf(out+pos,"@%d.%d.%d-%d.%d.%d\n",t->m.start_pos_in_token,t->m.start_pos_in_char,t->m.start_pos_in_letter,
                                              t->m.end_pos_in_token,t->m.end_pos_in_char,t->m.end_pos_in_letter);
   pos=pos+n;
   u_sprintf(out+pos,".\n");
   return;
}
fatal_error("Invalid tag type %d in TfstTag_to_string\n",t->type);
}


/**
 * This function fills the 'token_content' field of the given tfst
 */
void compute_token_contents(Tfst* t) {
if (t==NULL) {
	fatal_error("NULL error in compute_token_contents\n");
}
if (t->current_sentence==NO_SENTENCE_LOADED) {
	fatal_error("Cannot compute token contents when no sentence is loaded\n");
}
t->token_content=(unichar**)malloc(t->token_sizes->nbelems*sizeof(unichar*));
if (t->token_content==NULL) {
	fatal_alloc_error("compute_token_contents");
}
unichar tmp[4096];
int j,pos=0;
for (int i=0;i<t->token_sizes->nbelems;i++) {
	for (j=0;j<t->token_sizes->tab[i];j++) {
		tmp[j]=t->text[pos];
		pos++;
	}
	tmp[j]='\0';
	t->token_content[i]=u_strdup(tmp);
}
}

