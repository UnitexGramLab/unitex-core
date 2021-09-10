/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Alphabet.h"
#include "Copyright.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "Grf_lib.h"
#include "GrfBeauty.h"
#include "Seq2Grf.h"
#include "Ustring.h"
#include "String_hash.h"
#include "File.h"
#include "SingleGraph.h"
#include "Sentence2Grf.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {


int build_sequence_automaton(char* txt,char* grf,Alphabet* alph,int only_stop,int max_wildcards,
    int n_delete,int n_insert,int n_replace,int beautify,VersatileEncodingConfig* vec,
    int case_sensitive);

const char* usage_Seq2Grf =
    "Usage: Seq2Grf [OPTIONS] <txt>\n"
    "\n"
    "  <txt> : the sequence text file\n"
    "\n"
    "OPTIONS:\n"
    "  -a ALPH/--alphabet=ALPH: the alphabet file\n"
    "  -o XXX/--output=XXX: the output GRF file\n"
    "  -s/--only-stop : only consider {STOP}-separated sequences\n"
    "  -b/--beautify: apply the grf beautifying algorithm\n"
    "  -n/--no_beautify: do not apply the grf beautifying algorithm (default)\n"
    "  --case-sensitive: all letter tokens are protected with double-quotes (default)\n"
    "  --case-insensitive: letter tokens are not protected with double-quotes\n"
    "  -w n_wildcards\n"
    "  -i n_insertion\n"
    "  -r n_replace\n"
    "  -d n_delete\n"
    "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
    "  -h/--help: this help\n"
    "\n"
    "Constructs the sequence automaton : one single automaton that recognizes\n"
    "all the sequences from the given text file. The sequences must be delimited either\n"
    "with a newline or the special tag {STOP}. \n"
    ;

static void usage() {
  display_copyright_notice();
  u_printf(usage_Seq2Grf);
}

const char* optstring_Seq2Grf = ":a:o:i:r:d:Vhbnk:q:w:sm";
const struct option_TS lopts_Seq2Grf[] = {
  {"alphabet",required_argument_TS,    NULL, 'a' },
  {"output", required_argument_TS, NULL, 'o' },
  {"wildcards", required_argument_TS,NULL, 'w'},
  {"insert", required_argument_TS,NULL, 'i'},
  {"replace", required_argument_TS,NULL, 'r'},
  {"delete", required_argument_TS,NULL, 'd'},
  {"only-stop", no_argument_TS,NULL, 's'},
  {"beautify", no_argument_TS,NULL, 'b'},
  {"no_beautify", no_argument_TS,NULL, 'n'},
  {"input_encoding", required_argument_TS, NULL, 'k' },
  {"output_encoding", required_argument_TS, NULL, 'q' },
  {"case-sensitive", no_argument_TS, NULL, 1 },
  {"case-insensitive", no_argument_TS, NULL, 2 },
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help", no_argument_TS, NULL, 'h' },
  { NULL, no_argument_TS, NULL, 0 }
};

int main_Seq2Grf(int argc, char* const argv[]) {
if (argc==1) {
  usage();
  return SUCCESS_RETURN_CODE;
}
char alphabet[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
char foo;
int case_sensitive=1;
int max_wildcards=0,n_delete=0,n_replace=0,n_insert=0,only_stop=0,beautify=0;
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_Seq2Grf,lopts_Seq2Grf,&index))) {
  switch (val) {
    case 'a': {
      if (options.vars()->optarg[0] == '\0') {
        error("You must specify a non empty alphabet file name\n");
        return USAGE_ERROR_CODE;
      }
      strcpy(alphabet, options.vars()->optarg);
      break;
    }
    case 'o': {
      strcpy(output, options.vars()->optarg);
      break;
    }
    case 'w': {
      if (1!=sscanf(options.vars()->optarg,"%d%c",&max_wildcards,&foo)) {
        error("Invalid wildcards number argument: %s\n",options.vars()->optarg);
        return USAGE_ERROR_CODE;
      }
      break;
    }
    case 'i': {
      if (1!=sscanf(options.vars()->optarg,"%d%c",&n_insert,&foo)) {
        error("Invalid insertions argument: %s\n",options.vars()->optarg);
        return USAGE_ERROR_CODE;
      }
      break;
    }
    case 'r': {
      if (1!=sscanf(options.vars()->optarg,"%d%c",&n_replace,&foo)) {
        error("Invalid replacements argument: %s\n",options.vars()->optarg);
        return USAGE_ERROR_CODE;
      }
      break;
    }
    case 'd': {
      if (1!=sscanf(options.vars()->optarg,"%d%c",&n_delete,&foo)) {
        error("Invalid deletions argument: %s\n",options.vars()->optarg);
        return USAGE_ERROR_CODE;
      }
      break;
    }
    case 's': {
      only_stop=1;
      break;
    }
    case 'b': {
      beautify=1;
      break;
    }
    case 1: {
      case_sensitive=1;
      break;
    }
    case 2: {
      case_sensitive=0;
      break;
    }
    case 'n': {
      beautify=0;
      break;
    }
    case 'V': only_verify_arguments = true;
      break;
    case 'h': {
      usage();
      return SUCCESS_RETURN_CODE;
    }
    case 'k': {
      if (options.vars()->optarg[0] == '\0') {
        error("Empty input_encoding argument\n");
        return USAGE_ERROR_CODE;
      }
      decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input), options.vars()->optarg);
      break;
    }
    case 'q': {
      if (options.vars()->optarg[0] == '\0') {
        error("Empty output_encoding argument\n");
        return USAGE_ERROR_CODE;
      }
      decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output), options.vars()->optarg);
      break;
    }
    case ':': index == -1 ? error("Missing argument for option -%c\n", options.vars()->optopt) :
                            error("Missing argument for option --%s\n", lopts_Seq2Grf[index].name);
              return USAGE_ERROR_CODE;
    case '?': index == -1 ? error("Invalid option -%c\n", options.vars()->optopt) :
                            error("Invalid option --%s\n", options.vars()->optarg);
              return USAGE_ERROR_CODE;
  }
  index=-1;
}

if (options.vars()->optind!=argc-1) {
  error("Invalid arguments: rerun with --help\n");
  return USAGE_ERROR_CODE;
}

if (output[0]=='\0') {
  error("Error: you must specify the output .grf file\n");
  return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

Alphabet* alph=NULL;
if (alphabet[0]!='\0' ) {
  alph=load_alphabet(&vec,alphabet,0);
}

int return_value = build_sequence_automaton(argv[options.vars()->optind],output,alph,only_stop,max_wildcards,n_delete,
    n_insert,n_replace,beautify,&vec,case_sensitive);

free_alphabet(alph);
return return_value;
}


void read_letter_token(Ustring* tmp,unichar* line,unsigned int *i,Alphabet* alph,
    int case_sensitive) {
if (case_sensitive) {
  u_strcpy(tmp,"\"");
}
do {
  u_strcat(tmp,line[(*i)++]);
} while (is_letter(line[*i],alph));
if ('\''==line[*i]) {
  /* We want things like "l'accent" to be displayed in two boxes:   l'   accent */
  u_strcat(tmp,"'");
  (*i)++;
}
if (case_sensitive) {
  u_strcat(tmp,"\"");
}
}


void read_digit_token(Ustring* tmp,unichar* line,unsigned int *i) {
do {
  u_strcat(tmp,line[(*i)++]);
} while (u_is_digit(line[*i]));
}


void read_tag(Ustring* tmp,unichar* line,unsigned int *i) {
int backup=*i;
u_strcpy(tmp,"<");
while (line[*i]!='>' && line[*i]!='\0') {
  u_strcat(tmp,line[(*i)++]);
}
if (line[*i]=='\0') {
  /* The sequence may have contained a single < char, so we return this
   * char as a token */
  *i=backup+1;
  u_strcpy(tmp,"<");
  return;
}
u_strcat(tmp,">");
(*i)++;
}


const static char* special[]={"{S}",
    "<MIN>","<!MIN>",
    "<MAJ>","<!MAJ>",
    "<MOT>","<!MOT>",
    "<PRE>","<!PRE>",
    "<TOKEN>","<!TOKEN>",
    "<NB>",
    "<DIC>","<!DIC>",
    "<SDIC>","<!SDIC>",
    "<CDIC>","<!CDIC>",
    "<TDIC>","<!TDIC>",
    NULL
};


int read_special(vector_ptr* tokens,unichar* line,unsigned int *pos) {
for (int i=0;special[i]!=NULL;i++) {
  if (u_starts_with(line+(*pos),special[i])) {
    vector_ptr_add(tokens,u_strdup(special[i]));
    (*pos)+=(unsigned int)strlen(special[i]);
    return 1;
  }
}
return 0;
}


vector_ptr* tokenize_sequence(Ustring* line,Alphabet* alph,int case_sensitive) {
vector_ptr* tokens=new_vector_ptr(32);
Ustring* tmp=new_Ustring(128);
unsigned int i=0;
while (i!=line->len) {
  empty(tmp);
  if (is_letter(line->str[i],alph)) {
    read_letter_token(tmp,line->str,&i,alph,case_sensitive);
    vector_ptr_add(tokens,u_strdup(tmp->str));
  } else if (line->str[i]==' ' || line->str[i]=='\t'
      || line->str[i]=='\r' || line->str[i]=='\n') {
    /* We ignore separators */
    i++;
  } else if (read_special(tokens,line->str,&i)) {
    /* Nothing to do */
  } else if (line->str[i]=='<') {
    i++;
    read_tag(tmp,line->str,&i);
    vector_ptr_add(tokens,u_strdup(tmp->str));
  } else if (u_is_digit(line->str[i])) {
    read_digit_token(tmp,line->str,&i);
    vector_ptr_add(tokens,u_strdup(tmp->str));
  }
  else {
    /* We read a one-char token */
    u_strcat(tmp,line->str[i]);
    i++;
    vector_ptr_add(tokens,u_strdup(tmp->str));
  }
}
free_Ustring(tmp);
return tokens;
}


static unichar TOKEN[]={'<','T','O','K','E','N','>','\0'};

/**
 * Adds the given token sequence to the automaton
 */
void add_sequence(vector_ptr* seq,SingleGraph automaton,struct string_hash* tokens) {
int from_state=0,dest_state,token_number;
for (int i=0;i<seq->nbelems;i++) {
  dest_state=automaton->number_of_states;
  SingleGraphState s=add_state(automaton);
  if (i==(seq->nbelems-1)) {
    set_final_state(s);
  }
  token_number=get_value_index((unichar*)(seq->tab[i]),tokens);
  add_outgoing_transition(automaton->states[from_state],token_number,dest_state);
  from_state=dest_state;
}
}


/**
 * Adds the given token sequence to the automaton
 */
void add_sequence(unichar** seq,int size,SingleGraph automaton,struct string_hash* tokens) {
int from_state=0,dest_state,token_number;
for (int i=0;i<size;i++) {
  dest_state=automaton->number_of_states;
  SingleGraphState s=add_state(automaton);
  if (i==(size-1)) {
    set_final_state(s);
  }
  token_number=get_value_index((unichar*)(seq[i]),tokens);
  add_outgoing_transition(automaton->states[from_state],token_number,dest_state);
  from_state=dest_state;
}
}


/**
 * If there is less than 2 non <TOKEN> items and if the sequence length >=2,
 * then we consider that the sequence was too much transformed and so, we
 * return 0 to indicate that it should not be accepted; otherwise we return 1.
 */
int check_sequence(unichar* res[],int n,int size_origin) {
int n_tokens=0,n_lex;
for (int i=0;i<n;i++) {
  if (!u_strcmp(res[i],TOKEN)) {
    n_tokens++;
  }
}
n_lex=n-n_tokens;
if (n_lex<2 && size_origin>=2) return 0;
if (size_origin<=2 && n_tokens>0) return 0;
return 1;
}


void work(unichar* t[],int size,int current,int errors,int insert,int replace,int suppr,char last_op,
      unichar* res[],int pos_res,SingleGraph automaton,struct string_hash* tags) {
if (current==size) {
  /* We add the current sequence to the automaton, but only if it contains
   * enough non <TOKEN> items
   */
  if (!check_sequence(res,pos_res,size)) {
    return;
  }
  add_sequence(res,pos_res,automaton,tags);
  if (errors==0) {
    /* If we are done, we quit */
    return;
  }
  /* If we have reached the end of the array, we could only consider insertions,
   * but the specifications forbids this */
  return;
}
/* Normal case */
res[pos_res]=t[current];
work(t,size,current+1,errors,insert,replace,suppr,0,res,pos_res+1,automaton,tags);
/* Now, we consider errors */
if (errors==0) return;
if (insert!=0 && last_op!='S' && pos_res>0) {
  /* We refuse insertions before the sequence */
  res[pos_res]=TOKEN;
  work(t,size,current,errors-1,insert-1,replace,suppr,'I',res,pos_res+1,automaton,tags);
}
if (suppr!=0 && last_op!='I') {
  work(t,size,current+1,errors-1,insert,replace,suppr-1,'S',res,pos_res,automaton,tags);
}
if (replace!=0) {
  res[pos_res]=TOKEN;
  work(t,size,current+1,errors-1,insert,replace-1,suppr,'R',res,pos_res+1,automaton,tags);
}
}


void process_sequence(Ustring* line,Alphabet* alph,int max_wildcards,int n_delete,int n_insert,
    int n_replace,SingleGraph automaton,struct string_hash* tokens,
    int case_sensitive) {
vector_ptr* tok=tokenize_sequence(line,alph,case_sensitive);
unichar** variants=(unichar**)malloc(sizeof(unichar*)*(tok->nbelems+max_wildcards));
work((unichar**)(tok->tab),tok->nbelems,0,max_wildcards,n_insert,n_replace,n_delete,0,variants,0,automaton,tokens);
free_vector_ptr(tok,free);
free(variants);
}


int build_sequence_automaton(char* txt,char* output,Alphabet* alph,int only_stop,int max_wildcards,
    int n_delete,int n_insert,int n_replace,int do_beautify,VersatileEncodingConfig* vec,
    int case_sensitive) {
U_FILE* f=u_fopen(vec,txt,U_READ);
if (f==NULL) {
  error("Cannot open file %s\n",txt);
  return DEFAULT_ERROR_CODE;
}

Ustring* line=new_Ustring(1024);
Ustring* seq=new_Ustring(1024);
SingleGraph automaton=new_SingleGraph(INT_TAGS);
/* We force state #0 to be the initial state */
SingleGraphState initial=add_state(automaton);
set_initial_state(initial);
struct string_hash* tokens=new_string_hash(128);
get_value_index(EPSILON,tokens);
int first_on_this_line=0,line_start;
u_printf("Seq2Grf: reading sequence file...\n");

while (EOF!=readline(line,f)) {
  /* If there is {STOP} tag, it is always a sequence delimiter */
  line_start=0;
  for (unsigned int i=0;i<line->len;i++) {
    if (u_starts_with(line->str+i,"{STOP}")) {
      /* If there is a previous buffer, we append the current
       * part to the previous, with a \n if necessary
       */
      if (first_on_this_line) {
        if (seq->len!=0) {
          u_strcat(seq,"\n");
        }
        first_on_this_line=0;
      }
      line->str[i]='\0';
      u_strcat(seq,line->str+line_start);
      process_sequence(seq,alph,max_wildcards,n_delete,n_insert,n_replace,automaton,tokens,
          case_sensitive);
      empty(seq);
      i=i+5;
      line_start=i+1;
      continue;
    }
  }
  if (first_on_this_line) {
    if (seq->len!=0) {
      u_strcat(seq,"\n");
    }
    first_on_this_line=0;
  }
  u_strcat(seq,line->str+line_start);
  if (!only_stop) {
    process_sequence(seq,alph,max_wildcards,n_delete,n_insert,n_replace,automaton,tokens,
        case_sensitive);
    empty(seq);
  }
  first_on_this_line=1;
}
process_sequence(seq,alph,max_wildcards,n_delete,n_insert,n_replace,automaton,tokens,
    case_sensitive);

u_fclose(f);
free_Ustring(line);
free_Ustring(seq);

u_printf("Seq2Grf: minimizing graph...\n");
minimize(automaton,1);

Grf* grf=sentence_to_grf(automaton,tokens,12);
if (do_beautify) {
  u_printf("Seq2Grf: beautifying graph...\n");
  beautify(grf,alph);
}

U_FILE* f_output=u_fopen(vec,output,U_WRITE);
if (f_output==NULL) {
  error("Cannot create file %s\n",output);
  free_Grf(grf);
  free_string_hash(tokens);
  free_SingleGraph(automaton,NULL);
  return DEFAULT_ERROR_CODE;
}

save_Grf(f_output,grf);

u_fclose(f_output);
free_Grf(grf);
free_string_hash(tokens);
free_SingleGraph(automaton,NULL);
u_printf("Seq2Grf: done.\n");
return SUCCESS_RETURN_CODE;
}


} // namespace unitex
