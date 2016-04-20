/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include <ctype.h>

#include "Ustring.h"
#include "Symbol.h"
#include "Fst2Automaton.h"
#include "ElagFstFilesIO.h"

#define MAXBUF  1024

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/**
 * Loads the tags of the given .fst2 file. Returns 0 in case of success; -1 otherwise.
 * Note that the position in the file is unchanged after a call to this function.
 */
int load_elag_fst2_tags(Elag_fst_file_in* fst) {
/* We backup the position in the file, and we come back at the
 * beginning of the file */
long fpos=ftell(fst->f);
rewind(fst->f);
/* Now, we go to the tags section, skipping all the automata */
unichar buf[MAXBUF];
int i=0;
int len;
while (i<fst->nb_automata) {
   if ((len=u_fgets(buf,MAXBUF,fst->f))==EOF) {
      error("load_fst_tags: %s: unexpected EOF\n",fst->name);
      return -1;
   }
   if (buf[0]=='f' && isspace(buf[1])) {
      i++;
   }
   /* If we have read the beginning of a long line, we skip the rest of the line */
   while ((len==MAXBUF-1) && (buf[len-1]!='\n')) {
      len=u_fgets(buf,MAXBUF,fst->f);
   }
}
Ustring* ustr=new_Ustring(64);
while (readline(ustr,fst->f) && ustr->str[0]!='f') {
   if (ustr->str[0]!='%' && ustr->str[0]!='@') {
      error("load_fst_tags: %s: bad symbol line: '%S'\n",fst->name,ustr->str);
      return -1;
   }
   /* +1 because we ignore the % or @ at the beginning of the line */
   symbol_t* symbol=load_grammar_symbol(fst->language,ustr->str+1);
   /* If 'symbol' is NULL, then an error message has already
    * been printed. Moreover, we want to associate NULL to the
    * string, so that we don't exit the function. Whatever it is,
    * we add the symbol to the symbols of the .fst2 */
   get_value_index(ustr->str+1,fst->symbols,INSERT_IF_NEEDED,symbol);
}
if (*ustr->str==0) {
   fatal_error("load_fst_tags: unexpected EOF\n");
}
free_Ustring(ustr);
/* We set back the position in the file */
fseek(fst->f,fpos,SEEK_SET);
return 0;
}


/**
 * Loads a .fst2 file with the given name and type, according to the
 * given language description.
 */
Elag_fst_file_in* load_elag_fst2_file(const VersatileEncodingConfig* vec,const char* fname,language_t* language) {
Elag_fst_file_in* fstf=(Elag_fst_file_in*)malloc(sizeof(Elag_fst_file_in));
if (fstf==NULL) {
   fatal_alloc_error("load_elag_fst2_file");
}
fstf->name=strdup(fname);
if (fstf->name==NULL) {
   fatal_alloc_error("load_elag_fst2_file");
}
if ((fstf->f=u_fopen(vec,fname,U_READ))==NULL) {
   error("load_fst_file: unable to open '%s' for reading\n",fname);
   goto error_fstf;
}
unichar buf[MAXBUF];
if (u_fgets(buf,MAXBUF,fstf->f)==EOF) {
   error("load_fst_file: '%s' is empty\n",fname);
   goto error_f;
}
if (!u_is_digit(*buf)) {
   error("load_fst_file: %s: bad file format\n",fname);
   goto error_f;
}
fstf->nb_automata=u_parse_int(buf);
fstf->language=language;
fstf->type=FST_GRAMMAR;
fstf->pos0=(int)ftell(fstf->f);
fstf->symbols=new_string_hash_ptr(64);
fstf->renumber=NULL;
if (load_elag_fst2_tags(fstf)==-1) {
   error("load_fst_file: %s: cannot load symbols\n",fstf->name);
   goto error_symbols;
}
fstf->pos=0;
return fstf;
/* If an error occurs */
error_symbols: free_string_hash_ptr(fstf->symbols,(void(*)(void*))free_symbols);

error_f: u_fclose(fstf->f);

error_fstf: free(fstf->name);

free(fstf);
return NULL;
}


/**
 * Closes the given file and frees the memory associated to the structure.
 */
void fst_file_close_in(Elag_fst_file_in* fstf) {
if (fstf==NULL) return;
if (fstf->name!=NULL) free(fstf->name);
u_fclose(fstf->f);
free_string_hash_ptr(fstf->symbols,(void(*)(void*))free_symbols);
if (fstf->renumber!=NULL) free(fstf->renumber);
free(fstf);
}


/**
 * Loads and returns an automaton from the given .fst2.
 * Returns NULL if there is no more automaton to load.
 */
Fst2Automaton* load_automaton(Elag_fst_file_in* fstf) {
if (fstf->pos>=fstf->nb_automata) {
   return NULL;
}
Ustring* ustr=new_Ustring();
readline(ustr,fstf->f);
const unichar* p=ustr->str;
if (p[0]!='-') {
   fatal_error("load_automaton: %s: bad file format\n",fstf->name);
}
p++;
int i=u_parse_int(p,&p);
if (i!=fstf->pos+1) {
   /* We make sure that the automaton number is what it should be */
   fatal_error("load_automaton: %s: parsing error with line '%S' ('-%d ...' expected)\n",fstf->name,ustr->str,fstf->pos+1);
}
/* Now p points on the automaton name */
p++;
Fst2Automaton* A=new_Fst2Automaton(p);
while (readline(ustr,fstf->f) && ustr->str[0]!='f') {
   /* If there is a state to read */
   p=ustr->str;
   SingleGraphState state=add_state(A->automaton);
   if (*p=='t') {
      /* If necessary, we set the state final */
      set_final_state(state);
   }
   /* We puts p on the first digit */
   while (*p!='\0' && !u_is_digit(*p)) {
      p++;
   }
   while (*p!='\0') {
      /* If there is a transition to read */
      int tag_number=u_parse_int(p,&p);
      if (fstf->renumber!=NULL) {
         tag_number=fstf->renumber[tag_number];
      }
      while (*p==' ') {
         p++;
      }
      if (!u_is_digit(*p)) {
         fatal_error("load_automaton: %s: bad file format (line='%S')\n",fstf->name,ustr->str);
      }
      int state_number=u_parse_int(p,&p);
      symbol_t* tmp=(symbol_t*)fstf->symbols->value[tag_number];
      if (tmp!=NULL) {
         /* If it is a good symbol (successfully loaded), we add transition(s) */
         if (fstf->type!=FST_TEXT) {
            add_all_outgoing_transitions(state,tmp,state_number);
         } else {
            /* In a text automaton, we add one transition per element of
             * the symbol list. For instance, if we have:
             *
             * tmp = "{domestique,.N:fs}" => "{domestique,.N:ms}" => NULL
             *
             * then we add two transitions. */
            add_all_outgoing_transitions(state,tmp,state_number);
         }
      }
      while (*p==' ') {
         p++;
      }
   }
}
if (*ustr->str=='\0') {
   fatal_error("load_automaton: unexpected end of file\n");
}
if (A->automaton->number_of_states==0) {
   error("load_automaton: automaton with no state\n");
} else {
   set_initial_state(A->automaton->states[0]);
}
fstf->pos++;
free_Ustring(ustr);
return A;
}


/**
 * This function sets the position in the given .fst2 immediately
 * before the nth automaton. For instance, if we have n=2, the
 * file position will be set at the beginning of the line "-2 .....".
 */
void fst_file_seek(Elag_fst_file_in* fstin,int n) {
if (n<=0 || n>fstin->nb_automata) {
   fatal_error("fst_file_seek(%d): automaton number should be in [1;%d]\n",n,fstin->nb_automata);
}
/* If necessary, we return at the beginning of the file */
if (n<fstin->pos) {
   fseek(fstin->f,fstin->pos0,SEEK_SET);
   fstin->pos=0;
}
unichar buf[MAXBUF];
int len;
while (fstin->pos<n-1) {
   if ((len=u_fgets(buf,MAXBUF,fstin->f))==EOF) {
      fatal_error("fst_file_seek: %s: unexpected EOF\n",fstin->name);
   }
   if (buf[0]=='f' && isspace(buf[1])) {
      fstin->pos++;
   }
   /* In case of a long line, we read the rest of the line */
   while ((len==MAXBUF-1) && (buf[len-1] !='\n')) {
      len=u_fgets(buf,MAXBUF,fstin->f);
   }
}
}


/**
 * Loads and returns the automaton #n in the given .fst2 file.
 * Note that n must be in [1;number of automata].
 */
Fst2Automaton* fst_file_autalmot_load(Elag_fst_file_in* fstin,int n) {
fst_file_seek(fstin,n);
return load_automaton(fstin);
}


/**
 * Opens a .fst2 file in output mode and returns the associated fst_file_out_t
 * structure, or NULL in case of error.
 */
Elag_fst_file_out* fst_file_out_open(const VersatileEncodingConfig* vec,const char* fname,int type) {
Elag_fst_file_out* res=(Elag_fst_file_out*)malloc(sizeof(Elag_fst_file_out));
if (res==NULL) {
   fatal_alloc_error("fst_file_out_open");
}
if (type<0 || type>=FST_BAD_TYPE) {
   fatal_error("fst_file_out_open: bad FST_TYPE\n");
}
if ((res->f=u_fopen(vec,fname,U_WRITE))==NULL) {
   error("fst_out_open: unable to open '%s'\n",fname);
   free(res);
   return NULL;
}
res->fstart=ftell(res->f);
u_fprintf(res->f,"0000000000\n");
res->name=strdup(fname);
if (res->name==NULL) {
   fatal_alloc_error("fst_file_out_open");
}
res->type=type;
res->nb_automata=0;
res->labels=new_string_hash(16);
/* We add <E> to the tags in order to be sure that this special tag will have #0 */
get_value_index(EPSILON,res->labels);
return res;
}


/**
 * Writes the fst's labels.
 */
void write_fst_tags(Elag_fst_file_out* fstout) {
for (int i=0;i<fstout->labels->size;i++) {
   u_fprintf(fstout->f,"%%%S\n",fstout->labels->value[i]);
}
u_fprintf(fstout->f,"f\n");
}


/**
 * Saves the labels of the given .fst2, closes the file
 * and frees the associated memory.
 */
void fst_file_close_out(Elag_fst_file_out* fstout) {
write_fst_tags(fstout);
fseek(fstout->f,fstout->fstart,SEEK_SET);
/* We print the number of automata on 10 digits */
u_fprintf(fstout->f,"%010d",fstout->nb_automata);
u_fclose(fstout->f);
free_string_hash(fstout->labels);
if (fstout->name!=NULL) free(fstout->name);
free(fstout);
}



void PNC_trans_write(Elag_fst_file_out * fstf, int to) {

  unichar label[4];
  int idx;

  label[1] = 0;

  for (const unichar * pnc = PUNC_TAB; *pnc; pnc++) {

    if (*pnc != '{') {
      label[0] = *pnc;

      idx=get_value_index(label,fstf->labels);
      u_fprintf(fstf->f, "%d %d ", idx, to);
    }
  }

}



void CHFA_trans_write(Elag_fst_file_out * fstf, int to) {

  unichar label[2];
  int idx;

  label[1] = 0;

  for (unichar C = '0'; C <= '9'; C++) {

    label[0] = C;

    idx=get_value_index(label,fstf->labels);

    u_fprintf(fstf->f, "%d %d ", idx, to);
  }
}



void LEXIC_trans_write(Elag_fst_file_out * fstf, int to) {

  unichar label[8];
  int idx;

  u_strcpy(label, "<MOT>");

  idx=get_value_index(label,fstf->labels);

  u_fprintf(fstf->f, "%d %d ", idx, to);

  u_strcpy(label, "<!MOT>");

  idx=get_value_index(label,fstf->labels);

  u_fprintf(fstf->f, "%d %d ", idx, to);
}


/**
 * Saves the given automaton into the given .fst2 file.
 */
void fst_file_write(Elag_fst_file_out* fstf,const Fst2Automaton* A) {
Ustring* tag=new_Ustring();
void (*symbol_to_tag)(const symbol_t*,Ustring*)=NULL;
switch (fstf->type) {
   case FST_TEXT:
      symbol_to_tag=symbol_to_text_label;
      break;

   case FST_GRAMMAR:
      symbol_to_tag=symbol_to_grammar_label;
      break;

  case FST_LOCATE:
      symbol_to_tag=symbol_to_locate_label;
      break;

  default:
      fatal_error("fst_file_write: invalid fstf->type: %d\n",fstf->type);
}
/* We save the graph number and name */
u_fprintf(fstf->f,"-%d %S\n",fstf->nb_automata+1,A->name);
int index;
unichar deflabel[]={'<','d','e','f','>',0};
for (int q=0;q<A->automaton->number_of_states;q++) {
   SingleGraphState state=A->automaton->states[q];
   u_fprintf(fstf->f,"%C ",is_final_state(state)?'t':':');
   for (Transition* t=state->outgoing_transitions;t!=NULL;t=t->next) {
      if (t->tag_number==-1) {
         /* If we are in the case of an "EMPTY" transition created because
          * the automaton was emptied as trim time */
         u_strcpy(tag,"EMPTY");
      } else {
         symbol_t* symbol=t->label;
         symbol_to_tag(symbol,tag);
      }
      if (fstf->type==FST_LOCATE) {
         /* If we are saving a Locate .fst2, we have to perform
          * some special things */
         if (u_strcmp(tag->str, "<PNC>") == 0) {
            PNC_trans_write(fstf, t->state_number);
         } else if (u_strcmp(tag->str, "<CHFA>") == 0 || u_strcmp(tag->str, "<NB>") == 0) {
            CHFA_trans_write(fstf, t->state_number);
         } else if (u_strcmp(tag->str, "<.>") == 0) {
            LEXIC_trans_write(fstf, t->state_number);
         } else {
            goto normal_output;
         }
      } else {
         /* If we have a normal transition to print */
         normal_output:
         index=get_value_index(tag->str,fstf->labels);
         u_fprintf(fstf->f,"%d %d ",index,t->state_number);
      }
   }
   if (state->default_state!=-1) {
      if (fstf->type!=FST_GRAMMAR) {
         error("Unexpected <def> label in text/locate automaton\n");
      }
      index=get_value_index(deflabel,fstf->labels);
      u_fprintf(fstf->f,"%d %d ",index,state->default_state);
   }
   u_fputc('\n',fstf->f);
}
u_fprintf(fstf->f,"f \n");
free_Ustring(tag);
fstf->nb_automata++;
}


/**
 * Loads and returns the first automaton of the given .fst2 file.
 */
Fst2Automaton* load_elag_grammar_automaton(const VersatileEncodingConfig* vec,const char* fst2,language_t* language) {
if (language==NULL) {
   fatal_error("NULL language error in load_elag_grammar_automaton\n");
}
Elag_fst_file_in* fstin=load_elag_fst2_file(vec,fst2,language);
if (fstin==NULL) {
   error("Unable to open '%s'\n", fst2);
   return NULL;
}
if (fstin->nb_automata!=1) {
   fatal_error("Elag grammar '%s' is not supposed to contain more than 1 automaton\n");
}
Fst2Automaton* A=fst_file_autalmot_load(fstin,1);
/* As we use the symbols of fstin in A, we must not free them here */
fst_file_close_in(fstin);
return A;
}


/**
 * Loads a .tfst file with the given name, according to the
 * given language description.
 */
Elag_Tfst_file_in* load_tfst_file(const VersatileEncodingConfig* vec,const char* fname,language_t* language) {
Elag_Tfst_file_in* fstf=(Elag_Tfst_file_in*)malloc(sizeof(Elag_Tfst_file_in));
if (fstf==NULL) {
   fatal_alloc_error("load_tfst_file");
}
fstf->tfst=open_text_automaton(vec,fname);
fstf->language=language;
return fstf;
}


/**
 * Removes transitions that are tagged by a NULL symbol.
 */
Transition* filter_NULL_symbols(Transition* t) {
if (t==NULL) return NULL;
if (t->label==NULL) {
   Transition* tmp=t->next;
   free_Transition(t,NULL);
   return filter_NULL_symbols(tmp);
}
t->next=filter_NULL_symbols(t->next);
return t;
}





/**
 * Loads the given sentence automaton and converts its transitions tagged with integers
 * into transitions tagged with symbol_t*
 */
void load_tfst_sentence_automaton(Elag_Tfst_file_in* input,int n) {
load_sentence(input->tfst,n);
input->tfst->automaton->tag_type=PTR_TAGS;
/* We must replace integer transitions by symbol transitions */
for (int i=0;i<input->tfst->automaton->number_of_states;i++) {
   SingleGraphState state=input->tfst->automaton->states[i];
   Transition* t=state->outgoing_transitions;
   while (t!=NULL) {
       /* We constructs the symbol list corresponding to the transition tag number */
      TfstTag* tag=(TfstTag*)input->tfst->tags->tab[t->tag_number];
      symbol_t* symbols=load_text_symbol(input->language,tag->content,t->tag_number);
      t->label=symbols;
      /* And we replace a single transition with a symbol list by several transitions
       * with one symbol each */
      if (t->label!=NULL) {
          while (t->label->next!=NULL) {
              Transition* foo=new_Transition_no_dup(t->label->next,t->state_number,t->next);
              t->label->next=NULL;
              t->next=foo;
              t=t->next;
          }
      }
      t=t->next;
   }
   state->outgoing_transitions=filter_NULL_symbols(state->outgoing_transitions);
}
/* Finally, we must indicate that now we deal a pointer tags */
input->tfst->automaton->tag_type=PTR_TAGS;
}



/**
 * Closes the given file and frees the memory associated to the structure.
 */
void tfst_file_close_in(Elag_Tfst_file_in* fstf) {
if (fstf==NULL) return;
close_text_automaton(fstf->tfst);
/* We MUST NOT free the language since we have not allocated it in load_tfst_file */
//free_language_t(fstf->language);
free(fstf);
}

} // namespace unitex
