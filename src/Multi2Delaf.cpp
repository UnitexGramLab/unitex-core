/**
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

#include "Multi2Delaf.h"

#include <stdlib.h>

#include "DELA.h"
#include "File.h"
#include "Pattern.h"
#include "Unicode.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif


#define INPUTSIZEBUFFER 4096

namespace unitex {

const char* COMMA_COPY  = ",copy";
const char* DOT_COPY    = ".copy";
const char* PLUS_COPY   = "+copy";
const char* COLUMN_COPY = ":copy";

/**
 * Set *str to the next no blank character.
 * Assumes that *str ended with '\0'.
 * Returns 1 if *str ends with blank char, otherwise returns 0.
 */
int advance_to_next_no_blank_char(unichar** str) {
  int i = 0;
  while ((*str)[i] != '\0' && ((*str)[i] == ' ' || (*str)[i] == '\t')) {
    i++;
  }
  if ((*str)[i] == '\0') {
    return 1;
  }
  *str = *str + i;
  return 0;
}

/**
 * Set *str to the next delaf tag.
 * Assumes that *str ended with '\0'.
 * Returns 1 if *str does not contain any more delaf tag, otherwise returns 0.
 */
int advance_to_next_delaf_tag(unichar** str) {
  int i = 0;
  while ((*str)[i] != '\0' && (*str)[i] != '{') {
    i++;
  }
  if ((*str)[i] == '\0') {
    return 1;
  }
  *str = *str + i;
  return 0;
}

/**
 * Return a new allocated unicode string describing the lemma.
 * Set *ptr to the next unread character.
 * Assumes that (*ptr)[0] == ','.
 */
unichar* tokenize_lemma(unichar** ptr, const char* config_filename) {
  unichar* line = *ptr;
  // try to read ,,copy
  if (line[0] == ',' && line[1] == ',') {
    if (!u_starts_with(line + 1, COMMA_COPY)) {
      fatal_error("Double ',' in file: %s, line: '%S'\n", config_filename,
                  line);
    }
    *ptr = line + 1 + strlen(COMMA_COPY);
    if (**ptr != '\0' && **ptr != '.' && **ptr != '+' && **ptr != ':') {
      fatal_error("Double ',' in file: %s, line: '%S'\n", config_filename,
                  line);
    }
    return u_strdup(COMMA_COPY);
  }
  int i = 1;
  while (line[i] != '\0' && line[i] != '.' && line[i] != '+' &&
         line[i] != ':') {
    i++;
  }
  *ptr = line + i;
  return u_strndup(line + 1, i - 1);
}

/**
 * Return a new allocated unicode string describing the part of speech.
 * Set *ptr to the next unread character.
 * Assumes that (*ptr)[0] == '.'.
 */
unichar* tokenize_part_of_speech(unichar** ptr, const char* config_filename) {
  unichar* line = *ptr;
  // try to read ..copy
  if (line[0] == '.' && line[1] == '.') {
    if (!u_starts_with(line + 1, DOT_COPY)) {
      fatal_error("Double '.' in file: %s, line: '%S'\n", config_filename,
                  line);
    }
    *ptr = line + 1 + strlen(DOT_COPY);
    if (**ptr != '\0' && **ptr != '+' && **ptr != ':') {
      fatal_error("Double '.' in file: %s, line: '%S'\n", config_filename,
                  line);
    }
    return u_strdup(DOT_COPY);
  }
  int i = 1;
  while (line[i] != '\0' && line[i] != '+' && line[i] != ':') {
    i++;
  }
  *ptr = line + i;
  return u_strndup(line + 1, i - 1);
}

/**
 * Return a new allocated unicode string describing a semantic code.
 * Set *ptr to the next unread character.
 * Assumes that (*ptr)[0] == '+'.
 */
unichar* tokenize_one_semantic_code(unichar** ptr,
                                    const char* config_filename) {
  unichar* line = *ptr;
  // try to read ++copy
  if (line[0] == '+' && line[1] == '+') {
    if (!u_starts_with(line + 1, PLUS_COPY)) {
      fatal_error("Double '+' in file: %s, line: '%S'\n", config_filename,
                  line);
    }
    *ptr = line + 1 + strlen(PLUS_COPY);
    if (**ptr != '\0' && **ptr != '+' && **ptr != ':') {
      fatal_error("Double '+' in file: %s, line: '%S'\n", config_filename,
                  line);
    }
    return u_strdup(PLUS_COPY);
  }
  int i = 1;
  while (line[i] != '\0' && line[i] != '+' && line[i] != ':') {
    i++;
  }
  *ptr = line + i;
  return u_strndup(line + 1, i - 1);
}

/**
 * Return a new allocated list of semantic codes.
 * Set *ptr to the next unread character.
 * Assumes that (*ptr)[0] == '+'.
 */
struct list_ustring* tokenize_semantic_codes(unichar** ptr,
                                             const char* config_filename) {
  unichar* line              = *ptr;
  unichar* next_code         = line;
  struct list_ustring* codes = NULL;
  while (*next_code != '\0' && *next_code != ':') {
    if (*next_code == '+') {
      unichar* new_code =
          tokenize_one_semantic_code(&next_code, config_filename);
      codes = sorted_insert(new_code, codes);
      free(new_code);
    }
  }
  *ptr = next_code;
  return codes;
}

/**
 * Return a new allocated unicode string describing an inflectional code.
 * Set *ptr to the next unread character.
 * Assumes that (*ptr)[0] == ':'.
 */
unichar* tokenize_one_inflectional_code(unichar** ptr,
                                        const char* config_filename) {
  unichar* line = *ptr;
  // try to read ::copy
  if (line[0] == ':' && line[1] == ':') {
    if (!u_starts_with(line + 1, COLUMN_COPY)) {
      fatal_error("Double ':' in file: %s, line: '%S'\n", config_filename,
                  line);
    }
    *ptr = line + 1 + strlen(COLUMN_COPY);
    if (**ptr != '\0' && **ptr != ':') {
      fatal_error("Double ':' in file: %s, line: '%S'\n", config_filename,
                  line);
    }
    return u_strdup(COLUMN_COPY);
  }
  int i = 1;
  while (line[i] != '\0' && line[i] != ':') {
    i++;
  }
  *ptr = line + i;
  return u_strndup(line + 1, i - 1);
}

/**
 * Return a new allocated list of inflectional codes.
 * Assumes that str[0] == ':'.
 */
struct list_ustring* tokenize_inflectional_codes(unichar* str,
                                                 const char* config_filename) {
  struct list_ustring* codes = NULL;
  unichar* next_code         = str;
  while (*next_code != '\0') {
    if (*next_code == ':') {
      unichar* new_code =
          tokenize_one_inflectional_code(&next_code, config_filename);
      codes = sorted_insert(new_code, codes);
      free(new_code);
    }
  }
  return codes;
}

struct ConfigCommand* new_config_command(
    unichar* lemma, unichar* part_of_speech,
    struct list_ustring* semantic_codes,
    struct list_ustring* inflectional_codes) {
  struct ConfigCommand* command = NULL;
  if (NULL ==
      (command = (struct ConfigCommand*)malloc(sizeof(struct ConfigCommand)))) {
    fatal_alloc_error("new_config_command");
  }
  command->lemma              = lemma;
  command->part_of_speech     = part_of_speech;
  command->semantic_codes     = semantic_codes;
  command->inflectional_codes = inflectional_codes;
  return command;
}

/**
 * Tokenize a config command.
 * Assumes that str ends by '\0'.
 * Assumes that empty space are removed from the begin,
 * using advance_to_next_no_blank_char.
 * Raise a fatal error in case of malformed command.
 */
struct ConfigCommand* tokenize_config_command(unichar* str,
                                              const char* config_filename) {
  unichar* lemma                          = NULL;
  unichar* part_of_speech                 = NULL;
  struct list_ustring* semantic_codes     = NULL;
  struct list_ustring* inflectional_codes = NULL;
  unichar* ptr                            = str;
  if (str[0] == ',') {
    lemma = tokenize_lemma(&ptr, config_filename);
  }
  if (ptr[0] == '.') {
    part_of_speech = tokenize_part_of_speech(&ptr, config_filename);
  }
  if (ptr[0] == '+') {
    semantic_codes = tokenize_semantic_codes(&ptr, config_filename);
  }
  if (ptr[0] == ':') {
    inflectional_codes = tokenize_inflectional_codes(ptr, config_filename);
  }
  return new_config_command(lemma, part_of_speech, semantic_codes,
                            inflectional_codes);
}

void free_config_command(struct ConfigCommand* command) {
  if (command->lemma) {
    free(command->lemma);
  }
  if (command->part_of_speech) {
    free(command->part_of_speech);
  }
  if (command->semantic_codes) {
    free_list_ustring(command->semantic_codes);
  }
  if (command->inflectional_codes) {
    free_list_ustring(command->inflectional_codes);
  }
  free(command);
}

/**
 * Return 1 if error occurs, otherwise 0.
 * Assumes that pattern is suround by < >.
 * Assumes that empty space are removed from the begin,
 * using advance_to_next_no_blank_char.
 */
int recognize_pattern_token(unichar** ptr, unichar* res) {
  unichar* line  = *ptr;
  int index_line = 0;
  int index_res  = 0;
  if (line[index_line] != '<') {
    return 1;
  }
  index_line++;
  while (line[index_line] != '\0') {
    if (line[index_line] != '>') {
      res[index_res] = line[index_line];
      index_res++;
      index_line++;
    } else {
      res[index_res] = '\0';
      *ptr           = line + index_line + 1;
      return 0;
    }
  }
  return 1;
}


/**
 * Return 1 if error occurs, otherwise 0.
 * Assumes that empty space are removed from the begin,
 * using advance_to_next_no_blank_char.
 */
int tokenize_nb_required_tag(unichar** ptr, int* res) {
  unichar* line                   = *ptr;
  int index                       = 0;
  int index_buffer                = 0;
  unichar buffer[INPUTSIZEBUFFER] = {0};
  if (line[0] != '{') {
    return 1;
  }
  index++;
  while (line[index] != '\0') {
    if (line[index] != '}') {
      if (line[index] < '0' || line[index] > '9') {
        return 1;
      }
      buffer[index_buffer] = line[index];
      index++;
      index_buffer++;
    } else {
      if (index == 1) {
        // {} is a syntax error
        return 1;
      }
      res[index_buffer] = '\0';
      *res              = u_parse_int(buffer, NULL);
      *ptr              = line + index + 1;
      return 0;
    }
  }
  return 1;
}

struct ConfigLine* new_config_line(struct pattern* pattern, int nb_required_tag,
                                   struct ConfigCommand* config_command) {
  struct ConfigLine* line = NULL;
  if (NULL == (line = (struct ConfigLine*)malloc(sizeof(struct ConfigLine)))) {
    fatal_alloc_error("new_config_line");
  }
  line->pattern         = pattern;
  line->nb_required_tag = nb_required_tag;
  line->config_command  = config_command;
  return line;
}

/**
 * Tokenize a config line.
 * Returns a struct ConfigLine* if there is a well-formed line.
 * otherwise returns NULL.
 * Raises a fatal error in case of malformed line.
 */
struct ConfigLine* tokenize_config_line(unichar* line,
                                        const char* config_filename) {
  unichar* nextNoEmptyUnichar           = line;
  unichar patternToken[INPUTSIZEBUFFER] = {0};
  if (advance_to_next_no_blank_char(&nextNoEmptyUnichar)) {
    return NULL;  // skip empty line
  }
  if (nextNoEmptyUnichar[0] == '#') {
    return NULL;  // skip comment line
  }
  if (recognize_pattern_token(&nextNoEmptyUnichar, patternToken)) {
    fatal_error(
        "Lexical mask must be enclosed in < >, like <be.V:K> in file: %s, "
        "line: '%S'\n",
        config_filename, line);
  }
  // build pattern
  struct pattern* pattern = build_pattern(patternToken, NULL, 0, NULL);
  if (advance_to_next_no_blank_char(&nextNoEmptyUnichar)) {
    // if there is no command, we skip the current line like comment line
    return NULL;
  }
  // build nb_required_tag
  int nb_required_tag = Multi2Delaf::NOT_SPECIFIED;
  if (nextNoEmptyUnichar[0] == '{') {
    if (tokenize_nb_required_tag(&nextNoEmptyUnichar, &nb_required_tag)) {
      fatal_error("Braces must contain number in file: %s, line: '%S'\n",
                  config_filename, line);
    }
  }
  if (advance_to_next_no_blank_char(&nextNoEmptyUnichar)) {
    // if there is no command, we skip the current line like comment line
    return NULL;
  }
  // build config command
  struct ConfigCommand* config_command =
      tokenize_config_command(nextNoEmptyUnichar, config_filename);
  if (u_strcmp(config_command->lemma, COMMA_COPY) == 0 &&
      nb_required_tag != Multi2Delaf::NOT_SPECIFIED && nb_required_tag != 1) {
    fatal_error(
        "Command ,,copy is incompatible with an integer enclosed in curly "
        "braces, except for {1} in file: %s, line: '%S'\n",
        config_filename, line);
  }
  if (u_strcmp(config_command->part_of_speech, DOT_COPY) == 0 &&
      nb_required_tag != Multi2Delaf::NOT_SPECIFIED && nb_required_tag != 1) {
    fatal_error(
        "Command ..copy is incompatible with an integer enclosed in curly "
        "braces, except for {1} in file:%s, line: '%S'\n",
        config_filename, line);
  }
  return new_config_line(pattern, nb_required_tag, config_command);
}

void free_config_line(void* void_line) {
  if(void_line == NULL) {
    return;
  }
  struct ConfigLine* line = (struct ConfigLine*)void_line;
  free_pattern(line->pattern);
  free_config_command(line->config_command);
  free(line);
}

/**
 * Return the first delaf tag at the address *ptr.
 * Return NULL to indicate that there is no more tag.
 * Set *ptr to the next unread character.
 * Raises a fatal error if the delaf tag is not enclosed in curly braces.
 */
struct dela_entry* tokenize_delaf_tag(unichar** ptr) {
  unichar* line               = *ptr;
  unichar* next_delaf_tag = line;
  if (advance_to_next_delaf_tag(&next_delaf_tag)) {
    return NULL;  // end of the line, no more dela_entry
  }
  int i = 1; // 1 to skip opening brace '{'
  while (next_delaf_tag[i] != '\0' && next_delaf_tag[i] != '}') {
    i++;
  }
  if (next_delaf_tag[i] == '\0') {
    fatal_error("Unclosed curly brace: Delaf tag must be enclosed in curly braces, line: '%S'\n",
                line);
  }
  unichar* token_dela_entry = u_strndup(next_delaf_tag + 1, i - 1);
  struct dela_entry* tag    = tokenize_DELAF_line(token_dela_entry);
  free(token_dela_entry);
  *ptr = next_delaf_tag + i + 1;  // + 1 to skip closing brace '}'
  return tag;
}

/**
 * Read a line from the configuration file, and save the result in buffer.
 * Returns EOF if end of file occurs, otherwise the number of unichar readed.
 */
int read_line_config_file(U_FILE* config_file, unichar* buffer,
                          int size_buffer) {
  int c = 0;
  int i = 0;
  while (i < size_buffer - 1 && EOF != (c = u_fgetc(config_file))) {
    if (c == '\r') {
      if ('\n' == (c = u_fgetc(config_file))) {
        buffer[i] = '\0';
        return i;
      }
      unichar rest_of_line[INPUTSIZEBUFFER] = {0};
      int j                                 = 0;
      while (j < INPUTSIZEBUFFER - 1 && EOF != (c = u_fgetc(config_file))) {
        if (c != '\n') {
          rest_of_line[j] = c;
          j++;
        } else {
          rest_of_line[j] = '\0';
          u_fprintf(U_STDERR, "\\r not followed by a \\n, '%S' is ignored\n",
                    rest_of_line);
          return i + j;
        }
      }
    }
    if (c == '\n') {
      buffer[i] = '\0';
      return i;
    }
    buffer[i] = c;
    i++;
  }
  buffer[i] = '\0';
  return EOF;
}

/**
 * Return the number of delaf tags that match the pattern.
 */
int nb_delaf_tag_that_match_pattern(struct list_pointer* delaf_tags,
                                    const struct pattern* pattern) {
  int res = 0;
  while (delaf_tags != NULL) {
    struct dela_entry* tag = (struct dela_entry*)delaf_tags->pointer;
    if (is_entry_compatible_with_pattern(tag, pattern)) {
      res++;
    }
    delaf_tags = delaf_tags->next;
  }
  return res;
}

/**
 * Return a new allocated unicode string.
 */
unichar* escape_inflected_input(const unichar* input) {
  unichar buffer[INPUTSIZEBUFFER] = {0};
  int i_input                     = 0;
  int i_buffer                    = 0;
  while (i_input < INPUTSIZEBUFFER - 1 && i_buffer < INPUTSIZEBUFFER - 1) {
    if (input[i_input] == ' ' && input[i_input + 1] == '\0') {
      buffer[i_buffer] = '\0';
      return u_strdup(buffer);
    }
    if (input[i_input] == '=' || input[i_input] == '.' ||
        input[i_input] == ',') {
      buffer[i_buffer] = '\\';
      i_buffer++;
    }
    buffer[i_buffer] = input[i_input];
    i_buffer++;
    i_input++;
  }
  buffer[i_buffer < INPUTSIZEBUFFER - 1 ? i_buffer : INPUTSIZEBUFFER - 1] =
      '\0';
  return u_strdup(buffer);
}

/**
 * Returns a new string containing the first one and the second one without duplicates.
 */
unichar* complete_first_with_second(const unichar* first,
                                    const unichar* second) {
  unichar to_add[INPUTSIZEBUFFER] = {0};
  unichar res[INPUTSIZEBUFFER]    = {0};
  int j                           = 0;
  for (size_t i = 0; i < u_strlen(second); i++) {
    if (u_strchr(first, second[i]) == NULL) {
      to_add[j] = second[i];
      j++;
    }
  }
  to_add[j] = '\0';
  u_sprintf(res, "%S%S", first, to_add);
  return u_strdup(res);
}

/**
 * Returns a new list where ::copy has been replaced by the inflectional codes of the tag.
 */
struct list_ustring* clone_and_replace_copy_command(
    const struct list_ustring* inflectional_command,
    const struct dela_entry* tag) {
  struct list_ustring* res = NULL;

  while (inflectional_command != NULL) {
    if (u_strcmp(inflectional_command->string, COLUMN_COPY) != 0) {
      res = sorted_insert(inflectional_command->string, res);
    } else {
      for (int i = 0; i < tag->n_inflectional_codes; i++) {
        res = sorted_insert(tag->inflectional_codes[i], res);
      }
    }
    inflectional_command = inflectional_command->next;
  }
  return res;
}

/**
 * Create a new allocated list containing the Cartesian product of the two lists in parameter
 * and substituate ::copy command by codes in the delaf tag.
 * Suppose that l2 is not the empty list.
 */
struct list_ustring* product(struct list_ustring* l1, struct list_ustring* l2) {
  unichar* tmp_code           = NULL;
  struct list_ustring* res    = NULL;
  struct list_ustring* ptr_l1 = l1;
  struct list_ustring* ptr_l2 = l2;
  if (l1 == NULL) {
    return clone(l2);
  }
  while (ptr_l1 != NULL) {
    ptr_l2 = l2;
    while (ptr_l2 != NULL) {
      tmp_code = complete_first_with_second(ptr_l1->string, ptr_l2->string);
      res      = sorted_insert(tmp_code, res);
      free(tmp_code);
      ptr_l2 = ptr_l2->next;
    }
    ptr_l1 = ptr_l1->next;
  }
  return res;
}

/**
 * Return a new allocated unicode string.
 */
unichar* build_output_codes(const struct list_ustring* codes, char prefix) {
  unichar buffer[INPUTSIZEBUFFER] = {0};
  if (codes == NULL) {
    return u_strdup("");
  }
  while (codes != NULL) {
    if (u_strlen(buffer) + u_strlen(codes->string) + 2 >= INPUTSIZEBUFFER - 1) {
      fatal_error(
          "internal err(build_output_codes): buffer is not "
          "big enough\n");
    }
    u_sprintf(buffer, "%S%c%S", buffer, prefix, codes->string);
    codes = codes->next;
  }
  return u_strdup(buffer);
}

/*=================================================================
 * Multi2Delaf class method
 *================================================================= */

Multi2Delaf::Multi2Delaf(const char* config_filename)
    : _config_filename{config_filename} {
}

Multi2Delaf::~Multi2Delaf() {
  free_list_pointer(_config_lines, free_config_line);
}

/**
 * Read the configuration file that specifies how to transcode the 
 * multidelaf string into a delaf tag.
 * Raises a fatal error in case of malformed file.
 */
void Multi2Delaf::parse_config_file() {
  U_FILE* config_file = u_fopen(&_vec, _config_filename, U_READ);
  if (config_file == NULL) {
    fatal_error("Cannot open configuration file %s\n", _config_filename);
  }
  load_config_file(config_file);
  u_fclose(config_file);
}

/**
 * Translate a multidelaf string to a delaf tag using the config file.
 * Make the translation in place in the buffer.
 * Supposes that the config file is already readed.
 */
void Multi2Delaf::translate_multidelaf_to_delaf(const unichar* inflected_input,
                                                unichar* buffer) const {
  unichar* ptr                    = buffer;
  struct list_pointer* delaf_tags = NULL;
  struct dela_entry* new_tag      = NULL;
  while (NULL != (new_tag = tokenize_delaf_tag(&ptr))) {
    delaf_tags = new_list_pointer(new_tag, delaf_tags);
  }
  unichar* inflected          = escape_inflected_input(inflected_input);
  unichar* lemma              = retrieve_lemma(delaf_tags, buffer);
  unichar* part_of_speech     = retrieve_part_of_speech(delaf_tags, buffer);
  unichar* semantic_codes     = retrieve_semantic_codes(delaf_tags);
  unichar* inflectional_codes = retrieve_inflectional_codes(delaf_tags);
  if (u_strlen(inflected) + u_strlen(lemma) + u_strlen(part_of_speech) +
          u_strlen(semantic_codes) + u_strlen(inflectional_codes) + 2 >=
      INPUTSIZEBUFFER) {
    fatal_error(
        "internal err(Multi2Delaf::translate_multidelaf_to_delaf): buffer is "
        "not big enough\n");
  }
  u_sprintf(buffer, "%S,%S.%S%S%S", inflected, lemma, part_of_speech,
            semantic_codes, inflectional_codes);
  free(lemma);
  free(inflected);
  free(part_of_speech);
  free(semantic_codes);
  free(inflectional_codes);
  struct list_pointer* delaf_tags_ptr = delaf_tags;
  while (delaf_tags_ptr != NULL) {
    free_dela_entry((struct dela_entry*)delaf_tags_ptr->pointer);
    delaf_tags_ptr = delaf_tags_ptr->next;
  }
  free_list_pointer(delaf_tags);
}



/**
 * Load the configuration file.
 * Raises a fatal error in case of malformed file.
 */
void Multi2Delaf::load_config_file(U_FILE* config_file) {
  unichar line[INPUTSIZEBUFFER] = {0};
  int eof                       = 0;
  struct list_pointer* lines_index = NULL;
  while (EOF !=
         (eof = read_line_config_file(config_file, line, INPUTSIZEBUFFER))) {
    struct ConfigLine* config_line =
        tokenize_config_line(line, filename_without_path(_config_filename));
    if (config_line != NULL) {
      if(_config_lines == NULL) {
        _config_lines = new_list_pointer(config_line, NULL);
      } else {
        lines_index = _config_lines;
        while(lines_index->next != NULL) {
          lines_index = lines_index->next;
        }
        lines_index->next = new_list_pointer(config_line, NULL);
      }
    }
  }
  // the last line is potentially a config line
  struct ConfigLine* config_line =
      tokenize_config_line(line, filename_without_path(_config_filename));
  if (config_line != NULL) {
    if (_config_lines == NULL) {
      _config_lines = new_list_pointer(config_line, NULL);
    }else {
      lines_index = _config_lines;
      while(lines_index->next != NULL) {
        lines_index = lines_index->next;
      }
      lines_index->next = new_list_pointer(config_line, NULL);
    }
  }
}

/**
 * Retrieve the lemma according to the specification:
 * We look every line of _config_lines and keep the lemma of the first-one matching containing a lemma.
 * If no lemma is corresponding, raises a fatal_error().
 * Return a new allocated unicode string describing the lemma.
 */
unichar* Multi2Delaf::retrieve_lemma(struct list_pointer* delaf_tags,
                                     const unichar* multidelaf_string) const {
  struct list_pointer* current_line_ptr = _config_lines;
  struct list_pointer* delaf_tags_ptr   = delaf_tags;
  struct dela_entry* tag                = NULL;

  while (current_line_ptr != NULL) {
    struct ConfigLine* current_line =
        (struct ConfigLine*)current_line_ptr->pointer;
    if (current_line->config_command->lemma == NULL) {
      current_line_ptr = current_line_ptr->next;
      continue;
    }
    delaf_tags_ptr = delaf_tags;
    while (delaf_tags_ptr != NULL) {
      tag = (struct dela_entry*)delaf_tags_ptr->pointer;
      if (is_entry_compatible_with_pattern(tag, current_line->pattern)) {
        if (u_strcmp(current_line->config_command->lemma, COMMA_COPY) == 0) {
          if (nb_delaf_tag_that_match_pattern(delaf_tags,
                                              current_line->pattern) != 1) {
            fatal_error(
                "Command ,,copy can be interpreted for several delaf line: "
                "%S\n",
                multidelaf_string);
          }
          return u_strdup(tag->lemma);
        }
        if (current_line->nb_required_tag == 1 ||
            current_line->nb_required_tag == Multi2Delaf::NOT_SPECIFIED) {
          return u_strdup(current_line->config_command->lemma);
        }
        if (current_line->nb_required_tag ==
            nb_delaf_tag_that_match_pattern(delaf_tags,
                                            current_line->pattern)) {
          return u_strdup(current_line->config_command->lemma);
        }
      }
      delaf_tags_ptr = delaf_tags_ptr->next;
    }
    if (current_line->nb_required_tag == 0) {
      return u_strdup(current_line->config_command->lemma);
    }
    current_line_ptr = current_line_ptr->next;
  }
  fatal_error("No lemma is provided for this multidelaf string: %S\n",
              multidelaf_string);
  return NULL;
}

/**
 * Retrieves the part_of_speech according to the specification:
 * We look every line of config lines and keep the part_of_speech of the first-one matching containing a part_of_speech.
 * If no part_of_speech is corresponding, raises a fatal_error().
 * Return a new allocated unicode string describing the part of speech.
 */
unichar* Multi2Delaf::retrieve_part_of_speech(
    struct list_pointer* delaf_tags, const unichar* multidelaf_string) const {
  struct list_pointer* current_line_ptr = _config_lines;
  struct list_pointer* delaf_tag_ptr    = delaf_tags;
  struct dela_entry* tag                = NULL;
  while (current_line_ptr != NULL) {
    struct ConfigLine* current_line =
        (struct ConfigLine*)current_line_ptr->pointer;
    if (current_line->config_command->part_of_speech == NULL) {
      current_line_ptr = current_line_ptr->next;
      continue;
    }
    delaf_tag_ptr = delaf_tags;
    while (delaf_tag_ptr != NULL) {
      tag = (struct dela_entry*)delaf_tag_ptr->pointer;
      if (is_entry_compatible_with_pattern(tag, current_line->pattern)) {
        if (u_strcmp(current_line->config_command->part_of_speech, DOT_COPY) ==
            0) {
          if (nb_delaf_tag_that_match_pattern(delaf_tags,
                                              current_line->pattern) != 1) {
            fatal_error(
                "Command ..copy can be interpreted for several delaf tag: %S\n",
                multidelaf_string);
          }
          return u_strdup(tag->semantic_codes[0]);
        }
        if (current_line->nb_required_tag == 1 ||
            current_line->nb_required_tag == Multi2Delaf::NOT_SPECIFIED) {
          return u_strdup(current_line->config_command->part_of_speech);
        }
        if (current_line->nb_required_tag ==
            nb_delaf_tag_that_match_pattern(delaf_tags,
                                            current_line->pattern)) {
          return u_strdup(current_line->config_command->part_of_speech);
        }
      }
      delaf_tag_ptr = delaf_tag_ptr->next;
    }
    if (current_line->nb_required_tag == 0) {
      return u_strdup(current_line->config_command->part_of_speech);
    }
    current_line_ptr = current_line_ptr->next;
  }
  fatal_error(
      "No grammatical cathegory is provided for this multidelaf string: %S\n",
      multidelaf_string);
  return NULL;
}

/**
 * Retrieves semantic codes according to the specification:
 * The semantic codes of the multidelaf string are obtained by taking the union
 * of the semantic codes of each tag assigned by the configuration file.
 * If no semantic_codes is corresponding, return an empty string.
 * Return a new allocated unicode string describing semantic codes.
 */
unichar* Multi2Delaf::retrieve_semantic_codes(
    struct list_pointer* delaf_tags) const {
  struct list_ustring* codes            = NULL;
  struct list_ustring* ptr_command      = NULL;
  struct list_pointer* config_lines_ptr = _config_lines;
  struct list_pointer* delaf_tag_ptr    = delaf_tags;
  struct dela_entry* tag                = NULL;

  while (delaf_tag_ptr != NULL) {
    tag              = (struct dela_entry*)delaf_tag_ptr->pointer;
    config_lines_ptr = _config_lines;
    while (config_lines_ptr != NULL) {
      struct ConfigLine* line = (struct ConfigLine*)config_lines_ptr->pointer;
      if (line->config_command->semantic_codes == NULL) {
        config_lines_ptr = config_lines_ptr->next;
        continue;
      }
      if (is_entry_compatible_with_pattern(tag, line->pattern)) {
        ptr_command = line->config_command->semantic_codes;
        while (ptr_command != NULL) {
          if (line->nb_required_tag == Multi2Delaf::NOT_SPECIFIED ||
              line->nb_required_tag ==
                  nb_delaf_tag_that_match_pattern(delaf_tags, line->pattern)) {
            if (u_strcmp(ptr_command->string, PLUS_COPY) == 0) {
              for (int i = 1; i < tag->n_semantic_codes;
                   i++) {  // begin at 1 to skip the grammatical catergory
                codes = sorted_insert(tag->semantic_codes[i], codes);
              }
            } else {
              if (line->nb_required_tag != 0) {
                codes = sorted_insert(ptr_command->string, codes);
              }
            }
          }
          ptr_command = ptr_command->next;
        }
      } else {
        if (0 == line->nb_required_tag &&
            0 == nb_delaf_tag_that_match_pattern(delaf_tags, line->pattern)) {
          struct list_ustring* ptr_command =
              line->config_command->semantic_codes;
          while (ptr_command != NULL) {
            codes       = sorted_insert(ptr_command->string, codes);
            ptr_command = ptr_command->next;
          }
        }
      }
      config_lines_ptr = config_lines_ptr->next;
    }
    delaf_tag_ptr = delaf_tag_ptr->next;
  }
  unichar* res = build_output_codes(codes, '+');
  if (codes) {
    free_list_ustring(codes);
  }
  return res;
}



/**
 * Retrieves inflectional codes according to the specification:
 * The inflectional codes of the multidelaf string are obtained by taking the union
 * of the inflectional codes of each tag assigned by the configuration file.
 * If no inflectional_codes is corresponding, return an empty string.
 * Return a new allocated unicode string describing inflectional codes.
 */
unichar* Multi2Delaf::retrieve_inflectional_codes(
    struct list_pointer* delaf_tags) const {
  struct list_ustring* codes         = NULL;
  struct list_ustring* tmp_codes     = NULL;
  struct list_ustring* ptr_command   = NULL;
  struct list_pointer* line_ptr      = _config_lines;
  struct list_pointer* delaf_tag_ptr = delaf_tags;
  struct dela_entry* tag             = NULL;

  while (delaf_tag_ptr != NULL) {
    tag      = (struct dela_entry*)delaf_tag_ptr->pointer;
    line_ptr = _config_lines;
    while (line_ptr != NULL) {
      struct ConfigLine* line = (struct ConfigLine*)line_ptr->pointer;
      tmp_codes               = codes;
      if (line->config_command->inflectional_codes == NULL) {
        line_ptr = line_ptr->next;
        continue;
      }
      if (is_entry_compatible_with_pattern(tag, line->pattern)) {
        ptr_command = line->config_command->inflectional_codes;

        if (line->nb_required_tag == Multi2Delaf::NOT_SPECIFIED ||
            line->nb_required_tag ==
                nb_delaf_tag_that_match_pattern(delaf_tags, line->pattern)) {
          if (is_in_list(COLUMN_COPY, ptr_command)) {
            struct list_ustring* tmp_lst =
                clone_and_replace_copy_command(ptr_command, tag);
            if (tmp_lst == NULL) {
              codes = product(NULL, tmp_codes);
            } else {
              codes = product(tmp_codes, tmp_lst);
              free_list_ustring(tmp_lst);
            }
            if (tmp_codes) {
              free_list_ustring(tmp_codes);
            }
          } else {
            codes = product(tmp_codes, ptr_command);
            if (tmp_codes) {
              free_list_ustring(tmp_codes);
            }
          }
        }

      } else {
        if (0 == line->nb_required_tag &&
            0 == nb_delaf_tag_that_match_pattern(delaf_tags, line->pattern)) {
          struct list_ustring* ptr_command =
              line->config_command->inflectional_codes;
          codes = product(tmp_codes, ptr_command);
          if (tmp_codes) {
            free_list_ustring(tmp_codes);
          }
        }
      }
      line_ptr = line_ptr->next;
    }
    delaf_tag_ptr = delaf_tag_ptr->next;
  }
  unichar* res = build_output_codes(codes, ':');
  if (codes) {
    free_list_ustring(codes);
  }
  return res;
}

}  // namespace unitex
