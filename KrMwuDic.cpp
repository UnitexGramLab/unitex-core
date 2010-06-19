/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Error.h"
#include "KrMwuDic.h"
#include "StringParsing.h"
#include "DELA.h"
#include "MF_SU_morphoBase.h"
#include "MF_SU_morpho.h"
#include "Vector.h"
#include "String_hash.h"


#define MAX_LINE_SIZE 4096
#define MAX_PARTS 32

void write_grf_start_things(U_FILE* grf,int *offset,int *start_state_offset,int *current_state,
                            int *end_state);
void write_grf_end_things(U_FILE* grf,int offset,int start_state_offset,int current_state,
                          vector_int* state_index);
int tokenize_kr_mwu_dic_line(unichar** part,unichar* line);
void produce_mwu_entries(U_FILE* grf,int n_parts,struct dela_entry** entries,MultiFlex_ctx* ctx,
                         Korean* korean,
                         struct l_morpho_t* morpho,
                         Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input,
                         vector_int* state_index,int *current_state,int end_state,int *line,
                         struct string_hash* subgraphs,int *subgraph_Y);


/**
 * Builds the .grf dictionary corresponding to the given Korean compound DELAS.
 */
void create_mwu_dictionary(U_FILE* delas,U_FILE* grf,MultiFlex_ctx* ctx,
                           Korean* korean,struct l_morpho_t* morpho,
                           Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input) {
unichar line[MAX_LINE_SIZE];
int size_line;
int line_number=-1;
unichar* part[MAX_PARTS];
struct dela_entry* entries[MAX_PARTS];
int n_parts;
int offset;
int current_state;
int start_state_offset;
int end_state;
write_grf_start_things(grf,&offset,&start_state_offset,&current_state,&end_state);
vector_int* state_index=new_vector_int(16);
struct string_hash* subgraphs=new_string_hash(DONT_USE_VALUES);
int line_grf=1;
int subgraph_Y=20;
while ((size_line=u_fgets(line,MAX_LINE_SIZE,delas))!=EOF) {
   /* We place the line counter here, so we can use 'continue' */
   line_number++;
   if (size_line==MAX_LINE_SIZE-1) {
      error("Line %d ignored because it is too long (>%d chars)\n",line_number,MAX_LINE_SIZE-1);
      int foo;
      /* We skip the remaining characters of the line */
      while ((foo=u_fgetc(delas))!=EOF && foo!='\n') {}
      if (foo==EOF) {
         /* If the failing line was the last one, we exit the main loop */
         break;
      }
      continue;
   }
   if (size_line>0) {
      if (line[size_line-1]=='\n') {
        line[size_line-1]='\0';
      }
   }
   if (line[0]=='\0') {
      continue;
   }
   /* We split the line */
   if ((n_parts=tokenize_kr_mwu_dic_line(part,line))==-1) {
      /* If there is an error, we ignore this line */
      continue;
   }
   /* Then we check that the n_parts-1 first ones are valid DELAF entries
    * and that the last one is a valid DELAS entry. But first, we initialize
    * the array */
   for (int i=0;i<n_parts;i++) {
      entries[i]=NULL;
   }
   int OK=1;
   for (int i=0;i<n_parts-1;i++) {
      entries[i]=tokenize_DELAF_line(part[i]);
      if (entries==NULL) {
         OK=0;
         break;
      }
   }
   if (OK) {
      entries[n_parts-1]=tokenize_DELAS_line(part[n_parts-1],NULL);
      OK=(entries[n_parts-1]!=NULL);
   }
   if (OK) {
      /* If everything went OK, we can start inflecting the root of the last
       * component */
      produce_mwu_entries(grf,n_parts,entries,ctx,korean,morpho,
            encoding_output,bom_output,mask_encoding_compatibility_input,state_index,
            &current_state,end_state,&line_grf,subgraphs,&subgraph_Y);
   }
   /* We free the 'part' and 'entries' tab*/
   for (int i=0;i<n_parts;i++) {
      free(part[i]);
      free_dela_entry(entries[i]);
   }
}
write_grf_end_things(grf,offset,start_state_offset,current_state,state_index);
free_vector_int(state_index);
free_string_hash(subgraphs);
}


/**
 * 'line' is supposed to be of the form '{AA}{BB}{CC}...'. This function
 * tries to split 'line' into as many parts as needed, without the round brackets.
 * The parts are stored in 'part'. The size of 'part' is returned.
 * For '{AA}{BB}{CC}', we have:
 *
 * part[0]="AA"   part[1]="BB"   part[2]="CC"   return value=3
 *
 * Returns -1 in case of error.
 */
int tokenize_kr_mwu_dic_line(unichar** part,unichar* line) {
if (line==NULL) {
   fatal_error("NULL error in tokenize_kr_mwu_dic_line\n");
}
if (line[0]=='\0') {
   fatal_error("Empty line error in tokenize_kr_mwu_dic_line\n");
}
int pos=0,n_parts=0,new_pos;
unichar temp[MAX_LINE_SIZE];
while (line[pos]!='\0') {
   if (line[pos]!='{') {
      error("Line should start with '{':\n%S\n",line);
      for (int i=0;i<n_parts;i++) {free(part[i]);}
      return -1;
   }
   pos++;
   new_pos=pos;
   int res=parse_string(line,&new_pos,temp,P_CLOSING_ROUND_BRACKET,P_EMPTY,NULL);
   if (res!=P_OK || line[new_pos]=='\0') {
      error("Invalid DELAS line:\n%S\n",line);
      for (int i=0;i<n_parts;i++) {free(part[i]);}
      return -1;
   }
   if (n_parts==MAX_PARTS) {
      error("Invalid DELAS line with too many components (>%d):\n%S\n",MAX_PARTS,line);
      for (int i=0;i<n_parts;i++) {free(part[i]);}
      return -1;
   }
   part[n_parts++]=u_strdup(temp);
   pos=new_pos+1;
}
if (n_parts==1) {
   error("Invalid DELAS line with only one component:\n%S\n",line);
   free(part[0]);
   return -1;
}
return n_parts;
}


/**
 * Adds the couple name/state in the given structure.
 */
void add_subgraph(struct string_hash* subgraphs,unichar* name,int state) {
subgraphs->size=state;
get_value_index(name,subgraphs);
}


/**
 * Returns the state number associated to the given graph name or -1 if
 * not found.
 */
int get_subgraph(struct string_hash* subgraphs,unichar* name) {
int n=get_value_index(name,subgraphs,DONT_INSERT);
if (n==NO_VALUE_INDEX) {
   /* NO_VALUE_INDEX==-1 but it's safer to do that in case the value ever changes */
   return -1;
}
return n;
}


/**
 * Takes a code of the form +XXXX and turns it into a YYYY sequence with
 * no + at the beginning and where all # have been replaced by :
 */
void get_post_position_graph_name(const unichar* code,unichar* name) {
code++;
int i=0;
while ((name[i]=code[i])!='\0') {
   if (name[i]=='#') {
      name[i]=':';
   }
   i++;
}
}


/**
 * Adds the given compound entries to the given grf.
 */
void produce_mwu_entries(U_FILE* grf,int n_parts,struct dela_entry** entries,MultiFlex_ctx* ctx,
                         Korean* korean,struct l_morpho_t* morpho,
                         Encoding encoding_output,int bom_output,int mask_encoding_compatibility_input,
                         vector_int* state_index,int *current_state,int end_state,int *line,
                         struct string_hash* subgraphs,int *subgraph_Y) {
SU_forms_T forms;
SU_init_forms(&forms); //Allocate the space for forms and initialize it to null values
char inflection_code[1024];
unichar code_gramm[1024];
int semitic;
/* We take the first grammatical code, and we extract from it the name
 * of the inflection transducer to use */
get_inflection_code(entries[n_parts-1]->semantic_codes[0],
                    inflection_code, code_gramm, &semitic);
/* And we inflect the word */
const char* pkgdir="";
SU_inflect(ctx,morpho,encoding_output,bom_output,mask_encoding_compatibility_input,
      entries[n_parts-1]->lemma,inflection_code,
      entries[n_parts-1]->filters, &forms, semitic, korean, pkgdir);
if (forms.no_forms==0) {
   /* If no form was generated, we have nothing to do */
   SU_delete_inflection(&forms);
   return;
}
/* We have to save the first n_parts-1 components in the graph. We also
 * have to add the first state of the path in the 'state_index' vector */
vector_int_add(state_index,*current_state);
/* line is an approximation for a row coordinate in the grf */
unichar inflected_jamo[1024];
int foo_offset=-1;
for (int i=0;i<n_parts-1;i++) {
   /* We convert all inflected forms to Jamo, in order to speed up
    * dictionary lookup */
   Hanguls_to_Jamos(entries[i]->inflected,inflected_jamo,korean,0);
   u_fprintf(grf,"\"%S\" %d %d 1 %d \n",inflected_jamo,200+i*500,20+(*line)*50,(*current_state)+1);
   (*current_state)++;
   u_fprintf(grf,"\"<E>/%S,%S.%S",entries[i]->inflected,
                               entries[i]->lemma,entries[i]->semantic_codes[0]);
   for (int j=1;j<entries[i]->n_semantic_codes;j++) {
      u_fprintf(grf,"+%S",entries[i]->semantic_codes[j]);
   }
   for (int j=0;j<entries[i]->n_inflectional_codes;j++) {
      u_fprintf(grf,":%S",entries[i]->inflectional_codes[j]);
   }
   if (i<n_parts-2) {
      u_fprintf(grf,"}{\" %d %d 1 %d \n",400+i*500,20+(*line)*50,(*current_state)+1);
   } else {
      /* If this is the n_parts-2 component, we may have to generate several outputs
       * transitions, so we use the offset+0000000000 trick again */
      u_fprintf(grf,"}{\" %d %d %d ",400+i*500,20+(*line)*50,forms.no_forms);
      foo_offset=ftell(grf);
      for (int k=0;k<forms.no_forms;k++) {
         u_fprintf(grf,"0000000000 ");
      }
      u_fprintf(grf,"\n");
   }
   (*current_state)++;
}

/* Now, we have to produce all the forms generated from the root inflection process */
int x=n_parts-1;
for (int i = 0; i < forms.no_forms; i++,(*line)++) {
   (*current_state)++;
   Hanguls_to_Jamos(forms.forms[i].form,inflected_jamo,korean,0);
   u_fprintf(grf,"\"%S\" %d %d 1 %d \n",inflected_jamo,200+x*500,20+(*line)*50,(*current_state));
   /* We must backtrack to add the transition to this state */
   fseek(grf,foo_offset,SEEK_SET);
   u_fprintf(grf,"%010d ",(*current_state)-1);
   /* We note the new offset and we return at the end of the file */
   foo_offset=ftell(grf);
   fseek(grf,0,SEEK_END);

   (*current_state)++;
   u_fprintf(grf,"\"<E>/%S,%S.%S", forms.forms[i].form,
                                  entries[n_parts-1]->lemma, code_gramm);
   /* We add the semantic codes, if any */
   for (int j = 1; j < entries[n_parts-1]->n_semantic_codes; j++) {
      u_fprintf(grf, "+%S", entries[n_parts-1]->semantic_codes[j]);
   }
   /* We may not want to output this information */
   unichar* code=forms.forms[i].local_semantic_code;
   if (code != NULL && code[0]!='\0') {
      if (code[0]!='+') {
         fatal_error("Invalid code %S produced by inflection grammar %s: should start with a +\n",code,inflection_code);
      }
      u_fprintf(grf, "%S", code);
   } else {
      fatal_error("Error in inflection grammar %s: it does not produce a +XXX code \n",inflection_code);
   }
   if (forms.forms[i].raw_features != NULL
         && forms.forms[i].raw_features[0] != '\0') {
      u_fprintf(grf, ":%S", forms.forms[i].raw_features);
   }
   /* Now, we must get the number of the state corresponding to the
    * post-position graph */
   int subgraph_state=get_subgraph(subgraphs,code);
   if (subgraph_state==-1) {
      /* If this is the first time we see this graph, we add it */
      subgraph_state=(*current_state);
      (*current_state)++;
      add_subgraph(subgraphs,code,subgraph_state);
      u_fprintf(grf, "}\" %d %d 1 %d \n",400+x*500,20+(*line)*50,subgraph_state);
      unichar graph_name[FILENAME_MAX];
      get_post_position_graph_name(code,graph_name);
      u_fprintf(grf, "\":%S\" 3600 %d 1 %d \n",graph_name,(*subgraph_Y),end_state);
      (*subgraph_Y)=(*subgraph_Y)+50;
   } else {
      u_fprintf(grf, "}\" %d %d 1 %d \n",400+x*500,20+(*line)*50,subgraph_state);
   }
}
SU_delete_inflection(&forms);

}


/**
 * Writes the grf header and somes states. '*offset' is used to remember where
 * in the file is to be written the number of states that we don't know yet.
 * We will temporarily write ten zeros that should be overwritten at the end of the
 * graph creation process. We use the same trick for the state that will contain
 * all transitions to all created paths. As this state will contain as many transitions
 * as compound entries, we can't create it right now. We will create it at the end, but
 * we will need to backtrack in the file to write its actual number the unique
 * output of state #2. This is why we use '*start_state_offset'.
 */
void write_grf_start_things(U_FILE* grf,int *offset,int *start_state_offset,int *current_state,int *end_state) {
u_fprintf(grf,"#Unigraph\n");
u_fprintf(grf,"SIZE 4000 840\n");
u_fprintf(grf,"FONT Haansoft Batang:  10\n");
u_fprintf(grf,"OFONT Haansoft Batang:B 10\n");
u_fprintf(grf,"BCOLOR 16777215\n");
u_fprintf(grf,"FCOLOR 0\n");
u_fprintf(grf,"ACOLOR 13487565\n");
u_fprintf(grf,"SCOLOR 255\n");
u_fprintf(grf,"CCOLOR 255\n");
u_fprintf(grf,"DBOXES y\n");
u_fprintf(grf,"DFRAME y\n");
u_fprintf(grf,"DDATE y\n");
u_fprintf(grf,"DFILE y\n");
u_fprintf(grf,"DDIR n\n");
u_fprintf(grf,"DRIG n\n");
u_fprintf(grf,"DRST n\n");
u_fprintf(grf,"FITS 100\n");
u_fprintf(grf,"PORIENT L\n");
u_fprintf(grf,"#\n");
(*offset)=ftell(grf);
u_fprintf(grf,"0000000000\n");
u_fprintf(grf,"\"<E>\" 70 20 1 2 \n");
u_fprintf(grf,"\"\" 3800 20 0 \n");
u_fprintf(grf,"\"$<\" 105 20 1 ");
(*start_state_offset)=ftell(grf);
u_fprintf(grf,"0000000000 \n");
/*
 * We don't write this one, since it must written as the last state of the grf,
 * in 'write_grf_end_things'
 *
 * u_fprintf(grf,"\"<E>//{\" 152 200 0 \n");*/
u_fprintf(grf,"\"$>\" 3750 20 1 1 \n");
(*end_state)=3;
(*current_state)=4;
}


/**
 * We create the state that will have transitions to all states specified in the
 * 'state_index' vector. We add it as the last state of the grf, and then, we
 * go back in the file to write the number of states and the index of this last state.
 */
void write_grf_end_things(U_FILE* grf,int offset,int start_state_offset,int current_state,
                          vector_int* state_index) {
u_fprintf(grf,"\"<E>//{\" 152 20 %d ",state_index->nbelems);
for (int i=0;i<state_index->nbelems;i++) {
   u_fprintf(grf,"%d ",state_index->tab[i]);
}
u_fprintf(grf,"\n");
fseek(grf,offset,SEEK_SET);
u_fprintf(grf,"%010d",current_state+1);
fseek(grf,start_state_offset,SEEK_SET);
u_fprintf(grf,"%010d",current_state);
/* For safety, we return at the end of the file */
fseek(grf,0,SEEK_END);
}
