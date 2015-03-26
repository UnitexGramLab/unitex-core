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




/*
*/

/* LingRessourcePackage.cpp */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "Unicode.h"


#include "Af_stdio.h"
#include "File.h"
#include "DirHelper.h"

#include "FilePackType.h"
#include "FileUnPack.h"
#include "UnpackFile.h"

#include "UnpackFileTool.h"
#include "LingRessourcePackage.h"
#include "PersistenceInterface.h"

#ifndef NO_UNITEX_LOGGER



#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {
#ifndef HAS_LOGGER_NAMESPACE
#define HAS_LOGGER_NAMESPACE 1
#endif

namespace logger {

    /*

UNITEX_FUNC int UNITEX_CALL ExtractFilesFromUnitexArchiveZZ(
    const char* packFileName,
    int opt_extract_without_path,
    const char* prefix_extracting_name,
    int transform_path_separator)
{
    int quiet = 1;
    return do_extract_from_pack_archive(
        packFileName,
        opt_extract_without_path,
        prefix_extracting_name,
        transform_path_separator, quiet);
}
*/

static int is_filename_end_directory_separator(const char* filename)
{
	if (filename == NULL)
		return 0;
	if ((*filename) == '\0')
		return 0;
	char c = *(filename + strlen(filename) - 1);
	return ((c == '/') || (c == '\\')) ? 1 : 0;
}


static int is_filename_graph(const char* filename)
{
    size_t len = strlen(filename);

    if (len > 4)
    {
        const char* ext4 = filename + len - 5;
        if ((strcmp(ext4, ".fst2") == 0) || (strcmp(ext4, ".FST2") == 0) || (strcmp(ext4, ".Fst2") == 0))
            return 1;
    }

    return 0;
}


int is_filename_dictionary(const char* filename)
{
    size_t len = strlen(filename);

    if (len > 4)
    {
        const char* ext4 = filename + len - 5;
        const char* ext3 = filename + len - 4;
        if ((strcmp(ext3, ".bin") == 0) || (strcmp(ext3, ".BIN") == 0) || (strcmp(ext3, ".Bin") == 0))
            return 1;
        if ((strcmp(ext4, ".bin2") == 0) || (strcmp(ext4, ".BIN2") == 0) || (strcmp(ext4, ".Bin2") == 0))
            return 1;
    }

    return 0;
}


int is_filename_alphabet(const char* filename)
{
    size_t len = strlen(filename);
    if (len >= 12)
    {
        const char* suffix = filename + len - 12;

        if ((strcmp(suffix, "alphabet.txt") == 0) || (strcmp(suffix, "Alphabet.txt") == 0) || (strcmp(suffix, "ALPHABET.TXT") == 0))
            return 1;
    }

    return 0;
}



int install_ling_resource_package(const char* package_name, const char* prefix_destination,
	int transform_path_separator,
    int persist_file, int persist_graph, int persist_dictionary, int persist_alphabet,
    char*** file_list,
    char*** persist_graph_list, char*** persist_dictionary_list, char*** persist_alphabet_list)
{
    int result = 1;

    char** fileListArchive = createFileInPackArchiveListFileWithTransformPathSeparator(package_name, transform_path_separator);
    char** files_persisted_list = NULL;
    char** graph_persisted_list = NULL;
    char** dictionary_persisted_list = NULL;
    char** alphabet_persisted_list = NULL;
    int nb_persisted_graph = 0;
    int nb_persisted_dictionary = 0;
    int nb_persisted_alphabet = 0;

    unsigned int nb_file_archive = countFileInPackArchiveListFile(fileListArchive);
    if (nb_file_archive == 0)
    {
        error("found no file in package %s\n", fileListArchive);
        return 0;
    }

    if (persist_file)
    {
        int quiet = 1;
        int opt_extract_without_path = 0;
        int do_extract_res = do_extract_from_pack_archive(
            package_name,
            opt_extract_without_path,
            prefix_destination,
            transform_path_separator, quiet);

        if (do_extract_res != 0)
        {
            error("error in extracting package %s to %s\n", package_name, prefix_destination);
            return 0;
        }
    }

    if (file_list)
    {
        files_persisted_list = (char**)malloc(sizeof(char*) * (nb_file_archive + 1));
        if (files_persisted_list == NULL)
        {
            fatal_alloc_error("install_ling_resource_package");
        }
        *files_persisted_list = NULL;
    }

    if (persist_graph && persist_graph_list)
    {
        graph_persisted_list = (char**)malloc(sizeof(char*) * (nb_file_archive + 1));
        if (graph_persisted_list == NULL)
        {
            fatal_alloc_error("install_ling_resource_package");
        }
        *graph_persisted_list = NULL;
    }

    if (persist_dictionary && persist_dictionary_list)
    {
        dictionary_persisted_list = (char**)malloc(sizeof(char*) * (nb_file_archive + 1));
        if (dictionary_persisted_list == NULL)
        {
            fatal_alloc_error("install_ling_resource_package");
        }
        *dictionary_persisted_list = NULL;
    }

    if (persist_alphabet && persist_alphabet_list)
    {
        alphabet_persisted_list = (char**)malloc(sizeof(char*) * (nb_file_archive + 1));
        if (alphabet_persisted_list == NULL)
        {
            fatal_alloc_error("install_ling_resource_package");
        }
        *alphabet_persisted_list = NULL;
    }

    size_t max_len_filename = 1;
    for (unsigned int i = 0; i < nb_file_archive; i++)
    {
        size_t len = strlen(*(fileListArchive + i));
        if (len > max_len_filename)
            max_len_filename = len;
    }

    size_t len_prefix = strlen(prefix_destination);
    size_t size_buffer_persist_filename = max_len_filename + len_prefix + 0x10;
    char* buffer_persist_filename = (char*)malloc(sizeof(char) * (size_buffer_persist_filename + 1));
    char* full_filename = (char*)malloc(sizeof(char) * (size_buffer_persist_filename + 1));

    if ((buffer_persist_filename == NULL) || (full_filename == NULL))
    {
        fatal_alloc_error("install_ling_resource_package");
    }

    for (unsigned int i = 0; i < nb_file_archive; i++)
    {
        const char* curFileNameInPackage = *(fileListArchive + i);
        strcpy(full_filename, prefix_destination);
        strcpy(full_filename + len_prefix, curFileNameInPackage);
		transform_fileName_separator(full_filename + len_prefix, transform_path_separator);

        if (files_persisted_list)
        {
            char* dup_persisted_filename = (char*)malloc(strlen(full_filename) + 1);
            if (dup_persisted_filename == NULL) {
                fatal_alloc_error("install_ling_resource_package");
            }
            strcpy(dup_persisted_filename, full_filename);
			*(files_persisted_list + i) = dup_persisted_filename;
			*(files_persisted_list + i + 1) = NULL;
        }

        if (persist_graph && is_filename_graph(full_filename))
        {
            // result_load==0 error
            *buffer_persist_filename = '\0';
            int result_load = standard_load_persistence_fst2(full_filename, buffer_persist_filename, size_buffer_persist_filename);
            if (!result_load)
            {
                result = 0;
                error("error in persistence %s\n", full_filename);
            }
            else if (strcmp(full_filename, buffer_persist_filename) != 0)
            {
                result = 0;
                error("persistence system change filename %s to %s\n", full_filename, buffer_persist_filename);
            }
            else
            {
                if (graph_persisted_list != NULL)
                {
                    char* dup_persisted_filename = (char*)malloc(strlen(buffer_persist_filename) + 1);
                    if (dup_persisted_filename == NULL) {
                        fatal_alloc_error("install_ling_resource_package");
                    }
                    strcpy(dup_persisted_filename, buffer_persist_filename);
                    *(graph_persisted_list + nb_persisted_graph) = dup_persisted_filename;
                    *(graph_persisted_list + nb_persisted_graph + 1) = NULL;
                }
                nb_persisted_graph++;
            }
        }

        if (persist_dictionary && is_filename_dictionary(full_filename))
        {
            // result_load==0 error
            *buffer_persist_filename = '\0';
            int result_load = standard_load_persistence_dictionary(full_filename, buffer_persist_filename, size_buffer_persist_filename);
            if (!result_load)
            {
                result = 0;
                error("error in persistence %s\n", full_filename);
            }
            else if (strcmp(full_filename, buffer_persist_filename) != 0)
            {
                result = 0;
                error("persistence system change filename %s to %s\n", full_filename, buffer_persist_filename);
            }
            else
            {
                if (dictionary_persisted_list != NULL)
                {
                    char* dup_persisted_filename = (char*)malloc(strlen(buffer_persist_filename) + 1);
                    if (dup_persisted_filename == NULL) {
                        fatal_alloc_error("install_ling_resource_package");
                    }
                    strcpy(dup_persisted_filename, buffer_persist_filename);
                    *(dictionary_persisted_list + nb_persisted_dictionary) = dup_persisted_filename;
                    *(dictionary_persisted_list + nb_persisted_dictionary + 1) = NULL;
                }
                nb_persisted_dictionary++;
            }
        }

        if (persist_alphabet && is_filename_alphabet(full_filename))
        {
            // result_load==0 error
            *buffer_persist_filename = '\0';
            int result_load = standard_load_persistence_alphabet(full_filename, buffer_persist_filename, size_buffer_persist_filename);
            if (!result_load)
            {
                result = 0;
                error("error in persistence %s\n", full_filename);
            }
            else if (strcmp(full_filename, buffer_persist_filename) != 0)
            {
                result = 0;
                error("persistence system change filename %s to %s\n", full_filename, buffer_persist_filename);
            }
            else
            {
                if (alphabet_persisted_list != NULL)
                {
                    char* dup_persisted_filename = (char*)malloc(strlen(buffer_persist_filename) + 1);
                    if (dup_persisted_filename == NULL) {
                        fatal_alloc_error("install_ling_resource_package");
                    }
                    strcpy(dup_persisted_filename, buffer_persist_filename);
                    *(alphabet_persisted_list + nb_persisted_alphabet) = dup_persisted_filename;
                    *(alphabet_persisted_list + nb_persisted_alphabet + 1) = NULL;
                }
                nb_persisted_alphabet++;
            }
        }
    }

    if (file_list != NULL)
    {
        *file_list = files_persisted_list;
    }

    if (persist_graph_list != NULL)
    {
        *persist_graph_list = graph_persisted_list;
    }

    if (persist_dictionary_list != NULL)
    {
        *persist_dictionary_list = dictionary_persisted_list;
    }

    if (persist_alphabet_list != NULL)
    {
        *persist_alphabet_list = alphabet_persisted_list;
    }

    freeFileInPackArchiveListFile(fileListArchive);

    free(full_filename);
    free(buffer_persist_filename);

    return result;
}


UNITEX_FUNC int UNITEX_CALL InstallLingResourcePackage(const char* package_name, const char* prefix_destination,
    int folder_separator_transformation,
    int persist_file, int persist_graph, int persist_dictionary, int persist_alphabet,
    char*** file_list, char*** persist_graph_list, char*** persist_dictionary_list, char*** persist_alphabet_list)
{
    return install_ling_resource_package(package_name, prefix_destination,
        folder_separator_transformation,
        persist_file, persist_graph, persist_dictionary, persist_alphabet,
        file_list, persist_graph_list, persist_dictionary_list, persist_alphabet_list);
}


int uninstall_ling_resource_package(const char* package_name, const char* prefix_destination,
    int folder_separator_transformation,
    int persist_file, int persist_graph, int persist_dictionary, int persist_alphabet)
{


    int result = 1;
    int transform_path_separator = 0;

    //int quiet = 1;
    //int opt_extract_without_path = 0;

    char** fileListArchive = createFileInPackArchiveListFileWithTransformPathSeparator(package_name, transform_path_separator);
    unsigned int nb_file_archive = countFileInPackArchiveListFile(fileListArchive);
    if (nb_file_archive == 0)
    {
        error("found no file in package %s\n", fileListArchive);
        return 0;
    }



    size_t max_len_filename = 1;
    for (unsigned int i = 0; i < nb_file_archive; i++)
    {
        size_t len = strlen(*(fileListArchive + i));
        if (len > max_len_filename)
            max_len_filename = len;
    }

    size_t len_prefix = strlen(prefix_destination);
    size_t size_buffer_persist_filename = max_len_filename + len_prefix + 0x10;

    char* full_filename = (char*)malloc(sizeof(char) * (size_buffer_persist_filename + 1));

    if (full_filename == NULL)
    {
        fatal_alloc_error("uninstall_ling_resource_package");
    }

    for (unsigned int i = 0; i < nb_file_archive; i++)
    {
        const char* curFileNameInPackage = *(fileListArchive + i);
        strcpy(full_filename, prefix_destination);
        strcpy(full_filename + len_prefix, curFileNameInPackage);
        transform_fileName_separator(full_filename + len_prefix, folder_separator_transformation);


        if (persist_graph && is_filename_graph(full_filename))
        {
            standard_unload_persistence_fst2(full_filename);
        }

        if (persist_dictionary && is_filename_dictionary(full_filename))
        {
            standard_unload_persistence_dictionary(full_filename);
        }

        if (persist_alphabet && is_filename_alphabet(full_filename))
        {
            standard_unload_persistence_alphabet(full_filename);
        }
    }

    // we remove the file only after unload all persisted file.
    // we don't want unload a dictionary after delete the .inf
    if (persist_file)
    {
        for (unsigned int i = 0; i < nb_file_archive; i++)
        {
            const char* curFileNameInPackage = *(fileListArchive + i);
            strcpy(full_filename, prefix_destination);
            strcpy(full_filename + len_prefix, curFileNameInPackage);
            transform_fileName_separator(full_filename + len_prefix, folder_separator_transformation);

            if (af_remove(full_filename) != 0)
				if (!is_filename_end_directory_separator(full_filename))
				{
					error("error in remove file %s\n", full_filename);
				}
        }
    }

    freeFileInPackArchiveListFile(fileListArchive);
    free(full_filename);

    return result;

}

UNITEX_FUNC int UNITEX_CALL UninstallLingResourcePackage(const char* package_name, const char* prefix_destination,
    int folder_separator_transformation,
    int persist_file, int persist_graph, int persist_dictionary, int persist_alphabet)
{
    return uninstall_ling_resource_package(package_name, prefix_destination,
        folder_separator_transformation,
        persist_file, persist_graph, persist_dictionary, persist_alphabet);
}

int uninstall_ling_resource_package_by_list(char** file_list,
    char** persist_graph_list, char** persist_dictionary_list, char** persist_alphabet_list)
{
    int result = 1;
    if (persist_graph_list != NULL)
    {
        int i = 0;
        while (*(persist_graph_list + i) != NULL)
        {
            standard_unload_persistence_fst2(*(persist_graph_list + i));
            i++;
        }
    }

    if (persist_dictionary_list != NULL)
    {
        int i = 0;
        while (*(persist_dictionary_list + i) != NULL)
        {
            standard_unload_persistence_dictionary(*(persist_dictionary_list + i));
            i++;
        }
    }

    if (persist_alphabet_list != NULL)
    {
        int i = 0;
        while (*(persist_alphabet_list + i) != NULL)
        {
            standard_unload_persistence_alphabet(*(persist_alphabet_list + i));
            i++;
        }
    }

    if (file_list != NULL)
    {
        int i = 0;
        while (*(file_list + i) != NULL)
        {
			const char* remove_filename = *(persist_alphabet_list + i);
            if (af_remove(remove_filename) != 0)
				if (!is_filename_end_directory_separator(remove_filename))
				{
					error("error in remove file %s\n", remove_filename);
					result = 0;
				}
            i++;
        }
    }

    return result;
}


UNITEX_FUNC int UNITEX_CALL UninstallLingResourcePackagByList(char** file_list,
    char** persist_graph_list, char** persist_dictionary_list, char** persist_alphabet_list)
{
    return uninstall_ling_resource_package_by_list(file_list,
        persist_graph_list, persist_dictionary_list, persist_alphabet_list);
}


static unsigned int count_list(char** archiveListFile)
{
    unsigned int i = 0;
	if (archiveListFile == NULL)
		return 0;

    while ((*(archiveListFile + i)) != NULL)
    {
        i++;
    }
    return i;
}


static void free_list(char** archiveListFile)
{
    unsigned int i = 0;
	if (archiveListFile == NULL)
		return;

    while ((*(archiveListFile + i)) != NULL)
    {
        free(*(archiveListFile + i));
        i++;
    }
    free(archiveListFile);
}


static const char* access_item_in_list(char** list, unsigned int pos)
{
    return *(list + pos);
}

void free_installed_resource_package_list(char** archiveListFile)
{
    free_list(archiveListFile);
}

UNITEX_FUNC void UNITEX_CALL FreeInstalledResourcePackageList(char** l)
{
    return free_installed_resource_package_list(l);
}


void count_items_in_installed_resource_package_list(char** list)
{
    count_list(list);
}

UNITEX_FUNC void UNITEX_CALL CountItemsInInstalledResourcePackageList(char** list)
{
    count_items_in_installed_resource_package_list(list);
}

const char* access_item_in_installed_resource_package_list(char** list, unsigned int pos)
{
    return access_item_in_list(list, pos);
}

UNITEX_FUNC const char* UNITEX_CALL AccessItemInInstalledResourcePackageList(char** list, unsigned int pos)
{
    return access_item_in_installed_resource_package_list(list, pos);
}




/////////////////////////////////////////////


UNITEX_FUNC int UNITEX_CALL WriteFilesListToFile(const char* filename_out, char** list)
{
    return write_files_list_to_file(filename_out, list);
}

int write_files_list_to_file(const char* filename_out, char** list)
{
    ABSTRACTFILE *fileout = af_fopen(filename_out, "wb");

    if (fileout == 0)
        return 0;
    int i = 0;
    while ((*(list + i)) != NULL)
    {
        af_fwrite((*(list + i)), strlen((*(list + i))),1, fileout);
        af_fwrite("\n",1, 1,fileout);
        i++;
    }
    af_fclose(fileout);

    return 1;
}


static void resize_char_multiarray(int nb_entry_needed, int* nb_entry, char***array)
{
    if (nb_entry_needed < (*nb_entry))
        return;

    if ((*array) == NULL)
    {
        int new_nb_entry = 0x10;
        *array = (char**)malloc(sizeof(char*) * new_nb_entry);
        if ((*array) == NULL)
        {
            fatal_alloc_error("resize_char_multiarray");
        }
        *nb_entry = new_nb_entry;
    }
    else
    {
        int new_nb_entry = (*nb_entry) * 2;
        *array = (char**)realloc(*array, sizeof(char*) * new_nb_entry);
        if ((*array) == NULL)
        {
            fatal_alloc_error("resize_char_multiarray");
        }
        *nb_entry = new_nb_entry;
    }
}



char** read_list_files_from_file(const char* filein_name)
{
	if (filein_name == NULL)
		return NULL;

	if ((*filein_name) == '\0')
		return NULL;

    ABSTRACTMAPFILE* af_map_text = af_open_mapfile(filein_name, MAPFILE_OPTION_READ, 0);
    const char* ptr = (const char*)af_get_mapfile_pointer(af_map_text);

    size_t size_text = (af_get_mapfile_size(af_map_text) / sizeof(char));

    size_t pos = 0;

    int nb_entry_list = 0;
    int nb_entry_list_allocated = 0;
    char** list = NULL;
    resize_char_multiarray(nb_entry_list + 1, &nb_entry_list_allocated, &list);
    *list = NULL;


    while (pos < size_text)
    {
        char c = *(ptr + pos);
        if (c == '\0')
            break;
        if ((c == '\r') || (c == '\n'))
        {
            pos++;
            continue;
        }

        size_t size_line = 0;

        while ((pos + size_line) < size_text)
        {
            char c2 = *(ptr + pos + size_line);
            if ((c2 == '\0') || (c2 == '\r') || (c2 == '\n'))
                break;
            size_line++;
        }

        if (size_line > 0)
        {
            char* new_line_content = (char*)malloc(size_line + 1);

            if (new_line_content == NULL)
            {
                fatal_alloc_error("resize_char_multiarray");
            }
            memcpy(new_line_content, ptr + pos, size_line);
            *(new_line_content + size_line) = '\0';


            resize_char_multiarray(nb_entry_list + 2, &nb_entry_list_allocated, &list);
            *(list + nb_entry_list) = new_line_content;
            *(list + nb_entry_list + 1) = NULL;
            nb_entry_list++;

            pos += size_line;
        }
    }

    af_release_mapfile_pointer(af_map_text, ptr);
    af_close_mapfile (af_map_text);

	return list;
}


UNITEX_FUNC char** UNITEX_CALL ReadListFilesFromFile(const char* filein_name)
{
    return read_list_files_from_file(filein_name);
}


void free_list_files_from_file(char**list)
{
    return free_installed_resource_package_list(list);
}


UNITEX_FUNC void UNITEX_CALL FreeListFilesFromFile(char** list)
{
    return free_list_files_from_file(list);
}

unsigned int count_items_in_list_files_from_file(char** list)
{
    return count_list(list);
}

UNITEX_FUNC unsigned int UNITEX_CALL CountItemsInListFilesFromFile(char** list)
{
    return count_items_in_list_files_from_file(list);
}

const char* access_item_in_list_files_from_file(char** list, unsigned int pos)
{
    return access_item_in_list(list, pos);
}

UNITEX_FUNC const char* UNITEX_CALL AccessItemInListFilesFromFile(char** list, unsigned int pos)
{
    return access_item_in_list_files_from_file(list, pos);
}

}
}
#endif
