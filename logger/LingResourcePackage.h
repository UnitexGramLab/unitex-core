/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

/* LingResourcePackage.h */


#ifndef NO_UNITEX_LOGGER



#ifndef _LING_RESOURCE_PACKAGE_H
#define _LING_RESOURCE_PACKAGE_H 1

#include "AbstractCallbackFuncModifier.h"

#ifdef __cplusplus
#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

    namespace logger {
        extern "C" {
#endif



            int install_ling_resource_package(const char* package_name, const char* prefix_destination,
                int folder_separator_transformation,
                int persist_file, int persist_graph, int persist_dictionary, int persist_alphabet, int keep_persisted_file,
                char*** file_list, char*** persist_graph_list, char*** persist_dictionary_list, char*** persist_alphabet_list);

            UNITEX_FUNC int UNITEX_CALL InstallLingResourcePackage(const char* package_name, const char* prefix_destination,
                int folder_separator_transformation,
                int persist_file, int persist_graph, int persist_dictionary, int persist_alphabet,int keep_persisted_file,
                char*** file_list, char*** persist_graph_list, char*** persist_dictionary_list, char*** persist_alphabet_list);


            int uninstall_ling_resource_package(const char* package_name, const char* prefix_destination,
                int persist_file, int folder_separator_transformation,
                int persist_graph, int persist_dictionary, int persist_alphabet, int kept_persisted_file);

            UNITEX_FUNC int UNITEX_CALL UninstallLingResourcePackage(const char* package_name, const char* prefix_destination,
                int persist_file, int folder_separator_transformation,
                int persist_graph, int persist_dictionary, int persist_alphabet, int kept_persisted_file);


            int uninstall_ling_resource_package_by_list(char** file_list,
                char** persist_graph_list, char** persist_dictionary_list, char** persist_alphabet_list);

            UNITEX_FUNC int UNITEX_CALL UninstallLingResourcePackagByList(char** file_list,
                char** persist_graph_list, char** persist_dictionary_list, char** persist_alphabet_list);

            void free_installed_resource_package_list(char**);

            UNITEX_FUNC void UNITEX_CALL FreeInstalledResourcePackageList(char**);


            void count_items_in_installed_resource_package_list(char**);

            UNITEX_FUNC void UNITEX_CALL CountItemsInInstalledResourcePackageList(char**);

            const char* access_item_in_installed_resource_package_list(char** list, unsigned int pos);

            UNITEX_FUNC const char* UNITEX_CALL AccessItemInInstalledResourcePackageList(char** list, unsigned int pos);



            UNITEX_FUNC int UNITEX_CALL WriteFilesListToFile(const char* filename_out, char** list);

            int write_files_list_to_file(const char* filename_out, char** list);

            UNITEX_FUNC char** UNITEX_CALL ReadListFilesFromFile(const char* filein_name);

            char** read_list_files_from_file(const char* filein_name);

            void free_list_files_from_file(char**list);

            UNITEX_FUNC void UNITEX_CALL FreeListFilesFromFile(char** list);

            unsigned int count_items_in_list_files_from_file(char** list);

            UNITEX_FUNC unsigned int UNITEX_CALL CountItemsInListFilesFromFile(char** list);

            const char* access_item_in_list_files_from_file(char** list, unsigned int pos);

            UNITEX_FUNC const char* UNITEX_CALL AccessItemInListFilesFromFile(char** list, unsigned int pos);

#ifdef __cplusplus
        } // extern "C"
    } // namespace logger
} // namespace unitex
#endif






#endif

#endif
