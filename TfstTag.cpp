/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Tfst.h"
#include "File.h"
#include "Copyright.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "TfstTag.h"
#include "LocateMatches.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

static void process(Tfst* in,Tfst* out,struct match_list* matches);

const char* usage_TfstTag =
         "Usage: TfstTag [OPTIONS] <tfst>\n"
         "\n"
         "  <tfst>: the .tfst file that contains the text automaton.\n"
         "\n"
         "OPTIONS:\n"
         "  -i XXX/--ind_file=XXX: the .ind file to use\n"
         "  -o XXX/--output=XXX: output .tfst (by default, the given .tfst is modified)\n"
		 "  -h/--help: this help\n"
         "\n"
         "Adds transitions to the given .tfst. Transitions are taken from a .ind file\n"
		 "as the tags.ind produced by Dico.\n\n";


static void usage() {
display_copyright_notice();
u_printf(usage_TfstTag);
}


const char* optstring_TfstTag=":i:o:hk:q:";
const struct option_TS lopts_TfstTag[]= {
      {"ind_file",required_argument_TS,NULL,'i'},
      {"output",required_argument_TS,NULL,'o'},
      {"input_encoding",required_argument_TS,NULL,'k'},
      {"output_encoding",required_argument_TS,NULL,'q'},
      {"help",no_argument_TS,NULL,'h'},
      {NULL,no_argument_TS,NULL,0}
};


int main_TfstTag(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return 0;
}

char ind[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
int val,index=-1;
struct OptVars* vars=new_OptVars();
while (EOF!=(val=getopt_long_TS(argc,argv,optstring_TfstTag,lopts_TfstTag,&index,vars))) {
   switch(val) {
   case 'i': strcpy(ind,vars->optarg);
             break;
   case 'o': strcpy(output,vars->optarg);
             break;
   case 'k': if (vars->optarg[0]=='\0') {
                fatal_error("Empty input_encoding argument\n");
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),vars->optarg);
             break;
   case 'q': if (vars->optarg[0]=='\0') {
                fatal_error("Empty output_encoding argument\n");
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),vars->optarg);
             break;
   case 'h': usage(); return 0;
   case ':': if (index==-1) fatal_error("Missing argument for option -%c\n",vars->optopt);
             else fatal_error("Missing argument for option --%s\n",lopts_TfstTag[index].name);
   case '?': if (index==-1) fatal_error("Invalid option -%c\n",vars->optopt);
             else fatal_error("Invalid option --%s\n",vars->optarg);
             break;
   }
   index=-1;
}

if (ind[0]=='\0') {
   fatal_error("You must specify a .ind file\n");
}
U_FILE* f=u_fopen(&vec,ind,U_READ);
if (f==NULL) {
	fatal_error("Cannot load ind file %s\n",ind);
}
unichar header;
struct match_list* matches=load_match_list(f,NULL,&header);
u_fclose(f);
if (header!='X') {
	fatal_error("Invalid ind file %s\n",ind);
}

if (vars->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return 1;
}
char in_tfst[FILENAME_MAX];
char in_tind[FILENAME_MAX];
strcpy(in_tfst,argv[vars->optind]);
remove_extension(argv[vars->optind],in_tind);
strcat(in_tind,".tind");
char out_tfst[FILENAME_MAX];
char out_tind[FILENAME_MAX];
if (output[0]!='\0') {
	strcpy(out_tfst,output);
	remove_extension(output,out_tind);
	strcat(out_tind,".tind");
} else {
	strcpy(out_tfst,"tmp__.tfst");
	strcpy(out_tind,"tmp__.tind");
}
U_FILE* f_out_tfst=u_fopen(&vec,out_tfst,U_WRITE);
if (f_out_tfst==NULL) {
	fatal_error("Cannot create %s\n",out_tfst);
}
U_FILE* f_out_tind=u_fopen(&vec,out_tind,U_WRITE);
if (f_out_tind==NULL) {
	fatal_error("Cannot create %s\n",out_tind);
}

Tfst* input_tfst=open_text_automaton(&vec,argv[vars->optind]);
Tfst* output_tfst=new_Tfst(f_out_tfst,f_out_tind,input_tfst->N);
process(input_tfst,output_tfst,matches);
close_text_automaton(input_tfst);
close_text_automaton(output_tfst);
if (output[0]=='\0') {
	/* If we wanted to replace the given text automaton, we do it now */
	::remove(in_tfst);
	::remove(in_tind);
	::rename(out_tfst,in_tfst);
	::rename(out_tind,in_tind);
}
free_OptVars(vars);
u_printf("Done.\n");
return 0;
}


static void process_matches(Tfst* in,struct match_list* *matches) {
int sentence,start,end,pos;
int epsilon=0;
while (*matches!=NULL) {
	if ((*matches)->output==NULL) {
		fatal_error("Unexpected NULL output in process_matches\n");
	}
	if (3!=u_sscanf((*matches)->output,"%d %d %d:%n",&sentence,&start,&end,&pos)) {
		fatal_error("Invalid tagging output in process_matches:\n_%S_\n",(*matches)->output);
	}
	if (sentence!=in->current_sentence) break;
	unichar* s=(*matches)->output+pos;
	struct match_list* tmp=(*matches);
	if (!u_strcmp(s,"<E>")) {
		/* We add an epsilon transition */
		add_outgoing_transition(in->automaton->states[start],0,end);
		epsilon=1;
	} else {
		/* We have to create a new tag */
		TfstTag* tag=new_TfstTag(T_STD);
		tag->content=u_strdup(s);
		tag->m=tmp->m;
		int index=vector_ptr_add(in->tags,tag);
		add_outgoing_transition(in->automaton->states[start],index,end);
	}
	(*matches)=(*matches)->next;
	free_match_list_element(tmp);
}
if (epsilon) {
	remove_epsilon_transitions(in->automaton,0);
}
}


/**
 * Process all tagging matches to produce the output .tfst.
 * The match list is freed by this function.
 */
static void process(Tfst* in,Tfst* out,struct match_list* matches) {
u_fprintf(out->tfst,"%010d\n",out->N);
for (int i=1;i<=in->N;i++) {
	load_sentence(in,i);
	/* We reverse transitions to have an identical output if the sentence is
	 * not modified */
	reverse_transition_lists(in->automaton);
	process_matches(in,&matches);
	save_current_sentence(in,out->tfst,out->tind,NULL,0,NULL);
}
}

} // namespace unitex
