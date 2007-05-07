 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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
#include "File.h"
#include "Copyright.h"
#include "DELA.h"
#include "String_hash.h"
#include "IOBuffer.h"
#include "Error.h"
#include "Buffer.h"
#include "StringParsing.h"


#define MAX_TAG_LENGTH 4000
#define KEEP_CARRIDGE_RETURN 0
#define REMOVE_CARRIDGE_RETURN 1
#define BUFFER_SIZE 1000000
/* When we are at less than 'MARGIN_BEFORE_BUFFER_END' from the end of the buffer,
 * we will refill it, unless we are at the end of the input file. */
#define MARGIN_BEFORE_BUFFER_END (MAX_TAG_LENGTH+1000)


void normalize(FILE*,FILE*,int,struct string_hash*);


void usage() {
u_printf("%S",COPYRIGHT);
u_printf("Usage: Normalize <text> [-no_CR] [-f=EQUIV]\n");
u_printf("     <text>   : text file to be normalized\n");
u_printf("     -no_CR   : this optional parameter indicates that every separator\n");
u_printf("                sequence will be turned into a single space\n\n");
u_printf("     -f=EQUIV : this optional parameter specifies a configuration file\n");
u_printf("                EQUIV that contains replacement instructions in the form\n");
u_printf("                of lines like: input_sequence tab output_sequence\n");
u_printf("                By default, the program only replace { and } by [ and ]\n");
u_printf("Turns every sequence of separator chars (space, tab, new line) into one.\n");
u_printf("If a separator sequence contains a new line char, it is turned to a single new\n");
u_printf("line (except with -no_CR); if not, it is turned into a single space. As\n");
u_printf("a side effect, new line sequences are converted into the Windows style: \\r\\n.\n");
u_printf("If you specifies replacement rules with -f, they will be applied prior\n");
u_printf("to the separator normalization, so you have to take care if you manipulate\n");
u_printf("separators in your replacement rules.\n");
u_printf("The result is stored in a file named file_name.snt.\n");
}


int main(int argc, char **argv) {
/* Every Unitex program must start by this instruction,
 * in order to avoid display problems when called from
 * the graphical interface */
setBufferMode();
FILE* f;
FILE* f_out;
if (argc<2 || argc >4) {
   usage();
   return 0;
}
int mode=KEEP_CARRIDGE_RETURN;
struct string_hash* replacement_rules=NULL;
/* We check the parameters */
if (argc==3) {
   if (strcmp(argv[2],"-no_CR")) {
      if (argv[2][0]=='-' && argv[2][1]=='f' && argv[2][2]=='=') {
         replacement_rules=load_key_value_list(&(argv[2][3]),'\t');
         if (replacement_rules==NULL) {
            error("Cannot load replacement rules file %s\n",&(argv[2][3]));
         }
      }
      else {
         error("Wrong parameter: %s\n",argv[2]);
         return 1;
      }
   }
   else {
      mode=REMOVE_CARRIDGE_RETURN;
   }
}
if (argc==4) {
   if (strcmp(argv[2],"-no_CR")) {
      error("Wrong parameter: %s\n",argv[2]);
      return 1;
   }
   mode=REMOVE_CARRIDGE_RETURN;
   if (argv[3][0]=='-' && argv[3][1]=='f' && argv[3][2]=='=') {
      replacement_rules=load_key_value_list(&(argv[3][3]),'\t');
      if (replacement_rules==NULL) {
         error("Cannot load replacement rules file %s\n",&(argv[3][3]));
      }
   }
   else {
      error("Wrong parameter: %s\n",argv[3]);
      return 1;
   }
}
if (replacement_rules==NULL) {
   /* If there is no replacement rules file, we simulate one */
   replacement_rules=new_string_hash();
}
/* If there is a replacement rule file, we ensure that there are replacement
 * rules for { and }. If not, we add our default ones, so that in any case,
 * we are sure to have rules for { and } */
unichar key[2];
unichar value[2];
u_strcpy(key,"{");
u_strcpy(value,"[");
get_value_index(key,replacement_rules,INSERT_IF_NEEDED,value);
u_strcpy(key,"}");
u_strcpy(value,"]");
get_value_index(key,replacement_rules,INSERT_IF_NEEDED,value);
char tmp_file[FILENAME_MAX];
get_extension(argv[1],tmp_file);
if (!strcmp(tmp_file,".snt")) {
   /* If the file to process has allready the .snt extension, we temporary rename it to
    * .snt.normalizing */
   strcpy(tmp_file,argv[1]);
   strcat(tmp_file,".normalizing");
   rename(argv[1],tmp_file);
   f=u_fopen(tmp_file,U_READ);
}
else {
   /* Otherwise, we just open the file */
   tmp_file[0]='\0';
   f=u_fopen(argv[1],U_READ);
}
if (f==NULL) {
   error("Cannot open file %s\n",argv[1]);
   return 1;
}
/* We set the destination file */
char dest_file[FILENAME_MAX];
remove_extension(argv[1],dest_file);
strcat(dest_file,".snt");
f_out=u_fopen(dest_file,U_WRITE);
if (f_out==NULL) {
   error("Cannot create file %s\n",dest_file);
   u_fclose(f);
   return 1;
}
u_printf("Normalizing %s...\n",argv[1]);
normalize(f,f_out,mode,replacement_rules);
u_printf("\n");
u_fclose(f);
u_fclose(f_out);
free_string_hash(replacement_rules);
if (tmp_file[0]!='\0') {
   /* If we have used a temporary file, we delete it */
   remove(tmp_file);
}
return 0;
}


/**
 * This function produces a normalized version of 'input' and stores it into 'ouput'.
 * The following rules are applied in the given order:
 * 
 * 1) If there is a { at the current position, we try to read a {S}, a {STOP} or
 *    a tag token like {today,.ADV}. If we fail, we replace the { and the }, if any, 
 *    according to the replacement rules. Otherwise, we let the token unchanged.
 * 2) If there is one or more replacement rules that can apply to the current
 *    position in 'input', then we apply the longest one.
 * 3) If we we find a separator (space, tab, new line) sequence, we replace it:
 *    - by a new line if the sequence contains one and if 'carridge_return_policy' is
 *      set to KEEP_CARRIDGE_RETURN;
 *    - by a space otherwise.
 * 4) We copy the character that was read to the output.   
 * 
 * Note that 'replacements' is supposed to contain replacement rules for { and }
 */
void normalize(FILE* input,FILE* output,int carridge_return_policy,struct string_hash* replacements) {
unichar tmp[MAX_TAG_LENGTH];
struct buffer* buffer=new_buffer(BUFFER_SIZE,UNICHAR_BUFFER);
unichar* buff=buffer->unichar_buffer;
/* We define some things that will be used for parsing the buffer */
unichar stop_chars[3];
stop_chars[0]='}';
stop_chars[1]='{';
stop_chars[2]='\0';
unichar forbidden_chars[2];
forbidden_chars[0]='\n';
forbidden_chars[1]='\0';
unichar open_bracket[2];
open_bracket[0]='{';
open_bracket[1]='\0';
unichar close_bracket[2];
close_bracket[0]='}';
close_bracket[1]='\0';
/* First, we fill the buffer */
fill_buffer(buffer,input);
int current_start_pos=0;
u_printf("First block...              \r");
int current_block=1;
while (current_start_pos<buffer->size) {
   if (!buffer->end_of_file
       && current_start_pos>(buffer->size-MARGIN_BEFORE_BUFFER_END)) {
      /* If we must change of block and if we can */
      u_printf("Block %d...              \r",++current_block);
      fill_buffer(buffer,current_start_pos,input);
      current_start_pos=0;
   }
   if (buff[current_start_pos]=='{') {
      /* If we have a {, we try to find a sequence like {....}, that does not contain
       * new lines. If the sequence contains protected character, we want to keep them
       * protected. */
      int old_position=current_start_pos;
      /* If we don't increase the position, the parse will stop on the initial { */
      current_start_pos++;
      tmp[0]='{';
      int code=parse_string(buff,&current_start_pos,&(tmp[1]),stop_chars,forbidden_chars,NULL);
      if (code==P_FORBIDDEN_CHAR || code==P_BACKSLASH_AT_END || buff[current_start_pos]!='}') {
         /* If we have found a new line or a {, or if there is
          * a backslash at the end of the buffer, or if we have reached the end
          * of the buffer, we assume that the initial
          * { was not a tag beginning, so we print the substitute of { */
         u_fprintf(output,"%S",replacements->value[get_value_index(open_bracket,replacements)]);
         /* And we rewind the current position after the { */
         current_start_pos=old_position+1;
      }
      else {
         /* If we have read a sequence like {....}, we assume that there won't be
          * a buffer overflow if we add the } */
         u_strcat(tmp,close_bracket);
         if (!u_strcmp(tmp,"{S}") || !u_strcmp(tmp,"{STOP}") || check_tag_token(tmp)) {
            /* If this is a special tag or a valid tag token, we just print
             * it to the output */
            u_fprintf(output,"%S",tmp);
            current_start_pos++;
         }
         else {
            /* If we have a non valid tag token, we print the equivalent of {
             * and we rewind the current position after the { */
            u_fprintf(output,"%S",replacements->value[get_value_index(open_bracket,replacements)]);
            current_start_pos=old_position+1;
         }
      }
   }
   else {
      /* If we have a character that is not {, first we try to look if there
       * is a replacement to do */
      int key_length;
      int index=get_longest_key_index(&buff[current_start_pos],&key_length,replacements);
      if (index!=NO_VALUE_INDEX) {
         /* If there is something to replace */
         u_fprintf(output,"%S",replacements->value[index]);
         current_start_pos=current_start_pos+key_length;
      }
      else {
         if (buff[current_start_pos]==' ' || buff[current_start_pos]=='\t'
             || buff[current_start_pos]=='\n') {
            /* If we have a separator, we try to read the longest separator sequence
             * that we can read. By the way, we note if it contains a new line */
            int new_line=0;
            while (buff[current_start_pos]==' ' || buff[current_start_pos]=='\t'
                   || buff[current_start_pos]=='\n') {
               /* Note 1: no bound check is needed, since an unichar buffer is always
                *        ended by a \0
                * 
                * Note 2: we don't take into account the case of a buffer ended by
                *         separator while it's not the end of file: that would mean
                *         that the text contains something like MARGIN_BEFORE_BUFFER_END
                *         contiguous separators. Such a text would not be a reasonable one.
                */
               if (buff[current_start_pos]=='\n') {
                  new_line=1;
               }
               current_start_pos++;
            }
            if (new_line && (carridge_return_policy==KEEP_CARRIDGE_RETURN)) {
               /* We print a new line if the sequence contains one and if we are
                * allowed to; otherwise, we print a space. */
               u_fputc('\n',output);
            }
            else {
               u_fputc(' ',output);
            }
         }
         else {
            /* If, finally, we have a normal character to normalize, we just print it */
            u_fputc(buff[current_start_pos++],output);
         }
      }
   }
}
free_buffer(buffer);
}

