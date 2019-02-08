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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Unicode.h"
#include "DELA.h"
#include "PackInf.h"
#include "DictionaryTree.h"
#include "String_hash.h"
#include "AutomatonDictionary2Bin.h"
#include "File.h"
#include "Copyright.h"
#include "Error.h"
#include "UnitexGetOpt.h"
#include "Compress.h"
#include "ProgramInvoker.h"
#include "BitArray.h"
#include "CompressedDic.h"
#include "Ustring.h"
#include "UnitexRevisionInfo.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

const char* usage_Compress =
"Usage:\n"
"  Compress [options] DICTIONARY\n"
"  Compress [options] DICTIONARY... -o BINFILE\n"
"  \n"
"Compress one or more DELAF dictionaries into a finite state automaton.\n"
"\n"
"Arguments:\n"
"  DICTIONARY                      input plain text dictionary in DELAF format\n"
" \n"
"Compression options:\n"
"  -f, --flip                      specifies that the inflected and lemma forms\n"
"                                  must be swapped. This option could be used to\n"
"                                  construct an inverse dictionary\n"
"  -s, --semitic                   uses the semitic compression algorithm. This\n"
"                                  option is useful to reduce the size of the\n"
"                                  output when dealing with semitic languages\n"
" \n"
"Output options:\n"
"  -t TYPE, --output_type=TYPE     specifies the type of the output file.\n"
"                                  Optional types are: bin1, produces a .bin file\n"
"                                  with the associated inflectional codes stored\n"
"                                  in a .inf file; bin2, produces a .bin file\n"
"                                  with the associated inflectional codes bundled\n"
"                                  in it, i.e. no .inf file is created\n"
"                                  [default: bin1]\n"
"  -o BINFILE, --output=BINFILE    filename used to write the produced automaton\n"
"  -p, --pack-inf                  create a packed inf file (.inp)\n"
" \n"
"Deprecated options:\n"
"  --v1                            produces an old style .bin file with a size\n"
"                                  limitation to 16MB. Use instead --output_type\n"
"  --v2                            produces a .bin file with no file size\n"
"                                  limitation and with a smaller size than --v1.\n"
"                                  Same as --output_type=bin1 [default]\n"
"  --bin2                          Same as --output_type=bin2\n"
" \n"
"Encoding options:\n"
"  -k ENCODING, --input_encoding=ENCODING\n"
"                                  [default: utf16le-bom,utf16be-bom,utf8-bom]\n"
"  -q ENCODING, --output_encoding=ENCODING\n"
"                                  [default: utf16le-bom]\n"
" \n"
"Other options:\n"
"  -V, --only-verify-arguments     only verify arguments syntax and exit\n"
"  -h, --help                      show this help message and exit\n"
"  --version                       show version and exit\n"
"";

const char* optstring_Compress = ":fpo:hk:t:Vq:s";

const struct option_TS lopts_Compress[] = {
  { (char *) "bin2"                 , no_argument_TS       , NULL,   2  },
  { (char *) "pack-inf"             , no_argument_TS       , NULL,  'p' },
  { (char *) "flip"                 , no_argument_TS       , NULL,  'f' },
  { (char *) "help"                 , no_argument_TS       , NULL,  'h' },
  { (char *) "only-verify-arguments", no_argument_TS       , NULL,  'V' },
  { (char *) "semitic"              , no_argument_TS       , NULL,  's' },
  { (char *) "v1"                   , no_argument_TS       , NULL,   3  },
  { (char *) "v2"                   , no_argument_TS       , NULL,   4  },
  { (char *) "version"              , no_argument_TS       , NULL,   1  },
  { (char *) "input_encoding"       , required_argument_TS , NULL,  'k' },
  { (char *) "output"               , required_argument_TS , NULL,  'o' },
  { (char *) "output_type"          , required_argument_TS , NULL,  't' },
  { (char *) "output_encoding"      , required_argument_TS , NULL,  'q' },
  { NULL                            , no_argument_TS       , NULL,   0  }
};

static void usage() {
  display_copyright_notice();
  u_printf(usage_Compress);
}

static const size_t step_filename_buffer =
                    ((((DIC_WORD_SIZE * sizeof(unichar)) / 0x10) + 1) * 0x10);

// function used to minimize a dictionary tree, i.e. to construct a minimal ADFA
typedef void(*minimize_func)(struct dictionary_node* root,
                             struct bit_array* used_inf_values,
                             Abstract_allocator prv_alloc);

/**
 * This function writes the number of INF codes 'n' at the beginning
 * of the file named 'name'. This file is supposed to be a UTF-16
 * Little-Endian one, starting by a sequence made of ten zeros.
 */
static void write_INF_file_header(const VersatileEncodingConfig* vec,
                                  const char* name,
                                  int n) {
  U_FILE* f = u_fopen(vec, name, U_MODIFY);
  char number[11] = {0};
  number[10] = 0;
  int offset = 9;
  for (;;) {
      number[offset] = (char)((n%10)+'0');
      n /= 10;
      if (offset == 0)
          break;
      --offset;
  }

  u_fprintf(f, number);
  u_fclose(f);
}

/**
 * @brief Builds a tree representation of a DELAF dictionary
 *
 * Builds a tree representation of a DELAF dictionary pointed by \a filename.
 * The output \root structure represents the starting node of the dictionary
 * being loaded. This function returns a succeed status if at least one entry
 * was processed, this is true even if the dictionary contains invalid entries
 *
 * @param[in] vec encoding I/O Configuration
 * @param[in] filename null-terminated string, with a filename to process
 * @param[in] FLIP inflected and lemma forms must be swapped 0:no 1:yes
 * @param[in] semitic use the semitic compression algorithm 0:no 1:yes
 * @param[out] dictionary_list insert other dictionaries referred by \a filename
 * @param[out] root initial state of the dictionary tree
 * @param[out] inf_codes all the INF codes used by the dictionary tree
 * @param[out] n_entries total entries processed without comments
 * @param[out] n_lines total lines scanned including comments
 * @param[out] n_line_errors total lines errors including comments
 * @param[out] n_file_includes total files included from \a filename
 * @return SUCCESS_RETURN_CODE or other status code. See Error.h for details
 * @author Cristian Martinez (based in a previous version of Sébastien Paumier)
 */
static int build_tree_from_dictionary(
                     const VersatileEncodingConfig* vec,
                     const unichar* filename,
                     int FLIP,
                     int semitic,
                     list_ustring_ptr dictionary_list,
                     struct dictionary_node* root,
                     struct string_hash* inf_codes,
                     int* n_entries,
                     int* n_lines,
                     int* n_line_errors,
                     int* n_file_includes,
                     Abstract_allocator compress_abstract_allocator,
                     Abstract_allocator compress_tokenize_abstract_allocator) {
  // reserve some heap memory to manipule strings
  char* heap_buffer = (char*) malloc(step_filename_buffer * 7);
  if (heap_buffer == NULL) {
    alloc_error("compress_dictionary");
    return ALLOC_ERROR_CODE;
  }

  // null-terminated string version of the dictionary filename
  char* filename_as_char           = (heap_buffer + (step_filename_buffer * 0));

  // this is because u_fopen do not deal for now with unicode filenames
  u_to_char_n(filename_as_char, filename, step_filename_buffer);

  // try to open the dictionary
  U_FILE* file_handler = u_fopen(vec, filename_as_char, U_READ);
  if (!file_handler) {
    error("Could not open %S\n", filename);
    free(heap_buffer);
    return DEFAULT_ERROR_CODE;
  }

  // filename is a directory not a plain text file
  if(is_directory(filename_as_char)) {
    error("%S: Is a directory\n", filename);
    free(heap_buffer);
    return DEFAULT_ERROR_CODE;
  }

  // store meta-comment lines, i.e. comments starting by //!
  char* meta_comment               = (heap_buffer + (step_filename_buffer * 1));

  // store the name of a file to be inserted in the dictionary_list
  char* meta_filename              = (heap_buffer + (step_filename_buffer * 2));

  // store the resolved filename version of meta_filename
  char* meta_resolved_filename     = (heap_buffer + (step_filename_buffer * 3));

  // a temporal buffer to hold a compressed line that will be used to
  // rebuild the whole DELAF line
  unichar* compress_line = (unichar*)(heap_buffer + (step_filename_buffer * 4));

  // current line counter including comments
  int current_line  = 0;

  // current entry counter, i.e. without taking in account comments
  int current_entry = 0;

  // current line string
  Ustring* line     = new_Ustring(DIC_WORD_SIZE);

  // represents an entry of the current dictionary
  struct dela_entry* entry = NULL;

  // set to 1 when the abstract allocator
  int tokenize_allocator_has_clean = ((get_allocator_flag(
                                       compress_tokenize_abstract_allocator) &
                                       AllocatorCleanPresent) != 0);

  // read dictionary line-by-line
  while (EOF != readline(line, file_handler)) {
    switch (line->str[0]) {
      // disallow empty lines
      case '\0':
        // empty lines should not appear in a .dic file
        error("Line %d: empty line\n", current_line+1);
        (*n_line_errors)++;
        break;

      // deal with line comments
      case '/':
        // test first if we are dealing with a meta-comment
        // a meta-comment starts by //!
        if ((line->str[1] && line->str[1] == '/') &&
            (line->str[2] && line->str[2] == '!')) {
          // ignore spaces after //!
          int i = 3;
          while (line->str[i] && line->str[i] == ' ') ++i;

          // get all characters until the end-of-line
          int j = i;
          while (line->str[j]) ++j;

          // ignore final spaces
          while (j > i && line->str[j-1] && line->str[j-1] == ' ') --j;

          // if it's not an empty line
          if (j > i) {
            // copy the trimmed line into the meta-comment buffer
            u_to_char_n(meta_comment, line->str+i, j-i);

            // meta-comment processing

            // case No 1: Including a dictionary filename
            // if the meta-comment is a valid file name,
            // We try to process it as a dictionary file

            // convert to the native path representation
            // i.e /foo/bar.dic <-> \foo\bar.dic
            to_native_path_separators(meta_comment);

            // meta_comment is a relative filename e.g. "bar.dic"
            if (!is_absolute_path(meta_comment)) {
              // get the path of the current dictionary file
              get_path(filename_as_char, meta_filename);
              // make sure that meta_filename ends with a path separator
              add_path_separator(meta_filename);
              // finally, meta_filename is equal to the current file path
              // plus the meta comment filename
              strcat(meta_filename, meta_comment);
            // absolute filename e.g. "/foo/bar.dic"
            } else {
              // warn about use of absolute path names
              error("Absolute paths to include a dictionary inside another "
                    "is discouraged\n"
                    "%s:%d: %s\n",
                    filename_without_path(filename_as_char),
                    current_line+1,
                    meta_comment);
              (*n_line_errors)++;
              // finally, meta_filename is equal the absolute filename
              strcpy(meta_filename, meta_comment);
            }  // (!is_absolute_path(meta_comment))

            // gets the real path of the file that is being included
            // this is necessary to avoid recursive file inclusions
            if (get_real_path(meta_filename,
                              meta_resolved_filename) != SUCCESS_RETURN_CODE) {
              error("Cannot include dictionary. File does not exist\n"
                    "%s:%d: %s\n",
                    filename_without_path(filename_as_char),
                    current_line+1,
                    meta_filename);
              (*n_line_errors)++;
              break;
            }

            // only include in the list if it is not already present
            if (!is_in_list(meta_resolved_filename, dictionary_list, u_strcmp_ignore_case)) {
                dictionary_list = insert_at_end_of_list(meta_resolved_filename,
                                                        dictionary_list);
                ++(*n_file_includes);
            // warn about a possible recursive inclusion
            } else {
              error("Ignoring dictionary because it already exists "
                    "in the processing queue\n"
                    "%s:%d: %s\n",
                    filename_without_path(filename_as_char),
                    current_line+1,
                    meta_resolved_filename);
              (*n_line_errors)++;
            }
          }  // if (j > i)
        }  // if((s->str[1] && s->str[1] == '/') &&

        // from now we're dealing with a traditional comment line
        break;

      default:
        // if we have a line, we tokenize it

        // reinitialize entry to NULL, in this way if replace_special_equal_signs()
        // fails, an error will be threw
        entry = NULL;
        /* First, to avoid problems, we replace by the char #1 any occurrence
         * of '=' that should be replaced by ' ' and and '-'. For instance:
         *
         * =, X.Y   =>   $, X.Y  ($ stands here for the char whose code is 1)
         * \=, X.Y  =>  \=, X.Y
         * \\=, X.Y =>  \$, X.Y
         */
        if(replace_special_equal_signs(line->str) == SUCCESS_RETURN_CODE) {
           // tokenize the current entry
           entry = tokenize_DELAF_line(line->str,  // entry to parse
                                       1,          // comments are allowed at EOL
                                       NULL,       // must print error messages
                                       compress_tokenize_abstract_allocator);
        }

        // if the entry is not well-formed throw an error indicating
        // the file name and the line number where the error happened
        if (entry == NULL) {
          error("%s:%d\n",
                filename_without_path(filename_as_char),
                current_line+1);
          (*n_line_errors)++;
          // breaks switch
          break;
        }

        // if the entry is well-formed
        // The unescaped = that were not in the inflected or lemma form must
        // be restored as real = character
        for (int i = 0; i < entry->n_semantic_codes; ++i) {
          replace_unprotected_equal_sign(entry->semantic_codes[i],
                                         (unichar)'=');
        }
        for (int i = 0; i < entry->n_inflectional_codes; ++i) {
          replace_unprotected_equal_sign(entry->inflectional_codes[i],
                                         (unichar)'=');
        }
        if (FLIP) {
          // If the "-flip" parameter has been used, we flip
          // the inflected form and the lemma of the entry
          unichar* o       = entry->inflected;
          entry->inflected = entry->lemma;
          entry->lemma     = o;
        }
        if (contains_unprotected_equal_sign(entry->inflected)
            || contains_unprotected_equal_sign(entry->lemma)) {
          /* If the inflected form or lemma contains any unprotected = sign,
           * we must insert the space entry and the - entry:
           * pomme=de=terre, .N  ->  pomme de terre, pomme de terre.N
           *                        pomme-de-terre, pomme-de-terre.N
           */
          unichar* inflected = u_strdup(entry->inflected,
                                        compress_tokenize_abstract_allocator);

          unichar* lemma     = u_strdup(entry->lemma,
                                        compress_tokenize_abstract_allocator);
          if (inflected == NULL || lemma == NULL) {
            alloc_error("main_Compress");
#           if (defined(UNITEX_LIBRARY) || defined(UNITEX_RELEASE_MEMORY_AT_EXIT))
            if (tokenize_allocator_has_clean == 0) {
              free_dela_entry(entry, compress_tokenize_abstract_allocator);
            }
#           endif
            free_Ustring(line);
            u_fclose(file_handler);
            free(heap_buffer);
            return ALLOC_ERROR_CODE;
          }

          unichar* inf_tmp = (unichar*)(heap_buffer + (step_filename_buffer * 5));
          unichar* lem_tmp = (unichar*)(heap_buffer + (step_filename_buffer * 6));
          u_strcpy_sized(inf_tmp, DIC_WORD_SIZE, entry->inflected);
          u_strcpy_sized(lem_tmp, DIC_WORD_SIZE, entry->lemma);

          // we replace the unprotected = signs by spaces
          replace_unprotected_equal_sign(entry->inflected, (unichar)' ');
          replace_unprotected_equal_sign(entry->lemma, (unichar)' ');

          // we insert "pomme de terre, pomme de terre.N"
          get_compressed_line(entry, compress_line, semitic);
          add_entry_to_dictionary_tree(entry->inflected,
                                       compress_line,
                                       root,
                                       inf_codes,
                                       current_line,
                                       compress_abstract_allocator);

          // and then we insert "pomme-de-terre, pomme-de-terre.N"
          u_strcpy(entry->inflected, inf_tmp);
          u_strcpy(entry->lemma, lem_tmp);

          // we replace the unprotected = signs by minus
          free_cb(entry->inflected, compress_tokenize_abstract_allocator);
          entry->inflected = inflected;
          free_cb(entry->lemma, compress_tokenize_abstract_allocator);
          entry->lemma     = lemma;
          replace_unprotected_equal_sign(entry->inflected, (unichar)'-');
          replace_unprotected_equal_sign(entry->lemma, (unichar)'-');
          get_compressed_line(entry, compress_line, semitic);
          add_entry_to_dictionary_tree(entry->inflected,
                                       compress_line,
                                       root,
                                       inf_codes,
                                       current_line,
                                       compress_abstract_allocator);
        } else {
          get_compressed_line(entry, compress_line, semitic);
          add_entry_to_dictionary_tree(entry->inflected,
                                       compress_line,
                                       root,
                                       inf_codes,
                                       current_line,
                                       compress_abstract_allocator);
        }

        // and last, but not least: don't forget to free your memory
        // or it would be impossible to compress large dictionaries
        if (tokenize_allocator_has_clean == 0) {
          free_dela_entry(entry, compress_tokenize_abstract_allocator);
        } else {
          clean_allocator(compress_tokenize_abstract_allocator);
        }
        // only increment if the entry is well-formed
        current_entry++;
        break;
    }  // switch(line->str[0])

    current_line++;

    // We print something at regular intervals in order to show
    // that the program actually works
    if (current_line%10000 == 0) {
      u_printf("%d line%s read...\r", current_line, (current_line > 1)? "s":"");
      if (compress_tokenize_abstract_allocator != NULL) {
        if (tokenize_allocator_has_clean == 0) {
          close_abstract_allocator(compress_tokenize_abstract_allocator);
          compress_tokenize_abstract_allocator =
              create_abstract_allocator("main_Compress_tokenize",
                                        AllocatorCreationFlagAutoFreePrefered |
                                        AllocatorCreationFlagCleanPrefered);
        }
      }
    }
  }  // while (EOF!=readline(line, file_handler)) {

  *n_lines   = current_line;
  *n_entries = current_entry;

  free_Ustring(line);
  u_fclose(file_handler);
  free(heap_buffer);

  // always succeed if we reach the end of the function,
  // this is true even if the dictionary contains invalid lines,
  // test against n_line_errors to check for invalid lines
  return SUCCESS_RETURN_CODE;
}

/**
 * @brief Builds a tree representation of a list of DELAF dictionaries
 *
 * Builds a tree representation of a list of DELAF dictionaries
 * pointed by \a dictionary_list. The output \a root structure
 * represents the starting node of the dictionary tree being created.
 *
 * @param[in] vec encoding I/O Configuration
 * @param[in] FLIP inflected and lemma forms must be swapped 0:no 1:yes
 * @param[in] semitic use the semitic compression algorithm 0:no 1:yes
 * @param[in] dictionary_list of dictionaries to process
 * @param[out] root initial state of the dictionary tree
 * @param[out] inf_codes all the INF codes used by the dictionary tree
 * @param[out] n_files total file read
 * @param[out] n_lines total lines scanned including commentaries
 * @param[out] n_entries total entries processed without file commentaries
 * @return SUCCESS_RETURN_CODE or other status code. See Error.h for details
 * @author Cristian Martinez
 */
static int build_tree_from_dictionary_list(
                     const VersatileEncodingConfig* vec,
                     int FLIP,
                     int semitic,
                     list_ustring_ptr dictionary_list,
                     struct dictionary_node* root,
                     struct string_hash* inf_codes,
                     int* n_entries,
                     int* n_lines,
                     int* n_line_errors,
                     int* n_files,
                     Abstract_allocator compress_abstract_allocator,
                     Abstract_allocator compress_tokenize_abstract_allocator) {
  // number of entries that were processed in the file that is being read
  int current_file_total_entries      = 0;

  // number of lines that were scanned in the file that is being read
  int current_file_total_lines        = 0;

  // number of lines that were scanned in the file that is being read
  int current_file_total_line_errors  = 0;

  // number of files that were included in the file that is being read
  int current_file_total_includes     = 0;

  // points to the dictionary filename that is being compressed
  list_ustring_ptr current_dictionary = dictionary_list;

  // default return value
  int return_value = SUCCESS_RETURN_CODE;

  // loop to process all files or until an error happens
  while (current_dictionary != NULL &&
         return_value == SUCCESS_RETURN_CODE) {
    u_printf("Compressing %S...\n", current_dictionary->string);

    current_file_total_lines        = 0;
    current_file_total_entries      = 0;
    current_file_total_line_errors  = 0;
    current_file_total_includes     = 0;

    return_value = build_tree_from_dictionary(
       vec,                                    // I/O encoding
       current_dictionary->string,             // current dictionary filename
       FLIP,                                   // inflected and lemma swap
       semitic,                                // semitic compression algorithm
       dictionary_list,                        // dictionaries filenames
       root,                                   // automaton initial state
       inf_codes,                              // all the INF codes
       &current_file_total_entries,            // entries processed
       &current_file_total_lines,              // lines scanned
       &current_file_total_line_errors,        // lines with errors
       &current_file_total_includes,           // total includes
       compress_abstract_allocator,
       compress_tokenize_abstract_allocator);

    // throw an error if there are not entries to process in this dictionary
    if (return_value == SUCCESS_RETURN_CODE &&
        current_file_total_includes == 0    &&
        current_file_total_entries  == 0) {
      error("%S: Empty dictionary\n", current_dictionary->string);
    }

    // update counters
    (*n_entries)     += current_file_total_entries;
    (*n_lines)       += current_file_total_lines;
    (*n_line_errors) += current_file_total_line_errors;
    (*n_files)++;

    // get the next filename to be processed
    current_dictionary = current_dictionary->next;
  }  // while (current_dictionary != NULL)

  // succeed only if there are entries to process
  if(return_value == SUCCESS_RETURN_CODE && *n_entries <= 0) {
    return DEFAULT_ERROR_CODE;
  }

  return return_value;
}

/**
 * @brief Creates an inflectional information file
 *
 * @param[in] vec encoding I/O Configuration
 * @param[in] filename null-terminated string, with the filename to create
 * @param[in] used_inf_values INF codes that are referenced by a dictionary tree
 * @param[in] inf_codes all the INF codes used by a dictionary tree
 * @param[out] inf_indirection indirect references to reordered INF codes
 * @param[out] n_used_inf_codes total number of INF codes used
 * @return SUCCESS_RETURN_CODE or other status code. See Error.h for details
 * @remark This function it's only useful when creating a BIN_CLASSIC dictionary
 * @author Cristian Martinez (based in a previous version of Sébastien Paumier)
 */
static int create_and_save_inf(const VersatileEncodingConfig* vec,
                               const char* filename,
                               const struct bit_array* used_inf_values,
                               const struct string_hash* inf_codes,
                               int* inf_indirection,
                               int* n_used_inf_codes) {
  // try to create the .inf file
  U_FILE* INF_file = u_fopen(vec, filename, U_WRITE);
  if (INF_file == NULL) {
    error("Could not create %s\n", filename);
    return DEFAULT_ERROR_CODE;
  }

  // First, we print a sequence of zeros at the beginning of the .inf file
  // in order to book some place, so that we can later come and write there
  // the number of lines of this file.
  u_fprintf(INF_file, "0000000000\n");

  // Now we reorder INF codes in order to group the ones that are actually
  // used so that we can save space in the .inf file by not saving codes
  // that are never referenced in the .bin file
  int last = inf_codes->size-1;
  // this -1 initialization is used for safety checking
  for (int i = 0; i < inf_codes->size; ++i) {
    inf_indirection[i]=-1;
  }
  for (int i = 0; i < inf_codes->size && i <= last; ++i) {
    if (get_value(used_inf_values, i)) {
      // a used INF value stays at its place
      (*n_used_inf_codes)++;
      inf_indirection[i] = i;
    } else {
       // We have found an unused INF code. We look for a used one at
       // the end of the array to swap them
      while (last > i && !get_value(used_inf_values, last)) {
        last--;
      }
      if (last == i) {
        // we have finished
        break;
      }
      (*n_used_inf_codes)++;
      // we redirect the old used INF code
      inf_indirection[last]  = i;
      // and we swap codes
      unichar* tmpInfValue   = inf_codes->value[i];
      inf_codes->value[i]    = inf_codes->value[last];
      inf_codes->value[last] = tmpInfValue;
      last--;
    }  // if (get_value(used_inf_values, i))
  }  // for (int i=0;i<INF_codes->size && i<=last;i++)

  // now we can dump the INF codes into the .inf file
  dump_n_values(INF_file, inf_codes, *n_used_inf_codes);
  u_fclose(INF_file);

  // and write the number of INF codes at the beginning
  write_INF_file_header(vec, filename, *n_used_inf_codes);

  return SUCCESS_RETURN_CODE;
}

/**
 * @brief Minimizes and save a DELAF dictionary tree into a classic bin file
 *
 * @param[in] vec encoding I/O Configuration for the \a output_inf file
 * @param[in] bin_filename null-terminated string, with the output .bin filename
 * @param[in] inf_filename null-terminated string, with the output .inf filename
 * @param[in] new_style_bin  set 0: old style, 1: new style (bin > 16Mb)
 * @param[in] inf_codes all the INF codes used by the dictionary tree
 * @param[in] minimize function that minimizes the dictionary tree
 * @param[in,out] root initial state of the dictionary tree
 * @param[out] n_inf_codes total number of inflectional codes used
 * @param[out] n_states total number of states of the automaton
 * @param[out] n_transitions total number of transitions of the automaton
 * @param[out] bin_size  size of the resulting .bin file
 * @return SUCCESS_RETURN_CODE or other status code. See Error.h for details
 * @author Cristian Martinez
 */
static int minimize_and_save_tree_as_bin_classic(
                                    const VersatileEncodingConfig* vec,
                                    const char* bin_filename,
                                    const char* inf_filename,
                                    int new_style_bin,
                                    const struct string_hash* INF_codes,
                                    minimize_func minimize,
                                    struct dictionary_node* root,
                                    int* n_inf_codes,
                                    int* n_states,
                                    int* n_transitions,
                                    int* bin_size,
                                    Abstract_allocator prv_alloc = NULL) {
  // Array to iterate in order through INF_codes using indirect references
  int* inf_indirection = (int*) malloc(sizeof(int) * INF_codes->size);
  if (!inf_indirection) {
    alloc_error("main_Compress");
    return ALLOC_ERROR_CODE;
  }

  // bit array to track INF codes that are actually referenced in the .bin file
  struct bit_array* used_inf_values = new_bit_array(INF_codes->size, ONE_BIT);

  // we build a minimal acyclic automaton
  minimize(root, used_inf_values, prv_alloc);

  // for a classic .bin, we need to create an associated .inf file
  int return_value = create_and_save_inf(
                        vec,               // I/O encoding
                        inf_filename,      // .inf filename
                        used_inf_values,   // INF codes that are actually used
                        INF_codes,         // all the INF codes
                        inf_indirection,   // references to reordered INF codes
                        n_inf_codes);      // number of inflectional codes used

  // continue only if previous functions have succeeded
  if (return_value == SUCCESS_RETURN_CODE) {
    // now, try to dump the minimal transducer into a .bin file
    create_and_save_bin(root,              // automaton initial state
                        bin_filename,      // .bin file name
                        n_states,          // number of states of the automaton
                        n_transitions,     // number of transitions of the automaton
                        bin_size,          // size of the resulting .bin file
                        inf_indirection,   // references to reordered INF codes
                        new_style_bin,     // 0: old style, 1: new style (>16Mb)
                        BIN_CLASSIC);      // a classic .bin type
  }

  free_bit_array(used_inf_values);
  free(inf_indirection);
  return return_value;
}

/**
 * @brief Minimizes and save a DELAF dictionary tree into a bin2 file
 *
 * @param[in] bin_filename null-terminated string, with the output .bin2 filename
 * @param[in] inf_codes all the INF codes used by the dictionary tree
 * @param[in] minimize function that minimizes the dictionary tree
 * @param[in,out] root initial state of the dictionary tree
 * @param[out] n_inf_codes total number of inflectional codes used
 * @param[out] n_states total number of states of the automaton
 * @param[out] n_transitions total number of transitions of the automaton
 * @param[out] bin_size  size of the resulting .bin file
 * @return SUCCESS_RETURN_CODE or other status code. See Error.h for details
 * @author Cristian Martinez
 */
static int minimize_and_save_tree_as_bin_two(
                                   const char* bin_filename,
                                   struct string_hash* INF_codes,
                                   minimize_func minimize,
                                   struct dictionary_node* root,
                                   int* n_states,
                                   int* n_transitions,
                                   int* bin_size,
                                   Abstract_allocator prv_alloc = NULL) {
  // for a .bin2 dictionary, we need to place first the inf codes on
  // the transitions outputs
  move_outputs_on_transitions(root, INF_codes);

  // bit array to track INF codes that are actually referenced in the .bin file
  struct bit_array* used_inf_values = new_bit_array(INF_codes->size, ONE_BIT);

  // we build a minimal acyclic automaton
  minimize(root, used_inf_values, prv_alloc);

  // now, try to dump the minimal transducer into a .bin2 file
  create_and_save_bin(root,              // automaton initial state
                      bin_filename,      // .bin file name
                      n_states,          // number of states of the automaton
                      n_transitions,     // number of transitions of the automaton
                      bin_size,          // size of the resulting .bin file
                      NULL,              // bin2 no use references to INF codes
                      1,                 // always use the new dictionary style
                      BIN_BIN2);         // a .bin2 dictionary type

  free_bit_array(used_inf_values);

  return SUCCESS_RETURN_CODE;
}


int pseudo_main_Compress(const VersatileEncodingConfig* vec,
                         int flip, int semitic, char* dic, int new_style_bin) {
ProgramInvoker* invoker = new_ProgramInvoker(main_Compress, "main_Compress");
char tmp[200];
tmp[0] = 0;
get_reading_encoding_text(tmp, sizeof(tmp)-1, vec->mask_encoding_compatibility_input);
if (tmp[0] != '\0') {
    add_argument(invoker, "-k");
    add_argument(invoker, tmp);
}
tmp[0] = 0;
get_writing_encoding_text(tmp, sizeof(tmp)-1, vec->encoding_output, vec->bom_output);
if (tmp[0] != '\0') {
  add_argument(invoker, "-q");
  add_argument(invoker, tmp);
}
if (flip) {
  add_argument(invoker, "-f");
}
if (semitic) {
  add_argument(invoker, "-s");
}
if (new_style_bin) {
  add_argument(invoker, "--v2");
} else {
  add_argument(invoker, "--v1");
}

add_argument(invoker, dic);
int ret = invoke(invoker);
free_ProgramInvoker(invoker);
return ret;
}

/**
 * This program reads a .dic file and compress it into a .bin and a .inf file.
 * First, it builds a tree with all the entries, and then, it builds a minimal
 * transducer from this tree, using the Dominique Revuz's algorithm.
 */
int main_Compress(int argc, char* const argv[]) {
if (argc == 1) {
  usage();
  return SUCCESS_RETURN_CODE;
}

// reserve some heap memory to manipulate string buffers
char* buffer_filename   = (char*) malloc(step_filename_buffer * 5);
if (buffer_filename == NULL) {
  alloc_error("main_Compress");
  return ALLOC_ERROR_CODE;
}

// name of the destination file, e.g. foo.dic produces foo.bin
char* bin_filename      = (buffer_filename + (step_filename_buffer * 0));
*bin_filename           = '\0';

// name of the file where the packed inflectional codes will be stored
char* inf_filename      = (buffer_filename + (step_filename_buffer * 1));
*inf_filename           = '\0';

// name of the file where the packed inflectional codes will be stored packed
char* inp_filename      = (buffer_filename + (step_filename_buffer * 2));
*inp_filename           = '\0';

// real path of the dictionary filename that is being processed
char* resolved_filename = (buffer_filename + (step_filename_buffer * 3));
*resolved_filename      = '\0';

// output_type: "bin1" or "bin2"
char* output_type       = (buffer_filename + (step_filename_buffer * 4));
*output_type            = '\0';

// specifies if the inflected and lemma forms must be swapped 0:no 1:yes
int FLIP = 0;

//  BIN_CLASSIC  .bin/.inf dictionary
//  BIN_BIN2     .bin2 dictionary style, with outputs included in the transducer
BinType bin_type        = BIN_CLASSIC;

// pack_inp = 1 : convert inf to inp
int pack_inp = 0;

//  0 : produces an old style .bin file
//  1 : produces a new style  .bin file, with no file size limitation to 16Mb
int new_style_bin       = 1;

// specifies if the semitic compression algorithm will be used
int semitic             = 0;

// describes the encoding configuration for I/O
VersatileEncodingConfig vec = VEC_DEFAULT;

// parse command line
int val, index = -1;
bool only_verify_arguments = false;
UnitexGetOpt options;
while (EOF != (val = options.parse_long(argc, argv, optstring_Compress, lopts_Compress, &index))) {
  switch (val) {
    case 'f': FLIP    = 1; break;
    case 'p': pack_inp = 1; break;
    case 's': semitic = 1; break;
    case  1 : // this is according to the "Standards for Command Line Interfaces"
              // https://goo.gl/7UgLC8
              u_printf("Compress (Unitex) %s\n",get_unitex_semver_string());
              return SUCCESS_RETURN_CODE;
    case  2 : new_style_bin = 1; bin_type = BIN_BIN2;    break;
    case  3 : new_style_bin = 0; bin_type = BIN_CLASSIC; break;
    case  4 : new_style_bin = 1; bin_type = BIN_CLASSIC; break;
    case 'V': only_verify_arguments = true;
              break;
    case 'h': usage();
              free(buffer_filename);
              return SUCCESS_RETURN_CODE;
    case 't': if (options.vars()->optarg[0] == '\0') {
                error("You must specify a non empty argument for --output_type\n");
                free(buffer_filename);
                return USAGE_ERROR_CODE;
              }
              strcpy(output_type, options.vars()->optarg);
              break;
    case 'o': if (options.vars()->optarg[0] == '\0') {
                error("You must specify a non empty output\n");
                free(buffer_filename);
                return USAGE_ERROR_CODE;
              }
              strcpy(bin_filename, options.vars()->optarg);
              break;
    case 'k': if (options.vars()->optarg[0] == '\0') {
                error("Empty input_encoding argument\n");
                free(buffer_filename);
                return USAGE_ERROR_CODE;
              }
              decode_reading_encoding_parameter(
                  &(vec.mask_encoding_compatibility_input),
                  options.vars()->optarg);
              break;
    case 'q': if (options.vars()->optarg[0] == '\0') {
                error("Empty output_encoding argument\n");
                free(buffer_filename);
                return USAGE_ERROR_CODE;
              }
              decode_writing_encoding_parameter(
                  &(vec.encoding_output),
                  &(vec.bom_output),
                  options.vars()->optarg);
              break;
    case ':': index == -1 ?
               error("Missing argument for option -%c\n",  options.vars()->optopt) :
               error("Missing argument for option --%s\n", lopts_Compress[index].name);
              free(buffer_filename);
              return USAGE_ERROR_CODE;
    case '?': index == -1 ? error("Invalid option -%c\n", options.vars()->optopt) :
                            error("Invalid option --%s\n", options.vars()->optarg);
              free(buffer_filename);
              return USAGE_ERROR_CODE;
  }
  index = -1;
}

// invalid arguments
if (options.vars()->optind == argc) {
  error("Invalid arguments: rerun with --help\n");
  free(buffer_filename);
  return USAGE_ERROR_CODE;
}

// an output filename is mandatory when compressing more than one file
if (options.vars()->optind != argc-1 && bin_filename[0] == '\0') {
  error("You must use the -o option when there are more than one .dic\n");
  free(buffer_filename);
  return USAGE_ERROR_CODE;
}

// check if --output_type was passed an equal to bin1 or bin2
if (output_type[0] != '\0') {
  if      (strcmp(output_type,"bin1") == 0) {
    new_style_bin = 1;
    bin_type = BIN_CLASSIC;
  }
  else if (strcmp(output_type,"bin2") == 0) {
    new_style_bin = 1;
    bin_type = BIN_BIN2;
  }
  else {
    error("%s: Invalid option for --output_type, must be 'bin1' or 'bin2'\n", output_type);
    return USAGE_ERROR_CODE;
  }
}

// returns here if we're only verifying the arguments syntax
if (only_verify_arguments) {
  // freeing all allocated memory
  free(buffer_filename);
  return SUCCESS_RETURN_CODE;
}

// If the output .bin name was not passed as an argument,
// we compute a default filename for it
if (bin_filename[0] == '\0') {
  strcpy(bin_filename, argv[options.vars()->optind]);
  remove_extension(bin_filename);
  switch (bin_type) {
    case BIN_CLASSIC: strcat(bin_filename, ".bin");  break;
    case BIN_BIN2:    strcat(bin_filename, ".bin2"); break;
  }
}

// compute the name of the output .inf file associated to a classic .bin
remove_extension(bin_filename, inf_filename);
strcat(inf_filename, ".inf");

remove_extension(bin_filename, inp_filename);
strcat(inp_filename, ".inp");

// list to store the filenames to process
list_ustring_ptr dictionary_list = NULL;

// insert in the list all filenames that were passed as argument
for (; options.vars()->optind != argc; (options.vars()->optind)++) {
  // to prevent including a file repeatedly, we only stored resolved filenames
  // @see get_real_path for more information
  if (get_real_path(argv[options.vars()->optind],
                   resolved_filename) == SUCCESS_RETURN_CODE) {
    // only include in the list if it is not already present
    if (!is_in_list(resolved_filename, dictionary_list, u_strcmp_ignore_case)) {
      dictionary_list = new_list_ustring(resolved_filename,
                                         dictionary_list);
    // warn about a possible duplicate inclusion
    } else {
      error("The specified filename occurred more than once in the list "
            "of input dictionaries\n%s\n",
            resolved_filename);
    }
  // if get_real_path fails, file cannot be opened
  } else {
    error("Cannot open %s\n", argv[options.vars()->optind]);
  }  // get_real_path(argv[options.vars()->optind]
}  // for (;options.vars()->optind!=argc;(options.vars()->optind)++)

// only continue if there are at least one file to process
if(!dictionary_list) {
  error("There are no files to process\n");
  free(buffer_filename);
  return DEFAULT_ERROR_CODE;
}

Abstract_allocator compress_abstract_allocator =
    create_abstract_allocator("main_Compress",
    AllocatorCreationFlagAutoFreePrefered);

Abstract_allocator compress_tokenize_abstract_allocator =
    create_abstract_allocator("main_Compress_tokenize_first",
    AllocatorCreationFlagAutoFreePrefered |
    AllocatorCreationFlagCleanPrefered);

// root of the dictionary tree
struct dictionary_node* root  = new_dictionary_node(compress_abstract_allocator);

// structure that will contain all the INF codes
struct string_hash* INF_codes = new_string_hash();

int return_value  = SUCCESS_RETURN_CODE; // default return code
int n_entries     = 0;                   // number of entries processed
int n_lines       = 0;                   // number of lines scanned
int n_line_errors = 0;                   // number of line errors
int n_files       = 0;                   // number of files read
int n_inf_codes   = 0;                   // number of inflectional codes used
int n_states      = 0;                   // number of states of the automaton
int n_transitions = 0;                   // number of transitions of the automaton
int bin_size      = 0;                   // size of the resulting .bin file

// build a tree representation of all the DELAF entries pointed
// by the files in the dictionary list
return_value = build_tree_from_dictionary_list(
                       &vec,             // I/O encoding
                       FLIP,             // inflected and lemma must be swapped
                       semitic,          // semitic compression algorithm
                       dictionary_list,  // dictionaries filenames
                       root,             // automaton initial state
                       INF_codes,        // all the INF codes
                       &n_entries,       // number of entries processed
                       &n_lines,         // number of lines scanned
                       &n_line_errors,   // number of line errors
                       &n_files,         // number of files read
                       compress_abstract_allocator,
                       compress_tokenize_abstract_allocator);

// minimize and save the tree in a binary file always that there are
// at least one entry to process
if (return_value == SUCCESS_RETURN_CODE) {
  switch (bin_type) {
    // classical .bin file
    case BIN_CLASSIC:
      return_value = minimize_and_save_tree_as_bin_classic(
                       &vec,             // .inf I/O encoding
                       bin_filename,     // output .bin filename
                       inf_filename,     // output .inf filename
                       new_style_bin,    // 0: old style, 1: new style (>16Mb)
                       INF_codes,        // all the INF codes
                       minimize_tree,    // function to construct a minimal ADFA
                       root,             // automaton initial state
                       &n_inf_codes,     // inflectional codes used
                       &n_states,        // states of the automaton
                       &n_transitions,   // transitions of the automaton
                       &bin_size,        // size of the resulting .bin file
                       compress_abstract_allocator);
      break;
    // .bin2 style, with outputs included in the transducer
    case BIN_BIN2:
      return_value = minimize_and_save_tree_as_bin_two(
                       bin_filename,     // output .bin filename
                       INF_codes,        // all the INF codes
                       minimize_tree,    // function to construct a minimal ADFA
                       root,             // automaton initial state
                       &n_states,        // states of the automaton
                       &n_transitions,   // transitions of the automaton
                       &bin_size,        // size of the resulting .bin file
                       compress_abstract_allocator);
      break;
  }
}

// finally, print some stats
if (return_value == SUCCESS_RETURN_CODE) {
  u_printf("Binary file: %d bytes\n",     bin_size);
  u_printf("%d file%s read\n",            n_files,       (n_files         > 1)? "s"   :  "");
  u_printf("%d line%s scanned\n",         n_lines,       (n_lines         > 1)? "s"   :  "");
  if(n_line_errors > 0 ) {
    u_printf("%d line%s with errors\n",   n_line_errors, (n_line_errors   > 1)? "s"   :  "");
  }
  u_printf("%d entr%s processed\n",       n_entries,     (n_entries       > 1)? "ies" : "y");
  if (bin_type == BIN_CLASSIC) {
    u_printf("%d INF entr%s created\n",   n_inf_codes,   (n_inf_codes     > 1)? "ies" : "y");
  }
  u_printf("%d states, %d transitions\n", n_states,       n_transitions);
} else {
  error("Compress terminated with errors.\n");
}

/*
 * WARNING: we do not free the 'INF_codes' structure because of a slowness
 *          problem with very large INF lines.
 */
# if (defined(UNITEX_LIBRARY) || defined(UNITEX_RELEASE_MEMORY_AT_EXIT))
// cleanup to avoid leaks when using as library
free_dictionary_node(root, compress_abstract_allocator);
close_abstract_allocator(compress_abstract_allocator);
close_abstract_allocator(compress_tokenize_abstract_allocator);
free_string_hash(INF_codes);
# endif

free_list_ustring(dictionary_list);
if (pack_inp && (bin_type!=BIN_BIN2) && (return_value==SUCCESS_RETURN_CODE)) {
  if (!convert_inf_to_inp_pack_file(inf_filename, inp_filename)) {
    return_value = DEFAULT_ERROR_CODE;
  }
  af_remove(inf_filename);
}
free(buffer_filename);
return return_value;
}  // int main_Compress(int argc, char* const argv[])

}  // namespace unitex
