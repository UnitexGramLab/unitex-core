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

#include "Concordance.h"
#include "Unicode.h"
#include "LocateMatches.h"
#include "SortTxt.h"
#include "Error.h"
#include "StringParsing.h"
#include "Thai.h"
#include "NewLineShifts.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define PRLG_DELIMITOR 0x02
#define LEMMATIZE_DELIMITOR 0x03

int create_raw_text_concordance(U_FILE*,U_FILE*,ABSTRACTMAPFILE*,struct text_tokens*,int,int,
                                int*,int*,int,int,struct conc_opt*);
void compute_token_length(int*,struct text_tokens*);

static void create_modified_text_file(const VersatileEncodingConfig*,U_FILE*,ABSTRACTMAPFILE*,struct text_tokens*,
        char*,int,int,const int*, vector_uima_offset*,const char*, vector_offset* v_offsets);
void write_HTML_header(U_FILE*,int,struct conc_opt*);
void write_HTML_end(U_FILE*);
void reverse_initial_vowels_thai(unichar*);

struct buffer_mapped {
    ABSTRACTMAPFILE* amf;
    const int*int_buffer_;
    size_t nb_item;
    size_t pos_next_read;
    size_t skip;
    int size;
} ;

static void buf_map_int_pseudo_seek(struct buffer_mapped* buffer,size_t pos)
{
    buffer->pos_next_read=pos;
}

static size_t buf_map_int_pseudo_read(struct buffer_mapped* buffer,size_t size_requested)
{
    size_t size_max = buffer->nb_item - buffer->pos_next_read;
    if (size_requested>size_max)
        size_requested=size_max;

    buffer->skip=buffer->pos_next_read;
    buffer->pos_next_read+=size_requested;
    return size_requested;
}



static void process_match_content_in_lemmatize_mode(unichar* s,vector_ptr* outputs) {
int i;
for (i=1;s[i]!='\0' && s[i]!='/';i++) {}
if (s[i]=='\0') {
    fatal_error("Internal error in process_match_content_in_lemmatize_mode\n");
}
s[i]='\0';
i+=1;
for (;;) {
    int start=i;
    int j=i;
    while (s[j]!='\0' && s[j]!=LEMMATIZE_DELIMITOR) {
        j++;
    }
    int end=(s[j]=='\0');
    s[j]='\0';
    vector_ptr_add(outputs,u_strdup(s+start));
    if (end) {
        return;
    }
    i=j+1;
}
}


static int get_sentence_number(unichar* indices) {
int a,b,c,d;
u_sscanf(indices,"%d%d%d%d",&a,&b,&c,&d);
return c;
}


static void print_PRGL_tag_for_csv(U_FILE* out,unichar* s) {
if (s==NULL || s[0]=='\0') return;
if (s[0]!='[') {
    fatal_error("Invalid PRLG tag format in print_PRGL_tag_for_csv: <%S>\n",s);
}
unichar* tmp=u_strchr(s,' ');
if (tmp!=NULL) {
    *tmp='\0';
}
tmp=u_strchr(s,']');
if (tmp!=NULL) {
    *tmp='\0';
}
s++;
while (NULL!=(tmp=u_strchr(s,'-'))) {
    *tmp='\0';
    u_fprintf(out,"%S - ",s);
    s=tmp+1;
}
u_fputs(s,out);
}


static void print_dic_info_for_csv(U_FILE* out,unichar* s) {
unichar* tmp=u_strchr(s+1,'/');
if (tmp==NULL) {
    u_fprintf(out,"%S\t\t",s);
    return;
}
*tmp='\0';
if (s[0]=='{') {
    struct dela_entry* e=tokenize_tag_token(s,0,NULL);
    if (e==NULL) {
        fatal_error("");
    }
    u_fputs(e->inflected,out);
    free_dela_entry(e);
} else {
    u_fputs(s,out);
}

s=tmp+1;
tmp=u_strchr(s,'.');
if (tmp==NULL) {
    fatal_error("Missing . in print_dic_info_for_csv: <%S>\n",s);
}
*tmp='\t';
u_fprintf(out,"\t%S",s);
}


/**
 * This function builds a concordance from a 'concord.ind' file
 * described by the 'concordance' parameter. 'text' is supposed to
 * represent the 'text.cod' file from which the concordance index was
 * computed. 'tokens' represents the associated 'tokens.txt'
 * file. 'option.sort_mode' is an integer that represents the sort mode to be
 * used for creating the concordance. This parameter will be ignored
 * if the function must modify the text instead of building a
 * concordance.  'option.left_context' and 'option.right_context' specify the length
 * of the contexts in visible characters (for Thai, this number is
 * different from the number of unicode characters because of
 * diacritics). 'option.fontname' and 'fontsize' are used to set the font
 * that will be used if the output is an HTML file (if not, these
 * parameters will be ignored). 'option.directory' represents the
 * working directory.  'option.result_mode' indicates the kind of
 * output that is expected. If it value is "html" or "text", the
 * function will build an HTML or text concordance. If its value is of
 * the form "glossanet=xxx", the result will be an HTML concordance
 * designed for the GlossaNet system (http://glossa.fltr.ucl.ac.be),
 * and "xxx" will be taken as a parameter given to GlossaNet. Any
 * other value will be considered as a file name to use for producing
 * a modified version of the text. 'option.sort_alphabet' is the name of the
 * "Alphabet_sort.txt" file to use for sorting the lines of the
 * concordance. This parameter will be ignored if the output is a
 * modified text file or if the sort mode is TEXT_ORDER.
 * 'n_enter_char' is the number of new lines in the text, and
 * 'enter_pos' is an array that contains the positions of these new
 * lines. If 'option.thai_mode' is set to a non zero value, it indicates that
 * the concordance is a Thai one. This information is used to compute
 * correctly the context sizes.
 *
 *
 * Modifications made by Patrick Watrin (pwatrin@gmail.com) allow to
 * build index and axis files.
 *
 * WHAT IS AN AXIS FILE ?
 * ----------------------
 * "SIMR requires axes to be formatted with one "tic mark" per line.
 * A tic mark consists of a semantic unit (a token) and its position
 * in the text. By convention, the position of a token is the position
 * of its median character (I conjecture that this also works best in
 * terms of accuracy). Thus, a token's position is always a multiple
 * of 0.5." (http://nlp.cs.nyu.edu/GMA/docs/HOWTO-axis)
 *
 * EXAMPLE :
 * ---------
 * 012345678901
 * This segment
 * 2.5  9
 */
int create_concordance(const VersatileEncodingConfig* vec,U_FILE* concordance,ABSTRACTMAPFILE* text,struct text_tokens* tokens,
                        int n_enter_char,int* enter_pos,struct conc_opt* options) {
U_FILE* out;
U_FILE* f;
char temp_file_name[FILENAME_MAX];
struct string_hash* glossa_hash=NULL;
int open_bracket=-1;
int close_bracket=-1;
/* We compute the length of each token */
int* token_length=(int*)malloc(sizeof(int)*tokens->N);
if (token_length==NULL) {
   fatal_alloc_error("create_concordance");
}
compute_token_length(token_length,tokens);
if (options->result_mode==MERGE_) {
    /* If we have to produced a modified version of the original text, we
     * do it and return. */

    vector_offset* in_offsets = NULL;
    vector_offset* out_offsets = NULL;
    U_FILE* f_output_offsets = NULL;
    if ((options->input_offsets[0] != '\0') && (options->output_offsets[0] != '\0')) {

        in_offsets = load_offsets(vec, options->input_offsets);
        if (in_offsets == NULL) {
            fatal_error("Cannot load offset file %s\n", options->input_offsets);
        }

        out_offsets = new_vector_offset();


        /* We deal with offsets only if we have to produce output offsets */
        f_output_offsets = u_fopen(vec, options->output_offsets, U_WRITE);
        if (f_output_offsets == NULL) {
            error("Cannot create offset file %s\n", options->output_offsets);
            return 1;
        }

    }

    create_modified_text_file(vec,concordance,text,tokens,
            options->output,options->convLFtoCRLF,n_enter_char,enter_pos,options->uima_offsets,
            (out_offsets != NULL) ? NULL : options->output_offsets, out_offsets);

    if (f_output_offsets != NULL) {
        process_offsets(in_offsets, out_offsets, f_output_offsets);
        u_fclose(f_output_offsets);
    }
    free(token_length);
    free_vector_offset(in_offsets);
    free_vector_offset(out_offsets);
    return 0;
}
/* If the expected result is a concordance */
if (options->result_mode==GLOSSANET_) {
    /* The structure glossa_hash will be used to ignore duplicate lines
     * without sorting */
    glossa_hash=new_string_hash();
    /* Building GlossaNet concordances requires to locate square brackets in the
     * text. That's why we compute the token numbers associated to '[' and ']' */
    unichar r[2];
    r[0]='[';
    r[1]='\0';
    open_bracket=get_token_number(r,tokens);
    r[0]=']';
    close_bracket=get_token_number(r,tokens);
}
/* We set temporary and final file names */
strcpy(temp_file_name,options->working_directory);
strcat(temp_file_name,"concord_.txt");
if (options->output[0]=='\0') {
    strcpy(options->output,options->working_directory);
    if (options->result_mode==TEXT_ || options->result_mode==INDEX_
      || options->result_mode==UIMA_ || options->result_mode==AXIS_
      || options->result_mode==XALIGN_ || options->result_mode==DIFF_)
        strcat(options->output,"concord.txt");
    else if ((options->result_mode==XML_) || (options->result_mode==XML_WITH_HEADER_))
        strcat(options->output,"concord.xml");
    else if (options->result_mode==LEMMATIZE_)
        strcat(options->output,"lemmatize.html");
    else if (options->result_mode==CSV_)
        strcat(options->output,"export.csv");
    else strcat(options->output,"concord.html");
}
int N_MATCHES;

/* If we are in the 'xalign' mode, we don't need to sort the results.
 * So, we don't need to store the results in a temporary file */
if (options->result_mode==XALIGN_) f=u_fopen(UTF8,options->output,U_WRITE);
else f=u_fopen(vec,temp_file_name,U_WRITE);
if (f==NULL) {
    error("Cannot write %s\n",temp_file_name);
    free(token_length);
    return 1;
}
/* First, we create a raw text concordance.
 * NOTE: columns may have been reordered according to the sort mode. See the
 * comments of the 'create_raw_text_concordance' function for more details. */
N_MATCHES=create_raw_text_concordance(f,concordance,text,tokens,
                                      options->result_mode,n_enter_char,enter_pos,
                                      token_length,open_bracket,close_bracket,
                                      options);
u_fclose(f);
free(token_length);

if(options->result_mode==XALIGN_) return 0;

/* If necessary, we sort it by invoking the main function of the SortTxt program */
if (options->sort_mode!=TEXT_ORDER) {
   pseudo_main_SortTxt(vec,0,0,options->sort_alphabet,NULL,options->thai_mode,temp_file_name,0);
}
/* Now, we will take the sorted raw text concordance and we will:
 * 1) reorder the columns
 * 2) insert HTML info if needed
 */

f=u_fopen(vec,temp_file_name,U_READ);
if (f==NULL) {
    error("Cannot read %s\n",temp_file_name);
    return 1;
}
if (options->result_mode==TEXT_ || options->result_mode==INDEX_
      || options->result_mode==XML_ || options->result_mode==XML_WITH_HEADER_
      || options->result_mode==UIMA_ || options->result_mode==AXIS_
      || options->result_mode==DIFF_ || options->result_mode==CSV_) {
   /* If we have to produce a unicode text file, we open it */
   out=u_fopen(vec,options->output,U_WRITE);
}
else {
   /* Otherwise, we open it as a UTF8 HTML file */
   out=u_fopen(UTF8,options->output,U_WRITE);
}
if (out==NULL) {
    error("Cannot write %s\n",options->output);
    u_fclose(f);
    return 1;
}
/* If we have an HTML or a GlossaNet/script concordance, we must write an HTML
 * file header. */
if (options->result_mode==HTML_ || options->result_mode==GLOSSANET_ || options->result_mode==SCRIPT_
        || options->result_mode==LEMMATIZE_) {
    write_HTML_header(out,N_MATCHES,options);
}
if (options->result_mode==XML_WITH_HEADER_) {
  if ((vec->encoding_output == UTF16_LE) || (vec->encoding_output == BIG_ENDIAN_UTF16)) {
    u_fprintf(out,"<?xml version='1.0' encoding='UTF-16'?>\n<concord>\n");
  }
  else
  if ((vec->encoding_output == UTF8)) {
    u_fprintf(out,"<?xml version='1.0' encoding='UTF-8'?>\n<concord>\n");
  }
  else
    u_fprintf(out,"<?xml version='1.0'>\n<concord>\n");
}
if (options->result_mode==XML_) {
  u_fprintf(out,"<concord>\n");
}
unichar* unichar_buffer=(unichar*)malloc(sizeof(unichar)*((3000*4) + 100));
if (unichar_buffer==NULL) {
    fatal_alloc_error("create_concordance");
}
unichar* A = unichar_buffer + (3000 * 0);
unichar* B = unichar_buffer + (3000 * 1);
unichar* C = unichar_buffer + (3000 * 2);
unichar* href = unichar_buffer + (3000 * 3);
unichar* indices = unichar_buffer + (3000 * 4);
unichar* left=NULL;
unichar* middle=NULL;
unichar* right=NULL;
Ustring* PRLG_tag=new_Ustring(32);
int j;
int c;
int csv_line=1;
/* Now we process each line of the sorted raw text concordance */
while ((c=u_fgetc(f))!=EOF) {
    empty(PRLG_tag);
    j=0;
    /* We save the first column in A... */
    while (c!=0x09) {
        A[j++]=(unichar)c;
        c=u_fgetc(f);
    }
    A[j]='\0';
    c=u_fgetc(f);
    j=0;
    /* ...the second in B... */
    while (c!=0x09) {
        B[j++]=(unichar)c;
        c=u_fgetc(f);
    }
    B[j]='\0';
    c=u_fgetc(f);
    j=0;
    /* ...and the third in C */
    while (c!='\n' && c!='\t') {
        C[j++]=(unichar)c;
        c=u_fgetc(f);
    }
    C[j]='\0';
    indices[0]='\0';
    /* If there are indices to be read like "15 17 1", we read them */
    if (c=='\t') {
        c=u_fgetc(f);
        j=0;
        while (c!='\t' && c!='\n' && c!=PRLG_DELIMITOR) {
            indices[j++]=(unichar)c;
            c=u_fgetc(f);
        }
        indices[j]='\0';
        /*------------begin GlossaNet-------------------*/
        /* If we are in GlossaNet mode, we extract the url at the end of the line */
        if (options->result_mode==GLOSSANET_) {
            if (c!='\t') {
                error("ERROR in GlossaNet concordance: no URL found\n");
                href[0]='\0';
            } else {
                j=0;
                while ((c=u_fgetc(f))!='\n' && c!=PRLG_DELIMITOR) {
                    href[j++]=(unichar)c;
                }
                href[j]='\0';
            }
        }
        /*------------end GlossaNet-------------------*/
    }
    if (c==PRLG_DELIMITOR) {
        /* If there is a PRLG tag */
        c=u_fgetc(f);
        if (c!='[') {
            fatal_error("Invalid PRLG tag in create_concordance");
        }
        while (c!='\n') {
            u_strcat(PRLG_tag,(unichar)c);
            c=u_fgetc(f);
        }
        u_strcat(PRLG_tag,"  ");
    }
    /* Now we will reorder the columns according to the sort mode */
    switch(options->sort_mode) {
        case TEXT_ORDER: left=A; middle=B; right=C; break;
        case LEFT_CENTER: left=A; middle=B; right=C; break;
        case LEFT_RIGHT: left=A; right=B; middle=C; break;
        case CENTER_LEFT: middle=A; left=B; right=C; break;
        case CENTER_RIGHT: middle=A; right=B; left=C; break;
        case RIGHT_LEFT: right=A; left=B; middle=C; break;
        case RIGHT_CENTER: right=A; middle=B; left=C; break;
    }
    /* We use 'can_print_line' to decide if the concordance line must be
     * printed, because in GlossaNet mode, duplicates must be removed. */
    int can_print_line=1;
    if (options->result_mode==GLOSSANET_) {
        unichar line[4000];
        u_sprintf(line,"%S%S\t%S\t%S",PRLG_tag->str,left,middle,right);
        /* We test if the line was already seen */
        if (NO_VALUE_INDEX==get_value_index(line,glossa_hash,DONT_INSERT)) {
            can_print_line=1;
            get_value_index(line,glossa_hash);
        } else {
            can_print_line=0;
        }
    }
    /* If we can print the line */
    if (can_print_line) {
        if (options->sort_mode!=TEXT_ORDER) {
            /* If the concordance was sorted, the left sequence was reversed, and
             * then, we have to reverse it again. However, the Thai sort algorithm
             * requires to modify some vowels. That's why we must apply a special
             * procedure if we have a Thai sorted concordance. */
            if (options->thai_mode) reverse_initial_vowels_thai(left);
            /* Now we revert and print the left context */
            if (options->result_mode==HTML_ || options->result_mode==GLOSSANET_
                    || options->result_mode==SCRIPT_ || options->result_mode==LEMMATIZE_) {
                u_fprintf(out,"<tr><td nowrap>%S%HR",PRLG_tag->str,left);
            } else {u_fprintf(out,"%R",left);}
        } else {
            /* If the concordance is not sorted, we do not need to revert the
             * left context. */
            if (options->result_mode==HTML_ || options->result_mode==GLOSSANET_
                    || options->result_mode==SCRIPT_ || options->result_mode==LEMMATIZE_) {
                u_fprintf(out,"<tr><td nowrap>%S%HS",PRLG_tag->str,left);
            } else if (options->result_mode!=CSV_) {
                u_fprintf(out,"%S%S",PRLG_tag->str,left);
            }
        }
        /* If we must produce an HTML concordance, then we surround the
         * located sequence by HTML tags in order to make it an hyperlink.
         * This hyperlink will contain a fake URL of the form "X Y Z", where
         * X and Y are the starting and ending position of the sequence (in
         * tokens) and Z is the number of the sentence that contains the
         * sequence. */
        if (options->result_mode==HTML_) {
            u_fprintf(out,"<a href=\"%S\">%HS</a>%HS&nbsp;</td></tr>\n",indices,middle,right);
        } else if (options->result_mode==LEMMATIZE_) {
            /* In lemmatize mode we must extract everything after the / */
            vector_ptr* outputs=new_vector_ptr();
            process_match_content_in_lemmatize_mode(middle,outputs);
            u_fprintf(out,"<a href=\"%S\">%HS</a>",indices,middle);
            u_fprintf(out,"<!--%d-->",outputs->nbelems);
            for (int k=0;k<outputs->nbelems;k++) {
                unichar* z=(unichar*)outputs->tab[k];
                u_fprintf(out,"<!--%HS-->",z);
            }
            u_fprintf(out,"%HS&nbsp;</td></tr>\n",right);
            free_vector_ptr(outputs,free);
        } else if (options->result_mode==GLOSSANET_) {
            /* If we must produce a GlossaNet concordance, we turn the sequence
             * into an URL, using the given GlossaNet script. */
            u_fprintf(out,"<A HREF=\"%s?rec=%HS&adr=%HS",options->script,middle,href);
         u_fprintf(out,"\" style=\"color: rgb(0,0,128)\">%HS</A>%HS</td></tr>\n",middle,right);
        }
        /* If we must produce a script concordance */
        else if (options->result_mode==SCRIPT_) {
            u_fprintf(out,"<a href=\"%s%US",options->script,middle);
            u_fprintf(out,"\">%HS</a>%HS</td></tr>\n",middle,right);
        }
        /* If we must produce a text concordance */
        else if (options->result_mode==TEXT_) {
            if (!options->only_matches) u_fputc('\t',out);
            u_fputs(middle,out);
            if (!options->only_matches) u_fputc('\t',out);
            u_fprintf(out,"%S\n",right);
        } else if (options->result_mode==CSV_) {
            if (!u_strcmp(middle," ")) {
                u_fprintf(out,"%d\t\t\t \t\t\n",csv_line++);
            } else {
                u_fprintf(out,"%d\t%d\t",csv_line++,get_sentence_number(indices));
                print_PRGL_tag_for_csv(out,PRLG_tag->str);
                u_fprintf(out,"\t");
                print_dic_info_for_csv(out,middle);
                u_fprintf(out,"\n");
            }
        }
        /* If we must produce a concordance to be used by ConcorDiff*/
        else if (options->result_mode==DIFF_) {
            u_fprintf(out,"\t%S\t%S\t%S\n",middle,right,indices);
        }
      /* If must must produce an index file */
      else if (options->result_mode==INDEX_) {
         unichar idx[128];
         parse_string(indices,idx,P_SPACE);
         u_fprintf(out,"%S\t%S\n",idx,middle);
      }
      else if (options->result_mode==UIMA_) {
         char tmp1[100];
         u_to_char(tmp1,indices);
         int start,end;
         sscanf(tmp1,"%d %d",&start,&end);
         u_fprintf(out,"%d %d\t%S\n",start,end,middle);
      }
      else if ((options->result_mode==XML_) || (options->result_mode==XML_WITH_HEADER_)) {
         char tmp1[100];
         u_to_char(tmp1,indices);
         int start,end;
         sscanf(tmp1,"%d %d",&start,&end);
         u_fprintf(out,"<concordance start=\"%d\" end=\"%d\">%S</concordance>\n",start,end,middle);
      }
      /* If must must produce an axis file...
         VARIABLES :
         -----------
         - f1: position of the first character of a token
         - f2: position of the last character of a token
         - len : length of a token
            -> len = (f2+1) - f1
         - med : position of the median character of a token
            -> med = ((len+1)/2) + f1
      */
      else if (options->result_mode==AXIS_) {
         char tmp1[100];
         u_to_char(tmp1,indices);
         float f1,f2,len,med;
         sscanf(tmp1,"%f %f",&f1,&f2);
         len=(f2+1)-f1;
         med=((len+1)/2)+f1;
         u_fprintf(out,"%.1f\t%S\n",med,middle);
      }
    }
}
/* If we have an HTML or a GlossaNet concordance, we must write some
 * HTML closing tags. */
if ((options->result_mode==HTML_) || (options->result_mode==GLOSSANET_) ||
        options->result_mode==LEMMATIZE_) write_HTML_end(out);
if ((options->result_mode==XML_) || (options->result_mode==XML_WITH_HEADER_)){
  u_fprintf(out,"</concord>\n");
}
u_fclose(f);
af_remove(temp_file_name);
u_fclose(out);
free(unichar_buffer);
free_Ustring(PRLG_tag);
if (options->result_mode==GLOSSANET_) {
    free_string_hash(glossa_hash);
}
return 0;
}


/**
 * This function computes the length in unicode characters of
 * all the given tokens.
 */
void compute_token_length(int* token_length,struct text_tokens* tokens) {
int i;
for (i=0;i<tokens->N;i++) {
  token_length[i]=u_strlen(tokens->token[i]);
}
}


/**
 * This function writes the HTML header for an HTML or a GlossaNet concordance.
 */
void write_HTML_header(U_FILE* f,int number_of_matches,struct conc_opt* option) {
u_fprintf(f,"<html lang=en>\n");
u_fprintf(f,"<head>\n");
u_fprintf(f,"   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
u_fprintf(f,"   <title>%d match%s</title>\n",number_of_matches,(number_of_matches>1)?"es":"");
u_fprintf(f,"</head>\n");
u_fprintf(f,"<body>\n<table border=\"0\" cellpadding=\"0\" width=\"100%%\" style=\"font-family: '%s'; font-size: %d\">\n",option->fontname,option->fontsize);
}


/**
 * This function write the HTML closing tags for an HTML or a GlossaNet concordance.
 */
void write_HTML_end(U_FILE* f) {
u_fprintf(f,"</table></body>\n");
u_fprintf(f,"</html>\n");
}



/**
 * This function fills the string 'left' with the string of length
 * 'option.left_context' corresponding to the tokens located before the token number
 * 'pos'. 'token_length' is an array that gives the lengthes of the tokens.
 * 'buffer' contains the token numbers to work on. 'option.thai_mode' indicates by a non
 * zero value that we deal with a Thai sequence; in that case, we must do a special
 * operation in order to count correctly displayable characters.
 *
 * Note that extra spaces will be used to fill 'left' if there no left context enough,
 * in order to preserve alignment at display time.
 */
void extract_left_context(int pos,int pos_in_char,unichar* left,struct text_tokens* tokens,
                          struct conc_opt* option,int* token_length,
                          struct buffer_mapped* buffer) {
int i;
/* If there is no left context at all, we fill 'left' with spaces. */
if (pos==0 && pos_in_char==0) {
    for (i=0;i<option->left_context;i++) {
        left[i]=' ';
    }
    left[i]='\0';
    return;
}
i=0;
int count=0;
left[option->left_context]='\0';

if (pos_in_char==0) {
   /* If must start on the left of the match */
   pos--;
} else {

   /* If we have to take a prefix of the match's first token */
   unichar* s=tokens->token[buffer->int_buffer_[buffer->skip+pos]];
   for (int j=pos_in_char-1;j>=0;j--) {
      left[i++]=s[j];
   }
   pos--;
   if (pos==-1) {
      /* If the first token of the match was the first token at all,
       * we fill with spaces */
      for (;i<option->left_context;i++) {
         left[i]=' ';
      }
      left[i]='\0';
      mirror(left);
      return;
   }
}

int l=token_length[buffer->int_buffer_[buffer->skip+pos]]-1;
unichar* s=tokens->token[buffer->int_buffer_[buffer->skip+pos]];
/* We look for every token, until we have the correct number of displayable
 * characters. */
while (pos>=0 && count<option->left_context) {
    left[i]=s[l--];
    if (!option->thai_mode || !is_Thai_skipable(left[i])) {
        /* We increase the character count only we don't have a diacritic mark */
        count++;
    }
    i++;
    if (l<0) {
        /* If we must change of token */
        if (option->left_context_until_eos
                    && (buffer->int_buffer_[buffer->skip+pos] == tokens->SENTENCE_MARKER))
                  break; /* token was "{S}" */
        pos--;
        if (pos>=0) {
            /* And if we can, i.e. we are not at the beginning of the text */
            l=token_length[buffer->int_buffer_[buffer->skip+pos]]-1;
            s=tokens->token[buffer->int_buffer_[buffer->skip+pos]];
        }
    }
}
/* If it was not possible to get to correct number of characters because
 * the sequence was too close to the beginning of the text, we fill
 * 'left' with spaces. */
if (count!=option->left_context) {
    while (count++!=option->left_context) {
        left[i++]=' ';
    }
}
left[i]='\0';
/* Finally, we reverse the string because we want the left context and not its mirror.
 * Note that we cannot fill the buffer from the end because of Thai diacritics that
 * can make the length of left in characters greater than 'LEFT_CONTEXT_LENGTH'. */
mirror(left);
}


/**
 * This function fills 'middle' with the matched sequence represented by the token
 * range [start_pos,end_pos]. 'output' is the output sequence that was computed
 * during the locate operation. If not NULL, we ignore the original text and copy
 * this value to 'middle'; otherwise we concatenate the tokens that compose
 * the matched sequence. Note that we allways take the whole sequence, so that
 * 'middle' must have been carefully allocated. 'buffer' contains the token numbers
 * to work on.
 */
void extract_match(int start_pos,int start_pos_char,int end_pos,int end_pos_char,unichar* output,unichar* middle,
                    struct text_tokens* tokens,struct buffer_mapped* buffer) {
if (output!=NULL) {
   /* If there is an output, then the match is the output */
   u_strcpy(middle,output);
   return;
}
/* If there is no output, we compute the match from the text */
int j=0,k;
unichar* s;
if (start_pos_char!=0) {
   /* If the match doesn't start on the first char of the first token */
   s=tokens->token[buffer->int_buffer_[buffer->skip+start_pos]];
   int end=(end_pos==start_pos) ? (end_pos_char+1) : ((int)u_strlen(s));
   for (k=start_pos_char;k<end;k++) {
      middle[j++]=s[k];
   }
   if (start_pos==end_pos) {
      middle[j]='\0';
      return;
   }
   start_pos++;
}
for (int i=start_pos;i<end_pos;i++) {
   k=0;
    s=tokens->token[buffer->int_buffer_[buffer->skip+i]];
    while (s[k]!='\0') {
        middle[j++]=s[k++];
    }
}
/* We write the last token */
s=tokens->token[buffer->int_buffer_[buffer->skip+end_pos]];
for (k=0;k<=end_pos_char;k++) {
   middle[j++]=s[k];
}
middle[j]='\0';
}


/**
 * This function fills the string 'right' with the string of length
 * 'option.right_context'-'match_length' corresponding to the tokens located
 * after the token number 'pos'. Note that the 'right' may be empty if the match
 * was already greater or equal to 'option.right_context'.
 *
 * 'token_length' is an array that gives the lengthes of the tokens.
 * 'buffer' contains the token numbers to work on. 'option.thai_mode' indicates by a non
 * zero value that we deal with a Thai sequence; in that case, we must do a special
 * operation in order to count correctly displayable characters.
 */
void extract_right_context(int pos,int pos_char,unichar* right,struct text_tokens* tokens,
                           int match_length,struct conc_opt* option,
                           struct buffer_mapped* buffer) {
right[0]='\0';
if (match_length>=option->right_context) {
   /* We return if we have already overpassed the right context length
    * with the matched sequence */
    return;
}
int right_context_length=option->right_context-match_length;
int i=0;
int count=0;

/* We save the end of the last match token, if needed */
unichar* last_match_token=tokens->token[buffer->int_buffer_[buffer->skip+pos]];
for (int u=pos_char+1;last_match_token[u]!='\0';u++) {
   right[i++]=last_match_token[u];
}

/* We must start after the last token of the matched sequence */
pos++;
if (pos==buffer->size) {
   /* If this token was the last token of the text */
   return;
}
int l=0;
unichar* s=tokens->token[buffer->int_buffer_[buffer->skip+pos]];
while (pos<buffer->size && count<right_context_length) {
    right[i]=s[l++];
    if (!option->thai_mode || !is_Thai_skipable(right[i])) count++;
    i++;
    if (s[l]=='\0') {
        /* If we must change of token */
        if (option->right_context_until_eos
                    && (buffer->int_buffer_[buffer->skip+pos] == tokens->SENTENCE_MARKER))
                  break; /* token was "{S}" */
        pos++;
        if (pos<buffer->size) {
            /* And if we can */
            l=0;
            s=tokens->token[buffer->int_buffer_[buffer->skip+pos]];
        }
    }
}
/* We don't fill 'right' with spaces if we have reached the end of the text, because
 * there is no alignment problem on the right side of concordance. */
right[i]='\0';
}


/**
 * A GlossaNet concordance is supposed to have been computed on a text of the
 * following form:
 *
 * document1
 * [[url1]]
 * document2
 * [[url2]]
 * ...
 *
 * This function tries to find the URL between [[ and ]] that follows the matched sequence
 * in the text (i.e. after the token number 'end_pos'). If there is one, it is copied in
 * 'href'; if not, 'href' is empty. 1 is returned except if the function finds out that
 * the matched sequence is a part of an URL. In that case, 0 is returned in order to
 * indicate that this is not a real matched sequence.
 *
 * The function assumes that 'buffer' is large enough to find the URL. It may
 * not work anymore if the buffer size is too small.
 */
int extract_href(int end_pos,unichar* href,struct text_tokens* tokens,struct buffer_mapped* buffer,
                int open_bracket,int close_bracket) {
href[0]='\0';
if (open_bracket==-1 || close_bracket==-1) {
    /* If there are no both open and close square brackets, there
     * is no chance to find any URL. */
    return 1;
}
int i=end_pos+1;
int op=0;
int cl=0;
/* First, we look for [[ or ]] */
while (i<buffer->size && op!=2 && cl!=2) {
    if (buffer->int_buffer_[buffer->skip+i]==open_bracket) {op++;cl=0;}
    else if (buffer->int_buffer_[buffer->skip+i]==close_bracket) {cl++;op=0;}
    else {op=0;cl=0;}
    i++;
}
if (cl==2) {
    /* If we have found ]], it means that the matched sequence is part of
     * an URL. */
    return 0;
}
if (op!=2) {
    /* If we have reached the end of the buffer without finding [[ */
    return 1;
}
/* We concatenate all the tokens we find before ]] */
while (i+1<buffer->size && (buffer->int_buffer_[buffer->skip+i]!=close_bracket || buffer->int_buffer_[buffer->skip+i+1]!=close_bracket)) {
    u_strcat(href,tokens->token[buffer->int_buffer_[buffer->skip+i]]);
    i++;
}
if (buffer->int_buffer_[buffer->skip+i]!=close_bracket || buffer->int_buffer_[buffer->skip+i+1]!=close_bracket) {
    /* If we don't find ]], we empty href */
    href[0]='\0';
}
return 1;
}


/**
 * This function takes a string 's' that is the mirror of a Thai left context.
 * For sorting reasons, we must invert s[i] and s[i+1] when s[i] is
 * an initial vowel, because the Thai sort algorithm would behave strangely
 * when applied on raw reversed text. For more information (written in French),
 * see chapter 3.1 in:
 *
 * Paumier S�bastien, 2003. De la reconnaissance de formes linguistiques �
 * l'analyse syntaxique, Ph.D., Universit� Paris-Est Marne-la-Vall�e. Downloadable
 * at: http://igm.univ-mlv.fr/LabInfo/theses/
 *
 * You can also consult (in French too):
 *
 * Kosawat Krit, 2003. M�thodes de segmentation et d'analyse automatique de
 * textes tha�, Ph.D., Universit� Paris-Est Marne-la-Vall�e. Downloadable
 * at: http://igm.univ-mlv.fr/LabInfo/theses/
 */
void reverse_initial_vowels_thai(unichar* s) {
int i=0;
unichar c;
while (s[i]!='\0') {
    if (is_Thai_initial_vowel(s[i]) && s[i+1]!='\0') {
        c=s[i+1];
        s[i+1]=s[i];
        s[i]=c;
        i++;
    }
    i++;
}
}


/**
 * In lemmatize mode, all matches are supposed to be of the following form:
 *
 * input/output
 *
 * When there are ambiguous outputs, we want to factorize them in one match as follows:
 *
 * inputA/output1
 * inputA/output2
 * inputB/output3
 * inputB/output4
 * inputB/output5character
 *
 * will be turned into:
 *
 * inputA/output1*output2*output3
 * inputB/output4*output5
 *
 * where the * symbol stands for the character #3
 *
 * Note: this function does not need a struct match_list** since the head can never be removed.
 */
static void group_ambiguous_outputs(struct match_list* list) {
while (list!=NULL && list->next!=NULL) {
    if (are_ambiguous(list,list->next)) {
        int i;
        unichar* s=list->next->output;
        for (i=1;s[i]!='\0' && s[i]!='/';i++) {
            /* We must start at 1, because the input may be the "/" character */
        }
        if (s[i]=='\0') {
            /* The output does not contain / */
            fatal_error("Missing / in group_ambiguous_outputs\n");
        }
        s+=i+1;
        /* +2: +1 for \0 and +1 for the delimitor */
        int len=u_strlen(list->output);
        list->output=(unichar*)realloc(list->output,sizeof(unichar)*(2+len+u_strlen(s)));
        if (list->output==NULL) {
            fatal_alloc_error("group_ambiguous_outputs");
        }
        list->output[len]=LEMMATIZE_DELIMITOR;
        u_strcpy(list->output+len+1,s);
        struct match_list* tmp=list->next;
        list->next=list->next->next;
        free_match_list_element(tmp);
    } else {
        list=list->next;
    }
}
}


static void add_missing_spaces(struct match_list* *list) {
unichar U_SPACE[2]={' ','\0'};
while (*list!=NULL && (*list)->next!=NULL) {
    if ((*list)->m.end_pos_in_token!=(*list)->next->m.start_pos_in_token-1) {
        /* We insert a space here */
        struct match_list* next=(*list)->next;
        (*list)->next=new_match((*list)->m.end_pos_in_token+1,next->m.start_pos_in_token-1,U_SPACE,0,next,NULL);
        list=&((*list)->next->next);
    } else {
        list=&((*list)->next);
    }
}
}


/**
 * For a csv export, all ambiguous forms must be considered as untagged ones.
 * So, as ambiguous forms are supposed to have been factorized by
 * group_ambiguous_outputs to the format
 *
 * inputA/output1*output2*output3
 *
 * where the * symbol stands for the character #3, we just have to look for lines that
 * contain char #3 and to replace / by \0
 */
static void eliminate_codes_for_ambiguous_matches(struct match_list* list) {
while (list!=NULL) {
    if (list->output==NULL) {
        fatal_error("Unexpected NULL output in eliminate_codes_for_ambiguous_matches\n");
    }
    if (NULL!=u_strchr(list->output,LEMMATIZE_DELIMITOR)) {
        unichar* pos=u_strchr(list->output+1,'/');
        if (pos==NULL) {
            fatal_error("Internal error in eliminate_codes_for_ambiguous_matches\n");
        }
        *pos='\0';
    }
    list=list->next;
}
}


/**
 * This function reads a concordance index from the file 'concordance' and produces a
 * text file that is stored in 'output'. This file contains the lines of the concordance,
 * but the columns may have been moved according to the sort mode, and the left
 * context is reversed. For instance, if we have a concordance line like:
 *
 * ABC DEF GHI
 *
 * where ABC is the left context, DEF is the matched sequence and GHI is the right
 * context. If the sort mode is "CENTER_LEFT", the output will contain the following
 * line:
 *
 * DEF^CBA^GHI
 *
 * where ^ stands for the tabulation character. If there are extra information like
 * positions (HTML concordance) or URL (GlossaNet concordance), they are stored at the end
 * of the line:
 *
 * DEF^CBA^GHI^120 124 5
 *
 *
 * 'text' is the "text.cod" file. 'tokens' contains the text tokens.
 * 'option.left_context' and 'option.right_context' specify the lengthes of the
 * contexts to extract. 'expected_result' is used to know if the output is
 * a GlossaNet concordance. 'n_enter_char' is the number of new lines in the text,
 * and 'enter_pos' is an array that contains the positions of these new lines.
 * If 'option.thai_mode' is set to a non zero value, it indicates that the concordance
 * is a Thai one. This information is used to compute correctly the context sizes.
 *
 * The function returns the number of matches actually written to the output file.
 *
 * For the xalign mode we produce a concord file with the following information :
 *
 *    - Column 1: sentence number
 *    - Column 2: shift in chars from the beginning of the sentence to the left side of the match
 *    - Column 3: shift in chars from the beginning of the sentence to the right side of the match
 */
int create_raw_text_concordance(U_FILE* output,U_FILE* concordance,ABSTRACTMAPFILE* text,struct text_tokens* tokens,
                                int expected_result,
                                int n_enter_char,int* enter_pos,
                                int* token_length,int open_bracket,int close_bracket,
                                struct conc_opt* options) {
struct match_list* matches;
struct match_list* matches_tmp;
unichar* unichar_buffer=(unichar*)malloc(sizeof(unichar)*(MAX_CONTEXT_IN_UNITS+1)*4);
if (unichar_buffer==NULL) {
    fatal_alloc_error("create_raw_text_concordance");
}
unichar* left = unichar_buffer + ((MAX_CONTEXT_IN_UNITS+1) * 0);
unichar* middle = unichar_buffer + ((MAX_CONTEXT_IN_UNITS+1) * 1);
unichar* right = unichar_buffer + ((MAX_CONTEXT_IN_UNITS+1) * 2);
unichar* href = unichar_buffer + ((MAX_CONTEXT_IN_UNITS+1) * 3);
int number_of_matches=0;
int is_a_good_match=1;
int start_pos,end_pos;
int n_units_already_read=0;
/* First, we allocate a buffer to read the "text.cod" file */
struct buffer_mapped* buffer=(struct buffer_mapped*)malloc(sizeof(struct buffer_mapped));
if (buffer==NULL) {
    fatal_alloc_error("create_raw_text_concordance");
}
buffer->amf=(text);
buffer->int_buffer_=(const int*)af_get_mapfile_pointer(buffer->amf);
buffer->nb_item=af_get_mapfile_size(buffer->amf)/sizeof(int);
buffer->skip=0;
buffer->pos_next_read=0;
buffer->size=0;

u_printf("Loading concordance index...\n");
/* Then we load the concordance index. NULL means that the kind of output
 * doesn't matter. */
OutputPolicy op;
matches=load_match_list(concordance,&op,NULL);
if (options->result_mode==LEMMATIZE_ && op!=MERGE_OUTPUTS) {
    fatal_error("Invalid concordance in lemmatize mode: only MERGE concordances are allowed\n");
}
/* In an html concordance, we have to know the match number in the
 * concord.ind file, but we need a renumber system, since in
 * ambiguous output only mode, we have to keep trace of the original
 * match number
 */
vector_int* renumber=NULL;
if (options->only_ambiguous) {
    renumber=new_vector_int();
    filter_unambiguous_outputs(&matches,renumber);
}
if (options->result_mode==LEMMATIZE_ || options->result_mode==CSV_) {
    /* Note that this operation makes the renumber array invalid, if any,
     * so we delete it to avoid any problems */
    free_vector_int(renumber);
    renumber=NULL;
    group_ambiguous_outputs(matches);
    if (options->result_mode==CSV_) {
        eliminate_codes_for_ambiguous_matches(matches);
        add_missing_spaces(&matches);
    }
}
/* Then we fill the buffer with the beginning of the text */
buffer->size=(int)buf_map_int_pseudo_read(buffer,buffer->nb_item);
int start_pos_char;
int end_pos_char;
int current_sentence=1;
int position_in_chars=0;
int position_in_tokens=0;

/* Information needed by the 'xalign' mode
 * - position_from_eos : current position from the beginning of the sentence
 * - start_from_eos: position of the first character from the beginning of the sentence
 * - end_from_eos: position of the last character from the beginning of the sentence */
int position_from_eos=0;
int start_from_eos=0;
int end_from_eos=0;
int match_number=-1;
int concord_ind_match_number;
/* Now we can proceed all the matches, assuming that they are sorted by starting
 * position */
u_printf("Constructing concordance...\n");
while (matches!=NULL) {
    match_number++;
    if (renumber==NULL) {
        if (options->result_mode==LEMMATIZE_) {
            /* We don't use that information in lemmatize mode, so we use instead
             * the index of the first token (that should also be the only one if
             * there is no problem) */
            concord_ind_match_number=matches->m.start_pos_in_token;
        } else {
            concord_ind_match_number=match_number;
        }
    } else {
        concord_ind_match_number=renumber->tab[match_number];
    }
    /* Here, we are sure that the buffer contains all the tokens we need.
     * We adjust 'start_pos' and 'end_pos' so that the tokens that compose
     * the current match are between buffer[start_pos] and buffer[end_pos]. */
    start_pos=matches->m.start_pos_in_token-n_units_already_read;
    end_pos=matches->m.end_pos_in_token-n_units_already_read;
    start_pos_char=position_in_chars;
    /* We update the position in characters so that we know how
     * many characters there are before buffer[start_pos]. We update
     * the sentence number in the same way. */
    if (position_in_tokens>start_pos) {
       /* If we have to go backward, in the case a Locate made in "All matches mode" */
       for (int z=position_in_tokens-1; z>=start_pos; z--) {
         int token_size=0;
         if (options->original_file_offsets==0 || options->uima_offsets==NULL || buffer->int_buffer_[buffer->skip+z]!=tokens->SENTENCE_MARKER) {
            token_size=token_length[buffer->int_buffer_[buffer->skip+z]];
         }
         start_pos_char=start_pos_char-token_size;
         position_from_eos=position_from_eos-token_size;
         start_from_eos=position_from_eos;
         if (buffer->int_buffer_[buffer->skip+z]==tokens->SENTENCE_MARKER) {
            current_sentence--;
            error("Bug: concordances that contain a sentence marker {S} cannot be used in an unsorted concord.ind file\n");
            position_from_eos = 0;
            start_from_eos = 0;
         }
      }
       position_in_tokens=start_pos;
    }
    else {
       /* If we have to go forward */
      for (int z=position_in_tokens; z<start_pos; z++) {
         int token_size=0;
         if (options->original_file_offsets==0 || options->uima_offsets==NULL || buffer->int_buffer_[buffer->skip+z]!=tokens->SENTENCE_MARKER) {
            token_size=token_length[buffer->int_buffer_[buffer->skip+z]];
         }
         start_pos_char=start_pos_char+token_size;
         position_from_eos=position_from_eos+token_size;
         start_from_eos=position_from_eos;
         if (buffer->int_buffer_[buffer->skip+z]==tokens->SENTENCE_MARKER) {
            current_sentence++;
            position_from_eos = 0;
            start_from_eos = 0;
         }
      }
    }
    position_in_chars=start_pos_char;
    position_in_tokens=start_pos;

    if (matches->m.start_pos_in_token<matches->m.end_pos_in_token) {
       /* If the match is made of several tokens, we must set end_pos_in_char
        * to the beginning of the next token */
       int start_of_first_token=start_pos_char;
       start_pos_char=start_of_first_token+matches->m.start_pos_in_char;

       end_pos_char=start_of_first_token;
       end_from_eos=start_from_eos;

       /* We update 'end_pos_char' in the same way */
       for (int z=start_pos;z<end_pos;z++) {
          int token_size=0;
          if (options->original_file_offsets==0 || options->uima_offsets==NULL || buffer->int_buffer_[buffer->skip+z]!=tokens->SENTENCE_MARKER) {
             token_size=token_length[buffer->int_buffer_[buffer->skip+z]];
          }
          end_pos_char=end_pos_char+token_size;
          end_from_eos=end_from_eos+token_size;
       }
       end_pos_char=end_pos_char+matches->m.end_pos_in_char+1;
       end_from_eos=end_from_eos+matches->m.end_pos_in_char+1;
    } else {
       /* If we work on just one token, we can set directly start_pos_in_char
        * and end_pos_in_char. DO NOT SWAP THE FOLLOWING LINES! */
       end_pos_char=start_pos_char+matches->m.end_pos_in_char+1;
       start_pos_char=start_pos_char+matches->m.start_pos_in_char;
       end_from_eos=start_from_eos+matches->m.end_pos_in_char+1;
    }

    /* Now we extract the 3 parts of the concordance */
    extract_left_context(start_pos,matches->m.start_pos_in_char,left,tokens,options,token_length,buffer);
    extract_match(start_pos,matches->m.start_pos_in_char,end_pos,matches->m.end_pos_in_char,matches->output,middle,tokens,buffer);
    /* To compute the 3rd part (right context), we need to know the length of
     * the matched sequence in displayable characters. */
    int match_length_in_displayable_chars;
    if (options->thai_mode) {match_length_in_displayable_chars=u_strlen_Thai(middle);}
    else {
        if (options->result_mode==LEMMATIZE_) {
            /* In lemmatize mode, we hide the output */
            int i;
            for (i=1;middle[i]!='/' && middle[i]!='\0';i++) {}
            match_length_in_displayable_chars=i;
        } else {
            match_length_in_displayable_chars=u_strlen(middle);
        }
    }
    /* Then we can compute the right context */
    extract_right_context(end_pos,matches->m.end_pos_in_char,right,tokens,match_length_in_displayable_chars,
                              options,buffer);
    /* If we must produce a GlossaNet concordance, we look for a URL. After the
     * function call, 'is_a_good_match' can be set to 0 if the match
     * was a part of a URL instead of a valid match. */
    if (expected_result==GLOSSANET_) {
        is_a_good_match=extract_href(end_pos,href,tokens,buffer,open_bracket,close_bracket);
    }
    /* We compute the shift due to the new lines that count for 2 characters */
    unichar positions[1024];
    unichar positions_from_eos[100];
    /* And we use it to compute the bounds of the matched sequence in characters
     * from the beginning of the text file. */
    int shift=get_shift(n_enter_char,enter_pos,matches->m.start_pos_in_token,options->snt_offsets);
    start_pos_char=start_pos_char+shift;
    /* The shift value can be different at the end of the match since new lines
     * can occur inside a match. We add +1 because the last token of the match may
     * contain itself a shift, and get_shift returns the shift before the given
     * position */
    shift=get_shift(n_enter_char,enter_pos,matches->m.end_pos_in_token+1,options->snt_offsets);
    end_pos_char=end_pos_char+shift;
    if ((options->only_matches || options->original_file_offsets) && options->uima_offsets!=NULL) {
        /* In UIMA mode, we use the offset file to produce start and end positions
         * relative to the original input file, before any Unitex operation */
        int first_token=matches->m.start_pos_in_token;
        int last_token=matches->m.end_pos_in_token;
        start_pos_char=uima_offset_token_start_pos(options->uima_offsets,first_token);
        end_pos_char=uima_offset_token_end_pos(options->uima_offsets,last_token);
    }
    /* Finally, we copy the sequence bounds and t he sentence number into 'positions'. */
    u_sprintf(positions,"\t%d %d %d %d",start_pos_char,end_pos_char,current_sentence,concord_ind_match_number);
    const unichar* closest_tag=NULL;
    if (options->PRLG_data!=NULL) {
        int first_token=matches->m.start_pos_in_token;
        int offset_in_original_file=uima_offset_token_start_pos(options->uima_offsets,first_token);
        closest_tag=get_closest_PRLG_tag(options->PRLG_data,offset_in_original_file);
    }
    u_sprintf(positions_from_eos,"%d\t%d\t%d",current_sentence,start_from_eos,end_from_eos);
    /* Now we save the concordance line to the output file, but only if
     * it's a valid match. */
    if (is_a_good_match) {
        if (options->sort_mode!=TEXT_ORDER) {
            /* If we must reverse the left context in thai mode,
             * we must reverse initial vowels with their following consonants. */
            if (options->thai_mode) {
                reverse_initial_vowels_thai(left);
            }
        }
        /* We save the 3 parts of the concordance line according to the sort mode */
        switch(options->sort_mode) {
            case TEXT_ORDER:
            if(expected_result==XALIGN_) u_fprintf(output,"%S\t%S",positions_from_eos,middle);
                else u_fprintf(output,"%S\t%S\t%S",left,middle,right);
                break;
            case LEFT_CENTER:  u_fprintf(output,"%R\t%S\t%S",left,middle,right); break;
            case LEFT_RIGHT:   u_fprintf(output,"%R\t%S\t%S",left,right,middle); break;
            case CENTER_LEFT:  u_fprintf(output,"%S\t%R\t%S",middle,left,right); break;
            case CENTER_RIGHT: u_fprintf(output,"%S\t%S\t%R",middle,right,left);    break;
            case RIGHT_LEFT:   u_fprintf(output,"%S\t%R\t%S",right,left,middle); break;
            case RIGHT_CENTER: u_fprintf(output,"%S\t%S\t%R",right,middle,left);    break;
        }
        /* And we add the position information */
        if(expected_result!=XALIGN_) u_fputs(positions,output);
        /* And the GlossaNet URL if needed */
        if (expected_result==GLOSSANET_) {
            u_fprintf(output,"\t%S",href);
        }
        if (closest_tag!=NULL) {
            u_fprintf(output,"%C[%S",PRLG_DELIMITOR,closest_tag);
            int padding=options->PRLG_data->max_width-u_strlen(closest_tag);
            for (int k=0;k<padding;k++) u_fprintf(output," ");
            u_fprintf(output,"]");
        }
        u_fprintf(output,"\n");
        /* We increase the number of matches actually written to the output */
        number_of_matches++;
    }
    /* Finally, we go on the next match */
    matches_tmp=matches;
    matches=matches->next;
    free_match_list_element(matches_tmp);
}
af_release_mapfile_pointer(buffer->amf,buffer->int_buffer_);
free_vector_int(renumber);
free(unichar_buffer);
free(buffer);
return number_of_matches;
}


/**
 * This function prints the token 'buffer[offset_in_buffer]' to the output.
 * If the token is a space or a line break, which are the same in 'tokens',
 * the 'enter_pos' array is used to decide, whether a space or a line break
 * has to be printed. 'n_enter_char' is the length of the 'enter_pos' array.
 * 'pos_in_enter_pos' is the current position in this array. The function
 * returns the updated current position in the 'pos_in_enter_pos' array.
 */
static int fprint_token(U_FILE* output,int convLFtoCRLF,struct text_tokens* tokens,long int offset_in_buffer,
                int current_global_position,int n_enter_char,const int* enter_pos, int* len_written,
                int pos_in_enter_pos,struct buffer_mapped* buffer) {
/* We look for the new line that is closer (but after) to the token to print */
while (pos_in_enter_pos < n_enter_char) {
    if ((current_global_position+offset_in_buffer) < enter_pos[pos_in_enter_pos]) {
        /* We have found the new line that follows the token to print, so
         * we can stop. */
        break;
    }
    else if ((current_global_position+offset_in_buffer) > enter_pos[pos_in_enter_pos]) {
        /* The current new line is still before the token to print, so we go on */
        pos_in_enter_pos++;
        continue;
    }
    else if ((current_global_position+offset_in_buffer) == enter_pos[pos_in_enter_pos]) {
        /* The token to print is a new line, so we print it and return */
        pos_in_enter_pos++;
        if (output != NULL) {
            u_fputc_conv_lf_to_crlf_option((unichar)'\n', output, convLFtoCRLF);
        }
        if (len_written != NULL) {
            (*len_written) += 1+convLFtoCRLF;
        }
        return pos_in_enter_pos;
    }
}
/* The token to print is not a new line, so we print it and return */
const unichar* token_to_write=tokens->token[buffer->int_buffer_[buffer->skip + offset_in_buffer]];
if (output != NULL) {
    u_fputs_conv_lf_to_crlf_option(token_to_write, output, convLFtoCRLF);
}
if (len_written != NULL) {
    (*len_written) += (int)u_strlen(token_to_write);
}
return pos_in_enter_pos;
}


/**
 * This function saves the text from the token #'current_global_position'
 * to the token #'match_start'. The text is printed in the file 'output'.
 * The function returns the updated current position in the 'pos_in_enter_pos'
 * array.
 *
 * The function also makes sure that the last token #match_end has been loaded into the buffer.
 */
static int move_in_text_with_writing(int match_start,int match_end,ABSTRACTMAPFILE* /*text*/,struct text_tokens* tokens,
                                int current_global_position,U_FILE* output,int convLFtoCRLF,int* len_written,int* len_skipped,
                                int n_enter_char,const int* enter_pos,int pos_in_enter_pos,
                                struct buffer_mapped* buffer,int *pos_int_char) {
buf_map_int_pseudo_seek(buffer,current_global_position);
int last_pos_to_be_loaded=match_end+1;
/* We read what we want to write in the output file + all the tokens of the match */
buffer->size=(int)buf_map_int_pseudo_read(buffer,(last_pos_to_be_loaded-current_global_position));
if (buffer->size>0) {
   /* We indicate that we are at the beginning of a token */
   (*pos_int_char)=0;
}
int last_pos_to_be_written=buffer->size-(match_end+1-match_start);
for (int i=0;i<last_pos_to_be_written;i++) {
    pos_in_enter_pos=fprint_token(output,convLFtoCRLF,tokens,i,current_global_position,
                                    n_enter_char,enter_pos,len_written,pos_in_enter_pos,
                                    buffer);
}


if (len_skipped != NULL) {
    for (int i = last_pos_to_be_written;i+0<buffer->size;i++) {
        pos_in_enter_pos=fprint_token(NULL, convLFtoCRLF,tokens, i, current_global_position,
            n_enter_char, enter_pos, len_skipped, pos_in_enter_pos,
            buffer);
    }
}
return pos_in_enter_pos;
}


/**
 * This function saves all the text from the token n� 'current_global_position' to
 * the end.
 */
static int move_to_end_of_text_with_writing(ABSTRACTMAPFILE* /*text*/,struct text_tokens* tokens,
                                    int current_global_position,U_FILE* output,int convLFtoCRLF,int* len_written,
                                    int n_enter_char,const int* enter_pos,int pos_in_enter_pos,
                                    struct buffer_mapped* buffer) {
//long int address=current_global_position*sizeof(int);
//fseek(text,address,SEEK_SET);
buf_map_int_pseudo_seek(buffer,current_global_position);
//while (0!=(buffer->size = (int)fread(buffer->int_buffer,sizeof(int),buffer->MAXIMUM_BUFFER_SIZE,text))) {
while (0!=(buffer->size = (int)buf_map_int_pseudo_read(buffer,buffer->nb_item))) {
    for (long address=0;address<buffer->size;address++) {
        pos_in_enter_pos=fprint_token(output,convLFtoCRLF,tokens,address,current_global_position,
                                        n_enter_char,enter_pos,len_written,pos_in_enter_pos,buffer);
    }
   current_global_position = current_global_position+(int)buffer->nb_item;
}
return pos_in_enter_pos;
}


/**
 * This function loads the "concord.ind" file 'concordance' and uses it
 * to produce a modified version of the original text. The output is saved
 * in a file named 'output_name'. The output is obtained by replacing
 * matched sequences by their associated outputs (note that if there is no
 * output, the matched sequence is deleted). In case of overlapping matches,
 * priority is given to left most one. If 2 matches start at the same position,
 * the longest is preferred. If 2 matches start and end at the same positions,
 * then the first one is arbitrarily preferred.
 */
static void create_modified_text_file(const VersatileEncodingConfig* vec,U_FILE* concordance,ABSTRACTMAPFILE* text,
                                      struct text_tokens* tokens,char* output_name,int convLFtoCRLF,
                                      int n_enter_char,const int* enter_pos, vector_uima_offset* uima_offsets,
                                      const char* offset_file_name, vector_offset* v_offsets) {
U_FILE* output=u_fopen(vec,output_name,U_WRITE);
if (output==NULL) {
    u_fclose(concordance);
    af_close_mapfile(text);
    fatal_error("Cannot write file %s\n",output_name);
}
U_FILE* f_offsets=NULL;
if ((offset_file_name!=NULL) && ((*offset_file_name)!='\0')) {
    f_offsets=u_fopen(vec, offset_file_name, U_WRITE);
}
int do_offset_compute = f_offsets || v_offsets;
struct match_list* matches;
struct match_list* matches_tmp;
int current_global_position_in_token=0;
int current_global_position_in_char=0;
int pos_in_output=0;
/* We allocate a buffer to read the tokens of the text */
//struct buffer* buffer=new_buffer_for_file(INTEGER_BUFFER,text);
struct buffer_mapped* buffer=(struct buffer_mapped*)malloc(sizeof(struct buffer_mapped));
buffer->amf=(text);
buffer->int_buffer_=(const int*)af_get_mapfile_pointer(buffer->amf);
buffer->nb_item=af_get_mapfile_size(buffer->amf)/sizeof(int);
buffer->skip=0;
buffer->pos_next_read=0;
buffer->size=0;
int pos_in_original=0;
/* We load the match list */
matches=load_match_list(concordance,NULL,NULL);
int pos_in_enter_pos=0;
int pos_original_tokenized=0;
u_printf("Merging outputs with text...\n");
while (matches!=NULL) {
    while (matches!=NULL &&
              (matches->m.start_pos_in_token<current_global_position_in_token
                || (matches->m.start_pos_in_token==current_global_position_in_token && matches->m.start_pos_in_char<current_global_position_in_char)
               )
           ) {
        /* If we must ignore this match because it is overlapping a previous match */
        matches_tmp=matches;
        matches=matches->next;
        free_match_list_element(matches_tmp);
    }
    if (matches!=NULL) {
        /* There, we are sure that we have a valid match to process */
        int pos_in_output_before=pos_in_output;
        int size_skipped=0;

        int copied_begin_first_token = 0;

        pos_in_enter_pos=move_in_text_with_writing(matches->m.start_pos_in_token,matches->m.end_pos_in_token,text,tokens,
                                                    current_global_position_in_token,output,convLFtoCRLF,
                                                    do_offset_compute ? (&pos_in_output) : NULL,
                                                    do_offset_compute ? (&size_skipped) : NULL,
                                                    n_enter_char,enter_pos,pos_in_enter_pos,
                                                    buffer,&current_global_position_in_char);
        int size_copied=pos_in_output-pos_in_output_before;
        pos_original_tokenized+=size_copied;
        /* Now, we are sure that the buffer contains all we want */
        /* If the match doesn't start at the beginning of the token, we add the prefix */
        int zz=matches->m.start_pos_in_token-current_global_position_in_token;
        size_t pos_token_original_before_match=buffer->skip+zz;
        unichar* first_token=tokens->token[buffer->int_buffer_[pos_token_original_before_match]];
        if ((uima_offsets != NULL) && (matches->m.start_pos_in_token < uima_offset_tokens_count(uima_offsets))) {
            pos_in_original = uima_offset_token_start_pos(uima_offsets,matches->m.start_pos_in_token);
        }
        for (int i=current_global_position_in_char;i<matches->m.start_pos_in_char;i++) {
           u_fputc_conv_lf_to_crlf_option(first_token[i],output,convLFtoCRLF);
           pos_in_output++;
           pos_in_original++;
           pos_original_tokenized++;
           copied_begin_first_token++;
        }
        int pos_in_output_before_match=pos_in_output;
        if (matches->output!=NULL) {
            u_fputs_conv_lf_to_crlf_option(matches->output,output,convLFtoCRLF);
            pos_in_output+=(int)u_strlen(matches->output);
        }
        zz=matches->m.end_pos_in_token-current_global_position_in_token;
        size_t pos_token_original_after_match = buffer->skip + zz;
        unichar* last_token=tokens->token[buffer->int_buffer_[pos_token_original_after_match]];
        int pos_end_in_original=0;
        if ((uima_offsets != NULL) && (matches->m.end_pos_in_token < uima_offset_tokens_count(uima_offsets))) {
            pos_end_in_original = uima_offset_token_start_pos(uima_offsets,matches->m.end_pos_in_token);
            pos_end_in_original += matches->m.end_pos_in_char;
        }


        if (last_token[matches->m.end_pos_in_char+1]=='\0') {
           /* If we have completely consumed the last token of the match */
           current_global_position_in_token=matches->m.end_pos_in_token+1;
           current_global_position_in_char=0;
        } else {
           current_global_position_in_token=matches->m.end_pos_in_token;
           current_global_position_in_char=matches->m.end_pos_in_char+1;
        }
        /* If it was the last match or if the next match starts on another token,
         * we dump the end of the current token, if any */
        int nb_char_from_last_token=0;
        if (current_global_position_in_char!=0 &&
              (matches->next==NULL || matches->next->m.start_pos_in_token!=current_global_position_in_token)) {
           for (int i=current_global_position_in_char;last_token[i]!='\0';i++) {
              u_fputc_conv_lf_to_crlf_option(last_token[i],output,convLFtoCRLF);
              nb_char_from_last_token++;
           }
           /* We update the position in tokens so that 'move_to_end_of_text_with_writing'
            * will work fine */
           current_global_position_in_token++;
        }


        if (f_offsets && uima_offsets) {
            u_fprintf(f_offsets, "%d %d %d %d\n", pos_in_original, pos_end_in_original + 1, pos_in_output_before_match, pos_in_output);
        }

        if (f_offsets && (!uima_offsets)) {
            u_fprintf(f_offsets, "%d %d %d %d\n", pos_original_tokenized, pos_original_tokenized + ((size_skipped - copied_begin_first_token) - nb_char_from_last_token),
                pos_in_output_before_match, pos_in_output);
        }

        if (v_offsets) {
            vector_offset_add(v_offsets, pos_original_tokenized, pos_original_tokenized + ((size_skipped - copied_begin_first_token) - nb_char_from_last_token),
                pos_in_output_before_match, pos_in_output);
        }

        pos_original_tokenized += ((size_skipped - copied_begin_first_token));
        pos_in_output+=nb_char_from_last_token;

        /* We skip to the next match of the list */
        matches_tmp=matches;
        matches=matches->next;
        free_match_list_element(matches_tmp);
    }
}
/* Finally, we don't forget to dump all the text that may remain after the
 * last match. */
move_to_end_of_text_with_writing(text,tokens,current_global_position_in_token,output,convLFtoCRLF,
                                do_offset_compute ? (&pos_in_output) : NULL,
                                n_enter_char,enter_pos,pos_in_enter_pos,buffer);
af_release_mapfile_pointer(buffer->amf,buffer->int_buffer_);
free(buffer);
u_fclose(output);
if (f_offsets) {
    u_fclose(f_offsets);
}
u_printf("Done.\n");
}



/**
 * Allocates, initializes and returns a struct conc_opt.
 */
struct conc_opt* new_conc_opt() {
struct conc_opt* opt=(struct conc_opt*)malloc(sizeof(struct conc_opt));
if (opt==NULL) {
   fatal_alloc_error("new_conc_opt");
}
opt->only_ambiguous=0;
opt->sort_mode=TEXT_ORDER;
opt->left_context=0;
opt->right_context=0;
opt->left_context_until_eos=0;
opt->right_context_until_eos=0;
opt->thai_mode=0;
opt->fontname=NULL;
opt->fontsize=0;
opt->result_mode=HTML_;
opt->output[0]='\0';
opt->script=NULL;
opt->sort_alphabet=NULL;
opt->working_directory[0]='\0';
opt->snt_offsets=NULL;
opt->uima_offsets=NULL;
opt->PRLG_data=NULL;
opt->only_matches=0;
opt->original_file_offsets=0;
opt->output_offsets[0]='\0';
opt->input_offsets[0] = '\0';
opt->convLFtoCRLF=1;
return opt;
}


/**
 * Frees all the memory associated with the given structure.
 */
void free_conc_opt(struct conc_opt* opt) {
if (opt==NULL) return;
if (opt->fontname!=NULL) free(opt->fontname);
if (opt->script!=NULL) free(opt->script);
if (opt->sort_alphabet!=NULL) free(opt->sort_alphabet);
free_vector_int(opt->snt_offsets);
free_uima_offsets(opt->uima_offsets);
free_PRLG(opt->PRLG_data);
free(opt);
}

} // namespace unitex
