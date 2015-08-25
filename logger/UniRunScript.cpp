/*
 * Unitex
 *
 * Copyright (C) 2001-2014 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * additional information: http://www.ergonotics.com/unitex-contribution/
 * contact : unitex-contribution@ergonotics.com
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

static void addTextInDynamicString(char** dyn_string, size_t *pos, size_t *allocated, size_t buffer_margin, const char* add_str) {  
  if (add_str == NULL) {
    add_str = "";
  }
  size_t len_add = strlen(add_str);
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
        size_t len_search_variable = (size_t)(end_var - (line + i + 1));
        
        const char* value_variable = NULL;
        if (len_search_variable != 0)
          value_variable = search_value_for_variable(*variable_list, line + i + 1, len_search_variable);
        if (value_variable != NULL)
        {
          addTextInDynamicString(&build_string, &pos, &allocated, len_line, value_variable);
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


static char** ReadScriptFile(const VersatileEncodingConfig* vec, const char* scriptFileName, char** users_variables_param, int transform_path_separator) {
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

    size_t skip = 0;
    for (;;) {
      if (skip >= len_line)
        break;
      if ((*(line + skip) != '\t') && (*(line + skip) != ' '))
        break;
      skip ++ ;
    }

    char* charline = (char*)malloc((len_line - skip) + 1);
    if (charline == NULL) {
      alloc_error("ReadScriptFile");
      free(line);
      u_fclose(dest_read_script);
      free_users_variable(users_variables);
      freeScriptFile(Lines);       
      return NULL;
    }

    for (size_t i = 0; i < (len_line - skip); i++) {
      *(charline + i) = (char)*(line + skip + i);
    }

    *(charline + (len_line - skip)) = '\0';

    char* reworked = substitueInLine(charline, &users_variables, 1);
    transform_fileName_separator(reworked, transform_path_separator);

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


int run_scriptfile(const VersatileEncodingConfig* vec, const char*scriptFileName, char**users_variables, int verbose) {
  char** lines = ReadScriptFile(vec, scriptFileName, users_variables, UNPACKFILE_LIST_FOLDER_SEPARATOR_TRANSFORMATION_PLATFORM);

  int i = 0;
  for (;;)
  {
    const char* curLine = *(lines + i);
    if (curLine == NULL)
      break;

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
        run_command_direct(argc-1, argv+1, &command_found, &ret_tool);

        if (verbose && (!command_found))
          u_printf("Command %s not found\n", argv[0]);

        if (verbose)
          u_printf("Running result: %d\n\n", ret_tool);
      }

      free_std_arg_converted(argv);
    }
    i++;
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
"  -v/--verbose: emit message when running\n"
"  -V/--only-verify-arguments: only verify arguments syntax and exit\n"
"  -h/--help: this help\n"
"\n";


static void usage() {
  display_copyright_notice();
  u_printf(usage_UniRunScript);
}


const char* optstring_UniRunScript = ":Vhk:q:a:v";
const struct option_TS lopts_UniRunScript[] = {
  { "variable", required_argument_TS, NULL, 'a' },
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
  char unique_string[UNIQUE_STRING_FOR_POINTER_MAX_SIZE];
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
      const char * new_var_content = "";
      if (pos_equal != NULL)
      {
        new_var_content = pos_equal + 1;
        *pos_equal = '\0';
      }

      users_variables = insert_variable_and_value(users_variables, new_var_name, new_var_content);
      free(rework_var_assign);

      break;
    }
    

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

  int retvalue = run_scriptfile(&vec, scriptFile, users_variables, verbose);

  free_users_variable(users_variables);
  return retvalue;
}


//} // namespace logger
} // namespace unitex

#endif
