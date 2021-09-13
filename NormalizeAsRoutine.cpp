/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "DELA.h"
#include "Error.h"
#include "StringParsing.h"
#include "NormalizeAsRoutine.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

#define MAX_LINE_BUFFER_SIZE (32768)
#define MINIMAL_CHAR_IN_BUFFER_BEFORE_CONTINUE_LINE (256)

#define SIZE_OUTPUT_BUFFER 0x100

struct OUTBUF {
    unichar outbuf[SIZE_OUTPUT_BUFFER + 1];
    int pos;
};

static void WriteOufBuf(struct OUTBUF* pOutBuf, int convLFtoCRLF, const unichar* str, U_FILE *f,
        int flush) {
    for (;;) {

        while (((*str) != 0) && (pOutBuf->pos < SIZE_OUTPUT_BUFFER)) {
            pOutBuf->outbuf[pOutBuf->pos] = *str;
            pOutBuf->pos++;
            str++;
        }

        if ((pOutBuf->pos == SIZE_OUTPUT_BUFFER) || (flush != 0)) {
            pOutBuf->outbuf[pOutBuf->pos] = 0; // add null terminating marker
            u_fprintf_conv_lf_to_crlf_option(f,convLFtoCRLF, "%S", pOutBuf->outbuf);
            pOutBuf->pos = 0;
        }

        if ((*str) == 0)
            break;
    }
}

static void WriteOufBuf(struct OUTBUF* pOutBuf, int convLFtoCRLF, unichar c, U_FILE *f, int flush) {
    unichar u_array[2];
    u_array[0] = c;
    u_array[1] = 0;
    WriteOufBuf(pOutBuf,convLFtoCRLF, u_array, f, flush);
}


/**
 * This function computes the length of the longest prefix and suffix
 * that are common to the key and its associated value. Returns 0
 * if key and value are equals, 0 otherwise.
 */
static inline int get_real_replacement(const unichar* key,int key_size,unichar* value,int *pfx,int *sfx) {
*pfx=0;
*sfx=0;
while (*pfx!=key_size && key[*pfx]==value[*pfx]) (*pfx)++;
if (*pfx==key_size) return 0;
int i=key_size-1;
int j=u_strlen(value)-1;
while (i!=*pfx && j>=0 && key[i]==value[j]) {
    (*sfx)++;
    i--;
    j--;
}
return 1;
}

static inline int is_unicode_space(unichar c) {
    if(c==160 || (c>8191 && c<8201)) {
        return 1;
    }
    return 0;
}


/**
 * The Ustring version, in order to avoid buffer overflow in result.
 * Note that this version appends the string to result instead of replacing
 * its previous content.
 */
/**
 * this is a modified version of parse_string from StringParsing, considering
 * forbidden_chars = '\r' or '\n'
 * stop_char = '{' or '}'
 * chars_to_keep_protected = NULL
 * AND a modification: if a PROTECTION_CHAR is just before a \n or \r, we don't
 *   consider is at PROTECTION_CHAR, because a text in brace cannot contain them
 */
static int parse_string_into_brace(const unichar* s,int *ptr,Ustring* result) {
if (s[*ptr]=='\0') return P_EOS;
while (s[*ptr]!='\0') {
   int is_protection_char=0;
   if (s[*ptr]==PROTECTION_CHAR) {

      if (s[(*ptr)+1]=='\0') {
         /* It must not appear at the end of the string */
         return P_BACKSLASH_AT_END;
      }

      if ((s[(*ptr) + 1] != '\r') && (s[(*ptr)+1]!='\n')) {
         /* If there is a protection character (backslash) before end of line, we do like this is not a protection char */
         is_protection_char=1;
      }
   }

   if (is_protection_char) {
      /* If there is a protection character (backslash) */
      if (s[(*ptr)+1]=='\0') {
         /* It must not appear at the end of the string */
         return P_BACKSLASH_AT_END;
      }
      //if (chars_to_keep_protected==NULL) IS TRUE
      {
         /* If the character must keep its backslash (only when chars_to_keep_protected is not NULL) */
         u_strcat(result,PROTECTION_CHAR);
      }
      u_strcat(result,s[(*ptr)+1]);
      (*ptr)=(*ptr)+2;
   } else {
      /* If we have an unprotected character */
      if ((s[*ptr] == '{') || (s[*ptr] == '}')) {
         /* If it is a stop char, we have finished */
         return P_OK;
      }
      if ((s[*ptr] == '\n') || (s[*ptr] == '\r')) {
         /* If it is a forbidden char, it's an error */
         return P_FORBIDDEN_CHAR;
      }
      /* If it's a normal char, we copy it */
      u_strcat(result,s[(*ptr)++]);
   }
}
/* If we arrive here, we have reached the end of the string without error */
return P_OK;
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
 *    - by a new line if the sequence contains one and if 'carriage_return_policy' is
 *      set to KEEP_CARRIAGE_RETURN;
 *    - by a space otherwise.
 * 4) We copy the character that was read to the output.
 *
 * Note that 'replacements' is supposed to contain replacement rules for { and }
 */
int normalize(const char *fin, const char *fout, const VersatileEncodingConfig* vec,
        int carriage_return_policy, int convLFtoCRLF,const char *rules,
        vector_offset* offsets,
        int separator_normalization) {
    U_FILE* input;
    input = u_fopen(vec, fin, U_READ);
    if (input == NULL) {
        error("Cannot open file %s\n", fin);
        return 1;
    }

    U_FILE* output;
    output = u_fopen(vec, fout, U_WRITE);
    if (output == NULL) {
        error("Cannot create file %s\n", fout);
        u_fclose(input);
        return 1;
    }
    struct string_hash* replacements = NULL;
    if (rules != NULL && rules[0] != '\0') {
        replacements = load_key_value_list(rules, vec, '\t');
        if (replacements == NULL) {
            error("Cannot load replacement rules file %s\n", rules);
            replacements = new_string_hash();
        }
    }
    /* If there is no replacement rules file, we simulate one */
    else {
        replacements = new_string_hash();
    }

    /* If there is a replacement rule file, we ensure that there are replacement
     * rules for { and }. If not, we add our default ones, so that in any case,
     * we are sure to have rules for { and } */
    unichar key[2];
    unichar value[2];
    u_strcpy(key, "{");
    u_strcpy(value, "[");
    get_value_index(key, replacements, INSERT_IF_NEEDED, value);
    u_strcpy(key, "}");
    u_strcpy(value, "]");
    get_value_index(key, replacements, INSERT_IF_NEEDED, value);
    // now we'll assume length of adding new line is 1 + convLFtoCRLF
    convLFtoCRLF = (convLFtoCRLF == 0) ? 0 : 1;
    struct OUTBUF OutBuf;
    OutBuf.pos = 0;
    Ustring* tmp = new_Ustring(MAX_EXPECTED_TAG_LENGTH);
    //struct buffer* buffer=new_buffer_for_file(UNICHAR_BUFFER,input);

    long save_pos = ftell(input);
    fseek(input, 0, SEEK_END);
    long file_size_input = ftell(input);
    fseek(input, save_pos, SEEK_SET);

    int
            line_buffer_size = (int) (((file_size_input + 1)
                    < MAX_LINE_BUFFER_SIZE) ? (file_size_input + 1)
                    : MAX_LINE_BUFFER_SIZE);

    unichar *line_read;
    line_read = (unichar*) malloc((line_buffer_size + 0x10) * sizeof(unichar));
    if (line_read == NULL) {
        fatal_alloc_error("normalize");
    }

    /* We define some things that will be used for parsing the buffer */

    static const unichar open_bracket[] = { '{', 0 };
    static const unichar close_bracket[] = { '}', 0 };
    static const unichar empty_string[] = { 0 };

    int eof_found = 0;
    /* First, we fill the buffer */

    int lastline_was_terminated = 0;

    /* Here are the two variables used to compute offset diffs before and after
     * normalization. We used old_start_pos as a global position variable, whereas
     * current_start_pos is an index that is local to the current buffer */
    int old_start_pos=0,new_start_pos=0;

    /* Beginning of the big loop */
    while (eof_found == 0) {
        int current_start_pos = 0;
        const unichar* buff = line_read;
        int result_read = 0;
        result_read=u_fread_raw(line_read,line_buffer_size,input);
        if (result_read==0) break;
        while (current_start_pos < result_read) {
            if (/*(lastline_was_terminated == 0) &&*/ (eof_found == 0)
                    && (current_start_pos
                            + MINIMAL_CHAR_IN_BUFFER_BEFORE_CONTINUE_LINE
                            >= result_read)) {
                int i;
                int nb_to_keep = result_read - current_start_pos;
                for (i = 0; i < nb_to_keep; i++)
                    line_read[i] = line_read[current_start_pos + i];
                /* This is required to avoid bound checking */
                line_read[i]='\0';
                int result_read_continue = u_fread_raw(line_read+ nb_to_keep,
                        line_buffer_size - nb_to_keep, input);
                if (result_read_continue == 0) {
                    eof_found = lastline_was_terminated = 1;
                } else {
                    /* This is required to avoid bound checking */
                    line_read[nb_to_keep+result_read_continue]='\0';
                }
                result_read = nb_to_keep;
                current_start_pos = 0;

                if (result_read_continue > 0)
                    result_read += result_read_continue;
            }
            /* Now that we have a buffer ready to be processed, we will normalize it */
            if (buff[current_start_pos] == '{') {
                /* If we have a {, we try to find a sequence like {....}, that does not contain
                 * new lines. If the sequence contains protected character, we want to keep them
                 * protected. */
                int old_position = current_start_pos;
                /* If we don't increase the position, the parse will stop on the initial { */
                current_start_pos++;

                u_strcpy(tmp,"{");
                int code = parse_string_into_brace(buff, &current_start_pos, tmp);
                if (code == P_FORBIDDEN_CHAR || code == P_BACKSLASH_AT_END
                        || buff[current_start_pos] != '}') {
                    /* If we have found a new line or a {, or if there is
                     * a backslash at the end of the buffer, or if we have reached the end
                     * of the buffer, we assume that the initial
                     * { was not a tag beginning, so we print the substitute of { */
                    unichar* foo=replacements->value[get_value_index(open_bracket, replacements)];
                    if (offsets!=NULL) vector_offset_add(offsets,old_start_pos,old_start_pos+1,new_start_pos,new_start_pos+u_strlen(foo));
                        WriteOufBuf(&OutBuf, convLFtoCRLF, foo, output, 0);
                    /* And we rewind the current position after the { */
                    current_start_pos = old_position + 1;
                    old_start_pos++;
                    new_start_pos=new_start_pos+u_strlen(foo);
                } else {
                    /* If we have read a sequence like {....}, we assume that there won't be
                     * a buffer overflow if we add the } */
                    u_strcat(tmp, close_bracket);
                    if (!u_strcmp(tmp->str, "{S}") || !u_strcmp(tmp->str, "{STOP}")
                            || check_tag_token(tmp->str,0)) {
                        /* If this is a special tag or a valid tag token, we just print
                         * it to the output */
                        WriteOufBuf(&OutBuf, convLFtoCRLF, tmp->str, output, 0);
                        current_start_pos++;
                        int l=u_strlen(tmp);
                        old_start_pos=old_start_pos+l;
                        new_start_pos=new_start_pos+l;
                    } else {
                        /* If we have a non valid tag token, we print the equivalent of {
                         * and we rewind the current position after the { */
                        unichar* foo=replacements->value[get_value_index(open_bracket, replacements)];
                        if (offsets!=NULL) vector_offset_add(offsets,old_start_pos,old_start_pos+1,new_start_pos,new_start_pos+u_strlen(foo));
                        WriteOufBuf(&OutBuf, convLFtoCRLF, foo, output, 0);
                        current_start_pos = old_position + 1;
                        old_start_pos++;
                        new_start_pos=new_start_pos+u_strlen(foo);
                    }
                }
            } else {
                /* If we have a character that is not {, first we try to look if there
                 * is a replacement to do */
                int key_length;
                int index = get_longest_key_index(&buff[current_start_pos],
                        &key_length, replacements);
                if (index != NO_VALUE_INDEX) {
                    /* If there is something to replace */
                    unichar* foo=replacements->value[index];
                    int common_prefix,common_suffix;
                    int ret=get_real_replacement(buff+current_start_pos,key_length,foo,
                            &common_prefix,&common_suffix);
                    if (offsets!=NULL && ret) vector_offset_add(offsets,old_start_pos+common_prefix,old_start_pos+key_length-common_suffix,new_start_pos+common_prefix,new_start_pos+u_strlen(foo)-common_suffix);
                    /* If we have a replacement rule, we must use it rawly, in case it
                     * deals with separators. To do that, we flush the buffer first */
                    WriteOufBuf(&OutBuf, convLFtoCRLF, U_EMPTY, output, 1);
                    int len;
                    for (len=0;foo[len]!='\0';len++) {
                        u_fputc_raw(foo[len],output);
                    }
                    current_start_pos = current_start_pos + key_length;
                    old_start_pos = old_start_pos + key_length;
                    new_start_pos=new_start_pos+len;
                } else {
                    int old_position=current_start_pos;
                    if (separator_normalization && (buff[current_start_pos] == ' '
                            || buff[current_start_pos] == '\t'
                            || buff[current_start_pos] == '\n'
                            || buff[current_start_pos] == 0x0d
                            || is_unicode_space(buff[current_start_pos]))) {
                        /* If we have a separator, we try to read the longest separator sequence
                         * that we can read. By the way, we note if it contains a new line */
                        int new_line = 0;
                        while (buff[current_start_pos] == ' '
                                || buff[current_start_pos] == '\t'
                                || buff[current_start_pos] == '\n'
                                || buff[current_start_pos] == 0x0d
                                || is_unicode_space(buff[current_start_pos])) {
                            /* Note 1: no bound check is needed, since an unichar buffer is always
                             *        ended by a \0
                             *
                             * Note 2: we don't take into account the case of a buffer ended by
                             *         separator while it's not the end of file: that would mean
                             *         that the text contains something like MARGIN_BEFORE_BUFFER_END
                             *         contiguous separators. Such a text would not be a reasonable one.
                             */
                            if (buff[current_start_pos] == '\n'
                                    || buff[current_start_pos] == 0x0d) {
                                new_line = 1;
                            }
                            current_start_pos++;
                        }
                        int delta=current_start_pos-old_position;
                        if (new_line && (carriage_return_policy
                                == KEEP_CARRIAGE_RETURN)) {
                            /* We print a new line if the sequence contains one and if we are
                             * allowed to; otherwise, we print a space. */
                            if (offsets)
                            {
                                int must_push_offset = 1;
                                // if we replace exactly a CR LF by a CR LF, don't push offset
                                if ((convLFtoCRLF) && (delta == 2))
                                {
                                    if ((buff[old_position] == '\r') && (buff[old_position + 1] == '\n'))
                                        must_push_offset = 0;
                                }
                                if ((!convLFtoCRLF) && (delta == 1))
                                {
                                    if (buff[old_position] == '\n')
                                        must_push_offset = 0;
                                }
                                if (must_push_offset)
                                {
                                    vector_offset_add(offsets, old_start_pos, old_start_pos + delta, new_start_pos, new_start_pos + 1 + convLFtoCRLF);
                                }
                            }

                            old_start_pos=old_start_pos+delta;
                            new_start_pos+=1+convLFtoCRLF;
                            WriteOufBuf(&OutBuf, convLFtoCRLF, '\n', output, 0);
                        } else {
                            if ((delta!=1) || (buff[old_position] != ' ')) {
                                if (offsets!=NULL) vector_offset_add(offsets,old_start_pos,old_start_pos+delta,new_start_pos,new_start_pos+1);
                            }
                            old_start_pos=old_start_pos+delta;
                            new_start_pos++;
                            WriteOufBuf(&OutBuf, convLFtoCRLF, ' ', output, 0);
                        }
                    } else {
                        /* If, finally, we have a normal character to normalize, we just print it */
                        unichar c=buff[current_start_pos];
                        if (c=='\r') {
                            if (buff[current_start_pos+1]=='\n') {
                                /* If we have a real \r\n, we write \n if convLFtoCRLF=0 and keep unmodified \r\n if convLFtoCRLF=1 */
                                old_start_pos+=2;
                                new_start_pos+=1+convLFtoCRLF;
                                // we use 1 as param for convLFtoCRLF of WriteOufBuf to really write a \r\n
                                if ((offsets != NULL) && (!convLFtoCRLF)) {
                                    vector_offset_add(offsets, old_start_pos, old_start_pos + 2, new_start_pos, new_start_pos + 1);
                                }
                                current_start_pos+=2;
                                WriteOufBuf(&OutBuf, convLFtoCRLF, '\n', output, 0);
                            } else {
                                /* The text contains only \r and we will have to turn
                                 * it into \n if convLFtoCRLF==0, and \r\n if convLFtoCRLF==1 so there we be a shift of 1 or 2 */
                                if (offsets != NULL) {
                                    vector_offset_add(offsets, old_start_pos, old_start_pos + 1, new_start_pos, new_start_pos + 1 + convLFtoCRLF);
                                }
                                old_start_pos++;
                                new_start_pos+=1+convLFtoCRLF;
                                current_start_pos++;
                                WriteOufBuf(&OutBuf, convLFtoCRLF, '\n', output, 0);
                            }
                        } else if ((c=='\n') && convLFtoCRLF) {
                            /* \n => \r\n means a shift of 1 */
                            if (offsets!=NULL) vector_offset_add(offsets,old_start_pos,old_start_pos+1,new_start_pos,new_start_pos+2);
                            old_start_pos++;
                            new_start_pos+=2;
                            current_start_pos++;
                            WriteOufBuf(&OutBuf, convLFtoCRLF, '\n', output, 0);
                        } else {
                            old_start_pos++;
                            new_start_pos++;
                            current_start_pos++;
                            WriteOufBuf(&OutBuf, convLFtoCRLF, c, output, 0);
                        }
                    }
                }
            }
        }
    }

    WriteOufBuf(&OutBuf, convLFtoCRLF, empty_string, output, 1);

    free(line_read);
    free_string_hash(replacements);
    free_Ustring(tmp);
    u_fclose(input);
    u_fclose(output);
    return 0;
}

} // namespace unitex
