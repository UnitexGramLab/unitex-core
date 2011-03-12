/*
 * Unitex
 *
 * Copyright (C) 2001-2011 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


#define MAX_LINE_BUFFER_SIZE (32768)
#define MINIMAL_CHAR_IN_BUFFER_BEFORE_CONTINUE_LINE (256)

#define SIZE_OUTPUT_BUFFER 0x100

struct OUTBUF {
	unichar outbuf[SIZE_OUTPUT_BUFFER + 1];
	int pos;
};

void WriteOufBuf(struct OUTBUF* pOutBuf, const unichar* str, U_FILE *f,
		int flush) {
	for (;;) {

		while (((*str) != 0) && (pOutBuf->pos < SIZE_OUTPUT_BUFFER)) {
			pOutBuf->outbuf[pOutBuf->pos] = *str;
			pOutBuf->pos++;
			str++;
		}

		if ((pOutBuf->pos == SIZE_OUTPUT_BUFFER) || (flush != 0)) {
			pOutBuf->outbuf[pOutBuf->pos] = 0; // add null terminating marker
			u_fprintf(f, "%S", pOutBuf->outbuf);
			pOutBuf->pos = 0;
		}

		if ((*str) == 0)
			break;
	}
}

void WriteOufBuf(struct OUTBUF* pOutBuf, unichar c, U_FILE *f, int flush) {
	unichar u_array[2];
	u_array[0] = c;
	u_array[1] = 0;
	WriteOufBuf(pOutBuf, u_array, f, flush);
}


/**
 * This function computes the length of the longest prefix and suffix
 * that are common to the key and its associated value. Returns 0
 * if key and value are equals, 0 otherwise.
 */
int get_real_replacement(const unichar* key,int key_size,unichar* value,int *pfx,int *sfx) {
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
int normalize(const char *fin, const char *fout, Encoding encoding_output,
		int bom_output, int mask_encoding_compatibility_input,
		int carriage_return_policy, const char *rules,
		vector_offset* offsets,
		int separator_normalization) {
	U_FILE* input;
	input = u_fopen_existing_versatile_encoding(
			mask_encoding_compatibility_input, fin, U_READ);
	if (input == NULL) {
		error("Cannot open file %s\n", fin);
		return 1;
	}

	U_FILE* output;
	output = u_fopen_creating_versatile_encoding(encoding_output, bom_output,
			fout, U_WRITE);
	if (output == NULL) {
		error("Cannot create file %s\n", fout);
		u_fclose(input);
		return 1;
	}
	struct string_hash* replacements = NULL;
	if (rules != NULL && rules[0] != '\0') {
		replacements = load_key_value_list(rules,
				mask_encoding_compatibility_input, '\t');
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

	struct OUTBUF OutBuf;
	OutBuf.pos = 0;
	unichar tmp[MAX_TAG_LENGTH];
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

	static const unichar stop_chars[] = { '{', '}', 0 };
	static const unichar forbidden_chars[] = { '\n', 0 };
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
#if 0
		result_read=u_fgets_treat_cr_as_lf(line_read, line_buffer_size, input);
		if (result_read > 0)
			if (line_read[result_read - 1] == 0x0d)
				line_read[result_read - 1] = '\n';
		if (result_read == EOF)
			break;
#endif
		if (result_read==0) break;
#if 0
		if (lastline_was_terminated != 0)
			while (current_start_pos < result_read) {
				if (buff[current_start_pos] != ' ' && buff[current_start_pos]
						!= '\t' && buff[current_start_pos] != 0x0d
						&& buff[current_start_pos] != '\n')
					break;
				current_start_pos++;
			}

		lastline_was_terminated = 0;
		if (result_read > 0)
			if ((buff[result_read - 1] == '\n') || (buff[result_read - 1]
					== 0x0d))
				lastline_was_terminated = 1;
#endif
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
#if 0
				int result_read_continue = u_fgets_treat_cr_as_lf(line_read
						+ nb_to_keep, line_buffer_size - nb_to_keep, input);
				if (result_read_continue > 0)
					if (line_read[(result_read_continue + nb_to_keep) - 1]
							== 0x0d)
						line_read[(result_read_continue + nb_to_keep) - 1]
								= '\n';
				lastline_was_terminated = 0;
				if (result_read_continue == EOF)
					eof_found = lastline_was_terminated = 1;
#endif
				if (result_read_continue == 0) {
					eof_found = lastline_was_terminated = 1;
				} else {
					/* This is required to avoid bound checking */
					line_read[nb_to_keep+result_read_continue]='\0';
				}
#if 0
				if (result_read_continue > 0)
					if ((buff[(result_read_continue + nb_to_keep) - 1] == '\n')
							|| (buff[(result_read_continue + nb_to_keep) - 1]
									== 0x0d))
						lastline_was_terminated = 1;
#endif
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

				tmp[0] = '{';
				int code = parse_string(buff, &current_start_pos, &(tmp[1]),
						stop_chars, forbidden_chars, NULL);
				if (code == P_FORBIDDEN_CHAR || code == P_BACKSLASH_AT_END
						|| buff[current_start_pos] != '}') {
					/* If we have found a new line or a {, or if there is
					 * a backslash at the end of the buffer, or if we have reached the end
					 * of the buffer, we assume that the initial
					 * { was not a tag beginning, so we print the substitute of { */
					unichar* foo=replacements->value[get_value_index(open_bracket, replacements)];
					if (offsets!=NULL) vector_offset_add(offsets,old_start_pos,old_start_pos+1,new_start_pos,new_start_pos+u_strlen(foo));
					WriteOufBuf(&OutBuf, foo, output, 0);
					/* And we rewind the current position after the { */
					current_start_pos = old_position + 1;
					old_start_pos++;
					new_start_pos=new_start_pos+u_strlen(foo);
				} else {
					/* If we have read a sequence like {....}, we assume that there won't be
					 * a buffer overflow if we add the } */
					u_strcat(tmp, close_bracket);
					if (!u_strcmp(tmp, "{S}") || !u_strcmp(tmp, "{STOP}")
							|| check_tag_token(tmp)) {
						/* If this is a special tag or a valid tag token, we just print
						 * it to the output */
						WriteOufBuf(&OutBuf, tmp, output, 0);
						current_start_pos++;
						int l=u_strlen(tmp);
						old_start_pos=old_start_pos+l;
						new_start_pos=new_start_pos+l;
					} else {
						/* If we have a non valid tag token, we print the equivalent of {
						 * and we rewind the current position after the { */
						unichar* foo=replacements->value[get_value_index(open_bracket, replacements)];
						if (offsets!=NULL) vector_offset_add(offsets,old_start_pos,old_start_pos+1,new_start_pos,new_start_pos+u_strlen(foo));
						WriteOufBuf(&OutBuf, foo, output, 0);
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
					if (offsets!=NULL && ret) vector_offset_add(offsets,old_start_pos+common_prefix,old_start_pos+key_length-common_suffix,new_start_pos+common_suffix,new_start_pos+u_strlen(foo)-common_suffix);
					WriteOufBuf(&OutBuf, replacements->value[index], output, 0);
					current_start_pos = current_start_pos + key_length;
					old_start_pos = old_start_pos + key_length;
					new_start_pos=new_start_pos+u_strlen(foo);
				} else {
					int old_position=current_start_pos;
					if (separator_normalization && (buff[current_start_pos] == ' '
							|| buff[current_start_pos] == '\t'
							|| buff[current_start_pos] == '\n'
							|| buff[current_start_pos] == 0x0d)) {
						/* If we have a separator, we try to read the longest separator sequence
						 * that we can read. By the way, we note if it contains a new line */
						int new_line = 0;
						while (buff[current_start_pos] == ' '
								|| buff[current_start_pos] == '\t'
								|| buff[current_start_pos] == '\n'
								|| buff[current_start_pos] == 0x0d) {
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
							if (delta!=2) {
								/* We will deal with offsets only if they change, and
								 * as any sequence of 2 characters normalized into a
								 * \r\n will not modify offsets, we should do nothing
								 * in that case */
								if (offsets!=NULL) vector_offset_add(offsets,old_start_pos,old_start_pos+delta,new_start_pos,new_start_pos+2);
							}
							old_start_pos=old_start_pos+delta;
							new_start_pos+=2;
							WriteOufBuf(&OutBuf, '\n', output, 0);
						} else {
							if (delta!=1) {
								/* We will deal with offsets only if they change, and
								 * as any sequence of 1 character normalized into a
								 * single space will not modify offsets, we should do nothing
								 * in that case */
								if (offsets!=NULL) vector_offset_add(offsets,old_start_pos,old_start_pos+delta,new_start_pos,new_start_pos+1);
							}
							old_start_pos=old_start_pos+delta;
							new_start_pos++;
							WriteOufBuf(&OutBuf, ' ', output, 0);
						}
					} else {
						/* If, finally, we have a normal character to normalize, we just print it */
						WriteOufBuf(&OutBuf, buff[current_start_pos++], output,	0);
						old_start_pos++;
						new_start_pos++;
					}
				}
			}
		}
	}

	WriteOufBuf(&OutBuf, empty_string, output, 1);

	free(line_read);
	free_string_hash(replacements);

	u_fclose(input);
	u_fclose(output);
	return 0;
}

