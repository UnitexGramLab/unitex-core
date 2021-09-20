#ifndef NO_UNITEX_LOGGER



#ifndef _UNPACK_FILE_TOOLS_H
#define _UNPACK_FILE_TOOLS_H 1


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



#define UNPACKFILE_LIST_FOLDER_SEPARATOR_TRANSFORMATION_UNMODIFIED (0)
#define UNPACKFILE_LIST_FOLDER_SEPARATOR_TRANSFORMATION_PLATFORM   (1)
#define UNPACKFILE_LIST_FOLDER_SEPARATOR_TRANSFORMATION_UNIX       (2)
#define UNPACKFILE_LIST_FOLDER_SEPARATOR_TRANSFORMATION_WINDOWS    (3)


void transform_fileName_separator(char *filename, int transform_path_separator);

UNITEX_FUNC void UNITEX_CALL TransformFileNameSeparator(char *filename, int transform_path_separator);


char** createFileInPackArchiveListFile(const char* packFileName);

char** createFileInPackArchiveListFileWithTransformPathSeparator(const char* packFileName, int transform_path_separator);

void freeFileInPackArchiveListFile(char** archiveListFile);

unsigned int countFileInPackArchiveListFile(char** archiveListFile);


int do_list_file_in_pack_archive(const char* packFileName);

int do_list_file_in_pack_archive_to_file(const char* packFileName, const char* filename_out, int filename_only);

UNITEX_FUNC int UNITEX_CALL WriteListFileInPackArchiveToFile(const char* packFileName, const char* filename_out, int filename_only);

int do_extract_from_pack_archive_onefile(
    const char* packFileName,
    int opt_extract_without_path,
    const char* prefix_extracting_name,
    const char* filename_to_extract,
    int transform_path_separator, int quiet);

int do_extract_from_pack_archive(
    const char* packFileName,
    int opt_extract_without_path,
    const char* prefix_extracting_name,
    int transform_path_separator, int quiet);



UNITEX_FUNC char** UNITEX_CALL GetListOfFileInUnitexArchive(const char* packFileName);

UNITEX_FUNC char** UNITEX_CALL GetListOfFileInUnitexArchiveWithTransformPathSeparator(const char* packFileName, int transform_path_separator);

UNITEX_FUNC unsigned int UNITEX_CALL GetFilesNumberInListOfFileInUnitexArchive(char** archiveListFile);

UNITEX_FUNC void UNITEX_CALL ReleaseListOfFileInUnitexArchive(char** archiveListFile);


UNITEX_FUNC int UNITEX_CALL WriteListFileInPackArchiveToFile(const char* packFileName, const char* filename_out, int filename_only);

UNITEX_FUNC int UNITEX_CALL ExtractOneFileFromUnitexArchive(
    const char* packFileName,
    int opt_extract_without_path,
    const char* prefix_extracting_name,
    const char* filename_to_extract,
    int transform_path_separator);



UNITEX_FUNC int UNITEX_CALL ExtractFilesFromUnitexArchive(
    const char* packFileName,
    int opt_extract_without_path,
    const char* prefix_extracting_name,
    int transform_path_separator);


#ifdef __cplusplus
} // extern "C"
} // namespace logger
} // namespace unitex
#endif

#endif

#endif
