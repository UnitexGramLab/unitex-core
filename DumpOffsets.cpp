/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS)
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.smartversion.com/unitex-contribution/
 * contact : info@winimage.com
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "Alphabet.h"
#include "String_hash.h"
#include "File.h"
#include "Copyright.h"
#include "DELA.h"
#include "Error.h"
#include "Vector.h"
#include "HashTable.h"
#include "UnitexGetOpt.h"
#include "Token.h"
#include "Text_tokens.h"
#include "DumpOffsets.h"
#include "Offsets.h"

#define NEW_DENORMALIZE_FIX_EXPERIMENT

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_DumpOffsets =
         "Usage: DumpOffsets [OPTIONS] <txt>\n"
         "\n"
         "  <txt>: a offset file to read\n"
         "\n"
         "OPTIONS:\n"
         "  -o X/--old=X: name of old file to read\n"
         "  -n X/--new=X: name of new file to read\n"
         "  -p X/--output=X: name of output dump file to write\n"
         "  -f/--full: dump common text additionaly\n"
         "  -d/--denorm: denormalize the whole text\n"
         "  -q/--quiet: display no message\n"
         "  -c/--no_escape_sequence: don't escape text sequence\n"
         "  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
         "  -h/--help: this help\n"
         "\n"
         "DumpOffsets dump sequence offset to study them.\n"
         "\n" \
         "Example:\n" \
         "UnitexToolLogger Normalize -r .\\resource\\Norm.txt .\\work\\text_file.txt --output_offsets .\\work\\text_file_offset.txt\n" \
         "UnitexToolLogger DumpOffsets -o .\\work\\text_file_offset.txt -n .\\work\\text_file_offset.snt -p .\\work\\dump\\dump_offsets.txt .\\work\\text_file_offset.txt\n" \
         "\n" \
         "\n" \
         "\n" \
         "Other Usage: DumpOffsets [-m/--merge] [OPTIONS] <txt>\n"
         "\n"
         "  <txt>: a offset file to read\n"
         "\n"
         "OPTIONS:\n"
         "  -o X/--old=X: name of old offset file to read\n"
         "  -p X/--output=X: name of output merged offset file to write\n"
         "Merge two offset file produced by two successive modification of text\n"
         "\n" \
         "\n" \
         "\n" \
         "Other Usage: DumpOffsets [-v/--convert_modified_to_common] [OPTIONS] <txt>\n" \
         "\n" \
         "  <txt>: a offset file to read\n" \
         "\n" \
         "OPTIONS:\n" \
         "  -s N/--old_size=N: size of original file (in characters)\n" \
         "  -S N/--new_size=N: size of modified file (in characters)\n" \
         "  -p X/--output=X: name of output common offset file to write\n" \
         "  -h/--help: this help\n" \
         "Create an offset file which list offset of common string between the original and modified file. At least one size must be provided\n" \
         "\n" \
         "\n" \
         "\n" \
         "Other Usage: DumpOffsets [-M/--convert_common_to_modified] [OPTIONS] <txt>\n" \
         "\n" \
         "  <txt>: a offset file to read\n" \
         "\n" \
         "OPTIONS:\n" \
         "  -s N/--old_size=N: size of original file (in characters)\n" \
         "  -S N/--new_size=N: size of modified file (in characters)\n" \
         "  -p X/--output=X: name of output common offset file to write\n" \
         "  -h/--help: this help\n" \
         "Create a standard modified offset file from offset of common string between the original and modified file. Both size must be provided\n" \
         "\n" \
         "\n" \
         "\n" \
         "Other Usage: DumpOffsets -o <list_of_position_file_to_read.txt> -p <list_to_create> -T <offset_file_to_read>\n" \
         "\n" \
         "  <list_of_position_file_to_read.txt> is a text file with just one number (a position) at each line." \
         "\n" \
         "This will convert a list of position using the offset file\n" \
         "The created file contain the converted position at each line, with a + en end of line if the character\n" \
         "at this position is on result file, a - is it was removed.\n" \
         "Using -t instead -T will do the reverse translation\n" \
         "\n" \
         "\n" \
         ""
         ;

static void usage() {
  display_copyright_notice();
  u_printf(usage_DumpOffsets);
}


const char* optstring_DumpOffsets=":VhfumdvMtTs:S:o:n:p:k:q:r:";
const struct option_TS lopts_DumpOffsets[]={
  {"old",required_argument_TS, NULL,'o'},
  {"new",required_argument_TS,NULL,'n'},
  {"output",required_argument_TS,NULL,'p'},
  {"no_escape_sequence",required_argument_TS,NULL,'c'},
  {"merge",no_argument_TS,NULL,'m'},
  {"convert_modified_to_common",no_argument_TS,NULL,'v'},
  {"convert_common_to_modified",no_argument_TS,NULL,'M'},
  {"old_size",required_argument_TS,NULL,'s'},
  {"new_size",required_argument_TS,NULL,'S'},
  {"full",no_argument_TS,NULL,'f'},
  {"denorm",no_argument_TS,NULL,'d'},
  {"quiet",no_argument_TS,NULL,'u'},
  {"replacement_rules",required_argument_TS,NULL,'r'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help", no_argument_TS, NULL, 'h'},
  {NULL, no_argument_TS, NULL, 0}
};






static size_t around_power_two(size_t size_needed) {
    size_t size_alloc = 1;
    while (size_alloc < size_needed) {
        size_alloc *= 2;
    }
    return size_alloc;
}

/**
 * FIXME(jhondoe) This code runs well, but isn't really optimized
 */
#define READ_FILE_BUFFER_SIZE 65536
static unichar* read_text_file(U_FILE* f, int* filesize){
    *filesize = 0;
    size_t size_alloc_buffer = around_power_two(sizeof(unichar));
    unichar* text = (unichar *)malloc(size_alloc_buffer);
    if (!text){
        alloc_error("read_text_file");
        return NULL;
    }

    int size_read;
    int pos_in_text = 0;
    do {
        size_t new_size_alloc_buffer = around_power_two(sizeof(unichar)*(((size_t)pos_in_text) + READ_FILE_BUFFER_SIZE + 1));
        unichar* more_text = (new_size_alloc_buffer == size_alloc_buffer) ? text : (unichar *) realloc(text, new_size_alloc_buffer);
        if (!more_text) {
          alloc_error("read_text_file");
          free (text);
          return NULL;
        }
        text = more_text;
        size_alloc_buffer = new_size_alloc_buffer;
        size_read = u_fget_unichars_raw(text + pos_in_text, READ_FILE_BUFFER_SIZE, f);
        if (size_read == EOF) {
            size_read = 0;
        }
        pos_in_text += size_read;
        *(text + pos_in_text) = '\0';
    } while (size_read != 0);

    *filesize = pos_in_text;
    return text;
}


static void read_text_file(const VersatileEncodingConfig* cfg, const char*filename, unichar** buffer, int *filesize)
{
    U_FILE* f = u_fopen(cfg, filename, U_READ);
    if (f == NULL) {
        error("cannot read file %s", filename);
        return;
    }
    *buffer = read_text_file(f, filesize);
    u_fclose(f);
}


static int DumpSequence(U_FILE* f,const unichar* text, int textsize, int start, int end, int escape, int quotes)
{
    if (end < start)
    {
        u_fprintf(f, "Invalid sequence : end before start !\n");
        return 1;
    }
    else
    if (end > textsize)
    {
        u_fprintf(f, "Invalid sequence : end after end of file !\n");
        return 1;
    }
    else
    if (end == start)
    {
        if(quotes)
            u_fprintf(f, "empty sequence\n");
        return 0;
    }
    else
    {
        if(quotes)
            u_fprintf(f, "'");
        for (int i = start; i < end; i++) {
            unichar c = *(text + i);
            if (escape) {
                if (c >= 0x20) {
                    u_fputc(c, f);
                }
                else {
                    switch (c)
                    {
                        case '\r' : u_fprintf(f, "\\r"); break;
                        case '\n': u_fprintf(f, "\\n"); break;
                        case '\t': u_fprintf(f, "\\t"); break;
                        default: u_fprintf(f,"\\x%02x", (unsigned int)c); break;
                    }
                }
            }
            else {
                u_fputc_raw(c, f);
            }
        }
        if(quotes)
            u_fprintf(f, "'\n");
    }
    return 0;
}


/*
static int DenormalizeSequence(U_FILE* f,const unichar* old_text, int old_textsize, int old_start, int old_end,const unichar* new_text, int new_textsize, int new_start, int new_end,struct string_hash* replacements)
{
    if(old_end < old_start || new_end < new_start || old_end > old_textsize || new_end > new_textsize)
        return 1;
    else if(old_end == old_start || new_start == new_end)
        return 0;
    int i = old_start;
    unichar old_c = *(old_text + i);
    unichar *denorm_text = NULL;

    denorm_text = (unichar *)malloc(sizeof(unichar) * (new_end - new_start + 1));
    if(denorm_text == NULL)
        return 1;

    for(int k = new_start; k < new_end; k++)
        denorm_text[k-new_start] = *(new_text + k);
    denorm_text[new_end - new_start] = 0;

    int k = 0;
    unichar new_c = denorm_text[k];
    // First for loop replaces [] by {}
    while(i<old_end && k+new_start<new_end ) {
        while(!(old_c =='[' || old_c =='{' || old_c ==']' || old_c =='}') && i<old_end) {
            i++;
            old_c = *(old_text + i);
        }
        while(!(new_c =='[' || new_c =='{' || new_c ==']' || new_c =='}') && k+new_start<new_end) {
            k++;
            new_c = denorm_text[k];
        }

        if(old_c=='{' && new_c=='[')
            denorm_text[k] = old_c;
        else if(old_c=='}' && new_c==']')
            denorm_text[k] = old_c;
        i++;
        k++;
    }
    i = old_start;
    int j = 0;
    unichar* old_str = NULL;
    old_str = (unichar *)malloc(sizeof(unichar) * 2);
    old_c = *(old_text + i);
    old_str[0] = old_c;
    old_str[1] = '\0';
    new_c = denorm_text[j];

    // Second for loop restores white spaces
    // We are assuming that the replacement rules change a character into a whitespace
    while(i<old_end && j+new_start<new_end ) {

        while(!(old_c ==' ' || old_c =='\t' || old_c =='\n' || old_c =='\r') && i<old_end && get_value_index(old_str, replacements,DONT_INSERT)==NO_VALUE_INDEX) {
            i++;
            old_c = *(old_text + i);
            old_str[0] = old_c;
            old_str[1] = '\0';
        }

        while(!(new_c ==' ' || new_c =='\t' || new_c =='\n' || new_c =='\r') && j+new_start<new_end) {
            u_fputc(new_c, f);
            j++;
            new_c = denorm_text[j];
        }

        while((new_c ==' ' || new_c =='\t' || new_c =='\n' || new_c =='\r') && j+new_start<new_end) {
            j++;
            new_c = denorm_text[j];
        }

        while((old_c ==' ' || old_c =='\t' || old_c =='\n' || old_c =='\r' || get_value_index(old_str, replacements,DONT_INSERT)!=NO_VALUE_INDEX) && i<old_end) {
            // Do not add \r with \n if the original file did not have it
            u_fputc_raw(old_c, f);
            i++;
            old_c = *(old_text + i);
            old_str[0] = old_c;
            old_str[1] = '\0';
        }
    }
    while(j+new_start<new_end) {
        u_fputc(new_c, f);
        j++;
        new_c = denorm_text[j];
    }
    if(old_str != NULL)
        free(old_str);
    if(denorm_text !=NULL)
        free(denorm_text);

    return 0;
}
*/


static int DenormalizeSequence_new(U_FILE* f,const unichar* old_text, int old_textsize, int old_start, int old_end,const unichar* new_text, int new_textsize, int new_start, int new_end,struct string_hash* replacements)
{
    if(old_end < old_start || new_end < new_start || old_end > old_textsize || new_end > new_textsize)
        return 1;
    else if(old_end == old_start || new_start == new_end)
        return 0;
    int i = old_start;
    unichar old_c = *(old_text + i);

    unichar* old_str = NULL;
    old_str = (unichar *)malloc(sizeof(unichar) * 2);

    int j = new_start;
    unichar new_c = *(new_text + j);

    int prev_i = -1;
    int prev_j = -1;
    while(i<old_end && j<new_end ) {
        // protect against infinite loop
        if ((i == prev_i) && (j == prev_j))
        {
            error("infinite loop in denormalize_new, exit from loop\n");
            return 1;
        }
        prev_i = i;
        prev_j = j;
        while(i<old_end && j<new_end && old_c == new_c) {
            u_fputc_raw(new_c, f);
            i++;
            j++;
            old_c = *(old_text + i);
            new_c = *(new_text + j);
        }
        old_str[0] = old_c;
        old_str[1] = '\0';
        int idx = get_value_index(old_str, replacements,DONT_INSERT);
        if(idx!=NO_VALUE_INDEX && replacements->value[idx][0]== new_c) {
            u_fputc_raw(old_c, f);
            i++;
            j += u_strlen(replacements->value[idx]);
            old_c = *(old_text + i);
            new_c = *(new_text + j);
        }
        int wht_spaces = 0;
        while(j < new_end && (new_c ==' ' || new_c =='\t' || new_c =='\n' || new_c =='\r')) {
            j++;
            new_c = *(new_text + j);
            wht_spaces = 1;
        }

        while(i < old_end && (old_c ==' ' || old_c =='\t' || old_c =='\n' || old_c =='\r') && wht_spaces == 1) {
            u_fputc_raw(old_c, f);
            i++;
            old_c = *(old_text + i);
        }

        if(old_c != new_c) {
            if(old_c ==' ' || old_c =='\t' || old_c =='\n' || old_c =='\r') {
                if(j-1 > new_start && (*(new_text + j - 1) ==' ' || *(new_text + j - 1) =='\r' || *(new_text + j - 1) =='\n')) {
                    while(i<old_end && (old_c ==' ' || old_c =='\t' || old_c =='\n' || old_c =='\r')) {
                        u_fputc_raw(old_c, f);
                        i++;
                        old_c = *(old_text + i);
                    }
                }
                else if(j-1 > new_start && i-1 > old_start && *(new_text + j - 1) == *(old_text + i - 1) ) {
                    if(i+1 < old_end && old_c =='\r' &&  *(old_text + i + 1) == '\n') {
                        if(i+3 < old_end && j+1 < new_end  && *(old_text + i + 2)== new_c && *(old_text + i + 3)== *(new_text + j + 1)) {
                            u_fputc_raw(old_c, f);
                            u_fputc_raw('\n', f);
                            i += 2;
                            old_c = *(old_text + i);
                        }
                        else if(new_c=='<'){
                            while(j+1<new_end && !(new_c== '>' && *(new_text + j + 1) ==' ')) {
                                u_fputc_raw(new_c, f);
                                j++;
                                new_c = *(new_text + j);
                            }
                            if(new_c== '>' && *(new_text + j + 1) ==' ') {
                                u_fputc_raw(new_c, f);
                                u_fputc_raw(old_c, f);
                                u_fputc_raw('\n', f);
                                i += 2;
                                old_c = *(old_text + i);
                                j++;
                                new_c = *(new_text + j);
                            }
                        }
                    }
                    else if(old_c ==' ' && new_c=='<') {
                        while(j+1<new_end && !(new_c== '>' && 
                                    (*(new_text + j + 1) ==' ' ||
                                     *(new_text + j + 1) =='\n'))) {
                            u_fputc_raw(new_c, f);
                            j++;
                            new_c = *(new_text + j);
                         }
                        u_fputc_raw(new_c, f);
                        u_fputc_raw(' ', f);
                        j += 2;
                        new_c = *(new_text + j);
                        i++;
                        old_c = *(old_text + i);
                    }
                    else if(old_c=='\n' && i-1 > old_start && *(old_text + i - 1)!='\r' &&  i+1 < old_end && *(new_text + j)== *(old_text + i + 1)) {
                        u_fputc_raw(old_c, f);
                        i++;
                        old_c = *(old_text + i);
                    }
                    else if(old_c=='\n' && new_c=='<') {
                        int resume = i;
                        /*Check if the tag in the old text is same as the tag in the new text*/
                        i++;
                        old_c = *(old_text + i);
                        while(old_c != new_c) {
                            i++;
                            if (i >= old_end) {
                                error("End of old text reached!\n");
                                break;
                            }
                            old_c = *(old_text + i);
                        }
                        /*if they are same then we write white spaces*/
                        if(i+3<old_end && j+3<new_end && *(old_text + i + 1) == *(new_text + j + 1) && *(old_text + i + 2) == *(new_text + j + 2)) {
                            i = resume;
                            old_c = *(old_text + i);
                            while(old_c != new_c) {
                                u_fputc_raw(old_c, f);
                                i++;
                                if (i >= old_end) {
                                    error("End of old text reached in denormalize!!\n");
                                    break;
                                }
                                old_c = *(old_text + i);
                            }
                        }
                        else { //otherwise write the new tag first
                            i = resume;
                            old_c = *(old_text + i);
                            while(new_c != '>') {
                                u_fputc(new_c,f);
                                j++;
                                if (j >= new_end) {
                                    error("End of new text reached in denormalize!!\n");
                                    break;
                                }
                                new_c = *(new_text + j);
                            }
                            u_fputc(new_c,f);
                            j++;
                            new_c = *(new_text + j);
                        }
                    }
#ifdef NEW_DENORMALIZE_FIX_EXPERIMENT
                    else if(old_c ==' ' || old_c =='\t' || old_c =='\n' || old_c =='\r') { //add the missing white spaces back
                        while(old_c ==' ' || old_c =='\t' || old_c =='\n' || old_c =='\r') {
                            u_fputc_raw(old_c, f);
                            i++;
                            if (i >= old_end) {
                                error("End of old text reached in denormalize!!!\n");
                                break;
                            }
                            old_c = *(old_text + i);
                        }
                    }
#endif
                    else {
                        while(j < new_end && old_c != new_c) {
                            u_fputc_raw(new_c, f);
                            j++;
                            new_c = *(new_text + j);
                        }
                    }
                }
                else {
                    u_fputc_raw(old_c, f);
                    i++;
                    old_c = *(old_text + i);
                }
            }
#ifdef NEW_DENORMALIZE_FIX_EXPERIMENT
            else if(new_c !='<' && i < old_end) {
                if(new_c=='{') {
                    /* check if it is annotation in raw format
                     If yes then preference is given to the new text*/
                    int look_ahead = j+2;
                    int is_annotation = 0;
                    while(look_ahead < new_end) {
                        if(*(new_text+look_ahead-1) ==',' && *(new_text+look_ahead) =='.') {
                            is_annotation = 1;
                        }
                        if(*(new_text+look_ahead-2) =='}') {
                            break;
                        }
                        look_ahead++;
                    }


                    if(look_ahead == new_end && *(new_text+look_ahead-1) == '}') {
                        look_ahead += 1;
                    }
                    if(is_annotation == 1 && *(new_text+look_ahead-2) =='}') {
                        /* The braces are around the text from the old text
                         so we need to increment the pointer in the old text also
                         */
                        while(j<look_ahead-1) {
                            u_fputc_raw(new_c, f);
                            j++;
                            if(new_c == *(old_text + i)) {
                                i++;
                            }
                            new_c = *(new_text + j);
                        }
                        old_c = *(old_text + i);
                    }
                }
                /*we know that characters don't match and the new text
                 * is not the start of a tag so we give preference to the
                 * old text
                 */
                while(i < old_end && old_c != new_c) {
                    u_fputc_raw(old_c, f);
                    i++;
                    old_c = *(old_text + i);
                }
            }
            else if(new_c =='<') {
                /* Most likely it is a tag*/
                int look_ahead = j;
                while(new_c == '<') {
                    while(look_ahead < new_end && new_c !='>') {
                    // Go to the end of the tag
                        look_ahead++;
                        new_c = *(new_text + look_ahead);
                    }
                    look_ahead++;
                    new_c = *(new_text + look_ahead);
                }
                if(new_c != old_c && old_c != ' ') {
                //if the text after the tag(s) is not same then print old text first
                    while(old_c != new_c && i < old_end) {
                        u_fputc_raw(old_c, f);
                        i++;
                        old_c = *(old_text + i);
                    }
                }
                //else print the tag
                for(int pos = j; pos < look_ahead; pos++) {
                    u_fputc_raw(*(new_text+pos),f);
                }
                j = look_ahead;
            }
#endif
            else {
                while(j < new_end && old_c != new_c) {
                    u_fputc_raw(new_c, f);
                    j++;
                    new_c = *(new_text + j);
                }
            }
        }
    }
#ifdef NEW_DENORMALIZE_FIX_EXPERIMENT
    for (;i<old_end; i++) {
        old_c = *(old_text + i);
        u_fputc_raw(old_c, f);
    }
#endif
    for (;j<new_end; j++) {
        new_c = *(new_text + j);
        u_fputc_raw(new_c, f);
    }
    if(old_str != NULL)
        free(old_str);
    return 0;
}


static int CompareCommon(U_FILE* f,
    const unichar* old_text, int old_size, int old_pos, int old_limit,
    const unichar* new_text, int new_size, int new_pos, int new_limit)
{
    if ((old_limit < old_pos) || (new_limit < new_pos)) {
        u_fprintf(f, "Invalid common sequence : end before start !\n");
        return 1;
    } else if ((old_limit > old_size) || (new_limit > new_size)) {
        u_fprintf(f, "Invalid commonsequence : end after end of file !\n");
        return 1;
    } else if ((old_limit - old_pos) != (new_limit - new_pos)) {
        u_fprintf(f, "Invalid common sequence : size mismatch !\n");
        return 1;
    }

    for (int i = 0; i < (old_limit - old_pos); i++) {
        if ((*(old_text + old_pos + i)) != (*(new_text + new_pos + i))) {
            u_fprintf(f, "Difference on common sequence !\n");
            return 1;
        }
    }
    return 0;
}


static void load_offset_translation(const VersatileEncodingConfig* vec, const char* name, int** pos_from_file, int* nb_position) {
    U_FILE* f = u_fopen(vec, name, U_READ);
    if (!f) {
     error("Cannot open file %s", name);
     return;
    }

    int nb_allocated = 1;
    *pos_from_file = (int*)malloc(sizeof(int) * nb_allocated);

    if ((*pos_from_file) == NULL) {
        alloc_error("load_offset_translation");
        u_fclose(f);
        return;
    }

    int a,n;

    *nb_position = 0;
    while ((n = u_fscanf(f, "%d", &a)) != EOF) {
        if (n != 1) {
            error("Corrupted file %s\n", name);
            free(*pos_from_file);
            u_fclose(f);
            return;
        }
        if ((*nb_position) >= nb_allocated) {
            nb_allocated *= 2;
            *pos_from_file = (int*)realloc(*pos_from_file, sizeof(int) * nb_allocated);
            if ((*pos_from_file) == NULL) {
                alloc_error("load_offset_translation");
                free(*pos_from_file);
                u_fclose(f);
            }
        }
        *((*pos_from_file) + (*nb_position)) = a;
        (*nb_position)++;
    }
    free(*pos_from_file);
    u_fclose(f);
    return ;
}


int DumpOffsetApply(const VersatileEncodingConfig* vec, const char* old_filename, const char* new_filename,
    const char* offset_file_name, const char* output,
    int full, int quiet, int escape,int quotes)
{
    unichar* old_text = NULL;
    int old_read_size = 0;
    read_text_file(vec, old_filename, &old_text, &old_read_size);

    unichar* new_text = NULL;
    int new_read_size = 0;
    read_text_file(vec, new_filename, &new_text, &new_read_size);

    vector_offset* offsets = load_offsets(vec, offset_file_name);
    if (offsets == NULL) {
        free(old_text);
        free(new_text);
        error("Cannot read file %s", offset_file_name);
        return DEFAULT_ERROR_CODE;
    }
    int coherency = 1;
    U_FILE* fout = u_fopen(vec, output, U_WRITE);
    if (fout == NULL) {
        error("Cannot create file %s", output);
        free_vector_offset(offsets);
        free(old_text);
        free(new_text);
        return DEFAULT_ERROR_CODE;
    }
    for (int i = 0; i < offsets->nbelems; i++) {
        Offsets curOffset = offsets->tab[i];
        Offsets prevOffset;
        if (i > 0) {
            prevOffset = offsets->tab[i - 1];
        }
        else {
            prevOffset.old_end = prevOffset.new_end = 0;
        }
        if (CompareCommon(fout, old_text, old_read_size, prevOffset.old_end, curOffset.old_start,
            new_text, new_read_size, prevOffset.new_end, curOffset.new_start)) {
            coherency = 0;
        }
        if (full) {
            u_fprintf(fout, "===========================================\n\n");
            u_fprintf(fout, "Common zone:\n\n");
            DumpSequence(fout, old_text, old_read_size, prevOffset.old_end, curOffset.old_start, escape, quotes);
            DumpSequence(fout, new_text, new_read_size, prevOffset.new_end, curOffset.new_start, escape, quotes);
        }
        if ((i > 0) || (full)) {
            u_fprintf(fout, "-------------------------------------------\n\n");
        }
        u_fprintf(fout, "%8d: %d.%d -> %d.%d\n", i, curOffset.old_start, curOffset.old_end, curOffset.new_start, curOffset.new_end);
        if (DumpSequence(fout, old_text, old_read_size, curOffset.old_start, curOffset.old_end, escape, quotes)) {
            coherency = 0;
        }
        if (DumpSequence(fout, new_text, new_read_size, curOffset.new_start, curOffset.new_end, escape, quotes)) {
            coherency = 0;
        }

        if ((i + 1) == offsets->nbelems) {
            if (CompareCommon(fout, old_text, old_read_size, curOffset.old_end, old_read_size,
                new_text, new_read_size, curOffset.new_end, new_read_size))
            {
                coherency = 0;
            }

            if ((full) && ((curOffset.old_end != old_read_size) || (curOffset.new_end != new_read_size))) {
                u_fprintf(fout, "===========================================\n\n");
                u_fprintf(fout, "Last Common zone:\n\n");
                DumpSequence(fout, old_text, old_read_size, curOffset.old_end, old_read_size, escape, quotes);
                DumpSequence(fout, new_text, new_read_size, curOffset.new_end, new_read_size, escape, quotes);
            }
        }
    }

    if (offsets->nbelems == 0)
    {
        if (CompareCommon(fout, old_text, old_read_size, 0, old_read_size,
            new_text, new_read_size, 0, new_read_size))
        {
            coherency = 0;
        }

        if ((full) && ((0 != old_read_size) || (0 != new_read_size))) {
            u_fprintf(fout, "===========================================\n\n");
            u_fprintf(fout, "Last Common zone:\n\n");
            DumpSequence(fout, old_text, old_read_size, 0, old_read_size, escape, quotes);
            DumpSequence(fout, new_text, new_read_size, 0, new_read_size, escape, quotes);
        }
    }

    free_vector_offset(offsets);
    free(old_text);
    free(new_text);
    u_fprintf(fout, "\n\nOffset file is %s.\n", coherency ? "coherent" : "not coherent");
    u_fclose(fout);
    if (!quiet) {
        u_printf("Offset file is %s.\n", coherency ? "coherent" : "not coherent");
        u_printf("\nDumpOffsets dump done, file %s created.\n", output);
    }
    return coherency ? 0 : 1;
}

int Denormalize(const VersatileEncodingConfig* vec, const char* old_filename, const char* new_filename,
    const char* offset_file_name, const char* output,int escape, int quotes, const char* rules) {
    unichar* old_text = NULL;
    int old_read_size = 0;
    read_text_file(vec, old_filename, &old_text, &old_read_size);

    unichar* new_text = NULL;
    int new_read_size = 0;
    read_text_file(vec, new_filename, &new_text, &new_read_size);

    vector_offset* offsets = load_offsets(vec, offset_file_name);
    if (offsets == NULL) {
        error("Cannot read file %s", offset_file_name);
        return 1;
    }

    U_FILE* fout = u_fopen(vec, output, U_WRITE);

    struct string_hash* replacements = NULL;
    if (rules != NULL && rules[0] != '\0') {
        replacements = load_key_value_list(rules, vec, '\t');
    if (replacements == NULL) {
            error("Cannot load replacement rules file %s\n", rules);
            replacements = new_string_hash();
        }
    }
    else {
    replacements = new_string_hash();
    }

    for (int i = 0; i < offsets->nbelems; i++) {
        Offsets curOffset = offsets->tab[i];
        Offsets prevOffset;
        if (i > 0) {
            prevOffset = offsets->tab[i - 1];
        }
        else {
            prevOffset.old_end = prevOffset.new_end = 0;
        }
        DumpSequence(fout, new_text, new_read_size, prevOffset.new_end, curOffset.new_start, escape, quotes);
        DenormalizeSequence_new(fout, old_text, old_read_size, curOffset.old_start, curOffset.old_end, new_text, new_read_size, curOffset.new_start, curOffset.new_end,replacements);
    }

    free_string_hash(replacements);
    free_vector_offset(offsets);
    free(old_text);
    free(new_text);
    u_fclose(fout);

    return 0;
}

int main_DumpOffsets(int argc,char* const argv[]) {
if (argc==1) {
   usage();
   return SUCCESS_RETURN_CODE;
}

char old_filename[FILENAME_MAX]="";
char new_filename[FILENAME_MAX]="";
char output[FILENAME_MAX]="";
char offset_file_name[FILENAME_MAX]="";
char rules[FILENAME_MAX]="";
VersatileEncodingConfig vec=VEC_DEFAULT;
int escape=1;
int merge=0;
int full=0;
int quiet=0;
int denorm=0;
int convert_modified_to_common=0;
int convert_common_to_modified=0;
int translate_position_file=0;
int translate_position_file_invert=0;
int old_size=-1;
int new_size=-1;
int val,index=-1;
int quotes=1;
//char foo=0;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF!=(val=options.parse_long(argc,argv,optstring_DumpOffsets,lopts_DumpOffsets,&index))) {
   switch(val) {
   case 'o': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty old file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(old_filename, options.vars()->optarg);
             break;
   case 'n': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty new file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(new_filename, options.vars()->optarg);
             break;
   case 'p': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty output file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(output, options.vars()->optarg);
             break;
   case 'r': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty replacement rule file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(rules,options.vars()->optarg);
             break;
   case 's': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty size for old file\n");
                return USAGE_ERROR_CODE;
             }
             old_size = atoi(options.vars()->optarg);
             break;
   case 'S': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty size for new file\n");
                return USAGE_ERROR_CODE;
             }
             new_size = atoi(options.vars()->optarg);
             break;
   case 'k': if (options.vars()->optarg[0]=='\0') {
                error("Empty input_encoding argument\n");
                return USAGE_ERROR_CODE;
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
                error("Empty output_encoding argument\n");
                return USAGE_ERROR_CODE;
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   case 'c': escape = 0; break;
   case 'f': full = 1; break;
   case 'm': merge = 1; break;
   case 'v': convert_modified_to_common = 1; break;
   case 'M': convert_common_to_modified = 1; break;
   case 't': translate_position_file = 1; break;
   case 'T': translate_position_file_invert = 1; break;
   case 'u': quiet = 1; break;
   case 'd': denorm = 1; break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': usage();
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_DumpOffsets[index].name);
             return USAGE_ERROR_CODE;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
   }
   index=-1;
}

if (options.vars()->optind!=argc-1) {
   error("Invalid arguments: rerun with --help\n");
   return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
  // freeing all allocated memory
  return SUCCESS_RETURN_CODE;
}

strcpy(offset_file_name, argv[options.vars()->optind]);

if (translate_position_file || translate_position_file_invert) {
    int* pos_from_file = NULL;
    int nb_translations = 0;
    load_offset_translation(&vec, old_filename, &pos_from_file, &nb_translations);
    vector_offset* modified_offset = load_offsets(&vec, offset_file_name);
    offset_translation* translation = (offset_translation*)malloc(sizeof(offset_translation) * (nb_translations + 1));
    for (int i = 0;i < nb_translations;i++) {
        (translation + i)->position_to_translate = *(pos_from_file + i);
        (translation + i)->sort_order = i;
    }
    free(pos_from_file);
    translate_offset(translation, nb_translations, modified_offset, translate_position_file_invert ? 1 : 0);
    free_vector_offset(modified_offset);

    U_FILE* f_output_translation = u_fopen(&vec, output, U_WRITE);
    for (int i = 0;i < nb_translations;i++) {
        u_fprintf(f_output_translation, "%d %s\n",
            (translation + i)->translated_position,
            ((translation + i)->translation_pos_in_common == -1) ? "-" : "+");
    }
    u_fclose(f_output_translation);
    free(translation);
    return SUCCESS_RETURN_CODE;
} else if (convert_modified_to_common) {
    int ret_value = 1;
    vector_offset* modified_offset = NULL;
    if ((old_size == -1) && (new_size == -1)) {
        error("you must specify old or new filesize");
    }
    else {
        modified_offset = load_offsets(&vec, offset_file_name);
    }
    if (modified_offset == NULL) {
        error("error in reading offset file %s\n", offset_file_name);
    } else {
        vector_offset* common_offset = modified_offsets_to_common(modified_offset, old_size, new_size);
        if (common_offset == NULL) {
            error("invalid offset conversion from file %s with file size from %d to %d\n", old_size, new_size);
        }
        else {
            save_offsets(&vec, output, common_offset);
            free_vector_offset(common_offset);
            ret_value=0;
        }
        free_vector_offset(modified_offset);
    }
    return ret_value;
} else if (convert_common_to_modified) {
    int ret_value = 1;
    vector_offset* common_offset = NULL;
    if ((old_size == -1) && (new_size == -1)) {
        error("you must specify old and new filesize");
    }
    else {
        common_offset = load_offsets(&vec, offset_file_name);
    }
    if (common_offset == NULL) {
        error("error in reading offset file %s\n", offset_file_name);
    }
    else {
        vector_offset* modified_offset = common_offsets_to_modified(common_offset, old_size, new_size);
        if (modified_offset == NULL) {
            error("invalid offset conversion from file %s with file size from %d to %d\n", old_size, new_size);
        }
        else {
            save_offsets(&vec, output, modified_offset);
            free_vector_offset(modified_offset);
            ret_value=0;
        }
        free_vector_offset(common_offset);
    }
    return ret_value;
} else if (merge) {
    vector_offset* prev_offsets = load_offsets(&vec, old_filename);
    vector_offset* offsets = load_offsets(&vec, offset_file_name);

    U_FILE* f_output_offsets = u_fopen(&vec, output, U_WRITE);
    if (f_output_offsets == NULL) {
        error("Cannot create offset file %s\n", output);
        return DEFAULT_ERROR_CODE;
    }
    process_offsets(prev_offsets, offsets, f_output_offsets);
    free_vector_offset(prev_offsets);
    free_vector_offset(offsets);
    u_fclose(f_output_offsets);
    if (!quiet) {
        u_printf("\nDumpOffsets dump done, file %s created.\n", output);
    }
    return SUCCESS_RETURN_CODE;
} else if (denorm) {
    quotes = 0;
    escape = 0;
    return Denormalize(&vec, old_filename, new_filename, offset_file_name, output,escape, quotes, rules);
}
else {
    return DumpOffsetApply(&vec, old_filename, new_filename, offset_file_name, output,
        full, quiet, escape, quotes);
}


}


////////////////////////////////////////////////////////////

typedef struct
{
    char original_file[FILENAME_MAX];
    char processed_file[FILENAME_MAX];
    char output_file[FILENAME_MAX];
    char preprocessed_offset[FILENAME_MAX];
    char processed_offset[FILENAME_MAX];
    char input_offsets[FILENAME_MAX];
    char output_offsets[FILENAME_MAX];
} UnPreprocessParam;


#define COPY_BUFFER_SIZE 0x800

int copy_chars_between_unicharfiles(U_FILE* input, U_FILE* output, int* pos_in, int* pos_out, int nb_needed, unichar* buffer, size_t size_buffer)
{
    int result = 0;
    while (nb_needed > 0) {
        int nb_this = (nb_needed < (int)(size_buffer-1)) ? nb_needed : (int)(size_buffer-1);
        //int result_read = u_fread_raw(buffer, nb_this, input);
        int result_read = u_fget_unichars_raw(buffer, nb_this, input);
        if (result_read < 0) {
            result_read = 0;
        }
        if (result_read >= (int)size_buffer)
            result_read = (int)(size_buffer - 1);
        *(buffer + result_read) = 0;

        if ((result_read>0) && (output != NULL)) {
            u_fputs_raw(buffer, output);
        }
        result += result_read;
        nb_needed -= result_read;
        if ((result_read == 0) || (nb_needed == 0))
            break;
    }
    if (pos_in != NULL) {
        (*pos_in) += result;
    }
    if (pos_out != NULL) {
        (*pos_out) += result;
    }
    return result;
}


int skip_chars_in_unicharfiles(U_FILE* input, int* pos_in, int nb_needed, unichar* buffer, size_t size_buffer)
{
    return copy_chars_between_unicharfiles(input, NULL, pos_in, NULL, nb_needed, buffer, size_buffer);
}


// if (*accumulated_skip) > 0, we skip *accumulated_skip char and let * accumulated_skip = 0
// if (*accumulated_skip) < 0, we do nothing
int skip_chars_in_unicharfiles_as_possible(U_FILE* input, int* pos_in, int* accumulated_skip, unichar* buffer, size_t size_buffer)
{
    if ((*accumulated_skip) < 0)
        return 0;
    int result = copy_chars_between_unicharfiles(input, NULL, pos_in, NULL, *accumulated_skip, buffer, size_buffer);
    (*accumulated_skip) -= result;
    return result;
}

#define uprep_min(a,b) (((a)<(b)) ? (a) : (b))

int runUnPreprocess(const UnPreprocessParam*params, const VersatileEncodingConfig *vec)
{
    vector_offset* v_preprocessed_offset = NULL;
    vector_offset* v_processed_offset = NULL;
    vector_offset* v_modification_offset = NULL;
    unichar buffer_for_copy[COPY_BUFFER_SIZE];
    size_t buffer_for_copy_size = COPY_BUFFER_SIZE;

    if (params->preprocessed_offset[0] != '\0') {
        v_preprocessed_offset = load_offsets(vec, params->preprocessed_offset);
        }


    if (params->processed_offset[0] != '\0') {
        v_processed_offset = load_offsets(vec, params->processed_offset);
    }

    U_FILE* f_input_original_file = u_fopen(vec, params->original_file, U_READ);
    U_FILE* f_input_processed = u_fopen(vec, params->processed_file, U_READ);
    U_FILE* f_output = NULL;

    if ((v_preprocessed_offset == NULL) || (v_processed_offset == NULL) || (f_input_original_file == NULL) || (f_input_processed == NULL)) {
        error("Cannot open source files\n");
    }
    else
    {
        if ((f_output = u_fopen(vec, params->output_file, U_WRITE)) == NULL) {
            error("Cannot open output file files %s\n", params->output_file);
        }
    }

    if (f_output == NULL) {
        free_vector_offset(v_preprocessed_offset);
        free_vector_offset(v_processed_offset);
        u_fclose(f_input_original_file);
        u_fclose(f_input_processed);
        return DEFAULT_ERROR_CODE;
    }

    if (params->output_offsets[0] != '\0') {
        v_modification_offset = new_vector_offset();
    }

    int pos_in_processed_offset = 0;
    int pos_in_preprocessed_offset = 0;
    int pos_in_original_file = 0;
    int pos_in_preprocessed_file = 0;
    int pos_in_processed_file = 0;
    int pos_in_output = 0;
    int delta_output_against_processed = 0;
    int size_skip_original_for_ignored_offset = 0;

    for (;;)
    {
        // we search position in next change in processed_offset
        int next_stop_processed = (pos_in_processed_offset >= v_processed_offset->nbelems) ?
            -1 : v_processed_offset->tab[pos_in_processed_offset].old_start;


        // now, between pos_in_preprocessed_file and next_stop_preprocessed, we can integrate revert modification made by pre-processing

        int next_stop_preprocessed = (pos_in_preprocessed_offset >= v_preprocessed_offset->nbelems) ?
                -1 : v_preprocessed_offset->tab[pos_in_preprocessed_offset].new_start;


        enum {
            preprocess_next_action_skip_preprocess,
            preprocess_next_action_apply_preprocess,
            preprocess_next_action_apply_process,
            preprocess_next_action_break
        } preprocess_next_action ;

        // if there is no more preprocess offset, just apply next process offset
        if ((next_stop_processed == -1) && (next_stop_preprocessed == -1))
        {
            preprocess_next_action = preprocess_next_action_break;
        }
        else
        if (next_stop_preprocessed == -1) {
            preprocess_next_action = preprocess_next_action_apply_process;
        }
        else // if process offset is before next preprocess offset and doesn't overlap
        if (v_processed_offset->tab[pos_in_processed_offset].old_end <= v_preprocessed_offset->tab[pos_in_preprocessed_offset].new_start) {
            preprocess_next_action = preprocess_next_action_apply_process;
        }
        else // if process offset is after next preprocess offset and doesn't overlap
        if (v_preprocessed_offset->tab[pos_in_preprocessed_offset].new_end <= v_processed_offset->tab[pos_in_processed_offset].old_start) {
            preprocess_next_action = preprocess_next_action_apply_preprocess;
        }
        else {
            preprocess_next_action = preprocess_next_action_skip_preprocess;
        }


        if (preprocess_next_action == preprocess_next_action_break)
            break;

        int first_stop;
        if ((next_stop_processed != -1) && (next_stop_preprocessed != -1))
            first_stop = (next_stop_processed < next_stop_preprocessed) ? next_stop_processed : next_stop_preprocessed;
        else first_stop = (next_stop_preprocessed == -1) ? next_stop_processed : next_stop_preprocessed;

        int until_first_stop = first_stop - pos_in_preprocessed_file;
        // copy unmodified text before first stop from processed file
        copy_chars_between_unicharfiles(f_input_processed, f_output, &pos_in_processed_file, &pos_in_output, until_first_stop, buffer_for_copy, buffer_for_copy_size);
        pos_in_preprocessed_file += until_first_stop;

        size_skip_original_for_ignored_offset += until_first_stop;

        // test if there the next modification is a processed modification to keep
        if (preprocess_next_action == preprocess_next_action_apply_process)
        {
            int size_old_in_process_modification = v_processed_offset->tab[pos_in_processed_offset].old_end - v_processed_offset->tab[pos_in_processed_offset].old_start;
            int size_new_in_process_modification = v_processed_offset->tab[pos_in_processed_offset].new_end - v_processed_offset->tab[pos_in_processed_offset].new_start;

            // copy next text from processed file
            copy_chars_between_unicharfiles(f_input_processed, f_output, &pos_in_processed_file, &pos_in_output, size_new_in_process_modification, buffer_for_copy, buffer_for_copy_size);
            pos_in_preprocessed_file += size_old_in_process_modification;

            size_skip_original_for_ignored_offset += size_old_in_process_modification;

            pos_in_processed_offset++;
        }
        else
        // test if there the next modification if a preprocessed modification which overlap and will be ignored
        // if this test is true, we knwown that next_stop_processed > next_stop_preprocessed)

        // we test if there is overlap. If there is overlap, we ignore preprocessed modification
        if (preprocess_next_action == preprocess_next_action_skip_preprocess)
        {
            int size_old_in_preprocess_skip = v_preprocessed_offset->tab[pos_in_preprocessed_offset].old_end - v_preprocessed_offset->tab[pos_in_preprocessed_offset].old_start;
            int size_new_in_preprocess_skip = v_preprocessed_offset->tab[pos_in_preprocessed_offset].new_end - v_preprocessed_offset->tab[pos_in_preprocessed_offset].new_start;
            size_skip_original_for_ignored_offset += size_old_in_preprocess_skip - size_new_in_preprocess_skip;

            pos_in_preprocessed_offset++;
        }
        else // here if (preprocess_next_action == preprocess_next_action_apply_process)
        {
        // the modification is a preprocessed modification to keep

            int size_old_in_preprocess_keep = v_preprocessed_offset->tab[pos_in_preprocessed_offset].old_end - v_preprocessed_offset->tab[pos_in_preprocessed_offset].old_start;
            int size_new_in_preprocess_keep = v_preprocessed_offset->tab[pos_in_preprocessed_offset].new_end - v_preprocessed_offset->tab[pos_in_preprocessed_offset].new_start;

            // copy text from first, original text to replace texte modified by preprocessing and still in processed file
            if (v_modification_offset != NULL) vector_offset_add(v_modification_offset,
                pos_in_processed_file, pos_in_processed_file + size_new_in_preprocess_keep,
                pos_in_processed_file + delta_output_against_processed, pos_in_processed_file + delta_output_against_processed + size_old_in_preprocess_keep);
            delta_output_against_processed += (size_old_in_preprocess_keep - size_new_in_preprocess_keep);

            skip_chars_in_unicharfiles_as_possible(f_input_original_file, &pos_in_original_file, &size_skip_original_for_ignored_offset, buffer_for_copy, buffer_for_copy_size);
            copy_chars_between_unicharfiles(f_input_original_file, f_output, &pos_in_original_file, &pos_in_output, size_old_in_preprocess_keep, buffer_for_copy, buffer_for_copy_size);

            pos_in_preprocessed_file+=size_new_in_preprocess_keep;
            skip_chars_in_unicharfiles(f_input_processed, &pos_in_processed_file, size_new_in_preprocess_keep, buffer_for_copy, buffer_for_copy_size);

            pos_in_preprocessed_offset++;
        }
    }

    // copy unmodified text after all offsets
    for (;;)
    {
        int nb_copy_this = copy_chars_between_unicharfiles(f_input_processed, f_output, &pos_in_processed_file, &pos_in_output, (int)buffer_for_copy_size, buffer_for_copy, buffer_for_copy_size);
        if (nb_copy_this == 0)
            break;
    }

    if (v_modification_offset != NULL) {
        vector_offset* v_input_offsets = NULL;


        /* We deal with offsets only if we have to produce output offsets */
        if (params->input_offsets[0] != '\0') {
            v_input_offsets = load_offsets(vec, params->input_offsets);
        }
        U_FILE* f_output_offsets = u_fopen(vec, params->output_offsets, U_WRITE);
        if (f_output_offsets == NULL) {
            error("Cannot create offset file %s\n", params->output_offsets);
        }
        else {
            process_offsets(v_input_offsets, v_modification_offset, f_output_offsets);
            u_fclose(f_output_offsets);
        }
        free_vector_offset(v_input_offsets);
        free_vector_offset(v_modification_offset);
    }


    u_fclose(f_output);
    free_vector_offset(v_preprocessed_offset);
    free_vector_offset(v_processed_offset);
    u_fclose(f_input_original_file);
    u_fclose(f_input_processed);

    return SUCCESS_RETURN_CODE;
}


const char* usage_UnPreprocess =
    "Usage: usage_UnPreprocess [OPTIONS] <txt>\n" \
    "\n" \
    "  -r X/--original_file=X: name of corpus file before preprocessing\n" \
    "  -p X/--preprocessed_file=X: name of corpus file after preprocessing\n" \
    "  -d X/--processed_file=X: name of corpus file after complete processing\n" \
    "  -f X/--preprocessed_offset=X: name of offset file about preprocessing step\n" \
    "  -s X/--processed_offset=X: name of file file about processing step\n" \
    "  --output_offsets = XXX: offset file to be produced between processed file and result.\n" \
    "  <txt>: a modified version of processed file to be produced, where modification\n" \
    "              made durring preprocessing step are reverted if possible.\n" \
    "\n" \
    "OPTIONS:\n" \
    "\n" \
    "\n" \
    "" \
    ;

static void display_usage_UnPreprocess() {
    display_copyright_notice();
    u_printf(usage_UnPreprocess);
}

const char* optstring_UnPreprocess = "h$:@:r:d:f:s:Vq:r:";
const struct option_TS lopts_UnPreprocess[] = {

  {"original_file",required_argument_TS,NULL,'r'},
  {"processed_file",required_argument_TS,NULL,'d'},
  {"preprocessed_offset",required_argument_TS,NULL,'f'},
  {"processed_offset",required_argument_TS,NULL,'s'},

  {"input_offsets",required_argument_TS,NULL,'$'},
  {"output_offsets",required_argument_TS,NULL,'@'},
  {"input_encoding",required_argument_TS,NULL,'k'},
  {"output_encoding",required_argument_TS,NULL,'q'},
  {"only_verify_arguments",no_argument_TS,NULL,'V'},
  {"help", no_argument_TS, NULL, 'h'},
  {NULL, no_argument_TS, NULL, 0 }
};


int main_UnPreprocess(int argc, char* const argv[]) {
if (argc==1) {
   display_usage_UnPreprocess();
   return SUCCESS_RETURN_CODE;
}

int val,index=-1;
UnitexGetOpt options;
UnPreprocessParam params;
VersatileEncodingConfig vec = VEC_DEFAULT;
bool only_verify_arguments = false;

params.output_offsets[0] = params.input_offsets[0] =
params.original_file[0] = params.processed_file[0] = params.output_file[0] =
params.processed_offset[0] = params.preprocessed_offset[0] = '\0';

while (EOF!=(val=options.parse_long(argc,argv,optstring_UnPreprocess,lopts_UnPreprocess,&index))) {
   switch(val) {

   case 'r': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(params.original_file,options.vars()->optarg);
             break;

   case 'd': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(params.processed_file,options.vars()->optarg);
             break;


   case 'f': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(params.preprocessed_offset,options.vars()->optarg);
             break;


   case 's': if (options.vars()->optarg[0]=='\0') {
                error("You must specify a non empty file name\n");
                return USAGE_ERROR_CODE;
             }
             strcpy(params.processed_offset,options.vars()->optarg);
             break;

   case '$': if (options.vars()->optarg[0]=='\0') {
              error("You must specify a non empty input offset file name\n");
              return USAGE_ERROR_CODE;
             }
             strcpy(params.input_offsets,options.vars()->optarg);
             break;

   case '@': if (options.vars()->optarg[0]=='\0') {
              error("You must specify a non empty output offset file name\n");
              return USAGE_ERROR_CODE;
             }
             strcpy(params.output_offsets,options.vars()->optarg);
             break;

   case 'k': if (options.vars()->optarg[0]=='\0') {
              error("Empty input_encoding argument\n");
              return USAGE_ERROR_CODE;
             }
             decode_reading_encoding_parameter(&(vec.mask_encoding_compatibility_input),options.vars()->optarg);
             break;
   case 'q': if (options.vars()->optarg[0]=='\0') {
              error("Empty output_encoding argument\n");
              return USAGE_ERROR_CODE;
             }
             decode_writing_encoding_parameter(&(vec.encoding_output),&(vec.bom_output),options.vars()->optarg);
             break;
   case 'V': only_verify_arguments = true;
             break;
   case 'h': display_usage_UnPreprocess();
             return SUCCESS_RETURN_CODE;
   case ':': index==-1 ? error("Missing argument for option -%c\n",options.vars()->optopt) :
                         error("Missing argument for option --%s\n",lopts_UnPreprocess[index].name);
             return USAGE_ERROR_CODE;
             break;
   case '?': index==-1 ? error("Invalid option -%c\n",options.vars()->optopt) :
                         error("Invalid option --%s\n",options.vars()->optarg);
             return USAGE_ERROR_CODE;
             break;
   }
   index = -1;
}



if (options.vars()->optind != argc - 1) {
    error("Invalid arguments: rerun with --help\n");
    return USAGE_ERROR_CODE;
}

if (only_verify_arguments) {
    // freeing all allocated memory
    return SUCCESS_RETURN_CODE;
}

strcpy(params.output_file, argv[options.vars()->optind]);

int result = runUnPreprocess(&params, &vec);
if (SUCCESS_RETURN_CODE == result) {
    u_printf("UnPreprocess done.\n");
}
return result;
}

} // namespace unitex
