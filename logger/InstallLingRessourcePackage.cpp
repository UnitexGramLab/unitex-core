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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include "Unicode.h"
#include "Copyright.h"
#include "UnitexGetOpt.h"

#include "FilePackType.h"
#include "UnpackFileTool.h"
#include "LingRessourcePackage.h"
#include "InstallLingRessourcePackage.h"



#ifndef NO_UNITEX_LOGGER

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


#ifdef HAS_LOGGER_NAMESPACE
    using namespace ::unitex::logger;
#endif


        extern const char* optstring_InstallLingRessourcePackage;
        extern const struct option_TS lopts_InstallLingRessourcePackage[];
        extern const char* usage_InstallLingRessourcePackage;


        const char* usage_InstallLingRessourcePackage =
            "Usage : InstallLingRessourcePackage [OPTIONS]\n"
            "\n"
            "Install resource from linguisting resource package:\n"
            "    InstallLingRessourcePackage -p <LingRessourcePackageFileName> -x <LocationPrefix>\n"
            "  Optionals arguments:\n"
            "         [-f filename_all_files.txt] [-a filename_list_alphabet.txt]"
            "         [-g filename_list_graph.txt] [-d filename_list_dictionary.txt]\n"
            "         [-F] [-A] [-G] [-D]\n"
            "  -f, -a, -g ,-d create text file with list of resource\n"
            "  -F, -A, -G ,-D prevent install files, or persist alphabet, graph and dictionary\n"
            "\n"
            "\n"
            "Uninstall resource from linguisting resource package:\n"
            "    InstallLingRessourcePackage -p <LingRessourcePackageFileName> -x <LocationPrefix>\n"
            "  Optionals arguments:\n"
            "         [-F] [-A] [-G] [-D]\n"
            "  <LingRessourcePackageFileName> and <LocationPrefix> must have been used on installing\n"
            "  -F, -A, -G ,-D prevent uninstall files, or unpersist alphabet, graph and dictionary\n"
            "\n"
            "Uninstall resource from file with list of resource:\n"
            "    InstallLingRessourcePackage -u\n"
            "         [-f filename_all_files.txt] [-a filename_list_alphabet.txt]"
            "         [-g filename_list_graph.txt] [-d filename_list_dictionary.txt]\n"
            "   File must have been created on installing\n"
            "   List File are not removed (you can uses DuplicateFile -d)\n"
            "\n"
            "\n"
            "  LingRessourcePackageFileName is a full pathname to a linguistic resource package file\n"
            "      which is an uncompressed zip-like file collection.n"
            "    This file can be create with with zip (using -0 -X options) or PackFile in Unitex\n"
            "    Without -G, files ending with .fst2 will be persisted as graph\n"
            "    Without -D, files ending with .bin (with an associated .inf) or .bin2 will be persisted as dict.\n"
            "    Without -A, files ending with Alphabet.txt will be persisted as alphabet.\n"
            "\n"
            "other options:"
            "  -n/--no_translate_path_separator: do not translate separator in filename from pack\n"
            "  -t/--translate_path_separator_to_native: translate separator in filename from pack to current platform\n"
            ""
            ;

        static void usage() {
            u_printf("%S", COPYRIGHT);
            u_printf(usage_InstallLingRessourcePackage);
        }


        const char* optstring_InstallLingRessourcePackage = ":vtnuFGDAf:g:d:a:hx:p:k:q:";
        const struct option_TS lopts_InstallLingRessourcePackage[] = {
            { "input_encoding", required_argument_TS, NULL, 'k' },
            { "output_encoding", required_argument_TS, NULL, 'q' },
            { "help", no_argument_TS, NULL, 'h' },


            { "list_files", required_argument_TS, NULL, 'f' },
            { "list_graph", required_argument_TS, NULL, 'g' },
            { "list_dictionary", required_argument_TS, NULL, 'd' },
            { "list_alphabet", required_argument_TS, NULL, 'a' },
            { "no_persist_file", no_argument_TS, NULL, 'F' },
            { "no_persist_graph", no_argument_TS, NULL, 'G' },
            { "no_persist_dictionary", no_argument_TS, NULL, 'D' },
            { "no_persist_alphabet", no_argument_TS, NULL, 'A' },
            { "verbose", no_argument_TS, NULL, 'v' },
            { "uninstall", no_argument_TS, NULL, 'u' },
            { "prefix", required_argument_TS, NULL, 'x' },
            { "package", required_argument_TS, NULL, 'p' },
            { "no_translate_path_separator", no_argument_TS, NULL, 'n' },
            { "translate_path_separator_to_native", no_argument_TS, NULL, 't' },
            { NULL, no_argument_TS, NULL, 0 }
        };

        int main_InstallLingRessourcePackage(int argc, char* const argv[])
        {

            if (argc == 1) {
                usage();
                return 0;
            }
            char ListFile_FileName[FILENAME_MAX + 0x20] = "";
            char ListGraph_FileName[FILENAME_MAX + 0x20] = "";
            char ListDico_FileName[FILENAME_MAX + 0x20] = "";
            char ListAlphabet_FileName[FILENAME_MAX + 0x20] = "";
            char Prefix_Name[FILENAME_MAX + 0x20] = "";
            char Package_FileName[FILENAME_MAX + 0x20] = "";
            Encoding encoding_output = DEFAULT_ENCODING_OUTPUT;
            int bom_output = DEFAULT_BOM_OUTPUT;
            int mask_encoding_compatibility_input = DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT;
            int val, index = -1;
            int uninstall = 0;
            int persist_file = 1;
            int persist_graph = 1;
            int persist_dictionary = 1;
            int persist_alphabet = 1;
            int verbose = 0;
            int transform_path_separator = -1;
            //int persistence_alphabet = 0;
            struct OptVars* vars = new_OptVars();
            while (EOF != (val = getopt_long_TS(argc, argv, optstring_InstallLingRessourcePackage, lopts_InstallLingRessourcePackage, &index, vars))) {
                switch (val) {


                case 't': transform_path_separator = UNPACKFILE_LIST_FOLDER_SEPARATOR_TRANSFORMATION_PLATFORM; break;

                case 'n': transform_path_separator = UNPACKFILE_LIST_FOLDER_SEPARATOR_TRANSFORMATION_UNMODIFIED; break;

                case 'k': if (vars->optarg[0] == '\0') {
                              fatal_error("Empty input_encoding argument\n");
                }
                          decode_reading_encoding_parameter(&mask_encoding_compatibility_input, vars->optarg);
                          break;
                case 'q': if (vars->optarg[0] == '\0') {
                              fatal_error("Empty output_encoding argument\n");
                }
                          decode_writing_encoding_parameter(&encoding_output, &bom_output, vars->optarg);
                          break;


                case 'x': if (vars->optarg[0] == '\0') {
                    fatal_error("You must specify a non empty argument\n");
                }
                          strcpy(Prefix_Name, vars->optarg);
                          break;

                case 'p': if (vars->optarg[0] == '\0') {
                    fatal_error("You must specify a non empty argument\n");
                }
                          strcpy(Package_FileName, vars->optarg);
                          break;

                case 'f': if (vars->optarg[0] == '\0') {
                    fatal_error("You must specify a non empty argument\n");
                }
                          strcpy(ListFile_FileName, vars->optarg);
                          break;

                case 'g': if (vars->optarg[0] == '\0') {
                    fatal_error("You must specify a non empty argument\n");
                }
                          strcpy(ListGraph_FileName, vars->optarg);
                          break;

                case 'd': if (vars->optarg[0] == '\0') {
                    fatal_error("You must specify a non empty argument\n");
                }
                          strcpy(ListDico_FileName, vars->optarg);
                          break;

                case 'a': if (vars->optarg[0] == '\0') {
                    fatal_error("You must specify a non empty argument\n");
                }
                          strcpy(ListAlphabet_FileName, vars->optarg);
                          break;

                case 'v': verbose = 1; break;

                case 'F': persist_file = 0; break;
                case 'G': persist_graph = 0; break;
                case 'D': persist_dictionary = 0; break;
                case 'A': persist_alphabet = 0; break;


                case 'u': uninstall = 1; break;

                case 'h': usage(); return 0;
                case ':': if (index == -1) fatal_error("Missing argument for option -%c\n", vars->optopt);
                          else fatal_error("Missing argument for option --%s\n", lopts_InstallLingRessourcePackage[index].name);
                case '?': if (index == -1) fatal_error("Invalid option -%c\n", vars->optopt);
                          else fatal_error("Invalid option --%s\n", vars->optarg);
                          break;
                }
                index = -1;
            }
            /*
            if (vars->optind != argc - 1) {
                fatal_error("Invalid arguments: rerun with --help\n");
            }*/
            free_OptVars(vars);

            if (transform_path_separator == -1)
            {
                if (strchr(Prefix_Name, '/') != NULL)
                {
                    transform_path_separator = UNPACKFILE_LIST_FOLDER_SEPARATOR_TRANSFORMATION_UNIX;
                }
                else
                if (strchr(Prefix_Name, '\\') != NULL)
                {
                    transform_path_separator = UNPACKFILE_LIST_FOLDER_SEPARATOR_TRANSFORMATION_WINDOWS;
                }
                else
                {
                    transform_path_separator = UNPACKFILE_LIST_FOLDER_SEPARATOR_TRANSFORMATION_UNMODIFIED;
                }
            }

            int success = 1;

            if (!uninstall)
            {
                if ((*Package_FileName) == '\0')
                {
                    fatal_error("no linguistic resource package provided.\n");
                }

                if (((persist_file == 0) && ((*ListFile_FileName) != '\0')) ||
                    ((persist_graph == 0) && ((*ListGraph_FileName) != '\0')) ||
                    ((persist_dictionary == 0) && ((*ListDico_FileName) != '\0')) ||
                    ((persist_alphabet == 0) && ((*ListAlphabet_FileName) != '\0')))
                {
                    fatal_error("You cannot disable a persistence and provide list filename.\n");
                }

                if (verbose)
                    u_printf("Install resource from package %s to prefix %s\n", Package_FileName, Prefix_Name);

                char** list_installed_file = NULL;
                char** list_installed_graph = NULL;
                char** list_installed_dictionary = NULL;
                char** list_installed_alphabet = NULL;

                int result_install =
                    install_ling_resource_package(Package_FileName, Prefix_Name,
                    transform_path_separator,
                    persist_file, persist_graph, persist_dictionary, persist_alphabet,
                    &list_installed_file, &list_installed_graph, &list_installed_dictionary, &list_installed_alphabet);
                if (!result_install)
                {
                    error("error on installing resource from %s with prefix %s\n", Package_FileName, Prefix_Name);
                    success = 0;
                }

                if (list_installed_file && ((*ListFile_FileName) != '\0')) {
                    if (!write_files_list_to_file(ListFile_FileName, list_installed_file))
                    {
                        error("Cannot write list of file to %s\n", ListFile_FileName);
                        success = 0;
                    }
                }

                if (list_installed_graph && ((*ListGraph_FileName) != '\0')) {
                    if (!write_files_list_to_file(ListGraph_FileName, list_installed_graph))
                    {
                        error("Cannot write list of file to %s\n", ListGraph_FileName);
                        success = 0;
                    }
                }

                if (list_installed_dictionary && ((*ListDico_FileName) != '\0')) {
                    if (!write_files_list_to_file(ListDico_FileName, list_installed_dictionary))
                    {
                        error("Cannot write list of file to %s\n", ListDico_FileName);
                        success = 0;
                    }
                }

                if (list_installed_alphabet && ((*ListAlphabet_FileName) != '\0')) {
                    if (!write_files_list_to_file(ListAlphabet_FileName, list_installed_alphabet))
                    {
                        error("Cannot write list of file to %s\n", ListAlphabet_FileName);
                        success = 0;
                    }
                }

                free_installed_resource_package_list(list_installed_file);
                free_installed_resource_package_list(list_installed_graph);
                free_installed_resource_package_list(list_installed_dictionary);
                free_installed_resource_package_list(list_installed_alphabet);
            }
            else // uninstall
            {
                if (verbose)
                    u_printf("Uninstall resource from package %s to prefix %s\n", Package_FileName, Prefix_Name);

                if ((*Package_FileName) != '\0')
                {

                    int result_uninstall =
                        uninstall_ling_resource_package(Package_FileName, Prefix_Name,
                        transform_path_separator,
                        persist_file, persist_graph, persist_dictionary, persist_alphabet);
                    if (!result_uninstall)
                    {
                        error("error on uninstalling resource from package %s on prefix\n", Package_FileName, Prefix_Name);
                        success = 0;
                    }
                }
                else // uninstall by list
                {
                    if (verbose)
                        u_printf("Uninstall resource from lists file\n");

                    char** list_installed_file = read_list_files_from_file(ListFile_FileName);;
                    char** list_installed_graph = read_list_files_from_file(ListGraph_FileName);
                    char** list_installed_dictionary = read_list_files_from_file(ListDico_FileName);
                    char** list_installed_alphabet = read_list_files_from_file(ListAlphabet_FileName);

                    int result_uninstall = uninstall_ling_resource_package_by_list(list_installed_file,
                        list_installed_graph, list_installed_dictionary, list_installed_alphabet);

                    if (!result_uninstall)
                    {
                        error("error on uninstalling resource\n");
                        success = 0;
                    }

                    free_list_files_from_file(list_installed_file);
                    free_list_files_from_file(list_installed_graph);
                    free_list_files_from_file(list_installed_dictionary);
                    free_list_files_from_file(list_installed_alphabet);
                }
            }

            return success ? 0 : 1;
        }

//    } // namespace logger
} // namespace unitex

#endif
