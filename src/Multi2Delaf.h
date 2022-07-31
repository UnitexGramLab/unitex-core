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

#ifndef MULTI2DELAFH
#define MULTI2DELAFH

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

#include <memory>
#include <vector>

#include "DELA.h"
#include "Pattern.h"
#include "Unicode.h"
#include "base/compiler/keyword/eq_delete.h"

namespace unitex {

/**
 * Describes a command in the file that configures the parser for multidelaf strings.
 * A multidelaf string is an output of Explore graph paths made of a sequence of 
 * Delaf tags.
 */
struct ConfigCommand {
  unichar* lemma;
  unichar* part_of_speech;
  struct list_ustring* semantic_codes;
  struct list_ustring* inflectional_codes;
};

/**
 * Describes a line in the file that configures the parser for multidelaf strings.
 * A multidelaf string is an output of Explore graph paths made of a sequence of 
 * Delaf tags.
 */
struct ConfigLine {
  struct pattern* pattern;  // lexical mask
  // if _nb_required_tag equals to NOT_SPECIFIED, the number of delaf tags that must match the pattern is not specified
  // else if _nb_required_tag equals to 0, the pattern must not match any delaf tag
  // otherwise the numer of delaf tag that must match the pattern
  int nb_required_tag;
  struct ConfigCommand* config_command;
};

/**
 * Multi2Delaf class
 */
class Multi2Delaf {
 public:
  static constexpr const char* const COMMA_COPY  = ",copy";
  static constexpr const char* const DOT_COPY    = ".copy";
  static constexpr const char* const PLUS_COPY   = "+copy";
  static constexpr const char* const COLUMN_COPY = ":copy";
  static constexpr int NOT_SPECIFIED = -1;  // nb_required_tag default value

  Multi2Delaf(const char* config_filename);
  // Not copyable or movable
  Multi2Delaf(const Multi2Delaf&) UNITEX_EQ_DELETE;
  Multi2Delaf(Multi2Delaf&&) UNITEX_EQ_DELETE;
  Multi2Delaf& operator=(const Multi2Delaf& other) UNITEX_EQ_DELETE;
  Multi2Delaf& operator=(Multi2Delaf&& other) UNITEX_EQ_DELETE;
  void parse_config_file();
  void translate_multidelaf_to_delaf(const unichar* inflected_input,
                                     unichar* buffer) const;
  ~Multi2Delaf();

 private:
  static int read_line_config_file(U_FILE* config_file, unichar* buffer,
                                   int size_buffer);
  void load_config_file(U_FILE* config_file);
  static struct dela_entry* tokenize_delaf_tag(unichar** next);
  static int nb_delaf_tag_that_match_pattern(
      const std::vector<struct dela_entry*>& delaf_tags,
      const struct pattern* pattern);
  static unichar* escape_inflected_input(const unichar* input);
  unichar* retrieve_lemma(const std::vector<struct dela_entry*>& delaf_tags,
                          const unichar* multidelaf_string) const;
  unichar* retrieve_part_of_speech(
      const std::vector<struct dela_entry*>& delaf_tags,
      const unichar* multidelaf_string) const;
  unichar* retrieve_semantic_codes(
      const std::vector<struct dela_entry*>& delaf_tags) const;
  static unichar* complete_first_with_second(const unichar* first,
                                             const unichar* second);
  static struct list_ustring* clone_and_replace_copy_command(
      const struct list_ustring* inflectional_command,
      const struct dela_entry* tag);
  static struct list_ustring* product(struct list_ustring* l1,
                                      struct list_ustring* l2);
  unichar* retrieve_inflectional_codes(
      const std::vector<struct dela_entry*>& delaf_tags) const;
  static unichar* build_output_codes(const struct list_ustring* list,
                                     char separator);
  const VersatileEncodingConfig _vec = VEC_DEFAULT;
  std::vector<struct ConfigLine*> _config_lines;
  const char* _config_filename;
};

}  // namespace unitex

#endif
