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

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "DELA.h"
#include "File.h"
#include "Pattern.h"
#include "Unicode.h"


#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif


#define INPUTSIZEBUFFER 4096

namespace unitex {


/*=================================================================
 * ConfigCommand class method
 *================================================================= */

/**
 * Tokenize a config command.
 * Assumes that str ends by '\0'.
 * Assumes that empty space are removed from the begin,
 * using ConfigLine::advances_to_next_no_blank_char.
 * Raise a fatal error in case of malformed command.
 */
std::unique_ptr<ConfigCommand> ConfigCommand::tokenize_config_command(
    unichar* str, const char* config_filename) {
  unichar* lemma                          = nullptr;
  unichar* part_of_speech                 = nullptr;
  struct list_ustring* semantic_codes     = nullptr;
  struct list_ustring* inflectional_codes = nullptr;
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
  return std::make_unique<ConfigCommand>(lemma, part_of_speech, semantic_codes,
                                         inflectional_codes);
}

ConfigCommand::ConfigCommand(unichar* lemma, unichar* part_of_speech,
                             struct list_ustring* semantic_codes,
                             struct list_ustring* inflectional_codes)
    : _lemma{lemma},
      _part_of_speech{part_of_speech},
      _semantic_codes{semantic_codes},
      _inflectional_codes{inflectional_codes} {
}

ConfigCommand::~ConfigCommand() {
  if (_lemma) {
    free(_lemma);
  }
  if (_part_of_speech) {
    free(_part_of_speech);
  }
  if (_semantic_codes) {
    free_list_ustring(_semantic_codes);
  }
  if (_inflectional_codes) {
    free_list_ustring(_inflectional_codes);
  }
}

unichar* ConfigCommand::get_lemma() const {
  return _lemma;
}

unichar* ConfigCommand::get_part_of_speech() const {
  return _part_of_speech;
}

struct list_ustring* ConfigCommand::get_semantic_codes() const {
  return _semantic_codes;
}

struct list_ustring* ConfigCommand::get_inflectional_codes() const {
  return _inflectional_codes;
}

/**
 * Return a new allocated unicode string describing the lemma.
 * Set *ptr to the next unread character.
 * Assumes that (*ptr)[0] == ','.
 */
unichar* ConfigCommand::tokenize_lemma(unichar** ptr,
                                       const char* config_filename) {
  unichar* line = *ptr;
  // try to read ,,copy
  if (line[0] == ',' && line[1] == ',') {
    if (!u_starts_with(line + 1, COMMA_COPY)) {
      fatal_error("Double ',' in file: %s, line: `%S`\n", config_filename,
                  line);
    }
    *ptr = line + 1 + strlen(COMMA_COPY);
    if (**ptr != '\0' && **ptr != '.' && **ptr != '+' && **ptr != ':') {
      fatal_error("Double ',' in file: %s, line: `%S`\n", config_filename,
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
unichar* ConfigCommand::tokenize_part_of_speech(unichar** ptr,
                                                const char* config_filename) {
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
unichar* ConfigCommand::tokenize_one_semantic_code(
    unichar** ptr, const char* config_filename) {
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
struct list_ustring* ConfigCommand::tokenize_semantic_codes(
    unichar** ptr, const char* config_filename) {
  unichar* line              = *ptr;
  unichar* next_code         = line;
  struct list_ustring* codes = nullptr;
  while (*next_code != '\0' && *next_code != ':') {
    if (*next_code == '+') {
      unichar* new_code =
          tokenize_one_semantic_code(&next_code, config_filename);
      if (codes == nullptr) {
        codes = new_list_ustring(new_code);
      } else if (!is_in_list(new_code, codes)) {
        insert_at_end_of_list(new_code, codes);
      }
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
unichar* ConfigCommand::tokenize_one_inflectional_code(
    unichar** ptr, const char* config_filename) {
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
struct list_ustring* ConfigCommand::tokenize_inflectional_codes(
    unichar* str, const char* config_filename) {
  struct list_ustring* codes = nullptr;
  unichar* next_code         = str;
  while (*next_code != '\0') {
    if (*next_code == ':') {
      unichar* new_code =
          tokenize_one_inflectional_code(&next_code, config_filename);
      if (codes == nullptr) {
        codes = new_list_ustring(new_code);
      } else if (!is_in_list(new_code, codes)) {
        insert_at_end_of_list(new_code, codes);
      }
      free(new_code);
    }
  }
  return codes;
}

/*=================================================================
 * ConfigLine class method
 *================================================================= */

/**
 * Tokenize a config line.
 * Returns a std::unique_ptr<ConfigLine> if there is a well-formed line.
 * otherwise returns nullptr.
 * Raises a fatal error in case of malformed line.
 */
std::unique_ptr<ConfigLine> ConfigLine::tokenize_config_line(
    unichar* line, const char* config_filename) {
  unichar* nextNoEmptyUnichar           = line;
  unichar patternToken[INPUTSIZEBUFFER] = {0};
  if (advance_to_next_no_blank_char(&nextNoEmptyUnichar)) {
    return nullptr;  // skip empty line
  }
  if (nextNoEmptyUnichar[0] == '#') {
    return nullptr;  // skip comment line
  }
  if (recognize_pattern_token(&nextNoEmptyUnichar, patternToken)) {
    fatal_error(
        "Lexical mask must be enclosed in < >, like <be.V:K> in file: %s, "
        "line: '%S'\n",
        config_filename, line);
  }
  // build pattern
  struct pattern* pattern = build_pattern(patternToken, nullptr, 0, nullptr);
  if (advance_to_next_no_blank_char(&nextNoEmptyUnichar)) {
    // if there is no command, we skip the current line like comment line
    return nullptr;
  }
  // build nb_required_tag
  int nb_required_tag = NOT_SPECIFIED;
  if (nextNoEmptyUnichar[0] == '{') {
    if (tokenize_nb_required_tag(&nextNoEmptyUnichar, &nb_required_tag)) {
      fatal_error("Braces must contain number in file: %s, line: '%S'\n",
                  config_filename, line);
    }
  }
  if (advance_to_next_no_blank_char(&nextNoEmptyUnichar)) {
    // if there is no command, we skip the current line like comment line
    return nullptr;
  }
  // build config command
  auto config_commang = ConfigCommand::tokenize_config_command(
      nextNoEmptyUnichar, config_filename);
  if (u_strcmp(config_commang->get_lemma(), ConfigCommand::COMMA_COPY) == 0 &&
      nb_required_tag != NOT_SPECIFIED && nb_required_tag != 1) {
    fatal_error(
        "Command ,,copy is incompatible with an integer enclosed in curly "
        "braces, except for {1} in file: %s, line: '%S'\n",
        config_filename, line);
  }
  if (u_strcmp(config_commang->get_part_of_speech(), ConfigCommand::DOT_COPY) ==
          0 &&
      nb_required_tag != NOT_SPECIFIED && nb_required_tag != 1) {
    fatal_error(
        "Command ..copy is incompatible with an integer enclosed in curly "
        "braces, except for {1}in file:%s, line: '%S'\n",
        config_filename, line);
  }
  return std::make_unique<ConfigLine>(pattern, nb_required_tag,
                                      std::move(config_commang));
}

ConfigLine::ConfigLine(struct pattern* pattern, int nb_required_tag,
                       std::shared_ptr<ConfigCommand> config_command)
    : _pattern{pattern},
      _nb_required_tag{nb_required_tag},
      _config_command{std::move(config_command)} {
}

ConfigLine::~ConfigLine() {
  free_pattern(_pattern);
}

struct pattern* ConfigLine::get_pattern() const {
  return _pattern;
}

int ConfigLine::get_nb_required_tag() const {
  return _nb_required_tag;
}

std::shared_ptr<ConfigCommand> ConfigLine::get_config_command() const {
  return _config_command;
}

/**
 * Set *str to the next no blank character.
 * Assumes that *str ended with '\0'.
 * Returns 1 if *str ends with blank char, otherwise returns 0.
 */
int ConfigLine::advance_to_next_no_blank_char(unichar** str) {
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
 * Return 1 if error occurs, otherwise 0.
 * Assumes that pattern is suround by < >.
 * Assumes that empty space are removed from the begin,
 * using ConfigLine::advances_to_next_no_blank_char.
 */
int ConfigLine::recognize_pattern_token(unichar** ptr, unichar* res) {
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
 * using ConfigLine::advances_to_next_no_blank_char.
 */
int ConfigLine::tokenize_nb_required_tag(unichar** ptr, int* res) {
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
      *res              = u_parse_int(buffer, nullptr);
      *ptr              = line + index + 1;
      return 0;
    }
  }
  return 1;
}


/*=================================================================
 * Multi2Delaf class method
 *================================================================= */

Multi2Delaf::Multi2Delaf(const char* config_filename)
    : _config_filename{config_filename} {
}

/**
 * Read the configuration file that specifies how to transcode the 
 * multidelaf string into a delaf tag.
 * Raises a fatal error in case of malformed file.
 */
void Multi2Delaf::parse_config_file() {
  U_FILE* config_file = u_fopen(&_vec, _config_filename, U_READ);
  if (config_file == nullptr) {
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
  unichar* ptr               = buffer;
  auto delaf_tags            = std::vector<struct dela_entry*>();
  struct dela_entry* new_tag = nullptr;
  while (nullptr != (new_tag = tokenize_delaf_tag(&ptr))) {
    delaf_tags.push_back(new_tag);
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
  for (const auto& tag : delaf_tags) {
    free_dela_entry(tag);
  }
}

/**
 * Read a line from the configuration file, and save the result in buffer.
 * Returns EOF if end of file occurs, otherwise the number of unichar readed.
 */
int Multi2Delaf::read_line_config_file(U_FILE* config_file, unichar* buffer,
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
 * Load the configuration file.
 * Raises a fatal error in case of malformed file.
 */
void Multi2Delaf::load_config_file(U_FILE* config_file) {
  unichar line[INPUTSIZEBUFFER] = {0};
  int eof                       = 0;
  while (EOF !=
         (eof = read_line_config_file(config_file, line, INPUTSIZEBUFFER))) {
    auto config_line = ConfigLine::tokenize_config_line(
        line, filename_without_path(_config_filename));
    if (config_line != nullptr) {
      _config_lines.emplace_back(std::move(config_line));
    }
  }
  // the last line is potentially a config line
  auto config_line = ConfigLine::tokenize_config_line(
      line, filename_without_path(_config_filename));
  if (config_line != nullptr) {
    _config_lines.emplace_back(std::move(config_line));
  }
}

/**
 * Return the first delaf tag at the address *ptr.
 * Return nullptr to indicate that there is no more tag.
 * Set *ptr to the next unread character.
 * Raises a fatal error if the delaf tag is not enclosed in curly braces.
 */
struct dela_entry* Multi2Delaf::tokenize_delaf_tag(unichar** ptr) {
  unichar* line               = *ptr;
  unichar* next_no_blank_char = line;
  if (ConfigLine::advance_to_next_no_blank_char(&next_no_blank_char)) {
    return nullptr;  // end of the line, no more dela_entry
  }
  if (next_no_blank_char[0] != '{') {
    fatal_error("Delaf tag must be enclosed in curly braces, line: '%S'\n '",
                line);
  }
  int i = 1;
  while (next_no_blank_char[i] != '\0' && next_no_blank_char[i] != '}') {
    i++;
  }
  if (next_no_blank_char[i] == '\0') {
    fatal_error("Delaf tag must be enclosed in curly braces, line: '%S'\n '",
                line);
  }
  unichar* token_dela_entry = u_strndup(line + 1, i - 1);
  struct dela_entry* tag    = tokenize_DELAF_line(token_dela_entry);
  free(token_dela_entry);
  *ptr = line + i + 1;  // + 1 to skip closing brace '}'
  return tag;
}

/**
 * Return the number of delaf tags that match the pattern.
 */
int Multi2Delaf::nb_delaf_tag_that_match_pattern(
    const std::vector<struct dela_entry*>& delaf_tags,
    const struct pattern* pattern) {
  return std::count_if(delaf_tags.begin(), delaf_tags.end(),
                       [&pattern](const auto& tag) {
                         return is_entry_compatible_with_pattern(tag, pattern);
                       });
}

/**
 * Return a new allocated unicode string.
 */
unichar* Multi2Delaf::escape_inflected_input(const unichar* input) {
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
  buffer[std::min(i_buffer, INPUTSIZEBUFFER - 1)] = '\0';
  return u_strdup(buffer);
}

/**
 * Retrieve the lemma according to the specification:
 * We look every line of _config_lines and keep the lemma of the first-one matching containing a lemma.
 * If no lemma is corresponding, raises a fatal_error().
 * Return a new allocated unicode string describing the lemma.
 */
unichar* Multi2Delaf::retrieve_lemma(
    const std::vector<struct dela_entry*>& delaf_tags,
    const unichar* multidelaf_string) const {
  for (const auto& current_line : _config_lines) {
    if (current_line->get_config_command()->get_lemma() == nullptr) {
      continue;
    }
    for (const auto& tag : delaf_tags) {
      if (is_entry_compatible_with_pattern(tag, current_line->get_pattern())) {
        if (u_strcmp(current_line->get_config_command()->get_lemma(),
                     ConfigCommand::COMMA_COPY) == 0) {
          if (nb_delaf_tag_that_match_pattern(
                  delaf_tags, current_line->get_pattern()) != 1) {
            fatal_error(
                "Command ,,copy can be interpreted for several delaf line: "
                "%S\n",
                multidelaf_string);
          }
          return u_strdup(tag->lemma);
        }
        if (current_line->get_nb_required_tag() == 1 ||
            current_line->get_nb_required_tag() == ConfigLine::NOT_SPECIFIED) {
          return u_strdup(current_line->get_config_command()->get_lemma());
        }
        if (current_line->get_nb_required_tag() ==
            nb_delaf_tag_that_match_pattern(delaf_tags,
                                            current_line->get_pattern())) {
          return u_strdup(current_line->get_config_command()->get_lemma());
        }
      }
    }
    if (current_line->get_nb_required_tag() == 0) {
      return u_strdup(current_line->get_config_command()->get_lemma());
    }
  }
  fatal_error("No lemma is provided for this multidelaf string: %S\n",
              multidelaf_string);
  return nullptr;
}

/**
 * Retrieves the part_of_speech according to the specification:
 * We look every line of config lines and keep the part_of_speech of the first-one matching containing a part_of_speech.
 * If no part_of_speech is corresponding, raises a fatal_error().
 * Return a new allocated unicode string describing the part of speech.
 */
unichar* Multi2Delaf::retrieve_part_of_speech(
    const std::vector<struct dela_entry*>& delaf_tags,
    const unichar* multidelaf_string) const {
  for (const auto& current_line : _config_lines) {
    if (current_line->get_config_command()->get_part_of_speech() == nullptr) {
      continue;
    }
    for (const auto& tag : delaf_tags) {
      if (is_entry_compatible_with_pattern(tag, current_line->get_pattern())) {
        if (u_strcmp(current_line->get_config_command()->get_part_of_speech(),
                     ConfigCommand::DOT_COPY) == 0) {
          if (nb_delaf_tag_that_match_pattern(
                  delaf_tags, current_line->get_pattern()) != 1) {
            fatal_error(
                "Command ..copy can be interpreted for several delaf tag: %S\n",
                multidelaf_string);
          }
          return u_strdup(tag->semantic_codes[0]);
        }
        if (current_line->get_nb_required_tag() == 1 ||
            current_line->get_nb_required_tag() == ConfigLine::NOT_SPECIFIED) {
          return u_strdup(
              current_line->get_config_command()->get_part_of_speech());
        }
        if (current_line->get_nb_required_tag() ==
            nb_delaf_tag_that_match_pattern(delaf_tags,
                                            current_line->get_pattern())) {
          return u_strdup(
              current_line->get_config_command()->get_part_of_speech());
        }
      }
    }
    if (current_line->get_nb_required_tag() == 0) {
      return u_strdup(current_line->get_config_command()->get_part_of_speech());
    }
  }
  fatal_error(
      "No grammatical cathegory is provided for this multidelaf string: %S\n",
      multidelaf_string);
  return nullptr;
}

/**
 * Retrieves semantic codes according to the specification:
 * The semantic codes of the multidelaf string are obtained by taking the union
 * of the semantic codes of each tag assigned by the configuration file.
 * If no semantic_codes is corresponding, return an empty string.
 * Return a new allocated unicode string describing semantic codes.
 */
unichar* Multi2Delaf::retrieve_semantic_codes(
    const std::vector<struct dela_entry*>& delaf_tags) const {
  struct list_ustring* codes       = nullptr;
  struct list_ustring* ptr_command = nullptr;

  for (const auto& tag : delaf_tags) {
    for (const auto& line : _config_lines) {
      if (line->get_config_command()->get_semantic_codes() == nullptr) {
        continue;
      }
      if (is_entry_compatible_with_pattern(tag, line->get_pattern())) {
        ptr_command = line->get_config_command()->get_semantic_codes();
        while (ptr_command != nullptr) {
          if (line->get_nb_required_tag() == ConfigLine::NOT_SPECIFIED ||
              line->get_nb_required_tag() ==
                  nb_delaf_tag_that_match_pattern(delaf_tags,
                                                  line->get_pattern())) {
            if (u_strcmp(ptr_command->string, ConfigCommand::PLUS_COPY) == 0) {
              for (int i = 1; i < tag->n_semantic_codes;
                   i++) {  // begin at 1 to skip the grammatical catergory
                if (codes == nullptr) {
                  codes = new_list_ustring(tag->semantic_codes[i]);
                } else if (!is_in_list(tag->semantic_codes[i], codes)) {
                  insert_at_end_of_list(tag->semantic_codes[i], codes);
                }
              }
            } else {
              if (line->get_nb_required_tag() != 0) {
                if (codes == nullptr) {
                  codes = new_list_ustring(ptr_command->string);
                } else if (!is_in_list(ptr_command->string, codes)) {
                  insert_at_end_of_list(ptr_command->string, codes);
                }
              }
            }
          }
          ptr_command = ptr_command->next;
        }
      } else {
        if (0 == line->get_nb_required_tag() &&
            0 == nb_delaf_tag_that_match_pattern(delaf_tags,
                                                 line->get_pattern())) {
          struct list_ustring* ptr_command =
              line->get_config_command()->get_semantic_codes();
          while (ptr_command != nullptr) {
            if (codes == nullptr) {
              codes = new_list_ustring(ptr_command->string);
            } else {
              if (!is_in_list(ptr_command->string, codes)) {
                insert_at_end_of_list(ptr_command->string, codes);
              }
            }
            ptr_command = ptr_command->next;
          }
        }
      }
    }
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
    const std::vector<struct dela_entry*>& delaf_tags) const {
  struct list_ustring* codes       = nullptr;
  struct list_ustring* ptr_command = nullptr;

  for (const auto& tag : delaf_tags) {
    for (const auto& line : _config_lines) {
      if (line->get_config_command()->get_inflectional_codes() == nullptr) {
        continue;
      }
      if (is_entry_compatible_with_pattern(tag, line->get_pattern())) {
        ptr_command = line->get_config_command()->get_inflectional_codes();
        while (ptr_command != nullptr) {
          if (line->get_nb_required_tag() == ConfigLine::NOT_SPECIFIED ||
              line->get_nb_required_tag() ==
                  nb_delaf_tag_that_match_pattern(delaf_tags,
                                                  line->get_pattern())) {
            if (u_strcmp(ptr_command->string, ConfigCommand::COLUMN_COPY) ==
                0) {
              for (int i = 0; i < tag->n_inflectional_codes; i++) {
                if (codes == nullptr) {
                  codes = new_list_ustring(tag->inflectional_codes[i]);
                } else if (!is_in_list(tag->inflectional_codes[i], codes)) {
                  insert_at_end_of_list(tag->inflectional_codes[i], codes);
                }
              }
            } else {
              if (line->get_nb_required_tag() != 0) {
                if (codes == nullptr) {
                  codes = new_list_ustring(ptr_command->string);
                } else if (!is_in_list(ptr_command->string, codes)) {
                  insert_at_end_of_list(ptr_command->string, codes);
                }
              }
            }
          }
          ptr_command = ptr_command->next;
        }
      } else {
        if (0 == line->get_nb_required_tag() &&
            0 == nb_delaf_tag_that_match_pattern(delaf_tags,
                                                 line->get_pattern())) {
          struct list_ustring* ptr_command =
              line->get_config_command()->get_inflectional_codes();
          while (ptr_command != nullptr) {
            if (codes == nullptr) {
              codes = new_list_ustring(ptr_command->string);
            } else {
              if (!is_in_list(ptr_command->string, codes)) {
                insert_at_end_of_list(ptr_command->string, codes);
              }
            }
            ptr_command = ptr_command->next;
          }
        }
      }
    }
  }
  unichar* res = build_output_codes(codes, ':');
  if (codes) {
    free_list_ustring(codes);
  }
  return res;
}

/**
 * Return a new allocated unicode string.
 */
unichar* Multi2Delaf::build_output_codes(const struct list_ustring* codes,
                                         char prefix) {
  unichar buffer[INPUTSIZEBUFFER] = {0};
  if (codes == nullptr) {
    return u_strdup("");
  }
  while (codes != nullptr) {
    if (u_strlen(buffer) + u_strlen(codes->string) + 2 >= INPUTSIZEBUFFER - 1) {
      fatal_error(
          "internal err(Multi2Delaf::build_output_codes): buffer is not "
          "big enough\n");
    }
    u_sprintf(buffer, "%S%c%S", buffer, prefix, codes->string);
    codes = codes->next;
  }
  return u_strdup(buffer);
}

}  // namespace unitex