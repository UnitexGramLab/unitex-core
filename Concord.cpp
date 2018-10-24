/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "Text_tokens.h"
#include "String_hash.h"
#include "List_int.h"
#include "Alphabet.h"
#include "Concordance.h"
#include "File.h"
#include "Copyright.h"
#include "LocatePattern.h"
#include "Error.h"
#include "Snt.h"
#include "Copyright.h"
#include "UnitexGetOpt.h"
#include "ProgramInvoker.h"
#include "Concord.h"
#include "Offsets.h"
#include "PRLG.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_Concord =
    "Usage: Concord [OPTIONS] <concord>\n"
    "\n"
    "  <concord>: a concord.ind file generated by the locate program\n"
    "\n"
    "OPTIONS:\n"
    "  -f FONT/--font=FONT: the name of the font to use if the output is an HTML file\n"
    "  -s N/--fontsize=N: the font size to use if the output is an HTML file\n"
    "  -l X/--left=X: left context length in chars; if followed by s (i.e. \"80s\") indicates\n"
    "                 that the match can end before the limit if there is a {S}. Default=0\n"
    "  -r X/--right=X: the same for right context. Default=0\n"
    "  --only_matches: this option will force empty left and right contexts. Moreover, if\n"
    "                  used with -t/--text, Concord will not surround matches with tabulations\n"
    "  --only_ambiguous: Only displays identical occurrences with ambiguous outputs\n"
    "                    Note: this option forces sort to be in text order, except in combination\n"
    "                          with option --lemmatize.\n"
    "\n"
    "Sort order options:\n"
    "  --TO: text order (default)\n"
    "  --LC: left, center\n"
    "  --LR: left, right\n"
    "  --CL: center, left\n"
    "  --CR: center, right\n"
    "  --RL: right, left\n"
    "  --RC: right, center\n"
    "\n"
    "Output options:\n"
    "  -H/--html: produces an HTML concordance file (default)\n"
    "  -t=X/--text=X: produces a plain text concordance file named X. If X is\n"
    "                 omitted, the result is stored into concord.txt\n"
    "  -g SCRIPT/--glossanet=SCRIPT: produces a glossanet HTML concordance file\n"
    "  -p SCRIPT/--script=SCRIPT: produces a HTML concordance file where occurrences\n"
    "                             are links described by SCRIPT\n"
    "  -i/--index: produces an index of the concordance\n"
    "  -u offsets/--uima=offsets: produces an index of the concordance relative to the\n"
    "                             original text file, before any Unitex operation. offsets\n"
    "                             is supposed to be the file produced by Tokenize's\n"
    "                             --output_offsets option. If 'offsets' is omitted,\n"
    "                             the program acts as for the -i option, adding the ending\n"
    "                             position of the match.\n"
    "  -e/--xml: produces xml index of the concordance\n"
    "  -w/--xml-with-header: produces xml index of the concordance with header\n"
    "  -X/--escape-matches: escape xml matches\n"
    "  --lemmatize: produces a special HTML concordance used by the lemmatization interface\n"
    "               in the GUI.\n"
    "  NOTE: both -e and -w options accepts an offset file, as -u does\n"
    "  --input_offsets=XXX: base offset file to be used. Uses with --merge=YYY\n"
    "  --output_offsets=XXX: offset file to be produced. Uses with --merge=YYY\n"
    "\n"
    "  --PRLG=X,Y: produces a concordance for PRLG corpora where each line is prefixed\n"
    "              by information extracted with Unxmlize's --PRLG option. X is the\n"
    "              file produced by Unxmlize's --PRLG option and Y is the file produced\n"
    "              by Tokenize's --output_offsets option. Note that if this option is\n"
    "              used in addition with -u, the Y argument overrides the argument of -u\n"
    "  -A/--axis: produces an axis file for the concordance (cf. [Melamed 06])\n"
    "  -x/--xalign: produces an index file for XAlign display\n"
    "  -m TXT/--merge=TXT: produces a file named TXT which is the SNT file\n"
    "                      merged with the match results\n"
    "  --export_csv: produce a tab-separated export.csv file in text order of the following format:\n"
    "\n"
    "                A B C D E F, where:\n"
    "\n"
    "                A=number of the line in the .csv file\n"
    "                B=number of the sentence\n"
    "                C=PRLG reference, if any\n"
    "                D=the inflected form as found in the text\n"
    "                E=the lemma, if any\n"
    "                F=the codes, if any\n"
    "                To work, this option must be invoked for special concord.ind files, that\n"
    "                are supposed to contain every text token that is neither a {S} nor a space.\n"
    "\n"
    "  -d DIR/--directory=DIR: does not work in the same directory than <concord> but in DIR\n"
    "  -a ALPH/--alphabet=ALPH : the char order file used for sorting\n"
    "  -T/--thai: option to use for Thai concordances\n"
    "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
    "  -h/--help: this help\n"
    "\n"
    "Extracts the matches stored in <concord>, and stores them into a UTF-8\n"
    "HTML file saved in the <concord> directory or produces a text file, according\n"
    "to the mode parameter\n"
    "\nExamples:\n"
    "Concord tutu.ind \"-fCourier New\" -s12 -l40 -r40 --TO --html \"-ad:\\My dir\\alph.txt\"\n"
    "    -> produces an HTML file\n"
    "\n"
    "Concord tutu.ind \"--merge=C:\\My dir\\RES.SNT\"\n"
    "    -> produces a text file named \"C:\\My dir\\RES.SNT\"\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_Concord);
}


int pseudo_main_Concord(const VersatileEncodingConfig* vec,
                        const char* index_file,const char* font,int fontsize,
                        int left_context,int right_context,const char* sort_order,
                        const char* output,const char* directory,const char* alphabet,
                        int thai,int only_ambiguous,int only_matches) {
ProgramInvoker* invoker=new_ProgramInvoker(main_Concord,"main_Concord");
char tmp[256];
{
    tmp[0]=0;
    get_reading_encoding_text(tmp,sizeof(tmp)-1,vec->mask_encoding_compatibility_input);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-k");
        add_argument(invoker,tmp);
    }

    tmp[0]=0;
    get_writing_encoding_text(tmp,sizeof(tmp)-1,vec->encoding_output,vec->bom_output);
    if (tmp[0] != '\0') {
        add_argument(invoker,"-q");
        add_argument(invoker,tmp);
    }
}
if (font!=NULL) {
   add_argument(invoker,"-f");
   add_argument(invoker,font);
   sprintf(tmp,"%d",fontsize);
   add_argument(invoker,"-s");
   add_argument(invoker,tmp);
}
sprintf(tmp,"%d",left_context);
add_argument(invoker,"-l");
add_argument(invoker,tmp);
sprintf(tmp,"%d",right_context);
add_argument(invoker,"-r");
add_argument(invoker,tmp);
if (sort_order==NULL) {
   add_argument(invoker,"--TO");
} else {
   add_argument(invoker,sort_order);
}
add_argument(invoker,output);
if (directory!=NULL) {
   add_argument(invoker,"-d");
   add_argument(invoker,directory);
}
if (alphabet!=NULL) {
   add_argument(invoker,"-a");
   add_argument(invoker,alphabet);
}
if (thai) {
   add_argument(invoker,"-T");
}
if (only_ambiguous) {
   add_argument(invoker,"--only_ambiguous");
}
if (only_matches) {
   add_argument(invoker,"--only_matches");
}
add_argument(invoker,index_file);
int ret=invoke(invoker);
free_ProgramInvoker(invoker);
return ret;
}


const char* optstring_Concord=":f:s:l:r:Ht::e::w::g:p:iu::Axm:a:Td:VLXh$:@:k:q:";
const struct option_TS lopts_Concord[]= {
  {"font",required_argument_TS,NULL,'f'},
  {"fontsize",required_argument_TS,NULL,'s'},
  {"left",required_argument_TS,NULL,'l'},
  {"right",required_argument_TS,NULL,'r'},
  {"only_ambiguous",no_argument_TS,NULL,8},
  {"only_matches",no_argument_TS,NULL,10},
  {"lemmatize",no_argument_TS,NULL,11},
  {"export_csv",no_argument_TS,NULL,12},
  {"TO",no_argument_TS,NULL,0},
  {"LC",no_argument_TS,NULL,1},
  {"LR",no_argument_TS,NULL,2},
  {"CL",no_argument_TS,NULL,3},
  {"CR",no_argument_TS,NULL,4},
  {"RL",no_argument_TS,NULL,5},
  {"RC",no_argument_TS,NULL,6},
  {"html",no_argument_TS,NULL,'H'},
  {"text",optional_argument_TS,NULL,'t'},
  {"xml",optional_argument_TS,NULL,'e'},
  {"xml-with-header",optional_argument_TS,NULL,'w'},
  {"glossanet",required_argument_TS,NULL,'g'},
  {"script",required_argument_TS,NULL,'p'},
  {"index",no_argument_TS,NULL,'i'},
  {"uima",optional_argument_TS,NULL,'u'},
  {"escape-matches",no_argument_TS,NULL,'X'},
  {"axis",no_argument_TS,NULL,'A'},
  {"xalign",no_argument_TS,NULL,'x'},
  {"diff",no_argument_TS,NULL,7},
  {"merge",required_argument_TS,NULL,'m'},
  {"alphabet",required_argument_TS,NULL,'a'},
  {"thai",no_argument_TS,NULL,'T'},
  {"directory",required_argument_TS,NULL,'d'},
  {"PRLG",required_argument_TS,NULL,9},
  {"no_convert_lf_to_crlf",no_argument_TS,NULL,'L'},
  {"output_offsets",required_argument_TS,NULL,'@'},
  {"input_offsets",required_argument_TS,NULL,'$'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help",no_argument_TS,NULL,'h'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {NULL,no_argument_TS,NULL,0}
};

/**
 * The same than main, but no call to setBufferMode.
 */
int main_Concord(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

int val,index=-1;
struct conc_opt* concord_options = new_conc_opt();
char foo;
VersatileEncodingConfig vec=VEC_DEFAULT;
int ret;
char offset_file[FILENAME_MAX]="";
char PRLG[FILENAME_MAX]="";
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_Concord,lopts_Concord,&index))) {
   switch(val) {
   case 'f': if (options.vars()->optarg[0]=='\0') {
                error("Empty font name argument\n");
                free_conc_opt(concord_options);
                return USAGE_ERROR_CODE;
             }
             concord_options->fontname=strdup(options.vars()->optarg);
             if (concord_options->fontname==NULL) {
                alloc_error("main_Concord");
                free_conc_opt(concord_options);
                return ALLOC_ERROR_CODE;
             }
             break;
   case 's': if (1!=sscanf(options.vars()->optarg,"%d%c",&(concord_options->fontsize),&foo)) {
                /* foo is used to check that the font size is not like "45gjh" */
                error("Invalid font size argument: %s\n",options.vars()->optarg);
                free_conc_opt(concord_options);
                return USAGE_ERROR_CODE;
             }
             break;
   case 'l': ret=sscanf(options.vars()->optarg,"%d%c%c",&(concord_options->left_context),&foo,&foo);
             if (ret==0 || ret==3 || (ret==2 && foo!='s') || concord_options->left_context<0) {
                error("Invalid left context argument: %s\n",options.vars()->optarg);
                free_conc_opt(concord_options);
                return USAGE_ERROR_CODE;
             }
             if (ret==2) {
                concord_options->left_context_until_eos=1;
             }
             break;
   case 'r': ret=sscanf(options.vars()->optarg,"%d%c%c",&(concord_options->right_context),&foo,&foo);
             if (ret==0 || ret==3 || (ret==2 && foo!='s') || concord_options->right_context<0) {
                error("Invalid right context argument: %s\n",options.vars()->optarg);
                free_conc_opt(concord_options);
                return USAGE_ERROR_CODE;
             }
             if (ret==2) {
                concord_options->right_context_until_eos=1;
             }
             break;
   case 'L': concord_options->convLFtoCRLF=0; break;
   case 'X': concord_options->escape_matches=1; break;
   case 0: concord_options->sort_mode=TEXT_ORDER; break;
   case 1: concord_options->sort_mode=LEFT_CENTER; break;
   case 2: concord_options->sort_mode=LEFT_RIGHT; break;
   case 3: concord_options->sort_mode=CENTER_LEFT; break;
   case 4: concord_options->sort_mode=CENTER_RIGHT; break;
   case 5: concord_options->sort_mode=RIGHT_LEFT; break;
   case 6: concord_options->sort_mode=RIGHT_CENTER; break;
   case 7: concord_options->result_mode=DIFF_; break;
   case 8: concord_options->only_ambiguous=1; break;
   case 9: {
     strcpy(PRLG,options.vars()->optarg);
     char* pos=strchr(PRLG,',');
     if (pos==NULL || pos==PRLG || *(pos+1)=='\0') {
       error("Invalid argument for option --PRLG: %s\n",options.vars()->optarg);
       free_conc_opt(concord_options);
       return USAGE_ERROR_CODE;
     }
     *pos='\0';
     strcpy(offset_file,pos+1);
     break;
   }
   case 10: concord_options->only_matches=1; break;
   case 11: concord_options->result_mode=LEMMATIZE_; break;
   case 12: concord_options->result_mode=CSV_; break;
   case 'H': concord_options->result_mode=HTML_; break;
   case 't': {
     concord_options->result_mode=TEXT_;
     if (options.vars()->optarg!=NULL) {
       strcpy(concord_options->output,options.vars()->optarg);
     }
     break;
   }
   case 'g': concord_options->result_mode=GLOSSANET_;
             if (options.vars()->optarg[0]=='\0') {
                error("Empty glossanet script argument\n");
                free_conc_opt(concord_options);
                return USAGE_ERROR_CODE;
             }
             concord_options->script=strdup(options.vars()->optarg);
             if (concord_options->script==NULL) {
                alloc_error("main_Concord");
                free_conc_opt(concord_options);
                return ALLOC_ERROR_CODE;
             }
             break;
   case 'p': concord_options->result_mode=SCRIPT_;
             if (options.vars()->optarg[0]=='\0') {
                error("Empty script argument\n");
                free_conc_opt(concord_options);
                return USAGE_ERROR_CODE;
             }
             concord_options->script=strdup(options.vars()->optarg);
             if (concord_options->script==NULL) {
                alloc_error("main_Concord");
                free_conc_opt(concord_options);
                return ALLOC_ERROR_CODE;
             }
             break;
   case 'i': concord_options->result_mode=INDEX_; break;
   case 'u': concord_options->result_mode=UIMA_;
             if (options.vars()->optarg!=NULL) {
                strcpy(offset_file,options.vars()->optarg);
             }
             concord_options->original_file_offsets=1;
             break;
   case 'e': concord_options->result_mode=XML_;
             if (options.vars()->optarg!=NULL) {
                strcpy(offset_file, options.vars()->optarg);
                concord_options->original_file_offsets=1;
             }
             break;
   case 'w': concord_options->result_mode=XML_WITH_HEADER_;
             if (options.vars()->optarg!=NULL) {
                strcpy(offset_file, options.vars()->optarg);
                concord_options->original_file_offsets = 1;
             }
             break;
   case '$': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_offsets argument\n");
                free_conc_opt(concord_options);
                return USAGE_ERROR_CODE;
             }
             strcpy(concord_options->input_offsets,options.vars()->optarg);
             break;
   case '@': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_offsets argument\n");
                free_conc_opt(concord_options);
                return USAGE_ERROR_CODE;
             }
             strcpy(concord_options->output_offsets,options.vars()->optarg);
             break;
   case 'A': concord_options->result_mode=AXIS_; break;
   case 'x': concord_options->result_mode=XALIGN_; break;
   case 'm': concord_options->result_mode=MERGE_;
             if (options.vars()->optarg[0]=='\0') {
                error("Empty output file name argument\n");
                free_conc_opt(concord_options);
                return USAGE_ERROR_CODE;
             }
             strcpy(concord_options->output,options.vars()->optarg);
             break;
   case 'a': if (options.vars()->optarg[0]=='\0') {
                error("Empty alphabet argument\n");
                free_conc_opt(concord_options);
                return USAGE_ERROR_CODE;
             }
             concord_options->sort_alphabet=strdup(options.vars()->optarg);
             if (concord_options->sort_alphabet==NULL) {
                alloc_error("main_Concord");
                free_conc_opt(concord_options);
                return ALLOC_ERROR_CODE;            }
             break;
   case 'T': concord_options->thai_mode=1; break;
   case 'd': if (options.vars()->optarg[0]=='\0') {
                error("Empty snt directory argument\n");
                free_conc_opt(concord_options);
                return USAGE_ERROR_CODE;
             }
             strcpy(concord_options->working_directory,options.vars()->optarg);
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             free_conc_opt(concord_options);
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_Concord[index].name);
             free_conc_opt(concord_options);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             free_conc_opt(concord_options);
             return USAGE_ERROR_CODE;
   case 'k': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_encoding argument\n");
                free_conc_opt(concord_options);
                return USAGE_ERROR_CODE;
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                free_conc_opt(concord_options);
                return USAGE_ERROR_CODE;
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   }
   index=-1;
}

if (options.vars()->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   free_conc_opt(concord_options);
   return USAGE_ERROR_CODE;
}
if (concord_options->fontname==NULL || concord_options->fontsize<=0) {
   if (concord_options->result_mode==HTML_ || concord_options->result_mode==GLOSSANET_) {
      error("The specified output mode is an HTML file: you must specify font parameters\n");
      free_conc_opt(concord_options);
      return USAGE_ERROR_CODE;
   }
}

if (only_verify_arguments) {
  // freeing all allocated memory
  free_conc_opt(concord_options);
  return SUCCESS_RETURN_CODE;
}

U_FILE* concor=u_fopen(&vec,argv[options.vars()->optind],U_READ);
if (concor==NULL) {
   error("Cannot open concordance index file %s\n",argv[options.vars()->optind]);
   free_conc_opt(concord_options);
   return DEFAULT_ERROR_CODE;
}

if (concord_options->working_directory[0]=='\0') {
   get_path(argv[options.vars()->optind],concord_options->working_directory);
}
if (concord_options->only_matches) {
  concord_options->left_context=0;
  concord_options->right_context=0;
}
/* We compute the name of the files associated to the text */
struct snt_files* snt_files=new_snt_files_from_path(concord_options->working_directory);
ABSTRACTMAPFILE* text=af_open_mapfile(snt_files->text_cod,MAPFILE_OPTION_READ,0);
if (text==NULL) {
  error("Cannot open file %s\n",snt_files->text_cod);
  free_snt_files(snt_files);
  u_fclose(concor);
  free_conc_opt(concord_options);
  return DEFAULT_ERROR_CODE;
}
struct text_tokens* tok=load_text_tokens(&vec,snt_files->tokens_txt);
if (tok==NULL) {
  error("Cannot load text token file %s\n",snt_files->tokens_txt);
  af_close_mapfile(text);
  free_snt_files(snt_files);
  u_fclose(concor);
  free_conc_opt(concord_options);
  return DEFAULT_ERROR_CODE;
}

U_FILE* f_enter=u_fopen(BINARY,snt_files->enter_pos,U_READ);
int n_enter_char=0;
int* enter_pos=NULL;
/* New lines are encoded in 'enter.pos' files. Those files will disappear in the future */
if (f_enter==NULL) {
  error("Cannot open file %s\n",snt_files->enter_pos);
}
else {
  long size=get_file_size(f_enter);
  enter_pos=(int*)malloc(size);
  if (enter_pos==NULL) {
    alloc_error("main_Concord");
    u_fclose(f_enter);
    free_text_tokens(tok);
    af_close_mapfile(text);
    free_snt_files(snt_files);
    u_fclose(concor);
    free_conc_opt(concord_options);
    return ALLOC_ERROR_CODE;
  }
  n_enter_char=(int)fread(enter_pos,sizeof(int),size/sizeof(int),f_enter);
  if (n_enter_char!=(int)(size/sizeof(int))) {
    error("Read error on enter.pos file in main_Concord\n");
    u_fclose(f_enter);
    free(enter_pos);
    free_text_tokens(tok);
    af_close_mapfile(text);
    free_snt_files(snt_files);
    u_fclose(concor);
    free_conc_opt(concord_options);
    return DEFAULT_ERROR_CODE;
  }
  u_fclose(f_enter);
}
if (concord_options->result_mode==INDEX_ || concord_options->result_mode==UIMA_ ||
    concord_options->result_mode==XML_ || concord_options->result_mode==XML_WITH_HEADER_ ||
    concord_options->result_mode==AXIS_) {
   /* We force some options for index, uima and axis files */
   concord_options->left_context=0;
   concord_options->right_context=0;
   concord_options->sort_mode=TEXT_ORDER;
}
if (concord_options->only_ambiguous && concord_options->result_mode!=LEMMATIZE_) {
  /* We force text order when displaying only ambiguous outputs */
  concord_options->sort_mode=TEXT_ORDER;
}
if (concord_options->result_mode==HTML_ || concord_options->result_mode==DIFF_ || concord_options->result_mode==LEMMATIZE_) {
  /* We need the offset file if and only if we have to produce
   * an html concordance with positions in .snt file */
  concord_options->snt_offsets=load_snt_offsets(snt_files->snt_offsets_pos);
  if (concord_options->snt_offsets==NULL) {
    error("Cannot read snt offset file %s\n",snt_files->snt_offsets_pos);
    free(enter_pos);
    free_text_tokens(tok);
    af_close_mapfile(text);
    free_snt_files(snt_files);
    u_fclose(concor);
    free_conc_opt(concord_options);
    return DEFAULT_ERROR_CODE;
  }
}
if (offset_file[0]!='\0') {
  concord_options->uima_offsets=load_uima_offsets(&vec,offset_file);
  if (concord_options->uima_offsets==NULL) {
    error("Cannot read offset file %s\n",offset_file);
    free(enter_pos);
    free_text_tokens(tok);
    af_close_mapfile(text);
    free_snt_files(snt_files);
    u_fclose(concor);
    free_conc_opt(concord_options);
    return DEFAULT_ERROR_CODE;
  }
}
if (PRLG[0]!='\0') {
  concord_options->PRLG_data=load_PRLG_data(&vec,PRLG);
  if (concord_options->PRLG_data==NULL) {
    error("Cannot read PRLG file %s\n",PRLG);
    free(enter_pos);
    free_text_tokens(tok);
    af_close_mapfile(text);
    free_snt_files(snt_files);
    u_fclose(concor);
    free_conc_opt(concord_options);
    return DEFAULT_ERROR_CODE;
  }
}
if (concord_options->result_mode==CSV_) {
  concord_options->sort_mode=TEXT_ORDER;
  concord_options->only_matches=1;
}

/* Once we have set all parameters, we call the function that
 * will actually create the concordance. */
create_concordance(&vec,concor,text,tok,n_enter_char,enter_pos,concord_options);

free(enter_pos);
free_text_tokens(tok);
af_close_mapfile(text);
free_snt_files(snt_files);
u_fclose(concor);
free_conc_opt(concord_options);

u_printf("Done.\n");

return SUCCESS_RETURN_CODE;
}

} // namespace unitex
