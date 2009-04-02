/*
 * Unitex
 *
 * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Unicode.h"
#include "String_hash.h"
#include "Fst2.h"
#include "File.h"
#include "Copyright.h"
#include "IOBuffer.h"
#include "Error.h"
#include "Transitions.h"
#include "getopt.h"
#include "Tfst.h"
#include "Grf2Fst2.h"


void usage() {
   u_printf("%S", COPYRIGHT);
   u_printf("Usage: RebuildTfst <tfst>\n"
      "\n"
      "  <tfst>: text automaton to be rebuilt\n"
      "\n"
      "OPTIONS:\n"
      "  -h/--help: this help\n"
      "\n"
      "Rebuilds the text automaton taking into account sentence graphs that have\n"
      "been manually modified. The text automaton is modified.\n");
}



SingleGraph create_copy_of_fst2_subgraph(Fst2* fst2,int n);
unichar** create_tfst_tags(Fst2* fst2,int *n_tags);


int main(int argc,char* argv[]) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();

if (argc==1) {
   usage();
   return 0;
}

const char* optstring=":h";
const struct option lopts[]= {
   { "help", no_argument, NULL, 'h' }, 
   { NULL, no_argument, NULL, 0 }
};
int val, index=-1;
optind=1;
while (EOF!=(val=getopt_long(argc,argv,optstring,lopts,&index))) {
   switch (val) {
   case 'h':
      usage();
      return 0;
   case ':':
      if (index==-1)
         fatal_error("Missing argument for option -%c\n", optopt);
      else
         fatal_error("Missing argument for option --%s\n", lopts[index].name);
   case '?':
      if (index==-1)
         fatal_error("Invalid option -%c\n", optopt);
      else
         fatal_error("Invalid option --%s\n", optarg);
      break;
   }
   index=-1;
}

if (optind!=argc-1) {
   fatal_error("Invalid arguments: rerun with --help\n");
}

char input_tfst[FILENAME_MAX];
char input_tind[FILENAME_MAX];
strcpy(input_tfst,argv[optind]);
remove_extension(input_tfst,input_tind);
strcat(input_tind,".tind");

u_printf("Loading %s...\n",input_tfst);
Tfst* tfst = open_text_automaton(input_tfst);
if (tfst==NULL) {
   fatal_error("Unable to load %s automaton\n",input_tfst);
}
char basedir[FILENAME_MAX];
get_path(input_tfst,basedir);
char output_tfst[FILENAME_MAX];
sprintf(output_tfst, "%s.new.tfst",input_tfst);
char output_tind[FILENAME_MAX];
sprintf(output_tind, "%s.new.tind",input_tfst);

FILE* f_tfst;
if ((f_tfst = u_fopen(output_tfst, U_WRITE)) == NULL) {
   fatal_error("Unable to open %s for writing\n", output_tfst);
}
FILE* f_tind;
if ((f_tind = fopen(output_tind, U_WRITE)) == NULL) {
   u_fclose(f_tfst);
   fatal_error("Unable to open %s for writing\n", output_tind);
}
u_fprintf(f_tfst,"%010d\n",tfst->N);
for (int i = 1; i <= tfst->N; i++) {
   if ((i % 100) == 0) {
      u_printf("%d/%d sentences rebuilt...\n", i, tfst->N);
   }
   load_sentence(tfst,i);
   
   char grfname[FILENAME_MAX];
   sprintf(grfname, "%ssentence%d.grf", basedir, i);
   unichar** tags=NULL;
   int n_tags=-1;
   if (fexists(grfname)) {
      /* If there is a .grf for the current sentence, then we must 
       * take it into account */
      if (0==pseudo_main_Grf2Fst2(grfname,0,NULL,1,1)) {
         /* We proceed only if the graph compilation was a success */
         char fst2name[FILENAME_MAX];
         sprintf(fst2name, "%ssentence%d.fst2", basedir, i);
         Fst2* fst2=load_fst2(fst2name,0);
         remove(fst2name);
         free_SingleGraph(tfst->automaton,NULL);
         tfst->automaton=create_copy_of_fst2_subgraph(fst2,1);
         tags=create_tfst_tags(fst2,&n_tags);
         free_Fst2(fst2);
      } else {
         error("Error: %s is not a valid sentence automaton\n",grfname);
      }
   }
   save_current_sentence(tfst,f_tfst,f_tind,tags,n_tags);
   if (tags!=NULL) {
      /* If necessary, we free the tags we created */
      for (int i=0;i<n_tags;i++) {
         free(tags[i]);
      }
      free(tags);
   }
}
u_printf("Text automaton rebuilt.\n");
close_text_automaton(tfst);
u_fclose(f_tfst);
fclose(f_tind);

/* make a backup and replace old automaton with new */
char backup_tfst[FILENAME_MAX];
char backup_tind[FILENAME_MAX];
sprintf(backup_tfst,"%s.bck",input_tfst);
sprintf(backup_tind,"%s.bck",input_tind);
/* We remove the existing backup files, if any */
remove(backup_tfst);
remove(backup_tind);
rename(input_tfst,backup_tfst);
rename(input_tind,backup_tind);
rename(output_tfst,input_tfst);
rename(output_tind,input_tind);
u_printf("\nYou can find a backup of the original files in:\n    %s\nand %s\n",
         backup_tfst,backup_tind);
return 0;
}


/**
 * Creates a SingleGraph copy of the given .fst2 subgraph, using
 * the same tag numeration.
 */
SingleGraph create_copy_of_fst2_subgraph(Fst2* fst2,int n) {
int n_states=fst2->number_of_states_per_graphs[n];
SingleGraph g=new_SingleGraph(n_states,INT_TAGS);
int shift=fst2->initial_states[n];
for (int i=0;i<n_states;i++) {
   SingleGraphState dest=add_state(g);
   Fst2State src=fst2->states[i+shift];
   if (is_initial_state(src)) {
      set_initial_state(dest);
   }
   if (is_final_state(src)) {
      set_final_state(dest);
   }
   Transition* t=src->transitions;
   while (t!=NULL) {
      add_outgoing_transition(dest,t->tag_number,t->state_number);
      t=t->next;
   }
}
return g;
}


/**
 * Allocates, initializes and returns a string array containing ready-to-dump strings
 * corresponding to the given .fst2's tags.
 */
unichar** create_tfst_tags(Fst2* fst2,int *n_tags) {
if (fst2->states[0]->transitions==NULL) {
   /* If we have an empty sentence automaton, we just need the <E> tag */
   *n_tags=1;
} else {
   *n_tags=fst2->number_of_tags;
}
unichar** tags=(unichar**)malloc(sizeof(unichar*)*fst2->number_of_tags);
if (tags==NULL) {
   fatal_error("Not enough memory in create_tfst_tags\n");
}
tags[0]=u_strdup("@<E>\n.\n");
unichar tmp[4096];
TfstTag foo;
foo.type=T_STD;
for (int i=1;i<fst2->number_of_tags;i++) {
   foo.content=fst2->tags[i]->input;
   u_sscanf(fst2->tags[i]->output,"%d %d %d %d",&(foo.start_pos_token),&(foo.start_pos_char),
                                                &(foo.end_pos_token),&(foo.end_pos_char));
   TfstTag_to_string(&foo,tmp);
   tags[i]=u_strdup(tmp);
   if (tags[i]==NULL) {
      fatal_error("Not enough memory in create_tfst_tags\n");
   }
}
return tags;
}
