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

#include "Copyright.h"

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


static void addTextInDynamicString(char** dyn_string, size_t *pos, size_t *allocated, size_t buffer_margin, const char* add_str)
{	
	if (add_str == NULL)
		add_str = "";
	size_t len_add = strlen(add_str);
	size_t needed_size = (*pos) + len_add + buffer_margin + 1;
	if (needed_size >= (*allocated))
	{
		size_t new_allocated_size = 1;
		while (new_allocated_size <= needed_size)
			new_allocated_size *= 2;

		char* new_string_buffer = ((*dyn_string) == NULL) ? ((char*)malloc(new_allocated_size)) : ((char*)realloc(*dyn_string, new_allocated_size));
		if (new_string_buffer == NULL)
		{
			fatal_alloc_error("addTextInDynamicString");
		}
		*dyn_string = new_string_buffer;
	}
	memcpy((*dyn_string) + (*pos), add_str, len_add + 1);
	(*pos) += len_add;	
}


// char** subsitution contain on even entry variable name and on odd entry variable content
static char* substitueInLine(const char* line, char** variable_list)
{
	size_t len_line = strlen(line);
	char* build_string = NULL;
	size_t pos = 0;
	size_t allocated = 0;
	// allocated build_string with at last line size
	addTextInDynamicString(&build_string, &pos, &allocated, len_line, NULL);
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
				int j = 0;
				for (;;)
				{
					if (*(variable_list + j) == NULL)
						break;
					size_t len_var_in_list = strlen(*(variable_list + j));
					if (len_var_in_list == len_search_variable)
					{
						if (memcmp(*(variable_list + j), line + i + 1, len_var_in_list) == 0)
						{
							is_variable = 1;
							addTextInDynamicString(&build_string, &pos, &allocated, len_line, *(variable_list + j + 1));
							i = (end_var - line);
						}
					}
					j += 2;
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


static char** ReadScriptFile(const VersatileEncodingConfig* vec, const char* scriptFileName, char** users_variables, int transform_path_separator)
{
	unichar* line = NULL;
	size_t size_buffer_line = 0;

	char ** Lines = (char**)malloc(sizeof(char*));
	*Lines = NULL;
	int nbLines = 0;


	U_FILE* dest_read_script = u_fopen(vec, scriptFileName, U_READ);
	if (dest_read_script == NULL)
	{
		char* transformed_scriptFileName = strdup(scriptFileName);
		transform_fileName_separator(transformed_scriptFileName, transform_path_separator);
		dest_read_script = u_fopen(vec, transformed_scriptFileName, U_READ);
		free(transformed_scriptFileName);

	}
	if (dest_read_script == NULL){
		fatal_error("Cannot open file %s\n", scriptFileName);
		exit(1);
	}



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
		for (;;)
		{
			if (skip >= len_line)
				break;
			if ((*(line + skip) != '\t') && (*(line + skip) != ' '))
				break;
			skip ++ ;
		}
		char* charline = (char*)malloc((len_line - skip) + 1);
		for (size_t i = 0; i < (len_line - skip); i++)
			*(charline + i) = (char)*(line + skip + i);
		*(charline + (len_line - skip)) = '\0';



		char* reworked = substitueInLine(charline, users_variables);
		transform_fileName_separator(reworked, transform_path_separator);

		Lines = (char**)realloc(Lines, sizeof(char*) * (nbLines+2));
		*(Lines + nbLines) = reworked;
		*(Lines + nbLines + 1) = NULL;
		nbLines ++ ;
		free(charline);
	}
	if (line != NULL) {
		free(line);
	}
	u_fclose(dest_read_script);
	return Lines;
}


static void freeScriptFile(char** list)
{
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


int run_scriptfile(const VersatileEncodingConfig* vec, const char*scriptFileName, char**users_variables, int verbose)
{
	char**lines = ReadScriptFile(vec, scriptFileName, users_variables, UNPACKFILE_LIST_FOLDER_SEPARATOR_TRANSFORMATION_PLATFORM);

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

	return 0;
}

const char* usage_UniRunScript =
"Usage : RunScript [OPTIONS] <scriptfile>\n"
"\n"
"  <scriptfile>: a script file to run\n"
"\n"
"OPTIONS:\n"
"  -a X/--variable=X=Y: uses X as variable with Y content\n"
"  -v/--verbose: emit message when running\n"
"\n";


static void usage() {
	u_printf("%S", COPYRIGHT);
	u_printf(usage_UniRunScript);
}


const char* optstring_UniRunScript = ":hk:q:a:v";
const struct option_TS lopts_UniRunScript[] = {
	{ "variable", required_argument_TS, NULL, 'a' },
	{ "verbose", no_argument_TS, NULL, 'v' },
	{ "input_encoding", required_argument_TS, NULL, 'k' },
	{ "output_encoding", required_argument_TS, NULL, 'q' },
	{ "help", no_argument_TS, NULL, 'h' },
	{ NULL, no_argument_TS, NULL, 0 }
};




int main_UniRunScript(int argc, char* const argv[])
{

	if (argc == 1) {
		usage();
		return 0;
	}

	int verbose = 0;

	VersatileEncodingConfig vec = VEC_DEFAULT;
	int val, index = -1;
	struct OptVars* vars = new_OptVars();
	int nb_user_variable = 1;
	char** users_variables = (char**)malloc((sizeof(char*) * 2) * 2);
	if (users_variables == NULL)
	{
		fatal_alloc_error("main_RunScript");
	}
	*(users_variables) = strdup("UNIQUE_VALUE");
	// speca needed : (0x10+1) with margin
	*(users_variables+1) = (char*)malloc(0x20);
	if (((*users_variables) == NULL) || ((*(users_variables+1)) == NULL))
	{
		fatal_alloc_error("main_RunScript");
	}

	fillUniqueStringForPointer((const void*)&verbose, *(users_variables + 1));

	*(users_variables+2) = *(users_variables + 3) = NULL;
	
	while (EOF != (val = getopt_long_TS(argc, argv, optstring_UniRunScript, lopts_UniRunScript, &index, vars))) {
		switch (val) {

		case 'v': verbose = 1; break;

		case 'a':
		{
			if (vars->optarg[0] == '\0') {
				fatal_error("You must specify a non empty variable\n");
			}

			users_variables = (char**)realloc(users_variables, (nb_user_variable + 2) * 2 * sizeof(char*));
			char* new_var_name = strdup(vars->optarg);
			
			char * pos_equal = strchr(new_var_name,'=');
			char * new_var_content;
			if (pos_equal != NULL)
			{
				new_var_content = strdup(pos_equal + 1);
				*pos_equal = '\0';
			}
			else
			{
				new_var_content = strdup("");
			}
			*(users_variables + (nb_user_variable * 2)) = new_var_name;
			*(users_variables + (nb_user_variable * 2) + 1) = new_var_content;
			nb_user_variable++;
			*(users_variables + (nb_user_variable * 2)) = *(users_variables + (nb_user_variable * 2) + 1) = NULL;

			break;
		}
		

		case 'k': if (vars->optarg[0] == '\0') {
			fatal_error("Empty input_encoding argument\n");
		}
				  decode_reading_encoding_parameter(&vec.mask_encoding_compatibility_input, vars->optarg);
				  break;
		case 'q': if (vars->optarg[0] == '\0') {
			fatal_error("Empty output_encoding argument\n");
		}
				  decode_writing_encoding_parameter(&vec.encoding_output, &vec.bom_output, vars->optarg);
				  break;
		case 'h': usage(); free_OptVars(vars); return 0;
		case ':': if (index == -1) fatal_error("Missing argument for option -%c\n", vars->optopt);
				  else fatal_error("Missing argument for option --%s\n", lopts_UniRunScript[index].name);
		case '?': if (index == -1) fatal_error("Invalid option -%c\n", vars->optopt);
				  else fatal_error("Invalid option --%s\n", vars->optarg);
				  break;
		}
		index = -1;
	}

	if (vars->optind != argc - 1) {
		fatal_error("Invalid arguments: rerun with --help\n");
	}

	const char* scriptFile = argv[vars->optind];

	int retvalue = run_scriptfile(&vec, scriptFile, users_variables, verbose);

	for (int i = 0; i < (nb_user_variable * 2); i++)
		free(*(users_variables + i));

	free(users_variables);
	free_OptVars(vars);
	return retvalue;
}


//} // namespace logger
} // namespace unitex

#endif
