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

/*
 * File created and contributed by Gilles Vollant (Ergonotics SAS)
 * as part of an UNITEX optimization and reliability effort
 *
 * additional information: http://www.smartversion.com/unitex-contribution/
 * contact : info@winimage.com
 *
 */

#if ((!(defined(NO_UNITEX_LOGGER))) && (!(defined(NO_UNITEX_RUNLOGGER_AUTOINSTALL))))

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


#include "Unicode.h"
#include "Copyright.h"
#include "UnitexGetOpt.h"

#include "ActivityLoggerPlugCallback.h"
#include "File.h"

#include "FilePack.h"
#include "FileUnPack.h"
#include "FilePackCrc32.h"

#include "DirHelper.h"
#include "SyncTool.h"
#include "SyncLogger.h"

#include "RunTools.h"
#include "ReworkArg.h"

#include "Error.h"
#include "UnusedParameter.h"
#include "AbstractFilePlugCallback.h"
#include "UserCancellingPlugCallback.h"
#include "RunTools.h"
#include "UnpackFileTool.h"
#include "UniRunScript.h"
#include "LingResourcePackage.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
/*
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {
*/
using namespace ::unitex::logger;

static void freeScriptFile(char** list);

static char** build_empty_users_variables() {
  char ** users_variables = (char**)malloc(sizeof(char*) * 2);
  if (users_variables == NULL) {
    alloc_error("build_empty_users_variables");
    return NULL;
  }
  *users_variables = *(users_variables + 1) = NULL;

  return users_variables;
}


static void free_users_variable(char** users_variables) {
  int i;
  if (users_variables == NULL) {
    return;
  }

  i = 0;
  while ((*(users_variables + i)) != NULL) {
    free(*(users_variables + i));
    i++;
  }
  free(users_variables);
}


static char** clone_users_variable(char** users_variables) {
  int nb_items = 0;
  if (users_variables != NULL) {
    while ((*(users_variables + nb_items)) != NULL) {
      nb_items++;
    }
  }

  char** cloned_users_variable = (char**)malloc(sizeof(char*) * (nb_items + 3));
  if (cloned_users_variable == NULL) {
    alloc_error("clone_users_variable");
    return NULL;
  }

  for (int i = 0; i < nb_items; i++) {
    *(cloned_users_variable + i) = strdup(*(users_variables + i));
    if ((*(cloned_users_variable + i)) == NULL){
      alloc_error("clone_users_variable");
      free_users_variable(cloned_users_variable);
      return NULL;
    }
  }
  *(cloned_users_variable + nb_items) = NULL;
  *(cloned_users_variable + nb_items + 1) = NULL;
  *(cloned_users_variable + nb_items + 2) = NULL;

  return cloned_users_variable;
}

static const char* search_value_for_variable(char** variable_list,const char* var_name, size_t len_search_variable = 0)
{
  if (var_name == NULL)
    return NULL;
  if (len_search_variable == 0)
    len_search_variable = strlen(var_name);

  int j = 0;

  for (;;)
  {
    if (*(variable_list + j) == NULL)
      break;
    size_t len_var_in_list = strlen(*(variable_list + j));
    if (len_var_in_list == len_search_variable)
    {
      if (len_var_in_list == len_search_variable)
      {
        if (memcmp(*(variable_list + j), var_name, len_var_in_list) == 0)
        {
          return *(variable_list + j + 1);
        }
      }
    }
    j += 2;
  }
  return NULL;
}


static char** insert_variable_and_value(char** variable_list, const char* var_name, const char* value) {
  if (var_name == NULL) {
    return variable_list;
  }

  if (value == NULL) {
    value = "";
  }

  int j = 0;

  for (;;) {
    if (*(variable_list + j) == NULL) {
      break;
    }
    if (strcmp(*(variable_list + j), var_name) == 0) {
      free(*(variable_list + j + 1));
      *(variable_list + j + 1) = strdup(value);
      if ((*(variable_list + j + 1)) == NULL) {
        alloc_error("insert_variable_and_value");
        return NULL;
      }
      return variable_list;
    }
    j += 2;
  }

  char** new_variable_list = (char**)realloc(variable_list, sizeof(char*)*(j + 4));
  if (new_variable_list == NULL) {
    alloc_error("insert_variable_and_value");
    return NULL;
  }

  *(new_variable_list + j) = strdup(var_name);
  *(new_variable_list + j + 1) = strdup(value);
  if (((*(new_variable_list + j)) == NULL) || ((*(new_variable_list + j + 1)) == NULL)) {
    alloc_error("insert_variable_and_value");
    free_users_variable(new_variable_list);
    return NULL;
  }

  *(new_variable_list + j + 2) = *(new_variable_list + j + 3) = NULL;

  return new_variable_list;
}

static void addTextInDynamicString(char** dyn_string, size_t *pos, size_t *allocated, size_t buffer_margin, const char* add_str, int limit_size=-1) {
  if (add_str == NULL) {
    add_str = "";
  }
  size_t len_add = strlen(add_str);
  if ((limit_size != -1) && (limit_size < ((int)len_add)))
    len_add = (size_t)limit_size;
  size_t needed_size = (*pos) + len_add + buffer_margin + 1;
  if (needed_size >= (*allocated)) {
    size_t new_allocated_size = 1;
    while (new_allocated_size <= needed_size) {
      new_allocated_size *= 2;
    }

    char* new_string_buffer = ((*dyn_string) == NULL) ? ((char*)malloc(new_allocated_size)) : ((char*)realloc(*dyn_string, new_allocated_size));
    if (new_string_buffer == NULL) {
      alloc_error("addTextInDynamicString");
      return;
    }
    *dyn_string = new_string_buffer;
  }
  memcpy((*dyn_string) + (*pos), add_str, len_add + 1);
  (*pos) += len_add;
}


// char** subsitution contain on even entry variable name and on odd entry variable content
static char* substitueInLine(const char* line, char*** variable_list, int assignement_allowed)
{
  size_t len_line = strlen(line);
  char* build_string = NULL;
  size_t pos = 0;
  size_t allocated = 0;
  // allocated build_string with at last line size
  addTextInDynamicString(&build_string, &pos, &allocated, len_line, NULL);

  if (assignement_allowed)
  {
    const char* browse = line;
    const char* special_start_var_name = NULL;
    const char* special_possible_end_var_name = NULL;
    while (((*browse) == ' ') || ((*browse) == '\t'))
      browse++;
    if ((*browse) != '\0')
    {
      special_start_var_name = browse;
      while (((*browse) != ' ') && ((*browse) != '\t') && ((*browse) != '\0') && ((*browse) != '='))
        browse++;
      special_possible_end_var_name = browse;
      while (((*browse) == ' ') || ((*browse) == '\t'))
        browse++;
      if (((*browse) == '=') && (special_possible_end_var_name != special_start_var_name))
      {
        char* assign_var_name = (char*)malloc((special_possible_end_var_name - special_start_var_name) + 1);
        if (assign_var_name == NULL) {
          alloc_error("substitueInLine");
          return NULL;
        }
        memcpy(assign_var_name, special_start_var_name, special_possible_end_var_name - special_start_var_name);
        *(assign_var_name + (special_possible_end_var_name - special_start_var_name)) = '\0';

        browse++;

        while (((*browse) == ' ') || ((*browse) == '\t'))
          browse++;

        char* assign_value = substitueInLine(browse, variable_list, 0);
        //u_printf("assign '%s' value '%s'\n", assign_var_name, assign_value);
        *variable_list = insert_variable_and_value(*variable_list, assign_var_name, assign_value);
        free(assign_var_name);
        free(assign_value);
        return build_string;
      }
    }
  }

  for (size_t i = 0; i < len_line; i++)
  {
    char c = *(line + i);
    int is_variable = 0;
    if (c == '{')
    {
      const char* end_var = strchr(line + i + 1, '}');
      if (end_var != NULL)
      {
        int skip_path_separator = 0;
        int skip_extension = 0;
        size_t len_search_variable = (size_t)(end_var - (line + i + 1));
        size_t skip_prefix_size = 0;

        // if we have {/VARIABLE_NAME}, we remove the path component of a pathname
        // if we have {.VARIABLE_NAME}, we remove the extension component of a pathname
        for (;;) {
          if (*(line + skip_prefix_size + i + 1) == '/') {
            skip_prefix_size ++;
            skip_path_separator = 1;
          } else
          if (*(line + skip_prefix_size + i + 1) == '.') {
            skip_prefix_size ++;
            skip_extension = 1;
          } else
          {
            break;
          }
        }

        const char* value_variable = NULL;
        if (len_search_variable != 0)
          value_variable = search_value_for_variable(*variable_list, line + i + 1 + skip_prefix_size, len_search_variable - skip_prefix_size);
        if ((value_variable != NULL) && (skip_path_separator))
        {
          const char* browse_variable_content = value_variable;
          for (;;) {
            char c2 = *browse_variable_content;
            if (c2 == '\0')
              break;
            if ((c2 == '/') || (c2 == '\\'))
              value_variable = browse_variable_content + 1;
            browse_variable_content++;
          }
        }

        int limit_size = -1;
        if ((value_variable != NULL) && (skip_extension))
        {
          int browse_value_pos=0;
          for (;;)
          {
            char c2 = (*(value_variable + browse_value_pos));
            if (c2 == '\0')
              break;
            if (c2 == '.') {
              limit_size = browse_value_pos;
              break;
            }
            browse_value_pos++;
          }
        }

        if (value_variable != NULL)
        {
          addTextInDynamicString(&build_string, &pos, &allocated, len_line, value_variable, limit_size);
          is_variable = 1;
          i = (end_var - line);
        }
      }
    }

    if (!is_variable)
    {
      *(build_string + pos) = c;
      pos++;
    }
  }

  *(build_string + pos) = '\0';
  return build_string;
}

static char* ConvertUnicharToUtf8(const unichar* line, int len_line)
{
    int pos = 0;
    int utf8_size = 0;
    for (;;)
    {
        if ((len_line != -1) && (pos >= len_line))
            break;
        unichar c = *(line + pos);
        if (c == 0) break;
        if (c <= 0x7F) utf8_size += 1;
        else if (c <= 0x7FF) utf8_size += 2;
        else utf8_size += 3;

    pos++;
    }

    char* str = (char*)malloc(utf8_size + 1);
    if (str == NULL) {
        alloc_error("ConvertToUtf8");
        return NULL;
    }

    unsigned char* browse_str = (unsigned char*)str;
    pos = 0;
    for (;;) {
        if ((len_line != -1) && (pos >= len_line))
            break;
        unichar c = *(line + pos);
        if (c == 0) break;
        if (c<=0x7F) *(browse_str++) = (unsigned char)c;
        else if (c <= 0x7FF) {
            *(browse_str++) = (unsigned char)(0xC0 | (c >> 6));
            *(browse_str++) = (unsigned char)(0x80 | (c & 0x3F));
        }
        else {
            *(browse_str++) = (unsigned char)(0xE0 | (c >> 12));
            *(browse_str++) = (unsigned char)(0x80 | ((c >> 6) & 0x3F));
            *(browse_str++) = (unsigned char)(0x80 | (c & 0x3F));
        }
    pos++;
    }
  *(browse_str) = '\0';
    return str;
}





static unichar* load_file_in_memory(const char* filename, VersatileEncodingConfig *vec) {
    unichar *line = NULL;
    size_t size_buffer_line = 0;
    unichar* content;
    size_t size_content_filled = 0;

    content = (unichar*)malloc(sizeof(unichar));
    if (content == NULL) {
        alloc_error("load_file_in_memory");
        return NULL;
    }
    *content = 0;
    U_FILE *uread = u_fopen(vec, filename, U_READ);
    if (uread==NULL)    {
        free(content);
        return NULL;
    }
    if (uread != NULL) {
        while (u_fgets_dynamic_buffer(&line, &size_buffer_line, uread) != EOF) {
            size_t new_line_size = u_strlen(line);
            unichar* new_content = (unichar*)realloc(content, (size_content_filled + new_line_size + 1) * sizeof(unichar));
            if (content == NULL) {
                free(content);
                u_fclose(uread);
                if (line != NULL)
                    free(line);
                return NULL;
            }
            content = new_content;
            u_strcpy(new_content + size_content_filled, line);
            size_content_filled += new_line_size;
        }
        if (line != NULL)
            free(line);
        u_fclose(uread);
    }

    return content;
}


static char* load_file_in_utf8_memory(const char* filename, VersatileEncodingConfig *vec) {
    unichar* unicontent = load_file_in_memory(filename, vec);
    if (unicontent == NULL) {
        return NULL;
    }
    char* utf8content = ConvertUnicharToUtf8(unicontent, -1);
    free(unicontent);
    return utf8content;
}


static char** ReadScriptFile(const VersatileEncodingConfig* vec, const char* scriptFileName, char** users_variables_param, int transform_path_separator, int transform_path_separator_content) {
  char ** Lines = (char**)malloc(sizeof(char*));
  if (Lines == NULL) {
    alloc_error("ReadScriptFile");
    return NULL;
  }
  *Lines = NULL;
  int nbLines = 0;

  char** users_variables = clone_users_variable(users_variables_param);

  U_FILE* dest_read_script = u_fopen(vec, scriptFileName, U_READ);
  if (dest_read_script == NULL) {
    char* transformed_scriptFileName = strdup(scriptFileName);
    if (transformed_scriptFileName == NULL) {
      alloc_error("ReadScriptFile");
      free_users_variable(users_variables);
      freeScriptFile(Lines);
      return NULL;
    }

    transform_fileName_separator(transformed_scriptFileName, transform_path_separator);
    dest_read_script = u_fopen(vec, transformed_scriptFileName, U_READ);
    free(transformed_scriptFileName);

  }
  if (dest_read_script == NULL) {
    error("Cannot open file %s\n", scriptFileName);
    free_users_variable(users_variables);
    freeScriptFile(Lines);
    return NULL;
  }

  unichar* line = NULL;
  size_t size_buffer_line = 0;
  while (u_fgets_dynamic_buffer(&line, &size_buffer_line, dest_read_script) != EOF){
    size_t len_line = u_strlen(line);
    for (;;)
    {
      if (len_line == 0)
        break;
      if ((*(line + len_line - 1) != '\t') && (*(line + len_line - 1) != ' '))
        break;
      len_line -- ;
    }

    char* charline = ConvertUnicharToUtf8(line, (int)(len_line));

    if (charline == NULL) {
      free(line);
      u_fclose(dest_read_script);
      free_users_variable(users_variables);
      freeScriptFile(Lines);
      return NULL;
    }

    char* reworked = substitueInLine(charline, &users_variables, 1);
    if (transform_path_separator_content) {
        transform_fileName_separator(reworked, transform_path_separator);
    }

    char** more_Lines = (char**)realloc(Lines, sizeof(char*) * (nbLines + 2));
    if (more_Lines != NULL) {
        Lines = more_Lines;
    } else {
     alloc_error("ReadScriptFile");
     free(charline);
     free(line);
     u_fclose(dest_read_script);
     free_users_variable(users_variables);
     freeScriptFile(Lines);
     return NULL;
    }
    *(Lines + nbLines) = reworked;
    *(Lines + nbLines + 1) = NULL;
    nbLines ++ ;
    free(charline);
  }

  free(line);
  u_fclose(dest_read_script);
  free_users_variable(users_variables);
  return Lines;
}


static void freeScriptFile(char** list) {
  unsigned int i = 0;
  if (list == NULL)
    return;

  while ((*(list + i)) != NULL)
  {
    free(*(list + i));
    i++;
  }
  free(list);
}


int run_scriptfile(const VersatileEncodingConfig* vec, const char*scriptFileName, char**users_variables, int do_run, int transform_path_separator_content, const char*output_translated_script_filename, int verbose) {
  char** lines = ReadScriptFile(vec, scriptFileName, users_variables, UNPACKFILE_LIST_FOLDER_SEPARATOR_TRANSFORMATION_PLATFORM, transform_path_separator_content);

  if (lines == NULL) {
      error("Error in reading script file %s\n",scriptFileName);
      return DEFAULT_ERROR_CODE;
  }

  ABSTRACTFILE* fOut = NULL;
  if ((output_translated_script_filename != NULL) && ((*output_translated_script_filename) != '\0')) {
    fOut = af_fopen(output_translated_script_filename, "wb");
  }
  if (fOut != NULL) {
    int i = 0;
    for (;;)
    {
      const char* curLine = *(lines + i);
      if (curLine == NULL)
        break;
      af_fwrite(curLine, strlen(curLine), 1, fOut);
      af_fwrite("\n", 1, 1, fOut);
      i++;
    }
    af_fclose(fOut);
  }

  if (do_run) {
    int i = 0;
    for (;;)
    {
      const char* curLine = *(lines + i);
      if (curLine == NULL)
        break;

      while (((*curLine) == '\t') || ((*curLine) == ' ')) curLine++;

      if (((*curLine) != '#') && ((*curLine) != '\0'))
      {
        int argc = 0;
        char** argv = do_convert_command_line_synth_to_std_arg(curLine, strlen(curLine), &argc);

        if (argc > 1)
        {
          if (verbose)
            u_printf("Running: %s\n", curLine);

          //int ret_tool = main_UnitexTool_C_internal(argc, (char**)argv);
          int ret_tool = 0;
          int command_found = -1;
          run_command_direct(argc - 1, argv + 1, &command_found, &ret_tool);

          if (verbose && (!command_found))
            u_printf("Command %s not found\n", argv[0]);

          if (verbose)
            u_printf("Running result: %d\n\n", ret_tool);
        }

        free_std_arg_converted(argv);
      }
      i++;
    }
  }

  freeScriptFile(lines);

  return SUCCESS_RETURN_CODE;
}

const char* usage_UniRunScript =
"Usage : RunScript [OPTIONS] <scriptfile>\n"
"\n"
"  <scriptfile>: a script file to run\n"
"\n"
"OPTIONS:\n"
"  -a X/--variable=X=Y: uses X as variable with Y content\n"
"  -c X/--variable_file_content=X=Y: uses X as variable with reading file Y content\n"
"  -o X/--output_script=X: dump the script content after replacing variable to file\n"
"  -T/--no_translate_separator: don't translate path separator in variable content\n"
"  -R/--no_run: don't run the script\n"
"  -v/--verbose: emit message when running\n"
"  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
"  -h/--help: this help\n"
"\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_UniRunScript);
}


const char* optstring_UniRunScript = ":VTRhk:q:o:c:a:v";
const struct option_TS lopts_UniRunScript[] = {
  { "variable", required_argument_TS, NULL, 'a' },
  { "variable_file_content", required_argument_TS, NULL, 'c' },
  { "no_translate_separator", no_argument_TS, NULL, 'T' },
  { "no_run", no_argument_TS, NULL, 'R' },
  { "verbose", no_argument_TS, NULL, 'v' },
  { "input_encoding", required_argument_TS, NULL, 'k' },
  { "output_encoding", required_argument_TS, NULL, 'q' },
  { "only_verify_arguments",no_argument_TS,NULL,'V'},
  { "help", no_argument_TS, NULL, 'h' },
  { NULL, no_argument_TS, NULL, 0 }
};

int main_UniRunScript(int argc, char* const argv[]) {
  if (argc == 1) {
    usage();
    return SUCCESS_RETURN_CODE;
  }

  int verbose = 0;
  VersatileEncodingConfig vec = VEC_DEFAULT;
  int val, index = -1;
  int translate_separator_variable = 1;
  int do_run = 1;
  char unique_string[UNIQUE_STRING_FOR_POINTER_MAX_SIZE];
  char output_translated_script_filename[0x400] = "";
  fill_unique_string_for_pointer((const void*)&verbose, unique_string);
  char** users_variables = build_empty_users_variables();
  users_variables = insert_variable_and_value(users_variables, "UNIQUE_VALUE", unique_string);
  bool only_verify_arguments = false;
  UnitexGetOpt options;
  while (EOF != (val=options.parse_long(argc, argv, optstring_UniRunScript, lopts_UniRunScript, &index))) {
    switch (val) {
    case 'v': verbose = 1;
              break;
    case 'a':
    case 'c':
    {
      if (options.vars()->optarg[0] == '\0') {
        error("You must specify a non empty variable\n");
        free_users_variable(users_variables);
        return USAGE_ERROR_CODE;
      }

      char* rework_var_assign = strdup(options.vars()->optarg);
      if (rework_var_assign == NULL) {
        alloc_error("main_RunScript");
        free_users_variable(users_variables);
        return ALLOC_ERROR_CODE;
      }

      char* new_var_name = rework_var_assign;

      char * pos_equal = strchr(rework_var_assign, '=');
      const char * new_var_param = "";
      if (pos_equal != NULL)
      {
        new_var_param = pos_equal + 1;
        *pos_equal = '\0';
      }

      if (val == 'a') {
        users_variables = insert_variable_and_value(users_variables, new_var_name, new_var_param);
      }
      else {
        char* new_var_content = load_file_in_utf8_memory(new_var_param, &vec);
        if (new_var_content == NULL) {
          error("Error getting file %s content\n", new_var_param);
          free_users_variable(users_variables);
          return DEFAULT_ERROR_CODE;
        }
        users_variables = insert_variable_and_value(users_variables, new_var_name, new_var_content);
        free(new_var_content);
      }

      free(rework_var_assign);

      break;
    }

        case 'o': if (options.vars()->optarg[0] == '\0') {
                        error("You must specify a non empty output filename\n");
                        free_users_variable(users_variables);
                        return USAGE_ERROR_CODE;
                    }
                  strcpy(output_translated_script_filename, options.vars()->optarg);
                  break;



        case 'T': translate_separator_variable = 0;
            break;

        case 'R': do_run = 0;
            break;
    case 'k': if (options.vars()->optarg[0] == '\0') {
                error("Empty input_encoding argument\n");
                free_users_variable(users_variables);
                return USAGE_ERROR_CODE;
              }
          decode_reading_encoding_parameter(&vec.mask_encoding_compatibility_input, options.vars()->optarg);
          break;
    case 'q': if (options.vars()->optarg[0] == '\0') {
                error("Empty output_encoding argument\n");
                free_users_variable(users_variables);
                return USAGE_ERROR_CODE;
              }
          decode_writing_encoding_parameter(&vec.encoding_output, &vec.bom_output, options.vars()->optarg);
          break;
   case 'V': only_verify_arguments = true;
          break;
    case 'h': usage();
          free_users_variable(users_variables);
          return SUCCESS_RETURN_CODE;
    case ':': index == -1 ? error("Missing argument for option -%c\n", options.vars()->optopt) :
                            error("Missing argument for option --%s\n", lopts_UniRunScript[index].name);
          free_users_variable(users_variables);
          return USAGE_ERROR_CODE;
    case '?': index == -1 ? error("Invalid option -%c\n", options.vars()->optopt) :
                            error("Invalid option --%s\n", options.vars()->optarg);
          free_users_variable(users_variables);
          return USAGE_ERROR_CODE;
    }
    index = -1;
  }

  if (options.vars()->optind != argc - 1) {
    error("Invalid arguments: rerun with --help\n");
    free_users_variable(users_variables);
    return USAGE_ERROR_CODE;
  }

  if (only_verify_arguments) {
    // freeing all allocated memory
    free_users_variable(users_variables);
    return SUCCESS_RETURN_CODE;
  }

  const char* scriptFile = argv[options.vars()->optind];

  int retvalue = run_scriptfile(&vec, scriptFile, users_variables, do_run, translate_separator_variable, output_translated_script_filename, verbose);

  free_users_variable(users_variables);
  return retvalue;
}



/***************************************************************************/


typedef struct
{
    char* src_filename;
    char* dest_filename;

    unsigned int time_work;
    int done;
    int result;
    int thread_num;

} WORK_ITEM;

typedef struct
{
    const char* script_name;
    const char* resource_dir;
    const char* corpus_work_dir;
    const VersatileEncodingConfig* vec;

    WORK_ITEM* wrk_item_array;
    int next_job;
    int count_job;
    int verbose_when_run;
    int verbose_direct_stdout;


    SYNC_Mutex_OBJECT mutex;

} CONFIG_BATCH;


static int get_next_job(CONFIG_BATCH* config_batch, unsigned int iNumThread, int prev_job)
{
    SyncGetMutex(config_batch->mutex);
    int start_job = config_batch->next_job;
    if (config_batch->next_job != -1)
    {
        config_batch->next_job++;
        if (config_batch->next_job == config_batch->count_job)
        {
            config_batch->next_job = -1;
        }
    }

    WORK_ITEM * finished_job = (config_batch->wrk_item_array) + prev_job;
    WORK_ITEM * new_job = (config_batch->wrk_item_array) + start_job;


    char* buffer_info = NULL;
    if ((prev_job != -1) && (config_batch->verbose_when_run))
        buffer_info = (char*)malloc((strlen(finished_job->src_filename) + strlen(finished_job->dest_filename) + 0x80) * sizeof(char*));
    if (buffer_info != NULL)
    {
        sprintf(buffer_info,"%u: finished work %s to %s job %.2f sec\n", iNumThread, finished_job->src_filename, finished_job->dest_filename, finished_job->time_work / 1000.);
        if (config_batch->verbose_direct_stdout)
            puts(buffer_info);
        else
            u_printf("%s\n", buffer_info);
        free(buffer_info);
        buffer_info = NULL;
    }


    if ((start_job != -1) && (config_batch->verbose_when_run))
        buffer_info = (char*)malloc((strlen(new_job->src_filename) + strlen(new_job->dest_filename) + 0x80) * sizeof(char*));
    if (buffer_info != NULL)
    {
        sprintf(buffer_info, "%u: start work %s to %s job\n", iNumThread, new_job->src_filename, new_job->dest_filename);
        if (config_batch->verbose_direct_stdout)
            puts(buffer_info);
        else
            u_printf("%s\n", buffer_info);
        free(buffer_info);
        buffer_info = NULL;
    }


    SyncReleaseMutex(config_batch->mutex);

    return start_job;
}


static void SYNC_CALLBACK_UNITEX ThreadFuncBatch(void* privateDataPtr, unsigned int iNumThread)
{
    CONFIG_BATCH* config_batch = (CONFIG_BATCH*)privateDataPtr;
    int num_job = -1;
    while ((num_job = get_next_job(config_batch, iNumThread, num_job)) != -1)
    {
        WORK_ITEM * current_job = (config_batch->wrk_item_array) + num_job;
        hTimeElapsed calc_work_time = SyncBuidTimeMarkerObject();

        char* buf_cmd_line = (char*)malloc(0x100 +
            (2 * strlen(config_batch->resource_dir)) + strlen(config_batch->script_name) +
            strlen(current_job->src_filename) + strlen(current_job->dest_filename));


        if (buf_cmd_line != NULL)
        {
            char unique_string[UNIQUE_STRING_FOR_POINTER_MAX_SIZE];
            fill_unique_string_for_pointer((const void*)&num_job, unique_string);
            char** users_variables = build_empty_users_variables();
            users_variables = insert_variable_and_value(users_variables, "UNIQUE_VALUE", unique_string);
            users_variables = insert_variable_and_value(users_variables, "CORPUS_WORK_DIR", config_batch->corpus_work_dir);
            users_variables = insert_variable_and_value(users_variables, "INPUT_FILE_1", current_job->src_filename);
            users_variables = insert_variable_and_value(users_variables, "OUTPUT_FILE_1", current_job->dest_filename);
            users_variables = insert_variable_and_value(users_variables, "PACKAGE_DIR", config_batch->resource_dir);


            strcpy(buf_cmd_line, config_batch->resource_dir);
            strcat(buf_cmd_line, PATH_SEPARATOR_STRING);
            strcat(buf_cmd_line, config_batch->script_name);

            const char* scriptFile = buf_cmd_line;
            current_job->thread_num = (int)iNumThread;
            int retvalue = run_scriptfile(config_batch->vec, scriptFile, users_variables, 1,1,NULL, config_batch->verbose_when_run);
            free_users_variable(users_variables);
            current_job->result = retvalue;

            free(buf_cmd_line);
        }
        else
            current_job->result = -1;

        current_job->time_work = SyncGetMSecElapsed(calc_work_time);
        current_job->done = 1;
    }
}

typedef struct {
    char unique_string[UNIQUE_STRING_FOR_POINTER_MAX_SIZE];
    char defaultResDir[UNIQUE_STRING_FOR_POINTER_MAX_SIZE + 0x20];
    char defaultWorkDir[UNIQUE_STRING_FOR_POINTER_MAX_SIZE + 0x20];
    char buffer_info[0x200];
} run_package_script_batch_text_buffer;

static int run_package_script_batch_internal(const VersatileEncodingConfig* vec,
                                             int verbose, int verbose_list_at_end, int verbose_when_run, int verbose_direct_stdout,
                                             int quiet_tool, int quiet_tool_err,
                                             const char* package_name, const char* script_name,
                                             const char*src_dir, const char* dest_dir,
                                             const char*resource_dir, const char* corpus_work_dir,
                                             int nb_threads, unsigned int stack_size, char** file_list)
{

    run_package_script_batch_text_buffer* textbuf = (run_package_script_batch_text_buffer*)malloc(sizeof(run_package_script_batch_text_buffer));
    if (textbuf == NULL) {
        alloc_error("main_UniBatchRunScript");
        return ALLOC_ERROR_CODE;
    }
    memset(textbuf, 0, sizeof(run_package_script_batch_text_buffer));


    int must_free_file_list = 0;
    //const char*resource_dir = "*UnitexPkgResource"; // VFS

    //String cmd_install = "UnitexTool InstallLingResourcePackage -p " + ling_package + " -x \"" + resource_dir + "\" -v";

    fill_unique_string_for_pointer((const void*)textbuf, textbuf->unique_string);

    if ((resource_dir == NULL) || ((*resource_dir) == '\0'))
    {
        if (is_filename_in_abstract_file_space("*resourceDir"))
            sprintf(textbuf->defaultResDir, "*resourceDir_%s", textbuf->unique_string);
        else if (is_filename_in_abstract_file_space("$:resourceDir"))
            sprintf(textbuf->defaultResDir, "$:resourceDir_%s", textbuf->unique_string);
        else
            sprintf(textbuf->defaultResDir, "resourceDir_%s", textbuf->unique_string);
        resource_dir = textbuf->defaultResDir;
    }

    if ((corpus_work_dir == NULL) || ((*corpus_work_dir) == '\0'))
    {
        if (is_filename_in_abstract_file_space("*workDir"))
            sprintf(textbuf->defaultWorkDir, "*workDir_%s", textbuf->unique_string);
        else if (is_filename_in_abstract_file_space("$:workDir"))
            sprintf(textbuf->defaultWorkDir, "$:workDir_%s", textbuf->unique_string);
        else
            sprintf(textbuf->defaultWorkDir, "workDir_%s", textbuf->unique_string);
        corpus_work_dir = textbuf->defaultWorkDir;
    }

    const char* args_install[] = { "UnitexTool", "InstallLingResourcePackage","-p", package_name,"-x",resource_dir,"-v",NULL };

    int install = 0;
    int command_found;
    run_command_direct((6 + verbose) - 1, ((char**)args_install) + 1, &command_found, &install);

    if (install != 0)
    {
        error("error in install resource %s", package_name);
        free(textbuf);
        return DEFAULT_ERROR_CODE;
    }

    if (file_list == NULL)
    {
        file_list = af_get_list_file(src_dir);
        if (file_list == NULL) {
            free(textbuf);
            return DEFAULT_ERROR_CODE;
        }
        must_free_file_list = 1;
    }


    int nb_files = 0;
    while ((*(file_list + nb_files)) != NULL) nb_files++;

    CONFIG_BATCH config_batch;
    config_batch.vec = vec;
    config_batch.count_job = nb_files;
    config_batch.next_job = (nb_files>0) ? 0 : -1;
    config_batch.resource_dir = resource_dir;
    config_batch.corpus_work_dir = corpus_work_dir;
    config_batch.mutex = SyncBuildMutex();
    config_batch.script_name = script_name;
    config_batch.verbose_when_run = verbose_when_run;
    config_batch.verbose_direct_stdout = verbose_direct_stdout && verbose_when_run;

    config_batch.wrk_item_array = (WORK_ITEM*)malloc(sizeof(WORK_ITEM) * (nb_files + 1));
    for (int i = 0;i < nb_files;i++)
    {
        WORK_ITEM * current_job = (config_batch.wrk_item_array) + i;
        const char* original_filename = *(file_list + i);
        current_job->done = 0;
        current_job->result = -1;
        current_job->time_work = 0;
        current_job->thread_num = 0;
        current_job->src_filename = (char*)malloc(strlen(src_dir) + strlen(original_filename) + 0x10);
        current_job->dest_filename = (char*)malloc(strlen(dest_dir) + strlen(original_filename) + 0x40);
        strcpy(current_job->src_filename, src_dir);
        strcat(current_job->src_filename, PATH_SEPARATOR_STRING);
        size_t pos_src_after_dir = strlen(current_job->src_filename);
        // if the list contain full filepath, we remove it
        if (strlen(original_filename) > pos_src_after_dir)
            if (memcmp(original_filename, current_job->src_filename, pos_src_after_dir) == 0)
                *current_job->src_filename = '\0';
        strcat(current_job->src_filename, original_filename);

        strcpy(current_job->dest_filename, dest_dir);
        strcat(current_job->dest_filename, PATH_SEPARATOR_STRING);
        strcat(current_job->dest_filename, current_job->src_filename + pos_src_after_dir);
        strcat(current_job->dest_filename, ".result.txt");
    }



    if ((verbose) || (verbose_when_run))
    {
            sprintf(textbuf->buffer_info, "start %d jobs using %d threads\n", config_batch.count_job, nb_threads);
            if (verbose_direct_stdout)
                puts(textbuf->buffer_info);
            else
                u_printf("%s\n", textbuf->buffer_info);
    }

    int trash_out = 0;
    t_fnc_stdOutWrite fnc_out = NULL;
    void* private_out = NULL;

    if (quiet_tool == 1) {
        GetStdWriteCB(stdwrite_kind_out, &trash_out, &fnc_out, &private_out);
        SetStdWriteCB(stdwrite_kind_out, 1, NULL, NULL);
    }

    int trash_err = 0;
    t_fnc_stdOutWrite fnc_err = NULL;
    void* private_err = NULL;
    if (quiet_tool_err == 1) {
        GetStdWriteCB(stdwrite_kind_err, &trash_err, &fnc_err, &private_err);
        SetStdWriteCB(stdwrite_kind_err, 1, NULL, NULL);
    }

    hTimeElapsed calc_works_time = SyncBuidTimeMarkerObject();

    if ((nb_threads > 0) || (stack_size != 0))
    {
        void** ptrptr = (void**)malloc(sizeof(void*)*(nb_threads + 1));
        if (ptrptr != NULL)
        {
            for (int i = 0;i < nb_threads;i++)
                *(ptrptr + i) = &config_batch;
            SyncDoRunThreadsWithStackSize(nb_threads, ThreadFuncBatch, ptrptr, stack_size);
            free(ptrptr);
        }
    }
    else
    {
        ThreadFuncBatch((void*)&config_batch, 0);
    }


    unsigned int time_works = SyncGetMSecElapsed(calc_works_time);


    if (quiet_tool == 1) {
        SetStdWriteCB(stdwrite_kind_out, trash_out, fnc_out, private_out);
    }
    if (quiet_tool_err) {
        SetStdWriteCB(stdwrite_kind_err, trash_err, fnc_err, private_err);
    }

    for (int i = 0;i < nb_files;i++)
    {
        WORK_ITEM * current_job = (config_batch.wrk_item_array) + i;

        if (verbose_list_at_end)
        {
            char* buffer_info_list = (char*)malloc((strlen(current_job->src_filename) + strlen(current_job->dest_filename) + 0x80) * sizeof(char*));

            if (buffer_info_list != NULL)
            {
                sprintf(buffer_info_list, "%d: %s to %s result %d take %.3f sec on thread %d\n", i, current_job->src_filename, current_job->dest_filename,
                    current_job->result, current_job->time_work / 1000., current_job->thread_num);
                if (verbose_direct_stdout)
                    puts(buffer_info_list);
                else
                    u_printf("%s\n", buffer_info_list);
                free(buffer_info_list);
            }
        }
        free(current_job->src_filename);
        free(current_job->dest_filename);
    }


    if ((verbose) || (verbose_when_run))
    {
        sprintf(textbuf->buffer_info, "done %d jobs taking %.3f sec using %d threads\n", config_batch.count_job, time_works/1000.,nb_threads);
        if (verbose_direct_stdout)
            puts(textbuf->buffer_info);
        else
            u_printf("%s\n", textbuf->buffer_info);
    }

    free(config_batch.wrk_item_array);
    SyncDeleteMutex(config_batch.mutex);
    if (must_free_file_list)
        af_release_list_file(src_dir, file_list);


    const char* args_uninstall[] = { "UnitexTool", "InstallLingResourcePackage","-p", package_name,"-x",resource_dir,"-u","-v",NULL };
    int uninstall = 0;
    run_command_direct((7+verbose) - 1, ((char**)args_uninstall) + 1, &command_found, &uninstall);

    //int uninstall = main_UnitexTool_C(8, (char**)args_uninstall);
    if (uninstall != 0)
    {
        error("error in uninstall resource %s", package_name);
        free(textbuf);
        return DEFAULT_ERROR_CODE;
    }
    free(textbuf);
    return 0;
}








const char* usage_UniBatchRunScript =
"Usage : BatchRunScript [OPTIONS] <scriptfile>\n"
"\n"
"  <scriptfile>: a script file to run\n"
"\n"
"OPTIONS:\n"
"  -i XXX/--input_dir=XXX : directory with input files\n"
"  -o XXX/--output_dir=XXX : directory with input files\n"
"  -r XXX/--resource_dir=XXX : directory where resource are installed\n"
"  -w XXX/--corpus_work_dir=XXX : directory for working on corpus\n"
"  -s XXX/--script_name=XXX : name of script file\n"
"  -t N/--thread=N: create N thread\n"
"  -z Nk/--stack-size=Nk: set stack size with N kilobytes (N multiple of 64)\n"
"  -v/--verbose: emit batch info message\n"
"  -l/--verbose_final: emit batch info list after running\n"
"  -p/--verbose_progress: emit batch info message when running\n"
"  -d/--verbose_direct: emit batch info message when running\n"
"  -m/--quiet: suppress working output\n"
"  -f/--quietter: suppress working output and error\n"
"  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
"  -h/--help: this help\n"
"\n";


static void UniBatchRunScript_usage() {
    display_copyright_notice();
    u_printf(usage_UniBatchRunScript);
}


const char* optstring_UniBatchRunScript = ":Vhfi:o:s:t:z:k:q:vmldpr:w:";
const struct option_TS lopts_UniBatchRunScript[] = {
    { "quiet", no_argument_TS, NULL, 'm' },
    { "quietter", no_argument_TS, NULL, 'f' },
    { "verbose", no_argument_TS, NULL, 'v' },
    { "verbose_final", no_argument_TS, NULL, 'l' },
    { "verbose_progress", no_argument_TS, NULL, 'p' },
    { "verbose_direct", no_argument_TS, NULL, 'd' },
    { "input_dir", required_argument_TS, NULL, 'i' },
    { "output_dir", required_argument_TS, NULL, 'o' },
    { "corpus_work_dir", required_argument_TS, NULL, 'w' },
    { "resource_dir", required_argument_TS, NULL, 'r' },
    { "script_name", required_argument_TS, NULL, 's' },
    { "thread", required_argument_TS, NULL, 't' },
    { "stack-size",required_argument_TS,NULL,'z' },
    { "input_encoding", required_argument_TS, NULL, 'k' },
    { "output_encoding", required_argument_TS, NULL, 'q' },
    { "only_verify_arguments",no_argument_TS,NULL, 'V' },
    { "help", no_argument_TS, NULL, 'h' },
    { NULL, no_argument_TS, NULL, 0 }
};

typedef struct
{
    char input_dir[FILENAME_MAX + 0x20] ;
    char output_dir[FILENAME_MAX + 0x20] ;
    char script_name[FILENAME_MAX + 0x20];
    char corpus_work_dir[FILENAME_MAX + 0x20] ;
    char resource_dir[FILENAME_MAX + 0x20] ;
} UniBatchRunScript_text_buffer;

int main_UniBatchRunScript(int argc, char* const argv[]) {
    if (argc == 1) {
        UniBatchRunScript_usage();
        return SUCCESS_RETURN_CODE;
    }


    UniBatchRunScript_text_buffer* textbuf = (UniBatchRunScript_text_buffer*)malloc(sizeof(UniBatchRunScript_text_buffer));
    if (textbuf == NULL) {
        alloc_error("main_UniBatchRunScript");
        return ALLOC_ERROR_CODE;
    }
    memset(textbuf, 0, sizeof(UniBatchRunScript_text_buffer));

    int verbose = 0;
    int verbose_when_run = 0;
    int quiet_out = 0;
    int quiet_err = 0;
    int verbose_direct_stdout = 0;
    int verbose_list_at_end = 0;
    VersatileEncodingConfig vec = VEC_DEFAULT;
    int val, index = -1;
    bool only_verify_arguments = false;
    UnitexGetOpt options;
    int nb_threads = 1;
    unsigned int stack_size = 0;
    char foo;

    strcpy(textbuf->script_name, "script");
    strcat(textbuf->script_name, PATH_SEPARATOR_STRING);
    strcat(textbuf->script_name, "standard.uniscript");
    while (EOF != (val = options.parse_long(argc, argv, optstring_UniBatchRunScript, lopts_UniBatchRunScript, &index))) {
        switch (val) {
        case 'm': quiet_out = 1;
            break;

        case 'f': quiet_out = quiet_err = 1;
            break;

        case 'v': verbose = 1;
            break;

        case 'l': verbose = verbose_list_at_end = 1;
            break;

        case 'p': verbose = verbose_when_run = 1;
            break;

        case 'd': verbose = verbose_direct_stdout = 1;
            break;

        case 'w': if (options.vars()->optarg[0] == '\0') {
                        error("You must specify a non corpus working directory\n");
                        free(textbuf);
                        return USAGE_ERROR_CODE;
                    }
                  strcpy(textbuf->corpus_work_dir, options.vars()->optarg);
                  break;

        case 'r': if (options.vars()->optarg[0] == '\0') {
                        error("You must specify a non empty resource directory\n");
                        free(textbuf);
                        return USAGE_ERROR_CODE;
                    }
                  strcpy(textbuf->resource_dir, options.vars()->optarg);
                  break;

        case 'i': if (options.vars()->optarg[0] == '\0') {
                        error("You must specify a non empty input dir\n");
                        free(textbuf);
                        return USAGE_ERROR_CODE;
                    }
                  strcpy(textbuf->input_dir, options.vars()->optarg);
                  break;

        case 'o': if (options.vars()->optarg[0] == '\0') {
                        error("You must specify a non empty output dir\n");
                        free(textbuf);
                        return USAGE_ERROR_CODE;
                    }
                  strcpy(textbuf->output_dir, options.vars()->optarg);
                  break;

        case 's': if (options.vars()->optarg[0] == '\0') {
                        error("You must specify a non empty script name\n");
                        free(textbuf);
                        return USAGE_ERROR_CODE;
                    }
                  strcpy(textbuf->script_name, options.vars()->optarg);
                  break;

        case 't': if (1 != sscanf(options.vars()->optarg, "%d%c", &nb_threads, &foo) || nb_threads < 0) {
                        /* foo is used to check that arg is not like "45gjh" */
                        error("Invalid nb_thread argument: %s\n", options.vars()->optarg);
                        free(textbuf);
                        return USAGE_ERROR_CODE;
                    }
                  break;

        case 'z':
        {
            char cSuffix = 0;
            int r = sscanf(options.vars()->optarg, "%u%c%c", &stack_size, &cSuffix, &foo);
            if ((r == 2) && ((cSuffix == 'k') || (cSuffix == 'K')))
            {
                stack_size = stack_size * 1024;
                break;
            }

            if ((r == 2) && ((cSuffix == 'm') || (cSuffix == 'M')))
            {
                stack_size = stack_size * 1024 * 1024;
                break;
            }

            if (r == 1) break;

            error("Invalid stack-size argument: %s\n", options.vars()->optarg);
            free(textbuf);
            return USAGE_ERROR_CODE;
        }

        case 'k': if (options.vars()->optarg[0] == '\0') {
                        error("Empty input_encoding argument\n");
                        free(textbuf);
                        return USAGE_ERROR_CODE;
                    }
                  decode_reading_encoding_parameter(&vec.mask_encoding_compatibility_input, options.vars()->optarg);
                  break;
        case 'q': if (options.vars()->optarg[0] == '\0') {
                        error("Empty output_encoding argument\n");
                        free(textbuf);
                        return USAGE_ERROR_CODE;
                    }
                  decode_writing_encoding_parameter(&vec.encoding_output, &vec.bom_output, options.vars()->optarg);
                  break;
        case 'V': only_verify_arguments = true;
            break;
        case 'h': UniBatchRunScript_usage();
            free(textbuf);
            return SUCCESS_RETURN_CODE;
        case ':': index == -1 ? error("Missing argument for option -%c\n", options.vars()->optopt) :
            error("Missing argument for option --%s\n", lopts_UniRunScript[index].name);
            return USAGE_ERROR_CODE;
        case '?': index == -1 ? error("Invalid option -%c\n", options.vars()->optopt) :
            error("Invalid option --%s\n", options.vars()->optarg);
            free(textbuf);
            return USAGE_ERROR_CODE;
        }
        index = -1;
    }

    if ((options.vars()->optind != argc - 1) || ((*(textbuf->input_dir)) == '\0') || ((*(textbuf->output_dir)) == '\0')) {
        error("Invalid arguments: rerun with --help\n");
        free(textbuf);
        return USAGE_ERROR_CODE;
    }

    if (only_verify_arguments) {
        // freeing all allocated memory
        free(textbuf);
        return SUCCESS_RETURN_CODE;
    }

    const char* package_name = argv[options.vars()->optind];

    if (verbose_when_run && quiet_out)
        verbose_direct_stdout = 1;

    int retvalue = run_package_script_batch_internal(&vec,
        verbose, verbose_list_at_end, verbose_when_run, verbose_direct_stdout,
        quiet_out, quiet_err,
        package_name, textbuf->script_name,
        textbuf->input_dir, textbuf->output_dir,
        textbuf->resource_dir, textbuf->corpus_work_dir,
        nb_threads, stack_size, NULL);

    free(textbuf);
    return retvalue;
}



//} // namespace logger
} // namespace unitex

#endif

